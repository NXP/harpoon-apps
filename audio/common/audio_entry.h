/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_ENTRY_H_
#define _AUDIO_ENTRY_H_

void *audio_control_init(uint8_t thread_count);
void audio_control_loop(void *context);
void audio_process_data(void *context, uint8_t thread_id);

#endif /* _AUDIO_ENTRY_H_ */
