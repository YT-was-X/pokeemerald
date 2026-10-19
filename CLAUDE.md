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
- **Instant text:** `RunTextPrinters` in `src/text.c` draws every character up to the next prompt, scroll or pause in one frame (capped at `MAX_TEXT_CHARS_PER_FRAME`), at every text speed (`AddTextPrinter` sets the per-character delay to 0), and copies the window to VRAM once per frame. Line scrolling also happens in one frame. The Text Speed row was removed from the Options menu (`src/option_menu.c`); `optionsTextSpeed` stays in the save but is unused.
- **Double-speed Birch intro:** while `sBirchSpeechDoubleSpeed` is set (`src/main_menu.c`), `CB2_MainMenu` runs tasks and sprite animations twice per frame, clearing new key presses before the second pass. Normal palette fades still run at normal speed.
- **Instant Nurse Joy healing:** `Common_EventScript_PkmnCenterNurse` in `data/scripts/pkmn_center_nurse.inc` goes straight to healing, skipping the yes/no question (and the Gold Card greeting, which is now unused).
- **Shiny Charm (new key item `ITEM_SHINY_CHARM`, after `ITEM_OLD_SEA_MAP`):** uses the Star Piece icon recolored gold (`graphics/items/icon_palettes/shiny_charm.pal`). Put in the PC once the first time the bedroom or Poké Center PC is opened (`GiveShinyCharmOnce` in `src/player_pc.c`, guarded by `FLAG_RECEIVED_SHINY_CHARM`, formerly `FLAG_UNUSED_0x020`). While it's in the **bag**, Pokémon generated for the player roll their personality up to `SHINY_CHARM_ROLLS` (3) times and keep the first shiny one (`GetShinyRolls`/`IsShinyForPlayer` in `src/pokemon.c`; used by `CreateBoxMon`, `CreateMonWithNature`, `CreateMonWithGenderNatureLetter` and daycare eggs in `src/daycare.c`). Odds go from 1/8192 to about 1/2731.
- **Badge shiny bonus:** each gym badge adds a 1-in-8 chance of one extra roll in `GetShinyRolls` (8 badges = always +1). Odds: 1/8192 with no badges, 1/4096 with all 8; with the charm, 1/2731 to 1/2048. The shiny threshold (`SHINY_ODDS`) is unchanged, so already-caught Pokémon never change appearance.
- **Event islands unlocked:** after `FLAG_SYS_GAME_CLEAR`, the first PC visit puts the Eon Ticket, Aurora Ticket, Old Sea Map and Mystic Ticket in the PC and sets the four `FLAG_ENABLE_SHIP_*` flags, like the Mystery Gift scripts did (`GiveEventTicketsOnce` in `src/player_pc.c`, guarded by `FLAG_RECEIVED_EVENT_TICKETS`, formerly `FLAG_UNUSED_0x021`). The Lilycove ferry needs the ticket in the bag. The island maps and scripts are vanilla.

## Where things live
- Starting bedroom PC items: `sNewGamePCItems` in `src/player_pc.c` (vanilla: 1 Potion)
- New-game setup: `src/new_game.c`
- Item data and prices: `src/data/items.h`
- Mart item lists: the `pokemart` lists in `data/maps/*/scripts.inc`
- Event flags and vars: `include/constants/flags.h`, `include/constants/vars.h`
