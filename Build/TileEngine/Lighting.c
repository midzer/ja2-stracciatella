/****************************************************************************************
* JA2 Lighting Module
*
*		Tile-based, ray-casted lighting system.
*
*		Lights are precalculated into linked lists containing offsets from 0,0, and a light
* level to add at that tile. Lists are constructed by casting a ray from the origin of
* the light, and each tile stopped at is stored as a node in the list. To draw the light
* during runtime, you traverse the list, checking at each tile that it isn't of the type
* that can obscure light. If it is, you keep traversing the list until you hit a node
* with a marker LIGHT_NEW_RAY, which means you're back at the origin, and have skipped
* the remainder of the last ray.
*
* Written by Derek Beland, April 14, 1997
*
***************************************************************************************/

#include "HImage.h"
#include "Overhead.h"
#include "math.h"
#include "VObject.h"
#include "WorldDef.h"
#include "RenderWorld.h"
#include "Input.h"
#include "SysUtil.h"
#include "Debug.h"
#include "WCheck.h"
#include "Edit_Sys.h"
#include "Isometric_Utils.h"
#include "Line.h"
#include "Animation_Data.h"
#include "Radar_Screen.h"
#include "Render_Dirty.h"
#include "Sys_Globals.h"
#include "TileDef.h"
#include "Lighting.h"
#include "Structure_Internals.h"
#include "Structure_Wrap.h"
#include "Rotting_Corpses.h"
#include "FileMan.h"
#include "Environment.h"
#include "PathAI.h"
#include "MemMan.h"


#define MAX_LIGHT_TEMPLATES 32 // maximum number of light types


// stucture of node in linked list for lights
typedef struct LIGHT_NODE
{
	INT16 iDX;
	INT16 iDY;
	UINT8 uiFlags;
	UINT8 ubLight;
} LIGHT_NODE;
CASSERT(sizeof(LIGHT_NODE) == 6)


struct LightTemplate
{
	LIGHT_NODE* lights;
	UINT16*     rays;
	UINT16      n_lights;
	UINT16      n_rays;
	char*       name;
};

static LightTemplate g_light_templates[MAX_LIGHT_TEMPLATES];


// Sprite data
LIGHT_SPRITE	LightSprites[MAX_LIGHT_SPRITES];

// Lighting system general data
UINT8						ubAmbientLightLevel=DEFAULT_SHADE_LEVEL;

SGPPaletteEntry g_light_color = { 0, 0, 0, 0 };

static SGPPaletteEntry gpOrigLight = { 0, 0, 0, 0 };


/*
UINT16 gusShadeLevels[16][3]={{500, 500, 500},				// green table
															{450, 450, 450},		// bright
															{350, 350, 350},
															{300, 300, 300},
															{255, 255, 255},		// normal
															{227, 227, 227},
															{198, 198, 198},
															{171, 171, 171},
															{143, 143, 143},
															{115, 115, 160},				// darkening
															{87, 87, 176},
															{60, 60, 160},
															{48, 48, 192},
															{36, 36, 208},
															{18, 18, 224},
															{48, 222, 48}};
*/
// Externed in Rotting Corpses.c
//Kris' attempt at blue night lights
/*
UINT16 gusShadeLevels[16][3]={{500, 500, 500},				// green table
															{450, 450, 450},		// bright
															{350, 350, 350},
															{300, 300, 300},
															{255, 255, 255},		// normal
															{215, 215, 227},
															{179, 179, 179},
															{149, 149, 149},
															{125, 125, 128},
															{104, 104, 128},				// darkening
															{86, 86, 128},
															{72, 72, 128},
															{60, 60, 128},
															{36, 36, 208},
															{18, 18, 224},
															{48, 222, 48}};
*/

/*
//Linda's final version

UINT16 gusShadeLevels[16][3] =
{
	500, 500, 500,
	450, 450, 450,	//bright
	350, 350, 350,
	300, 300, 300,
	255, 255, 255,	//normal
	222, 200, 200,
	174, 167, 167,
	150, 137, 137,
	122, 116, 116,	//darkening
	96, 96, 96,
	77, 77, 84,
	58, 58, 69,
	44, 44, 66,			//night
	36, 36, 244,
	18, 18, 224,
	48, 222, 48,
};
*/

// JA2 Gold:
static UINT16 gusShadeLevels[16][3] =
{
	{ 500, 500, 500 },
	{ 450, 450, 450 }, // bright
	{ 350, 350, 350 },
	{ 300, 300, 300 },
	{ 255, 255, 255 }, // normal
	{ 231, 199, 199 },
	{ 209, 185, 185 },
	{ 187, 171, 171 },
	{ 165, 157, 157 }, // darkening
	{ 143, 143, 143 },
	{ 121, 121, 129 },
	{  99,  99, 115 },
	{  77,  77, 101 }, // night
	{  36,  36, 244 },
	{  18,  18, 224 },
	{  48, 222,  48 }
};

//Set this true if you want the shadetables to be loaded from the text file.
BOOLEAN gfLoadShadeTablesFromTextFile =		FALSE;

void LoadShadeTablesFromTextFile()
{
	FILE *fp;
	INT32 i, j;
	INT32 num;
	if( gfLoadShadeTablesFromTextFile )
	{
		fp = fopen( "ShadeTables.txt", "r" );
		Assert( fp );
		if( fp )
		{
			for( i = 0; i < 16; i++ )
			{
				for( j = 0; j < 3; j++ )
				{
					char str[10];
					fscanf( fp, "%s", str );
					sscanf( str, "%d", &num );
					gusShadeLevels[i][j] = (UINT16)num;
				}
			}
			fclose( fp );
		}
	}
}


static LightTemplate* LightLoad(const char* pFilename);


/****************************************************************************************
 InitLightingSystem

	Initializes the lighting system.

***************************************************************************************/
BOOLEAN InitLightingSystem(void)
{
	LoadShadeTablesFromTextFile();

	// init all light lists
	memset(g_light_templates, 0, sizeof(g_light_templates));

	// init all light sprites
	memset(LightSprites, 0, sizeof(LightSprites));

	if (LightLoad("TRANSLUC.LHT") != g_light_templates)
	{
		DebugMsg(TOPIC_GAME, DBG_LEVEL_0, "Failed to load translucency template");
		return(FALSE);
	}

	return(TRUE);
}


// THIS MUST BE CALLED ONCE ALL SURFACE VIDEO OBJECTS HAVE BEEN LOADED!
BOOLEAN SetDefaultWorldLightingColors(void)
{
	static const SGPPaletteEntry pPal = { 0, 0, 0 };
	LightSetColor(&pPal);
	return TRUE;
}


static BOOLEAN LightDelete(LightTemplate*);


/****************************************************************************************
 ShutdownLightingSystem

	Closes down the lighting system. Any lights that were created are destroyed, and the
	memory attached to them freed up.

***************************************************************************************/
BOOLEAN ShutdownLightingSystem(void)
{
UINT32 uiCount;

	// free up all allocated light nodes
	for(uiCount=0; uiCount < MAX_LIGHT_TEMPLATES; uiCount++)
	{
		LightTemplate* const t = &g_light_templates[uiCount];
		if (t->lights != NULL) LightDelete(t);
	}

	return(TRUE);
}

/****************************************************************************************
 LightReset

	Removes all currently active lights, without undrawing them.

***************************************************************************************/
BOOLEAN LightReset(void)
{
	// reset all light lists
	for (UINT32 uiCount = 0; uiCount < MAX_LIGHT_TEMPLATES; ++uiCount)
	{
		LightTemplate* const t = &g_light_templates[uiCount];
		if (t->lights != NULL) LightDelete(t);
	}

	// init all light sprites
	memset(LightSprites, 0, sizeof(LightSprites));

	if (LightLoad("TRANSLUC.LHT") != g_light_templates)
	{
		DebugMsg(TOPIC_GAME, DBG_LEVEL_0, "Failed to load translucency template");
		return(FALSE);
	}

	// Loop through mercs and reset light value
	FOR_ALL_NON_PLANNING_SOLDIERS(s) s->light = NULL;

	return(TRUE);
}


/* Creates a new node, and appends it to the template list. The index into the
 * list is returned. */
static UINT16 LightCreateTemplateNode(LightTemplate* const t, const INT16 iX, const INT16 iY, const UINT8 ubLight)
{
	const UINT16 n_lights = t->n_lights;
	Assert((t->lights == NULL) == (n_lights == 0));

	t->lights = REALLOC(t->lights, LIGHT_NODE, n_lights + 1);
	if (t->lights == NULL) return 65535;

	LIGHT_NODE* const l = &t->lights[n_lights];
	l->iDX     = iX;
	l->iDY     = iY;
	l->ubLight = ubLight;
	l->uiFlags = 0;

	t->n_lights = n_lights + 1;
	return n_lights;
}


/* Adds a node to the template list. If the node does not exist, it creates a
 * new one.  Returns the index into the list. */
static UINT16 LightAddTemplateNode(LightTemplate* const t, const INT16 iX, const INT16 iY, const UINT8 ubLight)
{
	for (UINT16 i = 0; i < t->n_lights; ++i)
	{
		if (t->lights[i].iDX == iX && t->lights[i].iDY == iY) return i;
	}
	return LightCreateTemplateNode(t, iX, iY, ubLight);
}


// Adds a node to the ray casting list.
static UINT16 LightAddRayNode(LightTemplate* const t, const INT16 iX, const INT16 iY, const UINT8 ubLight, const UINT16 usFlags)
{
	const UINT16 n_rays = t->n_rays;
	Assert((t->rays == NULL) == (n_rays == 0));

	t->rays = REALLOC(t->rays, UINT16, n_rays + 1);
	if (t->rays == NULL) return 65535;

	t->rays[n_rays] = LightAddTemplateNode(t, iX, iY, ubLight) | usFlags;
	t->n_rays       = n_rays + 1;
	return n_rays;
}


// Adds a node to the ray casting list.
static UINT16 LightInsertRayNode(LightTemplate* const t, const UINT16 usIndex, const INT16 iX, const INT16 iY, const UINT8 ubLight, const UINT16 usFlags)
{
	const UINT16 n_rays = t->n_rays;
	Assert((t->rays == NULL) == (n_rays == 0));
	Assert(usIndex <= n_rays);

	t->rays = REALLOC(t->rays, UINT16, n_rays + 1);
	if (t->rays == NULL) return 65535;

	memmove(t->rays + usIndex + 1, t->rays + usIndex, (n_rays - usIndex) * sizeof(*t->rays));

	t->rays[usIndex] = LightAddTemplateNode(t, iX, iY, ubLight) | usFlags;
	t->n_rays        = n_rays + 1;
	return n_rays;
}


static BOOLEAN LightTileHasWall(INT16 iSrcX, INT16 iSrcY, INT16 iX, INT16 iY);


// Returns TRUE/FALSE if the tile at the specified tile number can block light.
static BOOLEAN LightTileBlocked(INT16 iSrcX, INT16 iSrcY, INT16 iX, INT16 iY)
{
UINT16 usTileNo, usSrcTileNo;

	Assert(gpWorldLevelData!=NULL);

	usTileNo=MAPROWCOLTOPOS(iY, iX);
	usSrcTileNo=MAPROWCOLTOPOS(iSrcY, iSrcX);

	if ( usTileNo >= NOWHERE )
	{
		return( FALSE );
	}

	if ( usSrcTileNo >= NOWHERE )
	{
		return( FALSE );
	}

	if(gpWorldLevelData[ usTileNo ].sHeight > gpWorldLevelData[ usSrcTileNo ].sHeight)
		return(TRUE);
	{
		UINT16 usTileNo;
		LEVELNODE *pStruct;

		usTileNo=MAPROWCOLTOPOS(iY, iX);

		pStruct = gpWorldLevelData[ usTileNo ].pStructHead;
		if ( pStruct != NULL )
		{
			// IF WE ARE A WINDOW, DO NOT BLOCK!
			if ( FindStructure( usTileNo, STRUCTURE_WALLNWINDOW ) != NULL )
			{
				return( FALSE );
			}
		}
	}

	return(LightTileHasWall( iSrcX, iSrcY, iX, iY));
}


// Returns TRUE/FALSE if the tile at the specified coordinates contains a wall.
static BOOLEAN LightTileHasWall(INT16 iSrcX, INT16 iSrcY, INT16 iX, INT16 iY)
{
//LEVELNODE *pStruct;
UINT16 usTileNo;
UINT16 usSrcTileNo;
INT8		bDirection;
UINT8		ubTravelCost;
//INT8		bWallCount = 0;

	Assert(gpWorldLevelData!=NULL);

	usTileNo=MAPROWCOLTOPOS(iY, iX);
	usSrcTileNo=MAPROWCOLTOPOS(iSrcY, iSrcX);

	if ( usTileNo == usSrcTileNo )
	{
		return( FALSE );
	}

	//if ( usTileNo == 10125 || usTileNo == 10126 )
	//{
	//	int i = 0;
	//}

	if ( usTileNo >= NOWHERE )
	{
		return( FALSE );
	}

	if ( usSrcTileNo >= NOWHERE )
	{
		return( FALSE );
	}

	// Get direction
	//bDirection = atan8( iX, iY, iSrcX, iSrcY );
	bDirection = atan8( iSrcX, iSrcY, iX, iY );

	ubTravelCost = gubWorldMovementCosts[ usTileNo ][ bDirection ][ 0 ];

	if ( ubTravelCost == TRAVELCOST_WALL  )
	{
		return( TRUE );
	}

	if ( IS_TRAVELCOST_DOOR( ubTravelCost ) )
	{
		ubTravelCost = DoorTravelCost( NULL, usTileNo, ubTravelCost, TRUE, NULL );

		if ( ubTravelCost == TRAVELCOST_OBSTACLE || ubTravelCost == TRAVELCOST_DOOR )
		{
			return( TRUE );
		}
	}

#if 0
	UINT16 usWallOrientation;
	pStruct = gpWorldLevelData[ usTileNo ].pStructHead;
	while ( pStruct != NULL )
	{
		if ( pStruct->usIndex < NUMBEROFTILES )
		{
			const UINT32 uiType = GetTileType(pStruct->usIndex);

			// ATE: Changed to use last decordations rather than last decal
			// Could maybe check orientation value? Depends on our
			// use of the orientation value flags..
			if((uiType >= FIRSTWALL) && (uiType <=LASTDECORATIONS ))
			{
				usWallOrientation = GetWallOrientation(pStruct->usIndex);
				bWallCount++;
			}
		}

		pStruct=pStruct->pNext;
	}

	if ( bWallCount )
	{
		// ATE: If TWO or more - assume it's BLOCKED and return TRUE
		if ( bWallCount != 1 )
		{
			return( TRUE );
		}

		switch(usWallOrientation)
		{
			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				return( iSrcX < iX );

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				return( iSrcY < iY );

		}
	}

#endif

	return(FALSE);
}


// Removes a light template from the list, and frees up the associated node memory.
static BOOLEAN LightDelete(LightTemplate* const t)
{
	if (t->lights == NULL) return FALSE;

	MemFree(t->lights);
	t->lights = NULL;

	if (t->rays != NULL)
	{
		MemFree(t->rays);
		t->rays = NULL;
	}

	if (t->name != NULL)
	{
		MemFree(t->name);
		t->name = NULL;
	}

	t->n_lights = 0;
	t->n_rays   = 0;
	return TRUE;
}


/* Returns an available slot for a new light template. If none are available,
 * (-1) is returned. */
static INT32 LightGetFree(void)
{
	for (UINT32 i = 0; i != lengthof(g_light_templates); ++i)
	{
		if (g_light_templates[i].lights == NULL) return i;
	}
	return -1;
}


/* Calculates the 2D linear distance between two points. Returns the result in
 * a DOUBLE for greater accuracy. */
static DOUBLE LinearDistanceDouble(INT16 iX1, INT16 iY1, INT16 iX2, INT16 iY2)
{
INT32 iDx, iDy;

	iDx=abs(iX1-iX2);
	iDx*=iDx;
	iDy=abs(iY1-iY2);
	iDy*=iDy;

	return(sqrt((DOUBLE)(iDx+iDy)));
}

/****************************************************************************************
	LightTrueLevel

		Returns the light level at a particular level without fake lights

***************************************************************************************/
UINT8 LightTrueLevel( INT16 sGridNo, INT8 bLevel )
{
	LEVELNODE * pNode;
	INT32 iSum;

	if (bLevel == 0)
	{
		pNode = gpWorldLevelData[sGridNo].pLandHead;
	}
	else
	{
		pNode = gpWorldLevelData[sGridNo].pRoofHead;
	}

	if (pNode == NULL)
	{
		return( ubAmbientLightLevel );
	}
	else
	{
		iSum=pNode->ubNaturalShadeLevel - (pNode->ubSumLights - pNode->ubFakeShadeLevel );

		iSum=__min(SHADE_MIN, iSum);
		iSum=__max(SHADE_MAX, iSum);
		return( (UINT8) iSum );
	}
}


// Does the addition of light values to individual LEVELNODEs in the world tile list.
static void LightAddTileNode(LEVELNODE* const pNode, const UINT8 ubShadeAdd, const BOOLEAN fFake)
{
INT16 sSum;

	pNode->ubSumLights += ubShadeAdd;
	if (fFake)
	{
		pNode->ubFakeShadeLevel += ubShadeAdd;
	}

	// Now set max
	pNode->ubMaxLights = __max( pNode->ubMaxLights, ubShadeAdd );

	sSum=pNode->ubNaturalShadeLevel - pNode->ubMaxLights;

	sSum=__min(SHADE_MIN, sSum);
	sSum=__max(SHADE_MAX, sSum);

	pNode->ubShadeLevel=(UINT8)sSum;
}


// Does the subtraction of light values to individual LEVELNODEs in the world tile list.
static void LightSubtractTileNode(LEVELNODE* const pNode, const UINT8 ubShadeSubtract, const BOOLEAN fFake)
{
INT16 sSum;

	if (ubShadeSubtract > pNode->ubSumLights )
	{
		pNode->ubSumLights = 0;
	}
	else
	{
		pNode->ubSumLights -= ubShadeSubtract;
	}
	if (fFake)
	{
		if (ubShadeSubtract > pNode->ubFakeShadeLevel)
		{
			pNode->ubFakeShadeLevel = 0;
		}
		else
		{
			pNode->ubFakeShadeLevel -= ubShadeSubtract;
		}
	}

	// Now set max
	pNode->ubMaxLights = __min( pNode->ubMaxLights, pNode->ubSumLights );


	sSum=pNode->ubNaturalShadeLevel - pNode->ubMaxLights;

	sSum=__min(SHADE_MIN, sSum);
	sSum=__max(SHADE_MAX, sSum);

	pNode->ubShadeLevel=(UINT8)sSum;
}


static BOOLEAN LightIlluminateWall(INT16 iSourceX, INT16 iSourceY, INT16 iTileX, INT16 iTileY, LEVELNODE* pStruct);


// Adds a specified amount of light to all objects on a given tile.
static BOOLEAN LightAddTile(const INT16 iSrcX, const INT16 iSrcY, const INT16 iX, const INT16 iY, const UINT8 ubShade, const UINT32 uiFlags, const BOOLEAN fOnlyWalls)
{
LEVELNODE *pLand, *pStruct, *pObject, *pMerc, *pRoof, *pOnRoof;
UINT8 ubShadeAdd;
UINT32 uiTile;
BOOLEAN fLitWall=FALSE;
BOOLEAN fFake;

	Assert(gpWorldLevelData!=NULL);

	uiTile= MAPROWCOLTOPOS( iY, iX );

	if ( uiTile >= NOWHERE )
	{
		return( FALSE );
	}

	gpWorldLevelData[uiTile].uiFlags|=MAPELEMENT_REDRAW;

	//if((uiFlags&LIGHT_BACKLIGHT) && !(uiFlags&LIGHT_ROOF_ONLY))
	//	ubShadeAdd=ubShade*7/10;
	//else
		ubShadeAdd=ubShade;


	if (uiFlags&LIGHT_FAKE)
	{
		fFake = TRUE;
	}
	else
	{
		fFake = FALSE;
	}

	if(!(uiFlags&LIGHT_ROOF_ONLY) || (uiFlags&LIGHT_EVERYTHING))
	{
		pStruct = gpWorldLevelData[uiTile].pStructHead;
		while(pStruct!=NULL)
		{
			if ( pStruct->usIndex < NUMBEROFTILES )
			{
				if((gTileDatabase[ pStruct->usIndex ].fType != FIRSTCLIFFHANG) || (uiFlags&LIGHT_EVERYTHING))
				{
					if( (uiFlags&LIGHT_IGNORE_WALLS ) || gfCaves )
						LightAddTileNode(pStruct, ubShadeAdd, FALSE);
					else if(LightIlluminateWall(iSrcX, iSrcY, iX, iY, pStruct))
					{
						if(LightTileHasWall(iSrcX, iSrcY, iX, iY))
							fLitWall=TRUE;

						// ATE: Limit shade for walls if in caves
						if ( fLitWall && gfCaves )
						{
							LightAddTileNode(pStruct, __min(ubShadeAdd, SHADE_MAX + 5), FALSE);
						}
						else if ( fLitWall )
						{
							LightAddTileNode(pStruct, ubShadeAdd, FALSE);
						}
						else if ( !fOnlyWalls )
						{
							LightAddTileNode(pStruct, ubShadeAdd, FALSE);
						}
					}
				}
			}
			else
			{
				LightAddTileNode(pStruct, ubShadeAdd, FALSE);
			}
			pStruct=pStruct->pNext;
		}

		ubShadeAdd=ubShade;

    if ( !fOnlyWalls )
    {
		  pLand = gpWorldLevelData[uiTile].pLandHead;

		  while( pLand )
		  {
			  if( gfCaves || !fLitWall )
			  {
				  LightAddTileNode(pLand, ubShadeAdd, fFake);
			  }
			  pLand=pLand->pNext;
		  }

		  pObject = gpWorldLevelData[uiTile].pObjectHead;
		  while(pObject!=NULL)
		  {
			  if ( pObject->usIndex < NUMBEROFTILES )
			  {
				  LightAddTileNode(pObject, ubShadeAdd, FALSE);
			  }
			  pObject=pObject->pNext;
		  }

		  if(uiFlags&LIGHT_BACKLIGHT)
			  ubShadeAdd=(INT16)ubShade*7/10;

		  pMerc = gpWorldLevelData[uiTile].pMercHead;
		  while(pMerc!=NULL)
		  {
			  LightAddTileNode(pMerc, ubShadeAdd, FALSE);
			  pMerc=pMerc->pNext;
		  }
	  }
  }

	if((uiFlags&LIGHT_ROOF_ONLY) || (uiFlags&LIGHT_EVERYTHING))
	{
		pRoof = gpWorldLevelData[uiTile].pRoofHead;
		while(pRoof!=NULL)
		{
			if ( pRoof->usIndex < NUMBEROFTILES )
			{
				LightAddTileNode(pRoof, ubShadeAdd, fFake);
			}
			pRoof=pRoof->pNext;
		}

		pOnRoof = gpWorldLevelData[uiTile].pOnRoofHead;
		while(pOnRoof!=NULL)
		{
			LightAddTileNode(pOnRoof, ubShadeAdd, FALSE);

			pOnRoof=pOnRoof->pNext;
		}
	}
	return(TRUE);
}


// Subtracts a specified amount of light to a given tile.
static BOOLEAN LightSubtractTile(const INT16 iSrcX, const INT16 iSrcY, const INT16 iX, const INT16 iY, const UINT8 ubShade, const UINT32 uiFlags, const BOOLEAN fOnlyWalls)
{
LEVELNODE *pLand, *pStruct, *pObject, *pMerc, *pRoof, *pOnRoof;
UINT8 ubShadeSubtract;
UINT32 uiTile;
BOOLEAN fLitWall=FALSE;
BOOLEAN fFake; // only passed in to land and roof layers; others get fed FALSE

	Assert(gpWorldLevelData!=NULL);

	uiTile= MAPROWCOLTOPOS( iY, iX );

	if ( uiTile >= NOWHERE )
	{
		return( FALSE );
	}


	gpWorldLevelData[uiTile].uiFlags|=MAPELEMENT_REDRAW;

//	if((uiFlags&LIGHT_BACKLIGHT) && !(uiFlags&LIGHT_ROOF_ONLY))
//		ubShadeSubtract=ubShade*7/10;
//	else
		ubShadeSubtract=ubShade;

	if (uiFlags&LIGHT_FAKE)
	{
		fFake = TRUE;
	}
	else
	{
		fFake = FALSE;
	}

	if(!(uiFlags&LIGHT_ROOF_ONLY) || (uiFlags&LIGHT_EVERYTHING))
	{
		pStruct = gpWorldLevelData[uiTile].pStructHead;
		while(pStruct!=NULL)
		{
			if ( pStruct->usIndex < NUMBEROFTILES )
			{
				if((gTileDatabase[ pStruct->usIndex ].fType != FIRSTCLIFFHANG) || (uiFlags&LIGHT_EVERYTHING))
				{
					if( (uiFlags&LIGHT_IGNORE_WALLS ) || gfCaves )
						LightSubtractTileNode(pStruct, ubShadeSubtract, FALSE);
					else if(LightIlluminateWall(iSrcX, iSrcY, iX, iY, pStruct))
					{
						if(LightTileHasWall( iSrcX, iSrcY, iX, iY))
							fLitWall=TRUE;

						// ATE: Limit shade for walls if in caves
						if ( fLitWall && gfCaves )
						{
							LightSubtractTileNode(pStruct, __max(ubShadeSubtract - 5, 0), FALSE);
						}
						else if ( fLitWall )
						{
							LightSubtractTileNode(pStruct, ubShadeSubtract, FALSE);
						}
            else if ( !fOnlyWalls )
            {
						  LightSubtractTileNode(pStruct, ubShadeSubtract, FALSE);
            }
					}
				}
			}
      else
      {
				LightSubtractTileNode(pStruct, ubShadeSubtract, FALSE);
      }
			pStruct=pStruct->pNext;
		}

		ubShadeSubtract=ubShade;

    if ( !fOnlyWalls )
    {
		  pLand = gpWorldLevelData[uiTile].pLandHead;

		  while( pLand )
		  {
			  if( gfCaves || !fLitWall )
			  {
				  LightSubtractTileNode(pLand, ubShadeSubtract, fFake);
			  }
			  pLand=pLand->pNext;
		  }

		  pObject = gpWorldLevelData[uiTile].pObjectHead;
		  while(pObject!=NULL)
		  {
			  if ( pObject->usIndex < NUMBEROFTILES )
			  {
				  LightSubtractTileNode(pObject, ubShadeSubtract, FALSE);
			  }
			  pObject=pObject->pNext;
		  }

		  if(uiFlags&LIGHT_BACKLIGHT)
			  ubShadeSubtract=(INT16)ubShade*7/10;

		  pMerc = gpWorldLevelData[uiTile].pMercHead;
		  while(pMerc!=NULL)
		  {
			  LightSubtractTileNode(pMerc, ubShadeSubtract, FALSE);
			  pMerc=pMerc->pNext;
		  }
	  }
  }

	if((uiFlags&LIGHT_ROOF_ONLY) || (uiFlags&LIGHT_EVERYTHING))
	{
		pRoof = gpWorldLevelData[uiTile].pRoofHead;
		while(pRoof!=NULL)
		{
			if ( pRoof->usIndex < NUMBEROFTILES )
			{
				LightSubtractTileNode(pRoof, ubShadeSubtract, fFake);
			}
			pRoof=pRoof->pNext;
		}

		pOnRoof = gpWorldLevelData[uiTile].pOnRoofHead;
		while(pOnRoof!=NULL)
		{
			if ( pOnRoof->usIndex < NUMBEROFTILES )
			{
				LightSubtractTileNode(pOnRoof, ubShadeSubtract, FALSE);
			}
			pOnRoof=pOnRoof->pNext;
		}
	}

	return(TRUE);
}


// Sets the natural light level (as well as the current) on individual LEVELNODEs.
static void LightSetNaturalTileNode(LEVELNODE* pNode, UINT8 ubShade)
{
		Assert(pNode!=NULL);

		pNode->ubSumLights=0;
		pNode->ubMaxLights=0;
		pNode->ubNaturalShadeLevel = ubShade;
		pNode->ubShadeLevel = ubShade;
}


/* Sets the natural light value of all objects on a given tile to the specified
 * value.  This is the light value a tile has with no artificial lighting
 * affecting it. */
static BOOLEAN LightSetNaturalTile(INT16 iX, INT16 iY, UINT8 ubShade)
{
LEVELNODE *pLand, *pStruct, *pObject, *pRoof, *pOnRoof, *pTopmost, *pMerc;
UINT32 uiIndex;

	CHECKF(gpWorldLevelData!=NULL);

	uiIndex = MAPROWCOLTOPOS( iY, iX );

	Assert(uiIndex!=0xffff);

	ubShade=__max(SHADE_MAX, ubShade);
	ubShade=__min(SHADE_MIN, ubShade);

	pLand = gpWorldLevelData[ uiIndex ].pLandHead;

	while(pLand!=NULL)
	{
		LightSetNaturalTileNode(pLand, ubShade);
		pLand=pLand->pNext;
	}

	pStruct = gpWorldLevelData[ uiIndex ].pStructHead;

	while(pStruct!=NULL)
	{
		LightSetNaturalTileNode(pStruct, ubShade);
		pStruct=pStruct->pNext;
	}

	pObject = gpWorldLevelData[ uiIndex ].pObjectHead;
	while(pObject!=NULL)
	{
		LightSetNaturalTileNode(pObject, ubShade);
		pObject=pObject->pNext;
	}

	pRoof = gpWorldLevelData[ uiIndex ].pRoofHead;
	while(pRoof!=NULL)
	{
		LightSetNaturalTileNode(pRoof, ubShade);
		pRoof=pRoof->pNext;
	}

	pOnRoof = gpWorldLevelData[ uiIndex ].pOnRoofHead;
	while(pOnRoof!=NULL)
	{
		LightSetNaturalTileNode(pOnRoof, ubShade);
		pOnRoof=pOnRoof->pNext;
	}

	pTopmost = gpWorldLevelData[ uiIndex ].pTopmostHead;
	while(pTopmost!=NULL)
	{
		LightSetNaturalTileNode(pTopmost, ubShade);
		pTopmost=pTopmost->pNext;
	}

	pMerc = gpWorldLevelData[ uiIndex ].pMercHead;
	while(pMerc!=NULL)
	{
		LightSetNaturalTileNode(pMerc, ubShade);
		pMerc=pMerc->pNext;
	}
	return(TRUE);
}


/* Resets the light level of individual LEVELNODEs to the value contained in
 * the natural light level. */
static void LightResetTileNode(LEVELNODE* pNode)
{
	pNode->ubSumLights=0;
	pNode->ubMaxLights=0;
	pNode->ubShadeLevel = pNode->ubNaturalShadeLevel;
	pNode->ubFakeShadeLevel = 0;
}


/* Resets the light values of all objects on a given tile to the "natural"
 * light level for that tile. */
static BOOLEAN LightResetTile(INT16 iX, INT16 iY)
{
LEVELNODE *pLand, *pStruct, *pObject, *pRoof, *pOnRoof, *pTopmost, *pMerc;
UINT32 uiTile;

	CHECKF(gpWorldLevelData!=NULL);

	uiTile = MAPROWCOLTOPOS( iY, iX );

	CHECKF(uiTile!=0xffff);

	pLand = gpWorldLevelData[uiTile].pLandHead;

	while(pLand!=NULL)
	{
		LightResetTileNode(pLand);
		pLand=pLand->pNext;
	}

	pStruct = gpWorldLevelData[ uiTile ].pStructHead;

	while(pStruct!=NULL)
	{
		LightResetTileNode(pStruct);
		pStruct=pStruct->pNext;
	}

	pObject = gpWorldLevelData[ uiTile ].pObjectHead;
	while(pObject!=NULL)
	{
		LightResetTileNode(pObject);
		pObject=pObject->pNext;
	}

	pRoof = gpWorldLevelData[ uiTile ].pRoofHead;
	while(pRoof!=NULL)
	{
		LightResetTileNode(pRoof);
		pRoof=pRoof->pNext;
	}

	pOnRoof = gpWorldLevelData[ uiTile ].pOnRoofHead;
	while(pOnRoof!=NULL)
	{
		LightResetTileNode(pOnRoof);
		pOnRoof=pOnRoof->pNext;
	}

	pTopmost = gpWorldLevelData[ uiTile ].pTopmostHead;
	while(pTopmost!=NULL)
	{
		LightResetTileNode(pTopmost);
		pTopmost=pTopmost->pNext;
	}

	pMerc = gpWorldLevelData[ uiTile ].pMercHead;
	while(pMerc!=NULL)
	{
		LightResetTileNode(pMerc);
		pMerc=pMerc->pNext;
	}

	return(TRUE);
}


// Resets all tiles on the map to their baseline values.
static BOOLEAN LightResetAllTiles(void)
{
INT16 iCountY, iCountX;

	for(iCountY=0; iCountY < WORLD_ROWS; iCountY++)
		for(iCountX=0; iCountX < WORLD_COLS; iCountX++)
			LightResetTile(iCountX, iCountY);

	return(TRUE);
}


// Creates a new node, and adds it to the end of a light list.
static BOOLEAN LightAddNode(LightTemplate* const t, const INT16 iHotSpotX, const INT16 iHotSpotY, INT16 iX, INT16 iY, const UINT8 ubIntensity, const UINT16 uiFlags)
{
DOUBLE dDistance;
UINT8 ubShade;
INT32 iLightDecay;

	dDistance=LinearDistanceDouble(iX, iY, iHotSpotX, iHotSpotY);
	dDistance/=DISTANCE_SCALE;

	iLightDecay=(INT32)(dDistance*LIGHT_DECAY);

	if((iLightDecay >= (INT32)ubIntensity))
		ubShade=0;
	else
		ubShade=ubIntensity-(UINT8)iLightDecay;

	iX/=DISTANCE_SCALE;
	iY/=DISTANCE_SCALE;

	LightAddRayNode(t, iX, iY, ubShade, uiFlags);
	return(TRUE);
}


// Creates a new node, and inserts it after the specified node.
static BOOLEAN LightInsertNode(LightTemplate* const t, const UINT16 usLightIns, const INT16 iHotSpotX, const INT16 iHotSpotY, INT16 iX, INT16 iY, const UINT8 ubIntensity, const UINT16 uiFlags)
{
DOUBLE dDistance;
UINT8 ubShade;
INT32 iLightDecay;

	dDistance=LinearDistanceDouble(iX, iY, iHotSpotX, iHotSpotY);
	dDistance/=DISTANCE_SCALE;

	iLightDecay=(INT32)(dDistance*LIGHT_DECAY);

	if((iLightDecay >= (INT32)ubIntensity))
		ubShade=0;
	else
		ubShade=ubIntensity-(UINT8)iLightDecay;

	iX/=DISTANCE_SCALE;
	iY/=DISTANCE_SCALE;

	LightInsertRayNode(t, usLightIns, iX, iY, ubShade, uiFlags);

	return(TRUE);
}


/* Traverses the linked list until a node with the LIGHT_NEW_RAY marker, and
 * returns the pointer. If the end of list is reached, NULL is returned. */
static UINT16 LightFindNextRay(const LightTemplate* const t, const UINT16 usIndex)
{
	UINT16 usNodeIndex = usIndex;
	while ((usNodeIndex < t->n_rays) && !(t->rays[usNodeIndex] & LIGHT_NEW_RAY))
		usNodeIndex++;

	return(usNodeIndex);
}


/* Casts a ray from an origin to an end point, creating nodes and adding them
 * to the light list. */
static BOOLEAN LightCastRay(LightTemplate* const t, const INT16 iStartX, const INT16 iStartY, const INT16 iEndPointX, const INT16 iEndPointY, const UINT8 ubStartIntens, const UINT8 ubEndIntens)
{
INT16 AdjUp, AdjDown, ErrorTerm, XAdvance, XDelta, YDelta;
INT32 WholeStep, InitialPixelCount, FinalPixelCount, i, j, RunLength;
INT16 iXPos, iYPos, iEndY, iEndX;
UINT16 usCurNode=0, usFlags=0;
BOOLEAN fInsertNodes=FALSE;

		if((iEndPointX > 0) && (iEndPointY > 0))
			usFlags=LIGHT_BACKLIGHT;

   /* We'll always draw top to bottom, to reduce the number of cases we have to
   handle, and to make lines between the same endpoints draw the same pixels */
		if (iStartY > iEndPointY)
		{
			iXPos=iEndPointX;
			iEndX=iStartX;
			iYPos=iEndPointY;
			iEndY=iStartY;
			fInsertNodes=TRUE;
		}
		else
		{
			iXPos=iStartX;
			iEndX=iEndPointX;
			iYPos=iStartY;
			iEndY=iEndPointY;
		}

	 /* Figure out whether we're going left or right, and how far we're
      going horizontally */
   if ((XDelta = (iEndX - iXPos)) < 0)
   {
      XAdvance = -1;
      XDelta = -XDelta;
   }
   else
   {
      XAdvance = 1;
   }
   /* Figure out how far we're going vertically */
   YDelta = iEndY - iYPos;

	 // Check for 0 length ray
	 if((XDelta==0) && (YDelta==0))
		 return(FALSE);

	//DebugMsg(TOPIC_GAME, DBG_LEVEL_0, String("Drawing (%d,%d) to (%d,%d)", iXPos, iYPos, iEndX, iEndY));
	LightAddNode(t, 32767, 32767, 32767, 32767, 0, LIGHT_NEW_RAY);
	if (fInsertNodes) usCurNode = t->n_rays;

		/* Special-case horizontal, vertical, and diagonal lines, for speed
      and to avoid nasty boundary conditions and division by 0 */
   if (XDelta == 0)
   {
     /* Vertical line */
		 if(fInsertNodes)
		 {
				for (i=0; i<=YDelta; i++)
				{
					LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
		 }
		 else
		 {
				for (i=0; i<=YDelta; i++)
				{
					LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
		 }
     return(TRUE);
   }
   if (YDelta == 0)
   {
      /* Horizontal line */
		 if(fInsertNodes)
		 {
				for (i=0; i<=XDelta; i++)
				{
					LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
			  }
		 }
		 else
		 {
				for (i=0; i<=XDelta; i++)
				{
					LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
			  }
		 }
      return(TRUE);
   }
   if (XDelta == YDelta)
   {
      /* Diagonal line */
		 if(fInsertNodes)
		 {
	      for (i=0; i<=XDelta; i++)
		    {
					LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
					iYPos++;
				}
		 }
		 else
		 {
				for (i=0; i<=XDelta; i++)
				{
					LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
					iYPos++;
				}
		 }
      return(TRUE);
   }

   /* Determine whether the line is X or Y major, and handle accordingly */
   if (XDelta >= YDelta)
   {
			/* X major line */
      /* Minimum # of pixels in a run in this line */
      WholeStep = XDelta / YDelta;

      /* Error term adjust each time Y steps by 1; used to tell when one
         extra pixel should be drawn as part of a run, to account for
         fractional steps along the X axis per 1-pixel steps along Y */
      AdjUp = (XDelta % YDelta) * 2;

      /* Error term adjust when the error term turns over, used to factor
         out the X step made at that time */
      AdjDown = YDelta * 2;

      /* Initial error term; reflects an initial step of 0.5 along the Y
         axis */
      ErrorTerm = (XDelta % YDelta) - (YDelta * 2);

      /* The initial and last runs are partial, because Y advances only 0.5
         for these runs, rather than 1. Divide one full run, plus the
         initial pixel, between the initial and last runs */
      InitialPixelCount = (WholeStep / 2) + 1;
      FinalPixelCount = InitialPixelCount;

      /* If the basic run length is even and there's no fractional
         advance, we have one pixel that could go to either the initial
         or last partial run, which we'll arbitrarily allocate to the
         last run */
      if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
      {
         InitialPixelCount--;
      }
      /* If there're an odd number of pixels per run, we have 1 pixel that can't
         be allocated to either the initial or last partial run, so we'll add 0.5
         to error term so this pixel will be handled by the normal full-run loop */
      if ((WholeStep & 0x01) != 0)
      {
         ErrorTerm += YDelta;
      }
      /* Draw the first, partial run of pixels */
      //DrawHorizontalRun(&ScreenPtr, XAdvance, InitialPixelCount, Color);
			if(fInsertNodes)
			{
				for (i=0; i<InitialPixelCount; i++)
				{
					LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
				}
			}
			else
			{
				for (i=0; i<InitialPixelCount; i++)
		    {
					LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
				}
			}
			iYPos++;

      /* Draw all full runs */
      for (j=0; j<(YDelta-1); j++)
      {
         RunLength = WholeStep;  /* run is at least this long */
         /* Advance the error term and add an extra pixel if the error
            term so indicates */
         if ((ErrorTerm += AdjUp) > 0)
         {
            RunLength++;
            ErrorTerm -= AdjDown;   /* reset the error term */
         }
         /* Draw this scan line's run */
         //DrawHorizontalRun(&ScreenPtr, XAdvance, RunLength, Color);
				 if(fInsertNodes)
				 {
						for (i=0; i<RunLength; i++)
						{
							LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
							iXPos+=XAdvance;
						}
				 }
				 else
				 {
						for (i=0; i<RunLength; i++)
						{
							LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
							iXPos+=XAdvance;
						}
				 }
				iYPos++;
      }
      /* Draw the final run of pixels */
      //DrawHorizontalRun(&ScreenPtr, XAdvance, FinalPixelCount, Color);
      if(fInsertNodes)
			{
				for (i=0; i<FinalPixelCount; i++)
				{
					LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
				}
			}
			else
			{
				for (i=0; i<FinalPixelCount; i++)
				{
					LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
				}
			}
			iYPos++;
   }
   else
   {
		 /* Y major line */

      /* Minimum # of pixels in a run in this line */
      WholeStep = YDelta / XDelta;

      /* Error term adjust each time X steps by 1; used to tell when 1 extra
         pixel should be drawn as part of a run, to account for
         fractional steps along the Y axis per 1-pixel steps along X */
      AdjUp = (YDelta % XDelta) * 2;

      /* Error term adjust when the error term turns over, used to factor
         out the Y step made at that time */
      AdjDown = XDelta * 2;

      /* Initial error term; reflects initial step of 0.5 along the X axis */
      ErrorTerm = (YDelta % XDelta) - (XDelta * 2);

      /* The initial and last runs are partial, because X advances only 0.5
         for these runs, rather than 1. Divide one full run, plus the
         initial pixel, between the initial and last runs */
      InitialPixelCount = (WholeStep / 2) + 1;
      FinalPixelCount = InitialPixelCount;

      /* If the basic run length is even and there's no fractional advance, we
         have 1 pixel that could go to either the initial or last partial run,
         which we'll arbitrarily allocate to the last run */
      if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
      {
         InitialPixelCount--;
      }
      /* If there are an odd number of pixels per run, we have one pixel
         that can't be allocated to either the initial or last partial
         run, so we'll add 0.5 to the error term so this pixel will be
         handled by the normal full-run loop */
      if ((WholeStep & 0x01) != 0)
      {
         ErrorTerm += XDelta;
      }
      /* Draw the first, partial run of pixels */
			if(fInsertNodes)
			{
				for (i=0; i<InitialPixelCount; i++)
				{
					LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
			}
			else
			{
				for (i=0; i<InitialPixelCount; i++)
				{
					LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
			}
			iXPos+=XAdvance;
      //DrawVerticalRun(&ScreenPtr, XAdvance, InitialPixelCount, Color);

      /* Draw all full runs */
      for (j=0; j<(XDelta-1); j++)
      {
         RunLength = WholeStep;  /* run is at least this long */
         /* Advance the error term and add an extra pixel if the error
            term so indicates */
         if ((ErrorTerm += AdjUp) > 0)
         {
            RunLength++;
            ErrorTerm -= AdjDown;   /* reset the error term */
         }
         /* Draw this scan line's run */
         //DrawVerticalRun(&ScreenPtr, XAdvance, RunLength, Color);
				if(fInsertNodes)
				{
					for (i=0; i<RunLength; i++)
					{
						LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
						iYPos++;
					}
				}
				else
				{
					for (i=0; i<RunLength; i++)
					{
						LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
						iYPos++;
					}
				}
				iXPos+=XAdvance;
      }
      /* Draw the final run of pixels */
      //DrawVerticalRun(&ScreenPtr, XAdvance, FinalPixelCount, Color);
			if(fInsertNodes)
			{
				for (i=0; i<FinalPixelCount; i++)
				{
					LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
			}
			else
			{
				for (i=0; i<FinalPixelCount; i++)
				{
					LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
			}
			iXPos+=XAdvance;
   }
	 return(TRUE);
}


// Creates an elliptical light, taking two radii.
static BOOLEAN LightGenerateElliptical(LightTemplate* const t, const UINT8 iIntensity, const INT16 iA, const INT16 iB)
{
INT16 iX, iY;
INT32 WorkingX, WorkingY;
DOUBLE ASquared;
DOUBLE BSquared;
DOUBLE Temp;

	iX=0;
	iY=0;
	ASquared = (DOUBLE) iA * iA;
	BSquared = (DOUBLE) iB * iB;

   /* Draw the four symmetric arcs for which X advances faster (that is,
      for which X is the major axis) */
   /* Draw the initial top & bottom points */
	LightCastRay(t, iX, iY, (INT16)iX, (INT16)(iY+iB), iIntensity, 1);
	LightCastRay(t, iX, iY, (INT16)iX, (INT16)(iY-iB), iIntensity, 1);

   /* Draw the four arcs */
   for (WorkingX = 0; ; )
	 {
      /* Advance one pixel along the X axis */
      WorkingX++;

      /* Calculate the corresponding point along the Y axis. Guard
         against floating-point roundoff making the intermediate term
         less than 0 */
      Temp = BSquared - (BSquared * WorkingX * WorkingX / ASquared);

			if(Temp >= 0)
				WorkingY= (INT32)(sqrt(Temp)+0.5);
			else
				WorkingY=0;

      /* Stop if X is no longer the major axis (the arc has passed the
         45-degree point) */
      if(((DOUBLE)WorkingY/BSquared) <= ((DOUBLE)WorkingX/ASquared))
         break;

      /* Draw the 4 symmetries of the current point */
			LightCastRay(t, iX, iY, (INT16)(iX+WorkingX), (INT16)(iY-WorkingY), iIntensity, 1);
			LightCastRay(t, iX, iY, (INT16)(iX-WorkingX), (INT16)(iY-WorkingY), iIntensity, 1);
			LightCastRay(t, iX, iY, (INT16)(iX+WorkingX), (INT16)(iY+WorkingY), iIntensity, 1);
			LightCastRay(t, iX, iY, (INT16)(iX-WorkingX), (INT16)(iY+WorkingY), iIntensity, 1);
   }

   /* Draw the four symmetric arcs for which Y advances faster (that is,
      for which Y is the major axis) */
   /* Draw the initial left & right points */
	LightCastRay(t, iX, iY, (INT16)(iX+iA), iY, iIntensity, 1);
	LightCastRay(t, iX, iY, (INT16)(iX-iA), iY, iIntensity, 1);

   /* Draw the four arcs */
   for (WorkingY = 0; ; )
	 {
      /* Advance one pixel along the Y axis */
      WorkingY++;

      /* Calculate the corresponding point along the X axis. Guard
         against floating-point roundoff making the intermediate term
         less than 0 */
      Temp = ASquared - (ASquared * WorkingY * WorkingY / BSquared);

			if(Temp >= 0)
				WorkingX = (INT32)(sqrt(Temp)+0.5);
			else
				WorkingX = 0;

      /* Stop if Y is no longer the major axis (the arc has passed the
         45-degree point) */
      if (((DOUBLE)WorkingX/ASquared) < ((DOUBLE)WorkingY/BSquared))
         break;

      /* Draw the 4 symmetries of the current point */
			LightCastRay(t, iX, iY, (INT16)(iX+WorkingX), (INT16)(iY-WorkingY), iIntensity, 1);
			LightCastRay(t, iX, iY, (INT16)(iX-WorkingX), (INT16)(iY-WorkingY), iIntensity, 1);
			LightCastRay(t, iX, iY, (INT16)(iX+WorkingX), (INT16)(iY+WorkingY), iIntensity, 1);
			LightCastRay(t, iX, iY, (INT16)(iX-WorkingX), (INT16)(iY+WorkingY), iIntensity, 1);
   }

	return(TRUE);
}


// Creates an square light, taking two radii.
static BOOLEAN LightGenerateSquare(LightTemplate* const t, const UINT8 iIntensity, const INT16 iA, const INT16 iB)
{
INT16 iX, iY;

	for(iX=0-iA; iX <= 0+iA; iX++)
		LightCastRay(t, 0, 0, iX, (INT16)(0-iB), iIntensity, 1);

	for(iX=0-iA; iX <= 0+iA; iX++)
		LightCastRay(t, 0, 0, iX, (INT16)(0+iB), iIntensity, 1);

	for(iY=0-iB; iY <= 0+iB; iY++)
		LightCastRay(t, 0, 0, (INT16)(0-iA), iY, iIntensity, 1);

	for(iY=0-iB; iY <= 0+iB; iY++)
		LightCastRay(t, 0, 0, (INT16)(0+iA), iY, iIntensity, 1);


	/*for(iY=0-iB; iY <= 0+iB; iY++)
		LightCastRay(t, 0, iY, (INT16)(0+iA), iY, iIntensity, 1);

	for(iY=0+iB; iY >= 0-iB; iY--)
		LightCastRay(t, 0, iY, (INT16)(0-iA), iY, iIntensity, 1);

	for(iX=0-iA; iX <= 0+iA; iX++)
		LightCastRay(t, iX, 0, iX, (INT16)(0+iB), iIntensity, 1);

	for(iX=0+iA; iX >= 0-iA; iX--)
		LightCastRay(t, iX, 0, iX, (INT16)(0-iB), iIntensity, 1); */

	return(TRUE);
}


/****************************************************************************************
	LightSetBaseLevel

		Sets the current and natural light settings for all tiles in the world.

***************************************************************************************/
BOOLEAN LightSetBaseLevel(UINT8 iIntensity)
{
	INT16 iCountY, iCountX;

	ubAmbientLightLevel=iIntensity;

	if( !gfEditMode )
	{
		// Loop for all good guys in tactical map and add a light if required
		FOR_ALL_MERCS(i)
		{
			SOLDIERTYPE* const s = *i;
			if (s->bTeam == gbPlayerNum) ReCreateSoldierLight(s);
		}
	}

	for(iCountY=0; iCountY < WORLD_ROWS; iCountY++)
		for(iCountX=0; iCountX < WORLD_COLS; iCountX++)
			LightSetNaturalTile(iCountX, iCountY, iIntensity);

	LightSpriteRenderAll();

	if(iIntensity >= LIGHT_DUSK_CUTOFF)
		RenderSetShadows(FALSE);
	else
		RenderSetShadows(TRUE);


	return(TRUE);
}


BOOLEAN LightAddBaseLevel(const UINT8 iIntensity)
{
INT16 iCountY, iCountX;

	ubAmbientLightLevel=__max(SHADE_MAX, ubAmbientLightLevel-iIntensity);

	for(iCountY=0; iCountY < WORLD_ROWS; iCountY++)
		for(iCountX=0; iCountX < WORLD_COLS; iCountX++)
			LightAddTile(iCountX, iCountY, iCountX, iCountY, iIntensity, LIGHT_IGNORE_WALLS|LIGHT_EVERYTHING, FALSE);

	if(ubAmbientLightLevel >= LIGHT_DUSK_CUTOFF)
		RenderSetShadows(FALSE);
	else
		RenderSetShadows(TRUE);

	return(TRUE);
}


BOOLEAN LightSubtractBaseLevel(const UINT8 iIntensity)
{
INT16 iCountY, iCountX;

	ubAmbientLightLevel=__min(SHADE_MIN, ubAmbientLightLevel+iIntensity);

	for(iCountY=0; iCountY < WORLD_ROWS; iCountY++)
		for(iCountX=0; iCountX < WORLD_COLS; iCountX++)
			LightSubtractTile(iCountX, iCountY, iCountX, iCountY, iIntensity, LIGHT_IGNORE_WALLS|LIGHT_EVERYTHING, FALSE);

	if(ubAmbientLightLevel >= LIGHT_DUSK_CUTOFF)
		RenderSetShadows(FALSE);
	else
		RenderSetShadows(TRUE);

	return(TRUE);
}


LightTemplate* LightCreateOmni(const UINT8 ubIntensity, const INT16 iRadius)
{
INT32 iLight;

	iLight=LightGetFree();
	if (iLight == -1) return NULL;
	LightTemplate* const t = &g_light_templates[iLight];

	LightGenerateElliptical(t, ubIntensity, iRadius * DISTANCE_SCALE, iRadius * DISTANCE_SCALE);

	char usName[14];
	sprintf(usName, "LTO%d.LHT", iRadius);
	t->name = MALLOCN(char, strlen(usName) + 1);
	strcpy(t->name, usName);

	return t;
}


// Creates a square light
static LightTemplate* LightCreateSquare(UINT8 ubIntensity, INT16 iRadius1, INT16 iRadius2)
{
INT32 iLight;

	iLight=LightGetFree();
	if (iLight == -1) return NULL;
	LightTemplate* const t = &g_light_templates[iLight];

	LightGenerateSquare(t, ubIntensity, iRadius1 * DISTANCE_SCALE, iRadius2 * DISTANCE_SCALE);

	char usName[14];
	sprintf(usName, "LTS%d-%d.LHT", iRadius1, iRadius2);
	t->name = MALLOCN(char, strlen(usName) + 1);
	strcpy(t->name, usName);

	return t;
}


// Creates an elliptical light (two separate radii)
static LightTemplate* LightCreateElliptical(UINT8 ubIntensity, INT16 iRadius1, INT16 iRadius2)
{
INT32 iLight;

	iLight=LightGetFree();
	if (iLight == -1) return NULL;
	LightTemplate* const t = &g_light_templates[iLight];

	LightGenerateElliptical(t, ubIntensity, iRadius1 * DISTANCE_SCALE, iRadius2 * DISTANCE_SCALE);

	char usName[14];
	sprintf(usName, "LTE%d-%d.LHT", iRadius1, iRadius2);
	t->name = MALLOCN(char, strlen(usName) + 1);
	strcpy(t->name, usName);

	return t;
}


// Renders a light template at the specified X,Y coordinates.
static BOOLEAN LightIlluminateWall(INT16 iSourceX, INT16 iSourceY, INT16 iTileX, INT16 iTileY, LEVELNODE* pStruct)
{
//	return( LightTileHasWall( iSourceX, iSourceY, iTileX, iTileY ) );

#if 0
	UINT16 usWallOrientation = GetWallOrientation(pStruct->usIndex);
	switch(usWallOrientation)
	{
		case NO_ORIENTATION:
			return(TRUE);

		case INSIDE_TOP_RIGHT:
		case OUTSIDE_TOP_RIGHT:
			return(iSourceX >= iTileX);

		case INSIDE_TOP_LEFT:
		case OUTSIDE_TOP_LEFT:
			return(iSourceY >= iTileY);

	}
	return(FALSE);

#endif

  return( TRUE );
}


BOOLEAN LightDraw(const LIGHT_SPRITE* const l)
{
UINT32 uiFlags;
INT32		iOldX, iOldY;
BOOLEAN	fBlocked = FALSE;
BOOLEAN fOnlyWalls;

//MAP_ELEMENT * pMapElement;

	const LightTemplate* const t = l->light_template;
	if (t->lights == NULL) return FALSE;

	// clear out all the flags
	for (UINT16 uiCount = 0; uiCount < t->n_lights; ++uiCount)
	{
		t->lights[uiCount].uiFlags &= ~LIGHT_NODE_DRAWN;
	}

/*
	if (!(l->uiFlags & MERC_LIGHT))
	{
		uiFlags |= LIGHT_FAKE

		pMapElement = &(gpWorldLevelData[]);
		if (pMapElement->pLand != NULL)
		{
			// only do this for visible locations
			// adjust tile's recorded light level
			pMapElement->sSumRealLight1+=sShadeAdd;

			sSum=pMapElement->pLand->ubNaturalShadeLevel - __max( pMapElement->sSumRealLights[0], pMapElement->pLand->sSumLights[1]);

			sSum=__min(SHADE_MIN, sSum);
			sSum=__max(SHADE_MAX, sSum);

			gpWorldLevelData[ ].ubRealShadeLevel = (UINT8) sSum;

		}

	}
*/

	const INT16 iX = l->iX;
	const INT16 iY = l->iY;

	iOldX = iX;
	iOldY = iY;

	for (UINT16 uiCount = 0; uiCount < t->n_rays; ++uiCount)
	{
		const UINT16 usNodeIndex = t->rays[uiCount];
		if(!(usNodeIndex&LIGHT_NEW_RAY))
		{
			fBlocked = FALSE;
      fOnlyWalls = FALSE;

			LIGHT_NODE* const pLight = &t->lights[usNodeIndex & ~LIGHT_BACKLIGHT];

			if (!(l->uiFlags & LIGHT_SPR_ONROOF))
			{
				if(LightTileBlocked( (INT16)iOldX, (INT16)iOldY, (INT16)(iX+pLight->iDX), (INT16)(iY+pLight->iDY)))
				{
					uiCount = LightFindNextRay(t, uiCount);

          fOnlyWalls = TRUE;
					fBlocked = TRUE;
				}
			}

			if(!(pLight->uiFlags&LIGHT_NODE_DRAWN) && (pLight->ubLight) )
			{
				uiFlags=(UINT32)(usNodeIndex&LIGHT_BACKLIGHT);
				if (l->uiFlags & MERC_LIGHT)       uiFlags |= LIGHT_FAKE;
				if (l->uiFlags & LIGHT_SPR_ONROOF) uiFlags |= LIGHT_ROOF_ONLY;

				LightAddTile(iOldX, iOldY, iX + pLight->iDX, iY + pLight->iDY, pLight->ubLight, uiFlags, fOnlyWalls);

				pLight->uiFlags|=LIGHT_NODE_DRAWN;
			}

			if ( fBlocked )
			{
				iOldX = iX;
				iOldY = iY;
			}
			else
			{
				iOldX = iX+pLight->iDX;
				iOldY = iY+pLight->iDY;
			}

		}
		else
		{
			iOldX = iX;
			iOldY = iY;
		}
	}

	return(TRUE);
}


static BOOLEAN LightRevealWall(const INT16 sX, const INT16 sY, const INT16 sSrcX, const INT16 sSrcY)
{
	Assert(gpWorldLevelData != NULL);

	const UINT32 uiTile = MAPROWCOLTOPOS(sY, sX);

	// IF A FENCE, RETURN FALSE
	if (IsFencePresentAtGridno(uiTile)) return FALSE;

	LEVELNODE* const head = gpWorldLevelData[uiTile].pStructHead;

	BOOLEAN fDoRightWalls = (sX >= sSrcX);
	BOOLEAN fDoLeftWalls  = (sY >= sSrcY);

	for (const LEVELNODE* i = head; i != NULL; i = i->pNext)
	{
		if (i->uiFlags & LEVELNODE_CACHEDANITILE) continue;

		const TILE_ELEMENT* const TileElem = &gTileDatabase[i->usIndex];
		switch(TileElem->usWallOrientation)
		{
			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				if (!fDoRightWalls) fDoLeftWalls = FALSE;
				break;

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				if (!fDoLeftWalls) fDoRightWalls = FALSE;
				break;
		}
	}

	BOOLEAN fHitWall  = FALSE;
	BOOLEAN fRerender = FALSE;
	for (LEVELNODE* i = head; i != NULL; i = i->pNext)
	{
		if (i->uiFlags & LEVELNODE_CACHEDANITILE) continue;

		const TILE_ELEMENT* const TileElem = &gTileDatabase[i->usIndex];
		switch (TileElem->usWallOrientation)
		{
			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				fHitWall = TRUE;
				if (fDoRightWalls && sX >= sSrcX)
				{
					i->uiFlags |= LEVELNODE_REVEAL;
					fRerender   = TRUE;
				}
				break;

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				fHitWall = TRUE;
				if (fDoLeftWalls && sY >= sSrcY)
				{
					i->uiFlags |= LEVELNODE_REVEAL;
					fRerender   = TRUE;
				}
				break;
		}
	}

	if (fRerender) SetRenderFlags(RENDER_FLAG_FULL);
	return fHitWall;
}


static BOOLEAN LightHideWall(const INT16 sX, const INT16 sY, const INT16 sSrcX, const INT16 sSrcY)
{
	Assert(gpWorldLevelData != NULL);

	UINT32     const uiTile = MAPROWCOLTOPOS(sY, sX);
	LEVELNODE* const head   = gpWorldLevelData[uiTile].pStructHead;

	BOOLEAN fDoRightWalls = (sX >= sSrcX);
	BOOLEAN fDoLeftWalls  = (sY >= sSrcY);

	for (const LEVELNODE* i = head; i != NULL; i = i->pNext)
	{
		if (i->uiFlags & LEVELNODE_CACHEDANITILE) continue;

		const TILE_ELEMENT* const te = &gTileDatabase[i->usIndex];
		switch (te->usWallOrientation)
		{
			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				if (!fDoRightWalls) fDoLeftWalls = FALSE;
				break;

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				if (!fDoLeftWalls) fDoRightWalls = FALSE;
				break;
		}
	}

	BOOLEAN fHitWall  = FALSE;
	BOOLEAN fRerender = FALSE;
	for (LEVELNODE* i = head; i != NULL; i = i->pNext)
	{
		if (i->uiFlags & LEVELNODE_CACHEDANITILE) continue;

		const TILE_ELEMENT* const te = &gTileDatabase[i->usIndex];
		switch (te->usWallOrientation)
		{
			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				fHitWall = TRUE;
				if (fDoRightWalls && sX >= sSrcX)
				{
					i->uiFlags &= ~LEVELNODE_REVEAL;
					fRerender   = TRUE;
				}
				break;

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				fHitWall = TRUE;
				if (fDoLeftWalls && sY >= sSrcY)
				{
					i->uiFlags &= ~LEVELNODE_REVEAL;
					fRerender   = TRUE;
				}
				break;
		}
	}

	if (fRerender) SetRenderFlags(RENDER_FLAG_FULL);
	return fHitWall;
}


// Tags walls as being translucent using a light template.
static BOOLEAN CalcTranslucentWalls(INT16 iX, INT16 iY)
{
	LightTemplate* const t = &g_light_templates[0];
	if (t->lights == NULL) return FALSE;

	for (UINT16 uiCount = 0; uiCount < t->n_rays; ++uiCount)
	{
		const UINT16 usNodeIndex = t->rays[uiCount];
		if (!(usNodeIndex & LIGHT_NEW_RAY))
		{
			const LIGHT_NODE* const pLight= &t->lights[usNodeIndex & ~LIGHT_BACKLIGHT];

			//Kris:  added map boundary checking!!!
			if(LightRevealWall(
				 (INT16)min(max((iX+pLight->iDX),0),WORLD_COLS-1),
				 (INT16)min(max((iY+pLight->iDY),0),WORLD_ROWS-1),
				 (INT16)min(max(iX,0),WORLD_COLS-1),
				 (INT16)min(max(iY,0),WORLD_ROWS-1)
				))
			{
				uiCount = LightFindNextRay(t, uiCount);
				SetRenderFlags(RENDER_FLAG_FULL);
			}
		}
	}

	return(TRUE);
}


static BOOLEAN LightGreenTile(INT16 sX, INT16 sY, INT16 sSrcX, INT16 sSrcY)
{
LEVELNODE *pStruct, *pLand;
UINT32 uiTile;
BOOLEAN fRerender=FALSE, fHitWall=FALSE;
TILE_ELEMENT *TileElem;

	Assert(gpWorldLevelData!=NULL);

	uiTile=MAPROWCOLTOPOS(sY, sX);
	pStruct=gpWorldLevelData[uiTile].pStructHead;

	while(pStruct!=NULL)
	{
		TileElem = &(gTileDatabase[pStruct->usIndex]);
		switch(TileElem->usWallOrientation)
		{
			case NO_ORIENTATION:
				break;

			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				fHitWall=TRUE;
				if(sX >= sSrcX)
				{
					pStruct->uiFlags|=LEVELNODE_REVEAL;
					fRerender=TRUE;
				}
				break;

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				fHitWall=TRUE;
				if(sY >= sSrcY)
				{
					pStruct->uiFlags|=LEVELNODE_REVEAL;
					fRerender=TRUE;
				}
				break;
		}
		pStruct=pStruct->pNext;
	}

	//if(fRerender)
	//{
		pLand=gpWorldLevelData[uiTile].pLandHead;
		while(pLand!=NULL)
		{
			pLand->ubShadeLevel=0;
			pLand=pLand->pNext;
		}

		gpWorldLevelData[uiTile].uiFlags|=MAPELEMENT_REDRAW;
		SetRenderFlags(RENDER_FLAG_MARKED);
	//}

	return(fHitWall);
}

/****************************************************************************************
	LightShowRays

		Draws a template by making the ground tiles green. Must be polled once for
each tile drawn to facilitate animating the drawing process for debugging.

***************************************************************************************/
BOOLEAN LightShowRays(INT16 iX, INT16 iY, BOOLEAN fReset)
{
	static UINT16 uiCount = 0;

	if (fReset) uiCount = 0;

	LightTemplate* const t = &g_light_templates[0];
	if (t->lights == NULL) return FALSE;

	if (uiCount < t->n_rays)
	{
		const UINT16 usNodeIndex = t->rays[uiCount];
		if (!(usNodeIndex & LIGHT_NEW_RAY))
		{
			const LIGHT_NODE* const pLight = &t->lights[usNodeIndex & ~LIGHT_BACKLIGHT];
			if(LightGreenTile((INT16)(iX+pLight->iDX), (INT16)(iY+pLight->iDY), iX, iY))
			{
				uiCount = LightFindNextRay(t, uiCount);
				SetRenderFlags(RENDER_FLAG_MARKED);
			}
		}

		uiCount++;
		return(TRUE);
	}
	else
		return(FALSE);
}


// Removes the green from the tiles that was drawn to show the path of the rays.
static BOOLEAN LightHideGreen(INT16 sX, INT16 sY, INT16 sSrcX, INT16 sSrcY)
{
LEVELNODE *pStruct, *pLand;
UINT32 uiTile;
BOOLEAN fRerender=FALSE, fHitWall=FALSE;
TILE_ELEMENT *TileElem;

	Assert(gpWorldLevelData!=NULL);

	uiTile=MAPROWCOLTOPOS(sY, sX);
	pStruct=gpWorldLevelData[uiTile].pStructHead;

	while(pStruct!=NULL)
	{
		TileElem = &(gTileDatabase[pStruct->usIndex]);
		switch(TileElem->usWallOrientation)
		{
			case NO_ORIENTATION:
				break;

			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				fHitWall=TRUE;
				if(sX >= sSrcX)
				{
					pStruct->uiFlags&=(~LEVELNODE_REVEAL);
					fRerender=TRUE;
				}
				break;

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				fHitWall=TRUE;
				if(sY >= sSrcY)
				{
					pStruct->uiFlags&=(~LEVELNODE_REVEAL);
					fRerender=TRUE;
				}
				break;
		}
		pStruct=pStruct->pNext;
	}

	//if(fRerender)
	//{
		pLand=gpWorldLevelData[uiTile].pLandHead;
		while(pLand!=NULL)
		{
			pLand->ubShadeLevel=pLand->ubNaturalShadeLevel;
			pLand=pLand->pNext;
		}

		gpWorldLevelData[uiTile].uiFlags|=MAPELEMENT_REDRAW;
		SetRenderFlags(RENDER_FLAG_MARKED);
	//}

	return(fHitWall);
}

/****************************************************************************************
	LightHideRays

		Hides walls that were revealed by CalcTranslucentWalls.

***************************************************************************************/
BOOLEAN LightHideRays(INT16 iX, INT16 iY)
{
	const LightTemplate* const t = &g_light_templates[0];
	if (t->lights == NULL) return FALSE;

	for (UINT16 uiCount = 0; uiCount < t->n_rays; ++uiCount)
	{
		const UINT16 usNodeIndex = t->rays[uiCount];
		if (!(usNodeIndex & LIGHT_NEW_RAY))
		{
			const LIGHT_NODE* const pLight = &t->lights[usNodeIndex & ~LIGHT_BACKLIGHT];
			if(LightHideWall((INT16)(iX+pLight->iDX), (INT16)(iY+pLight->iDY), iX, iY))
			{
				uiCount = LightFindNextRay(t, uiCount);
				SetRenderFlags(RENDER_FLAG_MARKED);
			}
		}
	}

	return(TRUE);
}

/****************************************************************************************
	ApplyTranslucencyToWalls

		Hides walls that were revealed by CalcTranslucentWalls.

***************************************************************************************/
BOOLEAN ApplyTranslucencyToWalls(INT16 iX, INT16 iY)
{
	LightTemplate* const t = &g_light_templates[0];
	if (t->lights == NULL) return FALSE;

	for (UINT16 uiCount = 0; uiCount < t->n_rays; ++uiCount)
	{
		const UINT16 usNodeIndex = t->rays[uiCount];
		if (!(usNodeIndex & LIGHT_NEW_RAY))
		{
			const LIGHT_NODE* const pLight = &t->lights[usNodeIndex & ~LIGHT_BACKLIGHT];
			//Kris:  added map boundary checking!!!
			if(LightHideWall(
				 (INT16)min(max((iX+pLight->iDX),0),WORLD_COLS-1),
				 (INT16)min(max((iY+pLight->iDY),0),WORLD_ROWS-1),
				 (INT16)min(max(iX,0),WORLD_COLS-1),
				 (INT16)min(max(iY,0),WORLD_ROWS-1)
				))
			{
				uiCount = LightFindNextRay(t, uiCount);
				SetRenderFlags(RENDER_FLAG_FULL);
			}
		}
	}

	return(TRUE);
}


// Reverts all tiles a given light affects to their natural light levels.
static BOOLEAN LightErase(const LIGHT_SPRITE* const l)
{
UINT32 uiFlags;
INT32		iOldX, iOldY;
BOOLEAN	fBlocked = FALSE;
BOOLEAN fOnlyWalls;

	LightTemplate* const t = l->light_template;
	if (t->lights == NULL) return FALSE;

	// clear out all the flags
	for (UINT16 uiCount = 0; uiCount < t->n_lights; ++uiCount)
	{
		t->lights[uiCount].uiFlags &= ~LIGHT_NODE_DRAWN;
	}

	const INT16 iX = l->iX;
	const INT16 iY = l->iY;
	iOldX = iX;
	iOldY = iY;

	for (UINT16 uiCount = 0; uiCount < t->n_rays; ++uiCount)
	{
		const UINT16 usNodeIndex = t->rays[uiCount];
		if (!(usNodeIndex & LIGHT_NEW_RAY))
		{
			fBlocked = FALSE;
      fOnlyWalls = FALSE;

			LIGHT_NODE* const pLight = &t->lights[usNodeIndex & ~LIGHT_BACKLIGHT];

			if (!(l->uiFlags&LIGHT_SPR_ONROOF))
			{
				if(LightTileBlocked( (INT16)iOldX, (INT16)iOldY, (INT16)(iX+pLight->iDX), (INT16)(iY+pLight->iDY)))
				{
					uiCount = LightFindNextRay(t, uiCount);

          fOnlyWalls = TRUE;
					fBlocked = TRUE;
				}
			}

			if(!(pLight->uiFlags&LIGHT_NODE_DRAWN) && (pLight->ubLight) )
			{
				uiFlags=(UINT32)(usNodeIndex&LIGHT_BACKLIGHT);
				if (l->uiFlags & MERC_LIGHT)       uiFlags |= LIGHT_FAKE;
				if (l->uiFlags & LIGHT_SPR_ONROOF) uiFlags |= LIGHT_ROOF_ONLY;

				LightSubtractTile(iOldX, iOldY, iX + pLight->iDX, iY + pLight->iDY, pLight->ubLight, uiFlags, fOnlyWalls);
				pLight->uiFlags|=LIGHT_NODE_DRAWN;
			}

			if ( fBlocked )
			{
				iOldX = iX;
				iOldY = iY;
			}
			else
			{
				iOldX = iX+pLight->iDX;
				iOldY = iY+pLight->iDY;
			}
		}
		else
		{
			iOldX = iX;
			iOldY = iY;
		}
	}

	return(TRUE);
}


/****************************************************************************************
	LightSave

		Saves the light list of a given template to a file. Passing in NULL for the
	filename forces the system to save the light with the internal filename (recommended).

***************************************************************************************/
BOOLEAN LightSave(const LightTemplate* const t, const char* const pFilename)
{
	if (t->lights == NULL) return FALSE;

	const char* const pName = (pFilename != NULL ? pFilename : t->name);
	const HWFILE      hFile = FileOpen(pName, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS);
	if (hFile == 0) return FALSE;

	FileWrite(hFile, &t->n_lights, sizeof(t->n_lights));
	FileWrite(hFile, t->lights,    sizeof(*t->lights) * t->n_lights);
	FileWrite(hFile, &t->n_rays,   sizeof(t->n_rays));
	FileWrite(hFile, t->rays,      sizeof(*t->rays)   * t->n_rays);

	FileClose(hFile);
	return TRUE;
}


/* Loads a light template from disk. The light template is returned, or NULL if
 * the file wasn't loaded. */
static LightTemplate* LightLoad(const char* pFilename)
{
	INT32 iLight = LightGetFree();
	if (iLight == -1) return NULL;

	const HWFILE hFile = FileOpen(pFilename, FILE_ACCESS_READ);
	if (hFile == 0) return NULL;

	LightTemplate* const t = &g_light_templates[iLight];

	FileRead(hFile, &t->n_lights, sizeof(t->n_lights));
	t->lights = MALLOCN(LIGHT_NODE, t->n_lights);
	if (t->lights == NULL)
	{
		t->n_lights = 0;
		return NULL;
	}
	FileRead(hFile, t->lights, sizeof(*t->lights) * t->n_lights);

	FileRead(hFile, &t->n_rays, sizeof(t->n_rays));
	t->rays = MALLOCN(UINT16, t->n_rays);
	if (t->rays ==NULL)
	{
		t->n_lights = 0;
		t->n_rays   = 0;
		MemFree(t->lights);
		return NULL;
	}
	FileRead(hFile, t->rays, sizeof(*t->rays) * t->n_rays);

	FileClose(hFile);

	t->name = MALLOCN(char, strlen(pFilename) + 1);
	strcpy(t->name, pFilename);
	return t;
}


/* Figures out whether a light template is already in memory, or needs to be
 * loaded from disk. Returns the template, or NULL if it couldn't be loaded. */
static LightTemplate* LightLoadCachedTemplate(const char* pFilename)
{
INT32 iCount;

	for(iCount=0; iCount < MAX_LIGHT_TEMPLATES; iCount++)
	{
		LightTemplate* const t = &g_light_templates[iCount];
		const char* const name = t->name;
		if (name != NULL && strcasecmp(pFilename, name) == 0) return t;
	}

	return(LightLoad(pFilename));
}


const SGPPaletteEntry* LightGetColor(void)
{
	return &gpOrigLight;
}



extern void SetAllNewTileSurfacesLoaded( BOOLEAN fNew );

void LightSetColor(const SGPPaletteEntry* const pPal)
{
	Assert(pPal != NULL);

	if (pPal->peRed   != g_light_color.peRed   ||
			pPal->peGreen != g_light_color.peGreen ||
			pPal->peBlue  != g_light_color.peBlue)
	{	//Set the entire tileset database so that it reloads everything.  It has to because the
		//colors have changed.
		SetAllNewTileSurfacesLoaded( TRUE );
	}

	// before doing anything, get rid of all the old palettes
	DestroyTileShadeTables( );

	g_light_color = *pPal;
	gpOrigLight   = *pPal;

	BuildTileShadeTables( );

	// Build all palettes for all soldiers in the world
	// ( THIS FUNCTION WILL ERASE THEM IF THEY EXIST )
	RebuildAllSoldierShadeTables( );

	SetRenderFlags(RENDER_FLAG_FULL);
}

//---------------------------------------------------------------------------------------
// Light Manipulation Layer
//---------------------------------------------------------------------------------------


// Returns the next available sprite.
static LIGHT_SPRITE* LightSpriteGetFree(void)
{
	for (LIGHT_SPRITE* l = LightSprites; l != endof(LightSprites); ++l)
	{
		if (!(l->uiFlags & LIGHT_SPR_ACTIVE)) return l;
	}
	return NULL;
}


LIGHT_SPRITE* LightSpriteCreate(const char* const pName)
{
	LIGHT_SPRITE* const l = LightSpriteGetFree();
	if (l == NULL) return NULL;

	memset(l, 0, sizeof(LIGHT_SPRITE));
	l->iX          = WORLD_COLS + 1;
	l->iY          = WORLD_ROWS + 1;

	l->light_template = LightLoadCachedTemplate(pName);
	if (l->light_template == NULL) return NULL;

	l->uiFlags |= LIGHT_SPR_ACTIVE;
	return l;
}


BOOLEAN LightSpriteFake(LIGHT_SPRITE* const l)
{
	if (l->uiFlags & LIGHT_SPR_ACTIVE)
	{
		l->uiFlags |= MERC_LIGHT;
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


static BOOLEAN LightSpriteDirty(const LIGHT_SPRITE* l);


BOOLEAN LightSpriteDestroy(LIGHT_SPRITE* const l)
{
	if (l->uiFlags & LIGHT_SPR_ACTIVE)
	{
		if (l->uiFlags & LIGHT_SPR_ERASE)
		{
			if (l->iX < WORLD_COLS && l->iY < WORLD_ROWS)
			{
				LightErase(l);
				LightSpriteDirty(l);
			}
			l->uiFlags &= ~LIGHT_SPR_ERASE;
		}

		l->uiFlags &= ~LIGHT_SPR_ACTIVE;
		return(TRUE);
	}

	return(FALSE);
}


/********************************************************************************
* LightSpriteRenderAll
*
*		Resets all tiles in the world to the ambient light level, and redraws all
* active lights.
*
********************************************************************************/
BOOLEAN LightSpriteRenderAll(void)
{
INT32 iCount;

	LightResetAllTiles();
	for(iCount=0; iCount < MAX_LIGHT_SPRITES; iCount++)
	{
		LIGHT_SPRITE* const l = &LightSprites[iCount];
		l->uiFlags &= ~LIGHT_SPR_ERASE;

		if (l->uiFlags & LIGHT_SPR_ACTIVE && (l->uiFlags & LIGHT_SPR_ON))
		{
			LightDraw(l);
			l->uiFlags |= LIGHT_SPR_ERASE;
			LightSpriteDirty(l);
		}
	}

	return(TRUE);
}


void LightSpritePosition(LIGHT_SPRITE* const l, const INT16 iX, const INT16 iY)
{
	Assert(l->uiFlags & LIGHT_SPR_ACTIVE);

	if (l->iX == iX && l->iY == iY) return;

	if (l->uiFlags & LIGHT_SPR_ERASE)
	{
		if (l->iX < WORLD_COLS && l->iY < WORLD_ROWS)
		{
			LightErase(l);
			LightSpriteDirty(l);
		}
	}

	l->iX = iX;
	l->iY = iY;

	if (l->uiFlags & LIGHT_SPR_ON)
	{
		if (l->iX < WORLD_COLS && l->iY < WORLD_ROWS)
		{
			LightDraw(l);
			l->uiFlags |= LIGHT_SPR_ERASE;
			LightSpriteDirty(l);
		}
	}
}


BOOLEAN LightSpriteRoofStatus(LIGHT_SPRITE* const l, BOOLEAN fOnRoof)
{
	if ( fOnRoof &&  (l->uiFlags & LIGHT_SPR_ONROOF)) return FALSE;
	if (!fOnRoof && !(l->uiFlags & LIGHT_SPR_ONROOF)) return FALSE;

	if (l->uiFlags & LIGHT_SPR_ACTIVE)
	{
		if (l->uiFlags & LIGHT_SPR_ERASE)
		{
			if (l->iX < WORLD_COLS && l->iY < WORLD_ROWS)
			{
				LightErase(l);
				LightSpriteDirty(l);
			}
		}

		if (fOnRoof)
		{
			l->uiFlags |= LIGHT_SPR_ONROOF;
		}
		else
		{
			l->uiFlags &= ~LIGHT_SPR_ONROOF;
		}


		if (l->uiFlags & LIGHT_SPR_ON)
		{
			if (l->iX < WORLD_COLS && l->iY < WORLD_ROWS)
			{
				LightDraw(l);
				l->uiFlags |= LIGHT_SPR_ERASE;
				LightSpriteDirty(l);
			}
		}
	}
	else
		return(FALSE);

	return(TRUE);
}


void LightSpritePower(LIGHT_SPRITE* const l, const BOOLEAN fOn)
{
	l->uiFlags = l->uiFlags & ~LIGHT_SPR_ON | (fOn ? LIGHT_SPR_ON : 0);
}


// Sets the flag for the renderer to draw all marked tiles.
static BOOLEAN LightSpriteDirty(const LIGHT_SPRITE* const l)
{
#if 0 // XXX was commented out
	INT16 iLeft_s;
	INT16 iTop_s;
	CellXYToScreenXY(l->iX * CELL_X_SIZE, l->iY * CELL_Y_SIZE, &iLeft_s, &iTop_s);

	const LightTemplate* const t = &g_light_templates[l->iTemplate];

	iLeft_s += t->x_off;
	iTop_s  += t->y_off;

	const INT16 iMapLeft   = l->iX + t->map_left;
	const INT16 iMapTop    = l->iY + t->map_top;
	const INT16 iMapRight  = l->iX + t->map_right;
	const INT16 iMapBottom = l->iY + t->map_bottom;

	UpdateSaveBuffer();
	AddBaseDirtyRect(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y);
	AddBaseDirtyRect(iLeft_s, iTop_s, (INT16)(iLeft_s + t->width), (INT16)(iTop_s + t->height));
#endif

	SetRenderFlags(RENDER_FLAG_MARKED);

	return(TRUE);
}


static void AddSaturatePalette(SGPPaletteEntry Dst[256], const SGPPaletteEntry Src[256], const SGPPaletteEntry* Bias)
{
	UINT8 r = Bias->peRed;
	UINT8 g = Bias->peGreen;
	UINT8 b = Bias->peBlue;
	for (UINT i = 0; i < 256; i++)
	{
		Dst[i].peRed   = __min(Src[i].peRed   + r, 255);
		Dst[i].peGreen = __min(Src[i].peGreen + g, 255);
		Dst[i].peBlue  = __min(Src[i].peBlue  + b, 255);
	}
}


static void CreateShadedPalettes(UINT16* Shades[16], const SGPPaletteEntry ShadePal[256])
{
	const UINT16* sl0 = gusShadeLevels[0];
	Shades[0] = Create16BPPPaletteShaded(ShadePal, sl0[0], sl0[1], sl0[2], TRUE);
	for (UINT i = 1; i < 16; i++)
	{
		const UINT16* sl = gusShadeLevels[i];
		Shades[i] = Create16BPPPaletteShaded(ShadePal, sl[0], sl[1], sl[2], FALSE);
	}
}


void CreateBiasedShadedPalettes(UINT16* Shades[16], const SGPPaletteEntry ShadePal[256])
{
	SGPPaletteEntry LightPal[256];
	AddSaturatePalette(LightPal, ShadePal, &g_light_color);
	CreateShadedPalettes(Shades, LightPal);
}


/**********************************************************************************************
 CreateObjectPaletteTables

		Creates the shading tables for 8-bit brushes. One highlight table is created, based on
	the object-type, 3 brightening tables, 1 normal, and 11 darkening tables. The entries are
	created iteratively, rather than in a loop to allow hand-tweaking of the values. If you
	change the HVOBJECT_SHADE_TABLES symbol, remember to add/delete entries here, it won't
	adjust automagically.

**********************************************************************************************/
void CreateTilePaletteTables(const HVOBJECT pObj)
{
	Assert(pObj != NULL);

	// build the shade tables
	CreateBiasedShadedPalettes(pObj->pShades, pObj->pPaletteEntry);

	// build neutral palette as well!
	// Set current shade table to neutral color
	pObj->pShadeCurrent = pObj->pShades[4];
}


const char* LightSpriteGetTypeName(const LIGHT_SPRITE* const l)
{
	return l->light_template->name;
}
