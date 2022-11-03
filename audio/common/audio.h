/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

#define MAX_AUDIO_DATA_THREADS 2

struct play_pipeline_config {
	const struct audio_pipeline_config *cfg[MAX_AUDIO_DATA_THREADS];
};

struct audio_config {
	uint32_t rate;
	uint32_t period;
	uint8_t pipeline_id;

	void (*event_send)(void *, uint8_t);
	void *event_data;

	void *data;
	void *async_sem;
};

struct event {
	unsigned int type;
	uintptr_t data;
};

enum {
	EVENT_TYPE_DATA,
	EVENT_TYPE_RESET,
	EVENT_TYPE_RESET_ASYNC
};

/* generic audio callbacks */
void *play_pipeline_init(void *parameters);
int play_pipeline_run(void *handle, struct event *e);
void play_pipeline_stats(void *handle);
void play_pipeline_exit(void *handle);

/* AVB-specific callbacks */
void *play_pipeline_init_avb(void *parameters);
void play_pipeline_ctrl_avb(void *handle);
void play_pipeline_exit_avb(void *handle);

extern const struct audio_pipeline_config pipeline_dtmf_config;
extern const struct audio_pipeline_config pipeline_sine_config;
extern const struct audio_pipeline_config pipeline_loopback_config;
extern const struct audio_pipeline_config pipeline_full_config;
extern const struct audio_pipeline_config pipeline_full_avb_config;
extern const struct audio_pipeline_config pipeline_full_thread_0_config;
extern const struct audio_pipeline_config pipeline_full_thread_1_config;

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
