XD0:
  lda err
  sec
XD0_in:
  dex
  beq XD0_seg
  sbc dy
  bcs XD0_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+0*256,y
  sta PAT+0*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XD1
XD0_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+0*256,y
  sta PAT+0*256,y
  lda cnt
  bne XD0_more
  rts
XD0_more:
  lda #0
  sta segS
  bcs XD0_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XD1
XD0_same:
  jsr tileRight
  jsr segInit
  jmp XD0
XD1:
  lda err
  sec
XD1_in:
  dex
  beq XD1_seg
  sbc dy
  bcs XD1_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+1*256,y
  sta PAT+1*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XD2
XD1_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+1*256,y
  sta PAT+1*256,y
  lda cnt
  bne XD1_more
  rts
XD1_more:
  lda #0
  sta segS
  bcs XD1_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XD2
XD1_same:
  jsr tileRight
  jsr segInit
  jmp XD1
XD2:
  lda err
  sec
XD2_in:
  dex
  beq XD2_seg
  sbc dy
  bcs XD2_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+2*256,y
  sta PAT+2*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XD3
XD2_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+2*256,y
  sta PAT+2*256,y
  lda cnt
  bne XD2_more
  rts
XD2_more:
  lda #0
  sta segS
  bcs XD2_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XD3
XD2_same:
  jsr tileRight
  jsr segInit
  jmp XD2
XD3:
  lda err
  sec
XD3_in:
  dex
  beq XD3_seg
  sbc dy
  bcs XD3_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+3*256,y
  sta PAT+3*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XD4
XD3_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+3*256,y
  sta PAT+3*256,y
  lda cnt
  bne XD3_more
  rts
XD3_more:
  lda #0
  sta segS
  bcs XD3_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XD4
XD3_same:
  jsr tileRight
  jsr segInit
  jmp XD3
XD4:
  lda err
  sec
XD4_in:
  dex
  beq XD4_seg
  sbc dy
  bcs XD4_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+4*256,y
  sta PAT+4*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XD5
XD4_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+4*256,y
  sta PAT+4*256,y
  lda cnt
  bne XD4_more
  rts
XD4_more:
  lda #0
  sta segS
  bcs XD4_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XD5
XD4_same:
  jsr tileRight
  jsr segInit
  jmp XD4
XD5:
  lda err
  sec
XD5_in:
  dex
  beq XD5_seg
  sbc dy
  bcs XD5_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+5*256,y
  sta PAT+5*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XD6
XD5_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+5*256,y
  sta PAT+5*256,y
  lda cnt
  bne XD5_more
  rts
XD5_more:
  lda #0
  sta segS
  bcs XD5_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XD6
XD5_same:
  jsr tileRight
  jsr segInit
  jmp XD5
XD6:
  lda err
  sec
XD6_in:
  dex
  beq XD6_seg
  sbc dy
  bcs XD6_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+6*256,y
  sta PAT+6*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XD7
XD6_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+6*256,y
  sta PAT+6*256,y
  lda cnt
  bne XD6_more
  rts
XD6_more:
  lda #0
  sta segS
  bcs XD6_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XD7
XD6_same:
  jsr tileRight
  jsr segInit
  jmp XD6
XD7:
  lda err
  sec
XD7_in:
  dex
  beq XD7_seg
  sbc dy
  bcs XD7_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+7*256,y
  sta PAT+7*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jsr tileDown
  jmp XD0
XD7_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+7*256,y
  sta PAT+7*256,y
  lda cnt
  bne XD7_more
  rts
XD7_more:
  lda #0
  sta segS
  bcs XD7_same
  lda err
  adc dx
  sta err
  inc mptr
  jsr tileDown
  jsr segInit
  jmp XD0
XD7_same:
  jsr tileRight
  jsr segInit
  jmp XD7
XU0:
  lda err
  sec
XU0_in:
  dex
  beq XU0_seg
  sbc dy
  bcs XU0_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+0*256,y
  sta PAT+0*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jsr tileUp
  jmp XU7
XU0_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+0*256,y
  sta PAT+0*256,y
  lda cnt
  bne XU0_more
  rts
XU0_more:
  lda #0
  sta segS
  bcs XU0_same
  lda err
  adc dx
  sta err
  inc mptr
  jsr tileUp
  jsr segInit
  jmp XU7
XU0_same:
  jsr tileRight
  jsr segInit
  jmp XU0
XU1:
  lda err
  sec
XU1_in:
  dex
  beq XU1_seg
  sbc dy
  bcs XU1_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+1*256,y
  sta PAT+1*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XU0
XU1_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+1*256,y
  sta PAT+1*256,y
  lda cnt
  bne XU1_more
  rts
XU1_more:
  lda #0
  sta segS
  bcs XU1_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XU0
XU1_same:
  jsr tileRight
  jsr segInit
  jmp XU1
XU2:
  lda err
  sec
XU2_in:
  dex
  beq XU2_seg
  sbc dy
  bcs XU2_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+2*256,y
  sta PAT+2*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XU1
XU2_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+2*256,y
  sta PAT+2*256,y
  lda cnt
  bne XU2_more
  rts
XU2_more:
  lda #0
  sta segS
  bcs XU2_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XU1
XU2_same:
  jsr tileRight
  jsr segInit
  jmp XU2
XU3:
  lda err
  sec
XU3_in:
  dex
  beq XU3_seg
  sbc dy
  bcs XU3_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+3*256,y
  sta PAT+3*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XU2
XU3_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+3*256,y
  sta PAT+3*256,y
  lda cnt
  bne XU3_more
  rts
XU3_more:
  lda #0
  sta segS
  bcs XU3_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XU2
XU3_same:
  jsr tileRight
  jsr segInit
  jmp XU3
XU4:
  lda err
  sec
XU4_in:
  dex
  beq XU4_seg
  sbc dy
  bcs XU4_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+4*256,y
  sta PAT+4*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XU3
XU4_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+4*256,y
  sta PAT+4*256,y
  lda cnt
  bne XU4_more
  rts
XU4_more:
  lda #0
  sta segS
  bcs XU4_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XU3
XU4_same:
  jsr tileRight
  jsr segInit
  jmp XU4
XU5:
  lda err
  sec
XU5_in:
  dex
  beq XU5_seg
  sbc dy
  bcs XU5_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+5*256,y
  sta PAT+5*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XU4
XU5_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+5*256,y
  sta PAT+5*256,y
  lda cnt
  bne XU5_more
  rts
XU5_more:
  lda #0
  sta segS
  bcs XU5_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XU4
XU5_same:
  jsr tileRight
  jsr segInit
  jmp XU5
XU6:
  lda err
  sec
XU6_in:
  dex
  beq XU6_seg
  sbc dy
  bcs XU6_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+6*256,y
  sta PAT+6*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XU5
XU6_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+6*256,y
  sta PAT+6*256,y
  lda cnt
  bne XU6_more
  rts
XU6_more:
  lda #0
  sta segS
  bcs XU6_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XU5
XU6_same:
  jsr tileRight
  jsr segInit
  jmp XU6
XU7:
  lda err
  sec
XU7_in:
  dex
  beq XU7_seg
  sbc dy
  bcs XU7_in
  adc dx
  sta err
  stx u0
  lda segL
  sec
  sbc u0
  tax
  lda maskR,x
  stx u1
  ldx segS
  and maskL,x
  ora PAT+7*256,y
  sta PAT+7*256,y
  ldx u1
  inx
  stx segS
  ldx u0
  jmp XU6
XU7_seg:
  sbc dy
  sta err
  ldx segL
  lda maskR,x
  ldx segS
  and maskL,x
  ora PAT+7*256,y
  sta PAT+7*256,y
  lda cnt
  bne XU7_more
  rts
XU7_more:
  lda #0
  sta segS
  bcs XU7_same
  lda err
  adc dx
  sta err
  jsr tileRight
  jsr segInit
  jmp XU6
XU7_same:
  jsr tileRight
  jsr segInit
  jmp XU7
YL0:
  lda PAT+0*256,y
  ora mask
  sta PAT+0*256,y
  dex
  beq YL_done0
  lda err
  sec
  sbc dx
  sta err
  bcs YL0_next
  adc dy
  sta err
  asl mask
  bcc YL0_next
  rol mask
  jsr tileLeft
YL0_next:
  jmp YL1
YL_done0:
  rts
YL1:
  lda PAT+1*256,y
  ora mask
  sta PAT+1*256,y
  dex
  beq YL_done1
  lda err
  sec
  sbc dx
  sta err
  bcs YL1_next
  adc dy
  sta err
  asl mask
  bcc YL1_next
  rol mask
  jsr tileLeft
YL1_next:
  jmp YL2
YL_done1:
  rts
YL2:
  lda PAT+2*256,y
  ora mask
  sta PAT+2*256,y
  dex
  beq YL_done2
  lda err
  sec
  sbc dx
  sta err
  bcs YL2_next
  adc dy
  sta err
  asl mask
  bcc YL2_next
  rol mask
  jsr tileLeft
YL2_next:
  jmp YL3
YL_done2:
  rts
YL3:
  lda PAT+3*256,y
  ora mask
  sta PAT+3*256,y
  dex
  beq YL_done3
  lda err
  sec
  sbc dx
  sta err
  bcs YL3_next
  adc dy
  sta err
  asl mask
  bcc YL3_next
  rol mask
  jsr tileLeft
YL3_next:
  jmp YL4
YL_done3:
  rts
YL4:
  lda PAT+4*256,y
  ora mask
  sta PAT+4*256,y
  dex
  beq YL_done4
  lda err
  sec
  sbc dx
  sta err
  bcs YL4_next
  adc dy
  sta err
  asl mask
  bcc YL4_next
  rol mask
  jsr tileLeft
YL4_next:
  jmp YL5
YL_done4:
  rts
YL5:
  lda PAT+5*256,y
  ora mask
  sta PAT+5*256,y
  dex
  beq YL_done5
  lda err
  sec
  sbc dx
  sta err
  bcs YL5_next
  adc dy
  sta err
  asl mask
  bcc YL5_next
  rol mask
  jsr tileLeft
YL5_next:
  jmp YL6
YL_done5:
  rts
YL6:
  lda PAT+6*256,y
  ora mask
  sta PAT+6*256,y
  dex
  beq YL_done6
  lda err
  sec
  sbc dx
  sta err
  bcs YL6_next
  adc dy
  sta err
  asl mask
  bcc YL6_next
  rol mask
  jsr tileLeft
YL6_next:
  jmp YL7
YL_done6:
  rts
YL7:
  lda PAT+7*256,y
  ora mask
  sta PAT+7*256,y
  dex
  beq YL_done7
  lda err
  sec
  sbc dx
  sta err
  bcs YL7_next
  adc dy
  sta err
  asl mask
  bcc YL7_next
  rol mask
  jsr tileLeft
YL7_next:
  jsr tileDown
  jmp YL0
YL_done7:
  rts
YR0:
  lda PAT+0*256,y
  ora mask
  sta PAT+0*256,y
  dex
  beq YR_done0
  lda err
  sec
  sbc dx
  sta err
  bcs YR0_next
  adc dy
  sta err
  lsr mask
  bcc YR0_next
  ror mask
  jsr tileRight
YR0_next:
  jmp YR1
YR_done0:
  rts
YR1:
  lda PAT+1*256,y
  ora mask
  sta PAT+1*256,y
  dex
  beq YR_done1
  lda err
  sec
  sbc dx
  sta err
  bcs YR1_next
  adc dy
  sta err
  lsr mask
  bcc YR1_next
  ror mask
  jsr tileRight
YR1_next:
  jmp YR2
YR_done1:
  rts
YR2:
  lda PAT+2*256,y
  ora mask
  sta PAT+2*256,y
  dex
  beq YR_done2
  lda err
  sec
  sbc dx
  sta err
  bcs YR2_next
  adc dy
  sta err
  lsr mask
  bcc YR2_next
  ror mask
  jsr tileRight
YR2_next:
  jmp YR3
YR_done2:
  rts
YR3:
  lda PAT+3*256,y
  ora mask
  sta PAT+3*256,y
  dex
  beq YR_done3
  lda err
  sec
  sbc dx
  sta err
  bcs YR3_next
  adc dy
  sta err
  lsr mask
  bcc YR3_next
  ror mask
  jsr tileRight
YR3_next:
  jmp YR4
YR_done3:
  rts
YR4:
  lda PAT+4*256,y
  ora mask
  sta PAT+4*256,y
  dex
  beq YR_done4
  lda err
  sec
  sbc dx
  sta err
  bcs YR4_next
  adc dy
  sta err
  lsr mask
  bcc YR4_next
  ror mask
  jsr tileRight
YR4_next:
  jmp YR5
YR_done4:
  rts
YR5:
  lda PAT+5*256,y
  ora mask
  sta PAT+5*256,y
  dex
  beq YR_done5
  lda err
  sec
  sbc dx
  sta err
  bcs YR5_next
  adc dy
  sta err
  lsr mask
  bcc YR5_next
  ror mask
  jsr tileRight
YR5_next:
  jmp YR6
YR_done5:
  rts
YR6:
  lda PAT+6*256,y
  ora mask
  sta PAT+6*256,y
  dex
  beq YR_done6
  lda err
  sec
  sbc dx
  sta err
  bcs YR6_next
  adc dy
  sta err
  lsr mask
  bcc YR6_next
  ror mask
  jsr tileRight
YR6_next:
  jmp YR7
YR_done6:
  rts
YR7:
  lda PAT+7*256,y
  ora mask
  sta PAT+7*256,y
  dex
  beq YR_done7
  lda err
  sec
  sbc dx
  sta err
  bcs YR7_next
  adc dy
  sta err
  lsr mask
  bcc YR7_next
  ror mask
  jsr tileRight
YR7_next:
  jsr tileDown
  jmp YR0
YR_done7:
  rts
xdTab: .word XD0,XD1,XD2,XD3,XD4,XD5,XD6,XD7
xuTab: .word XU0,XU1,XU2,XU3,XU4,XU5,XU6,XU7
yrTab: .word YR0,YR1,YR2,YR3,YR4,YR5,YR6,YR7
ylTab: .word YL0,YL1,YL2,YL3,YL4,YL5,YL6,YL7
maskL: .byte 255,127,63,31,15,7,3,1
maskR: .byte 128,192,224,240,248,252,254,255
