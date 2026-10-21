// Test the exact self-contained HTML protocol against captured firmware fixtures.
const fs=require('fs'),vm=require('vm'),assert=require('assert');
const html=fs.readFileSync('tools/read-rx-capture.html','utf8');
const script=html.split('/* protocol begin:')[1].split('*/').slice(1).join('*/').split('/* protocol end */')[0];
const context=vm.createContext({Uint8Array,Date});vm.runInContext(script+'\nglobalThis.protocol=CaptureProtocol;',context);const p=context.protocol;
assert.strictEqual(p.crc16(new TextEncoder().encode('123456789')),0x31c3);
const dump=fs.readFileSync('build/rx-diagnostics-tests/captures.bin');
const decoded=p.decodeDump(dump);assert.strictEqual(decoded.captures.length,4);assert.strictEqual(decoded.last_sequence,8);
assert.deepStrictEqual(Array.from(decoded.captures,c=>c.sequence),[5,6,7,8]);assert.deepStrictEqual(Array.from(decoded.captures,c=>c.length),[240,240,240,24]);
function response(offset,n,encrypted){const part=dump.slice(offset,offset+n);const b=Uint8Array.from([0x41,5,part.length+4,0,offset&255,offset>>8,part.length,1,...part,255,255]);return Uint8Array.from([0xab,0xcd,part.length+8,0,...(encrypted?p.xor(b):b),0xdc,0xba]);}
let cases=0;
for(const encrypted of [false,true])for(let offset=0;offset<dump.length;offset+=64){
 const n=Math.min(64,dump.length-offset),req=p.request(offset,n,encrypted);
 assert.strictEqual(req.length,16);assert.strictEqual(req[0],0xab);const body=encrypted?p.xor(req.slice(4,14)):req.slice(4,14);
 assert.deepStrictEqual(Array.from(body.slice(0,4)),[0x40,5,4,0]);assert.strictEqual(body[4]|body[5]<<8,offset);assert.strictEqual(body[6],n);assert.strictEqual(p.crc16(body.slice(0,8)),body[8]|body[9]<<8);
 const wire=response(offset,n,encrypted);
 for(let cut=0;cut<wire.length;cut++){
  let b=Uint8Array.from([0x55,0x44,...wire.slice(0,cut)]);const early=p.takeFrame(b);assert.strictEqual(early.frame,null);
  const full=p.takeFrame(Uint8Array.from([...early.rest,...wire.slice(cut)]));assert(full.frame);assert.strictEqual(full.rest.length,0);
  assert.deepStrictEqual(Buffer.from(p.reply(full.frame,offset,encrypted)),dump.slice(offset,offset+n));cases++;
 }
 const bad=wire.slice();bad[bad.length-1]=0;assert.strictEqual(p.takeFrame(bad).frame,null);
 assert.throws(()=>p.reply(wire,offset+1,encrypted));
}
for(const offset of [-1,65536,NaN,0.5])assert.throws(()=>p.request(offset,1));
assert.throws(()=>p.request(0,65));assert.throws(()=>p.decodeDump(dump.slice(1)));
for(const at of [0,4,5,6,7,12,14,16+4,16+6]){const bad=Buffer.from(dump);bad[at]=255;assert.throws(()=>p.decodeDump(bad));}
console.log('PASS capture reader:',cases,'fragmented XOR/plain replies, CRC requests, ring records and corrupt/truncated/range rejection cases');

assert.strictEqual(decoded.wire_version,3);
assert(decoded.captures.every(c=>c.modem_mode==='A'&&c.receiver_profile==='A'&&c.reg58_configured===0x3fc3&&c.registers['58']===0x3fc3));
for(let mode=0;mode<4;mode++){const b=Buffer.from(dump);b[23]=mode;const c=p.decodeDump(b).captures[0];assert.strictEqual(c.receiver_profile,'ABCD'[mode]);assert.strictEqual(c.reg58_configured,0x3fc3);}
for(const version of [1,2]){
 const legacy=Buffer.alloc(1008);dump.copy(legacy,0,0,16);legacy[4]=version;legacy.writeUInt16LE(248,14);
 for(let i=0;i<4;i++){dump.copy(legacy,16+i*248,16+i*272,24+i*272);dump.copy(legacy,24+i*248,48+i*272,288+i*272);}
 const old=p.decodeDump(legacy);assert.strictEqual(old.wire_version,version);assert.strictEqual(old.captures[0].raw_hex,decoded.captures[0].raw_hex);
 for(let mode=0;mode<4;mode++){legacy[23]=mode;if(version===1&&mode)assert.throws(()=>p.decodeDump(legacy));else {const c=p.decodeDump(legacy).captures[0];assert.strictEqual(c.modem_mode,'ABCD'[mode]);assert.strictEqual(c.reg58_configured,[0x3fc3,0x3fc9,0x33c9,0x33c3][mode]);}}
}
assert.strictEqual(p.dumpSize(dump.subarray(0,64)),1104);
console.log('PASS D4 register snapshots/profile labels and D2/D3 backward compatibility');
