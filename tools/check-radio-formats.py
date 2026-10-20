#!/usr/bin/env python3
"""Reject buffer-format syntax outside the radio formatter's tested contract.
Checks source calls in every feature configuration, including inactive #ifdefs.
Device/callback printf retains its upstream formatter.
"""
import ast
from pathlib import Path
import re
ROOT=Path(__file__).resolve().parents[1]
TOKEN=re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[A-Za-z_]\w*|[^\s]',re.S)
SPEC=re.compile(r'%0?\d*(?:\.\d+)?[duocs%]')
CALLS={'sprintf':1,'snprintf':2,'vsnprintf':2,'vsprintf':1,
       'sprintf_':1,'snprintf_':2,'vsnprintf_':2,'vsprintf_':1}
# Upstream's two named alarm-mode strings contain no conversion directives.
DYNAMIC={'gSubMenu_AL_MOD[gSubMenuSelection]','gSubMenu_AL_MOD[selection]'}
def validate(value):
    pos=0
    while (pos:=value.find('%',pos))>=0:
        match=SPEC.match(value,pos)
        if not match:raise ValueError('unsupported conversion: '+value[pos:])
        spec=match[0];typ=spec[-1]
        if ('.' in spec and typ!='s') or ('l' in spec and typ not in 'duo'):
            raise ValueError('unsupported conversion: '+spec)
        pos=match.end()
    return True

def main():
    calls=0;formats=set()
    for file in sorted(ROOT.rglob('*.c')):
        if file.relative_to(ROOT).parts[0] in {'build','tests','external'}:continue
        source=file.read_text();tokens=[m for m in TOKEN.finditer(source) if not m[0].startswith(('/',)) or m[0]=='/']
        for i,t in enumerate(tokens[:-1]):
            if t[0] not in CALLS or tokens[i+1][0]!='(':continue
            depth=0;arguments=[];start=i+2
            for j in range(start,len(tokens)):
                v=tokens[j][0]
                if v=='(' or v=='[' or v=='{':depth+=1
                elif v in (')',']','}'):
                    if v==')' and not depth:
                        arguments.append(tokens[start:j]);break
                    depth-=1
                elif v==',' and not depth:arguments.append(tokens[start:j]);start=j+1
            else:raise ValueError('unclosed call '+str(file))
            fmt=arguments[CALLS[t[0]]]
            literals=[m[0] for m in fmt if m[0].startswith('"')]
            if not literals:
                expression=''.join(m[0] for m in fmt)
                if expression not in DYNAMIC:raise ValueError(f'{file}: unchecked dynamic format {expression}')
            for literal in literals:
                value=ast.literal_eval(literal);validate(value);formats.add(value)
            calls+=1
    # Negative fixtures ensure validation cannot silently admit a removed flag.
    for rejected in ['%x','%b','%+d','%-8s','%hhu','%.*u','%.*s','%lld','%p','%.2f']:
        try:validate(rejected)
        except ValueError:continue
        raise AssertionError('accepted unsupported syntax '+rejected)
    print(f'PASS {calls} buffer format call sites, {len(formats)} literal formats, inactive features and rejection fixtures')
    return formats
if __name__=='__main__':main()
