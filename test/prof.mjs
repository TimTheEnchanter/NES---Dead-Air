// Cycles per finished game frame, attributed to named routines.
import fs from 'fs';
const {NES}=await import(process.env.HOME+'/tools/jsnes/src/nes.js').then(m=>({NES:m.default||m.NES}));
const groups=['main','clear_map','update_player','build_view','enemy_update','advance','collide','update_fire','rel_pos','laser_test','in_view','select_model','build_ship_rot','build_m','build_mat','visible_close','visible_faces','transform','transform_far','storeVertex','project','prjSetup','prjMul','draw_edges','edge_clipped','clip2d','near_cross','line','start','segInit','lookNow','tileRight','tileLeft','tileDown','tileUp','apply_hits','update_sprites','draw_bolt','fmul','fmulFull','read_pad','nmi','irq','run_xfer','budUnits','sub32','pat_loop'];
const L={};for(const l of fs.readFileSync(process.argv[3],'utf8').split('\n')){const m=l.split(' ');if(m[2])L[m[2].slice(1)]=parseInt(m[1],16)}
const pts=groups.filter(g=>L[g]).map(g=>[L[g],g]);
for(const k of Object.keys(L)) if(/^(X[DU]\d|Y[RL]\d)/.test(k)&&L[k]>=0x8000) pts.push([L[k],'line blocks']);
pts.sort((a,b)=>a[0]-b[0]);
const owner=new Array(65536).fill('other');for(let i=0;i<pts.length;i++){const e=i+1<pts.length?pts[i+1][0]:0xF000;for(let a=pts[i][0];a<e;a++)owner[a]=pts[i][1]}
for(let a=0xF000;a<0xFFFA;a++)owner[a]='uploads';
const nes=new NES({onFrame(){},onAudioSample(){},emulateSound:false});
nes.loadROM(fs.readFileSync(process.argv[2]).toString('binary'));
const h={};const orig=nes.cpu.emulate.bind(nes.cpu);
nes.cpu.emulate=function(){const pc=(this.REG_PC+1)&0xffff;const c=orig();const o=owner[pc];h[o]=(h[o]||0)+c;return c};
const N=+process.argv[4]||1800;for(let i=0;i<N;i++)nes.frame();
const fr=nes.cpu.mem[0x300]|nes.cpu.mem[0x301]<<8;let tot=0;for(const k in h)tot+=h[k];
console.log('frames',fr,'cycles/frame',Math.round(tot/fr));
for(const[k,v]of Object.entries(h).sort((a,b)=>b[1]-a[1]).slice(0,26))console.log(String(Math.round(v/fr)).padStart(7),k);
