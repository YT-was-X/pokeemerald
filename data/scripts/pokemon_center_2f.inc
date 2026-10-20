@ Poké Center 2F services (left and middle counters; the right one is still the link Direct Corner)

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
