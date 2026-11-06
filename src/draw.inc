.segment "CODE"
; ---------- tile stepping ----------
; mptr tracks the current map cell. All routines return the cell's tile in Y,
; allocating one on first touch, and preserve X.
tileRight:
  inc mptr
  jmp lookNow
tileLeft:
  dec mptr
  jmp lookNow
tileDown:
  lda mptr
  clc
  adc #32
  sta mptr
  bcc lookNow
  inc mptr+1
  jmp lookNow
tileUp:
  lda mptr
  sec
  sbc #32
  sta mptr
  bcs lookNow
  dec mptr+1
lookNow:
  ldy #0
  lda (mptr),y
  beq @alloc
  tay
  rts
@alloc:
  stx saveX
  ldx nextTile
  cpx #RADIO_TILE      ; the radio font lives above this
  bcs @full
  txa
  sta (mptr),y
  tay
  lda mptr
  sta (lpLo),y
  lda mptr+1
  sec
  sbc #MAP/256-$20
  sta (lpHi),y
  lda #0
  sta PAT+$000,x
  sta PAT+$100,x
  sta PAT+$200,x
  sta PAT+$300,x
  sta PAT+$400,x
  sta PAT+$500,x
  sta PAT+$600,x
  sta PAT+$700,x
  inc nextTile
  ldx saveX
  rts
@full:
  ldy #$FF        ; scratch tile: drawn into, never displayed
  ldx saveX
  rts

; Prepares one tile-wide segment of an x-major line. In: segS, cnt.
; Out: X = pixels in segment, segL = its last bit.
segInit:
  lda #8
  sec
  sbc segS
  cmp cnt
  bcc :+
  lda cnt
:
  tax
  sta u0
  lda cnt
  sec
  sbc u0
  sta cnt
  lda segS
  clc
  adc u0
  sta segL
  dec segL
  rts

.include "lines.inc"

; ---------- line drawing ----------
; Draws from x0,y0 to x1,y1.
.proc line
  lda x1
  sec
  sbc x0
  bcs :+
  eor #$FF
  adc #1
:
  sta dx
  lda y1
  sec
  sbc y0
  bcs :+
  eor #$FF
  adc #1
:
  sta dy
  cmp dx
  bcc xmajor
  jmp ymajor
xmajor:
  ; order left to right
  lda x0
  cmp x1
  bcc :+
  ldx x1
  stx x0
  sta x1
  lda y0
  ldx y1
  stx y0
  sta y1
:
  lda dx
  lsr a
  sta err
  ldx dx
  inx
  stx cnt
  jsr start
  lda x0
  and #7
  sta segS
  jsr segInit
  lda y0
  and #7
  asl a
  stx saveX
  tax
  lda y1
  cmp y0
  bcs xd
  lda xuTab,x
  sta vecM
  lda xuTab+1,x
  sta vecM+1
  ldx saveX
  jmp (vecM)
xd:
  lda xdTab,x
  sta vecM
  lda xdTab+1,x
  sta vecM+1
  ldx saveX
  jmp (vecM)
ymajor:
  ; order top to bottom
  lda y0
  cmp y1
  bcc :+
  ldx y1
  stx y0
  sta y1
  lda x0
  ldx x1
  stx x0
  sta x1
:
  lda dy
  lsr a
  sta err
  jsr start
  lda x0
  and #7
  tax
  lda bitTab,x
  sta mask
  lda y0
  and #7
  asl a
  tax
  lda x1
  cmp x0
  bcs yr
  lda ylTab,x
  sta vecM
  lda ylTab+1,x
  sta vecM+1
  jmp yGo
yr:
  lda yrTab,x
  sta vecM
  lda yrTab+1,x
  sta vecM+1
yGo:
  ldx dy
  inx
  jmp (vecM)
start:
  lda y0
  lsr a
  lsr a
  lsr a
  tax
  lda x0
  lsr a
  lsr a
  lsr a
  clc
  adc mapRowLo,x
  sta mptr
  lda mapRowHi,x
  sta mptr+1
  jmp lookNow
.endproc

.proc draw_edges
  ldx eLast
eloop:
  stx vj
  lda emask,x
  and vis
  beq skip
  ldy ea,x
  lda eb,x
  sta hyV
  sty hxV
  lda offscreen
  beq fast
  jsr edge_clipped
  jmp skip
fast:
  lda scrXL,y
  sta x0
  lda scrYL,y
  sta y0
  ldy hyV
  lda scrXL,y
  sta x1
  lda scrYL,y
  sta y1
  jsr line
skip:
  ldx vj
  cpx eFirst
  beq :+
  dex
  jmp eloop
:
  rts
.endproc

; Title and game-over overlays: wireframe text, and fewer sprites.
.proc draw_mode
  lda gameMode
  cmp #MODE_PLAY
  bne :+
  rts
:
  cmp #MODE_DONE
  bne :+
  lda doneWin
  beq lostText
  lda #<textWin
  sta textP
  lda #>textWin
  sta textP+1
  jmp hudText
lostText:
  lda #<textLose
  sta textP
  lda #>textLose
  sta textP+1
hudText:
  lda #$FF
  sta OAM              ; no crosshair over the result
  jmp text
:
  cmp #MODE_OVER
  beq over
  lda #<textTitle
  sta textP
  lda #>textTitle
  sta textP+1
  ; title: no game sprites at all
  ldx #0
  lda #$FF
: sta OAM,x
  inx
  inx
  inx
  inx
  cpx #104
  bne :-
  jmp text
over:
  lda #<textOver
  sta textP
  lda #>textOver
  sta textP+1
  lda #$FF
  sta OAM              ; hide the crosshair; the score stays
  ldx #72
: sta OAM,x            ; and the empty shield row
  inx
  inx
  inx
  inx
  cpx #84
  bne :-
text:
  ldy #0
  lda (textP),y
  cmp #255
  beq done
  sta x0
  iny
  lda (textP),y
  sta y0
  iny
  lda (textP),y
  sta x1
  iny
  lda (textP),y
  sta y1
  lda textP
  clc
  adc #4
  sta textP
  bcc :+
  inc textP+1
:
  jsr line
  jmp text
done:
  rts
.endproc

