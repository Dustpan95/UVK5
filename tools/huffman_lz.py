"""Deterministic Huffman/LZ packing for immutable firmware resources.

Mode 1: ASCII literals, match lengths 3..18, 1024-byte window.
Mode 2: paired nibble literals, match lengths 2..17, 256-byte window.
Each stream has mode, 15 canonical code counts, ordered symbols, then bits.
Huffman codes are MSB first; distance fields are LSB first. Eight deterministic
weighted parses retain the smallest complete stream, including its tables.
"""
import collections
import heapq

def pack_huffman(data, nibbles=False):
 data=bytes(data)
 windowbits=8 if nibbles else 10;minlen=2 if nibbles else 3
 n=len(data);base=16 if nibbles else 128
 alphabet=list(range(16)) if nibbles else sorted(set(data))
 assert nibbles or all(v<128 for v in data)
 alphabet+=list(range(base,base+16));hist=collections.Counter()
 if nibbles:
  for v in data:hist[v>>4]+=1;hist[v&15]+=1
 else:hist.update(data)
 longest=[0]*n;distances=[0]*n;history={}
 for i in range(n-minlen+1):
  prefix=data[i:i+minlen];limit=min(minlen+15,n-i)
  for j in reversed(history.get(prefix,())):
   if i-j>(1<<windowbits):break
   k=minlen
   while k<limit and data[j+k]==data[i+k]:k+=1
   if k>longest[i]:longest[i]=k;distances[i]=i-j
   if k==limit:break
  history.setdefault(prefix,[]).append(i)
 best=None
 for iteration in range(8):
  heap=[(hist[s]+1,s,s) for s in alphabet];heapq.heapify(heap);serial=max(alphabet)+1
  while len(heap)>1:
   a=heapq.heappop(heap);b=heapq.heappop(heap);heapq.heappush(heap,(a[0]+b[0],serial,(a[2],b[2])));serial+=1
  lengths={}
  def walk(node,depth):
   if isinstance(node,int):lengths[node]=depth
   else:walk(node[0],depth+1);walk(node[1],depth+1)
  walk(heap[0][2],0);assert max(lengths.values())<=15
  costs=[0]*(n+1);choice=[1]*n
  for i in range(n-1,-1,-1):
   cost=lengths[data[i]>>4]+lengths[data[i]&15] if nibbles else lengths[data[i]]
   costs[i]=cost+costs[i+1]
   for k in range(minlen,longest[i]+1):
    cost=lengths[base+k-minlen]+windowbits+costs[i+k]
    if cost<=costs[i]:costs[i]=cost;choice[i]=k
  ordered=sorted(alphabet,key=lambda s:(lengths[s],s));codes={};code=0;prev=0
  for s in ordered:code<<=lengths[s]-prev;codes[s]=code;code+=1;prev=lengths[s]
  counts=[sum(lengths[s]==k for s in ordered) for k in range(1,16)]
  out=bytearray([2 if nibbles else 1]+counts+ordered);bits=0;bitcount=0;hist=collections.Counter()
  def write(value,count):
   nonlocal bits,bitcount
   bits|=value<<bitcount;bitcount+=count
   while bitcount>=8:out.append(bits&255);bits>>=8;bitcount-=8
  def symbol(s):
   hist[s]+=1
   for b in range(lengths[s]-1,-1,-1):write((codes[s]>>b)&1,1)
  i=0
  while i<n:
   k=choice[i]
   if k==1:
    if nibbles:symbol(data[i]>>4);symbol(data[i]&15)
    else:symbol(data[i])
   else:symbol(base+k-minlen);write(distances[i]-1,windowbits)
   i+=k
  if bitcount:out.append(bits)
  if best is None or len(out)<len(best):best=bytes(out)
  # Re-encode the fixed parse with its exact histogram. Unused length
  # symbols need no table entries; no decoder or stream-format change.
  used=sorted(hist)
  if not used:used=[0]
  exact=[(hist[s] or 1,s,s) for s in used];heapq.heapify(exact);serial=max(used)+1
  while len(exact)>1:
   a=heapq.heappop(exact);b=heapq.heappop(exact)
   heapq.heappush(exact,(a[0]+b[0],serial,(a[2],b[2])));serial+=1
  exact_lengths={}
  def exact_walk(node,depth):
   if isinstance(node,int):exact_lengths[node]=max(1,depth)
   else:exact_walk(node[0],depth+1);exact_walk(node[1],depth+1)
  exact_walk(exact[0][2],0)
  if max(exact_lengths.values())<=15:
   exact_order=sorted(used,key=lambda s:(exact_lengths[s],s))
   exact_codes={};code=0;prev=0
   for symbol_value in exact_order:
    code<<=exact_lengths[symbol_value]-prev;exact_codes[symbol_value]=code
    code+=1;prev=exact_lengths[symbol_value]
   counts=[sum(exact_lengths[s]==k for s in exact_order) for k in range(1,16)]
   out=bytearray([2 if nibbles else 1]+counts+exact_order);bits=0;bitcount=0
   def exact_symbol(symbol_value):
    for bit in range(exact_lengths[symbol_value]-1,-1,-1):write((exact_codes[symbol_value]>>bit)&1,1)
   i=0
   while i<n:
    k=choice[i]
    if k==1:
     if nibbles:exact_symbol(data[i]>>4);exact_symbol(data[i]&15)
     else:exact_symbol(data[i])
    else:exact_symbol(base+k-minlen);write(distances[i]-1,windowbits)
    i+=k
   if bitcount:out.append(bits)
   if best is None or len(out)<len(best):best=bytes(out)
 return best

