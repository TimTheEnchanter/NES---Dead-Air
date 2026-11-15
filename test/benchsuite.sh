#!/bin/sh
# Speed suite: test/benchsuite.sh [source dir] [updates]
# Builds -D BENCH=1 test starts and runs test/bench.mjs on each.
D=${1:-.}; N=${2:-300}; A=${CC65:-$HOME/tools/cc65/bin}; T=$(dirname $0)
for cfg in ${CFGS:-"arena:TESTM=0" "m2-fight:TESTM=2 TESTSTEP=1 TESTBACK=700" "m2-convoy:TESTM=2 TESTSTEP=2 TESTBACK=700" "m8-lancers:TESTM=10 TESTSTEP=0 TESTBACK=700" "m8-carrier:TESTM=10 TESTSTEP=1 TESTBACK=700" "m8-echoes:TESTM=10 TESTSTEP=2 TESTBACK=700"}; do
  name=${cfg%%:*}; defs=""; for d in ${cfg#*:}; do defs="$defs -D $d"; done
  (cd $D && $A/ca65 -g -D AUTOSTART=1 -D BENCH=1 $defs wire.s -o /tmp/bs.o && $A/ld65 -C nes.cfg /tmp/bs.o -o /tmp/bs.nes -Ln /tmp/bs.lbl --dbgfile /tmp/bs.dbg) || exit 1
  printf "%-11s " $name; node $T/bench.mjs /tmp/bs.nes /tmp/bs.lbl $N
done
