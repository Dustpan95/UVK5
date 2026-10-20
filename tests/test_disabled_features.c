/* SPDX-License-Identifier: GPL-3.0-only */
#include "ui/menu.h"
#include "dcs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    /* Real side-button choices must not offer a missing flashlight action. */
    for(unsigned i=0;i<gSubMenu_SIDEFUNCTIONS_size;i++)
        assert(gSubMenu_SIDEFUNCTIONS[i].id!=ACTION_OPT_FLASHLIGHT);
    const uint8_t tones[]={MENU_R_CTCS,MENU_T_CTCS,MENU_R_DCS,MENU_T_DCS};
    for(unsigned i=0;i<sizeof(tones);i++) {
        unsigned index=UI_MENU_GetMenuIdx(tones[i]);
        assert(MenuList[index].menu_id==tones[i] && MenuList[index].name[0]);
    }
    assert(CTCSS_Options[0]>0 && DCS_Options[0]>0);
    puts("PASS actual menu tables retain manual RX/TX CTCSS/DCS and exclude flashlight bindings");
}
