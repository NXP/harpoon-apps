/*
 * Copyright 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <math.h>
#include "audio_app.h"
#include "audio_element_avtp_source.h"
#include "audio_element.h"
#include "audio_format.h"
#include "audio_pipeline.h"
#include "avb_tsn/clock_domain.h"
#include "avb_tsn/crf_stream.h"
#include "avb_tsn/genavb.h"
#include "avb_tsn/media_clock.h"
#include "genavb/clock.h"
#include "genavb/control_clock_domain.h"
#include "genavb/genavb.h"
#include "genavb/sr_class.h"
#include "hlog.h"
#include "hrpn_ctrl.h"

#include "rtos_abstraction_layer.h"
#include "types.h"

/*
 * AVTP source: AVB audio stream listener
 *
 * Read from AVTP_RX_STREAM_N streams, with AVTP_RX_CHANNEL_N channels.
 */

struct avtp_output {
	struct audio_buffer *buf;
};

struct crf_stream {
	aar_crf_stream_t stream;
	unsigned int connected;
	int index;
};

struct avtp_stream {
	unsigned int id;
	unsigned int connected;
	unsigned int connection_flags;
	unsigned int sr_class;

	struct genavb_stream_handle *handle;
	unsigned int batch_size_ns;
	unsigned int cur_batch_size;
	unsigned int sample_size;

	struct audio_buffer *channel_buf[AVTP_RX_CHANNEL_N];	/* output audio buffer address */

	unsigned int err;
	unsigned int clock_err;
	unsigned int sync_err;
	unsigned int ts_err;
	unsigned int event_err;
	unsigned int start_underflow;
	unsigned int underflow;
	unsigned int overflow;
	unsigned int received;

	bool first_start;
	float sample_dt;

	bool convert;
	bool invert;			/* format conversion */
	unsigned int shift;		/* format conversion */
	unsigned int mask;		/* format conversion */
};

struct avtp_source_element {
	unsigned int stream_n;
	struct avtp_stream stream[AVTP_RX_STREAM_N];
	struct crf_stream crf_stream;
	unsigned int out_n;
	struct avtp_output out[AVTP_RX_STREAM_N * AVTP_RX_CHANNEL_N];

	genavb_clock_domain_t clock_domain;

	/* used in the control path to protect all streams' ->connected states */
	rtos_mutex_t mutex;
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

static void crf_listener_disconnect(struct avtp_source_element *avtp)
{
	struct crf_stream *crf_stream = &avtp->crf_stream;

	if (!crf_stream->connected)
		goto exit;

	crf_disconnect(&crf_stream->stream);
	crf_stream->connected = 0;
	crf_stream->index = -1;

	log_info("disconnected\n");

exit:
	return;
}

static void crf_listener_connect(struct avtp_source_element *avtp, unsigned int stream_index, struct genavb_stream_params *params, struct audio_element *element)
{
	struct crf_stream *crf_stream = &avtp->crf_stream;

	if (crf_stream->connected) {
		log_warn("stream already connected, exit.\n");

		goto exit;
	}

	if (!crf_connect(&crf_stream->stream, MEDIA_CLOCK_SLAVE, avtp->clock_domain, params)) {
		avtp->crf_stream.index = stream_index;
		crf_stream->connected = 1;
		log_info("connected, clock domain: %d\n", avtp->clock_domain);
	} else {
		log_err("connection failed\n");
	}

exit:
	return;
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

	stream->sample_dt = ((float)NSECS_PER_SEC) / (float)element->sample_rate;
	stream->sample_size = avdecc_fmt_sample_size(&params->format);
	cur_batch_size = element->period * stream->sample_size;
	stream->cur_batch_size = cur_batch_size;

	if (element->period < avdecc_fmt_samples_per_packet(&params->format, params->stream_class))
		cur_batch_size = avdecc_fmt_samples_per_packet(&params->format, params->stream_class) * avdecc_fmt_sample_size(&params->format);
	else
		cur_batch_size = stream->cur_batch_size;

	params->flags = stream->connection_flags;
	stream->sr_class = params->stream_class;

	if (avtp->clock_domain != GENAVB_CLOCK_DOMAIN_DEFAULT) {
		params->clock_domain = avtp->clock_domain;

		/* Before connecting any AVTP stream, check if we need to set the clock domain source for AVB_CLOCK_DOMAIN_0 */
		if (init_media_clock_source(&avtp->crf_stream.stream, params->clock_domain, params) < 0) {
			log_err("init_media_clock_source() failed for domain %d, can not connect stream input (%u)\n", avtp->clock_domain, stream_index);
			return;
		}
	}

	/* Create new AVTP stream, update stream_handle */
	if ((avb_result = genavb_stream_create(handle, &stream->handle, params, &cur_batch_size, 0)) != GENAVB_SUCCESS) {
		log_err("genavb_stream_create() failed: %s\n", genavb_strerror(avb_result));
		stream->cur_batch_size = 0;

		goto exit;
	}
	log_info("  stream batch size: %u\n", stream->cur_batch_size);
	log_info("  batch size: %u\n", cur_batch_size);

	stream->first_start = true;

	rtos_mutex_lock(&avtp->mutex, RTOS_WAIT_FOREVER);
	stream->connected = 1;
	rtos_mutex_unlock(&avtp->mutex);

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

	rtos_mutex_lock(&avtp->mutex, RTOS_WAIT_FOREVER);
	stream->connected = 0;
	rtos_mutex_unlock(&avtp->mutex);

	avb_result = genavb_stream_destroy(stream->handle);
	if (avb_result != GENAVB_SUCCESS) {
		log_err("genavb_stream_destroy() failed: %s\n", genavb_strerror(avb_result));
	}

	stream->handle = NULL;

exit:
	return;
}

static void avtp_source_element_response(void *ctrl_handle, uint32_t status)
{
	struct hrpn_resp_audio_element resp;

	if (ctrl_handle) {
		resp.type = HRPN_RESP_TYPE_AUDIO_ELEMENT_AVTP;
		resp.status = status;
		audio_app_ctrl_send(ctrl_handle, &resp, sizeof(resp));
	}
}

int avtp_source_element_ctrl(struct audio_element *element, struct hrpn_cmd_audio_element_avtp *cmd, unsigned int len, void *ctrl_handle)
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

		if (cmd->u.connect.stream_index >= avtp->stream_n + 1)
			goto err;

		/* FIXME fix mapping, as stream_index is for both CRF and AVTP streams, and that it is not mapped
		 * with the avtp_stream array
		 */
		if (avdecc_format_is_crf(&cmd->u.connect.stream_params.format))
			crf_listener_connect(avtp, cmd->u.connect.stream_index, &cmd->u.connect.stream_params, element);
		else
			avtp_source_connect(avtp, cmd->u.connect.stream_index, &cmd->u.connect.stream_params, element);

		break;

	case HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SOURCE_DISCONNECT:

		if ((len != sizeof(struct hrpn_cmd_audio_element_avtp_disconnect)))
			goto err;

		if (cmd->u.disconnect.stream_index >= avtp->stream_n + 1)
			goto err;

		if ((int)cmd->u.disconnect.stream_index == avtp->crf_stream.index)
			crf_listener_disconnect(avtp);
		else
			avtp_source_disconnect(avtp, cmd->u.disconnect.stream_index);

		break;

	default:
		goto err;
		break;
	}

	avtp_source_element_response(ctrl_handle, HRPN_RESP_STATUS_SUCCESS);

	return 0;

err:
	avtp_source_element_response(ctrl_handle, HRPN_RESP_STATUS_ERROR);

	return -1;
}

/* 1ms playback offset to cover: the worst case stack max processing time (500us) + worst case delay caused by misalignment
 * between the application batch and the stack batch.
 */
#define  GENAVB_PROCESSING_TIME_NS		1000000U
static int listener_timestamp_accept(unsigned int ts, unsigned int now, unsigned int period, unsigned int sample_rate, unsigned int sr_class)
{
	/* Timestamp + playback offset must be after now (otherwise packet are too late) */
	/* Timestamp must be before now + transit time + timing uncertainty (otherwise they arrived too early) */
	if (avtp_after(ts + GENAVB_PROCESSING_TIME_NS, now)
	&& avtp_before(ts, now + sr_class_max_transit_time(sr_class)
				+ sr_class_max_timing_uncertainty(sr_class)))
		return 1;

	return 0;
}

static int listener_receive(struct avtp_source_element *avtp, unsigned int stream_index, unsigned int period, unsigned int sample_rate)
{
	struct avtp_stream *stream = &avtp->stream[stream_index];
	int i, j, k;
	int read_bytes = 0;
#define PERIOD_MAX	32 /* TODO - use genavbstream_listener_receive() directly */
	uint32_t data[AVTP_RX_CHANNEL_N * PERIOD_MAX];
	int ret = -1;

	if (stream->first_start) {
#define  MAX_EVENTS		12
		struct genavb_event event[MAX_EVENTS] = {0};
		struct genavb_event *event_ts = NULL;
		unsigned int event_len = MAX_EVENTS;
		float tmp_float;
		uint32_t periods_to_wait;
		uint64_t now;
		unsigned int ts;
		int idx;

		read_bytes = genavb_stream_receive(stream->handle, &data, stream->cur_batch_size, event, &event_len);

		if (read_bytes < 0) {
			stream->sync_err++;
			stream->first_start = true;

			goto exit;

		} else if (read_bytes < stream->cur_batch_size) {
			stream->start_underflow++;
			stream->first_start = true;

			goto exit;
		}

		if (genavb_clock_gettime64(GENAVB_CLOCK_GPTP_0_0, &now) < 0) {
			stream->clock_err++;
			goto exit;
		}

		for (idx = 0; idx < event_len; idx++) {
			if (event[idx].event_mask  & (AVTP_TIMESTAMP_INVALID | AVTP_TIMESTAMP_UNCERTAIN))
				continue;

			ts = event[idx].ts - (event[idx].index / stream->sample_size) * (unsigned int)stream->sample_dt;
			if (!listener_timestamp_accept(ts, now, period, sample_rate, stream->sr_class)) {
				stream->ts_err++;
				goto exit;
			}

			if (!event_ts)
				event_ts = &event[idx];
		}

		if (!event_ts) {
			stream->event_err++;
			goto exit;
		}

		/* Calculate the delay between the current time and the time when the audio should be played */
		tmp_float = (float)(event_ts->ts + GENAVB_PROCESSING_TIME_NS - (uint32_t)now);
		/* take into account event index offset */
		tmp_float -= ((float)event_ts->index / stream->sample_size) * stream->sample_dt;
		tmp_float = tmp_float / (period * stream->sample_dt);
		/* ceil to get upper value and take 1 period of processing into account */
		periods_to_wait = (unsigned int)ceilf(tmp_float);

		if (periods_to_wait >= AUDIO_PIPELINE_AVB_MAX_BUFFER_SIZE - 2) {
			stream->sync_err++;

			goto exit;
		}

		for (i = 0; i < avtp_source_channel_n(); i++)
			audio_buf_reset(stream->channel_buf[i]);

		for (j = 0; j < avtp_source_channel_n()	; j++)
			audio_buf_write_silence(stream->channel_buf[j], periods_to_wait * period);

		stream->first_start = false;
	} else {
		read_bytes = genavb_stream_receive(stream->handle, &data, stream->cur_batch_size, NULL, NULL);
		if (read_bytes < 0) {
			stream->err++;
			stream->first_start = true;

			goto exit;

		} else if (read_bytes < stream->cur_batch_size) {
			stream->underflow++;
			stream->first_start = true;

			goto exit;
		}
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

	rtos_mutex_lock(&avtp->mutex, RTOS_WAIT_FOREVER);

	for (i = 0; i < avtp->stream_n; i++) {
		stream = &avtp->stream[i];

		if (stream->connected)
			ret = listener_receive(avtp, i, element->period, element->sample_rate);
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

	rtos_mutex_unlock(&avtp->mutex);

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
		stream->first_start = true;
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

	crf_listener_disconnect(avtp);

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
		log_info("  underflow: %u, overflow: %u, err: %u, received: %u\n",
			avtp->stream[i].underflow, avtp->stream[i].overflow, avtp->stream[i].err,
			avtp->stream[i].received);
		log_info("  sync errors => read: %u, underflow: %u, clock: %u, ts: %u, event: %u\n",
			avtp->stream[i].sync_err, avtp->stream[i].start_underflow, avtp->stream[i].clock_err,
		    avtp->stream[i].ts_err, avtp->stream[i].event_err);
	}
	log_info("crf rx connected: %u\n", avtp->crf_stream.connected);
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

	if (rtos_mutex_init(&avtp->mutex))
		goto err;

	element->run = avtp_source_element_run;
	element->reset = avtp_source_element_reset;
	element->exit = avtp_source_element_exit;
	element->dump = avtp_source_element_dump;
	element->stats = avtp_source_element_stats;

	avtp->clock_domain = config->u.avtp_source.clock_domain;

	avtp->stream_n = config->u.avtp_source.stream_n;
	avtp->out_n = avtp_source_out_n();

	k = 0;
	for (i = 0; i < avtp->stream_n; i++) {
		avtp->stream[i].connected = 0;
		avtp->stream[i].connection_flags = config->u.avtp_source.stream[i].flags;

		for (j = 0; j < avtp_source_channel_n()	; j++, k++) {
			avtp->stream[i].channel_buf[j] = &buffer[config->output[k]];
			avtp->stream[i].channel_buf[j]->input_type = element->type;
			avtp->stream[i].channel_buf[j]->input_element_id = element->element_id;
			avtp->stream[i].channel_buf[j]->input_id = k;
		}
	}

	avtp->crf_stream.connected = 0;
	avtp->crf_stream.index = -1;

	for (i = 0; i < avtp->out_n; i++)
		avtp->out[i].buf = &buffer[config->output[i]];

	avtp_source_element_dump(element);

	return 0;

err:
	return -1;
}
