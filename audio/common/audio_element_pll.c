/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_element_pll.h"
#include "audio_element.h"
#include "hlog.h"
#include "stats.h"

#include "sai_drv.h"

#define PLL_SAMPLING_PERIOD_MS 10
#define PLL_MAX_PPB	(200000)
#define PLL_MIN_PPB	(-200000)

enum {
	PLL_STATE_UNLOCKED,
	PLL_STATE_LOCKING,
	PLL_STATE_LOCKED
};

struct pll_element {
	void *src_sai;
	void *dst_sai;
	int pll_id;

	unsigned int period;
	unsigned int count;

	int state;
	unsigned int sample_count;

	uint32_t src_bcr;
	uint32_t dst_bcr;

	int64_t bclk_offset;
	uint64_t src_bclk;
	uint64_t initial_src_bclk;
	uint64_t prev_src_bclk;
	uint64_t dst_bclk;
	uint64_t initial_dst_bclk;

	/* PI */
	unsigned int _ki;
	unsigned int _kp;
	int64_t integral;
	int64_t prev_err;
	int64_t prev_bclk_ppb;

	struct {
		struct stats bclk_err;
		struct stats bclk_ppb;
	} stats;
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
	uint32_t src_bcr, dst_bcr, mask;
	int64_t bclk_err, dt_bclk, bclk_ppb, err;

	pll->count++;
	if (pll->count < pll->period)
		goto out;

	/* Track overall bitclock */
	mask = DisableGlobalIRQ();
	src_bcr = __sai_rx_bitclock(pll->src_sai);
	dst_bcr = __sai_rx_bitclock(pll->dst_sai);
	EnableGlobalIRQ(mask);

	pll->src_bclk += src_bcr - pll->src_bcr;
	pll->dst_bclk += dst_bcr - pll->dst_bcr;

	bclk_err = pll->src_bclk - pll->dst_bclk - pll->bclk_offset;
	dt_bclk = pll->src_bclk - pll->prev_src_bclk;

	if (bclk_err)
		pll->prev_src_bclk = pll->src_bclk;

	switch (pll->state) {
	case PLL_STATE_UNLOCKED:
	default:
		/* Initial phase offset */
		pll->bclk_offset = pll->src_bclk - pll->dst_bclk;

		pll->initial_src_bclk = pll->src_bclk;
		pll->initial_dst_bclk = pll->dst_bclk;

		pll->prev_src_bclk = pll->src_bclk;

		pll->state = PLL_STATE_LOCKING;

		goto exit;
		break;

	case PLL_STATE_LOCKING:
		/* Delay first estimation of ppb to reduce initial error */
		if (pll->sample_count < 10)
			goto exit;

		/* Initial frequency ratio, in ppb */
		bclk_ppb = (1000000000LL * (int64_t)(pll->src_bclk - pll->initial_src_bclk)) / (pll->dst_bclk - pll->initial_dst_bclk) - 1000000000LL;

		bclk_ppb += pll->prev_bclk_ppb;

		pll->integral = (bclk_ppb + 1000000000LL) * pll->_ki;
		err = 0;

		pll->state = PLL_STATE_LOCKED;

		break;

	case PLL_STATE_LOCKED:

		err = (bclk_err * 1000000000LL) / dt_bclk;
		pll->integral += (err + pll->prev_err) / 2;

		bclk_ppb = ((err / pll->_kp) + (pll->integral / pll->_ki)) - 1000000000LL;

		break;
	}

	if (bclk_ppb > PLL_MAX_PPB)
		bclk_ppb = PLL_MAX_PPB;
	else if (bclk_ppb < PLL_MIN_PPB)
		bclk_ppb = PLL_MIN_PPB;

	if (bclk_ppb != pll->prev_bclk_ppb) {
		pll_adjust(pll->pll_id, bclk_ppb);
		pll->prev_bclk_ppb = bclk_ppb;
	}

	pll->prev_err = err;

	stats_update(&pll->stats.bclk_err, bclk_err);
	stats_update(&pll->stats.bclk_ppb, bclk_ppb);

exit:
	pll->src_bcr = src_bcr;
	pll->dst_bcr = dst_bcr;

	pll->sample_count++;
	pll->count = 0;

out:
	return 0;
}

static void pll_element_reset(struct audio_element *element)
{
	struct pll_element *pll = element->data;

	pll->src_bclk = 0;
	pll->dst_bclk = 0;
	pll->count = pll->period;
	pll->state = PLL_STATE_UNLOCKED;
	pll->sample_count = 0;

	stats_reset(&pll->stats.bclk_err);
	stats_reset(&pll->stats.bclk_ppb);
}

static void pll_element_exit(struct audio_element *element)
{
	struct pll_element *pll = element->data;

	pll_adjust(pll->pll_id, 0);
}

static void pll_element_dump(struct audio_element *element)
{
	struct pll_element *pll = element->data;

	log_info("pll(%p/%p), period: %u\n", pll, element, pll->period);
}

static void pll_element_stats(struct audio_element *element)
{
	struct pll_element *pll = element->data;

	log_info("pll(%p), samples: %u\n",
		pll, pll->sample_count);

	stats_compute(&pll->stats.bclk_err);
	stats_compute(&pll->stats.bclk_ppb);

	stats_print(&pll->stats.bclk_err);
	stats_print(&pll->stats.bclk_ppb);

	stats_reset(&pll->stats.bclk_err);
	stats_reset(&pll->stats.bclk_ppb);
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
	struct pll_element_config *pll_config = &config->u.pll;

	element->run = pll_element_run;
	element->reset = pll_element_reset;
	element->exit = pll_element_exit;
	element->dump = pll_element_dump;
	element->stats = pll_element_stats;

	pll->period = (PLL_SAMPLING_PERIOD_MS * element->sample_rate) / element->period / 1000;
	pll->prev_bclk_ppb = 0;
	pll->_kp = 4;
	pll->_ki = 64;

	pll->src_sai = __sai_base(pll_config->src_sai_id);
	pll->dst_sai = __sai_base(pll_config->dst_sai_id);
	pll->pll_id = pll_config->pll_id;

	pll_adjust(pll->pll_id, 0);

	stats_init(&pll->stats.bclk_err, 31, "err", NULL);
	stats_init(&pll->stats.bclk_ppb, 31, "ppb", NULL);

	pll_element_reset(element);

	pll_element_dump(element);

	return 0;
}
