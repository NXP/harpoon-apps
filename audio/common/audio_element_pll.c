/*
 * Copyright 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rtos_abstraction_layer.h"

#include "audio_app.h"
#include "audio_element_pll.h"
#include "audio_element.h"
#include "rtos_apps/log.h"
#include "hrpn_ctrl.h"
#include "rtos_apps/stats.h"

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

	rtos_mutex_t mutex;
	bool enabled;

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
		struct rtos_apps_stats bclk_err;
		struct rtos_apps_stats bclk_ppb;
	} stats;
};

static void pll_element_reset(struct audio_element *element);

__WEAK void pll_adjust(int id, int64_t ppb)
{

}

static void pll_element_response(void *ctrl_handle, uint32_t status)
{
	struct hrpn_resp_audio_element resp;

	if (ctrl_handle) {
		resp.type = HRPN_RESP_TYPE_AUDIO_ELEMENT_PLL;
		resp.status = status;
		audio_app_ctrl_send(ctrl_handle, &resp, sizeof(resp));
	}
}

int pll_element_ctrl(struct audio_element *element, struct hrpn_cmd_audio_element_pll *cmd, unsigned int len, void *ctrl_handle)
{
	struct pll_element *pll;

	if (!element)
		goto err;

	if (element->type != AUDIO_ELEMENT_PLL)
		goto err;

	pll = element->data;

	switch (cmd->u.common.type) {
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_ENABLE:
		rtos_mutex_lock(&pll->mutex, RTOS_WAIT_FOREVER);

		pll_element_reset(element);
		pll->enabled = true;

		rtos_mutex_unlock(&pll->mutex);

		break;

	case HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_DISABLE:
		rtos_mutex_lock(&pll->mutex, RTOS_WAIT_FOREVER);

		pll->enabled = false;

		rtos_mutex_unlock(&pll->mutex);

		break;

	case HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_ID:
		rtos_mutex_lock(&pll->mutex, RTOS_WAIT_FOREVER);

		pll->pll_id = cmd->pll_id;

		pll_element_reset(element);

		rtos_mutex_unlock(&pll->mutex);

		break;

	default:
		goto err;
		break;
	}

	pll_element_response(ctrl_handle, HRPN_RESP_STATUS_SUCCESS);

	return 0;

err:
	pll_element_response(ctrl_handle, HRPN_RESP_STATUS_ERROR);
	return -1;
}

static int pll_element_run(struct audio_element *element)
{
	struct pll_element *pll = element->data;
	uint32_t src_bcr, dst_bcr, mask;
	int64_t bclk_err, dt_bclk, bclk_ppb, err;

	rtos_mutex_lock(&pll->mutex, RTOS_WAIT_FOREVER);

	if (!pll->enabled)
		goto out;

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

	rtos_apps_stats_update(&pll->stats.bclk_err, bclk_err);
	rtos_apps_stats_update(&pll->stats.bclk_ppb, bclk_ppb);

exit:
	pll->src_bcr = src_bcr;
	pll->dst_bcr = dst_bcr;

	pll->sample_count++;
	pll->count = 0;

out:
	rtos_mutex_unlock(&pll->mutex);

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

	rtos_apps_stats_reset(&pll->stats.bclk_err);
	rtos_apps_stats_reset(&pll->stats.bclk_ppb);
}

static void pll_element_exit(struct audio_element *element)
{
	struct pll_element *pll = element->data;

	pll_adjust(pll->pll_id, 0);
}

static void pll_element_dump(struct audio_element *element)
{
	struct pll_element *pll = element->data;

	log_info("pll(%p/%p), enabled: %u, period: %u\n", pll, element, pll->enabled, pll->period);
}

static void pll_element_stats(struct audio_element *element)
{
	struct pll_element *pll = element->data;

	log_info("pll(%p), samples: %u\n",
		pll, pll->sample_count);

	rtos_apps_stats_compute(&pll->stats.bclk_err);
	rtos_apps_stats_compute(&pll->stats.bclk_ppb);

	rtos_apps_stats_print(&pll->stats.bclk_err);
	rtos_apps_stats_print(&pll->stats.bclk_ppb);

	rtos_apps_stats_reset(&pll->stats.bclk_err);
	rtos_apps_stats_reset(&pll->stats.bclk_ppb);
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

	if (rtos_mutex_init(&pll->mutex))
		goto err;

	element->run = pll_element_run;
	element->reset = pll_element_reset;
	element->exit = pll_element_exit;
	element->dump = pll_element_dump;
	element->stats = pll_element_stats;

	pll->enabled = true;
	pll->period = (PLL_SAMPLING_PERIOD_MS * element->sample_rate) / element->period / 1000;
	pll->prev_bclk_ppb = 0;
	pll->_kp = 4;
	pll->_ki = 64;

	pll->src_sai = __sai_base(pll_config->src_sai_id);
	pll->dst_sai = __sai_base(pll_config->dst_sai_id);
	pll->pll_id = pll_config->pll_id;

	pll_adjust(pll->pll_id, 0);

	rtos_apps_stats_init(&pll->stats.bclk_err, 31, "err", NULL);
	rtos_apps_stats_init(&pll->stats.bclk_ppb, 31, "ppb", NULL);

	pll_element_reset(element);

	pll_element_dump(element);

	return 0;

err:
	return -1;
}
