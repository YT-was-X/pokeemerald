// Mauville Game Corner prize exchange: TMs, training items, evolution items and
// Pokémon bought with coins, picked from a scrolling list per category.
#include "global.h"
#include "coins.h"
#include "data.h"
#include "event_data.h"
#include "gift_mon_terminal.h"
#include "item.h"
#include "party_menu.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/game_corner_prizes.h"
#include "constants/gift_mon_terminal.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"

struct GameCornerPrize
{
    u16 id;    // item, or species for Pokémon
    u8 level;  // Pokémon only
    u16 coins;
};

static const struct GameCornerPrize sPrizeTMs[] =
{
    {ITEM_TM_DOUBLE_TEAM,  0, 1500},
    {ITEM_TM_TOXIC,        0, 2000},
    {ITEM_TM_PROTECT,      0, 2000},
    {ITEM_TM_GIGA_DRAIN,   0, 2500},
    {ITEM_TM_BRICK_BREAK,  0, 2500},
    {ITEM_TM_CALM_MIND,    0, 3000},
    {ITEM_TM_BULK_UP,      0, 3000},
    {ITEM_TM_DRAGON_CLAW,  0, 3000},
    {ITEM_TM_FOCUS_PUNCH,  0, 3000},
    {ITEM_TM_SLUDGE_BOMB,  0, 3000},
    {ITEM_TM_PSYCHIC,      0, 3500},
    {ITEM_TM_SHADOW_BALL,  0, 3500},
    {ITEM_TM_FLAMETHROWER, 0, 4000},
    {ITEM_TM_THUNDERBOLT,  0, 4000},
    {ITEM_TM_ICE_BEAM,     0, 4000},
    {ITEM_TM_EARTHQUAKE,   0, 4500},
    {ITEM_TM_OVERHEAT,     0, 4500},
    {ITEM_TM_FIRE_BLAST,   0, 4500},
    {ITEM_TM_THUNDER,      0, 4500},
    {ITEM_TM_BLIZZARD,     0, 4500},
    {ITEM_TM_HYPER_BEAM,   0, 5000},
};

static const struct GameCornerPrize sPrizeItems[] =
{
    {ITEM_RARE_CANDY,  0, 1000},
    {ITEM_HP_UP,       0,  800},
    {ITEM_PROTEIN,     0,  800},
    {ITEM_IRON,        0,  800},
    {ITEM_CALCIUM,     0,  800},
    {ITEM_ZINC,        0,  800},
    {ITEM_CARBOS,      0,  800},
    {ITEM_PP_UP,       0, 1500},
    {ITEM_PP_MAX,      0, 3000},
    {ITEM_QUICK_CLAW,  0, 1500},
    {ITEM_SCOPE_LENS,  0, 2000},
    {ITEM_FOCUS_BAND,  0, 2000},
    {ITEM_SHELL_BELL,  0, 2000},
    {ITEM_LUCKY_EGG,   0, 3000},
    {ITEM_LEFTOVERS,   0, 3000},
    {ITEM_CHOICE_BAND, 0, 3000},
    {ITEM_MASTER_BALL, 0, 9999},
};

static const struct GameCornerPrize sPrizeEvoItems[] =
{
    {ITEM_FIRE_STONE,     0, 1000},
    {ITEM_WATER_STONE,    0, 1000},
    {ITEM_THUNDER_STONE,  0, 1000},
    {ITEM_LEAF_STONE,     0, 1000},
    {ITEM_MOON_STONE,     0, 1000},
    {ITEM_SUN_STONE,      0, 1000},
    {ITEM_KINGS_ROCK,     0, 1500},
    {ITEM_METAL_COAT,     0, 1500},
    {ITEM_DRAGON_SCALE,   0, 1500},
    {ITEM_UP_GRADE,       0, 1500},
    {ITEM_DEEP_SEA_TOOTH, 0, 1500},
    {ITEM_DEEP_SEA_SCALE, 0, 1500},
};

// The FireRed/LeafGreen Game Corner Pokémon
static const struct GameCornerPrize sPrizePokemon[] =
{
    {SPECIES_ABRA,     10,  300},
    {SPECIES_CLEFAIRY, 10,  600},
    {SPECIES_DRATINI,  18, 2800},
    {SPECIES_SCYTHER,  25, 5500},
    {SPECIES_PINSIR,   25, 5500},
    {SPECIES_PORYGON,  26, 9999},
};

static const struct {
    const struct GameCornerPrize *prizes;
    u8 count;
} sPrizeCategories[NUM_PRIZE_CATEGORIES] =
{
    [PRIZE_CATEGORY_TMS]       = {sPrizeTMs,      ARRAY_COUNT(sPrizeTMs)},
    [PRIZE_CATEGORY_ITEMS]     = {sPrizeItems,    ARRAY_COUNT(sPrizeItems)},
    [PRIZE_CATEGORY_EVO_ITEMS] = {sPrizeEvoItems, ARRAY_COUNT(sPrizeEvoItems)},
    [PRIZE_CATEGORY_POKEMON]   = {sPrizePokemon,  ARRAY_COUNT(sPrizePokemon)},
};

#define MAX_PRIZES_PER_CATEGORY 24
#define PRIZE_NAME_LENGTH       24

// TM names ("TM13 ICE BEAM") must outlive the list menu
static EWRAM_DATA u8 sPrizeNames[MAX_PRIZES_PER_CATEGORY][PRIZE_NAME_LENGTH] = {0};

static const struct GameCornerPrize *GetPrize(u16 category, u16 index)
{
    if (category >= NUM_PRIZE_CATEGORIES || index >= sPrizeCategories[category].count)
        return NULL;
    return &sPrizeCategories[category].prizes[index];
}

static void CopyPrizeName(u16 category, const struct GameCornerPrize *prize, u8 *dest)
{
    if (category == PRIZE_CATEGORY_POKEMON)
    {
        StringCopy(dest, gSpeciesNames[prize->id]);
    }
    else if (category == PRIZE_CATEGORY_TMS)
    {
        dest = StringCopy(dest, GetItemName(prize->id));
        *dest++ = CHAR_SPACE;
        StringCopy(dest, gMoveNames[ItemIdToBattleMoveId(prize->id)]);
    }
    else
    {
        CopyItemName(prize->id, dest);
    }
}

// Special: VAR_0x8004 = category. Shows its prizes; VAR_RESULT = the chosen
// index, or MULTI_B_PRESSED. Use waitstate after it.
void ShowGameCornerPrizeMenu(void)
{
    struct ShopListEntry entries[MAX_PRIZES_PER_CATEGORY];
    u16 category = gSpecialVar_0x8004;
    u32 i, count = sPrizeCategories[category].count;

    for (i = 0; i < count; i++)
    {
        const struct GameCornerPrize *prize = GetPrize(category, i);

        CopyPrizeName(category, prize, sPrizeNames[i]);
        entries[i].name = sPrizeNames[i];
        entries[i].id = i;
        entries[i].price = prize->coins;
    }
    ShowShopListMenuCoins(entries, count);
}

// Special: VAR_0x8004 = category, VAR_0x8005 = index. Buffers STR_VAR_1 = name,
// STR_VAR_2 = coins and STR_VAR_3 = level (Pokémon). VAR_RESULT = TRUE for a Pokémon.
u16 BufferGameCornerPrizeInfo(void)
{
    const struct GameCornerPrize *prize = GetPrize(gSpecialVar_0x8004, gSpecialVar_0x8005);

    CopyPrizeName(gSpecialVar_0x8004, prize, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, prize->coins, STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar3, prize->level, STR_CONV_MODE_LEFT_ALIGN, 3);
    return gSpecialVar_0x8004 == PRIZE_CATEGORY_POKEMON;
}

// Special: VAR_0x8004 = category, VAR_0x8005 = index. VAR_RESULT = PRIZE_* result.
u16 TryBuyGameCornerPrize(void)
{
    const struct GameCornerPrize *prize = GetPrize(gSpecialVar_0x8004, gSpecialVar_0x8005);

    if (GetCoins() < prize->coins)
        return PRIZE_NOT_ENOUGH_COINS;

    if (gSpecialVar_0x8004 == PRIZE_CATEGORY_POKEMON)
    {
        switch (GiveBoughtMonToPlayer(prize->id, prize->level, FALSE))
        {
        case GIFT_MON_BOUGHT_PARTY:
            RemoveCoins(prize->coins);
            return PRIZE_BOUGHT_MON_PARTY;
        case GIFT_MON_BOUGHT_PC:
            RemoveCoins(prize->coins);
            return PRIZE_BOUGHT_MON_PC;
        default:
            return PRIZE_NO_ROOM_FOR_MON;
        }
    }

    if (!AddBagItem(prize->id, 1))
        return PRIZE_BAG_FULL;
    RemoveCoins(prize->coins);
    return PRIZE_BOUGHT_ITEM;
}
