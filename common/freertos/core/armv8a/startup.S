/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
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

	/*
	 * Mask all exceptions until _freertos_vector_table is registered,
	 * after vTaskStartScheduler() is called
	 */
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
	# TODO D-cache flush: make sure code relocation has reached memory
	# __flush_dcache_area(__ocramtext_start__, __ocramtext_init_end__ - __ocramtext_start__);
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
	# TODO D-cache flush: make sure code relocation has reached memory
	# __flush_dcache_area(__itcm_start__, __itcm_init_end__ - __itcm_start__);
	ic	ialluis
	isb

.LC12:
	ic	ialluis
	isb

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

	isb

	b	main

	.ltorg

/*      Temporary vector table, used up to FreeRTOS vTaskStartScheduler() */
	.globl vectors
	.align 11
vectors:
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