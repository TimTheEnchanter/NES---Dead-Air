; ---------- music ----------
; Two voices: triangle bass and pulse 1 melody, one step per tempo tick.
; Runs in the interrupts, so it and its data live in fixed space (HIGH2).
; musReq: track number to start (ignored if it is already playing), 128 stops.
.pushseg
.segment "HIGH2"
PULSE_VOL = $B5        ; 50% duty, constant volume 5

.proc music_tick
  ldx musReq
  beq run
  lda #0
  sta musReq
  cpx #128
  bcc start
stop:
  lda #0
  sta musP+1
  sta musCur
  lda #$80
  sta $4008
  lda #$30
  sta $4000
  rts
start:
  cpx musCur
  bne :+
  lda musP+1
  bne run              ; already playing: carry on
: stx musCur
  lda trackLo,x
  sta musP
  sta musStart
  lda trackHi,x
  sta musP+1
  sta musStart+1
  lda trackTempo,x
  sta musLen
  lda #1
  sta musWait
run:
  lda musP+1
  bne :+
  rts
: dec musWait
  beq step
  ; a short gap before each new note, unless the next step holds it
  lda musWait
  cmp #2
  bne done
  ldy #0
  lda (musP),y
  cmp #MUS_HOLD
  beq :+
  lda #$80
  sta $4008
: iny
  lda (musP),y
  cmp #MUS_HOLD
  beq done
  lda #$30
  sta $4000
done:
  rts
step:
  lda musLen
  sta musWait
  ldy #0
  lda (musP),y
  cmp #MUS_LOOP
  bne :+
  lda musStart
  sta musP
  lda musStart+1
  sta musP+1
  lda (musP),y
: cmp #MUS_END
  bne :+
  jmp stop
: cmp #MUS_HOLD
  beq tdone
  tax
  bne :+
  lda #$80
  sta $4008
  jmp tdone
: lda #$FF
  sta $4008
  lda triLo,x
  sta $400A
  lda triHi,x
  sta $400B
tdone:
  ldy #1
  lda (musP),y
  cmp #MUS_HOLD
  beq pdone
  tax
  bne :+
  lda #$30
  sta $4000
  jmp pdone
: lda #PULSE_VOL
  sta $4000
  lda #$08             ; no sweep
  sta $4001
  lda pulLo,x
  sta $4002
  lda pulHi,x
  sta $4003
pdone:
  lda musP
  clc
  adc #2
  sta musP
  bcc :+
  inc musP+1
: rts
.endproc
.popseg
