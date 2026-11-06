// Closed-loop pilot: reads the game state each vblank and steers at the nearest scout.
import fs from 'fs';
const {NES}=await import(process.env.HOME+'/tools/jsnes/src/nes.js').then(m=>({NES:m.default||m.NES}));
const L={};for(const l of fs.readFileSync(process.argv[3],'utf8').split('\n')){const m=l.split(' ');if(m[2])L[m[2].slice(1)]=parseInt(m[1],16)}
const runs=+process.argv[4]||5, slow=+process.argv[5]||1, gate=+process.argv[6]||110;
const THR=+process.env.THR||6;
const target=+process.env.TARGET||2;
const w=v=>((v+2048)%4096+4096)%4096-2048;
const R0=+process.env.R0||0;for(let run=R0;run<runs;run++){
 const nes=new NES({onFrame(){},onAudioSample(){},emulateSound:false});
 nes.loadROM(fs.readFileSync(process.argv[2]).toString('binary'));
 const m=nes.cpu.mem;const held=new Set();const log=[];let lastM=0,maxS=0;
 const set=(b,on)=>{if(on&&!held.has(b)){nes.buttonDown(1,b);held.add(b)}if(!on&&held.has(b)){nes.buttonUp(1,b);held.delete(b)}};
 let t=0,start=null,end=null,res=null,kills=[],mode0=0;const jitter=run*7;
 for(;t<60000;t++){
  const mode=m[0x31E];
  if(mode!==1){ for(const b of [4,5,6,7,1])set(b,false); const tap=(t+jitter)%24<3; set(0,tap); set(3,tap);
    if(start!==null&&mode===4&&mode0!==4){log.push(`M${m[0x31D]} ${m[L.doneWin]?'WIN':'LOSS'} ${((t-start)/60).toFixed(0)}s sh${m[0x30F]}`);start=null;}
    if(m[0x31D]>=target&&mode===4){end=t;res=m[L.doneWin]}
  } else {
    set(3,false); if(start===null)start=t;
    const cy=m[L.camYaw]*Math.PI/128,cp=m[L.camPitch]*Math.PI/128;
    const cam=[0,3,6].map(i=>m[L.camPos+i+1]|m[L.camPos+i+2]<<8);
    let best=null;
    for(const s of (m[L.alive]||m[L.alive+1]?[0,1]:[5])){ if(s<5&&!m[L.alive+s])continue;
      const p=[0,16,32].map(o=>m[L.shipPosL+o+s]|m[L.shipPosH+o+s]<<8);
      const d=p.map((v,i)=>w(v-cam[i]));
      const r=[Math.cos(cy)*d[0]-Math.sin(cy)*d[2],
        -Math.sin(cy)*Math.sin(cp)*d[0]+Math.cos(cp)*d[1]-Math.cos(cy)*Math.sin(cp)*d[2],
        Math.sin(cy)*Math.cos(cp)*d[0]+Math.sin(cp)*d[1]+Math.cos(cy)*Math.cos(cp)*d[2]];
      const dist=Math.hypot(...d); if(!best||dist<best.dist)best={r,dist,nav:s===5}}
    if(best&&t%slow===0){const{r}=best;const on=r[2]>0;
      set(6,r[0]<-40||(!on&&r[0]<=0));set(7,r[0]>40||(!on&&r[0]>0));set(4,on&&r[1]>40);set(5,on&&r[1]<-40);
      if(best.nav&&on&&m[0x32B]<THR&&Math.abs(r[0])<200){set(1,true);set(4,true);set(5,false);}else set(1,false);
      set(0,!best.nav&&on&&r[2]<760&&Math.hypot(r[0],r[1])<gate);} else set(0,false);
    if(t%slow===1)set(0,false);
  }
  nes.frame();mode0=mode;
  if(end!==null&&t>end+5)break;
 }
 console.log(`alive ${[0,1,2,3].map(i=>m[L.alive+i])} kills ${m[L.killsLeft]} phase ${m[L.mPhase]} step ${m[L.mStepL]} hp ${[0,1].map(i=>m[L.hp+i])} speed ${m[0x32B]}`);console.log(`run ${run}: ${log.join(' | ')||'-'} mode ${m[0x31E]} mission ${m[0x31D]} t ${(t/60)|0}, shields ${m[0x30F]}, enemy shots ${m[0x312]}, hit by ${m[0x313]}, crashes ${m[0x311]}, player hits ${m[0x310]}, fps ${(m[0x300]|m[0x301]<<8)*60/(m[0x302]|m[0x303]<<8)|0}`);
}
