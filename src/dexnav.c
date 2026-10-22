// DexNav: a Start menu screen listing the current map's land and water Pokémon,
// and a search mode that hides the chosen species in a nearby grass, sand,
// cave or water tile. Sneaking up on it starts a battle with bonuses that
// grow with the species' search level: perfect IVs ("potential" stars), a
// better held item chance and extra shiny rolls. Ported and simplified from
// pokeemerald-expansion's DexNav (no hidden Pokémon, egg moves or abilities).
#include "global.h"
#include "battle.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "dexnav.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "map_name_popup.h"
#include "menu.h"
#include "menu_helpers.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "pokemon_storage_system.h"
#include "pokemon_summary_screen.h"
#include "random.h"
#include "region_map.h"
#include "safari_zone.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "wild_encounter.h"
#include "window.h"
#include "constants/field_effects.h"
#include "constants/items.h"
#include "constants/map_types.h"
#include "constants/pokemon_icon.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/wild_encounter.h"

// Search rules
#define DEXNAV_TIMEOUT_FRAMES   (30 * 60) // The Pokémon leaves after 30 seconds
#define SNEAKING_PROXIMITY      4         // Within this many tiles, running or cycling scares it off
#define MAX_PROXIMITY           20        // Farther than this, the signal is lost
#define MIN_SPAWN_DISTANCE      3
#define SCAN_RADIUS_X           7         // The hiding spot is picked on screen
#define SCAN_RADIUS_Y           4

#define DEXNAV_CHAIN_MAX        100
#define SEARCH_LEVEL_MAX        255
#define MAX_POTENTIAL           3

enum {
    ENVIRONMENT_LAND,
    ENVIRONMENT_WATER,
};

// VAR_DEXNAV_SPECIES holds the registered species and its environment
#define DEXNAV_VAR_SPECIES_MASK 0x3FFF
#define DEXNAV_VAR_ENV_SHIFT    14

// Screen layout
enum {
    ROW_WATER,
    ROW_LAND_TOP,
    ROW_LAND_BOT,
    ROWS_COUNT,
};

#define COL_WATER_COUNT         NUM_WATER_MONS_ENCOUNTER_SLOTS
#define COL_LAND_COUNT          (NUM_LAND_MONS_ENCOUNTER_SLOTS / 2)

#define ROW_WATER_ICON_X        30
#define ROW_WATER_ICON_Y        35
#define ROW_LAND_ICON_X         20
#define ROW_LAND_TOP_ICON_Y     72
#define ROW_LAND_BOT_ICON_Y     (ROW_LAND_TOP_ICON_Y + 28)
#define ICON_SPACING            24

#define SPECIES_INFO_Y          5
#define SEARCH_LEVEL_Y          (SPECIES_INFO_Y + 48)
#define HELD_ITEM_INFO_Y        (SEARCH_LEVEL_Y + 24)
#define CHAIN_INFO_Y            (HELD_ITEM_INFO_Y + 24)

#define TYPE_ICONS_Y            69

#define TAG_DEXNAV_CURSOR       0x4005
#define TAG_DEXNAV_CAPTURED_ALL 0x4002
#define TAG_DEXNAV_NO_DATA      0x4007
#define TAG_DEXNAV_OWNED        0x4003
#define TAG_DEXNAV_STAR         0x4010

// Search window (overworld BG 0)
#define SEARCH_WINDOW_WIDTH     28
#define SEARCH_WINDOW_HEIGHT    3
#define SEARCH_WINDOW_BASE_TILE 0x100
#define SEARCH_FRAME_BASE_TILE  0x1D5
#define SEARCH_WINDOW_PALETTE   13

#define SPECIES_ICON_X          22
#define SEARCH_TEXT_X           32
#define SEARCH_LEVEL_TEXT_X     100
#define SEARCH_CHAIN_TEXT_X     150
#define SEARCH_ARROW_X          206

enum {
    WIN_INFO,
    WIN_REGISTERED,
};

struct DexNavSearch
{
    u16 species;
    u16 heldItem;
    u8 environment;
    u8 monLevel;
    u8 potential;
    u8 searchLevel;
    u8 proximity;
    s16 tileX;
    s16 tileY;
    u8 fldEffId;
    u8 fldEffSpriteId;
    u8 windowId;
    u8 iconSpriteId;
    u8 ownedSpriteId;
    u8 itemSpriteId;
    u8 starSpriteIds[MAX_POTENTIAL];
    u32 startTime;
};

struct DexNavGui
{
    u8 state;
    u8 cursorSpriteId;
    u8 cursorRow;
    u8 cursorCol;
    u8 typeIconSpriteIds[2];
    u16 landSpecies[NUM_LAND_MONS_ENCOUNTER_SLOTS];
    u16 waterSpecies[NUM_WATER_MONS_ENCOUNTER_SLOTS];
};

EWRAM_DATA u16 gDexNavSpecies = SPECIES_NONE;
EWRAM_DATA static struct DexNavSearch *sDexNavSearch = NULL;
EWRAM_DATA static struct DexNavGui *sDexNavGui = NULL;
EWRAM_DATA static u8 *sBg1TilemapBuffer = NULL;
EWRAM_DATA static u16 sPendingSearchSpecies = SPECIES_NONE;
EWRAM_DATA static u8 sPendingSearchEnvironment = 0;

static void CB2_DexNavSetup(void);
static void Task_DexNavMain(u8 taskId);
static void PrintCurrentSpeciesInfo(void);
static bool8 StartDexNavSearch(u16 species, u8 environment);

static const u32 sDexNavGuiTiles[] = INCGFX_U32("graphics/dexnav/gui_tiles.png", ".4bpp.lz");
static const u32 sDexNavGuiTilemap[] = INCGFX_U32("graphics/dexnav/gui_tilemap.bin", ".lz");
static const u16 sDexNavGuiPal[] = INCGFX_U16("graphics/dexnav/gui.pal", ".gbapal");
static const u32 sCursorGfx[] = INCGFX_U32("graphics/dexnav/cursor.png", ".4bpp.lz");
static const u16 sCursorPal[] = INCGFX_U16("graphics/dexnav/cursor.png", ".gbapal");
static const u32 sCapturedAllGfx[] = INCGFX_U32("graphics/dexnav/captured_all.png", ".4bpp.lz");
static const u32 sNoDataGfx[] = INCGFX_U32("graphics/dexnav/no_data.png", ".4bpp.lz");
static const u32 sStarGfx[] = INCGFX_U32("graphics/dexnav/star.png", ".4bpp.lz");
static const u32 sOwnedGfx[] = INCGFX_U32("graphics/dexnav/owned_icon.png", ".4bpp.lz");
static const u16 sSearchWindowPal[] = INCGFX_U16("graphics/text_window/dexnav.pal", ".gbapal");

static const u8 sText_NoInfo[] = _("--------");
static const u8 sText_CaptureFirst[] = _("Capture first!");
static const u8 sText_PressRToRegister[] = _("{R_BUTTON} TO REGISTER!");
static const u8 sText_SearchFor[] = _("SEARCH {STR_VAR_1}");
static const u8 sText_MonLevel[] = _("{LV}{STR_VAR_1}");
static const u8 sText_Chain[] = _("CHAIN {STR_VAR_1}");
static const u8 sText_ArrowLeft[] = _("{LEFT_ARROW}");
static const u8 sText_ArrowRight[] = _("{RIGHT_ARROW}");
static const u8 sText_ArrowUp[] = _("{UP_ARROW}");
static const u8 sText_ArrowDown[] = _("{DOWN_ARROW}");

static const u8 sFontColor_Black[3] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sFontColor_White[3] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};
static const u8 sSearchFontColor[3] = {1, 15, 13};

// Chance in % of 1, 2 and 3 perfect IVs, by search level (see sSearchLevelBrackets)
static const u8 sSearchLevelBrackets[] = {5, 10, 25, 50, 100};
static const u8 sPotentialChances[ARRAY_COUNT(sSearchLevelBrackets) + 1][MAX_POTENTIAL] =
{
    { 0,  0,  0}, //   0-4
    {14,  1,  0}, //   5-9
    {17,  9,  1}, //  10-24
    {17, 16,  7}, //  25-49
    {15, 17,  6}, //  50-99
    { 8, 24, 12}, // 100+
};

static const struct BgTemplate sDexNavBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .priority = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 3,
        .mapBaseIndex = 30,
        .priority = 1
    },
};

static const struct WindowTemplate sDexNavWindowTemplates[] =
{
    [WIN_INFO] =
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 5,
        .width = 9,
        .height = 15,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    [WIN_REGISTERED] =
    {
        .bg = 0,
        .tilemapLeft = 4,
        .tilemapTop = 0,
        .width = 26,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 200,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct OamData sOam_8x8 =
{
    .shape = SPRITE_SHAPE(8x8),
    .size = SPRITE_SIZE(8x8),
    .priority = 0,
};

static const struct OamData sOam_32x32 =
{
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 0,
};

static const struct OamData sOam_NoData =
{
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 1,
};

static const struct CompressedSpriteSheet sSpriteSheet_Cursor = {sCursorGfx, 32 * 32 / 2, TAG_DEXNAV_CURSOR};
static const struct SpritePalette sSpritePalette_Cursor = {sCursorPal, TAG_DEXNAV_CURSOR};
static const struct CompressedSpriteSheet sSpriteSheet_CapturedAll = {sCapturedAllGfx, 8 * 8 / 2, TAG_DEXNAV_CAPTURED_ALL};
static const struct CompressedSpriteSheet sSpriteSheet_NoData = {sNoDataGfx, 32 * 32 / 2, TAG_DEXNAV_NO_DATA};
static const struct CompressedSpriteSheet sSpriteSheet_Star = {sStarGfx, 8 * 8 / 2, TAG_DEXNAV_STAR};
static const struct CompressedSpriteSheet sSpriteSheet_Owned = {sOwnedGfx, 8 * 8 / 2, TAG_DEXNAV_OWNED};

static const struct SpriteTemplate sSpriteTemplate_Cursor =
{
    .tileTag = TAG_DEXNAV_CURSOR,
    .paletteTag = TAG_DEXNAV_CURSOR,
    .oam = &sOam_32x32,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_CapturedAll =
{
    .tileTag = TAG_DEXNAV_CAPTURED_ALL,
    .paletteTag = TAG_DEXNAV_CURSOR,
    .oam = &sOam_8x8,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_NoData =
{
    .tileTag = TAG_DEXNAV_NO_DATA,
    .paletteTag = POKE_ICON_BASE_PAL_TAG,
    .oam = &sOam_NoData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

// The search window's small icons all use the held item palette
static const struct SpriteTemplate sSpriteTemplate_HeldItem =
{
    .tileTag = TAG_HELD_ITEM,
    .paletteTag = TAG_HELD_ITEM,
    .oam = &sOam_8x8,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Star =
{
    .tileTag = TAG_DEXNAV_STAR,
    .paletteTag = TAG_HELD_ITEM,
    .oam = &sOam_8x8,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Owned =
{
    .tileTag = TAG_DEXNAV_OWNED,
    .paletteTag = TAG_HELD_ITEM,
    .oam = &sOam_8x8,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

//////////////////////
//// SHARED UTILS ////
//////////////////////

bool8 CanUseDexNav(void)
{
    return FlagGet(FLAG_SYS_POKEDEX_GET)
        && !GetSafariZoneFlag()
        && CurrentBattlePyramidLocation() == PYRAMID_LOCATION_NONE
        && !InBattlePike();
}

static u8 GetSearchLevel(u16 species)
{
    return gPokemonStoragePtr->dexNav.searchLevels[species];
}

static u8 GetDexNavChain(void)
{
    return gPokemonStoragePtr->dexNav.chain;
}

static void ResetDexNavChain(void)
{
    gPokemonStoragePtr->dexNav.chain = 0;
}

static bool8 IsSpeciesSeen(u16 species)
{
    return species != SPECIES_NONE && GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN);
}

static bool8 IsSpeciesCaught(u16 species)
{
    return species != SPECIES_NONE && GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT);
}

static const struct WildPokemonInfo *GetEncounterInfo(u8 environment)
{
    u16 headerId = GetCurrentMapWildMonHeaderId();
    const struct WildPokemonInfo *info;

    if (headerId == HEADER_NONE)
        return NULL;

    if (environment == ENVIRONMENT_WATER)
        info = gWildMonHeaders[headerId].waterMonsInfo;
    else
        info = gWildMonHeaders[headerId].landMonsInfo;

    if (info == NULL || info->encounterRate == 0)
        return NULL;
    return info;
}

static u8 GetEncounterSlotCount(u8 environment)
{
    return (environment == ENVIRONMENT_WATER) ? NUM_WATER_MONS_ENCOUNTER_SLOTS : NUM_LAND_MONS_ENCOUNTER_SLOTS;
}

// A random level from the map's slots for this species, 0 if it doesn't live here.
// The chain adds 1 level per 10, and 4% of searches add 10 more.
static u8 GenerateMonLevel(u16 species, u8 environment)
{
    const struct WildPokemonInfo *info = GetEncounterInfo(environment);
    u8 min = MAX_LEVEL, max = 0;
    u32 i, level;

    if (info == NULL)
        return 0;

    for (i = 0; i < GetEncounterSlotCount(environment); i++)
    {
        if (info->wildPokemon[i].species == species)
        {
            if (info->wildPokemon[i].minLevel < min)
                min = info->wildPokemon[i].minLevel;
            if (info->wildPokemon[i].maxLevel > max)
                max = info->wildPokemon[i].maxLevel;
        }
    }

    if (max == 0)
        return 0;

    level = min + Random() % (max - min + 1);
    level += GetDexNavChain() / 10;
    if (Random() % 100 < 4)
        level += 10;
    return min(level, MAX_LEVEL);
}

// Common item 50-75% and rare item 5-15% as the search level goes 0-100
// (wild Pokémon normally have 50% and 5%); 100% if both are the same item.
static u16 GenerateHeldItem(u16 species, u8 searchLevel)
{
    u16 common = gSpeciesInfo[species].itemCommon;
    u16 rare = gSpeciesInfo[species].itemRare;
    u32 bonus = min(searchLevel, 100);
    u32 rareChance = 5 + bonus / 10;
    u32 commonChance = 50 + bonus / 4;
    u32 rnd = Random() % 100;

    if (common == rare)
        return common;
    if (rare != ITEM_NONE && rnd < rareChance)
        return rare;
    if (rare != ITEM_NONE)
        rnd -= min(rnd, rareChance);
    if (common != ITEM_NONE && rnd < commonChance)
        return common;
    return ITEM_NONE;
}

static u8 GeneratePotential(u8 searchLevel)
{
    u32 bracket, i, total = 0;
    u32 rnd = Random() % 100;

    for (bracket = 0; bracket < ARRAY_COUNT(sSearchLevelBrackets); bracket++)
    {
        if (searchLevel < sSearchLevelBrackets[bracket])
            break;
    }

    for (i = 0; i < MAX_POTENTIAL; i++)
    {
        total += sPotentialChances[bracket][i];
        if (rnd < total)
            return i + 1;
    }
    return 0;
}

// Extra shiny rolls for a DexNav Pokémon, on top of GetShinyRolls' usual ones:
// 1 per 10 chain, 1 per 25 search levels, and 4 more in 4% of encounters
u32 GetDexNavShinyRolls(void)
{
    u32 rolls;

    if (gDexNavSpecies == SPECIES_NONE)
        return 0;

    rolls = GetDexNavChain() / 10 + GetSearchLevel(gDexNavSpecies) / 25;
    if (Random() % 100 < 4)
        rolls += 4;
    return rolls;
}

static void CreateDexNavWildMon(void)
{
    struct Pokemon *mon = &gEnemyParty[0];
    u8 stats[NUM_STATS];
    u32 i, j, temp;
    u8 perfectIv = MAX_PER_STAT_IVS;

    gDexNavSpecies = sDexNavSearch->species;
    CreateWildMon(sDexNavSearch->species, sDexNavSearch->monLevel);

    // Potential: that many different stats get a perfect IV
    for (i = 0; i < NUM_STATS; i++)
        stats[i] = i;
    for (i = 0; i < sDexNavSearch->potential; i++)
    {
        j = i + Random() % (NUM_STATS - i);
        SWAP(stats[i], stats[j], temp);
        SetMonData(mon, MON_DATA_HP_IV + stats[i], &perfectIv);
    }

    SetMonData(mon, MON_DATA_HELD_ITEM, &sDexNavSearch->heldItem);
    CalculateMonStats(mon);
}

/////////////////////
//// SEARCH MODE ////
/////////////////////

bool8 IsDexNavSearching(void)
{
    return sDexNavSearch != NULL;
}

static u8 GetPlayerDistance(s16 x, s16 y)
{
    s16 playerX, playerY;

    PlayerGetDestCoords(&playerX, &playerY);
    return abs(x - playerX) + abs(y - playerY);
}

static bool8 IsValidHidingSpot(s16 x, s16 y, u8 environment, u8 playerElevation)
{
    u8 behavior = MapGridGetMetatileBehaviorAt(x, y);
    u8 elevation = MapGridGetElevationAt(x, y);

    if (MapGridGetCollisionAt(x, y))
        return FALSE;
    if (elevation != 0 && elevation != 15 && playerElevation != 0 && elevation != playerElevation)
        return FALSE;
    if (GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;

    if (environment == ENVIRONMENT_WATER)
        return MetatileBehavior_IsWaterWildEncounter(behavior);
    else
        return MetatileBehavior_IsLandWildEncounter(behavior);
}

// Picks a random valid tile on screen, at least MIN_SPAWN_DISTANCE from the player
static bool8 PickHidingSpot(u8 environment)
{
    s16 playerX, playerY, x, y;
    u8 playerElevation = gObjectEvents[gPlayerAvatar.objectEventId].currentElevation;
    u32 count = 0, pick;

    PlayerGetDestCoords(&playerX, &playerY);

    for (y = playerY - SCAN_RADIUS_Y; y <= playerY + SCAN_RADIUS_Y; y++)
    {
        for (x = playerX - SCAN_RADIUS_X; x <= playerX + SCAN_RADIUS_X; x++)
        {
            if (GetPlayerDistance(x, y) >= MIN_SPAWN_DISTANCE && IsValidHidingSpot(x, y, environment, playerElevation))
                count++;
        }
    }

    if (count == 0)
        return FALSE;

    pick = Random() % count;
    for (y = playerY - SCAN_RADIUS_Y; y <= playerY + SCAN_RADIUS_Y; y++)
    {
        for (x = playerX - SCAN_RADIUS_X; x <= playerX + SCAN_RADIUS_X; x++)
        {
            if (GetPlayerDistance(x, y) >= MIN_SPAWN_DISTANCE && IsValidHidingSpot(x, y, environment, playerElevation))
            {
                if (pick-- == 0)
                {
                    sDexNavSearch->tileX = x;
                    sDexNavSearch->tileY = y;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

static u8 GetHidingFieldEffect(void)
{
    u8 behavior = MapGridGetMetatileBehaviorAt(sDexNavSearch->tileX, sDexNavSearch->tileY);

    if (sDexNavSearch->environment == ENVIRONMENT_WATER)
        return FLDEFF_WATER_SURFACING;
    if (GetCurrentMapType() == MAP_TYPE_UNDERGROUND)
        return FLDEFF_CAVE_DUST;
    if (MetatileBehavior_IsTallGrass(behavior))
        return FLDEFF_UNUSED_GRASS; // shaking grass
    if (MetatileBehavior_IsLongGrass(behavior))
        return FLDEFF_UNUSED_GRASS_2; // shaking long grass
    if (MetatileBehavior_IsSandOrDeepSand(behavior))
        return FLDEFF_UNUSED_SAND; // swirling sand
    return FLDEFF_CAVE_DUST;
}

static bool8 StartHidingFieldEffect(void)
{
    sDexNavSearch->fldEffId = GetHidingFieldEffect();
    gFieldEffectArguments[0] = sDexNavSearch->tileX;
    gFieldEffectArguments[1] = sDexNavSearch->tileY;
    gFieldEffectArguments[2] = 0xFF; // subpriority
    gFieldEffectArguments[3] = 2;    // priority
    sDexNavSearch->fldEffSpriteId = FieldEffectStart(sDexNavSearch->fldEffId);
    if (sDexNavSearch->fldEffSpriteId >= MAX_SPRITES)
    {
        FieldEffectActiveListRemove(sDexNavSearch->fldEffId);
        return FALSE;
    }
    return TRUE;
}

static s16 GetSearchWindowY(void)
{
    return GetWindowAttribute(sDexNavSearch->windowId, WINDOW_TILEMAP_TOP) * 8;
}

static void UpdateSearchWindow(void)
{
    u8 windowId = sDexNavSearch->windowId;
    bool8 close = (sDexNavSearch->proximity <= SNEAKING_PROXIMITY);
    s16 playerX, playerY, dx, dy;
    const u8 *arrow;
    u32 i;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));

    AddTextPrinterParameterized3(windowId, FONT_SMALL, SEARCH_TEXT_X, 0, sSearchFontColor, TEXT_SKIP_DRAW, gSpeciesNames[sDexNavSearch->species]);
    ConvertIntToDecimalStringN(gStringVar1, sDexNavSearch->monLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, sText_MonLevel);
    AddTextPrinterParameterized3(windowId, FONT_SMALL, SEARCH_LEVEL_TEXT_X, 0, sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);

    // Held item, shown up close
    if (close && sDexNavSearch->heldItem != ITEM_NONE)
    {
        CopyItemName(sDexNavSearch->heldItem, gStringVar1);
        AddTextPrinterParameterized3(windowId, FONT_SMALL, SEARCH_TEXT_X, 12, sSearchFontColor, TEXT_SKIP_DRAW, gStringVar1);
    }

    ConvertIntToDecimalStringN(gStringVar1, GetDexNavChain(), STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, sText_Chain);
    AddTextPrinterParameterized3(windowId, FONT_SMALL, SEARCH_CHAIN_TEXT_X, 12, sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);

    // Arrow pointing to the hiding spot
    PlayerGetDestCoords(&playerX, &playerY);
    dx = sDexNavSearch->tileX - playerX;
    dy = sDexNavSearch->tileY - playerY;
    if (abs(dx) <= 1 && abs(dy) <= 1)
        arrow = gText_EmptyString2;
    else if (abs(dx) > abs(dy))
        arrow = (dx < 0) ? sText_ArrowLeft : sText_ArrowRight;
    else
        arrow = (dy < 0) ? sText_ArrowUp : sText_ArrowDown;
    AddTextPrinterParameterized3(windowId, FONT_NORMAL, SEARCH_ARROW_X, 2, sSearchFontColor, TEXT_SKIP_DRAW, arrow);

    CopyWindowToVram(windowId, COPYWIN_GFX);

    if (sDexNavSearch->itemSpriteId != MAX_SPRITES)
        gSprites[sDexNavSearch->itemSpriteId].invisible = !close;
    for (i = 0; i < MAX_POTENTIAL; i++)
    {
        if (sDexNavSearch->starSpriteIds[i] != MAX_SPRITES)
            gSprites[sDexNavSearch->starSpriteIds[i]].invisible = !close;
    }
}

static void CreateSearchWindow(void)
{
    struct WindowTemplate template;
    s16 playerX, playerY;
    u8 top = 16;
    u32 i;
    s16 y;

    // Draw at the top if the Pokémon is in the lower half of the screen
    PlayerGetDestCoords(&playerX, &playerY);
    if (sDexNavSearch->tileY > playerY)
        top = 1;

    LoadBgTiles(0, gTextWindowFrame1_Gfx, 0x120, SEARCH_FRAME_BASE_TILE);
    LoadPalette(sSearchWindowPal, BG_PLTT_ID(SEARCH_WINDOW_PALETTE), PLTT_SIZE_4BPP);
    SetWindowTemplateFields(&template, 0, 1, top, SEARCH_WINDOW_WIDTH, SEARCH_WINDOW_HEIGHT, SEARCH_WINDOW_PALETTE, SEARCH_WINDOW_BASE_TILE);
    sDexNavSearch->windowId = AddWindow(&template);
    PutWindowTilemap(sDexNavSearch->windowId);
    DrawStdFrameWithCustomTileAndPalette(sDexNavSearch->windowId, FALSE, SEARCH_FRAME_BASE_TILE, SEARCH_WINDOW_PALETTE);

    // Sprites: species icon, Poké Ball if caught, held item and potential stars
    y = GetSearchWindowY();
    LoadSpriteSheet(&gSpriteSheet_HeldItem);
    LoadSpritePalette(&gSpritePalette_HeldItem);
    LoadCompressedSpriteSheetUsingHeap(&sSpriteSheet_Star);
    LoadCompressedSpriteSheetUsingHeap(&sSpriteSheet_Owned);
    LoadMonIconPalette(sDexNavSearch->species);

    sDexNavSearch->iconSpriteId = CreateMonIcon(sDexNavSearch->species, SpriteCB_MonIcon, SPECIES_ICON_X - 6, y + 8, 0, 0, FALSE);
    gSprites[sDexNavSearch->iconSpriteId].oam.priority = 0;

    sDexNavSearch->ownedSpriteId = MAX_SPRITES;
    if (IsSpeciesCaught(sDexNavSearch->species))
        sDexNavSearch->ownedSpriteId = CreateSprite(&sSpriteTemplate_Owned, SPECIES_ICON_X + 6, y + 4, 0);

    sDexNavSearch->itemSpriteId = MAX_SPRITES;
    if (sDexNavSearch->heldItem != ITEM_NONE)
        sDexNavSearch->itemSpriteId = CreateSprite(&sSpriteTemplate_HeldItem, SPECIES_ICON_X + 6, y + 18, 0);

    for (i = 0; i < MAX_POTENTIAL; i++)
    {
        sDexNavSearch->starSpriteIds[i] = MAX_SPRITES;
        if (i < sDexNavSearch->potential)
            sDexNavSearch->starSpriteIds[i] = CreateSprite(&sSpriteTemplate_Star, SPECIES_ICON_X - 18, y + 4 + i * 8, 0);
    }

    UpdateSearchWindow();
}

static void DestroySearchSprite(u8 spriteId)
{
    if (spriteId < MAX_SPRITES)
        DestroySprite(&gSprites[spriteId]);
}

void EndDexNavSearch(void)
{
    u32 i;

    if (sDexNavSearch == NULL)
        return;

    FieldEffectStop(&gSprites[sDexNavSearch->fldEffSpriteId], sDexNavSearch->fldEffId);

    FreeAndDestroyMonIconSprite(&gSprites[sDexNavSearch->iconSpriteId]);
    FreeMonIconPalette(sDexNavSearch->species);
    DestroySearchSprite(sDexNavSearch->ownedSpriteId);
    DestroySearchSprite(sDexNavSearch->itemSpriteId);
    for (i = 0; i < MAX_POTENTIAL; i++)
        DestroySearchSprite(sDexNavSearch->starSpriteIds[i]);
    FreeSpriteTilesByTag(TAG_HELD_ITEM);
    FreeSpriteTilesByTag(TAG_DEXNAV_STAR);
    FreeSpriteTilesByTag(TAG_DEXNAV_OWNED);
    FreeSpritePaletteByTag(TAG_HELD_ITEM);

    ClearStdWindowAndFrameToTransparent(sDexNavSearch->windowId, FALSE);
    CopyWindowToVram(sDexNavSearch->windowId, COPYWIN_FULL);
    RemoveWindow(sDexNavSearch->windowId);

    FREE_AND_SET_NULL(sDexNavSearch);
}

static void FailDexNavSearch(const u8 *script)
{
    ResetDexNavChain();
    EndDexNavSearch();
    ScriptContext_SetupScript(script);
}

// Returns TRUE if a script was started (the search failed to start)
static bool8 StartDexNavSearch(u16 species, u8 environment)
{
    u8 level;

    if (GetFlashLevel() > 0)
    {
        ScriptContext_SetupScript(EventScript_DexNavTooDark);
        return TRUE;
    }

    level = GenerateMonLevel(species, environment);
    if (level == 0 || (sDexNavSearch = AllocZeroed(sizeof(*sDexNavSearch))) == NULL)
    {
        ScriptContext_SetupScript(EventScript_DexNavNotFoundNearby);
        return TRUE;
    }

    sDexNavSearch->species = species;
    sDexNavSearch->environment = environment;
    sDexNavSearch->monLevel = level;
    if (!PickHidingSpot(environment) || !StartHidingFieldEffect())
    {
        FREE_AND_SET_NULL(sDexNavSearch);
        ScriptContext_SetupScript(EventScript_DexNavNotFoundNearby);
        return TRUE;
    }

    sDexNavSearch->searchLevel = GetSearchLevel(species);
    sDexNavSearch->heldItem = GenerateHeldItem(species, sDexNavSearch->searchLevel);
    sDexNavSearch->potential = GeneratePotential(sDexNavSearch->searchLevel);
    sDexNavSearch->proximity = GetPlayerDistance(sDexNavSearch->tileX, sDexNavSearch->tileY);
    sDexNavSearch->startTime = gMain.vblankCounter1;

    HideMapNamePopUpWindow();
    ChangeBgY_ScreenOff(0, 0, BG_COORD_SET);
    CreateSearchWindow();
    return FALSE;
}

static bool8 IsPlayerMovingTooFast(void)
{
    return (gPlayerAvatar.flags & (PLAYER_AVATAR_FLAG_DASH | PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE)) != 0;
}

// Called after every step. Returns TRUE if a script was started.
bool8 UpdateDexNavSearch(void)
{
    u8 prevProximity;

    if (sDexNavSearch == NULL)
        return FALSE;

    prevProximity = sDexNavSearch->proximity;
    sDexNavSearch->proximity = GetPlayerDistance(sDexNavSearch->tileX, sDexNavSearch->tileY);

    if (sDexNavSearch->proximity > MAX_PROXIMITY)
    {
        FailDexNavSearch(EventScript_DexNavLostSignal);
        return TRUE;
    }
    if (sDexNavSearch->proximity <= SNEAKING_PROXIMITY && IsPlayerMovingTooFast())
    {
        FailDexNavSearch(EventScript_DexNavMovedTooFast);
        return TRUE;
    }
    if (gMain.vblankCounter1 - sDexNavSearch->startTime > DEXNAV_TIMEOUT_FRAMES)
    {
        FailDexNavSearch(EventScript_DexNavGotAway);
        return TRUE;
    }
    if (sDexNavSearch->proximity == 0)
    {
        CreateDexNavWildMon();
        EndDexNavSearch();
        ScriptContext_SetupScript(EventScript_StartDexNavBattle);
        return TRUE;
    }

    if (sDexNavSearch->proximity <= SNEAKING_PROXIMITY && prevProximity > SNEAKING_PROXIMITY)
        PlaySE(SE_POKENAV_ON);
    UpdateSearchWindow();
    return FALSE;
}

// R in the overworld: search for the registered species, or stop searching.
// Returns TRUE if a script was started.
bool8 TryUseDexNavShortcut(void)
{
    u16 registered = VarGet(VAR_DEXNAV_SPECIES);
    u16 species = registered & DEXNAV_VAR_SPECIES_MASK;

    if (sDexNavSearch != NULL)
    {
        PlaySE(SE_POKENAV_OFF);
        EndDexNavSearch();
        return FALSE;
    }

    if (!CanUseDexNav() || species == SPECIES_NONE || species >= NUM_SPECIES)
        return FALSE;

    PlaySE(SE_DEX_SEARCH);
    return StartDexNavSearch(species, registered >> DEXNAV_VAR_ENV_SHIFT);
}

// Leaving the map ends the search and the chain
void DexNavOnMapChange(void)
{
    EndDexNavSearch();
    ResetDexNavChain();
}

// Defeating or catching a DexNav Pokémon raises its search level and the chain;
// anything else breaks the chain.
void DexNavOnBattleEnd(void)
{
    struct DexNavData *data = &gPokemonStoragePtr->dexNav;

    if (gDexNavSpecies == SPECIES_NONE)
        return;

    if (gBattleOutcome == B_OUTCOME_WON || gBattleOutcome == B_OUTCOME_CAUGHT)
    {
        if (data->searchLevels[gDexNavSpecies] < SEARCH_LEVEL_MAX)
            data->searchLevels[gDexNavSpecies]++;
        if (data->chain < DEXNAV_CHAIN_MAX)
            data->chain++;
    }
    else
    {
        data->chain = 0;
    }
    gDexNavSpecies = SPECIES_NONE;
}

static void Task_StartDexNavSearchAfterFade(u8 taskId)
{
    if (IsWeatherNotFadingIn())
    {
        DestroyTask(taskId);
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        StartDexNavSearch(sPendingSearchSpecies, sPendingSearchEnvironment);
    }
}

static void FieldCB_StartDexNavSearch(void)
{
    LockPlayerFieldControls();
    FadeInFromBlack();
    CreateTask(Task_StartDexNavSearchAfterFade, 8);
}

/////////////
//// GUI ////
/////////////

static void VBlankCB_DexNav(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_DexNav(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void FreeDexNavGui(void)
{
    FREE_AND_SET_NULL(sDexNavGui);
    FREE_AND_SET_NULL(sBg1TilemapBuffer);
    FreeAllWindowBuffers();
}

void CB2_OpenDexNav(void)
{
    sDexNavGui = AllocZeroed(sizeof(*sDexNavGui));
    sBg1TilemapBuffer = AllocZeroed(BG_SCREEN_SIZE);
    if (sDexNavGui == NULL || sBg1TilemapBuffer == NULL)
    {
        FREE_AND_SET_NULL(sDexNavGui);
        FREE_AND_SET_NULL(sBg1TilemapBuffer);
        SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
        return;
    }
    gMain.state = 0;
    SetMainCallback2(CB2_DexNavSetup);
}

static bool8 IsSpeciesInList(const u16 *list, u32 count, u16 species)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        if (list[i] == species)
            return TRUE;
    }
    return FALSE;
}

// Each species once, in encounter table order
static void LoadEncounterSpecies(u16 *dest, u8 environment)
{
    const struct WildPokemonInfo *info = GetEncounterInfo(environment);
    u32 count = GetEncounterSlotCount(environment);
    u32 i, n = 0;

    if (info == NULL)
        return;

    for (i = 0; i < count; i++)
    {
        u16 species = info->wildPokemon[i].species;
        if (species != SPECIES_NONE && !IsSpeciesInList(dest, n, species))
            dest[n++] = species;
    }
}

static bool8 HasCaughtAll(const u16 *list, u32 count)
{
    u32 i;

    if (list[0] == SPECIES_NONE)
        return FALSE;
    for (i = 0; i < count && list[i] != SPECIES_NONE; i++)
    {
        if (!IsSpeciesCaught(list[i]))
            return FALSE;
    }
    return TRUE;
}

static void DrawSpeciesIcon(u16 species, s16 x, s16 y)
{
    if (species == SPECIES_NONE)
        CreateSprite(&sSpriteTemplate_NoData, x, y, 0);
    else if (!IsSpeciesSeen(species))
        CreateMonIcon(SPECIES_NONE, SpriteCB_MonIcon, x, y, 0, 0, FALSE); // question mark
    else
        CreateMonIcon(species, SpriteCB_MonIcon, x, y, 0, 0, FALSE);
}

static void DrawSpeciesIcons(void)
{
    u32 i;

    LoadMonIconPalettes();
    LoadCompressedSpriteSheetUsingHeap(&sSpriteSheet_NoData);
    for (i = 0; i < NUM_WATER_MONS_ENCOUNTER_SLOTS; i++)
        DrawSpeciesIcon(sDexNavGui->waterSpecies[i], ROW_WATER_ICON_X + ICON_SPACING * i, ROW_WATER_ICON_Y);
    for (i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
    {
        DrawSpeciesIcon(sDexNavGui->landSpecies[i],
                        ROW_LAND_ICON_X + ICON_SPACING * (i % COL_LAND_COUNT),
                        (i < COL_LAND_COUNT) ? ROW_LAND_TOP_ICON_Y : ROW_LAND_BOT_ICON_Y);
    }

    LoadCompressedSpriteSheetUsingHeap(&sSpriteSheet_CapturedAll);
    if (HasCaughtAll(sDexNavGui->waterSpecies, NUM_WATER_MONS_ENCOUNTER_SLOTS))
        CreateSprite(&sSpriteTemplate_CapturedAll, 139, 17, 0);
    if (HasCaughtAll(sDexNavGui->landSpecies, NUM_LAND_MONS_ENCOUNTER_SLOTS))
        CreateSprite(&sSpriteTemplate_CapturedAll, 152, 58, 0);
}

static u8 GetSelectedEnvironment(void)
{
    return (sDexNavGui->cursorRow == ROW_WATER) ? ENVIRONMENT_WATER : ENVIRONMENT_LAND;
}

// The species under the cursor, or SPECIES_NONE if empty or not seen yet
static u16 GetSelectedSpecies(void)
{
    u16 species;

    switch (sDexNavGui->cursorRow)
    {
    case ROW_WATER:
        species = sDexNavGui->waterSpecies[sDexNavGui->cursorCol];
        break;
    case ROW_LAND_TOP:
        species = sDexNavGui->landSpecies[sDexNavGui->cursorCol];
        break;
    default:
        species = sDexNavGui->landSpecies[sDexNavGui->cursorCol + COL_LAND_COUNT];
        break;
    }

    if (!IsSpeciesSeen(species))
        return SPECIES_NONE;
    return species;
}

static u8 GetRowColumnCount(u8 row)
{
    return (row == ROW_WATER) ? COL_WATER_COUNT : COL_LAND_COUNT;
}

static void UpdateCursorPosition(void)
{
    struct Sprite *cursor = &gSprites[sDexNavGui->cursorSpriteId];

    switch (sDexNavGui->cursorRow)
    {
    case ROW_WATER:
        cursor->x = ROW_WATER_ICON_X + ICON_SPACING * sDexNavGui->cursorCol;
        cursor->y = ROW_WATER_ICON_Y;
        break;
    case ROW_LAND_TOP:
        cursor->x = ROW_LAND_ICON_X + ICON_SPACING * sDexNavGui->cursorCol;
        cursor->y = ROW_LAND_TOP_ICON_Y;
        break;
    default:
        cursor->x = ROW_LAND_ICON_X + ICON_SPACING * sDexNavGui->cursorCol;
        cursor->y = ROW_LAND_BOT_ICON_Y;
        break;
    }
    PrintCurrentSpeciesInfo();
}

static void SetTypeIcon(u8 type, s16 x, u8 spriteArrayId)
{
    struct Sprite *sprite = &gSprites[sDexNavGui->typeIconSpriteIds[spriteArrayId]];

    StartSpriteAnim(sprite, type);
    sprite->oam.paletteNum = gMoveTypeToOamPaletteNum[type];
    sprite->x = x + 16;
    sprite->y = TYPE_ICONS_Y + 8;
    sprite->invisible = FALSE;
}

static void PrintInfoText(u8 y, const u8 *text)
{
    AddTextPrinterParameterized3(WIN_INFO, FONT_SMALL, 0, y, sFontColor_Black, TEXT_SKIP_DRAW, text);
}

static void PrintCurrentSpeciesInfo(void)
{
    u16 species = GetSelectedSpecies();
    u8 type1, type2;
    u16 item;

    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));

    if (species == SPECIES_NONE)
    {
        PrintInfoText(SPECIES_INFO_Y, sText_NoInfo);
        PrintInfoText(SEARCH_LEVEL_Y, sText_NoInfo);
        PrintInfoText(HELD_ITEM_INFO_Y, sText_NoInfo);
        type1 = type2 = TYPE_MYSTERY;
    }
    else
    {
        PrintInfoText(SPECIES_INFO_Y, gSpeciesNames[species]);

        ConvertIntToDecimalStringN(gStringVar4, GetSearchLevel(species), STR_CONV_MODE_LEFT_ALIGN, 3);
        PrintInfoText(SEARCH_LEVEL_Y, gStringVar4);

        // The item it may hold in the wild (the common one if there are two)
        if (!IsSpeciesCaught(species))
        {
            PrintInfoText(HELD_ITEM_INFO_Y, sText_CaptureFirst);
        }
        else
        {
            item = gSpeciesInfo[species].itemCommon;
            if (item == ITEM_NONE)
                item = gSpeciesInfo[species].itemRare;
            if (item == ITEM_NONE)
            {
                PrintInfoText(HELD_ITEM_INFO_Y, gText_None);
            }
            else
            {
                CopyItemName(item, gStringVar4);
                PrintInfoText(HELD_ITEM_INFO_Y, gStringVar4);
            }
        }

        type1 = gSpeciesInfo[species].types[0];
        type2 = gSpeciesInfo[species].types[1];
    }

    ConvertIntToDecimalStringN(gStringVar4, GetDexNavChain(), STR_CONV_MODE_LEFT_ALIGN, 3);
    PrintInfoText(CHAIN_INFO_Y, gStringVar4);

    if (type1 == type2)
    {
        SetTypeIcon(type1, 186, 0);
        gSprites[sDexNavGui->typeIconSpriteIds[1]].invisible = TRUE;
    }
    else
    {
        SetTypeIcon(type1, 168, 0);
        SetTypeIcon(type2, 168 + 33, 1);
    }

    PutWindowTilemap(WIN_INFO);
    CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
}

static void PrintRegisteredSpecies(void)
{
    u16 species = VarGet(VAR_DEXNAV_SPECIES) & DEXNAV_VAR_SPECIES_MASK;
    u32 x;

    FillWindowPixelBuffer(WIN_REGISTERED, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
    {
        AddTextPrinterParameterized3(WIN_REGISTERED, FONT_NORMAL, 0, 0, sFontColor_White, TEXT_SKIP_DRAW, sText_PressRToRegister);
    }
    else
    {
        StringCopy(gStringVar1, gSpeciesNames[species]);
        StringExpandPlaceholders(gStringVar4, sText_SearchFor);
        AddTextPrinterParameterized3(WIN_REGISTERED, FONT_NORMAL, 0, 0, sFontColor_White, TEXT_SKIP_DRAW, gStringVar4);
    }

    GetMapName(gStringVar3, GetCurrentRegionMapSectionId(), 0);
    x = 26 * 8 - GetStringWidth(FONT_NORMAL, gStringVar3, 0) - 2;
    AddTextPrinterParameterized3(WIN_REGISTERED, FONT_NORMAL, x, 0, sFontColor_White, TEXT_SKIP_DRAW, gStringVar3);

    PutWindowTilemap(WIN_REGISTERED);
    CopyWindowToVram(WIN_REGISTERED, COPYWIN_FULL);
}

static void CreateGuiSprites(void)
{
    u32 i;

    LoadCompressedSpriteSheet(&gSpriteSheet_MoveTypes);
    LoadCompressedPalette(gMoveTypes_Pal, OBJ_PLTT_ID(13), 3 * PLTT_SIZE_4BPP);
    for (i = 0; i < ARRAY_COUNT(sDexNavGui->typeIconSpriteIds); i++)
    {
        sDexNavGui->typeIconSpriteIds[i] = CreateSprite(&gSpriteTemplate_MoveTypes, 10, 10, 2);
        gSprites[sDexNavGui->typeIconSpriteIds[i]].invisible = TRUE;
    }

    DrawSpeciesIcons();

    LoadCompressedSpriteSheet(&sSpriteSheet_Cursor);
    LoadSpritePalette(&sSpritePalette_Cursor);
    sDexNavGui->cursorSpriteId = CreateSprite(&sSpriteTemplate_Cursor, 0, 0, 0);
}

static void CB2_DexNavSetup(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        ClearScheduledBgCopiesToVram();
        ScanlineEffect_Stop();
        FreeAllSpritePalettes();
        ResetPaletteFade();
        ResetSpriteData();
        ResetTasks();
        gMain.state++;
        break;
    case 1:
        ResetVramOamAndBgCntRegs();
        ResetAllBgsCoordinates();
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sDexNavBgTemplates, ARRAY_COUNT(sDexNavBgTemplates));
        SetBgTilemapBuffer(1, sBg1TilemapBuffer);
        InitWindows(sDexNavWindowTemplates);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        gMain.state++;
        break;
    case 2:
        ResetTempTileDataBuffers();
        DecompressAndCopyTileDataToVram(1, sDexNavGuiTiles, 0, 0, 0);
        gMain.state++;
        break;
    case 3:
        if (!FreeTempTileDataBuffersIfPossible())
        {
            LZDecompressWram(sDexNavGuiTilemap, sBg1TilemapBuffer);
            CopyBgTilemapBufferToVram(1);
            gMain.state++;
        }
        break;
    case 4:
        LoadPalette(sDexNavGuiPal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
        LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        LoadEncounterSpecies(sDexNavGui->waterSpecies, ENVIRONMENT_WATER);
        LoadEncounterSpecies(sDexNavGui->landSpecies, ENVIRONMENT_LAND);
        // Start on the first land species, or on water if there's no land data
        sDexNavGui->cursorRow = (sDexNavGui->landSpecies[0] == SPECIES_NONE && sDexNavGui->waterSpecies[0] != SPECIES_NONE)
                              ? ROW_WATER : ROW_LAND_TOP;
        sDexNavGui->cursorCol = 0;
        gMain.state++;
        break;
    case 5:
        CreateGuiSprites();
        PrintRegisteredSpecies();
        UpdateCursorPosition();
        gMain.state++;
        break;
    case 6:
        ShowBg(0);
        ShowBg(1);
        BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_DexNav);
        CreateTask(Task_DexNavMain, 0);
        SetMainCallback2(CB2_DexNav);
        break;
    }
}

static void Task_DexNavExit(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeDexNavGui();
        DestroyTask(taskId);
        SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
    }
}

static void Task_DexNavExitAndSearch(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeDexNavGui();
        DestroyTask(taskId);
        gFieldCallback = FieldCB_StartDexNavSearch;
        SetMainCallback2(CB2_ReturnToField);
    }
}

static void MoveCursor(s8 rowDelta, s8 colDelta)
{
    u8 columns;

    if (rowDelta != 0)
    {
        sDexNavGui->cursorRow = (sDexNavGui->cursorRow + ROWS_COUNT + rowDelta) % ROWS_COUNT;
        columns = GetRowColumnCount(sDexNavGui->cursorRow);
        if (sDexNavGui->cursorCol >= columns)
            sDexNavGui->cursorCol = columns - 1;
    }
    if (colDelta != 0)
    {
        columns = GetRowColumnCount(sDexNavGui->cursorRow);
        sDexNavGui->cursorCol = (sDexNavGui->cursorCol + columns + colDelta) % columns;
    }
    PlaySE(SE_RG_BAG_CURSOR);
    UpdateCursorPosition();
}

static void Task_DexNavMain(u8 taskId)
{
    u16 species;

    if (gPaletteFade.active)
        return;

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_POKENAV_OFF);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_DexNavExit;
    }
    else if (JOY_NEW(DPAD_UP))
    {
        MoveCursor(-1, 0);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        MoveCursor(1, 0);
    }
    else if (JOY_NEW(DPAD_LEFT))
    {
        MoveCursor(0, -1);
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        MoveCursor(0, 1);
    }
    else if (JOY_NEW(R_BUTTON))
    {
        species = GetSelectedSpecies();
        if (species == SPECIES_NONE)
        {
            PlaySE(SE_FAILURE);
        }
        else
        {
            VarSet(VAR_DEXNAV_SPECIES, species | (GetSelectedEnvironment() << DEXNAV_VAR_ENV_SHIFT));
            PrintRegisteredSpecies();
            PlayCry_Normal(species, 0);
        }
    }
    else if (JOY_NEW(A_BUTTON))
    {
        species = GetSelectedSpecies();
        if (species == SPECIES_NONE)
        {
            PlaySE(SE_FAILURE);
        }
        else
        {
            sPendingSearchSpecies = species;
            sPendingSearchEnvironment = GetSelectedEnvironment();
            PlaySE(SE_DEX_SEARCH);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_DexNavExitAndSearch;
        }
    }
}
