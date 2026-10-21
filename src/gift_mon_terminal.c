// Gift Pokémon terminal on the second floor of every Pokémon Center.
// Sells gift and event Pokémon that Emerald can't otherwise get. Each gym
// badge unlocks more, and beating the Elite Four unlocks the legendaries.
// Pokémon can be bought any number of times.
#include "global.h"
#include "data.h"
#include "event_data.h"
#include "gift_mon_terminal.h"
#include "international_string_util.h"
#include "list_menu.h"
#include "malloc.h"
#include "menu.h"
#include "money.h"
#include "pokedex.h"
#include "pokemon.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "window.h"
#include "constants/gift_mon_terminal.h"
#include "constants/script_menu.h"
#include "constants/songs.h"
#include "constants/species.h"

#define UNLOCK_CHAMPION (NUM_BADGES + 1)

struct GiftMon
{
    u16 species;
    u8 level;
    u8 unlock; // number of badges needed, or UNLOCK_CHAMPION
    u32 price;
};

static const struct GiftMon sGiftMons[] =
{
    // 1 badge: Kanto starters
    {SPECIES_BULBASAUR,  5, 1, 5000},
    {SPECIES_CHARMANDER, 5, 1, 5000},
    {SPECIES_SQUIRTLE,   5, 1, 5000},
    // 2 badges: Johto starters
    {SPECIES_CHIKORITA,  5, 2, 5000},
    {SPECIES_CYNDAQUIL,  5, 2, 5000},
    {SPECIES_TOTODILE,   5, 2, 5000},
    // 3 badges
    {SPECIES_EEVEE,      5, 3, 8000},
    {SPECIES_TOGEPI,     5, 3, 8000},
    // 4 badges
    {SPECIES_LAPRAS,    25, 4, 15000},
    {SPECIES_PORYGON,   25, 4, 15000},
    // 5-8 badges: event Pokémon
    {SPECIES_JIRACHI,    5, 5, 30000},
    {SPECIES_CELEBI,    10, 6, 30000},
    {SPECIES_MEW,       10, 7, 40000},
    {SPECIES_DEOXYS,    30, 8, 40000},
    // Champion: legendaries
    {SPECIES_ARTICUNO,  50, UNLOCK_CHAMPION, 50000},
    {SPECIES_ZAPDOS,    50, UNLOCK_CHAMPION, 50000},
    {SPECIES_MOLTRES,   50, UNLOCK_CHAMPION, 50000},
    {SPECIES_MEWTWO,    70, UNLOCK_CHAMPION, 80000},
    {SPECIES_RAIKOU,    40, UNLOCK_CHAMPION, 50000},
    {SPECIES_ENTEI,     40, UNLOCK_CHAMPION, 50000},
    {SPECIES_SUICUNE,   40, UNLOCK_CHAMPION, 50000},
    {SPECIES_LUGIA,     45, UNLOCK_CHAMPION, 60000},
    {SPECIES_HO_OH,     45, UNLOCK_CHAMPION, 60000},
    {SPECIES_REGIROCK,  40, UNLOCK_CHAMPION, 50000},
    {SPECIES_REGICE,    40, UNLOCK_CHAMPION, 50000},
    {SPECIES_REGISTEEL, 40, UNLOCK_CHAMPION, 50000},
    {SPECIES_LATIAS,    40, UNLOCK_CHAMPION, 60000},
    {SPECIES_LATIOS,    40, UNLOCK_CHAMPION, 60000},
    {SPECIES_KYOGRE,    45, UNLOCK_CHAMPION, 70000},
    {SPECIES_GROUDON,   45, UNLOCK_CHAMPION, 70000},
    {SPECIES_RAYQUAZA,  70, UNLOCK_CHAMPION, 80000},
};

#define MENU_LEFT       1
#define MENU_TOP        1
#define MENU_WIDTH      17
#define MENU_MAX_SHOWN  6
#define MENU_PRICE_X    (MENU_WIDTH * 8 - 2)

// Same sprite tags as the scrollable multichoice menus (field_specials.c)
#define GFXTAG_SCROLL_ARROWS 2000
#define PALTAG_SCROLL_ARROWS 100

#define tListTaskId   data[0]
#define tWindowId     data[1]
#define tNumItems     data[2]
#define tArrowTaskId  data[3]

static EWRAM_DATA struct ListMenuItem *sListItems = NULL;
static EWRAM_DATA struct MonShopEntry *sShopEntries = NULL;
static EWRAM_DATA u16 sScrollOffset = 0;

static void Task_MonShopMenu(u8 taskId);
static void PrintMonShopPrice(u8 windowId, u32 itemId, u8 y);

static bool8 IsGiftMonUnlocked(const struct GiftMon *giftMon)
{
    u32 badges = 0;
    u32 i;

    if (giftMon->unlock == UNLOCK_CHAMPION)
        return FlagGet(FLAG_SYS_GAME_CLEAR);

    for (i = FLAG_BADGE01_GET; i < FLAG_BADGE01_GET + NUM_BADGES; i++)
    {
        if (FlagGet(i))
            badges++;
    }
    return badges >= giftMon->unlock;
}

// Special: VAR_RESULT = number of Pokémon the player can currently buy
void CountUnlockedGiftMons(void)
{
    u32 i;

    gSpecialVar_Result = 0;
    for (i = 0; i < ARRAY_COUNT(sGiftMons); i++)
    {
        if (IsGiftMonUnlocked(&sGiftMons[i]))
            gSpecialVar_Result++;
    }
}

// Special: show the list of unlocked Pokémon; VAR_RESULT = the chosen entry,
// or MULTI_B_PRESSED. Use waitstate after it.
void ShowGiftMonTerminalMenu(void)
{
    struct MonShopEntry entries[ARRAY_COUNT(sGiftMons)];
    u32 i, count = 0;

    for (i = 0; i < ARRAY_COUNT(sGiftMons); i++)
    {
        if (IsGiftMonUnlocked(&sGiftMons[i]))
        {
            entries[count].species = sGiftMons[i].species;
            entries[count].id = i;
            entries[count].price = sGiftMons[i].price;
            count++;
        }
    }
    ShowMonShopMenu(entries, count);
}

// Shows a list of Pokémon with prices, plus EXIT. VAR_RESULT = the chosen
// entry's id, or MULTI_B_PRESSED. The script must waitstate after it.
void ShowMonShopMenu(const struct MonShopEntry *entries, u32 numEntries)
{
    static const struct ScrollArrowsTemplate sArrowsTemplate = {
        .firstArrowType = SCROLL_ARROW_UP,
        .firstX = (MENU_LEFT + MENU_WIDTH / 2) * 8 + 4,
        .firstY = MENU_TOP * 8 - 4,
        .secondArrowType = SCROLL_ARROW_DOWN,
        .secondX = (MENU_LEFT + MENU_WIDTH / 2) * 8 + 4,
        .secondY = (MENU_TOP + MENU_MAX_SHOWN * 2) * 8 + 4,
        .tileTag = GFXTAG_SCROLL_ARROWS,
        .palTag = PALTAG_SCROLL_ARROWS,
        .palNum = 0,
    };
    struct ListMenuTemplate listTemplate = {0};
    struct WindowTemplate windowTemplate;
    struct ScrollArrowsTemplate arrows = sArrowsTemplate;
    u8 taskId = CreateTask(Task_MonShopMenu, 80);
    struct Task *task = &gTasks[taskId];
    u32 count;

    LockPlayerFieldControls();
    sListItems = AllocZeroed((numEntries + 1) * sizeof(*sListItems));
    sShopEntries = Alloc(numEntries * sizeof(*sShopEntries));
    for (count = 0; count < numEntries; count++)
    {
        sShopEntries[count] = entries[count];
        sListItems[count].name = gSpeciesNames[entries[count].species];
        sListItems[count].id = count;
    }
    sListItems[count].name = gText_Exit;
    sListItems[count].id = LIST_CANCEL;
    count++;
    task->tNumItems = count;

    windowTemplate = CreateWindowTemplate(0, MENU_LEFT, MENU_TOP, MENU_WIDTH, min(count, MENU_MAX_SHOWN) * 2, 15, 0x64);
    task->tWindowId = AddWindow(&windowTemplate);
    SetStandardWindowBorderStyle(task->tWindowId, FALSE);

    listTemplate.items = sListItems;
    listTemplate.totalItems = count;
    listTemplate.maxShowed = min(count, MENU_MAX_SHOWN);
    listTemplate.windowId = task->tWindowId;
    listTemplate.itemPrintFunc = PrintMonShopPrice;
    listTemplate.item_X = 8;
    listTemplate.upText_Y = 1;
    listTemplate.cursorPal = 2;
    listTemplate.fillValue = 1;
    listTemplate.cursorShadowPal = 3;
    listTemplate.scrollMultiple = LIST_MULTIPLE_SCROLL_DPAD;
    listTemplate.fontId = FONT_NORMAL;
    listTemplate.cursorKind = CURSOR_BLACK_ARROW;

    sScrollOffset = 0;
    task->tListTaskId = ListMenuInit(&listTemplate, 0, 0);
    task->tArrowTaskId = TASK_NONE;
    if (count > MENU_MAX_SHOWN)
    {
        arrows.fullyDownThreshold = count - MENU_MAX_SHOWN;
        task->tArrowTaskId = AddScrollIndicatorArrowPair(&arrows, &sScrollOffset);
    }
    CopyWindowToVram(task->tWindowId, COPYWIN_FULL);
}

static void PrintMonShopPrice(u8 windowId, u32 itemId, u8 y)
{
    u8 price[16];

    if (itemId == LIST_CANCEL)
        return;
    ConvertIntToDecimalStringN(gStringVar1, sShopEntries[itemId].price, STR_CONV_MODE_LEFT_ALIGN, 6);
    StringExpandPlaceholders(price, gText_PokedollarVar1);
    AddTextPrinterParameterized(windowId, FONT_NARROW, price,
                                GetStringRightAlignXOffset(FONT_NARROW, price, MENU_PRICE_X), y, TEXT_SKIP_DRAW, NULL);
}

static void CloseMonShopMenu(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    if (task->tArrowTaskId != TASK_NONE)
        RemoveScrollIndicatorArrowPair(task->tArrowTaskId);
    DestroyListMenuTask(task->tListTaskId, NULL, NULL);
    Free(sListItems);
    sListItems = NULL;
    Free(sShopEntries);
    sShopEntries = NULL;
    ClearStdWindowAndFrameToTransparent(task->tWindowId, TRUE);
    RemoveWindow(task->tWindowId);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_MonShopMenu(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    ListMenuGetScrollAndRow(gTasks[taskId].tListTaskId, &sScrollOffset, NULL);
    switch (input)
    {
    case LIST_NOTHING_CHOSEN:
        break;
    case LIST_CANCEL:
        gSpecialVar_Result = MULTI_B_PRESSED;
        PlaySE(SE_SELECT);
        CloseMonShopMenu(taskId);
        break;
    default:
        gSpecialVar_Result = sShopEntries[input].id;
        PlaySE(SE_SELECT);
        CloseMonShopMenu(taskId);
        break;
    }
}

// Special: VAR_0x8004 = entry; buffers STR_VAR_1 = species name,
// STR_VAR_2 = price and STR_VAR_3 = level for the confirmation message
void BufferGiftMonInfo(void)
{
    const struct GiftMon *giftMon = &sGiftMons[gSpecialVar_0x8004];

    StringCopy(gStringVar1, gSpeciesNames[giftMon->species]);
    ConvertIntToDecimalStringN(gStringVar2, giftMon->price, STR_CONV_MODE_LEFT_ALIGN, 6);
    ConvertIntToDecimalStringN(gStringVar3, giftMon->level, STR_CONV_MODE_LEFT_ALIGN, 3);
}

// Special: VAR_0x8004 = entry; buys it. VAR_RESULT = GIFT_MON_* result
void TryBuyGiftMon(void)
{
    const struct GiftMon *giftMon = &sGiftMons[gSpecialVar_0x8004];

    // Event Pokémon, so Mew and Deoxys obey
    gSpecialVar_Result = BuyMonForPlayer(giftMon->species, giftMon->level, giftMon->price, TRUE);
}

// Charges the price and gives the player a new Pokémon. Returns a GIFT_MON_* result.
u16 BuyMonForPlayer(u16 species, u8 level, u32 price, bool8 fatefulEncounter)
{
    struct Pokemon mon;
    u16 result, dexNum;

    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, price))
        return GIFT_MON_NO_MONEY;

    // Shiny rolls apply, as for any Pokémon created for the player
    CreateMon(&mon, species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    if (fatefulEncounter)
        SetMonData(&mon, MON_DATA_MODERN_FATEFUL_ENCOUNTER, &fatefulEncounter);

    switch (GiveMonToPlayer(&mon))
    {
    case MON_GIVEN_TO_PARTY:
        result = GIFT_MON_BOUGHT_PARTY;
        break;
    case MON_GIVEN_TO_PC:
        result = GIFT_MON_BOUGHT_PC;
        break;
    default:
        return GIFT_MON_NO_ROOM;
    }

    RemoveMoney(&gSaveBlock1Ptr->money, price);
    dexNum = SpeciesToNationalPokedexNum(species);
    GetSetPokedexFlag(dexNum, FLAG_SET_SEEN);
    GetSetPokedexFlag(dexNum, FLAG_SET_CAUGHT);
    return result;
}
