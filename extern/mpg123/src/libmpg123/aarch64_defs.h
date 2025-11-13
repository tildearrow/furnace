/* SPDX-License-Identifier: LGPL-2.1
 *
 *	aarch64_defs.h: Support macros for the aarch64 architectural features
 */

#ifndef SRC_LIBMPG123_AARCH64_DEFS_H_
#define SRC_LIBMPG123_AARCH64_DEFS_H_

/*
 * Guard this header so arm assembly files can just include it without the need
 * to if-def each instance.
 */
#ifdef __aarch64__

/*
 * References:
 *  - https://developer.arm.com/documentation/101028/0012/5--Feature-test-macros
 *  - https://github.com/ARM-software/abi-aa/blob/main/aaelf64/aaelf64.rst
 */

#if defined(__ARM_FEATURE_BTI_DEFAULT) && __ARM_FEATURE_BTI_DEFAULT == 1
  #define GNU_PROPERTY_AARCH64_BTI 1 /* bit 0 GNU Notes is for BTI support */
#else
  #define GNU_PROPERTY_AARCH64_BTI 0
#endif

#if defined(__ARM_FEATURE_PAC_DEFAULT)
  #define GNU_PROPERTY_AARCH64_POINTER_AUTH 2 /* bit 1 GNU Notes is for PAC support */
#else
  #define GNU_PROPERTY_AARCH64_POINTER_AUTH 0
#endif

/* Add the BTI support to GNU Notes section */
#if defined(__ASSEMBLER__) && defined(__ELF__)
#if GNU_PROPERTY_AARCH64_BTI != 0 || GNU_PROPERTY_AARCH64_POINTER_AUTH != 0
    .pushsection .note.gnu.property, "a"; /* Start a new allocatable section */
    .balign 8; /* align it on a byte boundry */
    .long 4; /* size of "GNU\0" */
    .long 0x10; /* size of descriptor */
    .long 0x5; /* NT_GNU_PROPERTY_TYPE_0 */
    .asciz "GNU";
    .long 0xc0000000; /* GNU_PROPERTY_AARCH64_FEATURE_1_AND */
    .long 4; /* Four bytes of data */
    .long (GNU_PROPERTY_AARCH64_BTI|GNU_PROPERTY_AARCH64_POINTER_AUTH); /* BTI or PAC is enabled */
    .long 0; /* padding for 8 byte alignment */
    .popsection; /* end the section */
#endif /* GNU Notes additions */
#endif /* if __ASSEMBLER__ and __ELF__ */

#endif /* __arch64__ */

#endif /* SRC_LIBMPG123_AARCH64_DEFS_H_ */
