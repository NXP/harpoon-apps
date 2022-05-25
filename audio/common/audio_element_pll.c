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
