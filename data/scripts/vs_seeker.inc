@ Run when the VS Seeker is used from the bag or the registered (Select) button
EventScript_UseVsSeeker::
	special UseVsSeeker
	goto_if_eq VAR_RESULT, VS_SEEKER_NOT_CHARGED, EventScript_VsSeekerNotCharged
	goto_if_eq VAR_RESULT, VS_SEEKER_NO_TRAINERS, EventScript_VsSeekerNoTrainers
	playse SE_PIN
	delay 48
	releaseall
	end

EventScript_VsSeekerNotCharged::
	msgbox Text_VsSeekerNotCharged, MSGBOX_DEFAULT
	releaseall
	end

EventScript_VsSeekerNoTrainers::
	msgbox Text_VsSeekerNoTrainers, MSGBOX_DEFAULT
	releaseall
	end

Text_VsSeekerNotCharged:
	.string "The VS SEEKER's battery isn't\n"
	.string "charged yet.\p"
	.string "It needs {STR_VAR_1} more steps.$"

Text_VsSeekerNoTrainers:
	.string "There are no TRAINERS nearby who\n"
	.string "want to battle again.$"
