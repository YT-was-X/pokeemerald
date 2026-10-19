#include "global.h"
#include "new_game.h"
#include "random.h"
#include "pokemon.h"
#include "roamer.h"
#include "pokemon_size_record.h"
#include "script.h"
#include "lottery_corner.h"
#include "play_time.h"
#include "mauville_old_man.h"
#include "match_call.h"
#include "lilycove_lady.h"
#include "load_save.h"
#include "pokeblock.h"
#include "dewford_trend.h"
#include "berry.h"
#include "rtc.h"
#include "easy_chat.h"
#include "event_data.h"
#include "money.h"
#include "trainer_hill.h"
#include "tv.h"
#include "coins.h"
#include "text.h"
#include "overworld.h"
#include "mail.h"
#include "battle_records.h"
#include "item.h"
#include "pokedex.h"
#include "apprentice.h"
#include "frontier_util.h"
#include "pokedex.h"
#include "save.h"
#include "link_rfu.h"
#include "main.h"
#include "contest.h"
#include "item_menu.h"
#include "pokemon_storage_system.h"
#include "pokemon_jump.h"
#include "decoration_inventory.h"
#include "secret_base.h"
#include "player_pc.h"
#include "field_specials.h"
#include "berry_powder.h"
#include "mystery_gift.h"
#include "union_room_chat.h"
#include "constants/items.h"
#include "constants/region_map_sections.h"
#include "constants/species.h"
#include "data.h"
#include "malloc.h"
#include "string_util.h"

extern const u8 EventScript_ResetAllMapFlags[];

static void ClearFrontierRecord(void);
static void WarpToTruck(void);
static void ResetMiniGamesRecords(void);

EWRAM_DATA bool8 gDifferentSaveFile = FALSE;
EWRAM_DATA bool8 gEnableContestDebugging = FALSE;

static const struct ContestWinner sContestWinnerPicDummy =
{
    .monName = _(""),
    .trainerName = _("")
};

void SetTrainerId(u32 trainerId, u8 *dst)
{
    dst[0] = trainerId;
    dst[1] = trainerId >> 8;
    dst[2] = trainerId >> 16;
    dst[3] = trainerId >> 24;
}

u32 GetTrainerId(u8 *trainerId)
{
    return (trainerId[3] << 24) | (trainerId[2] << 16) | (trainerId[1] << 8) | (trainerId[0]);
}

void CopyTrainerId(u8 *dst, u8 *src)
{
    s32 i;
    for (i = 0; i < TRAINER_ID_LENGTH; i++)
        dst[i] = src[i];
}

static void InitPlayerTrainerId(void)
{
    u32 trainerId = (Random() << 16) | GetGeneratedTrainerIdLower();
    SetTrainerId(trainerId, gSaveBlock2Ptr->playerTrainerId);
}

// L=A isnt set here for some reason.
static void SetDefaultOptions(void)
{
    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_MID;
    gSaveBlock2Ptr->optionsWindowFrameType = 0;
    gSaveBlock2Ptr->optionsSound = OPTIONS_SOUND_MONO;
    gSaveBlock2Ptr->optionsBattleStyle = OPTIONS_BATTLE_STYLE_SHIFT;
    gSaveBlock2Ptr->optionsBattleSceneOff = FALSE;
    gSaveBlock2Ptr->regionMapZoom = FALSE;
}

static void ClearPokedexFlags(void)
{
    gUnusedPokedexU8 = 0;
    memset(&gSaveBlock2Ptr->pokedex.owned, 0, sizeof(gSaveBlock2Ptr->pokedex.owned));
    memset(&gSaveBlock2Ptr->pokedex.seen, 0, sizeof(gSaveBlock2Ptr->pokedex.seen));
}

void ClearAllContestWinnerPics(void)
{
    s32 i;

    ClearContestWinnerPicsInContestHall();

    // Clear Museum paintings
    for (i = MUSEUM_CONTEST_WINNERS_START; i < NUM_CONTEST_WINNERS; i++)
        gSaveBlock1Ptr->contestWinners[i] = sContestWinnerPicDummy;
}

static void ClearFrontierRecord(void)
{
    CpuFill32(0, &gSaveBlock2Ptr->frontier, sizeof(gSaveBlock2Ptr->frontier));

    gSaveBlock2Ptr->frontier.opponentNames[0][0] = EOS;
    gSaveBlock2Ptr->frontier.opponentNames[1][0] = EOS;
}

static void WarpToTruck(void)
{
    SetWarpDestination(MAP_GROUP(MAP_INSIDE_OF_TRUCK), MAP_NUM(MAP_INSIDE_OF_TRUCK), WARP_ID_NONE, -1, -1);
    WarpIntoMap();
}

void Sav2_ClearSetDefault(void)
{
    ClearSav2();
    SetDefaultOptions();
}

void ResetMenuAndMonGlobals(void)
{
    gDifferentSaveFile = FALSE;
    ResetPokedexScrollPositions();
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    ResetBagScrollPositions();
    ResetPokeblockScrollPositions();
}

void NewGameInitData(void)
{
    if (gSaveFileStatus == SAVE_STATUS_EMPTY || gSaveFileStatus == SAVE_STATUS_CORRUPT)
        RtcReset();

    gDifferentSaveFile = TRUE;
    gSaveBlock2Ptr->encryptionKey = 0;
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    ResetPokedex();
    ClearFrontierRecord();
    ClearSav1();
    ClearAllMail();
    gSaveBlock2Ptr->specialSaveWarpFlags = 0;
    gSaveBlock2Ptr->gcnLinkFlags = 0;
    InitPlayerTrainerId();
    PlayTimeCounter_Reset();
    ClearPokedexFlags();
    InitEventData();
    ClearTVShowData();
    ResetGabbyAndTy();
    ClearSecretBases();
    ClearBerryTrees();
    SetMoney(&gSaveBlock1Ptr->money, 3000);
    SetCoins(0);
    ResetLinkContestBoolean();
    ResetGameStats();
    ClearAllContestWinnerPics();
    ClearPlayerLinkBattleRecords();
    InitSeedotSizeRecord();
    InitLotadSizeRecord();
    gPlayerPartyCount = 0;
    ZeroPlayerPartyMons();
    ResetPokemonStorageSystem();
    ClearRoamerData();
    ClearRoamerLocationData();
    gSaveBlock1Ptr->registeredItem = ITEM_NONE;
    ClearBag();
    NewGameInitPCItems();
    ClearPokeblocks();
    ClearDecorationInventories();
    InitEasyChatPhrases();
    SetMauvilleOldMan();
    InitDewfordTrend();
    ResetFanClub();
    ResetLotteryCorner();
    WarpToTruck();
    RunScriptImmediately(EventScript_ResetAllMapFlags);
    ResetMiniGamesRecords();
    InitUnionRoomChatRegisteredTexts();
    InitLilycoveLady();
    ResetAllApprenticeData();
    ClearRankingHallRecords();
    InitMatchCallCounters();
    ClearMysteryGift();
    WipeTrainerNameRecords();
    ResetTrainerHillResults();
    ResetContestLinkResults();
}

// ---------------------------------------------------------------------------
// New Game Plus: started from the bed in the player's room after the Elite Four.
// Everything is reset as for a new game except the player's name, gender,
// Trainer ID and options, plus:
// - every starter from previous runs, back to its first stage at level 5 with
//   fresh moves, waiting in the first PC box so the game starts as normal
// - all money and Master Balls
// - a head start of 1/NGP_SHINY_ROLL_DIVISOR of the current shiny rolls
// ---------------------------------------------------------------------------

// Each Hoenn starter species and the first stage of its line
static const u16 sStarterLines[][2] =
{
    {SPECIES_TREECKO,   SPECIES_TREECKO},
    {SPECIES_GROVYLE,   SPECIES_TREECKO},
    {SPECIES_SCEPTILE,  SPECIES_TREECKO},
    {SPECIES_TORCHIC,   SPECIES_TORCHIC},
    {SPECIES_COMBUSKEN, SPECIES_TORCHIC},
    {SPECIES_BLAZIKEN,  SPECIES_TORCHIC},
    {SPECIES_MUDKIP,    SPECIES_MUDKIP},
    {SPECIES_MARSHTOMP, SPECIES_MUDKIP},
    {SPECIES_SWAMPERT,  SPECIES_MUDKIP},
};

static u16 GetStarterBaseSpecies(u16 species)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sStarterLines); i++)
    {
        if (sStarterLines[i][0] == species)
            return sStarterLines[i][1];
    }
    return SPECIES_NONE;
}

static u32 GetPlayerOtId(void)
{
    return gSaveBlock2Ptr->playerTrainerId[0]
         | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
         | (gSaveBlock2Ptr->playerTrainerId[2] << 16)
         | (gSaveBlock2Ptr->playerTrainerId[3] << 24);
}

// The starter Prof. Birch gave the player: one of the Hoenn starter lines,
// caught by the player on Route 101 at level 5
static bool8 IsOriginalStarter(struct BoxPokemon *boxMon)
{
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);

    return species != SPECIES_NONE
        && !GetBoxMonData(boxMon, MON_DATA_IS_EGG)
        && GetStarterBaseSpecies(species) != SPECIES_NONE
        && GetBoxMonData(boxMon, MON_DATA_OT_ID) == GetPlayerOtId()
        && GetBoxMonData(boxMon, MON_DATA_MET_LOCATION) == MAPSEC_ROUTE_101
        && GetBoxMonData(boxMon, MON_DATA_MET_LEVEL) == 5;
}

#define NGP_MAX_STARTERS IN_BOX_COUNT

// Rebuild the starter as a level 5 first stage with its level 5 moves,
// keeping its personality (nature, gender, shininess), IVs, ability,
// nickname and original trainer
static void MakeLevel5Starter(struct Pokemon *dest, struct BoxPokemon *src)
{
    u8 nickname[POKEMON_NAME_LENGTH + 1];
    u8 otName[PLAYER_NAME_LENGTH + 1];
    u16 species = GetBoxMonData(src, MON_DATA_SPECIES);
    u32 data;
    u32 i;

    CreateMon(dest, GetStarterBaseSpecies(species), 5, 0, TRUE,
              GetBoxMonData(src, MON_DATA_PERSONALITY), OT_ID_PRESET, GetBoxMonData(src, MON_DATA_OT_ID));

    for (i = MON_DATA_HP_IV; i <= MON_DATA_SPDEF_IV; i++)
    {
        data = GetBoxMonData(src, i);
        SetMonData(dest, i, &data);
    }
    data = GetBoxMonData(src, MON_DATA_ABILITY_NUM);
    SetMonData(dest, MON_DATA_ABILITY_NUM, &data);
    data = GetBoxMonData(src, MON_DATA_LANGUAGE);
    SetMonData(dest, MON_DATA_LANGUAGE, &data);
    data = GetBoxMonData(src, MON_DATA_OT_GENDER);
    SetMonData(dest, MON_DATA_OT_GENDER, &data);
    GetBoxMonData(src, MON_DATA_OT_NAME, otName);
    SetMonData(dest, MON_DATA_OT_NAME, otName);

    // Only keep a real nickname, not the old species name
    GetBoxMonData(src, MON_DATA_NICKNAME, nickname);
    if (StringCompare(nickname, gSpeciesNames[species]) != 0)
        SetMonData(dest, MON_DATA_NICKNAME, nickname);

    data = MAPSEC_ROUTE_101;
    SetMonData(dest, MON_DATA_MET_LOCATION, &data);
    data = 5;
    SetMonData(dest, MON_DATA_MET_LEVEL, &data);
    data = ITEM_POKE_BALL;
    SetMonData(dest, MON_DATA_POKEBALL, &data);

    CalculateMonStats(dest);
}

// Rebuild every starter in the party and PC boxes into starters[]
static u32 CollectStarters(struct BoxPokemon *starters)
{
    struct Pokemon mon;
    u32 count = 0;
    u32 i, j;

    for (i = 0; i < PARTY_SIZE && count < NGP_MAX_STARTERS; i++)
    {
        if (IsOriginalStarter(&gPlayerParty[i].box))
        {
            MakeLevel5Starter(&mon, &gPlayerParty[i].box);
            starters[count++] = mon.box;
        }
    }
    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
    {
        for (j = 0; j < IN_BOX_COUNT && count < NGP_MAX_STARTERS; j++)
        {
            if (IsOriginalStarter(&gPokemonStoragePtr->boxes[i][j]))
            {
                MakeLevel5Starter(&mon, &gPokemonStoragePtr->boxes[i][j]);
                starters[count++] = mon.box;
            }
        }
    }
    return count;
}

static u16 CountMasterBalls(void)
{
    u32 count = CountTotalItemQuantityInBag(ITEM_MASTER_BALL);
    u32 i;

    for (i = 0; i < PC_ITEMS_COUNT; i++)
    {
        if (gSaveBlock1Ptr->pcItems[i].itemId == ITEM_MASTER_BALL)
            count += gSaveBlock1Ptr->pcItems[i].quantity;
    }
    return min(count, USHRT_MAX);
}

static u16 GetNewGamePlusShinyRolls(void)
{
    // Called as Champion, so all 8 badges make GetShinyRolls deterministic
    u32 rolls = GetShinyRolls();

    // Count the Shiny Charm even if it's stored in the PC
    if (!CheckBagHasItem(ITEM_SHINY_CHARM, 1) && CheckPCHasItem(ITEM_SHINY_CHARM, 1))
        rolls += SHINY_CHARM_ROLLS - 1;

    return rolls / NGP_SHINY_ROLL_DIVISOR;
}

void NewGamePlusInitData(void)
{
    struct BoxPokemon *starters = AllocZeroed(sizeof(struct BoxPokemon) * NGP_MAX_STARTERS);
    u32 numStarters = (starters != NULL) ? CollectStarters(starters) : 0;
    u32 i;
    u8 trainerId[TRAINER_ID_LENGTH];
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    u16 masterBalls = CountMasterBalls();
    u16 shinyRolls = GetNewGamePlusShinyRolls();
    u16 dexNum;

    memcpy(trainerId, gSaveBlock2Ptr->playerTrainerId, TRAINER_ID_LENGTH);

    NewGameInitData();

    memcpy(gSaveBlock2Ptr->playerTrainerId, trainerId, TRAINER_ID_LENGTH);
    SetMoney(&gSaveBlock1Ptr->money, money);
    if (masterBalls != 0 && !AddBagItem(ITEM_MASTER_BALL, masterBalls))
        AddPCItem(ITEM_MASTER_BALL, masterBalls);
    VarSet(VAR_NGP_SHINY_ROLLS, shinyRolls);

    for (i = 0; i < numStarters; i++)
    {
        gPokemonStoragePtr->boxes[0][i] = starters[i];

        dexNum = SpeciesToNationalPokedexNum(GetBoxMonData(&starters[i], MON_DATA_SPECIES));
        GetSetPokedexFlag(dexNum, FLAG_SET_SEEN);
        GetSetPokedexFlag(dexNum, FLAG_SET_CAUGHT);
    }
    if (starters != NULL)
        Free(starters);
}

// Special, called from the bed script after the screen fades out
void StartNewGamePlus(void)
{
    SetMainCallback2(CB2_NewGamePlus);
}

static void ResetMiniGamesRecords(void)
{
    CpuFill16(0, &gSaveBlock2Ptr->berryCrush, sizeof(struct BerryCrush));
    SetBerryPowder(&gSaveBlock2Ptr->berryCrush.berryPowderAmount, 0);
    ResetPokemonJumpRecords();
    CpuFill16(0, &gSaveBlock2Ptr->berryPick, sizeof(struct BerryPickingResults));
}
