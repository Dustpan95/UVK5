#!/usr/bin/env python3
"""Generate provisional 128x64 UI designs using the existing radio fonts.

Design artifacts only: this does not change firmware behavior. Requires Pillow
for PNG/contact-sheet export and host GCC to read the original font tables.
"""
from pathlib import Path
import ctypes
import io
import base64
import json
import subprocess
import textwrap
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'docs/design'
IMAGES = OUT / 'mockups'
BUILD = ROOT / 'build/design'
for p in (IMAGES, BUILD): p.mkdir(parents=True, exist_ok=True)
subprocess.run(['gcc', '-shared', '-fPIC', '-I', str(ROOT), str(ROOT/'font.c'), '-o', str(BUILD/'fonts.so')], check=True)
lib = ctypes.CDLL(str(BUILD/'fonts.so'))
small = ((ctypes.c_uint8*6)*94).in_dll(lib,'gFontSmall')
mini = ((ctypes.c_uint8*3)*96).in_dll(lib,'gFont3x5')
big = ((ctypes.c_uint8*14)*94).in_dll(lib,'gFontBig')
digits = ((ctypes.c_uint8*20)*11).in_dll(lib,'gFontBigDigits')
INK = '#17221c'; LCD = '#cbd2ba'
frames = {}
class Screen:
    def __init__(self): self.im = Image.new('1',(128,64),0)
    def pixel(self,x,y,on=1):
        assert 0<=x<128 and 0<=y<64, (x,y)
        self.im.putpixel((x,y),on)
    def rect(self,x,y,w,h,on=1):
        for yy in range(y,y+h):
            for xx in range(x,x+w): self.pixel(xx,yy,on)
    def text(self,s,x,y,font='small',inverse=False):
        advance={'mini':4,'small':7,'big':8}[font]
        width=len(s)*advance-(1 if s else 0)
        assert x>=0 and x+width<=128,(s,x,width)
        assert y>=0 and y+({'mini':5,'small':7,'big':16}[font])<=64,(s,y)
        for j,c in enumerate(s):
            if c==' ': continue
            if font=='mini': cols=mini[ord(c)-32];half=3
            elif font=='small': cols=small[ord(c)-33];half=6
            else: cols=big[ord(c)-33];half=7
            for i,v in enumerate(cols):
                page=i//half;col=i%half
                for bit in range(8):
                    if (v>>bit)&1: self.pixel(x+j*advance+col,y+page*8+bit,0 if inverse else 1)
    def centered(self,s,y,font='small',inverse=False):
        width=len(s)*{'mini':4,'small':7,'big':8}[font]-1
        self.text(s,(128-width)//2,y,font,inverse)
    def freq(self,s,x,y):
        width=sum(3 if c=='.' else 13 for c in s)
        assert x+width<=128 and y+16<=64,(s,x,y,width)
        for c in s:
            if c=='.': self.rect(x,y+13,3,2);x+=3;continue
            for i,v in enumerate(digits[ord(c)-48]):
                for bit in range(8):
                    if (v>>bit)&1:self.pixel(x+2+i%10,y+(i//10)*8+bit)
            x+=13
    def line(self,y): self.rect(0,y,128,1)
    def status(self,mode='RX',aprs=True,unread=2):
        self.rect(0,0,128,8)
        self.text(mode,2,1,'mini',True)
        self.text('FM H',19,1,'mini',True)
        if aprs:self.text('APRS',47,1,'mini',True)
        if unread:self.text('M'+str(unread),75,1,'mini',True)
        battery='76%'
        self.text(battery,126-(len(battery)*7-1),0,'small',True)
    def footer(self,s): self.rect(0,57,128,7);self.centered(s,58,'mini',True)
    def save(self,key):
        rgb=Image.new('RGB',(128,64),LCD)
        for y in range(64):
            for x in range(128):
                if self.im.getpixel((x,y)):rgb.putpixel((x,y),tuple(bytes.fromhex(INK[1:])))
        path=IMAGES/(key+'.png');rgb.resize((640,320),Image.Resampling.NEAREST).save(path)
        buf=io.BytesIO();rgb.save(buf,format='PNG')
        frames[key]='data:image/png;base64,'+base64.b64encode(buf.getvalue()).decode()

def home(layout='focus',active='A',tx=False,notification=False,tune=0):
    s=Screen();s.status('TX' if tx else 'RX')
    main='146.520' if active=='A' else '144.390'
    if tune:main='146.532' if tune>0 else '146.507'
    suffix='50' if tune else '00'
    if layout.startswith('hybrid-'):
        memory=layout!='hybrid-vfo'
        named=layout=='hybrid-memory'
        number=layout=='hybrid-unnamed-number'
        if memory:
            s.text(active+'  CH012',2,9,'mini')
        else:
            s.text('VFO '+active,2,10,'mini')
        if named:
            s.centered('LOCAL RPT' if active=='A' else 'APRS MON',16,'big')
            s.centered('146.94000' if active=='A' else '144.39000',34)
            s.centered('SHIFT -0.600  H W',42,'mini')
        elif number:
            s.centered('CH-012',16,'big')
            s.centered(main+suffix,34)
            s.centered('SQL3 W 12.5kHz',42,'mini')
        else:
            s.freq(main,23,20);s.text(suffix,109,28)
            s.centered('SQL3 W 12.5kHz',39,'mini')
        s.text('B 144.390 APRS' if active=='A' else 'A 146.520 VOICE',4,49)
    elif layout=='focus':
        s.centered('LOCAL SIMPLEX' if active=='A' else 'APRS MONITOR',10)
        s.freq(main,23,21)
        s.text(suffix,109,29)
        s.text(active,4,23)
        s.centered('SQL3 W 12.5kHz',39,'mini')
        s.text('B 144.390 APRS' if active=='A' else 'A 146.520 VOICE',4,48)
    elif layout=='dual':
        s.text('A LOCAL SIMPLEX  H W',2,10,'mini');s.freq('146.520' if not tune else main,24,17);s.text(suffix,110,25)
        s.line(34)
        s.text('B APRS MONITOR   L W',2,37,'mini');s.freq('144.390',24,43);s.text('00',110,51)
        s.rect(1,17 if active=='A' else 44,5,12)
    else:
        s.centered('LOCAL RPT',11,'big')
        s.centered('146.94000',31)
        s.centered('SHIFT -0.600  H W',42,'mini')
        s.text('B 144.390 APRS',4,49)
    if tx:
        start=42 if layout in ('hybrid-memory','hybrid-unnamed-number') else 34 if layout=='dual' else 38
        s.rect(0,start,128,57-start,0);s.centered('TX  00:04  HIGH',42);s.centered('TIMEOUT 01:56',51,'mini')
    elif notification:
        start=47 if layout.startswith('hybrid-') else {'focus':44,'dual':34,'channel':47}[layout]
        s.rect(0,start,128,64-start,0);s.line(start)
        s.text('MSG N0CALL-7',2,start+2,'mini');s.text('MEET AT TRAILHEAD',2,start+10)
    elif layout!='dual':s.footer('MENU   F+5:APRS   EXIT:A/B')
    return s
for layout in ('focus','dual','channel','hybrid-memory','hybrid-vfo','hybrid-unnamed-frequency','hybrid-unnamed-number'):
    home(layout).save('home-'+layout)
    home(layout,active='B').save('home-'+layout+'-b')
    home(layout,tx=True).save('home-'+layout+'-tx')
    home(layout,notification=True).save('home-'+layout+'-notice')
    home(layout,tune=1).save('home-'+layout+'-up')
    home(layout,tune=-1).save('home-'+layout+'-down')

def menu(aprs=False,selected=0):
    s=Screen();s.status()
    s.centered('APRS' if aprs else 'MENU',10,'mini')
    items=['STATIONS','MESSAGES','BEACON','APRS SETTINGS'] if aprs else ['RADIO','SCAN','APRS','SETTINGS']
    for i,item in enumerate(items):
        y=19+i*9
        if i==selected:s.rect(0,y-1,128,9)
        s.text(str(i+1)+' '+item,4,y,'small',i==selected)
    s.footer('UP/DN:SELECT  MENU:OPEN')
    return s
for i in range(4):
    menu(False,i).save('menu-'+str(i));menu(True,i).save('aprs-menu-'+str(i))

def stations(selected=0):
    s=Screen();s.status();s.centered('STATIONS  3 HEARD',10,'mini')
    for i,(call,age) in enumerate([('N0CALL-7','00:12'),('N0CALL-9','02:08'),('N0CALL-1','06:41')]):
        y=22+i*10
        if selected==i:s.rect(0,y-1,128,9)
        s.text(call,3,y,'small',selected==i);s.text(age,91,y,'mini',selected==i)
    s.footer('UP/DN:SELECT  MENU:OPEN');return s
for i in range(3):stations(i).save('stations-'+str(i))
s=Screen();s.status();s.centered('N0CALL-7',11,'big');s.centered('49.05833N',31);s.centered('072.02916W',41);s.footer('HEARD 00:12   EXIT:LIST');s.save('station-detail')

def inbox(selected=0):
    s=Screen();s.status();s.centered('MESSAGES  2 NEW',10,'mini')
    for i,call in enumerate(['*N0CALL-7','*N0CALL-9',' N0CALL-1']):
        y=22+i*10
        if selected==i:s.rect(0,y-1,128,9)
        s.text(call,3,y,'small',selected==i)
    s.footer('MENU:READ   EXIT:APRS');return s
for i in range(3):inbox(i).save('messages-'+str(i))
s=Screen();s.status();s.text('FROM N0CALL-7',2,10);s.text('TO MYCALL  /  HEARD 00:12',2,19,'mini')
for i,line in enumerate(textwrap.wrap('MEET AT TRAILHEAD AT 14:30. CAN YOU COPY?',17)):
    s.text(line,2,27+i*9)
s.footer('MENU:REPLY   EXIT:INBOX');s.save('message-detail')
s=Screen();s.status();s.centered('BEACON',12,'big');s.centered('CALLSIGN NOT SET',33);s.centered('SET CALLSIGN + POSITION',46,'mini');s.footer('MENU:SETUP   EXIT:APRS');s.save('beacon-setup')
s=Screen();s.status();s.centered('SCAN  03 / 24',11);s.freq('146.940',24,24);s.centered('SIGNAL FOUND - HOLD',43,'mini');s.footer('MENU:HOLD  EXIT:STOP');s.save('scan')
s=Screen();s.status();s.centered('APRS SETTINGS',10,'mini')
for i,line in enumerate(['CALL MYCALL','SSID 7','POS NOT SET','ALERT BANNER']):s.text(line,3,20+i*9)
s.footer('MENU:EDIT   EXIT:APRS');s.save('aprs-settings')

# Proposed messaging workflow: keypad multi-tap plus saved quick messages.
s=Screen();s.status();s.centered('NEW MESSAGE',10,'mini')
s.text('TO N0CALL-7_',2,23);s.text('ABC  2:A/B/C',2,37,'mini')
s.text('*:DELETE  #:NEXT',2,47,'mini');s.footer('MENU:BODY   EXIT:BACK');s.save('message-recipient')
s=Screen();s.status();s.text('TO N0CALL-7',2,10,'mini');s.text('22/67  ABC',85,10,'mini')
s.text('MEET AT TRAILHEAD',2,23);s.text('AT 14_',2,33)
s.text('2:ABC  #:NEXT  *:DELETE',2,47,'mini');s.footer('MENU:OPTIONS  EXIT:BACK');s.save('message-compose')
s=Screen();s.status();s.centered('QUICK MESSAGES',10,'mini')
for i,line in enumerate(['ARRIVED SAFELY','NEED ASSISTANCE','CAN YOU COPY?']):
    if i==0:s.rect(0,21,128,9)
    s.text(line,2,22+i*10,'small',i==0)
s.footer('MENU:INSERT  EXIT:BODY');s.save('message-quick')
s=Screen();s.status();s.centered('REVIEW MESSAGE',10,'mini');s.text('TO N0CALL-7',2,20)
s.text('MEET AT TRAILHEAD',2,32);s.text('AT 14:30',2,41)
s.footer('MENU:SEND   EXIT:EDIT');s.save('message-review')
s=Screen();s.status();s.centered('TO N0CALL-7',12);s.centered('WAITING FOR ACK',29)
s.centered('ATTEMPT 1    00:12',44,'mini');s.footer('MENU:DETAIL  EXIT:INBOX');s.save('message-pending')
s=Screen();s.status();s.centered('TO N0CALL-7',12);s.centered('ACK RECEIVED',29)
s.centered('MESSAGE 01',44,'mini');s.footer('MENU:DETAIL  EXIT:INBOX');s.save('message-acked')
s=Screen();s.status();s.centered('STATIC BEACON',10,'mini');s.text('CALL MYCALL-7',2,20)
s.text('49.05833N',2,31);s.text('072.02916W',2,41)
s.footer('MENU:REVIEW  EXIT:APRS');s.save('beacon-static')

# Contact sheets show alternatives without assuming any option was selected.
try:
 titlefont=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',22)
 smallfont=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',16)
except OSError:titlefont=smallfont=ImageFont.load_default()
def sheet(name,cards,cols=3):
    w,h,pad=560,348,24
    rows=(len(cards)+cols-1)//cols
    image=Image.new('RGB',(cols*w+pad,(h+pad)*rows+pad),'#f5f6f3');d=ImageDraw.Draw(image)
    for i,(key,title,subtitle) in enumerate(cards):
        x=pad+(i%cols)*w;y=pad+(i//cols)*(h+pad)
        d.text((x,y),title,font=titlefont,fill='#1c2d25')
        d.text((x,y+30),subtitle,font=smallfont,fill='#526259')
        lcd=Image.open(IMAGES/(key+'.png')).resize((512,256),Image.Resampling.NEAREST)
        image.paste(lcd,(x,y+65))
    image.save(OUT/(name+'.png'))
sheet('home-alternatives',[
 ('home-focus','A. Active VFO focus','Large active frequency + secondary VFO strip'),
 ('home-dual','B. Equal dual VFOs','Both frequencies remain prominent'),
 ('home-channel','C. Channel-name focus','Prioritize saved channel names and repeater info')])
sheet('home-comparison',[
 ('home-hybrid-memory','Memory mode - named channel','Prominent channel name + full frequency below'),
 ('home-hybrid-vfo','VFO mode','Large full-precision frequency + secondary VFO'),
 ('home-hybrid-memory-notice','Brief APRS banner','Keep the main readout; temporarily cover the strip')])
sheet('home-review',[
 ('home-hybrid-memory','Named memory channel','Large channel name, with full frequency below'),
 ('home-hybrid-vfo','VFO mode','Large frequency and smaller secondary VFO'),
 ('home-hybrid-unnamed-frequency','Unnamed memory channel','Large frequency, with smaller channel number'),
 ('home-hybrid-memory-notice','Selected brief APRS banner','Preserve the main display; cover the secondary strip')],cols=2)
sheet('unnamed-memory-options',[
 ('home-hybrid-unnamed-frequency','Selected: large frequency','Unnamed memory; smaller channel number'),
 ('home-hybrid-unnamed-number','Earlier alternative: channel number','Not the selected fallback')],cols=2)
sheet('aprs-comparison',[
 ('home-hybrid-memory-notice','Brief notification','Keep the radio screen; open traffic when ready'),
 ('message-detail','Open incoming message','Show the message immediately; EXIT returns'),
 ('stations-0','Station directory','One row per station; sort by last heard'),
 ('station-detail','Station details','Callsign, received coordinates and age'),
 ('messages-0','Message inbox','Unread markers and sender list'),
 ('beacon-setup','Transmit setup proposal','Require callsign and position before beaconing')])
sheet('messaging-review',[
 ('message-recipient','1. Recipient','Proposed multi-tap callsign/SSID entry'),
 ('message-compose','2. Compose','Multi-tap text; 67-character counter'),
 ('message-quick','3. Saved quick messages','Insert text, then edit before sending'),
 ('message-review','4. Review and send','Recipient and text visible before transmission'),
 ('message-pending','5. Waiting for acknowledgment','Proposed delivery state; retry policy still open'),
 ('message-acked','6. Acknowledged','A remote ACK confirms receipt by that station')],cols=3)
(OUT/'frames.json').write_text(json.dumps(frames,separators=(',',':')))
if (OUT/'review.template.html').exists():
    (OUT/'review.html').write_text((OUT/'review.template.html').read_text().replace('__FRAMES__',json.dumps(frames,separators=(',',':'))))
print(f'Generated {len(frames)} exact-resolution mockup states and comparison sheets.')
