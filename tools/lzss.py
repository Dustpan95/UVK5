"""Deterministic 4096-byte-window, 3..18-byte-match LZSS for trusted resources.

Choose the smallest complete byte stream, including each eight-token flag byte.
The firmware format and decoder are unchanged. Only the build-time parser varies.
"""
def pack(data):
 n=len(data)
 lengths=[0]*n;distances=[0]*n;history={}
 for i in range(n):
  if i+3>n:continue
  prefix=bytes(data[i:i+3])
  limit=min(18,n-i)
  for j in reversed(history.get(prefix,())):
   if i-j>4096:break
   count=3
   while count<limit and data[j+count]==data[i+count]:count+=1
   if count>lengths[i]:lengths[i]=count;distances[i]=i-j
   if count==limit:break
  history.setdefault(prefix,[]).append(i)
 # Phase zero pays for the next flag byte; a partial final group needs no pad.
 costs=[[0]*8 for _ in range(n+1)]
 choices=[[1]*8 for _ in range(n)]
 for i in range(n-1,-1,-1):
  for phase in range(8):
   next_phase=(phase+1)%8
   cost=1+costs[i+1][next_phase];length=1
   for count in range(3,lengths[i]+1):
    candidate=2+costs[i+count][next_phase]
    if candidate<=cost:cost=candidate;length=count
   costs[i][phase]=cost+(phase==0);choices[i][phase]=length
 result=bytearray();i=0;phase=0
 while i<n:
  if phase==0:flagpos=len(result);result.append(0)
  count=choices[i][phase]
  if count==1:result[flagpos]|=1<<phase;result.append(data[i])
  else:
   distance=distances[i]-1
   result+=bytes([distance&255,(distance>>8)<<4|(count-3)])
  i+=count;phase=(phase+1)%8
 assert len(result)==costs[0][0]
 return result
