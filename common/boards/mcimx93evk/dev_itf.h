/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device interface for genAVB
 */
#ifndef _DEV_ITF_H_
#define _DEV_ITF_H_

#include "fsl_clock.h"
#include "fsl_enet_qos.h"

/*
 * 24MHz XTAL oscillator
 * Primary clock source for all PLLs
 */
static inline uint32_t dev_get_pll_ref_freq(void)
{
	/* TODO */ return 0;
}

/*
 * Audio Pll tuning
 */
static inline void dev_write_audio_pll_num(uint32_t num)
{
	/* TODO */
}

static inline uint32_t dev_read_audio_pll_num(void)
{
	/* TODO */ return 0;
}

static inline uint32_t dev_read_audio_pll_denom(void)
{
	/* TODO */ return 0;
}

static inline uint32_t dev_read_audio_pll_post_div(void)
{
	/* TODO */ return 0;
}

/*
 * Enet QOS module frequency (clk_csr_i)
 */
static inline uint32_t dev_get_enet_core_freq(void *base)
{
	if (base == ENET_QOS)
		return CLOCK_GetIpFreq(kCLOCK_Root_WakeupAxi);
	else
		return 0; //TODO
}

/*
 * TPM counter input frequency (lptpm_clk)
 */
static inline uint32_t dev_get_tpm_counter_freq(void *base)
{
	if (base == TPM1)
		return CLOCK_GetIpFreq(kCLOCK_Root_BusAon);
	else if (base == TPM2)
		return CLOCK_GetIpFreq(kCLOCK_Root_Tpm2);
	else if (base == TPM3)
		return CLOCK_GetIpFreq(kCLOCK_Root_BusWakeup);
	else if (base == TPM4)
		return CLOCK_GetIpFreq(kCLOCK_Root_Tpm4);
	else if (base == TPM5)
		return CLOCK_GetIpFreq(kCLOCK_Root_Tpm5);
	else if (base == TPM6)
		return CLOCK_GetIpFreq(kCLOCK_Root_Tpm6);
	else
		return 0;
}

/*
 * Enet Qos 1588 timer frequency (ptp_ref_i)
 */
static inline uint32_t dev_get_enet_1588_freq(void *base)
{
	if (base == ENET_QOS)
		return CLOCK_GetIpFreq(kCLOCK_Root_EnetTimer2);
	else
		return 0;
}

#endif /* _DEV_ITF_H_ */
