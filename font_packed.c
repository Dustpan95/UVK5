/* SPDX-License-Identifier: GPL-3.0-only
 * UVK5 lossless font expansion. Glyph copyright and license in font_packed.inc;
 * original font.c and its full Apache-2.0 notice are retained unchanged.
 */
#include "font.h"
#include "helper/packed.h"
#include "font_packed.inc"
FONT_Storage gFonts;
_Static_assert(sizeof(FONT_Storage)==2388
#ifdef ENABLE_SMALL_BOLD
    +564
#endif
    ,"Font layout must match the generator's byte sequence");
void FONT_Init(void)
{
    PACK_Expand(&gFonts,sizeof(gFonts),gPackedFonts);
}
