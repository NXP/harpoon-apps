/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

struct play_pipeline_config {
	const struct audio_pipeline_config *cfg;
};

struct audio_config {
	uint32_t rate;
	uint32_t period;

	void (*event_send)(void *, uint8_t);
	void *event_data;

	void *data;
};

struct event {
	unsigned int type;
	uintptr_t data;
};

enum {
	EVENT_TYPE_TX_RX,
	EVENT_TYPE_START
};

void *play_dtmf_init(void *parameters);
void *play_music_init(void *parameters);
void *play_sine_init(void *parameters);
void *rec_play_init(void *parameters);
void *play_pipeline_init(void *parameters);

int play_dtmf_run(void *handle, struct event *e);
int play_music_run(void *handle, struct event *e);
int play_sine_run(void *handle, struct event *e);
int rec_play_run(void *handle, struct event *e);
int play_pipeline_run(void *handle, struct event *e);

void play_dtmf_stats(void *handle);
void play_music_stats(void *handle);
void play_sine_stats(void *handle);
void rec_play_stats(void *handle);
void play_pipeline_stats(void *handle);

void play_dtmf_exit(void *handle);
void play_music_exit(void *handle);
void play_sine_exit(void *handle);
void rec_play_exit(void *handle);
void play_pipeline_exit(void *handle);

extern const struct audio_pipeline_config pipeline_dtmf_config;
extern const struct audio_pipeline_config pipeline_full_config;

/* assign_nonzero_valid_val(): Validate and assign nonzero value.
 * "value" == 0: "var" use default vale, return 0;
 * "value" != 0: if "value" is in array "supported_list", "var" = "value",
 * 		 then return 0, otherwise return -1;
 */
#define assign_nonzero_valid_val(var, value, valid_list)		\
({									\
	int __i, __ret = -1;						\
									\
	if ((value) == 0) {						\
		__ret = 0;						\
	} else {							\
		for (__i = 0; __i < ARRAY_SIZE((valid_list)); __i++) {	\
			if ((value) == (valid_list)[__i]) {		\
				(var) = (value);			\
				__ret = 0;				\
				break;					\
			}						\
		}							\
	}								\
	(__ret);							\
})

#endif /* _AUDIO_H_ */
