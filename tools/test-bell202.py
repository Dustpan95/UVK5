#!/usr/bin/env python3
"""Generate audio from the actual C encoder for independent decoder testing.

Synthetic waveform only: this does not exercise BK4819 output or MCU timing.
Requires `atest` (Dire Wolf) in PATH. Uses only Python standard libraries.
"""
from pathlib import Path
import math
import struct
import subprocess
import wave
root=Path(__file__).resolve().parents[1]
out=root/'build/bell202';out.mkdir(parents=True,exist_ok=True)
subprocess.run(['gcc','-std=c11','-Wall','-Wextra','-Werror','-I',str(root),
                str(root/'tests/bell202_dump.c'),str(root/'experiments/bell202_symbols.c'),str(root/'app/aprs_message.c'),
                '-o',str(out/'encode')],check=True)
def address(call,ssid,last):
    return bytes(ord(c)<<1 for c in call.ljust(6))+bytes([0x60|ssid<<1|last])
for label,payload,expected in [
 ('position',b'!4903.50N/07201.75W-Test','!4903.50N/07201.75W-Test'),
 ('message',b':MYCALL   :MEET AT TRAILHEAD{01',':MYCALL   :MEET AT TRAILHEAD{01'),
 ('stuffing',b':MYCALL   :'+b'~'*67,':MYCALL   :'+ '~'*67)]:
    body=address('APZUVK',0,False)+address('N0CALL',15,True)+b'\x03\xf0'+payload
    command=[str(out/'encode')]
    if label=='position':
        command.append('--beacon');body=b'';expected='!4903.50N/07201.75W-'
    if label=='message':
        command.append('--message');body=b'MEET AT TRAILHEAD'
        expected=':MYCALL   :MEET AT TRAILHEAD{01'
    symbols=subprocess.run(command,input=body,stdout=subprocess.PIPE,check=True).stdout
    for corrupt in (False,True):
        encoded=bytearray(symbols)
        if corrupt:
            # Alter one emitted tone symbol inside frame data, retaining FCS.
            encoded[60*8+160]^=1
        phase=0.; samples=[0]*4800
        for symbol in encoded:
            frequency=2200 if symbol==ord('1') else 1200
            for _ in range(40):
                samples.append(round(12000*math.sin(phase)))
                phase=(phase+2*math.pi*frequency/48000)%(2*math.pi)
        samples.extend([0]*4800)
        name=label+('-corrupt' if corrupt else '')
        path=out/(name+'.wav')
        with wave.open(str(path),'wb') as wav:
            wav.setnchannels(1);wav.setsampwidth(2);wav.setframerate(48000)
            wav.writeframes(struct.pack('<'+'h'*len(samples),*samples))
        result=subprocess.run(['atest','-F','0',str(path)],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        (out/(name+'.log')).write_text(result.stdout)
        assert result.returncode==0, result.stdout
        if corrupt:
            assert '0 packets decoded' in result.stdout, result.stdout
        else:
            assert 'N0CALL-15>APZUVK:'+expected in result.stdout, result.stdout
        print('PASS independent Dire Wolf audio', 'corruption rejection:' if corrupt else 'decode:',label)
print('Synthetic Bell 202 validated; BK4819 timing and RF remain untested.')
