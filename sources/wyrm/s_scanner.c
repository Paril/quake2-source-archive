#include "g_local.h"

#define SCANNER_CENTER

void Use_Quad (edict_t *ent, gitem_t *item);
void    Use_Invulnerability (edict_t *ent, gitem_t *item);
qboolean Pickup_Health (edict_t *ent, edict_t *other);
qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other);
qboolean CTFPickup_Flag(edict_t *ent, edict_t *other);
qboolean CTFPickup_Tech (edict_t *ent, edict_t *other);
void            Use_MediPak (edict_t *ent, gitem_t *inv);

extern gitem_t *flag1_item;
extern gitem_t *flag2_item;

void	ClearScanner(gclient_t *client)
{
	client->pers.scanner_active = 0;
        client->pers.scanner_mode = 0;
}
void Toggle_Scanner (edict_t *ent)
{
        int index;

        if ((!ent->client) || (ent->health<=0) || (ent->movetype == MOVETYPE_NOCLIP))
		return;

        if (!ent->client->pers.scanner_active)
        {
                index = ITEM_INDEX(FindItem("cells"));
                if (!ent->client->pers.inventory[index])
                {
                        gi.cprintf (ent, PRINT_HIGH, "No cells for Scanner.\n");
                        return;
                }
                ent->client->pers.scanner_cells = 0;
        }

	// toggle low on/off bit (and clear scores/inventory display if required)

        if ((ent->client->pers.scanner_active & 1) && (ent->client->pers.scanner_mode < 2))
        {
                if (ent->client->pers.scanner_mode == 0)
                {
                        ent->client->pers.scanner_mode = 1;
                        return;
                }
                else if (ctf->value && (ent->client->pers.scanner_mode == 1))
                {
                        ent->client->pers.scanner_mode = 2;
                        return;
                }
        }
        
	if ((ent->client->pers.scanner_active ^= 1) & 1)
        {
                ent -> client -> showinventory = 0;
                ent -> client -> showscores = 0;
                ent -> client -> showhelp = 0;
        }
	// set "just changed" bit
        ent->client->pers.scanner_active |= 2;
        ent->client->pers.scanner_mode = 0;

	gi.sound (ent, CHAN_AUTO, gi.soundindex ("misc/comp_up.wav"), 1, ATTN_STATIC, 0);
}
void ShowScanner(edict_t *ent,char *layout)
{
        edict_t *target;

	char	stats[64],
                        *tag, *tag2;

	vec3_t	v;
        int index;

        //Check if enough ammo
        index = ITEM_INDEX(FindItem("cells"));
        if ((!ent->client->pers.inventory[index]) && (!ent->client->pers.scanner_cells))
        {
                ClearScanner(ent->client);
                return;
        }
        //Remove cells....
        if (!ent->client->pers.scanner_cells)
        {
                ent->client->pers.inventory[index]--;
                ent->client->pers.scanner_cells = SCANNER_FRAMES_PER_CELL;
        }

        ent->client->pers.scanner_cells--;

#ifdef SCANNER_CENTER
	// Main scanner graphic draw
        Com_sprintf (stats, sizeof(stats),"xv 80 yv 40 picn %s ", PIC_SCANNER_TAG);
	SAFE_STRCAT(layout,stats,LAYOUT_MAX_LENGTH);
        Com_sprintf (stats, sizeof(stats),"xv 122 yv 30 string2 TYPE: xv 160 string %s ", (ent->client->pers.scanner_mode == 0) ? "Life" : (ent->client->pers.scanner_mode == 1) ? "Items" : "Ctf");
	SAFE_STRCAT(layout,stats,LAYOUT_MAX_LENGTH);
#endif

#ifndef SCANNER_CENTER
	// Main scanner graphic draw
        Com_sprintf (stats, sizeof(stats),"xl 0 yt 4 picn %s ", PIC_SCANNER_TAG);
	SAFE_STRCAT(layout,stats,LAYOUT_MAX_LENGTH);
        Com_sprintf (stats, sizeof(stats),"xl 42 yt 164 string2 TYPE: xl 80 string %s ", (ent->client->pers.scanner_mode == 0) ? "Life" : (ent->client->pers.scanner_mode == 1) ? "Items" : "Ctf");
	SAFE_STRCAT(layout,stats,LAYOUT_MAX_LENGTH);
#endif

        // Scanner dots
        for (target = g_edicts; target < &g_edicts[globals.num_edicts]; target++)
	{
		float	len;

		int		hd;

                if (!target->inuse)
                        continue;
                if (target->movetype == MOVETYPE_NOCLIP)
                        continue;
                if (target->solid == SOLID_NOT)
                        continue;

		// in use 
                if (ent->client->pers.scanner_mode == 0)
                {
                        if (!(target->client||((target->svflags & SVF_MONSTER)&&(target->movetype == MOVETYPE_STEP))) || (target == ent) || (target -> health <= 0))
                                continue;
                }
                else if (ent->client->pers.scanner_mode == 1)
                {
                        if (target->touch != Touch_Item)
                                continue;
                        if (!target->item)
                                continue;
                }
                else if (ent->client->pers.scanner_mode == 2)
                {
                        int noctf = 1;

                        if (target->touch == Touch_Item)
                        {
                                if (target->item->pickup == CTFPickup_Tech)
                                        noctf = 0;
                                if (target->item->pickup == CTFPickup_Flag)
                                        noctf = 0;
                                
                        }

                        if ((target->client) && (target != ent) && (target->health > 0))
                                noctf = 0;

                        if (noctf)
                                continue;
                }


		// calc player to enemy vector
                VectorSubtract (ent->s.origin, target->s.origin, v);

		// save height differential
		hd = v[2] / SCANNER_UNIT;

		// remove height component
		v[2] = 0;

		// calc length of distance from top down view (no z)
		len = VectorLength (v) / SCANNER_UNIT;

		// in range ?
		if (len <= SCANNER_RANGE)
		{
			int		sx,
					sy;

			vec3_t	dp;

			vec3_t	normal = {0,0,-1};


			// normal vector to enemy
			VectorNormalize(v);

			// rotate round player view angle (yaw)
			RotatePointAroundVector( dp, normal, v, ent->s.angles[1]);

			// scale to fit scanner range (80 = pixel range of scanner)
			VectorScale(dp,len*80/SCANNER_RANGE,dp);

			// calc screen (x,y) (2 = half dot width)
			// Set output ...
#ifdef SCANNER_CENTER
			sx = (160 + dp[1]) - 2;
			sy = (120 + dp[0]) - 2;
#endif

#ifndef SCANNER_CENTER
                        sx = (80 + dp[1]) - 2;
                        sy = (80 + dp[0]) - 2 + 4;
#endif

			// setup dot graphic
			tag = PIC_DOT_TAG;
                        tag2  = "";

                        if (target->client)
                        {
                                if (ent->client->pers.scanner_mode == 0)
                                {
                                        if (target->client->quad_framenum > level.framenum)
                                                tag = PIC_QUADDOT_TAG;

                                        if (target->client->invincible_framenum > level.framenum)
                                                tag = PIC_INVDOT_TAG;
                                }
                                else if (ent->client->pers.scanner_mode == 2)
                                {
                                        if (!flag1_item || !flag2_item)
                                                CTFInit();
                                        
                                        switch (target->client->resp.ctf_team) {
                                        case CTF_TEAM1:
                                                if (target->client->pers.inventory[ITEM_INDEX(flag2_item)])
                                                        tag2 = PIC_BLUE_ROUND;
                                                tag = PIC_INVDOT_TAG;

                                                break;
                                        case CTF_TEAM2:
                                                if (target->client->pers.inventory[ITEM_INDEX(flag1_item)])
                                                        tag2 = PIC_RED_ROUND;
                                                tag = PIC_QUADDOT_TAG;
                                                
                                                break;
                                        }


                                }
                        }

                        if (target->touch == Touch_Item && target->item)
                        {
                                if (target->item->use == Use_Quad)
                                        tag = PIC_QUADDOT_TAG;
                                else if (target->item->use == Use_Invulnerability)
                                        tag = PIC_INVDOT_TAG;
                                else if ((target->item->use == Use_MediPak)||
                                         (target->item->pickup == Pickup_Health) ||
                                         (target->item->pickup == Pickup_Adrenaline)
                                        )
                                        tag = PIC_HEALTHDOT_TAG;
                                else if (target->item->pickup == CTFPickup_Tech)
                                        tag = PIC_TECHDOT_TAG;

                                else if (target->item->pickup == CTFPickup_Flag)
                                {
                                        if (strcmp(target->classname, "item_flag_team1") == 0)
                                        {
                                                tag = PIC_INVDOT_TAG;
                                                tag2 = PIC_RED_ROUND;
                                        }
                                        else
                                        {
                                                tag = PIC_QUADDOT_TAG;
                                                tag2 = PIC_BLUE_ROUND;
                                        }
                                }
                        }

			// Set output ...
                        if (*tag2 != '\0')
                        {
#ifdef SCANNER_CENTER
                                Com_sprintf (stats, sizeof(stats),"xv %i yv %i picn %s ",
                                                sx-2,
                                                sy-2,
                                                tag2);
#endif



#ifndef SCANNER_CENTER
                                Com_sprintf (stats, sizeof(stats),"xl %i yt %i picn %s ",
                                                sx-2,
                                                sy-2,
                                                tag2);
#endif

                                SAFE_STRCAT(layout,stats,LAYOUT_MAX_LENGTH);
                        }

                        if (hd < 0)
                        {
                                sy = sy - 5;
                                if (tag == PIC_DOT_TAG)
                                        tag = PIC_DOT_UP_TAG;
                                else if (tag == PIC_QUADDOT_TAG)
                                        tag = PIC_QUADDOT_UP_TAG;
                                else if (tag == PIC_INVDOT_TAG)
                                        tag = PIC_INVDOT_UP_TAG;
                                else if (tag == PIC_TECHDOT_TAG)
                                        tag = PIC_TECHDOT_UP_TAG;
                                else if (tag == PIC_HEALTHDOT_TAG)
                                        tag = PIC_HEALTHDOT_UP_TAG;

                        }
                        else if (hd > 0)
                        {
                                if (tag == PIC_DOT_TAG)
                                        tag = PIC_DOT_DOWN_TAG;
                                else if (tag == PIC_QUADDOT_TAG)
                                        tag = PIC_QUADDOT_DOWN_TAG;
                                else if (tag == PIC_INVDOT_TAG)
                                        tag = PIC_INVDOT_DOWN_TAG;
                                else if (tag == PIC_TECHDOT_TAG)
                                        tag = PIC_TECHDOT_DOWN_TAG;
                                else if (tag == PIC_HEALTHDOT_TAG)
                                        tag = PIC_HEALTHDOT_DOWN_TAG;
                        }

#ifdef SCANNER_CENTER
			// Set output ...
			Com_sprintf (stats, sizeof(stats),"xv %i yv %i picn %s ",
					sx,
					sy,
					tag);
#endif

#ifndef SCANNER_CENTER
			// Set output ...
                        Com_sprintf (stats, sizeof(stats),"xl %i yt %i picn %s ",
					sx,
					sy,
					tag);

#endif
			SAFE_STRCAT(layout,stats,LAYOUT_MAX_LENGTH);
		}
	}
}
