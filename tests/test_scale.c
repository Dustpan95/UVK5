/* SPDX-License-Identifier: Apache-2.0 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "app/aprs_scale.h"
int main(void)
{
    /* Exhaustively compare every valid compressed ordinate with the old exact
     * 64-bit arithmetic. There is no tolerance or loss of position precision. */
    for(uint32_t n=0;n<=68566680u;n++) {
        assert(APRS_ScaleMillion(n,190463u)==(uint64_t)n*1000000u/190463u);
        assert(APRS_ScaleMillion(n,380926u)==(uint64_t)n*1000000u/380926u);
    }
    puts("PASS exact compressed-position scaling: 137,133,362 values, both divisors");
}
