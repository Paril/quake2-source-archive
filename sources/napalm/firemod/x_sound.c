/*==========================================================================
//  x_sound.c -- by Patrick Martin              Last updated:  2-5-1999
//--------------------------------------------------------------------------
//  This file contains code that plays various player sounds.
//========================================================================*/

#include "g_local.h"
#include "x_sound.h"


/*========================================================================*/

//*-----------------------------------------------------/ New Code /--------
//  This checks if the PPM's drown wav is 'drown' instead of 'drown1'.
//------------------------------------------------------------------------*/
qboolean PBM_SfxAltDrown (edict_t *ent)
{
        char    *info;
        char    ppm[16];

        info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
        sscanf (info, "%15[^/]", ppm);

        if (Q_stricmp(ppm, "female") == 0)
                return true;
        if (Q_stricmp(ppm, "crakhor") == 0)
                return true;

        return false;
}

//*-----------------------------------------------------/ New Code /--------
//  This is called when a player drowns.
//------------------------------------------------------------------------*/
void PBM_SfxDrown (edict_t *ent)
{
        if (PBM_SfxAltDrown (ent))
                gi.sound (ent, CHAN_VOICE, gi.soundindex("*drown.wav"), 1, ATTN_NORM, 0);
        else
                gi.sound (ent, CHAN_VOICE, gi.soundindex("*drown1.wav"), 1, ATTN_NORM, 0);
}

//*-----------------------------------------------------/ New Code /--------
//  This is called when a player leaves the water.
//------------------------------------------------------------------------*/
void PBM_SfxGasp (edict_t *ent, int i)
{
/* The dead make no noise. */
        if (ent->health <= 0)
                return;

        gi.sound (ent, CHAN_VOICE, gi.soundindex(va("player/gasp%i.wav", i)), 1, ATTN_NORM, 0);
}

//*-----------------------------------------------------/ New Code /--------
//  This is called when a player gets hurt and is not on fire.
//------------------------------------------------------------------------*/
void PBM_SfxPain (edict_t *ent)
{
        int     l, r;

/* If underwater, make gurp sounds instead. */
        if (ent->waterlevel == 3)
        {       if (rand() & 1)
                        gi.sound (ent, CHAN_VOICE, gi.soundindex("*gurp1.wav"), 1, ATTN_NORM, 0);
                else
                        gi.sound (ent, CHAN_VOICE, gi.soundindex("*gurp2.wav"), 1, ATTN_NORM, 0);
                return;
        }

        r = 1 + (rand()&1);
        if (ent->health < 25)
                l = 25;
        else if (ent->health < 50)
                l = 50;
        else if (ent->health < 75)
                l = 75;
        else
                l = 100;

        gi.sound (ent, CHAN_VOICE, gi.soundindex(va("*pain%i_%i.wav", l, r)), 1, ATTN_NORM, 0);
}


/*===========================/  Rhino Stuff  /===========================*/

/*------------------------------------------------------/ New Code /--------
//  This determines if the player's PPM is some huge beast.
//------------------------------------------------------------------------*/
qboolean PBM_IsBeast (edict_t *ent)
{
        return false;
}

/*------------------------------------------------------/ New Code /--------
//  This is called when a player gets hurt and is on fire or in lava.
//------------------------------------------------------------------------*/
void PBM_SfxBurn (edict_t *ent)
{
/* Beast-like players scream like Stroggs; the rest scream normally. */
        if (PBM_IsBeast (ent))
        {       if (rand()&1)
                        gi.sound (ent, CHAN_VOICE, gi.soundindex("player/lava1.wav"), 1, ATTN_NORM, 0);
                else
                        gi.sound (ent, CHAN_VOICE, gi.soundindex("player/lava2.wav"), 1, ATTN_NORM, 0);
        }
        else
        {       if (rand()&1)
                        gi.sound (ent, CHAN_VOICE, gi.soundindex("player/burn1.wav"), 1, ATTN_NORM, 0);
                else
                        gi.sound (ent, CHAN_VOICE, gi.soundindex("player/burn2.wav"), 1, ATTN_NORM, 0);
        }
}

/*------------------------------------------------------/ New Code /--------
//  This is called when a player makes a footstep.
//------------------------------------------------------------------------*/
void PBM_SfxFootstep (edict_t *ent)
{
/* The dead make no noise. */
        if (ent->health <= 0)
                return;

        ent->s.event = EV_FOOTSTEP;
}


/*===========================/  END OF FILE  /===========================*/
