/* SPDX-License-Identifier: GPL-3.0-only
 * Verify generated streams with the actual firmware decoder and guarded output.
 */
#include "helper/packed.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void)
{
    uint32_t sizes[2];unsigned cases=0;
    while(fread(sizes,sizeof(sizes),1,stdin)==1) {
        unsigned n=sizes[0],compressed=sizes[1];assert(n<=16384 && compressed<=32768);
        uint8_t *expected=malloc(n+1),*input=malloc(compressed ? compressed:1),*output=malloc(n+32);
        assert(expected && input && output);
        assert(fread(expected,1,n,stdin)==n && fread(input,1,compressed,stdin)==compressed);
        memset(output,0xa5,n+32);
#ifdef PACK_HUFFMAN_STREAM
        PACK_HuffmanExpand(output+16,n,input);
#else
        PACK_Expand(output+16,n,input);
#endif
        assert(!memcmp(output+16,expected,n));
        for(unsigned i=0;i<16;++i) assert(output[i]==0xa5 && output[n+16+i]==0xa5);
        free(expected);free(input);free(output);++cases;
    }
    assert(feof(stdin) && cases>=50);
#ifdef PACK_HUFFMAN_STREAM
    printf("PASS Huffman/LZ resources: %u streams, exact bytes, guarded output, exact input bounds and overlapping matches\n",cases);
#else
    puts("PASS optimal resource streams decode exactly with guarded output, overlap and 4096-byte-window cases");
#endif
}
