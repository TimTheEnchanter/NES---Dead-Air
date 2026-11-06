.segment "FLIGHT"
; ---------- multiply ----------
; Quarter-square style: T(a+m)-T(a-m) gives a*m/128 in 8.8.
; SETM prepares the pointers for multiplier A, which must not be -128: the
; negated pointer's page is fixed, and -128 is the one value that would change it. PROD then multiplies
; by the biased value in Y, leaving lo in X and hi in A.
.macro SETM base
  eor #$80
  sta base
  sta base+4
  eor #$FF
  clc
  adc #1
  sta base+2
  sta base+6
.endmacro
.macro PROD base
  lda (base),y
  sec
  sbc (base+2),y
  tax
  lda (base+4),y
  sbc (base+6),y
.endmacro
; First term goes to t0/t1, later terms accumulate there.
.macro TERM1 base
  PROD base
  stx t0
  sta t1
.endmacro
.macro TERMN base
  PROD base
  sta u0
  txa
  clc
  adc t0
  sta t0
  lda u0
  adc t1
  sta t1
.endmacro

; A * X (signed) -> hi byte in A
fmul:
  stx u1
  SETM tmpM
  lda u1
  eor #$80
  tay
  PROD tmpM
  rts

