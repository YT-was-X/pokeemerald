#ifndef GUARD_CONSTANTS_GAME_CORNER_PRIZES_H
#define GUARD_CONSTANTS_GAME_CORNER_PRIZES_H

// Prize categories (VAR_0x8004 for the Game Corner prize specials)
#define PRIZE_CATEGORY_TMS        0
#define PRIZE_CATEGORY_ITEMS      1
#define PRIZE_CATEGORY_EVO_ITEMS  2
#define PRIZE_CATEGORY_POKEMON    3
#define NUM_PRIZE_CATEGORIES      4

// Results of the TryBuyGameCornerPrize special
#define PRIZE_BOUGHT_ITEM       0
#define PRIZE_BOUGHT_MON_PARTY  1
#define PRIZE_BOUGHT_MON_PC     2
#define PRIZE_NOT_ENOUGH_COINS  3
#define PRIZE_BAG_FULL          4
#define PRIZE_NO_ROOM_FOR_MON   5

#endif // GUARD_CONSTANTS_GAME_CORNER_PRIZES_H
