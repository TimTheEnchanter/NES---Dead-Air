XD0:
  lda err
  sec
XD0_in:
  dey
  beq XD0_seg
  sbc dy
  bcs XD0_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+0*256,x
  sta PAT+0*256,x
  pla
  sta prev
  jmp XD1
XD0_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+0*256,x
  sta PAT+0*256,x
  lda cnt
  bne XD0_more
  rts
XD0_more:
  bcs XD0_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb1
  jsr allocX
  jmp xb1+1
xb1:
  tax
  jsr segNext
  jmp XD1
XD0_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb2
  jsr allocX
  jmp xb2+1
xb2:
  tax
  jsr segNext
  jmp XD0
XD1:
  lda err
  sec
XD1_in:
  dey
  beq XD1_seg
  sbc dy
  bcs XD1_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+1*256,x
  sta PAT+1*256,x
  pla
  sta prev
  jmp XD2
XD1_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+1*256,x
  sta PAT+1*256,x
  lda cnt
  bne XD1_more
  rts
XD1_more:
  bcs XD1_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb3
  jsr allocX
  jmp xb3+1
xb3:
  tax
  jsr segNext
  jmp XD2
XD1_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb4
  jsr allocX
  jmp xb4+1
xb4:
  tax
  jsr segNext
  jmp XD1
XD2:
  lda err
  sec
XD2_in:
  dey
  beq XD2_seg
  sbc dy
  bcs XD2_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+2*256,x
  sta PAT+2*256,x
  pla
  sta prev
  jmp XD3
XD2_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+2*256,x
  sta PAT+2*256,x
  lda cnt
  bne XD2_more
  rts
XD2_more:
  bcs XD2_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb5
  jsr allocX
  jmp xb5+1
xb5:
  tax
  jsr segNext
  jmp XD3
XD2_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb6
  jsr allocX
  jmp xb6+1
xb6:
  tax
  jsr segNext
  jmp XD2
XD3:
  lda err
  sec
XD3_in:
  dey
  beq XD3_seg
  sbc dy
  bcs XD3_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+3*256,x
  sta PAT+3*256,x
  pla
  sta prev
  jmp XD4
XD3_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+3*256,x
  sta PAT+3*256,x
  lda cnt
  bne XD3_more
  rts
XD3_more:
  bcs XD3_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb7
  jsr allocX
  jmp xb7+1
xb7:
  tax
  jsr segNext
  jmp XD4
XD3_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb8
  jsr allocX
  jmp xb8+1
xb8:
  tax
  jsr segNext
  jmp XD3
XD4:
  lda err
  sec
XD4_in:
  dey
  beq XD4_seg
  sbc dy
  bcs XD4_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+4*256,x
  sta PAT+4*256,x
  pla
  sta prev
  jmp XD5
XD4_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+4*256,x
  sta PAT+4*256,x
  lda cnt
  bne XD4_more
  rts
XD4_more:
  bcs XD4_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb9
  jsr allocX
  jmp xb9+1
xb9:
  tax
  jsr segNext
  jmp XD5
XD4_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb10
  jsr allocX
  jmp xb10+1
xb10:
  tax
  jsr segNext
  jmp XD4
XD5:
  lda err
  sec
XD5_in:
  dey
  beq XD5_seg
  sbc dy
  bcs XD5_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+5*256,x
  sta PAT+5*256,x
  pla
  sta prev
  jmp XD6
XD5_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+5*256,x
  sta PAT+5*256,x
  lda cnt
  bne XD5_more
  rts
XD5_more:
  bcs XD5_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb11
  jsr allocX
  jmp xb11+1
xb11:
  tax
  jsr segNext
  jmp XD6
XD5_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb12
  jsr allocX
  jmp xb12+1
xb12:
  tax
  jsr segNext
  jmp XD5
XD6:
  lda err
  sec
XD6_in:
  dey
  beq XD6_seg
  sbc dy
  bcs XD6_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+6*256,x
  sta PAT+6*256,x
  pla
  sta prev
  jmp XD7
XD6_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+6*256,x
  sta PAT+6*256,x
  lda cnt
  bne XD6_more
  rts
XD6_more:
  bcs XD6_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb13
  jsr allocX
  jmp xb13+1
xb13:
  tax
  jsr segNext
  jmp XD7
XD6_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb14
  jsr allocX
  jmp xb14+1
xb14:
  tax
  jsr segNext
  jmp XD6
XD7:
  lda err
  sec
XD7_in:
  dey
  beq XD7_seg
  sbc dy
  bcs XD7_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+7*256,x
  sta PAT+7*256,x
  pla
  sta prev
  lda mptr
  clc
  adc #32
  sta mptr
  bcc md15
  inc mptr+1
md15:
  ldx #0
  lda (mptr,x)
  bne xb16
  jsr allocX
  jmp xb16+1
xb16:
  tax
  jmp XD0
XD7_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+7*256,x
  sta PAT+7*256,x
  lda cnt
  bne XD7_more
  rts
XD7_more:
  bcs XD7_same
  lda err
  adc dx
  sta err
  inc mptr
  lda mptr
  clc
  adc #32
  sta mptr
  bcc md17
  inc mptr+1
md17:
  ldx #0
  lda (mptr,x)
  bne xb18
  jsr allocX
  jmp xb18+1
xb18:
  tax
  jsr segNext
  jmp XD0
XD7_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb19
  jsr allocX
  jmp xb19+1
xb19:
  tax
  jsr segNext
  jmp XD7
XU0:
  lda err
  sec
XU0_in:
  dey
  beq XU0_seg
  sbc dy
  bcs XU0_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+0*256,x
  sta PAT+0*256,x
  pla
  sta prev
  lda mptr
  sec
  sbc #32
  sta mptr
  bcs mu20
  dec mptr+1
mu20:
  ldx #0
  lda (mptr,x)
  bne xb21
  jsr allocX
  jmp xb21+1
xb21:
  tax
  jmp XU7
XU0_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+0*256,x
  sta PAT+0*256,x
  lda cnt
  bne XU0_more
  rts
XU0_more:
  bcs XU0_same
  lda err
  adc dx
  sta err
  inc mptr
  lda mptr
  sec
  sbc #32
  sta mptr
  bcs mu22
  dec mptr+1
mu22:
  ldx #0
  lda (mptr,x)
  bne xb23
  jsr allocX
  jmp xb23+1
xb23:
  tax
  jsr segNext
  jmp XU7
XU0_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb24
  jsr allocX
  jmp xb24+1
xb24:
  tax
  jsr segNext
  jmp XU0
XU1:
  lda err
  sec
XU1_in:
  dey
  beq XU1_seg
  sbc dy
  bcs XU1_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+1*256,x
  sta PAT+1*256,x
  pla
  sta prev
  jmp XU0
XU1_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+1*256,x
  sta PAT+1*256,x
  lda cnt
  bne XU1_more
  rts
XU1_more:
  bcs XU1_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb25
  jsr allocX
  jmp xb25+1
xb25:
  tax
  jsr segNext
  jmp XU0
XU1_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb26
  jsr allocX
  jmp xb26+1
xb26:
  tax
  jsr segNext
  jmp XU1
XU2:
  lda err
  sec
XU2_in:
  dey
  beq XU2_seg
  sbc dy
  bcs XU2_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+2*256,x
  sta PAT+2*256,x
  pla
  sta prev
  jmp XU1
XU2_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+2*256,x
  sta PAT+2*256,x
  lda cnt
  bne XU2_more
  rts
XU2_more:
  bcs XU2_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb27
  jsr allocX
  jmp xb27+1
xb27:
  tax
  jsr segNext
  jmp XU1
XU2_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb28
  jsr allocX
  jmp xb28+1
xb28:
  tax
  jsr segNext
  jmp XU2
XU3:
  lda err
  sec
XU3_in:
  dey
  beq XU3_seg
  sbc dy
  bcs XU3_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+3*256,x
  sta PAT+3*256,x
  pla
  sta prev
  jmp XU2
XU3_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+3*256,x
  sta PAT+3*256,x
  lda cnt
  bne XU3_more
  rts
XU3_more:
  bcs XU3_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb29
  jsr allocX
  jmp xb29+1
xb29:
  tax
  jsr segNext
  jmp XU2
XU3_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb30
  jsr allocX
  jmp xb30+1
xb30:
  tax
  jsr segNext
  jmp XU3
XU4:
  lda err
  sec
XU4_in:
  dey
  beq XU4_seg
  sbc dy
  bcs XU4_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+4*256,x
  sta PAT+4*256,x
  pla
  sta prev
  jmp XU3
XU4_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+4*256,x
  sta PAT+4*256,x
  lda cnt
  bne XU4_more
  rts
XU4_more:
  bcs XU4_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb31
  jsr allocX
  jmp xb31+1
xb31:
  tax
  jsr segNext
  jmp XU3
XU4_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb32
  jsr allocX
  jmp xb32+1
xb32:
  tax
  jsr segNext
  jmp XU4
XU5:
  lda err
  sec
XU5_in:
  dey
  beq XU5_seg
  sbc dy
  bcs XU5_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+5*256,x
  sta PAT+5*256,x
  pla
  sta prev
  jmp XU4
XU5_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+5*256,x
  sta PAT+5*256,x
  lda cnt
  bne XU5_more
  rts
XU5_more:
  bcs XU5_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb33
  jsr allocX
  jmp xb33+1
xb33:
  tax
  jsr segNext
  jmp XU4
XU5_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb34
  jsr allocX
  jmp xb34+1
xb34:
  tax
  jsr segNext
  jmp XU5
XU6:
  lda err
  sec
XU6_in:
  dey
  beq XU6_seg
  sbc dy
  bcs XU6_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+6*256,x
  sta PAT+6*256,x
  pla
  sta prev
  jmp XU5
XU6_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+6*256,x
  sta PAT+6*256,x
  lda cnt
  bne XU6_more
  rts
XU6_more:
  bcs XU6_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb35
  jsr allocX
  jmp xb35+1
xb35:
  tax
  jsr segNext
  jmp XU5
XU6_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb36
  jsr allocX
  jmp xb36+1
xb36:
  tax
  jsr segNext
  jmp XU6
XU7:
  lda err
  sec
XU7_in:
  dey
  beq XU7_seg
  sbc dy
  bcs XU7_in
  adc dx
  sta err
  lda (mrp),y
  pha
  eor prev
  ora PAT+7*256,x
  sta PAT+7*256,x
  pla
  sta prev
  jmp XU6
XU7_seg:
  sbc dy
  sta err
  lda (mrp),y
  eor prev
  ora PAT+7*256,x
  sta PAT+7*256,x
  lda cnt
  bne XU7_more
  rts
XU7_more:
  bcs XU7_same
  lda err
  adc dx
  sta err
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb37
  jsr allocX
  jmp xb37+1
xb37:
  tax
  jsr segNext
  jmp XU6
XU7_same:
  inc mptr
  ldx #0
  lda (mptr,x)
  bne xb38
  jsr allocX
  jmp xb38+1
xb38:
  tax
  jsr segNext
  jmp XU7
YL0:
  lda PAT+0*256,y
  ora mask
  sta PAT+0*256,y
  dex
  beq YL0_done
  lda err
  sbc dx
  sta err
  bcs YL1
  adc dy
  sta err
  asl mask
  bcs YL0_wrap
  sec
  jmp YL1
YL0_wrap:
  rol mask
  dec mptr
  ldy #0
  lda (mptr),y
  bne yb40
  jsr lookAlloc
  jmp yb40+1
yb40:
  tay
  sec
  jmp YL1
YL0_done:
  rts
YL1:
  lda PAT+1*256,y
  ora mask
  sta PAT+1*256,y
  dex
  beq YL1_done
  lda err
  sbc dx
  sta err
  bcs YL2
  adc dy
  sta err
  asl mask
  bcs YL1_wrap
  sec
  jmp YL2
YL1_wrap:
  rol mask
  dec mptr
  ldy #0
  lda (mptr),y
  bne yb42
  jsr lookAlloc
  jmp yb42+1
yb42:
  tay
  sec
  jmp YL2
YL1_done:
  rts
YL2:
  lda PAT+2*256,y
  ora mask
  sta PAT+2*256,y
  dex
  beq YL2_done
  lda err
  sbc dx
  sta err
  bcs YL3
  adc dy
  sta err
  asl mask
  bcs YL2_wrap
  sec
  jmp YL3
YL2_wrap:
  rol mask
  dec mptr
  ldy #0
  lda (mptr),y
  bne yb44
  jsr lookAlloc
  jmp yb44+1
yb44:
  tay
  sec
  jmp YL3
YL2_done:
  rts
YL3:
  lda PAT+3*256,y
  ora mask
  sta PAT+3*256,y
  dex
  beq YL3_done
  lda err
  sbc dx
  sta err
  bcs YL4
  adc dy
  sta err
  asl mask
  bcs YL3_wrap
  sec
  jmp YL4
YL3_wrap:
  rol mask
  dec mptr
  ldy #0
  lda (mptr),y
  bne yb46
  jsr lookAlloc
  jmp yb46+1
yb46:
  tay
  sec
  jmp YL4
YL3_done:
  rts
YL4:
  lda PAT+4*256,y
  ora mask
  sta PAT+4*256,y
  dex
  beq YL4_done
  lda err
  sbc dx
  sta err
  bcs YL5
  adc dy
  sta err
  asl mask
  bcs YL4_wrap
  sec
  jmp YL5
YL4_wrap:
  rol mask
  dec mptr
  ldy #0
  lda (mptr),y
  bne yb48
  jsr lookAlloc
  jmp yb48+1
yb48:
  tay
  sec
  jmp YL5
YL4_done:
  rts
YL5:
  lda PAT+5*256,y
  ora mask
  sta PAT+5*256,y
  dex
  beq YL5_done
  lda err
  sbc dx
  sta err
  bcs YL6
  adc dy
  sta err
  asl mask
  bcs YL5_wrap
  sec
  jmp YL6
YL5_wrap:
  rol mask
  dec mptr
  ldy #0
  lda (mptr),y
  bne yb50
  jsr lookAlloc
  jmp yb50+1
yb50:
  tay
  sec
  jmp YL6
YL5_done:
  rts
YL6:
  lda PAT+6*256,y
  ora mask
  sta PAT+6*256,y
  dex
  beq YL6_done
  lda err
  sbc dx
  sta err
  bcs YL7
  adc dy
  sta err
  asl mask
  bcs YL6_wrap
  sec
  jmp YL7
YL6_wrap:
  rol mask
  dec mptr
  ldy #0
  lda (mptr),y
  bne yb52
  jsr lookAlloc
  jmp yb52+1
yb52:
  tay
  sec
  jmp YL7
YL6_done:
  rts
YL7:
  lda PAT+7*256,y
  ora mask
  sta PAT+7*256,y
  dex
  beq YL7_done
  lda err
  sbc dx
  sta err
  bcs YL7_down
  adc dy
  sta err
  asl mask
  bcs YL7_wrap
  jmp YL7_down
YL7_wrap:
  rol mask
  dec mptr
YL7_down:
  lda mptr
  clc
  adc #32
  sta mptr
  bcc md53
  inc mptr+1
md53:
  ldy #0
  lda (mptr),y
  bne yb55
  jsr lookAlloc
  jmp yb55+1
yb55:
  tay
  sec
  jmp YL0
YL7_done:
  rts
YR0:
  lda PAT+0*256,y
  ora mask
  sta PAT+0*256,y
  dex
  beq YR0_done
  lda err
  sbc dx
  sta err
  bcs YR1
  adc dy
  sta err
  lsr mask
  bcs YR0_wrap
  sec
  jmp YR1
YR0_wrap:
  ror mask
  inc mptr
  ldy #0
  lda (mptr),y
  bne yb57
  jsr lookAlloc
  jmp yb57+1
yb57:
  tay
  sec
  jmp YR1
YR0_done:
  rts
YR1:
  lda PAT+1*256,y
  ora mask
  sta PAT+1*256,y
  dex
  beq YR1_done
  lda err
  sbc dx
  sta err
  bcs YR2
  adc dy
  sta err
  lsr mask
  bcs YR1_wrap
  sec
  jmp YR2
YR1_wrap:
  ror mask
  inc mptr
  ldy #0
  lda (mptr),y
  bne yb59
  jsr lookAlloc
  jmp yb59+1
yb59:
  tay
  sec
  jmp YR2
YR1_done:
  rts
YR2:
  lda PAT+2*256,y
  ora mask
  sta PAT+2*256,y
  dex
  beq YR2_done
  lda err
  sbc dx
  sta err
  bcs YR3
  adc dy
  sta err
  lsr mask
  bcs YR2_wrap
  sec
  jmp YR3
YR2_wrap:
  ror mask
  inc mptr
  ldy #0
  lda (mptr),y
  bne yb61
  jsr lookAlloc
  jmp yb61+1
yb61:
  tay
  sec
  jmp YR3
YR2_done:
  rts
YR3:
  lda PAT+3*256,y
  ora mask
  sta PAT+3*256,y
  dex
  beq YR3_done
  lda err
  sbc dx
  sta err
  bcs YR4
  adc dy
  sta err
  lsr mask
  bcs YR3_wrap
  sec
  jmp YR4
YR3_wrap:
  ror mask
  inc mptr
  ldy #0
  lda (mptr),y
  bne yb63
  jsr lookAlloc
  jmp yb63+1
yb63:
  tay
  sec
  jmp YR4
YR3_done:
  rts
YR4:
  lda PAT+4*256,y
  ora mask
  sta PAT+4*256,y
  dex
  beq YR4_done
  lda err
  sbc dx
  sta err
  bcs YR5
  adc dy
  sta err
  lsr mask
  bcs YR4_wrap
  sec
  jmp YR5
YR4_wrap:
  ror mask
  inc mptr
  ldy #0
  lda (mptr),y
  bne yb65
  jsr lookAlloc
  jmp yb65+1
yb65:
  tay
  sec
  jmp YR5
YR4_done:
  rts
YR5:
  lda PAT+5*256,y
  ora mask
  sta PAT+5*256,y
  dex
  beq YR5_done
  lda err
  sbc dx
  sta err
  bcs YR6
  adc dy
  sta err
  lsr mask
  bcs YR5_wrap
  sec
  jmp YR6
YR5_wrap:
  ror mask
  inc mptr
  ldy #0
  lda (mptr),y
  bne yb67
  jsr lookAlloc
  jmp yb67+1
yb67:
  tay
  sec
  jmp YR6
YR5_done:
  rts
YR6:
  lda PAT+6*256,y
  ora mask
  sta PAT+6*256,y
  dex
  beq YR6_done
  lda err
  sbc dx
  sta err
  bcs YR7
  adc dy
  sta err
  lsr mask
  bcs YR6_wrap
  sec
  jmp YR7
YR6_wrap:
  ror mask
  inc mptr
  ldy #0
  lda (mptr),y
  bne yb69
  jsr lookAlloc
  jmp yb69+1
yb69:
  tay
  sec
  jmp YR7
YR6_done:
  rts
YR7:
  lda PAT+7*256,y
  ora mask
  sta PAT+7*256,y
  dex
  beq YR7_done
  lda err
  sbc dx
  sta err
  bcs YR7_down
  adc dy
  sta err
  lsr mask
  bcs YR7_wrap
  jmp YR7_down
YR7_wrap:
  ror mask
  inc mptr
YR7_down:
  lda mptr
  clc
  adc #32
  sta mptr
  bcc md70
  inc mptr+1
md70:
  ldy #0
  lda (mptr),y
  bne yb72
  jsr lookAlloc
  jmp yb72+1
yb72:
  tay
  sec
  jmp YR0
YR7_done:
  rts
xdTab: .word XD0,XD1,XD2,XD3,XD4,XD5,XD6,XD7
xuTab: .word XU0,XU1,XU2,XU3,XU4,XU5,XU6,XU7
yrTab: .word YR0,YR1,YR2,YR3,YR4,YR5,YR6,YR7
ylTab: .word YL0,YL1,YL2,YL3,YL4,YL5,YL6,YL7
maskL: .byte 255,127,63,31,15,7,3,1
maskR: .byte 128,192,224,240,248,252,254,255
maskQ: .byte 255,254,252,248,240,224,192,128
.assert <maskQ <= 248, error, "maskQ must not cross a page"
