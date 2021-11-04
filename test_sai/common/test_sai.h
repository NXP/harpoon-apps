/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TEST_SAI_H_
#define _TEST_SAI_H_

uint32_t get_sai_id(I2S_Type *base);

void sai_setup(uint8_t sai_id);

void sai_test_task(void *parameters);

#endif /* _TEST_SAI_H_ */
