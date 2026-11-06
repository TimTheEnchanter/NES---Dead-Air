#!/bin/bash
# Frame rate and accuracy for a ROM source, across the standard flights.
cd ~/nes-wire; C=~/tools/mesen-libretro/Libretro/mesen_libretro.so; DEF="-D AUTOSTART=1 $*"
for pair in "still:/dev/null" "evade:test/flights/evade.txt" "fire:test/flights/holdfire.txt"; do n=${pair%%:*}; f=${pair#*:}
 ~/tools/cc65/bin/ca65 $DEF wire.s -o /tmp/s.o && ~/tools/cc65/bin/ld65 -C nes.cfg /tmp/s.o -o /tmp/s.nes
 INPUT=$f ./test/run $C /tmp/s.nes 3660 /tmp/q | tail -1 | awk -v n=$n '{split($2,a,"=");split($3,b,"=");printf "%-6s %.1f fps\n",n,a[2]*60/b[2]}'
 ~/tools/cc65/bin/ca65 -D NOSPRITE=1 $DEF wire.s -o /tmp/s.o && ~/tools/cc65/bin/ld65 -C nes.cfg /tmp/s.o -o /tmp/r.nes
 INPUT=$f RAW=/tmp/fr.bin ./test/run $C /tmp/r.nes 900 /tmp/q >/dev/null; echo "       $(python3 test/ref.py /tmp/fr.bin 2>&1 | head -1)"
done
~/tools/cc65/bin/ca65 -D NOSPRITE=1 -D NSHIPS=1 $DEF wire.s -o /tmp/s.o && ~/tools/cc65/bin/ld65 -C nes.cfg /tmp/s.o -o /tmp/s1.nes
VIS=1 INPUT=test/flights/evade.txt RAW=/tmp/v.bin ./test/run $C /tmp/s1.nes 2400 /tmp/q >/dev/null; python3 test/cull.py /tmp/v.bin
