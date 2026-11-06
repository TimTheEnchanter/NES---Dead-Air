; ---------- ship AI ----------
; Every AI ship moves each frame. Only one steers, in rotation, so the cost stays
; flat; its turn rates are multiplied by aiStep to make up for the skipped frames.

; Puts ship X back where it was before moving, and sends it away.
ai_undo_move:
  lda hSaveL,x
  sta shipPosL,x
  lda hSaveL+16,x
  sta shipPosL+16,x
  lda hSaveL+32,x
  sta shipPosL+32,x
  lda hSaveH,x
  sta shipPosH,x
  lda hSaveH+16,x
  sta shipPosH+16,x
  lda hSaveH+32,x
  sta shipPosH+32,x
  jsr break_len
  sta hBreak,x
  rts

; Frames a break-away lasts in steering turns.
break_len:
  lda aiStep
  cmp #1
  beq :+
  lda #14
  rts
:
  lda #40
  rts

.proc enemy_update
  ; the bolt flies whether or not its owner survives
  lda alive+4
  beq noBolt
  dec boltLife
  bne :+
  lda #0
  sta alive+4
  jmp noBolt
:
  lda #4
  sta advI
  lda bv
  sta curV
  lda bv+1
  sta curV+1
  lda bv+2
  sta curV+2
  SCALEDT BOLT_SPEED
  sta advS
  jsr advance
noBolt:
  ; one ship thinks
  ldx aiNext
  stx ei
  lda alive,x
  beq :+
  lda hPassive,x
  bne :+
  jsr ai_steer
:
  ldx aiNext
  inx
  cpx aiCount
  bcc :+
  ldx #0
:
  stx aiNext
  ; all ships move
  ldx #0
move:
  stx ei
  lda alive,x
  beq next
  lda shipPosL,x
  sta hSaveL,x
  lda shipPosL+16,x
  sta hSaveL+16,x
  lda shipPosL+32,x
  sta hSaveL+32,x
  lda shipPosH,x
  sta hSaveH,x
  lda shipPosH+16,x
  sta hSaveH+16,x
  lda shipPosH+32,x
  sta hSaveH+32,x
  stx advI
  lda fvX,x
  sta curV
  lda fvY,x
  sta curV+1
  lda fvZ,x
  sta curV+2
  lda hBreak,x
  bne fast
  lda hTurn,x
  beq cruise
  lda dt
  jmp go
cruise:
  SCALEDT H_SPEED
  jmp go
fast:
  SCALEDT H_BREAK_SPEED
go:
  sta advS
  jsr advance
next:
  ldx ei
  inx
  cpx aiCount
  beq :+
  jmp move
:
  rts
.endproc

; (target - ship X) on one axis, wrapped, scaled to 8 bits by 32.
.macro TGTDELTA tgt, wOff, dest
  .local have
  ldx ei
  lda tgt
  sec
  sbc shipPosL+wOff,x
  sta u0
  lda tgt+1
  sbc shipPosH+wOff,x
  and #$0F
  cmp #$08
  bcc have
  ora #$F0
have:
  asl u0
  rol a
  asl u0
  rol a
  asl u0
  rol a
  sta dest
.endmacro

; Scouts hunt the player. Hale hunts the first scout alive, or stays near the player.
.proc ai_target
  lda ei
  cmp #HALE
  bne player
  ldx #0
  lda alive
  bne have
  inx
  lda alive+1
  beq player
have:
  lda shipPosL,x
  sta aiTX
  lda shipPosH,x
  sta aiTX+1
  lda shipPosL+16,x
  sta aiTY
  lda shipPosH+16,x
  sta aiTY+1
  lda shipPosL+32,x
  sta aiTZ
  lda shipPosH+32,x
  sta aiTZ+1
  rts
player:
  lda camPos+1
  sta aiTX
  lda camPos+2
  sta aiTX+1
  lda camPos+4
  sta aiTY
  lda camPos+5
  sta aiTY+1
  lda camPos+7
  sta aiTZ
  lda camPos+8
  sta aiTZ+1
  rts
.endproc

.macro ADDYAW v
  lda hYaw,x
  clc
  adc v
  sta hYaw,x
.endmacro

; Steers ship ei by the sign of the target's position in the ship's own axes.
.proc ai_steer
  ldy hYaw,x
  lda sinTab,y
  sta sa
  tya
  clc
  adc #64
  tay
  lda sinTab,y
  sta ca
  ldy hPitch,x
  lda sinTab,y
  sta sb
  tya
  clc
  adc #64
  tay
  lda sinTab,y
  sta cb
  lda sa
  ldx sb
  jsr fmul
  sta sasb
  lda sa
  ldx cb
  jsr fmul
  sta sacb
  lda ca
  ldx sb
  jsr fmul
  sta casb
  lda ca
  ldx cb
  jsr fmul
  sta cacb
  jsr ai_target
  TGTDELTA aiTX, 0, dDx
  TGTDELTA aiTY, 16, dDy
  TGTDELTA aiTZ, 32, dDz
  ; right = (ca, 0, -sa)
  lda ca
  ldx dDx
  jsr fmul
  sta eR
  lda #0
  sec
  sbc sa
  ldx dDz
  jsr fmul
  clc
  adc eR
  sta eR
  ; up = (sa*sb, cb, ca*sb)
  lda sasb
  ldx dDx
  jsr fmul
  sta eU
  lda cb
  ldx dDy
  jsr fmul
  clc
  adc eU
  sta eU
  lda casb
  ldx dDz
  jsr fmul
  clc
  adc eU
  sta eU
  ; forward = (sa*cb, -sb, ca*cb)
  ldx ei
  lda sacb
  sta fvX,x
  ldx dDx
  jsr fmul
  sta eF
  lda #0
  sec
  sbc sb
  ldx ei
  sta fvY,x
  ldx dDy
  jsr fmul
  clc
  adc eF
  sta eF
  ldx ei
  lda cacb
  sta fvZ,x
  ldx dDz
  jsr fmul
  clc
  adc eF
  sta eF
  ; close to the target: start a break-away
  ldx ei
  lda hBreak,x
  bne breaking
  ; true distance, not just the part along the nose
  ldy #2
: lda dDx,y
  bpl :+
  eor #$FF
  clc
  adc #1
:
  cmp #H_CLOSE
  bcs steer
  dey
  bpl :--
  jsr break_len
  sta hBreak,x
breaking:
  lda #0
  sta hTurn,x
  dec hBreak,x
  ; peel off in a fixed turn: steering away by error fails when dead ahead
  jsr break_len
  lsr a
  cmp hBreak,x
  bcs :+
  lda aiStep
  asl a
  asl a
  sta advI
  ADDYAW advI
:
  jmp pitchDone
steer:
  ; well off the nose: slow down and turn harder, or it just orbits its target
  lda #0
  sta hTurn,x
  lda eR
  bpl :+
  eor #$FF
  clc
  adc #1
:
  sta advI
  lda eU
  bpl :+
  eor #$FF
  clc
  adc #1
:
  clc
  adc advI
  sta advI
  lda eF
  bmi hardTurn
  cmp advI
  bcs :+
hardTurn:
  lda #1
  sta hTurn,x
:
  ; turn step for this update
  lda aiStep
  ldy hTurn,x
  beq :+
  asl a
:
  sta advS
  lda eR
  bmi left
  cmp #3
  bcc yawDone
  ADDYAW advS
  jmp yawDone
left:
  cmp #$FE
  bcs yawDone
  lda #0
  sec
  sbc advS
  sta advI
  ADDYAW advI
yawDone:
  ; dead astern neither side wins: pick one
  lda eF
  bpl :+
  lda eR
  clc
  adc #2
  cmp #5
  bcs :+
  ADDYAW aiStep
:
  lda eU
  bmi down
  cmp #3
  bcc pitchDone
  lda hPitch,x
  sec
  sbc aiStep
  sta hPitch,x
  jmp pitchDone
down:
  cmp #$FE
  bcs pitchDone
  lda hPitch,x
  clc
  adc aiStep
  sta hPitch,x
pitchDone:
  ; fire when the target is close to the nose
  ldx ei
  lda hCool,x
  beq :+
  dec hCool,x
  rts
:
  lda hBreak,x
  beq :+
  rts
:
  lda eF
  bpl :+
  rts
:
  cmp #H_CLOSE
  bcs :+
  rts
:
  lsr a
  lsr a
  lsr a
  clc
  adc #3               ; the steering dead zone must fit inside the firing cone
  sta advI             ; allowed error: 1 in 8, plus the dead zone
  lda eR
  bpl :+
  eor #$FF
  clc
  adc #1
:
  cmp advI
  bcc :+
  rts
:
  lda eU
  bpl :+
  eor #$FF
  clc
  adc #1
:
  cmp advI
  bcc :+
  rts
:
  lda ei
  cmp #HALE
  bne enemy
  jmp hale_shoots
enemy:
  lda alive+4
  beq :+
  rts
:
  ; launch from this ship, along its nose
  ldx ei
  lda fvX,x
  sta bv
  lda fvY,x
  sta bv+1
  lda fvZ,x
  sta bv+2
  lda shipPosL,x
  sta shipPosL+4
  lda shipPosH,x
  sta shipPosH+4
  lda shipPosL+16,x
  sta shipPosL+20
  lda shipPosH+16,x
  sta shipPosH+20
  lda shipPosL+32,x
  sta shipPosL+36
  lda shipPosH+32,x
  sta shipPosH+36
  lda #1
  sta alive+4
  lda #BOLT_LIFE
  sta boltLife
  jsr reload_len
  sta hCool,x
  inc statShots
  SFXREQ SFX_ENEMY
  lda #R_BOLT
  jsr say_radio
no:
  rts
.endproc

; Steering turns between shots.
reload_len:
  lda aiStep
  cmp #1
  beq :+
  lda #70              ; roughly eight seconds per scout at mission frame rates
  rts
:
  lda #H_RELOAD
  rts

; Hale's shots are not simulated. A good firing position wears a scout down,
; but only the player can finish it.
hale_shoots:
  lda #24
  ldx ei
  sta hCool,x
  ldx #0
  lda alive
  bne :+
  inx
  lda alive,x
  beq :+               ; nothing left to shoot at
:
  lda alive,x
  beq :+
  lda hp,x
  cmp #2
  bcc :+
  dec hp,x
  lda #R_HALE_HIT
  jsr say_radio
:
  rts

; Queues a line unless one has only just appeared. A = line number.
say_radio:
  ldx radioT
  cpx #90
  bcs :+
  sta radioReq
:
  rts

