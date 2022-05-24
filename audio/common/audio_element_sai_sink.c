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

#define SAI_TX_FIFO_PERIODS	2

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

struct sai_line {
	void *base;
	unsigned int id;
	unsigned int sai_id;
	unsigned int min;
	unsigned int max;
	unsigned int underflow;
	unsigned int overflow;
};

struct sai_sink_element {
	unsigned int map_n;
	struct sai_sink_map *map;
	unsigned int in_n;
	struct sai_input *in;
	unsigned int sai_n;
	struct sai_line *line;
	unsigned int line_n;
	void **base;
	bool started;
};

static void sai_sink_element_fifo_write(struct audio_element *element)
{
	struct sai_sink_element *sai = element->data;
	struct sai_sink_map *map;
	int i, j;

	/* Fill fifo with input buffer data */

	for (i = 0; i < sai->in_n; i++) {
		if (sai->in[i].convert)
				audio_convert_to(audio_buf_read_addr(sai->in[i].buf, 0), element->period, sai->in[i].invert, sai->in[i].mask, sai->in[i].shift);
	}

	for (i = 0; i < element->period; i++) {
		for (j = 0; j < sai->map_n; j++) {
			map = &sai->map[j];

			*map->tx_fifo = __audio_buf_read_uint32(map->in, i);
		}
	}

	for (i = 0; i < sai->in_n; i++)
		audio_buf_read_update(sai->in[i].buf, element->period);
}

static int sai_sink_element_run(struct audio_element *element)
{
	struct sai_sink_element *sai = element->data;
	struct sai_line *line;
	unsigned int level;
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
		/* Check SAI Tx Fifo level */
		for (i = 0; i < sai->line_n; i++) {
			line = &sai->line[i];

			level = __sai_tx_level(line->base, line->id);

			if (level <= line->min) {
				/* tx underflow */
				line->underflow++;
				goto err;
			}

			if (level >= line->max) {
				/* tx overflow */
				line->overflow++;
				goto err;
			}
		}
	} else {
		audio_sample_t val = AUDIO_SAMPLE_SILENCE;

		/* Fill head of input buffer with silence */
		for (i = 0; i < sai->in_n; i++) {
			for (j = 0; j < element->period; j++)
				audio_buf_write_head(sai->in[i].buf, &val, 1);
		}

		sai_sink_element_fifo_write(element);
	}

	sai_sink_element_fifo_write(element);

	if (!sai->started) {
		for (i = 0; i < sai->sai_n; i++)
			__sai_enable_tx(sai->base[i], false);

		sai->started = true;
	}

	return 0;

err:
	return -1;
}

static void sai_sink_element_reset(struct audio_element *element)
{
	struct sai_sink_element *sai = element->data;
	int i;

	for (i = 0; i < sai->sai_n; i++)
		__sai_disable_tx(sai->base[i]);

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

	for (i = 0; i < sai->line_n; i++)
		log_info("line: %u, sai(%u, %u)\n", i, sai->line[i].sai_id, sai->line[i].id);

	for (i = 0; i < sai->in_n; i++)
		audio_buf_dump(sai->in[i].buf);
}

static void sai_sink_element_stats(struct audio_element *element)
{
	struct sai_sink_element *sai = element->data;
	int i;

	for (i = 0; i < sai->line_n; i++) {
		log_info("tx line: %u, sai(%u, %u)\n",
			 i, sai->line[i].sai_id, sai->line[i].id);

		log_info("  underflow: %u, overflow: %u\n",
			sai->line[i].underflow, sai->line[i].overflow);
	}
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

static unsigned int sai_sink_line_size(struct audio_element_config *config)
{
	struct sai_tx_config *sai_config;
	unsigned int line_size = 0;
	int i;

	for (i = 0; i < config->u.sai_sink.sai_n; i++) {
		sai_config = &config->u.sai_sink.sai[i];

		line_size += sai_config->line_n;
	}

	return line_size;
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

			if ((SAI_TX_FIFO_PERIODS * line_config->channel_n * config->period) > SAI_TX_MAX_FIFO_SIZE) {
				log_err("sai sink: invalid tx fifo size: %u\n", SAI_TX_FIFO_PERIODS * line_config->channel_n * config->period);
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
	size += sai_sink_line_size(config) * sizeof(struct sai_line);
	size += config->u.sai_source.sai_n * sizeof(void *);

	return size;
}

int sai_sink_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	struct sai_sink_element *sai = element->data;
	struct sai_tx_config *sai_config;
	struct sai_tx_line_config *line_config;
	struct sai_sink_map *map;
	struct sai_line *line;
	int i, j, k, l;

	element->run = sai_sink_element_run;
	element->reset = sai_sink_element_reset;
	element->exit = sai_sink_element_exit;
	element->dump = sai_sink_element_dump;
	element->stats = sai_sink_element_stats;

	sai->started = false;

	sai->map_n = sai_sink_map_size(config);
	sai->in_n = sai->map_n;
	sai->sai_n = config->u.sai_sink.sai_n;
	sai->line_n = sai_sink_line_size(config);

	sai->map = (struct sai_sink_map *)((uint8_t *)sai + sizeof(struct sai_sink_element));
	sai->in = (struct sai_input *)((uint8_t *)sai->map + sai->map_n * sizeof(struct sai_sink_map));
	sai->line = (struct sai_line *)((uint8_t *)sai->in + sai->in_n * sizeof(struct sai_input));
	sai->base = (void **)((uint8_t *)sai->line + sai->line_n * sizeof(struct sai_line));

	l = 0;
	line = &sai->line[0];
	for (i = 0; i < config->u.sai_sink.sai_n; i++) {
		sai_config = &config->u.sai_sink.sai[i];

		sai->base[i] = __sai_base(sai_config->id);
		if (!sai->base[i])
			goto err;

		for (j = 0; j < sai_config->line_n; j++) {
			line_config = &sai_config->line[j];

			line->base = sai->base[i];
			line->id = line_config->id;
			line->sai_id = sai_config->id;

			line->min = 0;
			line->max = line_config->channel_n * element->period + 1;
			line++;

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
		sai->in[i].convert = true;
		sai->in[i].invert = false;
		sai->in[i].shift = 0;
		sai->in[i].mask = 0xffffffff;
	}

	sai_sink_element_dump(element);

	return 0;
err:
	return -1;
}
