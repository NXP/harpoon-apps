/*
 * Jailhouse AArch64 support
 *
 * Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
 * Copyright 2021 NXP Semiconductors
 *
 * Authors:
 *  Antonios Motakis <antonios.motakis@huawei.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 * Alternatively, you can use or redistribute this file under the following
 * BSD license:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

.macro	ventry	label
	.align	7
	b	\label
.endm

	.section ".boot", "ax"
	.globl Reset_Handler
Reset_Handler:
	ldr	x0, =vectors
	msr	vbar_el1, x0

	/* Mask all exceptions */
	msr	DAIFSet, #0xf

	/* Disable alignment fault checking */
	mrs	x0, sctlr_el1
	bic	x0, x0, #(0x1 << 1) /* SCTLR_A_BIT */
	orr	x0, x0, #(0x1 << 2) /* SCTLR_C_BIT: data cache ON */
	orr	x0, x0, #(0x1 << 12) /* SCTLR_I_BIT: instr cache ON */
	msr	sctlr_el1, x0

	# set the stack pointer for EL0
	msr	SPSel, #0
	ldr	x0, =__el0_stack
	mov	sp, x0

	isb

	# set the stack pointer for EL1
	msr	SPSel, #1
	ldr	x0, =__el1_stack
	mov	sp, x0

/*     Loop to copy data from read only memory to RAM. The ranges
 *      of copy from/to are specified by following symbols evaluated in
 *      linker script.
 *      __data_load__: End of code section, i.e., begin of data sections to copy from.
 *      __noncachedata_start__/__noncachedata_end__ : none cachable region
 *      __data_start__/__data_init_end__: RAM address range that data should be
 *      copied to. Both must be aligned to 64-byte boundary.  */

	ldr	x1, =__data_load__
	ldr	x2, =__data_start__
	ldr	x3, =__data_init_end__

/* Here is an implementation that favors performance over code size. */
	subs	x3, x3, x2
	ble	.LC1
.LC0:
	subs	x3, x3, #8
	ldr	x0, [x1, x3]
	str	x0, [x2, x3]
	bgt	.LC0
.LC1:
#ifdef __STARTUP_INITIALIZE_NONCACHEDATA
	ldr	x1, =__noncachedata_load__
	ldr	x2, =__noncachedata_start__
	ldr	x3, =__noncachedata_init_end__
/* Here is an implementation that favors performance over code size. */
	subs	x3, x3, x2
	ble	.LC3
.LC2:
	subs	x3, x3, #8
	ldr	x0, [x1, x3]
	str	x0, [x2, x3]
	bgt	.LC2
.LC3:
/* zero init ncache data section */
	ldr	x0, =__noncachedata_init_end__
	ldr	x1, =__noncachedata_end__
	sub	x1, x1, x0
	cbz	x1, .L__noncachedata_cleared
.LC4:
	sub	x1, x1, #8
	str	xzr, [x0], #8
	cbnz	x1, .LC4
.L__noncachedata_cleared:
#endif /* __STARTUP_INITIALIZE_NONCACHEDATA */

#ifdef __STARTUP_CLEAR_BSS
/*     This part of work usually is done in C library startup code. Otherwise,
 *     define this macro to enable it in this startup.
 *
 *     Loop to zero out BSS section, which uses following symbols
 *     in linker script:
 *      __bss_start__: start of BSS section. Must align to 8
 *      __bss_end__: end of BSS section. Must align to 8
 */
	ldr	x0, =__bss_start__
	ldr	x1, =__bss_end__
	sub	x1, x1, x0
	cbz	x1, .L__bss_cleared
.LC5:
	sub	x1, x1, #8
	str	xzr, [x0], #8
	cbnz	x1, .LC5
.L__bss_cleared:
#endif /* __STARTUP_CLEAR_BSS */

/* Copy OCRAM code from RO region to OCRAM */
	ldr	x1, =__ocramtext_load__
	ldr	x2, =__ocramtext_start__
	ldr	x3, =__ocramtext_init_end__

	subs	x3, x3, x2
	ble	.LC7
.LC6:
	subs	x3, x3, #8
	ldr	x0, [x1, x3]
	str	x0, [x2, x3]
	bgt	.LC6
.LC7:

/* Copy OCRAM data from RO region to OCRAM */
	ldr	x1, =__ocramdata_load__
	ldr	x2, =__ocramdata_start__
	ldr	x3, =__ocramdata_init_end__

	subs	x3, x3, x2
	ble	.LC9
.LC8:
	subs	x3, x3, #8
	ldr	x0, [x1, x3]
	str	x0, [x2, x3]
	bgt	.LC8
.LC9:
/* zero init OCRAM bss */
	ldr	x0, =__ocramdata_init_end__
	ldr	x1, =__ocramdata_end__
	sub	x1, x1, x0
	cbz	x1, .L__ocramdata_cleared
.LC10:
	sub	x1, x1, #8
	str	xzr, [x0], #8
	cbnz	x1, .LC10
.L__ocramdata_cleared:

/* Copy ITCM code from RO region to ITCM */
	ldr	x1, =__itcm_load__
	ldr	x2, =__itcm_start__
	ldr	x3, =__itcm_init_end__

	subs	x3, x3, x2
	ble	.LC12
.LC11:
	subs	x3, x3, #8
	ldr	x0, [x1, x3]
	str	x0, [x2, x3]
	bgt	.LC11
.LC12:

/* Copy DTCM data from RO region to DTCM */
	ldr	x1, =__dtcm_load__
	ldr	x2, =__dtcm_start__
	ldr	x3, =__dtcm_init_end__

	subs	x3, x3, x2
	ble	.LC14
.LC13:
	subs	x3, x3, #8
	ldr	x0, [x1, x3]
	str	x0, [x2, x3]
	bgt	.LC13
.LC14:
/* zero init DTCM bss */
	ldr	x0, =__dtcm_init_end__
	ldr	x1, =__dtcm_end__
	sub	x1, x1, x0
	cbz	x1, .L__dtcm_cleared
.LC15:
	sub	x1, x1, #8
	str	xzr, [x0], #8
	cbnz	x1, .LC15
.L__dtcm_cleared:

	# enable FPU/NEON instructions
	mov	x0, #(3 << 20)
	msr	cpacr_el1, x0

	msr	daif, xzr

	isb

	b	main

handle_irq:
	sub sp, sp, #(16 * 16)
	stp x0, x1, [sp, #(0 * 16)]
	stp x2, x3, [sp, #(1 * 16)]
	stp x4, x5, [sp, #(2 * 16)]
	stp x6, x7, [sp, #(3 * 16)]
	stp x8, x9, [sp, #(4 * 16)]
	stp x10, x11, [sp, #(5 * 16)]
	stp x12, x13, [sp, #(6 * 16)]
	stp x14, x15, [sp, #(7 * 16)]
	stp x16, x17, [sp, #(8 * 16)]
	stp x18, x19, [sp, #(9 * 16)]
	stp x20, x21, [sp, #(10 * 16)]
	stp x22, x23, [sp, #(11 * 16)]
	stp x24, x25, [sp, #(12 * 16)]
	stp x26, x27, [sp, #(13 * 16)]
	stp x28, x29, [sp, #(14 * 16)]
	str x30, [sp, #(15 * 16)]

	bl	DefaultISR

	ldp x0, x1, [sp, #(0 * 16)]
	ldp x2, x3, [sp, #(1 * 16)]
	ldp x4, x5, [sp, #(2 * 16)]
	ldp x6, x7, [sp, #(3 * 16)]
	ldp x8, x9, [sp, #(4 * 16)]
	ldp x10, x11, [sp, #(5 * 16)]
	ldp x12, x13, [sp, #(6 * 16)]
	ldp x14, x15, [sp, #(7 * 16)]
	ldp x16, x17, [sp, #(8 * 16)]
	ldp x18, x19, [sp, #(9 * 16)]
	ldp x20, x21, [sp, #(10 * 16)]
	ldp x22, x23, [sp, #(11 * 16)]
	ldp x24, x25, [sp, #(12 * 16)]
	ldp x26, x27, [sp, #(13 * 16)]
	ldp x28, x29, [sp, #(14 * 16)]
	ldr x30, [sp, #(15 * 16)]
	add sp, sp, #(16 * 16)

	eret

.weak DefaultISR
	b	.

	.globl vectors
	.align 11
/*      Temporary vector table, used up to FreeRTOS vTaskStartScheduler() */
vectors:
	ventry	.
	ventry	.
	ventry	.
	ventry	.

	ventry	.
	ventry	handle_irq
	ventry	.
	ventry	.

	ventry	.
	ventry	handle_irq
	ventry	.
	ventry	.

	ventry	.
	ventry	.
	ventry	.
	ventry	.

	.ltorg

/*      FreeRTOS vector table used after FreeRTOS vTaskStartScheduler() is called */
	.globl _freertos_vector_table
	.align 11
_freertos_vector_table:
	ventry	FreeRTOS_SWI_Handler
	ventry	FreeRTOS_IRQ_Handler
	ventry	.
	ventry	.

	ventry	FreeRTOS_SWI_Handler
	ventry	FreeRTOS_IRQ_Handler
	ventry	.
	ventry	.

	ventry	.
	ventry	.
	ventry	.
	ventry	.

	ventry	.
	ventry	.
	ventry	.
	ventry	.
