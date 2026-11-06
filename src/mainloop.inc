; ---------- per-frame main loop ----------
.proc clear_map
  ; forget the cells used by the previous frame
  ldx #0
  ldy #1
  lda prevIdx
  asl a
  clc
  adc #LISTS
  sta lpLo+1
  adc #1
  sta lpHi+1
@l:
  cpy prevCnt
  bcs @d
  lda (lpLo),y
  sta mptr
  lda (lpHi),y
  clc
  adc #MAP/256-$20
  sta mptr+1
  lda #0
  sta (mptr,x)
  iny
  bne @l
@d:
  rts
.endproc

main:
  lda #1
  sta bufFree
  sta prevCnt
  sta jobCnt
@frame:
  lda bufFree
  beq @frame
  lda #0
  sta bufFree
  MARK 1
  jsr clear_map
  MARK 2
  lda drawIdx
  sta prevIdx
  ; select this frame's list
  asl a
  clc
  adc #LISTS
  sta lpLo+1
  adc #1
  sta lpHi+1
  lda #1
  sta nextTile
  jsr update_player
  jsr update_fire
  lda #0
  sta ship
@ships:
  ldx ship
  lda alive,x
  bne :+
  jmp @next
:
  MARK 3
  jsr rel_pos
  jsr laser_test
  ; skip ships wholly behind the camera
  ldx ship
  lda #100
  sta marginL
  lda #0
  sta marginH
  lda shipModel,x
  beq :+
  lda #<500
  sta marginL
  lda #>500
  sta marginH
:
  lda relZ
  clc
  adc marginL
  lda relZ+1
  adc marginH
  bpl :+
  jsr ship_marker
  jmp @next
:
  jsr in_view
  bcs :+
  jsr ship_marker
  jmp @next
:
  jsr select_model
  jsr build_ship_rot
  jsr build_m
  lda mShift
  bne @closeFaces
  lda vFirst
  bne @farFaces
@closeFaces:
  jsr visible_close
  jmp @faced
@farFaces:
  jsr visible_faces
@faced:
  MARK 4
  lda #0
  sta offscreen
  lda mShift
  bne @near            ; large hulls need true perspective
  lda relZ+1
  bmi @near
  cmp #3
  bcc @near
  jsr transform_far
  jmp @xdone
@near:
  jsr build_mat
  jsr transform
@xdone:
  MARK 5
  jsr draw_edges
  MARK 6
@next:
  inc ship
  lda gameMode
  cmp #MODE_PLAY
  bne @drawn           ; the text screens show the hunter alone
  lda ship
  cmp #NSHIPS
  beq @drawn
  jmp @ships
@drawn:
  lda markerT
  beq :+
  dec markerT
  jmp :++
:
  lda #$FF
  sta OAM+88
:
  jsr apply_hits
  jsr update_sprites
  jsr draw_bolt
  jsr draw_nav
  jsr draw_mode
.ifdef DEBUG
  ; a fixed marker line lets the harness check the picture never moves
  lda #40
  sta x0
  lda #150
  sta x1
  lda #2
  sta y0
  sta y1
  jsr line
.endif
  lda nextTile
  sta prevCnt
  sta statTiles
  cmp statPeak
  bcc :+
  sta statPeak
:
  lda yaw
  sta statYaw
  lda pitch
  sta statPitch
  lda vis
  sta statVis
  ldx #8
: lda camPos,x
  sta statDbg,x
  dex
  bpl :-
  lda camYaw
  sta statDbg+9
  lda camPitch
  sta statDbg+10
  lda speed
  sta statDbg+11
  ; snapshot the hunter with the camera, for the test harness
  lda shipPosL
  sta statHunter+0
  lda shipPosH
  sta statHunter+1
  lda shipPosL+16
  sta statHunter+2
  lda shipPosH+16
  sta statHunter+3
  lda shipPosL+32
  sta statHunter+4
  lda shipPosH+32
  sta statHunter+5
  lda hYaw
  sta statHunter+6
  lda hPitch
  sta statHunter+7
  ldx #NSHIPS-1
  lda #0
  sta u0
: asl u0
  lda alive,x
  ora u0
  sta u0
  dex
  bpl :-
  lda u0
  sta statAlive
  lda score
  sta statScore
  lda shields
  sta statShields
  ; hand off to the interrupt side
  lda nextTile
  sta jobCnt
  lda drawIdx
  sta jobIdx
  lda #1
  sta drawDone
  ; next list
  ldx drawIdx
  inx
  cpx #3
  bne :+
  ldx #0
:
  stx drawIdx
  inc statFrames
  bne :+
  inc statFrames+1
:
  jmp @frame

shipYaw:   .byte 0,90,170,40,0,0,0,0,0,0,0,0,0,0,0,0
shipPitch: .byte 0,60,120,200,0,0,0,0,0,0,0,0,0,0,0,0
; world positions, split lo/hi; offsets 0, 16, 32 select X, Y, Z
shipInitL:
  .byte <0,<(-500),<650,<300,0,0,0,0,0,0,0,0,0,0,0,0
  .byte <0,<150,<(-200),<400,0,0,0,0,0,0,0,0,0,0,0,0
  .byte <700,<1400,<1900,<(-1200),0,0,0,0,0,0,0,0,0,0,0,0
shipInitH:
  .byte >0,>(-500),>650,>300,0,0,0,0,0,0,0,0,0,0,0,0
  .byte >0,>150,>(-200),>400,0,0,0,0,0,0,0,0,0,0,0,0
  .byte >700,>1400,>1900,>(-1200),0,0,0,0,0,0,0,0,0,0,0,0


