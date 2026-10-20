/* SPDX-License-Identifier: GPL-3.0-only */
#include "display_text.h"
#include "helper/packed.h"
#include "display_text_packed.inc"
DisplayResources gDisplayResources;
_Static_assert(sizeof(DisplayResources)==DISPLAY_RESOURCE_BYTES,"resource layout padding");
void DISPLAY_TEXT_Init(void)
{
    /* Main calls this before any screen or formatter uses the pooled text. */
    PACK_Expand(&gDisplayResources,sizeof(gDisplayResources),gPackedDisplayText);
}
