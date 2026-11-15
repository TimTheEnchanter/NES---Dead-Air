; ---------- radio lines ----------
; Main thread only. Maps the radio bank, copies line A into radioBuf, maps the
; flight bank back, then flags the line for the picture-bottom interrupt.
; Interrupts never read banked data, so the brief switch is safe.
.pushseg
.segment "HIGH"
BANK_RADIO = 6
radio_fetch:
  tax
  lda #0
  sta radioReq         ; the interrupt must not read a half-copied line
  lda #BANK_RADIO
  jsr map_bank
  lda radioLo,x
  sta textP
  lda radioHi,x
  sta textP+1
  ldy #23
: lda (textP),y
  sta radioBuf,y
  dey
  bpl :-
  lda #BANK_FLIGHT
  jsr map_bank
  lda #1
  sta radioReq
  rts
.popseg
