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

//#ifdef FOG


#include <cmath> // prevent problems between C and STL

extern "C"
{
#include "cgf_sfx_fog.h"
#include "../g_local.h"
//#include "sabin_debug.h"
#include <windows.h>
#include <GL/gl.h> 

/*
#ifdef CGF
// AI update related to fog
#include "cgf_tactics_visibility.h"
#endif
*/
}

#include <vector>
#include <fstream>
#include <strstream>
using namespace std;

// defaults
static vec_t  m_FogDensity     = 0.1000;
static int    m_FogEnabled     = false;
static int    m_FogType        =    2; // linear
static int    m_FogStart       =  200;
static int    m_FogEnd         = 1600;
static vec_t  m_FogColor[4]    = {0.5, 0.5, 0.5, 1};
static char   m_Settings[64];


const int     kMaxViewingDistance  = 8192;
const int     kViewingDistanceStep =   64;
static vec_t  m_FogViewReduction[kMaxViewingDistance / kViewingDistanceStep];
//static vec_t  m_FogViewReduction[128];

// cvar for fog and video mode
static cvar_t*  fog     = 0;
static cvar_t*  vid_ref = 0;


// forward decls
void     CGF_SFX_ActivateFog(vec_t       aDensity,
                             int         aType,
                             vec3_t      aColor,
                             vec_t       aBegin,
                             vec_t       anEnd
                            );
void     CGF_SFX_DeactivateFog();
qboolean     CGF_SFX_FindFogPreferences(const char* aMapName, 
                                    vec_t*      aDensity,
                                    int*        aType,
                                    vec_t*      aRed,
                                    vec_t*      aGreen,
                                    vec_t*      aBlue,
                                    vec_t*      aBegin,
                                    vec_t*      aEnd
                                   );
void     CGF_SFX_InitFogForDistance(int aFogMode, vec_t aFogDensity, vec_t aFogBegin, vec_t aFogEnd);


void     CGF_SFX_InstallFogSupport()
{
  // reset settings
  m_Settings[0] = 0;//char(0);

  // if fog > 0, then enabled
  fog     = gi.cvar("fog", "0", 0);
  vid_ref = gi.cvar("vid_ref", "", 0);

  // prevent issueing gl calls if no opengl present
  m_FogEnabled = (   (fog->value > 0) 
                  && (0 != stricmp(vid_ref->string, "soft"))
                 );

  // if not fog enabled, act upon it
  if (!m_FogEnabled)
    {
      if (fog->value == 0)
        {
          strcpy(m_Settings, "cvar fog is 0");
//          CGF_Debug_LogMessage("fog disabled: cvar fog is 0");
        } 
      else
        {
          strcpy(m_Settings, "using software rendering");
//          CGF_Debug_LogMessage("fog disabled: using software rendering");
        }
      gi.cvar_set("fog", "0");
    }
}


void     CGF_SFX_AdjustFogForMap(const char* aMapName)
{
  // verify fog if m_FogEnabled, and reset if required
  fog = gi.cvar("fog", "0", 0);
  vid_ref = gi.cvar("vid_ref", "", 0);

  if (m_FogEnabled)
    {
      if (!(m_FogEnabled = (fog->value != 0)))
        {
          #ifdef CGF
          CGF_Debug_LogMessage("fog disabled: fog set to 0 or video mode changed");
          // change visibility info for AI
          CGF_Tactics_IncorporateChangedFogSettings();
          #endif
        }
    }
  else
    {
      m_FogEnabled = (   (fog->value > 0) 
                      && (0 != stricmp(vid_ref->string, "soft"))
                     );
      if (m_FogEnabled)
        {
          #ifdef CGF
          CGF_Debug_LogMessage("fog enabled : fog set to 1 or video mode changed");
          // change visibility info for AI
          CGF_Tactics_IncorporateChangedFogSettings();
          #endif
        }
    }

  // process fog setting
  if (m_FogEnabled)
    {
      vec_t  d;
      int    t;
      vec_t  s;
      vec_t  e;
      vec3_t rgb;
      if (   (!aMapName)
          || (!CGF_SFX_FindFogPreferences(aMapName, &d, &t, &rgb[0], &rgb[1], &rgb[2], &s, &e))
         )
        {
          if (!CGF_SFX_FindFogPreferences("default", &d, &t, &rgb[0], &rgb[1], &rgb[2], &s, &e))
            {
              d = m_FogDensity;
              t = m_FogType;
              VectorCopy(m_FogColor, rgb);
              s = m_FogStart;
              e = m_FogEnd;
            } 
        }  
      // convert d to value usable for gl lib
      d /= 100;
      // convert fog type
      // 0 == GL_EXP, 1 == GL_EXP2, 2 == GL_LINEAR
      if ((t < 0) || (t > 2))
        t = m_FogType;
      if (t < 2)
        t     = t + 0x800;
      else 
        t     = GL_LINEAR;

      if (d > 0)
        CGF_SFX_ActivateFog(d, t, rgb, s, e);
      else
        CGF_SFX_DeactivateFog();
    }
  else
    {
      CGF_SFX_DeactivateFog();
    }
}


qboolean     CGF_SFX_FindFogPreferences(const char* aMapName, 
                                    vec_t*      aDensity,
                                    int*        aType,
                                    vec_t*      aRed,
                                    vec_t*      aGreen,
                                    vec_t*      aBlue,
                                    vec_t*      aBegin,
                                    vec_t*      aEnd
                                   )
{
  *aDensity = 0;

  // try to read preferences from file action\cgf\fog\<mapname>.fog
  // format for file is:
  // // format: mapname density  fogtype  red green blue

  char buf_filename[256];

  ostrstream filename(buf_filename, 255);

  cvar_t* gamedir;
  cvar_t* basedir;

	gamedir = gi.cvar ("game", "", 0);
	basedir = gi.cvar("basedir", ".", 0);

  // obtain file location
  filename << basedir->string << '\\' << gamedir->string 
           << '\\' << "cgf\\fog\\" << aMapName << ".fog" << char(0);

  // open file, append mode
  ifstream preferences(buf_filename, ios_base::in); 
  if (preferences.good())
    {
      char name[128];
      memset(name,0, sizeof(name));
      preferences >> name;

      // skip comment lines if any
      while (   (strlen(name) >= 2)
             && (name[0] == '/')
             && (   (name[1] == '/')
                 || (name[1] == '*')
                )
            )
        {
          preferences.getline(name, 128);
          memset(name,0, sizeof(name));
          preferences >> name;
        }

      /* test for name similarity removed 
      if (   (strlen(name))
          && ( 0 == stricmp(name, aMapName))
         )
      */ 
        {
          preferences >> *aDensity;
          if (*aDensity > 0)
            {
              preferences >> *aType;
              preferences >> *aRed;
              preferences >> *aGreen;
              preferences >> *aBlue;
              preferences >> *aBegin;
              preferences >> *aEnd;
            }
          // adjust settings info
          ostrstream settings(buf_filename, 63);
          settings << "cgf\\fog\\" << aMapName << ".fog" << char(0);   
          strcpy(m_Settings, buf_filename); 
          return true;
        }  
     }
  return false;
}


void     CGF_SFX_ActivateFog(vec_t       aDensity,
                             int         aType,
                             vec3_t      aColor,
                             vec_t       aBegin,
                             vec_t       anEnd
                            )
{
  #ifdef CGF
  if (CGF_Debug_IsLogging())
    {  
      char message[256];
      if (aDensity)
        sprintf(message,
                "fog settings: mode %d, density %.2f, rgb=[%.2f, %.2f, %.2f], begin %d, end %d",
                 (aType == GL_LINEAR ? 2 : aType - GL_FOG_START), 
                 aDensity, aColor[0], aColor[1], aColor[2], (int) aBegin, (int) anEnd
               );
      else
        sprintf(message,
                "fog settings: mode %d, density %.2f == disabled",
                 (aType == GL_LINEAR ? 2 : aType - GL_FOG_START), 
                 aDensity
               );
      CGF_Debug_LogMessage(message);
    }
  #endif
  CGF_SFX_InitFogForDistance(aType, aDensity, aBegin, anEnd);

  glEnable (GL_FOG);                       // turn on fog, otherwise you won't see any

  glFogi   (GL_FOG_MODE,    aType);       
  if (aType == GL_LINEAR)
    {
		  glFogf (GL_FOG_START, aBegin);
  		glFogf (GL_FOG_END,   anEnd);
    }
  
  glFogfv  (GL_FOG_COLOR,   aColor);   
  glFogf   (GL_FOG_DENSITY, aDensity); 
  glHint   (GL_FOG_HINT,    GL_NICEST);
}


void     CGF_SFX_DeactivateFog()
{
  CGF_SFX_InitFogForDistance(0, 0, 0, 0);

  // prevent opengl calls when no opengl
  if (   (vid_ref)
      && (0 == stricmp(vid_ref->string, "soft"))
     )
    return;

  glDisable (GL_FOG);                   // turn off fog
}


int      CGF_SFX_IsFogEnabled()
{
  return m_FogEnabled;
}


void     CGF_SFX_InitFogForDistance(int aFogMode, vec_t aFogDensity, vec_t aFogBegin, vec_t aFogEnd)
{
  // determine how many cells
  int cells;
  cells = kMaxViewingDistance / kViewingDistanceStep;

  vec_t dist;
  vec_t maxdist;
  vec_t mindist;

  if (aFogDensity == 0.0)
    aFogMode = 0;

  // adjust fog density value to our values
  aFogDensity *= 100;

  switch (aFogMode)
  {
    case GL_LINEAR:
      maxdist = aFogEnd; 
      mindist = aFogBegin; 
      break;  
    case GL_EXP:
      maxdist = 400.0 / sqrt(aFogDensity); 
      break;  
    case GL_EXP2:
      maxdist = 130.0 / aFogDensity; 
      break;  
    default :
      maxdist = kMaxViewingDistance; 
  }
  
  dist = 0;
  for (int cell = 0; cell < cells; cell++)
    {
       vec_t reduction;
       switch (aFogMode)
       {
         case GL_LINEAR:
           if (dist < mindist)
             reduction = 1;
           else
           if (dist > maxdist)
             reduction = 0;
           else
             reduction = 1 - (dist - mindist) / (maxdist - mindist);
           break;
         case GL_EXP:
           reduction = 1 - dist / maxdist; 
           break;  
         case GL_EXP2:
           reduction = 1 - (dist / maxdist) * (dist / maxdist); 
           break;  
         default :
           reduction = 1.0;
       }
       if (reduction < 0)
         reduction = 0;
       m_FogViewReduction[cell] = reduction;

       dist += kViewingDistanceStep;
    }
}


const char* CGF_SFX_FogSettingsInfo()
{
  return (const char*) m_Settings;
}


vec_t    CGF_SFX_GetFogForDistance(vec_t dist)
{
  int index;
  index = ((int) dist) / (int) kViewingDistanceStep;
  assert( (index < (kMaxViewingDistance / kViewingDistanceStep))
        );
  return m_FogViewReduction[index];
}


vec_t    CGF_SFX_GetViewingDistanceUpToReduction(vec_t aReduction)
{
  int index;
  index = (kMaxViewingDistance / kViewingDistanceStep) - 1;
  while (   (index >= 0)
         && (m_FogViewReduction[index] < aReduction)
        )
    index--;

  return (vec_t) (index * kViewingDistanceStep);
}

/*
#ifdef _DEBUG
extern "C" void     CGF_SFX_Fog_DisplayRods(edict_t* aClient);

void     CGF_SFX_Fog_DisplayRods(edict_t* aClient)
{
  vec3_t forward;
  vec3_t v1;
  vec3_t v2;
  bool   half;
  bool   ten;

  half   = false;
  ten    = false;

  AngleVectors(aClient->s.angles, forward, 0, 0);
  forward[2] = 0;
  VectorNormalize(forward);

  for (int rod = 0; rod < 20; rod ++)
    {
      VectorMA(aClient->s.origin, 100 * ((float) rod), forward, v1);
      VectorCopy(v1, v2);
      v2[2] -= 1000;
      SABIN_Debug_DisplayLine(v1, v2, 60);

      if (   (!half) 
          && (half = (CGF_SFX_GetFogForDistance(100 * ((float) rod)) < 0.5))
         )
        {
          SABIN_Debug_DisplayPoint(v1, RF_SHELL_RED, 60);
        }
        
      if (   (!ten) 
          && (ten = (CGF_SFX_GetFogForDistance(100 * ((float) rod)) < 0.1))
         )
        {
          SABIN_Debug_DisplayPoint(v1, RF_SHELL_RED, 60);
        }
    }
  
}
#endif
*/

//#endif
