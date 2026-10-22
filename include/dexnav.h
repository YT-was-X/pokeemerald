#ifndef GUARD_DEXNAV_H
#define GUARD_DEXNAV_H

// The species the current wild battle was found with the DexNav (SPECIES_NONE otherwise)
extern u16 gDexNavSpecies;

bool8 CanUseDexNav(void);
void CB2_OpenDexNav(void);
bool8 IsDexNavSearching(void);
bool8 TryUseDexNavShortcut(void);
bool8 UpdateDexNavSearch(void);
void EndDexNavSearch(void);
void DexNavOnMapChange(void);
void DexNavOnBattleEnd(void);
u32 GetDexNavShinyRolls(void);

#endif // GUARD_DEXNAV_H
