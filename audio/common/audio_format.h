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

static inline void audio_invert_int32(int32_t *val)
{
	/* FIXME */
}

/*
 * Generic integer audio format conversion (from SAI to internal format)
 *
 * It implements:
 * - 32bit inversion of input
 * if invert is true, for endianess conversion, plus,
 * - output = (input & mask) << shift
 */
static inline void audio_convert_from(int32_t *samples, unsigned int len, bool invert, uint32_t mask, uint32_t shift)
{
	int32_t *val;
	int i;

	for (i = 0; i < len; i++) {
		val = &samples[i];

		if (invert)
			audio_invert_int32(val);

		*val = (*val & mask) << shift;
	}
}

/* Generic integer audio format conversion (from internal to SAI format)
 *
 * It implements:
 * - output = (input >> shift) & mask
 * plus,
 * - 32bit inversion of output
 * if invert is true, for endianess conversion.
 */
static inline void audio_convert_to(int32_t *samples, unsigned int len, bool invert, uint32_t mask, uint32_t shift)
{
	int32_t *val;
	int i;

	for (i = 0; i < len; i++) {
		val = &samples[i];

		*val = (*val >> shift) & mask;

		if (invert)
			audio_invert_int32(val);
	}
}

#endif /* _AUDIO_FORMAT_H_ */
