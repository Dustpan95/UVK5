#!/usr/bin/env python3
"""Differentially test every source buffer format and truncation capacity."""
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
root=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('formats',root/'tools/check-radio-formats.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
formats=module.main()
checks=[]
for text in sorted(formats):
    args=[]
    for match in module.SPEC.finditer(text):
        fmt=match[0];kind=fmt[-1]
        if '*' in fmt:args.append('(int)(scenario%24)')
        if kind=='%':continue
        if kind=='s':args.append('texts[scenario%4]')
        elif kind=='c':args.append('(int)(scenario%3 ? 65+scenario%26 : 0)')
        elif kind=='d':args.append('signed_value' if 'l' not in fmt else '(long)signed_value')
        else:args.append('unsigned_value' if 'l' not in fmt else 'long_value')
    checks.append('        Compare('+re.sub(r'\\u00([0-9a-f]{2})',lambda m:'\\'+format(int(m[1],16),'03o'),json.dumps(text))+(' ,'+','.join(args) if args else '')+');')
# Keep the original comparison routine unchanged; cover the original caller
# formats rather than copying the implementation's branch conditions.
old=(root/'tests/test_format_buffer.c').read_text();source=old[:old.index('int main(void)')]
source+='''int sprintf_(char *,const char *,...);
int reference_sprintf(char *,const char *,...);
static unsigned unbounded_checks;
#define Unbounded(...) do { \\
    char a_buf[1024],b_buf[1024];memset(a_buf,0x55,sizeof a_buf);memset(b_buf,0x55,sizeof b_buf); \\
    int a=sprintf_(a_buf,__VA_ARGS__),b=reference_sprintf(b_buf,__VA_ARGS__); \\
    assert(a==b && !memcmp(a_buf,b_buf,sizeof a_buf));++unbounded_checks; \\
} while(0)
int main(void) {
    const unsigned numbers[]={0,1,9,10,15,16,99,100,999,100000,14439000,43000000,180000000,2147483647u,4294967295u};
    const char *texts[]={"","A","N0CALL-15","1234567890123456789012345678901234567890123456789012345678901234567890123"};
    for(unsigned scenario=0;scenario<64;scenario++) {
        unsigned unsigned_value=numbers[scenario%15];
        unsigned long long_value=scenario==63 ? ULONG_MAX:(unsigned long)unsigned_value;(void)long_value;
        int signed_value=(int)(unsigned_value%INT_MAX);if(scenario&1) signed_value=-signed_value;
'''+ '\n'.join(checks)+'\n'+'\n'.join(call.replace('Compare(', 'Unbounded(') for call in checks)+'''
    }
    /* The original formatter negates INT_MIN in signed arithmetic. Use libc
     * as the oracle for that defined output, across every truncation length. */
    for(unsigned limit=0;limit<64;limit++) {
        char actual[64],expected[64];memset(actual,0x55,sizeof actual);memset(expected,0x55,sizeof expected);
        int a=snprintf_(actual,limit,"%012d",INT_MIN);
        int b=snprintf(expected,limit,"%012d",INT_MIN);
        assert(a==b && !memcmp(actual,expected,sizeof actual));
    }
    assert(snprintf_(NULL,0,"%05d %s",-19,"TEST")==10);
    printf("PASS radio formatter: %u comparisons of every source format, truncated buffers, embedded NULs, unsigned boundaries and INT_MIN; %u unbounded comparisons\\n",checks,unbounded_checks);
}
'''
out=root/'build/format';out.mkdir(parents=True,exist_ok=True);(out/'radio_test.c').write_text(source)
compiler=os.environ.get('CC','gcc')
flags=['-std=c11','-Wall','-Wextra','-Werror','-g','-I',str(root),'-DPRINTF_INCLUDE_CONFIG_H']
if os.environ.get('SANITIZE','1')=='1':flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
subprocess.run([compiler,*flags,str(out/'radio_test.c'),str(root/'external/printf/printf.c'),str(out/'reference.o'),'-o',str(out/'radio_test')],check=True)
subprocess.run([str(out/'radio_test')],check=True)
