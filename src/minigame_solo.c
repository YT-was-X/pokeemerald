// Starts the wireless minigames for solo play with computer partners
// (see include/minigame_solo.h). Called from the minigame NPC on the
// Pokémon Center 2F (data/scripts/pokemon_center_2f.inc).
#include "global.h"
#include "berry_crush.h"
#include "dodrio_berry_picking.h"
#include "event_data.h"
#include "link.h"
#include "minigame_solo.h"
#include "overworld.h"
#include "pokemon_jump.h"
#include "random.h"
#include "string_util.h"
#include "constants/global.h"

EWRAM_DATA u8 gSoloMinigamePlayers = 0;

static const u8 sCpuNames[][PLAYER_NAME_LENGTH + 1] =
{
    _("ZOE"),
    _("KAI"),
    _("MAYA"),
    _("LEO"),
    _("NOVA"),
    _("FINN"),
    _("IVY"),
    _("REX"),
};

// Fills gLinkPlayers with the player and numPlayers - 1 computer players
static void SetUpSoloPlayers(u8 numPlayers)
{
    u32 i, nameOffset = Random() % ARRAY_COUNT(sCpuNames);

    gSoloMinigamePlayers = numPlayers;
    for (i = 0; i < numPlayers; i++)
    {
        struct LinkPlayer *player = &gLinkPlayers[i];

        if (i == 0)
        {
            StringCopy(player->name, gSaveBlock2Ptr->playerName);
            player->gender = gSaveBlock2Ptr->playerGender;
            player->trainerId = gSaveBlock2Ptr->playerTrainerId[0]
                              | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
                              | (gSaveBlock2Ptr->playerTrainerId[2] << 16)
                              | (gSaveBlock2Ptr->playerTrainerId[3] << 24);
        }
        else
        {
            StringCopy(player->name, sCpuNames[(nameOffset + i) % ARRAY_COUNT(sCpuNames)]);
            player->gender = (nameOffset + i) % 2;
            player->trainerId = Random32();
        }
        player->language = GAME_LANGUAGE;
        player->version = VERSION_EMERALD;
    }
}

void EndSoloMinigame(void)
{
    gSoloMinigamePlayers = 0;
}

// Specials. VAR_0x8004 = party slot of the chosen Pokémon (Pokémon Jump, Dodrio).
// The script continues once the game ends.
void PlaySoloPokemonJump(void)
{
    SetUpSoloPlayers(MAX_RFU_PLAYERS);
    StartPokemonJump(gSpecialVar_0x8004, CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

void PlaySoloDodrioBerryPicking(void)
{
    SetUpSoloPlayers(MAX_RFU_PLAYERS);
    StartDodrioBerryPicking(gSpecialVar_0x8004, CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

void PlaySoloBerryCrush(void)
{
    SetUpSoloPlayers(MAX_RFU_PLAYERS);
    StartBerryCrush(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}
