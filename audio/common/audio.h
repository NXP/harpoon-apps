/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

void play_dtmf_task(void *parameters);
void play_music_task(void *parameters);
void play_sine_task(void *parameters);
void rec_play_task(void *parameters);
void rec_play2_task(void *parameters);

#endif /* _AUDIO_H_ */
