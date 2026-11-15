.segment "CODE"
; ---------- tile lookup ----------
; mptr tracks the current map cell. lookNow returns the cell's tile in Y,
; allocating one on first touch, and preserves X. The line blocks in
; lines.inc inline the lookup and call lookAlloc (Y = 0) or allocX only for
; a cell with no tile yet.
lookNow:
  ldy #0
  lda (mptr),y
  beq lookAlloc
  tay
  rts
lookAlloc:
  lda gateOpen
  beq @gate
@go:
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
.ifdef DEBUG
  cmp #$20
  bcc @bad
  cmp #$24
  bcc @okHi
@bad:
  inc dbgBad
@okHi:
.endif
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
@gate:
  jsr wait_buf
  jmp @go

; Waits until the interrupt side has uploaded the last frame's tiles, then
; claims the tile buffer for this frame. Keeps X and Y.
wait_buf:
@w:
  lda bufFree
  beq @w
  lda #0
  sta bufFree
  lda #1
  sta gateOpen
  rts

; allocX: lookAlloc for the x-major blocks, which keep the tile in X and
; the pixel count in Y.
allocX:
  sty saveY
  ldy #0
  jsr lookAlloc
  tya
  tax
  ldy saveY
  rts

; Next tile-wide segment of an x-major line, starting at bit 0.
; Out: Y = pixels in it, mrp set for its last bit, prev = 0, cnt reduced.
segNext:
  lda #0
  sta prev
  lda cnt
  cmp #8
  bcc @short
  sbc #8
  sta cnt
  ldy #8
  lda #<maskQ          ; last bit 7
  sta mrp
  rts
@short:
  tay
  lda #<(maskQ+8)      ; last bit cnt-1
  sec
  sbc cnt
  sta mrp
  lda #0
  sta cnt
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
  sty saveX            ; the tile, held in X by the blocks
  lda #>maskQ
  sta mrp+1
  ; first segment: from bit x0&7 to the tile edge or the line's end
  lda x0
  and #7
  tay
  lda maskL,y
  eor #$FF
  sta prev             ; bits left of the first pixel count as drawn
  sty u0
  lda #8
  sec
  sbc u0
  cmp cnt
  bcc :+
  lda cnt
: sta u1               ; pixels in the segment
  lda cnt
  sec
  sbc u1
  sta cnt
  lda #<(maskQ+8)      ; mrp = maskQ + 7 - last bit
  sec
  sbc u0
  sec
  sbc u1
  sta mrp
  lda y0
  and #7
  asl a
  tax
  lda y1
  cmp y0
  bcs xd
  lda xuTab,x
  sta vecM
  lda xuTab+1,x
  sta vecM+1
  jmp xGo
xd:
  lda xdTab,x
  sta vecM
  lda xdTab+1,x
  sta vecM+1
xGo:
  ldx saveX
  ldy u1
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
  sec                  ; the blocks keep carry set between pixels
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

