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
OP=dict(END=0,SPEAK=1,NL=2,PAGE=3,CHOICE=4,FLAG=5,GOTO=6,FLY=7,SAVE=8,LOAD=9,IF=10,LOC=11,CLEAR=12,TITLE=13,CHAPTER=14,LOST=15,SPARE=16,FLAG2=17,IF2=18)
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
FL_CORVANE,FL_GHOST,FL_ESCAPED,FL_CONVOY,FL_TRUST_REN=1,2,4,8,16
# second flags byte
F2_REN_HURT,F2_REN_ALONE,F2_HALE_LOST,F2_SPARED,F2_REN_OK,F2_VOSS_PLAN=1,2,4,8,16,32
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
label('m2Lose'); b(OP['FLAG'],FL_CONVOY)
label('m2Lost'); b(OP['LOST']); loc(1)
say('HALE_S','We lost the convoy, Six. We are on half rations by the end of the week.')
say('VOSS_H','No parts either. Strip one of the spare fighters to keep the rest flying.')
say('VOSS_H','Then we will be hungry. Dismissed.')
goto('m2Ren')
label('m2Win')
b(OP['IF'],FL_CONVOY); o.write('  .word m2Lost\n')
loc(1)
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
label('sceneM3'); loc(0)
say('VOSS','An old Concord relay buoy at Kesh woke up last night.')
say('VOSS_H','It is broadcasting on Lantern codes. Anyone listening could trace it to us.')
say('REN','Sir, if the voice is using that buoy, I could learn where it comes from.')
say('VOSS_H','So could the Serrath. Six, Hale: kill the buoy.')
say('HALE','Shooting the mailbox. Easiest job all month.')
b(OP['IF'],FL_CONVOY); o.write('  .word m3Short\n'); goto('m3Go')
label('m3Short')
say('VOSS_H','And fly carefully. We cannot spare another fighter.')
label('m3Go')
b(OP['FLY'],3); o.write('  .word m3Win,m3Lose\n')
label('m3Lose'); b(OP['LOST']); loc(1)
say('HALE_S','I took a hit covering you. If she had not warned us, we would both be gone.')
say('HALE_S','Commander, that voice saved our lives out there.')
goto('m3Deck')
label('m3Win'); loc(1)
say('HALE','Six, it knew. It called the ambush before they jumped us.')
say('HALE','Whoever that is, she saved our necks.')
label('m3Deck')
say('VOSS_H','Or she set the trap and wanted us grateful.')
say('VOSS_H','Okafor, shut that channel down and purge every recording. That is an order.')
say('REN','... Yes, sir.')
loc(2)
say('MARA','So she called the ambush. Teller always did see trouble coming.')
say('MARA_H','She always broke high and left, too. I told her it would get her killed one day.')
say('MARA','If that is really her, I want to hear it from her. Not from a buoy.')
say('REN','Six. They purged the recordings, but I kept a copy.')
say('REN','There is an old listening post at Hollow Reach. From there I could hear her clearly.')
say('REN','I cannot fly the shuttle out there alone.')
ask('REN','Will you take me?','I will fly you.','m3Ren','Voss should know.','m3Voss')
label('m3Ren'); b(OP['FLAG'],FL_TRUST_REN)
say('REN','Thank you. Before first watch, then. Tell no one, not even Hale.')
goto('m3Save')
label('m3Voss')
say('REN','Then go and tell him. I thought you of all people would want to know.')
label('m3Save')
b(OP['SAVE'],2)
label('m4Start')
b(OP['IF'],FL_TRUST_REN); o.write('  .word scene4A\n'); goto('scene4B')

# ---- Mission 4A: Open Channel
label('scene4A'); loc(1)
say('REN','The shuttle is fueled. Hale is covering the deck log for us.')
say('HALE','You two owe me. If Voss asks, I never saw a thing.')
say('HALE_S','Bring her back in one piece, Six.')
say('REN','Keep them off the shuttle. I will do the listening.')
b(OP['FLY'],4); o.write('  .word m4aWin,m4aLose\n')
label('m4aLose'); b(OP['FLAG2'],F2_REN_HURT); loc(1)
say('REN','Six... my head. It was like all of them were shouting at once.')
say('HALE_S','She is bleeding, Six. Get her to the medbay.')
goto('m4aAfter')
label('m4aWin'); loc(1)
say('REN','Dozens of them, Six. All Lanterns. All still talking.')
say('REN','And the signal gets louder whenever you fly. As if it knows you.')
label('m4aAfter')
say('VOSS_H','Okafor. You took a shuttle without orders.')
say('VOSS_H','You are grounded until I say otherwise.')
say('VOSS','Six. We will talk about this later.')
loc(2)
say('MARA_H','You flew her out there to listen to ghosts.')
say('MARA_H','Do not sit here, Six. Not tonight.')
goto('m4Save')

# ---- Mission 4B: Standing Orders
label('scene4B'); loc(0)
say('VOSS','Okafor wants to use the old listening post at Hollow Reach.')
say('VOSS_H','I want it gone before she can. Thank you for telling me, Six.')
say('VOSS_H','Dray flies your wing. Hale stays aboard.')
say('MARA','Good. Let us end this.')
b(OP['FLY'],5); o.write('  .word m4bWin,m4bLose\n')
label('m4bLose'); b(OP['LOST']); loc(1)
say('MARA_H','The post is still standing, and we lost a fighter getting nowhere.')
say('VOSS_H','Then I send another team. Another fighter we cannot spare.')
goto('m4bBar')
label('m4bWin'); loc(1)
say('MARA','It is done. Six... at the end, did you hear something?')
say('MARA_H','No. Forget I asked.')
label('m4bBar'); loc(2)
say('MARA','You were never the same after Corvane, you know.')
say('MARA','None of us were. But you most of all.')
say('REN','I have nothing to say to you, Six.')
b(OP['FLAG2'],F2_REN_ALONE)

label('m4Save')
b(OP['SAVE'],3)

# ---- Mission 5: Old Habits
label('sceneM5'); loc(0)
say('VOSS','A Serrath picket has found our hiding place. If it reports, we are finished.')
say('VOSS_H','Every fighter flies. Nobody comes home until that picket is silent.')
b(OP['IF'],FL_TRUST_REN); o.write('  .word m5Grounded\n'); goto('m5Hale')
label('m5Grounded')
say('VOSS_H','Okafor stays grounded. Keep the channel clean.')
label('m5Hale')
say('HALE','Every fighter means me too, Six. Try to keep up.')
b(OP['FLY'],6); o.write('  .word m5Win,m5Lose\n')
label('m5Lose'); b(OP['LOST']); loc(1)
say('VOSS_H','The picket got a signal out. We move tonight, and we lose another fighter doing it.')
goto('m5After')
label('m5Win'); loc(1)
say('VOSS','The picket is silent. Good work, Six.')
label('m5After')
b(OP['IF2'],F2_HALE_LOST); o.write('  .word m5Lost\n')
say('HALE_S','That one on my tail... I owe you, Six.')
say('HALE','If I do not make it home, tell my daughter about me. Promise me.')
goto('m5Bar')
label('m5Lost')
say('MARA_H','Hale is gone, Six. I heard it on the open channel.')
say('VOSS','Tomas Hale. Log it.')
label('m5Bar'); loc(2)
b(OP['IF2'],F2_HALE_LOST); o.write('  .word m5Seat\n'); goto('m5Mara')
label('m5Seat')
say('MARA','His seat stays empty. Nobody sits there.')
say('MARA_H','From now on, I fly your wing.')
label('m5Mara')
say('MARA','That last ship broke high and left.')
say('MARA_H','Teller always did that. Always. I told her it would get her killed.')
b(OP['IF2'],F2_REN_ALONE); o.write('  .word m5Apology\n')
b(OP['IF2'],F2_REN_HURT); o.write('  .word m5Hurt\n')
say('REN','It flew like her, Six. I heard it in the static too.')
goto('m5Save')
label('m5Hurt')
say('REN','My head still rings, Six. But I can hear them more clearly now.')
goto('m5Save')
label('m5Apology')
say('REN','...')
ask('LOG','You find Ren alone.','I am sorry.','m5Sorry','Say nothing.','m5Save')
label('m5Sorry'); b(OP['FLAG2'],F2_REN_OK)
say('REN','Thank you. I have not slept, Six. The voices are louder every night.')
label('m5Save')
b(OP['SAVE'],4)

# ---- Mission 6: The Ledger
label('sceneM6'); loc(0)
say('VOSS','Long range picked up a wreck drifting into the Vey lane. Concord registry: Ashgrove.')
say('REN','Ashgrove was an intelligence ship, Commander. She was at Corvane.')
say('VOSS_H','... Then we strip her before the Serrath do. Okafor, you are cleared to fly.')
say('MARA_H','Intelligence ships keep records. Of everything.')
b(OP['IF2'],F2_HALE_LOST); o.write('  .word m6Mara\n')
say('HALE','Reading someone else\'s mail. My favourite.')
goto('m6Go')
label('m6Mara')
say('MARA','I am on your wing, Six. Stay close to the wreck.')
label('m6Go')
b(OP['FLY'],7); o.write('  .word m6Win,m6Lose\n')
label('m6Lose'); b(OP['LOST']); b(OP['LOST']); loc(1)
say('REN','We got out with part of it. Enough. But we left two fighters behind.')
goto('m6Truth')
label('m6Win'); loc(1)
say('REN','I have it. All of it.')
label('m6Truth'); loc(0)
say('REN','These are the Ashgrove\'s orders. Concord intelligence found the Array before the battle.')
say('REN','Commander Voss was part of that operation.')
say('MARA_H','And the retreat? Who ordered it?')
say('REN','He did. To keep the Serrath from studying the Array. Half the Lanterns were still out there.')
say('MARA_H','Teller was still out there.')
say('VOSS','... Yes.')
say('REN','There is more. Six, your file.')
say('LOG','LANTERN SIX: KILLED IN ACTION, CORVANE.')
say('LOG','LANTERN SIX: RECOVERED. NINE MINUTES LATER.')
say('REN','Nine minutes, Six. You were dead for nine minutes.')
b(OP['IF2'],F2_HALE_LOST); o.write('  .word m6Plans\n')
say('HALE_S','That is not possible. Six, tell me that is not possible.')
label('m6Plans')
say('VOSS_H','I will not defend what I did. But we have to decide what happens now.')
say('VOSS','We send these records to Concord command, and hold the Vigil until they come.')
say('REN','Or we go to Corvane ourselves, before the Serrath finish what they started.')
ask('LOG','Whose plan do you back?','Voss: tell Concord.','m6Voss','Ren: go to Corvane.','m6Ren')
label('m6Voss'); b(OP['FLAG2'],F2_VOSS_PLAN)
say('VOSS','Thank you, Six. Whatever they do to me.')
goto('m6Save')
label('m6Ren')
say('REN','Then we go quietly. I know a way past the picket.')
label('m6Save')
b(OP['SAVE'],5)
label('m6End')
loc(3)
say('LOG','End of Mission 6. Progress saved.')
b(OP['SPEAK'],P['LOG']); o.write('  .byte "Spare fighters: "\n'); b(OP['SPARE']); o.write(f'  .byte ".",{OP["NL"]}\n'); b(OP['PAGE'])
say('LOG','The story continues in Mission 7, which is not built yet.')
b(OP['TITLE'])
o.write('.segment "MATH"\nsaveTab:\n  .byte 4\n  .word sceneM2\n  .byte 4\n  .word sceneM3\n  .byte 4\n  .word m4Start\n  .byte 4\n  .word sceneM5\n  .byte 4\n  .word sceneM6\n  .byte 4\n  .word m6End\n')
print('script chars',total)
