// Black market on the second floor of every Pokémon Center: a Team Magma or
// Team Aqua grunt who sells a few Pokémon each day and buys the player's.
// The stock comes from the in-game clock's day count, so it needs no save data.
#include "global.h"
#include "data.h"
#include "event_data.h"
#include "gift_mon_terminal.h"
#include "item.h"
#include "mail.h"
#include "money.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "rtc.h"
#include "string_util.h"
#include "constants/black_market.h"
#include "constants/items.h"
#include "constants/species.h"

#define STOCK_SIZE 5

struct BlackMarketMon
{
    u16 species;
    u16 basePrice;
};

// Pokémon you can't catch in Emerald, plus a few rare Hoenn ones
static const struct BlackMarketMon sBlackMarketPool[] =
{
    {SPECIES_GROWLITHE,   4000},
    {SPECIES_PONYTA,      4000},
    {SPECIES_FARFETCHD,   4000},
    {SPECIES_GASTLY,      4000},
    {SPECIES_ONIX,        4000},
    {SPECIES_CUBONE,      4000},
    {SPECIES_LICKITUNG,   4000},
    {SPECIES_TANGELA,     4000},
    {SPECIES_SEEL,        4000},
    {SPECIES_DUNSPARCE,   4000},
    {SPECIES_MR_MIME,     5000},
    {SPECIES_JYNX,        5000},
    {SPECIES_MURKROW,     5000},
    {SPECIES_MISDREAVUS,  5000},
    {SPECIES_SLOWPOKE,    5000},
    {SPECIES_SWINUB,      5000},
    {SPECIES_YANMA,       5000},
    {SPECIES_QWILFISH,    5000},
    {SPECIES_KANGASKHAN,  6000},
    {SPECIES_TAUROS,      6000},
    {SPECIES_ELECTABUZZ,  6000},
    {SPECIES_MAGMAR,      6000},
    {SPECIES_HITMONLEE,   6000},
    {SPECIES_HITMONCHAN,  6000},
    {SPECIES_TYROGUE,     6000},
    {SPECIES_SNEASEL,     6000},
    {SPECIES_EXEGGCUTE,   6000},
    {SPECIES_CHIMECHO,    6000},
    {SPECIES_OMANYTE,     7000},
    {SPECIES_KABUTO,      7000},
    {SPECIES_SCYTHER,     8000},
    {SPECIES_DITTO,       8000},
    {SPECIES_FEEBAS,      8000},
    {SPECIES_ABSOL,       8000},
    {SPECIES_RELICANTH,   8000},
    {SPECIES_CHANSEY,    10000},
    {SPECIES_AERODACTYL, 12000},
    {SPECIES_SNORLAX,    12000},
    {SPECIES_DRATINI,    15000},
    {SPECIES_LARVITAR,   15000},
    {SPECIES_BAGON,      15000},
    {SPECIES_BELDUM,     15000},
};

// Bought Pokémon are Lv. 5 plus 5 per badge (up to Lv. 45), or Lv. 50 after the Elite Four
#define LEVEL_BASE         5
#define LEVEL_PER_BADGE    5
#define LEVEL_CHAMPION    50
#define PRICE_PER_LEVEL  200

// What the grunt pays for the player's Pokémon
#define SELL_PRICE_PER_LEVEL  100
#define SELL_PRICE_PER_IV      50
#define SELL_SHINY_MULTIPLIER  10

static u8 GetBlackMarketLevel(void)
{
    u32 i, badges = 0;

    if (FlagGet(FLAG_SYS_GAME_CLEAR))
        return LEVEL_CHAMPION;

    for (i = FLAG_BADGE01_GET; i < FLAG_BADGE01_GET + NUM_BADGES; i++)
    {
        if (FlagGet(i))
            badges++;
    }
    return LEVEL_BASE + LEVEL_PER_BADGE * badges;
}

static u32 GetBlackMarketPrice(u32 poolId)
{
    return sBlackMarketPool[poolId].basePrice + PRICE_PER_LEVEL * GetBlackMarketLevel();
}

// Today's stock: STOCK_SIZE different pool entries picked with a generator
// seeded by the day count, so every Pokémon Center has the same stock all day
static void GetTodaysStock(u8 *stock)
{
    u32 seed, i, j;

    RtcCalcLocalTime();
    seed = gLocalTime.days * 2654435761u + 0x5BD1E995;
    for (i = 0; i < STOCK_SIZE; i++)
    {
        do
        {
            seed = seed * 1103515245 + 12345;
            stock[i] = (seed >> 16) % ARRAY_COUNT(sBlackMarketPool);
            for (j = 0; j < i && stock[j] != stock[i]; j++)
                ;
        } while (j != i);
    }
}

// Special: show today's stock; VAR_RESULT = the chosen pool entry, or
// MULTI_B_PRESSED. Use waitstate after it.
void ShowBlackMarketMenu(void)
{
    struct MonShopEntry entries[STOCK_SIZE];
    u8 stock[STOCK_SIZE];
    u32 i;

    GetTodaysStock(stock);
    for (i = 0; i < STOCK_SIZE; i++)
    {
        entries[i].species = sBlackMarketPool[stock[i]].species;
        entries[i].id = stock[i];
        entries[i].price = GetBlackMarketPrice(stock[i]);
    }
    ShowMonShopMenu(entries, STOCK_SIZE);
}

// Special: VAR_0x8004 = pool entry; buffers STR_VAR_1 = species name,
// STR_VAR_2 = price and STR_VAR_3 = level for the confirmation message
void BufferBlackMarketMonInfo(void)
{
    StringCopy(gStringVar1, gSpeciesNames[sBlackMarketPool[gSpecialVar_0x8004].species]);
    ConvertIntToDecimalStringN(gStringVar2, GetBlackMarketPrice(gSpecialVar_0x8004), STR_CONV_MODE_LEFT_ALIGN, 6);
    ConvertIntToDecimalStringN(gStringVar3, GetBlackMarketLevel(), STR_CONV_MODE_LEFT_ALIGN, 3);
}

// Special: VAR_0x8004 = pool entry; buys it. VAR_RESULT = GIFT_MON_* result
void TryBuyBlackMarketMon(void)
{
    gSpecialVar_Result = BuyMonForPlayer(sBlackMarketPool[gSpecialVar_0x8004].species,
                                         GetBlackMarketLevel(),
                                         GetBlackMarketPrice(gSpecialVar_0x8004),
                                         FALSE);
}

static u32 GetBlackMarketSellPrice(struct Pokemon *mon)
{
    u32 price, ivTotal = 0;
    u32 i;

    for (i = 0; i < NUM_STATS; i++)
        ivTotal += GetMonData(mon, MON_DATA_HP_IV + i);

    price = SELL_PRICE_PER_LEVEL * GetMonData(mon, MON_DATA_LEVEL) + SELL_PRICE_PER_IV * ivTotal;
    if (IsMonShiny(mon))
        price *= SELL_SHINY_MULTIPLIER;
    return price;
}

// Whether the party still has a Pokémon that can battle without the one in slot
static bool8 HasOtherUsableMon(u8 slot)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (i != slot
         && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG)
         && GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0)
            return TRUE;
    }
    return FALSE;
}

// Special: VAR_0x8004 = party slot. VAR_RESULT = BLACK_MARKET_SELL_* result.
// Buffers STR_VAR_1 = nickname, STR_VAR_2 = price, STR_VAR_3 = held item name.
u16 CheckBlackMarketSell(void)
{
    struct Pokemon *mon = &gPlayerParty[gSpecialVar_0x8004];
    u16 item = GetMonData(mon, MON_DATA_HELD_ITEM);

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, GetBlackMarketSellPrice(mon), STR_CONV_MODE_LEFT_ALIGN, 7);
    if (item != ITEM_NONE)
        CopyItemName(item, gStringVar3);

    if (GetMonData(mon, MON_DATA_IS_EGG))
        return BLACK_MARKET_SELL_EGG;
    if (!HasOtherUsableMon(gSpecialVar_0x8004))
        return BLACK_MARKET_SELL_LAST_MON;
    if (ItemIsMail(item))
        return BLACK_MARKET_SELL_MAIL;
    if (item != ITEM_NONE && !CheckBagHasSpace(item, 1))
        return BLACK_MARKET_SELL_BAG_FULL;
    return BLACK_MARKET_SELL_OK;
}

// Special: VAR_0x8004 = party slot (already checked with CheckBlackMarketSell).
// Pays for the Pokémon and removes it, putting its held item in the bag.
// VAR_RESULT = TRUE if an item went back to the bag.
u16 SellBlackMarketMon(void)
{
    struct Pokemon *mon = &gPlayerParty[gSpecialVar_0x8004];
    u16 item = GetMonData(mon, MON_DATA_HELD_ITEM);

    AddMoney(&gSaveBlock1Ptr->money, GetBlackMarketSellPrice(mon));
    if (item != ITEM_NONE)
        AddBagItem(item, 1);
    ZeroMonData(mon);
    CompactPartySlots();
    CalculatePlayerPartyCount();
    return item != ITEM_NONE;
}
