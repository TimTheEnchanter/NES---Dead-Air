; ---------- echo tint ----------
; The echo drone shows in its own colour by switching the 16x16 squares it
; covers to background palette 1. The picture sits at nametable column 4, row 3,
; 192x128 pixels: attribute bytes rows 0-4, columns 1-6 (30 bytes). The radio
; row shares the bottom squares, so they are never tinted.
.pushseg
.segment "MATH"
ECHO_MODEL = 16
CONCORD_MODEL = 32
CARRIER_MODEL = $0A
PICKET_MODEL = $38      ; lancer shape (bit 3), tinted (bits 4-5)
ATTR_COST  = 450       ; IRQ cycles kept back for write_attrs

; Frame start: forget last frame's squares while a tint is fading out.
tint_start:
  lda tintFrames
  beq :++
  dec tintFrames
  ldx #29
  lda #0
: sta attrWork,x
  dex
  bpl :-
:
  rts

; The drone was just transformed: tint the squares under its vertices.
.proc tint_mark
  lda #3               ; keep sending for a few frames after it goes
  sta tintFrames
  lda #255
  sta tintX0
  sta tintY0
  lda #0
  sta tintX1
  sta tintY1
  ldx vFirst
vl:
  lda behind,x
  bne nextV
  ; clamp to the picture
  lda scrXH,x
  bmi xlo
  bne xhi
  lda scrXL,x
  cmp #192
  bcc xok
xhi:
  lda #191
  bne xok
xlo:
  lda #0
xok:
  cmp tintX0
  bcs :+
  sta tintX0
: cmp tintX1
  bcc :+
  sta tintX1
:
  lda scrYH,x
  bmi ylo
  bne yhi
  lda scrYL,x
  cmp #127
  bcc yok
yhi:
  lda #126
  bne yok
ylo:
  lda #1
yok:
  cmp tintY0
  bcs :+
  sta tintY0
: cmp tintY1
  bcc :+
  sta tintY1
:
nextV:
  inx
  cpx vEnd
  bne vl
  lda tintX0
  cmp #255
  bne :+
  rts                  ; nothing in front of the camera
:
  ; squares: X = (x+32)/16, Y = (y+24)/16, kept above the radio row
  lda tintX0
  clc
  adc #32
  lsr a
  lsr a
  lsr a
  lsr a
  sta tintX0
  lda tintX1
  clc
  adc #32
  lsr a
  lsr a
  lsr a
  lsr a
  sta tintX1
  lda tintY0
  clc
  adc #24
  lsr a
  lsr a
  lsr a
  lsr a
  sta tintY0
  lda tintY1
  clc
  adc #24
  lsr a
  lsr a
  lsr a
  lsr a
  cmp #9
  bcc :+
  lda #8
: sta tintY1
  ldy tintY0
rowL:
  ldx tintX0
colL:
  ; byte (y/2)*6 + x/2 - 1, bits ((y&1)*2 + (x&1))*2
  tya
  lsr a
  sta u0
  asl a
  adc u0
  asl a                ; (y/2)*6
  sta u0
  txa
  lsr a
  clc
  adc u0
  sec
  sbc #1
  sta u0
  tya
  and #1
  asl a
  sta u1
  txa
  and #1
  ora u1
  stx u1
  tax
  lda tintBits,x
  ldx u0
  ora attrWork,x
  sta attrWork,x
  ldx u1
  inx
  cpx tintX1
  beq colL
  bcc colL
  iny
  cpy tintY1
  beq rowL
  bcc rowL
  rts
.endproc
tintBits: .byte $01,$04,$10,$40

; Frame done: hand the squares to the interrupt side.
tint_send:
  lda tintFrames
  sta tintSend
  beq :++
  ldx #29
: lda attrWork,x
  sta attrSend,x
  dex
  bpl :-
:
  rts

; In the picture-bottom interrupt, with rendering off: write the squares into
; the hidden nametable, which the next flip will show.
; Below the picture, with rendering off: set the tint palette for whichever
; buffer the next NMI will show. It flips when the frame is ready and at least
; two vblanks have passed, the same test the NMI makes.
tint_palette:
  ldx front
  lda xfState
  cmp #ST_FLIP
  bne :+
  lda sinceFlip
  beq :+
  txa
  eor #1
  tax
: bit PPUSTATUS
  lda #$3F
  sta PPUADDR
  lda #$05
  sta PPUADDR
  lda tintColor        ; this mission's tint: colour 1 or 2 by buffer, and 3
  ldy #$0F
  cpx #0
  beq :+
  lda #$0F
  ldy tintColor
: sta PPUDATA
  sty PPUDATA
  lda tintColor
  sta PPUDATA
  lda #0               ; out of palette space, or the blank lines take its colour
  sta PPUADDR
  sta PPUADDR
  rts

write_attrs:
  lda front
  eor #1
  asl a
  asl a
  ora #$23
  sta attrHi
  bit PPUSTATUS
  ldx #0
  ldy #$C1
@row:
  lda attrHi
  sta PPUADDR
  sty PPUADDR
  .repeat 6
  lda attrSend,x
  sta PPUDATA
  inx
  .endrep
  tya
  clc
  adc #8
  tay
  cpy #$C1+40
  bne @row
  rts
.popseg
