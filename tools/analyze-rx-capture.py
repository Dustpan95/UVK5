#!/usr/bin/env python3
"""Offline diagnostic hypotheses; never changes firmware/RF or accepts bad FCS."""
import argparse,itertools,json
from pathlib import Path

def fcs(data):
    crc=0xffff
    for byte in data:
        crc^=byte
        for _ in range(8):crc=(crc>>1)^0x8408 if crc&1 else crc>>1
    return crc^0xffff

def frame(segment):
    bits=[];ones=0
    for bit in segment:
        if bit:
            ones+=1
            if ones>=6:return None
            bits.append(1)
        else:
            if ones!=5:bits.append(0)
            ones=0
    if len(bits)%8 or not 19<=len(bits)//8<=192:return None
    data=bytes(sum(v<<i for i,v in enumerate(bits[n:n+8])) for n in range(0,len(bits),8))
    return data

def describe(data):
    addresses=[];offset=0
    while offset+7<=len(data)-2 and len(addresses)<10:
        a=data[offset:offset+7];call=''.join(chr(v>>1) for v in a[:6]).rstrip()
        if any(v&1 for v in a[:6]) or not call or any(not(c.isascii() and (c.isupper() or c.isdigit())) for c in call):return None
        ssid=(a[6]>>1)&15;addresses.append(call+(f'-{ssid}' if ssid else ''));offset+=7
        if a[6]&1:break
    if len(addresses)<2 or data[offset:offset+2]!=b'\x03\xf0':return None
    return {'destination':addresses[0],'source':addresses[1],'path':addresses[2:],
            'information':data[offset+2:-2].decode('ascii',errors='replace')}

def analyze(raw,swap=False,msb=True,coding='nrzi'):
    if swap:raw=b''.join(raw[i:i+2][::-1] for i in range(0,len(raw),2))
    levels=[(v>>i)&1 for v in raw for i in (range(7,-1,-1) if msb else range(8))]
    bits=[];prev=0
    for level in levels:
        bits.append(int(level==prev) if coding=='nrzi' else level if coding=='direct' else level^1);prev=level
    flag=[0,1,1,1,1,1,1,0]
    positions=[i for i in range(len(bits)-7) if bits[i:i+8]==flag]
    candidates=[]
    for a,b in zip(positions,positions[1:]):
        if b>=a+8:candidates.append((a+8,b,False))
    # The modem sync detector consumes bits before its FIFO. Test the explicit
    # post-sync hypothesis at 16 possible FIFO phases, requiring a valid FCS.
    if positions:
        for start in range(min(16,positions[0])):candidates.append((start,positions[0],True))
    aligned=0;valid=[];seen=set()
    for a,b,primed in candidates:
        data=frame(bits[a:b])
        if data is None:continue
        aligned+=1
        if fcs(data[:-2])!=int.from_bytes(data[-2:],'little') or data in seen:continue
        seen.add(data);valid.append({'offset_bits':a,'post_sync_hypothesis':primed,'frame_hex':data.hex(),'aprs':describe(data)})
    return {'swap_word_bytes':swap,'bit_order':'msb' if msb else 'lsb','coding':coding,
            'flags':len(positions),'aligned_candidates':aligned,'valid_fcs_frames':valid}

def report(document):
    if document.get('format')!='UVK6_RX_CAPTURE_V1':raise ValueError('Unknown capture file format')
    results=[]
    for capture in document['captures']:
        raw=bytes.fromhex(capture['raw_hex'])
        if len(raw)!=capture['length'] or len(raw)>240:raise ValueError('Invalid capture length')
        modes=[analyze(raw,swap,msb,coding) for swap,msb,coding in itertools.product([False,True],[True,False],['nrzi','direct','inverted'])]
        results.append({'sequence':capture['sequence'],'length':len(raw),'modes':modes})
    return {'purpose':'Offline bit-order/coding/post-sync hypotheses; valid FCS required, not hardware proof','captures':results}

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('capture',type=Path);args=p.parse_args()
    print(json.dumps(report(json.loads(args.capture.read_text())),indent=2))
