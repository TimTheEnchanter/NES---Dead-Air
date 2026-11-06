# Dead Air

An NES wireframe space-combat game. `dead-air.nes` is the game. The engine source keeps the name `wire.s`. The `AUTOSTART` test builds open a free arena with a hunter ship and three tumbling targets, and `wire2.nes` has two.
Controls: Start begins, the D-pad steers, A fires, and holding B with up or down sets speed.
Mapper 4 (MMC3), 256K PRG, 8K battery WRAM, 8K CHR RAM, NTSC.

Build: `make` (needs ca65/ld65; adjust the paths at the top of the Makefile).

- `wire.s`: top level, constants, and includes
- `src/`: memory, flight, drawing, main loop, story engine, uploads, sound, and boot code
- `nes.cfg`: the bank map; RAM is allocated by the linker, so overlaps fail the build
- `gen_tables.py`: math tables and ship model, written to `tables.inc`
- `test/flights/`: scripted inputs for the crash, shooting, respawn, and game-over tests
- `test/`: headless Mesen runner, frame checks, `ref.py` reference model, `enemy.py` AI checks, cycle profiler

Timing: rendering must start at a fixed moment, so `run_xfer` charges every code
path to `bud`. If you change that code, refit the `K_` charges with `test/fit.mjs`, then count
slips at several `BUD_B` values with `test/slips.py` and centre it in the clean range.
`SETM` multipliers must never be -128. `test/compare.sh` runs the standard flights.

Web preview: `web/build.py` embeds `dead-air.nes` and a bundled jsnes into
`web/dead-air.html`. The published page can also load a newer `.nes` file.

Sound: `sound_tick` runs in the picture-bottom interrupt, so its length cannot move the
picture. Effects are tables in `gen_tables.py`. `test/audio.py` checks captured audio
against recorded game events. `sound-demo.wav` is a capture from Mesen.

Game modes: title, play, and game over are in `attract` and `draw_mode`. Text is drawn with the line
renderer from `gen_tables.py`. Old test flights build with `-D AUTOSTART=1` to skip the title.

Story: `story/art.py` draws the portraits as code, and `story/gen_story.py` builds tiles, screen layout, and the scene script into `story.inc`. Story mode has its own NMI path and rebuilds flight on exit.

Missions: `gen_tables.py` holds each mission as 13-byte steps, one per nav point: kind (empty, fight, land), position, arrival and clear radio lines, ship count, hit points, freighter bits, and wait. `src/mission.inc` plays them. The Vigil is ship 3 and freighters use the same slab hull, scaled with `shipModel` as a left shift. `TESTM`, `TESTSTEP`, and `TESTBACK` start a build at a given nav point.

Campaign: scene scripts in `story/gen_story.py` use FLY, IF, SAVE, LOAD, LOC, and TITLE ops. The save is six bytes with a checksum at $7F00 in battery RAM. `missionId` 0 is the free arena used by `AUTOSTART` tests.

Banks: bank 0 ($8000) holds tables and never changes. $A000 holds flight code (bank 1), story assets (2), or scripts (3 up). Banks 30-31 are fixed. Interrupts never switch banks, and `test/bankcheck.py` checks that fixed code cannot reach the flight bank during story mode. Bank 5-29 are free.

AI: `src/ai.inc` moves every AI ship each frame but steers only one, in rotation. Hale is ship 2 in missions and cannot be hit or collided with. Radio lines come from `gen_tables.py` and appear on a text row under the view, using 32 reserved background tiles shared by both bitplanes.

`test/bot.mjs` is a closed-loop pilot: `node test/bot.mjs dead-air.nes <labels> <runs> <reaction ticks> <aim window>`. Its results are the best check on difficulty.
