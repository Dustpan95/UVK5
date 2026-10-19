/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "helper/packed.h"
#include "ui/menu_hints.inc"
#include "ui/menu_hints_packed.inc"
int main(void) {
    uint8_t decoded[MENU_HINT_BYTES];
    assert(sizeof(sHintPool)==sizeof(decoded));
    PACK_Expand(decoded,sizeof(decoded),gPackedMenuHints);
    assert(!memcmp(decoded,sHintPool,sizeof(decoded)));
    puts("PASS every original menu hint byte preserved");
}
