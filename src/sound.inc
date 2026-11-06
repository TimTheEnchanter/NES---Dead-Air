; ---------- sound ----------
; Runs once per frame from the picture-bottom interrupt, where its length has no
; effect on when the picture starts.
; X = effect id. Called from the main loop; a single byte write hands it over.
play_sfx:
  lda sfxChan,x
  tay
  stx u0
  lda sfxPrio,x
  ldx sndReq,y
  cmp sfxPrio,x
  bcc :+
  lda u0
  sta sndReq,y
:
  rts

.macro SNDCH ch, base, pulseHi
  .local idle, go, stop, noreq, same, done
  ldx sndReq+ch
  beq noreq
  lda #0
  sta sndReq+ch
  lda sfxPrio,x
  cmp sndPrio+ch
  bcc noreq
  sta sndPrio+ch
  lda sfxLo,x
  sta sndPtr+ch*2
  lda sfxHi,x
  sta sndPtr+ch*2+1
  inc statSnd-1,x
.if pulseHi
  lda #$FF
  sta sndHi+ch
.endif
noreq:
  lda sndPtr+ch*2+1
  beq done
  ldy #0
  lda (sndPtr+ch*2),y
  beq stop
  sta base
  iny
  lda (sndPtr+ch*2),y
  sta base+2
.if pulseHi
  iny
  lda (sndPtr+ch*2),y
  cmp sndHi+ch
  beq same
  sta sndHi+ch
  sta base+3
same:
.else
  lda #0
  sta base+3
.endif
  lda sndPtr+ch*2
  clc
  adc #3
  sta sndPtr+ch*2
  bcc done
  inc sndPtr+ch*2+1
  jmp done
stop:
  lda #$30
  sta base
  lda #0
  sta sndPtr+ch*2+1
  sta sndPrio+ch
  sta sndEng           ; forces the engine sound back on
done:
.endmacro

; Called at 60 Hz from the interrupt, so short presses are never missed however
; long a game frame takes. Start is latched until the game consumes it.
read_pad:
  lda #1
  sta $4016
  lda #0
  sta $4016
  ldx #8
: lda $4016
  lsr a
  rol padNew
  dex
  bne :-
  lda padNew
  and #$10
  beq :+
  lda pad
  and #$10
  bne :+
  lda #1
  sta startLatch
:
  lda padNew
  and #$90
  beq :+
  lda pad
  and #$90
  bne :+
  lda #1
  sta confirmLatch
:
  lda padNew
  sta pad
  rts

; Called from the picture-bottom interrupt while the PPU is idle.
radio_tick:
  lda radioReq
  beq @timer
  ldx #0
  stx radioReq
  sec
  sbc #1
  ldx #0               ; ten lines of 24 fit in an index: page them
  cmp #10
  bcc :+
  sbc #10
  ldx #1
:
  stx radioPage
  asl a
  asl a
  asl a
  sta u1hold
  asl a
  adc u1hold           ; line * 24
  ldy #220
  sty radioT
  sta radioOff
  tax
  lda #$22
  jsr radio_row
  ldx radioOff
  lda #$26
  jmp radio_row
@timer:
  lda radioT
  beq @done
  dec radioT
  bne @done
  ldx #$FF             ; clear both nametables
  lda #$22
  jsr radio_row
  lda #$26
  jmp radio_row
@done:
  rts

; A = nametable high byte, X = offset into radioText, or $FF to blank the row.
radio_row:
  bit PPUSTATUS
  sta PPUADDR
  lda #$64             ; row 19, column 4
  sta PPUADDR
  ldy #24
  cpx #$FF
  bne :++
  lda #0
: sta PPUDATA
  dey
  bne :-
  rts
:
  lda radioPage
  bne :++
: lda radioText,x
  sta PPUDATA
  inx
  dey
  bne :-
  rts
: lda radioText+240,x
  sta PPUDATA
  inx
  dey
  bne :-
  rts

sound_tick:
  jsr read_pad
  SNDCH 0, $4000, 1
  SNDCH 1, $4004, 1
  SNDCH 2, $400C, 0
  jsr music_tick
  ; engine: a quiet low rumble that rises with speed, under the noise effects
  lda sndPtr+5
  bne @same
  ldx speed
  lda gameMode
  cmp #MODE_PLAY
  beq :+
  ldx #0
:
  inx
  cpx sndEng
  beq @same
  stx sndEng
  dex
  lda engCtl,x
  sta $400C
  lda engPer,x
  sta $400E
  lda #0
  sta $400F
@same:
  rts

; Triangle music: one note per step; 0 rests, 254 loops, 255 ends.
.proc music_tick
  ldx musReq
  beq run
  lda #0
  sta musReq
  cpx #1
  beq title
  cpx #2
  beq over
  ; stop
  sta musP+1
  lda #$80
  sta $4008
  rts
title:
  lda #<musTitle
  sta musP
  lda #>musTitle
  sta musP+1
  lda #10
  bne setLen
over:
  lda #<musOver
  sta musP
  lda #>musOver
  sta musP+1
  lda #14
setLen:
  sta musLen
  lda #1
  sta musWait
run:
  lda musP+1
  beq done
  dec musWait
  beq step
  ; a short gap before each note keeps them distinct
  lda musWait
  cmp #2
  bne done
  lda #$80
  sta $4008
done:
  rts
step:
  lda musLen
  sta musWait
  ldy #0
  lda (musP),y
  cmp #254
  bne :+
  lda #<musTitle
  sta musP
  lda #>musTitle
  sta musP+1
  lda (musP),y
:
  cmp #255
  bne :+
  lda #$80
  sta $4008
  lda #0
  sta musP+1
  rts
:
  tax
  inc musP
  bne :+
  inc musP+1
:
  cpx #0
  beq rest
  lda #$FF
  sta $4008
  lda noteLo,x
  sta $400A
  lda noteHi,x
  ora #$08
  sta $400B
  rts
rest:
  lda #$80
  sta $4008
  rts
.endproc
