.segment "HIGH"
; ---------- PPU transfer state machine ----------
; Runs inside IRQ (bottom of picture) and NMI. Spends at most `bud` cycles.
.macro SPEND k
  lda bud
  sec
  sbc #<(k)
  sta bud
  lda bud+1
  sbc #>(k)
  sta bud+1
.endmacro

.proc run_xfer
top:
  lda xfState
  bne go
  lda drawDone
  bne takeJob
exit:
  rts
takeJob:
  lda #0
  sta drawDone
  lda jobIdx
  sta xfList
  lda jobCnt
  sta xfCnt
  lda #1
  sta xfI
  lda #ST_PAT
  sta xfState
  SPEND K_TAKE
go:
  lda xfState
  cmp #ST_FLIP
  beq exit
  lda bud+1            ; need room for a chunk's charge plus one item, or a
  cmp #2               ; charge with no upload would start the picture early
  bcc exit
  ; items left in this state
  ldx xfState
  lda xfCnt
  cpx #ST_CLEAR
  bne :+
  lda oldCnt
:
  sec
  sbc xfI
  bne :+
  jmp nextState
:
  sta xn
  cpx #ST_PAT
  beq patCh
  ; --- nametable chunk: 32 cycles per item
ntCh:
  SPEND K_NT
  jsr budUnits
  lsr a
  lsr a
  cmp xn
  bcs :+
  sta xn
:
  lda xn
  bne :+
  rts
:
  ldx xfState
  cpx #ST_CLEAR
  bne :+
  lda oldList
  asl a
  tax
  lda clearVec,x
  sta vec
  lda clearVec+1,x
  sta vec+1
  jmp ntRun
:
  lda xfList
  asl a
  tax
  lda newVec,x
  sta vec
  lda newVec+1,x
  sta vec+1
ntRun:
  ldx xfI
  ldy xn
  jsr tramp
  lda xn
  jsr sub32
  jmp advance
  ; --- pattern chunk: 96 cycles per item
patCh:
  SPEND K_PAT
  jsr budUnits
  lsr a
  lsr a
  tax
  lda div3,x
  cmp xn
  bcs :+
  sta xn
:
  lda xn
  bne :+
  rts
:
patRun:
  ldx xfI
  ldy xn
  jsr pat_loop
  lda xn
  jsr sub32
  lda xn
  jsr sub32
  lda xn
  jsr sub32
advance:
  lda xfI
  clc
  adc xn
  sta xfI
  jmp top
nextState:
  lda xfState
  cmp #ST_PAT
  bne :+
  ; CPU buffer is now free: the main loop may draw the next frame
  lda #ST_NEW
  sta xfState
  lda #1
  sta xfI
  sta bufFree
  SPEND K_NEXT
  jmp top
:
  cmp #ST_NEW
  bne :+
  lda #ST_FLIP
  sta xfState
  rts
:
  ; end of ST_CLEAR
  lda #ST_IDLE
  sta xfState
  SPEND K_NEXT
  jmp top
tramp:
  jmp (vec)
.endproc

; A = min(255, bud/8)
budUnits:
  lda bud+1
  cmp #8
  bcc :++
  ldx #5               ; same length as the path below, for stable NMI timing
: dex
  bne :-
  nop
  nop
  lda #255
  rts
:
  sta u1hold
  lda bud
  lsr u1hold
  ror a
  lsr u1hold
  ror a
  lsr u1hold
  ror a
  rts

; A*32 subtracted from bud
sub32:
  sta xnt
  lsr a
  lsr a
  lsr a
  sta xnh
  lda xnt
  asl a
  asl a
  asl a
  asl a
  asl a
  sta xnt
  lda bud
  sec
  sbc xnt
  sta bud
  lda bud+1
  sbc xnh
  sta bud+1
  rts

.macro NTLOOPS name, pg, clearing
.segment "LOOPS"
.align 64
name:
: lda 256*(pg+1),x
  ora ntOr
  sta PPUADDR
  lda 256*(pg),x
  sta PPUADDR
.if clearing
  lda #0
  sta PPUDATA
.else
  stx PPUDATA
  nop
.endif
  inx
  dey
  bne :-
  rts
.segment "HIGH"
.endmacro
NTLOOPS clear0, LISTS+0, 1
NTLOOPS clear1, LISTS+2, 1
NTLOOPS clear2, LISTS+4, 1
NTLOOPS new0,   LISTS+0, 0
NTLOOPS new1,   LISTS+2, 0
NTLOOPS new2,   LISTS+4, 0
clearVec: .word clear0,clear1,clear2
newVec:   .word new0,new1,new2

.segment "LOOPS"
.align 64
pat_loop:
: lda ppuHi,x
  sta PPUADDR
  lda ppuLo,x
  ora planeOr
  sta PPUADDR
  lda PAT+$000,x
  sta PPUDATA
  lda PAT+$100,x
  sta PPUDATA
  lda PAT+$200,x
  sta PPUDATA
  lda PAT+$300,x
  sta PPUDATA
  lda PAT+$400,x
  sta PPUDATA
  lda PAT+$500,x
  sta PPUDATA
  lda PAT+$600,x
  sta PPUDATA
  lda PAT+$700,x
  sta PPUDATA
  nop
  nop
  nop
  inx
  dey
  bne :-
  rts
.segment "HIGH"

.proc set_back
  ; hidden nametable bit and bitplane follow the displayed one
  lda front
  eor #1
  tax
  asl a
  asl a
  sta ntOr
  txa
  asl a
  asl a
  asl a
  sta planeOr
  rts
.endproc

