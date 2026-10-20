#define PRINTF_DISABLE_SUPPORT_LONG_LONG
#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL
#define PRINTF_DISABLE_SUPPORT_PTRDIFF_T
#define PRINTF_DISABLE_SUPPORT_FLOAT
/* Radio buffer output needs integer/string formatting. Keep the original
 * device/callback formatter and allow byte-for-byte regression comparisons. */
#ifndef PRINTF_LEGACY_BUFFER
#define PRINTF_COMPACT_BUFFER
#endif
