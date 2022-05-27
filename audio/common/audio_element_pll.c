/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_element_pll.h"
#include "audio_element.h"
#include "hlog.h"

struct pll_element {
};

extern const ccm_analog_frac_pll_config_t g_audioPll1Config;
extern const ccm_analog_frac_pll_config_t g_audioPll2Config;

#define FracPLL_FDIV_CTL1_Offset (8U)

/* fout = fin * (mainDiv + dsm / 2^16) / (preDiv * 2^postDiv)
 * fnew = (1 + ppb/10^9) * fout
 * fnew = fin * (mainDiv + dsm_new / 2^16) / (preDiv * 2^postDiv)
 * dsm_new = dsm + ppb * (2^16 * mainDiv + dsm) / 10^9
 */
static void pll_adjust(int id, int64_t ppb)
{
	int16_t orig_dsm;
	int64_t mainDiv;
	int64_t dsm;

	switch (id) {
	case kCLOCK_AudioPll1Ctrl:
		orig_dsm = g_audioPll1Config.dsm;
		mainDiv = g_audioPll1Config.mainDiv;
		break;

	case kCLOCK_AudioPll2Ctrl:
		orig_dsm = g_audioPll2Config.dsm;
		mainDiv = g_audioPll2Config.mainDiv;
		break;

	default:
		goto exit;
		break;
	}

	dsm = orig_dsm + (ppb * ((1 << 16) * mainDiv + orig_dsm)) / 1000000000LL;

	if (dsm <= -(1 << 15))
		dsm = - ((1 << 15) - 1);
	else if (dsm >= (1 << 15))
		dsm = (1 << 15) - 1;

	CCM_ANALOG_TUPLE_REG_OFF(CCM_ANALOG, id, FracPLL_FDIV_CTL1_Offset) = (int16_t)dsm;

exit:
	return;
}

static int pll_element_run(struct audio_element *element)
{
	struct pll_element *pll = element->data;

	return 0;
}

static void pll_element_reset(struct audio_element *element)
{
	struct pll_element *pll = element->data;
}

static void pll_element_exit(struct audio_element *element)
{
}

static void pll_element_dump(struct audio_element *element)
{
	struct pll_element *pll = element->data;
}

int pll_element_check_config(struct audio_element_config *config)
{
	return 0;
}

unsigned int pll_element_size(struct audio_element_config *config)
{
	return sizeof(struct pll_element);
}

int pll_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	struct pll_element *pll = element->data;

	element->run = pll_element_run;
	element->reset = pll_element_reset;
	element->exit = pll_element_exit;
	element->dump = pll_element_dump;

	pll_element_dump(element);

	return 0;
}
