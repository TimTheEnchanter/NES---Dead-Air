import {createRequire} from 'module';import fs from 'fs';
const require=createRequire('/root/x/');
let pw;try{pw=require('playwright')}catch{pw=await import('/usr/local/lib/node_modules/playwright/index.mjs').then(m=>m.default||m)}
const b=await pw.chromium.launch();
for(const[w,h,name]of[[1200,800,'desk'],[400,800,'phone']]){
 const p=await b.newPage({viewport:{width:w,height:h}});const errs=[];p.on('pageerror',e=>errs.push(e.message));p.on('console',m=>m.type()==='error'&&errs.push(m.text()));
 await p.setContent('<!doctype html><html><head><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1"></head><body>'+fs.readFileSync('dead-air.html','utf8')+'</body></html>',{waitUntil:'domcontentloaded'});
 await p.waitForTimeout(3000);
 const m0=await p.evaluate(()=>M()[0x31E]);await p.keyboard.press('Enter',{delay:80});await p.waitForTimeout(500);const m1=await p.evaluate(()=>M()[0x31E]);console.log('mode title->after Enter',m0,m1);await p.waitForTimeout(1200);await p.screenshot({path:`/tmp/story_${name}.png`});for(let i=0;i<70&&(await p.evaluate(()=>M()[0x31E]))===3;i++){await p.keyboard.press('KeyZ',{delay:80});await p.waitForTimeout(350)}console.log('mode after text',await p.evaluate(()=>M()[0x31E]),'flags',await p.evaluate(()=>M()[0x31B]));if(name==='desk'){const r=await p.evaluate(()=>{saveCopy=[0x44,0x41,1,0,5,0xD1];boot(rom);return [...M().slice(0x7F00,0x7F06)]});console.log('restored save bytes',r);await p.waitForTimeout(800);await p.keyboard.press('Enter',{delay:80});await p.waitForTimeout(1500);await p.screenshot({path:'/tmp/menu.png'});console.log('menu mode',await p.evaluate(()=>M()[0x31E]));await p.evaluate(()=>{saveCopy=null;for(let i=0;i<6;i++)M()[0x7F00+i]=0;boot(rom)});await p.waitForTimeout(800);await p.keyboard.press('Enter',{delay:80});await p.waitForTimeout(800);for(let i=0;i<70&&(await p.evaluate(()=>M()[0x31E]))===3;i++){await p.keyboard.press('KeyZ',{delay:80});await p.waitForTimeout(350)}}
 await p.keyboard.down('KeyZ');await p.waitForTimeout(900);await p.keyboard.up('KeyZ');await p.keyboard.down('KeyX');await p.keyboard.down('ArrowUp');await p.waitForTimeout(150);await p.keyboard.up('ArrowUp');await p.keyboard.up('KeyX');await p.keyboard.down('ArrowRight');await p.waitForTimeout(300);await p.keyboard.up('ArrowRight');
 await p.waitForTimeout(1500);
 const o=await p.evaluate(()=>({fps:$('fps').textContent,speed:$('speed').textContent,score:$('score').textContent,shields:$('shields').textContent,tiles:$('tiles').textContent,sw:document.documentElement.scrollWidth,cw:document.documentElement.clientWidth,nmi:word(0x302),lit:[...g.getImageData(0,0,256,240).data].filter((v,i)=>i%4==0&&v>64).length}));
 console.log(name,o,errs.slice(0,4).map(s=>s.slice(0,120)));
 if(name==='desk'){await p.click('#sound');await p.waitForTimeout(1500);console.log('audio',await p.evaluate(()=>({btn:$('sound').textContent,ctx:audio&&audio.state,peak:Math.max(...ring.map(Math.abs)),t0:performance.now()})));
  const a=await p.evaluate(()=>word(0x302));await p.waitForTimeout(3000);const b2=await p.evaluate(()=>word(0x302));console.log('vblanks per second with sound',Math.round((b2-a)/3));await p.click('#flight');await p.waitForFunction(()=>$('result').textContent.startsWith('Test flight:'),null,{timeout:80000});console.log(await p.textContent('#result'))}
 await p.screenshot({path:`/tmp/page_${name}.png`,fullPage:true});
}
await b.close();
