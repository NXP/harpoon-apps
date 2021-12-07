/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

struct audio_config {
	unsigned int rate;

	void (*event_send)(void *, uint8_t);
	void *event_data;
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
void *rec_play2_init(void *parameters);

int play_dtmf_run(void *handle, struct event *e);
int play_music_run(void *handle, struct event *e);
int play_sine_run(void *handle, struct event *e);
int rec_play_run(void *handle, struct event *e);
int rec_play2_run(void *handle, struct event *e);

void play_dtmf_exit(void *handle);
void play_music_exit(void *handle);
void play_sine_exit(void *handle);
void rec_play_exit(void *handle);
void rec_play2_exit(void *handle);

#endif /* _AUDIO_H_ */
