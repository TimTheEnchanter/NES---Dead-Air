; ---------- ROM banks ----------
; $8000: tables, always bank 0. $A000: flight code or story data. $C000 up: fixed.
; Only the main loop switches banks; the interrupts never do.
BANK_FLIGHT = 1
BANK_ASSETS = 2
BANK_SCRIPT = 3
map_bank:
  pha
  lda #7
  sta $8000
  pla
  sta $8001
  rts

; ---------- story screens ----------
; A separate display mode: normal full-screen rendering, no raster tricks. The
; main loop queues one job; the NMI runs it. Leaving rebuilds flight from scratch.
ppu_copy:
  ldy #0
: lda (spadP),y
  sta PPUDATA
  iny
  bne :-
  inc spadP+1
  dex
  bne :-
  rts

.proc story_enter
  sei
  lda #0
  sta PPUCTRL
  sta PPUMASK
  sta $E000
  ldx #$FF
  txs
  lda #128
  sta musReq
  lda #MODE_STORY
  sta gameMode
  jsr sound_stop_engine
  lda #BANK_ASSETS
  jsr map_bank
  ; patterns: frame tiles from 1, font from 32, portraits from 128
  bit PPUSTATUS
  lda #$00
  sta PPUADDR
  lda #$10
  sta PPUADDR
  ldx #0
: lda storyMisc,x
  sta PPUDATA
  inx
  cpx #160
  bne :-
  lda #$02
  sta PPUADDR
  lda #$00
  sta PPUADDR
  lda #<storyFont
  sta spadP
  lda #>storyFont
  sta spadP+1
  lda #96
  sta sCnt
font:
  ldy #0
: lda (spadP),y
  sta PPUDATA
  iny
  cpy #8
  bne :-
  lda #0
  ldy #8
: sta PPUDATA
  dey
  bne :-
  lda spadP
  clc
  adc #8
  sta spadP
  bcc :+
  inc spadP+1
:
  dec sCnt
  bne font
  ; portrait tiles
  lda #<storyPort
  sta spadP
  lda #>storyPort
  sta spadP+1
  lda #PORT_TILES
  sta sCnt
port:
  ldy #0
: lda (spadP),y
  sta PPUDATA
  iny
  cpy #16
  bne :-
  lda spadP
  clc
  adc #16
  sta spadP
  bcc :+
  inc spadP+1
:
  dec sCnt
  bne port
  ; blank nametable, then the layout
  lda #$20
  sta PPUADDR
  lda #$00
  sta PPUADDR
  lda #0
  ldx #4
  ldy #0
: sta PPUDATA
  iny
  bne :-
  dex
  bne :-
  lda #<storyRuns
  sta spadP
  lda #>storyRuns
  sta spadP+1
run:
  jsr run_byte
  beq runsDone
  sta PPUADDR
  jsr run_byte
  sta PPUADDR
  jsr run_byte
  tax
: jsr run_byte
  sta PPUDATA
  dex
  bne :-
  jmp run
runsDone:
  lda #$23
  sta PPUADDR
  lda #$C0
  sta PPUADDR
  ldx #0
: lda storyAttr,x
  sta PPUDATA
  inx
  cpx #64
  bne :-
  ; palettes: grey text, then the portrait
  lda #$3F
  sta PPUADDR
  lda #$00
  sta PPUADDR
  ldx #0
: lda storyBgPal,x
  sta PPUDATA
  inx
  cpx #8
  bne :-
  ; no sprites here
  lda #$FF
  ldx #0
: sta OAM,x
  inx
  bne :-
  lda #>OAM
  sta OAMDMA
  lda scriptBank
  jsr map_bank
  lda resumePtr
  sta textP
  lda resumePtr+1
  sta textP+1
  lda #3
  sta sCol
  lda #16
  sta sRow
  lda #0
  sta qJob
  sta confirmLatch
  lda #1
  sta storyFlag
: bit PPUSTATUS
  bpl :-
  lda #0
  sta PPUSCROLL
  sta PPUSCROLL
  lda #CTRLV
  sta PPUCTRL
  lda #%00001010
  sta PPUMASK
  jmp story_loop
.endproc

run_byte:
  ldy #0
  lda (spadP),y
  inc spadP
  bne :+
  inc spadP+1
:
  cmp #0
  rts

storyBgPal: .byte $0F,$30,$10,$00, $0F,$0F,$0F,$0F

sound_stop_engine:
  lda #$30
  sta $400C
  lda #0
  sta sndEng
  rts

wait_tick:
  lda tick
: cmp tick
  beq :-
  rts

wait_job:
  lda qJob
  bne wait_job
  rts

; A = character, written at the cursor.
put_char:
  sta qBuf
  jsr set_addr
  lda #1
  sta qLen
  lda #3
  sta qJob
  jsr wait_job
  inc sCol
  rts

; qAddr from sCol, sRow
set_addr:
  lda sRow
  lsr a
  lsr a
  lsr a
  ora #$20
  sta qAddr+1
  lda sRow
  asl a
  asl a
  asl a
  asl a
  asl a
  ora sCol
  sta qAddr
  rts

next_byte:
  ldy #0
  lda (textP),y
  inc textP
  bne :+
  inc textP+1
:
  rts

.proc story_loop
  jsr next_byte
  cmp #32
  bcc :+
  jmp char
:
  cmp #7
  bcc :+
  jmp more_ops
:
  cmp #1
  bne :+
  jmp speak
:
  cmp #2
  bne :+
  jmp nl
:
  cmp #3
  bne :+
  jmp page
:
  cmp #4
  bne :+
  jmp choice
:
  cmp #5
  beq flag
  cmp #0
  bne :+
  jmp story_loop       ; END is unused: scenes leave through FLY or TITLE
:
  ; goto
  jsr next_byte
  pha
  jsr next_byte
  sta textP+1
  pla
  sta textP
  jmp story_loop
flag:
  jsr next_byte
  ora storyFlags
  sta storyFlags
  jmp story_loop
char:
  pha
  jsr put_char
  pla
  cmp #32
  beq :+
  lda sCol
  and #3
  bne :+
  ldx #SFX_BLIP
  jsr play_sfx
:
  lda pad
  and #$90
  bne story_loop       ; holding A or Start speeds the text up
  jsr wait_tick
  jsr wait_tick
  jmp story_loop
nl:
  lda #3
  sta sCol
  inc sRow
  inc sRow
  jmp story_loop
speak:
  jsr next_byte
  sta sPort
  lda #2
  sta qJob
  jsr wait_job
  jmp story_loop
page:
  lda #0
  sta confirmLatch
wait:
  lda tick
  and #16
  beq :+
  lda #9
  bne :++
:
  lda #0
:
  sta qBuf
  jsr cursor
  lda confirmLatch
  beq wait
  lda #0
  sta qBuf
  jsr cursor
  jsr clear_box
  jmp story_loop
cursor:
  lda #$22
  sta qAddr+1
  lda #$FC             ; row 23, column 28
  sta qAddr
  lda #1
  sta qLen
  lda #3
  sta qJob
  jmp wait_job
.endproc

; Script ops that change what the game is doing.
.proc more_ops
  cmp #7
  bne :+
  jsr next_byte        ; FLY mission, win scene, lose scene
  sta missionId
  jsr next_byte
  sta winPtr
  jsr next_byte
  sta winPtr+1
  jsr next_byte
  sta losePtr
  jsr next_byte
  sta losePtr+1
  jmp enter_flight
:
  cmp #8
  bne :+
  jsr next_byte        ; SAVE point
  jsr write_save
  jmp story_loop
:
  cmp #9
  bne :+
  lda saveArea+4       ; LOAD
  sta storyFlags
  lda saveArea+3
  asl a
  adc saveArea+3
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
:
  cmp #10
  bne :+
  jsr next_byte        ; IF flag, scene
  sta spad0
  jsr next_byte
  sta sTargA
  jsr next_byte
  sta sTargA+1
  lda storyFlags
  and spad0
  beq :++
  lda sTargA
  sta textP
  lda sTargA+1
  sta textP+1
  jmp story_loop
:
  cmp #11
  bne :++
  jsr next_byte        ; LOC name
  jsr show_location
:
  jmp story_loop
:
  cmp #14
  bne :+
  jsr next_byte        ; CHAPTER bank, scene: jump across banks
  sta spad0
  jsr next_byte
  sta sTargA
  jsr next_byte
  sta sTargA+1
  lda spad0
  sta scriptBank
  jsr map_bank
  lda sTargA
  sta textP
  lda sTargA+1
  sta textP+1
  jmp story_loop
:
  cmp #12
  bne :+
  lda #0               ; CLEAR flags
  sta storyFlags
  jmp story_loop
:
  ; TITLE
  lda #0
  sta missionId
  jmp enter_title
.endproc

show_location:
  sta spad0
  asl a
  asl a
  sta u0
  asl a
  adc u0
  adc spad0            ; index * 13
  tax
  ldy #0
: lda locNames,x
  sta qBuf,y
  inx
  iny
  cpy #13
  bne :-
  lda #$21
  sta qAddr+1
  lda #$2E             ; row 9, column 14
  sta qAddr
  lda #13
  sta qLen
  lda #3
  sta qJob
  jmp wait_job

; Save record: 'D','A', version, resume point, flags, checksum.
save_sum:
  lda saveArea
  clc
  adc saveArea+1
  adc saveArea+2
  adc saveArea+3
  adc saveArea+4
  eor #$5A
  rts

; A = resume point
write_save:
  sta saveArea+3
  lda storyFlags
  sta saveArea+4
  lda #'D'
  sta saveArea
  lda #'A'
  sta saveArea+1
  lda #1
  sta saveArea+2
  jsr save_sum
  sta saveArea+5
  rts

; Chooses the scene that Start opens.
pick_start:
  lda #BANK_SCRIPT
  sta scriptBank
  lda #<sceneStart
  sta resumePtr
  lda #>sceneStart
  sta resumePtr+1
  lda #0
  sta storyFlags
  lda saveArea
  cmp #'D'
  bne :+
  lda saveArea+1
  cmp #'A'
  bne :+
  lda saveArea+2
  cmp #1
  bne :+
  lda saveArea+3
  cmp #2               ; resume points in saveTab
  bcs :+
  jsr save_sum
  cmp saveArea+5
  bne :+
  lda #<menu
  sta resumePtr
  lda #>menu
  sta resumePtr+1
:
  rts

clear_box:
  lda #1
  sta qJob
  jsr wait_job
  lda #3
  sta sCol
  lda #16
  sta sRow
  rts

put_line:
  jsr next_byte
  cmp #2
  beq :+
  jsr put_char
  jmp put_line
:
  rts

; A = 0 or 1: draws the arrow beside the selected line and clears the other.
choice_arrow:
  lda #3
  sta sCol
  lda #20
  sta sRow
  ldx #0
  lda sSel
  bne :+
  ldx #10
:
  txa
  jsr put_char
  lda #3
  sta sCol
  lda #22
  sta sRow
  ldx #0
  lda sSel
  beq :+
  ldx #10
:
  txa
  jsr put_char
  rts

.proc choice
  jsr next_byte
  sta sTargA
  jsr next_byte
  sta sTargA+1
  jsr next_byte
  sta sTargB
  jsr next_byte
  sta sTargB+1
  lda #5
  sta sCol
  lda #20
  sta sRow
  jsr put_line
  lda #5
  sta sCol
  lda #22
  sta sRow
  jsr put_line
  lda #0
  sta sSel
  jsr choice_arrow
  lda #0
  sta confirmLatch
  sta spad0
wait:
  jsr wait_tick
  lda pad
  and #$0C
  tax
  cpx spad0
  beq :+
  stx spad0
  cpx #0
  beq :+
  lda sSel
  eor #1
  sta sSel
  jsr choice_arrow
:
  lda confirmLatch
  beq wait
  lda #0
  sta confirmLatch
  lda sSel
  beq :+
  lda sTargB
  sta textP
  lda sTargB+1
  sta textP+1
  jmp go
:
  lda sTargA
  sta textP
  lda sTargA+1
  sta textP+1
go:
  jsr clear_box
  jmp story_loop
.endproc

story_nmi:
  txa
  pha
  tya
  pha
  inc tick
  lda qJob
  beq jobsDone
  cmp #1
  beq jobClear
  cmp #2
  beq jobPort
  ; put a run of bytes
  bit PPUSTATUS
  lda qAddr+1
  sta PPUADDR
  lda qAddr
  sta PPUADDR
  ldx #0
: lda qBuf,x
  sta PPUDATA
  inx
  cpx qLen
  bne :-
  jmp jobEnd
jobClear:
  ; text rows 16 to 22
  bit PPUSTATUS
  lda #$22
  sta PPUADDR
  lda #$03
  sta PPUADDR
  lda #0
  ldy #7
@row:
  ldx #26
: sta PPUDATA
  dex
  bne :-
  ; skip the six cells between rows
  ldx #6
: bit PPUDATA
  dex
  bne :-
  dey
  bne @row
  jmp jobEnd
jobPort:
  jsr port_job
jobEnd:
  lda #0
  sta qJob
jobsDone:
  bit PPUSTATUS
  lda #0
  sta PPUSCROLL
  sta PPUSCROLL
  lda #CTRLV
  sta PPUCTRL
  jsr sound_tick
  pla
  tay
  pla
  tax
  pla
  rti

; Swaps the portrait, its palette, and the speaker's name in one vblank.
.proc port_job
  lda sPort
  asl a
  adc sPort
  tay
  ldx #0
  lda #$3F
  sta PPUADDR
  lda #$05
  sta PPUADDR
  lda portPal,y
  sta PPUDATA
  lda portPal+1,y
  sta PPUDATA
  lda portPal+2,y
  sta PPUDATA
  ; 36 tiles: sPort * 36
  lda sPort
  asl a
  asl a
  sta u0
  asl a
  asl a
  asl a
  adc u0
  tax
  lda #$20
  sta spad0
  lda #$84
row:
  ldy spad0
  sty PPUADDR
  sta PPUADDR
  pha
  ldy #6
: lda portMap,x
  sta PPUDATA
  inx
  dey
  bne :-
  pla
  clc
  adc #32
  bcc :+
  inc spad0
:
  cmp #$44
  beq :+
  jmp row
:
  ; name, row 14 column 3
  lda sPort
  asl a
  adc sPort
  asl a
  asl a
  tax
  lda #$21
  sta PPUADDR
  lda #$C3
  sta PPUADDR
  ldy #12
: lda portName,x
  sta PPUDATA
  inx
  dey
  bne :-
  rts
.endproc

enter_title:
  lda #1
  sta musReq
  lda #MODE_TITLE
  sta nextMode
  jmp flight_reset

enter_flight:
  lda #MODE_PLAY
  sta nextMode
  ; fall through

; Back to the 3D display: everything the story changed is rebuilt.
.proc flight_reset
  sei
  lda #0
  sta PPUCTRL
  sta PPUMASK
  ldx #$FF
  txs
  lda #BANK_FLIGHT
  jsr map_bank
  ; clear background patterns and both nametables
  bit PPUSTATUS
  lda #0
  sta PPUADDR
  sta PPUADDR
  ldx #$10
  ldy #0
: sta PPUDATA
  iny
  bne :-
  dex
  bne :-
  lda #$20
  sta PPUADDR
  lda #0
  sta PPUADDR
  ldx #8
: sta PPUDATA
  iny
  bne :-
  dex
  bne :-
  jsr load_radio_font
  lda #$3F
  sta PPUADDR
  lda #0
  sta PPUADDR
  ldx #0
: lda palette,x
  sta PPUDATA
  inx
  cpx #32
  bne :-
  ; forget all tiles
  lda #$68
  sta ptr+1
  lda #0
  sta ptr
  tay
: sta (ptr),y
  iny
  bne :-
  inc ptr+1
  ldx ptr+1
  cpx #$6C
  bne :-
  sta ptr
  ldx #$FF
  lda #$FF
  ldx #0
: sta OAM,x
  inx
  bne :-
  lda #0
  sta storyFlag
  sta radioReq
  sta radioT
  sta xfState
  sta drawDone
  sta front
  sta oldList
  sta curShown
  sta sinceFlip
  jsr set_back
  lda #1
  sta curCnt
  sta oldCnt
  sta drawIdx
  sta prevIdx
  jsr new_game
  lda statNmi
  sta lastNmi
  lda #84
.ifndef NOSPRITE
  sta OAM+0
.endif
  lda #1
  sta OAM+1
  lda #124
  sta OAM+3
  lda #0
  sta OAM+2
  lda #0
  sta startLatch       ; a press left over from the story must not skip the title
  lda nextMode
  sta gameMode
  cmp #MODE_PLAY
  bne :+
  SFXREQ SFX_START
:
  bit PPUSTATUS
  lda #0
  sta PPUSCROLL
  sta PPUSCROLL
  lda #CTRLV
  sta PPUCTRL
  cli
  jmp main
.endproc


