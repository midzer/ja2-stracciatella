#ifndef __INTERFACE_UTILS_H
#define __INTERFACE_UTILS_H

#include "Item_Types.h"
#include "JA2Types.h"


#define		DRAW_ITEM_STATUS_ATTACHMENT1		200
#define		DRAW_ITEM_STATUS_ATTACHMENT2		201
#define		DRAW_ITEM_STATUS_ATTACHMENT3		202
#define		DRAW_ITEM_STATUS_ATTACHMENT4		203

void DrawSoldierUIBars(const SOLDIERTYPE* pSoldier, INT16 sXPos, INT16 sYPos, BOOLEAN fErase, SGPVSurface* buffer);

void DrawItemUIBarEx(const OBJECTTYPE* pObject, UINT8 ubStatus, INT16 sXPos, INT16 sYPos, INT16 sHeight, INT16 sColor1, INT16 sColor2, SGPVSurface* buffer);

void RenderSoldierFace(const SOLDIERTYPE* pSoldier, INT16 sFaceX, INT16 sFaceY);

// load portraits for cars
BOOLEAN LoadCarPortraitValues( void );

// get rid of the loaded portraits for cars
void UnLoadCarPortraits( void );


#endif
