; ---------- missions ----------
; A mission is a list of 13-byte steps, each a nav point:
; kind, X, Y, Z (words), arrival line, clear line, ships, ship health, freighter
; bits, wait. Phases: 0 flying to the point, 1 fighting, 2 waiting there.
NAV_R     = 350
LAND_R    = 450
CALL_R    = 1100         ; the Vigil calls you in from here
S_EMPTY   = 0
S_FIGHT   = 1
S_LAND    = 2
SP_KIND   = 0
SP_ARRIVE = 7
SP_CLEAR  = 8
SP_COUNT  = 9
SP_HP     = 10
SP_FRE    = 11
SP_WAIT   = 12
VIGIL     = 3            ; ship index
NAVPT     = 5            ; position slot of the nav point

.proc setup_mission
  lda #3
  sta aiCount
  sta aiStep
  ldx #3
: lda #0
  sta alive,x
  sta hPassive,x
  sta shipModel,x
  sta hBreak,x
  sta hTurn,x
  sta hPitch,x
  lda #30
  sta hCool,x
  dex
  bpl :-
  lda #MISSION_SHIELDS
  sta shields
  ; the Vigil, behind the launch point
  lda #<VIGIL_X
  sta shipPosL+VIGIL
  lda #>VIGIL_X
  sta shipPosH+VIGIL
  lda #<VIGIL_Y
  sta shipPosL+16+VIGIL
  lda #>VIGIL_Y
  sta shipPosH+16+VIGIL
  lda #<VIGIL_Z
  sta shipPosL+32+VIGIL
  lda #>VIGIL_Z
  sta shipPosH+32+VIGIL
  lda #1
  sta alive+VIGIL
  lda #2
  sta shipModel+VIGIL
  lda #40              ; lying at an angle, so she shows her length
  sta hYaw+VIGIL
  ; Hale launches alongside
  lda #<(-220)
  sta shipPosL+HALE
  lda #>(-220)
  sta shipPosH+HALE
  lda #<(-70)
  sta shipPosL+16+HALE
  lda #>(-70)
  sta shipPosH+16+HALE
  lda #120
  sta shipPosL+32+HALE
  lda #0
  sta shipPosH+32+HALE
  sta hYaw+HALE
  sta fvX+HALE
  sta fvY+HALE
  lda #127
  sta fvZ+HALE
  lda #1
  sta alive+HALE
  ldx missionId
  lda missionLo,x
  sta mStepL
  lda missionHi,x
  sta mStepH
.ifdef TESTSTEP
  ; regression builds: start at a chosen nav point, some way short of it
  lda mStepL
  clc
  adc #(13*TESTSTEP)
  sta mStepL
  bcc :+
  inc mStepH
:
  jsr load_step
  lda #0
  sta camPos+1
  sta camPos+4
  sta camPos+7
  lda shipPosH+NAVPT
  and #$0F
  sta camPos+2
  lda shipPosL+NAVPT
  sta camPos+1
  lda shipPosL+32+NAVPT
  sec
  sbc #<TESTBACK
  sta camPos+7
  lda shipPosH+32+NAVPT
  sbc #>TESTBACK
  and #$0F
  sta camPos+8
  lda shipPosL+16+NAVPT
  sta camPos+4
  lda shipPosH+16+NAVPT
  and #$0F
  sta camPos+5
  rts
.else
  jmp load_step
.endif
.endproc

load_step:
  lda #0
  sta mPhase
  lda mStepL
  sta spadP
  lda mStepH
  sta spadP+1
  ldy #1
  lda (spadP),y
  sta shipPosL+NAVPT
  iny
  lda (spadP),y
  sta shipPosH+NAVPT
  iny
  lda (spadP),y
  sta shipPosL+16+NAVPT
  iny
  lda (spadP),y
  sta shipPosH+16+NAVPT
  iny
  lda (spadP),y
  sta shipPosL+32+NAVPT
  iny
  lda (spadP),y
  sta shipPosH+32+NAVPT
  rts

next_step:
  lda mStepL
  clc
  adc #13
  sta mStepL
  bcc :+
  inc mStepH
:
  jmp load_step

; Carry set when the player is inside a cube of half-width t0/t1 around the
; nav point. The world wraps at 4096 units.
near_nav:
  lda #<NAV_R
  sta t0
  lda #>NAV_R
  jmp near_cube
near_land:
  lda #<LAND_R
  sta t0
  lda #>LAND_R
near_cube:
  sta t1
  ldx #0               ; 0, 16, 32 world axis; 1, 4, 7 camera axis
  ldy #1
axis:
  lda shipPosL+NAVPT,x
  sec
  sbc camPos,y
  sta u0
  lda shipPosH+NAVPT,x
  sbc camPos+1,y
  and #$0F
  cmp #$08
  bcc pos
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
pos:
  cmp t1
  bcc inside
  bne far
  lda u0
  cmp t0
  bcs far
inside:
  iny
  iny
  iny
  txa
  clc
  adc #16
  tax
  cpx #48
  bne axis
  sec
  rts
far:
  clc
  rts

; Two ships appear beyond the nav point. Freighter bits make a slot passive.
.proc spawn_step
  ldy #SP_COUNT
  lda (spadP),y
  sta killsLeft
  ldy #SP_FRE
  lda (spadP),y
  sta u1
  ldx #0
l:
  lda shipPosL+NAVPT
  clc
  adc spawnX,x
  sta shipPosL,x
  lda shipPosH+NAVPT
  adc spawnXH,x
  sta shipPosH,x
  lda shipPosL+16+NAVPT
  clc
  adc spawnY,x
  sta shipPosL+16,x
  lda shipPosH+16+NAVPT
  adc #0
  sta shipPosH+16,x
  lda shipPosL+32+NAVPT
  clc
  adc spawnZ,x
  sta shipPosL+32,x
  lda shipPosH+32+NAVPT
  adc spawnZH,x
  sta shipPosH+32,x
  lda #1
  sta alive,x
  ldy #SP_HP
  lda (spadP),y
  sta hp,x
  lda #0
  sta hBreak,x
  sta hPitch,x
  sta hTurn,x
  sta hPassive,x
  sta shipModel,x
  lda #128             ; facing the arriving player
  sta hYaw,x
  lda #0
  sta fvX,x
  sta fvY,x
  lda #$81
  sta fvZ,x
  lda #30
  sta hCool,x
  lsr u1
  bcc :+
  ; a freighter: big, slow, and on a fixed course
  lda #1
  sta hPassive,x
  sta shipModel,x
  sta hTurn,x
:
  inx
  cpx killsLeft
  beq :+
  jmp l
:
  rts
.endproc

spawnX:  .byte <(-260),<260
spawnXH: .byte >(-260),>260
spawnY:  .byte 40,110
spawnZ:  .byte <500,<760
spawnZH: .byte >500,>760

.proc mission_tick
  lda missionId
  bne :+
  rts
:
  lda gameMode
  cmp #MODE_PLAY
  beq :+
  rts
:
  lda mStepL
  sta spadP
  lda mStepH
  sta spadP+1
  ; never leave a pilot stopped dead after a collision
  lda bumpT
  bne :+
  lda speed
  bne :+
  lda #START_SPEED
  sta speed
:
  lda mStage
  bne :+
  inc mStage
  lda #R_WING
  jsr say_radio
:
  lda mPhase
  cmp #1
  beq fighting
  cmp #2
  beq waiting
  ; flying to the point
  ldy #SP_KIND
  lda (spadP),y
  cmp #S_LAND
  bne notLand
  jsr near_land
  bcs :+
  lda mStage
  cmp #2
  beq noCall
  lda #<CALL_R
  sta t0
  lda #>CALL_R
  jsr near_cube
  bcc noCall
  lda #2
  sta mStage
  ldy #SP_ARRIVE
  lda (spadP),y
  ldx #0
  stx radioT
  jsr say_radio
noCall:
  rts
:
  lda #1
  jmp end_mission
notLand:
  jsr near_nav
  bcs :+
  rts
:
  ldy #SP_ARRIVE
  lda (spadP),y
  ldx #0
  stx radioT           ; arrival lines are never dropped
  jsr say_radio
  ldy #SP_KIND
  lda (spadP),y
  cmp #S_FIGHT
  beq :+
  ldy #SP_WAIT
  lda (spadP),y
  sta mWait
  lda #2
  sta mPhase
  rts
:
  lda #1
  sta mPhase
  jmp spawn_step
fighting:
  lda killsLeft
  beq :+
  rts
:
  ldy #SP_CLEAR
  lda (spadP),y
  ldx #0
  stx radioT
  jsr say_radio
  jmp next_step
waiting:
  lda mWait
  sec
  sbc dt
  sta mWait
  bcc :+
  rts
:
  ldy #SP_CLEAR
  lda (spadP),y
  beq :+
  ldx #0
  stx radioT
  jsr say_radio
:
  jmp next_step
.endproc

