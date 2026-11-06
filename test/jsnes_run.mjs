import fs from 'fs';
const {NES}=await import(process.env.HOME+'/tools/jsnes/src/nes.js').then(m=>({NES:m.default||m.NES}));
let fb=null;
const nes=new NES({onFrame(b){fb=b},onAudioSample(){}});
nes.loadROM(fs.readFileSync(process.argv[2]).toString('binary'));
const frames=+process.argv[3], script=process.argv[5]?fs.readFileSync(process.argv[5],'utf8').trim().split('\n').map(l=>l.split(' ').map(Number)):[];
const map={0:8?0:0};
const K={8:0,0:1,2:2,3:3,4:4,5:5,6:6,7:7}; // retro id -> jsnes button
const out=fs.openSync(process.argv[4],'w');let held=new Set();
for(let i=1;i<=frames;i++){
  const want=new Set();
  for(const[a,b,m]of script) if(i>=a&&i<b) for(let id=0;id<9;id++) if(m>>id&1) want.add(K[id]);
  for(const k of want) if(!held.has(k)) nes.buttonDown(1,k);
  for(const k of held) if(!want.has(k)) nes.buttonUp(1,k);
  held=want; nes.frame();
  const b=Buffer.alloc(256*240+24);
  for(let p=0;p<256*240;p++){const c=fb[p];b[p]=(c&0xff)>0x20||((c>>8)&0xff)>0x20||((c>>16)&0xff)>0x20?1:0}
  const r=nes.cpu.mem;const o=256*240;
  for(let k=0;k<12;k++)b[o+k]=r[0x320+k];
  b[o+12]=r[0x306];b[o+13]=r[0x307];b[o+14]=r[0x304];b[o+15]=r[0x30A];b[o+16]=r[0x302];b[o+17]=r[0x300];b[o+18]=r[0x301];
  fs.writeSync(out,b);
}
const r=nes.cpu.mem;console.log('frames',r[0x300]|r[0x301]<<8,'nmi',r[0x302]|r[0x303]<<8);
