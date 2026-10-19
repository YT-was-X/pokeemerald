# CLAUDE.md

Personal Pokémon Emerald ROM hack built on the pret decompilation.

## Branches
- `postgame-tweaks`: all hack work happens here. Commit and push changes to it.
- `master`: untouched mirror of pret/pokeemerald (byte-matching build). Don't commit hack changes to it.
- `expansion`: stock rh-hideout/pokeemerald-expansion. Separate codebase with an incompatible save format; leave it alone unless asked.

## Building
- The SessionStart hook (`.claude/hooks/session-start.sh`) installs the toolchains only.
- **Only build when the user asks.** Use `make modern -j4`, which produces `test_PokemonEmerald_Modern.gba`.
- The matching build (`make`) no longer reproduces `rom.sha1` on this branch. That's expected.
- To compile a single file without building the ROM: `make MODERN=1 build/modern/src/<file>.o`.
- After a build, send `test_PokemonEmerald_Modern.gba` to the user with SendUserFile (`display: attach`). They test it on a phone emulator.

## Save compatibility
The user plays on existing saves. Avoid changes to the `SaveBlock1`, `SaveBlock2` or `PokemonStorage` layouts (`include/global.h`, `include/pokemon_storage_system.h`); they break existing saves. If a change requires it, say so first. SaveBlock1 has only about 120 bytes free.

## Changes so far
- **Post-game Master Balls:** after `FLAG_SYS_GAME_CLEAR`, any normal mart that sells a Poké Ball item also lists the Master Ball first (`AddPostGameMartItems` in `src/shop.c`). The Master Ball price is 50,000 (`src/data/items.h`).
- **Running Shoes from the start:** the `FLAG_SYS_B_DASH` check was removed from the run check in `src/field_player_avatar.c`, so running works on existing saves too. Mom's Running Shoes scene in Littleroot still plays; it's harmless. Maps that disallow running still do.
- **Running in Poké Centers and Marts:** `allow_running` set to true in the `map.json` of every `*_PokemonCenter_1F/2F`, every `*_Mart` and `EverGrandeCity_PokemonLeague_1F`.
- **Instant text:** `RunTextPrinters` in `src/text.c` draws every character up to the next prompt, scroll or pause in one frame (capped at `MAX_TEXT_CHARS_PER_FRAME`), at every text speed (`AddTextPrinter` sets the per-character delay to 0), and copies the window to VRAM once per frame. Line scrolling also happens in one frame. The Options Text Speed setting no longer has a visible effect.
- **Instant Nurse Joy healing:** `Common_EventScript_PkmnCenterNurse` in `data/scripts/pkmn_center_nurse.inc` goes straight to healing, skipping the yes/no question (and the Gold Card greeting, which is now unused).

## Where things live
- Starting bedroom PC items: `sNewGamePCItems` in `src/player_pc.c` (vanilla: 1 Potion)
- New-game setup: `src/new_game.c`
- Item data and prices: `src/data/items.h`
- Mart item lists: the `pokemart` lists in `data/maps/*/scripts.inc`
- Event flags and vars: `include/constants/flags.h`, `include/constants/vars.h`
