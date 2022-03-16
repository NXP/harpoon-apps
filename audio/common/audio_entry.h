/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_ENTRY_H_
#define _AUDIO_ENTRY_H_

void *audio_control_init(void);
void audio_control_loop(void *context);
void audio_process_data(void *context);

#endif /* _AUDIO_ENTRY_H_ */
