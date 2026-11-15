// Deterministic speed benchmark for -D BENCH=1 builds (fixed world step):
//   node test/bench.mjs <rom> <labels> <game frames> [held buttons, e.g. 7 or 0,7]
// The world advances the same way per game frame whatever the draw speed, so
// two builds see the same scenes. The state hash checks that; NES frames used
// and main-thread work per update are the speed measures.
import fs from 'fs';
const {NES}=await import(process.env.HOME+'/tools/jsnes/src/nes.js').then(m=>({NES:m.default||m.NES}));
const L={};for(const l of fs.readFileSync(process.argv[3],'utf8').split('\n')){const m=l.split(' ');if(m[2])L[m[2].slice(1)]=parseInt(m[1],16)}
const K=+process.argv[4]||600, btn=(process.argv[5]||'').split(',').filter(Boolean).map(Number);
const START=+process.env.START||20;
const nes=new NES({onFrame(){},onAudioSample(){},emulateSound:false});
nes.loadROM(fs.readFileSync(process.argv[2]).toString('binary'));
const m=nes.cpu.mem;
const W0=L.wait_buf??(L.main+8);
let on=false,depth=0,busy=0,idle=0,intc=0,fb=0;const perBusy=[];
const orig=nes.cpu.emulate.bind(nes.cpu);
nes.cpu.emulate=function(){const pc=(this.REG_PC+1)&0xffff;if(pc===L.nmi+1||pc===L.irq+1)depth++;/* jsnes runs a handler's first instruction (pha) with the interrupt itself */const op=m[pc];const c=orig();
  if(on){if(depth)intc+=c;else if(pc>=W0&&pc<W0+5)idle+=c;else{busy+=c;fb+=c}}
  if(op===0x40&&depth)depth--;return c};
const prof=process.env.PROF?(await import('./prof_hook.mjs')).attach(nes,process.env.PROF,L):null;
let games=0,nesF=0,hash=0,picH=0;const trace=[];
const w0=nes.cpu.write.bind(nes.cpu);
nes.cpu.write=(a,v)=>{const r=w0(a,v);if(a===0x300){const f=m[0x300]|m[0x301]<<8;
  if(f===START){on=true}
  else if(on&&f>START){games++;perBusy.push(fb);fb=0;
    const st=[f,...[0,1,2,3,4,5,6,7,8].map(i=>m[L.camPos+i]),m[L.camYaw],m[L.camPitch],...[0,1,2,3].map(i=>m[L.alive+i]),m[L.shields],m[L.mStepL]];
    if(process.env.PICDUMP){const img=new Uint8Array(256*240/8);for(let c=0;c<960;c++){const t=m[0x6800+c];if(!t||t===255)continue;const cx=c&31,cy=c>>5;for(let r=0;r<8;r++){img[(cy*8+r)*32+cx]=m[0x6000+r*256+t];}}fs.appendFileSync(process.env.PICDUMP,img);}
    if(process.env.PIC){let h=0;for(let c=0;c<960;c++){const t=m[0x6800+c];if(!t)continue;for(let r=0;r<8;r++){const v=t===255?0:m[0x6000+r*256+t];if(v)h=(Math.imul(h,33)+((c*8+r)*256+v))>>>0}}picH=(Math.imul(picH,31)+h)>>>0;if(process.env.PICLOG)console.log('PIC',f,h.toString(16));}
    for(const x of st)hash=(Math.imul(hash,31)+x)>>>0; if(process.env.TRACEH)trace.push(st.join(' '));
    if(games>=K)on=false;}}
  return r};
for(let i=0;i<30000&&(games<K);i++){
  if(i===30)for(const b of btn)nes.buttonDown(1,b);
  nes.frame(); if(on)nesF++; if(prof&&on)prof.tick();
}
if(prof)prof.report();
for(const t of trace)console.log('H',t);
perBusy.sort((a,b)=>a-b);const q=p=>perBusy[Math.floor(p*(perBusy.length-1))];
console.log(`${games} updates in ${nesF} NES frames = ${(games*60/nesF).toFixed(2)} fps | main work avg ${Math.round(busy/games)} p50 ${q(.5)} p90 ${q(.9)} max ${q(1)} | idle ${Math.round(idle/games)} | interrupts ${Math.round(intc/games)} | hash ${hash.toString(16)}${process.env.PIC?' picture '+picH.toString(16):''}`);
