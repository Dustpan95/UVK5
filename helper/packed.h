/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_PACKED_H
#define UVK5_PACKED_H
#include <stdint.h>
/* Expands build-generated, trusted tables. Not an external-input decoder.
 * Output size and backreferences are validated by resource equivalence tests.
 */
void PACK_Expand(void *output, unsigned size, const uint8_t *encoded);
#endif
