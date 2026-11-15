// Closed-loop pilot: reads the game state each vblank and steers at the nearest scout.
import fs from 'fs';
const {NES}=await import(process.env.HOME+'/tools/jsnes/src/nes.js').then(m=>({NES:m.default||m.NES}));
const L={};for(const l of fs.readFileSync(process.argv[3],'utf8').split('\n')){const m=l.split(' ');if(m[2])L[m[2].slice(1)]=parseInt(m[1],16)}
const runs=+process.argv[4]||5, slow=+process.argv[5]||1, gate=+process.argv[6]||110;
const THR=+process.env.THR||6;
const NOISE=+process.env.NOISE||0;   // chance per decision that the pilot hesitates (seeded by run)
const target=+process.env.TARGET||2;
const w=v=>((v+2048)%4096+4096)%4096-2048;
const R0=+process.env.R0||0;for(let run=R0;run<runs;run++){
 let lastFB; const shots=[]; const nes=new NES({onFrame(fb){lastFB=fb},onAudioSample(){},emulateSound:false});
 nes.loadROM(fs.readFileSync(process.argv[2]).toString('binary'));
 const m=nes.cpu.mem; if(process.env.SAVEINIT){const v=process.env.SAVEINIT.split(',').map(Number);v.forEach((x,i)=>m[0x7F00+i]=x);} if(process.env.WATCHW){const w0=nes.cpu.write.bind(nes.cpu);nes.cpu.write=(a,v)=>{if((a===0x469||a===0x46B)&&v>70&&v<255){const sp=nes.cpu.REG_SP&0xFF;console.log('WATCHW',a.toString(16),v,'pc',nes.cpu.REG_PC.toString(16),'M',m[0x31D],'ret',[1,2,3,4,5,6].map(i=>m[0x100+((sp+i)&0xFF)].toString(16)).join(' '),'step',m[L.mStepL],'kills',m[L.killsLeft],'eb',m[L.echoBreaks]);}return w0(a,v);};}const held=new Set();const log=[];let lastM=0,maxS=0;
 const prof=process.env.PROF?(await import('./prof_hook.mjs')).attach(nes,process.env.PROF,L):null;
 const set=(b,on)=>{if(on&&!held.has(b)){nes.buttonDown(1,b);held.add(b)}if(!on&&held.has(b)){nes.buttonUp(1,b);held.delete(b)}};
 let t=0,start=null,end=null,res=null,kills=[],mode0=0;const jitter=run*7;let sd=(run+1)*2654435761>>>0;const rnd=()=>{sd=(sd+0x6D2B79F5)>>>0;let z=sd;z=Math.imul(z^z>>>15,z|1);z^=z+Math.imul(z^z>>>7,z|61);return((z^z>>>14)>>>0)/4294967296};
 for(;t<60000;t++){
  if(prof)prof.tick();
  if(process.env.STORYLOG&&m[L.storyFlag]){const v=nes.ppu.vramMem;const rows=[];for(let r=13;r<24;r++){let x='';for(let c=2;c<30;c++){const q=v[0x2000+r*32+c];x+=q>=32&&q<128?String.fromCharCode(q):' ';}rows.push(x.trimEnd());}const pg=rows.filter(x=>x.trim()).join(' / ').replace(/\s+/g,' ').trim();if(pg!==globalThis.lastPg){globalThis.lastPg=pg;console.log('| '+pg);}}
  const mode=m[0x31E];
  if(mode!==1){ for(const b of [4,5,6,7,1])set(b,false); if(process.env.PICK2&&(!process.env.PICKLATE||globalThis.lateOn))set(5,(t%40)<20); if(m[0x31D]>=10&&mode===4)globalThis.lateOn=true; const tap=(t+jitter)%24<3; set(0,tap); set(3,tap);
    if(start!==null&&mode===4&&mode0!==4){log.push(`M${m[0x31D]} ${m[L.doneWin]?'WIN':'LOSS'} ${((t-start)/60).toFixed(0)}s sh${m[0x30F]} fl${m[0x31B]}/${m[L.storyFlags2]}`);start=null;}
    if(m[0x31D]>=target&&mode===4){end=t;res=m[L.doneWin]}
  } else {
    set(3,false); if(start===null)start=t;
    const cy=m[L.camYaw]*Math.PI/128,cp=m[L.camPitch]*Math.PI/128;
    const cam=[0,3,6].map(i=>m[L.camPos+i+1]|m[L.camPos+i+2]<<8);
    let best=null;
    const spare=s=>(process.env.SPARE&&m[L.shipModel+s]===16&&m[L.hPassive+s]===1)||(m[L.shipModel+s]===32&&m[L.hPassive+s]===1);
    const cands=m[L.guardOn]&&m[L.alive]?[0]:((m[L.alive]&&!spare(0))||(m[L.alive+1]&&!spare(1))?[0,1]:[5]);
    for(const s of cands){ if(s<5&&(!m[L.alive+s]||spare(s)))continue;
      const p=[0,16,32].map(o=>m[L.shipPosL+o+s]|m[L.shipPosH+o+s]<<8);
      const d=p.map((v,i)=>w(v-cam[i]));
      const r=[Math.cos(cy)*d[0]-Math.sin(cy)*d[2],
        -Math.sin(cy)*Math.sin(cp)*d[0]+Math.cos(cp)*d[1]-Math.cos(cy)*Math.sin(cp)*d[2],
        Math.sin(cy)*Math.cos(cp)*d[0]+Math.sin(cp)*d[1]+Math.cos(cy)*Math.cos(cp)*d[2]];
      const dist=Math.hypot(...d); if(!best||dist<best.dist)best={r,dist,nav:s===5}}
    if(process.env.BESTLOG&&t%120===0&&m[0x31D]===4&&m[L.mStepL]===24)console.log('best',best&&best.nav,best&&Math.round(best.dist),best&&best.r.map(Math.round),'cam',cam,'yaw',m[L.camYaw],'pitch',m[L.camPitch]);
    if(process.env.NAVLOG&&best&&best.nav&&t%300===0)console.log('nav',Math.round(best.dist),best.r.map(Math.round),'spd',m[0x32B],'pitch',m[L.camPitch],'yaw',m[L.camYaw],'held',[...held]);
    if(best&&t%slow===0&&!(NOISE&&rnd()<NOISE)){const{r}=best;const on=r[2]>0;
      set(6,r[0]<-40||(!on&&r[0]<=0));set(7,r[0]>40||(!on&&r[0]>0));set(4,on&&r[1]>40);set(5,on&&r[1]<-40);
      if(on&&m[0x32B]<THR&&Math.abs(r[0])<200&&(best.nav||(process.env.FT&&best.dist>600))){set(1,true);set(4,true);set(5,false);}else if(m[0x32B]>THR){set(1,true);set(5,true);set(4,false);} else set(1,false);
      set(0,!best.nav&&on&&r[2]<760&&Math.hypot(r[0],r[1])<gate);
      {const pr=m[L.camPitch],pa=(pr>127?pr-256:pr)*Math.PI/128; if(Math.abs(pa)>1.05){set(1,false);set(4,pa<0);set(5,pa>0);}}} else set(0,false);
    if(t%slow===1)set(0,false);
  }
  if(process.env.DBG&&mode===1){const k=m[L.mStepL]+'/'+m[L.mPhase]+(m[0x31D]===4||m[0x31D]===7||m[0x31D]===8||m[0x31D]===11?'/s'+m[L.shuttleHP]:'')+(m[0x31D]===10?'/a'+m[L.alive]+m[L.alive+1]+'m'+m[L.shipModel]+'k'+m[L.killsLeft]:'')+(m[0x31D]===9?'/seen'+m[L.sneakSeen]+'w'+m[L.sneakWarned]+'L'+(m[0x31B]>>5):'')+(m[0x31D]===6?'/h'+m[L.alive+2]+'e'+m[L.echoDis]:'');if(k!==lastM){log.push(`${k}@${(t/60)|0}`);lastM=k}}
  if(process.env.TRACE&&mode===1&&t%600===0&&m[0x31D]===+(process.env.TRACE)&&(!process.env.TSTEP||m[L.mStepL]===+process.env.TSTEP)){const cam=[0,3,6].map(i=>m[L.camPos+i+1]|m[L.camPos+i+2]<<8);const d=[0,1].map(s=>m[L.alive+s]?Math.round(Math.hypot(...[0,16,32].map((o,i)=>w((m[L.shipPosL+o+s]|m[L.shipPosH+o+s]<<8)-cam[i])))):'-');console.log(`t${(t/60)|0} d ${d} hp ${[0,1].map(i=>m[L.hp+i])} brk ${[0,1].map(i=>m[L.hBreak+i])} turn ${[0,1].map(i=>m[L.hTurn+i])} spd ${m[0x32B]} sh ${m[0x30F]}`)}
  if(process.env.CRASHLOG&&m[0x311]!==(globalThis.lc??0)){globalThis.lc=m[0x311];const cam=[0,3,6].map(i=>m[L.camPos+i+1]|m[L.camPos+i+2]<<8);console.log(`crash M${m[0x31D]} step ${m[L.mStepL]} `+[0,1,2,3].map(s=>m[L.alive+s]?`${s}/${m[L.shipModel+s]}:${Math.round(Math.hypot(...[0,16,32].map((o,i)=>w((m[L.shipPosL+o+s]|m[L.shipPosH+o+s]<<8)-cam[i]))))}`:'').join(' ')+` spd ${m[0x32B]}`)}
  if(process.env.RADIOLOG&&m[L.radioT]===219){console.log(`M${m[0x31D]} line ${m[L.radioPage]*10+m[L.radioOff]/24+1} kills ${m[L.killsLeft]} models ${[0,1].map(s=>m[L.shipModel+s])}`)}
  try{ nes.frame(); }catch(e){ console.log('CRASH',e.message,'t',t,'M',m[0x31D],'mode',m[0x31E],'step',m[L.mStepL],'phase',m[L.mPhase],'alive',[0,1,2,3,4].map(i=>m[L.alive+i]),'models',[0,1,2,3].map(i=>m[L.shipModel+i]),'echoDis',m[L.echoDis],'guard',m[L.guardOn],'mode2',m[L.missionMode],'tint',m[L.tintFrames],m[L.tintSend],'xf',m[L.xfState],'radio',m[L.radioReq],m[L.radioPage],'SP',nes.cpu.REG_SP.toString(16),'stack',Array.from({length:16},(_,i)=>m[0x1F0+i].toString(16)).join(' ')); /*CRASHDUMP*/ process.exit(1);}
  if(process.env.SHOTS&&mode===1&&(m[L.shipModel]===16||m[L.shipModel]===32)&&m[L.alive]&&t%20===0&&shots.length<12){const a=new Uint8Array(256*240*3);for(let i=0;i<256*240;i++){const c=lastFB[i];a[i*3]=c&255;a[i*3+1]=(c>>8)&255;a[i*3+2]=(c>>16)&255;}shots.push(Buffer.from(a)); if(shots.length===12)fs.writeFileSync(process.env.SHOTS,Buffer.concat(shots));}
  mode0=mode;
  if(end!==null&&t>end+5)break;
  if(m[0x7F03]===7&&m[0x7F00]===0x44&&process.env.ENDSTOP){console.log('ENDED t',(t/60)|0,'flags2',m[0x7F06]);break;}
 }
 if(prof)prof.report();
 if(process.env.SAVEDUMP)console.log('SAVEBYTES',[...Array(27).keys()].map(i=>m[0x7F00+i]).join(','),'difficulty var',m[L.difficulty]);
 console.log(`save resume ${m[0x7F03]} flags ${m[0x7F04]} flags2 ${m[0x7F06]} diff ${m[0x7F08]}/${m[0x7F09]}`);console.log(`alive ${[0,1,2,3].map(i=>m[L.alive+i])} kills ${m[L.killsLeft]} phase ${m[L.mPhase]} step ${m[L.mStepL]} hp ${[0,1].map(i=>m[L.hp+i])} speed ${m[0x32B]}`);console.log(`run ${run}: ${log.join(' | ')||'-'} mode ${m[0x31E]} mission ${m[0x31D]} t ${(t/60)|0}, shields ${m[0x30F]}, enemy shots ${m[0x312]}, hit by ${m[0x313]}, crashes ${m[0x311]}, player hits ${m[0x310]}, fps ${(m[0x300]|m[0x301]<<8)*60/(m[0x302]|m[0x303]<<8)|0}`);
}
