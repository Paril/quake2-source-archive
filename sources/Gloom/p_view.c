/*
    Quake II Glooom, a total conversion mod for Quake II
    Copyright (C) 1999-2007  Gloom Developers

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define MOD_BURNING MOD_MELEE

#include "g_local.h"
#include "m_player.h"
#include "m_mech.h"
#include "m_stalker.h"
#include "m_engineer.h"
#include "m_wraith.h"
void tripwire_activate (edict_t *mine);
void PMenu_Update(edict_t *ent);
void EngineerDie(edict_t *ent);
extern void GuardianMirrorSpawn(edict_t *ent);
extern void GuardianMirrorThink(edict_t *ent);
extern void GuardianRegen(edict_t *ent);

#define current_player level.current_entity
//static        edict_t         *current_player;
static  gclient_t       *current_client;

static  vec3_t  forward, right, up;
float   xyspeed;

float   bobmove;
int             bobcycle;               // odd cycles are right foot going forward
float   bobfracsin;             // sin(bobfrac*M_PI)

/*
SV_CalcRoll

*/
float SV_CalcRoll (vec3_t angles, vec3_t velocity)
{
        float   sign;
        float   side;
        float   value;

        side = DotProduct (velocity, right);
        sign = side < 0 ? -1 : 1;
        side = (float)fabs(side);

        value = sv_rollangle->value;

        if (side < sv_rollspeed->value)
                side = side * value / sv_rollspeed->value;
        else
                side = value;

        return side*sign;

}


/*
P_DamageFeedback

Handles color blends and view kicks
*/
void P_DamageFeedback (edict_t *player)
{
        gclient_t       *client;
        float   side;
        int             realcount, count, kick;
        vec3_t  v;
        int             r, l, heal;
        static  vec3_t  power_color = {0.0, 1.0, 0.0};
        static  vec3_t  acolor = {1.0, 1.0, 1.0};
        static  vec3_t  bcolor = {1.0, 0.0, 0.0};

        client = player->client;

        heal = 0;
	if (player->health > client->resp.healdetect)
	  heal = player->health - client->resp.healdetect;
	client->resp.healdetect = player->health;
	r = GetArmor(player);
	if (r > client->resp.repairdetect)
	  heal += r-client->resp.repairdetect;
	client->resp.repairdetect = r;
	if (heal > 0) LogHeal(player, heal);
	
	//Wrong placement for reducing push velocity fraction
	if (player->groundentity) {
	  r = (player->waterlevel>1)?7:6;

	  if (client->resp.pushfraction > r)
	    client->resp.pushfraction -= r;
	  else
	    client->resp.pushfraction = 0;
	} else
	if (client->resp.pushfraction > 75)
	  client->resp.pushfraction--;


        // flash the backgrounds behind the status numbers
        client->ps.stats[STAT_FLASHES] = 0;

        // total points of damage shot at the player this frame
        count = (client->damage_blood + client->damage_armor + client->damage_parmor);
        if (count == 0)
                return;         // didn't take any damage

        if (client->damage_blood)
                client->ps.stats[STAT_FLASHES] |= 1;
        if (client->damage_armor && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
                client->ps.stats[STAT_FLASHES] |= 2;

        realcount = count;
        if (count < 10)
                count = 10;     // always make a visible effect

        // start a pain animation if still in the player model
        if (client->damage_blood || client->damage_armor) {
        if (client->anim_priority < ANIM_PAIN && player->s.modelindex == 255)
        {
                int i, start, end;

                client->anim_priority = ANIM_PAIN;

                i = randomMT() & 2;

                if ((client->ps.pmove.pm_flags & PMF_DUCKED) && !classlist[client->resp.class_type].fixview)
                {
                        i = 4;
                }

                if (client->resp.flying)
                        i = 5;

                switch (i)
                {
                case 5:
                        // wraith fly pain
                        start = FRAME_PAIN_FLYING_S;
                        end = FRAME_PAIN_FLYING_E;
                        break;
                case 4:
                        // duck pain
                        start = FRAME_DUCKPAIN_S;
                        end = FRAME_DUCKPAIN_E;
                        break;
                case 1:
                        start = FRAME_PAIN2_S;
                        end = FRAME_PAIN2_E;
                        break;
                case 2:
                        start = FRAME_PAIN3_S;
                        end = FRAME_PAIN3_E;

                        if (client->resp.class_type != CLASS_WRAITH)
                                break;
                        // wraith drops to case 0/3
                default:
                case 3:
                case 0:
                        start = FRAME_PAIN1_S;
                        end = FRAME_PAIN1_E;
                        break;
                }

                player->s.frame = FrameReference (player, start)-1;
                client->anim_end = FrameReference (player, end);
        }

        //r1: always reset ht beep even if not playing pain sound
        if (player->client->resp.class_type == CLASS_HEAVY)
                player->client->missile_target = 0;

        // play an apropriate pain sound
        if (player->health > 0 && (client->damage_armor || client->damage_parmor || client->damage_blood > ((randomMT()&3)?1:0)) && (level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
        {
                float max = player->max_health;
                float cur = player->health;

                if(player->client->resp.team == TEAM_ALIEN)
                        player->pain_debounce_time = level.time + 0.2f;
                else {
                        player->pain_debounce_time = level.time + 0.5f;

                        if (player->client->resp.class_type == CLASS_MECH) {
                                cur = player->client->resp.inventory[ITEM_INDEX(player->client->armor)] + player->health;
                                max = classlist[player->client->resp.class_type].armorcount;
                        }
                        //r1: ht reset code was here
                        //} else 
                }
#ifdef CACHE_CLIENTSOUNDS
                if (cur/max < 0.25f)
                        l = 6;
                else if (cur/max < 0.50f)
                        l = 4;
                else if (cur/max < 0.75f)
                        l = 2;
                else
                        l = 0;

                r = randomMT()&1;

                gi.sound (player, CHAN_AUTO, SoundIndex(classlist[player->client->resp.class_type].sounds[SOUND_PAIN+l+r]), 1, ATTN_IDLE, 0);
#else
                if (cur/max < 0.25)
                        l = 25;
                else if (cur/max < 0.50)
                        l = 50;
                else if (cur/max < 0.75)
                        l = 75;
                else
                        l = 100;

                r = 1 + (randomMT()&1);

                gi.sound (player, CHAN_AUTO, gi.soundindex(va("*pain%i_%i.wav", l, r)), 1, ATTN_IDLE, 0);
#endif
        }
        }

        // the total alpha of the blend is always proportional to count
        if (client->damage_alpha < 0)
                client->damage_alpha = 0;
        client->damage_alpha += count*0.01f;
        if (client->damage_alpha < 0.2f)
                client->damage_alpha = 0.2;
        if (client->damage_alpha > 0.5f)
                client->damage_alpha = 0.5;             // don't go too saturated

        // the color of the blend will vary based on how much was absorbed
        // by different armors
        VectorClear (v);
        if (client->damage_parmor)
                VectorMA (v, (float)client->damage_parmor/realcount, power_color, v);
        if (client->damage_armor)
                VectorMA (v, (float)client->damage_armor/realcount, acolor, v);
        if (client->damage_blood)
                VectorMA (v, (float)client->damage_blood/realcount, bcolor, v);
        VectorCopy (v, client->damage_blend);


        //
        // calculate view angle kicks
        //
        kick = abs(client->damage_knockback);
        if (kick && player->health > 0) // kick of 0 means no view adjust at all
        {
                kick = (float)(kick * 100) / player->health;

                if (kick < count*0.5)
                        kick = count*0.5f;
                if (kick > 50)
                        kick = 50;

                VectorSubtract (client->damage_from, player->s.origin, v);
                VectorNormalize (v);

                side = DotProduct (v, right);
                client->v_dmg_roll = kick*side*0.3f;

                side = -DotProduct (v, forward);
                client->v_dmg_pitch = kick*side*0.3f;

                client->v_dmg_time = level.time + DAMAGE_TIME;
        }

        //
        // clear totals
        //
        client->damage_blood =
        client->damage_armor =
        client->damage_parmor =
        client->damage_knockback = 0;
}

/*
SV_CalcViewOffset

Auto pitching on slopes?

        fall from 128: 400 = 160000
        fall from 256: 580 = 336400
        fall from 384: 720 = 518400
        fall from 512: 800 = 640000
        fall from 640: 960 =

        damage = deltavelocity*deltavelocity    * 0.0001

*/
void SV_CalcViewOffset (edict_t *ent)
{
        float           *angles;
        float           bob;
        float           ratio;
        float           delta;
        vec3_t          v;
        int bigbob;

        // base angles
        angles = ent->client->ps.kick_angles;

        // if dead, don't add any kick
        if (ent->health > 0)
        {
                // add angles based on weapon kick

                VectorCopy (ent->client->kick_angles, angles);

                // add angles based on damage kick

                ratio = (ent->client->v_dmg_time - level.time) / DAMAGE_TIME;
                if (ratio < 0)
                {
                        ratio = 0;
                        ent->client->v_dmg_pitch = 0;
                        ent->client->v_dmg_roll = 0;
                }
                angles[PITCH] += ratio * ent->client->v_dmg_pitch;
                angles[ROLL] += ratio * ent->client->v_dmg_roll;

                // add pitch based on fall kick

                ratio = (ent->client->fall_time - level.time) / FALL_TIME;
                if (ratio < 0)
                        ratio = 0;
                angles[PITCH] += ratio * ent->client->fall_value;

                // add angles based on velocity

                delta = DotProduct (ent->velocity, forward);
                angles[PITCH] += delta*run_pitch->value;

                delta = DotProduct (ent->velocity, right);
                angles[ROLL] += delta*run_roll->value;

                // add angles based on bob
                if (!ent->client->resp.flying) {
                  if(classlist[ent->client->resp.class_type].bob)
                        bigbob = 1;
                  else
                        bigbob = 0;

                  delta = bobfracsin * bob_pitch->value * xyspeed;
                  if (ent->client->ps.pmove.pm_flags & PMF_DUCKED || bigbob)
                        delta *= 6;             // crouching
                  angles[PITCH] += delta;
                  delta = bobfracsin * bob_roll->value * xyspeed;
                  if (ent->client->ps.pmove.pm_flags & PMF_DUCKED || bigbob)
                        delta *= 6;             // crouching
                  if (bobcycle & 1)
                        delta = -delta;
                  angles[ROLL] += delta;
                }
        }

        // base origin

        VectorClear (v);

        // add view height

        v[2] += ent->viewheight;

        // add fall height

        ratio = (ent->client->fall_time - level.time) / FALL_TIME;
        if (ratio < 0)
                ratio = 0;
        v[2] -= ratio * ent->client->fall_value * 0.4f;

        // add bob height

        bob = bobfracsin * xyspeed * bob_up->value;
        if (bob > 6)
                bob = 6;
        //if(bob < -6)
                //bob = -6;
        //gi.DebugGraph (bob *2, 255);

        v[2] += bob;

        // add kick offset

        VectorAdd (v, ent->client->kick_origin, v);

        // absolutely bound offsets
        // so the view can never be outside the player box

        if (v[0] < -14)
                v[0] = -14;
        else if (v[0] > 14)
                v[0] = 14;
        if (v[1] < -14)
                v[1] = -14;
        else if (v[1] > 14)
                v[1] = 14;

        if (v[2] < -31)
                v[2] = -31;

        else if (v[2] > 31)
                v[2] = 31;

        VectorCopy (v, ent->client->ps.viewoffset);

        if (ent->client->resp.turret)
                ent->client->ps.viewoffset[2] += 16;

        //TODO: does this look good?
        if (ent->client->resp.class_type == CLASS_HATCHLING && ent->health > 0)
                ent->client->ps.viewoffset[2] -= 8;
}

/*
SV_CalcGunOffset
*/
void SV_CalcGunOffset (edict_t *ent)
{
        int             i;
        float   delta;

        // gun angles from bobbing
        ent->client->ps.gunangles[ROLL] = xyspeed * bobfracsin * 0.002f;
        ent->client->ps.gunangles[YAW] = xyspeed * bobfracsin * 0.01f;
        if (bobcycle & 1)
        {
                ent->client->ps.gunangles[ROLL] = -ent->client->ps.gunangles[ROLL];
                ent->client->ps.gunangles[YAW] = -ent->client->ps.gunangles[YAW];
        }

        ent->client->ps.gunangles[PITCH] = xyspeed * bobfracsin * 0.005f;

        // gun angles from delta movement
        for (i=0 ; i<3 ; i++)
        {
                delta = ent->client->oldviewangles[i] - ent->client->ps.viewangles[i];
                if (delta > 180)
                        delta -= 360;
                if (delta < -180)
                        delta += 360;
                if (delta > 45)
                        delta = 45;
                if (delta < -45)
                        delta = -45;
                if (i == YAW)
                        ent->client->ps.gunangles[ROLL] += 0.1f*delta;
                ent->client->ps.gunangles[i] += 0.2f * delta;
        }

        // gun height
        VectorClear (ent->client->ps.gunoffset);
//      ent->ps->gunorigin[2] += bob;

        // gun_x / gun_y / gun_z are development tools
        for (i=0 ; i<3 ; i++)
        {
                ent->client->ps.gunoffset[i] += forward[i]*(gun_y->value);
                ent->client->ps.gunoffset[i] += right[i]*gun_x->value;
                ent->client->ps.gunoffset[i] += up[i]* (-gun_z->value);
        }
}


/*
SV_AddBlend
*/
void SV_AddBlend (float r, float g, float b, float a, float *v_blend)
{
        float   a2, a3;

        if (a <= 0)
                return;
        a2 = v_blend[3] + (1-v_blend[3])*a;     // new total alpha
        a3 = v_blend[3]/a2;             // fraction of color from old

        v_blend[0] = v_blend[0]*a3 + r*(1-a3);
        v_blend[1] = v_blend[1]*a3 + g*(1-a3);
        v_blend[2] = v_blend[2]*a3 + b*(1-a3);
        v_blend[3] = a2;
}

/*
SV_CalcBlend
*/
void SV_CalcBlend (edict_t *ent)
{
        int             contents;
        vec3_t  vieworg;
        int             remaining;
        float   fog;
        edict_t*chased;
        
        if (ent->client->pers.badname) { //TODO: Try use layout to put conback centered on screen if colour is 9
          int colour = (int)(g_enforce_names->value*10)%10;

          if (!colour)
            ent->client->ps.blend[0] = ent->client->ps.blend[1] = ent->client->ps.blend[2] = 0;
          else
          if (colour == 1) { //BSOD theme
            ent->client->ps.blend[0] = ent->client->ps.blend[1] = 0;
            ent->client->ps.blend[2] = 0.5;
          } else
          if (colour == 2) { //dark green
            ent->client->ps.blend[0] = ent->client->ps.blend[2] = 0.05;
            ent->client->ps.blend[1] = 0.2;
          } else
          if (colour == 3) { //dark red
            ent->client->ps.blend[2] = ent->client->ps.blend[2] = 0.05;
            ent->client->ps.blend[0] = 0.2;
          } else
          if (colour == 4) { //dark blue
            ent->client->ps.blend[0] = ent->client->ps.blend[1] = 0.05;
            ent->client->ps.blend[2] = 0.2;
          } else
            ent->client->ps.blend[0] = ent->client->ps.blend[1] = ent->client->ps.blend[2] = 0.25; //colour 5 and unused numbers

          ent->client->ps.blend[3] = 1;
          return;
        }

        if (chased = GetChasedTargetPlayer(ent)) {
          memcpy (ent->client->ps.blend, chased->client->ps.blend, sizeof(ent->client->ps.blend));
          ent->viewheight = chased->viewheight;
          
          return;
        }

        ent->client->ps.blend[0] = ent->client->ps.blend[1] =
                ent->client->ps.blend[2] = ent->client->ps.blend[3] = 0;

        // add for contents
        VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);
        contents = gi.pointcontents (vieworg);
        if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) )
                ent->client->ps.rdflags |= RDF_UNDERWATER;
        else
                ent->client->ps.rdflags &= ~RDF_UNDERWATER;

        if (contents & (CONTENTS_SOLID) && !ent->client->resp.team)
//                SV_AddBlend (0.075, 0.075, 0.075, 0.35, ent->client->ps.blend);
                SV_AddBlend (0.0, 0.0, 0.0, 0.315, ent->client->ps.blend);
        else
        if (contents & (CONTENTS_SOLID|CONTENTS_LAVA))
                SV_AddBlend (1.0, 0.3, 0.0, 0.6, ent->client->ps.blend);
        else if (contents & CONTENTS_SLIME)
                SV_AddBlend (0.0, 0.1, 0.05, 0.6, ent->client->ps.blend);
        else if (contents & CONTENTS_WATER)
                SV_AddBlend (0.5, 0.3, 0.2, 0.4, ent->client->ps.blend);
        else
        if (level.leaked && sv_cheats->value)
          SV_AddBlend (0.075, 0.075, 0.075, 0.25, ent->client->ps.blend);

        if (ent->deadflag && ent->client->resp.class_type != CLASS_OBSERVER)
                SV_AddBlend (0.5, 0.0, 0.0, 0.4, ent->client->ps.blend);

        if (!ent->deadflag && ent->client->resp.class_type == CLASS_GUARDIAN && ent->s.modelindex == 0)
                SV_AddBlend (0.0, 0.0, 0.0, 0.25, ent->client->ps.blend);

        if (!ent->deadflag && ent->client->resp.class_type == CLASS_COMMANDO && !(ent->s.renderfx & RF_IR_VISIBLE))
                SV_AddBlend (0.0, 0.0, 0.0, 0.2, ent->client->ps.blend);

        // add for powerups
        if (ent->client->invincible_framenum > level.framenum)
        {
                remaining = ent->client->invincible_framenum - level.framenum;
                if (remaining > 15 || (remaining & 2) )
                        SV_AddBlend (1, 1, 0, 0.08, ent->client->ps.blend);
        }

        // Gloom pseudo fog
        if((fog=ent->client->fogged_alpha) > 0.0)
                SV_AddBlend (ent->client->fog_blend[0], ent->client->fog_blend[1], ent->client->fog_blend[2], (fog>=5.0?fog-5:fog), ent->client->ps.blend);

        if(ent->client->blinded_alpha > 0.0)
                SV_AddBlend (0.8, 0.8, 0.8, ent->client->blinded_alpha, ent->client->ps.blend);


        // add for damage
        if (ent->client->damage_alpha > 0)
                SV_AddBlend (ent->client->damage_blend[0],ent->client->damage_blend[1]
                ,ent->client->damage_blend[2], ent->client->damage_alpha, ent->client->ps.blend);

        if (ent->client->bonus_alpha > 0)
                SV_AddBlend (0.85, 0.7, 0.3, ent->client->bonus_alpha, ent->client->ps.blend);

        // drop the damage value
        ent->client->damage_alpha -= 0.06;
        if (ent->client->damage_alpha < 0)
                ent->client->damage_alpha = 0;

        // drop the bonus value
        ent->client->bonus_alpha -= 0.1;
        if (ent->client->bonus_alpha < 0)
                ent->client->bonus_alpha = 0;

        ent->client->blinded_alpha -= 0.03;
        if (ent->client->blinded_alpha < 0)
                ent->client->blinded_alpha = 0;

        // GLOOM - Smoke Grenade
        if (ent->client->smokeintensity)
        {
                float x = (float) ent->client->smokeintensity/100;
                SV_AddBlend(0.7, 0.7, 0.7, x, ent->client->ps.blend);
                if (ent->client->smokeintensity > 0) {
                        if (ent->client->smokeintensity > 50)
                                ent->client->smokeintensity -= 2;
                        else
                                ent->client->smokeintensity -= 3;

                        if (ent->client->smokeintensity < 0)
                                ent->client->smokeintensity = 0;
                }
        }
}

/*
P_FallingDamage
*/
void P_FallingDamage (edict_t *ent)
{
        float   delta;
        int             damage;
        vec3_t  dir;
        float scale = 1;
        if (ent->client->ps.pmove.gravity < 0) scale = -1;
        
        if ((((int)dmflags->value & DF_GLOOMPMOVE) == 0) && ent->client->resp.class_type == CLASS_EXTERM && ent->client->resp.flying)
          scale = -1; //exterm old push flight

        //if (ent->s.modelindex != 255 && ent->client->resp.class_type != CLASS_GUARDIAN)
                //return;               // not in the player model

        if (ent->deadflag == DEAD_DEAD)
                return;

        if (ent->movetype == MOVETYPE_NOCLIP)
                return;

        //brian: reversed it, used to say which ones didn't get hurt, I made it say which ones DO get hurt
        if ((ent->client->resp.team == TEAM_ALIEN) && (ent->client->resp.class_type != CLASS_BREEDER) && (ent->client->resp.class_type != CLASS_GUARDIAN) && (ent->client->resp.class_type != CLASS_STALKER)) {
                return;
        }

        if ((scale*ent->client->oldvelocity[2] < 0) && (scale*ent->velocity[2] > scale*ent->client->oldvelocity[2]) && (!ent->groundentity))
        {
                delta = ent->client->oldvelocity[2];
        }
        else
        {
                if (!ent->groundentity)
                        return;
                delta = ent->velocity[2] - ent->client->oldvelocity[2];
        }
        delta = delta*delta * 0.0001f;

        // no falling damage when grappling
        if ((ent->client->ctf_grapple && ent->client->ctf_grapple->spawnflags > CTF_GRAPPLE_STATE_FLY))
                return;

        // never take falling damage if completely underwater
        if (ent->waterlevel == 3)
                return;
        if (ent->waterlevel == 2)
                delta *= 0.5f;
        if (ent->waterlevel == 1)
                delta *= 0.75f;

        if (delta < 1)
                return;

        if (delta < 15)
        {
                if (ent->client->resp.team == TEAM_HUMAN)
                {
                        if(ent->client->resp.class_type != CLASS_MECH && ent->client->resp.class_type != CLASS_ENGINEER)
                        {
                                if(ent->client->resp.class_type == CLASS_EXTERM) 
                                {
#ifdef CACHE_CLIENTSOUNDS
                                        if(!ent->client->resp.flying)
                                        gi.sound (ent, CHAN_AUTO, SoundIndex (exterm_step), 1, ATTN_NORM, 0);
#else
                                        if(!ent->client->resp.flying)
                                        gi.sound (ent, CHAN_AUTO, gi.soundindex ("*step.wav"), 1, ATTN_NORM, 0);
#endif
                                }
                                else 
                                {
                                        if (classlist[ent->client->resp.class_type].footsteps) 
                                        {
                                                ent->s.event = EV_FOOTSTEP;
                                        }
                                }
                        }
                        return;
                }
        }

        ent->client->fall_value = delta*0.5f;

        if (ent->client->fall_value > 40)
                ent->client->fall_value = 40;

        ent->client->fall_time = level.time + FALL_TIME;

        if (delta > 30)
        {
                if (ent->health > 0)
                {
                        if (delta >= 55) 
                        {
                                if (ent->client->resp.class_type == CLASS_ENGINEER)
                                {
#ifdef CACHE_CLIENTSOUNDS
                                        gi.sound (ent, CHAN_AUTO, SoundIndex(classlist[CLASS_ENGINEER].sounds[SOUND_PAIN+5]), 1, ATTN_NORM, 0);
#else
                                        gi.sound (ent, CHAN_AUTO, gi.soundindex ("*pain25_1.wav"), 1, ATTN_IDLE, 0);
#endif
                                } 
                                else 
                                {
                                        //r1: s.event is much more net efficient than explicit gi.sound...
//#ifdef CACHE_CLIENTSOUNDS
//                                      gi.sound (ent, CHAN_AUTO, SoundIndex(classlist[ent->client->resp.class_type].sounds[SOUND_FALL]), 1, ATTN_NORM, 0);
//#else
                                        ent->s.event = EV_FALLFAR;
//#endif
                                }
                        } 
                        else 
                        {
                                if (ent->client->resp.class_type == CLASS_ENGINEER) 
                                {
#ifdef CACHE_CLIENTSOUNDS
                                        gi.sound (ent, CHAN_AUTO, SoundIndex(classlist[CLASS_ENGINEER].sounds[SOUND_PAIN + 4]), 1, ATTN_NORM, 0);
#else
                                        gi.sound (ent, CHAN_AUTO, gi.soundindex ("*pain50_1.wav"), 1, ATTN_IDLE, 0);
#endif
                                } 
                                else 
                                {
//#ifdef CACHE_CLIENTSOUNDS
//                                      gi.sound (ent, CHAN_AUTO, SoundIndex(classlist[ent->client->resp.class_type].sounds[SOUND_FALL+1]), 1, ATTN_NORM, 0);
//#else
                                        ent->s.event = EV_FALL;
                                }
                        }
//#endif
                }
                ent->pain_debounce_time = level.time;   // no normal pain sound


                damage = (delta * ((delta / 140)*2))*0.75f;

                VectorSet (dir, 0, 0, 1);

                if (damage < 2)
                        damage = 2;
                //damage *= 1.1;

                //poor guardian legs get smashed...
                if (ent->client->resp.class_type == CLASS_GUARDIAN)
                        damage *= 1.5f;

                T_Damage (ent, world, world, dir, ent->s.origin, vec3_origin, damage, 0, DAMAGE_IGNORE_RESISTANCES | DAMAGE_NO_ARMOR, MOD_FALLING);
        }
        else
        {
                if (ent->client->resp.team == TEAM_HUMAN) 
                {
                        if (classlist[ent->client->resp.class_type].footsteps) 
                        {
#ifdef CACHE_CLIENTSOUNDS
                                gi.sound (ent, CHAN_AUTO, SoundIndex(player_land1), 1, ATTN_NORM, 0);
#else
//                              ent->s.event = EV_FALLSHORT;
#endif
                        }
                }
                return;
        }
}

/*
P_WorldEffects
*/
void P_WorldEffects (void)
{
        //qboolean      breather;
        //qboolean      envirosuit;
        int                     waterlevel, old_waterlevel;

        if (current_player->movetype == MOVETYPE_NOCLIP)
        {
                current_player->air_finished = level.time + 12; // don't need air
                return;
        }

        waterlevel = current_player->waterlevel;
        old_waterlevel = current_client->old_waterlevel;
        current_client->old_waterlevel = waterlevel;


//      breather = current_client->breather_framenum > level.framenum;
//      envirosuit = current_client->enviro_framenum > level.framenum;

        //
        // if just entered a water volume, play a sound
        //
        if (!old_waterlevel && waterlevel)
        {
//              PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
                if (current_player->watertype & CONTENTS_LAVA && current_player->client->resp.team == TEAM_HUMAN && (current_player->client->resp.class_type == CLASS_GRUNT || current_player->client->resp.class_type == CLASS_BIO ||current_player->client->resp.class_type == CLASS_SHOCK ||current_player->client->resp.class_type == CLASS_HEAVY ||current_player->client->resp.class_type == CLASS_COMMANDO))
                        gi.sound (current_player, CHAN_AUTO, SoundIndex (player_lava_in), 1, ATTN_NORM, 0);
                else if (current_player->watertype & CONTENTS_SLIME){
                        if (current_client->resp.team==TEAM_ALIEN)
                                gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_watr_in), 1, ATTN_NORM, 0);
                        else
                                gi.sound (current_player, CHAN_AUTO, SoundIndex (player_watr_in), 1, ATTN_NORM, 0);
                }else if (current_player->watertype & CONTENTS_WATER){
                        if (current_client->resp.team==TEAM_ALIEN)
                                gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_watr_in), 1, ATTN_NORM, 0);
                        else
                                gi.sound (current_player, CHAN_AUTO, SoundIndex (player_watr_in), 1, ATTN_NORM, 0);
                }
                current_player->flags |= FL_INWATER;

                // clear damage_debounce, so the pain sound will play immediately
                current_player->damage_debounce_time = level.time - 1;
        }

        //
        // if just completely exited a water volume, play a sound
        //
        if (old_waterlevel && ! waterlevel)
        {
//              PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
                if (current_client->resp.team==TEAM_ALIEN)
                        gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_watr_out), 1, ATTN_NORM, 0);
                else
                        gi.sound (current_player, CHAN_AUTO, SoundIndex (player_watr_out), 1, ATTN_NORM, 0);
                current_player->flags &= ~FL_INWATER;
        }

        //
        // check for head just going under water
        //
        if (old_waterlevel != 3 && waterlevel == 3)
        {
                if (current_client->resp.team==TEAM_ALIEN)
                        gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_watr_un), 1, ATTN_NORM, 0);
                else
                        gi.sound (current_player, CHAN_AUTO, SoundIndex (player_watr_un), 1, ATTN_NORM, 0);
        }

        //
        // check for head just coming out of water
        //
        if (old_waterlevel == 3 && waterlevel != 3)
        {
                if (current_player->air_finished < level.time)
                {       // gasp for air
                        if (current_player->client->resp.team==TEAM_ALIEN){
                                gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_gasp1), 1, ATTN_NORM, 0);
                        }else{
                                gi.sound (current_player, CHAN_AUTO, SoundIndex (player_gasp1), 1, ATTN_NORM, 0);
                        }
//                      PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
                }
                else    if (current_player->air_finished < level.time + 11)
                {       // just break surface
                        if (current_player->client->resp.team==TEAM_ALIEN){
                                gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_gasp2), 1, ATTN_NORM, 0);
                        }else{
                                gi.sound (current_player, CHAN_AUTO, SoundIndex (player_gasp2), 1, ATTN_NORM, 0);
                        }
                }
        }


        //
        // check for sizzle damage. If not fried first, then drown/short
        //
        if (waterlevel && (current_player->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) ) {
          if (waterlevel < 3) current_player->air_finished = level.time + 12;

                if (current_player->watertype & CONTENTS_LAVA)
                {
                        /*if (current_player->health > 0
                                && current_player->pain_debounce_time <= level.time
                                && current_player->client->invincible_framenum < level.framenum && current_player->client->resp.team == TEAM_HUMAN)
                        {
                                if (randomMT()&1)
                                        gi.sound (current_player, CHAN_AUTO, SoundIndex (player_burn1), 1, ATTN_NORM, 0);
                                else
                                        gi.sound (current_player, CHAN_AUTO, SoundIndex (player_burn2), 1, ATTN_NORM, 0);
                                current_player->pain_debounce_time = level.time + 1;
                        }*/

                        //if (envirosuit)       // take 1/3 damage with envirosuit
                        //      T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_LAVA);
                        //else
                        T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 9*waterlevel, 0, DAMAGE_IGNORE_RESISTANCES, MOD_LAVA);
                }

                if (current_player->watertype & CONTENTS_SLIME)
                {// Gloom
                        if (current_player->client->resp.team != TEAM_ALIEN)
                        {       // no damage from slime with envirosuit
                                T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3*waterlevel, 0, 0, MOD_SLIME);
                        }
                }
        }

        //
        // check for drowning
        //
        if (waterlevel == 3)
        {
                switch (current_player->client->resp.class_type){
                case CLASS_EXTERM:
                        T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3 /* 9 */, 0, DAMAGE_NO_ARMOR, MOD_SHORT);
                        return;
                case CLASS_MECH:
                        T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 18 /* 60 */, 0, DAMAGE_NO_ARMOR, MOD_SHORT);
                        return;
                case CLASS_ENGINEER:
                        T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 9 /* 3 */, 0, DAMAGE_NO_ARMOR, MOD_SHORT);
                        return;
                }

                // if out of air, start drowning
                if (current_player->air_finished < level.time)
                {       // drown!
                        if (current_player->client->next_drown_time < level.time
                                && current_player->health > 0)
                        {
                                int dmg = 4 + random() * 9;
                                current_player->client->next_drown_time = level.time + 1;

                                // play a gurp sound instead of a normal pain sound
                                if (current_player->health <= dmg){
                                        if (current_client->resp.team==TEAM_ALIEN)
                                                gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_drown1), 1, ATTN_NORM, 0);
                                        else
                                                gi.sound (current_player, CHAN_AUTO, SoundIndex (player_drown1), 1, ATTN_NORM, 0);
                                }else if (randomMT()&1){
                                        if (current_client->resp.team==TEAM_ALIEN)
                                                gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_gurp1), 1, ATTN_NORM, 0);
                                        else
                                                gi.sound (current_player, CHAN_AUTO, gi.soundindex("*gurp1.wav"), 1, ATTN_NORM, 0);
                                }else{
                                        if (current_client->resp.team==TEAM_ALIEN)
                                                gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_gurp2), 1, ATTN_NORM, 0);
                                        else
                                                gi.sound (current_player, CHAN_AUTO, gi.soundindex("*gurp2.wav"), 1, ATTN_NORM, 0);
                                }

                                current_player->pain_debounce_time = level.time;

                                T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR | DAMAGE_IGNORE_RESISTANCES, MOD_WATER);
                        }
                }
        }
        else if (waterlevel == 2)
        {
                current_player->air_finished = level.time + 12;
                switch (current_player->client->resp.class_type){
                        case CLASS_EXTERM:
                                T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3 /* 9 */, 0, DAMAGE_NO_ARMOR, MOD_SHORT);
                                return;
                        case CLASS_MECH:
                                T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 18 /* 60 */, 0, DAMAGE_NO_ARMOR, MOD_SHORT);
                                return;
                        case CLASS_ENGINEER:
                                T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 9 /* 3 */, 0, DAMAGE_NO_ARMOR, MOD_SHORT);
                                return;
                }
        }
        else
        {
                current_player->air_finished = level.time + 12;
        }
}

void UpdateStructurePreview(edict_t *ent)
{
    edict_t*pe = ent->client->preview_ent;

    trace_t tr, tr2;
    vec3_t forward, angles, drop, start;

    if (ent->client->menu.entries != engineer_menu) {
        ViewStructurePreview(ent, 0);
        return;
    }

    VectorCopy (ent->s.origin, start);

    if (pe->count == ENT_TRIPWIRE_BOMB) {
        VectorCopy (ent->client->v_angle, angles);
        start[2] += ent->viewheight;
    }
    else {
        angles[0] = 0;
        angles[1] = ent->client->v_angle[1];
        angles[2] = 0;
    }

    AngleVectors (angles, forward, NULL, NULL);
    VectorMA(start, pe->dmg_radius, forward, forward);

    
    if (pe->count == ENT_TELEPORTER_D) 
        pe->s.angles[1] = ent->s.angles[1] + 180;
    else
        pe->s.angles[1] = ent->s.angles[1];



    if (pe->count == ENT_TRIPWIRE_BOMB) {

        if (!(gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD))
            tr = gi.trace (start, NULL, NULL, forward, ent, MASK_SOLID);
        else
            tr.fraction = 1;

        if (tr.fraction < 1.0 && !tr.startsolid && !(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
        {
            pe->s.effects &= ~EF_COLOR_SHELL;
            pe->s.renderfx &= ~RF_SHELL_RED;
            vectoangles(tr.plane.normal, pe->s.angles);
        }
        else {
            pe->s.effects |= EF_COLOR_SHELL;
            pe->s.renderfx |= RF_SHELL_RED;
        }
        VectorCopy(tr.endpos, pe->s.origin);
    }

    else {
        VectorCopy(forward, drop);
        drop[2] -= 1024;

        tr = gi.trace(forward, pe->mins, pe->maxs, drop, pe, MASK_MONSTERSOLID);
        if (tr.startsolid || gi.pointcontents(tr.endpos) & CONTENTS_NOBUILD) {
            if (pe->count != ENT_DETECTOR) {
                pe->s.effects |= EF_COLOR_SHELL;
                pe->s.renderfx |= RF_SHELL_RED;
                VectorCopy(forward, pe->s.origin);
            }
            else {
                tr2 = gi.trace (start, pe->mins, pe->maxs, forward, ent, MASK_SOLID);
                if (tr2.fraction < 1.0)
                VectorCopy(tr2.endpos, pe->s.origin);
            }
        }
        else {  
                pe->s.effects &= ~EF_COLOR_SHELL;
                pe->s.renderfx &= ~RF_SHELL_RED;
                VectorCopy(tr.endpos, pe->s.origin);
         }
    }

  
    gi.linkentity(pe);

    if (pe->target_ent) {
        pe->target_ent->s.effects = pe->s.effects;
        pe->target_ent->s.renderfx = pe->s.renderfx;
        VectorCopy(pe->s.origin, pe->target_ent->s.origin);
        gi.linkentity(pe->target_ent);
    }
}

void ViewStructurePreview (edict_t*ent, int selected)
{
    edict_t*pe, *be;

    if (ent->client->preview_ent) {
        if (ent->client->preview_ent->target_ent)
            G_FreeEdict(ent->client->preview_ent->target_ent);

        G_FreeEdict(ent->client->preview_ent);

        ent->client->preview_ent = NULL;
    }

    if (!selected) return;
    
    ent->client->preview_ent = pe = G_Spawn();
    pe->classname = "preview";

    pe->owner = ent;
    pe->svflags = SVF_SEND_OWNER;
    pe->s.effects = EF_SPHERETRANS;
    pe->s.renderfx = RF_FULLBRIGHT;
    ext.SetEntityMask(pe->s.number, 16 << ent->client->resp.team);

    switch (selected) {
                case 2:
                    //Teleport
                    pe->count = ENT_TELEPORTER_D;
                    pe->s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
                    VectorSet (pe->mins, -24, -24, -24);
                    VectorSet (pe->maxs, 24, 24, -8);
                    pe->dmg_radius = 70;

                    break;
                case 3: // Turret
                case 4: //MG turret
                    pe->count = ENT_TURRET;
                    pe->dmg_radius = 128;

                    if (selected == 3) {
                        pe->s.modelindex = gi.modelindex("models/turret/gun.md2");
                        pe->s.frame = 4;
                    } else
                        pe->s.modelindex = gi.modelindex("models/turret/mgun.md2");

                    be = G_Spawn();
                    be->classname = "preview_base";

                    be->s.modelindex = gi.modelindex("models/turret/base.md2");
                    pe->target_ent = be;
                    be->owner = ent;
                    be->svflags = SVF_SEND_OWNER;
                    be->s.effects = EF_SPHERETRANS;
                    ext.SetEntityMask(be->s.number, 16 << ent->client->resp.team);
                    VectorSet (pe->mins, -24, -24, -13);
                    VectorSet (pe->maxs, 24, 24, 24);
                   break;
                case 5:
                    // Detector
                    pe->count = ENT_DETECTOR;
                    pe->dmg_radius = 60;
                    pe->s.modelindex = gi.modelindex("models/objects/detector/tris.md2");
                    pe->s.effects &= ~EF_SPHERETRANS;   // detector is hardly visible with this
                    pe->s.renderfx |= RF_TRANSLUCENT;
                    VectorSet (pe->mins, -8, -8, 0);
                    VectorSet (pe->maxs, 8, 8, 8);
                    break;
                case 6:
                   // Mine
                    pe->count = ENT_TRIPWIRE_BOMB;
                    pe->dmg_radius = 74;
                    pe->s.modelindex = gi.modelindex("models/objects/tripwire/tris.md2");
                    break;
                case 7:
                    //Depot
                    pe->count = ENT_AMMO_DEPOT;
                    pe->dmg_radius = 50;
                    pe->s.modelindex = gi.modelindex("models/objects/depot/tris.md2");
                    VectorSet (pe->mins, -16, -16, -24);
                    VectorSet (pe->maxs, 16, 16, 0);
            }

    UpdateStructurePreview(ent);
}

/*
G_SetClientEffects
*/
void G_SetClientEffects (edict_t *ent)
{
        int             pa_type;
        int             remaining;

        ent->s.effects = 0;
        ent->s.renderfx = 0;

        if (!ent->client)
                return;

        if (level.intermissiontime && (level.intermissiontime < level.framenum))
                return;

        if (!ent->deadflag && ent->health > 0) {
                ent->s.renderfx |= RF_IR_VISIBLE;
        } else {
                //r1: still in IR when playing death anims
                if (ent->s.frame != ent->client->anim_end) {
                        ent->s.renderfx |= RF_IR_VISIBLE;
                }
                return;
        }

        if (ent->client->resp.flying && ent->client->resp.class_type== CLASS_EXTERM)
        {
                if (ent->client->resp.upgrades & UPGRADE_CHARON_BOOSTER)
                        ent->s.effects |= EF_GRENADE;
                ent->s.effects |= EF_ROCKET;
        }

        if (ent->client->powerarmor_time > level.time)
        {
                pa_type = PowerArmorType (ent);
                if (pa_type == POWER_ARMOR_SCREEN)
                {
                        ent->s.effects |= EF_POWERSCREEN;
                }
                else if (pa_type == POWER_ARMOR_SHIELD)
                {
                        ent->s.effects |= EF_COLOR_SHELL;
                        ent->s.renderfx |= RF_SHELL_GREEN;
                }
        }

//        if (ent->client->resp.parasited)
//                ent->s.effects |= EF_FLIES;

        if (ent->client->resp.class_type == CLASS_HATCHLING && (ent->client->resp.upgrades & UPGRADE_TRANS_MEMBRANE))
                ent->s.effects |= EF_SPHERETRANS;

/*      if (ent->client->quad_framenum > level.framenum)
        {
                remaining = ent->client->quad_framenum - level.framenum;
                if (remaining > 30 || (remaining & 4) )
                        ent->s.effects |= EF_QUAD;
        }*/

        if (ent->client->invincible_framenum > level.framenum && ent->client->resp.team==TEAM_HUMAN){
                remaining = ent->client->invincible_framenum - level.framenum;
                if (remaining > 30 || (remaining & 4) )
                        ent->s.effects |= EF_HALF_DAMAGE;
        }

        // show cheaters!!!
        if (ent->flags & FL_GODMODE)
        {
                ent->s.effects |= EF_COLOR_SHELL;
                ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
        }

        if (ent->client->resp.team==TEAM_ALIEN){
                /*edict_t *e=NULL;
                // FIXME: findrad = slow = moved to detector think, increased detector think speed... small comrpromise
                while ((e=findradius(e,ent->s.origin,350))){
                        if (e->enttype == ENT_DETECTOR)
                                ent->client->glow_time = level.time + 1;
                }*/

                if (ent->client->grenade_blow_up > 0)
                {
                        if (ent->client->grenade_blow_up == 1)
                                ent->s.effects |= EF_FLAG1;
                        else
                                ent->s.effects |= EF_FLAG2;
                }

                if (ent->client->glow_time >= level.time)
                {
                        if (ent->client->resp.class_type == CLASS_KAMIKAZE)
                        {
                                ent->s.effects |= EF_COLOR_SHELL;
                                ent->s.renderfx |= RF_SHELL_RED;
                        }
                        else if (ent->client->resp.class_type == CLASS_HATCHLING && (ent->client->resp.upgrades & UPGRADE_TRANS_MEMBRANE))
                        {
                                //no effect!
                        }
                        else
                        {
                                ent->s.effects |= EF_DOUBLE;
                        }
                }
        }

        // guardian cloaks instantly and commando goes stealth after a while when still
        if (xyspeed > 1.0f || !ent->groundentity)
        {
                if (ent->client->resp.class_type == CLASS_GUARDIAN)
                {
                        if (!((ent->client->resp.upgrades & UPGRADE_CELL_WALL) || xyspeed < 150) || ent->client->grenade_blow_up > 0 || ent->client->glow_time >= level.time) {
                                ent->s.modelindex = 255;
                        }
                }
                else if (ent->client->resp.class_type == CLASS_COMMANDO)
                {
                        if (!(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
                                ent->fly_sound_debounce_time = level.framenum + 5;
                        else
                                xyspeed = 0;
                                //ent->s.renderfx &= ~RF_IR_VISIBLE;
                }
        }

        if (!(xyspeed > 1.0f || !ent->groundentity)) {
                if (ent->client->resp.class_type == CLASS_GUARDIAN) {
                        if (ent->client->grenade_blow_up > 0 || ent->client->glow_time >= level.time) {
                                ent->s.modelindex = 255;

                       }
                       else {
                                if (ent->s.modelindex && level.framenum > ent->fly_sound_debounce_time) {
                                        ent->s.modelindex = 0;
                                        if (g_guardian_regen_mode->value == 2) GuardianRegen(ent);

                                        if (sv_extended && !ent->client->mirror_ent && ent->s.modelindex == 0) {
                                            GuardianMirrorSpawn(ent);
                                        }

                                } else if (ent->s.modelindex == 0) {
                                        //r1: set effects so the server still sends position info to the client
                                        //(avoids new ent spikes on uncloaking, allows guardian "glow" to be 100% accurate)
                                        ent->s.effects |= EF_SPHERETRANS;
                                        if (g_guardian_regen_mode->value == 2) GuardianRegen(ent);

                                }
                        }
                } else if (ent->client->resp.class_type == CLASS_COMMANDO && (level.framenum > ent->fly_sound_debounce_time) && !ent->client->resp.primed) {
                        ent->s.renderfx &= ~RF_IR_VISIBLE;
                        //ent->s.renderfx |= RF_TRANSLUCENT;
                }
        }
        else
        {
                if (ent->client->resp.class_type == CLASS_GUARDIAN && ((ent->client->resp.upgrades & UPGRADE_CELL_WALL) || xyspeed < 150))
                {
                        if (ent->s.modelindex == 0)
                        {
                                ent->client->last_reload_time = level.time + 5;
                                ent->s.effects |= EF_SPHERETRANS;
                        }
                        if (sv_extended && !ent->client->mirror_ent && ent->s.modelindex == 0) {
                                GuardianMirrorSpawn(ent);
                        }
                }
        }
}

/*
G_SetClientEvent
*/
void G_SetClientEvent (edict_t *ent)
{
        if (ent->s.event)
                return;

        if (classlist[ent->client->resp.class_type].footsteps)
        if ( ent->groundentity && xyspeed > 225)
        {
                if ( (int)(ent->client->bobtime+bobmove) != bobcycle )
                {
                        if(ent->client->resp.class_type == CLASS_EXTERM)
                        {
                                gi.sound (ent, CHAN_AUTO, SoundIndex (exterm_step), 1, ATTN_NORM, 0);
//                              ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
                        }
                        else
                                ent->s.event = EV_FOOTSTEP;
                }
        }
}

/*
G_SetClientSound
*/
void G_SetClientSound (edict_t *ent)
{
        if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) && (ent->client->resp.team != TEAM_ALIEN))
                ent->s.sound = SoundIndex (player_fry);
        else if(ent->client->grenade_blow_up > 0)
                ent->s.sound = SoundIndex (weapons_tick2);
        else if (ent->client->resp.flying && ent->client->resp.class_type==CLASS_WRAITH)
                ent->s.sound = SoundIndex (wraith_fly);
        else if (ent->client->resp.class_type==CLASS_EXTERM) {
          ent->s.sound = 0;
          if (ent->client->resp.flying && ((level.framenum-ent->client->resp.flying)&3) == 1) {
            if (randomMT()&3 == 3) ent->client->resp.flying += 3; //randomize a bit the loop
            gi.sound (ent, CHAN_ITEM, SoundIndex(weapons_flame), 1, ATTN_NORM, 0);
          }
        } else if (ent->client->weapon_sound)
                ent->s.sound = ent->client->weapon_sound;
        else if (ent->random && ent->client->resp.class_type==CLASS_EXTERM)
          ent->s.sound = SoundIndex(weapons_rg_hum);
        else
          ent->s.sound = 0;

}

/*
G_SetClientFrame
*/
void G_SetClientFrame (edict_t *ent)
{
        gclient_t       *client;
        qboolean        duck, run, fly;

        if (ent->s.modelindex != 255 && (ent->client->resp.class_type != CLASS_GUARDIAN && ent->health > 0))
                return;         // not in the player model

        client = ent->client;

        if (client->ps.pmove.pm_flags & PMF_DUCKED)     {
                if (!classlist[client->resp.class_type].fixview)
                        duck = true;
                else
                        duck = false;
        }else
                duck = false;

        if (xyspeed > 1.0){
                run = true;
        }else{
                run = false;
        }
        if (client->resp.flying && client->resp.class_type==CLASS_WRAITH)
                fly=true;
        else
                fly=false;

        // check for stand/duck and stop/go transitions
        if (fly != client->anim_fly && client->anim_priority <= ANIM_JUMP)
                goto newanim;
        if (duck != client->anim_duck && client->anim_priority < ANIM_DEATH)
                goto newanim;
        if (run != client->anim_run && client->anim_priority == ANIM_BASIC)
                goto newanim;
        if (!ent->groundentity && client->anim_priority <= ANIM_WAVE)
                goto newanim;

        if (ent->s.frame < client->anim_end) {  // continue an animation
                ent->s.frame++;

                if (client->anim_priority == ANIM_DEATH){
                        // Evil hard coding:
                        if(ent->client->resp.class_type == CLASS_ENGINEER)
                                EngineerDie(ent);
                        // FIXME: breederdie was broken, fixed, and removed :)
                        if(ent->client->resp.class_type == CLASS_BREEDER)
                                BreederDie(ent);
                }

                // Play mech walk sounds        -- Should be in G_SetClientEvent
                if(ent->client->resp.class_type == CLASS_MECH){
                        if(ent->s.frame == MECH_WALK_S + 2 || ent->s.frame == MECH_WALK_S + 8 ||
                           ent->s.frame == MECH_WALKSHOOT_S+2 || ent->s.frame == MECH_WALKSHOOT_S+8)
                        {
#ifdef CACHE_CLIENTSOUNDS
                                gi.sound(ent, CHAN_AUTO, SoundIndex(mech_step), 1, ATTN_NORM, 0);
#else
                                gi.sound(ent, CHAN_AUTO, gi.soundindex("*mechwalk.wav"), 1, ATTN_NORM, 0);
#endif
                        }
                }
                return;
        }

        // don't let death to new anim part
        if (client->anim_priority == ANIM_DEATH){
                return;
        }

        if (client->anim_priority == ANIM_JUMP && !fly){
                if (!ent->groundentity)
                        return;         // stay there
                ent->client->anim_priority = ANIM_WAVE;

                if(ent->client->resp.class_type == CLASS_STALKER)
                {
                        ent->s.frame = STALK_LAND_S;
                        ent->client->anim_end = STALK_LAND_E;
//              }
//              else if(ent->client->resp.class_type == CLASS_DRONE){
//                      ent->s.frame = FrameReference(ent, FRAME_JUMP_S)+4;
//                      ent->client->anim_end = FrameReference(ent, FRAME_JUMP_E);
                } else
                {
                        //ent->s.frame = FrameReference(ent, FRAME_JUMP_S)+2;
                        ent->s.frame++;
                        ent->client->anim_end = FrameReference(ent, FRAME_JUMP_E);
                }
                return;
        }

newanim:

        // return to either a running or standing frame
        client->anim_priority = ANIM_BASIC;
        client->anim_duck = duck;
        client->anim_run = run;

        if (fly){

                // new fly
                ent->s.frame = FrameReference(ent, FRAME_FLY_S);
                client->anim_end = FrameReference(ent, FRAME_FLY_E);
                client->anim_priority = ANIM_JUMP;

                client->anim_fly = fly; // fly state on

        } else if (!fly && client->anim_fly) {

                // end fly
                ent->s.frame = FrameReference(ent, FRAME_FLY_FALL_S);
                client->anim_end = FrameReference(ent, FRAME_FLY_FALL_E);
                client->anim_priority = ANIM_JUMP;

                client->anim_fly = fly; // fly state off

        } else if(!ent->groundentity){
                if (ent->client->resp.turret) {
                        ent->s.frame = FrameReference(ent, FRAME_STAND_S);
                        client->anim_end = FrameReference(ent, FRAME_STAND_E);
                        return;
                } else {
                        client->anim_priority = ANIM_JUMP;
                }

                // drone anim fixed in frame defines/m_drone.h
                //if(ent->client->resp.class_type == CLASS_DRONE){

                //      ent->s.frame = FrameReference(ent, FRAME_JUMP_S)+2; // +3
                //      ent->client->anim_end = FrameReference(ent, FRAME_JUMP_S)+3;


                if(ent->client->resp.class_type == CLASS_STALKER) {

                        ent->s.frame = FrameReference(ent, FRAME_JUMP_S)+4;
                        ent->client->anim_end = FrameReference(ent, FRAME_JUMP_S)+4;

                }else{
                        if (ent->s.frame != FrameReference(ent, FRAME_JUMP_S)+1)
                                ent->s.frame = FrameReference(ent, FRAME_JUMP_S);

                        client->anim_end = FrameReference(ent, FRAME_JUMP_S)+1;
                }

        } else if (run){
                if (duck){
                        ent->s.frame = FrameReference(ent, FRAME_DUCKRUN_S);
                        client->anim_end = FrameReference(ent, FRAME_DUCKRUN_E);
                }else{
                        ent->s.frame = FrameReference(ent, FRAME_RUN_S);
                        client->anim_end = FrameReference(ent, FRAME_RUN_E);
                }
        }else if (duck){
                ent->s.frame = FrameReference(ent, FRAME_DUCK_S);
                client->anim_end = FrameReference(ent, FRAME_DUCK_E);
        }else{ // standing
                ent->s.frame = FrameReference(ent, FRAME_STAND_S);
                client->anim_end = FrameReference(ent, FRAME_STAND_E);
        }

}

/*
 * ClientEndServerFrames
 */
void ClientEndServerFrames (void)
{
        edict_t *ent;
        int i;

        //r1: replenishing build points hack (wrong place i know but needed to be called before stats is populated)
        for (i = 0; i < MAXTEAMS; i++)
        {
                if (!team_info.buildpool[i])
                        continue;

                if (replenishbp->value && i == TEAM_ALIEN)
                {
                        if (team_info.buildtime[i] < level.framenum)
                        {
                                team_info.points[i] -= replenishbp_amt->value;
                                team_info.buildpool[i] -= replenishbp_amt->value;
                                team_info.buildtime[i] = level.framenum + replenishbp_tick->value;
                        }
                }
                else
                {
                        team_info.points[i] -= team_info.buildpool[i];
                        team_info.buildpool[i] = 0;
                }
        }

        // calc the player views now that all pushing
        // and damage has been added
        ent = g_edicts+1;
        do
        {
                if (ent->inuse)
                        ClientEndServerFrame (ent);
                ent++;
        }
        while (ent->client);
}

void ClientThinkEndServerFrame(edict_t*ent); //p_client.c


static float clampr(float n, float r)
{
  if (n < -r) return -r;
  if (n > r) return r;
  return n;
}

/*
 * ClientEndServerFrame
 *
 * Called for each player at the end of the server frame
 * and right after spawning
 */
void DeathmatchScoreboard (edict_t *ent);

static void GetIntervalVelocity(edict_t *ent, vec3_t outvel, vec3_t outoldvel)
{
  if (!ent->client) {
    if (outvel) VectorCopy(ent->velocity, outvel);
    if (outoldvel) VectorCopy(ent->velocity, outoldvel);
    return;
  }

  if (ent->client->resp.pmove_time) {
    vec3_t delta;
    float  dftime = 1000.0f/ent->client->resp.pmove_time;

    ent->client->resp.pmove_time = 0;

    VectorSubtract(ent->s.origin, ent->client->resp.pm_oldpos, delta);
    VectorCopy(ent->s.origin, ent->client->resp.pm_oldpos);

    VectorCopy(ent->client->resp.pm_lastvel, ent->client->resp.pm_oldvel);

    VectorScale(delta, dftime, ent->client->resp.pm_lastvel);
  }
  
  if (outvel) VectorCopy(ent->client->resp.pm_lastvel, outvel);
  if (outoldvel) VectorCopy(ent->client->resp.pm_oldvel, outoldvel);
}


enum {
  P_Mow    = 1,
  P_Squish = 2,
  P_Rigid  = 4,		//Rigid armour takes the above damages
  P_Bounce = 8,		//Boucing  aliens (that never crater when falling) do less velocity squish damage
  P_Squish_Hack = 16	//Guard squished by just jumping
};


static uint8_t class_pflags[] = {
  /*CLASS_GRUNT 	*/ 0,
  /*CLASS_HATCHLING 	*/ P_Bounce,
  /*CLASS_HEAVY 	*/ 0,
  /*CLASS_COMMANDO 	*/ 0,
  /*CLASS_DRONE 	*/ P_Bounce,
  /*CLASS_MECH  	*/ P_Squish | P_Rigid,
  /*CLASS_SHOCK 	*/ 0,
  /*CLASS_STALKER 	*/ P_Squish | P_Mow | P_Rigid,
  /*CLASS_BREEDER 	*/ P_Squish,
  /*CLASS_ENGINEER 	*/ P_Rigid,
  /*CLASS_GUARDIAN 	*/ P_Squish | P_Mow | P_Rigid | P_Squish_Hack,
  /*CLASS_KAMIKAZE 	*/ P_Rigid,
  /*CLASS_EXTERM 	*/ P_Squish | P_Rigid ,
  /*CLASS_STINGER 	*/ P_Squish | P_Mow | P_Bounce,
  /*CLASS_WRAITH 	*/ P_Bounce,
  /*CLASS_BIO   	*/ 0,
  /*CLASS_OBSERVER 	*/ 0
};


static void PushAndMow(edict_t *ent)
{
  static DEBUGITEM(squish);

  vec3_t vel;
  uint_least8_t pf = class_pflags[ent->client->resp.class_type];

/*  if (!pf) {
    ent->client->resp.pm_squish = NULL;
    return;
  }*/

  GetIntervalVelocity(ent, NULL, vel);

  if (ent->client->resp.pm_squish) {
    edict_t*bug = ent->client->resp.pm_squish;
    ent->client->resp.pm_squish = NULL;

    if (bug->solid && bug->takedamage && (sv_gravity->value > 1)) { //only if still damageable and can squish it
      damage_t dmg;
      float gb = bug->client?1:0;				//Structures are can hold standing weights
      float vdmg = 0.5 - (vel[2]*2+250) / /*sv_gravity->value*/ 800;
      if (vdmg < 0.5) vdmg = 0.5;
      if (vdmg > 10.5) vdmg = 10.5;

      if (pf & P_Squish_Hack && !(ent->spawnflags & SPAWNFLAG_SQUISH_HACK))
        pf &= ~P_Squish;

      if (!(pf & P_Squish)) {
        dmg.scale = gb+vdmg*0.75-1.5;	//Non squishers need to fall from high place to squish
      } else
      if (!(pf & P_Bounce))
        dmg.scale = gb+(vdmg*vdmg-0.25); //Gravity base damage + squared velocity
      else
        dmg.scale = gb+(vdmg-0.5); //Gravity base damage + simple velocity, since alien bounces without taking damage means dissipates the impact

      if (dmg.scale > 0) {
        dmg.targ = bug;
        dmg.inflictor = dmg.attacker = ent;
        VectorSet(dmg.dir, 0, 0, -1);
        VectorCopy(ent->s.origin, dmg.point); dmg.point[2] += ent->mins[2];
        VectorSet(dmg.normal, 0, 0, 1);
        dmg.damage = 10+15*classlist[ent->client->resp.class_type].frags_needed; 	//Intended to be overriden always
        if (ent->client->resp.class_type == CLASS_EXTERM) dmg.damage = 32;		//Only for reference
        dmg.distance = dmg.radius = -1;
        dmg.knockback = 0;
        dmg.dflags = DAMAGE_NO_KNOCKBACK;
        if (bug->client && !(class_pflags[bug->client->resp.class_type] & P_Rigid))
          dmg.dflags |= DAMAGE_NO_ARMOR;
        dmg.mod = MOD_NEW_SQUISH;
        DAMAGE_FIELDS(13);


        if (Debug(squish))
          gi.cprintf(ent, PRINT_HIGH, "Squishing a %s: dmg=%.0f, g+v=%.3f\n", bug->classname, dmg.scale*dmg.damage, dmg.scale);

        if (Damage(&dmg)) {
          ent->client->resp.squish_count += dmg.scale*dmg.damage;
          if (ent->client->resp.squish_debounce < level.framenum || ent->client->resp.squish_count >= 25) {
            ent->client->resp.squish_count = 0;
            ent->client->resp.squish_debounce = level.framenum + 10;
            gi.sound(ent, CHAN_AUTO, SoundIndex(mutant_mutatck2), 1, ATTN_NORM, 0);
          }
        }
      }
    }
  }

  //TODO: Mowing should go into pmove so can push without losing all speed

}

void ClientEndServerFrame (edict_t *ent)
{
	qboolean gloom_pmove = ((int)dmflags->value & DF_GLOOMPMOVE) != 0;
        float   bobtime;
        int             i;
        float   buildtimer = ent->client->build_timer - level.time;
        vec3_t distance;

        if (ent->client->last_ucmd_frame == level.framenum-1)
	  ClientThinkEndServerFrame(ent);


        if (buildtimer < 0) buildtimer = 0;

        current_player = ent;
        current_client = ent->client;

        /*
         * If the origin or velocity have changed since ClientThink(),
         * update the pmove values.     This will happen when the client
         * is pushed by a bmodel or kicked by an explosion.
         *
         * If it wasn't updated here, the view position would lag a frame
         * behind the body position when pushed -- "sinking into plats"
         */
         
        if (!ent->client->pers.badname) {
          for (i=0 ; i<3 ; i++) {
            current_client->ps.pmove.origin[i] = ent->s.origin[i]*8;
            current_client->ps.pmove.velocity[i] = ent->velocity[i]*8;
          }
        } else {
          current_client->ps.pmove.origin[0] = current_client->ps.pmove.origin[1] = current_client->ps.pmove.origin[2] = 32767;  //move outside map so can't hear sounds either
          VectorClear(current_client->ps.pmove.velocity);
          
          if (g_enforce_names->value >= 2 && ent->random < level.time) {
#define numrndsounds 63
            static uint8_t sounds[numrndsounds] = {
              items_s_health, misc_keytry, weapons_noammo, weapons_machgf1b,  misc_fhit3, misc_activate,
              weapons_acid, weapons_pistgf1, weapons_gg_on, weapons_subfire, mutant_mutatck1, cocoon_drud1,
              mutant_mutatck2, player_watr_out, switches_butn2, misc_spawn1, alien_watr_in, world_electro, weapons_grenlb1b,
              weapons_flame,  weapons_railgf1a, misc_bdeath, tank_thud,
              organ_organe2, organ_organe3, organ_organe4, alien_watr_out, alien_watr_in, misc_h2ohit1, turret_Tlasexplode,
              alien_watr_un,  world_spark1, weapons_hgrent1a, world_spark3, world_ric1, player_watr_un, misc_udeath,
              weapons_hgrenb1a, weapons_pulsfire, misc_ssdie, items_l_health, weapons_magshot,
              misc_ar1_pkup, organ_twang, weapons_misload, organ_growegg2, organ_growegg,
              world_stp_watr, weapons_hgrenb2a, weapons_tick2, weapons_rocklx1a, weapons_davelas,
              weapons_refill, detector_alarm1, detector_alarm2, detector_alarm3, player_burn1,
              player_burn2, weapons_webshot1, weapons_webshot2,  player_lava_in, weapons_shotweb1, organ_healer1,
            };

            int s = (uint16_t)(randomMT()>>20)%numrndsounds;
            unicastSound(ent, SoundIndex(sounds[s]), 0.75+0.25*crandom());
            if (randomMT()&7 == 7)
              ent->random = level.time+0.09f;
            else
              ent->random = level.time+1+0.8*crandom();
#undef numsounds
          }
        }

        VectorSubtract(ent->s.origin, ent->client->oldorigin, distance);
        VectorCopy(ent->s.origin, ent->client->oldorigin);
        if (ent->s.event <= EV_FALLFAR && VectorLength(distance) < 0.255) {
          if (current_client->nomoveframes < 255)
            current_client->nomoveframes++;
        } else
          current_client->nomoveframes = 0;
          
       
        if (ent->client->teamoverlaytime && level.time > ent->client->teamoverlaytime) {
          ent->client->teamoverlayindex = 0;
          ent->client->teamoverlaytime = 0;
        }
        
        if (ent->client->resp.fire_points > 1) {
          ent->client->resp.fire_points -= 1;
          if (ent->client->resp.in_flames) {
            vec3_t upwards = {crandom()*0.2f, crandom()*0.2f, 1.2f};
            vec3_t origin = {
              ent->s.origin[0]-clampr(ent->velocity[0]*0.25f, 32)+crandom()*8,
              ent->s.origin[1]-clampr(ent->velocity[1]*0.25f, 32)+crandom()*8,
              ent->s.origin[2]-clampr(ent->velocity[2]*0.25f, 32)+crandom()*8
            };
            edict_t*fire = fire_fire(g_edicts + ent->client->resp.fire_attacker, origin, upwards, 10);
            if (fire) {
              fire->gib1 = MOD_BURNING;
              fire->newcount = 4;
              VectorScale(fire->velocity, 0.1f, fire->velocity);
            }
          }

        } else
          ent->client->resp.fire_points = 0;

        /*
         * If the end of unit layout is displayed, don't give
         * the player any normal movement attributes
         */
        if (level.intermissiontime && (level.intermissiontime < level.framenum))
        {
                // FIXME: add view drifting here?
                /*current_client->ps.blend[3] = 0;
                current_client->ps.fov = 90;
                VectorCopy (level.intermission_origin, ent->s.origin);
                VectorCopy (level.intermission_angle, current_client->ps.viewangles);*/
                return;
        }

        AngleVectors (ent->client->v_angle, forward, right, up);

	 if (ent->health > 0)
	  PushAndMow(ent);
	 else
	  ent->client->resp.pm_squish = NULL;

        // burn from lava, etc
        P_WorldEffects ();

	if (ent->client->resp.parasited && ent->health > 0 && ((level.framenum%10) == (ent-g_edicts)%10)) {
	  edict_t*attacker = g_edicts+ent->client->resp.parasited;
	  T_Damage(ent, attacker, attacker, vec3_origin, ent->s.origin, vec3_origin, 1, 0, DAMAGE_NO_ARMOR, MOD_PARASITE);
	}

        /*
         * set model angles from view angles so other things in
         * the world can tell which direction you are looking
         */
        if (ent->client->v_angle[PITCH] > 180)
                ent->s.angles[PITCH] = (-360 + ent->client->v_angle[PITCH])/3;
        else
                ent->s.angles[PITCH] = ent->client->v_angle[PITCH]/3;
        ent->s.angles[YAW] = ent->client->v_angle[YAW];
        ent->s.angles[ROLL] = 0;
        ent->s.angles[ROLL] = SV_CalcRoll (ent->s.angles, ent->velocity)*4;

        // calculate speed and cycle to be used for all cyclic walking effects
        xyspeed = sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);
        
        if (ent->client->pers.idletime*FRAMETIME < 5+buildtimer || xyspeed > 1)
          ent->client->pers.builderframes++;

        for (i = 0; i < 4;i++) {
                ent->client->lastspeeds[i] = ent->client->lastspeeds[i+1];
        }
        ent->client->lastspeeds[4] = xyspeed;

        if (xyspeed < 5)
        {
                bobmove = 0;
                current_client->bobtime = 0;    // start at beginning of cycle again
        }
        else if (ent->groundentity)
        {       // so bobbing only cycles when on ground
                if (xyspeed > 210)
                        bobmove = 0.25;
                else if (xyspeed > 100)
                bobmove = 0.125;
                else
                        bobmove = 0.0625;
        }

        bobtime = (current_client->bobtime += bobmove);

        if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
                bobtime *= 4;

        bobcycle = (int)bobtime;
        bobfracsin = fabs(sin(bobtime*M_PI));

        // detect hitting the floor
        P_FallingDamage (ent);

        // apply all the damage taken this frame
        P_DamageFeedback (ent);

        // determine the view offsets
        if (!ent->client->chase_target)
                SV_CalcViewOffset (ent);

        // determine the gun offsets
        SV_CalcGunOffset (ent);

        /* determine the full screen color blend
         * must be after viewoffset, so eye contents can be accurately determined
         * FIXME: with client prediction, the contents should be determined by the client
         */
        SV_CalcBlend (ent);
        if (ent->flags & FL_DEAD_OBSERVER)
          SV_AddBlend (0.35f, 0.1f, 0.1f, 0.55f, ent->client->ps.blend);


        G_SetStats (ent);

        G_SetClientEvent (ent);

        G_SetClientEffects (ent);

        G_SetClientSound (ent);

        G_SetClientFrame (ent);

        fun_postclient(ent);
        
#ifdef R1Q2_BUILD
        if (basetest->value) {
                VectorCopy (ent->mins, ent->client->ps.mins);
                VectorCopy (ent->maxs, ent->client->ps.maxs);
        }
#endif

        VectorCopy (ent->velocity, ent->client->oldvelocity);
        VectorCopy (ent->client->ps.viewangles, ent->client->oldviewangles);

        // clear weapon kicks
        VectorClear (ent->client->kick_origin);
        VectorClear (ent->client->kick_angles);

        //ent->client->kick_angles[0] = 100;

//        if (ent->client->resp.class_type == CLASS_EXTERM)
//                ent->client->resp.flying = false;

        // if the scoreboard is up, update it
        if (level.framenum % 50 == 2*((ent-g_edicts)&15) && ent->client->showscores)  //tachikoma: not all players at the same time, but in turns
        {
                // if it was opened on "update" frame, don't reupdate (and cause overflow)
                if (!ent->client->menu.entries && ent->client->showscores < level.framenum)
                        DeathmatchScoreboard (ent);
                //} else if (ent->client->menu.entries == team_menu) {
                //      PMenu_Update(ent);
                //}

        }

        ent->client->resp.lasttkcounter = ent->client->resp.midtkcounter;
        ent->client->resp.midtkcounter = ent->client->resp.teamkills;


        if (gloom_pmove && ent->client->gpms.grapple_state &&
            (ent->client->gpms.grapple_state < 3 || Distance(ent->s.origin, ent->client->gpms.grapple_point) > 128 || Distance(ent->s.origin, ent->client->gpms.grapple_hold) > 0.125f)
           ) { //draw pmove hatchling/kamikaze grapple
          //Works both as a way to send grapple data to predicting clients, and as fallback display for non predicting clients
          gi.WriteByte (svc_temp_entity);
          gi.WriteByte(TE_MEDIC_CABLE_ATTACK);
          gi.WriteShort(ent - g_edicts);
          gi.WritePosition(ent->s.origin);
          gi.WritePosition(ent->client->gpms.grapple_point);
          gi.unicast(ent, false);
          VectorCopy(ent->s.origin, ent->client->gpms.grapple_hold);
        }

        ent->client->svframeflags = 0; //prepare for next frame
}


