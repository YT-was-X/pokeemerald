@ DexNav (src/dexnav.c)
EventScript_StartDexNavBattle::
	lockall
	playse SE_PIN
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	waitse
	dowildbattle
	releaseall
	end

EventScript_DexNavNotFoundNearby::
	msgbox DexNav_Text_NotFoundNearby, MSGBOX_SIGN
	end

EventScript_DexNavMovedTooFast::
	msgbox DexNav_Text_MovedTooFast, MSGBOX_SIGN
	end

EventScript_DexNavGotAway::
	msgbox DexNav_Text_GotAway, MSGBOX_SIGN
	end

EventScript_DexNavLostSignal::
	msgbox DexNav_Text_LostSignal, MSGBOX_SIGN
	end

EventScript_DexNavTooDark::
	msgbox DexNav_Text_TooDark, MSGBOX_SIGN
	end

DexNav_Text_NotFoundNearby:
	.string "It couldn't be found nearby.\n"
	.string "Try looking in a different area!$"

DexNav_Text_MovedTooFast:
	.string "The POKéMON got away!\p"
	.string "Hold {B_BUTTON} to walk slowly when\n"
	.string "you get close.$"

DexNav_Text_GotAway:
	.string "The POKéMON got away!$"

DexNav_Text_LostSignal:
	.string "There is no reaction.\n"
	.string "The signal was lost!$"

DexNav_Text_TooDark:
	.string "It's too dark to search\n"
	.string "for a POKéMON!$"
