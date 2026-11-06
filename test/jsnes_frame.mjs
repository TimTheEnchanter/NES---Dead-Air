import fs from 'fs';
const {NES}=await import(process.env.HOME+'/tools/jsnes/src/nes.js').then(m=>({NES:m.default||m.NES}));
let fb=null;
const nes=new NES({onFrame(b){fb=b},onAudioSample(){}});
nes.loadROM(fs.readFileSync(process.argv[2]).toString('binary'));
const n=+process.argv[3];
for(let i=0;i<n;i++) nes.frame();
const out=Buffer.alloc(256*240);
for(let i=0;i<256*240;i++){const p=fb[i]&0xffffff;out[i]=p>0x202020?255:0}
fs.writeFileSync('/tmp/js.raw',out);
