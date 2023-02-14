/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_element_avtp_sink.h"
#include "audio_element.h"
#include "audio_format.h"
#include "avb_tsn/genavb.h"
#include "genavb/genavb.h"
#include "genavb/streaming.h"
#include "genavb/types.h"
#include "hlog.h"
#include "hrpn_ctrl.h"
#include "mailbox.h"

#include "os/semaphore.h"

/*
 * AVTP sink: AVB audio stream talker
 *
 * Write to AVTP_TX_STREAM_N streams, with AVTP_TX_CHANNEL_N channels.
 */

struct avtp_input {
	struct audio_buffer *buf;
};

struct avtp_stream {
	unsigned int id;
	unsigned int connected;

	struct genavb_stream_handle *handle;
	unsigned int batch_size_ns;
	unsigned int cur_batch_size;

	struct audio_buffer *channel_buf[AVTP_TX_CHANNEL_N]; /* input audio buffer address */

	unsigned int err;
	unsigned int underflow;
	unsigned int overflow;
	unsigned int sent;

	unsigned int count; /* small logic that avoid reading */
	unsigned int until; /* data when stream in underflow. */

	bool convert;
	bool invert;	    /* format conversion */
	unsigned int shift; /* format conversion */
	unsigned int mask;  /* format conversion */
};

struct avtp_sink_element {
	unsigned int stream_n;
	struct avtp_stream stream[AVTP_TX_STREAM_N];
	unsigned int in_n;
	struct avtp_input in[AVTP_TX_STREAM_N * AVTP_TX_CHANNEL_N];

	/* used in the control path to protect all streams' ->connected states */
	os_sem_t semaphore;
};

static unsigned int avtp_sink_stream_n()
{
	return AVTP_TX_STREAM_N;
}
static unsigned int avtp_sink_channel_n(void)
{
	return AVTP_TX_CHANNEL_N;
}
static unsigned int avtp_sink_in_n(void)
{
	return AVTP_TX_STREAM_N * AVTP_TX_CHANNEL_N;
}

static void avtp_sink_connect(struct avtp_sink_element *avtp, unsigned int stream_index, struct genavb_stream_params *params, unsigned int period)
{
	struct avtp_stream *stream = &avtp->stream[stream_index];
	struct genavb_handle *handle;
	int avb_result;
	unsigned int cur_batch_size;
	bool invert;
	uint32_t mask;
	unsigned int shift;

	if (stream->connected) {
		log_warn("stream already connected, exit.\n");

		goto exit;
	}

	handle = get_genavb_handle();
	if (!handle) {
		log_err("get_genavb_handle() failed: null genavb_handle\n");

		goto exit;
	}

	/* analyze params received from AVDECC */
	if (avdecc_format_is_aaf_pcm(&params->format)) {
		log_info("  audio format: AAF 2chans 24/32bits\n");
		invert = true;
		shift = 0;
		mask = 0xffffff00;
	} else if (avdecc_format_is_61883_6(&params->format)) {
		log_info("  audio format: 61883-6 AM824\n");
		invert = true;
		shift = 8;
		mask = 0x00ffffff;
	} else {
		log_err("unsupported format!\n");

		goto exit;
	}

	if (avdecc_fmt_channels_per_sample(&params->format) > avtp_sink_channel_n()) {
		log_err("unsupported number of channels: %u > %u\n", avdecc_fmt_channels_per_sample(&params->format), avtp_sink_channel_n());

		goto exit;
	}

	stream->convert = true;
	stream->invert = invert;
	stream->shift = shift;
	stream->mask = mask;

	cur_batch_size = period * avdecc_fmt_sample_size(&params->format);
	stream->cur_batch_size = cur_batch_size;

	params->flags = 0; /* disable media clock recovery */
	params->talker.latency = 500000;

	/* Create new AVTP stream, update stream_handle */
	if ((avb_result = genavb_stream_create(handle, &stream->handle, params, &cur_batch_size, 0)) != GENAVB_SUCCESS) {
		log_err("genavb_stream_create() failed: %s\n", genavb_strerror(avb_result));
		stream->cur_batch_size = 0;
		os_sem_give(&avtp->semaphore, 0);

		goto exit;
	}
	log_info("  batch size: %u\n", cur_batch_size);

	stream->count = 0;
	stream->until = 1;

	os_sem_take(&avtp->semaphore, 0, OS_SEM_TIMEOUT_MAX);
	stream->connected = 1;
	os_sem_give(&avtp->semaphore, 0);

exit:
	return;
}

static void avtp_sink_disconnect(struct avtp_sink_element *avtp, unsigned int stream_index)
{
	struct avtp_stream *stream = &avtp->stream[stream_index];
	int avb_result;

	if (!stream->connected) {
		goto exit;
	}

	os_sem_take(&avtp->semaphore, 0, OS_SEM_TIMEOUT_MAX);
	stream->connected = 0;
	os_sem_give(&avtp->semaphore, 0);

	avb_result = genavb_stream_destroy(stream->handle);
	if (avb_result != GENAVB_SUCCESS) {
		log_err("genavb_stream_destroy() failed: %s\n", genavb_strerror(avb_result));
	}

	stream->handle = NULL;

exit:
	return;
}

static void avtp_sink_element_response(struct mailbox *m, uint32_t status)
{
	struct hrpn_resp_audio_element resp;

	if (m) {
		resp.type = HRPN_RESP_TYPE_AUDIO_ELEMENT_AVTP;
		resp.status = status;
		mailbox_resp_send(m, &resp, sizeof(resp));
	}
}

int avtp_sink_element_ctrl(struct audio_element *element, struct hrpn_cmd_audio_element_avtp *cmd, unsigned int len, struct mailbox *m)
{
	struct avtp_sink_element *avtp;

	if (!element)
		goto err;

	if (element->type != AUDIO_ELEMENT_AVTP_SINK)
		goto err;

	avtp = element->data;

	switch (cmd->u.common.type) {
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SINK_CONNECT:

		if ((len != sizeof(struct hrpn_cmd_audio_element_avtp_connect)))
			goto err;

		if (cmd->u.connect.stream_index >= avtp->stream_n)
			goto err;
		avtp_sink_connect(avtp, cmd->u.connect.stream_index, &cmd->u.connect.stream_params, element->period);

		break;

	case HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SINK_DISCONNECT:

		if ((len != sizeof(struct hrpn_cmd_audio_element_avtp_disconnect)))
			goto err;

		if (cmd->u.disconnect.stream_index >= avtp->stream_n)
			goto err;

		avtp_sink_disconnect(avtp, cmd->u.disconnect.stream_index);

		break;

	default:
		goto err;
		break;
	}

	avtp_sink_element_response(m, HRPN_RESP_STATUS_SUCCESS);

	return 0;

err:
	avtp_sink_element_response(m, HRPN_RESP_STATUS_ERROR);

	return -1;
}

static int talker_send(struct avtp_sink_element *avtp, unsigned int stream_index, unsigned int period)
{
	struct avtp_stream *stream = &avtp->stream[stream_index];
	int ret = -1;
	int i, j, k;
	int write_total;

  if (stream->convert)
	  for (i = 0; i < avtp_sink_channel_n(); i++)
			audio_convert_to(audio_buf_read_addr(stream->channel_buf[i], 0), period, stream->invert, stream->mask, stream->shift);

#define PERIOD_MAX 32
	uint32_t data[AVTP_TX_CHANNEL_N * PERIOD_MAX] = { 0 }; /* dynamic size with period ? */
	k = 0;
	for (j = 0; j < period; j++) {
		for (i = 0; i < avtp_sink_channel_n(); i++) {
			data[k] = __audio_buf_read_uint32(stream->channel_buf[i], j);
			k++;
		}
	}

	write_total = genavb_stream_send(stream->handle, &data, stream->cur_batch_size, NULL, 0);

	if (write_total < 0) {
		stream->err++;
		ret = -1;
		goto exit;
	}
	if (write_total < (int)stream->cur_batch_size) {
		stream->overflow++;
		ret = -1;
		goto exit;
	}
	stream->sent++;

	ret = 0;
exit:
	return ret;
}

static int avtp_sink_element_run(struct audio_element *element)
{
	struct avtp_sink_element *avtp = element->data;
	struct avtp_stream *stream;
	int i, j;
	os_sem_take(&avtp->semaphore, 0, OS_SEM_TIMEOUT_MAX);
	for (i = 0; i < avtp->stream_n; i++) {
		stream = &avtp->stream[i];

		if (stream->connected)
			talker_send(avtp, i, element->period);

		for (j = 0; j < avtp_sink_channel_n(); j++)
			audio_buf_read_update(stream->channel_buf[j], element->period);
	}

	os_sem_give(&avtp->semaphore, 0);

	return 0;
}

static void avtp_sink_element_reset(struct audio_element *element)
{
	struct avtp_sink_element *avtp = element->data;
	struct avtp_stream *stream;
	struct avb_event event;
	int i, rc;

	event.event_mask = AVTP_FLUSH;
	event.index = 0;
	for (i = 0; i < avtp->stream_n; i++) {
		stream = &avtp->stream[i];
		if (stream->handle){
			rc = genavb_stream_send(stream->handle, NULL, 0, &event, 1);
			if (rc)
				log_err("genavb_stream flush failed !\n");
		}
	}

	for (i = 0; i < avtp->in_n; i++)
		audio_buf_reset(avtp->in[i].buf);
}

static void avtp_sink_element_exit(struct audio_element *element)
{
	struct avtp_sink_element *avtp = element->data;
	int i;

	for (i = 0; i < avtp->stream_n; i++)
		avtp_sink_disconnect(avtp, i);
}

static void avtp_sink_element_dump(struct audio_element *element)
{
	struct avtp_sink_element *avtp = element->data;
	int i;

	log_info("avtp sink(%p/%p)\n", avtp, element);

	for (i = 0; i < avtp->stream_n; i++)
		log_info("stream %d %sconnected\n", i, avtp->stream[i].connected ? "" : "dis");

	for (i = 0; i < avtp->in_n; i++)
		audio_buf_dump(avtp->in[i].buf);
}

static void avtp_sink_element_stats(struct audio_element *element)
{
	struct avtp_sink_element *avtp = element->data;
	int i;

	for (i = 0; i < avtp->stream_n; i++) {
		log_info("tx stream: %u, avtp(%p, %u)\n", i, avtp->stream[i].handle, avtp->stream[i].id);

		log_info("  connected: %u\n", avtp->stream[i].connected);
		log_info("  batch size: %u\n", avtp->stream[i].cur_batch_size);
		log_info("  underflow: %u, overflow: %u err: %u sent: %u\n", avtp->stream[i].underflow, avtp->stream[i].overflow, avtp->stream[i].err, avtp->stream[i].sent);
	}
}

int avtp_sink_element_check_config(struct audio_element_config *config)
{
	if (config->outputs) {
		log_err("avtp sink: invalid outputs: %u\n", config->inputs);
		goto err;
	}

	if (config->inputs != avtp_sink_in_n()) {
		log_err("avtp sink: invalid inputs: %u\n", config->inputs);
		goto err;
	}

	if (config->u.avtp_sink.stream_n > avtp_sink_stream_n()) {
		log_err("number of streams not supported: %u != %u\n",
			       config->u.avtp_sink.stream_n, avtp_sink_stream_n());
		goto err;
	}

	return 0;

	err:
		return -1;
}

unsigned int avtp_sink_element_size(struct audio_element_config *config)
{
	return sizeof(struct avtp_sink_element);
}

int avtp_sink_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	struct avtp_sink_element *avtp = element->data;
	int i, j, k;

	if (os_sem_init(&avtp->semaphore, 1))
		goto err;

	element->run = avtp_sink_element_run;
	element->reset = avtp_sink_element_reset;
	element->exit = avtp_sink_element_exit;
	element->dump = avtp_sink_element_dump;
	element->stats = avtp_sink_element_stats;

	avtp->stream_n = config->u.avtp_sink.stream_n;
	avtp->in_n = avtp_sink_in_n();

	k = 0;
	for (i = 0; i < avtp->stream_n; i++) {
		avtp->stream[i].connected = 0;

		for (j = 0; j < avtp_sink_channel_n(); j++, k++)
			avtp->stream[i].channel_buf[j] = &buffer[config->input[k]];
	}

	for (i = 0; i < avtp->in_n; i++)
		avtp->in[i].buf = &buffer[config->input[i]];

	avtp_sink_element_dump(element);

	return 0;

err:
	return -1;
}
