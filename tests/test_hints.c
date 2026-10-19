/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "helper/packed.h"
#include "ui/menu_hints.inc"
#include "ui/menu_hints_packed.inc"
#ifdef TEST_HINT_LOOKUP
#include "ui/menu.h"
#include "ui/vuurwerk_menu.h"
#endif
int main(void) {
    uint8_t decoded[MENU_HINT_BYTES];
    assert(sizeof(sHintPool)==sizeof(decoded));
    PACK_Expand(decoded,sizeof(decoded),gPackedMenuHints);
    assert(!memcmp(decoded,sHintPool,sizeof(decoded)));
    puts("PASS every original menu hint byte preserved");
#ifdef TEST_HINT_LOOKUP
    assert(!strcmp(VUURWERK_MENU_GetHint(MENU_D_LIVE_DEC),"LIVE DTMF DECODER ON SCREEN"));
    assert(!strcmp(VUURWERK_MENU_GetHint(MENU_VOL),"BATTERY VOLTAGE READOUT"));
    assert(!strcmp(VUURWERK_MENU_GetHint(MENU_TDR),"DUAL WATCH / CROSSBAND RX"));
    assert(!strcmp(VUURWERK_MENU_GetHint(MENU_SCANWATCH),"WATCH OTHER VFO IN SCAN"));
    assert(!VUURWERK_MENU_GetHint(MENU_ABOUT));
#ifdef ENABLE_AM_FIX
    assert(!strcmp(VUURWERK_MENU_GetHint(MENU_AM_FIX),"AM OVERLOAD AUTO GAIN FIX"));
#endif
    puts("PASS actual menu hint lookup after optional AM-fix slot");
#endif
}
