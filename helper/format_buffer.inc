/* SPDX-License-Identifier: GPL-3.0-only
 * Compact buffer formatting for the radio's integer/string displays.
 * Included by printf.c; device/callback output keeps its upstream formatter.
 */
typedef struct {
  char *buffer;
  size_t size, used;
} FormatBuffer;

static void FormatByte(FormatBuffer *out, char c)
{
  if (out->buffer && out->used < out->size) out->buffer[out->used] = c;
  ++out->used;
}

static int FormatBufferPrint(char *buffer, size_t size, const char *format, va_list va)
{
  FormatBuffer out = {buffer, size, 0};
  while (*format) {
    if (*format != '%') { FormatByte(&out, *format++); continue; }
    ++format;
    bool left = false, zero = false, alternate = false;
    char sign = 0;
    for (;;) {
      char flag = *format;
      if (flag == '-') left = true;
      else if (flag == '0') zero = true;
      else if (flag == '+') sign = '+';
      else if (flag == ' ') { if (!sign) sign = ' '; }
      else if (flag == '#') alternate = true;
      else break;
      ++format;
    }
    unsigned width = 0, precision = 0;
    if (*format == '*') {
      int value = va_arg(va, int); ++format;
      if (value < 0) { left = true; width = 0u - (unsigned)value; }
      else width = value;
    } else while (*format >= '0' && *format <= '9') width = width * 10 + *format++ - '0';
    bool precise = *format == '.';
    if (precise) {
      ++format;
      if (*format == '*') {
        int value = va_arg(va, int); ++format;
        precision = value > 0 ? (unsigned)value : 0;
      } else while (*format >= '0' && *format <= '9') precision = precision * 10 + *format++ - '0';
    }
    char length = 0;
    if (*format == 'l' || *format == 'h' || *format == 'z' || *format == 'j') {
      length = *format++;
      if (length == 'h' && *format == 'h') { length = 'H'; ++format; }
    }
    char type = *format++;
    char digits[sizeof(unsigned long) * 8], prefix[3];
    const char *text = digits;
    unsigned count = 0, prefixes = 0, zeros = 0;
    bool number = type == 'd' || type == 'i' || type == 'u' || type == 'x' || type == 'X' || type == 'o' || type == 'b';
    if (number) {
      unsigned base = type == 'o' ? 8 : type == 'b' ? 2 : type == 'x' || type == 'X' ? 16 : 10;
      bool wide = length == 'l' || length == 'z' || length == 'j';
      unsigned long value;
      if (type == 'd' || type == 'i') {
        long signedValue;
        if (wide) signedValue = va_arg(va, long);
        else {
          int v = va_arg(va, int);
          signedValue = length == 'h' ? (short)v : length == 'H' ? (char)v : v;
        }
        value = (unsigned long)signedValue;
        if (signedValue < 0) { sign = '-'; value = 0ul - value; }
        if (sign) prefix[prefixes++] = sign;
      } else {
        if (wide) value = va_arg(va, unsigned long);
        else {
          unsigned v = va_arg(va, unsigned);
          value = length == 'h' ? (unsigned short)v : length == 'H' ? (unsigned char)v : v;
        }
      }
      if (alternate && value && base != 10) {
        prefix[prefixes++] = '0';
        if (base == 16 || base == 2) prefix[prefixes++] = base == 2 ? 'b' : type;
      }
      if (value || !precise || precision) do {
        unsigned digit = value % base;
        digits[count++] = digit < 10 ? '0' + digit : (type == 'X' ? 'A' : 'a') + digit - 10;
        value /= base;
      } while (value);
      if (precision > count) zeros = precision - count;
      if (precise) zero = false;
    } else if (type == 's') {
      text = va_arg(va, const char *);
      while (text[count] && (!precise || count < precision)) ++count;
      zero = false;
    } else {
      digits[0] = type == 'c' ? (char)va_arg(va, int) : type;
      count = 1; zero = false;
    }
    unsigned total = count + zeros + prefixes;
    unsigned padding = width > total ? width - total : 0;
    if (!left && !zero) while (padding) { FormatByte(&out, ' '); --padding; }
    for (unsigned i = 0; i < prefixes; ++i) FormatByte(&out, prefix[i]);
    if (!left && zero) while (padding) { FormatByte(&out, '0'); --padding; }
    while (zeros) { FormatByte(&out, '0'); --zeros; }
    if (number) while (count) FormatByte(&out, digits[--count]);
    else for (unsigned i = 0; i < count; ++i) FormatByte(&out, text[i]);
    if (left) while (padding) { FormatByte(&out, ' '); --padding; }
  }
  if (buffer && size) buffer[out.used < size ? out.used : size - 1] = 0;
  return (int)out.used;
}
