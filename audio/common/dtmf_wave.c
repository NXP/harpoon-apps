/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "math.h"

#include "FreeRTOS.h"

#include "os/assert.h"
#include "os/stdlib.h"

#define PI 3.141592654

static void generate_sinewave(uint32_t *buf, int lfreq, int rfreq,
		uint32_t sample_rate, uint32_t duration_us, uint32_t *phase)
{
    int i;
    size_t nsamples = sample_rate * duration_us / 1000;
    for (i = 0; i < nsamples; i++) {
        double v1, v2;
        int32_t w1, w2;

        v1 = 0.5 * sin((i + *phase)* 2 * PI * lfreq / sample_rate);
        w1 = (int32_t)(v1 * ((1 << 30) - 1));
        v2 = 0.5 * sin((i + *phase) * 2 * PI * rfreq / sample_rate);
        w2 = (int32_t)(v2 * ((1 << 30) - 1));
        *buf++ = w1;
        *buf++ = w2;
    }

    *phase += nsamples;
}

static void get_dtmf_freqs(char key, int *freq1, int *freq2)
{
    switch (key) {
        case '1':
        case '2':
        case '3':
        case 'A':
            /* 697 Hz */
            *freq1 = 697;
            break;

        case '4':
        case '5':
        case '6':
        case 'B':
            /* 770 Hz */
            *freq1 = 770;
            break;

        case '7':
        case '8':
        case '9':
        case 'C':
            /* 852 Hz */
            *freq1 = 852;
            break;

        case '*':
        case '0':
        case '#':
        case 'D':
            /* 941 Hz */
            *freq1 = 941;
            break;
    }

    switch (key) {
        case '1':
        case '4':
        case '7':
        case '*':
            /* 1209 Hz */
            *freq2 = 1209;
            break;

        case '2':
        case '5':
        case '8':
        case '0':
            /* 1336 Hz */
            *freq2 = 1336;
            break;

        case '3':
        case '6':
        case '9':
        case '#':
            /* 1477 Hz */
            *freq2 = 1477;
            break;

        case 'A':
        case 'B':
        case 'C':
        case 'D':
            /* 1633 Hz */
            *freq2 = 1633;
            break;
    }
}

void generate_dtmf_tone(uint32_t *buf, size_t buf_size, char lkey, char rkey,
		uint32_t sample_rate, uint32_t duration_us, uint32_t *phase)
{
    int lfreq1 = 0, lfreq2 = 0, rfreq1 = 0, rfreq2 = 0;
    uint32_t *buf2;
    uint32_t dtmf_phase = *phase;
    int i;

    buf2 = os_malloc(buf_size);
    os_assert(buf2, "generate_dtmf_tone() failed with memory allocation error.");
    memset(buf2, 0, buf_size);

    /* Get dtmf frequencies for left and right channels */
    get_dtmf_freqs(lkey, &lfreq1, &lfreq2);
    get_dtmf_freqs(rkey, &rfreq1, &rfreq2);

    /* Fill primary sinewave */
    generate_sinewave(buf, lfreq1, rfreq1, sample_rate, duration_us, &dtmf_phase);

    /* Add secondary sinewave */
    generate_sinewave(buf2, lfreq2, rfreq2, sample_rate, duration_us, phase);

    /* mix buf and buf2 */
    for (i = 0; i < buf_size / 4; i ++) {
        buf[i] += buf2[i];
    }

    os_free(buf2);
}
