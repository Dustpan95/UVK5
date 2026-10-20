/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int vsnprintf_(char *, size_t, const char *, va_list);
int snprintf_(char *, size_t, const char *, ...);
int reference_vsnprintf(char *, size_t, const char *, va_list);
void _putchar(char c) {(void)c;}
static unsigned checks;

static void Compare(const char *format, ...)
{
    for (unsigned limit = 0; limit < 96; ++limit) {
        char actual[128], expected[128];
        memset(actual, 0x5a, sizeof(actual));
        memset(expected, 0x5a, sizeof(expected));
        va_list a, b;
        va_start(a, format); va_copy(b, a);
        int got = vsnprintf_(actual, limit, format, a);
        int want = reference_vsnprintf(expected, limit, format, b);
        va_end(a); va_end(b);
        if (got != want || memcmp(actual, expected, sizeof(actual))) {
            fprintf(stderr, "format mismatch %s at capacity %u: %d != %d\n", format, limit, got, want);
            assert(0);
        }
        ++checks;
    }
}

int main(void)
{
    const unsigned numbers[] = {0, 1, 9, 10, 15, 16, 99, 100, 999, 100000,
        14439000, 43000000, 180000000, 2147483647, 4294967295u};
    const char *unsignedFormats[] = {"%u", "%3u", "%03u", "%05u", "%-8u",
        "%.0u", "%.6u", "%8.6u", "%08.6u", "%x", "%X", "%o", "%b"};
    const char *signedFormats[] = {"%d", "%i", "%3d", "%05d", "%+8d", "% 8d",
        "%+-8d", "%.0d", "%08.6d"};
    for (unsigned i = 0; i < sizeof(numbers)/sizeof(*numbers); ++i) {
        unsigned n = numbers[i];
        for (unsigned f = 0; f < sizeof(unsignedFormats)/sizeof(*unsignedFormats); ++f) Compare(unsignedFormats[f], n);
        if (n <= INT_MAX) {
            for (unsigned f = 0; f < sizeof(signedFormats)/sizeof(*signedFormats); ++f) {
                Compare(signedFormats[f], (int)n); Compare(signedFormats[f], -(int)n);
            }
        }
        Compare("%lu.%05lu%c", (unsigned long)n, (unsigned long)(n % 100000), 'N');
        Compare("%3u.%05u", n / 100000, n % 100000);
        Compare("D%03oN", n);
        Compare("%d\n%u.%06u\nMHz", (int)(n % 100), n / 100000, n % 100000);
        Compare("%hhu %hu %hhd %hd", n, n, (int)(n % INT_MAX), (int)(n % INT_MAX));
    }
    const char *texts[] = {"", "A", "N0CALL-15", "1234567890123456789012345678901234567890123456789012345678901234567890123"};
    for (unsigned i = 0; i < sizeof(texts)/sizeof(*texts); ++i) {
        const char *s = texts[i];
        Compare("%s", s); Compare("%8s", s); Compare("%-8.3s", s);
        Compare("%c%.16s", '>', s); Compare("%.8s\n%.8s", s, s);
        Compare("%s %s", "TO", s); Compare("%c%c%s", '>', '*', s);
        for (int width = -10; width <= 10; ++width)
            for (int precision = -1; precision < 12; ++precision)
                Compare("%*.*s", width, precision, s);
    }
    Compare("%c %3u.%05u", 'A', 144u, 39000u);
    Compare("%cTX AUDIO %u", '>', 127u);
    Compare("%03u %02u.%02u%c", 180u, 0u, 0u, 'W');
    Compare("%u%% %c %5c", 100u, 'H', 'L');
    Compare("%c%c", 0, 'Z');
    Compare("literal only %%");
    /* The inherited formatter overflows signed negation for INT_MIN. Check
     * this boundary against libc instead of invoking that reference path. */
    char actual[32], expected[32];
    assert(snprintf_(actual, sizeof(actual), "%d", INT_MIN) ==
           snprintf(expected, sizeof(expected), "%d", INT_MIN));
    assert(!strcmp(actual, expected));
    printf("PASS compact buffer formatter: %u differential/truncation checks\n", checks);
    return 0;
}
