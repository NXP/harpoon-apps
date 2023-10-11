/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GEN_SW_MBOX_CONFIG_H_
#define GEN_SW_MBOX_CONFIG_H_

#include "os/irq.h"

//! @def GEN_SW_MBOX_BASE
//!
//! Specify the sharing memory base address used for mailbox.
#define GEN_SW_MBOX_BASE (0xb8500000)
//@}

//! @def GEN_SW_MBOX_IRQ
//!
//! Specify the interrupt number for handling the received message.
#define GEN_SW_MBOX_IRQ (109)
//@}

//! @def GEN_SW_MBOX_REMOTE_IRQ
//!
//! Specify the interrupt number to trigger for notifying remote when send message.
#define GEN_SW_MBOX_REMOTE_IRQ (108)
//@}

//! @def GEN_SW_MBOX_IRQ_PRIO
//!
//! Specify the priority of the interrupt.
#define GEN_SW_MBOX_IRQ_PRIO OS_IRQ_PRIO_DEFAULT
//@}

#endif /* GEN_SW_MBOX_CONFIG_H_ */
