#ifndef GUARD_MINIGAME_SOLO_H
#define GUARD_MINIGAME_SOLO_H

// Solo play of the wireless minigames (Pokémon Jump, Dodrio Berry Picking,
// Berry Crush): the player is the link leader and the other players are
// computer-controlled, simulated locally instead of over the link.
// Total number of players in a solo game, or 0 for normal link play.
extern u8 gSoloMinigamePlayers;

#define IsSoloMinigame() (gSoloMinigamePlayers != 0)

void EndSoloMinigame(void);

// Replaces "Communication standby…" in solo play
extern const u8 gText_SoloPleaseWait[];

#endif // GUARD_MINIGAME_SOLO_H
