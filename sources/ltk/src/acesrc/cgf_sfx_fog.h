/****************************************************************************/
/*                                                                          */
/*    project    : CGF                  (c) 1999 William van der Sterren    */
/*                                                                          */
/*      file     : cgf_sfx_fog.h       "fog openGL stuff"                   */
/*      author(s): William van der Sterren                                  */
/*      version  : 0.5                                                      */
/*                                                                          */
/*      date (last revision):  Aug 31, 99                                   */
/*      date (creation)     :  Aug 31, 99                                   */
/*                                                                          */
/*                                                                          */
/*      revision history                                                    */
/*      -- date ---- | -- revision ---------------------- | -- revisor --   */
/*                   |                                    |                 */
/*                                                                          */
/****************************************************************************/


#ifndef __CGF_SFX_FOG_
#define __CGF_SFX_FOG_

#ifndef AMIGA
typedef float vec_t;
#endif

#define     kFogMaxVisibilityReduction 0.02

void        CGF_SFX_InstallFogSupport();

void        CGF_SFX_AdjustFogForMap(const char* aMapName);

int         CGF_SFX_IsFogEnabled();

vec_t       CGF_SFX_GetFogForDistance(vec_t aDist);

vec_t       CGF_SFX_GetViewingDistanceUpToReduction(vec_t aReduction);

const char* CGF_SFX_FogSettingsInfo();

/*
// debugging facilities
#ifdef _DEBUG

typedef struct edict_s edict_t;

void     CGF_SFX_Fog_DisplayRods(edict_t* aClient);

#endif
*/

#endif // __CGF_SFX_FOG_
