// Story transcript reader for -D STORYTEST builds:
//   node test/story_read.mjs <rom> <choices, e.g. 2,1> [frames]
// Presses Fire through the pages and prints each new page of the text box.
// At each choice it presses Down (choice-1) times first. Text is read from
// the nametable, so this checks wording and branching, not looks.
import fs from 'fs';
const {NES}=await import(process.env.HOME+'/tools/jsnes/src/nes.js').then(m=>({NES:m.default||m.NES}));
let FB;const nes=new NES({onFrame(fb){FB=fb},onAudioSample(){},emulateSound:false});
nes.loadROM(fs.readFileSync(process.argv[2]).toString('binary'));
if(process.env.SAVEINIT)process.env.SAVEINIT.split(',').map(Number).forEach((x,i)=>nes.cpu.mem[0x7F00+i]=x);
const choices=(process.argv[3]||'').split(',').filter(Boolean).map(Number);
const N=+process.argv[4]||6000;
const page=()=>{const v=nes.ppu.vramMem;const rows=[];for(let r=13;r<24;r++){let s='';for(let c=2;c<30;c++){const t=v[0x2000+r*32+c];s+=t>=32&&t<128?String.fromCharCode(t):' ';}rows.push(s.trimEnd());}
  return rows.filter(x=>x.trim()).join(' / ').replace(/\s+/g,' ').trim();};
let last='',seen=[],ci=0,cool=0;
for(let i=0;i<60;i++)nes.frame();nes.buttonDown(1,3);for(let i=0;i<6;i++)nes.frame();nes.buttonUp(1,3);
const FS=(process.env.FRAMESHOTS||'').split(',').filter(Boolean).map(Number);
for(let f=0;f<N;f++){
  nes.frame();
  if(FS.includes(f)){const a=Buffer.alloc(256*240*3);for(let i=0;i<256*240;i++){const c=FB[i];a[i*3]=c&255;a[i*3+1]=(c>>8)&255;a[i*3+2]=(c>>16)&255;}fs.appendFileSync(process.env.SHOTOUT,a);}
  if(cool>0){cool--;continue;}
  const p=page();
  if(p!==last){last=p;seen.push(p);console.log('|',p);
    if(process.env.SHOTRE&&new RegExp(process.env.SHOTRE).test(p)&&!globalThis.shotDone?.[p.slice(0,20)]){(globalThis.shotDone??={})[p.slice(0,20)]=1;for(let i=0;i<150;i++)nes.frame();const a=Buffer.alloc(256*240*3);for(let i=0;i<256*240;i++){const c=FB[i];a[i*3]=c&255;a[i*3+1]=(c>>8)&255;a[i*3+2]=(c>>16)&255;}fs.appendFileSync(process.env.SHOTOUT,a);last=page();}}
  const isChoice=/(Choose a chapter|The story is complete|What do you do\?|How do you save them\?|Whose plan|How do you answer|Will you take me|You find Ren alone|Choose a difficulty|Saved progress)/.test(p);
  if(isChoice){for(let i=0;i<120;i++)nes.frame();const n=ci<choices.length?choices[ci]:1;ci++;
    for(let k=1;k<n;k++){nes.buttonDown(1,5);for(let i=0;i<6;i++)nes.frame();nes.buttonUp(1,5);for(let i=0;i<10;i++)nes.frame();}
    console.log('| [choice '+n+'] '+page());}
  nes.buttonDown(1,0);for(let i=0;i<4;i++)nes.frame();nes.buttonUp(1,0);cool=30;
  if(/Choose New game to fly it again/.test(p))break;
}
