/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/stdio.h"

#include "audio_pipeline.h"
#include "audio.h"

extern struct audio_pipeline_config pipeline_config;

#define DEFAULT_PERIOD		8
#define DEFAULT_SAMPLE_RATE	48000

static const int supported_period[] = {2, 4, 8, 16, 32};
static const uint32_t supported_rate[] = {44100, 48000, 88200, 96000, 176400, 192000};

void play_pipeline_stats(void *handle)
{
}

int play_pipeline_run(void *handle, struct event *e)
{
	struct audio_pipeline *pipeline = handle;
	int err;

	err = audio_pipeline_run(pipeline);
	err = audio_pipeline_run(pipeline);
	err = audio_pipeline_run(pipeline);
	err = audio_pipeline_run(pipeline);
	err = audio_pipeline_run(pipeline);
	err = audio_pipeline_run(pipeline);

	return err;
}

void *play_pipeline_init(void *parameters)
{
	struct audio_config *cfg = parameters;
	struct audio_pipeline *pipeline;
	size_t period = DEFAULT_PERIOD;
	uint32_t rate = DEFAULT_SAMPLE_RATE;

	if (assign_nonzero_valid_val(period, cfg->period, supported_period) != 0) {
		os_printf("Period %d frames is not supported\r\n", cfg->period);
		goto err;
	}

	if (assign_nonzero_valid_val(rate, cfg->rate, supported_rate) != 0) {
		os_printf("Rate %d Hz is not supported\r\n", cfg->rate);
		goto err;
	}

	/* override pipeline configuration */
	pipeline_config.sample_rate = rate;
	pipeline_config.period = period;

	pipeline = audio_pipeline_init(&pipeline_config);
	if (!pipeline)
		goto err;

	audio_pipeline_dump(pipeline);

	os_printf("Starting pipeline (Sample Rate: %d Hz)\r\n",
			rate);

	return pipeline;

err:
	return NULL;
}

void play_pipeline_exit(void *handle)
{
	struct audio_pipeline *pipeline = handle;

	audio_pipeline_exit(pipeline);

	os_printf("\r\nEnd.\r\n");
}
