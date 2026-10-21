// Fossil revival at Devon Corp. 2F: besides the Root and Claw Fossils, the
// researcher also revives the Kanto fossils (Helix, Dome and Old Amber).
// VAR_WHICH_FOSSIL_REVIVED holds the fossil being revived (1 + index in sFossils;
// 1 and 2 are the vanilla Root and Claw values).
#include "global.h"
#include "event_data.h"
#include "gift_mon_terminal.h"
#include "item.h"
#include "string_util.h"
#include "constants/items.h"
#include "constants/species.h"

struct Fossil
{
    u16 item;
    u16 species;
};

static const struct Fossil sFossils[] =
{
    {ITEM_ROOT_FOSSIL,  SPECIES_LILEEP},
    {ITEM_CLAW_FOSSIL,  SPECIES_ANORITH},
    {ITEM_HELIX_FOSSIL, SPECIES_OMANYTE},
    {ITEM_DOME_FOSSIL,  SPECIES_KABUTO},
    {ITEM_OLD_AMBER,    SPECIES_AERODACTYL},
};

#define FOSSIL_ID(index) ((index) + 1)
#define FOSSIL_INDEX(id) ((id) - 1)

// Special: VAR_RESULT = number of different fossils in the bag
u16 CountFossilsInBag(void)
{
    u32 i, count = 0;

    for (i = 0; i < ARRAY_COUNT(sFossils); i++)
    {
        if (CheckBagHasItem(sFossils[i].item, 1))
            count++;
    }
    return count;
}

// Special: VAR_RESULT = the fossil id of the first fossil in the bag
u16 GetFirstFossilInBag(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sFossils); i++)
    {
        if (CheckBagHasItem(sFossils[i].item, 1))
            return FOSSIL_ID(i);
    }
    return 0;
}

// Special: list the fossils in the bag; VAR_RESULT = the chosen fossil id,
// or MULTI_B_PRESSED. Use waitstate after it.
void ShowFossilChoiceMenu(void)
{
    struct ShopListEntry entries[ARRAY_COUNT(sFossils)];
    u32 i, count = 0;

    for (i = 0; i < ARRAY_COUNT(sFossils); i++)
    {
        if (CheckBagHasItem(sFossils[i].item, 1))
        {
            entries[count].name = GetItemName(sFossils[i].item);
            entries[count].id = FOSSIL_ID(i);
            entries[count].price = 0;
            count++;
        }
    }
    ShowShopListMenu(entries, count);
}

// Special: VAR_0x8004 = fossil id. Takes the fossil and starts reviving it.
// Buffers STR_VAR_1 = the fossil's name.
void HandFossilToResearcher(void)
{
    u16 item = sFossils[FOSSIL_INDEX(gSpecialVar_0x8004)].item;

    CopyItemName(item, gStringVar1);
    RemoveBagItem(item, 1);
    VarSet(VAR_FOSSIL_RESURRECTION_STATE, 1);
    VarSet(VAR_WHICH_FOSSIL_REVIVED, gSpecialVar_0x8004);
}

// Special: VAR_RESULT = the species of the revived fossil Pokémon
u16 GetRevivedFossilSpecies(void)
{
    u16 id = VarGet(VAR_WHICH_FOSSIL_REVIVED);

    if (id < FOSSIL_ID(0) || id > FOSSIL_ID(ARRAY_COUNT(sFossils) - 1))
        id = FOSSIL_ID(0);
    return sFossils[FOSSIL_INDEX(id)].species;
}
