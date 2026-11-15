// Cycle profile for bot runs: PROF=<ld65 dbg file>. Counts only flight frames
// (game mode 1) and reports cycles per game frame by group, plus line pixels.
import fs from 'fs';
const GROUPS=[
 ['wait',/^WAIT$/],
 ['line inner loops',/^(X[DU]\d|Y[RL]\d)/],
 ['tile lookup',/^(lookNow|tileRight|tileLeft|tileDown|tileUp)$/],
 ['line setup',/^(line|xmajor|ymajor|xd|yr|yGo|start|segInit)$/],
 ['edges and clipping',/^(draw_edges|eloop|fast|skip|edge_clipped|clip2d|near_cross)/],
 ['clear map',/^clear_map/],
 ['uploads (interrupts)',/^(pat_loop|new\d|clear\d|go|top|takeJob|patCh|ntCh|ntRun|nextState|run_xfer|budUnits|exit|K_)/],
 ['blank spin (interrupts)',/^(pg|low|done)$/],
 ['nmi/irq other',/^(nmi|irq|sound_tick|radio_tick|write_attrs|tint_palette|flipdone|noflip)/],
 ['3D math',/^(transform|transform_far|project|prjSetup|prjMul|fmul|fmulFull|build_m|build_mat|build_ship_rot|storeVertex|rel_pos|visible_|sub32|vloop|rloop|jloop|floop|scale|LOCAL-MACRO)/],
];
export function attach(nes,dbg,L){
  const segs={},labs=[];const keep=new Set(['CODE','HIGH','LOOPS','HIGH2','MATH','FLIGHT']);
  const txt=fs.readFileSync(dbg,'utf8').split('\n');
  for(const l of txt)if(l.startsWith('seg\t'))segs[+/id=(\d+)/.exec(l)[1]]=/name="([^"]+)"/.exec(l)[1];
  for(const l of txt){if(!l.startsWith('sym\t')||!/type=lab/.test(l))continue;const n=/name="([^"]+)"/.exec(l)[1];if(n.startsWith('@'))continue;const s=/seg=(\d+)/.exec(l);if(!s||!keep.has(segs[+s[1]]))continue;labs.push([parseInt(/val=0x([0-9A-F]+)/i.exec(l)[1],16),n]);}
  labs.sort((a,b)=>a[0]-b[0]);const labsAddr={};for(const[a,n]of labs)labsAddr[n]??=a;const owner=new Array(65536).fill('?');let j=0;
  for(let a=0x8000;a<0x10000;a++){while(j+1<labs.length&&labs[j+1][0]<=a)j++;owner[a]=labs[j][0]<=a?labs[j][1]:'?'}
  for(let a=L.wait_buf;a<L.wait_buf+5;a++)owner[a]='WAIT';
  const grp=new Array(65536);for(let a=0;a<65536;a++){const o=owner[a];const g=GROUPS.find(([,re])=>re.test(o));grp[a]=g?g[0]:'other game code'}
  const m=nes.cpu.mem;const h={},hl={};let pix=0,lines=0,on=false,f0=0,cyc=0;
  const orig=nes.cpu.emulate.bind(nes.cpu);
  nes.cpu.emulate=function(){const pc=(this.REG_PC+1)&0xffff;
    if(on&&pc===L.line){const dx=Math.abs(m[L.x1]-m[L.x0]),dy=Math.abs(m[L.y1]-m[L.y0]);pix+=Math.max(dx,dy)+1;lines++;}
    const c=orig();if(on){h[grp[pc]]=(h[grp[pc]]||0)+c;hl[owner[pc]]=(hl[owner[pc]]||0)+c;cyc+=c}return c};
  let fr=0,last=0,tl=[],nf=0;const tc={};let gm=0,gn=0,gl=-1;const hist={},mainH=[];const fl={},heavy={};let nh=0;
  const IDLE=new Set(['wait','blank spin (interrupts)','uploads (interrupts)','nmi/irq other']);
  const o3=nes.cpu.emulate;nes.cpu.emulate=function(){const pc=(this.REG_PC+1)&0xffff;const c=o3.call(this);if(on&&!IDLE.has(grp[pc])){gm+=c;fl[owner[pc]]=(fl[owner[pc]]||0)+c}return c};
  const o2=nes.cpu.emulate;nes.cpu.emulate=function(){const pc=(this.REG_PC+1)&0xffff;const c=o2.call(this);if(on&&process.env.TIMELINE){const g=grp[pc];tc[g]=(tc[g]||0)+c;}return c};
  return {
    tick(){if(on){const g=m[0x300];gn++;if(g!==gl){if(gl>=0){hist[gn]=(hist[gn]||0)+1;mainH.push(gm);if(gm>+(process.env.HEAVY||60000)){nh++;for(const k in fl)heavy[k]=(heavy[k]||0)+fl[k]}}for(const k in fl)fl[k]=0;gl=g;gn=0;gm=0}}if(process.env.TIMELINE&&on){nf++;if(nf>600&&tl.length<40){tl.push(`nes ${nf} game ${m[0x300]} xf ${m[L.xfState]} dd ${m[L.drawDone]} bf ${m[L.bufFree]} job ${m[L.jobCnt]} | `+Object.entries(tc).filter(([k,v])=>v>300).map(([k,v])=>k.split(' ')[0]+':'+v).join(' '))}for(const k in tc)tc[k]=0;}const mode=m[0x31E];const f=m[0x300]|m[0x301]<<8;if(on)fr+=(f-last+65536)&65535;last=f;on=mode===1;},
    report(){if(!fr)return;mainH.sort((a,b)=>a-b);const q=p=>mainH[Math.floor(p*(mainH.length-1))];if(nh){console.log('HEAVY frames',nh);const hg={};for(const[k,v]of Object.entries(heavy)){const g=grp[labsAddr[k]]||'?';hg[g]=(hg[g]||0)+v}for(const[k,v]of Object.entries(hg).sort((a,b)=>b[1]-a[1]))console.log('HEAVYG',String(Math.round(v/nh)).padStart(7),k);for(const[k,v]of Object.entries(heavy).sort((a,b)=>b[1]-a[1]).slice(0,40))console.log('HEAVY',String(Math.round(v/nh)).padStart(7),k,grp[labsAddr[k]]||'')}console.log('PROF NES frames per game frame',JSON.stringify(hist),'main work cycles p10/p50/p90/max',q(.1),q(.5),q(.9),q(1));for(const x of tl)console.log('TL',x);console.log(`PROF flight game frames ${fr}, cycles/frame ${Math.round(cyc/fr)}, lines/frame ${(lines/fr).toFixed(1)}, pixels/frame ${Math.round(pix/fr)}, line cycles/pixel ${((h['line inner loops']+h['tile lookup']+h['line setup'])/pix).toFixed(1)}`);
      for(const[k,v]of Object.entries(h).sort((a,b)=>b[1]-a[1]))console.log('PROF',String(Math.round(v/fr)).padStart(7),k);
      if(process.env.PROFN)for(const[k,v]of Object.entries(hl).sort((a,b)=>b[1]-a[1]).slice(0,+process.env.PROFN))console.log('PROFL',String(Math.round(v/fr)).padStart(7),k,grp[labsAddr[k]]||'');}
  };
}
