/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _AUDIO_FORMAT_H_
#define _AUDIO_FORMAT_H_

static inline int32_t audio_double_to_int32(double v)
{
	/* [-1.0, 1.0] -> [-0x7fffffff, 0x7fffffff] */
	return (int32_t)(v * 0x7fffffff);
}

#endif /* _AUDIO_FORMAT_H_ */
