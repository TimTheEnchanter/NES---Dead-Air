; ---------- chapter select ----------
; Unlocked by finishing the story. Every save also records the flags each
; chapter started with, so a replayed chapter branches the way it did then;
; choices made in it carry forward as usual.
.pushseg
.segment "HIGH2"
CHAP_W = 25             ; each name: 24 characters and a line end
CHAP_N = 10             ; 8 chapters, Lantern Two, Back
CHAP_L2 = 8
F2_SPARED_BIT = 8

; A = resume point just saved. 0-6 start chapters 2-8; 7 is the end.
; saveDone: bit 0 the story is finished, bit 1 Lantern Two is unlocked
; (finished with the echo drone spared); saveDone+1 = saveDone eor $C3.
snap_save:
  cmp #7
  bcc @chapter
  jsr story_done
  lda #0
  bcc :+
  lda saveDone         ; keep an unlock already earned
: and #2
  ora #1
  tax
  lda storyFlags2
  and #F2_SPARED_BIT
  beq :+
  txa
  ora #2
  tax
: stx saveDone
  txa
  eor #$C3
  sta saveDone+1
  rts
@chapter:
  asl a
  tax
  lda storyFlags
  sta saveSnap,x
  lda storyFlags2
  sta saveSnap+1,x
  jsr snap_sum
  sta saveSnapSum
  rts

snap_sum:
  ldx #13
  lda #0
  clc
: adc saveSnap,x
  dex
  bpl :-
  eor #$3C
  rts

; Carry set if the story has been finished on this save.
story_done:
  lda saveDone
  eor #$C3
  cmp saveDone+1
  bne :+
  lda saveDone
  lsr a                ; bit 0 into carry
  rts
: clc
  rts

; Carry set if Lantern Two is unlocked.
l2_open:
  jsr story_done
  bcc :+
  lda saveDone
  lsr a
  lsr a                ; bit 1 into carry
: rts

; Script: CHAPSEL, CHAP_N names of CHAP_W bytes (8 chapters, Lantern Two,
; Back), then .word chapter-one target, back target, Lantern Two target.
chapter_pick:
  lda textP
  sta chapBase
  lda textP+1
  sta chapBase+1
  lda saveArea+8       ; replays keep the saved difficulty
  eor #$5A
  cmp saveArea+9
  beq :+
  lda #0
  sta saveArea+8
: lda saveArea+8
  sta difficulty
  lda #0
  sta chapSel
  sta sSel
  jsr choice_arrow     ; the arrow sits on row 20
  jsr chap_draw
  lda #0
  sta confirmLatch
  sta spad0
@wait:
  jsr wait_tick
  lda pad
  and #$0C             ; up or down
  tax
  cpx spad0
  beq @still
  stx spad0
  cpx #0
  beq @still
  cpx #$08             ; up
  bne @down
@up:
  dec chapSel
  bpl :+
  lda #CHAP_N-1
  sta chapSel
: jsr l2_skip
  bcs @up
  bcc @moved
@down:
  inc chapSel
  lda chapSel
  cmp #CHAP_N
  bcc :+
  lda #0
  sta chapSel
: jsr l2_skip
  bcs @down
@moved:
  ldx #SFX_BLIP
  jsr play_sfx
  jsr chap_draw
@still:
  lda confirmLatch
  beq @wait
  lda #0
  sta confirmLatch
  jsr clear_box
  ; the two targets follow the names
  lda chapBase
  clc
  adc #<(CHAP_W*CHAP_N)
  sta textP
  lda chapBase+1
  adc #>(CHAP_W*CHAP_N)
  sta textP+1
  lda chapSel
  beq @first
  cmp #CHAP_N-1
  beq @back
  cmp #CHAP_L2
  beq @lantern2
  ; chapters 2-8: their saved flags, then the scene
  sec
  sbc #1
  sta spad0            ; resume point 0-6
  jsr snap_sum
  cmp saveSnapSum
  beq @snapOk
  ldx #13              ; damaged: start clean rather than branch on noise
  lda #0
@wipe:
  sta saveSnap,x
  dex
  bpl @wipe
  jsr snap_sum
  sta saveSnapSum
@snapOk:
  lda spad0
  asl a
  tax
  lda saveSnap,x
  sta storyFlags
  lda saveSnap+1,x
  sta storyFlags2
  lda spad0
  asl a
  adc spad0
  tax
  lda saveTab,x
  sta scriptBank
  lda saveTab+1,x
  sta textP
  lda saveTab+2,x
  sta textP+1
  lda scriptBank
  jsr map_bank
  jmp story_loop
@lantern2:
  lda #0               ; the flashback carries no flags
  sta storyFlags
  sta storyFlags2
  jsr next_byte        ; skip two targets
  jsr next_byte
@back:
  jsr next_byte        ; skip the chapter-one target
  jsr next_byte
@first:
  jsr next_byte
  pha
  jsr next_byte
  sta textP+1
  pla
  sta textP
  jmp story_loop

; Carry set if chapSel is on Lantern Two while it is still locked.
l2_skip:
  lda chapSel
  cmp #CHAP_L2
  bne :+
  jsr l2_open
  bcs :+
  sec
  rts
: clc
  rts

; Draws the selected name on row 20.
chap_draw:
  lda textP
  pha
  lda textP+1
  pha
  lda chapSel
  tax
  lda chapBase
  sta textP
  lda chapBase+1
  sta textP+1
  cpx #0
  beq @at
@skip:
  lda textP
  clc
  adc #CHAP_W
  sta textP
  bcc @noCarry
  inc textP+1
@noCarry:
  dex
  bne @skip
@at:
  ; the whole name in one vblank, so quick presses are never missed
  ldy #23
@copy:
  lda (textP),y
  sta qBuf,y
  dey
  bpl @copy
  lda #5
  sta sCol
  lda #20
  sta sRow
  jsr set_addr
  lda #24
  sta qLen
  lda #3
  sta qJob
  jsr wait_job
  pla
  sta textP+1
  pla
  sta textP
  rts
.popseg
