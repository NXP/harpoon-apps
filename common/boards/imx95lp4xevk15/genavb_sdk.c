/*
 * Copyright 2023, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device interface for genAVB
 */
#include "genavb_sdk.h"

/*
 * 24MHz XTAL oscillator
 * Primary clock source for all PLLs
 */
uint32_t dev_get_pll_ref_freq(void)
{
	/* TODO */ return 0;
}

/*
 * Audio Pll tuning
 */
void dev_write_audio_pll_num(uint32_t num)
{
	/* TODO */
}

uint32_t dev_read_audio_pll_num(void)
{
	/* TODO */ return 0;
}

uint32_t dev_read_audio_pll_denom(void)
{
	/* TODO */ return 1;
}

uint32_t dev_read_audio_pll_post_div(void)
{
	/* TODO */ return 1;
}

/*
 * TPM counter input frequency (lptpm_clk)
 */
uint32_t dev_get_tpm_counter_freq(void *base)
{
	if (base == TPM1)
		return HAL_ClockGetIpFreq(hal_clock_busaon);
	else if (base == TPM2)
		return HAL_ClockGetIpFreq(hal_clock_tpm2);
	else if (base == TPM3)
		return HAL_ClockGetIpFreq(hal_clock_buswakeup);
	else if (base == TPM4)
		return HAL_ClockGetIpFreq(hal_clock_tpm4);
	else if (base == TPM5)
		return HAL_ClockGetIpFreq(hal_clock_tpm5);
	else if (base == TPM6)
		return HAL_ClockGetIpFreq(hal_clock_tpm6);
	else
		return 0;
}
