/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_element_avtp_source.h"
#include "audio_element.h"
#include "audio_format.h"
#include "avb_tsn/genavb.h"
#include "genavb/genavb.h"
#include "mailbox.h"
#include "hlog.h"
#include "hrpn_ctrl.h"

#include "os/semaphore.h"

/*
 * AVTP source: AVB audio stream listener
 *
 * Read from AVTP_RX_STREAM_N streams, with AVTP_RX_CHANNEL_N channels.
 */

struct avtp_output {
	struct audio_buffer *buf;
};

struct avtp_stream {
	unsigned int id;
	unsigned int connected;

	struct genavb_stream_handle *handle;
	unsigned int batch_size_ns;
	unsigned int cur_batch_size;

	struct audio_buffer *channel_buf[AVTP_RX_CHANNEL_N];	/* output audio buffer address */

	unsigned int err;
	unsigned int underflow;
	unsigned int overflow;
	unsigned int received;

	unsigned int count;		/* small logic that avoid reading */
	unsigned int until;		/* data when stream in underflow. */

	bool convert;
	bool invert;			/* format conversion */
	unsigned int shift;		/* format conversion */
	unsigned int mask;		/* format conversion */
};

struct avtp_source_element {
	unsigned int stream_n;
	struct avtp_stream stream[AVTP_RX_STREAM_N];
	unsigned int out_n;
	struct avtp_output out[AVTP_RX_STREAM_N * AVTP_RX_CHANNEL_N];

	/* used in the control path to protect all streams' ->connected states */
	os_sem_t semaphore;
};

static unsigned int avtp_source_stream_n()
{
	return AVTP_RX_STREAM_N;
}
static unsigned int avtp_source_channel_n(void)
{
	return AVTP_RX_CHANNEL_N;
}
static unsigned int avtp_source_out_n(void)
{
	return AVTP_RX_STREAM_N * AVTP_RX_CHANNEL_N;
}

static void avtp_source_connect(struct avtp_source_element *avtp, unsigned int stream_index, struct genavb_stream_params *params, struct audio_element *element)
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

	if (avdecc_fmt_channels_per_sample(&params->format) > avtp_source_channel_n()) {
		log_err("unsupported number of channels: %u > %u\n",
				avdecc_fmt_channels_per_sample(&params->format), avtp_source_channel_n());

		goto exit;
	}

	if (avdecc_fmt_sample_rate(&params->format) != element->sample_rate) {
		log_err("unsupported sample rate: %u != %u\n", avdecc_fmt_sample_rate(&params->format), element->sample_rate);

		goto exit;
	}

	stream->convert = true;
	stream->invert = invert;
	stream->shift = shift;
	stream->mask = mask;

	cur_batch_size = element->period * avdecc_fmt_sample_size(&params->format);
	stream->cur_batch_size = cur_batch_size;

	if (element->period < avdecc_fmt_samples_per_packet(&params->format, params->stream_class))
		cur_batch_size = avdecc_fmt_samples_per_packet(&params->format, params->stream_class) * avdecc_fmt_sample_size(&params->format);
	else
		cur_batch_size = stream->cur_batch_size;

	params->flags = 0; /* disable media clock recovery */

	/* Create new AVTP stream, update stream_handle */
	if ((avb_result = genavb_stream_create(handle, &stream->handle, params, &cur_batch_size, 0)) != GENAVB_SUCCESS) {
		log_err("genavb_stream_create() failed: %s\n", genavb_strerror(avb_result));
		stream->cur_batch_size = 0;

		goto exit;
	}
	log_info("  stream batch size: %u\n", stream->cur_batch_size);
	log_info("  batch size: %u\n", cur_batch_size);

	stream->count = 0;
	stream->until = 1;

	os_sem_take(&avtp->semaphore, 0, OS_SEM_TIMEOUT_MAX);
	stream->connected = 1;
	os_sem_give(&avtp->semaphore, 0);

exit:
	return;
}

static void avtp_source_disconnect(struct avtp_source_element *avtp, unsigned int stream_index)
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

static void avtp_source_element_response(struct mailbox *m, uint32_t status)
{
	struct hrpn_resp_audio_element resp;

	if (m) {
		resp.type = HRPN_RESP_TYPE_AUDIO_ELEMENT_AVTP;
		resp.status = status;
		mailbox_resp_send(m, &resp, sizeof(resp));
	}
}

int avtp_source_element_ctrl(struct audio_element *element, struct hrpn_cmd_audio_element_avtp *cmd, unsigned int len, struct mailbox *m)
{
	struct avtp_source_element *avtp;

	if (!element)
		goto err;

	if (element->type != AUDIO_ELEMENT_AVTP_SOURCE)
		goto err;

	avtp = element->data;

	switch (cmd->u.common.type) {
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SOURCE_CONNECT:

		if ((len != sizeof(struct hrpn_cmd_audio_element_avtp_connect)))
			goto err;

		if (cmd->u.connect.stream_index >= avtp->stream_n)
			goto err;

		avtp_source_connect(avtp, cmd->u.connect.stream_index, &cmd->u.connect.stream_params, element);

		break;

	case HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SOURCE_DISCONNECT:

		if ((len != sizeof(struct hrpn_cmd_audio_element_avtp_disconnect)))
			goto err;

		if (cmd->u.disconnect.stream_index >= avtp->stream_n)
			goto err;

		avtp_source_disconnect(avtp, cmd->u.disconnect.stream_index);

		break;

	default:
		goto err;
		break;
	}

	avtp_source_element_response(m, HRPN_RESP_STATUS_SUCCESS);

	return 0;

err:
	avtp_source_element_response(m, HRPN_RESP_STATUS_ERROR);

	return -1;
}

static int listener_receive(struct avtp_source_element *avtp, unsigned int stream_index, unsigned int period)
{
	struct avtp_stream *stream = &avtp->stream[stream_index];
	int i, j, k;
	int read_bytes = 0;
#define PERIOD_MAX	32 /* TODO - use genavbstream_listener_receive() directly */
	uint32_t data[AVTP_RX_CHANNEL_N * PERIOD_MAX];
	int ret = -1;

	if (stream->count < stream->until) {
		/* tempo to accumulate enough data on the AVB network */
		stream->count++;

		goto exit;
	}

	read_bytes = genavb_stream_receive(stream->handle, &data, stream->cur_batch_size, NULL, NULL);

	if (read_bytes < 0) {
		stream->err++;
		stream->count = 0;

		goto exit;

	} else if (read_bytes < stream->cur_batch_size) {
		stream->underflow++;
		stream->count = 0;

		goto exit;
	}

	stream->received++;

	k = 0;
	for (j = 0; j < period; j++) {

		for (i = 0; i < avtp_source_channel_n(); i++){
			__audio_buf_write_uint32(stream->channel_buf[i], j, data[k]);
			k++;
		}
	}

	if (stream->convert) {
		for (i = 0; i < avtp_source_channel_n(); i++)
			audio_convert_from(audio_buf_write_addr(stream->channel_buf[i], 0), period, stream->invert, stream->mask, stream->shift);
	}

	ret = 0;

exit:
	return ret;
}

static int avtp_source_element_run(struct audio_element *element)
{
	struct avtp_source_element *avtp = element->data;
	struct avtp_stream *stream;
	int i, j;
	int ret;

	os_sem_take(&avtp->semaphore, 0, OS_SEM_TIMEOUT_MAX);

	for (i = 0; i < avtp->stream_n; i++) {
		stream = &avtp->stream[i];

		if (stream->connected)
			ret = listener_receive(avtp, i, element->period);
		else
			ret = -1;

		if (!ret) {
			for (j = 0; j < avtp_source_channel_n()	; j++)
				audio_buf_write_update(stream->channel_buf[j], element->period);
		} else {
			for (j = 0; j < avtp_source_channel_n()	; j++)
				audio_buf_write_silence(stream->channel_buf[j], element->period);
		}
	}

	os_sem_give(&avtp->semaphore, 0);

	return 0;
}

static void avtp_source_element_reset(struct audio_element *element)
{
	struct avtp_source_element *avtp = element->data;
	uint32_t data[AVTP_RX_CHANNEL_N * PERIOD_MAX];
	struct avtp_stream *stream;
	int read_bytes = 0;
	int i;

	for (i = 0; i < avtp->stream_n; i++) {
		stream = &avtp->stream[i];
		stream->count = 0;
		do {
			if (stream->handle)
				read_bytes = genavb_stream_receive(stream->handle, &data, stream->cur_batch_size, NULL, NULL);
		} while (read_bytes);
	}

	for (i = 0; i < avtp->out_n; i++)
		audio_buf_reset(avtp->out[i].buf);
}

static void avtp_source_element_exit(struct audio_element *element)
{
	struct avtp_source_element *avtp = element->data;
	int i;

	for (i = 0; i < avtp->stream_n; i++)
		avtp_source_disconnect(avtp, i);
}

static void avtp_source_element_dump(struct audio_element *element)
{
	struct avtp_source_element *avtp = element->data;
	int i;

	log_info("avtp source(%p/%p)\n", avtp, element);

	for (i = 0; i < avtp->stream_n; i++)
		log_info("stream %d %sconnected\n",
			i, avtp->stream[i].connected ? "" : "dis");

	for (i = 0; i < avtp->out_n; i++)
		audio_buf_dump(avtp->out[i].buf);
}

static void avtp_source_element_stats(struct audio_element *element)
{
	struct avtp_source_element *avtp = element->data;
	int i;

	for (i = 0; i < avtp->stream_n; i++) {
		log_info("rx stream: %u, avtp(%p, %u)\n",
			 i, avtp->stream[i].handle, avtp->stream[i].id);

		log_info("  connected: %u\n",
			avtp->stream[i].connected);
		log_info("  batch size: %u\n",
			avtp->stream[i].cur_batch_size);
		log_info("  underflow: %u, overflow: %u err: %u received: %u\n",
			avtp->stream[i].underflow, avtp->stream[i].overflow,
			avtp->stream[i].err, avtp->stream[i].received);
	}
}

int avtp_source_element_check_config(struct audio_element_config *config)
{
	if (config->inputs) {
		log_err("avtp source: invalid inputs: %u\n", config->inputs);
		goto err;
	}

	if (config->outputs != avtp_source_out_n()) {
		log_err("avtp source: invalid outputs: %u\n", config->outputs);
		goto err;
	}

	if (config->u.avtp_source.stream_n > avtp_source_stream_n()) {
		log_err("number of streams not supported: %u != %u\n",
			       config->u.avtp_source.stream_n, avtp_source_stream_n());
		goto err;
	}

	return 0;

err:
	return -1;
}

unsigned int avtp_source_element_size(struct audio_element_config *config)
{
	return sizeof(struct avtp_source_element);
}

int avtp_source_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	struct avtp_source_element *avtp = element->data;
	int i, j, k;

	if (os_sem_init(&avtp->semaphore, 1))
		goto err;

	element->run = avtp_source_element_run;
	element->reset = avtp_source_element_reset;
	element->exit = avtp_source_element_exit;
	element->dump = avtp_source_element_dump;
	element->stats = avtp_source_element_stats;

	avtp->stream_n = config->u.avtp_source.stream_n;
	avtp->out_n = avtp_source_out_n();

	k = 0;
	for (i = 0; i < avtp->stream_n; i++) {
		avtp->stream[i].connected = 0;

		for (j = 0; j < avtp_source_channel_n()	; j++, k++) {
			avtp->stream[i].channel_buf[j] = &buffer[config->output[k]];
			avtp->stream[i].channel_buf[j]->input_type = element->type;
			avtp->stream[i].channel_buf[j]->input_element_id = element->element_id;
			avtp->stream[i].channel_buf[j]->input_id = k;
		}
	}

	for (i = 0; i < avtp->out_n; i++)
		avtp->out[i].buf = &buffer[config->output[i]];

	avtp_source_element_dump(element);

	return 0;

err:
	return -1;
}
