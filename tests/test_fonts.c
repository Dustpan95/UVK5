/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdio.h>
#include "font.h"
int main(void)
{
    FONT_Init();FONT_Init(); /* Reinitialization must remain bit-exact. */
    fwrite(gFontBig,1,sizeof(gFontBig),stdout);
    fwrite(gFontBigDigits,1,sizeof(gFontBigDigits),stdout);
    fwrite(gFont3x5,1,sizeof(gFont3x5),stdout);
    fwrite(gFontSmall,1,sizeof(gFontSmall),stdout);
#ifdef ENABLE_SMALL_BOLD
    fwrite(gFontSmallBold,1,sizeof(gFontSmallBold),stdout);
#endif
    return ferror(stdout)?1:0;
}
