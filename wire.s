; Wireframe space-combat prototype for the NES (MMC3, 8K WRAM, 8K CHR RAM).
; Letterboxed 192x128 view, tiles allocated per frame, bitplane double-buffer.
.ifndef NSHIPS
NSHIPS = 4
.endif
MODE_TITLE = 0
MODE_PLAY  = 1
MODE_OVER  = 2
MODE_STORY = 3
MODE_DONE  = 4
.ifndef SCOUT_HP
SCOUT_HP = 4
.endif
.ifndef MISSION_SHIELDS
MISSION_SHIELDS = 5
.endif
HALE = 2                ; ship index of the wingman in missions
RADIO_TILE = 223        ; the radio font; tile 255 is the scratch tile
MAXSPEED = 20
HIT_R    = 80            ; laser hit radius around a ship
HIT_R2   = 3200          ; HIT_R squared / 2
CRASH_R  = 130           ; closest approach to a ship's centre
CRASH_R2 = 8450          ; CRASH_R squared / 2
SHIP_HP  = 2
.ifndef START_SHIELDS
START_SHIELDS = 3
.endif
RESPAWN  = 200           ; game frames
FIRE_GAP = 6             ; game frames between shots
H_SPEED  = 4             ; hunter speed per vblank
H_BREAK_SPEED = 6
BOLT_SPEED = 30          ; bolt speed per vblank
BOLT_LIFE  = 45          ; game frames
H_CLOSE  = 16            ; break away inside about 512 units
H_RELOAD = 45            ; game frames between enemy shots
.ifndef START_SPEED
START_SPEED = 2
.endif
PPUCTRL=$2000
PPUMASK=$2001
PPUSTATUS=$2002
PPUSCROLL=$2005
PPUADDR=$2006
PPUDATA=$2007
OAMDMA=$4014

PATPAGE = $60            ; $6000-$67FF: 8 row-arrays of 256 bytes
PAT     = $6000          ; row r of tile t lives at PAT + r*256 + t
MAP     = $6800          ; $6800-$6BFF: tile for each nametable cell
LISTS   = $6C            ; 3 position lists; each is lo page, hi page
CTRLV   = %10001000      ; NMI on, BG patterns $0000, sprites $1000
ST_IDLE=0
ST_PAT=1
ST_NEW=2
ST_FLIP=3
ST_CLEAR=4
; Tuned against Mesen so rendering starts mid-scanline on line 23.
.ifndef BUD_B
BUD_B = 4485
.endif
BUD_A = 8100
IRQ_LINES = 134         ; picture plus the radio line
.ifndef K_NT
K_NT=268
.endif
.ifndef K_PAT
K_PAT=368
.endif
.ifndef K_TAKE
K_TAKE=45
.endif
.ifndef K_NEXT
K_NEXT=125
.endif

.macro SFXREQ id
  ldx #id
  jsr play_sfx
.endmacro

.macro MARK n
.ifdef DEBUG
  lda #n
  sta $4100
  sta statStage
.endif
.endmacro

.segment "HEADER"
  .byte "NES",$1A, 16, 0, $43, $00, 1, 0,0,0,0,0,0,0

.include "src/memory.inc"
.include "tables.inc"
.include "story.inc"

.include "src/flight_math.inc"
.include "src/flight_game.inc"
.include "src/mission.inc"
.include "src/draw.inc"
.include "src/ai.inc"
.include "src/mainloop.inc"
.include "src/story_engine.inc"
.include "src/xfer.inc"
.include "src/sound.inc"
.include "src/boot.inc"
