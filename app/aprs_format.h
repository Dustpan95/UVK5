/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_FORMAT_H
#define UVK5_APRS_FORMAT_H
#include <stdint.h>
#include <stdbool.h>
/* In-range microdegrees; caller supplies 9..11 bytes including the NUL.
 * Return excludes the NUL. spaces=false produces the on-air coordinate form. */
unsigned APRS_FormatDM(char *out,int32_t value,bool longitude,bool spaced);
#endif
