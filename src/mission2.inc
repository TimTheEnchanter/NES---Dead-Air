; ---------- Mission 5 pieces: the echo drone and the wingman in danger ----------
.pushseg
.segment "MATH"
F2_HALE_LOST = 4
F2_SPARED    = 8
F2_PATROL    = 64
ECHO_SPARE   = 480     ; vblanks the drone drifts before it slips away
ECHO_FRE     = $20     ; freighter byte: slot 0 is the echo drone
CONCORD_FRE  = $40     ; with no freighter bits: the wave is Concord pilots
GUARD_WARN   = 2       ; high byte of the timer when Hale calls for help again

; After a fight spawns: make slot 0 the echo drone if the step says so.
spawn_post:
  ldy #SP_KIND
  lda (spadP),y
  cmp #S_FIGHT
  bne post2
  ldy #SP_WAIT
  lda (spadP),y
  bpl :+
  and #$7F             ; wait bit 7: slot 0 is this model, fixed in place
  sta shipModel
  lda #2
  sta hPassive
  lda #4               ; its escort is an ordinary scout
  sta hp+1
  jmp post2
: cmp #1
  bne :+
  lda storyFlags2      ; wait 1: the Concord patrol takes this wave for you
  and #F2_PATROL
  beq post2
  lda #0
  sta alive
  sta alive+1
  sta killsLeft
  lda #R_PATROLHELP
  jmp postSay
: cmp #2
  bne post2
  lda storyFlags2      ; wait 2: Teller holds one of them off
  and #F2_SPARED
  beq post2
  lda #0
  sta alive+1
  lda #1
  sta killsLeft
  lda #R_TELLERHELP
postSay:
  ldx #0
  stx radioT
  jsr say_radio
post2:
  ldy #SP_FRE
  lda (spadP),y
  and #3
  bne :++
  lda (spadP),y
  and #CONCORD_FRE     ; no freighters: this bit means Concord pilots
  beq :++
  ldx killsLeft
: dex
  bmi :+
  lda #CONCORD_MODEL
  sta shipModel,x
  jmp :-
: ldy #SP_FRE
  lda (spadP),y
  and #WRECK_FRE
  beq :+
  lda protSlot
  bne :+               ; already set by the first wave
  lda #VIGIL           ; a defence of the wreck or the Vigil: it takes the hits
  sta protSlot
  lda #200
  sta protR
  lda missionId        ; Six's fighter is smaller than a wreck
  cmp #M_LANTERN2
  lda #WRECK_HP
  bcc protHp
  lda #SIX_HP
protHp:
  jsr easy_half        ; Easy: half again as tough
  sta shuttleHP
: ldy #SP_FRE
  lda (spadP),y
  and #ECHO_FRE
  beq :+
  lda #ECHO_MODEL
  sta shipModel
  lda #0
  sta echoDis
  sta echoT
  sta echoT+1
  sta echoBreaks
: rts

; A hit has just landed on hitShip. At its last hit the drone stops and drifts.
echo_hit:
  ldx hitShip
  lda shipModel,x
  cmp #CONCORD_MODEL
  bne :+
  jmp concord_hit
: cmp #ECHO_MODEL
  bne :+
  lda hp,x
  cmp #1
  bne :+
  lda echoDis
  bne :+
  inc echoDis
  lda #1
  sta hPassive,x       ; no more steering
  sta hTurn,x          ; slow and straight
  lda #0
  sta hBreak,x
  lda #R_ECHOSTOP
  ldx #0
  stx radioT
  jsr say_radio
: rts

; Called at the start of a break turn, advI = turn. The echo turns the other
; way and climbs, and the wingman notices the second and third time.
echo_break:
  ldx ei
  lda shipModel,x
  cmp #ECHO_MODEL
  bne :++
  lda #0
  sec
  sbc advI
  sta advI
  lda hPitch,x         ; positive pitch points the nose down: climb
  sec
  sbc aiStep
  sbc aiStep
  sta hPitch,x
  inc echoBreaks
  lda echoBreaks
  cmp #2
  bne :+
  lda #R_HIGHLEFT
  ldx #0
  stx radioT
  jsr say_radio
  ldx ei
  rts
: cmp #4
  bne :+
  lda #R_THATSHER
  ldx #0
  stx radioT
  jsr say_radio
: ldx ei
  rts

; Each frame in a mission: a drone left alone slips away, and that is noted.
echo_tick:
  lda echoDis
  beq :+
  lda alive
  beq :+
  lda shipModel
  cmp #ECHO_MODEL
  bne :+
  lda echoT
  clc
  adc dt
  sta echoT
  bcc :+
  inc echoT+1
: lda echoDis
  beq echoDone
  lda echoT+1
  cmp #>ECHO_SPARE
  bcc echoDone
  lda echoT
  cmp #<ECHO_SPARE
  bcc echoDone
  lda alive
  beq echoDone
  lda #0               ; spared: it drifts out of sight
  sta alive
  sta echoDis
  dec killsLeft
  lda storyFlags2
  ora #F2_SPARED
  sta storyFlags2
  lda #R_ECHOLEFT
  ldx #0
  stx radioT
  jsr say_radio
echoDone:
  rts

; Guard step arrival: spawn, then put slot 0 right on Hale's tail.
guard_start:
  jsr spawn_step
  lda #0
  sta guardSaid
  lda alive+HALE
  bne :+
  rts                  ; no wingman to save
: lda fvX+HALE
  ldx #0
  jsr behind_hale
  lda fvY+HALE
  ldx #16
  jsr behind_hale
  lda fvZ+HALE
  ldx #32
  jsr behind_hale
  lda hYaw+HALE        ; chasing him, on his heading
  sta hYaw
  lda hPitch+HALE
  sta hPitch
  lda fvX+HALE
  sta fvX
  lda fvY+HALE
  sta fvY
  lda fvZ+HALE
  sta fvZ
  lda #3               ; the one on his tail is a little easier to finish
  sta hp
  lda #1
  sta guardOn
  ldy #SP_LIMIT
  lda (spadP),y
  jsr easy_more        ; Easy: more time to save him
  sta guardT+1
  lda #0
  sta guardT
  rts

; A = one component of Hale's heading, X = axis offset (0, 16, 32):
; slot 0 goes twice that far behind him.
behind_hale:
  sta u1
  ldy #0
  cmp #$80
  bcc :+
  dey
: sty u0
  asl u1
  rol u0
  lda shipPosL+HALE,x
  sec
  sbc u1
  sta shipPosL,x
  lda shipPosH+HALE,x
  sbc u0
  and #$0F
  sta shipPosH,x
  rts

; Each frame of a guard step: Hale is guardLost if the timer runs out first.
guard_tick:
  lda guardOn
  bne :+
  rts
: lda alive
  bne :+
  lda #0               ; you got it off him
  sta guardOn
  lda #R_THANKS
  ldx #0
  stx radioT
  jmp say_radio
: lda guardT
  sec
  sbc dt
  sta guardT
  bcs :+
  dec guardT+1
  bmi guardLost
: lda guardT+1
  cmp #GUARD_WARN
  bne :+
  lda guardSaid
  bne :+
  inc guardSaid
  lda #R_HURRY
  ldx #0
  stx radioT
  jmp say_radio
: rts
guardLost:
  lda #0
  sta guardOn
  sta alive+HALE
  lda #R_HALEHIT
  ldx #0
  stx radioT
  jsr say_radio
  lda #3               ; flying alone now: his lines go quiet
  sta missionMode
  lda storyFlags2
  ora #F2_HALE_LOST
  sta storyFlags2
  rts
.popseg
