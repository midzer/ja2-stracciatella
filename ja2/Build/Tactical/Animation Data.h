#ifndef ANIMATION_DATA_H_
#define ANIMATION_DATA_H_

#include "vobject.h"

#include "Soldier Control.h"
#include "Overhead.h"
#include "Structure Internals.h"


// DEFINES FOR ANIMATION PROFILES
#define		TILE_FLAG_FEET					0x0001
#define		TILE_FLAG_MID						0x0002
#define		TILE_FLAG_HEAD					0x0004
#define		TILE_FLAG_CANMOVE				0x0008
#define		TILE_FLAG_NORTH_HALF		0x0010
#define		TILE_FLAG_SOUTH_HALF		0x0020
#define		TILE_FLAG_WEST_HALF			0x0040
#define		TILE_FLAG_EAST_HALF			0x0080
#define		TILE_FLAG_TOP_HALF			0x0100
#define		TILE_FLAG_BOTTOM_HALF		0x0200

#define		ANIM_DATA_FLAG_NOFRAMES	0x01

// ENUMERATIONS

// BODY TYPES
// RGM = Regular Male
// (RG) = Body desc ( Regular - RG, Short Stocky ( SS ), etc
// (M) = Sex, Male, female
#define	IS_MERC_BODY_TYPE( p )   ( ( p->ubBodyType <= REGFEMALE ) ? ( TRUE ) : ( FALSE ) )
#define IS_CIV_BODY_TYPE( p ) ( (p->ubBodyType >= FATCIV) && (p->ubBodyType <= CRIPPLECIV) )

#define NUMSOLDIERBODYTYPES	4

enum
{
	REGMALE = 0,
	BIGMALE,
	STOCKYMALE,
	REGFEMALE,
	ADULTFEMALEMONSTER,
	AM_MONSTER,
	YAF_MONSTER,
	YAM_MONSTER,
	LARVAE_MONSTER,
	INFANT_MONSTER,
	QUEENMONSTER,
	FATCIV,
	MANCIV,
	MINICIV,
	DRESSCIV,
	HATKIDCIV,
	KIDCIV,
	CRIPPLECIV,

	COW,
	CROW,
	BLOODCAT,

	ROBOTNOWEAPON,

	HUMVEE,
	TANK_NW,
	TANK_NE,
	ELDORADO,
	ICECREAMTRUCK,
	JEEP,

  TOTALBODYTYPES

} SoldierBodyTypes;


// Enumerations
// This enumeration defines the graphic image file per animation

// RGM = Regular Male
// (RG) = Body desc ( Regular - RG, Short Stocky ( SS ), etc
// (M) = Sex, Male, female
typedef enum
{
	RGMBASICWALKING = 0,
	RGMSTANDING,
	RGMCROUCHING,
	RGMSNEAKING,
	RGMRUNNING,
	RGMPRONE,
	RGMSTANDAIM,
	RGMHITHARD,
	RGMHITSTAND,
	RGMHITHARDBLOOD,
	RGMCROUCHAIM,
	RGMHITFALLBACK,
	RGMROLLOVER,
	RGMCLIMBROOF,
	RGMFALL,
	RGMFALLF,
	RGMHITCROUCH,
	RGMHITPRONE,
	RGMHOPFENCE,
	RGMPUNCH,
	RGMNOTHING_STD,
	RGMNOTHING_WALK,
	RGMNOTHING_RUN,
	RGMNOTHING_SWAT,
	RGMNOTHING_CROUCH,
	RGMHANDGUN_S_SHOT,
	RGMHANDGUN_C_SHOT,
	RGMHANDGUN_PRONE,
	RGMDIE_JFK,
	RGMOPEN,
	RGMPICKUP,
	RGMSTAB,
	RGMSLICE,
	RGMCSTAB,
	RGMMEDIC,
	RGMDODGE,
	RGMSTANDDWALAIM,
	RGMRAISE,
	RGMTHROW,
	RGMLOB,
	RGMKICKDOOR,
	RGMRHIT,
	RGM_SQUISH,
	RGM_LOOK,
	RGM_PULL,
	RGM_SPIT,
	RGMWATER_R_WALK,
	RGMWATER_R_STD,
	RGMWATER_N_WALK,
	RGMWATER_N_STD,
	RGMWATER_DIE,
	RGMWATER_N_AIM,
	RGMWATER_R_AIM,
	RGMWATER_DBLSHT,
	RGMWATER_TRANS,
	RGMDEEPWATER_TRED,
	RGMDEEPWATER_SWIM,
	RGMDEEPWATER_DIE,
	RGMMCLIMB,
	RGMHELIDROP,
	RGMLOWKICK,
	RGMNPUNCH,
	RGMSPINKICK,
	RGMSLEEP,
	RGMSHOOT_LOW,
	RGMCDBLSHOT,
	RGMHURTSTANDINGN,
	RGMHURTSTANDINGR,
	RGMHURTWALKINGN,
	RGMHURTWALKINGR,
	RGMHURTTRANS,
	RGMTHROWKNIFE,
	RGMBREATHKNIFE,
	RGMPISTOLBREATH,
	RGMCOWER,
	RGMROCKET,
	RGMMORTAR,
	RGMSIDESTEP,
	RGMDBLBREATH,
	RGMPUNCHLOW,
	RGMPISTOLSHOOTLOW,
	RGMWATERTHROW,
	RGMRADIO,
	RGMCRRADIO,
	RGMBURN,
	RGMDWPRONE,
	RGMDRUNK,
	RGMPISTOLDRUNK,
	RGMCROWBAR,
	RGMJUMPOVER,

	BGMWALKING,
	BGMSTANDING,
	BGMCROUCHING,
	BGMSNEAKING,
	BGMRUNNING,
	BGMPRONE,
	BGMSTANDAIM,
	BGMHITHARD,
	BGMHITSTAND,
	BGMHITHARDBLOOD,
	BGMCROUCHAIM,
	BGMHITFALLBACK,
	BGMROLLOVER,
	BGMCLIMBROOF,
	BGMFALL,
	BGMFALLF,
	BGMHITCROUCH,
	BGMHITPRONE,
	BGMHOPFENCE,
	BGMPUNCH,
	BGMNOTHING_STD,
	BGMNOTHING_WALK,
	BGMNOTHING_RUN,
	BGMNOTHING_SWAT,
	BGMNOTHING_CROUCH,
	BGMHANDGUN_S_SHOT,
	BGMHANDGUN_C_SHOT,
	BGMHANDGUN_PRONE,
	BGMDIE_JFK,
	BGMOPEN,
	BGMPICKUP,
	BGMSTAB,
	BGMSLICE,
	BGMCSTAB,
	BGMMEDIC,
	BGMDODGE,
	BGMSTANDDWALAIM,
	BGMRAISE,
	BGMTHROW,
	BGMLOB,
	BGMKICKDOOR,
	BGMRHIT,
	BGMSTANDAIM2,
	BGMFLEX,
	BGMSTRECH,
	BGMSHOEDUST,
	BGMHEADTURN,
	BGMWATER_R_WALK,
	BGMWATER_R_STD,
	BGMWATER_N_WALK,
	BGMWATER_N_STD,
	BGMWATER_DIE,
	BGMWATER_N_AIM,
	BGMWATER_R_AIM,
	BGMWATER_DBLSHT,
	BGMWATER_TRANS,
	BGMDEEPWATER_TRED,
	BGMDEEPWATER_SWIM,
	BGMDEEPWATER_DIE,
	BGMHELIDROP,
	BGMSLEEP,
	BGMSHOOT_LOW,
	BGMTHREATENSTAND,
	BGMCDBLSHOT,
	BGMHURTSTANDINGN,
	BGMHURTSTANDINGR,
	BGMHURTWALKINGN,
	BGMHURTWALKINGR,
	BGMHURTTRANS,
	BGMTHROWKNIFE,
	BGMBREATHKNIFE,
	BGMPISTOLBREATH,
	BGMCOWER,
	BGMRAISE2,
	BGMROCKET,
	BGMMORTAR,
	BGMSIDESTEP,
	BGMDBLBREATH,
	BGMPUNCHLOW,
	BGMPISTOLSHOOTLOW,
	BGMWATERTHROW,
	BGMWALK2,
	BGMRUN2,
	BGMIDLENECK,
	BGMCROUCHTRANS,
	BGMRADIO,
	BGMCRRADIO,
	BGMDWPRONE,
	BGMDRUNK,
	BGMPISTOLDRUNK,
	BGMCROWBAR,
	BGMJUMPOVER,


	RGFWALKING,
	RGFSTANDING,
	RGFCROUCHING,
	RGFSNEAKING,
	RGFRUNNING,
	RGFPRONE,
	RGFSTANDAIM,
	RGFHITHARD,
	RGFHITSTAND,
	RGFHITHARDBLOOD,
	RGFCROUCHAIM,
	RGFHITFALLBACK,
	RGFROLLOVER,
	RGFCLIMBROOF,
	RGFFALL,
	RGFFALLF,
	RGFHITCROUCH,
	RGFHITPRONE,
	RGFHOPFENCE,
	RGFPUNCH,
	RGFNOTHING_STD,
	RGFNOTHING_WALK,
	RGFNOTHING_RUN,
	RGFNOTHING_SWAT,
	RGFNOTHING_CROUCH,
	RGFHANDGUN_S_SHOT,
	RGFHANDGUN_C_SHOT,
	RGFHANDGUN_PRONE,
	RGFDIE_JFK,
	RGFOPEN,
	RGFPICKUP,
	RGFSTAB,
	RGFSLICE,
	RGFCSTAB,
	RGFMEDIC,
	RGFDODGE,
	RGFSTANDDWALAIM,
	RGFRAISE,
	RGFTHROW,
	RGFLOB,
	RGFKICKDOOR,
	RGFRHIT,
	RGFCLEAN,
	RGFKICKSN,
	RGFALOOK,
	RGFWIPE,
	RGFWATER_R_WALK,
	RGFWATER_R_STD,
	RGFWATER_N_WALK,
	RGFWATER_N_STD,
	RGFWATER_DIE,
	RGFWATER_N_AIM,
	RGFWATER_R_AIM,
	RGFWATER_DBLSHT,
	RGFWATER_TRANS,
	RGFDEEPWATER_TRED,
	RGFDEEPWATER_SWIM,
	RGFDEEPWATER_DIE,
	RGFHELIDROP,
	RGFSLEEP,
	RGFSHOOT_LOW,
	RGFCDBLSHOT,
	RGFHURTSTANDINGN,
	RGFHURTSTANDINGR,
	RGFHURTWALKINGN,
	RGFHURTWALKINGR,
	RGFHURTTRANS,
	RGFTHROWKNIFE,
	RGFBREATHKNIFE,
	RGFPISTOLBREATH,
	RGFCOWER,
	RGFROCKET,
	RGFMORTAR,
	RGFSIDESTEP,
	RGFDBLBREATH,
	RGFPUNCHLOW,
	RGFPISTOLSHOOTLOW,
	RGFWATERTHROW,
	RGFRADIO,
	RGFCRRADIO,
	RGFSLAP,
	RGFDWPRONE,
	RGFDRUNK,
	RGFPISTOLDRUNK,
	RGFCROWBAR,
	RGFJUMPOVER,

	AFMONSTERSTANDING,
	AFMONSTERWALKING,
	AFMONSTERATTACK,
	AFMONSTERCLOSEATTACK,
	AFMONSTERSPITATTACK,
	AFMONSTEREATING,
	AFMONSTERDIE,
	AFMUP,
	AFMJUMP,
	AFMMELT,

	LVBREATH,
	LVDIE,
	LVWALK,

	IBREATH,
	IWALK,
	IDIE,
	IEAT,
	IATTACK,

	QUEENMONSTERSTANDING,
	QUEENMONSTERREADY,
	QUEENMONSTERSPIT_SW,
	QUEENMONSTERSPIT_E,
	QUEENMONSTERSPIT_NE,
	QUEENMONSTERSPIT_S,
	QUEENMONSTERSPIT_SE,
	QUEENMONSTERDEATH,
	QUEENMONSTERSWIPE,

	FATMANSTANDING,
	FATMANWALKING,
	FATMANRUNNING,
	FATMANDIE,
	FATMANASS,
	FATMANACT,
	FATMANCOWER,
	FATMANDIE2,
	FATMANCOWERHIT,

	MANCIVSTANDING,
	MANCIVWALKING,
	MANCIVRUNNING,
	MANCIVDIE,
	MANCIVACT,
	MANCIVCOWER,
	MANCIVDIE2,
	MANCIVSMACKED,
	MANCIVPUNCH,
	MANCIVCOWERHIT,

	MINICIVSTANDING,
	MINICIVWALKING,
	MINICIVRUNNING,
	MINICIVDIE,
	MINISTOCKING,
	MINIACT,
	MINICOWER,
	MINIDIE2,
	MINICOWERHIT,

	DRESSCIVSTANDING,
	DRESSCIVWALKING,
	DRESSCIVRUNNING,
	DRESSCIVDIE,
	DRESSCIVACT,
	DRESSCIVCOWER,
	DRESSCIVDIE2,
	DRESSCIVCOWERHIT,

	HATKIDCIVSTANDING,
	HATKIDCIVWALKING,
	HATKIDCIVRUNNING,
	HATKIDCIVDIE,
	HATKIDCIVJFK,
	HATKIDCIVYOYO,
	HATKIDCIVACT,
	HATKIDCIVCOWER,
	HATKIDCIVDIE2,
	HATKIDCIVCOWERHIT,
	HATKIDCIVSKIP,

	KIDCIVSTANDING,
	KIDCIVWALKING,
	KIDCIVRUNNING,
	KIDCIVDIE,
	KIDCIVJFK,
	KIDCIVARMPIT,
	KIDCIVACT,
	KIDCIVCOWER,
	KIDCIVDIE2,
	KIDCIVCOWERHIT,
	KIDCIVSKIP,

	CRIPCIVSTANDING,
	CRIPCIVWALKING,
	CRIPCIVRUNNING,
	CRIPCIVBEG,
	CRIPCIVDIE,
	CRIPCIVDIE2,
	CRIPCIVKICK,

	COWSTANDING,
	COWWALKING,
	COWDIE,
	COWEAT,

	CROWWALKING,
	CROWFLYING,
	CROWEATING,
	CROWDYING,

	CATBREATH,
	CATWALK,
	CATRUN,
	CATREADY,
	CATHIT,
	CATDIE,
	CATSWIPE,
	CATBITE,

	ROBOTNWBREATH,
	ROBOTNWWALK,
	ROBOTNWHIT,
	ROBOTNWDIE,
	ROBOTNWSHOOT,

	HUMVEE_BASIC,
	HUMVEE_DIE,

	TANKNW_READY,
	TANKNW_SHOOT,
	TANKNW_DIE,

	TANKNE_READY,
	TANKNE_SHOOT,
	TANKNE_DIE,

	ELDORADO_BASIC,
	ELDORADO_DIE,

	ICECREAMTRUCK_BASIC,
	ICECREAMTRUCK_DIE,

	JEEP_BASIC,
	JEEP_DIE,

	BODYEXPLODE,

	NUMANIMATIONSURFACETYPES

} AnimationSurfaceTypes;

// Enumerations for struct data
typedef enum
{
	S_STRUCT,
	C_STRUCT,
	P_STRUCT,
	F_STRUCT,
	FB_STRUCT,
	DEFAULT_STRUCT,
	NUM_STRUCT_IDS,
	NO_STRUCT = 120,
};

// Struct for animation 'surface' information
typedef struct
{
	UINT16								  ubName;
	CHAR8										Filename[ 50 ];
	CHAR8										bStructDataType;
	UINT8										ubFlags;
	UINT32									uiNumDirections;
	UINT32									uiNumFramesPerDir;
	HVOBJECT								hVideoObject;
	void										*Unused;
	INT8										bUsageCount;
	INT8										bProfile;

} AnimationSurfaceType;


typedef struct
{
	CHAR8										Filename[ 50 ];
	STRUCTURE_FILE_REF *		pStructureFileRef;

} AnimationStructureType;


AnimationSurfaceType		gAnimSurfaceDatabase[ NUMANIMATIONSURFACETYPES ];
AnimationStructureType	gAnimStructureDatabase[ TOTALBODYTYPES ][ NUM_STRUCT_IDS ];


// Functions
BOOLEAN InitAnimationSystem( );
BOOLEAN DeInitAnimationSystem( );
BOOLEAN LoadAnimationSurface(  UINT16 usSoldierID, UINT16 usSurfaceIndex, UINT16 usAnimState );
BOOLEAN UnLoadAnimationSurface(  UINT16 usSoldierID, UINT16 usSurfaceIndex );
void ClearAnimationSurfacesUsageHistory( UINT16 usSoldierID );


void DeleteAnimationProfiles( );
BOOLEAN LoadAnimationProfiles( );

STRUCTURE_FILE_REF	*GetAnimationStructureRef( UINT16 usSoldierID, UINT16 usSurfaceIndex, UINT16 usAnimState );
STRUCTURE_FILE_REF	*GetDefaultStructureRef( UINT16 usSoldierID );

// Profile data
ANIM_PROF		*gpAnimProfiles;
UINT8				gubNumAnimProfiles;

void ZeroAnimSurfaceCounts( );

#endif
