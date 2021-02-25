/*****************************************************************

	Cam Client source code - by Paul Jordan

	..............................................................

	This file is Copyright(c) 1998, Paul Jordan, All Rights Reserved.

	..............................................................

	All other files are Copyright(c) Id Software, Inc.

	Please see liscense.txt in the source directory for the copyright
	information regarding those files belonging to Id Software, Inc.

	..............................................................
	
	Should you decide to release a modified version of the Camera, you
    MUST include the following text (minus the BEGIN and END lines) in 
    the documentation for your modification, and also on all web pages 
	related to your modifcation, should they exist.

	--- BEGIN ---

	The Client Camera is a product of Paul Jordan, and is available from
	the Quake2 Camera homepage at http://www.prismnet.com/~jordan/q2cam, 
    Or as part of the Eraser Bot at http://impact.frag.com.

	This program is a modification of the Quake2 Client Camera, and is
    therefore in NO WAY supported by Paul Jordan.

	This program MUST NOT be sold in ANY form. If you have paid for 
	this product, you should contact Paul Jordan immediately, via
	the Quake2 Camera Client homepage.

	--- END ---

    Adios and have fun,

	Paul Jordan

 *****************************************************************/

#define MAX_VISIBLE_RANGE   1000
#define CAMERA_SWITCH_TIME  20
#define CAMERA_DEAD_SWITCH_TIME 2
#define TRUE 1
#define FALSE 0

void CameraThink(edict_t *ent, usercmd_t *ucmd);
void PlayerDied(edict_t *ent);

typedef struct sPlayerList_s
{
    edict_t     *pEntity;
    struct sPlayerList_s *pNext;
} sPlayerList;

void CameraCmd();
void MakeCamera(edict_t *ent);
void EntityListRemove(edict_t *pEntity);
void EntityListAdd(edict_t *pEntity);
unsigned long EntityListNumber();
void PrintEntityList();
void EnitityListClean();
sPlayerList *EntityListHead();
sPlayerList *EntityListNext(sPlayerList *pCurrent);
sPlayerList *pTempFind;


