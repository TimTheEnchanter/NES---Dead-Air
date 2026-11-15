
nmi:
  pha
.ifdef DEBUGNEST              ; moves the picture: timing checks leave it out
  lda dbgIn
  beq :+
  inc dbgNest
:
.endif
  lda storyFlag
  beq :+
  jmp story_nmi
:
  txa
  pha
  tya
  pha
  inc statNmi
  bne :+
  inc statNmi+1
:
  lda #<BUD_B
  sta bud
  lda #>BUD_B
  sta bud+1
  inc sinceFlip
  lda xfState
  cmp #ST_FLIP
  bne noflip
  lda sinceFlip
  cmp #2
  bcc noflip
  cmp #3
  bcc :+
  inc statSlow
:
  lda #0
  sta sinceFlip
  ; show the finished frame: only palette entries change
  lda front
  eor #1
  sta front
  tax
  bit PPUSTATUS
  lda #$3F
  sta PPUADDR
  lda #$01
  sta PPUADDR
  lda palRA,x          ; same cycles as the ROM tables: absolute,x
  sta PPUDATA
  lda palRB,x
  sta PPUDATA
  ; leave palette space, or forced-blank lines would show this colour
  lda #0
  sta PPUADDR
  sta PPUADDR
  jsr set_back
  ; the frame that was showing is now hidden: scrub it from its nametable
  lda xfList
  ldx curShown
  sta curShown
  stx oldList
  lda curCnt
  sta oldCnt
  lda xfCnt
  sta curCnt
  lda #1
  sta xfI
  lda #ST_CLEAR
  sta xfState
  jmp flipdone
noflip:
  ; same length as the flip path above, so timing is stable
  ldx #19
: dex
  bne :-
flipdone:
  jsr run_xfer
  ; spin away what is left: every path above charges its exact cost to bud,
  ; because any timing variation here would move the whole picture
  ldx bud+1
  bmi done
  beq low
pg:
  ldy #50
: dey
  bne :-
  dex
  bne pg
low:
  lda bud
  lsr a
  lsr a
  lsr a
  beq done
  tay
: bit $00
  dey
  bne :-
done:
  lda #>OAM
  sta OAMDMA
  ; arm the end-of-picture interrupt
  lda #IRQ_LINES
  sta $C000
  sta $C001
  sta $E001
  bit PPUSTATUS
  lda front
  asl a
  asl a
  sta PPUADDR
  lda #$60             ; coarse Y = 3
  sta PPUADDR
  lda #%00011110
  sta PPUMASK
  pla
  tay
  pla
  tax
  pla
  rti

irq:
  pha
  sta $E000            ; ack and disable
.ifdef DEBUGNEST
  lda #1
  sta dbgIn
.endif
  lda #0
  sta PPUMASK          ; forced blank starts here
  txa
  pha
  tya
  pha
  jsr sound_tick
  jsr radio_tick
  lda #<BUD_A
  sta bud
  lda #>BUD_A
  sta bud+1
  lda tintSend         ; leave room for the tint squares
  beq :+
  lda #<(BUD_A-ATTR_COST)
  sta bud
  lda #>(BUD_A-ATTR_COST)
  sta bud+1
: lda radioBusy        ; the radio row's writes come out of the same time
  beq :+
  lda #0
  sta radioBusy
  lda bud
  sec
  sbc #<RADIO_COST
  sta bud
  lda bud+1
  sbc #>RADIO_COST
  sta bud+1
: jsr run_xfer
  lda xfState
  cmp #ST_ATTR
  bne :+
  jsr write_attrs
  lda #ST_FLIP
  sta xfState
:
  jsr tint_palette     ; the tint palette follows the coming flip
.ifdef DEBUGNEST
  lda #0
  sta dbgIn
.endif
  pla
  tay
  pla
  tax
  pla
  rti

palA: .byte $30,$0F
palB: .byte $0F,$30
.assert <palRA < $FF, error, "palRA must not cross a page: the NMI's timing counts on it"
.assert <palRB < $FF, error, "palRB must not cross a page: the NMI's timing counts on it"
FLASH_COL = $27          ; Lantern Two: the past, drawn in amber
; Rendering off: colour 3 (both bitplanes: line overlaps, the radio font)
; follows the line colour, so the flashback is amber throughout.
flash_idx3:
  lda missionId
  cmp #M_LANTERN2
  bne :+
  lda #$3F
  sta PPUADDR
  lda #$03
  sta PPUADDR
  lda #FLASH_COL
  sta PPUDATA
: rts
; Line colours for this mission: white, or amber for the flashback.
line_colour:
  ldx #$30
  lda missionId
  cmp #M_LANTERN2
  bne :+
  ldx #FLASH_COL
: stx palRA
  stx palRB+1
  lda #$0F
  sta palRA+1
  sta palRB
  rts
TINT = $2C              ; the echo drone's colour
palC: .byte TINT,$0F
palD: .byte $0F,TINT
palette:
  .byte $0F,$30,$0F,$30, $0F,TINT,$0F,TINT, $0F,$0F,$0F,$0F, $0F,$0F,$0F,$0F
  .byte $0F,$2A,$1A,$30, $0F,$16,$16,$16, $0F,$27,$17,$30, $0F,$0F,$0F,$0F
boltTiles:
  .byte $00,$18,$3C,$3C,$18,$00,$00,$00, 0,0,0,0,0,0,0,0
  .byte $00,$7E,$7E,$7E,$7E,$00,$00,$00, 0,0,0,0,0,0,0,0
crosshair:
  .byte %00010000,%00010000,%00000000,%11000110,%00000000,%00010000,%00010000,%00000000


; Radio font: the same glyph in both bitplanes shows through either frame.
load_radio_font:
  lda #>(RADIO_TILE*16)
  sta PPUADDR
  lda #<(RADIO_TILE*16)
  sta PPUADDR
  ldx #0
@tile:
  ldy #8
  stx u0
: lda radioFont,x
  sta PPUDATA
  inx
  dey
  bne :-
  ldx u0
  ldy #8
: lda radioFont,x
  sta PPUDATA
  inx
  dey
  bne :-
  cpx #0
  bne @tile
  rts

reset:
  sei
  cld
  lda #0               ; power-on bank mode is unknown; only $E000 up is certain
  sta $8000
  ldx #$40
  stx $4017
  ldx #$FF
  txs
  inx
  stx PPUCTRL
  stx PPUMASK
  stx $4010
  stx $E000
  bit PPUSTATUS
: bit PPUSTATUS
  bpl :-
: bit PPUSTATUS
  bpl :-
  ; MMC3: 8K CHR RAM straight through, WRAM on, vertical mirroring
  ldx #5
: stx $8000
  lda chrBanks,x
  sta $8001
  dex
  bpl :-
  lda #6
  sta $8000
  lda #0
  sta $8001
  lda #7
  sta $8000
  lda #1
  sta $8001
  lda #0
  sta $A000
  lda #$80
  sta $A001
  ; clear RAM
  lda #0
  tax
: sta $00,x
  sta $0300,x
  sta $0400,x
  sta $0500,x
  sta $0600,x
  sta $0700,x
  inx
  bne :-
  ; multiply pointers: the table pages are fixed
  ldx #0
: lda #>sqrLo
  sta mat+1,x
  sta mat+3,x
  lda #>sqrHi
  sta mat+5,x
  sta mat+7,x
  txa
  clc
  adc #8
  tax
  cpx #8*14
  bne :-
  ldx #0
  lda #$FF
: sta OAM,x
  inx
  bne :-
  ; clear WRAM
  lda #$60
  sta ptr+1
  lda #0
  sta ptr
  tay
: sta (ptr),y
  iny
  bne :-
  inc ptr+1
  ldx ptr+1
  cpx #$7F             ; the last page holds the save
  bne :-
  ; clear CHR RAM and both nametables ($0000-$27FF)
  bit PPUSTATUS
  lda #0
  sta PPUADDR
  sta PPUADDR
  ldx #$28
  ldy #0
: sta PPUDATA
  iny
  bne :-
  dex
  bne :-
  jsr load_radio_font
  ; crosshair sprite pattern, tile 1 of the $1000 table
  lda #$10
  sta PPUADDR
  lda #$10
  sta PPUADDR
  ldx #0
: lda crosshair,x
  sta PPUDATA
  inx
  cpx #8
  bne :-
  ; bolt and shield tiles 2 and 3, digits from tile 16
  lda #$10
  sta PPUADDR
  lda #$20
  sta PPUADDR
  ldx #0
: lda boltTiles,x
  sta PPUDATA
  inx
  cpx #32
  bne :-
  lda #$11
  sta PPUADDR
  lda #$00
  sta PPUADDR
  ldx #0
@digit:
  ldy #8
: lda digits,x
  sta PPUDATA
  inx
  dey
  bne :-
  ldy #8
  lda #0
: sta PPUDATA
  dey
  bne :-
  cpx #80
  bne @digit
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
  jsr flash_idx3       ; Lantern Two: overlaps and radio text in amber too
  lda #0
  sta PPUSCROLL
  sta PPUSCROLL
  ; centre of the view
  lda #84
.ifndef NOSPRITE
  sta OAM+0
.endif
  lda #1
  sta OAM+1
  lda #0
  sta OAM+2
  lda #124
  sta OAM+3
  lda #1
  sta curCnt
  sta oldCnt
.ifdef TESTM
  lda #TESTM
  sta missionId
.endif
.ifdef TESTEASY
  lda #1
  sta difficulty
.endif
  jsr new_game
.ifdef AUTOSTART
  lda #MODE_PLAY
  sta gameMode
.else
  lda #1
  sta musReq
.endif
  jsr set_back
  ; drawing lists start at list 1 so `curShown`=0 is empty
  lda #1
  sta drawIdx
  sta prevIdx
  lda #$30
  sta $4000
  sta $4004
  sta $400C
  lda #$08
  sta $4001            ; sweep units off
  sta $4005
  lda #$80
  sta $4008
  lda #$0F             ; pulses, triangle, noise
  sta $4015
  lda #1
  sta sndEng
  lda #CTRLV
  sta PPUCTRL
  cli
  jmp main

chrBanks: .byte 0,2,4,5,6,7

.segment "VECTORS"
  .word nmi, reset, irq
