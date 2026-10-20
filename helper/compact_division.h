/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_COMPACT_DIVISION_H
#define UVK5_COMPACT_DIVISION_H
#include <stdint.h>
/* Fundamental uint64_t return uses r0=quotient, r1=remainder on ARM AAPCS.
 * The assembly entry aliases also support quotient-only runtime callers. */
uint64_t COMPACT_UDivMod(uint32_t numerator,uint32_t denominator);
uint64_t COMPACT_SDivMod(int32_t numerator,int32_t denominator);
#endif
