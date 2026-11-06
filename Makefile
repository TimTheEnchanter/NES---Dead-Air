CA=$(HOME)/tools/cc65/bin/ca65
LD=$(HOME)/tools/cc65/bin/ld65
all: dead-air.nes wire2.nes wire_dbg.nes
story.inc: story/gen_story.py story/art.py
	python3 story/gen_story.py
tables.inc lines.inc: gen_tables.py
	python3 gen_tables.py
dead-air.nes: wire.s $(wildcard src/*.inc) tables.inc lines.inc story.inc nes.cfg
	$(CA) wire.s -o wire.o && $(LD) -C nes.cfg wire.o -o $@ -m wire.map
wire2.nes: wire.s $(wildcard src/*.inc) tables.inc lines.inc story.inc nes.cfg
	$(CA) -D NSHIPS=2 wire.s -o wire2.o && $(LD) -C nes.cfg wire2.o -o $@
wire4.nes: wire.s $(wildcard src/*.inc) tables.inc lines.inc story.inc nes.cfg
	$(CA) -D NSHIPS=4 wire.s -o wire4.o && $(LD) -C nes.cfg wire4.o -o $@
wire_dbg.nes: wire.s $(wildcard src/*.inc) tables.inc lines.inc story.inc nes.cfg
	$(CA) -D DEBUG=1 wire.s -o wire_dbg.o && $(LD) -C nes.cfg wire_dbg.o -o $@
