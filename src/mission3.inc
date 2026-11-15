; ---------- Mission 6 pieces: the wreck, and what you protect ----------
.pushseg
.segment "CODE"
WRECK_HP  = 20
WRECK_FRE = $10        ; freighter byte on a defend step: protect the wreck

; The Vigil in her usual place, ready to take you home.
vigil_home:
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
  lda #0
  sta hPitch+VIGIL
  sta protSlot
  rts

; After setup: in an escort the shuttle is what you protect; in a wreck
; mission the wreck stands in the Vigil's slot until it is time to land.
mission_extras:
  ldx missionId
  lda tintTab,x
  sta tintColor
  lda missionMode
  cmp #3
  bne :+
  lda #0               ; flying alone
  sta alive+HALE
: cmp #2
  bne :+
  lda #HALE
  sta protSlot
  lda #80
  sta protR
  lda #SHUTTLE_HP
  sta shuttleHP
: ldx missionId
  lda wreckTab,x
  beq :++
  cmp #2
  bne :+
  ; the Array: a vast spire standing on end where the Vigil would be
  lda #<ARRAY_X
  sta shipPosL+VIGIL
  lda #>ARRAY_X
  sta shipPosH+VIGIL
  lda #<ARRAY_Y
  sta shipPosL+16+VIGIL
  lda #>ARRAY_Y
  sta shipPosH+16+VIGIL
  lda #<ARRAY_Z
  sta shipPosL+32+VIGIL
  lda #>ARRAY_Z
  sta shipPosH+32+VIGIL
  lda #3
  sta shipModel+VIGIL
  lda #64
  sta hPitch+VIGIL
  rts
: lda #<WRECK_X
  sta shipPosL+VIGIL
  lda #>WRECK_X
  sta shipPosH+VIGIL
  lda #<WRECK_Y
  sta shipPosL+16+VIGIL
  lda #>WRECK_Y
  sta shipPosH+16+VIGIL
  lda #<WRECK_Z
  sta shipPosL+32+VIGIL
  lda #>WRECK_Z
  sta shipPosH+32+VIGIL
  lda #100             ; adrift, nose down
  sta hYaw+VIGIL
  lda #30
  sta hPitch+VIGIL
: rts

; Carry clear if the enemy bolt is within protR of ship protSlot on one axis.
.macro PAXIS wOff
  .local pos, ok
  ldx protSlot
  lda shipPosL+4+wOff
  sec
  sbc shipPosL+wOff,x
  sta u0
  lda shipPosH+4+wOff
  sbc shipPosH+wOff,x
  and #$0F
  beq pos
  cmp #$0F
  beq :+
  jmp pmiss
: lda u0               ; behind: within if u0 + protR wraps past 256
  clc
  adc protR
  bcs ok
  jmp pmiss
pos:
  lda u0
  cmp protR
  bcc ok
  jmp pmiss
ok:
.endmacro

; Enemy bolts can hit what you are protecting: Ren's shuttle or the wreck.
.proc prot_hits
  ldx protSlot
  bne :+
  rts
: lda alive,x
  bne :+
  rts
: lda alive+4
  bne :+
  rts
:
  PAXIS 0
  PAXIS 16
  PAXIS 32
  lda #0
  sta alive+4
  SFXREQ SFX_CRASH
  dec shuttleHP
  bne :+
  lda #0               ; crippled: the mission is lost
  jmp end_mission
: ldx shuttleHP
  lda protSlot
  cmp #HALE
  bne wreck
  lda #R_RENHIT
  cpx #2
  bne say
  lda #R_RENFAIL
  bne say
wreck:
  lda #R_WRECKHIT
  cpx #5
  bne say
  lda #R_WRECKLOW
say:
  ldx #0
  stx radioT
  jmp say_radio
pmiss:
  rts
.endproc
.popseg
