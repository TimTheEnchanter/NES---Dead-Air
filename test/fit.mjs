import fs from 'fs';
const {NES}=await import(process.env.HOME+'/tools/jsnes/src/nes.js').then(m=>({NES:m.default||m.NES}));
const L={};for(const l of fs.readFileSync(process.argv[3],'utf8').split('\n')){const m=l.split(' ');if(m[2])L[m[2].slice(1)]=parseInt(m[1],16)}
const P=[L.takeJob,L.ntRun,L.patRun,L.nextState];
const nes=new NES({onFrame(){},onAudioSample(){}});
nes.loadROM(fs.readFileSync(process.argv[2]).toString('binary'));
let cyc=0,t0=null,v=[0,0,0,0];const rows=[];
const orig=nes.cpu.emulate.bind(nes.cpu);
nes.cpu.emulate=function(){const pc=this.REG_PC+1;if(pc===L.nmi){t0=cyc;v=[0,0,0,0]}if(t0!==null){const i=P.indexOf(pc);if(i>=0)v[i]++}const c=orig();cyc+=c;return c};
const w=nes.mmap.write.bind(nes.mmap);
nes.mmap.write=function(a,val){if(a===0x2001&&val&&t0!==null){rows.push([cyc-t0,...v]);t0=null}return w(a,val)};
const N=+process.argv[4]||1500;
const sc=[[40,60,[1,4]],[60,N,[7]]];
for(let i=0;i<N;i++){for(const[a,b,ks]of sc){if(i===a)ks.forEach(k=>nes.buttonDown(1,k));if(i===b)ks.forEach(k=>nes.buttonUp(1,k))}nes.frame()}
fs.writeFileSync('/tmp/fit.json',JSON.stringify(rows));
