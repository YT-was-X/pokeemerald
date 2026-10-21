// Battle Frontier Exchange Service Corner: an extra clerk with more prizes for BP,
// picked from a scrolling list per category (like the Game Corner prizes).
#include "global.h"
#include "data.h"
#include "event_data.h"
#include "gift_mon_terminal.h"
#include "item.h"
#include "party_menu.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/frontier_exchange_prizes.h"
#include "constants/items.h"
#include "constants/moves.h"

struct FrontierPrize
{
    u16 item;
    u16 bp;
};

static const struct FrontierPrize sPrizesTraining[] =
{
    {ITEM_RARE_CANDY,     4},
    {ITEM_PP_UP,          8},
    {ITEM_PP_MAX,        24},
    {ITEM_HEART_SCALE,    2},
    {ITEM_EVERSTONE,      8},
    {ITEM_POWER_WEIGHT,  32},
    {ITEM_POWER_BRACER,  32},
    {ITEM_POWER_BELT,    32},
    {ITEM_POWER_ANKLET,  32},
    {ITEM_POWER_LENS,    32},
    {ITEM_POWER_BAND,    32},
    {ITEM_MACHO_BRACE,   16},
    {ITEM_SOOTHE_BELL,   16},
    {ITEM_EXP_SHARE,     24},
    {ITEM_LUCKY_EGG,     32},
};

static const struct FrontierPrize sPrizesHoldItems[] =
{
    {ITEM_SHELL_BELL,    32},
    {ITEM_LAX_INCENSE,   32},
    {ITEM_SEA_INCENSE,   16},
    {ITEM_LIGHT_BALL,    32},
    {ITEM_THICK_CLUB,    32},
    {ITEM_METAL_POWDER,  16},
    {ITEM_LIECHI_BERRY,  32},
    {ITEM_GANLON_BERRY,  32},
    {ITEM_SALAC_BERRY,   32},
    {ITEM_PETAYA_BERRY,  32},
    {ITEM_APICOT_BERRY,  32},
    {ITEM_LANSAT_BERRY,  48},
    {ITEM_STARF_BERRY,   48},
};

static const struct FrontierPrize sPrizesTypeItems[] =
{
    {ITEM_SILK_SCARF,     24},
    {ITEM_CHARCOAL,       24},
    {ITEM_MYSTIC_WATER,   24},
    {ITEM_MAGNET,         24},
    {ITEM_MIRACLE_SEED,   24},
    {ITEM_NEVER_MELT_ICE, 24},
    {ITEM_BLACK_BELT,     24},
    {ITEM_POISON_BARB,    24},
    {ITEM_SOFT_SAND,      24},
    {ITEM_SHARP_BEAK,     24},
    {ITEM_TWISTED_SPOON,  24},
    {ITEM_SILVER_POWDER,  24},
    {ITEM_HARD_STONE,     24},
    {ITEM_SPELL_TAG,      24},
    {ITEM_DRAGON_FANG,    24},
    {ITEM_BLACK_GLASSES,  24},
    {ITEM_METAL_COAT,     24},
};

static const struct FrontierPrize sPrizesEvoItems[] =
{
    {ITEM_FIRE_STONE,     8},
    {ITEM_WATER_STONE,    8},
    {ITEM_THUNDER_STONE,  8},
    {ITEM_LEAF_STONE,     8},
    {ITEM_MOON_STONE,     8},
    {ITEM_SUN_STONE,      8},
    {ITEM_KINGS_ROCK,    16},
    {ITEM_METAL_COAT,    16},
    {ITEM_DRAGON_SCALE,  16},
    {ITEM_UP_GRADE,      16},
    {ITEM_DEEP_SEA_TOOTH, 16},
    {ITEM_DEEP_SEA_SCALE, 16},
};

static const struct FrontierPrize sPrizesTMs[] =
{
    {ITEM_TM_REFLECT,      16},
    {ITEM_TM_LIGHT_SCREEN, 16},
    {ITEM_TM_REST,         16},
    {ITEM_TM_TOXIC,        24},
    {ITEM_TM_PROTECT,      24},
    {ITEM_TM_HIDDEN_POWER, 24},
    {ITEM_TM_RETURN,       24},
    {ITEM_TM_AERIAL_ACE,   24},
    {ITEM_TM_BRICK_BREAK,  32},
    {ITEM_TM_CALM_MIND,    32},
    {ITEM_TM_BULK_UP,      32},
    {ITEM_TM_DRAGON_CLAW,  32},
    {ITEM_TM_GIGA_DRAIN,   32},
    {ITEM_TM_SLUDGE_BOMB,  32},
    {ITEM_TM_SHADOW_BALL,  40},
    {ITEM_TM_OVERHEAT,     40},
    {ITEM_TM_PSYCHIC,      48},
    {ITEM_TM_FLAMETHROWER, 48},
    {ITEM_TM_THUNDERBOLT,  48},
    {ITEM_TM_ICE_BEAM,     48},
    {ITEM_TM_EARTHQUAKE,   48},
    {ITEM_TM_HYPER_BEAM,   48},
};

static const struct {
    const struct FrontierPrize *prizes;
    u8 count;
} sPrizeCategories[NUM_BP_PRIZE_CATEGORIES] =
{
    [BP_PRIZE_CATEGORY_TRAINING]   = {sPrizesTraining,  ARRAY_COUNT(sPrizesTraining)},
    [BP_PRIZE_CATEGORY_HOLD_ITEMS] = {sPrizesHoldItems, ARRAY_COUNT(sPrizesHoldItems)},
    [BP_PRIZE_CATEGORY_TYPE_ITEMS] = {sPrizesTypeItems, ARRAY_COUNT(sPrizesTypeItems)},
    [BP_PRIZE_CATEGORY_EVO_ITEMS]  = {sPrizesEvoItems,  ARRAY_COUNT(sPrizesEvoItems)},
    [BP_PRIZE_CATEGORY_TMS]        = {sPrizesTMs,       ARRAY_COUNT(sPrizesTMs)},
};

#define MAX_PRIZES_PER_CATEGORY 24
#define PRIZE_NAME_LENGTH       24

// TM names ("TM13 ICE BEAM") must outlive the list menu
static EWRAM_DATA u8 sPrizeNames[MAX_PRIZES_PER_CATEGORY][PRIZE_NAME_LENGTH] = {0};

static const struct FrontierPrize *GetPrize(u16 category, u16 index)
{
    if (category >= NUM_BP_PRIZE_CATEGORIES || index >= sPrizeCategories[category].count)
        return NULL;
    return &sPrizeCategories[category].prizes[index];
}

static void CopyPrizeName(const struct FrontierPrize *prize, u8 *dest)
{
    if (prize->item >= ITEM_TM01 && prize->item <= ITEM_TM50)
    {
        dest = StringCopy(dest, GetItemName(prize->item));
        *dest++ = CHAR_SPACE;
        StringCopy(dest, gMoveNames[ItemIdToBattleMoveId(prize->item)]);
    }
    else
    {
        CopyItemName(prize->item, dest);
    }
}

// Special: VAR_0x8004 = category. Shows its prizes; VAR_RESULT = the chosen
// index, or MULTI_B_PRESSED. Use waitstate after it.
void ShowFrontierPrizeMenu(void)
{
    struct ShopListEntry entries[MAX_PRIZES_PER_CATEGORY];
    u16 category = gSpecialVar_0x8004;
    u32 i, count = sPrizeCategories[category].count;

    for (i = 0; i < count; i++)
    {
        const struct FrontierPrize *prize = GetPrize(category, i);

        CopyPrizeName(prize, sPrizeNames[i]);
        entries[i].name = sPrizeNames[i];
        entries[i].id = i;
        entries[i].price = prize->bp;
    }
    ShowShopListMenuBP(entries, count);
}

// Special: VAR_0x8004 = category, VAR_0x8005 = index. Buffers STR_VAR_1 = name
// and STR_VAR_2 = BP.
void BufferFrontierPrizeInfo(void)
{
    const struct FrontierPrize *prize = GetPrize(gSpecialVar_0x8004, gSpecialVar_0x8005);

    CopyPrizeName(prize, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, prize->bp, STR_CONV_MODE_LEFT_ALIGN, 3);
}

// Special: VAR_0x8004 = category, VAR_0x8005 = index. VAR_RESULT = BP_PRIZE_* result.
u16 TryBuyFrontierPrize(void)
{
    const struct FrontierPrize *prize = GetPrize(gSpecialVar_0x8004, gSpecialVar_0x8005);

    if (gSaveBlock2Ptr->frontier.battlePoints < prize->bp)
        return BP_PRIZE_NOT_ENOUGH;
    if (!AddBagItem(prize->item, 1))
        return BP_PRIZE_BAG_FULL;
    gSaveBlock2Ptr->frontier.battlePoints -= prize->bp;
    return BP_PRIZE_BOUGHT;
}
