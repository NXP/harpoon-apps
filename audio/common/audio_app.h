/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_APP_H_
#define _AUDIO_APP_H_

#include <stdint.h>

int audio_app_ctrl_send(void *ctrl_handle, void *data, uint32_t len);
int audio_app_ctrl_recv(void *ctrl_handle, void *data, uint32_t *len);
void *audio_app_ctrl_init(void);
void audio_app_main(void);

#endif /* _AUDIO_APP_H_ */
