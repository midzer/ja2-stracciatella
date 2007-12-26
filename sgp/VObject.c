#include "Debug.h"
#include "MemMan.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "WCheck.h"


// ******************************************************************************
//
// Video Object SGP Module
//
// Video Objects are used to contain any imagery which requires blitting. The data
// is contained within a Direct Draw surface. Palette information is in both
// a Direct Draw Palette and a 16BPP palette structure for 8->16 BPP Blits.
// Blitting is done via Direct Draw as well as custum blitters. Regions are
// used to define local coordinates within the surface
//
// Second Revision: Dec 10, 1996, Andrew Emmons
//
// *******************************************************************************


#define COMPRESS_TRANSPARENT				0x80
#define COMPRESS_RUN_MASK						0x7F


typedef struct VOBJECT_NODE
{
	HVOBJECT hVObject;
  struct VOBJECT_NODE* next;
#ifdef SGP_VIDEO_DEBUGGING
	char* pName;
	char* pCode;
#endif
} VOBJECT_NODE;

static VOBJECT_NODE* gpVObjectHead = NULL;
static VOBJECT_NODE* gpVObjectTail = NULL;
UINT32 guiVObjectSize = 0;


BOOLEAN InitializeVideoObjectManager(void)
{
	//Shouldn't be calling this if the video object manager already exists.
	//Call shutdown first...
	Assert(gpVObjectHead == NULL);
	Assert(gpVObjectTail == NULL);
	gpVObjectHead = NULL;
	gpVObjectTail = NULL;
	return TRUE;
}


static BOOLEAN DeleteVideoObject(SGPVObject*);


BOOLEAN ShutdownVideoObjectManager(void)
{
	while (gpVObjectHead != NULL)
	{
		VOBJECT_NODE* curr = gpVObjectHead;
		gpVObjectHead = gpVObjectHead->next;
		DeleteVideoObject(curr->hVObject);
#ifdef SGP_VIDEO_DEBUGGING
		if (curr->pName != NULL) MemFree(curr->pName);
		if (curr->pCode != NULL) MemFree(curr->pCode);
#endif
		MemFree(curr);
	}
	gpVObjectHead = NULL;
	gpVObjectTail = NULL;
	guiVObjectSize = 0;
	return TRUE;
}


#ifdef JA2TESTVERSION
static UINT32 CountVideoObjectNodes(void)
{
	UINT32 i = 0;
	for (const VOBJECT_NODE* curr = gpVObjectHead; curr != NULL; curr = curr->next)
	{
		i++;
	}
	return i;
}
#endif


static void AddStandardVideoObject(HVOBJECT hVObject)
{
	if (hVObject == NULL) return;

	VOBJECT_NODE* Node = MemAlloc(sizeof(*Node));
	Assert(Node != NULL); // out of memory?
	Node->hVObject = hVObject;

	Node->next = NULL;
#ifdef SGP_VIDEO_DEBUGGING
	Node->pName = NULL;
	Node->pCode = NULL;
#endif

	if (gpVObjectTail == NULL)
	{
		gpVObjectHead = Node;
	}
	else
	{
		gpVObjectTail->next = Node;
	}
	gpVObjectTail = Node;

	guiVObjectSize++;
#ifdef JA2TESTVERSION
	if (CountVideoObjectNodes() != guiVObjectSize)
	{
		guiVObjectSize = guiVObjectSize;
	}
#endif
}


static SGPVObject* CreateVideoObject(HIMAGE);


SGPVObject* AddStandardVideoObjectFromHImage(HIMAGE hImage)
{
	SGPVObject* const vo = CreateVideoObject(hImage);
	AddStandardVideoObject(vo);
	return vo;
}


static SGPVObject* CreateVideoObjectFromFile(const char* filename);


SGPVObject* AddStandardVideoObjectFromFile(const char* ImageFile)
{
	SGPVObject* const vo = CreateVideoObjectFromFile(ImageFile);
	AddStandardVideoObject(vo);
	return vo;
}


BOOLEAN DeleteVideoObjectFromIndex(SGPVObject* const vo)
{
	VOBJECT_NODE* prev = NULL;
	VOBJECT_NODE* curr = gpVObjectHead;
	while (curr != NULL)
	{
		if (curr->hVObject == vo)
		{ //Found the node, so detach it and delete it.
			DeleteVideoObject(vo);

			if (curr == gpVObjectHead) gpVObjectHead = gpVObjectHead->next;
			if (curr == gpVObjectTail) gpVObjectTail = prev;
			if (prev != NULL) prev->next = curr->next;

#ifdef SGP_VIDEO_DEBUGGING
			if (curr->pName != NULL) MemFree(curr->pName);
			if (curr->pCode != NULL) MemFree(curr->pCode);
#endif
			MemFree(curr);
			guiVObjectSize--;
#ifdef JA2TESTVERSION
			if (CountVideoObjectNodes() != guiVObjectSize)
			{
				guiVObjectSize = guiVObjectSize;
			}
#endif
			return TRUE;
		}
		prev = curr;
		curr = curr->next;
	}
	return FALSE;
}


static BOOLEAN BltVideoObjectToBuffer(UINT16* pBuffer, UINT32 uiDestPitchBYTES, const SGPVObject* hSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY);


/* Given an index to the dest and src vobject contained in ghVideoObjects */
BOOLEAN BltVideoObject(const UINT32 uiDestVSurface, const SGPVObject* const src, const UINT16 usRegionIndex, const INT32 iDestX, const INT32 iDestY)
{
	UINT32 uiPitch;
	UINT16* pBuffer = (UINT16*)LockVideoSurface(uiDestVSurface, &uiPitch);
	if (pBuffer == NULL) return FALSE;

	BOOLEAN Ret = BltVideoObjectToBuffer(pBuffer, uiPitch, src, usRegionIndex, iDestX, iDestY);

	UnLockVideoSurface(uiDestVSurface);
	return Ret;
}


static BOOLEAN SetVideoObjectPalette(HVOBJECT hVObject, const SGPPaletteEntry* pSrcPalette);


static SGPVObject* CreateVideoObject(HIMAGE hImage)
{
	if (hImage == NULL)
	{
		DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Invalid hImage pointer given");
		return NULL;
	}

	if (!(hImage->fFlags & IMAGE_TRLECOMPRESSED))
	{
		DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Invalid Image format given.");
		return NULL;
	}

	HVOBJECT hVObject = MemAlloc(sizeof(*hVObject));
	CHECKF(hVObject != NULL);
	memset(hVObject, 0, sizeof(*hVObject));

	ETRLEData TempETRLEData;
	CHECKF(GetETRLEImageData(hImage, &TempETRLEData));

	hVObject->usNumberOfObjects = TempETRLEData.usNumberOfObjects;
	hVObject->pETRLEObject      = TempETRLEData.pETRLEObject;
	hVObject->pPixData          = TempETRLEData.pPixData;
	hVObject->uiSizePixData     = TempETRLEData.uiSizePixData;
	hVObject->ubBitDepth        = hImage->ubBitDepth;

	if (hImage->ubBitDepth == 8)
	{
		SetVideoObjectPalette(hVObject, hImage->pPalette);
	}

	return hVObject;
}


static SGPVObject* CreateVideoObjectFromFile(const char* const Filename)
{
	HIMAGE hImage = CreateImage(Filename, IMAGE_ALLIMAGEDATA);
	if (hImage == NULL)
	{
		DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, String("Invalid Image Filename '%s' given", Filename));
		return NULL;
	}

	HVOBJECT vObject = CreateVideoObject(hImage);

	DestroyImage(hImage);
	return vObject;
}


// Palette setting is expensive, need to set both DDPalette and create 16BPP palette
static BOOLEAN SetVideoObjectPalette(HVOBJECT hVObject, const SGPPaletteEntry* pSrcPalette)
{
	Assert(hVObject != NULL);
	Assert(pSrcPalette != NULL);

	// Create palette object if not already done so
	if (hVObject->pPaletteEntry == NULL)
	{
		// Create palette
		hVObject->pPaletteEntry = MemAlloc(sizeof(SGPPaletteEntry) * 256);
		CHECKF(hVObject->pPaletteEntry != NULL);
	}
	memcpy(hVObject->pPaletteEntry, pSrcPalette, sizeof(SGPPaletteEntry) * 256);

	if (hVObject->p16BPPPalette != NULL) MemFree(hVObject->p16BPPPalette);
	hVObject->p16BPPPalette = Create16BPPPalette(pSrcPalette);
	hVObject->pShadeCurrent = hVObject->p16BPPPalette;

	return TRUE;
}


// Deletes all palettes, surfaces and region data
static BOOLEAN DeleteVideoObject(SGPVObject* const hVObject)
{
	CHECKF(hVObject != NULL);

	DestroyObjectPaletteTables(hVObject);

	if (hVObject->pPaletteEntry != NULL) MemFree(hVObject->pPaletteEntry);
	if (hVObject->pPixData      != NULL) MemFree(hVObject->pPixData);
	if (hVObject->pETRLEObject  != NULL) MemFree(hVObject->pETRLEObject);

	if (hVObject->ppZStripInfo != NULL)
	{
		for (UINT32 usLoop = 0; usLoop < hVObject->usNumberOfObjects; usLoop++)
		{
			if (hVObject->ppZStripInfo[usLoop] != NULL)
			{
				MemFree(hVObject->ppZStripInfo[usLoop]->pbZChange);
				MemFree(hVObject->ppZStripInfo[usLoop]);
			}
		}
		MemFree(hVObject->ppZStripInfo);
	}

	MemFree(hVObject);

	return TRUE;
}


// High level blit function encapsolates ALL effects and BPP
static BOOLEAN BltVideoObjectToBuffer(UINT16* const pBuffer, const UINT32 uiDestPitchBYTES, const SGPVObject* const hSrcVObject, const UINT16 usIndex, const INT32 iDestX, const INT32 iDestY)
{
	Assert(pBuffer != NULL);
	Assert(hSrcVObject != NULL);

	switch (hSrcVObject->ubBitDepth)
	{
			case 16:
				break;

			case 8:
				if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect))
					Blt8BPPDataTo16BPPBufferTransparentClip(pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect);
				else
					Blt8BPPDataTo16BPPBufferTransparent(pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex);
				break;
	}

	return TRUE;
}


/* Destroys the palette tables of a video object. All memory is deallocated, and
 * the pointers set to NULL. Be careful not to try and blit this object until
 * new tables are calculated, or things WILL go boom. */
BOOLEAN DestroyObjectPaletteTables(HVOBJECT hVObject)
{
	for (UINT32 x = 0; x < HVOBJECT_SHADE_TABLES; x++)
	{
		if (hVObject->fFlags & VOBJECT_FLAG_SHADETABLE_SHARED) continue;

		if (hVObject->pShades[x] != NULL)
		{
			if (hVObject->pShades[x] == hVObject->p16BPPPalette)
			{
				hVObject->p16BPPPalette = NULL;
			}

			MemFree(hVObject->pShades[x]);
			hVObject->pShades[x] = NULL;
		}
	}

	if (hVObject->p16BPPPalette != NULL)
	{
		MemFree(hVObject->p16BPPPalette);
		hVObject->p16BPPPalette = NULL;
	}

	hVObject->pShadeCurrent = NULL;

	return TRUE;
}


UINT16 SetObjectShade(HVOBJECT pObj, UINT32 uiShade)
{
	Assert(pObj != NULL);
	Assert(uiShade >= 0);
	Assert(uiShade < HVOBJECT_SHADE_TABLES);

	if (pObj->pShades[uiShade] == NULL)
	{
		DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Attempt to set shade level to NULL table");
		return FALSE;
	}

	pObj->pShadeCurrent = pObj->pShades[uiShade];
	return TRUE;
}


/* Given a VOBJECT and ETRLE image index, retrieves the value of the pixel
 * located at the given image coordinates. The value returned is an 8-bit
 * palette index */
BOOLEAN GetETRLEPixelValue(UINT8* pDest, HVOBJECT hVObject, UINT16 usETRLEIndex, UINT16 usX, UINT16 usY)
{
	CHECKF(hVObject != NULL);
	CHECKF(usETRLEIndex < hVObject->usNumberOfObjects);

	const ETRLEObject* pETRLEObject = &hVObject->pETRLEObject[usETRLEIndex];

	CHECKF(usX < pETRLEObject->usWidth);
	CHECKF(usY < pETRLEObject->usHeight);

	// Assuming everything's okay, go ahead and look...
	UINT8* pCurrent = &((UINT8*)hVObject->pPixData)[pETRLEObject->uiDataOffset];

	// Skip past all uninteresting scanlines
	for (UINT16 usLoopY = 0; usLoopY < usY; usLoopY++)
	{
		while (*pCurrent != 0)
		{
			if (*pCurrent & COMPRESS_TRANSPARENT)
			{
				pCurrent++;
			}
			else
			{
				pCurrent += *pCurrent & COMPRESS_RUN_MASK;
			}
		}
	}

	// Now look in this scanline for the appropriate byte
	UINT16 usLoopX = 0;
	do
	{
		UINT16 ubRunLength = *pCurrent & COMPRESS_RUN_MASK;

		if (*pCurrent & COMPRESS_TRANSPARENT)
		{
			if (usLoopX + ubRunLength >= usX)
			{
				*pDest = 0;
				return TRUE;
			}
			else
			{
				pCurrent++;
			}
		}
		else
		{
			if (usLoopX + ubRunLength >= usX)
			{
				// skip to the correct byte; skip at least 1 to get past the byte defining the run
				pCurrent += (usX - usLoopX) + 1;
				*pDest = *pCurrent;
				return TRUE;
			}
			else
			{
				pCurrent += ubRunLength + 1;
			}
		}
		usLoopX += ubRunLength;
	}
	while (usLoopX < usX);
	// huh???
	return FALSE;
}


const ETRLEObject* GetVideoObjectETRLESubregionProperties(const SGPVObject* const vo, const UINT16 usIndex)
{
	CHECKN(usIndex < vo->usNumberOfObjects);
	return &vo->pETRLEObject[usIndex];
}


BOOLEAN BltVideoObjectOutline(const UINT32 uiDestVSurface, const SGPVObject* const hSrcVObject, const UINT16 usIndex, const INT32 iDestX, const INT32 iDestY, const INT16 s16BPPColor, const BOOLEAN fDoOutline)
{
	UINT32 uiPitch;
	UINT16* pBuffer = (UINT16*)LockVideoSurface(uiDestVSurface, &uiPitch);
	if (pBuffer == NULL) return FALSE;

	if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect))
	{
		Blt8BPPDataTo16BPPBufferOutlineClip(pBuffer, uiPitch, hSrcVObject, iDestX, iDestY, usIndex, s16BPPColor, fDoOutline, &ClippingRect);
	}
	else
	{
		Blt8BPPDataTo16BPPBufferOutline(pBuffer, uiPitch, hSrcVObject, iDestX, iDestY, usIndex, s16BPPColor, fDoOutline);
	}

	UnLockVideoSurface(uiDestVSurface);
	return TRUE;
}


BOOLEAN BltVideoObjectOutlineShadow(const UINT32 uiDestVSurface, const SGPVObject* const src, const UINT16 usIndex, const INT32 iDestX, const INT32 iDestY)
{
	UINT32 uiPitch;
	UINT16* pBuffer = (UINT16*)LockVideoSurface(uiDestVSurface, &uiPitch);
	if (pBuffer == NULL) return FALSE;

	if (BltIsClipped(src, iDestX, iDestY, usIndex, &ClippingRect))
	{
		Blt8BPPDataTo16BPPBufferOutlineShadowClip(pBuffer, uiPitch, src, iDestX, iDestY, usIndex, &ClippingRect);
	}
	else
	{
		Blt8BPPDataTo16BPPBufferOutlineShadow(pBuffer, uiPitch, src, iDestX, iDestY, usIndex);
	}

	UnLockVideoSurface(uiDestVSurface);
	return TRUE;
}


#ifdef SGP_VIDEO_DEBUGGING

typedef struct DUMPINFO
{
	UINT32 Counter;
	char Name[256];
	char Code[256];
} DUMPINFO;


static void DumpVObjectInfoIntoFile(const char* filename, BOOLEAN fAppend)
{
	if (guiVObjectSize == 0) return;

	FILE* fp = fopen(filename, fAppend ? "a" : "w");
	Assert(fp != NULL);

	//Allocate enough strings and counters for each node.
	DUMPINFO* Info = MemAlloc(sizeof(*Info) * guiVObjectSize);
	memset(Info, 0, sizeof(*Info) * guiVObjectSize);

	//Loop through the list and record every unique filename and count them
	UINT32 uiUniqueID = 0;
	for (const VOBJECT_NODE* curr = gpVObjectHead; curr != NULL; curr = curr->next)
	{
		const char* Name = curr->pName;
		const char* Code = curr->pCode;
		BOOLEAN fFound = FALSE;
		for (UINT32 i = 0; i < uiUniqueID; i++)
		{
			if (strcasecmp(Name, Info[i].Name) == 0 && strcasecmp(Code, Info[i].Code) == 0)
			{ //same string
				fFound = TRUE;
				Info[i].Counter++;
				break;
			}
		}
		if (!fFound)
		{
			strcpy(Info[uiUniqueID].Name, Name);
			strcpy(Info[uiUniqueID].Code, Code);
			Info[uiUniqueID].Counter++;
			uiUniqueID++;
		}
	}

	//Now dump the info.
	fprintf(fp, "-----------------------------------------------\n");
	fprintf(fp, "%d unique vObject names exist in %d VObjects\n", uiUniqueID, guiVObjectSize);
	fprintf(fp, "-----------------------------------------------\n\n");
	for (UINT32 i = 0; i < uiUniqueID; i++)
	{
		fprintf(fp, "%d occurrences of %s\n%s\n\n", Info[i].Counter, Info[i].Name, Info[i].Code);
	}
	fprintf(fp, "\n-----------------------------------------------\n\n");

	//Free all memory associated with this operation.
	MemFree(Info);
	fclose(fp);
}


//Debug wrapper for adding vObjects
static void RecordVObject(const char* Filename, UINT32 uiLineNum, const char* pSourceFile)
{
	//record the filename of the vObject (some are created via memory though)
	gpVObjectTail->pName = MemAlloc(strlen(Filename) + 1);
	strcpy(gpVObjectTail->pName, Filename);

	//record the code location of the calling creating function.
	char str[256];
	sprintf(str, "%s -- line(%d)", pSourceFile, uiLineNum);
	gpVObjectTail->pCode = MemAlloc(strlen(str) + 1);
	strcpy(gpVObjectTail->pCode, str);
}


SGPVObject* AddAndRecordVObjectFromHImage(HIMAGE hImage, UINT32 uiLineNum, const char* pSourceFile)
{
	SGPVObject* const vo = AddStandardVideoObjectFromHImage(hImage);
	if (vo != NO_VOBJECT) RecordVObject("<IMAGE>", uiLineNum, pSourceFile);
	return vo;
}


SGPVObject* AddAndRecordVObjectFromFile(const char* ImageFile, UINT32 uiLineNum, const char* pSourceFile)
{
	SGPVObject* const vo = AddStandardVideoObjectFromFile(ImageFile);
	if (vo != NULL) RecordVObject(ImageFile, uiLineNum, pSourceFile);
	return vo;
}


void PerformVideoInfoDumpIntoFile(const char* filename, BOOLEAN fAppend)
{
	DumpVObjectInfoIntoFile(filename, fAppend);
	DumpVSurfaceInfoIntoFile(filename, TRUE);
}

#endif
