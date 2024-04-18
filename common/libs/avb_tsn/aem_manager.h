/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AEM_MANAGER_H_
#define _AEM_MANAGER_H_

typedef enum {
	AEM_ENTITY_MIN_ID = 0,
	AEM_ENTITY_TALKER_LISTENER_AUDIO_DEFAULT_ID = AEM_ENTITY_MIN_ID,
	AEM_ENTITY_LISTENER_TALKER_AUDIO_SINGLE_MILAN_ID,
	AEM_ENTITY_MAX_ID
} aem_entity_id_t;

#define AEM_ID_VALID(id) ((id) >= AEM_ENTITY_MIN_ID && (id) < AEM_ENTITY_MAX_ID)

int aem_manager_create_entities(void);

#endif /* _AEM_MANAGER_H_ */
