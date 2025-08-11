/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 @brief AECP common code
 @details Handles AECP stack
*/

#include "log.h"

#include "genavb/aem.h"
#include "genavb/aem_helpers.h"

#include "aem_manager.h"
#include "aem_manager_helpers.h"
#include "aem_manager_rtos.h"

struct aem_desc_hdr aem_desc_list[AEM_ENTITY_MAX_ID][AEM_NUM_DESC_TYPES] = {
	[AEM_ENTITY_TALKER_LISTENER_AUDIO_DEFAULT_ID] = {},
	[AEM_ENTITY_LISTENER_TALKER_AUDIO_SINGLE_MILAN_ID] = {},
};

extern struct aem_desc_handler desc_handler[AEM_NUM_DESC_TYPES];

extern void talker_listener_audio_default_init(struct aem_desc_hdr *aem_desc);
extern void listener_talker_audio_single_milan_init(struct aem_desc_hdr *aem_desc);

int aem_manager_create_entities(void)
{
	int rc = 0;

	entity_desc_handler_init(desc_handler);

	if ((aem_entity_create(aem_desc_list[AEM_ENTITY_TALKER_LISTENER_AUDIO_DEFAULT_ID], talker_listener_audio_default_init) < 0)
	 || (aem_entity_create(aem_desc_list[AEM_ENTITY_LISTENER_TALKER_AUDIO_SINGLE_MILAN_ID], listener_talker_audio_single_milan_init) < 0)
		) {

		log_err("entity generation failed\n");
		rc = -1;
		goto out;
	}

out:
	return rc;
}
