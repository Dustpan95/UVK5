"""Deterministic 4096-byte-window, 3..18-byte-match LZSS for trusted resources."""
def pack(data):
 result=bytearray();i=0
 while i<len(data):
  flagpos=len(result);result.append(0)
  for bit in range(8):
   if i==len(data):break
   best=0;distance=0
   for j in range(max(0,i-4096),i):
    n=0
    while n<18 and i+n<len(data) and data[j+n]==data[i+n]:n+=1
    if n>best:best=n;distance=i-j
   if best>=3:
    result+=bytes([(distance-1)&255,((distance-1)>>8)<<4|(best-3)]);i+=best
   else:result[flagpos]|=1<<bit;result.append(data[i]);i+=1
 return result
