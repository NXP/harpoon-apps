/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

void *play_dtmf_init(void *parameters);
void *play_music_init(void *parameters);
void *play_sine_init(void *parameters);
void *rec_play_init(void *parameters);
void *rec_play2_init(void *parameters);

int play_dtmf_run(void *handle);
int play_music_run(void *handle);
int play_sine_run(void *handle);
int rec_play_run(void *handle);
int rec_play2_run(void *handle);

void play_dtmf_exit(void *handle);
void play_music_exit(void *handle);
void play_sine_exit(void *handle);
void rec_play_exit(void *handle);
void rec_play2_exit(void *handle);

#endif /* _AUDIO_H_ */
