#ifndef GUARD_GIFT_MON_TERMINAL_H
#define GUARD_GIFT_MON_TERMINAL_H

// A Pokémon listed in a shop menu (gift terminal or black market)
struct MonShopEntry
{
    u16 species;
    u16 id; // returned in VAR_RESULT when chosen
    u32 price;
};

void ShowMonShopMenu(const struct MonShopEntry *entries, u32 numEntries);
u16 BuyMonForPlayer(u16 species, u8 level, u32 price, bool8 fatefulEncounter);

#endif // GUARD_GIFT_MON_TERMINAL_H
