/* SPDX-License-Identifier: GPL-3.0-only */
#include "ui/display_text.h"
/* The host screen tests bypass firmware Main; reproduce its resource setup. */
__attribute__((constructor)) static void InitializeDisplayText(void)
{
    DISPLAY_TEXT_Init();
}
