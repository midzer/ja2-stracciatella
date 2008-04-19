#include "HImage.h"
#include "Debug.h"
#include "Structure.h"
#include "TileDef.h"
#include "Animation_Cache.h"
#include "Animation_Data.h"
#include "Sys_Globals.h"
#include "Debug_Control.h"
#include "Tile_Surface.h"
#include "Tile_Cache.h"
#include "FileMan.h"
#include "MemMan.h"


typedef struct TILE_CACHE_STRUCT
{
	char Filename[150];
	char zRootName[30];
	STRUCTURE_FILE_REF* pStructureFileRef;
} TILE_CACHE_STRUCT;


static UINT32 guiNumTileCacheStructs    = 0;
static const UINT32 guiMaxTileCacheSize = 50;
static UINT32 guiCurTileCacheSize       = 0;
static INT32  giDefaultStructIndex      = -1;


TILE_CACHE_ELEMENT		*gpTileCache = NULL;
static TILE_CACHE_STRUCT* gpTileCacheStructInfo = NULL;



BOOLEAN InitTileCache(  )
{
	char TilecacheFilePattern[512];

	snprintf(TilecacheFilePattern, lengthof(TilecacheFilePattern), "%s/Data/TILECACHE/*.jsd", GetBinDataPath());

	UINT32				cnt;
	GETFILESTRUCT FileInfo;
	INT16					sFiles = 0;

	gpTileCache = MALLOCN(TILE_CACHE_ELEMENT, guiMaxTileCacheSize);

	// Zero entries
	for ( cnt = 0; cnt < guiMaxTileCacheSize; cnt++ )
	{
		gpTileCache[ cnt ].pImagery = NULL;
		gpTileCache[ cnt ].sStructRefID = -1;
	}

	guiCurTileCacheSize = 0;


	// OK, look for JSD files in the tile cache directory and
	// load any we find....
	if (GetFileFirst(TilecacheFilePattern, &FileInfo))
	{
		while( GetFileNext(&FileInfo) )
		{
			sFiles++;
		}
		GetFileClose(&FileInfo);
	}

	// Allocate memory...
	if ( sFiles > 0 )
	{
		cnt = 0;

		guiNumTileCacheStructs = sFiles;

		gpTileCacheStructInfo = MALLOCN(TILE_CACHE_STRUCT, sFiles);

		// Loop through and set filenames
		if (GetFileFirst(TilecacheFilePattern, &FileInfo))
		{
			while( GetFileNext(&FileInfo) )
			{
				sprintf(gpTileCacheStructInfo[cnt].Filename, "%s/Data/TILECACHE/%s", GetBinDataPath(), FileInfo.zFileName);

				// Get root name
				GetRootName( gpTileCacheStructInfo[ cnt ].zRootName, gpTileCacheStructInfo[ cnt ].Filename );

				// Load struc data....
				gpTileCacheStructInfo[ cnt ].pStructureFileRef = LoadStructureFile( gpTileCacheStructInfo[ cnt ].Filename );

#ifdef JA2TESTVERSION
				if ( gpTileCacheStructInfo[ cnt ].pStructureFileRef == NULL )
				{
					SET_ERROR(  "Cannot load tilecache JSD: %s", gpTileCacheStructInfo[ cnt ].Filename );
				}
#endif
        if (strcasecmp(gpTileCacheStructInfo[cnt].zRootName, "l_dead1") == 0)
        {
           giDefaultStructIndex = cnt;
        }

				cnt++;
			}
			GetFileClose(&FileInfo);
		}
	}

	return( TRUE );
}

void DeleteTileCache( )
{
	UINT32 cnt;

	// Allocate entries
	if ( gpTileCache != NULL )
	{
		// Loop through and delete any entries
		for ( cnt = 0; cnt < guiMaxTileCacheSize; cnt++ )
		{
			if ( gpTileCache[ cnt ].pImagery != NULL )
			{
				DeleteTileSurface( gpTileCache[ cnt ].pImagery );
			}
		}
		MemFree( gpTileCache );
	}

	if ( gpTileCacheStructInfo != NULL )
	{
		MemFree( gpTileCacheStructInfo );
	}

	guiCurTileCacheSize = 0;
}


static INT16 FindCacheStructDataIndex(const char* cFilename)
{
	UINT32 cnt;

	for ( cnt = 0; cnt < guiNumTileCacheStructs; cnt++ )
	{
		if (strcasecmp(gpTileCacheStructInfo[cnt].zRootName, cFilename) == 0)
		{
			return(	 (INT16)cnt );
		}
	}

	return( -1 );
}


INT32 GetCachedTile(const char* const filename)
{
	// Check to see if surface exists already
	for (UINT32 cnt = 0; cnt < guiCurTileCacheSize; ++cnt)
	{
		TILE_CACHE_ELEMENT* const i = &gpTileCache[cnt];
		if (i->pImagery == NULL) continue;

		if (strcasecmp(i->zName, filename) != 0) continue;

		// Found surface, return
		++i->sHits;
		return (INT32)cnt;
	}

	// Check if max size has been reached
	if (guiCurTileCacheSize == guiMaxTileCacheSize)
	{
		// cache out least used file
		UINT32 ubLowestIndex = 0;
		INT16  sMostHits     = 15000;
		for (UINT32 cnt = 0; cnt < guiCurTileCacheSize; ++cnt)
		{
			const TILE_CACHE_ELEMENT* const i = &gpTileCache[cnt];
			if (i->sHits < sMostHits)
			{
				sMostHits = i->sHits;
				ubLowestIndex = cnt;
			}
		}

		// Bump off lowest index
		TILE_CACHE_ELEMENT* const del = &gpTileCache[ubLowestIndex];
		DeleteTileSurface(del->pImagery);
		del->sHits        = 0;
		del->pImagery     = NULL;
		del->sStructRefID = -1;
	}

	// If here, Insert at an empty slot
	// Find an empty slot
	for (UINT32 cnt = 0; cnt < guiMaxTileCacheSize; ++cnt)
	{
		TILE_CACHE_ELEMENT* const i = &gpTileCache[cnt];
		if (i->pImagery != NULL) continue;

		// Insert here
		i->pImagery = LoadTileSurface(filename);
		if (i->pImagery == NULL) return -1;

		strcpy(i->zName, filename);
		i->sHits = 1;

		GetRootName(i->zRootName, filename);

		i->sStructRefID = FindCacheStructDataIndex(i->zRootName);
		if (i->sStructRefID != -1) // ATE: Add z-strip info
		{
			AddZStripInfoToVObject(i->pImagery->vo, gpTileCacheStructInfo[i->sStructRefID].pStructureFileRef, TRUE, 0);
		}

		const AuxObjectData* const aux = i->pImagery->pAuxData;
		i->ubNumFrames = (aux != NULL ? aux->ubNumberOfFrames : 1);

		// Has our cache size increased?
		if (cnt >= guiCurTileCacheSize) guiCurTileCacheSize = cnt + 1;

		return cnt;
	}

	return -1;
}


BOOLEAN RemoveCachedTile( INT32 iCachedTile )
{
	UINT32			cnt;

	// Find tile
	for ( cnt = 0; cnt < guiCurTileCacheSize; cnt++ )
	{
		if ( gpTileCache[ cnt ].pImagery != NULL )
		{
			if ( cnt == (UINT32)iCachedTile )
			{
				 // Found surface, decrement hits
				 gpTileCache[ cnt ].sHits--;

				 // Are we at zero?
				 if ( gpTileCache[ cnt ].sHits == 0 )
				 {
						DeleteTileSurface( gpTileCache[ cnt ].pImagery );
						gpTileCache[ cnt ].pImagery = NULL;
						gpTileCache[ cnt ].sStructRefID = -1;
						return TRUE;
				 }
			}
		}
	}

	return( FALSE );
}


static STRUCTURE_FILE_REF* GetCachedTileStructureRef(INT32 iIndex)
{
	if ( iIndex == -1 )
	{
		return( NULL );
	}

	if ( gpTileCache[ iIndex ].sStructRefID == -1 )
	{
		return( NULL );
	}

	return( gpTileCacheStructInfo[ gpTileCache[ iIndex ].sStructRefID ].pStructureFileRef );
}


STRUCTURE_FILE_REF* GetCachedTileStructureRefFromFilename(const char* cFilename)
{
	INT16 sStructDataIndex;

	// Given filename, look for index
	sStructDataIndex = FindCacheStructDataIndex( cFilename );

	if ( sStructDataIndex == -1 )
	{
		return( NULL );
	}

	return( gpTileCacheStructInfo[ sStructDataIndex ].pStructureFileRef );
}


void CheckForAndAddTileCacheStructInfo( LEVELNODE *pNode, INT16 sGridNo, UINT16 usIndex, UINT16 usSubIndex )
{
	STRUCTURE_FILE_REF *pStructureFileRef;

	pStructureFileRef = GetCachedTileStructureRef( usIndex );

	if ( pStructureFileRef != NULL)
	{
		if ( !AddStructureToWorld( sGridNo, 0, &( pStructureFileRef->pDBStructureRef[ usSubIndex ] ), pNode ) )
    {
      if ( giDefaultStructIndex != -1 )
      {
        pStructureFileRef = gpTileCacheStructInfo[ giDefaultStructIndex ].pStructureFileRef;

	      if ( pStructureFileRef != NULL)
	      {
		      AddStructureToWorld( sGridNo, 0, &( pStructureFileRef->pDBStructureRef[ usSubIndex ] ), pNode );
        }
      }
    }
	}
}

void CheckForAndDeleteTileCacheStructInfo( LEVELNODE *pNode, UINT16 usIndex )
{
	STRUCTURE_FILE_REF *pStructureFileRef;

	if ( usIndex >= TILE_CACHE_START_INDEX )
	{
		pStructureFileRef = GetCachedTileStructureRef( ( usIndex - TILE_CACHE_START_INDEX ) );

		if ( pStructureFileRef != NULL)
		{
			DeleteStructureFromWorld( pNode->pStructureData );
		}
	}
}


void GetRootName(char* pDestStr, const char* pSrcStr)
{
	// Remove path and extension
	char* cEndOfName;

	// Remove path
	char cTempFilename[120];
	strcpy( cTempFilename, pSrcStr );
	cEndOfName = strrchr( cTempFilename, '/' );
	if (cEndOfName != NULL)
	{
		cEndOfName++;
		strcpy( pDestStr, cEndOfName );
	}
	else
	{
		strcpy( pDestStr, cTempFilename );
	}

	// Now remove extension...
	cEndOfName = strchr( pDestStr, '.' );
	if (cEndOfName != NULL)
	{
		*cEndOfName = '\0';
	}

}
