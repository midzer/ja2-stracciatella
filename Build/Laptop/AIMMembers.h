#ifndef __AIMMEMBERS_H_
#define __AIMMEMBERS_H_

BOOLEAN EnterAIMMembers(void);
void ExitAIMMembers(void);
void HandleAIMMembers(void);
BOOLEAN RenderAIMMembers(void);


BOOLEAN DrawNumeralsToScreen(INT32 iNumber, INT8 bWidth, UINT16 usLocX, UINT16 usLocY, UINT32 ulFont, UINT8 ubColor);

void DisplayTextForMercFaceVideoPopUp(const wchar_t* pString, size_t Length);
void EnterInitAimMembers(void);
BOOLEAN RenderAIMMembersTopLevel(void);
void ResetMercAnnoyanceAtPlayer( UINT8 ubMercID );
void DisableNewMailMessage(void);
void DisplayPopUpBoxExplainingMercArrivalLocationAndTime(void);


// which mode are we in during video conferencing?..0 means no video conference
extern UINT8 gubVideoConferencingMode;

//TEMP!!!
#ifdef JA2TESTVERSION
void TempHiringOfMercs( UINT8 ubNumberOfMercs, BOOLEAN fReset );
#endif


#if defined ( JA2TESTVERSION ) || defined ( JA2DEMO )
 void DemoHiringOfMercs(void);
#endif



#endif
