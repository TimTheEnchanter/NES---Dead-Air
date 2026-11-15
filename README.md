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

Missions: `gen_tables.py` holds each mission as 14-byte steps, one per nav point: kind (empty, fight, land), position, arrival and clear radio lines, ship count, hit points, freighter bits, wait, and a time limit after which the survivors escape. Kind 3 is an ambush: a silent wait, then the warning line and ships spawned behind the player. Freighter bits with $80 make fixed targets standing on end at the nav point (the relay buoy). Big hulls collide as three spheres along their length. Keep nav points within about 1300 units of the Vigil on each axis: the world wraps at 4096. `src/mission.inc` plays them. The Vigil is ship 3 and freighters use the same slab hull, scaled with `shipModel` as a left shift. `TESTM`, `TESTSTEP`, and `TESTBACK` start a build at a given nav point.

Mission modes (`missionModeTab`): 0 Hale on your wing, 1 Mara (her lines via `radioMara`), 2 Ren's shuttle in the wingman slot, no wingman (`radioRen`). Kind 4 is defend: hold for the time limit while waves attack; the wait byte holds a halfway line. Ship models (`shipModel`): 0 scout, 1-3 slab at that scale, 4 small slab (shuttle), 8 lancer. Freighter byte: bits 0-1 freighters, 2-3 lancers, $40 big, $80 hold. Mission code lives in the fixed HIGH bank. A second flags byte (ops FLAG2, IF2) is saved at $7F06 with a check byte at $7F07.

Echo tint: `src/tint.inc`. The echo drone (`shipModel` 16, a scout hull) is drawn in background palette 1 by setting the attribute squares under its vertices. The main loop builds `attrWork`; `tint_send` hands it over with the frame; transfer state ST_ATTR (after ST_NEW) waits for the picture-bottom IRQ, which writes the 30 attribute bytes into the hidden nametable and sets palette 1 for the buffer the next NMI will show. Nothing in the cycle-exact NMI path changed. Mission 5 pieces (`src/mission2.inc`): step kind 5 (guard) puts slot 0 on the wingman's tail with a timer; the echo drone never fires, breaks high and left, and drifts when down to its last hit, setting a hidden flag if left alone. Mission mode 3 is flying alone (`radioSolo`).

Protecting: `protSlot` is the ship enemies attack and whose hits are counted (`prot_hits` in `src/mission3.inc`): Ren's shuttle in 4A, or the wreck in Mission 6, which stands in the Vigil's slot until the landing step brings the Vigil back (`vigil_home`). A defend step with freighter-byte $10 protects the wreck.

Mission 7 (`src/mission4.inc`): Concord pilots (`shipModel` 32, scout hull, tinted; `tintTab` sets the tint colour per mission) are disabled at their last hit. Step kind 6 (sneak) places a picket in slot 2 at the nav point plus (count*8, hp*8); inside `WARN_R` Ren warns, inside `DETECT_R` two scouts launch and a spare fighter is lost. Kind 7 (finish) ends the mission without landing. Mode 3 flies with nobody on your wing.

Finale (Mission 8): fight steps use the wait byte as a code: bit 7 makes slot 0 that model, fixed in place (the Serrath carrier is `shipModel` $0A, the lancer hull at scale 2); 1 skips the wave if the Concord patrol came; 2 drops one echo if you spared Teller. The Array is the slab hull at scale 3, standing on end in the Vigil's slot. The finale script is in bank 5 (`SCRIPT3`).

Radio: text lives in bank 6 (`RADIO`). `radio_fetch` maps it for a moment, copies the line into `radioBuf`, and maps the flight bank back; the picture-bottom interrupt only reads RAM.

Story tests: `STORYTARGET=<label> python3 story/gen_story.py` plus `-D STORYTEST=1 -D STORYF2=<flags>` builds a ROM that opens at that scene in bank 5.

Picture timing: `test/slips.py` needs a `DEBUG` build. It finds the fixed marker line and counts frames where it is off its usual row (27) or missing. The clean range of `BUD_B` is about 4465-4500; 4485 is the setting.

Spare fighters: story flag bits 5-7 count fighters lost (script op LOST); op SPARE prints how many remain of `SPARE_START`.

Campaign: scene scripts in `story/gen_story.py` use FLY, IF, SAVE, LOAD, LOC, and TITLE ops. The save is six bytes with a checksum at $7F00 in battery RAM. `missionId` 0 is the free arena used by `AUTOSTART` tests.

Banks: bank 0 ($8000) holds tables and never changes. $A000 holds flight code (bank 1), story assets (2), or scripts (3 up). Banks 30-31 are fixed. Interrupts never switch banks, and `test/bankcheck.py` checks that fixed code cannot reach the flight bank during story mode. Bank 5-29 are free.

AI: `src/ai.inc` moves every AI ship each frame but steers only one, in rotation. Hale is ship 2 in missions and cannot be hit or collided with. Radio lines come from `gen_tables.py` and appear on a text row under the view, using 32 reserved background tiles shared by both bitplanes.

`test/bot.mjs` is a closed-loop pilot: `node test/bot.mjs dead-air.nes <labels> <runs> <reaction ticks> <aim window>`. Its results are the best check on difficulty.
