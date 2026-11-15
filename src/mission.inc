; ---------- missions ----------
; Lives in the fixed high bank: it only runs during flight, when the flight
; bank is mapped, so its calls into flight code are safe.
.pushseg
.segment "HIGH"
; A mission is a list of 14-byte steps, each a nav point:
; kind, X, Y, Z (words), arrival line, clear line, ships, ship health, freighter
; bits, wait, time limit (256-frame units, 0 = none). Phases: 0 flying to the point, 1 fighting, 2 waiting there.
NAV_R     = 350
LAND_R    = 450
CALL_R    = 1100         ; the Vigil calls you in from here
S_EMPTY   = 0
S_FIGHT   = 1
S_LAND    = 2
S_AMBUSH  = 3
S_DEFEND  = 4            ; hold for the time limit; waves keep coming
S_GUARD   = 5            ; one scout goes for the wingman, on a timer
SHUTTLE_HP = 6
SP_KIND   = 0
SP_ARRIVE = 7
SP_CLEAR  = 8
SP_COUNT  = 9
SP_HP     = 10
SP_FRE    = 11
SP_WAIT   = 12
SP_LIMIT  = 13
STEP_LEN  = 14
FL_CONVOY = 8            ; story flag: the convoy escaped
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
  lda #0
  sta guardOn
  sta echoDis
  ldx missionId
  lda missionModeTab,x
  bne :+
  lda storyFlags2      ; Hale is gone: Mara flies your wing from now on
  and #F2_HALE_LOST
  beq :+
  lda #1
: sta missionMode
  jsr vigil_home       ; the Vigil, behind the launch point
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
  lda missionMode
  cmp #2
  bne :+
  lda #4               ; Ren's shuttle flies in the wingman's place
  sta shipModel+HALE
:
  jsr mission_extras   ; Ren's shuttle to protect, or a wreck in the Vigil's place
  ldx missionId
  lda missionLo,x
  sta mStepL
  lda missionHi,x
  sta mStepH
.ifdef TESTSTEP
  ; regression builds: start at a chosen nav point, some way short of it
  lda mStepL
  clc
  adc #(STEP_LEN*TESTSTEP)
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
  ldy #SP_KIND
  lda (spadP),y
  cmp #S_LAND
  bne :+
  jmp vigil_home       ; time to land: the Vigil is back in her slot
: rts

next_step:
  lda mStepL
  clc
  adc #STEP_LEN
  sta mStepL
  bcc :+
  inc mStepH
:
  jmp load_step

; Carry set when the player is inside a cube of half-width t0/t1 around the
; nav point. The world wraps at 4096 units.
near_nav:
  jsr ref_escort
  lda #<NAV_R
  sta t0
  lda #>NAV_R
  jmp near_cube
near_land:
  jsr ref_camera
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
  sbc refPos,y
  sta u0
  lda shipPosH+NAVPT,x
  sbc refPos+1,y
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

; refPos: where arrival is measured from. The player, or in an escort the shuttle.
ref_escort:
  lda missionMode
  cmp #2
  bne ref_camera
  lda alive+HALE
  beq ref_camera
  lda shipPosL+HALE
  sta refPos+1
  lda shipPosH+HALE
  sta refPos+2
  lda shipPosL+16+HALE
  sta refPos+4
  lda shipPosH+16+HALE
  sta refPos+5
  lda shipPosL+32+HALE
  sta refPos+7
  lda shipPosH+32+HALE
  sta refPos+8
  rts
ref_camera:
  ldx #8
: lda camPos,x
  sta refPos,x
  dex
  bpl :-
  rts

; Two ships appear beyond the nav point. Freighter bits make a slot passive.
.proc spawn_step
  ldy #SP_COUNT
  lda (spadP),y
  sta killsLeft
  ldy #SP_FRE
  lda (spadP),y
  and #3
  sta u1               ; freighters
  lda (spadP),y
  lsr a
  lsr a
  and #3
  sta mU2               ; lancers
  lda (spadP),y
  and #$40
  sta mBig
  lda (spadP),y
  and #$80
  sta mHold
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
  bcc notHold          ; not a freighter
  ; a freighter: big, slow, and on a fixed course
  lda #1
  sta hPassive,x
  sta shipModel,x
  sta hTurn,x
  lda #64              ; the convoy runs side-on, away from the Vigil
  sta hYaw,x
  lda #0
  sta fvZ,x
  lda #127
  sta fvX,x
  lda mHold
  beq notHold
  ; a fixed target on the nav point, standing on end
  lda mBig
  beq :+
  lda #2
  sta shipModel,x      ; the listening post: twice the buoy's size
: lda #2
  sta hPassive,x
  lda #64
  sta hPitch,x
  lda #0
  sta fvZ,x
  sta fvX,x
  lda #127
  sta fvY,x
  lda shipPosL+NAVPT
  sta shipPosL,x
  lda shipPosH+NAVPT
  sta shipPosH,x
  lda shipPosL+16+NAVPT
  sta shipPosL+16,x
  lda shipPosH+16+NAVPT
  sta shipPosH+16,x
  lda shipPosL+32+NAVPT
  sta shipPosL+32,x
  lda shipPosH+32+NAVPT
  sta shipPosH+32,x
notHold:
  lsr mU2
  bcc :+
  lda #8               ; a lancer: tougher by four hits
  sta shipModel,x
  lda hp,x
  clc
  adc #4
  sta hp,x
:
  inx
  cpx killsLeft
  beq :+
  jmp l
:
  rts
.endproc

; Ships appear about 640 units behind the player, chasing.
.proc spawn_behind
  ldy #SP_COUNT
  lda (spadP),y
  sta killsLeft
  ldy camYaw
  lda sinTab,y
  sta behS
  tya
  clc
  adc #64
  tay
  lda sinTab,y
  sta behC
  ldx #0
l:
  ; X = cam - 5s +/- c
  lda behS
  eor #$FF
  clc
  adc #1
  sta u1
  lda behC
  cpx #0
  bne :+
  eor #$FF
  clc
  adc #1
: jsr acc5
  lda camPos+1
  clc
  adc behAcc
  sta shipPosL,x
  lda camPos+2
  adc behAcc+1
  and #$0F
  sta shipPosH,x
  ; Z = cam - 5c -/+ s
  lda behC
  eor #$FF
  clc
  adc #1
  sta u1
  lda behS
  cpx #0
  beq :+
  eor #$FF
  clc
  adc #1
: jsr acc5
  lda camPos+7
  clc
  adc behAcc
  sta shipPosL+32,x
  lda camPos+8
  adc behAcc+1
  and #$0F
  sta shipPosH+32,x
  ; Y: a little above
  lda spawnY,x
  clc
  adc camPos+4
  sta shipPosL+16,x
  lda camPos+5
  adc #0
  and #$0F
  sta shipPosH+16,x
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
  sta fvY,x
  lda camYaw
  sta hYaw,x
  lda behS
  sta fvX,x
  lda behC
  sta fvZ,x
  lda #90              ; a moment to turn before they open fire
  sta hCool,x
  inx
  cpx killsLeft
  beq :+
  jmp l
:
  rts
.endproc

; behAcc = 5 * u1 + A, all signed bytes, as a 16-bit sum.
acc5:
  pha
  lda #0
  sta behAcc
  sta behAcc+1
  ldy #5
: lda u1
  jsr acc_sb
  dey
  bne :-
  pla
acc_sb:
  pha
  clc
  adc behAcc
  sta behAcc
  pla
  bmi :+
  lda behAcc+1
  adc #0
  sta behAcc+1
  rts
: lda behAcc+1
  adc #$FF
  sta behAcc+1
  rts

spawnX:  .byte <(-260),<260
spawnXH: .byte >(-260),>260
spawnY:  .byte 40,110
spawnZ:  .byte <500,<760
spawnZH: .byte >500,>760

.proc defend_tick
  lda mLimit
  sec
  sbc dt
  sta mLimit
  bcs :+
  dec mLimit+1
  bmi held
:
  ; halfway: the step's wait byte holds a line for it
  lda mSaid
  bne :+
  lda mLimit+1
  cmp mHalf
  bcs :+
  inc mSaid
  ldy #SP_WAIT
  lda (spadP),y
  ldx #0
  stx radioT
  jsr say_radio
:
  ; another wave once this one is down, unless time is nearly up
  lda killsLeft
  bne :+
  lda mLimit+1
  cmp #3
  bcc :+
  jmp spawn_step
: rts
held:
  lda #0
  sta killsLeft
  sta alive
  sta alive+1
  ldy #SP_CLEAR
  lda (spadP),y
  ldx #0
  stx radioT
  jsr say_radio
  jmp next_step
.endproc

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
  jsr prot_hits
  jsr echo_tick
  lda gameMode
  cmp #MODE_PLAY
  beq :+
  rts                  ; the shuttle was lost
:
  lda mStepL
  sta spadP
  lda mStepH
  sta spadP+1
  ; after a crash, give the pilot about three seconds to turn, then their
  ; speed back. A pilot who stops on purpose is left alone.
  lda crashSpeed
  beq calm
  lda speed
  beq :+
  lda #0               ; they throttled up themselves
  sta crashSpeed
  sta stopT
  jmp calm
: lda stopT
  clc
  adc dt
  sta stopT
  cmp #180
  bcc calm
  lda crashSpeed
  sta speed
  lda #0
  sta crashSpeed
  sta stopT
calm:
  lda mStage
  bne :+
  inc mStage
  lda #R_WING
  jsr say_radio
:
  lda mPhase
  cmp #1
  bne :+
  jmp fighting
:
  cmp #2
  bne :+
  jmp waiting
:
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
  ldy #SP_KIND
  lda (spadP),y
  cmp #S_AMBUSH
  bne :+
  ; quiet first: the warning comes when the wait runs out
  ldy #SP_WAIT
  lda (spadP),y
  sta mWait
  lda #2
  sta mPhase
  rts
:
  ldy #SP_ARRIVE
  lda (spadP),y
  ldx #0
  stx radioT           ; arrival lines are never dropped
  jsr say_radio
  ldy #SP_KIND
  lda (spadP),y
  cmp #S_GUARD
  bne :+
  lda #1
  sta mPhase
  lda #0
  sta mLimit
  sta mLimit+1
  jmp guard_start
: cmp #S_FIGHT
  beq :+
  cmp #S_DEFEND
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
  lda #0
  sta mLimit
  ldy #SP_LIMIT
  lda (spadP),y
  sta mLimit+1
  lsr a
  sta mHalf
  lda #0
  sta mSaid
  jsr spawn_step
  jmp spawn_post
fighting:
  ldy #SP_KIND
  lda (spadP),y
  cmp #S_DEFEND
  bne :+
  jmp defend_tick
: cmp #S_GUARD
  bne :+
  jsr guard_tick
:
  lda killsLeft
  bne :+
  jmp cleared
:
  lda mLimit+1
  ora mLimit
  bne :+
  rts                  ; no time limit
:
  lda mLimit
  sec
  sbc dt
  sta mLimit
  bcs :+
  dec mLimit+1
  bmi escape
:
  lda mLimit+1
  cmp #4               ; about 17 seconds left
  bne :+
  lda mLimit
  cmp #8
  bcs :+
  lda #R_RUN
  ldx #0
  stx radioT
  jsr say_radio
:
  rts
escape:
  ; the survivors jump out
  ldy #SP_COUNT
  lda (spadP),y
  cmp killsLeft
  bne partial
  lda storyFlags
  ora #FL_CONVOY
  sta storyFlags
  lda #R_JUMPED
  jmp gone
partial:
  lda #R_ONEAWAY
gone:
  ldx #0
  stx radioT
  jsr say_radio
  lda #0
  sta killsLeft
  sta alive
  sta alive+1
  jmp next_step
cleared:
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
  ldy #SP_KIND
  lda (spadP),y
  cmp #S_AMBUSH
  bne :+
  ldy #SP_ARRIVE
  lda (spadP),y
  ldx #0
  stx radioT
  jsr say_radio
  lda #1
  sta mPhase
  lda #0
  sta mLimit
  sta mLimit+1
  jmp spawn_behind
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
.popseg
