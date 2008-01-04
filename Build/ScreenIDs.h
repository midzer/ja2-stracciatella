#ifndef __SCREEN_IDS
#define __SCREEN_IDS

enum ScreenTypes
{
	EDIT_SCREEN,
	ERROR_SCREEN = 3,
	INIT_SCREEN,
	GAME_SCREEN,
	ANIEDIT_SCREEN,
	PALEDIT_SCREEN,
	DEBUG_SCREEN,
	MAP_SCREEN,
	LAPTOP_SCREEN,
	LOADSAVE_SCREEN,
	MAPUTILITY_SCREEN,
	FADE_SCREEN,
	MSG_BOX_SCREEN,
	MAINMENU_SCREEN,
	AUTORESOLVE_SCREEN,
	SAVE_LOAD_SCREEN,
	OPTIONS_SCREEN,
	SHOPKEEPER_SCREEN,
	SEX_SCREEN,
	GAME_INIT_OPTIONS_SCREEN,
	DEMO_EXIT_SCREEN,
	INTRO_SCREEN,
	CREDIT_SCREEN,

#ifdef JA2BETAVERSION
	AIVIEWER_SCREEN,
#endif

//#ifdef JA2BETAVERSION
	QUEST_DEBUG_SCREEN,
//#endif

	MAX_SCREENS
};

#endif
