/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_element_sai_source.h"
#include "audio_element.h"
#include "audio_format.h"
#include "hlog.h"

#include "sai_drv.h"

/*
 Sai source

 Read from one (or more) sai instances, with one (or more) physical lines, with one (or more) channels,
 scatter data storing each individual channel in a different output buffer.
 Configuration specifies: sai instances, lines per instance, channels per instance and output buffers
 Runtime specifies: map scatter array (fifo address to read, outbut buffer)

use case 1
 single line, multiple channels, multiple buffers

use case 2
 multiple lines, multiple channels, multiple buffers

use case 3
 multiple sai, multiple lines, multiple channels. similar to 2 but sai address may change at each step.

*/

extern uint32_t sai_dummy[65];

struct sai_source_map {
	volatile uint32_t *rx_fifo;	/* sai rx fifo address */
	struct audio_buffer *out;	/* output audio buffer address */
};

struct sai_output {
	struct audio_buffer *buf;
	bool convert;
	bool invert;			/* format conversion */
	unsigned int shift;		/* format conversion */
	unsigned int mask;		/* format conversion */
	int32_t val;			/* silence sample, in source format */
};

struct sai_source_element {
	unsigned int map_n;
	struct sai_source_map *map;
	unsigned int out_n;
	struct sai_output *out;
	unsigned int sai_n;
	void **base;
	bool started;
};

static void *sai_baseaddr(unsigned int id)
{
	if (!id)
		return &sai_dummy;

	return __sai_base(id);
}

static int sai_source_element_run(struct audio_element *element)
{
	struct sai_source_element *sai = element->data;
	struct sai_source_map *map;
	int32_t val;
	int i, j;

#if 0
	/* WIP */
	switch (sync_mode) {
	case SYNC_FULL:
		/* scheduling source is synchronous to audio clock, frame sync and fifo level, e.g:
		   - SAI IRQ (for a single line)
		   - SAI IRQ (for multi line, with no individual fifo reset)
		   sai fifo level is guaranteed, can read fixed amount of samples at each iteration
		   may check for fifo errors
		 */
	case SYNC_PARTIAL:
		/* scheduling source is synchronous to audio clock, but not frame sync, fifo level, e.g.:
                   - GPT timer IRQ with audio clock source
		   - SAI IRQ, for a different SAI instance, using same clock, but not hardware sync mode
		   - SAI IRQ, for a different SAI instance, but fifo was reset after start 
		   On first iteration, after start, number of samples may vary, need to compensate.
		   Need to take into account software jitter for proper compensation.
		   In following iterations, fifo level should be guaranteed.
		   May increase end to end latency of up to one sample.
		*/
	case SYNC_NONE:
		/* scheduling source asynchronous to audio clock:
		   - GPT timer IRQ, with system clock source
                   - Slave SAI with different audio clock
		   in this case samples may be accumulating in the playback fifo, need to check level and compensate (drop samples)
		*/
	}
#endif
	if (sai->started) {
		/* Check for SAI Rx Fifo errors */
		for (i = 0; i < sai->sai_n; i++)
			if (__sai_rx_error(sai->base[i]))
				goto err;

		for (i = 0; i < element->period; i++) {
			for (j = 0; j < sai->map_n; j++) {
				map = &sai->map[j];

				val = *map->rx_fifo;

				__audio_buf_write(map->out, i, &val, 1);
			}
		}
	} else {
		/* Fill output buffer with silence */
		for (i = 0; i < sai->out_n; i++)
			for (j = 0; j < element->period; j++)
				__audio_buf_write(sai->out[i].buf, j, &sai->out[i].val, 1);

		sai->started = true;
	}

	for (i = 0; i < sai->out_n; i++) {
		if (sai->out[i].convert)
			audio_convert_from(audio_buf_write_addr(sai->out[i].buf), element->period, sai->out[i].invert, sai->out[i].mask, sai->out[i].shift);

		audio_buf_write_update(sai->out[i].buf, element->period);
	}

	return 0;

err:
	return -1;
}

static void sai_source_element_reset(struct audio_element *element)
{
	struct sai_source_element *sai = element->data;
	int i;

	for (i = 0; i < sai->sai_n; i++) {
		if (sai->base[i] != &sai_dummy)
			__sai_rx_reset(sai->base[i]);
	}

	for (i = 0; i < sai->out_n; i++)
		audio_buf_reset(sai->out[i].buf);

	sai->started = false;
}

static void sai_source_element_exit(struct audio_element *element)
{
}

static void sai_source_element_dump(struct audio_element *element)
{
	struct sai_source_element *sai = element->data;
	int i;

	log_info("sai source(%p/%p)\n", sai, element);
	log_info("  outputs: %u\n", sai->out_n);
	log_info("  mapping:\n");

	for (i = 0; i < sai->map_n; i++)
		log_info("    %p => %p\n", sai->map[i].rx_fifo, sai->map[i].out);

	for (i = 0; i < sai->out_n; i++)
		audio_buf_dump(sai->out[i].buf);
}

static unsigned int sai_source_map_size(struct audio_element_config *config)
{
	struct sai_rx_config *sai_config;
	struct sai_rx_line_config *line_config;
	unsigned int map_size = 0;
	int i, j;

	for (i = 0; i < config->u.sai_source.sai_n; i++) {
		sai_config = &config->u.sai_source.sai[i];

		for (j = 0; j < sai_config->line_n; j++) {
			line_config = &sai_config->line[j];

			map_size += line_config->channel_n;
		}
	}

	return map_size;
}

int sai_source_element_check_config(struct audio_element_config *config)
{
	struct sai_rx_config *sai_config;
	struct sai_rx_line_config *line_config;
	int i, j;

	if (config->inputs) {
		log_err("sai source: invalid inputs: %u\n", config->inputs);
		goto err;
	}

	if (config->outputs != sai_source_map_size(config)) {
		log_err("sai source: invalid outputs: %u\n", config->outputs);
		goto err;
	}

	if (config->u.sai_source.sai_n > SAI_RX_MAX_INSTANCE) {
		log_err("sai source: invalid instances: %u\n", config->u.sai_source.sai_n);
		goto err;
	}

	for (i = 0; i < config->u.sai_source.sai_n; i++) {
		sai_config = &config->u.sai_source.sai[i];

		if (sai_config->id >= SAI_RX_MAX_ID) {
			log_err("sai source: invalid sai: %u\n", sai_config->id);
			goto err;
		}

		if (sai_config->line_n > SAI_RX_INSTANCE_MAX_LINE) {
			log_err("sai source: invalid lines: %u\n", sai_config->line_n);
			goto err;
		}

		for (j = 0; j < sai_config->line_n; j++) {
			line_config = &sai_config->line[j];

			if (line_config->id >= SAI_RX_INSTANCE_MAX_LINE) {
				log_err("sai source: invalid line: %u\n", line_config->id);
				goto err;
			}

			if (line_config->channel_n > SAI_RX_INSTANCE_MAX_CHANNELS) {
				log_err("sai source: invalid channels: %u\n", line_config->channel_n);
				goto err;
			}
		}
	}

	return 0;

err:
	return -1;
}

unsigned int sai_source_element_size(struct audio_element_config *config)
{
	unsigned int size;

	size = sizeof(struct sai_source_element);
	size += sai_source_map_size(config) * (sizeof(struct sai_source_map) + sizeof(struct sai_output));
	size += config->u.sai_source.sai_n * sizeof(void *);

	return size;
}

int sai_source_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	struct sai_source_element *sai = element->data;
	struct sai_rx_config *sai_config;
	struct sai_rx_line_config *line_config;
	struct sai_source_map *map;
	int i, j, k, l;

	element->run = sai_source_element_run;
	element->reset = sai_source_element_reset;
	element->exit = sai_source_element_exit;
	element->dump = sai_source_element_dump;

	sai->started = false;

	sai->map_n = sai_source_map_size(config);
	sai->out_n = sai->map_n;
	sai->sai_n = config->u.sai_source.sai_n;

	sai->map = (struct sai_source_map *)((uint8_t *)sai + sizeof(struct sai_source_element));
	sai->out = (struct sai_output *)((uint8_t *)sai->map + sai->map_n * sizeof(struct sai_source_map));
	sai->base = (void **)((uint8_t *)sai->out + sai->out_n * sizeof(struct sai_output));

	l = 0;
	for (i = 0; i < config->u.sai_source.sai_n; i++) {
		sai_config = &config->u.sai_source.sai[i];

		sai->base[i] = sai_baseaddr(sai_config->id);
		if (!sai->base[i])
			goto err;

		for (j = 0; j < sai_config->line_n; j++) {
			line_config = &sai_config->line[j];

			for (k = 0; k < line_config->channel_n; k++) {
				map = &sai->map[l];

				map->rx_fifo = __sai_rx_fifo_addr(sai->base[i], line_config->id);
				map->out = &buffer[config->output[l]];
				l++;
			}
		}
	}

	for (i = 0; i < sai->out_n; i++) {
		sai->out[i].buf = &buffer[config->output[i]];
		sai->out[i].convert = false;
		sai->out[i].val = 0;
	}

	sai_source_element_dump(element);

	return 0;

err:
	return -1;
}
