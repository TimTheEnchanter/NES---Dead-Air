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
  cpx #8               ; the stats keep seven counters; story flags follow
  bcs :+
  inc statSnd-1,x
:
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
  lda #0
  sta radioReq
  ldy #220
  sty radioT
  sty radioBusy
  ldx #0
  lda #$22
  jsr radio_row
  ldx #0
  lda #$26
  jmp radio_row
@timer:
  lda radioT
  beq @done
  dec radioT
  bne @done
  lda #1
  sta radioBusy
  ldx #$FF             ; clear both nametables
  lda #$22
  jsr radio_row
  lda #$26
  jmp radio_row
@done:
  rts

; A = nametable high byte, X = 0 to write radioBuf, or $FF to blank the row.
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
: lda radioBuf,x
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

