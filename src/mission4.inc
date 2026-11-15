; ---------- Mission 7 pieces: Concord pilots, pickets, and lost fighters ----------
.pushseg
.segment "HIGH"
DETECT_R = 350         ; picket sees you inside this cube
WARN_R   = 600         ; Ren warns you inside this one
DETECT_EASY = 260      ; Easy: pickets see less far

; A Concord fighter at its last hit is disabled, not killed: it drifts, and
; can no longer be hurt. It counts as done for the wave.
concord_hit:
  lda hPassive,x
  beq :+
  lda #200             ; already disabled: it can't be hurt any more
  sta hp,x
  rts
: lda hp,x
  cmp #2
  bcs :+
  lda #1
  sta hPassive,x
  sta hTurn,x
  lda #0
  sta hBreak,x
  lda #200
  sta hp,x
  dec killsLeft
  lda #R_C_DIS
  ldx tintColor
  cpx #CONCORD_TINT
  beq cdSay
  lda #R_E_DIS         ; an echo, cut loose from the Array
cdSay:
  ldx #0
  stx radioT
  jsr say_radio
: rts

; One more spare fighter gone, kept in story flag bits 5-7.
lose_fighter:
  lda storyFlags
  cmp #(SPARE_START*32)
  bcs :+
  adc #32
  sta storyFlags
: rts

; Sneak step load: a picket (lancer hull, fixed, can't be shot) sits at the nav
; point plus (count*8, 0, hp*8), signed. Slot 2: nobody flies with you.
sneak_place:
  ldy #SP_COUNT
  lda (spadP),y
  ldx #0
  jsr picket_axis
  lda #0
  ldx #16
  jsr picket_axis
  ldy #SP_HP
  lda (spadP),y
  ldx #32
  jsr picket_axis
  lda #1
  sta alive+HALE
  lda #PICKET_MODEL    ; a lancer hull, drawn in the mission's tint (red)
  sta shipModel+HALE
  lda #2
  sta hPassive+HALE
  lda #64
  sta hYaw+HALE
  lda #0
  sta hPitch+HALE
  sta sneakSeen
  sta sneakWarned
  rts
; A = signed offset/8, X = axis: picket = nav + A*8
picket_axis:
  ldy #0
  cmp #$80
  bcc :+
  dey
: sty u0
  asl a
  rol u0
  asl a
  rol u0
  asl a
  rol u0
  clc
  adc shipPosL+NAVPT,x
  sta shipPosL+HALE,x
  lda u0
  adc shipPosH+NAVPT,x
  and #$0F
  sta shipPosH+HALE,x
  rts

; Any other step: in a solo mission, no picket is left behind.
sneak_clear:
  lda missionMode
  cmp #3
  bne :+
  lda #0
  sta alive+HALE
: rts

; Carry set when the camera is inside a cube of half-width t0/t1 around the
; ship in slot HALE.
near_picket:
  ldx #0
  ldy #1
@axis:
  lda shipPosL+HALE,x
  sec
  sbc camPos,y
  sta u0
  lda shipPosH+HALE,x
  sbc camPos+1,y
  and #$0F
  cmp #$08
  bcc @pos
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
@pos:
  cmp t1
  bcc @in
  bne @far
  lda u0
  cmp t0
  bcs @far
@in:
  iny
  iny
  iny
  txa
  clc
  adc #16
  tax
  cpx #48
  bne @axis
  sec
  rts
@far:
  clc
  rts

; Each frame while flying a sneak step: warn when close; if seen, two scouts
; launch from the picket and a spare fighter is lost.
sneak_tick:
  ldy #SP_KIND
  lda (spadP),y
  cmp #S_SNEAK
  bne @out
  lda sneakSeen
  bne @out
  lda alive+HALE
  bne :+
@out:
  rts
:
  jsr sneak_ping
  lda sneakWarned
  bne @detect
  lda #<WARN_R
  sta t0
  lda #>WARN_R
  sta t1
  jsr near_picket
  bcs :+
  rts
: inc sneakWarned
  lda #R_P_CLOSE
  ldx #0
  stx radioT
  jsr say_radio
@detect:
  lda #<DETECT_R
  ldx difficulty
  beq :+
  lda #<DETECT_EASY
: sta t0
  lda #>DETECT_R
  sta t1
  jsr near_picket
  bcs :+
  rts
: inc sneakSeen
  jsr lose_fighter
  lda #R_P_SEEN
  ldx #0
  stx radioT
  jsr say_radio
  ; two scouts from the picket
  ldx #1
@s:
  lda shipPosL+HALE
  clc
  adc launchX,x
  sta shipPosL,x
  lda shipPosH+HALE
  adc launchXH,x
  and #$0F
  sta shipPosH,x
  lda shipPosL+16+HALE
  sta shipPosL+16,x
  lda shipPosH+16+HALE
  sta shipPosH+16,x
  lda shipPosL+32+HALE
  sta shipPosL+32,x
  lda shipPosH+32+HALE
  sta shipPosH+32,x
  lda #1
  sta alive,x
  lda #3
  sta hp,x
  lda #0
  sta hBreak,x
  sta hPitch,x
  sta hTurn,x
  sta hPassive,x
  sta shipModel,x
  sta fvX,x
  sta fvY,x
  lda #127
  sta fvZ,x
  lda #45
  sta hCool,x
  dex
  bpl @s
  lda #2
  sta killsLeft
  lda #1
  sta mPhase
  lda #0
  sta mLimit
  sta mLimit+1
@done:
  rts
; Inside the warning range a ping repeats, faster the closer you are.
sneak_ping:
  lda #<WARN_R
  sta t0
  lda #>WARN_R
  sta t1
  jsr near_picket
  bcs :+
  rts
: lda #40              ; vblanks between pings, far out
  sta mU2
  lda #<450
  sta t0
  lda #>450
  sta t1
  jsr near_picket
  bcc :+
  lda #20
  sta mU2
  lda #<380
  sta t0
  lda #>380
  sta t1
  jsr near_picket
  bcc :+
  lda #10
  sta mU2
: lda pingT
  sec
  sbc dt
  sta pingT
  bcc :+
  cmp mU2              ; closing in: cut the wait short
  bcc :++
  lda mU2
  sta pingT
  rts
: lda mU2
  sta pingT
  SFXREQ SFX_PING
: rts

launchX:  .byte <(-180),<180
launchXH: .byte >(-180),>180
.popseg
