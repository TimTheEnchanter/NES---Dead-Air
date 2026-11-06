#!/usr/bin/env python3
"""Builds story.inc: font, portraits, the screen layout, and the scene script."""
import sys,os,textwrap
sys.path.insert(0,os.path.dirname(__file__))
from art import PORTRAITS,PAL
FONT=open(os.path.expanduser('~/tools/cc65/libsrc/nes/nestiles.chr'),'rb').read()
o=open('story.inc','w')
def bytes_(name,v,per=32):
    o.write(f'{name}:\n')
    for i in range(0,len(v),per): o.write('  .byte '+','.join(str(x) for x in v[i:i+per])+'\n')
# ---- tiles: 0 blank, 1-10 frame and stars, 32-127 font, 128+ portraits
T=dict(TL=1,TR=2,BL=3,BR=4,H=5,V=6,STAR=7,DIM=8,MORE=9,ARROW=10)
def tile(rows,colour=1):
    p=[0]*8;q=[0]*8
    for y,r in enumerate(rows):
        for x,ch in enumerate(r):
            v=colour if ch=='#' else (2 if ch=='+' else 0)
            p[y]|=(v&1)<<(7-x); q[y]|=(v>>1)<<(7-x)
    return p+q
misc=[tile(r) for r in (
 ['........','........','........','...#####','...#....','...#....','...#....','...#....'],
 ['........','........','........','####....','...#....','...#....','...#....','...#....'],
 ['...#....','...#....','...#....','...#####','........','........','........','........'],
 ['...#....','...#....','...#....','####....','........','........','........','........'],
 ['........','........','........','########','........','........','........','........'],
 ['...#....','...#....','...#....','...#....','...#....','...#....','...#....','...#....'],
 ['........','...#....','........','.#...#..','........','........','...#....','........'],
 ['........','........','..+.....','........','.....+..','........','........','........'],
 ['........','.#####..','..###...','...#....','........','........','........','........'],
 ['.#......','.##.....','.###....','.####...','.###....','.##.....','.#......','........'])]
o.write('.segment "ASSETS"\nstoryMisc:\n')
for m in misc: o.write('  .byte '+','.join(map(str,m))+'\n')
font=[]
for c in range(32,128): font+=list(FONT[c*16:c*16+8])
bytes_('storyFont',font)
# ---- portraits: 6x6 tiles, shared tiles are stored once
uniq={};data=[];maps=[]
for n,g,e in PORTRAITS:
    m=[]
    for ty in range(6):
        for tx in range(6):
            p=[0]*8;q=[0]*8
            for y in range(8):
                for x in range(8):
                    v=g[ty*8+y][tx*8+x]; p[y]|=(v&1)<<(7-x); q[y]|=(v>>1)<<(7-x)
            k=tuple(p+q)
            if k==(0,)*16: m.append(0); continue
            if k not in uniq: uniq[k]=128+len(uniq); data+=list(k)
            m.append(uniq[k])
    maps.append(m)
assert len(uniq)<=128,len(uniq)
print('portrait tiles',len(uniq),'bytes',len(data))
o.write(f'PORT_TILES={len(uniq)}\n'); bytes_('storyPort',data)
o.write('.segment "MATH"\n'); bytes_('portMap',sum(maps,[]),36)
names={'hale':'HALE','voss':'CMDR. VOSS','mara':'MARA DRAY','ren':'REN OKAFOR','log':'VIGIL LOG'}
bytes_('portPal',sum([list(PAL[n]) for n,_,_ in PORTRAITS],[]),3)
nm=[]
for n,_,_ in PORTRAITS: nm+=[ord(c) for c in names[n].ljust(12)]
bytes_('portName',nm,12)
o.write('.segment "ASSETS"\n')
# ---- screen layout
nt=[[0]*32 for _ in range(30)]
def frame(x0,y0,x1,y1):
    nt[y0][x0]=T['TL'];nt[y0][x1]=T['TR'];nt[y1][x0]=T['BL'];nt[y1][x1]=T['BR']
    for x in range(x0+1,x1): nt[y0][x]=nt[y1][x]=T['H']
    for y in range(y0+1,y1): nt[y][x0]=nt[y][x1]=T['V']
def put(x,y,s):
    for i,c in enumerate(s): nt[y][x+i]=ord(c)
frame(3,3,10,10); frame(12,3,28,10); frame(2,12,29,24)
for x,y,t in ((14,5,'STAR'),(19,4,'DIM'),(23,6,'STAR'),(26,5,'DIM'),(16,7,'DIM'),(21,8,'STAR'),(26,8,'DIM'),(14,8,'DIM')): nt[y][x]=T[t]
put(14,2,'CVS VIGIL')
runs=[]
for y in range(30):
    x=0
    while x<32:
        if nt[y][x]==0: x+=1; continue
        s=x
        while x<32 and nt[y][x]!=0: x+=1
        runs+=[0x20+(y*32+s>>8),(y*32+s)&255,x-s]+nt[y][s:x]
attr=[0]*64
for y in range(4,10,2):
    for x in range(4,10,2):
        attr[(y//4)*8+x//4]|=1<<(((x%4)//2)+2*((y%4)//2))*2 if 0 else 1<<(2*(((x%4)//2)+2*((y%4)//2)))
bytes_('storyRuns',runs+[0])
bytes_('storyAttr',attr)
# ---- the script
OP=dict(END=0,SPEAK=1,NL=2,PAGE=3,CHOICE=4,FLAG=5,GOTO=6,FLY=7,SAVE=8,LOAD=9,IF=10,LOC=11,CLEAR=12,TITLE=13,CHAPTER=14)
P={'HALE':0,'HALE_S':1,'VOSS':2,'VOSS_H':3,'MARA':4,'MARA_H':5,'REN':6,'LOG':7}
LOCS=['BRIEFING ROOM','FLIGHT DECK','CREW BAR','VIGIL LOG']
o.write('.segment "MATH"\n')
bytes_('locNames',sum([[ord(c) for c in l.center(13)] for l in LOCS],[]),13)
W,LINES=26,4
total=0
def b(*v): o.write('  .byte '+','.join(str(x) for x in v)+'\n')
def label(n): o.write(n+':\n')
def say(who,text):
    global total
    b(OP['SPEAK'],P[who])
    lines=textwrap.wrap(text,W); assert len(lines)<=LINES,text
    for l in lines:
        o.write('  .byte "'+l+f'",{OP["NL"]}\n'); total+=len(l)
    b(OP['PAGE'])
def ask(who,prompt,a,la,bb,lb):
    b(OP['SPEAK'],P[who]); o.write(f'  .byte "{prompt}",{OP["NL"]}\n'); b(OP['CHOICE']); o.write(f'  .word {la},{lb}\n')
    o.write(f'  .byte "{a}",{OP["NL"]}\n  .byte "{bb}",{OP["NL"]}\n')
def loc(i): b(OP['LOC'],i)
def goto(l): b(OP['GOTO']); o.write(f'  .word {l}\n')
FL_CORVANE,FL_GHOST,FL_ESCAPED,FL_CONVOY=1,2,4,8
o.write('.segment "SCRIPT1"\nscript:\n')
label('menu'); loc(3)
ask('LOG','Saved progress found.','Continue','doLoad','New game','doNew')
label('doLoad'); b(OP['LOAD'])
label('doNew'); b(OP['CLEAR'])
label('sceneStart'); loc(0)
say('VOSS','Lantern Six. You are late.')
say('VOSS_H','Serrath scouts crossed the Vey lane an hour ago. Two ships, no escort.')
say('HALE','Two? Commander, that is barely a warm-up.')
say('VOSS_H','Two ships that could report a carrier that is not supposed to exist.')
say('VOSS','As far as anyone knows, this squadron died at Corvane.')
say('HALE_S','Easy to forget. Then the mail does not come.')
say('HALE','You sleep at all, Six? You look like a ghost.')
ask('HALE','How do you answer?','Still seeing Corvane.','sceneA','Ghost is the job.','sceneB')
label('sceneA'); b(OP['FLAG'],FL_CORVANE)
say('HALE_S','Yeah. Me too. Stay on my wing, all right?')
goto('sceneC')
label('sceneB'); b(OP['FLAG'],FL_GHOST)
say('HALE','Ha! Put that on the squadron patch.')
label('sceneC')
say('VOSS_H','Find them. Kill them. Come home. No radio unless you are dying.')
say('VOSS','From launch, we are dead air.')
say('HALE','Cheerful as ever. See you out there, Six.')
b(OP['FLY'],1); o.write('  .word m1Win,m1Lose\n')
label('m1Lose'); b(OP['FLAG'],FL_ESCAPED); loc(1)
say('HALE_S','Easy, Six. I got you. Shields were gone and one scout was still running.')
say('VOSS_H','Then they know where we are. We jump early, and we do not get a second warning.')
goto('deck')
label('m1Win'); loc(1)
say('HALE','Two scouts, no witnesses. You owe me a drink for the second one.')
say('VOSS','Clean work, Six. Nobody reports us today.')
b(OP['IF'],FL_CORVANE); o.write('  .word winQuiet\n'); goto('deck')
label('winQuiet')
say('HALE_S','You flew fine out there. Corvane did not come with you this time.')
label('deck')
say('REN','Commander, something just came in on the deck channel. It is on no band we use.')
say('LOG','... antern ... ix ... do you ... hear ...')
say('VOSS_H','Static. Log it and shut it off, Okafor.')
say('REN','Yes, sir.')
label('sceneBar'); loc(2)
say('MARA','So you are the one Hale will not stop talking about.')
say('MARA_H','Fly how you like. Just do not make me write another name on that wall.')
say('HALE','That is Mara being friendly. Enjoy it.')
b(OP['IF'],FL_ESCAPED); o.write('  .word barWorry\n'); goto('barRen')
label('barWorry')
say('MARA_H','Those scouts got away, and now the whole ship pays for it.')
label('barRen')
say('REN','Six, a moment. That was not static.')
say('REN','I cleaned it up. It said your call sign. Nobody out there knows we are alive.')
say('MARA_H','Then somebody is playing games with a dead channel. Leave it alone, Okafor.')
b(OP['SAVE'],0)
b(OP['CHAPTER'],4); o.write('  .word sceneM2\n')
o.write('.segment "SCRIPT2"\n')
label('sceneM2'); loc(0)
say('VOSS','Serrath convoy at Tarn Gap. Two freighters, light escort.')
say('VOSS_H','We run on what we steal. Cripple the cargo haulers and the salvage crew does the rest.')
say('HALE','Stealing lunch from the enemy. I joined the right squadron.')
say('REN','Commander, the signal came back overnight. Stronger.')
say('VOSS_H','Later, Okafor. Lanterns launch in ten.')
b(OP['IF'],FL_ESCAPED); o.write('  .word m2Worry\n'); goto('m2Go')
label('m2Worry')
say('VOSS_H','And after those scouts, assume they are looking for us.')
label('m2Go')
b(OP['FLY'],2); o.write('  .word m2Win,m2Lose\n')
label('m2Lose'); b(OP['FLAG'],FL_CONVOY); loc(1)
say('HALE_S','We lost the convoy, Six. We are on half rations by the end of the week.')
say('VOSS_H','Then we will be hungry. Dismissed.')
goto('m2Ren')
label('m2Win'); loc(1)
say('HALE','Full holds and nobody shot at the cook. Best day in months.')
say('VOSS','Good. Get some rest while it lasts.')
label('m2Ren'); loc(2)
say('REN','Six, you heard it out there too. I finished cleaning the recording.')
say('REN','It is not an echo of an old signal. It is a voice, live, and it is using our codes.')
say('REN','Lantern Two. Jun Teller.')
say('MARA_H','Teller is dead. I watched it happen, and I do not need a ghost story from you.')
say('MARA','Whoever is doing this, they took that voice from somewhere. I want to know where.')
say('HALE_S','Six, if that really is Teller, then what did we leave out there?')
b(OP['SAVE'],1)
label('m2End')
loc(3)
say('LOG','End of Mission 2. Progress saved.')
say('LOG','The story continues in Mission 3, which is not built yet.')
b(OP['TITLE'])
o.write('.segment "MATH"\nsaveTab:\n  .byte 4\n  .word sceneM2\n  .byte 4\n  .word m2End\n')
print('script chars',total)
