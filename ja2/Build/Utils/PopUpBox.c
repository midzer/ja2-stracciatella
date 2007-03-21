#include "PopUpBox.h"
#include "SysUtil.h"
#include "Debug.h"
#include "Video.h"
#include "MemMan.h"
#include "VSurface.h"
#include "VObject_Blitters.h"
#include "WCheck.h"


typedef struct PopUpString {
	STR16 pString;
	UINT8 ubForegroundColor;
	UINT8 ubBackgroundColor;
	UINT8 ubHighLight;
	UINT8 ubShade;
	UINT8 ubSecondaryShade;
	UINT32 uiFont;
	BOOLEAN fColorFlag;
	BOOLEAN fHighLightFlag;
	BOOLEAN fShadeFlag;
	BOOLEAN fSecondaryShadeFlag;
} PopUpString;

typedef struct PopUpBox {
	SGPRect  Dimensions;
	SGPPoint Position;
	UINT32 uiLeftMargin;
	UINT32 uiRightMargin;
	UINT32 uiBottomMargin;
	UINT32 uiTopMargin;
	UINT32 uiLineSpace;
	INT32 iBorderObjectIndex;
	INT32 iBackGroundSurface;
	UINT32 uiFlags;
	UINT32 uiBuffer;
	UINT32 uiSecondColumnMinimunOffset;
	UINT32 uiSecondColumnCurrentOffset;
	UINT32 uiBoxMinWidth;
	BOOLEAN fUpdated;
	BOOLEAN fShowBox;

	PopUpString* Text[MAX_POPUP_BOX_STRING_COUNT];
	PopUpString* pSecondColumnString[MAX_POPUP_BOX_STRING_COUNT];
} PopUpBox;

static PopUpBox* PopUpBoxList[MAX_POPUP_BOX_COUNT];
static UINT32 guiCurrentBox;


#define BORDER_WIDTH  16
#define BORDER_HEIGHT  8
#define TOP_LEFT_CORNER     0
#define TOP_EDGE            4
#define TOP_RIGHT_CORNER    1
#define SIDE_EDGE           5
#define BOTTOM_LEFT_CORNER  2
#define BOTTOM_EDGE         4
#define BOTTOM_RIGHT_CORNER 3


static void InitPopUpBoxList(void)
{
	memset(PopUpBoxList, 0, sizeof(PopUpBoxList));
}


static void InitPopUpBox(INT32 hBoxHandle)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);
	memset(Box, 0, sizeof(*Box));
}



void SetLineSpace(INT32 hBoxHandle, UINT32 uiLineSpace)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);
	Box->uiLineSpace = uiLineSpace;
}


UINT32 GetLineSpace( INT32 hBoxHandle )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return( 0 );

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);
	// return number of pixels between lines for this box
	return Box->uiLineSpace;
}



void SpecifyBoxMinWidth( INT32 hBoxHandle, INT32 iMinWidth )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Box->uiBoxMinWidth = iMinWidth;

	// check if the box is currently too small
	if (Box->Dimensions.iRight < iMinWidth)
	{
		Box->Dimensions.iRight = iMinWidth;
	}
}


static void SetBoxFlags(INT32 hBoxHandle, UINT32 uiFlags);
static void SetBoxSecondColumnCurrentOffset(INT32 hBoxHandle, UINT32 uiCurrentOffset);


BOOLEAN CreatePopUpBox(INT32 *phBoxHandle, SGPRect Dimensions, SGPPoint Position, UINT32 uiFlags)
{
	INT32 iCounter=0;
	INT32 iCount=0;

	// find first free box
	for(iCounter=0; ( iCounter < MAX_POPUP_BOX_COUNT ) && ( PopUpBoxList[iCounter] != NULL ); iCounter++);

	if( iCounter >= MAX_POPUP_BOX_COUNT )
	{
		// ran out of available popup boxes - probably not freeing them up right!
		Assert(0);
		return FALSE;
	}

	iCount=iCounter;
	*phBoxHandle=iCount;

	PopUpBox* pBox = MemAlloc(sizeof(*pBox));
	if (pBox == NULL)
	{
		return FALSE;
	}
	PopUpBoxList[iCount]=pBox;

	InitPopUpBox(iCount);
	SetBoxPosition(iCount, Position);
	SetBoxSize(iCount, Dimensions);
	SetBoxFlags(iCount, uiFlags);

	for(iCounter=0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		pBox->Text[iCounter] = NULL;
		pBox->pSecondColumnString[iCounter] = NULL;
	}

	SetCurrentBox(iCount);
	SpecifyBoxMinWidth( iCount, 0 );
	SetBoxSecondColumnMinimumOffset( iCount, 0 );
	SetBoxSecondColumnCurrentOffset( iCount, 0 );

	pBox->fUpdated = FALSE;

	return TRUE;
}


static void SetBoxFlags(INT32 hBoxHandle, UINT32 uiFlags)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Box->uiFlags  = uiFlags;
	Box->fUpdated = FALSE;
}


void SetMargins(INT32 hBoxHandle, UINT32 uiLeft, UINT32 uiTop, UINT32 uiBottom, UINT32 uiRight)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Box->uiLeftMargin   = uiLeft;
	Box->uiRightMargin  = uiRight;
	Box->uiTopMargin    = uiTop;
	Box->uiBottomMargin = uiBottom;

	Box->fUpdated = FALSE;
}


UINT32 GetTopMarginSize( INT32 hBoxHandle )
{
	// return size of top margin, for mouse region offsets

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return( 0 );

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	return Box->uiTopMargin;
}


void ShadeStringInBox( INT32 hBoxHandle, INT32 iLineNumber )
{
	// shade iLineNumber Line in box indexed by hBoxHandle

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	if (Box->Text[iLineNumber] != NULL)
	{
		// set current box
		SetCurrentBox( hBoxHandle );

		// shade line
		Box->Text[iLineNumber]->fShadeFlag = TRUE;
	}
}

void UnShadeStringInBox( INT32 hBoxHandle, INT32 iLineNumber )
{
	// unshade iLineNumber in box indexed by hBoxHandle

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	if (Box->Text[iLineNumber] != NULL)
	{
		// set current box
		SetCurrentBox( hBoxHandle );

		// shade line
		Box->Text[iLineNumber]->fShadeFlag = FALSE;
	}
}


void SecondaryShadeStringInBox( INT32 hBoxHandle, INT32 iLineNumber )
{
	// shade iLineNumber Line in box indexed by hBoxHandle

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	if (Box->Text[iLineNumber] != NULL)
	{
		// set current box
		SetCurrentBox( hBoxHandle );

		// shade line
		Box->Text[iLineNumber]->fSecondaryShadeFlag = TRUE;
	}
}

void UnSecondaryShadeStringInBox( INT32 hBoxHandle, INT32 iLineNumber )
{
	// unshade iLineNumber in box indexed by hBoxHandle

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	if (Box->Text[iLineNumber] != NULL)
	{
		// set current box
		SetCurrentBox( hBoxHandle );

		// shade line
		Box->Text[iLineNumber]->fSecondaryShadeFlag = FALSE;
	}
}



void SetBoxBuffer(INT32 hBoxHandle, UINT32 uiBuffer)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Box->uiBuffer = uiBuffer;
	Box->fUpdated = FALSE;
}


void SetBoxPosition( INT32 hBoxHandle,SGPPoint Position )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Box->Position.iX = Position.iX;
	Box->Position.iY = Position.iY;
	Box->fUpdated    = FALSE;
}


void GetBoxPosition( INT32 hBoxHandle, SGPPoint *Position )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Position->iX = Box->Position.iX;
	Position->iY = Box->Position.iY;
}

void SetBoxSize(INT32 hBoxHandle,SGPRect Dimensions)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Box->Dimensions.iLeft   = Dimensions.iLeft;
	Box->Dimensions.iBottom = Dimensions.iBottom;
	Box->Dimensions.iRight  = Dimensions.iRight;
	Box->Dimensions.iTop    = Dimensions.iTop;
	Box->fUpdated           = FALSE;
}


void GetBoxSize( INT32 hBoxHandle, SGPRect *Dimensions )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Dimensions->iLeft   = Box->Dimensions.iLeft;
	Dimensions->iBottom = Box->Dimensions.iBottom;
	Dimensions->iRight  = Box->Dimensions.iRight;
	Dimensions->iTop    = Box->Dimensions.iTop;
}


void SetBorderType(INT32 hBoxHandle, INT32 iBorderObjectIndex)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);
	Box->iBorderObjectIndex = iBorderObjectIndex;
}

void SetBackGroundSurface(INT32 hBoxHandle, INT32 iBackGroundSurfaceIndex)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);
	Box->iBackGroundSurface = iBackGroundSurfaceIndex;
}


static void RemoveCurrentBoxPrimaryText(INT32 hStringHandle);


// adds a FIRST column string to the CURRENT popup box
void AddMonoString(INT32 *hStringHandle, const wchar_t *pString)
{
	STR16 pLocalString = NULL;
	INT32 iCounter = 0;

	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);

	// find first free slot in list
	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT && Box->Text[iCounter] != NULL; iCounter++);

	if ( iCounter >= MAX_POPUP_BOX_STRING_COUNT )
	{
		// using too many text lines, or not freeing them up properly
		Assert(0);
		return;
	}

	PopUpString* pStringSt = MemAlloc(sizeof(*pStringSt));
	if (pStringSt == NULL)
		return;

	pLocalString = MemAlloc(sizeof(*pLocalString) * (wcslen(pString) + 1));
	if (pLocalString == NULL)
		return;

	wcscpy(pLocalString, pString);

	RemoveCurrentBoxPrimaryText( iCounter );

	Box->Text[iCounter]                      = pStringSt;
	Box->Text[iCounter]->fColorFlag          = FALSE;
	Box->Text[iCounter]->pString             = pLocalString;
	Box->Text[iCounter]->fShadeFlag          = FALSE;
	Box->Text[iCounter]->fHighLightFlag      = FALSE;
	Box->Text[iCounter]->fSecondaryShadeFlag = FALSE;

	*hStringHandle=iCounter;

	Box->fUpdated = FALSE;
}


static void RemoveCurrentBoxSecondaryText(INT32 hStringHandle);


// adds a SECOND column string to the CURRENT popup box
void AddSecondColumnMonoString( INT32 *hStringHandle, const wchar_t *pString )
{
	STR16 pLocalString=NULL;
	INT32 iCounter=0;


	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL );

	// find the LAST USED text string index
	for (iCounter = 0; iCounter + 1 < MAX_POPUP_BOX_STRING_COUNT && Box->Text[iCounter + 1] != NULL; iCounter++);

	if ( iCounter >= MAX_POPUP_BOX_STRING_COUNT )
	{
		// using too many text lines, or not freeing them up properly
		Assert(0);
		return;
	}

	PopUpString* pStringSt = MemAlloc(sizeof(*pStringSt));
	if (pStringSt == NULL)
		return;

	pLocalString = MemAlloc(sizeof(*pLocalString) * (wcslen(pString) + 1));
	if (pLocalString == NULL)
		return;

	wcscpy(pLocalString, pString);

	RemoveCurrentBoxSecondaryText( iCounter );

	Box->pSecondColumnString[iCounter]                 = pStringSt;
	Box->pSecondColumnString[iCounter]->fColorFlag     = FALSE;
	Box->pSecondColumnString[iCounter]->pString        = pLocalString;
	Box->pSecondColumnString[iCounter]->fShadeFlag     = FALSE;
	Box->pSecondColumnString[iCounter]->fHighLightFlag = FALSE;

	*hStringHandle=iCounter;
}


// Adds a COLORED first column string to the CURRENT box
static void AddColorString(INT32* hStringHandle, STR16 pString)
{
	STR16 pLocalString;
	INT32 iCounter = 0;

	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);

	// find first free slot in list
	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT && Box->Text[iCounter] != NULL; iCounter++);

	if ( iCounter >= MAX_POPUP_BOX_STRING_COUNT )
	{
		// using too many text lines, or not freeing them up properly
		Assert(0);
		return;
	}

	PopUpString* pStringSt = MemAlloc(sizeof(*pStringSt));
	if (pStringSt == NULL)
		return;

	pLocalString = MemAlloc(sizeof(*pLocalString) * (wcslen(pString) + 1));
	if (pLocalString == NULL)
		return;

	wcscpy(pLocalString, pString);

	RemoveCurrentBoxPrimaryText( iCounter );

	Box->Text[iCounter]             = pStringSt;
	Box->Text[iCounter]->fColorFlag = TRUE;
	Box->Text[iCounter]->pString    = pLocalString;

	*hStringHandle=iCounter;

	Box->fUpdated = FALSE;
}


static void ResizeBoxForSecondStrings(INT32 hBoxHandle)
{
	INT32 iCounter = 0;
	UINT32 uiBaseWidth, uiThisWidth;


	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* pBox = PopUpBoxList[hBoxHandle];
	Assert( pBox );

	uiBaseWidth = pBox->uiLeftMargin + pBox->uiSecondColumnMinimunOffset;

	// check string sizes
	for( iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++ )
	{
		if( pBox->Text[ iCounter ] )
		{
			uiThisWidth = uiBaseWidth + StringPixLength( pBox->Text[ iCounter ]->pString, pBox->Text[ iCounter ]->uiFont );

			if( uiThisWidth > pBox->uiSecondColumnCurrentOffset )
			{
				pBox->uiSecondColumnCurrentOffset = uiThisWidth;
			}
		}
	}
}



UINT32 GetNumberOfLinesOfTextInBox( INT32 hBoxHandle )
{
	INT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return( 0 );

	// count number of lines
	// check string size
	for( iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++ )
	{
		if( PopUpBoxList[ hBoxHandle ]->Text[ iCounter ] == NULL )
		{
			break;
		}
	}

	return( iCounter );
}



void SetBoxFont(INT32 hBoxHandle, UINT32 uiFont)
{
	UINT32 uiCounter;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	for ( uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++ )
	{
		if (Box->Text[uiCounter] != NULL)
		{
			Box->Text[uiCounter]->uiFont = uiFont;
		}
	}

	// set up the 2nd column font
	SetBoxSecondColumnFont(hBoxHandle, uiFont);

	Box->fUpdated = FALSE;
}

void SetBoxSecondColumnMinimumOffset( INT32 hBoxHandle, UINT32 uiWidth )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBoxList[hBoxHandle]->uiSecondColumnMinimunOffset = uiWidth;
}


static void SetBoxSecondColumnCurrentOffset(INT32 hBoxHandle, UINT32 uiCurrentOffset)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBoxList[hBoxHandle]->uiSecondColumnCurrentOffset = uiCurrentOffset;
}

void SetBoxSecondColumnFont(INT32 hBoxHandle, UINT32 uiFont)
{
	UINT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->pSecondColumnString[iCounter])
		{
			Box->pSecondColumnString[iCounter]->uiFont = uiFont;
		}
	}

	Box->fUpdated = FALSE;
}

UINT32 GetBoxFont( INT32 hBoxHandle )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return( 0 );

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);
	Assert(Box->Text[0]);

	// return font id of first line of text of box
	return Box->Text[0]->uiFont;
}


// set the foreground color of this string in this pop up box
void SetBoxLineForeground( INT32 iBox, INT32 iStringValue, UINT8 ubColor )
{
	if ( ( iBox < 0 ) || ( iBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[iBox];

	Assert(Box);
	Assert(Box->Text[iStringValue]);

	Box->Text[iStringValue]->ubForegroundColor = ubColor;
}

void SetBoxSecondaryShade( INT32 iBox, UINT8 ubColor )
{
	UINT32 uiCounter;

	if ( ( iBox < 0 ) || ( iBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[iBox];

	Assert(Box);

	for ( uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++ )
	{
		if (Box->Text[uiCounter] != NULL)
		{
			Box->Text[uiCounter]->ubSecondaryShade = ubColor;
		}
	}
}


// The following functions operate on the CURRENT box

static void SetPopUpStringFont(INT32 hStringHandle, UINT32 uiFont)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);
	Assert(Box->Text[hStringHandle]);

	Box->Text[hStringHandle]->uiFont = uiFont;
}


static void SetPopUpSecondColumnStringFont(INT32 hStringHandle, UINT32 uiFont)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);
	Assert(Box->pSecondColumnString[hStringHandle]);

	Box->pSecondColumnString[hStringHandle]->uiFont = uiFont;
}


static void SetStringSecondaryShade(INT32 hStringHandle, UINT8 ubColor)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);
	Assert(Box->Text[hStringHandle]);

	Box->Text[hStringHandle]->ubSecondaryShade = ubColor;
}


static void SetStringForeground(INT32 hStringHandle, UINT8 ubColor)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);
	Assert(Box->Text[hStringHandle]);

	Box->Text[hStringHandle]->ubForegroundColor = ubColor;
}


static void SetStringBackground(INT32 hStringHandle, UINT8 ubColor)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);
	Assert(Box->Text[hStringHandle]);

	Box->Text[hStringHandle]->ubBackgroundColor = ubColor;
}


static void SetStringHighLight(INT32 hStringHandle, UINT8 ubColor)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);
	Assert(Box->Text[hStringHandle]);

	Box->Text[hStringHandle]->ubHighLight = ubColor;
}


static void SetStringShade(INT32 hStringHandle, UINT8 ubShade)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);
	Assert(Box->Text[hStringHandle]);

	Box->Text[hStringHandle]->ubShade = ubShade;
}


static void SetStringSecondColumnForeground(INT32 hStringHandle, UINT8 ubColor)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL );
	Assert(Box->pSecondColumnString[hStringHandle]);

	Box->pSecondColumnString[hStringHandle]->ubForegroundColor = ubColor;
}


static void SetStringSecondColumnBackground(INT32 hStringHandle, UINT8 ubColor)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);
	Assert(Box->pSecondColumnString[hStringHandle]);

	Box->pSecondColumnString[hStringHandle]->ubBackgroundColor = ubColor;
}


static void SetStringSecondColumnHighLight(INT32 hStringHandle, UINT8 ubColor)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL );
	Assert(Box->pSecondColumnString[hStringHandle]);

	Box->pSecondColumnString[hStringHandle]->ubHighLight = ubColor;
}


static void SetStringSecondColumnShade(INT32 hStringHandle, UINT8 ubShade)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);
	Assert(Box->pSecondColumnString[hStringHandle]);

	Box->pSecondColumnString[hStringHandle]->ubShade = ubShade;
}



void SetBoxForeground(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 uiCounter;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for ( uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++ )
	{
		if (Box->Text[uiCounter] != NULL)
		{
			Box->Text[uiCounter]->ubForegroundColor = ubColor;
		}
	}
}

void SetBoxBackground(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 uiCounter;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for ( uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++ )
	{
		if (Box->Text[uiCounter] != NULL)
		{
			Box->Text[uiCounter]->ubBackgroundColor = ubColor;
		}
	}
}

void SetBoxHighLight(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 uiCounter;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for ( uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++ )
	{
		if (Box->Text[uiCounter] != NULL)
		{
			Box->Text[uiCounter]->ubHighLight = ubColor;
		}
	}
}

void SetBoxShade(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 uiCounter;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for ( uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++ )
	{
		if (Box->Text[uiCounter] != NULL)
		{
			Box->Text[uiCounter]->ubShade = ubColor;
		}
	}
}

void SetBoxSecondColumnForeground(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->pSecondColumnString[iCounter])
		{
			Box->pSecondColumnString[iCounter]->ubForegroundColor = ubColor;
		}
	}
}

void SetBoxSecondColumnBackground(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->pSecondColumnString[iCounter])
		{
			Box->pSecondColumnString[iCounter]->ubBackgroundColor = ubColor;
		}
	}
}

void SetBoxSecondColumnHighLight(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->pSecondColumnString[iCounter])
		{
			Box->pSecondColumnString[iCounter]->ubHighLight = ubColor;
		}
	}
}

void SetBoxSecondColumnShade(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->pSecondColumnString[iCounter])
		{
			Box->pSecondColumnString[iCounter]->ubShade = ubColor;
		}
	}
}


static void HighLightLine(INT32 hStringHandle)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);

	if (!Box->Text[hStringHandle])
		return;
	Box->Text[hStringHandle]->fHighLightFlag = TRUE;
}


// is this line int he current boxed in a shaded state?
static BOOLEAN GetShadeFlag(INT32 hStringHandle)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return(FALSE);

	const PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);

	if (!Box->Text[hStringHandle])
		return FALSE;

	return Box->Text[hStringHandle]->fShadeFlag;
}


static BOOLEAN GetSecondaryShadeFlag(INT32 hStringHandle)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return(FALSE);

	const PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);

	if (!Box->Text[hStringHandle])
		return FALSE;

	return Box->Text[hStringHandle]->fSecondaryShadeFlag;
}


void HighLightBoxLine( INT32 hBoxHandle, INT32 iLineNumber )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	// highlight iLineNumber Line in box indexed by hBoxHandle

	if (PopUpBoxList[hBoxHandle]->Text[iLineNumber] != NULL)
	{
		// set current box
		SetCurrentBox( hBoxHandle );

		// highlight line
		HighLightLine( iLineNumber );
	}
}

BOOLEAN GetBoxShadeFlag( INT32 hBoxHandle, INT32 iLineNumber )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return(FALSE);

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	if (Box->Text[iLineNumber] != NULL)
	{
		return Box->Text[iLineNumber]->fShadeFlag;
	}


	return( FALSE );
}


static BOOLEAN GetBoxSecondaryShadeFlag(INT32 hBoxHandle, INT32 iLineNumber)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return(FALSE);

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	if (Box->Text[iLineNumber] != NULL)
	{
		return Box->Text[iLineNumber]->fSecondaryShadeFlag;
	}


	return( FALSE );
}

void UnHighLightLine(INT32 hStringHandle)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);

	if (!Box->Text[hStringHandle])
		return;
	Box->Text[hStringHandle]->fHighLightFlag = FALSE;
}

void UnHighLightBox(INT32 hBoxHandle)
{
	INT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->Text[iCounter])
			Box->Text[iCounter]->fHighLightFlag = FALSE;
	}
}


static void UnHighLightSecondColumnLine(INT32 hStringHandle)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);

	if (!Box->pSecondColumnString[hStringHandle])
		return;

	Box->pSecondColumnString[hStringHandle]->fHighLightFlag = FALSE;
}


static void UnHighLightSecondColumnBox(INT32 hBoxHandle)
{
	INT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->pSecondColumnString[iCounter])
			Box->pSecondColumnString[iCounter]->fHighLightFlag = FALSE;
	}
}


static void RemoveOneCurrentBoxString(INT32 hStringHandle, BOOLEAN fFillGaps)
{
	UINT32 uiCounter=0;

	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);
	Assert( hStringHandle < MAX_POPUP_BOX_STRING_COUNT );

	RemoveCurrentBoxPrimaryText( hStringHandle );
	RemoveCurrentBoxSecondaryText( hStringHandle );

	if ( fFillGaps )
	{
		// shuffle all strings down a slot to fill in the gap
		for ( uiCounter = hStringHandle; uiCounter < ( MAX_POPUP_BOX_STRING_COUNT - 1 ); uiCounter++ )
		{
			Box->Text[uiCounter]                = Box->Text[uiCounter + 1];
			Box->pSecondColumnString[uiCounter] = Box->pSecondColumnString[uiCounter + 1];
		}
	}

	Box->fUpdated = FALSE;
}



void RemoveAllCurrentBoxStrings( void )
{
	UINT32 uiCounter;

	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	for(uiCounter=0; uiCounter <MAX_POPUP_BOX_STRING_COUNT; uiCounter++)
		RemoveOneCurrentBoxString( uiCounter, FALSE );
}


static void GetCurrentBox(INT32 *hBoxHandle);


void RemoveBox(INT32 hBoxHandle)
{
	INT32 hOldBoxHandle;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	GetCurrentBox(&hOldBoxHandle);
	SetCurrentBox(hBoxHandle);

	RemoveAllCurrentBoxStrings();

	MemFree(PopUpBoxList[hBoxHandle]);
	PopUpBoxList[hBoxHandle] = NULL;

	if (hOldBoxHandle != hBoxHandle)
		SetCurrentBox(hOldBoxHandle);
}



void ShowBox(INT32 hBoxHandle)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	if (Box != NULL)
	{
		if (Box->fShowBox == FALSE)
		{
			Box->fShowBox = TRUE;
			Box->fUpdated = FALSE;
		}
	}
}

void HideBox(INT32 hBoxHandle)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	if (Box != NULL)
	{
		if (Box->fShowBox == TRUE)
		{
			Box->fShowBox = FALSE;
			Box->fUpdated = FALSE;
		}
	}
}



void SetCurrentBox(INT32 hBoxHandle)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	guiCurrentBox = hBoxHandle;
}


static void GetCurrentBox(INT32 *hBoxHandle)
{
	*hBoxHandle = guiCurrentBox;
}



void DisplayBoxes(UINT32 uiBuffer)
{
	UINT32 uiCounter;

	for( uiCounter=0; uiCounter < MAX_POPUP_BOX_COUNT; uiCounter++ )
	{
		DisplayOnePopupBox( uiCounter, uiBuffer );
	}
}


static BOOLEAN DrawBox(UINT32 uiCounter);
static BOOLEAN DrawBoxText(UINT32 uiCounter);


void DisplayOnePopupBox( UINT32 uiIndex, UINT32 uiBuffer )
{
	if ( ( uiIndex < 0 ) || ( uiIndex >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[uiIndex];

	if (Box != NULL)
	{
		if (Box->uiBuffer == uiBuffer && Box->fShowBox)
		{
			DrawBox(uiIndex);
			DrawBoxText(uiIndex);
		}
	}
}



// force an update of this box
void ForceUpDateOfBox( UINT32 uiIndex )
{
	if ( ( uiIndex < 0 ) || ( uiIndex >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[uiIndex];

	if (Box != NULL)
	{
		Box->fUpdated = FALSE;
	}
}


static BOOLEAN DrawBox(UINT32 uiCounter)
{
	// will build pop up box in usTopX, usTopY with dimensions usWidth and usHeight
	UINT32 uiNumTilesWide;
	UINT32 uiNumTilesHigh;
	UINT32 uiCount=0;
	UINT32 uiDestPitchBYTES;
	UINT32 uiSrcPitchBYTES;
	UINT16* pDestBuf;
	UINT8  *pSrcBuf;
	SGPRect clip;
	UINT16 usTopX, usTopY;
	UINT16 usWidth, usHeight;


	if ( ( uiCounter < 0 ) || ( uiCounter >= MAX_POPUP_BOX_COUNT ) )
		return(FALSE);

	PopUpBox* Box = PopUpBoxList[uiCounter];

	Assert(Box != NULL);

	// only update if we need to

	if (Box->fUpdated == TRUE)
	{
		return( FALSE );
	}

	Box->fUpdated = TRUE;

	if (Box->uiFlags & POPUP_BOX_FLAG_RESIZE)
	{
		ResizeBoxToText(uiCounter);
	}

	usTopX   = (UINT16)Box->Position.iX;
	usTopY   = (UINT16)Box->Position.iY;
	usWidth  = (UINT16)(Box->Dimensions.iRight  - Box->Dimensions.iLeft);
	usHeight = (UINT16)(Box->Dimensions.iBottom - Box->Dimensions.iTop);

	// check if we have a min width, if so then update box for such
	if (Box->uiBoxMinWidth && usWidth < Box->uiBoxMinWidth)
	{
		usWidth = (INT16)Box->uiBoxMinWidth;
	}

	// make sure it will fit on screen!
	Assert( usTopX + usWidth  <= 639 );
	Assert( usTopY + usHeight <= 479 );

	// subtract 4 because the 2 2-pixel corners are handled separately
	uiNumTilesWide=((usWidth-4)/BORDER_WIDTH);
	uiNumTilesHigh=((usHeight-4)/BORDER_HEIGHT);

	clip.iLeft=0;
	clip.iRight = clip.iLeft + usWidth;
	clip.iTop=0;
	clip.iBottom=clip.iTop+usHeight;

	// blit in texture first, then borders
	// blit in surface
	pDestBuf = (UINT16*)LockVideoSurface(Box->uiBuffer, &uiDestPitchBYTES);
	HVSURFACE hSrcVSurface = GetVideoSurface(Box->iBackGroundSurface);
	CHECKF(hSrcVSurface != NULL);
	pSrcBuf = LockVideoSurface(Box->iBackGroundSurface, &uiSrcPitchBYTES);
	Blt8BPPDataSubTo16BPPBuffer( pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf,uiSrcPitchBYTES,usTopX,usTopY, &clip);
	UnLockVideoSurface(Box->iBackGroundSurface);
	UnLockVideoSurface(Box->uiBuffer);
	HVOBJECT hBoxHandle = GetVideoObject(Box->iBorderObjectIndex);

	// blit in 4 corners (they're 2x2 pixels)
	BltVideoObject(Box->uiBuffer, hBoxHandle, TOP_LEFT_CORNER,     usTopX,               usTopY);
	BltVideoObject(Box->uiBuffer, hBoxHandle, TOP_RIGHT_CORNER,    usTopX + usWidth - 2, usTopY);
	BltVideoObject(Box->uiBuffer, hBoxHandle, BOTTOM_RIGHT_CORNER, usTopX + usWidth - 2, usTopY + usHeight - 2);
	BltVideoObject(Box->uiBuffer, hBoxHandle, BOTTOM_LEFT_CORNER,  usTopX,               usTopY + usHeight - 2);

	// blit in edges
	if (uiNumTilesWide > 0)
	{
		// full pieces
		for (uiCount = 0; uiCount < uiNumTilesWide; uiCount++)
		{
			BltVideoObject(Box->uiBuffer, hBoxHandle, TOP_EDGE,    usTopX + 2 + uiCount * BORDER_WIDTH, usTopY);
			BltVideoObject(Box->uiBuffer, hBoxHandle, BOTTOM_EDGE, usTopX + 2 + uiCount * BORDER_WIDTH, usTopY + usHeight - 2);
		}

		// partial pieces
		BltVideoObject(Box->uiBuffer, hBoxHandle, TOP_EDGE,    usTopX + usWidth - 2 - BORDER_WIDTH, usTopY);
		BltVideoObject(Box->uiBuffer, hBoxHandle, BOTTOM_EDGE, usTopX + usWidth - 2 - BORDER_WIDTH, usTopY + usHeight - 2);
	}
	if (uiNumTilesHigh > 0)
	{
		// full pieces
		for (uiCount = 0; uiCount < uiNumTilesHigh; uiCount++)
		{
			BltVideoObject(Box->uiBuffer, hBoxHandle, SIDE_EDGE, usTopX,               usTopY + 2 + uiCount * BORDER_HEIGHT);
			BltVideoObject(Box->uiBuffer, hBoxHandle, SIDE_EDGE, usTopX + usWidth - 2, usTopY + 2 + uiCount * BORDER_HEIGHT);
		}

		// partial pieces
		BltVideoObject(Box->uiBuffer, hBoxHandle, SIDE_EDGE, usTopX,               usTopY + usHeight - 2 - BORDER_HEIGHT);
		BltVideoObject(Box->uiBuffer, hBoxHandle, SIDE_EDGE, usTopX + usWidth - 2, usTopY + usHeight - 2 - BORDER_HEIGHT);
	}

	InvalidateRegion( usTopX, usTopY, usTopX + usWidth, usTopY + usHeight );
	return TRUE;
}


static BOOLEAN DrawBoxText(UINT32 uiCounter)
{
	UINT32 uiCount = 0;
	INT16 uX, uY;

	if ( ( uiCounter < 0 ) || ( uiCounter >= MAX_POPUP_BOX_COUNT ) )
		return(FALSE);

	const PopUpBox* Box = PopUpBoxList[uiCounter];

	Assert(Box != NULL);

	//clip text?
	if (Box->uiFlags & POPUP_BOX_FLAG_CLIP_TEXT)
	{
		SetFontDestBuffer
		(
			Box->uiBuffer,
			Box->Position.iX + Box->uiLeftMargin - 1,
			Box->Position.iY + Box->uiTopMargin,
			Box->Position.iX + Box->Dimensions.iRight  - Box->uiRightMargin,
			Box->Position.iY + Box->Dimensions.iBottom - Box->uiBottomMargin,
			FALSE
		);
	}

	for (uiCount = 0; uiCount < MAX_POPUP_BOX_STRING_COUNT; uiCount++)
	{
		// there is text in this line?
		if (Box->Text[uiCount])
		{
			// set font
			SetFont(Box->Text[uiCount]->uiFont);

			// are we highlighting?...shading?..or neither
			if (Box->Text[uiCount]->fHighLightFlag == FALSE && Box->Text[uiCount]->fShadeFlag == FALSE && Box->Text[uiCount]->fSecondaryShadeFlag == FALSE)
			{
				// neither
				SetFontForeground(Box->Text[uiCount]->ubForegroundColor);
			}
			else if (Box->Text[uiCount]->fHighLightFlag == TRUE)
			{
				// highlight
				SetFontForeground(Box->Text[uiCount]->ubHighLight);
			}
			else if (Box->Text[uiCount]->fSecondaryShadeFlag == TRUE)
			{
				SetFontForeground(Box->Text[uiCount]->ubSecondaryShade);
			}
			else
			{
				//shading
				SetFontForeground(Box->Text[uiCount]->ubShade);
			}

			// set background
			SetFontBackground(Box->Text[uiCount]->ubBackgroundColor);

			// cnetering?
			if (Box->uiFlags & POPUP_BOX_FLAG_CENTER_TEXT)
			{
				FindFontCenterCoordinates
				(
					(INT16)(Box->Position.iX + Box->uiLeftMargin),
					(INT16)(Box->Position.iY + uiCount * GetFontHeight(Box->Text[uiCount]->uiFont) + Box->uiTopMargin + uiCount * Box->uiLineSpace),
					(INT16)(Box->Dimensions.iRight - (Box->uiRightMargin + Box->uiLeftMargin + 2)),
					(INT16)GetFontHeight(Box->Text[uiCount]->uiFont),
					Box->Text[uiCount]->pString,
					(INT32)Box->Text[uiCount]->uiFont,
					&uX, &uY
				);
			}
			else
			{
				uX = (INT16)(Box->Position.iX + Box->uiLeftMargin);
				uY = (INT16)(Box->Position.iY + uiCount * GetFontHeight(Box->Text[uiCount]->uiFont) + Box->uiTopMargin + uiCount * Box->uiLineSpace);
			}

			// print
			//gprintfdirty(uX, uY, L"%S", Box->Text[uiCount]->pString);
			mprintf(uX, uY, L"%S", Box->Text[uiCount]->pString);
		}


		// there is secondary text in this line?
		if (Box->pSecondColumnString[uiCount])
		{
			// set font
			SetFont(Box->pSecondColumnString[uiCount]->uiFont);

			// are we highlighting?...shading?..or neither
			if (Box->pSecondColumnString[uiCount]->fHighLightFlag == FALSE && Box->pSecondColumnString[uiCount]->fShadeFlag == FALSE)
			{
				// neither
				SetFontForeground(Box->pSecondColumnString[uiCount]->ubForegroundColor);
			}
			else if (Box->pSecondColumnString[uiCount]->fHighLightFlag == TRUE)
			{
				// highlight
				SetFontForeground(Box->pSecondColumnString[uiCount]->ubHighLight);
			}
			else
			{
				//shading
				SetFontForeground(Box->pSecondColumnString[uiCount]->ubShade);
			}

			// set background
			SetFontBackground(Box->pSecondColumnString[uiCount]->ubBackgroundColor);

			// cnetering?
			if (Box->uiFlags & POPUP_BOX_FLAG_CENTER_TEXT)
			{
				FindFontCenterCoordinates
				(
					(INT16)(Box->Position.iX + Box->uiLeftMargin),
					(INT16)(Box->Position.iY + uiCount * GetFontHeight(Box->pSecondColumnString[uiCount]->uiFont) + Box->uiTopMargin + uiCount * Box->uiLineSpace),
					(INT16)(Box->Dimensions.iRight - (Box->uiRightMargin + Box->uiLeftMargin + 2)),
					(INT16)GetFontHeight(Box->pSecondColumnString[uiCount]->uiFont),
					Box->pSecondColumnString[uiCount]->pString,
					(INT32)Box->pSecondColumnString[uiCount]->uiFont,
					&uX, &uY
				);
			}
			else
			{
				uX = (INT16)(Box->Position.iX + Box->uiLeftMargin + Box->uiSecondColumnCurrentOffset);
				uY = (INT16)(Box->Position.iY + uiCount * GetFontHeight(Box->pSecondColumnString[uiCount]->uiFont) + Box->uiTopMargin + uiCount * Box->uiLineSpace);
			}

			// print
			//gprintfdirty(uX, uY, L"%S", Box->Text[uiCount]->pString);
			mprintf(uX, uY, L"%S", Box->pSecondColumnString[uiCount]->pString);
		}
	}


	if (Box->uiBuffer != guiSAVEBUFFER)
	{
		InvalidateRegion
		(
			Box->Position.iX + Box->uiLeftMargin - 1,
			Box->Position.iY + Box->uiTopMargin,
			Box->Position.iX + Box->Dimensions.iRight  - Box->uiRightMargin,
			Box->Position.iY + Box->Dimensions.iBottom - Box->uiBottomMargin
		);
	}

	SetFontDestBuffer(FRAME_BUFFER, 0, 0, 640, 480, FALSE);

	return TRUE;
}


void ResizeBoxToText(INT32 hBoxHandle)
{
	// run through lines of text in box and size box width to longest line plus margins
	// height is sum of getfontheight of each line+ spacing
	INT32 iWidth=0;
	INT32 iHeight=0;
	INT32 iCurrString=0;
	INT32 iSecondColumnLength = 0;


	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	if (!Box)
		return;

	ResizeBoxForSecondStrings( hBoxHandle );

	iHeight = Box->uiTopMargin + Box->uiBottomMargin;

	for ( iCurrString = 0; iCurrString < MAX_POPUP_BOX_STRING_COUNT; iCurrString++ )
	{
		if (Box->Text[iCurrString] != NULL)
		{
			if (Box->pSecondColumnString[iCurrString] != NULL)
			{
				iSecondColumnLength = StringPixLength(Box->pSecondColumnString[iCurrString]->pString, Box->pSecondColumnString[iCurrString]->uiFont);
				if (Box->uiSecondColumnCurrentOffset + iSecondColumnLength + Box->uiLeftMargin + Box->uiRightMargin > (UINT32)iWidth)
				{
					iWidth = Box->uiSecondColumnCurrentOffset + iSecondColumnLength + Box->uiLeftMargin + Box->uiRightMargin;
				}
			}

			if (StringPixLength(Box->Text[iCurrString]->pString, Box->Text[iCurrString]->uiFont) + Box->uiLeftMargin + Box->uiRightMargin > (UINT32)iWidth)
				iWidth = StringPixLength(Box->Text[iCurrString]->pString, Box->Text[iCurrString]->uiFont) + Box->uiLeftMargin + Box->uiRightMargin;

			//vertical
			iHeight += GetFontHeight(Box->Text[iCurrString]->uiFont) + Box->uiLineSpace;
		}
		else
		{
			// doesn't support gaps in text array...
			break;
		}
	}
	Box->Dimensions.iBottom = iHeight;
	Box->Dimensions.iRight  = iWidth;
}


BOOLEAN IsBoxShown( UINT32 uiHandle )
{
	if ( ( uiHandle < 0 ) || ( uiHandle >= MAX_POPUP_BOX_COUNT ) )
		return(FALSE);

	const PopUpBox* Box = PopUpBoxList[uiHandle];

	if (Box == NULL)
	{
		return ( FALSE );
	}

	return Box->fShowBox;
}


void MarkAllBoxesAsAltered( void )
{
	INT32 iCounter = 0;

	// mark all boxes as altered
	for( iCounter = 0; iCounter < MAX_POPUP_BOX_COUNT; iCounter++ )
	{
		ForceUpDateOfBox( iCounter );
	}
}


void HideAllBoxes( void )
{
	INT32 iCounter = 0;

	// hide all the boxes that are shown
	for(iCounter=0; iCounter < MAX_POPUP_BOX_COUNT; iCounter++)
	{
		HideBox( iCounter );
	}
}


static void RemoveCurrentBoxPrimaryText(INT32 hStringHandle)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);
	Assert( hStringHandle < MAX_POPUP_BOX_STRING_COUNT );

	// remove & release primary text
	if (Box->Text[hStringHandle] != NULL)
	{
		if (Box->Text[hStringHandle]->pString)
		{
			MemFree(Box->Text[hStringHandle]->pString);
		}

		MemFree(Box->Text[hStringHandle]);
		Box->Text[hStringHandle] = NULL;
	}
}


static void RemoveCurrentBoxSecondaryText(INT32 hStringHandle)
{
	if ( ( guiCurrentBox < 0 ) || ( guiCurrentBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[guiCurrentBox];

	Assert(Box != NULL);
	Assert( hStringHandle < MAX_POPUP_BOX_STRING_COUNT );

	// remove & release secondary strings
	if (Box->pSecondColumnString[hStringHandle] != NULL)
	{
		if (Box->pSecondColumnString[hStringHandle]->pString)
		{
			MemFree(Box->pSecondColumnString[hStringHandle]->pString);
		}

		MemFree(Box->pSecondColumnString[hStringHandle]);
		Box->pSecondColumnString[hStringHandle] = NULL;
	}
}
