@ Poké Center 2F services: Day Care counter (left), gift terminal (middle), black market grunt (right)

@ Day Care counter: the same Day Care as Route 117, without the trip
PokemonCenter_2F_EventScript_DaycareCounter::
	setvar VAR_TEMP_F, DAYCARE_COUNTER_REMOTE
	lock
	faceplayer
	msgbox PokemonCenter_2F_Text_DaycareCounterWelcome, MSGBOX_DEFAULT
	special GetDaycareMonNicknames
	specialvar VAR_RESULT, GetDaycareState
	goto_if_eq VAR_RESULT, DAYCARE_EGG_WAITING, Route117_EventScript_DaycareEggWaiting
	goto Route117_PokemonDayCare_EventScript_DaycareWoman
	end

@ Gift Pokémon terminal
PokemonCenter_2F_EventScript_GiftTerminal::
	lock
	faceplayer
	msgbox PokemonCenter_2F_Text_TerminalWelcome, MSGBOX_DEFAULT
	specialvar VAR_RESULT, CountUnlockedGiftMons
	goto_if_eq VAR_RESULT, 0, PokemonCenter_2F_EventScript_TerminalNothingYet
PokemonCenter_2F_EventScript_TerminalChoose::
	message PokemonCenter_2F_Text_WhichPokemon
	waitmessage
	special ShowGiftMonTerminalMenu
	waitstate
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemonCenter_2F_EventScript_TerminalGoodbye
	copyvar VAR_0x8004, VAR_RESULT
	special BufferGiftMonInfo
	msgbox PokemonCenter_2F_Text_ConfirmBuy, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_2F_EventScript_TerminalChoose
	special TryBuyGiftMon
	goto_if_eq VAR_RESULT, GIFT_MON_NO_MONEY, PokemonCenter_2F_EventScript_TerminalNoMoney
	goto_if_eq VAR_RESULT, GIFT_MON_NO_ROOM, PokemonCenter_2F_EventScript_TerminalNoRoom
	copyvar VAR_0x8005, VAR_RESULT
	playse SE_SHOP
	waitse
	message PokemonCenter_2F_Text_ReceivedMon
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	waitbuttonpress
	goto_if_eq VAR_0x8005, GIFT_MON_BOUGHT_PC, PokemonCenter_2F_EventScript_TerminalSentToPC
	goto PokemonCenter_2F_EventScript_TerminalChoose
	end

PokemonCenter_2F_EventScript_TerminalSentToPC::
	msgbox PokemonCenter_2F_Text_SentToPC, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_TerminalChoose
	end

PokemonCenter_2F_EventScript_TerminalNoMoney::
	msgbox PokemonCenter_2F_Text_NotEnoughMoney, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_TerminalChoose
	end

PokemonCenter_2F_EventScript_TerminalNoRoom::
	msgbox PokemonCenter_2F_Text_NoRoom, MSGBOX_DEFAULT
	release
	end

PokemonCenter_2F_EventScript_TerminalNothingYet::
	msgbox PokemonCenter_2F_Text_NothingYet, MSGBOX_DEFAULT
	release
	end

PokemonCenter_2F_EventScript_TerminalGoodbye::
	msgbox PokemonCenter_2F_Text_TerminalGoodbye, MSGBOX_DEFAULT
	release
	end

@ Black market: a Team Magma or Team Aqua grunt who buys and sells Pokémon
PokemonCenter_2F_EventScript_MagmaGrunt::
	lock
	faceplayer
	msgbox PokemonCenter_2F_Text_MagmaGruntIntro, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_GruntBuyOrSell
	end

PokemonCenter_2F_EventScript_AquaGrunt::
	lock
	faceplayer
	msgbox PokemonCenter_2F_Text_AquaGruntIntro, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_GruntBuyOrSell
	end

PokemonCenter_2F_EventScript_GruntBuyOrSell::
	message PokemonCenter_2F_Text_GruntBuyOrSell
	waitmessage
	multichoice 0, 0, MULTI_BUY_SELL_QUIT, FALSE
	switch VAR_RESULT
	case 0, PokemonCenter_2F_EventScript_GruntBuy
	case 1, PokemonCenter_2F_EventScript_GruntSell
	msgbox PokemonCenter_2F_Text_GruntGoodbye, MSGBOX_DEFAULT
	release
	end

PokemonCenter_2F_EventScript_GruntBuy::
	message PokemonCenter_2F_Text_GruntTodaysStock
	waitmessage
	special ShowBlackMarketMenu
	waitstate
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemonCenter_2F_EventScript_GruntBuyOrSell
	copyvar VAR_0x8004, VAR_RESULT
	special BufferBlackMarketMonInfo
	msgbox PokemonCenter_2F_Text_GruntConfirmBuy, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_2F_EventScript_GruntBuy
	special TryBuyBlackMarketMon
	goto_if_eq VAR_RESULT, GIFT_MON_NO_MONEY, PokemonCenter_2F_EventScript_GruntNoMoney
	goto_if_eq VAR_RESULT, GIFT_MON_NO_ROOM, PokemonCenter_2F_EventScript_GruntNoRoom
	copyvar VAR_0x8005, VAR_RESULT
	playse SE_SHOP
	waitse
	message PokemonCenter_2F_Text_ReceivedMon
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	waitbuttonpress
	call_if_eq VAR_0x8005, GIFT_MON_BOUGHT_PC, PokemonCenter_2F_EventScript_GruntSentToPC
	goto PokemonCenter_2F_EventScript_GruntBuy
	end

PokemonCenter_2F_EventScript_GruntSentToPC::
	msgbox PokemonCenter_2F_Text_SentToPC, MSGBOX_DEFAULT
	return

PokemonCenter_2F_EventScript_GruntNoMoney::
	msgbox PokemonCenter_2F_Text_GruntNoMoney, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_GruntBuy
	end

PokemonCenter_2F_EventScript_GruntNoRoom::
	msgbox PokemonCenter_2F_Text_GruntNoRoom, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_GruntBuyOrSell
	end

PokemonCenter_2F_EventScript_GruntSell::
	msgbox PokemonCenter_2F_Text_GruntWhichToSell, MSGBOX_DEFAULT
	special ChoosePartyMon
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PokemonCenter_2F_EventScript_GruntBuyOrSell
	specialvar VAR_RESULT, CheckBlackMarketSell
	switch VAR_RESULT
	case BLACK_MARKET_SELL_EGG, PokemonCenter_2F_EventScript_GruntCantSellEgg
	case BLACK_MARKET_SELL_LAST_MON, PokemonCenter_2F_EventScript_GruntCantSellLastMon
	case BLACK_MARKET_SELL_MAIL, PokemonCenter_2F_EventScript_GruntCantSellMail
	case BLACK_MARKET_SELL_BAG_FULL, PokemonCenter_2F_EventScript_GruntCantSellBagFull
	msgbox PokemonCenter_2F_Text_GruntOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_2F_EventScript_GruntSell
	specialvar VAR_RESULT, SellBlackMarketMon
	playse SE_SHOP
	waitse
	call_if_eq VAR_RESULT, TRUE, PokemonCenter_2F_EventScript_GruntReturnedItem
	msgbox PokemonCenter_2F_Text_GruntSold, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_GruntSell
	end

PokemonCenter_2F_EventScript_GruntReturnedItem::
	msgbox PokemonCenter_2F_Text_GruntReturnedItem, MSGBOX_DEFAULT
	return

PokemonCenter_2F_EventScript_GruntCantSellEgg::
	msgbox PokemonCenter_2F_Text_GruntCantSellEgg, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_GruntSell
	end

PokemonCenter_2F_EventScript_GruntCantSellLastMon::
	msgbox PokemonCenter_2F_Text_GruntCantSellLastMon, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_GruntSell
	end

PokemonCenter_2F_EventScript_GruntCantSellMail::
	msgbox PokemonCenter_2F_Text_GruntCantSellMail, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_GruntSell
	end

PokemonCenter_2F_EventScript_GruntCantSellBagFull::
	msgbox PokemonCenter_2F_Text_GruntCantSellBagFull, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_GruntSell
	end

@ First-visit tour (CableClub_EventScript_Tutorial), given by TEALA at the gift terminal
PokemonCenter_2F_Text_TourIntro:
	.string "Hello!\n"
	.string "My name is TEALA.\p"
	.string "This must be your first time\n"
	.string "up here.\p"
	.string "Let me show you what this floor\n"
	.string "of our POKéMON CENTER offers.\p"
	.string "Right this way, please.$"

PokemonCenter_2F_Text_TourCounters:
	.string "There are three counters on this\n"
	.string "floor.\p"
	.string "The counter on the left is linked\n"
	.string "to the POKéMON DAY CARE on\l"
	.string "ROUTE 117.\p"
	.string "You may leave POKéMON there and\n"
	.string "pick up EGGS without going all\l"
	.string "the way to ROUTE 117.\p"
	.string "The middle counter is mine.\n"
	.string "It's the POKéMON GIFT TERMINAL.\p"
	.string "You may get rare POKéMON here.\n"
	.string "Each GYM BADGE you earn unlocks\l"
	.string "more of them.\p"
	.string "As for the counter on the right…\p"
	.string "Someone moved in and started\n"
	.string "buying and selling POKéMON.\p"
	.string "The POKéMON CENTER can't say\n"
	.string "where they come from…\p"
	.string "Please be careful over there.$"

PokemonCenter_2F_Text_DaycareCounterWelcome:
	.string "Hello! This counter is linked to the\n"
	.string "POKéMON DAY CARE on ROUTE 117.$"

PokemonCenter_2F_Text_TerminalWelcome:
	.string "Welcome to the POKéMON GIFT\n"
	.string "TERMINAL!\p"
	.string "Earn GYM BADGES to unlock more\n"
	.string "POKéMON. Become CHAMPION to\l"
	.string "unlock the legendary POKéMON!$"

PokemonCenter_2F_Text_WhichPokemon:
	.string "Which POKéMON would you like?$"

PokemonCenter_2F_Text_ConfirmBuy:
	.string "A Lv. {STR_VAR_3} {STR_VAR_1} costs ¥{STR_VAR_2}.\n"
	.string "Would you like it?$"

PokemonCenter_2F_Text_ReceivedMon:
	.string "{PLAYER} received {STR_VAR_1}!$"

PokemonCenter_2F_Text_SentToPC:
	.string "Your party is full, so {STR_VAR_1}\n"
	.string "was sent to your PC.$"

PokemonCenter_2F_Text_NotEnoughMoney:
	.string "Sorry, you don't have enough money.$"

PokemonCenter_2F_Text_NoRoom:
	.string "Sorry, your party and PC boxes are\n"
	.string "full. Please make some room first.$"

PokemonCenter_2F_Text_NothingYet:
	.string "No POKéMON are available yet.\n"
	.string "Please come back after earning\l"
	.string "your first GYM BADGE!$"

PokemonCenter_2F_Text_TerminalGoodbye:
	.string "Please come again!$"

PokemonCenter_2F_Text_MagmaGruntIntro:
	.string "Psst! TEAM MAGMA needs funds, and\n"
	.string "I need some cash on the side.\p"
	.string "I've got POKéMON from all over.\n"
	.string "No questions asked.$"

PokemonCenter_2F_Text_AquaGruntIntro:
	.string "Psst! TEAM AQUA's plans don't pay\n"
	.string "for themselves, you know.\p"
	.string "I deal in POKéMON from all over.\n"
	.string "No questions asked.$"

PokemonCenter_2F_Text_GruntBuyOrSell:
	.string "Buying or selling?$"

PokemonCenter_2F_Text_GruntGoodbye:
	.string "You never saw me. Got it?$"

PokemonCenter_2F_Text_GruntTodaysStock:
	.string "Here's what came in today.$"

PokemonCenter_2F_Text_GruntConfirmBuy:
	.string "A Lv. {STR_VAR_3} {STR_VAR_1}, ¥{STR_VAR_2}.\n"
	.string "Deal?$"

PokemonCenter_2F_Text_GruntNoMoney:
	.string "You're short on cash, kid.$"

PokemonCenter_2F_Text_GruntNoRoom:
	.string "Your party and PC boxes are full.\n"
	.string "Make some room first.$"

PokemonCenter_2F_Text_GruntWhichToSell:
	.string "Show me what you've got.$"

PokemonCenter_2F_Text_GruntOffer:
	.string "{STR_VAR_1}, huh? I'll give you\n"
	.string "¥{STR_VAR_2} for it. Deal?$"

PokemonCenter_2F_Text_GruntSold:
	.string "Pleasure doing business.$"

PokemonCenter_2F_Text_GruntReturnedItem:
	.string "{PLAYER} got back the held\n"
	.string "{STR_VAR_3}.$"

PokemonCenter_2F_Text_GruntCantSellEgg:
	.string "An EGG? I don't buy what I can't\n"
	.string "see.$"

PokemonCenter_2F_Text_GruntCantSellLastMon:
	.string "Then you'd have nothing left to\n"
	.string "battle with. No deal.$"

PokemonCenter_2F_Text_GruntCantSellMail:
	.string "Take the MAIL off it first.$"

PokemonCenter_2F_Text_GruntCantSellBagFull:
	.string "Your BAG can't fit its {STR_VAR_3}.\n"
	.string "Make some room first.$"
