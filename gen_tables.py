#!/usr/bin/env python3
"""Generates tables.inc: math tables and the ship model for Dead Air."""
import math,os
D, SCALE_NEAR, SCALE_FAR, SCALE_TINY = 240, 0.46, 0.32, 0.20   # camera distance, projection scales

# Original wedge-shaped hull: a pentagonal frustum (10 verts, 15 edges, 7 faces).
rear  = [(0,22),(70,6),(38,-20),(-38,-20),(-70,6)]
front = [(x*0.28,y*0.28) for x,y in rear]
V = [(round(x),round(y),-48) for x,y in rear]+[(round(x),round(y),58) for x,y in front]
F_raw = [[0,1,2,3,4],[9,8,7,6,5]]+[[i,(i+1)%5+5 if 0 else i+5,(i+1)%5+5,(i+1)%5] for i in range(5)]

def sub(a,b): return tuple(p-q for p,q in zip(a,b))
def cross(a,b): return (a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0])
def dot(a,b): return sum(p*q for p,q in zip(a,b))
cen = tuple(sum(c)/len(V) for c in zip(*V))
F=[]
for f in F_raw:
    n=cross(sub(V[f[1]],V[f[0]]),sub(V[f[2]],V[f[0]]))
    if dot(n,sub(V[f[0]],cen))<0: f=f[::-1]
    F.append(f[:3])          # 3D winding now faces outward
E={}
for i,f in enumerate(F_raw):
    for a,b in zip(f,f[1:]+f[:1]):
        k=(min(a,b),max(a,b)); E[k]=E.get(k,0)|(1<<i)
assert len(V)==10 and len(E)==15 and len(F)==7

# Low-detail model for distant ships: same span, 5 verts, 8 edges, 5 faces.
V2=[(0,22,-48),(70,6,-48),(0,-20,-48),(-70,6,-48),(0,2,58)]
F2_raw=[[0,1,2,3]]+[[i,4,(i+1)%4] for i in range(4)]
cen2=tuple(sum(c)/len(V2) for c in zip(*V2))
F2=[]
for f in F2_raw:
    n=cross(sub(V2[f[1]],V2[f[0]]),sub(V2[f[2]],V2[f[0]]))
    if dot(n,sub(V2[f[0]],cen2))<0: f=f[::-1]
    F2.append(f[:3])
E2={}
for i,f in enumerate(F2_raw):
    for a,b in zip(f,f[1:]+f[:1]):
        k=(min(a,b),max(a,b)); E2[k]=E2.get(k,0)|(1<<i)
assert len(E2)==8

# Slab hull: the Vigil and the freighters, scaled per ship. 8 verts, 12 edges, 6 faces.
V3=[(-35,14,-75),(35,14,-75),(35,-14,-75),(-35,-14,-75),(-16,6,75),(16,6,75),(16,-6,75),(-16,-6,75)]
F3_raw=[[0,1,2,3],[7,6,5,4],[0,4,5,1],[1,5,6,2],[2,6,7,3],[3,7,4,0]]
cen3=(0,0,0)
F3=[]
for f in F3_raw:
    n=cross(sub(V3[f[1]],V3[f[0]]),sub(V3[f[2]],V3[f[0]]))
    if dot(n,sub(V3[f[0]],cen3))<0: f=f[::-1]
    F3.append(f[:3])
E3={}
for i,f in enumerate(F3_raw):
    for a,b in zip(f,f[1:]+f[:1]):
        k=(min(a,b),max(a,b)); E3[k]=E3.get(k,0)|(1<<i)
assert len(E3)==12

# Lancer: a broad, heavy arrowhead. Convex, 6 verts, 8 triangles, 12 edges.
V4=[(0,0,95),(-75,0,-30),(75,0,-30),(0,26,-35),(0,-20,-35),(0,0,-60)]
import itertools
F4=[]
for a,b,c in itertools.combinations(range(len(V4)),3):
    n=cross(sub(V4[b],V4[a]),sub(V4[c],V4[a]))
    side=[dot(n,sub(V4[k],V4[a])) for k in range(len(V4)) if k not in (a,b,c)]
    if all(s<0 for s in side): F4.append([a,b,c])
    elif all(s>0 for s in side): F4.append([a,c,b])
assert len(F4)==8,F4
E4={}
for i,f in enumerate(F4):
    for a,b in zip(f,f[1:]+f[:1]):
        k=(min(a,b),max(a,b)); E4[k]=E4.get(k,0)|(1<<i)
assert len(E4)==12 and max(math.dist(v,(0,0,0)) for v in V4)<=100

R=max(math.dist(v,(0,0,0)) for v in V)
assert R<=100, R
def recip(z,s): return min(127,round(127*s*D/(D+z)))
for s,lim in ((SCALE_NEAR,63),(SCALE_FAR,46),(SCALE_TINY,32)):
    assert R*recip(-R,s)/128<lim-1,(R,s)

o=open('tables.inc','w')
def tab(name,vals,w='.byte'):
    o.write(f'{name}:\n')
    for i in range(0,len(vals),16):
        o.write(f'  {w} '+','.join(str(v) for v in vals[i:i+16])+'\n')
sq=[(i-256)**2//2 for i in range(512)]
o.write('.segment "MATH"\n')
tab('sqrLo',[v&255 for v in sq]); tab('sqrHi',[(v>>8)&255 for v in sq])
tab('sinTab',[round(127*math.sin(2*math.pi*i/256))&255 for i in range(256)])
tab('recipNear',[recip(i-128,SCALE_NEAR) for i in range(256)])
tab('recipFar',[recip(i-128,SCALE_FAR) for i in range(256)])
tab('recipTiny',[recip(i-128,SCALE_TINY) for i in range(256)])
tab('recipN',[min(127,round(127*128/max(i,128))) for i in range(256)])
tab('shiftTab',[0 if i==0 else i.bit_length() for i in range(256)])
tab('ppuHi',[i>>4 for i in range(256)]); tab('ppuLo',[(i<<4)&255 for i in range(256)])
tab('mapRowLo',[(((t+3)*32)&255)+4 for t in range(16)])
tab('mapRowHi',[0x68+(((t+3)*32)>>8) for t in range(16)])
tab('bitTab',[0x80>>i for i in range(8)])
tab('div3',[i//3 for i in range(256)])
DIG=['$3C,$66,$6E,$76,$66,$66,$3C,0','$18,$38,$18,$18,$18,$18,$7E,0','$3C,$66,$06,$0C,$18,$30,$7E,0','$3C,$66,$06,$1C,$06,$66,$3C,0','$0C,$1C,$3C,$6C,$7E,$0C,$0C,0','$7E,$60,$7C,$06,$06,$66,$3C,0','$1C,$30,$60,$7C,$66,$66,$3C,0','$7E,$06,$0C,$18,$30,$30,$30,0','$3C,$66,$66,$3C,$66,$66,$3C,0','$3C,$66,$66,$3E,$06,$0C,$38,0']
o.write('digits:\n'+''.join(f'  .byte {d}\n' for d in DIG))
o.write('sparkVX: .byte 3,2,0,254,253,254,0,2\nsparkVY: .byte 0,2,3,2,0,254,253,254\n')
# ---- sound effects: one (control, period low, period high) step per vblank ----
CPU=1789773
def per(hz): return max(8,min(2047,round(CPU/(16*hz)-1)))
def pulse(steps,duty):
    out=[]
    for hz,vol in steps:
        p=per(hz); out+=[0x30|duty<<6|vol,p&255,p>>8]
    return out+[0]
def noise(steps):
    out=[]
    for period,vol,mode in steps:
        out+=[0x30|vol,mode<<7|period,0]
    return out+[0]
laser=pulse([(1400*0.93**i,max(2,11-i)) for i in range(10)],2)
enemy=pulse([(620*0.95**i,max(2,10-i//2)) for i in range(16)],1)
hit=noise([(5,12,0),(6,10,0),(7,7,0),(8,4,0),(8,2,0)])
explo=noise([(9+min(i//6,6),max(1,15-i//3),0) for i in range(42)])
crash=noise([(12,15,1),(12,14,1),(13,13,0),(13,12,0)]+[(14,max(1,11-i//2),0) for i in range(20)])
blip=pulse([(880,3),(880,1)],2)
start=pulse([(f,9) for f in (392,392,392,392,523,523,523,523,659,659,659,659,784,784,784,784)]+[(784,max(1,8-i)) for i in range(8)],1)
SFX=[('Laser',laser,0,1),('Enemy',enemy,1,1),('Hit',hit,2,1),('Explo',explo,2,3),('Crash',crash,2,2),('Start',start,1,2),('Blip',blip,1,0)]
for n,d,c,pr in SFX: o.write(f'sfx{n}:\n  .byte '+','.join(str(v) for v in d)+'\n')
o.write('sfxLo: .byte 0,'+','.join(f'<sfx{n}' for n,*_ in SFX)+'\n')
o.write('sfxHi: .byte 0,'+','.join(f'>sfx{n}' for n,*_ in SFX)+'\n')
o.write('sfxPrio: .byte 0,'+','.join(str(pr) for *_,pr in SFX)+'\n')
o.write('sfxChan: .byte 0,'+','.join(str(c) for _,_,c,_ in SFX)+'\n')
for i,(n,*_) in enumerate(SFX): o.write(f'SFX_{n.upper()}={i+1}\n')
o.write('engCtl: .byte $30,'+','.join(str(0x30|min(4,2+s//6)) for s in range(1,21))+'\n')
o.write('engPer: .byte 0,'+','.join(str(max(10,14-s//5)) for s in range(1,21))+'\n')
# ---- vector text: strokes on a 4x6 grid ----
FONT={'A':[(0,6,2,0),(2,0,4,6),(1,4,3,4)],'E':[(4,0,0,0),(0,0,0,6),(0,6,4,6),(0,3,3,3)],'F':[(4,0,0,0),(0,0,0,6),(0,3,3,3)],
'G':[(4,0,0,0),(0,0,0,6),(0,6,4,6),(4,6,4,3),(4,3,2,3)],'H':[(0,0,0,6),(4,0,4,6),(0,3,4,3)],'I':[(2,0,2,6)],
'L':[(0,0,0,6),(0,6,4,6)],'M':[(0,6,0,0),(0,0,2,3),(2,3,4,0),(4,0,4,6)],'O':[(0,0,4,0),(4,0,4,6),(4,6,0,6),(0,6,0,0)],
'P':[(0,6,0,0),(0,0,4,0),(4,0,4,3),(4,3,0,3)],'R':[(0,6,0,0),(0,0,4,0),(4,0,4,3),(4,3,0,3),(1,3,4,6)],
'C':[(4,0,0,0),(0,0,0,6),(0,6,4,6)],'D':[(0,0,0,6),(0,0,3,0),(3,0,4,2),(4,2,4,4),(4,4,3,6),(3,6,0,6)],'N':[(0,6,0,0),(0,0,4,6),(4,6,4,0)],'S':[(4,0,0,0),(0,0,0,3),(0,3,4,3),(4,3,4,6),(4,6,0,6)],'T':[(0,0,4,0),(2,0,2,6)],'V':[(0,0,2,6),(2,6,4,0)],'W':[(0,0,1,6),(1,6,2,2),(2,2,3,6),(3,6,4,0)]}
def text(name,lines):
    out=[]
    for s,x,y,scale,pitch in lines:
        for i,ch in enumerate(s):
            for a,b,c,d in FONT.get(ch,[]):
                q=[x+i*pitch+a*scale,y+b*scale,x+i*pitch+c*scale,y+d*scale]
                assert 0<=q[0]<192 and 0<=q[2]<192 and 1<=q[1]<127 and 1<=q[3]<127,(s,q)
                out+=q
    o.write(f'{name}:\n')
    for i in range(0,len(out),16): o.write('  .byte '+','.join(str(v) for v in out[i:i+16])+'\n')
    o.write('  .byte 255\n')
text('textTitle',[('DEAD AIR',34,7,3,16),('PRESS START',36,104,2,11)])
text('textWin',[('MISSION',60,44,3,16),('COMPLETE',36,70,3,16)])
text('textLose',[('SHIELDS',60,44,3,16),('DOWN',84,70,3,16)])
text('textOver',[('GAME OVER',24,44,3,16),('PRESS START',36,104,2,11)])
# ---- music: triangle, one note per step ----
def tri(m):
    hz=440*2**((m-69)/12); return round(CPU/(32*hz)-1)
NOTES=sorted(set([33,40,45,41,48,53,36,43,52,38,50,57,28]))
ix={m:i+1 for i,m in enumerate(NOTES)}
o.write('noteLo: .byte 0,'+','.join(str(tri(m)&255) for m in NOTES)+'\n')
o.write('noteHi: .byte 0,'+','.join(str(tri(m)>>8) for m in NOTES)+'\n')
def bar(r): return [r,r+12,0,r,r+7,0,r+12,r+7]
title=bar(33)+bar(33)+bar(41-12+12-0 if 0 else 41)+bar(36)+bar(33)+bar(33)+bar(38)+[40,52,0,40,47 if 0 else 52,0,40,52]
o.write('musTitle: .byte '+','.join(str(ix[n] if n else 0) for n in title)+',254\n')
o.write('musOver: .byte '+','.join(str(ix[n]) for n in (52,50,48,45,40,33))+',255\n')
# ---- radio: one line of up to 24 capitals under the view ----
FONTROM=open(os.path.expanduser('~/tools/cc65/libsrc/nes/nestiles.chr'),'rb').read()
RC="ABCDEFGHIJKLMNOPQRSTUVWXYZ.,!?':"
assert len(RC)==32
o.write('radioFont:\n')
for c in RC: o.write('  .byte '+','.join(str(v) for v in FONTROM[ord(c)*16:ord(c)*16+8])+'\n')
RL=[('WING','HALE: ON YOUR WING, SIX.'),('TALLY','HALE: TWO SCOUTS AHEAD.'),('GOOD','HALE: GOOD HIT, SIX!'),('ONE','HALE: SCOUT DOWN!'),('BOLT','HALE: BREAK, SIX! BREAK!'),('HALE_HIT','HALE: I TAGGED ONE!'),('LOW','HALE: SHIELDS LOW, SIX!'),('STATIC',"... ANTERN ... IX ..."),
 ('NAV','HALE: NAV ONE. QUIET.'),('CLEAR','HALE: SCOUTS DOWN. NICE.'),('HOME','HALE: TAKE US HOME, SIX.'),('LAND','VIGIL: CLEAR TO LAND.'),
 ('CONVOY','HALE: CONVOY AHEAD!'),('CARGO','HALE: CARGO SECURED!'),('VOICE',"... SIX ... CAN YOU HEAR"),('EMPTY','HALE: NOTHING OUT HERE.'),('RUN',"HALE: THEY'RE RUNNING!"),('JUMPED','HALE: CONVOY JUMPED!'),('ONEAWAY','HALE: ONE GOT AWAY.'),
 ('BUOY',"HALE: THERE'S THE BUOY."),('BUOYDEAD',"HALE: BUOY'S DEAD."),('TBEHIND','TELLER: SIX, BEHIND YOU!'),('TNEXT','TELLER: NEXT POINT. TWO.'),('HAULER','HALE: ONE HAULER DOWN!'),
 ('LANCERDN','HALE: LANCER DOWN!'),
 # Mara on your wing
 ('M_WING','MARA: ON YOUR WING, SIX.'),('M_GOOD','MARA: GOOD HIT.'),('M_ONE','MARA: SCOUT DOWN.'),('M_BOLT','MARA: BREAK, SIX!'),
 ('M_HIT','MARA: TAGGED ONE.'),('M_LOW','MARA: SHIELDS LOW, SIX!'),('M_HOME','MARA: TAKE US HOME.'),('M_LANCERDN','MARA: LANCER DOWN.'),
 ('M_QUIET','MARA: QUIET. TOO QUIET.'),('M_LANCER','MARA: LANCER! WATCH IT.'),('M_POSTAHEAD','MARA: CLEAR. POST AHEAD.'),
 ('M_POST',"MARA: THERE'S THE POST."),('T_FLY','TELLER: THEY MAKE US FLY'),
 # Ren in the shuttle
 ('R_WING',"REN: SHUTTLE'S UP, SIX."),('R_ONE',"REN: THAT'S ONE!"),('R_BOLT',"REN: THEY'RE FIRING!"),('R_LOW','REN: SIX, YOUR SHIELDS!'),
 ('R_HOME','REN: TAKE US HOME, SIX.'),('RENHIT',"REN: WE'RE HIT, SIX!"),('RENFAIL','REN: SHUTTLE FAILING!'),
 ('R_LISTEN','REN: LISTENING. HOLD ON.'),('R_MORE','REN: MORE THAN ONE VOICE'),('R_OURS',"REN: THEY'RE ALL OURS."),
 ('R_BEHIND','REN: SIX, BEHIND US!'),
 # Mission 5: Hale in danger, and the echo drone
 ('ONTAIL',"HALE: ONE'S ON MY TAIL!"),('HURRY','HALE: SIX! HURRY!'),('THANKS','HALE: THANKS, SIX!'),('HALEHIT',"HALE: I'M HIT, SIX..."),
 ('ECHOCONTACT','HALE: ONE CONTACT. ALONE'),('ECHOSTOP','HALE: IT STOPPED. SIX?'),('HIGHLEFT','HALE: HIGH AND LEFT...'),
 ('THATSHER',"HALE: SIX, THAT'S TELLER"),('ECHODN',"HALE: IT'S GONE."),('ECHOLEFT','...      SIX      ...'),
 ('M_ECHOSTOP','MARA: IT STOPPED.'),('M_HIGHLEFT','MARA: HIGH AND LEFT...'),('M_THATSHER',"MARA: THAT'S HER. JUN."),('M_ECHODN',"MARA: IT'S GONE."),
 ('M_ECHOCONTACT','MARA: ONE CONTACT. ALONE'),
 ('ASH','HALE: ASHGROVE AHEAD.'),('M_ASH','MARA: ASHGROVE AHEAD.'),('DOCKED','REN: DOCKED. COPYING.'),
 ('ORDERS','REN: GOT THE ORDERS.'),('YOURFILE','REN: SIX... YOUR FILE.'),('WRECKHIT',"REN: THEY'RE HITTING US!"),
 ('WRECKLOW',"REN: HULL'S BREAKING UP!"),
 ('V_ECHOSTOP','REN: IT STOPPED, SIX.'),('V_THATSHER',"REN: SIX... THAT'S HER."),('V_HOME','VIGIL: COME HOME, SIX.')]
# Radio text is only ever shown in flight: the first pages live in the flight
# bank, the rest in the fixed CODE bank. One label per page of ten lines.
FLIGHT_PAGES=5
o.write(f'RADIO_PAGES={(len(RL)+9)//10}\n')
for i,(n,t) in enumerate(RL):
    assert len(t)<=24,t
    if i%10==0:
        o.write('.segment "%s"\nradioT%d:\n'%('FLIGHT' if i//10<FLIGHT_PAGES else 'CODE',i//10))
    o.write('  .byte '+','.join(str(223+RC.index(c) if c!=' ' else 0) for c in t.center(24))+'\n')
    o.write(f'R_{n}={i+1}\n')
o.write('.segment "MATH"\n')
# ---- missions: one 13-byte step per nav point
EMPTY,FIGHT,LAND,AMBUSH,DEFEND=0,1,2,3,4
BIG=0x40   # with HOLD: a target twice the size
LANCER0=4  # freighter-byte bit: slot 0 is a lancer
ECHO=0x20  # freighter-byte bit: slot 0 is the echo drone
GUARD=5
PROTECT=0x10  # freighter byte on a defend step: the wreck takes the hits
WRECK=(-700,60,600)
HOLD=0x80   # freighter bits: the targets stay put, standing on end
import os
LIMIT_M2=int(os.environ.get('LIMIT_M2','11'))
# limit: a fight's time limit in units of 256 frames (about 4.3 s); survivors escape
def step(kind,x,y,z,arrive,clear,count=0,hp=0,fre=0,wait=0,limit=0):
    return (kind,x&255,(x>>8)&255,y&255,(y>>8)&255,z&255,(z>>8)&255,arrive,clear,count,hp,fre,wait,limit)
RN={n:i+1 for i,(n,_) in enumerate(RL)}
VIGIL=(0,0,-500)
# Keep every point within about 1300 units of the Vigil on each axis: the world
# wraps at 4096, so anything past 2048 shows the carrier ahead of you.
MISSIONS=[
 [step(EMPTY,700,0,700,RN['NAV'],0,wait=150),
  step(FIGHT,-500,120,800,RN['TALLY'],RN['CLEAR'],count=2,hp=4),
  step(EMPTY,-1100,0,-200,RN['STATIC'],RN['HOME'],wait=240),
  step(LAND,*VIGIL,RN['LAND'],0)],
 [step(EMPTY,-800,100,500,RN['EMPTY'],0,wait=120),
  step(FIGHT,600,-80,700,RN['TALLY'],RN['CLEAR'],count=2,hp=4),
  step(FIGHT,1000,60,-300,RN['CONVOY'],RN['CARGO'],count=2,hp=6,fre=3,limit=LIMIT_M2),
  step(EMPTY,300,0,-1300,RN['VOICE'],RN['HOME'],wait=240),
  step(LAND,*VIGIL,RN['LAND'],0)],
 [step(FIGHT,600,150,700,RN['BUOY'],RN['BUOYDEAD'],count=1,hp=5,fre=HOLD|1),
  step(AMBUSH,600,150,700,RN['TBEHIND'],RN['TNEXT'],count=2,hp=4,wait=200),
  step(FIGHT,-800,-80,400,RN['TALLY'],RN['CLEAR'],count=2,hp=4),
  step(LAND,*VIGIL,RN['LAND'],0)],
 # 4A Open Channel: escort Ren's shuttle; no Hale
 [step(DEFEND,-900,60,600,RN['R_LISTEN'],RN['R_OURS'],count=2,hp=3,wait=RN['R_MORE'],limit=21),
  step(AMBUSH,-900,60,600,RN['R_BEHIND'],RN['R_HOME'],count=2,hp=3,wait=150),
  step(LAND,*VIGIL,RN['LAND'],0)],
 # 4B Standing Orders: Mara on your wing, the first lancer, the listening post
 [step(EMPTY,-700,80,500,RN['M_QUIET'],0,wait=150),
  step(FIGHT,500,-60,700,RN['M_LANCER'],RN['M_POSTAHEAD'],count=2,hp=4,fre=LANCER0),
  step(FIGHT,1000,100,-200,RN['M_POST'],RN['T_FLY'],count=1,hp=12,fre=HOLD|BIG|1),
  step(LAND,*VIGIL,RN['LAND'],0)],
 # 5 Old Habits: a scout on Hale's tail, then the echo drone
 [step(FIGHT,700,0,600,RN['TALLY'],RN['CLEAR'],count=2,hp=4),
  step(GUARD,-300,100,900,RN['ONTAIL'],RN['CLEAR'],count=2,hp=4,limit=7),
  step(FIGHT,-900,40,200,RN['ECHOCONTACT'],RN['HOME'],count=1,hp=6,fre=ECHO),
  step(LAND,*VIGIL,RN['LAND'],0)],
 # 6 The Ledger: the Ashgrove wreck; defend it two minutes while Ren copies
 [step(EMPTY,*WRECK,RN['ASH'],0,wait=90),
  step(DEFEND,*WRECK,RN['DOCKED'],RN['YOURFILE'],count=2,hp=4,wait=RN['ORDERS'],fre=PROTECT|LANCER0,limit=26),
  step(LAND,*VIGIL,RN['LAND'],0)]]
MODES=[0,0,0,0,2,1,0,0]
o.write('wreckTab: .byte 0,0,0,0,0,0,0,1\n')
o.write(f'WRECK_X={WRECK[0]}\nWRECK_Y={WRECK[1]}\nWRECK_Z={WRECK[2]}\n')   # 0 Hale on your wing, 1 Mara, 2 Ren's shuttle and no wingman
o.write('missionModeTab: .byte '+','.join(map(str,MODES))+'\n')
# per-mode versions of the wingman's lines; 0 = nobody says it
MARA={'ASH':'M_ASH','ECHOSTOP':'M_ECHOSTOP','HIGHLEFT':'M_HIGHLEFT','THATSHER':'M_THATSHER','ECHODN':'M_ECHODN','ECHOCONTACT':'M_ECHOCONTACT','THANKS':None,'HURRY':None,'HALEHIT':None,'ONTAIL':None,'WING':'M_WING','GOOD':'M_GOOD','ONE':'M_ONE','BOLT':'M_BOLT','HALE_HIT':'M_HIT','LOW':'M_LOW','HOME':'M_HOME','LANCERDN':'M_LANCERDN','HAULER':None,'TALLY':None,'CLEAR':None}
REN={'WING':'R_WING','GOOD':None,'ONE':'R_ONE','BOLT':'R_BOLT','HALE_HIT':None,'LOW':'R_LOW','HOME':'R_HOME','LANCERDN':None,'HAULER':None,'TALLY':None,'CLEAR':None}
# alone, after Hale is lost: his lines go quiet; the Vigil speaks for a few
SOLO={n:None for n,t in RL if t.startswith('HALE:')}
SOLO.update({'ASH':'M_ASH','ECHOSTOP':'V_ECHOSTOP','THATSHER':'V_THATSHER','HOME':'V_HOME'})
for nm,mp in (('radioMara',MARA),('radioRen',REN),('radioSolo',SOLO)):
    row=[0]+[ (RN[mp[n]] if mp[n] else 0) if n in mp else i+1 for i,(n,_) in enumerate(RL)]
    tab(nm,row)
for m in MISSIONS:
    for s in m:
        x=s[1]|s[2]<<8; y=s[3]|s[4]<<8; z=s[5]|s[6]<<8
        sx=lambda v:v-65536 if v>32767 else v
        assert all(abs(sx(a)-b)<=1400 for a,b in zip((x,y,z),VIGIL)),s
for i,m in enumerate(MISSIONS):
    o.write(f'mission{i+1}:\n')
    for s in m: o.write('  .byte '+','.join(str(v) for v in s)+'\n')
o.write('missionLo: .byte 0,'+','.join(f'<mission{i+1}' for i in range(len(MISSIONS)))+'\n')
o.write('missionHi: .byte 0,'+','.join(f'>mission{i+1}' for i in range(len(MISSIONS)))+'\n')
o.write(f'VIGIL_X={VIGIL[0]}\nVIGIL_Y={VIGIL[1]}\nVIGIL_Z={VIGIL[2]}\n')
o.write(f'NV={len(V)}\nNF={len(F)}\nNE={len(E)}\nNV2={len(V2)}\nNF2={len(F2)}\nNE2={len(E2)}\nNV3={len(V3)}\nNF3={len(F3)}\nNE3={len(E3)}\nNV4={len(V4)}\nNF4={len(F4)}\nNE4={len(E4)}\n')
VA=V+V2+V3+V4; tab('vxu',[(v[0]&255)^0x80 for v in VA]); tab('vyu',[(v[1]&255)^0x80 for v in VA]); tab('vzu',[(v[2]&255)^0x80 for v in VA])
FA=F+[[i+10 for i in f] for f in F2]+[[i+15 for i in f] for f in F3]+[[i+23 for i in f] for f in F4]; tab('fa',[f[0] for f in FA]); tab('fb',[f[1] for f in FA]); tab('fc',[f[2] for f in FA])
def unit(f):
    n=cross(sub(VA[f[1]],VA[f[0]]),sub(VA[f[2]],VA[f[0]])); l=math.sqrt(dot(n,n)); return tuple(c/l for c in n)
N=[unit(f) for f in FA]
tab('nxu',[(round(127*n[0])&255)^0x80 for n in N]); tab('nyu',[(round(127*n[1])&255)^0x80 for n in N]); tab('nzu',[(round(127*n[2])&255)^0x80 for n in N])
tab('fd',[round(dot(n,VA[f[0]])) for n,f in zip(N,FA)])
ks=list(E); ks2=list(E2); ks3=list(E3); ks4=list(E4); tab('ea',[k[0] for k in ks]+[k[0]+10 for k in ks2]+[k[0]+15 for k in ks3]+[k[0]+23 for k in ks4]); tab('eb',[k[1] for k in ks]+[k[1]+10 for k in ks2]+[k[1]+15 for k in ks3]+[k[1]+23 for k in ks4]); tab('emask',[E[k] for k in ks]+[E2[k] for k in ks2]+[E3[k] for k in ks3]+[E4[k] for k in ks4])
print('radius',round(R,1),'verts',V)

# ---- line-drawing blocks: one code block per pixel row of a tile ----
L=[]
def e(s): L.append(s)
def plot(r):
    e(f"  lda PAT+{r}*256,y"); e("  ora mask"); e(f"  sta PAT+{r}*256,y")
for up in (0,1):
    n='U' if up else 'D'
    for r in range(8):
        nr=(r-1)&7 if up else (r+1)&7
        wrap=(r==0) if up else (r==7)
        stepT='tileUp' if up else 'tileDown'
        e(f"X{n}{r}:")
        e("  lda err"); e("  sec")
        e(f"X{n}{r}_in:"); e("  dex"); e(f"  beq X{n}{r}_seg")
        e("  sbc dy"); e(f"  bcs X{n}{r}_in")
        e("  adc dx"); e("  sta err")
        e("  stx u0"); e("  lda segL"); e("  sec"); e("  sbc u0"); e("  tax")
        e("  lda maskR,x"); e("  stx u1"); e("  ldx segS"); e("  and maskL,x")
        e(f"  ora PAT+{r}*256,y"); e(f"  sta PAT+{r}*256,y")
        e("  ldx u1"); e("  inx"); e("  stx segS"); e("  ldx u0")
        if wrap: e(f"  jsr {stepT}")
        e(f"  jmp X{n}{nr}")
        e(f"X{n}{r}_seg:")
        e("  sbc dy"); e("  sta err")
        e("  ldx segL"); e("  lda maskR,x"); e("  ldx segS"); e("  and maskL,x")
        e(f"  ora PAT+{r}*256,y"); e(f"  sta PAT+{r}*256,y")
        e("  lda cnt"); e(f"  bne X{n}{r}_more"); e("  rts")
        e(f"X{n}{r}_more:")
        e("  lda #0"); e("  sta segS")
        e(f"  bcs X{n}{r}_same")
        e("  lda err"); e("  adc dx"); e("  sta err")
        if wrap:
            e("  inc mptr"); e(f"  jsr {stepT}")
        else:
            e("  jsr tileRight")
        e("  jsr segInit"); e(f"  jmp X{n}{nr}")
        e(f"X{n}{r}_same:")
        e("  jsr tileRight"); e("  jsr segInit"); e(f"  jmp X{n}{r}")
for rt in (0,1):
    n='R' if rt else 'L'
    for r in range(8):
        e(f"Y{n}{r}:")
        plot(r)
        e("  dex"); e(f"  beq Y{n}_done{r}")
        e("  lda err"); e("  sec"); e("  sbc dx"); e("  sta err")
        e(f"  bcs Y{n}{r}_next")
        e("  adc dy"); e("  sta err")
        if rt:
            e("  lsr mask"); e(f"  bcc Y{n}{r}_next"); e("  ror mask"); e("  jsr tileRight")
        else:
            e("  asl mask"); e(f"  bcc Y{n}{r}_next"); e("  rol mask"); e("  jsr tileLeft")
        e(f"Y{n}{r}_next:")
        if r==7:
            e("  jsr tileDown"); e(f"  jmp Y{n}0")
        else:
            e(f"  jmp Y{n}{r+1}")
        e(f"Y{n}_done{r}:"); e("  rts")
for t,pre in (('xdTab','XD'),('xuTab','XU'),('yrTab','YR'),('ylTab','YL')):
    e(f"{t}: .word "+','.join(f"{pre}{r}" for r in range(8)))
e("maskL: .byte "+','.join(str(0xFF>>i) for i in range(8)))
e("maskR: .byte "+','.join(str((0xFF00>>(i+1))&255) for i in range(8)))
open('lines.inc','w').write('\n'.join(L)+'\n')
