// Execute the pinned updater's actual unpacker and flash guard/packet loop.
// Serial calls are mocks: no browser, COM port or radio is accessed.
const fs=require('fs'),vm=require('vm'),path=require('path'),assert=require('assert');
const {execFileSync}=require('child_process');
const [upstream,packedPath,rawPath,reportPath]=process.argv.slice(2);
const revision='7148b01f11357cb28e74fff3a9591b12a656ea13';
assert.strictEqual(execFileSync('git',['-C',upstream,'rev-parse','HEAD'],{encoding:'utf8'}).trim(),revision);
const packing=fs.readFileSync(path.join(upstream,'js/fwpack.js'),'utf8');
const flashing=fs.readFileSync(path.join(upstream,'js/tool_patcher.js'),'utf8');
function extract(name) {
 const start=flashing.indexOf('function '+name+'(');assert(start>=0);
 const brace=flashing.indexOf('{',start);let depth=1,end=brace+1;
 while(depth) {if(flashing[end]==='{')depth++;if(flashing[end]==='}')depth--;end++;assert(end<=flashing.length);}
 return (name==='flash_flashFirmware'?'async ':'')+flashing.slice(start,end);
}
const commands=[],logs=[];
const context=vm.createContext({Uint8Array,rawVersion:null,log:(x)=>logs.push(x),
 sendPacket:async(port,command)=>commands.push(command),readPacket:async()=>new Uint8Array(0x1a)});
vm.runInContext(packing+'\n'+extract('flash_generateCommand')+'\n'+extract('flash_flashFirmware'),context);
const packed=new Uint8Array(fs.readFileSync(packedPath)),raw=fs.readFileSync(rawPath);
const unpacked=context.unpack(packed);
assert.deepStrictEqual(Buffer.from(unpacked),raw);assert(logs.includes('CRC check passed...'));
assert.deepStrictEqual(Buffer.from(context.pack(unpacked)),Buffer.from(packed));
(async()=>{
 for(const size of [61440,61441]) {
  commands.length=0;
  await assert.rejects(context.flash_flashFirmware(null,new Uint8Array(size)),/boundary check failed/);
  assert.strictEqual(commands.length,0);
 }
 commands.length=0;
 await context.flash_flashFirmware(null,new Uint8Array(61439));
 assert.strictEqual(commands.length,240);
 commands.length=0;
 await context.flash_flashFirmware(null,unpacked);
 assert.strictEqual(commands.length,Math.ceil(raw.length/256));
 const payload=[];
 for(let i=0;i<commands.length;i++) {
  const c=commands[i];assert.strictEqual(c.length,272);
  assert.strictEqual((c[8]<<8)|c[9],i*256);
  assert.strictEqual((c[10]<<8)|c[11],Math.ceil(raw.length/256)*256);
  payload.push(...c.slice(16));
 }
 assert.deepStrictEqual(Buffer.from(payload.slice(0,raw.length)),raw);
 assert(payload.slice(raw.length).every(x=>x===0));
 const report={upstream:'egzumer/uvtools',revision,updater_limit_bytes:61439,
  firmware_bytes:raw.length,packed_bytes:packed.length,flash_packets:commands.length,
  checks:['actual unpacker CRC and exact payload','exact repack','61440/61441 rejected before serial calls',
          '61439 accepted','candidate accepted','all packet addresses, payload and zero padding'],
  status:'passed',limitations:'JavaScript with mocked serial only; no browser UI, USB, physical radio or RF test'};
 if(reportPath)fs.writeFileSync(reportPath,JSON.stringify(report,null,2)+'\n');
 console.log('PASS actual pinned Egzumer unpacker and flash loop:',raw.length,'/ 61439 bytes;',commands.length,'mocked packets');
})().catch(error=>{console.error(error);process.exitCode=1;});
