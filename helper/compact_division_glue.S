/* SPDX-License-Identifier: GPL-3.0-only
 * ARM EABI entry points: quotient in r0, remainder in r1. Both the primitive
 * uint64_t C return and quotient-only consumers have that register contract.
 */
.syntax unified
.cpu cortex-m0
.thumb
.section .text.compact_division_u,"ax",%progbits
.balign 4
.global __aeabi_uidivmod
.type __aeabi_uidivmod,%function
.thumb_func
__aeabi_uidivmod:
    ldr r3,.LUnsignedDivider
    bx r3
.balign 4
.LUnsignedDivider:
    .word COMPACT_UDivMod
.size __aeabi_uidivmod,.-__aeabi_uidivmod
.global __aeabi_uidiv
.thumb_set __aeabi_uidiv,__aeabi_uidivmod
.global __udivsi3
.thumb_set __udivsi3,__aeabi_uidivmod

.section .text.compact_division_s,"ax",%progbits
.balign 4
.global __aeabi_idivmod
.type __aeabi_idivmod,%function
.thumb_func
__aeabi_idivmod:
    ldr r3,.LSignedDivider
    bx r3
.balign 4
.LSignedDivider:
    .word COMPACT_SDivMod
.size __aeabi_idivmod,.-__aeabi_idivmod
.global __aeabi_idiv
.thumb_set __aeabi_idiv,__aeabi_idivmod
.global __divsi3
.thumb_set __divsi3,__aeabi_idivmod
.section .note.GNU-stack,"",%progbits
