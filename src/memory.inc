.segment "ZEROPAGE"
mat:     .res 8*10       ; M (9 elements) + projection; 4 pointers each
row:     .res 8*3        ; three scratch multipliers
tmpM:    .res 8
t0:      .res 1
t1:      .res 1
u0:      .res 1
u1:      .res 1
ptr:     .res 2
mptr:    .res 2
lpLo:    .res 2
lpHi:    .res 2
dx:      .res 1
dy:      .res 1
err:     .res 1
cnt:     .res 1
mask:    .res 1
segS:    .res 1
segL:    .res 1
saveX:   .res 1
vecM:    .res 2
x0:      .res 1
y0:      .res 1
x1:      .res 1
y1:      .res 1
nextTile:.res 1
vis:     .res 1
yaw:     .res 1
pitch:   .res 1
pitchAcc:.res 1
lastNmi: .res 1
ship:    .res 1
ca:      .res 1
sa:      .res 1
cb:      .res 1
sb:      .res 1
pad:     .res 1
camYaw:  .res 1
camPitch:.res 1
speed:   .res 1
dt:      .res 1
relX:    .res 2
relY:    .res 2
relZ:    .res 2
px:      .res 2
py:      .res 2
pz:      .res 2
outX:    .res 2
outY:    .res 2
ax:      .res 2
ay:      .res 2
bx:      .res 2
by:      .res 2
ix:      .res 2
iy:      .res 2
mx:      .res 2
my:      .res 2
oa:      .res 1
ob:      .res 1
clipBit: .res 1
rk:      .res 1
offscreen:.res 1
hx:      .res 1
hl:      .res 1
guard:   .res 1
vi:      .res 1
vj:      .res 1
hxV:     .res 1
hyV:     .res 1
hxA:     .res 1
lxA:     .res 1
hxB:     .res 1
lxB:     .res 1
hxC:     .res 1
lxC:     .res 1
sinceFlip:.res 1
sndPtr:  .res 6          ; current step of each effect channel
textP:   .res 2
storyFlag:.res 1
tick:    .res 1
qJob:    .res 1
qLen:    .res 1
qAddr:   .res 2
sTargA:  .res 2
sTargB:  .res 2
spad0:   .res 1
spadP:   .res 2
musP:    .res 2
vFirst:  .res 1
vEnd:    .res 1
fFirst:  .res 1
fLast:   .res 1
eFirst:  .res 1
eLast:   .res 1
drawIdx: .res 1          ; list being drawn (0..2)
prevIdx: .res 1
prevCnt: .res 1
; shared with the interrupt side
drawDone:.res 1
bufFree: .res 1
jobIdx:  .res 1
jobCnt:  .res 1
; interrupt-only
front:   .res 1
xfState: .res 1
xfI:     .res 1
xfCnt:   .res 1
xfList:  .res 1
oldList: .res 1
oldCnt:  .res 1
bud:     .res 2
xn:      .res 1
ntOr:    .res 1
planeOr: .res 1
vec:     .res 2

OAM = $0200
; statistics, read by the test harness

M_PRJ=72
NEAR=128
.define PL(i) mat+i
.define QL(i) mat+i+2
.define PH(i) mat+i+4
.define QH(i) mat+i+6

; ---- statistics at fixed addresses, read by the test harness and the web page
.segment "STATS"
statFrames: .res 2
statNmi: .res 2
statTiles: .res 1
statPeak: .res 1
statYaw: .res 1
statPitch: .res 1
statVis: .res 1
statStage: .res 1
statSlow: .res 1
statNear: .res 1
statClip: .res 1
statAlive: .res 1
statScore: .res 1
statShields: .res 1
statHits: .res 1
statCrash: .res 1
statShots: .res 1
statBoltHits: .res 1
statSnd: .res 7
storyFlags: .res 1
killsLeft: .res 1
missionId: .res 1
gameMode: .res 1
statPad: .res 1
statDbg: .res 16
statHunter: .res 8
.assert statHunter+8 <= $0340, error, "stats block overflow"

; ---- all other RAM: the linker places these, so nothing can overlap
.segment "BSS"
alive: .res 8
hp: .res 8
respawn: .res 8
markerT: .res 1
scriptBank: .res 1
hYaw: .res 4
hPitch: .res 4
hCool: .res 4
hBreak: .res 4
boltLife: .res 1
curV: .res 3
fv: .res 3
bv: .res 3
hSaveL: .res 48
hSaveH: .res 48
fvX: .res 4
fvY: .res 4
fvZ: .res 4
aiTX: .res 2
aiTY: .res 2
aiTZ: .res 2
aiCount: .res 1
aiNext: .res 1
aiStep: .res 1
ei: .res 1
mTick: .res 2
mStage: .res 1
mPhase: .res 1
mWait: .res 2
mStepL: .res 1
mStepH: .res 1
mShift: .res 1
marginL: .res 1
marginH: .res 1
shipModel: .res 4
hPassive: .res 4
radioReq: .res 1
radioT: .res 1
radioPage: .res 1
radioOff: .res 1
eR: .res 1
eU: .res 1
eF: .res 1
dDx: .res 1
dDy: .res 1
dDz: .res 1
advI: .res 1
advS: .res 1
sasb: .res 1
sacb: .res 1
casb: .res 1
cacb: .res 1
hTurn: .res 4
sndReq: .res 3
sndPrio: .res 3
sndHi: .res 2
sndEng: .res 1
padNew: .res 1
startLatch: .res 1
overT: .res 1
musReq: .res 1
musWait: .res 1
musLen: .res 1
confirmLatch: .res 1
qBuf: .res 32
doneWin: .res 1
nextMode: .res 1
winPtr: .res 2
losePtr: .res 2
resumePtr: .res 2
sCol: .res 1
sRow: .res 1
sPort: .res 1
sSel: .res 1
sCnt: .res 1
shipPosL: .res 48
shipPosH: .res 48
shipPosF: .res 48
sparkX: .res 8
sparkY: .res 8
firing: .res 1
cooldown: .res 1
laserT: .res 1
crossT: .res 1
shields: .res 1
score: .res 1
bestZ: .res 2
hitShip: .res 1
sparkT: .res 1
bumpT: .res 1
hitX: .res 6
eyeV: .res 6
camSave: .res 9
camPos: .res 9
Vv: .res 9
Rb: .res 9
Mv: .res 9
Eb: .res 3
camXL: .res 24
camXH: .res 24
camYL: .res 24
camYH: .res 24
camZL: .res 24
camZH: .res 24
scrXL: .res 24
scrXH: .res 24
scrYL: .res 24
scrYH: .res 24
behind: .res 24
f3: .res 6
b3: .res 6
curShown: .res 1
curCnt: .res 1
u1hold: .res 1
xnt: .res 1
xnh: .res 1

; ---- cartridge RAM. These tables need fixed pages; the linker reserves them.
.segment "WRAM"
wramTables: .res $1200
.assert wramTables = PAT, error, "tile tables must start at PAT"
.segment "SAVE"
saveArea: .res 6

