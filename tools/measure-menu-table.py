#!/usr/bin/env python3
"""Probe a shared enum-menu formatter in ignored oversized copies.
No production UI changes. Binaries are diagnostics and must never be flashed.
A smaller result would still need differential UI tests before adoption.
"""
from pathlib import Path
import concurrent.futures
import hashlib
import json
import os
import re
import shutil
import subprocess
root=Path(__file__).resolve().parents[1];out=root/'build/menu-table-size';out.mkdir(parents=True,exist_ok=True)
source=out/'source'
if source.exists():shutil.rmtree(source)
def ignore(directory,names):
    return [n for n in names if n in {'.git','build','__pycache__'} or n.endswith(('.o','.d','.map','.pyc')) or n.startswith('uvk6-experimental')]
shutil.copytree(root,source,ignore=ignore)
fingerprint=hashlib.sha256()
for p in sorted(source.rglob('*')):
    if p.is_file() and (p.suffix in {'.c','.h','.ld','.inc'} or p.name=='Makefile'):
        fingerprint.update(str(p.relative_to(source)).encode());fingerprint.update(p.read_bytes())
def build(name):
    work=out/name
    if work.exists():shutil.rmtree(work)
    shutil.copytree(source,work)
    p=work/'firmware.ld';p.write_text(p.read_text().replace('LENGTH = 60K','LENGTH = 128K'))
    if name=='shared_enum_table':
        p=work/'ui/menu.c';text=p.read_text();entries=[]
        for menu in ['TXP','SFT_D','W_N','SAVE','TDR','TOT','SC_REV','MDF','PTT_ID','BAT_TXT','PONMSG','ROGER','RESET','BATTYP','AM']:
            pattern=r'case MENU_'+menu+r':\s*strcpy\(String, (\w+)\[gSubMenuSelection\]\);\s*break;'
            matches=list(re.finditer(pattern,text));assert len(matches)==1,(menu,len(matches))
            entries.append(('MENU_'+menu,matches[0][1]));text=re.sub(pattern,'/* Choice handled by shared table. */',text,count=1)
        helper='''typedef struct {uint8_t id,width,count;const char *text;} MenuChoice;
static bool CopyEnumChoice(uint8_t id,char *out)
{
    static const MenuChoice choices[]={
'''
        helper+='\n'.join('        {'+menu+',sizeof('+table+'[0]),ARRAY_SIZE('+table+'),(const char *)'+table+'},' for menu,table in entries)
        helper+='''
    };
    for(unsigned i=0;i<ARRAY_SIZE(choices);i++) if(choices[i].id==id) {
        if((unsigned)gSubMenuSelection<choices[i].count)
            strcpy(out,choices[i].text+gSubMenuSelection*choices[i].width);
        return true;
    }
    return false;
}
'''
        marker='bool    gIsInSubMenu;';assert marker in text;text=text.replace(marker,helper+'\n'+marker,1)
        start=text.index('void UI_DisplayMenu(void)');needle='switch (UI_MENU_GetCurrentMenuId())';position=text.index(needle,start)
        text=text[:position]+'if(!CopyEnumChoice(UI_MENU_GetCurrentMenuId(),String))\n\t'+text[position:]
        p.write_text(text)
    result=subprocess.run(['docker','run','--rm','--user',f'{os.getuid()}:{os.getgid()}',
        '-v',str(work)+':/src','-w','/src','uvk6-build:verified','bash','-c','set -e; make -j2 ENABLE_APRS_TRANSPORT=1'],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    (work/'build.log').write_text(result.stdout)
    report={'profile':name,'exit_code':result.returncode}
    if not result.returncode:report['flash_bytes']=(work/'uvk6-experimental.bin').stat().st_size
    else:report['errors']=[s for s in result.stdout.splitlines() if 'error:' in s or 'undefined reference' in s][:8]
    print(json.dumps(report),flush=True);return report
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:results=list(pool.map(build,['baseline','shared_enum_table']))
(out/'results.json').write_text(json.dumps({'source_fingerprint':fingerprint.hexdigest(),'compiler':'ARM GCC 10.3.1','profile':'ENABLE_APRS_TRANSPORT=1','real_flash_limit':61440,'diagnostic_flash_limit':131072,'behavioral_equivalence':'unverified diagnostic; no UI change adopted','profiles':results},indent=2)+'\n')
