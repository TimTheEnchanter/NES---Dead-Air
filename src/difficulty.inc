; ---------- difficulty ----------
; difficulty: 0 Normal, 1 Easy. Chosen at a new game, kept in the save.
.pushseg
.segment "HIGH2"
; A shield hit. On Easy every other hit is shrugged off. Z set if none left.
take_hit:
  lda difficulty
  beq :+
  lda easyHalf
  eor #1
  sta easyHalf
  beq :+
  lda #1               ; this one is free: Z clear
  rts
: dec shields
  rts

; A = a timer's high byte: on Easy, three more units (about 13 seconds).
easy_more:
  ldx difficulty
  beq :+
  clc
  adc #3
: rts

; A = health: on Easy, half as much again.
easy_half:
  ldx difficulty
  beq :+
  sta u0
  lsr a
  clc
  adc u0
: rts
.popseg
