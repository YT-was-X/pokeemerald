#ifndef GUARD_GIFT_MON_TERMINAL_H
#define GUARD_GIFT_MON_TERMINAL_H

// An entry of a list menu shown from a script (gift terminal, black market, fossil choice)
struct ShopListEntry
{
    const u8 *name;
    u16 id; // returned in VAR_RESULT when chosen
    u32 price; // 0: no price shown
};

void ShowShopListMenu(const struct ShopListEntry *entries, u32 numEntries);
void ShowShopListMenuCoins(const struct ShopListEntry *entries, u32 numEntries);
u16 BuyMonForPlayer(u16 species, u8 level, u32 price, bool8 fatefulEncounter);
u16 GiveBoughtMonToPlayer(u16 species, u8 level, bool8 fatefulEncounter);

#endif // GUARD_GIFT_MON_TERMINAL_H
