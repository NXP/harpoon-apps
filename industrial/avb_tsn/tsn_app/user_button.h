/*
 * Copyright 2021-2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef USER_BUTTON_H_
#define USER_BUTTON_H_

#include <stdbool.h>
#include "rtos_abstraction_layer.h"

#if ENABLE_USER_BUTTON == 1
/*
 * Init wakeup GPIO handling task
*/
int init_gpio_handling_task();

/*
 * Register an event queue for user button input events
*/
int user_button_add_event_queue(rtos_mqueue_t **evt_queue);

/*
 * Trigger a user button input through software
*/
void user_button(bool from_isr);

#else
static inline int init_gpio_handling_task(void)
{
    return 0;
}

static inline int user_button_add_event_queue(rtos_mqueue_t **evt_queue)
{
    return 0;
}

static inline void user_button(bool from_isr)
{
}
#endif

#endif /* USER_BUTTON_H_ */
