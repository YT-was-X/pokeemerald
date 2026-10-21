@ Poké Center 2F services: Day Care counter (left), gift terminal (middle), black market grunt (right)

@ Day Care counter: the same Day Care as Route 117, without the trip
@ Day Care lady: leave and pick up POKéMON (Route 117 Day Care inside)
PokemonCenter_2F_EventScript_DaycareLady::
	setvar VAR_TEMP_F, DAYCARE_COUNTER_REMOTE
	lock
	faceplayer
	msgbox PokemonCenter_2F_Text_DaycareCounterWelcome, MSGBOX_DEFAULT
	special GetDaycareMonNicknames
	goto Route117_PokemonDayCare_EventScript_DaycareWoman
	end

@ Day Care man: EGGS and how the POKéMON are doing (Route 117 Day Care outside)
PokemonCenter_2F_EventScript_DaycareMan::
	goto Route117_EventScript_DaycareMan
	end

@ Pokémon Training Camp coach (sits on the left stool by the bottom wall): trains one
@ Pokémon at a time (EXP and optional EVs, src/training_camp.c); never makes Eggs
PokemonCenter_2F_EventScript_TrainingCamp::
	lock
	faceplayer
	specialvar VAR_RESULT, GetTrainingCampState
	goto_if_eq VAR_RESULT, TRUE, PokemonCenter_2F_EventScript_CampOccupied
	msgbox PokemonCenter_2F_Text_CampWelcome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_2F_EventScript_CampComeAgain
	specialvar VAR_RESULT, CountPartyNonEggMons
	goto_if_eq VAR_RESULT, 1, PokemonCenter_2F_EventScript_CampOnlyOneMon
	msgbox PokemonCenter_2F_Text_CampWhichMon, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	special ChooseSendDaycareMon
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PokemonCenter_2F_EventScript_CampComeAgain
	specialvar VAR_RESULT, CountPartyAliveNonEggMons_IgnoreVar0x8004Slot
	goto_if_eq VAR_RESULT, 0, PokemonCenter_2F_EventScript_CampOnlyOneAliveMon
	specialvar VAR_RESULT, CheckTrainingCampMon
	goto_if_eq VAR_RESULT, TRAINING_CAMP_MON_HAS_MAIL, PokemonCenter_2F_EventScript_CampHasMail
	specialvar VAR_0x8006, GetSelectedMonNicknameAndSpecies
	setvar VAR_0x8005, TRAINING_CAMP_NO_EVS
	msgbox PokemonCenter_2F_Text_CampTrainEvs, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_2F_EventScript_CampStoreMon
	message PokemonCenter_2F_Text_CampWhichStat
	waitmessage
	multichoice 0, 0, MULTI_TRAINING_CAMP_STATS, FALSE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemonCenter_2F_EventScript_CampComeAgain
	copyvar VAR_0x8005, VAR_RESULT
PokemonCenter_2F_EventScript_CampStoreMon::
	waitse
	playmoncry VAR_0x8006, CRY_MODE_NORMAL
	msgbox PokemonCenter_2F_Text_CampLeaveItToMe, MSGBOX_DEFAULT
	waitmoncry
	special StoreMonInTrainingCamp
	release
	end

PokemonCenter_2F_EventScript_CampOccupied::
	specialvar VAR_RESULT, BufferTrainingCampInfo
	call_if_eq VAR_RESULT, TRUE, PokemonCenter_2F_EventScript_CampReportEvs
	call_if_eq VAR_RESULT, FALSE, PokemonCenter_2F_EventScript_CampReportLevels
	msgbox PokemonCenter_2F_Text_CampTakeItBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_2F_EventScript_CampComeAgain
	specialvar VAR_RESULT, CalculatePlayerPartyCount
	goto_if_eq VAR_RESULT, PARTY_SIZE, PokemonCenter_2F_EventScript_CampPartyFull
	special BufferTrainingCampFee
	showmoneybox 0, 0
	msgbox PokemonCenter_2F_Text_CampFee, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_2F_EventScript_CampComeAgainHideMoney
	specialvar VAR_RESULT, IsEnoughForCostInVar0x8005
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_2F_EventScript_CampNotEnoughMoney
	specialvar VAR_RESULT, TakeMonFromTrainingCamp
	special SubtractMoneyFromVar0x8005
	updatemoneybox
	playse SE_SHOP
	msgbox PokemonCenter_2F_Text_CampHeresYourMon, MSGBOX_DEFAULT
	waitse
	hidemoneybox
	playmoncry VAR_RESULT, CRY_MODE_NORMAL
	msgbox PokemonCenter_2F_Text_CampTookBackMon, MSGBOX_DEFAULT
	waitmoncry
	release
	end

PokemonCenter_2F_EventScript_CampReportEvs::
	msgbox PokemonCenter_2F_Text_CampReportEvs, MSGBOX_DEFAULT
	return

PokemonCenter_2F_EventScript_CampReportLevels::
	msgbox PokemonCenter_2F_Text_CampReportLevels, MSGBOX_DEFAULT
	return

PokemonCenter_2F_EventScript_CampOnlyOneMon::
	msgbox PokemonCenter_2F_Text_CampOnlyOneMon, MSGBOX_DEFAULT
	release
	end

PokemonCenter_2F_EventScript_CampOnlyOneAliveMon::
	msgbox PokemonCenter_2F_Text_CampOnlyOneAliveMon, MSGBOX_DEFAULT
	release
	end

PokemonCenter_2F_EventScript_CampHasMail::
	msgbox PokemonCenter_2F_Text_CampHasMail, MSGBOX_DEFAULT
	release
	end

PokemonCenter_2F_EventScript_CampPartyFull::
	msgbox PokemonCenter_2F_Text_CampPartyFull, MSGBOX_DEFAULT
	release
	end

PokemonCenter_2F_EventScript_CampNotEnoughMoney::
	msgbox PokemonCenter_2F_Text_CampNotEnoughMoney, MSGBOX_DEFAULT
	hidemoneybox
	release
	end

PokemonCenter_2F_EventScript_CampComeAgainHideMoney::
	hidemoneybox
PokemonCenter_2F_EventScript_CampComeAgain::
	msgbox PokemonCenter_2F_Text_CampComeAgain, MSGBOX_DEFAULT
	release
	end

@ Move Tutor (sits beside the camp coach): the Fallarbor Move Relearner's service
PokemonCenter_2F_EventScript_MoveRelearner::
	lock
	faceplayer
	msgbox PokemonCenter_2F_Text_MoveRelearnerIntro, MSGBOX_DEFAULT
	goto FallarborTown_MoveRelearnersHouse_EventScript_AskTeachMove
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
	goto_if_eq VAR_RESULT, TRUE, PokemonCenter_2F_EventScript_TerminalConfirmItem
	msgbox PokemonCenter_2F_Text_ConfirmBuy, MSGBOX_YESNO
PokemonCenter_2F_EventScript_TerminalConfirmed::
	goto_if_eq VAR_RESULT, NO, PokemonCenter_2F_EventScript_TerminalChoose
	special TryBuyGiftMon
	goto_if_eq VAR_RESULT, GIFT_MON_NO_MONEY, PokemonCenter_2F_EventScript_TerminalNoMoney
	goto_if_eq VAR_RESULT, GIFT_MON_NO_ROOM, PokemonCenter_2F_EventScript_TerminalNoRoom
	goto_if_eq VAR_RESULT, GIFT_ITEM_BAG_FULL, PokemonCenter_2F_EventScript_TerminalBagFull
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

PokemonCenter_2F_EventScript_TerminalConfirmItem::
	msgbox PokemonCenter_2F_Text_ConfirmBuyItem, MSGBOX_YESNO
	goto PokemonCenter_2F_EventScript_TerminalConfirmed
	end

PokemonCenter_2F_EventScript_TerminalBagFull::
	msgbox PokemonCenter_2F_Text_BagFull, MSGBOX_DEFAULT
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

@ Minigame club: plays the wireless minigames solo, with computer partners (src/minigame_solo.c)
PokemonCenter_2F_EventScript_MinigameHost::
	lock
	faceplayer
	msgbox PokemonCenter_2F_Text_MinigameWelcome, MSGBOX_DEFAULT
PokemonCenter_2F_EventScript_ChooseMinigame::
	message PokemonCenter_2F_Text_WhichMinigame
	waitmessage
	multichoice 0, 0, MULTI_SOLO_MINIGAMES, FALSE
	switch VAR_RESULT
	case 0, PokemonCenter_2F_EventScript_PlayPokemonJump
	case 1, PokemonCenter_2F_EventScript_PlayDodrioBerryPicking
	case 2, PokemonCenter_2F_EventScript_PlayBerryCrush
	goto PokemonCenter_2F_EventScript_MinigameGoodbye
	end

PokemonCenter_2F_EventScript_PlayPokemonJump::
	special IsPokemonJumpSpeciesInParty
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_2F_EventScript_NoPokemonJumpMon
	msgbox PokemonCenter_2F_Text_EnterWhichPokemon, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	setvar VAR_0x8005, 0
	special ChooseMonForWirelessMinigame
	goto_if_ge VAR_0x8004, PARTY_SIZE, PokemonCenter_2F_EventScript_ChooseMinigame
	fadescreen FADE_TO_BLACK
	special PlaySoloPokemonJump
	goto PokemonCenter_2F_EventScript_MinigameThanks
	end

PokemonCenter_2F_EventScript_PlayDodrioBerryPicking::
	special IsDodrioInParty
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_2F_EventScript_NoDodrio
	msgbox PokemonCenter_2F_Text_EnterWhichPokemon, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	setvar VAR_0x8005, 1
	special ChooseMonForWirelessMinigame
	goto_if_ge VAR_0x8004, PARTY_SIZE, PokemonCenter_2F_EventScript_ChooseMinigame
	fadescreen FADE_TO_BLACK
	special PlaySoloDodrioBerryPicking
	goto PokemonCenter_2F_EventScript_MinigameThanks
	end

PokemonCenter_2F_EventScript_PlayBerryCrush::
	specialvar VAR_RESULT, PlayerHasBerries
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_2F_EventScript_NoBerries
	msgbox PokemonCenter_2F_Text_LetsBerryCrush, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	special PlaySoloBerryCrush
	goto PokemonCenter_2F_EventScript_MinigameThanks
	end

PokemonCenter_2F_EventScript_NoPokemonJumpMon::
	msgbox PokemonCenter_2F_Text_NoPokemonJumpMon, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_ChooseMinigame
	end

PokemonCenter_2F_EventScript_NoDodrio::
	msgbox PokemonCenter_2F_Text_NoDodrio, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_ChooseMinigame
	end

PokemonCenter_2F_EventScript_NoBerries::
	msgbox PokemonCenter_2F_Text_NoBerries, MSGBOX_DEFAULT
	goto PokemonCenter_2F_EventScript_ChooseMinigame
	end

PokemonCenter_2F_EventScript_MinigameThanks::
	msgbox PokemonCenter_2F_Text_MinigameThanks, MSGBOX_DEFAULT
	release
	end

PokemonCenter_2F_EventScript_MinigameGoodbye::
	msgbox PokemonCenter_2F_Text_MinigameGoodbye, MSGBOX_DEFAULT
	release
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
	.string "Leave two POKéMON with the\n"
	.string "DAY-CARE LADY, and the DAY-CARE\l"
	.string "MAN will give you any EGGS.\p"
	.string "There's no need to go all the way\n"
	.string "to ROUTE 117.\p"
	.string "The middle counter is mine.\n"
	.string "It's the POKéMON GIFT TERMINAL.\p"
	.string "You may get rare POKéMON and\n"
	.string "FOSSILS here.\p"
	.string "Each GYM BADGE you earn unlocks\n"
	.string "more of them.\p"
	.string "DEVON CORPORATION in RUSTBORO\n"
	.string "can bring FOSSILS back to life.\p"
	.string "As for the counter on the right…\p"
	.string "Someone moved in and started\n"
	.string "buying and selling POKéMON.\p"
	.string "The POKéMON CENTER can't say\n"
	.string "where they come from…\p"
	.string "Please be careful over there.\p"
	.string "Oh, and the boy on the right\n"
	.string "always wants someone to play\l"
	.string "WIRELESS minigames with him.\p"
	.string "The coach on the stools by the\n"
	.string "wall runs a POKéMON TRAINING CAMP.\p"
	.string "The MOVE TUTOR beside him can\n"
	.string "teach moves for a HEART SCALE.$"

PokemonCenter_2F_Text_MinigameWelcome:
	.string "Hi! My friends and I play the\n"
	.string "WIRELESS minigames all day.\p"
	.string "We need one more player.\n"
	.string "Want to join us?$"

PokemonCenter_2F_Text_WhichMinigame:
	.string "Which game do you want to play?$"

PokemonCenter_2F_Text_EnterWhichPokemon:
	.string "Which POKéMON will you enter?$"

PokemonCenter_2F_Text_LetsBerryCrush:
	.string "BERRY CRUSH it is!\n"
	.string "Bring a BERRY to crush!$"

PokemonCenter_2F_Text_NoPokemonJumpMon:
	.string "POKéMON JUMP needs a small POKéMON\n"
	.string "that can jump, like PIKACHU or\l"
	.string "MARILL.$"

PokemonCenter_2F_Text_NoDodrio:
	.string "DODRIO BERRY-PICKING needs a\n"
	.string "DODRIO in your party.$"

PokemonCenter_2F_Text_NoBerries:
	.string "BERRY CRUSH needs a BERRY.\n"
	.string "Come back when you have one!$"

PokemonCenter_2F_Text_MinigameThanks:
	.string "That was fun!\n"
	.string "Let's play again sometime!$"

PokemonCenter_2F_Text_MinigameGoodbye:
	.string "Come play with us anytime!$"

PokemonCenter_2F_Text_DaycareCounterWelcome:
	.string "Hello! This counter is linked to the\n"
	.string "POKéMON DAY CARE on ROUTE 117.$"

PokemonCenter_2F_Text_TerminalWelcome:
	.string "Welcome to the POKéMON GIFT\n"
	.string "TERMINAL!\p"
	.string "Earn GYM BADGES to unlock more\n"
	.string "POKéMON and FOSSILS. Become\l"
	.string "CHAMPION to unlock the legendary\l"
	.string "POKéMON!$"

PokemonCenter_2F_Text_WhichPokemon:
	.string "Which POKéMON would you like?$"

PokemonCenter_2F_Text_ConfirmBuy:
	.string "A Lv. {STR_VAR_3} {STR_VAR_1} costs ¥{STR_VAR_2}.\n"
	.string "Would you like it?$"

PokemonCenter_2F_Text_ConfirmBuyItem:
	.string "The {STR_VAR_1} costs ¥{STR_VAR_2}.\n"
	.string "Would you like it?$"

PokemonCenter_2F_Text_BagFull:
	.string "Sorry, your BAG is full.$"

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

PokemonCenter_2F_Text_CampWelcome:
	.string "Hey there! I run the POKéMON\n"
	.string "TRAINING CAMP!\p"
	.string "I train one POKéMON at a time.\n"
	.string "It earns EXP. as you walk, and it\l"
	.string "can work on one stat's EVs, too.\p"
	.string "It's ¥100, plus ¥100 for each level\n"
	.string "and ¥5 for each EV it gains.\p"
	.string "Want me to train one of yours?$"

PokemonCenter_2F_Text_CampWhichMon:
	.string "Which POKéMON should I train?$"

PokemonCenter_2F_Text_CampTrainEvs:
	.string "Should {STR_VAR_1} train its\n"
	.string "EVs, too?$"

PokemonCenter_2F_Text_CampWhichStat:
	.string "Which stat should it train?$"

PokemonCenter_2F_Text_CampLeaveItToMe:
	.string "Leave {STR_VAR_1} to me!\n"
	.string "I'll whip it into shape!$"

PokemonCenter_2F_Text_CampReportEvs:
	.string "Your {STR_VAR_1}'s training hard!\p"
	.string "It's grown by {STR_VAR_2} level(s) and\n"
	.string "gained {STR_VAR_3} EVs.$"

PokemonCenter_2F_Text_CampReportLevels:
	.string "Your {STR_VAR_1}'s training hard!\p"
	.string "It's grown by {STR_VAR_2} level(s).$"

PokemonCenter_2F_Text_CampTakeItBack:
	.string "Do you want to take it back?$"

PokemonCenter_2F_Text_CampFee:
	.string "That'll be ¥{STR_VAR_2} for\n"
	.string "{STR_VAR_1}'s training. OK?$"

PokemonCenter_2F_Text_CampHeresYourMon:
	.string "Thanks!\n"
	.string "Here's your POKéMON.$"

PokemonCenter_2F_Text_CampTookBackMon:
	.string "{PLAYER} took back {STR_VAR_1}\n"
	.string "from the TRAINING CAMP.$"

PokemonCenter_2F_Text_CampOnlyOneMon:
	.string "You've only got one POKéMON!\n"
	.string "Who would you travel with?$"

PokemonCenter_2F_Text_CampOnlyOneAliveMon:
	.string "Then you'd have no POKéMON that\n"
	.string "can battle!$"

PokemonCenter_2F_Text_CampHasMail:
	.string "Please take the MAIL off it first.$"

PokemonCenter_2F_Text_CampPartyFull:
	.string "Your team's full.\n"
	.string "Make room, then come see me.$"

PokemonCenter_2F_Text_CampNotEnoughMoney:
	.string "You don't have enough money…$"

PokemonCenter_2F_Text_CampComeAgain:
	.string "Come back anytime!$"

PokemonCenter_2F_Text_MoveRelearnerIntro:
	.string "I'm the MOVE TUTOR.\p"
	.string "For a HEART SCALE, I'll teach a\n"
	.string "POKéMON a move it could've learned.$"
