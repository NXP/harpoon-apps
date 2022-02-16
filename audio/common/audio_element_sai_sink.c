/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_element_sai_sink.h"
#include "audio_element.h"

static uint32_t fifo;

static uint32_t *sai_baseaddr[SAI_TX_MAX_ID] = {
	&fifo,
	&fifo,
	&fifo,
	&fifo,
	&fifo,
	&fifo,
	&fifo,
	&fifo
};

static const unsigned int line_tx_fifo_offset[SAI_TX_INSTANCE_MAX_LINE] = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

struct sai_sink_map {
	volatile uint32_t *tx_fifo;	/* sai tx fifo address */
	struct audio_buffer *in;	/* input audio buffer address */
	unsigned int shift;		/* format conversion */
	unsigned int mask;		/* format conversion */
	bool invert;			/* format conversion */
};

struct sai_sink_element {
	unsigned int map_n;
	struct sai_sink_map *map;
	unsigned int in_n;
	struct audio_buffer **in;
};

static inline void invert_int32(int32_t *val)
{
	/* invert a signed 32bit integer */
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

	for (i = 0; i < element->period; i++) {
		for (j = 0; j < sai->map_n; j++) {
			map = &sai->map[j];

			__audio_buf_read(map->in, i, &val, 1);

			/* do format conversion here if required */
			val = (val >> map->shift) & map->mask;

			if (map->invert)
				invert_int32(&val);

			*map->tx_fifo = val;
		}
	}

	for (i = 0; i < sai->in_n; i++)
		audio_buf_read_update(sai->in[i], element->period);

	return 0;
}

static void sai_sink_element_reset(struct audio_element *element)
{
	/* Disable all SAI Tx */
	/* Reset all SAI Tx fifo */
}

static void sai_sink_element_exit(struct audio_element *element)
{
}

static void sai_sink_element_dump(struct audio_element *element)
{
	struct sai_sink_element *sai = element->data;
	int i;

	os_printf("sai sink(%p/%p)\n\r", sai, element);
	os_printf("  inputs: %u\n\r", sai->in_n);
	os_printf("  mapping:\n\r");

	for (i = 0; i < sai->map_n; i++)
		os_printf("    %p => %p\n\r", sai->map[i].in, sai->map[i].tx_fifo);

	for (i = 0; i < sai->in_n; i++)
		audio_buf_dump(sai->in[i]);
}

unsigned int sai_sink_map_size(struct audio_element_config *config)
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

unsigned int sai_sink_element_size(struct audio_element_config *config)
{
	unsigned int size;

	size = sizeof(struct sai_sink_element);
	size += sai_sink_map_size(config) * (sizeof(struct sai_sink_map) + sizeof(struct audio_buffer *));

	return size;
}

int sai_sink_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	struct sai_sink_element *sai = element->data;
	struct sai_tx_config *sai_config;
	struct sai_tx_line_config *line_config;
	struct sai_sink_map *map;
	int i, j, k, l;

	if (config->outputs)
		goto err;

	if (config->inputs != sai_sink_map_size(config))
		goto err;

	element->run = sai_sink_element_run;
	element->reset = sai_sink_element_reset;
	element->exit = sai_sink_element_exit;
	element->dump = sai_sink_element_dump;

	sai->map_n = sai_sink_map_size(config);
	sai->in_n = sai->map_n;
	sai->map = (struct sai_sink_map *)((uint8_t *)sai + sizeof(struct sai_sink_element));
	sai->in = (struct audio_buffer **)((uint8_t *)sai->map + sai->map_n * sizeof(struct sai_sink_map));

	l = 0;
	for (i = 0; i < config->u.sai_sink.sai_n; i++) {
		sai_config = &config->u.sai_sink.sai[i];

		for (j = 0; j < sai_config->line_n; j++) {
			line_config = &sai_config->line[j];

			for (k = 0; k < line_config->channel_n; k++) {
				map = &sai->map[l];

				map->tx_fifo = sai_baseaddr[sai_config->id] + line_tx_fifo_offset[line_config->id];
				map->in = &buffer[config->input[l]];
				map->mask = 0xffffffff;
				map->shift = 0;
				map->invert = false;
				l++;
			}
		}
	}

	for (i = 0; i < sai->in_n; i++)
		sai->in[i] = &buffer[config->input[i]];

	sai_sink_element_dump(element);

	return 0;
err:
	return -1;
}
