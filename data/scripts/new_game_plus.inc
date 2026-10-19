@ New Game Plus: the bed in the player's room offers it once the player is Champion
PlayersHouse_2F_EventScript_Bed::
	lockall
	goto_if_unset FLAG_SYS_GAME_CLEAR, PlayersHouse_2F_EventScript_BedRelease
	msgbox PlayersHouse_2F_Text_StartNewGamePlus, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PlayersHouse_2F_EventScript_BedRelease
	msgbox PlayersHouse_2F_Text_NewGamePlusWarning, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PlayersHouse_2F_EventScript_BedRelease
	msgbox PlayersHouse_2F_Text_NewGamePlusSleep, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	special StartNewGamePlus
	waitstate
	end

PlayersHouse_2F_EventScript_BedRelease::
	releaseall
	end

PlayersHouse_2F_Text_StartNewGamePlus:
	.string "It's your bed. Rest here and begin\n"
	.string "a NEW GAME+?$"

PlayersHouse_2F_Text_NewGamePlusWarning:
	.string "Your adventure will start over.\p"
	.string "You'll keep your starter at Lv. 5,\n"
	.string "your money and your MASTER BALLS.\p"
	.string "Everything else will be gone once\n"
	.string "you save. Start a NEW GAME+?$"

PlayersHouse_2F_Text_NewGamePlusSleep:
	.string "{PLAYER} lay down and drifted\n"
	.string "off to sleep…$"
