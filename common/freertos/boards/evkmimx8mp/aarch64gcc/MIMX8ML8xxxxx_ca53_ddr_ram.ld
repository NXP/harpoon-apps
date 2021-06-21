/*
** ###################################################################
**     Processors:          MIMX8ML8CVNKZ
**                          MIMX8ML8DVNLZ
**
**     Compiler:            GNU C Compiler
**     Reference manual:    IMX8MPRM, Rev.D, 10/2020
**     Version:             rev. 4.0, 2020-11-16
**     Build:               b201031
**
**     Abstract:
**         Linker file for the GNU C Compiler
**
**     Copyright 2016 Freescale Semiconductor, Inc.
**     Copyright 2016-2021 NXP
**     All rights reserved.
**
**     SPDX-License-Identifier: BSD-3-Clause
**
**     http:                 www.nxp.com
**     mail:                 support@nxp.com
**
** ###################################################################
*/

/* Entry Point */
ENTRY(Reset_Handler)

HEAP_SIZE  = DEFINED(__heap_size__)  ? __heap_size__  : 0x0400;
STACK_SIZE = DEFINED(__stack_size__) ? __stack_size__ : 0x0400;

/* Specify the memory areas */
MEMORY
{
  m_interrupts          (RX)  : ORIGIN = 0xC0000000, LENGTH = 0x00002000 /*  8 kB */
  m_text                (RX)  : ORIGIN = 0xC0002000, LENGTH = 0x005FE000 /* ~6 MB */
  m_data                (RW)  : ORIGIN = 0xC0600000, LENGTH = 0x00600000 /*  6 MB */
  m_data_nc             (RW)  : ORIGIN = 0xC0C00000, LENGTH = 0x00400000 /*  4 MB */
  ocram_text            (RX)  : ORIGIN = 0x00900000, LENGTH = 0x00030000 /* 176 kB */
  ocram_data            (RW)  : ORIGIN = 0x00930000, LENGTH = 0x00030000 /* 176 kB */
  itcm                  (RX)  : ORIGIN = 0x007E0000, LENGTH = 0x00020000 /* 128 kB */
  dtcm                  (RW)  : ORIGIN = 0x00800000, LENGTH = 0x00020000 /* 128 kB */
}

/* Define output sections */
SECTIONS
{
/* The startup code goes first into internal RAM */
  .interrupts :
  {
    __VECTOR_TABLE = .;
    __Vectors = .;
    . = ALIGN(64);
    KEEP(*(.boot))     /* Startup code */
  } > m_interrupts

  .resource_table :
  {
    . = ALIGN(64);
    KEEP(*(.resource_table)) /* Resource table */
  } > m_text

  .ocram_text :
  {
    __ocramtext_load__ = LOADADDR(.ocram_text);
    . = ALIGN(4096);
    __ocramtext_start__ = .;
    /* insert here code that goes in OCRAM */
    *ocram.c.obj(.text*)
    __ocramtext_init_end__ = .;
    . = ALIGN(4096);
    __ocramtext_end__ = .;
  } > ocram_text AT > m_text

  .ocram_data :
  {
    __ocramdata_load__ = LOADADDR(.ocram_data);
    . = ALIGN(4096);
    __ocramdata_start__ = .;
    /* insert here data that goes in OCRAM */
    *ocram.c.obj(.data*)
    *ocram.c.obj(*.rodata)
    . = ALIGN(8);
    __ocramdata_init_end__ = .;
  } > ocram_data AT > m_text

  .ocram_bss (NOLOAD):
  {
    /* insert here bss sections that go in OCRAM */
    *ocram.c.obj(.bss*)
    . = ALIGN(4096);
    __ocramdata_end__ = .;
  } > ocram_data

  .itcm :
  {
    __itcm_load__ = LOADADDR(.itcm);
    . = ALIGN(4096);
    __itcm_start__ = .;
    /* insert here code that goes in ITCM */
    *tcm.c.obj(.text*)
    __itcm_init_end__ = .;
    . = ALIGN(4096);
    __itcm_end__ = .;
  } > itcm AT > m_text

  .dtcm :
  {
    __dtcm_load__ = LOADADDR(.dtcm);
    . = ALIGN(4096);
    __dtcm_start__ = .;
    /* insert here data that goes in DTCM */
    *tcm.c.obj(.data*)
    *tcm.c.obj(*.rodata)
    . = ALIGN(8);
    __dtcm_init_end__ = .;
  } > dtcm AT > m_text

  .dtcm_bss (NOLOAD):
  {
    /* insert here bss sections that go in OCRAM */
    *tcm.c.obj(.bss*)
    . = ALIGN(4096);
    __dtcm_end__ = .;
  } > dtcm

  /* The program code and other data goes into internal RAM */
  .text :
  {
    . = ALIGN(64);
    *(.text)                 /* .text sections (code) */
    *(.text*)                /* .text* sections (code) */
    *(.rodata)               /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)              /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)               /* glue arm to thumb code */
    *(.glue_7t)              /* glue thumb to arm code */
    *(.eh_frame)
    . = ALIGN(64);
    KEEP (*(.init))
    . = ALIGN(64);
    KEEP (*(.fini))
  } > m_text

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > m_text

  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } > m_text

 .ctors :
  {
    . = ALIGN(64);
   __CTOR_LIST__ = .;
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       from the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    __CTOR_END__ = .;
  } > m_text

  .dtors :
  {
    . = ALIGN(64);
   __DTOR_LIST__ = .;
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    __DTOR_END__ = .;
  } > m_text

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } > m_text

  .init_array :
  {
   . = ALIGN(64);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
   . = ALIGN(64);
    PROVIDE_HIDDEN (__init_array_end = .);
  } > m_text

  .fini_array :
  {
   . = ALIGN(64);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
   . = ALIGN(4096);
    PROVIDE_HIDDEN (__fini_array_end = .);
  } > m_text

  __text = ORIGIN(m_interrupts);
  __etext = .;    /* define a global symbol at end of code */
  __DATA_ROM = .; /* Symbol is used by startup for data initialization */

  .data : AT(__DATA_ROM)
  {
    __DATA_RAM = .;
    __data_start__ = .;      /* create a global symbol at data start */
    *(.data)                 /* .data sections */
    *(.data*)                /* .data* sections */
    KEEP(*(.jcr*))
    . = ALIGN(64);
    __data_end__ = .;        /* define a global symbol at data end */
  } > m_data

  __CACHE_REGION_START = ORIGIN(m_interrupts);
  __CACHE_REGION_SIZE  = 0;
  __NDATA_ROM = __DATA_ROM + SIZEOF(.data); /* Symbol is used by startup for ncache data initialization */

  .ncache.init : AT(__NDATA_ROM)
  {
    __noncachedata_start__ = .;   /* create a global symbol at ncache data start */
    *(NonCacheable.init)
    . = ALIGN(64);
    __noncachedata_init_end__ = .;   /* create a global symbol at initialized ncache data end */
  } > m_data_nc

  . = __noncachedata_init_end__;
  .ncache :
  {
    *(NonCacheable)
    . = ALIGN(4096);
    __noncachedata_end__ = .;     /* define a global symbol at ncache data end */
  } > m_data_nc

  __DATA_END = __DATA_ROM + (__data_end__ - __data_start__);
  text_end = ORIGIN(m_text) + LENGTH(m_text);
  ASSERT(__DATA_END <= text_end, "region m_text overflowed with text and data")

  /* Uninitialized data section */
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    . = ALIGN(64);
    __START_BSS = .;
    __bss_start__ = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(64);
    __bss_end__ = .;
    __END_BSS = .;
  } > m_data

  .heap :
  {
    . = ALIGN(64);
    __end__ = .;
    PROVIDE(end = .);
    __HeapBase = .;
    . += HEAP_SIZE;
    __HeapLimit = .;
    __heap_limit = .; /* Add for _sbrk */
  } > m_data


  .stack :
  {
    . = ALIGN(64);
    _el1_stack_end = .;
    . += STACK_SIZE;
    . = ALIGN(64);
    __el1_stack = .;
    _el0_stack_end = .;
    . += STACK_SIZE;
    . = ALIGN(64);
    __el0_stack = .;
  } > m_data


  /* Initializes stack on the end of block */
  __StackTop   = ORIGIN(m_data) + LENGTH(m_data);
  __StackLimit = __StackTop - 2 * STACK_SIZE;
  PROVIDE(__stack = __StackTop);

/*  .ARM.attributes 0 : { *(.ARM.attributes) } */

  ASSERT(__StackLimit >= __HeapLimit, "region m_data overflowed with stack and heap")
}
