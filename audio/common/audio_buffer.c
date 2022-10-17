/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_buffer.h"
#include "hlog.h"

/*
 * Audio buffer structure (circular) with single writter/reader.
 * Audio buffer uses linear memory storage.
 * When configuring pipeline, it's possible to re-use storage (and avoid memory copies).
 * Audio buffer has a compile time fixed sample format (64bit, double). Format conversion done in source/sink elements.
 * Audio buffer api's/variables all in sample units
 * Audio buffer size is a power of two.
 *
 * Many possible optimizations:
 * - Make all functions below, used in the data path, static inline (and move to header file)
 * - Make buffer size always a multiple of the processing period, if user(s) of the buffer always reads/writes full periods,
 *   then there is no need to check for read/write pointer wrap at each sample, only at the end.
 * - Add minimum alignment for audio buffer (at least frame aligned)
 * - Make buffer size always a power of two (in frame units), so all pointer wrapping can be done with a mask
 * - When copying data
 * 	a) unroll loops for small copies (e.g. 2, 4, 8 samples)
 * 	b) use inline assembly with floating point registers (up to 128bits copied per instruction)
 * 	c) use memcpy for big copies ( > 32 samples ?)
 *
*/
void audio_buf_dump(struct audio_buffer *buf)
{
	log_info("buf(%p): base %p, size %x/%x, read %x, write %x\n",
		  buf, buf->base, buf->size, buf->size_mask, buf->read, buf->write);
}

void audio_buf_init(struct audio_buffer *buf, audio_sample_t *base, unsigned int size)
{
	buf->base = base;
	buf->size = size;
	buf->read = 0;
	buf->write = 0;

	buf->size_mask = 0;
	while ((size >>= 1))
		buf->size_mask++;

	buf->size_mask = (1U << buf->size_mask) - 1;
}

unsigned int audio_buf_avail(struct audio_buffer *buf)
{
	unsigned int read = buf->read;
	unsigned int write = buf->write;

	if (write < read)
		return (write + buf->size) - read;
	else
		return write - read;
}

unsigned int audio_buf_free(struct audio_buffer *buf)
{
	return buf->size - audio_buf_avail(buf) - 1;
}

bool audio_buf_full(struct audio_buffer *buf)
{
	return (buf->read == buf->write - 1);
}

bool audio_buf_empty(struct audio_buffer *buf)
{
	return (buf->read == buf->write);
}

void audio_buf_write(struct audio_buffer *buf, audio_sample_t *samples, unsigned int len)
{
	int i;

	for (i = 0; i < len; i++) {
		buf->base[buf->write] = samples[i];
		buf->write = (buf->write + 1) & buf->size_mask;
	}
}

void audio_buf_write_head(struct audio_buffer *buf, audio_sample_t *samples, unsigned int len)
{
	unsigned int read;
	int i;

	read = buf->read = (buf->read - len) & buf->size_mask;
	for (i = 0; i < len; i++) {
		buf->base[read] = samples[i];
		read  = (read + 1) & buf->size_mask;
	}
}

void audio_buf_write_silence(struct audio_buffer *buf, unsigned int len)
{
	audio_sample_t silence = AUDIO_SAMPLE_SILENCE;
	int i;

	for (i = 0; i < len; i++)
		__audio_buf_write(buf, i, &silence, 1);

	audio_buf_write_update(buf, len);
}

void audio_buf_read(struct audio_buffer *buf, audio_sample_t *samples, unsigned int len)
{
	int i;

	for (i = 0; i < len; i++) {
		samples[i] = buf->base[buf->read];
		buf->read = (buf->read + 1) & buf->size_mask;
	}
}
