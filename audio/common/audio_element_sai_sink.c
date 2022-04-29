/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_element_sai_sink.h"
#include "audio_element.h"
#include "audio_format.h"
#include "hlog.h"

#include "sai_drv.h"

struct sai_sink_map {
	volatile uint32_t *tx_fifo;	/* sai tx fifo address */
	struct audio_buffer *in;	/* input audio buffer address */
};

struct sai_input {
	struct audio_buffer *buf;	/* input audio buffer address */
	bool convert;
	bool invert;			/* format conversion */
	unsigned int shift;		/* format conversion */
	unsigned int mask;		/* format conversion */
};

struct sai_sink_element {
	unsigned int map_n;
	struct sai_sink_map *map;
	unsigned int in_n;
	struct sai_input *in;
	unsigned int sai_n;
	void **base;
	bool started;
};

uint32_t sai_dummy[65] = {0};

static void *sai_baseaddr(unsigned int id)
{
	if (!id)
		return &sai_dummy;

	return __sai_base(id);
}

static int sai_sink_element_run(struct audio_element *element)
{
	struct sai_sink_element *sai = element->data;
	struct sai_sink_map *map;
	int32_t val;
	int i, j;

#if 0
	/* WIP */
	switch (sync_mode) {
	case SYNC_FULL:
		/* scheduling source is synchronous to audio clock, frame sync and fifo level, e.g:
		   - SAI IRQ (for a single line)
		   - SAI IRQ (for multi line, with no individual fifo reset)
		   sai fifo level is guaranteed, can write fixed amount of samples at each iteration
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
		/* Check for SAI Tx Fifo errors */
		for (i = 0; i < sai->sai_n; i++)
			if (__sai_tx_error(sai->base[i]))
				goto err;
	} else {
		/* Fill SAI Tx Fifo with silence */
		for (i = 0; i < element->period; i++) {
			for (j = 0; j < sai->map_n; j++) {
				map = &sai->map[j];

				val = 0;

				*map->tx_fifo = val;
			}
		}
	}

	for (i = 0; i < sai->in_n; i++) {
		if (sai->in[i].convert)
			audio_convert_to(audio_buf_read_addr(sai->in[i].buf), element->period, sai->in[i].invert, sai->in[i].mask, sai->in[i].shift);
	}

	for (i = 0; i < element->period; i++) {
		for (j = 0; j < sai->map_n; j++) {
			map = &sai->map[j];

			__audio_buf_read(map->in, i, &val, 1);

			*map->tx_fifo = val;
		}
	}

	if (!sai->started) {
		for (i = 0; i < sai->sai_n; i++) {
			/* FIXME */
			/* If some SAI's are only used for Rx, they will be missing below */
			/* For multi SAI sync, we should enable the asynchronous/master one last */
			if (sai->base[i] != &sai_dummy) {
				__sai_enable_rx(sai->base[i], false);
				__sai_enable_tx(sai->base[i], false);
			}
		}

		sai->started = true;
	}

	for (i = 0; i < sai->in_n; i++)
		audio_buf_read_update(sai->in[i].buf, element->period);

	return 0;

err:
	return -1;
}

static void sai_sink_element_reset(struct audio_element *element)
{
	struct sai_sink_element *sai = element->data;
	int i;

	for (i = 0; i < sai->sai_n; i++) {
		if (sai->base[i] != &sai_dummy)
			__sai_tx_reset(sai->base[i]);
	}

	sai->started = false;
}

static void sai_sink_element_exit(struct audio_element *element)
{
}

static void sai_sink_element_dump(struct audio_element *element)
{
	struct sai_sink_element *sai = element->data;
	int i;

	log_info("sai sink(%p/%p)\n", sai, element);
	log_info("  inputs: %u\n", sai->in_n);
	log_info("  mapping:\n");

	for (i = 0; i < sai->map_n; i++)
		log_info("    %p => %p\n", sai->map[i].in, sai->map[i].tx_fifo);

	for (i = 0; i < sai->in_n; i++)
		audio_buf_dump(sai->in[i].buf);
}

static unsigned int sai_sink_map_size(struct audio_element_config *config)
{
	struct sai_tx_config *sai_config;
	struct sai_tx_line_config *line_config;
	unsigned int map_size = 0;
	int i, j;

	for (i = 0; i < config->u.sai_sink.sai_n; i++) {
		sai_config = &config->u.sai_sink.sai[i];

		for (j = 0; j < sai_config->line_n; j++) {
			line_config = &sai_config->line[j];

			map_size += line_config->channel_n;
		}
	}

	return map_size;
}

int sai_sink_element_check_config(struct audio_element_config *config)
{
	struct sai_tx_config *sai_config;
	struct sai_tx_line_config *line_config;
	int i, j;

	if (config->outputs) {
		log_err("sai sink: invalid outputs: %u\n", config->outputs);
		goto err;
	}

	if (config->inputs != sai_sink_map_size(config)) {
		log_err("sai sink: invalid inputs: %u\n", config->inputs);
		goto err;
	}

	if (config->u.sai_sink.sai_n > SAI_RX_MAX_INSTANCE) {
		log_err("sai sink: invalid instances: %u\n", config->u.sai_sink.sai_n);
		goto err;
	}

	for (i = 0; i < config->u.sai_sink.sai_n; i++) {
		sai_config = &config->u.sai_sink.sai[i];

		if (sai_config->id >= SAI_TX_MAX_ID) {
			log_err("sai sink: invalid sai: %u\n", sai_config->id);
			goto err;
		}

		if (sai_config->line_n > SAI_TX_INSTANCE_MAX_LINE) {
			log_err("sai sink: invalid lines: %u\n", sai_config->line_n);
			goto err;
		}

		for (j = 0; j < sai_config->line_n; j++) {
			line_config = &sai_config->line[j];

			if (line_config->id >= SAI_TX_INSTANCE_MAX_LINE) {
				log_err("sai sink: invalid line: %u\n", line_config->id);
				goto err;
			}

			if (line_config->channel_n > SAI_TX_INSTANCE_MAX_CHANNELS) {
				log_err("sai sink: invalid channels: %u\n", line_config->channel_n);
				goto err;
			}
		}
	}

	return 0;

err:
	return -1;
}

unsigned int sai_sink_element_size(struct audio_element_config *config)
{
	unsigned int size;

	size = sizeof(struct sai_sink_element);
	size += sai_sink_map_size(config) * (sizeof(struct sai_sink_map) + sizeof(struct sai_input));
	size += config->u.sai_source.sai_n * sizeof(void *);

	return size;
}

int sai_sink_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	struct sai_sink_element *sai = element->data;
	struct sai_tx_config *sai_config;
	struct sai_tx_line_config *line_config;
	struct sai_sink_map *map;
	int i, j, k, l;

	element->run = sai_sink_element_run;
	element->reset = sai_sink_element_reset;
	element->exit = sai_sink_element_exit;
	element->dump = sai_sink_element_dump;

	sai->started = false;

	sai->map_n = sai_sink_map_size(config);
	sai->in_n = sai->map_n;
	sai->sai_n = config->u.sai_sink.sai_n;

	sai->map = (struct sai_sink_map *)((uint8_t *)sai + sizeof(struct sai_sink_element));
	sai->in = (struct sai_input *)((uint8_t *)sai->map + sai->map_n * sizeof(struct sai_sink_map));
	sai->base = (void **)((uint8_t *)sai->in + sai->in_n * sizeof(struct sai_input));

	l = 0;
	for (i = 0; i < config->u.sai_sink.sai_n; i++) {
		sai_config = &config->u.sai_sink.sai[i];

		sai->base[i] = sai_baseaddr(sai_config->id);
		if (!sai->base[i])
			goto err;

		for (j = 0; j < sai_config->line_n; j++) {
			line_config = &sai_config->line[j];

			for (k = 0; k < line_config->channel_n; k++) {
				map = &sai->map[l];

				map->tx_fifo = __sai_tx_fifo_addr(sai->base[i], line_config->id);
				map->in = &buffer[config->input[l]];
				l++;
			}
		}
	}

	for (i = 0; i < sai->in_n; i++) {
		sai->in[i].buf = &buffer[config->input[i]];
		sai->in[i].convert = false;
	}

	sai_sink_element_dump(element);

	return 0;
err:
	return -1;
}
