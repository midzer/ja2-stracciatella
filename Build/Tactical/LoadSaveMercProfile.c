#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveData.h"
#include "LoadSaveMercProfile.h"


void ExtractMercProfileUTF16(const BYTE* Src, MERCPROFILESTRUCT* Merc)
{
	const BYTE* S = Src;

#ifdef JA2DEMO
	EXTR_WSTR16(S, Merc->zName, lengthof(Merc->zName))
	EXTR_WSTR16(S, Merc->zNickname, lengthof(Merc->zNickname))
	EXTR_SKIP(S, 28)
	EXTR_U8(S, Merc->ubFaceIndex)
	EXTR_STR(S, Merc->PANTS, lengthof(Merc->PANTS))
	EXTR_STR(S, Merc->VEST, lengthof(Merc->VEST))
	EXTR_STR(S, Merc->SKIN, lengthof(Merc->SKIN))
	EXTR_STR(S, Merc->HAIR, lengthof(Merc->HAIR))
	EXTR_I8(S, Merc->bLife)
	EXTR_I8(S, Merc->bAgility)
	EXTR_I8(S, Merc->bLeadership)
	EXTR_I8(S, Merc->bDexterity)
	EXTR_I8(S, Merc->bStrength)
	EXTR_I8(S, Merc->bWisdom)
	EXTR_I8(S, Merc->bExpLevel)
	EXTR_I8(S, Merc->bMarksmanship)
	EXTR_I8(S, Merc->bMedical)
	EXTR_I8(S, Merc->bMechanical)
	EXTR_I8(S, Merc->bExplosive)
	EXTR_I8(S, Merc->bScientific)
	EXTR_SKIP(S, 1)
	EXTR_I16(S, Merc->sExpLevelGain)
	EXTR_I16(S, Merc->sLifeGain)
	EXTR_I16(S, Merc->sAgilityGain)
	EXTR_I16(S, Merc->sDexterityGain)
	EXTR_I16(S, Merc->sWisdomGain)
	EXTR_I16(S, Merc->sMarksmanshipGain)
	EXTR_I16(S, Merc->sMedicalGain)
	EXTR_I16(S, Merc->sMechanicGain)
	EXTR_I16(S, Merc->sExplosivesGain)
	EXTR_U8(S, Merc->ubBodyType)
	EXTR_I8(S, Merc->bVocalVolume)
	EXTR_U16(S, Merc->usEyesX)
	EXTR_U16(S, Merc->usEyesY)
	EXTR_U16(S, Merc->usMouthX)
	EXTR_U16(S, Merc->usMouthY)
	EXTR_SKIP(S, 2)
	EXTR_U32(S, Merc->uiEyeDelay)
	EXTR_U32(S, Merc->uiMouthDelay)
	EXTR_U32(S, Merc->uiBlinkFrequency)
	EXTR_U32(S, Merc->uiExpressionFrequency)
	EXTR_U16(S, Merc->sSectorX)
	EXTR_U16(S, Merc->sSectorY)
	EXTR_U32(S, Merc->uiDayBecomesAvailable)
	EXTR_U8(S, Merc->ubMiscFlags)
	EXTR_I8(S, Merc->bLifeMax)
	EXTR_I8(S, Merc->bExpLevelDelta)
	EXTR_I8(S, Merc->bLifeDelta)
	EXTR_I8(S, Merc->bAgilityDelta)
	EXTR_I8(S, Merc->bDexterityDelta)
	EXTR_I8(S, Merc->bWisdomDelta)
	EXTR_I8(S, Merc->bMarksmanshipDelta)
	EXTR_I8(S, Merc->bMedicalDelta)
	EXTR_I8(S, Merc->bMechanicDelta)
	EXTR_I8(S, Merc->bExplosivesDelta)
	EXTR_I8(S, Merc->bStrengthDelta)
	EXTR_I8(S, Merc->bLeadershipDelta)
	EXTR_SKIP(S, 1)
	EXTR_U16(S, Merc->usKills)
	EXTR_U16(S, Merc->usAssists)
	EXTR_U16(S, Merc->usShotsFired)
	EXTR_U16(S, Merc->usShotsHit)
	EXTR_U16(S, Merc->usBattlesFought)
	EXTR_U16(S, Merc->usTimesWounded)
	EXTR_U16(S, Merc->usTotalDaysServed)
	EXTR_I16(S, Merc->sLeadershipGain)
	EXTR_I16(S, Merc->sStrengthGain)
	EXTR_U32(S, Merc->uiBodyTypeSubFlags)
	EXTR_I16(S, Merc->sSalary)
	EXTR_I8(S, Merc->bSex)
	EXTR_I8(S, Merc->bEvolution)
	EXTR_I8(S, Merc->bPersonalityTrait)
	EXTR_I8(S, Merc->bSkillTrait)
	EXTR_I8(S, Merc->bReputationTolerance)
	EXTR_I8(S, Merc->bDeathRate)
	EXTR_I8(S, Merc->bSkillTrait2)
	EXTR_U8(S, Merc->ubMiscFlags2)
	EXTR_I8A(S, Merc->bBuddy, lengthof(Merc->bBuddy))
	EXTR_I8A(S, Merc->bHated, lengthof(Merc->bHated))
	EXTR_I8(S, Merc->bLearnToHate)
	EXTR_I8(S, Merc->bStealRate)
	EXTR_U8(S, Merc->bMinService)
	EXTR_U8(S, Merc->bSexist)
	EXTR_SKIP(S, 2)
	EXTR_U8A(S, Merc->bInvStatus, lengthof(Merc->bInvStatus))
	EXTR_U8A(S, Merc->bInvNumber, lengthof(Merc->bInvNumber))
	EXTR_U16A(S, Merc->usApproachFactor, lengthof(Merc->usApproachFactor))
	EXTR_I8(S, Merc->bMainGunAttractiveness)
	EXTR_I8(S, Merc->bArmourAttractiveness)
	EXTR_BOOL(S, Merc->fUseProfileInsertionInfo)
	EXTR_SKIP(S, 1)
	EXTR_I16(S, Merc->sGridNo)
	EXTR_U8(S, Merc->ubQuoteActionID) // XXX maybe should not get loaded, seems to contain garbage
	EXTR_U8(S, Merc->ubQuoteRecord) // XXX maybe should not get loaded, seems to contain garbage
	EXTR_U8(S, Merc->ubInvUndroppable)
	EXTR_U8A(S, Merc->ubRoomRangeStart, lengthof(Merc->ubRoomRangeStart)) // XXX maybe should not get loaded, seems to contain garbage
	EXTR_SKIP(S, 1)
	EXTR_U16A(S, Merc->inv, lengthof(Merc->inv))
	EXTR_I8A(S, Merc->bMercTownReputation, lengthof(Merc->bMercTownReputation))
	EXTR_U16A(S, Merc->usStatChangeChances, lengthof(Merc->usStatChangeChances))
	EXTR_U16A(S, Merc->usStatChangeSuccesses, lengthof(Merc->usStatChangeSuccesses))
	EXTR_U8(S, Merc->ubStrategicInsertionCode)
	EXTR_U8A(S, Merc->ubRoomRangeEnd, lengthof(Merc->ubRoomRangeEnd))
	EXTR_SKIP(S, 4)
	EXTR_U8(S, Merc->ubLastQuoteSaid)
	EXTR_I8(S, Merc->bRace)
	EXTR_I8(S, Merc->bNationality)
	EXTR_I8(S, Merc->bAppearance)
	EXTR_I8(S, Merc->bAppearanceCareLevel)
	EXTR_I8(S, Merc->bRefinement)
	EXTR_I8(S, Merc->bRefinementCareLevel)
	EXTR_I8(S, Merc->bHatedNationality)
	EXTR_I8(S, Merc->bHatedNationalityCareLevel)
	EXTR_I8(S, Merc->bRacist)
	EXTR_SKIP(S, 1)
	EXTR_U32(S, Merc->uiWeeklySalary)
	EXTR_U32(S, Merc->uiBiWeeklySalary)
	EXTR_I8(S, Merc->bMedicalDeposit)
	EXTR_I8(S, Merc->bAttitude)
	EXTR_I8(S, Merc->bBaseMorale)
	EXTR_SKIP(S, 1)
	EXTR_U16(S, Merc->sMedicalDepositAmount)
	EXTR_I8(S, Merc->bLearnToLike)
	EXTR_U8A(S, Merc->ubApproachVal, lengthof(Merc->ubApproachVal))
	EXTR_U8A(S, *Merc->ubApproachMod, sizeof(Merc->ubApproachMod) / sizeof(**Merc->ubApproachMod))
	EXTR_I8(S, Merc->bTown)
	EXTR_I8(S, Merc->bTownAttachment)
	EXTR_SKIP(S, 1)
	EXTR_U16(S, Merc->usOptionalGearCost)
	EXTR_I8A(S, Merc->bMercOpinion, lengthof(Merc->bMercOpinion))
	EXTR_I8(S, Merc->bApproached)
	EXTR_I8(S, Merc->bMercStatus)
	EXTR_I8A(S, Merc->bHatedTime, lengthof(Merc->bHatedTime))
	EXTR_I8(S, Merc->bLearnToLikeTime)
	EXTR_I8(S, Merc->bLearnToHateTime)
	EXTR_I8A(S, Merc->bHatedCount, lengthof(Merc->bHatedCount))
	EXTR_I8(S, Merc->bLearnToLikeCount)
	EXTR_I8(S, Merc->bLearnToHateCount)
	EXTR_U8(S, Merc->ubLastDateSpokenTo)
	EXTR_U8(S, Merc->bLastQuoteSaidWasSpecial)
	EXTR_I8(S, Merc->bSectorZ)
	EXTR_U16(S, Merc->usStrategicInsertionData)
	EXTR_I8(S, Merc->bFriendlyOrDirectDefaultResponseUsedRecently)
	EXTR_I8(S, Merc->bRecruitDefaultResponseUsedRecently)
	EXTR_I8(S, Merc->bThreatenDefaultResponseUsedRecently)
	EXTR_I8(S, Merc->bNPCData)
	EXTR_I32(S, Merc->iBalance) // XXX maybe should not get loaded, seems to contain garbage
	EXTR_I16(S, Merc->sTrueSalary)
	EXTR_U8(S, Merc->ubCivilianGroup)
	EXTR_U8(S, Merc->ubNeedForSleep)
	EXTR_U32(S, Merc->uiMoney)
	EXTR_I8(S, Merc->bNPCData2)
	EXTR_U8(S, Merc->ubMiscFlags3)
	EXTR_U8(S, Merc->ubDaysOfMoraleHangover)
	EXTR_U8(S, Merc->ubNumTimesDrugUseInLifetime)
	EXTR_U32(S, Merc->uiPrecedentQuoteSaid)
	Assert(S == Src + 696);

	Merc->uiProfileChecksum            = 0;
	Merc->sPreCombatGridNo             = 0;
	Merc->ubTimeTillNextHatedComplaint = 0;
	Merc->ubSuspiciousDeath            = 0;
	Merc->iMercMercContractLength      = 0;
	Merc->uiTotalCostToDate            = 0;

#else

	EXTR_WSTR16(S, Merc->zName, lengthof(Merc->zName))
	EXTR_WSTR16(S, Merc->zNickname, lengthof(Merc->zNickname))
	EXTR_SKIP(S, 28)
	EXTR_U8(S, Merc->ubFaceIndex)
	EXTR_STR(S, Merc->PANTS, lengthof(Merc->PANTS))
	EXTR_STR(S, Merc->VEST, lengthof(Merc->VEST))
	EXTR_STR(S, Merc->SKIN, lengthof(Merc->SKIN))
	EXTR_STR(S, Merc->HAIR, lengthof(Merc->HAIR))
	EXTR_I8(S, Merc->bSex)
	EXTR_I8(S, Merc->bArmourAttractiveness)
	EXTR_U8(S, Merc->ubMiscFlags2)
	EXTR_I8(S, Merc->bEvolution)
	EXTR_U8(S, Merc->ubMiscFlags)
	EXTR_U8(S, Merc->bSexist)
	EXTR_I8(S, Merc->bLearnToHate)
	EXTR_I8(S, Merc->bStealRate)
	EXTR_I8(S, Merc->bVocalVolume)
	EXTR_U8(S, Merc->ubQuoteRecord)
	EXTR_I8(S, Merc->bDeathRate)
	EXTR_I8(S, Merc->bScientific)
	EXTR_SKIP(S, 1)
	EXTR_I16(S, Merc->sExpLevelGain)
	EXTR_I16(S, Merc->sLifeGain)
	EXTR_I16(S, Merc->sAgilityGain)
	EXTR_I16(S, Merc->sDexterityGain)
	EXTR_I16(S, Merc->sWisdomGain)
	EXTR_I16(S, Merc->sMarksmanshipGain)
	EXTR_I16(S, Merc->sMedicalGain)
	EXTR_I16(S, Merc->sMechanicGain)
	EXTR_I16(S, Merc->sExplosivesGain)
	EXTR_U8(S, Merc->ubBodyType)
	EXTR_I8(S, Merc->bMedical)
	EXTR_U16(S, Merc->usEyesX)
	EXTR_U16(S, Merc->usEyesY)
	EXTR_U16(S, Merc->usMouthX)
	EXTR_U16(S, Merc->usMouthY)
	EXTR_SKIP(S, 2)
	EXTR_U32(S, Merc->uiEyeDelay)
	EXTR_U32(S, Merc->uiMouthDelay)
	EXTR_U32(S, Merc->uiBlinkFrequency)
	EXTR_U32(S, Merc->uiExpressionFrequency)
	EXTR_U16(S, Merc->sSectorX)
	EXTR_U16(S, Merc->sSectorY)
	EXTR_U32(S, Merc->uiDayBecomesAvailable)
	EXTR_I8(S, Merc->bStrength)
	EXTR_I8(S, Merc->bLifeMax)
	EXTR_I8(S, Merc->bExpLevelDelta)
	EXTR_I8(S, Merc->bLifeDelta)
	EXTR_I8(S, Merc->bAgilityDelta)
	EXTR_I8(S, Merc->bDexterityDelta)
	EXTR_I8(S, Merc->bWisdomDelta)
	EXTR_I8(S, Merc->bMarksmanshipDelta)
	EXTR_I8(S, Merc->bMedicalDelta)
	EXTR_I8(S, Merc->bMechanicDelta)
	EXTR_I8(S, Merc->bExplosivesDelta)
	EXTR_I8(S, Merc->bStrengthDelta)
	EXTR_I8(S, Merc->bLeadershipDelta)
	EXTR_SKIP(S, 1)
	EXTR_U16(S, Merc->usKills)
	EXTR_U16(S, Merc->usAssists)
	EXTR_U16(S, Merc->usShotsFired)
	EXTR_U16(S, Merc->usShotsHit)
	EXTR_U16(S, Merc->usBattlesFought)
	EXTR_U16(S, Merc->usTimesWounded)
	EXTR_U16(S, Merc->usTotalDaysServed)
	EXTR_I16(S, Merc->sLeadershipGain)
	EXTR_I16(S, Merc->sStrengthGain)
	EXTR_U32(S, Merc->uiBodyTypeSubFlags)
	EXTR_I16(S, Merc->sSalary)
	EXTR_I8(S, Merc->bLife)
	EXTR_I8(S, Merc->bDexterity)
	EXTR_I8(S, Merc->bPersonalityTrait)
	EXTR_I8(S, Merc->bSkillTrait)
	EXTR_I8(S, Merc->bReputationTolerance)
	EXTR_I8(S, Merc->bExplosive)
	EXTR_I8(S, Merc->bSkillTrait2)
	EXTR_I8(S, Merc->bLeadership)
	EXTR_I8A(S, Merc->bBuddy, lengthof(Merc->bBuddy))
	EXTR_I8A(S, Merc->bHated, lengthof(Merc->bHated))
	EXTR_I8(S, Merc->bExpLevel)
	EXTR_I8(S, Merc->bMarksmanship)
	EXTR_U8(S, Merc->bMinService)
	EXTR_I8(S, Merc->bWisdom)
	EXTR_SKIP(S, 2)
	EXTR_U8A(S, Merc->bInvStatus, lengthof(Merc->bInvStatus))
	EXTR_U8A(S, Merc->bInvNumber, lengthof(Merc->bInvNumber))
	EXTR_U16A(S, Merc->usApproachFactor, lengthof(Merc->usApproachFactor))
	EXTR_I8(S, Merc->bMainGunAttractiveness)
	EXTR_I8(S, Merc->bAgility)
	EXTR_BOOL(S, Merc->fUseProfileInsertionInfo)
	EXTR_SKIP(S, 1)
	EXTR_I16(S, Merc->sGridNo)
	EXTR_U8(S, Merc->ubQuoteActionID)
	EXTR_I8(S, Merc->bMechanical)
	EXTR_U8(S, Merc->ubInvUndroppable)
	EXTR_U8A(S, Merc->ubRoomRangeStart, lengthof(Merc->ubRoomRangeStart))
	EXTR_SKIP(S, 1)
	EXTR_U16A(S, Merc->inv, lengthof(Merc->inv))
	EXTR_I8A(S, Merc->bMercTownReputation, lengthof(Merc->bMercTownReputation))
	EXTR_U16A(S, Merc->usStatChangeChances, lengthof(Merc->usStatChangeChances))
	EXTR_U16A(S, Merc->usStatChangeSuccesses, lengthof(Merc->usStatChangeSuccesses))
	EXTR_U8(S, Merc->ubStrategicInsertionCode)
	EXTR_U8A(S, Merc->ubRoomRangeEnd, lengthof(Merc->ubRoomRangeEnd))
	EXTR_SKIP(S, 4)
	EXTR_U8(S, Merc->ubLastQuoteSaid)
	EXTR_I8(S, Merc->bRace)
	EXTR_I8(S, Merc->bNationality)
	EXTR_I8(S, Merc->bAppearance)
	EXTR_I8(S, Merc->bAppearanceCareLevel)
	EXTR_I8(S, Merc->bRefinement)
	EXTR_I8(S, Merc->bRefinementCareLevel)
	EXTR_I8(S, Merc->bHatedNationality)
	EXTR_I8(S, Merc->bHatedNationalityCareLevel)
	EXTR_I8(S, Merc->bRacist)
	EXTR_SKIP(S, 1)
	EXTR_U32(S, Merc->uiWeeklySalary)
	EXTR_U32(S, Merc->uiBiWeeklySalary)
	EXTR_I8(S, Merc->bMedicalDeposit)
	EXTR_I8(S, Merc->bAttitude)
	EXTR_I8(S, Merc->bBaseMorale)
	EXTR_SKIP(S, 1)
	EXTR_U16(S, Merc->sMedicalDepositAmount)
	EXTR_I8(S, Merc->bLearnToLike)
	EXTR_U8A(S, Merc->ubApproachVal, lengthof(Merc->ubApproachVal))
	EXTR_U8A(S, *Merc->ubApproachMod, sizeof(Merc->ubApproachMod) / sizeof(**Merc->ubApproachMod))
	EXTR_I8(S, Merc->bTown)
	EXTR_I8(S, Merc->bTownAttachment)
	EXTR_SKIP(S, 1)
	EXTR_U16(S, Merc->usOptionalGearCost)
	EXTR_I8A(S, Merc->bMercOpinion, lengthof(Merc->bMercOpinion))
	EXTR_I8(S, Merc->bApproached)
	EXTR_I8(S, Merc->bMercStatus)
	EXTR_I8A(S, Merc->bHatedTime, lengthof(Merc->bHatedTime))
	EXTR_I8(S, Merc->bLearnToLikeTime)
	EXTR_I8(S, Merc->bLearnToHateTime)
	EXTR_I8A(S, Merc->bHatedCount, lengthof(Merc->bHatedCount))
	EXTR_I8(S, Merc->bLearnToLikeCount)
	EXTR_I8(S, Merc->bLearnToHateCount)
	EXTR_U8(S, Merc->ubLastDateSpokenTo)
	EXTR_U8(S, Merc->bLastQuoteSaidWasSpecial)
	EXTR_I8(S, Merc->bSectorZ)
	EXTR_U16(S, Merc->usStrategicInsertionData)
	EXTR_I8(S, Merc->bFriendlyOrDirectDefaultResponseUsedRecently)
	EXTR_I8(S, Merc->bRecruitDefaultResponseUsedRecently)
	EXTR_I8(S, Merc->bThreatenDefaultResponseUsedRecently)
	EXTR_I8(S, Merc->bNPCData)
	EXTR_I32(S, Merc->iBalance)
	EXTR_I16(S, Merc->sTrueSalary)
	EXTR_U8(S, Merc->ubCivilianGroup)
	EXTR_U8(S, Merc->ubNeedForSleep)
	EXTR_U32(S, Merc->uiMoney)
	EXTR_I8(S, Merc->bNPCData2)
	EXTR_U8(S, Merc->ubMiscFlags3)
	EXTR_U8(S, Merc->ubDaysOfMoraleHangover)
	EXTR_U8(S, Merc->ubNumTimesDrugUseInLifetime)
	EXTR_U32(S, Merc->uiPrecedentQuoteSaid)
	EXTR_U32(S, Merc->uiProfileChecksum)
	EXTR_I16(S, Merc->sPreCombatGridNo)
	EXTR_U8(S, Merc->ubTimeTillNextHatedComplaint)
	EXTR_U8(S, Merc->ubSuspiciousDeath)
	EXTR_I32(S, Merc->iMercMercContractLength)
	EXTR_U32(S, Merc->uiTotalCostToDate)
	EXTR_SKIP(S, 4)
	Assert(S == Src + 716);
#endif
}


void ExtractMercProfile(const BYTE* Src, MERCPROFILESTRUCT* Merc)
{
	const BYTE* S = Src;

#ifdef _WIN32 // XXX HACK000A
	EXTR_WSTR16(S, Merc->zName, lengthof(Merc->zName))
	EXTR_WSTR16(S, Merc->zNickname, lengthof(Merc->zNickname))
#else
	EXTR_WSTR32(S, Merc->zName, lengthof(Merc->zName))
	EXTR_WSTR32(S, Merc->zNickname, lengthof(Merc->zNickname))
#endif
	EXTR_SKIP(S, 28)
	EXTR_U8(S, Merc->ubFaceIndex)
	EXTR_STR(S, Merc->PANTS, lengthof(Merc->PANTS))
	EXTR_STR(S, Merc->VEST, lengthof(Merc->VEST))
	EXTR_STR(S, Merc->SKIN, lengthof(Merc->SKIN))
	EXTR_STR(S, Merc->HAIR, lengthof(Merc->HAIR))
	EXTR_I8(S, Merc->bSex)
	EXTR_I8(S, Merc->bArmourAttractiveness)
	EXTR_U8(S, Merc->ubMiscFlags2)
	EXTR_I8(S, Merc->bEvolution)
	EXTR_U8(S, Merc->ubMiscFlags)
	EXTR_U8(S, Merc->bSexist)
	EXTR_I8(S, Merc->bLearnToHate)
	EXTR_I8(S, Merc->bStealRate)
	EXTR_I8(S, Merc->bVocalVolume)
	EXTR_U8(S, Merc->ubQuoteRecord)
	EXTR_I8(S, Merc->bDeathRate)
	EXTR_I8(S, Merc->bScientific)
	EXTR_SKIP(S, 1)
	EXTR_I16(S, Merc->sExpLevelGain)
	EXTR_I16(S, Merc->sLifeGain)
	EXTR_I16(S, Merc->sAgilityGain)
	EXTR_I16(S, Merc->sDexterityGain)
	EXTR_I16(S, Merc->sWisdomGain)
	EXTR_I16(S, Merc->sMarksmanshipGain)
	EXTR_I16(S, Merc->sMedicalGain)
	EXTR_I16(S, Merc->sMechanicGain)
	EXTR_I16(S, Merc->sExplosivesGain)
	EXTR_U8(S, Merc->ubBodyType)
	EXTR_I8(S, Merc->bMedical)
	EXTR_U16(S, Merc->usEyesX)
	EXTR_U16(S, Merc->usEyesY)
	EXTR_U16(S, Merc->usMouthX)
	EXTR_U16(S, Merc->usMouthY)
	EXTR_SKIP(S, 2)
	EXTR_U32(S, Merc->uiEyeDelay)
	EXTR_U32(S, Merc->uiMouthDelay)
	EXTR_U32(S, Merc->uiBlinkFrequency)
	EXTR_U32(S, Merc->uiExpressionFrequency)
	EXTR_U16(S, Merc->sSectorX)
	EXTR_U16(S, Merc->sSectorY)
	EXTR_U32(S, Merc->uiDayBecomesAvailable)
	EXTR_I8(S, Merc->bStrength)
	EXTR_I8(S, Merc->bLifeMax)
	EXTR_I8(S, Merc->bExpLevelDelta)
	EXTR_I8(S, Merc->bLifeDelta)
	EXTR_I8(S, Merc->bAgilityDelta)
	EXTR_I8(S, Merc->bDexterityDelta)
	EXTR_I8(S, Merc->bWisdomDelta)
	EXTR_I8(S, Merc->bMarksmanshipDelta)
	EXTR_I8(S, Merc->bMedicalDelta)
	EXTR_I8(S, Merc->bMechanicDelta)
	EXTR_I8(S, Merc->bExplosivesDelta)
	EXTR_I8(S, Merc->bStrengthDelta)
	EXTR_I8(S, Merc->bLeadershipDelta)
	EXTR_SKIP(S, 1)
	EXTR_U16(S, Merc->usKills)
	EXTR_U16(S, Merc->usAssists)
	EXTR_U16(S, Merc->usShotsFired)
	EXTR_U16(S, Merc->usShotsHit)
	EXTR_U16(S, Merc->usBattlesFought)
	EXTR_U16(S, Merc->usTimesWounded)
	EXTR_U16(S, Merc->usTotalDaysServed)
	EXTR_I16(S, Merc->sLeadershipGain)
	EXTR_I16(S, Merc->sStrengthGain)
	EXTR_U32(S, Merc->uiBodyTypeSubFlags)
	EXTR_I16(S, Merc->sSalary)
	EXTR_I8(S, Merc->bLife)
	EXTR_I8(S, Merc->bDexterity)
	EXTR_I8(S, Merc->bPersonalityTrait)
	EXTR_I8(S, Merc->bSkillTrait)
	EXTR_I8(S, Merc->bReputationTolerance)
	EXTR_I8(S, Merc->bExplosive)
	EXTR_I8(S, Merc->bSkillTrait2)
	EXTR_I8(S, Merc->bLeadership)
	EXTR_I8A(S, Merc->bBuddy, lengthof(Merc->bBuddy))
	EXTR_I8A(S, Merc->bHated, lengthof(Merc->bHated))
	EXTR_I8(S, Merc->bExpLevel)
	EXTR_I8(S, Merc->bMarksmanship)
	EXTR_U8(S, Merc->bMinService)
	EXTR_I8(S, Merc->bWisdom)
	EXTR_SKIP(S, 2)
	EXTR_U8A(S, Merc->bInvStatus, lengthof(Merc->bInvStatus))
	EXTR_U8A(S, Merc->bInvNumber, lengthof(Merc->bInvNumber))
	EXTR_U16A(S, Merc->usApproachFactor, lengthof(Merc->usApproachFactor))
	EXTR_I8(S, Merc->bMainGunAttractiveness)
	EXTR_I8(S, Merc->bAgility)
	EXTR_BOOL(S, Merc->fUseProfileInsertionInfo)
	EXTR_SKIP(S, 1)
	EXTR_I16(S, Merc->sGridNo)
	EXTR_U8(S, Merc->ubQuoteActionID)
	EXTR_I8(S, Merc->bMechanical)
	EXTR_U8(S, Merc->ubInvUndroppable)
	EXTR_U8A(S, Merc->ubRoomRangeStart, lengthof(Merc->ubRoomRangeStart))
	EXTR_SKIP(S, 1)
	EXTR_U16A(S, Merc->inv, lengthof(Merc->inv))
	EXTR_I8A(S, Merc->bMercTownReputation, lengthof(Merc->bMercTownReputation))
	EXTR_U16A(S, Merc->usStatChangeChances, lengthof(Merc->usStatChangeChances))
	EXTR_U16A(S, Merc->usStatChangeSuccesses, lengthof(Merc->usStatChangeSuccesses))
	EXTR_U8(S, Merc->ubStrategicInsertionCode)
	EXTR_U8A(S, Merc->ubRoomRangeEnd, lengthof(Merc->ubRoomRangeEnd))
	EXTR_SKIP(S, 4)
	EXTR_U8(S, Merc->ubLastQuoteSaid)
	EXTR_I8(S, Merc->bRace)
	EXTR_I8(S, Merc->bNationality)
	EXTR_I8(S, Merc->bAppearance)
	EXTR_I8(S, Merc->bAppearanceCareLevel)
	EXTR_I8(S, Merc->bRefinement)
	EXTR_I8(S, Merc->bRefinementCareLevel)
	EXTR_I8(S, Merc->bHatedNationality)
	EXTR_I8(S, Merc->bHatedNationalityCareLevel)
	EXTR_I8(S, Merc->bRacist)
	EXTR_SKIP(S, 1)
	EXTR_U32(S, Merc->uiWeeklySalary)
	EXTR_U32(S, Merc->uiBiWeeklySalary)
	EXTR_I8(S, Merc->bMedicalDeposit)
	EXTR_I8(S, Merc->bAttitude)
	EXTR_I8(S, Merc->bBaseMorale)
	EXTR_SKIP(S, 1)
	EXTR_U16(S, Merc->sMedicalDepositAmount)
	EXTR_I8(S, Merc->bLearnToLike)
	EXTR_U8A(S, Merc->ubApproachVal, lengthof(Merc->ubApproachVal))
	EXTR_U8A(S, *Merc->ubApproachMod, sizeof(Merc->ubApproachMod) / sizeof(**Merc->ubApproachMod))
	EXTR_I8(S, Merc->bTown)
	EXTR_I8(S, Merc->bTownAttachment)
	EXTR_SKIP(S, 1)
	EXTR_U16(S, Merc->usOptionalGearCost)
	EXTR_I8A(S, Merc->bMercOpinion, lengthof(Merc->bMercOpinion))
	EXTR_I8(S, Merc->bApproached)
	EXTR_I8(S, Merc->bMercStatus)
	EXTR_I8A(S, Merc->bHatedTime, lengthof(Merc->bHatedTime))
	EXTR_I8(S, Merc->bLearnToLikeTime)
	EXTR_I8(S, Merc->bLearnToHateTime)
	EXTR_I8A(S, Merc->bHatedCount, lengthof(Merc->bHatedCount))
	EXTR_I8(S, Merc->bLearnToLikeCount)
	EXTR_I8(S, Merc->bLearnToHateCount)
	EXTR_U8(S, Merc->ubLastDateSpokenTo)
	EXTR_U8(S, Merc->bLastQuoteSaidWasSpecial)
	EXTR_I8(S, Merc->bSectorZ)
	EXTR_U16(S, Merc->usStrategicInsertionData)
	EXTR_I8(S, Merc->bFriendlyOrDirectDefaultResponseUsedRecently)
	EXTR_I8(S, Merc->bRecruitDefaultResponseUsedRecently)
	EXTR_I8(S, Merc->bThreatenDefaultResponseUsedRecently)
	EXTR_I8(S, Merc->bNPCData)
	EXTR_I32(S, Merc->iBalance)
	EXTR_I16(S, Merc->sTrueSalary)
	EXTR_U8(S, Merc->ubCivilianGroup)
	EXTR_U8(S, Merc->ubNeedForSleep)
	EXTR_U32(S, Merc->uiMoney)
	EXTR_I8(S, Merc->bNPCData2)
	EXTR_U8(S, Merc->ubMiscFlags3)
	EXTR_U8(S, Merc->ubDaysOfMoraleHangover)
	EXTR_U8(S, Merc->ubNumTimesDrugUseInLifetime)
	EXTR_U32(S, Merc->uiPrecedentQuoteSaid)
	EXTR_U32(S, Merc->uiProfileChecksum)
	EXTR_I16(S, Merc->sPreCombatGridNo)
	EXTR_U8(S, Merc->ubTimeTillNextHatedComplaint)
	EXTR_U8(S, Merc->ubSuspiciousDeath)
	EXTR_I32(S, Merc->iMercMercContractLength)
	EXTR_U32(S, Merc->uiTotalCostToDate)
	EXTR_SKIP(S, 4)

#ifdef _WIN32 // XXX HACK000A
	Assert(S == Src + 716);
#else
	Assert(S == Src + 796);
#endif
}


BOOLEAN ExtractMercProfileFromFile(HWFILE File, MERCPROFILESTRUCT* Merc)
{
#ifdef _WIN32 // XXX HACK000A
	BYTE Data[716];
#else
	BYTE Data[796];
#endif
	BOOLEAN Ret = FileRead(File, Data, sizeof(Data));
	if (Ret) ExtractMercProfile(Data, Merc);
	return Ret;
}


void InjectMercProfile(BYTE* Dst, const MERCPROFILESTRUCT* Merc)
{
	BYTE* D = Dst;

#ifdef _WIN32 // XXX HACK000A
	INJ_WSTR16(D, Merc->zName, lengthof(Merc->zName))
	INJ_WSTR16(D, Merc->zNickname, lengthof(Merc->zNickname))
#else
	INJ_WSTR32(D, Merc->zName, lengthof(Merc->zName))
	INJ_WSTR32(D, Merc->zNickname, lengthof(Merc->zNickname))
#endif
	INJ_SKIP(D, 28)
	INJ_U8(D, Merc->ubFaceIndex)
	INJ_STR(D, Merc->PANTS, lengthof(Merc->PANTS))
	INJ_STR(D, Merc->VEST, lengthof(Merc->VEST))
	INJ_STR(D, Merc->SKIN, lengthof(Merc->SKIN))
	INJ_STR(D, Merc->HAIR, lengthof(Merc->HAIR))
	INJ_I8(D, Merc->bSex)
	INJ_I8(D, Merc->bArmourAttractiveness)
	INJ_U8(D, Merc->ubMiscFlags2)
	INJ_I8(D, Merc->bEvolution)
	INJ_U8(D, Merc->ubMiscFlags)
	INJ_U8(D, Merc->bSexist)
	INJ_I8(D, Merc->bLearnToHate)
	INJ_I8(D, Merc->bStealRate)
	INJ_I8(D, Merc->bVocalVolume)
	INJ_U8(D, Merc->ubQuoteRecord)
	INJ_I8(D, Merc->bDeathRate)
	INJ_I8(D, Merc->bScientific)
	INJ_SKIP(D, 1)
	INJ_I16(D, Merc->sExpLevelGain)
	INJ_I16(D, Merc->sLifeGain)
	INJ_I16(D, Merc->sAgilityGain)
	INJ_I16(D, Merc->sDexterityGain)
	INJ_I16(D, Merc->sWisdomGain)
	INJ_I16(D, Merc->sMarksmanshipGain)
	INJ_I16(D, Merc->sMedicalGain)
	INJ_I16(D, Merc->sMechanicGain)
	INJ_I16(D, Merc->sExplosivesGain)
	INJ_U8(D, Merc->ubBodyType)
	INJ_I8(D, Merc->bMedical)
	INJ_U16(D, Merc->usEyesX)
	INJ_U16(D, Merc->usEyesY)
	INJ_U16(D, Merc->usMouthX)
	INJ_U16(D, Merc->usMouthY)
	INJ_SKIP(D, 2)
	INJ_U32(D, Merc->uiEyeDelay)
	INJ_U32(D, Merc->uiMouthDelay)
	INJ_U32(D, Merc->uiBlinkFrequency)
	INJ_U32(D, Merc->uiExpressionFrequency)
	INJ_U16(D, Merc->sSectorX)
	INJ_U16(D, Merc->sSectorY)
	INJ_U32(D, Merc->uiDayBecomesAvailable)
	INJ_I8(D, Merc->bStrength)
	INJ_I8(D, Merc->bLifeMax)
	INJ_I8(D, Merc->bExpLevelDelta)
	INJ_I8(D, Merc->bLifeDelta)
	INJ_I8(D, Merc->bAgilityDelta)
	INJ_I8(D, Merc->bDexterityDelta)
	INJ_I8(D, Merc->bWisdomDelta)
	INJ_I8(D, Merc->bMarksmanshipDelta)
	INJ_I8(D, Merc->bMedicalDelta)
	INJ_I8(D, Merc->bMechanicDelta)
	INJ_I8(D, Merc->bExplosivesDelta)
	INJ_I8(D, Merc->bStrengthDelta)
	INJ_I8(D, Merc->bLeadershipDelta)
	INJ_SKIP(D, 1)
	INJ_U16(D, Merc->usKills)
	INJ_U16(D, Merc->usAssists)
	INJ_U16(D, Merc->usShotsFired)
	INJ_U16(D, Merc->usShotsHit)
	INJ_U16(D, Merc->usBattlesFought)
	INJ_U16(D, Merc->usTimesWounded)
	INJ_U16(D, Merc->usTotalDaysServed)
	INJ_I16(D, Merc->sLeadershipGain)
	INJ_I16(D, Merc->sStrengthGain)
	INJ_U32(D, Merc->uiBodyTypeSubFlags)
	INJ_I16(D, Merc->sSalary)
	INJ_I8(D, Merc->bLife)
	INJ_I8(D, Merc->bDexterity)
	INJ_I8(D, Merc->bPersonalityTrait)
	INJ_I8(D, Merc->bSkillTrait)
	INJ_I8(D, Merc->bReputationTolerance)
	INJ_I8(D, Merc->bExplosive)
	INJ_I8(D, Merc->bSkillTrait2)
	INJ_I8(D, Merc->bLeadership)
	INJ_I8A(D, Merc->bBuddy, lengthof(Merc->bBuddy))
	INJ_I8A(D, Merc->bHated, lengthof(Merc->bHated))
	INJ_I8(D, Merc->bExpLevel)
	INJ_I8(D, Merc->bMarksmanship)
	INJ_U8(D, Merc->bMinService)
	INJ_I8(D, Merc->bWisdom)
	INJ_SKIP(D, 2)
	INJ_U8A(D, Merc->bInvStatus, lengthof(Merc->bInvStatus))
	INJ_U8A(D, Merc->bInvNumber, lengthof(Merc->bInvNumber))
	INJ_U16A(D, Merc->usApproachFactor, lengthof(Merc->usApproachFactor))
	INJ_I8(D, Merc->bMainGunAttractiveness)
	INJ_I8(D, Merc->bAgility)
	INJ_BOOL(D, Merc->fUseProfileInsertionInfo)
	INJ_SKIP(D, 1)
	INJ_I16(D, Merc->sGridNo)
	INJ_U8(D, Merc->ubQuoteActionID)
	INJ_I8(D, Merc->bMechanical)
	INJ_U8(D, Merc->ubInvUndroppable)
	INJ_U8A(D, Merc->ubRoomRangeStart, lengthof(Merc->ubRoomRangeStart))
	INJ_SKIP(D, 1)
	INJ_U16A(D, Merc->inv, lengthof(Merc->inv))
	INJ_I8A(D, Merc->bMercTownReputation, lengthof(Merc->bMercTownReputation))
	INJ_U16A(D, Merc->usStatChangeChances, lengthof(Merc->usStatChangeChances))
	INJ_U16A(D, Merc->usStatChangeSuccesses, lengthof(Merc->usStatChangeSuccesses))
	INJ_U8(D, Merc->ubStrategicInsertionCode)
	INJ_U8A(D, Merc->ubRoomRangeEnd, lengthof(Merc->ubRoomRangeEnd))
	INJ_SKIP(D, 4)
	INJ_U8(D, Merc->ubLastQuoteSaid)
	INJ_I8(D, Merc->bRace)
	INJ_I8(D, Merc->bNationality)
	INJ_I8(D, Merc->bAppearance)
	INJ_I8(D, Merc->bAppearanceCareLevel)
	INJ_I8(D, Merc->bRefinement)
	INJ_I8(D, Merc->bRefinementCareLevel)
	INJ_I8(D, Merc->bHatedNationality)
	INJ_I8(D, Merc->bHatedNationalityCareLevel)
	INJ_I8(D, Merc->bRacist)
	INJ_SKIP(D, 1)
	INJ_U32(D, Merc->uiWeeklySalary)
	INJ_U32(D, Merc->uiBiWeeklySalary)
	INJ_I8(D, Merc->bMedicalDeposit)
	INJ_I8(D, Merc->bAttitude)
	INJ_I8(D, Merc->bBaseMorale)
	INJ_SKIP(D, 1)
	INJ_U16(D, Merc->sMedicalDepositAmount)
	INJ_I8(D, Merc->bLearnToLike)
	INJ_U8A(D, Merc->ubApproachVal, lengthof(Merc->ubApproachVal))
	INJ_U8A(D, *Merc->ubApproachMod, sizeof(Merc->ubApproachMod) / sizeof(**Merc->ubApproachMod))
	INJ_I8(D, Merc->bTown)
	INJ_I8(D, Merc->bTownAttachment)
	INJ_SKIP(D, 1)
	INJ_U16(D, Merc->usOptionalGearCost)
	INJ_I8A(D, Merc->bMercOpinion, lengthof(Merc->bMercOpinion))
	INJ_I8(D, Merc->bApproached)
	INJ_I8(D, Merc->bMercStatus)
	INJ_I8A(D, Merc->bHatedTime, lengthof(Merc->bHatedTime))
	INJ_I8(D, Merc->bLearnToLikeTime)
	INJ_I8(D, Merc->bLearnToHateTime)
	INJ_I8A(D, Merc->bHatedCount, lengthof(Merc->bHatedCount))
	INJ_I8(D, Merc->bLearnToLikeCount)
	INJ_I8(D, Merc->bLearnToHateCount)
	INJ_U8(D, Merc->ubLastDateSpokenTo)
	INJ_U8(D, Merc->bLastQuoteSaidWasSpecial)
	INJ_I8(D, Merc->bSectorZ)
	INJ_U16(D, Merc->usStrategicInsertionData)
	INJ_I8(D, Merc->bFriendlyOrDirectDefaultResponseUsedRecently)
	INJ_I8(D, Merc->bRecruitDefaultResponseUsedRecently)
	INJ_I8(D, Merc->bThreatenDefaultResponseUsedRecently)
	INJ_I8(D, Merc->bNPCData)
	INJ_I32(D, Merc->iBalance)
	INJ_I16(D, Merc->sTrueSalary)
	INJ_U8(D, Merc->ubCivilianGroup)
	INJ_U8(D, Merc->ubNeedForSleep)
	INJ_U32(D, Merc->uiMoney)
	INJ_I8(D, Merc->bNPCData2)
	INJ_U8(D, Merc->ubMiscFlags3)
	INJ_U8(D, Merc->ubDaysOfMoraleHangover)
	INJ_U8(D, Merc->ubNumTimesDrugUseInLifetime)
	INJ_U32(D, Merc->uiPrecedentQuoteSaid)
	INJ_U32(D, Merc->uiProfileChecksum)
	INJ_I16(D, Merc->sPreCombatGridNo)
	INJ_U8(D, Merc->ubTimeTillNextHatedComplaint)
	INJ_U8(D, Merc->ubSuspiciousDeath)
	INJ_I32(D, Merc->iMercMercContractLength)
	INJ_U32(D, Merc->uiTotalCostToDate)
	INJ_SKIP(D, 4)

#ifdef _WIN32 // XXX HACK000A
	Assert(D == Dst + 716);
#else
	Assert(D == Dst + 796);
#endif
}


BOOLEAN InjectMercProfileIntoFile(HWFILE File, const MERCPROFILESTRUCT* Merc)
{
#ifdef _WIN32 // XXX HACK000A
	BYTE Data[716];
#else
	BYTE Data[796];
#endif
	InjectMercProfile(Data, Merc);
	return FileWrite(File, Data, sizeof(Data));
}
