/* SPDX-License-Identifier: GPL-3.0-only
 * Buffer formats used by radio screens. See tools/check-radio-formats.py. */
typedef struct {char *buffer;size_t size,used;} FormatBuffer;
/* SIZE_MAX denotes sprintf: its caller supplies a valid, sufficient buffer.
 * Bounded snprintf keeps its capacity and NULL/zero-size behavior. */
static inline __attribute__((always_inline)) void FormatByte(FormatBuffer *out,char c)
{
    if(out->size==(size_t)-1 || (out->buffer && out->used<out->size)) out->buffer[out->used]=c;
    ++out->used;
}
static inline __attribute__((always_inline)) int FormatBufferPrint(char *buffer,size_t size,const char *format,va_list va)
{
    FormatBuffer out={buffer,size,0};
    while(*format) {
        if(*format!='%') {FormatByte(&out,*format++);continue;}
        ++format;
        bool zero=*format=='0';
        unsigned width=0,precision=0;
        while(*format>='0' && *format<='9') width=width*10+*format++-'0';
        bool precise=*format=='.';
        if(precise) {
            ++format;
            while(*format>='0' && *format<='9') precision=precision*10+*format++-'0';
        }
        char type=*format++;
        bool number=type=='d' || type=='u' || type=='o',negative=false;
        char digits[sizeof(unsigned)*3];
        const char *text=digits;unsigned count=0;
        if(number) {
            unsigned value;
            if(type=='d') {
                int v=va_arg(va,int);
                value=(unsigned)v;negative=v<0;
                if(negative) value=0u-value;
            } else value=va_arg(va,unsigned);
            unsigned base=type=='o' ? 8:10;
            char *end=digits+sizeof digits;char *start=end;
            do {*--start='0'+value%base;value/=base;} while(value);
            text=start;count=(unsigned)(end-start);
        } else if(type=='s') {
            text=va_arg(va,const char *);
            while(text[count] && (!precise || count<precision)) ++count;
        } else {digits[0]=type=='c' ? (char)va_arg(va,int):type;count=1;}
        unsigned total=count+negative,padding=width>total ? width-total:0;
        if(!number || !zero) while(padding) {FormatByte(&out,' ');--padding;}
        if(negative) FormatByte(&out,'-');
        while(padding) {FormatByte(&out,'0');--padding;}
        for(unsigned i=0;i<count;i++) FormatByte(&out,text[i]);
    }
    if(size==(size_t)-1) buffer[out.used]=0;
    else if(buffer && size) buffer[out.used<size ? out.used:size-1]=0;
    return (int)out.used;
}
