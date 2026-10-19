#!/usr/bin/env python3
"""Decode the actual C deadline scheduler's simulated-jitter tone trace.

Requires atest (Dire Wolf). No actual MCU/SPI/RF behavior is simulated or proven.
"""
from pathlib import Path
import math
import struct
import subprocess
import wave
root=Path(__file__).resolve().parents[1];out=root/'build/bell202-timing';out.mkdir(parents=True,exist_ok=True)
subprocess.run(['gcc','-std=c11','-Wall','-Wextra','-Werror','-I',str(root),
               str(root/'tests/bell202_dump.c'),str(root/'experiments/bell202_symbols.c'),
               str(root/'experiments/bell202_timing.c'),str(root/'app/aprs_message.c'),'-o',str(out/'trace')],check=True)
for label,body,expected in [('message',b'MEET AT TRAILHEAD',':MYCALL   :MEET AT TRAILHEAD{01'),
                           ('beacon',b'','!4903.50N/07201.75W-'),
                           ('ack',b'',':MYCALL   :ackaZ123'),('reject',b'',':MYCALL   :rejaZ123')]:
    result=subprocess.run([str(out/'trace'),'--timed','--'+label],input=body,stdout=subprocess.PIPE,check=True)
    (out/(label+'.csv')).write_bytes(result.stdout)
    events=[tuple(map(int,row.split(','))) for row in result.stdout.decode().splitlines()]
    assert events[-1][1]==0 and all(1200<=f<=2200 for _,f in events[:-1])
    samples=[0]*4800;phase=0.;index=0;origin=events[0][0]
    # Resample the irregular transition trace to an ordinary 48 kHz recording.
    for cycle in range(origin,events[-1][0],1000):
        while index+1<len(events)-1 and cycle>=events[index+1][0]:index+=1
        frequency=events[index][1]
        samples.append(round(12000*math.sin(phase)))
        phase=(phase+2*math.pi*frequency/48000)%(2*math.pi)
    samples.extend([0]*4800)
    path=out/(label+'.wav')
    with wave.open(str(path),'wb') as wav:
        wav.setnchannels(1);wav.setsampwidth(2);wav.setframerate(48000)
        wav.writeframes(struct.pack('<'+'h'*len(samples),*samples))
    decoded=subprocess.run(['atest','-F','0',str(path)],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    (out/(label+'.log')).write_text(decoded.stdout)
    assert decoded.returncode==0 and 'N0CALL-15>APZUVK:'+expected in decoded.stdout,decoded.stdout
    print('PASS independent Dire Wolf decode of C scheduler with synthetic SPI/IRQ jitter:',label)
print('Synthetic jitter budget decoded; physical tone timing, phase and RF remain untested.')
