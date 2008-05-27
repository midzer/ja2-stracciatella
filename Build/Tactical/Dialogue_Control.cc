#include "Font.h"
#include "Font_Control.h"
#include "GameLoop.h"
#include "Handle_UI.h"
#include "Interface.h"
#include "Isometric_Utils.h"
#include "Local.h"
#include "MessageBoxScreen.h"
#include "Soldier_Control.h"
#include "Encrypted_File.h"
#include "Faces.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "VSurface.h"
#include "WCheck.h"
#include "Overhead.h"
#include "Dialogue_Control.h"
#include "Message.h"
#include "Render_Dirty.h"
#include "Soldier_Profile.h"
#include "WordWrap.h"
#include "SysUtil.h"
#include "AIMMembers.h"
#include "Mercs.h"
#include "Interface_Dialogue.h"
#include "MercTextBox.h"
#include "RenderWorld.h"
#include "Soldier_Macros.h"
#include "Squads.h"
#include "ScreenIDs.h"
#include "Interface_Utils.h"
#include "StrategicMap.h"
#include "PreBattle_Interface.h"
#include "Game_Clock.h"
#include "Quests.h"
#include "Cursors.h"
#include "GameScreen.h"
#include "Random.h"
#include "Map_Screen_Helicopter.h"
#include "GameSettings.h"
#include "ShopKeeper_Interface.h"
#include "Map_Screen_Interface.h"
#include "Text.h"
#include "Merc_Contract.h"
#include "History.h"
#include "Town_Militia.h"
#include "Meanwhile.h"
#include "SkillCheck.h"
#include "Interface_Control.h"
#include "Finances.h"
#include "Civ_Quotes.h"
#include "Map_Screen_Interface_Map.h"
#include "OppList.h"
#include "AI.h"
#include "WorldMan.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Campaign.h"
#include "End_Game.h"
#include "LOS.h"
#include "QArray.h"
#include "JAScreens.h"
#include "Video.h"
#include "SoundMan.h"
#include "MemMan.h"
#include "Button_System.h"
#include "Container.h"


#define DIALOGUESIZE 240
#define   QUOTE_MESSAGE_SIZE		520

#define		TALK_PANEL_FACE_X				6
#define		TALK_PANEL_FACE_Y				9
#define		TALK_PANEL_NAME_X				5
#define		TALK_PANEL_NAME_Y				114
#define		TALK_PANEL_NAME_WIDTH		92
#define		TALK_PANEL_NAME_HEIGHT	15
#define		TALK_PANEL_MENU_STARTY	8
#define		TALK_PANEL_MENU_HEIGHT	24
#define		TALK_MENU_WIDTH					96
#define		TALK_MENU_HEIGHT				16

#define		DIALOGUE_DEFAULT_SUBTITLE_WIDTH		200
#define		TEXT_DELAY_MODIFIER			60


struct DIALOGUE_Q_STRUCT
{
	UINT16    usQuoteNum;
	UINT8     ubCharacterNum;
	INT8      bUIHandlerID;
	FACETYPE* face;
	INT32     iTimeStamp;
	UINT32    uiSpecialEventFlag;
	UINT32    uiSpecialEventData;
	UINT32    uiSpecialEventData2;
	UINT32    uiSpecialEventData3;
	UINT32    uiSpecialEventData4;
	BOOLEAN   fFromSoldier;
	BOOLEAN   fDelayed;
	BOOLEAN   fPauseTime;
};

typedef SGP::Queue<DIALOGUE_Q_STRUCT*> DialogueQueue;


BOOLEAN fExternFacesLoaded = FALSE;

FACETYPE* uiExternalStaticNPCFaces[NUMBER_OF_EXTERNAL_NPC_FACES];
const ProfileID g_external_face_profile_ids[] =
{
	SKYRIDER,
	FRED,
	MATT,
	OSWALD,
	CALVIN,
	CARL
};
CASSERT(lengthof(g_external_face_profile_ids) == NUMBER_OF_EXTERNAL_NPC_FACES)

UINT8	gubMercValidPrecedentQuoteID[ NUMBER_VALID_MERC_PRECEDENT_QUOTES ] =
					{ 80, 81, 82, 83, 86, 87, 88, 95, 97, 99, 100, 101, 102 };


UINT16	gusStopTimeQuoteList[ ] =
{
	QUOTE_BOOBYTRAP_ITEM,
	QUOTE_SUSPICIOUS_GROUND
};

UINT8							gubNumStopTimeQuotes = 2;


// QUEUE UP DIALOG!
#define		INITIAL_Q_SIZE				10
static DialogueQueue* ghDialogueQ;
FACETYPE	*gpCurrentTalkingFace	= NULL;
UINT8			gubCurrentTalkingID   = NO_PROFILE;
INT8			gbUIHandlerID;

INT32				giNPCReferenceCount = 0;
INT32				giNPCSpecialReferenceCount = 0;

INT16       gsExternPanelXPosition     = DEFAULT_EXTERN_PANEL_X_POS;
INT16       gsExternPanelYPosition     = DEFAULT_EXTERN_PANEL_Y_POS;

BOOLEAN			gfDialogueQueuePaused = FALSE;
static UINT16 gusSubtitleBoxWidth;
UINT16			gusSubtitleBoxHeight;
static VIDEO_OVERLAY* g_text_box_overlay = NULL;
BOOLEAN			gfFacePanelActive = FALSE;
UINT32			guiScreenIDUsedWhenUICreated;
wchar_t				gzQuoteStr[ QUOTE_MESSAGE_SIZE ];
MOUSE_REGION	gTextBoxMouseRegion;
MOUSE_REGION	gFacePopupMouseRegion;
BOOLEAN				gfUseAlternateDialogueFile = FALSE;

// set the top position value for merc dialogue pop up boxes
INT16 gsTopPosition = 20;


INT32 iDialogueBox = -1;


// the next said quote will pause time
BOOLEAN fPausedTimeDuringQuote = FALSE;
BOOLEAN fWasPausedDuringDialogue = FALSE;

INT8	gubLogForMeTooBleeds = FALSE;


// has the text region been created?
BOOLEAN fTextBoxMouseRegionCreated = FALSE;
BOOLEAN	fExternFaceBoxRegionCreated = FALSE;

// due to last quote system?
BOOLEAN fDialogueBoxDueToLastMessage = FALSE;

// last quote timers
UINT32 guiDialogueLastQuoteTime = 0;
UINT32 guiDialogueLastQuoteDelay = 0;


void UnPauseGameDuringNextQuote( void )
{
	fPausedTimeDuringQuote = FALSE;
}


static void PauseTimeDuringNextQuote(void)
{
	fPausedTimeDuringQuote = TRUE;
}

BOOLEAN DialogueActive( )
{
	if ( gpCurrentTalkingFace != NULL )
	{
		return( TRUE );
	}

	return( FALSE );
}


void InitalizeDialogueControl()
{
	ghDialogueQ         = new DialogueQueue(INITIAL_Q_SIZE);
	giNPCReferenceCount = 0;
}

void ShutdownDialogueControl()
{
	if (ghDialogueQ != NULL)
	{
		delete ghDialogueQ;
		ghDialogueQ = NULL;
		gfWaitingForTriggerTimer = FALSE;
	}

	// shutdown external static NPC faces
	ShutdownStaticExternalNPCFaces();

	// gte rid of portraits for cars
	UnLoadCarPortraits();
}


void InitalizeStaticExternalNPCFaces( void )
{
#ifndef JA2DEMO
	INT32 iCounter = 0;
	// go and grab all external NPC faces that are needed for the game who won't exist as soldiertypes

	if (fExternFacesLoaded) return;

	fExternFacesLoaded = TRUE;

	for( iCounter = 0; iCounter < NUMBER_OF_EXTERNAL_NPC_FACES; iCounter++ )
	{
		uiExternalStaticNPCFaces[iCounter] = InitFace(g_external_face_profile_ids[iCounter], NULL, FACE_FORCE_SMALL);
	}
#endif
}


void ShutdownStaticExternalNPCFaces( void )
{
	INT32 iCounter = 0;

	if (!fExternFacesLoaded) return;

	fExternFacesLoaded = FALSE;

	// remove all external npc faces
	for( iCounter = 0; iCounter < NUMBER_OF_EXTERNAL_NPC_FACES; iCounter++ )
	{
		DeleteFace( uiExternalStaticNPCFaces[ iCounter ] );
	}
}


void EmptyDialogueQueue()
{
	// If we have anything left in the queue, remove!
	if (ghDialogueQ != NULL)
	{
		delete ghDialogueQ;
		ghDialogueQ = new DialogueQueue(INITIAL_Q_SIZE);
	}

	gfWaitingForTriggerTimer = FALSE;
}


BOOLEAN DialogueQueueIsEmpty( )
{
	return ghDialogueQ && ghDialogueQ->IsEmpty();
}


BOOLEAN	DialogueQueueIsEmptyOrSomebodyTalkingNow( )
{
	if ( gpCurrentTalkingFace != NULL )
	{
		return( FALSE );
	}

	if ( !DialogueQueueIsEmpty( ) )
	{
		return( FALSE );
	}

	return( TRUE );
}

void DialogueAdvanceSpeech( )
{
	// Shut them up!
	InternalShutupaYoFace(gpCurrentTalkingFace, FALSE);
}


void StopAnyCurrentlyTalkingSpeech( )
{
	// ATE; Make sure guys stop talking....
	if ( gpCurrentTalkingFace != NULL )
	{
		InternalShutupaYoFace(gpCurrentTalkingFace, TRUE);
	}
}


static void CreateTalkingUI(INT8 bUIHandlerID, FACETYPE* face, UINT8 ubCharacterNum, const wchar_t* zQuoteStr);


// ATE: Handle changes like when face goes from
// 'external' to on the team panel...
void HandleDialogueUIAdjustments( )
{
	// OK, check if we are still taking
	if ( gpCurrentTalkingFace != NULL )
	{
		if ( gpCurrentTalkingFace->fTalking )
		{
			// ATE: Check for change in state for the guy currently talking on 'external' panel....
			if ( gfFacePanelActive )
			{
				const SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(gubCurrentTalkingID);
				if ( pSoldier )
				{
					if ( 0 )
					{
						// A change in plans here...
						// We now talk through the interface panel...
						if (gpCurrentTalkingFace->video_overlay != NULL)
						{
							RemoveVideoOverlay(gpCurrentTalkingFace->video_overlay);
							gpCurrentTalkingFace->video_overlay = NULL;
						}
						gfFacePanelActive = FALSE;

						RemoveVideoOverlay(g_text_box_overlay);
						g_text_box_overlay = NULL;

						if ( fTextBoxMouseRegionCreated )
						{
							MSYS_RemoveRegion( &gTextBoxMouseRegion );
							fTextBoxMouseRegionCreated = FALSE;
						}

						// Setup UI again!
						CreateTalkingUI(gbUIHandlerID, pSoldier->face, pSoldier->ubProfile, gzQuoteStr);
					}
				}
			}
		}
	}
}


static void CheckForStopTimeQuotes(UINT16 usQuoteNum);
static BOOLEAN ExecuteCharacterDialogue(UINT8 ubCharacterNum, UINT16 usQuoteNum, FACETYPE* face, UINT8 bUIHandlerID, BOOLEAN fFromSoldier);
static void HandleTacticalSpeechUI(UINT8 ubCharacterNum, FACETYPE* face);


void HandleDialogue()
{
	static BOOLEAN fOldEngagedInConvFlagOn = FALSE;

	// we don't want to just delay action of some events, we want to pause the whole queue, regardless of the event
	if (gfDialogueQueuePaused) return;

	bool const empty = ghDialogueQ->IsEmpty();

	if (empty && gpCurrentTalkingFace == NULL)
	{
		HandlePendingInitConv();
	}

	HandleCivQuote();

	// Alrighty, check for a change in state, do stuff appropriately....
	// Turned on
	if (!fOldEngagedInConvFlagOn && gTacticalStatus.uiFlags & ENGAGED_IN_CONV)
	{
		// OK, we have just entered...
		fOldEngagedInConvFlagOn = TRUE;

		PauseGame();
		LockPauseState(14);
	}
	else if (fOldEngagedInConvFlagOn && !(gTacticalStatus.uiFlags & ENGAGED_IN_CONV))
	{
		// OK, we left...
		fOldEngagedInConvFlagOn = FALSE;

		UnLockPauseState();
		UnPauseGame();

		// if we're exiting boxing with the UI lock set then DON'T OVERRIDE THIS!
		if (!(gTacticalStatus.uiFlags & IGNORE_ENGAGED_IN_CONV_UI_UNLOCK))
		{
			switch (gTacticalStatus.bBoxingState)
			{
				case WON_ROUND:
				case LOST_ROUND:
				case DISQUALIFIED:
					break;

				default:
					guiPendingOverrideEvent = LU_ENDUILOCK;
					HandleTacticalUI();

					// ATE: If this is NOT the player's turn.. engage AI UI lock!
					if (gTacticalStatus.ubCurrentTeam != gbPlayerNum)
					{
						// Setup locked UI
						guiPendingOverrideEvent = LU_BEGINUILOCK;
						HandleTacticalUI();
					}
					break;
			}
		}

		gTacticalStatus.uiFlags &= ~IGNORE_ENGAGED_IN_CONV_UI_UNLOCK;
	}

	if (gTacticalStatus.uiFlags & ENGAGED_IN_CONV &&
			!gfInTalkPanel                            && // Are we in here because of the dialogue system up?
			guiPendingScreen != MSG_BOX_SCREEN        && // ATE: NOT if we have a message box pending
			guiCurrentScreen != MSG_BOX_SCREEN)
	{
		// No, so we should lock the UI!
		guiPendingOverrideEvent = LU_BEGINUILOCK;
		HandleTacticalUI();
	}

	// OK, check if we are still taking
	if (gpCurrentTalkingFace != NULL)
	{
		if (gpCurrentTalkingFace->fTalking)
		{
			// ATE: OK, MANAGE THE DISPLAY OF OUR CURRENTLY ACTIVE FACE IF WE / IT CHANGES STATUS
			// THINGS THAT CAN CHANGE STATUS:
			//		CHANGE TO MAPSCREEN
			//		CHANGE TO GAMESCREEN
			//		CHANGE IN MERC STATUS TO BE IN A SQUAD
			//    CHANGE FROM TEAM TO INV INTERFACE

			// Where are we and where did this face once exist?
			if (guiScreenIDUsedWhenUICreated == GAME_SCREEN && guiCurrentScreen == MAP_SCREEN)
			{
				// GO FROM GAMESCREEN TO MAPSCREEN

				// delete face panel if there is one!
				if (gfFacePanelActive)
				{
					// Set face inactive!
					if (gpCurrentTalkingFace->video_overlay != NULL)
					{
						RemoveVideoOverlay(gpCurrentTalkingFace->video_overlay);
						gpCurrentTalkingFace->video_overlay = NULL;
					}

					if (fExternFaceBoxRegionCreated)
					{
						fExternFaceBoxRegionCreated = FALSE;
						MSYS_RemoveRegion(&gFacePopupMouseRegion);
					}

					// Set face inactive....
					gpCurrentTalkingFace->fCanHandleInactiveNow = TRUE;
					SetAutoFaceInActive(gpCurrentTalkingFace);
					HandleTacticalSpeechUI(gubCurrentTalkingID, gpCurrentTalkingFace);

          // ATE: Force mapscreen to set face active again.....
        	fReDrawFace = TRUE;
					DrawFace();

					gfFacePanelActive = FALSE;
				}

				guiScreenIDUsedWhenUICreated = guiCurrentScreen;
			}
			else if (guiScreenIDUsedWhenUICreated == MAP_SCREEN && guiCurrentScreen == GAME_SCREEN)
			{
				HandleTacticalSpeechUI(gubCurrentTalkingID, gpCurrentTalkingFace);
				guiScreenIDUsedWhenUICreated = guiCurrentScreen;
			}
			return;
		}

		// Check special flags
		// If we are done, check special face flag for trigger NPC!
		if (gpCurrentTalkingFace->uiFlags & FACE_PCTRIGGER_NPC)
		{
			 // Decrement refrence count...
			 giNPCReferenceCount--;

			 TriggerNPCRecord((UINT8)gpCurrentTalkingFace->uiUserData1, (UINT8)gpCurrentTalkingFace->uiUserData2);
			 //Reset flag!
			 gpCurrentTalkingFace->uiFlags &= ~FACE_PCTRIGGER_NPC;
		}

		if (gpCurrentTalkingFace->uiFlags & FACE_MODAL)
		{
			gpCurrentTalkingFace->uiFlags &= ~FACE_MODAL;
			EndModalTactical();
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Ending Modal Tactical Quote.");
		}

		if (gpCurrentTalkingFace->uiFlags & FACE_TRIGGER_PREBATTLE_INT)
		{
			UnLockPauseState();
			InitPreBattleInterface((GROUP*)gpCurrentTalkingFace->uiUserData1, TRUE);
			//Reset flag!
			gpCurrentTalkingFace->uiFlags &= ~FACE_TRIGGER_PREBATTLE_INT;
		}

		gpCurrentTalkingFace = NULL;
		gubCurrentTalkingID	 = NO_PROFILE;
		gTacticalStatus.ubLastQuoteProfileNUm = NO_PROFILE;

		if (fWasPausedDuringDialogue)
		{
			fWasPausedDuringDialogue = FALSE;
			UnLockPauseState();
			UnPauseGame();
		}
	}

	if (empty)
	{
		if (gfMikeShouldSayHi == TRUE)
		{
			SOLDIERTYPE* const pMike = FindSoldierByProfileID(MIKE);
			if (pMike)
			{
				INT16 const sPlayerGridNo = ClosestPC(pMike, NULL);
				if (sPlayerGridNo != NOWHERE)
				{
					SOLDIERTYPE* const player = WhoIsThere2(sPlayerGridNo, 0);
					if (player != NULL)
					{
						InitiateConversation(pMike, player, NPC_INITIAL_QUOTE, 0);
						gMercProfiles[pMike->ubProfile].ubMiscFlags2 |= PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE;
						// JA2Gold: special hack value of 2 to prevent dialogue from coming up more than once
						gfMikeShouldSayHi = 2;
					}
				}
			}
		}

		return;
	}

	// If here, pick current one from queue and play
	DIALOGUE_Q_STRUCT* const d = ghDialogueQ->Remove();

	// If we are in auto bandage, ignore any quotes!
	if (gTacticalStatus.fAutoBandageMode)
	{
		if (d->fPauseTime)
		{
			UnLockPauseState();
			UnPauseGame();
		}

		MemFree(d);
		return;
	}

	// Check time delay

	// Alrighty, check if this one is to be delayed until we gain control.
	// If so, place it back in!
	if (d->fDelayed &&
			gTacticalStatus.ubCurrentTeam != gbPlayerNum) // Are we not in our turn and not interrupted
	{
		//Place back in!
		ghDialogueQ->Add(d);
		return;
	}

	// ATE: OK: If a battle sound, and delay value was given, set time stamp
	// now...
	if (d->uiSpecialEventFlag == DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND &&
			d->uiSpecialEventData2 != 0                                   &&
			GetJA2Clock() - d->iTimeStamp < d->uiSpecialEventData2)
	{
		//Place back in!
		ghDialogueQ->Add(d);
		return;
	}

	// Try to find soldier...
	SOLDIERTYPE* s = FindSoldierByProfileIDOnPlayerTeam(d->ubCharacterNum);
	if (s != NULL && SoundIsPlaying(s->uiBattleSoundID))
	{
		//Place back in!
		ghDialogueQ->Add(d);
		return;
	}

	if (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN &&
			d->uiSpecialEventFlag == 0)
	{
		d->fPauseTime = TRUE;
	}

	if (d->fPauseTime && !GamePaused())
	{
		PauseGame();
		LockPauseState(15);
		fWasPausedDuringDialogue = TRUE;
	}

	// Now play first item in queue
	// If it's not a 'special' dialogue event, continue
	if (d->uiSpecialEventFlag == 0)
	{
		if (s && s->fMercAsleep) // wake grunt up to say
		{
			s->fMercAsleep = FALSE;

			// refresh map screen
			fCharacterInfoPanelDirty = TRUE;
			fTeamPanelDirty = TRUE;

			// allow them to go back to sleep
			TacticalCharacterDialogueWithSpecialEvent(s, d->usQuoteNum, DIALOGUE_SPECIAL_EVENT_SLEEP, 1, 0);
		}

		gTacticalStatus.ubLastQuoteSaid       = d->usQuoteNum;
		gTacticalStatus.ubLastQuoteProfileNUm = d->ubCharacterNum;

		// Setup face pointer
		gpCurrentTalkingFace = d->face;
		gubCurrentTalkingID  = d->ubCharacterNum;

		ExecuteCharacterDialogue(d->ubCharacterNum, d->usQuoteNum, d->face, d->bUIHandlerID, d->fFromSoldier);
	}
	else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SKIP_A_FRAME)
	{

	}
	else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE)
	{
		BOOLEAN const lock = d->uiSpecialEventData != 0;
		switch (d->uiSpecialEventData2)
		{
			case MAP_SCREEN: fLockOutMapScreenInterface = lock; break;
		}
	}
	else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_REMOVE_EPC)
	{
		gMercProfiles[(UINT8)d->uiSpecialEventData].ubMiscFlags &= ~PROFILE_MISC_FLAG_FORCENPCQUOTE;
		UnRecruitEPC((UINT8)d->uiSpecialEventData);
		ReBuildCharactersList();
	}
	else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTRACT_WANTS_TO_RENEW)
	{
		HandleMercIsWillingToRenew((UINT8)d->uiSpecialEventData);
	}
	else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTRACT_NOGO_TO_RENEW)
	{
		HandleMercIsNotWillingToRenew((UINT8)d->uiSpecialEventData);
	}
	else
	{
		if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_USE_ALTERNATE_FILES)
		{
			gfUseAlternateDialogueFile = TRUE;

			// Setup face pointer
			gpCurrentTalkingFace = d->face;
			gubCurrentTalkingID   = d->ubCharacterNum;

			ExecuteCharacterDialogue(d->ubCharacterNum, d->usQuoteNum, d->face, d->bUIHandlerID, d->fFromSoldier);

			gfUseAlternateDialogueFile = FALSE;
		}
		// We could have a special flag, but dialogue as well
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_PCTRIGGERNPC)
		{
			// Setup face pointer
			gpCurrentTalkingFace = d->face;
			gubCurrentTalkingID  = d->ubCharacterNum;

			ExecuteCharacterDialogue(d->ubCharacterNum, d->usQuoteNum, d->face, d->bUIHandlerID, d->fFromSoldier);

			// Setup face with data!
			gpCurrentTalkingFace->uiFlags     |= FACE_PCTRIGGER_NPC;
			gpCurrentTalkingFace->uiUserData1  = d->uiSpecialEventData;
			gpCurrentTalkingFace->uiUserData2  = d->uiSpecialEventData2;

		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SHOW_CONTRACT_MENU)
		{
			// Setup face pointer
			// ATE: THis is working with MARK'S STUFF :(
			// Need this stuff so that bSelectedInfoChar is set...
			SetInfoChar(s);

			fShowContractMenu = TRUE;
			RebuildContractBoxForMerc(s);
			bSelectedContractChar = bSelectedInfoChar;
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND)
		{
			s = FindSoldierByProfileID(d->ubCharacterNum);
			if (s)
			{
				InternalDoMercBattleSound(s, (UINT8)d->uiSpecialEventData, 0);
			}
		}

		if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SIGNAL_ITEM_LOCATOR_START)
		{
			// Turn off item lock for locators...
			gTacticalStatus.fLockItemLocators = FALSE;

			SlideToLocation((UINT16)d->uiSpecialEventData);

			gpCurrentTalkingFace = d->face;
			gubCurrentTalkingID  = d->ubCharacterNum;

			ExecuteCharacterDialogue(d->ubCharacterNum, d->usQuoteNum, d->face, d->bUIHandlerID, d->fFromSoldier);
		}

		if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_ENABLE_AI)
		{
			//OK, allow AI to work now....
			UnPauseAI();
		}

		if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_TRIGGERPREBATTLEINTERFACE)
		{
			UnLockPauseState();
			InitPreBattleInterface((GROUP*)d->uiSpecialEventData, TRUE);
		}

		if (d->uiSpecialEventFlag & DIALOGUE_ADD_EVENT_FOR_SOLDIER_UPDATE_BOX)
		{
			INT32 const iReason = d->uiSpecialEventData;
			switch (iReason)
			{
				case UPDATE_BOX_REASON_ADDSOLDIER:
				{
					SOLDIERTYPE* const pUpdateSoldier = GetMan(d->uiSpecialEventData2);
					if (pUpdateSoldier->bActive) AddSoldierToUpdateBox(pUpdateSoldier);
					break;
				}

				case UPDATE_BOX_REASON_SET_REASON:
					SetSoldierUpdateBoxReason(d->uiSpecialEventData2);
					break;

				case UPDATE_BOX_REASON_SHOW_BOX:
					ShowUpdateBox();
					break;
			}
		}

		if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_BEGINPREBATTLEINTERFACE)
		{
			// Setup face pointer
			gpCurrentTalkingFace = d->face;
			gubCurrentTalkingID  = d->ubCharacterNum;

			ExecuteCharacterDialogue(d->ubCharacterNum, d->usQuoteNum, d->face, d->bUIHandlerID, d->fFromSoldier);

			// Setup face with data!
			gpCurrentTalkingFace->uiFlags     |= FACE_TRIGGER_PREBATTLE_INT;
			gpCurrentTalkingFace->uiUserData1  = d->uiSpecialEventData;
			gpCurrentTalkingFace->uiUserData2  = d->uiSpecialEventData2;
		}

		if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SHOPKEEPER)
		{
			switch (d->uiSpecialEventData)
			{
				wchar_t zMoney[128];
				wchar_t zText[512];

				case 0:
					//popup a message stating the player doesnt have enough money
					SPrintMoney(zMoney, d->uiSpecialEventData2);
					swprintf(zText, lengthof(zText), SkiMessageBoxText[SKI_SHORT_FUNDS_TEXT], zMoney);
					DoSkiMessageBox(MSG_BOX_BASIC_STYLE, zText, SHOPKEEPER_SCREEN, MSG_BOX_FLAG_OK, ConfirmDontHaveEnoughForTheDealerMessageBoxCallBack);
					break;

				case 1:
					/* if the player is trading items, ask them if we should deduct money
					 * out the players account to cover the difference */
					SPrintMoney(zMoney, d->uiSpecialEventData2);
					swprintf(zText, lengthof(zText), SkiMessageBoxText[SKI_QUESTION_TO_DEDUCT_MONEY_FROM_PLAYERS_ACCOUNT_TO_COVER_DIFFERENCE], zMoney);
					DoSkiMessageBox(MSG_BOX_BASIC_STYLE, zText, SHOPKEEPER_SCREEN, MSG_BOX_FLAG_YESNO, ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack);
					break;

				case 2:
					/* ask them if we should deduct money out the players account to cover
					 * the difference */
					SPrintMoney(zMoney, d->uiSpecialEventData2);
					swprintf(zText, lengthof(zText), SkiMessageBoxText[SKI_QUESTION_TO_DEDUCT_MONEY_FROM_PLAYERS_ACCOUNT_TO_COVER_COST], zMoney);
					DoSkiMessageBox(MSG_BOX_BASIC_STYLE, zText, SHOPKEEPER_SCREEN, MSG_BOX_FLAG_YESNO, ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack);
					break;

				case 3: // this means a dialogue event is in progress
					giShopKeepDialogueEventinProgress = d->uiSpecialEventData2;
					break;

				case 4: // this means a dialogue event has ended
					giShopKeepDialogueEventinProgress = -1;
					break;

				case 5: // this means a dialogue event has ended
					gfSKIScreenExit = TRUE;
					break;

				case 6:
					if (guiCurrentScreen == SHOPKEEPER_SCREEN)
					{
						DisableButton(guiSKI_TransactionButton);
					}
					break;

				case 7:
					if (guiCurrentScreen == SHOPKEEPER_SCREEN)
					{
						EnableButton(guiSKI_TransactionButton);
					}
					break;
			}
		}

		if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_EXIT_MAP_SCREEN)
		{
			// select sector
			ChangeSelectedMapSector((INT16)d->uiSpecialEventData, (INT16)d->uiSpecialEventData2, (INT8)d->uiSpecialEventData3);
			RequestTriggerExitFromMapscreen(MAP_EXIT_TO_TACTICAL);
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_DISPLAY_STAT_CHANGE)
		{
			s = FindSoldierByProfileID(d->ubCharacterNum);
			if (s)
			{
				// tell player about stat increase
				wchar_t wTempString[128];
				BuildStatChangeString(wTempString, lengthof(wTempString), s->name, (BOOLEAN)d->uiSpecialEventData, (INT16)d->uiSpecialEventData2, (UINT8)d->uiSpecialEventData3);
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, wTempString);
			}
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_UNSET_ARRIVES_FLAG)
		{
			gTacticalStatus.bMercArrivingQuoteBeingUsed = FALSE;
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SKYRIDERMAPSCREENEVENT)
		{
			// Setup face pointer
			gpCurrentTalkingFace = d->face;
			gubCurrentTalkingID  = d->ubCharacterNum;

			// handle the monologue event
			HandleSkyRiderMonologueEvent(d->uiSpecialEventData, d->uiSpecialEventData2);
		}

		if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_MINESECTOREVENT)
		{
			// Setup face pointer
			gpCurrentTalkingFace = d->face;
			gubCurrentTalkingID  = d->ubCharacterNum;

			// set up the mine highlgith events
			SetUpAnimationOfMineSectors(d->uiSpecialEventData);
		}

		//Switch on our special events
		if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_GIVE_ITEM)
		{
			if (d->bUIHandlerID == DIALOGUE_NPC_UI)
			{
				HandleNPCItemGiven((UINT8)d->uiSpecialEventData, (OBJECTTYPE*)d->uiSpecialEventData2, (INT8)d->uiSpecialEventData3);
			}
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_TRIGGER_NPC)
		{
			if (d->bUIHandlerID == DIALOGUE_NPC_UI)
			{
				HandleNPCTriggerNPC((UINT8)d->uiSpecialEventData, (UINT8)d->uiSpecialEventData2, (BOOLEAN)d->uiSpecialEventData3, (UINT8)d->uiSpecialEventData4);
			}
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_GOTO_GRIDNO)
		{
			if (d->bUIHandlerID == DIALOGUE_NPC_UI)
			{
				HandleNPCGotoGridNo((UINT8)d->uiSpecialEventData, (UINT16)d->uiSpecialEventData2, (UINT8)d->uiSpecialEventData3);
			}
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_DO_ACTION)
		{
			if (d->bUIHandlerID == DIALOGUE_NPC_UI)
			{
				HandleNPCDoAction((UINT8)d->uiSpecialEventData, (UINT16)d->uiSpecialEventData2, (UINT8)d->uiSpecialEventData3);
			}
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CLOSE_PANEL)
		{
			if (d->bUIHandlerID == DIALOGUE_NPC_UI)
			{
				HandleNPCClosePanel();
			}
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SHOW_UPDATE_MENU)
		{
			SetUpdateBoxFlag(TRUE);
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTINUE_TRAINING_MILITIA)
		{
			s = FindSoldierByProfileID((UINT8)d->uiSpecialEventData);
			if (s != NULL)
			{
				HandleInterfaceMessageForContinuingTrainingMilitia(s);
			}
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_ENTER_MAPSCREEN)
		{
			if (!(guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN))
			{
				gfEnteringMapScreen    = TRUE;
				fEnterMapDueToContract = TRUE;
			}
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING)
		{
			s = FindSoldierByProfileID(d->ubCharacterNum);
			if (s != NULL)
			{
				// .. remove the fired soldier again
				BeginStrategicRemoveMerc(s, (UINT8)d->uiSpecialEventData);
			}
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING_NO_ASK_EQUIP)
		{
			s = FindSoldierByProfileID(d->ubCharacterNum);
			if (s != NULL)
			{
				// .. remove the fired soldier again
				StrategicRemoveMerc(s);
			}
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_MULTIPURPOSE)
		{
			if (d->uiSpecialEventData & MULTIPURPOSE_SPECIAL_EVENT_DONE_KILLING_DEIDRANNA)
			{
				HandleDoneLastKilledQueenQuote();
			}
			else if (d->uiSpecialEventData & MULTIPURPOSE_SPECIAL_EVENT_TEAM_MEMBERS_DONE_TALKING)
			{
				HandleDoneLastEndGameQuote();
			}
		}
		else if (d->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SLEEP)
		{
			if (s == NULL) return;

			// wake merc up or put them back down?
			s->fMercAsleep = (d->uiSpecialEventData != 0);

			// refresh map screen
			fCharacterInfoPanelDirty = TRUE;
			fTeamPanelDirty          = TRUE;
		}
	}

	s = FindSoldierByProfileID(d->ubCharacterNum);
	if (s && s->bTeam == gbPlayerNum)
	{
		CheckForStopTimeQuotes(d->usQuoteNum);
	}

	if (d->fPauseTime) fWasPausedDuringDialogue = TRUE;

	MemFree(d);
}


BOOLEAN DelayedTacticalCharacterDialogue( SOLDIERTYPE *pSoldier, UINT16 usQuoteNum )
{
	if ( pSoldier->ubProfile == NO_PROFILE )
	{
		return( FALSE );
	}

  if (pSoldier->bLife < CONSCIOUSNESS )
   return( FALSE );

	if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
		return( FALSE );

	if ( (AM_A_ROBOT( pSoldier )) )
	{
		return( FALSE );
	}

  if (pSoldier->bLife < OKLIFE && usQuoteNum != QUOTE_SERIOUSLY_WOUNDED )
   return( FALSE );

	if( pSoldier->bAssignment == ASSIGNMENT_POW )
	{
		return( FALSE );
	}

	return CharacterDialogue(pSoldier->ubProfile, usQuoteNum, pSoldier->face, DIALOGUE_TACTICAL_UI, TRUE, TRUE);
}


BOOLEAN TacticalCharacterDialogueWithSpecialEvent(const SOLDIERTYPE* pSoldier, UINT16 usQuoteNum, UINT32 uiFlag, UINT32 uiData1, UINT32 uiData2)
{
	if ( pSoldier->ubProfile == NO_PROFILE )
	{
		return( FALSE );
	}

	if ( uiFlag != DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND && uiData1 != BATTLE_SOUND_DIE1 )
	{
		if (pSoldier->bLife < CONSCIOUSNESS )
		 return( FALSE );

		if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
			return( FALSE );

	}

	return CharacterDialogueWithSpecialEvent(pSoldier->ubProfile, usQuoteNum, pSoldier->face, DIALOGUE_TACTICAL_UI, TRUE, FALSE, uiFlag, uiData1, uiData2);
}


static void CharacterDialogueWithSpecialEventEx(UINT8 ubCharacterNum, UINT16 usQuoteNum, FACETYPE* face, UINT8 bUIHandlerID, BOOLEAN fFromSoldier, BOOLEAN fDelayed, UINT32 uiFlag, UINT32 uiData1, UINT32 uiData2, UINT32 uiData3);


BOOLEAN TacticalCharacterDialogueWithSpecialEventEx( SOLDIERTYPE *pSoldier, UINT16 usQuoteNum, UINT32 uiFlag, UINT32 uiData1, UINT32 uiData2, UINT32 uiData3 )
{
	if ( pSoldier->ubProfile == NO_PROFILE )
	{
		return( FALSE );
	}

	if ( uiFlag != DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND && uiData1 != BATTLE_SOUND_DIE1 )
	{
		if (pSoldier->bLife < CONSCIOUSNESS )
		 return( FALSE );

		if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
			return( FALSE );

		if ( (AM_A_ROBOT( pSoldier )) )
		{
			return( FALSE );
		}

		if (pSoldier->bLife < OKLIFE && usQuoteNum != QUOTE_SERIOUSLY_WOUNDED )
		 return( FALSE );

		if( pSoldier->bAssignment == ASSIGNMENT_POW )
		{
			return( FALSE );
		}

	}

	CharacterDialogueWithSpecialEventEx(pSoldier->ubProfile, usQuoteNum, pSoldier->face, DIALOGUE_TACTICAL_UI, TRUE, FALSE, uiFlag, uiData1, uiData2, uiData3);
	return TRUE;
}


BOOLEAN TacticalCharacterDialogue(const SOLDIERTYPE* pSoldier, UINT16 usQuoteNum)
{
	if ( pSoldier->ubProfile == NO_PROFILE )
	{
		return( FALSE );
	}

	if ( AreInMeanwhile( ) )
	{
		return( FALSE );
	}

  if (pSoldier->bLife < CONSCIOUSNESS )
   return( FALSE );

  if (pSoldier->bLife < OKLIFE && usQuoteNum != QUOTE_SERIOUSLY_WOUNDED )
   return( FALSE );

	if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
		return( FALSE );

	if ( (AM_A_ROBOT( pSoldier )) )
	{
		return( FALSE );
	}

	if( pSoldier->bAssignment == ASSIGNMENT_POW )
	{
		return( FALSE );
	}

	// OK, let's check if this is the exact one we just played, if so, skip.
	if ( pSoldier->ubProfile == gTacticalStatus.ubLastQuoteProfileNUm &&
			 usQuoteNum == gTacticalStatus.ubLastQuoteSaid )
	{
		return( FALSE );
	}


	// If we are a robot, play the controller's quote!
	if ( pSoldier->uiStatusFlags & SOLDIER_ROBOT )
	{
		if ( CanRobotBeControlled( pSoldier ) )
		{
			return TacticalCharacterDialogue(pSoldier->robot_remote_holder, usQuoteNum);
		}
		else
		{
			return( FALSE );
		}
	}

	if ( AM_AN_EPC( pSoldier ) && !(gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags & PROFILE_MISC_FLAG_FORCENPCQUOTE) )
		return( FALSE );

	// Check for logging of me too bleeds...
	if ( usQuoteNum == QUOTE_STARTING_TO_BLEED )
	{
		if ( gubLogForMeTooBleeds )
		{
			// If we are greater than one...
			if ( gubLogForMeTooBleeds > 1 )
			{
				//Replace with me too....
				usQuoteNum = QUOTE_ME_TOO;
			}
			gubLogForMeTooBleeds++;
		}
	}

	return CharacterDialogue(pSoldier->ubProfile, usQuoteNum, pSoldier->face, DIALOGUE_TACTICAL_UI, TRUE, FALSE);
}

// This function takes a profile num, quote num, faceindex and a UI hander ID.
// What it does is queues up the dialog to be ultimately loaded/displayed
//				FACEINDEX
//						The face index is an index into an ACTIVE face. The face is considered to
//						be active, and if it's not, either that has to be handled by the UI handler
//						ir nothing will show.  What this function does is set the face to talking,
//						and the face sprite system should handle the rest.
//				bUIHandlerID
//						Because this could be used in any place, the UI handleID is used to differentiate
//						places in the game. For example, specific things happen in the tactical engine
//						that may not be the place where in the AIM contract screen uses.....

// NB;				The queued system is not yet implemented, but will be transpatent to the caller....


BOOLEAN CharacterDialogueWithSpecialEvent(const UINT8 ubCharacterNum, const UINT16 usQuoteNum, FACETYPE* const face, const UINT8 bUIHandlerID, const BOOLEAN fFromSoldier, const BOOLEAN fDelayed, const UINT32 uiFlag, const UINT32 uiData1, const UINT32 uiData2)
{
	// Allocate new item
	DIALOGUE_Q_STRUCT* QItem = MALLOCZ(DIALOGUE_Q_STRUCT);

	QItem->ubCharacterNum = ubCharacterNum;
	QItem->usQuoteNum			= usQuoteNum;
	QItem->face           = face;
	QItem->bUIHandlerID		= bUIHandlerID;
	QItem->iTimeStamp			= GetJA2Clock( );
	QItem->fFromSoldier		= fFromSoldier;
	QItem->fDelayed				= fDelayed;

	// Set flag for special event
	QItem->uiSpecialEventFlag		= uiFlag;
	QItem->uiSpecialEventData		= uiData1;
	QItem->uiSpecialEventData2	= uiData2;

	ghDialogueQ->Add(QItem);

	if ( uiFlag & DIALOGUE_SPECIAL_EVENT_PCTRIGGERNPC )
	{
		// Increment refrence count...
		giNPCReferenceCount++;
	}

	return( TRUE );
}


// Do special event as well as dialogue!
static void CharacterDialogueWithSpecialEventEx(UINT8 const ubCharacterNum, UINT16 const usQuoteNum, FACETYPE* const face, UINT8 const bUIHandlerID, BOOLEAN const fFromSoldier, BOOLEAN const fDelayed, UINT32 const uiFlag, UINT32 const uiData1, UINT32 const uiData2, UINT32 const uiData3)
{
	// Allocate new item
	DIALOGUE_Q_STRUCT* QItem = MALLOCZ(DIALOGUE_Q_STRUCT);

	QItem->ubCharacterNum = ubCharacterNum;
	QItem->usQuoteNum			= usQuoteNum;
	QItem->face           = face;
	QItem->bUIHandlerID		= bUIHandlerID;
	QItem->iTimeStamp			= GetJA2Clock( );
	QItem->fFromSoldier		= fFromSoldier;
	QItem->fDelayed				= fDelayed;

	// Set flag for special event
	QItem->uiSpecialEventFlag		= uiFlag;
	QItem->uiSpecialEventData		= uiData1;
	QItem->uiSpecialEventData2	= uiData2;
	QItem->uiSpecialEventData3	= uiData3;

	ghDialogueQ->Add(QItem);

	if ( uiFlag & DIALOGUE_SPECIAL_EVENT_PCTRIGGERNPC )
	{
		// Increment refrence count...
		giNPCReferenceCount++;
	}
}


BOOLEAN CharacterDialogue(const UINT8 ubCharacterNum, const UINT16 usQuoteNum, FACETYPE* const face, const UINT8 bUIHandlerID, const BOOLEAN fFromSoldier, const BOOLEAN fDelayed)
{
	// Allocate new item
	DIALOGUE_Q_STRUCT* QItem = MALLOCZ(DIALOGUE_Q_STRUCT);

	QItem->ubCharacterNum = ubCharacterNum;
	QItem->usQuoteNum			= usQuoteNum;
	QItem->face           = face;
	QItem->bUIHandlerID		= bUIHandlerID;
	QItem->iTimeStamp			= GetJA2Clock( );
	QItem->fFromSoldier		= fFromSoldier;
	QItem->fDelayed				= fDelayed;

	// check if pause already locked, if so, then don't mess with it
	if (!gfLockPauseState) QItem->fPauseTime = fPausedTimeDuringQuote;

	fPausedTimeDuringQuote = FALSE;

	ghDialogueQ->Add(QItem);
	return( TRUE );
}


void SpecialCharacterDialogueEvent(UINT32 const uiSpecialEventFlag, UINT32 const uiSpecialEventData1, UINT32 const uiSpecialEventData2, UINT32 const uiSpecialEventData3, FACETYPE* const face, UINT8 const bUIHandlerID)
{
	// Allocate new item
	DIALOGUE_Q_STRUCT* QItem = MALLOCZ(DIALOGUE_Q_STRUCT);

	QItem->uiSpecialEventFlag		= uiSpecialEventFlag;
	QItem->uiSpecialEventData		= uiSpecialEventData1;
	QItem->uiSpecialEventData2	= uiSpecialEventData2;
	QItem->uiSpecialEventData3	= uiSpecialEventData3;
	QItem->face                = face;
	QItem->bUIHandlerID		= bUIHandlerID;
	QItem->iTimeStamp			= GetJA2Clock( );

	// if paused state not already locked
	if (!gfLockPauseState) QItem->fPauseTime = fPausedTimeDuringQuote;

	fPausedTimeDuringQuote = FALSE;

	ghDialogueQ->Add(QItem);
}


void SpecialCharacterDialogueEventWithExtraParam(UINT32 const uiSpecialEventFlag, UINT32 const uiSpecialEventData1, UINT32 const uiSpecialEventData2, UINT32 const uiSpecialEventData3, UINT32 const uiSpecialEventData4, FACETYPE* const face, UINT8 const bUIHandlerID)
{
	// Allocate new item
	DIALOGUE_Q_STRUCT* QItem = MALLOCZ(DIALOGUE_Q_STRUCT);

	QItem->uiSpecialEventFlag		= uiSpecialEventFlag;
	QItem->uiSpecialEventData		= uiSpecialEventData1;
	QItem->uiSpecialEventData2	= uiSpecialEventData2;
	QItem->uiSpecialEventData3	= uiSpecialEventData3;
	QItem->uiSpecialEventData4	= uiSpecialEventData4;
	QItem->face                = face;
	QItem->bUIHandlerID		= bUIHandlerID;
	QItem->iTimeStamp			= GetJA2Clock( );

	// if paused state not already locked
	if (!gfLockPauseState) QItem->fPauseTime = fPausedTimeDuringQuote;

	fPausedTimeDuringQuote = FALSE;

	ghDialogueQ->Add(QItem);
}


static BOOLEAN GetDialogue(UINT8 ubCharacterNum, UINT16 usQuoteNum, UINT32 iDataSize, wchar_t* zDialogueText, size_t Length, CHAR8* zSoundString);


// execute specific character dialogue
static BOOLEAN ExecuteCharacterDialogue(const UINT8 ubCharacterNum, const UINT16 usQuoteNum, FACETYPE* const face, const UINT8 bUIHandlerID, const BOOLEAN fFromSoldier)
{
	CHAR8		zSoundString[ 164 ];

	// Check if we are dead now or not....( if from a soldier... )

	// Try to find soldier...
	const SOLDIERTYPE* const pSoldier = FindSoldierByProfileIDOnPlayerTeam(ubCharacterNum);
	if ( pSoldier != NULL )
	{
		// Check vital stats
		if (pSoldier->bLife < CONSCIOUSNESS )
		{
			return( FALSE );
		}

		if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
			return( FALSE );

		if ( (AM_A_ROBOT( pSoldier )) )
		{
			return( FALSE );
		}

		if (pSoldier->bLife < OKLIFE && usQuoteNum != QUOTE_SERIOUSLY_WOUNDED )
		{
			return( FALSE );
		}

		if( pSoldier->bAssignment == ASSIGNMENT_POW )
		{
			return( FALSE );
		}

		// sleeping guys don't talk.. go to standby to talk
		if (pSoldier->fMercAsleep)
		{
			// check if the soldier was compaining about lack of sleep and was alseep, if so, leave them alone
			if( ( usQuoteNum == QUOTE_NEED_SLEEP ) || ( usQuoteNum == QUOTE_OUT_OF_BREATH ) )
			{
				// leave them alone
				return ( TRUE );
			}

			// may want to wake up any character that has VERY important dialogue to say
			// MC to flesh out

		}

		// now being used in a different way...
		/*
		if ( ( (usQuoteNum == QUOTE_PERSONALITY_TRAIT &&
					(gMercProfiles[ubCharacterNum].bPersonalityTrait == FORGETFUL ||
					 gMercProfiles[ubCharacterNum].bPersonalityTrait == CLAUSTROPHOBIC ||
					 gMercProfiles[ubCharacterNum].bPersonalityTrait == NERVOUS ||
					 gMercProfiles[ubCharacterNum].bPersonalityTrait == NONSWIMMER ||
					 gMercProfiles[ubCharacterNum].bPersonalityTrait == FEAR_OF_INSECTS))
					//usQuoteNum == QUOTE_STARTING_TO_WHINE ||
#ifdef JA2BETAVERSION
					|| usQuoteNum == QUOTE_WHINE_EQUIPMENT) && (guiCurrentScreen != QUEST_DEBUG_SCREEN) )
#else
          ) )
#endif

		{
			// This quote might spawn another quote from someone
			FOR_ALL_IN_TEAM(s, gbPlayerNum)
			{
				if (s->ubProfile != ubCharacterNum &&
						OkControllableMerc(s) &&
						SpacesAway(pSoldier->sGridNo, s->sGridNo) < 5)
				{
					// if this merc disliked the whining character sufficiently and hasn't already retorted
					if (gMercProfiles[s->ubProfile].bMercOpinion[ubCharacterNum] < -2 &&
							!(s->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_ANNOYING_MERC))
					{
						// make a comment!
						TacticalCharacterDialogue(s, QUOTE_ANNOYING_PC);
						s->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_ANNOYING_MERC;
						break;
					}
				}
			}
		}
		*/
	}
	else
	{
		// If from a soldier, and he does not exist anymore, donot play!
		if ( fFromSoldier )
		{
			return( FALSE );
		}
	}

	// Check face index
	CHECKF(face != NULL);

  if (!GetDialogue(ubCharacterNum, usQuoteNum, DIALOGUESIZE, gzQuoteStr, lengthof(gzQuoteStr), zSoundString))
  {
    return( FALSE );
  }

	if( bUIHandlerID == DIALOGUE_EXTERNAL_NPC_UI )
	{
		// external NPC
		SetFaceTalking(face, zSoundString, gzQuoteStr);
	}
	else
	{
		// start "talking" system (portrait animation and start wav sample)
		SetFaceTalking(face, zSoundString, gzQuoteStr);
	}
	CreateTalkingUI(bUIHandlerID, face, ubCharacterNum, gzQuoteStr);

	// Set global handleer ID value, used when face desides it's done...
	gbUIHandlerID = bUIHandlerID;

	guiScreenIDUsedWhenUICreated = guiCurrentScreen;

	return( TRUE );
}


static void DisplayTextForExternalNPC(UINT8 ubCharacterNum, const wchar_t* zQuoteStr);
static void HandleExternNPCSpeechFace(FACETYPE* face);
static void HandleTacticalNPCTextUI(UINT8 ubCharacterNum, const wchar_t* zQuoteStr);
static void HandleTacticalTextUI(ProfileID profile_id, const wchar_t* zQuoteStr);


static void CreateTalkingUI(const INT8 bUIHandlerID, FACETYPE* const face, const UINT8 ubCharacterNum, const wchar_t* const zQuoteStr)
{
	// Show text, if on
	if (gGameSettings.fOptions[TOPTION_SUBTITLES] || !face->fValidSpeech)
	{
		switch (bUIHandlerID)
		{
			case DIALOGUE_TACTICAL_UI:           HandleTacticalTextUI(            ubCharacterNum, zQuoteStr); break;
			case DIALOGUE_NPC_UI:                HandleTacticalNPCTextUI(         ubCharacterNum, zQuoteStr); break;
			case DIALOGUE_CONTACTPAGE_UI:        DisplayTextForMercFaceVideoPopUp(                zQuoteStr); break;
			case DIALOGUE_SPECK_CONTACT_PAGE_UI: DisplayTextForSpeckVideoPopUp(                   zQuoteStr); break;
			case DIALOGUE_EXTERNAL_NPC_UI:       DisplayTextForExternalNPC(       ubCharacterNum, zQuoteStr); break;
			case DIALOGUE_SHOPKEEPER_UI:         InitShopKeeperSubTitledText(                     zQuoteStr); break;
		}
	}

	if (gGameSettings.fOptions[TOPTION_SPEECH])
	{
		switch (bUIHandlerID)
		{
			case DIALOGUE_TACTICAL_UI:           HandleTacticalSpeechUI(ubCharacterNum, face); break;
			case DIALOGUE_CONTACTPAGE_UI:                                                      break;
			case DIALOGUE_SPECK_CONTACT_PAGE_UI:                                               break;
			case DIALOGUE_EXTERNAL_NPC_UI:       HandleExternNPCSpeechFace(face);              break;
		}
	}
}


const char* GetDialogueDataFilename(UINT8 ubCharacterNum, UINT16 usQuoteNum, BOOLEAN fWavFile)
{
	static char zFileName[164];
	UINT8		ubFileNumID;

	// Are we an NPC OR an RPC that has not been recruited?
	// ATE: Did the || clause here to allow ANY RPC that talks while the talking menu is up to use an npc quote file
	if ( gfUseAlternateDialogueFile )
	{
		if ( fWavFile )
		{
			// build name of wav file (characternum + quotenum)
			sprintf(zFileName, "NPC_SPEECH/d_%03d_%03d.wav", ubCharacterNum, usQuoteNum);
		}
		else
		{
			// assume EDT files are in EDT directory on HARD DRIVE
			sprintf( zFileName,"NPCDATA/d_%03d.EDT", ubCharacterNum );
		}
	}
	else if ( ubCharacterNum >= FIRST_RPC &&
			( !( gMercProfiles[ ubCharacterNum ].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED )
			|| ProfileCurrentlyTalkingInDialoguePanel( ubCharacterNum )
			|| (gMercProfiles[ ubCharacterNum ].ubMiscFlags & PROFILE_MISC_FLAG_FORCENPCQUOTE) )
			)
	{
		ubFileNumID = ubCharacterNum;

		// ATE: If we are merc profile ID #151-154, all use 151's data....
		if (ubCharacterNum >= HERVE && ubCharacterNum <= CARLO)
		{
			ubFileNumID = HERVE;
		}

		// If we are character #155, check fact!
		if ( ubCharacterNum == MANNY && !gubFact[FACT_MANNY_IS_BARTENDER] )
		{
			ubFileNumID = MANNY;
		}


		if ( fWavFile )
		{
			sprintf( zFileName,"NPC_SPEECH/%03d_%03d.wav",ubFileNumID,usQuoteNum );
		}
		else
		{
		// assume EDT files are in EDT directory on HARD DRIVE
			sprintf( zFileName,"NPCDATA/%03d.EDT", ubFileNumID );
		}
	}
	else
	{
		if ( fWavFile )
		{
#if defined RUSSIAN || defined RUSSIAN_GOLD
			if (ubCharacterNum >= FIRST_RPC && gMercProfiles[ubCharacterNum].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED)
			{
				sprintf(zFileName, "SPEECH/r_%03d_%03d.wav", ubCharacterNum, usQuoteNum);
			}
			else
#endif
			{	// build name of wav file (characternum + quotenum)
				sprintf( zFileName,"SPEECH/%03d_%03d.wav",ubCharacterNum,usQuoteNum );
			}
		}
		else
		{
			// assume EDT files are in EDT directory on HARD DRIVE
			sprintf( zFileName,"MERCEDT/%03d.EDT", ubCharacterNum );
		}
	}

	return( zFileName );
}


static BOOLEAN GetDialogue(UINT8 ubCharacterNum, UINT16 usQuoteNum, UINT32 iDataSize, wchar_t* zDialogueText, size_t Length, CHAR8* zSoundString)
{
   // first things first  - grab the text (if player has SUBTITLE PREFERENCE ON)
   //if ( gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
   {
			const char* pFilename = GetDialogueDataFilename(ubCharacterNum, 0, FALSE);
			if (!LoadEncryptedDataFromFile(pFilename, zDialogueText, usQuoteNum * iDataSize, iDataSize) ||
					zDialogueText[0] == L'\0')
			{
				swprintf(zDialogueText, Length, L"I have no text in the EDT file (%d) %hs", usQuoteNum, pFilename);
#ifndef JA2BETAVERSION
				return( FALSE );
#endif
			}
   }

	// CHECK IF THE FILE EXISTS, IF NOT, USE DEFAULT!
	const char* pFilename = GetDialogueDataFilename(ubCharacterNum, usQuoteNum, TRUE);
	strcpy( zSoundString, pFilename );
 return(TRUE);
}


// Handlers for tactical UI stuff
static void HandleTacticalNPCTextUI(const UINT8 ubCharacterNum, const wchar_t* const zQuoteStr)
{
	wchar_t zText[ QUOTE_MESSAGE_SIZE ];

	// Setup dialogue text box
	if ( guiCurrentScreen != MAP_SCREEN )
	{
		gTalkPanel.fRenderSubTitlesNow = TRUE;
		gTalkPanel.fSetupSubTitles = TRUE;
	}

	// post message to mapscreen message system
	swprintf( gTalkPanel.zQuoteStr, lengthof(gTalkPanel.zQuoteStr), L"\"%ls\"", zQuoteStr );
	swprintf( zText, lengthof(zText), L"%ls: \"%ls\"", gMercProfiles[ ubCharacterNum ].zNickname, zQuoteStr );
	MapScreenMessage( FONT_MCOLOR_WHITE, MSG_DIALOG, L"%ls",  zText );
}


static void ExecuteTacticalTextBox(INT16 sLeftPosition, const wchar_t* pString);


// Handlers for tactical UI stuff
static void DisplayTextForExternalNPC(const UINT8 ubCharacterNum, const wchar_t* const zQuoteStr)
{
	wchar_t								zText[ QUOTE_MESSAGE_SIZE ];
	INT16									sLeft;


	// Setup dialogue text box
	if ( guiCurrentScreen != MAP_SCREEN )
	{
		gTalkPanel.fRenderSubTitlesNow = TRUE;
		gTalkPanel.fSetupSubTitles = TRUE;
	}

	// post message to mapscreen message system
	swprintf( gTalkPanel.zQuoteStr, lengthof(gTalkPanel.zQuoteStr), L"\"%ls\"", zQuoteStr );
	swprintf( zText, lengthof(zText), L"%ls: \"%ls\"", gMercProfiles[ ubCharacterNum ].zNickname, zQuoteStr );
	MapScreenMessage( FONT_MCOLOR_WHITE, MSG_DIALOG, L"%ls",  zText );

	if ( guiCurrentScreen == MAP_SCREEN )
	{
  	sLeft			 = ( gsExternPanelXPosition + 97 );
		gsTopPosition = gsExternPanelYPosition;
	}
  else
  {
	  sLeft			 = ( 110 );
  }

	ExecuteTacticalTextBox( sLeft, gTalkPanel.zQuoteStr );
}


static void HandleTacticalTextUI(const ProfileID profile_id, const wchar_t* const zQuoteStr)
{
	wchar_t								zText[ QUOTE_MESSAGE_SIZE ];
	INT16									sLeft = 0;

	//BUild text
	// How do we do this with defines?
	//swprintf(zText, L"\xb4\xa2 %ls: \xb5 \"%ls\"", gMercProfiles[ubCharacterNum].zNickname, zQuoteStr);
	swprintf( zText, lengthof(zText), L"\"%ls\"", zQuoteStr );
	sLeft	= 110;


	//previous version
	//sLeft = 110;

	ExecuteTacticalTextBox( sLeft, zText );

	swprintf(zText, lengthof(zText), L"%ls: \"%ls\"", gMercProfiles[profile_id].zNickname, zQuoteStr);
	MapScreenMessage( FONT_MCOLOR_WHITE, MSG_DIALOG, L"%ls",  zText );
}


static void ExecuteTacticalTextBoxForLastQuote(const INT16 sLeftPosition, const wchar_t* const pString)
{
	UINT32 uiDelay = FindDelayForString( pString );

	fDialogueBoxDueToLastMessage = TRUE;

	guiDialogueLastQuoteTime = GetJA2Clock();

	guiDialogueLastQuoteDelay = ( ( uiDelay < FINAL_TALKING_DURATION ) ?  FINAL_TALKING_DURATION : uiDelay );

	// now execute box
	ExecuteTacticalTextBox(sLeftPosition, pString );
}


static void RenderSubtitleBoxOverlay(VIDEO_OVERLAY* pBlitter);
static void TextOverlayClickCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void ExecuteTacticalTextBox(const INT16 sLeftPosition, const wchar_t* const pString)
{
	VIDEO_OVERLAY_DESC		VideoOverlayDesc;

	// check if mouse region created, if so, do not recreate
	if (fTextBoxMouseRegionCreated) return;

	memset( &VideoOverlayDesc, 0, sizeof( VIDEO_OVERLAY_DESC ) );

	// Prepare text box
	iDialogueBox = PrepareMercPopupBox( iDialogueBox , BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, pString, DIALOGUE_DEFAULT_SUBTITLE_WIDTH, 0, 0, 0, &gusSubtitleBoxWidth, &gusSubtitleBoxHeight );

	VideoOverlayDesc.sLeft			 = sLeftPosition;
	VideoOverlayDesc.sTop				 = gsTopPosition;
	VideoOverlayDesc.sRight			 = VideoOverlayDesc.sLeft + gusSubtitleBoxWidth;
	VideoOverlayDesc.sBottom		 = VideoOverlayDesc.sTop + gusSubtitleBoxHeight;
	VideoOverlayDesc.BltCallback = RenderSubtitleBoxOverlay;
	g_text_box_overlay = RegisterVideoOverlay(0, &VideoOverlayDesc);

	gsTopPosition = 20;

	//Define main region
	MSYS_DefineRegion( &gTextBoxMouseRegion,  VideoOverlayDesc.sLeft, VideoOverlayDesc.sTop,  VideoOverlayDesc.sRight, VideoOverlayDesc.sBottom, MSYS_PRIORITY_HIGHEST,
						 CURSOR_NORMAL, MSYS_NO_CALLBACK, TextOverlayClickCallback );

	fTextBoxMouseRegionCreated = TRUE;
}


static void FaceOverlayClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void RenderFaceOverlay(VIDEO_OVERLAY* pBlitter);


static void HandleExternNPCSpeechFace(FACETYPE* const face)
{
	VIDEO_OVERLAY_DESC		VideoOverlayDesc;

	// Enable it!
	SetAutoFaceActive(FACE_AUTO_DISPLAY_BUFFER, FACE_AUTO_RESTORE_BUFFER, face, 0, 0);

	// Set flag to say WE control when to set inactive!
	face->uiFlags |= FACE_INACTIVE_HANDLED_ELSEWHERE;

	if ( guiCurrentScreen != MAP_SCREEN )
	{
		// Setup video overlay!
		VideoOverlayDesc.sLeft			 = 10;
		VideoOverlayDesc.sTop				 = 20;
		VideoOverlayDesc.sRight			 = VideoOverlayDesc.sLeft + 99;
		VideoOverlayDesc.sBottom		 = VideoOverlayDesc.sTop + 98;
		VideoOverlayDesc.BltCallback = RenderFaceOverlay;
	}
	else
	{
		// Setup video overlay!

		VideoOverlayDesc.sLeft			 = gsExternPanelXPosition;
		VideoOverlayDesc.sTop				 = gsExternPanelYPosition;

		VideoOverlayDesc.sRight			 = VideoOverlayDesc.sLeft + 99;
		VideoOverlayDesc.sBottom		 = VideoOverlayDesc.sTop + 98;
		VideoOverlayDesc.BltCallback = RenderFaceOverlay;
	}

	gpCurrentTalkingFace->video_overlay = RegisterVideoOverlay(0, &VideoOverlayDesc);

	RenderAutoFace(face);

	// ATE: Create mouse region.......
	if ( !fExternFaceBoxRegionCreated )
	{
		fExternFaceBoxRegionCreated = TRUE;

		//Define main region
		MSYS_DefineRegion( &gFacePopupMouseRegion,  VideoOverlayDesc.sLeft, VideoOverlayDesc.sTop,  VideoOverlayDesc.sRight, VideoOverlayDesc.sBottom, MSYS_PRIORITY_HIGHEST,
							 CURSOR_NORMAL, MSYS_NO_CALLBACK, FaceOverlayClickCallback );
	}

	gfFacePanelActive = TRUE;
}


static void HandleTacticalSpeechUI(const UINT8 ubCharacterNum, FACETYPE* const face)
{
	VIDEO_OVERLAY_DESC		VideoOverlayDesc;
	BOOLEAN								fDoExternPanel = FALSE;

	memset( &VideoOverlayDesc, 0, sizeof( VIDEO_OVERLAY_DESC ) );

	// Get soldier pointer, if there is one...
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubCharacterNum);

	// PLEASE NOTE:  pSoldier may legally be NULL (e.g. Skyrider) !!!

	if ( pSoldier == NULL )
	{
		fDoExternPanel = TRUE;
	}
	else
	{
		// If we are not an active face!
		if ( guiCurrentScreen != MAP_SCREEN )
		{
			fDoExternPanel = TRUE;
		}
	}

	if ( fDoExternPanel )
	{
		// Enable it!
		SetAutoFaceActive(FACE_AUTO_DISPLAY_BUFFER, FACE_AUTO_RESTORE_BUFFER, face, 0, 0);

		// Set flag to say WE control when to set inactive!
		face->uiFlags |= FACE_INACTIVE_HANDLED_ELSEWHERE | FACE_MAKEACTIVE_ONCE_DONE;

		// IF we are in tactical and this soldier is on the current squad
		if ( ( guiCurrentScreen == GAME_SCREEN ) && ( pSoldier != NULL ) && ( pSoldier->bAssignment == iCurrentTacticalSquad ) )
		{
			// Make the interface panel dirty..
			// This will dirty the panel next frame...
			gfRerenderInterfaceFromHelpText = TRUE;
		}

		// Setup video overlay!
		VideoOverlayDesc.sLeft			 = 10;
		VideoOverlayDesc.sTop				 = 20;
		VideoOverlayDesc.sRight			 = VideoOverlayDesc.sLeft + 99;
		VideoOverlayDesc.sBottom		 = VideoOverlayDesc.sTop + 98;
		VideoOverlayDesc.BltCallback = RenderFaceOverlay;

		gpCurrentTalkingFace->video_overlay = RegisterVideoOverlay(0, &VideoOverlayDesc);

		RenderAutoFace(face);

		// ATE: Create mouse region.......
		if ( !fExternFaceBoxRegionCreated )
		{
			fExternFaceBoxRegionCreated = TRUE;

			//Define main region
			MSYS_DefineRegion( &gFacePopupMouseRegion,  VideoOverlayDesc.sLeft, VideoOverlayDesc.sTop,  VideoOverlayDesc.sRight, VideoOverlayDesc.sBottom, MSYS_PRIORITY_HIGHEST,
								 CURSOR_NORMAL, MSYS_NO_CALLBACK, FaceOverlayClickCallback );
		}

		gfFacePanelActive = TRUE;

	}
	else if ( guiCurrentScreen == MAP_SCREEN )
	{
		// Are we in mapscreen?
		// If so, set current guy active to talk.....
		if ( pSoldier != NULL )
		{
			ContinueDialogue( pSoldier, FALSE );
		}
	}

}


void HandleDialogueEnd( FACETYPE *pFace )
{
	if ( gGameSettings.fOptions[ TOPTION_SPEECH ] )
	{

		if ( pFace != gpCurrentTalkingFace )
		{
			//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"HandleDialogueEnd() face mismatch." );
			return;
		}

		if ( pFace->fTalking )
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"HandleDialogueEnd() face still talking." );
			return;
		}

		switch( gbUIHandlerID )
		{
			case DIALOGUE_TACTICAL_UI:

			if ( gfFacePanelActive )
			{
				// Set face inactive!
				pFace->fCanHandleInactiveNow = TRUE;
				SetAutoFaceInActive(pFace);
				gfFacePanelActive = FALSE;

				if ( fExternFaceBoxRegionCreated )
				{
					fExternFaceBoxRegionCreated = FALSE;
					MSYS_RemoveRegion(&(gFacePopupMouseRegion) );
				}
			}
			break;
			case DIALOGUE_NPC_UI:
			break;
			case DIALOGUE_EXTERNAL_NPC_UI:
				pFace->fCanHandleInactiveNow = TRUE;
				SetAutoFaceInActive(pFace);
				gfFacePanelActive = FALSE;

				if ( fExternFaceBoxRegionCreated )
				{
					fExternFaceBoxRegionCreated = FALSE;
					MSYS_RemoveRegion(&(gFacePopupMouseRegion) );
				}

			break;
		}
	}


  if ( gGameSettings.fOptions[ TOPTION_SUBTITLES ] || !pFace->fValidSpeech )
	{
		switch( gbUIHandlerID )
		{
			case DIALOGUE_TACTICAL_UI:
			case DIALOGUE_EXTERNAL_NPC_UI:
				// Remove if created
				if (g_text_box_overlay != NULL)
				{
					RemoveVideoOverlay(g_text_box_overlay);
					g_text_box_overlay = NULL;

					if ( fTextBoxMouseRegionCreated )
					{
						RemoveMercPopupBoxFromIndex( iDialogueBox );

						// reset box id
						iDialogueBox = -1;
						MSYS_RemoveRegion( &gTextBoxMouseRegion );
						fTextBoxMouseRegionCreated = FALSE;
					}

				}

				break;

			case DIALOGUE_NPC_UI:


				// Remove region
				if ( gTalkPanel.fTextRegionOn )
				{
					MSYS_RemoveRegion(&(gTalkPanel.TextRegion) );
					gTalkPanel.fTextRegionOn = FALSE;

				}

				SetRenderFlags( RENDER_FLAG_FULL );
				gTalkPanel.fRenderSubTitlesNow = FALSE;

				// Delete subtitle box
				RemoveMercPopupBoxFromIndex( iInterfaceDialogueBox );
				iInterfaceDialogueBox = -1;
				break;

			case DIALOGUE_CONTACTPAGE_UI:
				break;

			case DIALOGUE_SPECK_CONTACT_PAGE_UI:
				break;



		}
	}

  TurnOffSectorLocator();

  gsExternPanelXPosition     = DEFAULT_EXTERN_PANEL_X_POS;
  gsExternPanelYPosition     = DEFAULT_EXTERN_PANEL_Y_POS;

}


static void RenderFaceOverlay(VIDEO_OVERLAY* pBlitter)
{
	INT16 sFontX, sFontY;
	wchar_t					zTownIDString[50];


	if ( gpCurrentTalkingFace == NULL )
	{
		return;
	}

	if ( gfFacePanelActive )
	{
		const SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(gpCurrentTalkingFace->ubCharacterNum);

		// a living soldier?..or external NPC?..choose panel based on this
		const SGPVObject* const vo = (pSoldier ? guiCOMPANEL : guiCOMPANELB);
		BltVideoObject(pBlitter->uiDestBuff, vo, 0, pBlitter->sX, pBlitter->sY);

		// Display name, location ( if not current )
		SetFont( BLOCKFONT2 );
		SetFontBackground( FONT_MCOLOR_BLACK );
		SetFontForeground( FONT_MCOLOR_LTGRAY );

		if ( pSoldier )
		{
		  //reset the font dest buffer
		  SetFontDestBuffer(pBlitter->uiDestBuff, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

			FindFontCenterCoordinates(pBlitter->sX + 12, pBlitter->sY + 55, 73, 9, pSoldier->name, BLOCKFONT2, &sFontX, &sFontY);
			MPrint(sFontX, sFontY, pSoldier->name);

			// What sector are we in, ( and is it the same as ours? )
			if ( pSoldier->sSectorX != gWorldSectorX || pSoldier->sSectorY != gWorldSectorY || pSoldier->bSectorZ != gbWorldSectorZ || pSoldier->fBetweenSectors )
			{
				GetSectorIDString( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ, zTownIDString, lengthof(zTownIDString), FALSE );

        ReduceStringLength( zTownIDString, lengthof(zTownIDString), 64 , BLOCKFONT2 );

				FindFontCenterCoordinates(pBlitter->sX + 12, pBlitter->sY + 68, 73, 9, zTownIDString, BLOCKFONT2, &sFontX, &sFontY);
				MPrint(sFontX, sFontY, zTownIDString);
			}

		  //reset the font dest buffer
			SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

			DrawSoldierUIBars(pSoldier, pBlitter->sX + 69, pBlitter->sY + 47, FALSE, pBlitter->uiDestBuff);
		}
		else
		{
			FindFontCenterCoordinates(pBlitter->sX + 9, pBlitter->sY + 55, 73, 9, gMercProfiles[gpCurrentTalkingFace->ubCharacterNum].zNickname, BLOCKFONT2, &sFontX, &sFontY);
			MPrint(sFontX, sFontY, gMercProfiles[gpCurrentTalkingFace->ubCharacterNum].zNickname);
		}

		//RenderAutoFace( gpCurrentTalkingFace->iID );

		const SGPRect r = { 0, 0, gpCurrentTalkingFace->usFaceWidth, gpCurrentTalkingFace->usFaceHeight };
		BltVideoSurface(pBlitter->uiDestBuff, gpCurrentTalkingFace->uiAutoDisplayBuffer, pBlitter->sX + 14, pBlitter->sY + 6, &r);

		InvalidateRegion( pBlitter->sX, pBlitter->sY, pBlitter->sX + 99, pBlitter->sY + 98 );
	}
}


static void RenderSubtitleBoxOverlay(VIDEO_OVERLAY* pBlitter)
{
	if (g_text_box_overlay == NULL) return;

	RenderMercPopUpBoxFromIndex(iDialogueBox, pBlitter->sX, pBlitter->sY, pBlitter->uiDestBuff);
	InvalidateRegion(pBlitter->sX, pBlitter->sY, pBlitter->sX + gusSubtitleBoxWidth, pBlitter->sY + gusSubtitleBoxHeight);
}


/* Let Red talk, if he is in the list and the quote is QUOTE_AIR_RAID.  Choose
 * somebody else otherwise */
static void ChooseRedIfPresentAndAirRaid(SOLDIERTYPE*const*const mercs_in_sector, size_t merc_count, UINT16 quote)
{
	if (merc_count == 0) return;

	SOLDIERTYPE* chosen;
	if (quote == QUOTE_AIR_RAID)
	{
		for (SOLDIERTYPE*const* i = mercs_in_sector; i != mercs_in_sector + merc_count; ++i)
		{
			if ((*i)->ubProfile == RED)
			{
				chosen = *i;
				goto talk;
			}
		}
	}
	chosen = mercs_in_sector[Random(merc_count)];
talk:
	TacticalCharacterDialogue(chosen, quote);
}


void SayQuoteFromAnyBodyInSector( UINT16 usQuoteNum )
{
	// Loop through all our guys and randomly say one from someone in our sector
	size_t merc_count = 0;
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		// Add guy if he's a candidate...
		if (OkControllableMerc(s) &&
				!AM_AN_EPC(s) &&
				!(s->uiStatusFlags & SOLDIER_GASSED) &&
				!AM_A_ROBOT(s) &&
				!s->fMercAsleep)
		{
			if (gTacticalStatus.bNumFoughtInBattle[ENEMY_TEAM] == 0)
			{
				// quotes referring to Deidranna's men so we skip quote if there were no army guys fought
				if (usQuoteNum == QUOTE_SECTOR_SAFE &&
						(s->ubProfile == IRA || s->ubProfile == MIGUEL || s->ubProfile == SHANK))
				{
					continue;
				}
				if (usQuoteNum == QUOTE_ENEMY_PRESENCE &&
						(s->ubProfile == IRA || s->ubProfile == DIMITRI || s->ubProfile == DYNAMO || s->ubProfile == SHANK))
				{
					continue;
				}
			}

			mercs_in_sector[merc_count++] = s;
		}
	}

	ChooseRedIfPresentAndAirRaid(mercs_in_sector, merc_count, usQuoteNum);
}


void SayQuoteFromAnyBodyInThisSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, UINT16 usQuoteNum )
{
	// Loop through all our guys and randomly say one from someone in our sector
	size_t merc_count = 0;
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		// Add guy if he's a candidate...
		if (s->sSectorX == sSectorX && s->sSectorY == sSectorY && s->bSectorZ == bSectorZ && !AM_AN_EPC(s) && !(s->uiStatusFlags & SOLDIER_GASSED) && !AM_A_ROBOT(s) && !s->fMercAsleep)
		{
			mercs_in_sector[merc_count++] = s;
		}
	}

	ChooseRedIfPresentAndAirRaid(mercs_in_sector, merc_count, usQuoteNum);
}


void SayQuoteFromNearbyMercInSector( INT16 sGridNo, INT8 bDistance, UINT16 usQuoteNum )
{
	UINT8	ubNumMercs = 0;

	// Loop through all our guys and randomly say one from someone in our sector

	// run through list
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		// Add guy if he's a candidate...
		if (OkControllableMerc(s) &&
				PythSpacesAway(sGridNo, s->sGridNo) < bDistance &&
				!AM_AN_EPC(s) &&
				!(s->uiStatusFlags & SOLDIER_GASSED) &&
				!AM_A_ROBOT(s) &&
				!s->fMercAsleep &&
				SoldierTo3DLocationLineOfSightTest(s, sGridNo, 0, 0, MaxDistanceVisible(), TRUE))
		{
			if (usQuoteNum == 66 && Random(100) > EffectiveWisdom(s))
			{
				continue;
			}
			mercs_in_sector[ubNumMercs++] = s;
		}
	}

	if ( ubNumMercs > 0 )
	{
		SOLDIERTYPE* const chosen = mercs_in_sector[Random(ubNumMercs)];
		if (usQuoteNum == 66)
		{
			SetFactTrue( FACT_PLAYER_FOUND_ITEMS_MISSING );
		}
		TacticalCharacterDialogue(chosen, usQuoteNum);
	}
}


void SayQuote58FromNearbyMercInSector( INT16 sGridNo, INT8 bDistance, UINT16 usQuoteNum, INT8 bSex )
{
	UINT8	ubNumMercs = 0;

	// Loop through all our guys and randomly say one from someone in our sector

	// run through list
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		// Add guy if he's a candidate...
		if (OkControllableMerc(s) &&
				PythSpacesAway(sGridNo, s->sGridNo) < bDistance &&
				!AM_AN_EPC(s) &&
				!(s->uiStatusFlags & SOLDIER_GASSED) &&
				!AM_A_ROBOT(s) &&
				!s->fMercAsleep &&
				SoldierTo3DLocationLineOfSightTest(s, sGridNo, 0, 0, MaxDistanceVisible(), TRUE))
		{
			// ATE: This is to check gedner for this quote...
			if (QuoteExp_GenderCode[s->ubProfile] == 0 && bSex == FEMALE)
			{
				continue;
			}

			if (QuoteExp_GenderCode[s->ubProfile] == 1 && bSex == MALE)
			{
				continue;
			}

			mercs_in_sector[ubNumMercs++] = s;
		}
	}

	if ( ubNumMercs > 0 )
	{
		SOLDIERTYPE* const chosen = mercs_in_sector[Random(ubNumMercs)];
		TacticalCharacterDialogue(chosen, usQuoteNum);
	}
}


static void TextOverlayClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	static BOOLEAN fLButtonDown = FALSE;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fLButtonDown = TRUE;
	}

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP && fLButtonDown )
	{
		if(  gpCurrentTalkingFace != NULL )
		{
			InternalShutupaYoFace(gpCurrentTalkingFace, FALSE);

			// Did we succeed in shutting them up?
			if ( !gpCurrentTalkingFace->fTalking )
			{
				// shut down last quote box
				ShutDownLastQuoteTacticalTextBox( );
			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fLButtonDown = FALSE;
	}
}


static void FaceOverlayClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	static BOOLEAN fLButtonDown = FALSE;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fLButtonDown = TRUE;
	}

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP && fLButtonDown )
	{
		if(  gpCurrentTalkingFace != NULL )
		{
			InternalShutupaYoFace(gpCurrentTalkingFace, FALSE);
		}

	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fLButtonDown = FALSE;
	}
}

void ShutDownLastQuoteTacticalTextBox( void )
{
	if( fDialogueBoxDueToLastMessage )
	{
		RemoveVideoOverlay(g_text_box_overlay);
		g_text_box_overlay = NULL;

		if ( fTextBoxMouseRegionCreated )
		{
			MSYS_RemoveRegion( &gTextBoxMouseRegion );
			fTextBoxMouseRegionCreated = FALSE;
		}

		fDialogueBoxDueToLastMessage = FALSE;
	}
}


UINT32 FindDelayForString(const wchar_t* const sString)
{
	return( wcslen( sString ) * TEXT_DELAY_MODIFIER );
}

void BeginLoggingForBleedMeToos( BOOLEAN fStart )
{
	gubLogForMeTooBleeds = fStart;
}


void SetEngagedInConvFromPCAction( SOLDIERTYPE *pSoldier )
{
	// OK, If a good give, set engaged in conv...
	gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;
	gTacticalStatus.ubEngagedInConvFromActionMercID = pSoldier->ubID;
}

void UnSetEngagedInConvFromPCAction( SOLDIERTYPE *pSoldier )
{
	if ( gTacticalStatus.ubEngagedInConvFromActionMercID == pSoldier->ubID )
	{
		// OK, If a good give, set engaged in conv...
		gTacticalStatus.uiFlags &= ( ~ENGAGED_IN_CONV );
	}
}


static BOOLEAN IsStopTimeQuote(UINT16 usQuoteNum)
{
	INT32 cnt;

	for ( cnt = 0; cnt < gubNumStopTimeQuotes; cnt++ )
	{
		if ( gusStopTimeQuoteList[ cnt ] == usQuoteNum )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


static void CheckForStopTimeQuotes(UINT16 usQuoteNum)
{
	if ( IsStopTimeQuote( usQuoteNum ) )
	{
		// Stop Time, game
		EnterModalTactical( TACTICAL_MODAL_NOMOUSE );

		gpCurrentTalkingFace->uiFlags		|= FACE_MODAL;

		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Starting Modal Tactical Quote." );

	}
}

void SetStopTimeQuoteCallback( MODAL_HOOK pCallBack )
{
	gModalDoneCallback = pCallBack;
}


BOOLEAN IsMercSayingDialogue(UINT8 ubProfileID)
{
	if ( gpCurrentTalkingFace != NULL && gubCurrentTalkingID == ubProfileID )
	{
		return( TRUE );
	}
	return( FALSE );
}


BOOLEAN GetMercPrecedentQuoteBitStatus(const MERCPROFILESTRUCT* const p, UINT8 const ubQuoteBit)
{
	return (p->uiPrecedentQuoteSaid & 1 << (ubQuoteBit - 1)) != 0;
}


void SetMercPrecedentQuoteBitStatus(MERCPROFILESTRUCT* const p, UINT8 const ubBitToSet)
{
	p->uiPrecedentQuoteSaid |= 1 << (ubBitToSet - 1);
}


UINT8	GetQuoteBitNumberFromQuoteID( UINT32 uiQuoteID )
{
	UINT8 ubCnt;

	//loop through all the quotes
	for( ubCnt=0; ubCnt<NUMBER_VALID_MERC_PRECEDENT_QUOTES;ubCnt++)
	{
		if( gubMercValidPrecedentQuoteID[ ubCnt ] == uiQuoteID )
		{
			return( ubCnt );
		}
	}

	return( 0 );
}

void HandleShutDownOfMapScreenWhileExternfaceIsTalking( void )
{
	if ( ( fExternFaceBoxRegionCreated ) && ( gpCurrentTalkingFace) )
	{
		RemoveVideoOverlay(gpCurrentTalkingFace->video_overlay);
		gpCurrentTalkingFace->video_overlay = NULL;
	}
}


void HandleImportantMercQuote( SOLDIERTYPE * pSoldier, UINT16 usQuoteNumber )
{
	// wake merc up for THIS quote
	if( pSoldier->fMercAsleep )
	{
		TacticalCharacterDialogueWithSpecialEvent( pSoldier, usQuoteNumber, DIALOGUE_SPECIAL_EVENT_SLEEP, 0,0 );
		TacticalCharacterDialogue( pSoldier, usQuoteNumber );
		TacticalCharacterDialogueWithSpecialEvent( pSoldier, usQuoteNumber, DIALOGUE_SPECIAL_EVENT_SLEEP, 1,0 );
	}
	else
	{
		TacticalCharacterDialogue( pSoldier, usQuoteNumber );
	}
}


void HandleImportantMercQuoteLocked(SOLDIERTYPE* const s, UINT16 const quote)
{
	SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE, 1, MAP_SCREEN, 0, 0, 0);
	HandleImportantMercQuote(s, quote);
	SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE, 0, MAP_SCREEN, 0, 0, 0);
}


// handle pausing of the dialogue queue
void PauseDialogueQueue( void )
{
	gfDialogueQueuePaused = TRUE;
}

// unpause the dialogue queue
void UnPauseDialogueQueue( void )
{
	gfDialogueQueuePaused = FALSE;
}


void SetExternMapscreenSpeechPanelXY( INT16 sXPos, INT16 sYPos )
{
  gsExternPanelXPosition     = sXPos;
  gsExternPanelYPosition     = sYPos;
}
