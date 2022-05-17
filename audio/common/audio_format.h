/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _AUDIO_FORMAT_H_
#define _AUDIO_FORMAT_H_

/* Audio samples in double floating point format, and [-1.0, 1.0] range.
 * If the format is changed, most of the functions below need to be adjusted.
 * Using a format with less than 32bit (default hardware fifo width) would
 * require more extensives adjustments.
 */
typedef double audio_sample_t;

#define AUDIO_SAMPLE_SILENCE	((audio_sample_t)0.0)
#define AUDIO_SAMPLE_SCALE	((audio_sample_t)2147483647.0)	/* 2^31 - 1 */

static inline int32_t audio_sample_to_int32(audio_sample_t v)
{
	/* [-1.0, 1.0] -> [-0x7fffffff, 0x7fffffff] */
	return (int32_t)(v * AUDIO_SAMPLE_SCALE);
}

static inline audio_sample_t audio_int32_to_sample(int32_t v)
{
	/* [-0x7fffffff, 0x7fffffff] -> [-1.0, 1.0] */
	return (audio_sample_t)v / AUDIO_SAMPLE_SCALE;
}

static inline audio_sample_t audio_double_to_sample(double v)
{
	return (audio_sample_t)v;
}

static inline void audio_invert_int32(int32_t *val)
{
	/* FIXME */
}

/*
 * Generic audio format conversion (from SAI to internal format)
 *
 * When this function is called, the "samples" input/output buffer
 * is full of 32bit fifo format samples (typically 32bit signed integer).
 * The 32bit samples are aligned on audio_sample_t boundaries (so
 * possibly with gaps between samples).
 *
 * On exit all samples are converted, in place, to audio_sample_t format.
 *
 * It implements:
 * - 32bit inversion of input
 * if invert is true, for endianess conversion, plus,
 * - output = (input & mask) << shift
 * - conversion to audio_sample_t
 */
static inline void audio_convert_from(audio_sample_t *samples, unsigned int len, bool invert, uint32_t mask, uint32_t shift)
{
	int32_t val;
	int i;

	for (i = 0; i < len; i++) {
		val = *((int32_t *)&samples[i]);

		if (invert)
			audio_invert_int32(&val);

		val = (val & mask) << shift;

		samples[i] = audio_int32_to_sample(val);
	}
}

/* Generic audio format conversion (from internal to SAI format)
 *
 * When this function exits, the "samples" input/output buffer
 * is full of 32bit fifo format samples (typically 32bit signed integer).
 * The 32bit samples are aligned on audio_sample_t boundaries (so
 * possibly with gaps between samples).
 *
 * It implements:
 * - conversion from audio_sample_t
 * - output = (input >> shift) & mask
 * plus,
 * - 32bit inversion of output
 * if invert is true, for endianess conversion.
 */
static inline void audio_convert_to(audio_sample_t *samples, unsigned int len, bool invert, uint32_t mask, uint32_t shift)
{
	audio_sample_t *sample;
	int32_t *val;
	int i;

	for (i = 0; i < len; i++) {
		sample = &samples[i];
		val = (int32_t *)sample;

		*val = audio_sample_to_int32(*sample);

		*val = (*val >> shift) & mask;

		if (invert)
			audio_invert_int32(val);
	}
}

#endif /* _AUDIO_FORMAT_H_ */
