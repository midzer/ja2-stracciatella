#include "WorldDef.h"
#include "VSurface.h"
#include "Render_Dirty.h"
#include "SysUtil.h"
#include "WCheck.h"
#include "Faces.h"
#include "Overhead.h"
#include "Soldier_Profile.h"
#include "Bullets.h"
#include "LOS.h"
#include "WorldMan.h"
#include "Random.h"
#include "GameSettings.h"
#include "FileMan.h"
#include "Debug.h"


// Defines
#define		NUM_BULLET_SLOTS					50


// GLOBAL FOR FACES LISTING
BULLET	gBullets[ NUM_BULLET_SLOTS ];
UINT32  guiNumBullets = 0;


static INT32 GetFreeBullet(void)
{
	UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumBullets; uiCount++)
	{
		if((gBullets[uiCount].fAllocated==FALSE) )
			return((INT32)uiCount);
	}

	if(guiNumBullets < NUM_BULLET_SLOTS )
		return((INT32)guiNumBullets++);

	return(-1);
}


static void RecountBullets(void)
{
	INT32 uiCount;

	for(uiCount=guiNumBullets-1; (uiCount >=0) ; uiCount--)
	{
		if( ( gBullets[uiCount].fAllocated ) )
		{
			guiNumBullets=(UINT32)(uiCount+1);
			return;
		}
	}
	guiNumBullets = 0;
}


BULLET* CreateBullet(SOLDIERTYPE* const firer, const BOOLEAN fFake, const UINT16 usFlags)
{
	const INT32 iBulletIndex = GetFreeBullet();
	if (iBulletIndex == -1) return NULL;

	BULLET* const b = &gBullets[iBulletIndex];
	memset(b, 0, sizeof(*b));

	b->iBullet            = iBulletIndex;
	b->fAllocated         = TRUE;
	b->fLocated		        = FALSE;
	b->ubFirerID	        = firer->ubID;
	b->pFirer             = firer;
	b->usFlags		        = usFlags;
	b->usLastStructureHit = 0;
	b->fReal              = !fFake;

	return b;
}


void HandleBulletSpecialFlags(BULLET* pBullet)
{
	ANITILE_PARAMS	AniParams;
	FLOAT		dX, dY;
	UINT8		ubDirection;

	memset( &AniParams, 0, sizeof( ANITILE_PARAMS ) );

	if ( pBullet->fReal )
	{
		// Create ani tile if this is a spit!
		if ( pBullet->usFlags & ( BULLET_FLAG_KNIFE ) )
		{
			AniParams.sGridNo							= (INT16)pBullet->sGridNo;
			AniParams.ubLevelID						= ANI_STRUCT_LEVEL;
			AniParams.sDelay							= 100;
			AniParams.sStartFrame					= 3;
			AniParams.uiFlags							= ANITILE_CACHEDTILE | ANITILE_FORWARD | ANITILE_LOOPING | ANITILE_USE_DIRECTION_FOR_START_FRAME;
			AniParams.sX									= FIXEDPT_TO_INT32( pBullet->qCurrX );
			AniParams.sY									= FIXEDPT_TO_INT32( pBullet->qCurrY );
			AniParams.sZ									= CONVERT_HEIGHTUNITS_TO_PIXELS( FIXEDPT_TO_INT32( pBullet->qCurrZ ) );

			if ( pBullet->usFlags & ( BULLET_FLAG_CREATURE_SPIT ) )
			{
				AniParams.zCachedFile = "TILECACHE/spit2.sti";
			}
			else if ( pBullet->usFlags & ( BULLET_FLAG_KNIFE ) )
			{
				AniParams.zCachedFile = "TILECACHE/knifing.sti";
				pBullet->ubItemStatus = pBullet->pFirer->inv[ HANDPOS ].bStatus[0];
			}

			// Get direction to use for this guy....
			dX = ( (FLOAT)( pBullet->qIncrX ) / FIXEDPT_FRACTIONAL_RESOLUTION );
			dY = ( (FLOAT)( pBullet->qIncrY ) / FIXEDPT_FRACTIONAL_RESOLUTION );

			ubDirection = atan8( 0, 0, (INT16)( dX * 100 ), (INT16)( dY * 100 ) );

			AniParams.uiUserData3					= ubDirection;

			pBullet->pAniTile = CreateAnimationTile( &AniParams );

			// IF we are anything that needs a shadow.. set it here....
			if ( pBullet->usFlags & ( BULLET_FLAG_KNIFE ) )
			{
				AniParams.ubLevelID						= ANI_SHADOW_LEVEL;
				AniParams.sZ									= 0;
				pBullet->pShadowAniTile				= CreateAnimationTile( &AniParams );
			}

		}
	}
}


void RemoveBullet(BULLET* b)
{
	CHECKV(b != NULL);

	// decrease soldier's bullet count

	if (b->fReal)
	{
		// set to be deleted at next update
		b->fToDelete = TRUE;

		// decrement reference to bullet in the firer
		b->pFirer->bBulletsLeft--;
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("!!!!!!! Ending bullet, bullets left %d", b->pFirer->bBulletsLeft));

		if (b->usFlags & BULLET_FLAG_KNIFE)
		{
			// Delete ani tile
			if (b->pAniTile != NULL)
			{
				DeleteAniTile(b->pAniTile);
				b->pAniTile = NULL;
			}

			// Delete shadow
			if (b->usFlags & BULLET_FLAG_KNIFE)
			{
				if (b->pShadowAniTile != NULL)
				{
					DeleteAniTile(b->pShadowAniTile);
					b->pShadowAniTile = NULL;
				}
			}
		}
	}
	else
	{
		// delete this fake bullet right away!
		b->fAllocated = FALSE;
		RecountBullets();
	}
}


void LocateBullet(BULLET* b)
{
	if (!gGameSettings.fOptions[TOPTION_SHOW_MISSES]) return;
	if (b->pFirer == NULL) return;
	// Check if a bad guy fired!
	if (b->pFirer->bSide != gbPlayerNum) return;
	if (b->fLocated) return;

	b->fLocated = TRUE;

	//Only if we are in turnbased and noncombat
	if (!(gTacticalStatus.uiFlags & TURNBASED)) return;
	if (!(gTacticalStatus.uiFlags & INCOMBAT)) return;

	LocateGridNo(b->sGridNo);
}


void UpdateBullets(void)
{
	UINT32					uiCount;
	LEVELNODE				*pNode;
	BOOLEAN					fDeletedSome = FALSE;

	for ( uiCount = 0; uiCount < guiNumBullets; uiCount++ )
	{
		if ( gBullets[ uiCount ].fAllocated)
		{
			if (gBullets[ uiCount ].fReal && !( gBullets[ uiCount ].usFlags & BULLET_STOPPED ) )
			{
				// there are duplicate checks for deletion in case the bullet is deleted by shooting
				// someone at point blank range, in the first MoveBullet call in the FireGun code
				if ( gBullets[ uiCount ].fToDelete )
				{
					// Remove from old position
					gBullets[ uiCount ].fAllocated = FALSE;
					fDeletedSome = TRUE;
					continue;
				}

				//if ( !( gGameSettings.fOptions[ TOPTION_HIDE_BULLETS ] ) )
				{
					// ALRIGHTY, CHECK WHAT TYPE OF BULLET WE ARE

					if ( gBullets[ uiCount ].usFlags & ( BULLET_FLAG_CREATURE_SPIT | BULLET_FLAG_KNIFE | BULLET_FLAG_MISSILE | BULLET_FLAG_SMALL_MISSILE | BULLET_FLAG_TANK_CANNON | BULLET_FLAG_FLAME ) )
					{
					}
					else
					{
						RemoveStruct( gBullets[ uiCount ].sGridNo, BULLETTILE1 );
						RemoveStruct( gBullets[ uiCount ].sGridNo, BULLETTILE2 );
					}
				}

				MoveBullet( uiCount );
				if ( gBullets[ uiCount ].fToDelete )
				{
					// Remove from old position
					gBullets[ uiCount ].fAllocated = FALSE;
					fDeletedSome = TRUE;
					continue;
				}

				if ( gBullets[ uiCount ].usFlags & BULLET_STOPPED )
				{
					continue;
				}

				// Display bullet
				//if ( !( gGameSettings.fOptions[ TOPTION_HIDE_BULLETS ] ) )
				{
					if ( gBullets[ uiCount ].usFlags & ( BULLET_FLAG_KNIFE ) )
					{
						if ( gBullets[ uiCount ].pAniTile != NULL )
						{
							gBullets[ uiCount ].pAniTile->sRelativeX	= (INT16) FIXEDPT_TO_INT32( gBullets[ uiCount ].qCurrX );
							gBullets[ uiCount ].pAniTile->sRelativeY	= (INT16) FIXEDPT_TO_INT32( gBullets[ uiCount ].qCurrY );
							gBullets[ uiCount ].pAniTile->pLevelNode->sRelativeZ  = (INT16) CONVERT_HEIGHTUNITS_TO_PIXELS( FIXEDPT_TO_INT32( gBullets[ uiCount ].qCurrZ ) );

							if ( gBullets[ uiCount ].usFlags & ( BULLET_FLAG_KNIFE ) )
							{
								gBullets[ uiCount ].pShadowAniTile->sRelativeX	= (INT16) FIXEDPT_TO_INT32( gBullets[ uiCount ].qCurrX );
								gBullets[ uiCount ].pShadowAniTile->sRelativeY	= (INT16) FIXEDPT_TO_INT32( gBullets[ uiCount ].qCurrY );
							}

						}
					}
					// Are we a missle?
					else if ( gBullets[ uiCount ].usFlags & ( BULLET_FLAG_MISSILE | BULLET_FLAG_SMALL_MISSILE | BULLET_FLAG_TANK_CANNON | BULLET_FLAG_FLAME | BULLET_FLAG_CREATURE_SPIT ) )
					{
					}
					else
					{
						pNode = AddStructToTail( gBullets[ uiCount ].sGridNo, BULLETTILE1 );
						pNode->ubShadeLevel=DEFAULT_SHADE_LEVEL;
						pNode->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
						pNode->uiFlags |= ( LEVELNODE_USEABSOLUTEPOS | LEVELNODE_IGNOREHEIGHT );
						pNode->sRelativeX	= (INT16) FIXEDPT_TO_INT32( gBullets[ uiCount ].qCurrX );
						pNode->sRelativeY	= (INT16) FIXEDPT_TO_INT32( gBullets[ uiCount ].qCurrY );
						pNode->sRelativeZ = (INT16) CONVERT_HEIGHTUNITS_TO_PIXELS( FIXEDPT_TO_INT32( gBullets[ uiCount ].qCurrZ ) );

						// Display shadow
						pNode = AddStructToTail( gBullets[ uiCount ].sGridNo, BULLETTILE2 );
						pNode->ubShadeLevel=DEFAULT_SHADE_LEVEL;
						pNode->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
						pNode->uiFlags |= ( LEVELNODE_USEABSOLUTEPOS | LEVELNODE_IGNOREHEIGHT );
						pNode->sRelativeX	= (INT16) FIXEDPT_TO_INT32( gBullets[ uiCount ].qCurrX );
						pNode->sRelativeY	= (INT16) FIXEDPT_TO_INT32( gBullets[ uiCount ].qCurrY );
						pNode->sRelativeZ = (INT16)gpWorldLevelData[ gBullets[ uiCount ].sGridNo ].sHeight;
					}
				}
			}
			else
			{
				if ( gBullets[ uiCount ].fToDelete )
				{
					gBullets[ uiCount ].fAllocated = FALSE;
					fDeletedSome = TRUE;
				}
			}
		}
	}

	if ( fDeletedSome )
	{
		RecountBullets( );
	}
}


BULLET* GetBulletPtr(INT32 iBullet)
{
	CHECKN(0 <= iBullet && iBullet < NUM_BULLET_SLOTS);
	return &gBullets[iBullet];
}


void AddMissileTrail( BULLET *pBullet, FIXEDPT qCurrX, FIXEDPT qCurrY, FIXEDPT qCurrZ )
{
	ANITILE_PARAMS	AniParams;

	// If we are a small missle, don't show
	if ( pBullet->usFlags & ( BULLET_FLAG_SMALL_MISSILE | BULLET_FLAG_FLAME | BULLET_FLAG_CREATURE_SPIT ) )
	{
		if ( pBullet->iLoop < 5 )
		{
			return;
		}
	}

	// If we are a small missle, don't show
	if ( pBullet->usFlags & ( BULLET_FLAG_TANK_CANNON ) )
	{
		//if ( pBullet->iLoop < 40 )
		//{
			return;
		//}
	}


	memset( &AniParams, 0, sizeof( ANITILE_PARAMS ) );
	AniParams.sGridNo							= (INT16)pBullet->sGridNo;
	AniParams.ubLevelID						= ANI_STRUCT_LEVEL;
	AniParams.sDelay							= (INT16)( 100 + Random( 100 ) );
	AniParams.sStartFrame					= 0;
	AniParams.uiFlags							= ANITILE_CACHEDTILE | ANITILE_FORWARD | ANITILE_ALWAYS_TRANSLUCENT;
	AniParams.sX									= FIXEDPT_TO_INT32( qCurrX );
	AniParams.sY									= FIXEDPT_TO_INT32( qCurrY );
	AniParams.sZ									= CONVERT_HEIGHTUNITS_TO_PIXELS( FIXEDPT_TO_INT32( qCurrZ ) );


	if ( pBullet->usFlags & ( BULLET_FLAG_MISSILE | BULLET_FLAG_TANK_CANNON ) )
	{
		AniParams.zCachedFile = "TILECACHE/msle_smk.sti";
	}
	else if ( pBullet->usFlags & ( BULLET_FLAG_SMALL_MISSILE ) )
	{
		AniParams.zCachedFile = "TILECACHE/msle_sma.sti";
	}
	else if ( pBullet->usFlags & ( BULLET_FLAG_CREATURE_SPIT ) )
	{
		AniParams.zCachedFile = "TILECACHE/msle_spt.sti";
	}
	else if ( pBullet->usFlags & ( BULLET_FLAG_FLAME ) )
	{
		AniParams.zCachedFile = "TILECACHE/flmthr2.sti";
		AniParams.sDelay							= (INT16)( 100 );
	}

	CreateAnimationTile( &AniParams );
}


BOOLEAN SaveBulletStructureToSaveGameFile( HWFILE hFile )
{
	UINT16	usCnt;
	UINT32	uiBulletCount=0;

	//loop through and count the number of bullets
	for( usCnt=0; usCnt<NUM_BULLET_SLOTS; usCnt++ )
	{
		//if the bullet is active, save it
		if( gBullets[ usCnt ].fAllocated )
		{
			uiBulletCount++;
		}
	}

	//Save the number of Bullets in the array
	if (!FileWrite(hFile, &uiBulletCount, sizeof(UINT32))) return FALSE;

	if( uiBulletCount != 0 )
	{
		for( usCnt=0; usCnt<NUM_BULLET_SLOTS; usCnt++ )
		{
			//if the bullet is active, save it
			if( gBullets[ usCnt ].fAllocated )
			{
				//Save the the Bullet structure
				if (!FileWrite(hFile, &gBullets[usCnt], sizeof(BULLET))) return FALSE;
			}
		}
	}

	return( TRUE );
}


BOOLEAN LoadBulletStructureFromSavedGameFile( HWFILE hFile )
{
	//make sure the bullets are not allocated
	memset(gBullets, 0, sizeof(gBullets));

	//Load the number of Bullets in the array
	if (!FileRead(hFile, &guiNumBullets, sizeof(UINT32))) return FALSE;

	for (UINT i = 0; i < guiNumBullets; ++i)
	{
		BULLET* const b = &gBullets[i];
		//Load the the Bullet structure
		if (!FileRead(hFile, b, sizeof(*b))) return FALSE;

		//Set some parameters
		b->uiLastUpdate   = 0;
		b->pFirer         = ID2Soldier(b->ubFirerID);
		b->pAniTile       = NULL;
		b->pShadowAniTile = NULL;
		b->iBullet        = i;

		HandleBulletSpecialFlags(b);
	}

	return TRUE;
}


void StopBullet(BULLET* b)
{
	b->usFlags |= BULLET_STOPPED;

	RemoveStruct(b->sGridNo, BULLETTILE1);
	RemoveStruct(b->sGridNo, BULLETTILE2);
}


void DeleteAllBullets(void)
{
	for (UINT32 i = 0; i < guiNumBullets; ++i)
	{
		BULLET* const b = &gBullets[i];
		if (b->fAllocated)
		{
			// Remove from old position
			RemoveBullet(b);
			b->fAllocated = FALSE;
		}
	}

	RecountBullets( );
}
