; ---------- game state ----------

; Title and game-over screens: the scene keeps turning, nothing else moves.
.proc attract
  lda dt
  clc
  adc yaw
  sta yaw
  lda dt
  clc
  adc pitchAcc
  sta pitchAcc
  lsr a
  sta pitch
  lda gameMode
  cmp #MODE_DONE
  beq done
  cmp #MODE_TITLE
  bne over
  inc hYaw             ; show off the hunter on the title
  jsr pressed_start
  bcs :+
  jmp view
:
  jsr pick_start
  jmp story_enter
over:
  lda overT
  sec
  sbc dt
  bcs :+
  lda #0
:
  sta overT
  beq :+
  lda #0
  sta startLatch       ; a press during the pause does not count
  jmp view
:
  jsr pressed_start
  bcc view
  jsr go_title
view:
  jmp build_view
done:
  ; hold the result on screen, then on to the debrief
  lda overT
  sec
  sbc dt
  bcs :+
  lda #0
:
  sta overT
  bne view
  ldx doneWin
  beq lost
  lda winPtr
  sta resumePtr
  lda winPtr+1
  sta resumePtr+1
  jmp story_enter
lost:
  lda losePtr
  sta resumePtr
  lda losePtr+1
  sta resumePtr+1
  jmp story_enter
.endproc

; Carry set if Start has been pressed since the last call.
pressed_start:
  lda startLatch
  cmp #1
  lda #0
  sta startLatch
  rts

go_title:
  jsr new_game
  lda #MODE_TITLE
  sta gameMode
  lda #1
  sta musReq
  rts

lose_shields:
  lda missionId
  beq game_over
  lda #0
  sta shields
  jmp end_mission

game_over:
  lda #0
  sta shields
  sta speed
  sta alive+4
  lda #MODE_OVER
  sta gameMode
  lda #120             ; two seconds
  sta overT
  lda #2
  sta musReq
  rts

; pad bits: A B Sel Start Up Down Left Right
.proc update_player
  ; dt = vblanks since the last frame
  lda statNmi
  sec
  sbc lastNmi
  sta dt
  lda statNmi
  sta lastNmi
  lda gameMode
  cmp #MODE_PLAY
  beq :+
  jmp attract
:
  ; B held: up and down set speed instead of pitch
  lda pad
  and #$40
  beq steer
  lda pad
  and #$08
  beq :+
  lda speed
  cmp #MAXSPEED
  bcs :+
  inc speed
:
  lda pad
  and #$04
  beq :+
  lda speed
  beq :+
  dec speed
:
  jmp turn
steer:
  lda pad
  and #$08
  beq :+
  lda camPitch
  clc
  adc dt
  sta camPitch
:
  lda pad
  and #$04
  beq :+
  lda camPitch
  sec
  sbc dt
  sta camPitch
:
turn:
  lda pad
  and #$01
  beq :+
  lda camYaw
  clc
  adc dt
  sta camYaw
:
  lda pad
  and #$02
  beq :+
  lda camYaw
  sec
  sbc dt
  sta camYaw
:
  ; ship tumble runs on time, not frames
  lda dt
  clc
  adc yaw
  sta yaw
  lda dt
  clc
  adc pitchAcc
  sta pitchAcc
  lsr a
  sta pitch
  jsr build_view
  ldx #8
: lda camPos,x
  sta camSave,x
  dex
  bpl :-
  ; move along the view direction by speed*dt
  ldx dt
  cpx #5
  bcc :+
  ldx #4
:
  lda #0
: clc
  adc speed
  dex
  bne :-
  sta hl
  ldx #0
  ldy #0
@axis:
  sty hx
  lda hl
  ldy hx
  ldx Vv+6,y
  jsr fmulFull       ; A hi, X lo: forward*step/128 in 8.8
  stx u0
  sta u1
  lda hx
  asl a
  clc
  adc hx
  tay
  lda u0
  clc
  adc camPos,y
  sta camPos,y
  lda u1
  adc camPos+1,y
  sta camPos+1,y
  lda u1
  bmi @neg
  lda camPos+2,y
  adc #0
  jmp @st
@neg:
  lda camPos+2,y
  adc #$FF
@st:
  sta camPos+2,y
  ldy hx
  iny
  cpy #3
  beq :+
  jmp @axis
:
  jsr enemy_update
  jsr mission_tick
  jmp collide
.endproc

; Adds vector curV * advS to the position of object advI.
.proc advance
  ldy #0
axis:
  sty hx
  lda advS
  ldx curV,y
  jsr fmulFull
  stx u0
  sta u1
  lda hx
  asl a
  asl a
  asl a
  asl a
  clc
  adc advI
  tay
  lda u0
  clc
  adc shipPosF,y
  sta shipPosF,y
  lda u1
  adc shipPosL,y
  sta shipPosL,y
  lda u1
  bmi neg
  lda shipPosH,y
  adc #0
  jmp st
neg:
  lda shipPosH,y
  adc #$FF
st:
  sta shipPosH,y
  ldy hx
  iny
  cpy #3
  bne axis
  rts
.endproc

; Step per frame: speed * dt, with dt capped so it stays a signed byte.
.macro SCALEDT speed
  .local ok, l
  ldx dt
  cpx #5
  bcc ok
  ldx #4
ok:
  lda #0
l:
  clc
  adc #speed
  dex
  bne l
.endmacro

; Points to the hunter when it is out of view (OAM slot 22).
.proc ship_marker
  lda ship
  cmp #HALE
  bcs done
  lda missionId
  beq done
  lda #2
  sta markerT
  ; left or right edge by sign, top or bottom only when it is mostly vertical
  lda #89
  sta OAM+88
  lda #124
  sta OAM+91
  lda relX+1
  bmi left
  lda #208
  sta OAM+91
  jmp vert
left:
  lda #40
  sta OAM+91
vert:
  lda relY+1
  bmi lower
  cmp #1
  bcc tile
  lda #40
  sta OAM+88
  jmp tile
lower:
  cmp #$FF
  bcs tile
  lda #132
  sta OAM+88
tile:
  lda #3
  sta OAM+89
  lda #2
  sta OAM+90
done:
  rts
.endproc

; Nav marker (OAM slot 25): on the point if it is in view, else at the edge.
.proc draw_nav
  lda missionId
  beq hide
  lda mPhase
  bne hide
  lda #5
  sta ship
  jsr rel_pos
  ldx #5
: lda relX,x
  sta px,x
  dex
  bpl :-
  jsr project
  bcs edge
  lda outX+1
  ora outY+1
  bne edge
  lda outX
  cmp #192
  bcs edge
  lda outY
  cmp #128
  bcs edge
  clc
  adc #20
  sta OAM+100
  lda outX
  clc
  adc #28
  sta OAM+103
  jmp show
edge:
  lda #84
  sta OAM+100
  lda relX+1
  bmi left
  lda #212
  sta OAM+103
  jmp show
left:
  lda #36
  sta OAM+103
show:
  lda #2
  sta OAM+101
  lda #0
  sta OAM+102
  rts
hide:
  lda #$FF
  sta OAM+100
  rts
.endproc

; Enemy bolt sprite (OAM slot 21).
.proc draw_bolt
  lda alive+4
  beq hide
  lda #4
  sta ship
  jsr rel_pos
  ldx #5
: lda relX,x
  sta px,x
  dex
  bpl :-
  jsr project
  bcs hide
  lda outX+1
  ora outY+1
  bne hide
  lda outX
  cmp #192
  bcs hide
  lda outY
  cmp #128
  bcs hide
  clc
  adc #20
  sta OAM+84
  lda #2
  sta OAM+85
  sta OAM+86
  lda outX
  clc
  adc #28
  sta OAM+87
  rts
hide:
  lda #$FF
  sta OAM+84
  rts
.endproc

reset_hunter:
  ldx #0
: lda shipInitL,x
  sta shipPosL,x
  lda shipInitH,x
  sta shipPosH,x
  lda #0
  sta shipPosF,x
  txa
  clc
  adc #16
  tax
  cmp #48
  bne :-
  lda #128             ; facing the start position
  sta hYaw
  lda #0
  sta hPitch
  sta hBreak
  sta hTurn
  lda #20
  sta hCool
  rts

; Adds the square of one world-axis distance to t0/t1; carry set if out of range.
.macro CDELTA cOff, wOff
  .local neg, have, miss, done
  ldx vi
  lda shipPosL+wOff,x
  sec
  sbc camPos+cOff+1
  sta u0
  lda shipPosH+wOff,x
  sbc camPos+cOff+2
  and #$0F
  cmp #$08
  bcc have
  ora #$F0
  eor #$FF
  sta u1
  lda u0
  eor #$FF
  clc
  adc #1
  sta u0
  lda u1
  adc #0
have:
  cmp #0
  bne miss
  ldy u0
  cpy #CRASH_R
  bcs miss
  lda sqrLo+256,y
  clc
  adc t0
  sta t0
  lda sqrHi+256,y
  adc t1
  sta t1
  jmp done
miss:
  jmp noHit
done:
.endmacro

; Runs before anything is drawn, so the camera can never end up inside a hull.
.proc collide
  ldx #4               ; the bolt first, then the ships
ship_:
  stx vi
  lda alive,x
  bne :+
  jmp noHit
:
  cpx #HALE
  bcc :+
  lda missionId
  beq :+
  jmp noHit            ; Hale and the Vigil never collide with you
:
  lda #0
  sta t0
  sta t1
  CDELTA 0, 0
  CDELTA 3, 16
  CDELTA 6, 32
  lda t0
  cmp #<CRASH_R2
  lda t1
  sbc #>CRASH_R2
  bcs noHit
  jmp crash
noHit:
  ldx vi
  cpx #4
  bne :+
  ldx #NSHIPS
:
  dex
  bmi :+
  jmp ship_
:
  rts
boltHit:
  lda #0
  sta alive+4
  lda #30
  sta crossT
  inc statBoltHits
  SFXREQ SFX_CRASH
  lda shields
  cmp #2
  bne :+
  lda #R_LOW
  jsr say_radio
:
  dec shields
  bne :+
  jsr lose_shields
  rts
:
  ldx #NSHIPS-1
  jmp ship_
crash:
  lda vi
  cmp #4
  beq boltHit
  bne notBolt
notBolt:
  cmp aiCount
  bcs :+
  ; an AI ship caused it as well: put it back and send it away
  tax
  jsr ai_undo_move
:
  ldx #8
: lda camSave,x
  sta camPos,x
  dex
  bpl :-
  lda #0
  sta speed
  lda bumpT
  bne done
  lda #30
  sta bumpT
  sta crossT
  inc statCrash
  SFXREQ SFX_CRASH
  dec shields
  bne done
  jsr lose_shields
done:
  rts
.endproc

; Fresh game: all ships back, full shields, back at the start.
new_game:
  ldx #47
: lda shipInitL,x
  sta shipPosL,x
  lda shipInitH,x
  sta shipPosH,x
  lda #0
  sta shipPosF,x
  dex
  bpl :-
  jsr reset_hunter
  lda #0
  sta alive+4
  ldx #8
  lda #0
: sta camPos,x
  dex
  bpl :-
  sta score
  sta camYaw
  sta camPitch
  sta sparkT
  sta laserT
  lda #START_SHIELDS
  sta shields
  lda #START_SPEED
  sta speed
  ldx #3
: lda #1
  sta alive,x
  lda #SHIP_HP
  sta hp,x
  dex
  bpl :-
  lda #1
  sta aiCount
  sta aiStep
  lda #0
  sta aiNext
  sta mTick
  sta mTick+1
  sta mStage
  sta mPhase
  ldx #3
: sta shipModel,x
  sta hPassive,x
  dex
  bpl :-
  lda missionId
  beq :+
  jsr setup_mission
:
  rts

; Ends a mission. A = 1 for a win.
end_mission:
  sta doneWin
  lda #0
  sta speed
  sta alive+4
  lda #MODE_DONE
  sta gameMode
  lda #180
  sta overT
  rts

.proc update_fire
  lda #0
  sta firing
  lda #$7F
  sta bestZ+1
  lda #$FF
  sta bestZ
  sta hitShip
  lda bumpT
  beq :+
  dec bumpT
:
  lda crossT
  beq :+
  dec crossT
:
  lda laserT
  beq :+
  dec laserT
:
  lda gameMode
  cmp #MODE_PLAY
  beq :+
  rts
:
  lda cooldown
  beq :+
  dec cooldown
  rts
:
  lda pad
  and #$80
  beq :+
  lda #1
  sta firing
  lda #FIRE_GAP
  sta cooldown
  lda #3
  sta laserT
  SFXREQ SFX_LASER
:
  rts
.endproc

; Absolute value of a rel component, or carry set if it is at least HIT_R.
.macro HITAXIS v
  .local pos, have
  lda v
  sta u0
  lda v+1
  bpl pos
  lda #0
  sec
  sbc v
  sta u0
  lda #0
  sbc v+1
pos:
  bne miss
  ldy u0
  cpy #HIT_R
  bcs miss
  lda sqrLo+256,y
  clc
  adc t0
  sta t0
  lda sqrHi+256,y
  adc t1
  sta t1
.endmacro

; The shot runs straight down the view axis; the nearest ship on it takes the hit.
.proc laser_test
  lda firing
  bne :+
  rts
:
  lda relZ+1
  bpl :+
  rts
:
  lda missionId        ; in missions lasers reach about 770 units
  beq :+
  lda relZ+1
  cmp #3
  bcc :+
  rts
:
  lda ship
  cmp #HALE
  bcc :+
  lda missionId
  beq :+
  rts
:
  lda #0
  sta t0
  sta t1
  HITAXIS relX
  HITAXIS relY
  lda t0
  cmp #<HIT_R2
  lda t1
  sbc #>HIT_R2
  bcs miss
  lda relZ
  cmp bestZ
  lda relZ+1
  sbc bestZ+1
  bcs miss
  lda relZ
  sta bestZ
  lda relZ+1
  sta bestZ+1
  lda ship
  sta hitShip
  ldx #5
: lda relX,x
  sta hitX,x
  dex
  bpl :-
miss:
  rts
.endproc

.proc apply_hits
  ldx hitShip
  bpl :+
  jmp tick
:
  lda #12
  sta crossT
  inc statHits
  dec hp,x
  beq :+
  SFXREQ SFX_HIT
  lda #R_GOOD
  jsr say_radio
  jmp tick
:
  ldx hitShip
  lda #0
  sta alive,x
  lda #RESPAWN
  sta respawn,x
  inc score
  lda missionId
  beq :+
  dec killsLeft
  beq :+
  lda #R_ONE
  jsr say_radio
:
  SFXREQ SFX_EXPLO
  ; the explosion starts where the ship was
  ldx #5
: lda hitX,x
  sta px,x
  dex
  bpl :-
  jsr project
  lda #96
  ldx #64
  bcs setS
  lda outX+1
  ora outY+1
  bne centre
  lda outX
  ldx outY
  jmp setS
centre:
  lda #96
  ldx #64
setS:
  clc
  adc #28
  sta u0
  txa
  clc
  adc #21
  ldx #7
: sta sparkY,x
  pha
  lda u0
  sta sparkX,x
  pla
  dex
  bpl :-
  lda #18
  sta sparkT
tick:
  lda missionId
  beq :+
  rts                  ; nothing comes back during a mission
:
  ldx #NSHIPS-1
r:
  lda alive,x
  bne :+
  dec respawn,x
  bne :+
  lda #1
  sta alive,x
  lda #SHIP_HP
  sta hp,x
  cpx #0
  bne :+
  jsr reset_hunter
  ldx #0
:
  dex
  bpl r
  rts
.endproc

; OAM: 0 crosshair, 1-6 laser, 7-14 explosion, 15-17 score, 18-20 shields
.proc update_sprites
  lda crossT
  beq :+
  lda #1
:
  sta OAM+2
  ; laser bolts converge on the crosshair
  ldy laserT
  ldx #0
  cpy #0
  bne on
: lda #$FF
  sta OAM+4,x
  inx
  inx
  inx
  inx
  cpx #24
  bne :-
  jmp sparks
on:
  dey
  tya
  asl a
  asl a
  asl a
  sta u0
  asl a
  clc
  adc u0
  tay
  ldx #0
: lda laserTab,y
  sta OAM+4,x
  iny
  inx
  cpx #24
  bne :-
sparks:
  ldx #0
  ldy #0
  lda sparkT
  beq hide
  dec sparkT
: lda sparkX,x
  clc
  adc sparkVX,x
  sta sparkX,x
  sta OAM+28+3,y
  lda sparkY,x
  clc
  adc sparkVY,x
  sta sparkY,x
  sta OAM+28,y
  lda #2
  sta OAM+28+1,y
  lda #2
  sta OAM+28+2,y
  iny
  iny
  iny
  iny
  inx
  cpx #8
  bne :-
  jmp hud
hide:
  lda #$FF
: sta OAM+28,y
  iny
  iny
  iny
  iny
  cpy #32
  bne :-
hud:
  ; score, three digits
  lda score
  ldx #0
: cmp #100
  bcc :+
  sbc #100
  inx
  jmp :-
:
  stx u0
  ldx #0
: cmp #10
  bcc :+
  sbc #10
  inx
  jmp :-
:
  clc
  adc #16
  sta OAM+68+1
  txa
  clc
  adc #16
  sta OAM+64+1
  lda u0
  clc
  adc #16
  sta OAM+60+1
  lda #142
  sta OAM+60
  sta OAM+64
  sta OAM+68
  lda #0
  sta OAM+62
  sta OAM+66
  sta OAM+70
  lda #38
  sta OAM+63
  lda #46
  sta OAM+67
  lda #54
  sta OAM+71
  ; shields as pips
  ldx #0
  ldy #0
: lda #$FF
  cpy shields
  bcs :+
  lda #142
:
  sta OAM+72,x
  lda #3
  sta OAM+73,x
  lda #0
  sta OAM+74,x
  tya
  asl a
  asl a
  asl a
  clc
  adc #194
  sta OAM+75,x
  inx
  inx
  inx
  inx
  iny
  cpy #3
  bne :--
  ; shields four and five use the two spare sprite slots
  ldx #0
  ldy #3
: lda #$FF
  cpy shields
  bcs :+
  lda #142
:
  sta OAM+92,x
  lda #3
  sta OAM+93,x
  lda #0
  sta OAM+94,x
  tya
  asl a
  asl a
  asl a
  clc
  adc #194
  sta OAM+95,x
  inx
  inx
  inx
  inx
  iny
  cpy #5
  bne :--
  rts
laserTab:
  ; y, tile, attr, x for six bolts, one row per animation step (latest first)
  .byte 104,2,1,76, 104,2,1,172, 96,2,1,100, 96,2,1,148, 92,2,1,112, 92,2,1,136
  .byte 124,2,1,48, 124,2,1,200, 114,2,1,64, 114,2,1,184, 104,2,1,76, 104,2,1,172
  .byte 134,2,1,36, 134,2,1,212, 128,2,1,44, 128,2,1,204, 118,2,1,56, 118,2,1,192
.endproc

fmulFull:
  stx u1
  SETM tmpM
  lda u1
  eor #$80
  tay
  PROD tmpM
  rts

.proc build_view
  ldx camYaw
  lda sinTab,x
  sta sa
  txa
  clc
  adc #64
  tax
  lda sinTab,x
  sta ca
  ldx camPitch
  lda sinTab,x
  sta sb
  txa
  clc
  adc #64
  tax
  lda sinTab,x
  sta cb
  ; right = (cy, 0, -sy)
  lda ca
  sta Vv+0
  lda #0
  sta Vv+1
  sec
  sbc sa
  sta Vv+2
  ; up = (-sy*sp, cp, -cy*sp)
  lda sa
  ldx sb
  jsr fmul
  eor #$FF
  clc
  adc #1
  sta Vv+3
  lda cb
  sta Vv+4
  lda ca
  ldx sb
  jsr fmul
  eor #$FF
  clc
  adc #1
  sta Vv+5
  ; forward = (sy*cp, sp, cy*cp)
  lda sa
  ldx cb
  jsr fmul
  sta Vv+6
  lda sb
  sta Vv+7
  lda ca
  ldx cb
  jsr fmul
  sta Vv+8
  rts
.endproc

; Ship rotation, stored biased for use as a multiplicand.
.proc build_ship_rot
  ldx ship
  cpx aiCount
  bcc :+
  lda missionId
  beq :++
:
  lda hYaw,x
  jmp :++
:
  lda yaw
  clc
  adc shipYaw,x
:
  tay
  lda sinTab,y
  sta sa
  tya
  clc
  adc #64
  tay
  lda sinTab,y
  sta ca
  cpx aiCount
  bcc :+
  lda missionId
  beq :++
:
  lda hPitch,x
  jmp :++
:
  lda pitch
  clc
  adc shipPitch,x
:
  tay
  lda sinTab,y
  sta sb
  tya
  clc
  adc #64
  tay
  lda sinTab,y
  sta cb
  lda ca
  eor #$80
  sta Rb+0
  lda sa
  ldx sb
  jsr fmul
  eor #$80
  sta Rb+1
  lda sa
  ldx cb
  jsr fmul
  eor #$80
  sta Rb+2
  lda #$80
  sta Rb+3
  lda cb
  eor #$80
  sta Rb+4
  lda #0
  sec
  sbc sb
  eor #$80
  sta Rb+5
  lda #0
  sec
  sbc sa
  eor #$80
  sta Rb+6
  lda ca
  ldx sb
  jsr fmul
  eor #$80
  sta Rb+7
  lda ca
  ldx cb
  jsr fmul
  eor #$80
  sta Rb+8
  rts
.endproc

; One component of the rotated position: 15-bit value (xh*128+xl) times the
; multiplier in `base`, added to t0/t1.
.macro MUL16 base, hiv, lov
  .local pos
  ldy hiv
  PROD base
  cmp #$80
  ror a
  sta u1
  txa
  ror a
  sta u0
  ldy lov
  PROD base
  ldx #0
  cmp #$80
  bcc pos
  dex
pos:
  clc
  adc u0
  sta u0
  txa
  adc u1
  sta u1
  lda u0
  clc
  adc t0
  sta t0
  lda u1
  adc t1
  sta t1
.endmacro

; Relative position -> split bytes, biased, for MUL16.
.macro SPLIT cOff, wOff, hv, lv
  ldx ship
  lda shipPosL+wOff,x
  sec
  sbc camPos+cOff+1
  sta u0
  and #$7F
  eor #$80
  sta lv
  lda shipPosH+wOff,x
  sbc camPos+cOff+2
  ; the world wraps every 4096 units: sign-extend from bit 11
  and #$0F
  cmp #$08
  bcc :+
  ora #$F0
:
  asl u0
  rol a
  eor #$80
  sta hv
.endmacro

.proc rel_pos
  SPLIT 0, 0, hxA, lxA
  SPLIT 3, 16, hxB, lxB
  SPLIT 6, 32, hxC, lxC
  ldy #0
  ldx #0
rloop:
  stx rk
  lda Vv,x
  SETM row
  ldx rk
  lda Vv+1,x
  SETM row+8
  ldx rk
  lda Vv+2,x
  SETM row+16
  lda #0
  sta t0
  sta t1
  MUL16 row, hxA, lxA
  MUL16 row+8, hxB, lxB
  MUL16 row+16, hxC, lxC
  lda rk
  lsr a
  tay                  ; 0,1(from 3),3(from 6) -> table below
  ldx relIdx,y
  lda t0
  sta relX,x
  lda t1
  sta relX+1,x
  lda rk
  clc
  adc #3
  tax
  cpx #9
  beq :+
  jmp rloop
:
  rts
relIdx: .byte 0,2,2,4
.endproc

; Distant ships use the low-detail model, stored after the full one.
LOD_HI = 4               ; high byte of the view depth, so 1024 units
.proc select_model
  ldx ship
  lda shipModel,x
  beq :+
  sta mShift
  lda #NV+NV2
  sta vFirst
  lda #NV+NV2+NV3
  sta vEnd
  lda #NF+NF2
  sta fFirst
  lda #NF+NF2+NF3-1
  sta fLast
  lda #NE+NE2
  sta eFirst
  lda #NE+NE2+NE3-1
  sta eLast
  rts
:
  lda #0
  sta mShift
  lda relZ+1
  bmi full
  cmp #LOD_HI
  bcc full
  lda #NV
  sta vFirst
  lda #NV+NV2
  sta vEnd
  lda #NF
  sta fFirst
  lda #NF+NF2-1
  sta fLast
  lda #NE
  sta eFirst
  lda #NE+NE2-1
  sta eLast
  rts
full:
  lda #0
  sta vFirst
  sta fFirst
  sta eFirst
  lda #NV
  sta vEnd
  lda #NF-1
  sta fLast
  lda #NE-1
  sta eLast
  rts
.endproc

; Carry clear when the ship is certainly outside the view: the view is narrower
; than 45 degrees either way, so |x| > z + 100 cannot be on screen.
.proc in_view
  ldx #2
axis:
  lda relX,x
  sta u0
  lda relX+1,x
  bpl :+
  ; negate
  lda #0
  sec
  sbc relX,x
  sta u0
  lda #0
  sbc relX+1,x
:
  sta u1
  lda relZ
  clc
  adc marginL
  sta t0
  lda relZ+1
  adc marginH
  sta t1
  lda t0
  cmp u0
  lda t1
  sbc u1
  bvc :+
  eor #$80
:
  bpl :+
  clc
  rts
:
  dex
  dex
  bpl axis
  sec
  rts
.endproc

; M = V * R. Row i of V times column j of R.
.proc build_m
  ldx #0
iloop:
  stx rk
  lda Vv,x
  SETM row
  ldx rk
  lda Vv+1,x
  SETM row+8
  ldx rk
  lda Vv+2,x
  SETM row+16
  lda #0
  sta vi
jloop:
  ldx vi
  ldy Rb,x
  lda (row),y
  sec
  sbc (row+2),y
  tax
  lda (row+4),y
  sbc (row+6),y
  sta t1
  stx t0
  ldx vi
  ldy Rb+3,x
  TERMN row+8
  ldx vi
  ldy Rb+6,x
  TERMN row+16
  lda rk
  clc
  adc vi
  tax
  lda t1
  sta Mv,x
  inc vi
  lda vi
  cmp #3
  bne jloop
  lda rk
  clc
  adc #3
  tax
  cpx #9
  beq :+
  jmp iloop
:
  rts
.endproc

; Pointers for the vertex loop.
.proc build_mat
  ldx #0
  ldy #0
l:
  lda Mv,x
  eor #$80
  sta mat,y
  sta mat+4,y
  eor #$FF
  clc
  adc #1
  sta mat+2,y
  sta mat+6,y
  tya
  clc
  adc #8
  tay
  inx
  cpx #9
  bne l
  rts
.endproc

; Faces tested in ship space: the eye position is (M transposed) * -rel, scaled
; to 8 bits, and a face shows when n.E exceeds the plane distance.
.proc visible_faces
  ; scale rel so every component fits in +-63
  lda relX
  sta px
  lda relX+1
  sta px+1
  lda relY
  sta py
  lda relY+1
  sta py+1
  lda relZ
  sta pz
  lda relZ+1
  sta pz+1
  lda #0
  sta rk
shrink:
  ldx #4
chk:
  lda px,x
  clc
  adc #64
  lda px+1,x
  adc #0
  bne more
  lda px,x
  clc
  adc #64
  bmi more
  dex
  dex
  bpl chk
  jmp scaled
more:
  ldx #4
: lda px+1,x
  cmp #$80
  ror px+1,x
  ror px,x
  dex
  dex
  bpl :-
  inc rk
  jmp shrink
scaled:
  lda px
  SETM row
  lda py
  SETM row+8
  lda pz
  SETM row+16
  ; E[j] = -(rel . column j of M)
  ldx #0
  stx vi
eloop:
  ldx vi
  lda Mv,x
  eor #$80
  tay
  PROD row
  sta t1
  ldx vi
  lda Mv+3,x
  eor #$80
  tay
  PROD row+8
  clc
  adc t1
  sta t1
  ldx vi
  lda Mv+6,x
  eor #$80
  tay
  PROD row+16
  clc
  adc t1
  eor #$FF
  clc
  adc #1
  ldx vi
  sta Eb,x
  inc vi
  lda vi
  cmp #3
  bne eloop
  lda Eb+0
  SETM row
  lda Eb+1
  SETM row+8
  lda Eb+2
  SETM row+16
  lda #0
  sta vis
  ldx fLast
floop:
  stx vi
  ldy nxu,x
  TERM1 row
  ldx vi
  ldy nyu,x
  TERMN row+8
  ldx vi
  ldy nzu,x
  TERMN row+16
  ; compare with the plane distance at the same scale
  ldx vi
  lda fd,x
  ldy rk
  beq :++
: lsr a
  dey
  bne :-
:
  sta u0
  lda t1
  sec
  sbc u0
  bvc :+
  eor #$80
:
  bmi hidden
  ldx vi
  lda vis
  ora bitMask,x
  sta vis
hidden:
  ldx vi
  cpx fFirst
  beq :+
  dex
  jmp floop
:
  rts
.endproc

; Close ships need the eye position from the full 16-bit rel, or faces pop.
.macro SPLIT16 lo, hi, hv, lv
  lda lo
  and #$7F
  eor #$80
  sta lv
  lda lo
  asl a
  lda hi
  rol a
  eor #$80
  sta hv
.endmacro

.proc visible_close
  SPLIT16 relX, relX+1, hxA, lxA
  SPLIT16 relY, relY+1, hxB, lxB
  SPLIT16 relZ, relZ+1, hxC, lxC
  ; E[j] = -(rel . column j of M), computed in 16 bits
  ldx #0
  stx rk
eloop:
  lda Mv,x
  SETM row
  ldx rk
  lda Mv+3,x
  SETM row+8
  ldx rk
  lda Mv+6,x
  SETM row+16
  lda #0
  sta t0
  sta t1
  MUL16 row, hxA, lxA
  MUL16 row+8, hxB, lxB
  MUL16 row+16, hxC, lxC
  lda rk
  asl a
  tax
  lda #0
  sec
  sbc t0
  sta eyeV,x
  lda #0
  sbc t1
  sta eyeV+1,x
  ; large hulls are drawn scaled up, so measure the eye in model units
  ldy mShift
  beq :++
: lda eyeV+1,x
  cmp #$80
  ror eyeV+1,x
  ror eyeV,x
  dey
  bne :-
:
  inc rk
  ldx rk
  cpx #3
  beq :+
  jmp eloop
:
  ; scale to 8 bits, keeping the same angular precision at any range
  lda #0
  sta rk
shrink:
  ldx #4
chk:
  lda eyeV,x
  clc
  adc #100
  lda eyeV+1,x
  adc #0
  bne more
  lda eyeV,x
  clc
  adc #100
  cmp #200
  bcs more
  dex
  dex
  bpl chk
  jmp scaled
more:
  ldx #4
: lda eyeV+1,x
  cmp #$80
  ror eyeV+1,x
  ror eyeV,x
  dex
  dex
  bpl :-
  inc rk
  jmp shrink
scaled:
  lda eyeV
  SETM row
  lda eyeV+2
  SETM row+8
  lda eyeV+4
  SETM row+16
  lda #0
  sta vis
  ldx fLast
floop:
  stx vi
  ldy nxu,x
  TERM1 row
  ldx vi
  ldy nyu,x
  TERMN row+8
  ldx vi
  ldy nzu,x
  TERMN row+16
  ; compare with the plane distance as 8.8 at the same scale
  ldx vi
  lda fd,x
  sta u1
  lda #0
  ldy rk
  beq :++
: lsr u1
  ror a
  dey
  bne :-
:
  sta u0
  lda t0
  sec
  sbc u0
  lda t1
  sbc u1
  bmi hidden
  lda vis
  ora bitMask,x
  sta vis
hidden:
  ldx vi
  cpx fFirst
  beq :+
  dex
  jmp floop
:
  rts
.endproc

; rel + rounded t1. The world wraps at 4096 units, so this cannot overflow.
.macro ADDSTORE base, lo, hi
  .local pos, noScale
  ldy #0
  lda t1
  bpl pos
  dey
pos:
  sty u0
  ldy mShift
  beq noScale
: asl t0
  rol t1
  rol u0
  dey
  bne :-
noScale:
  lda t0
  asl a
  lda t1
  ldx vi
  adc base
  sta lo,x
  lda u0
  adc base+1
  sta hi,x
.endmacro

.proc transform
  ldx vFirst
vloop:
  stx vi
  ldy vxu,x
  TERM1 mat+0
  ldx vi
  ldy vyu,x
  TERMN mat+8
  ldx vi
  ldy vzu,x
  TERMN mat+16
  ADDSTORE relX, camXL, camXH
  ldx vi
  ldy vxu,x
  TERM1 mat+24
  ldx vi
  ldy vyu,x
  TERMN mat+32
  ldx vi
  ldy vzu,x
  TERMN mat+40
  ADDSTORE relY, camYL, camYH
  ldx vi
  ldy vxu,x
  TERM1 mat+48
  ldx vi
  ldy vyu,x
  TERMN mat+56
  ldx vi
  ldy vzu,x
  TERMN mat+64
  ADDSTORE relZ, camZL, camZH
  ; project
  ldx vi
  lda camXL,x
  sta px
  lda camXH,x
  sta px+1
  lda camYL,x
  sta py
  lda camYH,x
  sta py+1
  lda camZL,x
  sta pz
  lda camZH,x
  sta pz+1
  jsr project
  ldx vi
  bcc front
  lda #1
  sta behind,x
  sta offscreen
  jmp next
front:
  jsr storeVertex
next:
  ldx vi
  inx
  cpx vEnd
  beq :+
  jmp vloop
:
  rts
.endproc

; Saves outX/outY for vertex vi and flags anything outside the view.
.proc storeVertex
  ldx vi
  lda #0
  sta behind,x
  lda outX
  sta scrXL,x
  lda outY
  sta scrYL,x
  lda outX+1
  sta scrXH,x
  lda outY+1
  sta scrYH,x
  ora outX+1
  bne off
  lda outX
  cmp #192
  bcs off
  lda outY
  beq off
  cmp #127
  bcc done
off:
  lda #1
  sta offscreen
done:
  rts
.endproc

; Ships well beyond the near plane share one perspective divide. The top two
; rows of M are scaled once, leaving two spare bits (the depth here is at least
; 768, so rk is at least 2); each vertex then only needs its rotation sum.
.proc transform_far
  lda relZ
  sta pz
  lda relZ+1
  sta pz+1
  jsr prjSetup
  lda relX
  ldx relX+1
  jsr prjMul
  sta ix+1
  stx ix
  lda relY
  ldx relY+1
  jsr prjMul
  sta iy+1
  stx iy
  ldx #0
  stx hx
scale:
  ldy Mv,x
  tya
  eor #$80
  tay
  PROD mat+M_PRJ
  sta u1
  ldy rk
  dey
  dey
  beq :++
: cmp #$80
  ror a
  dey
  bne :-
:
  ldx hx
  pha
  txa
  asl a
  asl a
  asl a
  tay
  pla
  eor #$80
  sta mat,y
  sta mat+4,y
  eor #$FF
  clc
  adc #1
  sta mat+2,y
  sta mat+6,y
  inx
  stx hx
  cpx #6
  bne scale
  ldx vFirst
vloop:
  stx vi
  ldy vxu,x
  TERM1 mat+0
  ldx vi
  ldy vyu,x
  TERMN mat+8
  ldx vi
  ldy vzu,x
  TERMN mat+16
  ; offset = sum / 4
  lda t1
  cmp #$80
  ror a
  cmp #$80
  ror a
  ldy #0
  cmp #$80
  bcc :+
  dey
:
  clc
  adc ix
  sta outX
  tya
  adc ix+1
  sta outX+1
  lda outX
  clc
  adc #96
  sta outX
  bcc :+
  inc outX+1
:
  ldx vi
  ldy vxu,x
  TERM1 mat+24
  ldx vi
  ldy vyu,x
  TERMN mat+32
  ldx vi
  ldy vzu,x
  TERMN mat+40
  lda t1
  cmp #$80
  ror a
  cmp #$80
  ror a
  ldy #0
  cmp #$80
  bcc :+
  dey
:
  clc
  adc iy
  sta t0
  tya
  adc iy+1
  sta t1
  lda #64
  sec
  sbc t0
  sta outY
  lda #0
  sbc t1
  sta outY+1
  jsr storeVertex
  ldx vi
  inx
  cpx vEnd
  beq :+
  jmp vloop
:
  rts
.endproc

; Perspective: (px,py,pz) -> outX,outY. Carry set if the point is behind NEAR.
; All inputs are within +-16383.
.proc project
  lda pz+1
  bmi behindP
  bne ok
  lda pz
  cmp #NEAR
  bcs ok
behindP:
  sec
  rts
ok:
  jsr prjSetup
  ; X * r / 128
  lda px
  ldx px+1
  jsr prjMul
  sta outX+1
  stx outX
  lda py
  ldx py+1
  jsr prjMul
  sta t1
  stx t0
  lda outX
  clc
  adc #96
  sta outX
  lda outX+1
  adc #0
  sta outX+1
  lda #64
  sec
  sbc t0
  sta outY
  lda #0
  sbc t1
  sta outY+1
  clc
  rts
.endproc

; pz -> projection multiplier and shift count rk
.proc prjSetup
  ldx pz+1
  ldy shiftTab,x
  sty rk
  lda pz
  sta u0
  lda pz+1
  cpy #0
  beq :++
: lsr a
  ror u0
  dey
  bne :-
:
  ldy u0
  lda recipN,y
  SETM mat+M_PRJ
  rts
.endproc

; A=lo X=hi of a 15-bit value. Returns value*r/128 >> rk, lo in X, hi in A.
.proc prjMul
  sta u0
  and #$7F
  eor #$80
  sta hl
  lda u0
  asl a
  txa
  rol a
  eor #$80
  tay
  PROD mat+M_PRJ
  cmp #$80
  ror a
  sta t1
  txa
  ror a
  sta t0
  ldy hl
  PROD mat+M_PRJ
  clc
  adc t0
  sta t0
  lda t1
  adc #0
  sta t1
  ldy rk
  beq done
: lda t1
  cmp #$80
  ror t1
  ror t0
  dey
  bne :-
  ; round to nearest using the last bit shifted out
  lda t0
  adc #0
  sta t0
  lda t1
  adc #0
  sta t1
done:
  ldx t0
  lda t1
  rts
.endproc

; Region code for (A=lo,X=hi of x, stored in u0/u1 for y): 1 left 2 right 4 up 8 down
.macro OUTCODE xv, yv, dest
  lda #0
  sta dest
  lda xv+1
  bmi :+
  bne :++
  lda xv
  cmp #192
  bcs :++
  jmp :+++
:
  lda #1
  sta dest
  jmp :++
:
  lda #2
  sta dest
:
  lda yv+1
  bmi :+
  bne :++
  lda yv
  beq :+
  cmp #127
  bcs :++
  jmp :+++
:
  lda dest
  ora #4
  sta dest
  jmp :++
:
  lda dest
  ora #8
  sta dest
:
.endmacro

; Clips the segment a-b to the view by bisection. Carry clear = visible.
.proc clip2d
  lda #6
  sta guard
top:
  OUTCODE ax, ay, oa
  OUTCODE bx, by, ob
  lda oa
  ora ob
  bne :+
  clc
  rts
:
  lda oa
  and ob
  beq :+
  sec
  rts
:
  dec guard
  bne :+
  sec
  rts
:
  lda oa
  bne outA
  ; swap so `a` is the outside end
  ldx #3
: lda ax,x
  ldy bx,x
  sta bx,x
  sty ax,x
  dex
  bpl :-
  lda ob
  sta oa
outA:
  lda oa
  sta u0
  ; lowest set bit
  sec
  sbc #1
  eor u0
  and u0
  sta clipBit
  ; inside point starts at b
  lda bx
  sta ix
  lda bx+1
  sta ix+1
  lda by
  sta iy
  lda by+1
  sta iy+1
loop:
  lda ax
  clc
  adc ix
  sta mx
  lda ax+1
  adc ix+1
  cmp #$80
  ror a
  ror mx
  sta mx+1
  lda ay
  clc
  adc iy
  sta my
  lda ay+1
  adc iy+1
  cmp #$80
  ror a
  ror my
  sta my+1
  ; finished when the midpoint equals an end on both axes
  lda mx
  cmp ix
  bne notI
  lda mx+1
  cmp ix+1
  bne notI
  lda my
  cmp iy
  bne notI
  lda my+1
  cmp iy+1
  beq settled
notI:
  lda mx
  cmp ax
  bne notA
  lda mx+1
  cmp ax+1
  bne notA
  lda my
  cmp ay
  bne notA
  lda my+1
  cmp ay+1
  beq settled
notA:
  OUTCODE mx, my, u1
  lda u1
  and clipBit
  beq inside
  ldx #3
: lda mx,x
  sta ax,x
  dex
  bpl :-
  jmp loop
inside:
  lda mx
  sta ix
  lda mx+1
  sta ix+1
  lda my
  sta iy
  lda my+1
  sta iy+1
  jmp loop
settled:
  lda ix
  sta ax
  lda ix+1
  sta ax+1
  lda iy
  sta ay
  lda iy+1
  sta ay+1
  jmp top
.endproc

; Finds the point where a segment crosses the near plane. X = vertex in front,
; Y = vertex behind. Result in outX/outY.
.proc near_cross
  lda camXL,x
  sta f3+0
  lda camXH,x
  sta f3+1
  lda camYL,x
  sta f3+2
  lda camYH,x
  sta f3+3
  lda camZL,x
  sta f3+4
  lda camZH,x
  sta f3+5
  lda camXL,y
  sta b3+0
  lda camXH,y
  sta b3+1
  lda camYL,y
  sta b3+2
  lda camYH,y
  sta b3+3
  lda camZL,y
  sta b3+4
  lda camZH,y
  sta b3+5
  inc statNear
  lda #9
  sta guard
iter:
  ldx #4
: lda f3,x
  clc
  adc b3,x
  sta px,x
  lda f3+1,x
  adc b3+1,x
  cmp #$80
  ror a
  ror px,x
  sta px+1,x
  dex
  dex
  bpl :-
  lda pz+1
  bmi back
  bne frontM
  lda pz
  cmp #NEAR
  bcc back
frontM:
  ldx #5
: lda px,x
  sta f3,x
  dex
  bpl :-
  jmp cont
back:
  ldx #5
: lda px,x
  sta b3,x
  dex
  bpl :-
cont:
  dec guard
  bne iter
  ldx #5
: lda f3,x
  sta px,x
  dex
  bpl :-
  jmp project
.endproc


; Edge drawing when part of the ship may be outside the view.
.proc edge_clipped
  ldx hxV
  ldy hyV
  lda behind,x
  beq aFront
  lda behind,y
  beq :+
  rts                  ; both behind the camera
:
  ; a behind, b in front
  ldx hyV
  ldy hxV
  jsr near_cross
  lda outX
  sta ax
  lda outX+1
  sta ax+1
  lda outY
  sta ay
  lda outY+1
  sta ay+1
  ldx hyV
  lda scrXL,x
  sta bx
  lda scrXH,x
  sta bx+1
  lda scrYL,x
  sta by
  lda scrYH,x
  sta by+1
  jmp clip
aFront:
  lda behind,y
  beq both
  ; a in front, b behind
  jsr near_cross
  lda outX
  sta bx
  lda outX+1
  sta bx+1
  lda outY
  sta by
  lda outY+1
  sta by+1
  ldx hxV
  jmp loadA
both:
  ldx hyV
  lda scrXL,x
  sta bx
  lda scrXH,x
  sta bx+1
  lda scrYL,x
  sta by
  lda scrYH,x
  sta by+1
  ldx hxV
loadA:
  lda scrXL,x
  sta ax
  lda scrXH,x
  sta ax+1
  lda scrYL,x
  sta ay
  lda scrYH,x
  sta ay+1
clip:
  inc statClip
  jsr clip2d
  bcs :+
  lda ax
  sta x0
  lda ay
  sta y0
  lda bx
  sta x1
  lda by
  sta y1
  jsr line
:
  rts
.endproc

bitMask: .byte 1,2,4,8,16,32,64,1,2,4,8,16,1,2,4,8,16,32

