#include "global.h"
#include "battle.h"
#include "battle_message.h"
#include "daycare.h"
#include "event_data.h"
#include "mail.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "string_util.h"
#include "training_camp.h"
#include "constants/characters.h"
#include "constants/training_camp.h"

// Pokémon Training Camp: holds one Pokémon that gains TRAINING_CAMP_EXP_PER_STEP EXP
// per step, and 1 EV in its chosen stat every TRAINING_CAMP_STEPS_PER_EV steps. The
// gains are worked out from the step count when the Pokémon is taken back, like the
// vanilla Day Care. It never produces Eggs.

#define sCamp (gPokemonStoragePtr->trainingCamp)

static bool8 IsTrainingCampOccupied(void)
{
    return GetBoxMonData(&sCamp.mon, MON_DATA_SANITY_HAS_SPECIES);
}

void UpdateTrainingCampStepCounter(void)
{
    if (IsTrainingCampOccupied() && sCamp.steps != UINT32_MAX)
        sCamp.steps++;
}

// VAR_RESULT: TRUE if a Pokémon is training
u16 GetTrainingCampState(void)
{
    return IsTrainingCampOccupied();
}

static u32 GetTrainingCampExp(void)
{
    u16 species = GetBoxMonData(&sCamp.mon, MON_DATA_SPECIES);
    u32 maxExp = gExperienceTables[gSpeciesInfo[species].growthRate][MAX_LEVEL];
    u32 exp = GetBoxMonData(&sCamp.mon, MON_DATA_EXP);
    u32 gain = sCamp.steps;

    if (gain > (maxExp - exp) / TRAINING_CAMP_EXP_PER_STEP)
        return maxExp;
    return exp + gain * TRAINING_CAMP_EXP_PER_STEP;
}

static u8 GetTrainingCampLevelsGained(void)
{
    struct BoxPokemon tempMon = sCamp.mon;
    u32 exp = GetTrainingCampExp();

    SetBoxMonData(&tempMon, MON_DATA_EXP, &exp);
    return GetLevelFromBoxMonExp(&tempMon) - GetLevelFromBoxMonExp(&sCamp.mon);
}

static u16 GetTrainingCampEvsGained(void)
{
    u32 i, total = 0, gain, statEv;

    if (sCamp.evStat >= NUM_STATS)
        return 0;

    for (i = 0; i < NUM_STATS; i++)
        total += GetBoxMonData(&sCamp.mon, MON_DATA_HP_EV + i);
    statEv = GetBoxMonData(&sCamp.mon, MON_DATA_HP_EV + sCamp.evStat);

    gain = sCamp.steps / TRAINING_CAMP_STEPS_PER_EV;
    if (statEv >= TRAINING_CAMP_EV_STAT_CAP || total >= MAX_TOTAL_EVS)
        return 0;
    if (gain > TRAINING_CAMP_EV_STAT_CAP - statEv)
        gain = TRAINING_CAMP_EV_STAT_CAP - statEv;
    if (gain > MAX_TOTAL_EVS - total)
        gain = MAX_TOTAL_EVS - total;
    return gain;
}

// VAR_0x8004 = party slot. VAR_RESULT: TRAINING_CAMP_MON_OK or TRAINING_CAMP_MON_HAS_MAIL
u16 CheckTrainingCampMon(void)
{
    if (MonHasMail(&gPlayerParty[gSpecialVar_0x8004]))
        return TRAINING_CAMP_MON_HAS_MAIL;
    return TRAINING_CAMP_MON_OK;
}

// VAR_0x8004 = party slot, VAR_0x8005 = stat to EV train (or TRAINING_CAMP_NO_EVS)
void StoreMonInTrainingCamp(void)
{
    struct Pokemon *mon = &gPlayerParty[gSpecialVar_0x8004];

    sCamp.mon = mon->box;
    BoxMonRestorePP(&sCamp.mon);
    sCamp.steps = 0;
    sCamp.evStat = (gSpecialVar_0x8005 < NUM_STATS) ? gSpecialVar_0x8005 : TRAINING_CAMP_NO_EVS;
    ZeroMonData(mon);
    CompactPartySlots();
    CalculatePlayerPartyCount();
}

// STR_VAR_1 = nickname, STR_VAR_2 = levels gained, STR_VAR_3 = EVs gained and the
// stat ("12 ATTACK"). VAR_0x8005 = fee. VAR_RESULT: TRUE if it trains EVs.
u16 BufferTrainingCampInfo(void)
{
    u8 levels = GetTrainingCampLevelsGained();
    u16 evs = GetTrainingCampEvsGained();
    u8 *end;

    GetBoxMonNickname(&sCamp.mon, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, levels, STR_CONV_MODE_LEFT_ALIGN, 3);
    end = ConvertIntToDecimalStringN(gStringVar3, evs, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (sCamp.evStat < NUM_STATS)
    {
        *end++ = CHAR_SPACE;
        StringCopy(end, gStatNamesTable[sCamp.evStat]);
    }
    gSpecialVar_0x8005 = TRAINING_CAMP_BASE_FEE + TRAINING_CAMP_FEE_PER_LEVEL * levels + TRAINING_CAMP_FEE_PER_EV * evs;
    return sCamp.evStat < NUM_STATS;
}

// STR_VAR_2 = fee in VAR_0x8005
void BufferTrainingCampFee(void)
{
    ConvertIntToDecimalStringN(gStringVar2, gSpecialVar_0x8005, STR_CONV_MODE_LEFT_ALIGN, 6);
}

// The party must have room. VAR_RESULT: species, for the cry.
u16 TakeMonFromTrainingCamp(void)
{
    struct Pokemon mon;
    u16 species = GetBoxMonData(&sCamp.mon, MON_DATA_SPECIES);
    u16 evs = GetTrainingCampEvsGained();
    u32 exp = GetTrainingCampExp();

    GetBoxMonNickname(&sCamp.mon, gStringVar1);
    BoxMonToMon(&sCamp.mon, &mon);

    if (evs != 0)
    {
        u8 ev = GetMonData(&mon, MON_DATA_HP_EV + sCamp.evStat) + evs;
        SetMonData(&mon, MON_DATA_HP_EV + sCamp.evStat, &ev);
    }
    if (GetMonData(&mon, MON_DATA_LEVEL) != MAX_LEVEL)
    {
        SetMonData(&mon, MON_DATA_EXP, &exp);
        ApplyDaycareExperience(&mon); // levels up and learns moves like the Day Care
    }
    CalculateMonStats(&mon);

    gPlayerParty[PARTY_SIZE - 1] = mon;
    ZeroBoxMonData(&sCamp.mon);
    sCamp.steps = 0;
    sCamp.evStat = 0;
    CompactPartySlots();
    CalculatePlayerPartyCount();
    return species;
}
