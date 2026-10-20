/* SPDX-License-Identifier: GPL-3.0-only */
#include "dcs.h"
/* Host UI fixtures bypass Main's explicit early initialization. */
__attribute__((constructor)) static void InitializeDcs(void) {DCS_Init();}
