#include "Font.h"
#include "IMP_AboutUs.h"
#include "CharProfile.h"
#include "IMPVideoObjects.h"
#include "Text.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Debug.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Laptop.h"
#include "Button_System.h"
#include "Font_Control.h"


// IMP AboutUs buttons
INT32 giIMPAboutUsButton[1];
INT32 giIMPAboutUsButtonImage[1];

static void BtnIMPBackCallback(GUI_BUTTON *btn, INT32 reason);


static void CreateIMPAboutUsButtons(void);


void EnterIMPAboutUs( void )
{

	// create buttons
	CreateIMPAboutUsButtons( );

	// entry into IMP about us page
	RenderIMPAboutUs( );
}


static void DeleteIMPAboutUsButtons(void);


void ExitIMPAboutUs( void )
{
  // exit from IMP About us page

	// delete Buttons
	DeleteIMPAboutUsButtons( );
}


void RenderIMPAboutUs( void )
{
  // rneders the IMP about us page

	// the background
	RenderProfileBackGround( );

	// the IMP symbol
	RenderIMPSymbol( 106, 1 );

	// about us indent
	RenderAboutUsIndentFrame( 8, 130 );
  // about us indent
	RenderAboutUsIndentFrame( 258, 130 );
}


void HandleIMPAboutUs( void )
{
  // handles the IMP about us page
}


static void CreateIMPAboutUsButtons(void)
{

  // this function will create the buttons needed for th IMP about us page
  // the back button button
  giIMPAboutUsButtonImage[0]=  LoadButtonImage( "LAPTOP/button_3.sti" ,-1,0,-1,1,-1 );
   giIMPAboutUsButton[0] = CreateIconAndTextButton( giIMPAboutUsButtonImage[0], pImpButtonText[6], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  216, LAPTOP_SCREEN_WEB_UL_Y + ( 360 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnIMPBackCallback);

	 SetButtonCursor(giIMPAboutUsButton[0], CURSOR_WWW);
}


static void DeleteIMPAboutUsButtons(void)
{
  // this function destroys the buttons needed for the IMP about Us Page

  // the about back button
  RemoveButton(giIMPAboutUsButton[0] );
  UnloadButtonImage(giIMPAboutUsButtonImage[0] );
}


static void BtnIMPBackCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
     iCurrentImpPage = IMP_HOME_PAGE;
	}
}
