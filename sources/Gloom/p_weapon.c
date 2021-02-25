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

#include "g_local.h"
#include "m_player.h"
#include "m_drone.h"
#include "m_mech.h"
#include "m_stalker.h"
#include "m_guardian.h"
#include "m_stinger.h"
#include "m_wraith.h"
#include "m_engineer.h"

void Auto_Reload(edict_t *ent);
void fire_explosive (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod);
void fire_acid (edict_t *self, vec3_t start, vec3_t aimdir, int speed);
void kamikaze_explode(edict_t *self, int mod, int dflags);
void NoAmmoWeaponChange (edict_t *ent);

//static qboolean       is_quad;
//static byte           is_silenced;

void healthCheck(edict_t *self);
void DroneSlash(edict_t *ent, int damage, int kick, int range);
void GuardianSlash(edict_t *ent, int damage, float range, int offset);
void DroneWeb(edict_t *ent);
void WraithSpit(edict_t *ent);
void MechFire (edict_t *ent, float off);
void StalkerSpike (edict_t *ent);
void StingerFire (edict_t *ent);
void GuardianInfest(edict_t *self);
void tripwire_think (edict_t *self);
extern void spawn_spark (edict_t *ent);

/*int FindAmmoForGun (edict_t *ent)
{
        gclient_t       *cl;
        int                     i, index;
        const gitem_t           *it;

        cl = ent->client;

        if (!cl->weapon)
                return -1;

        // scan  for the next valid one
        for (i=0 ; i<MAX_ITEMS ; i++)
        {
                index = i;
                if (!cl->resp.inventory[index])
                        continue;
                it = &itemlist[index];
                if (! (it->flags & IT_CLIP) )
                        continue;
                if (!it->use)
                        continue;

                // weapon eats this kind of ammo?
                if (it->tag == cl->weapon->tag)
                //if(cl->ammo_index == ITEM_INDEX(FindItem(it->ammo)))
                {
                        return index;
                }
        }

        return -1;
}*/


void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t /*@out@*/result)
{
        vec3_t  _distance;

        VectorCopy (distance, _distance);
        if (client->resp.team == TEAM_ALIEN)
        {
                _distance[1] = 0;
        }
        else
        {
                if (client->pers.hand == LEFT_HANDED)
                        _distance[1] *= -1 ;
                else if (client->pers.hand == CENTER_HANDED)
                        _distance[1] = 0;
        }
        G_ProjectSource (point, _distance, forward, right, result);
}


/*
ShowGun

The old weapon has been dropped all the way, so make the new one
current
*/

static void ShowGun(edict_t *ent)
{

        if(ent->client->weapon)
        {

        switch (ent->client->resp.class_type) {
                case CLASS_GRUNT:
                        ent->s.modelindex2 = gi.modelindex("players/male/autogun.md2");
                        break;
                case CLASS_HEAVY:
                        ent->s.modelindex2 = gi.modelindex("players/hsold/weapon.md2");
                        break;
                case CLASS_COMMANDO:
                        ent->s.modelindex2 = gi.modelindex("players/male/smg.md2");
                        break;
                case CLASS_SHOCK:
                        ent->s.modelindex2 = gi.modelindex("players/male/shotgun.md2");
                        break;
                case CLASS_BIO:
                        ent->s.modelindex2 = gi.modelindex("players/female/weapon.md2");
                        break;
                default:
                        break;
                }
        }
}


void WeaponNotify(edict_t*ent)
{
  if (!(ent->client->pers.uflags & UFLAG_WEAPON_NOTIFY)) return;
  char*s = "$@o\n"; //default to observer
  int c = ent->client->resp.class_type;

  if (ent->health < 1) s = "$@o\n"; //obs/dead
  else  
  if (c == CLASS_BREEDER) s = "$@b\n";
  else
  if (c == CLASS_HATCHLING) s = "$@h\n";
  else
  if (c == CLASS_DRONE) s = "$@d\n";
  else
  if (c == CLASS_WRAITH) s = "$@w\n";
  else
  if (c == CLASS_KAMIKAZE) s = "$@k\n";
  else
  if (c == CLASS_STINGER) s = "$@t\n";
  else
  if (c == CLASS_GUARDIAN) s = "$@g\n";
  else
  if (c == CLASS_STALKER) s = "$@s\n";
  else
  if (c == CLASS_ENGINEER) s = "$@e\n";
  else
  if (c == CLASS_MECH) s = "$@c\n";
  else
  if (ent->client->weapon) {
    int w = ITEM_INDEX(ent->client->weapon);
    if (w == ITEM_WEAPON_PISTOL) s = "$@p\n";
    else
    if (w == ITEM_WEAPON_MAGNUM) s = "$@m\n";
    else
    if (w == ITEM_WEAPON_SHOTGUN) {
      if (ent->client->ammo_index != ITEM_AMMO_EXSHELLS)
        s = "$@st\n";
      else
        s = "$@ex\n";
    } else
    if (w == ITEM_WEAPON_SCATTERGUN) s = "$@sg\n";
    else
    if (w == ITEM_WEAPON_AUTOGUN) s = "$@mg\n";
    else
    if (w == ITEM_WEAPON_SUBGUN) s = "$@smg\n";
    else
    if (w == ITEM_WEAPON_ROCKET_LAUNCHER) s = "$@rl\n";
    else
    if (w == ITEM_WEAPON_PULSE_LASER) s = "$@xt\n";
  }

  stuffcmd(ent, s);
}


void ChangeWeapon (edict_t *ent)
{
        ent->client->lastweapon = ent->client->weapon;
        ent->client->weapon = ent->client->newweapon;
        ent->client->newweapon = NULL;

        if (ent->client->weapon) {
                const gitem_t   *it;
                int i;

                // see if the weapon as already been used (thus might have loaded ammo in)
                ent->client->ammo_index = 0; // forces reload if no ammo is found
                it = itemlist;
                for (i=0 ; i<game.num_items ; i++, it++)
                {
                        if (it->flags & IT_AMMO && it->clip_type == ent->client->weapon->clip_type && ent->client->resp.inventory[i]) {
                                ent->client->ammo_index = i;
                                break;
                        }
                }

                // update resp.shot_type to match the clip
                if (ent->client->ammo_index)
                {
                        it = itemlist;
                        for (i=0 ; i<game.num_items ; i++, it++)
                        {
                                // matches?
                                if (it->flags & IT_CLIP && it->tag == itemlist[ent->client->ammo_index].tag) {
                                        ent->client->resp.shot_type = i;
                                        break;
                                }
                        }
                }

        } else
                ent->client->ammo_index = 0;

        if (!ent->client->weapon)
        {       // dead
                ent->client->resp.shot_type = 0;
                ent->client->ps.gunindex = 0;
                ent->s.modelindex2 = 0;
                return;
        }

        ent->client->weaponstate = WEAPON_ACTIVATING;
        ent->client->ps.gunframe = 0;

        ent->client->ps.gunindex = gi.modelindex(ent->client->weapon->view_model);

        if(ent->client->weapon->active_sound)
                gi.sound(ent, CHAN_AUTO, gi.soundindex(ent->client->weapon->active_sound), 1, ATTN_NORM, 0);

        // play "change weapon" anim
        ent->client->anim_priority = ANIM_PAIN;
        if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        {
                ent->s.frame = FrameReference(ent, FRAME_DUCKPAIN_S);
                ent->client->anim_end = FrameReference(ent, FRAME_DUCKPAIN_E);
        }
        else
        {
                ent->s.frame = FrameReference(ent, FRAME_PAIN1_S);
                ent->client->anim_end = FrameReference(ent, FRAME_PAIN1_E);
        }
        
        WeaponNotify(ent);

        ShowGun(ent);

	ent->client->resp.upgrades &= ~UPGRADE_MORTAR_ENABLED;
}

/*
NoAmmoWeaponChange
*/
void Use_Weapon (edict_t *ent, const gitem_t *item);
void NoAmmoWeaponChange (edict_t *ent)
{
        int                     i,j;
        const gitem_t           *it, *temp;
        qboolean        use_weap=false;

        // search for any ammo, and try to change weapon if so
        // search for weapons and ammo/clips for them
        for (i=0 ; i<game.num_items ; i++)
        {
                it = itemlist + i;              

                // a weapon?
                if (!(it->flags & IT_WEAPON))
                        continue;

                // do we have it in inv?
                if (!ent->client->resp.inventory[i])
                        continue;

                // we don't want same weapon
                if (ent->client->weapon == it)
                        continue;

                // find any ammo or clips
                for (j=0 ; j<game.num_items ; j++)
                {
                        temp = itemlist + j;

                        if (!(temp->flags & (IT_CLIP|IT_AMMO)))
                                continue;

                        // same clip type?
                        if (temp->clip_type != it->clip_type)
                                continue;

                        if(ent->client->resp.inventory[j]) {
                                use_weap = true;
                                break;
                        }
                }

                // found ammo or clips for a weapon
                if(use_weap)
                {
                        Use_Weapon (ent, it);
                        break;
                }
        }
}

static void obstacle_clear_push(edict_t*ent)
{
  if (VectorLength(ent->velocity) < 1) {
    ent->postthink = NULL;
    ent->target_ent = NULL;
  }
}

/*
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
*/
//void SpawnDamage (int type,vec3_t origin,vec3_t normal, int damage);
void AlienWeaponThink (edict_t *ent)
{
 	qboolean gloom_pmove = (int)dmflags->value & DF_GLOOMPMOVE != 0;
        int i;
        edict_t *who;
        gclient_t *client;

        client = ent->client;

        // FIXME: Move delayed weapons/attacks off ClientThink weapon calls.

        
        if (((int)dmflags->value & DF_ALIEN_LIGHT) && !(ent->client->pers.uflags & UFLAG_NO_LIGHT)) {
          qboolean send = false;
          who = g_edicts;
          for (i = 0; i < game.maxclients; i++) {
            who++;
            if (who->inuse && who->client && who->client->pers.connected && who->health > 0) {
              if (who->flags & FL_SELFGLOW && gi.inPVS(ent->s.origin, who->s.origin)) {
                gi.WriteByte(svc_muzzleflash);
                gi.WriteShort(who - g_edicts);
                gi.WriteByte(MZ_NUKE8);
                send = true;
              } else
              if (who->client->resp.parasited && gi.inPVS(ent->s.origin, who->s.origin)) {
                gi.WriteByte(svc_muzzleflash);
                gi.WriteShort(who - g_edicts);
                gi.WriteByte(MZ_NUKE1);
                send = true;
              }
            }
          }
          if (send) gi.unicast(ent, false);
        } else {

          if (ent->flags & FL_SELFGLOW) {        //yuxxy yux hax
                gi.WriteByte(svc_muzzleflash);
                gi.WriteShort(ent - g_edicts);
//                gi.WriteByte(MZ_NUKE4);
                gi.WriteByte(MZ_NUKE8);
                /*gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_FLASHLIGHT);
                gi.WritePosition (ent->s.origin);
                gi.WriteShort (ent-g_edicts);*/
                gi.unicast(ent, false);
          }

          if (ent->client->resp.class_type == CLASS_GUARDIAN) { //Guard can always see parasited players
            qboolean send = false;
            who = g_edicts;
            for (i = 0; i < game.maxclients; i++) {
              who++;
              if (who->inuse && who->health > 0 && who->client && who->client->pers.connected && who->client->resp.parasited && gi.inPVS(ent->s.origin, who->s.origin)) {
                gi.WriteByte(svc_muzzleflash);
                gi.WriteShort(who - g_edicts);
                gi.WriteByte(MZ_NUKE1);
                send = true;
              }
            }
            if (send) gi.unicast(ent, false);
          }
        }

        switch (client->resp.class_type){
        case CLASS_BREEDER:
                if ( ((client->latched_buttons|client->buttons) & BUTTON_ATTACK) )
                {
                        trace_t tr;
                        vec3_t          angles, forward, start, end;

                        if (ent->fly_sound_debounce_time > level.framenum)
                                return;

                        VectorAdd (client->v_angle, client->kick_angles, angles);
                        VectorCopy (ent->s.origin, start);
                        start[2] += ent->viewheight;
                        AngleVectors (angles, forward, NULL, NULL);

                        VectorMA (start, 64, forward, end);

                        tr = gi.trace (start, NULL, NULL, end, ent, MASK_SHOT);

                        //if (self->client)
                //              PlayerNoise(self, tr.endpos, PNOISE_IMPACT);

                        if (tr.fraction >= 1.0)
                        {
                                return;
                        }

                        ent->fly_sound_debounce_time = level.framenum + 9;

                        client->anim_priority = ANIM_ATTACK;
                        ent->s.frame = 77;
                        client->anim_end = 85;
                        fun_breeder_bite(ent, &tr, forward);
                        if (tr.ent) {
                                if (tr.ent->client)
                                {
                                        int     kick = 100;
                                        int     dmg = 10;
                                        if (tr.ent->client->resp.class_type == CLASS_ENGINEER || tr.ent->client->resp.class_type == CLASS_GRUNT)
                                        {
                                                dmg = 15;
                                                kick = 150;
                                        }
                                        // && (tr.ent->client->resp.class_type == CLASS_ENGINEER || tr.ent->client->resp.class_type == CLASS_GRUNT)) {
                                        T_Damage (tr.ent,ent,ent,forward,tr.endpos,tr.plane.normal,dmg,kick,0,MOD_BREEDER_PUSH);

                                }
                                else if (tr.ent->enttype == ENT_CORPSE || tr.ent->enttype == ENT_HUMAN_BODY)
                                        T_Damage (tr.ent,ent,ent,forward,tr.endpos,tr.plane.normal,30,30,0,MOD_BREEDER_PUSH);

                                //r1: lame breeder push/nudge (its back !)
                                if ((tr.ent->svflags & SVF_MONSTER || tr.ent->enttype == ENT_AMMO_DEPOT) && !(((tr.ent->flags & FL_MAP_SPAWNED) && level.time < (60*xmins->value))) && !tr.ent->hurtflags && !tr.ent->targetname && !(gi.pointcontents(tr.ent->s.origin)&CONTENTS_NOBUILD)) {
                                        trace_t space;
                                        vec3_t mins, maxs;
                                        tr.ent->spawnflags |= SPAWNFLAG_BREEDER_PUSH;
                                        VectorCopy (tr.ent->mins, mins);
                                        VectorCopy (tr.ent->maxs, maxs);
                                        if (tr.ent->enttype == ENT_COCOON) {
                                                maxs[0] += 8;
                                                maxs[1] += 8;
                                                if (tr.ent->spawnflags & 32)
                                                  mins[2] -= 24;
                                                else
                                                  maxs[2] += 24;
                                                mins[0] -= 8;
                                                mins[1] -= 8;
                                        } else
                                        if (tr.ent->enttype == ENT_OBSTACLE) {
                                          tr.ent->target_ent = ent;
                                          tr.ent->postthink = obstacle_clear_push;
                                        }

                                        space = gi.trace (tr.ent->s.origin, mins, maxs, tr.ent->s.origin, tr.ent, CONTENTS_SOLID);
                                        if (space.fraction == 1.0) {
                                                VectorCopy (ent->velocity, tr.ent->velocity);
                                                tr.ent->velocity[2] = tr.ent->gravity;
                                                if (tr.ent->gravity < 0)
                                                  tr.ent->s.origin[2] += tr.ent->gravity*3; //ugly unstick from ceiling
                                                VectorScale (tr.ent->velocity, 2, tr.ent->velocity);
                                        }
                                }
                        }
                }
                break;
        case CLASS_HATCHLING:
                if (!gloom_pmove && SETTING_INT(hatchling_flags)&1 && client->buttons & BUTTON_ATTACK ) {
                  client->latched_buttons &= ~BUTTON_ATTACK;

                  if (!client->ctf_grapple && ent->wait < level.time) {
                     ent->wait = level.time+g_grapple_delay->value;
                     CTFGrappleFire (ent, vec3_origin, 10);
                  }
                }

                if (!(client->buttons & BUTTON_ATTACK))
                {
                        if (client->ctf_grapple)
                        {
                                CTFResetGrapple(client->ctf_grapple);
                                if (g_grapple_delay->value > 0.1f)
                                  ent->wait = 0;
                        }
                }
                break;
        case CLASS_DRONE:
                if (client->anim_priority < ANIM_ATTACK) {
                        if (client->buttons & BUTTON_ATTACK)
                        {
                                client->anim_priority = ANIM_ATTACK;
                                ent->s.frame = DRONE_ATTACK_S;
                                client->anim_end = DRONE_ATTACK_E;
                                client->weaponstate = WEAPON_DROPPING; // mark as slash

                        }
                        else if ((client->buttons & BUTTON_USE) && client->resp.inventory[client->ammo_index])
                        {
                                client->anim_priority = ANIM_ATTACK;
                                ent->s.frame = DRONE_ATTACK_S;
                                client->anim_end = DRONE_ATTACK_E;
                                client->weaponstate = WEAPON_RELOADING;// mark as web
                        }
                }

                if(ent->s.frame == DRONE_ATTACK_S+1 && client->weaponstate == WEAPON_DROPPING)
                {
                        DroneSlash(ent, 85+random()*10, 20, 90);
                        if (client->buttons & BUTTON_USE)
                        {
                                if (client->resp.inventory[client->ammo_index] == 5)
                                        client->last_reload_time = level.time + .8f;

                                DroneWeb (ent);
                        }
                }

                if(ent->s.frame == DRONE_ATTACK_S+1 && client->weaponstate == WEAPON_RELOADING)
                {
                        if (client->resp.inventory[client->ammo_index] == 5)
                                client->last_reload_time = level.time + .8f;
                        DroneWeb(ent);
                }
                
                if (client->last_reload_time < level.time && client->resp.inventory[client->ammo_index] < 5)
                {
                        client->resp.inventory[client->ammo_index]++;
                        client->last_reload_time = level.time + .8f;
                }

                break;
        case CLASS_WRAITH:
                if ( ((client->latched_buttons|client->buttons) & BUTTON_ATTACK) )
                {
                        if((ent->s.frame < WRAITH_SLASH_S) || (ent->s.frame > WRAITH_SLASH_E))
                        {
                                client->anim_priority = ANIM_ATTACK;
                                ent->s.frame = WRAITH_SLASH_S + 2;
                                client->anim_end = WRAITH_SLASH_E;
                        }

                }

                if(ent->s.frame == WRAITH_SLASH_S+4)
                {
                        DroneSlash(ent, 40 + random()*10, 5, 75);
                }

                if (client->resp.inventory[client->ammo_index] < 5 && client->last_reload_time < level.time && !(client->buttons & BUTTON_USE && client->resp.inventory[client->ammo_index] != 0))
                {
                        client->last_reload_time = level.time + .5f;
                        client->resp.inventory[client->ammo_index]++;
                }

                if ( (client->buttons & BUTTON_USE))
                {
                        if((ent->s.frame < WRAITH_FLYATT_S) || (ent->s.frame > WRAITH_FLYATT_E))
                        {
                                client->anim_priority = ANIM_ATTACK;
                                ent->s.frame = WRAITH_FLYATT_S;
                                client->anim_end = WRAITH_FLYATT_E;
                        }

                }
                if ((ent->s.frame == WRAITH_FLYATT_S + 1)  || ((client->buttons & BUTTON_USE) && (ent->s.frame == WRAITH_FLYATT_S + 3 || ent->s.frame == WRAITH_FLYATT_S + 5) ))
                {
                        WraithSpit(ent);
                }
                break;

        case CLASS_KAMIKAZE:
                if (!gloom_pmove && SETTING_INT(hatchling_flags)&1 && client->buttons & BUTTON_ATTACK ) {
                  client->latched_buttons &= ~BUTTON_ATTACK;

                  if (!client->ctf_grapple && ent->wait < level.time) {
                     ent->wait = level.time+g_grapple_delay->value;
                     CTFGrappleFire (ent, vec3_origin, 10);
                  }
                }

                if (!(client->buttons & BUTTON_ATTACK))
                {
                        if (client->ctf_grapple) {
                                CTFResetGrapple(client->ctf_grapple);
                        }
                }

                if ( (client->latched_buttons & BUTTON_USE)) // && level.framenum > client->invincible_framenum)
                {
                        int spikes = 0;
                        ent->dmg = 350;
                        ent->dmg_radius = 350;
                        if (client->resp.upgrades & UPGRADE_SPIKE_POUCH)
                                spikes = 1;
                        if (!dmg_override || !GetExplosionInfo(NULL, ent, ent, MOD_KAMIKAZE)) {
                          kamikaze_explode(ent, MOD_KAMIKAZE, DAMAGE_IGNORE_STRUCTS);
                          T_RadiusDamage(ent,ent,450,ent,48,MOD_KAMIKAZE, 0);
                          T_RadiusDamage(ent,ent,150,ent,100,MOD_KAMIKAZE, DAMAGE_IGNORE_STRUCTS);
                          T_RadiusDamage(ent,ent,50,ent,400,MOD_KAMIKAZE, 0);
                          T_RadiusDamage(ent,ent,450,ent,360,MOD_KAMIKAZE,DAMAGE_IGNORE_CLIENTS);
                        } else
                          kamikaze_explode(ent, MOD_KAMIKAZE, 0);
                        if (spikes) {
                          float tmin, tmax;
                          cvar_range(g_kamikaze_spike_time, &tmin, &tmax);
                          spew_spiketrops (ent, ent, 13, tmin, tmax-tmin);
                        }
                }
                break;
        case CLASS_STINGER:
                // flame overrides sting attack
                if (client->anim_priority < ANIM_ATTACK) {
                        if (client->buttons & BUTTON_ATTACK && client->resp.inventory[client->ammo_index])
                        {
                                        client->anim_priority = ANIM_ATTACK;
                                        ent->s.frame = STING_SHOT_S-1;
                                        client->anim_end = STING_SHOT_E;
                        }
                        else if (client->buttons & BUTTON_USE)
                        {
                                        client->anim_priority = ANIM_ATTACK;
                                        ent->s.frame = STING_STING_S+1;
                                        client->anim_end = STING_STING_E;
                        }
                }
                else if (client->anim_priority == ANIM_ATTACK) {
                        if (ent->s.frame > STING_SHOT_S && ent->s.frame < STING_SHOT_E && (client->buttons & (BUTTON_USE|BUTTON_ATTACK)) == BUTTON_USE)
                          client->anim_end = ent->s.frame;

                        if(ent->s.frame == STING_STING_S+2)
                        {
                                DroneSlash(ent, 100, 75, 110);
                        }
                        else if (client->anim_end == STING_SHOT_E && client->buttons & BUTTON_ATTACK)
                        {
                                StingerFire (ent);

                                if(ent->s.frame >= STING_SHOT_E-1) { // -3 looks better on 1.2 model
                                        ent->s.frame = STING_SHOT_S+1;
                                }

                        }
                }

                // start refill after stopping flame attack
                // FIXME: triggering slash and fire at the same time tends to break this
                //if (client->anim_end != STING_SHOT_E && (client->resp.inventory[client->ammo_index] < 10)) {
                if (!(client->buttons & (BUTTON_ATTACK|BUTTON_USE)) && (client->resp.inventory[client->ammo_index] < 10)) {
                        client->resp.inventory[client->ammo_index]++;
                }

                break;
        case CLASS_GUARDIAN:
                // FIXME: move off think_weapon
                // c4ed guard explodes with a timer
                if(client->grenade_blow_up > 0 && (client->grenade_time < level.time))
                {
                        ent->dmg = 370+random()*50;
                        ent->dmg_radius = 420+random()*50;
                        ent->dmg *= client->grenade_blow_up;
                        ent->dmg_radius *= client->grenade_blow_up;
                        if (!dmg_override || !GetExplosionInfo(NULL, ent, ent, MOD_EAT_C4)) {
                          T_RadiusDamage(ent,ent,450*(client->grenade_blow_up),ent,64*(client->grenade_blow_up),MOD_EAT_C4, 0);
                          T_RadiusDamage(ent,ent,350*(client->grenade_blow_up),ent,190*(client->grenade_blow_up*2),MOD_EAT_C4, 0);
                          T_RadiusDamage(ent,ent,100*(client->grenade_blow_up),ent,600*(client->grenade_blow_up),MOD_EAT_C4, 0);
                        }
                        /*ent->dmg = 200;
                        ent->dmg_radius = 200;
                        ent->dmg *= (client->grenade_blow_up > 1 ? client->grenade_blow_up + 2 : 1);
                        ent->dmg_radius *= (client->grenade_blow_up > 1 ? client->grenade_blow_up + 2 : 1);
                        client->grenade_blow_up = 0;*/
                        kamikaze_explode(ent, MOD_EAT_C4, 0);
                        client->grenade_blow_up = 0;
                        gi.sound(ent,CHAN_AUTO,SoundIndex (voice_toastie),1,ATTN_NORM,0);
                        //gi.sound(ent,CHAN_AUTO,SoundIndex (voice_toastie),1,ATTN_NORM,0);
                        //gi.sound(ent,CHAN_AUTO,SoundIndex (voice_toastie),1,ATTN_NORM,0);
                        return;
                }

                if (client->anim_priority < ANIM_ATTACK) {
                        if (client->buttons & BUTTON_ATTACK)
                        {
                                client->anim_priority = ANIM_ATTACK;
                                ent->s.frame = GUARD_ATTACK_S;
                                client->anim_end = GUARD_ATTACK_E;
                        } else if (client->buttons & BUTTON_USE)
                        {
                                client->anim_priority = ANIM_ATTACK;
                                ent->s.frame = GUARD_INFEST_S;
                                client->anim_end = GUARD_INFEST_E;
                        }
                }
                else if (client->anim_priority == ANIM_ATTACK) {
                        if(ent->s.frame == GUARD_ATTACK_S+4)
                        {
                                GuardianSlash(ent, 55, 110, 13);
                                //GuardianSlash(ent, 65, 128, -8);
                                ent->s.modelindex = 255;
                                ent->fly_sound_debounce_time = level.framenum + 10;
                        }
                        else if(ent->s.frame == GUARD_INFEST_S+5) {
                                // infest nearby bodies
                                ent->fly_sound_debounce_time = level.framenum + 15;
                                GuardianInfest(ent);
                        }
                }

                if (ent->s.modelindex == 0) {
                        for (who = g_edicts +1; who->client; who++) {
                                if (!who->inuse)
                                        continue;
                                if (who->client->resp.team != TEAM_NONE && who->health <= 0)
                                        continue;
                                if (who->client->resp.team == TEAM_HUMAN && who->client->resp.class_type != CLASS_BIO)
                                        continue;
                                if (who->client->resp.team == TEAM_ALIEN && ent->client->mirror_ent != NULL)
                                        continue;
                                if (who->client->resp.team == TEAM_NONE && ent->client->mirror_ent != NULL)
                                        continue;
                                if (who == ent)
                                        continue;
                                if (!gi.inPVS (who->s.origin, ent->s.origin))
                                        continue;
                                gi.WriteByte(svc_muzzleflash);
                                gi.WriteShort(ent - g_edicts);
                                gi.WriteByte(MZ_NUKE1 | MZ_SILENCED);
                                gi.unicast(who, false);
                        }
                }

                // FIXME: move this off think_weapon
                //i = ITEM_INDEX(client->armor);

                
            if (g_guardian_regen_mode->value == 1) {
                if (0 && level.framenum & 1) {
                        int pvs;
                        //int phs;
                        edict_t *from = NULL;

                        pvs = 0;

                        //gi.bprintf (PRINT_HIGH, "checking at %f\n", level.time);

                        while ((from = G_Find3(from, ENT_COCOON)) != NULL) {
                                //if ((from->flags & FL_MAP_SPAWNED) && level.time < (60*xmins->value))
                                //      continue;

                                if (gi.inPVS (from->s.origin, ent->s.origin)) {
                                        if (from->air_finished < level.time)
                                                pvs++;
                                }
                        }

                        //gi.bprintf (PRINT_HIGH, "phs %d pvs %d\n", phs, pvs);

                        from = NULL;

                        if (pvs < 4) {
                                while ((from = G_Find3(from, ENT_TELEPORTER)) != NULL) {
                                        if (gi.inPVS (from->s.origin, ent->s.origin)) {
                                                pvs = 0;
                                                break;
                                        }
                                }
                        }

                }


                //i = ITEM_INDEX(client->armor);


                if (!level.suddendeath) {
                        if (ent->s.modelindex == 0 && !(ent->client->resp.upgrades & UPGRADE_CELL_WALL)) {
                                //invisible guard gets a little bit of armor
                                if (client->resp.inventory[ITEM_ARMOR_COMBAT] < 70 && client->last_reload_time < level.time) {
                                        client->last_reload_time = level.time + g_guardian_regen_speed->value;
                                        client->resp.inventory[ITEM_ARMOR_COMBAT]++;
                                }
                                ent->client->last_move_time = level.time;
                        }/* else {
                                if (ent->client->last_move_time + 0.5 < level.time && client->resp.inventory[ITEM_ARMOR_COMBAT] > 0 && (level.framenum % 4) == 0) {
                                        client->resp.inventory[ITEM_ARMOR_COMBAT]--;
                                        //if (client->resp.inventory[i] < 0)
                                        //      client->resp.inventory[i] = 0;
                                }
                        }*/
                }
            }
                break;
        case CLASS_STALKER:

                if (client->anim_priority < ANIM_ATTACK) {
                        if (client->buttons & BUTTON_ATTACK)
                        {
                                if(randomMT() & 3)
                                {
                                        client->anim_priority = ANIM_ATTACK;
                                        ent->s.frame = STALK_ATTACK_S;
                                        client->anim_end = STALK_ATTACK_E;
                                }
                                else
                                {
                                        client->anim_priority = ANIM_ATTACK;
                                        ent->s.frame = STALK_BITE_S;
                                        client->anim_end = STALK_BITE_E;
                                }
                        }
                        else if ((client->buttons & BUTTON_USE) && client->resp.inventory[client->ammo_index] > 1)
                        {
                                client->anim_priority = ANIM_ATTACK;
                                ent->s.frame = STALK_HOOK_S;
                                client->anim_end = STALK_HOOK_E;
                                ent->random = level.time + 3.5f; //hijacking: random = spike regen time
                        }
                        else
                        {
                                int max = 20;
                                float rtime = 3.6f;
                                if (ent->client->resp.upgrades & UPGRADE_SPIKE_SAC) {
                                  max = 30;
                                  rtime = 3.3f;
                                }
                                if (ent->random < level.time && ent->client->resp.inventory[ITEM_AMMO_SPIKES] < max) {
                                   ent->client->resp.inventory[ITEM_AMMO_SPIKES] += 2;
                                   ent->random = level.time + rtime;
                                }
                        }
                }
                else if (client->anim_priority == ANIM_ATTACK) {
                        if(ent->s.frame == STALK_ATTACK_S+3)
                                DroneSlash(ent, 150, 100, 100);
                        else if(ent->s.frame == STALK_BITE_S+3)
                                DroneSlash(ent, 150, 100, 102);
                                // FIXME: If hit, give me health..
                        else if(ent->s.frame == STALK_HOOK_S+1)
                                StalkerSpike(ent);              // Fire spikes
                }

                break;
        }
}

static void foam_corrode(edict_t*ent)
{
  float radius = 96;
  int damage = 10;
  
  int i;
  vec3_t pos;
  int steps = 0;

  if (!ent->owner->inuse || (ent->owner->client && ent->owner->client->resp.team != TEAM_HUMAN)) {
    G_FreeEdict(ent);
    return;
  }
  
  
  VectorCopy(ent->s.origin, pos);
  for (i = 0; i < 3; i++) {
    vec3_t dir;
    float len, vert, speed;
    VectorSubtract(ent->s.origin, ent->ents[i]->s.origin, dir);
    vert = fabs(dir[2]);
    dir[2] = 0;
    len = VectorNormalize(dir)-vert;
    
    speed = (64-len)/32;
    if (speed > 0.1f) {
      if (speed > 2.5) speed = 2.5;
      steps++;
      
      VectorMA(pos, speed, dir, pos);
    }
  }
  
  if (steps) {
    trace_t tr = gi.trace(ent->s.origin, NULL, NULL, pos, ent, MASK_SOLID);
    if (tr.fraction > 0.01f) {
      VectorCopy(tr.endpos, ent->s.origin);
      gi.linkentity(ent);
    }
  }

  ent->nextthink = level.time+FRAMETIME;

  if ((ent->count & 3) == ent->newcount)
    T_RadiusDamage(ent, ent->owner, damage, ent, radius, MOD_CORRODED, DAMAGE_NO_FALLOFF | DAMAGE_NO_KNOCKBACK | DAMAGE_FRIENDLY_SPLASH);

  if (!--ent->count) G_FreeEdict(ent);
}

static void foam_explode(edict_t*ent)
{
  vec3_t origin, vel;
  edict_t* owner;
  int i, a;
  int r = randomMT()&255;
  float v = 120;
  vec3_t end = {ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]-32};
  trace_t tr = gi.trace(ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);
  if (tr.fraction == 1) v = 60;

  VectorCopy(ent->s.origin, origin);
  VectorCopy(ent->velocity, vel);
  owner = ent->owner;
  G_FreeEdict(ent);
  origin[2]++;
  vel[2] += v;
  
  edict_t*ents[4];
  
  for (i = 0; i < 4; i++) {
    ent = ents[i] = G_Spawn();
    ent->classname = "foam";
    ent->movetype = MOVETYPE_TOSS;
    ent->owner = owner;
    
    VectorCopy(origin, ent->s.origin);
    VectorCopy(vel, ent->velocity);
    a = (int)(r+crandom()*15)&255;
    ent->velocity[0] += v*sin(r*M_PI/128);
    ent->velocity[1] += v*cos(r*M_PI/128);
    r += 64;

    ent->s.modelindex = gi.modelindex("models/objects/smokexp/tris.md2");
    ent->count = 6/FRAMETIME;
    ent->newcount = i;
    ent->think = foam_corrode;
    ent->nextthink = level.time+FRAMETIME;
    ent->s.sound = SoundIndex(player_fry);
    ent->s.effects = /*EF_ANIM_ALL |*/ EF_FLAG1;
  }
  
  for (i = 0; i < 4; i++)
    for (r = 0; r < 3; r++)
      ents[i]->ents[r] = ents[(i+r+1) & 3];

  gi.sound(ent, CHAN_AUTO, SoundIndex(weapons_misload), 1, ATTN_NORM, 0);
}

static void foam_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  VectorClear(ent->velocity);
  foam_explode(ent);
}

static void mech_blaster_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  float radius = 100;
  int damage = 65;

  if (!surf || !(surf->flags & SURF_SKY)) {
    if (!plane)
      temp_impact(TE_BLASTER, ent->s.origin, vec3_origin);
    else
      temp_impact(TE_BLASTER, ent->s.origin, plane->normal);
    gi.multicast(ent->s.origin, MULTICAST_PVS);

    T_RadiusDamage(ent, ent->owner, damage, ent, radius, MOD_HYPERBLASTER, DAMAGE_LINEAR);
  }
  
  G_FreeEdict(ent);
}



void MechWeaponThink(edict_t *ent)
{
#if 0
        int xyspeed;
        static const gitem_t *item = NULL;
        static int i = 0;

        if (!item) 
        {
                item = FindItem("Mini Missiles");
                i = ITEM_INDEX(item);
        }

        //ent->client->ps.gunframe=5;

        //this is horrible. really.
//      if (ent->client->resp.inventory[i] != 1 && level.time > ent->client->last_reload_time) 
//      {
//              gi.cprintf (ent, PRINT_HIGH, "Missiles refilled!\n");
//              ent->client->resp.inventory[i] = 1;

//              gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_refill), 1, ATTN_NORM, 0);

                // flash the screen
//              ent->client->bonus_alpha = 0.25;

                // show icon and name on status bar
//              ent->client->ps.stats[STAT_PICKUP_ICON] = imagecache[item->item_id];
//              ent->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+i;
//              ent->client->pickup_msg_time = level.time + 3.0f;
//      }

//      if ((xyspeed = (int)(fabs(ent->velocity[0]) + fabs(ent->velocity[1]))))
//              ent->random = 0;
#endif

        int rocketmode = SETTING_INT(mech_rockets);
        
        
        if (rocketmode == 5) {
          if (ent->client->buttons & BUTTON_USE && ent->client->grenade_time < level.time) {
            edict_t *grenade;
            vec3_t  angles, forward, start;

            ent->client->resp.can_respawn = false;

            //ent->client->resp.inventory[ITEM_INDEX(item)]--;
            //ValidateSelectedItem (ent);

            VectorAdd(ent->client->v_angle, ent->client->kick_angles, angles);
            VectorCopy (ent->s.origin, start); start[2] += 16;
            AngleVectors (angles, forward, NULL, NULL);
            forward[2] = 0;
            VectorNormalize(forward);
            grenade = fire_grenade(ent, start, forward, 0, 100 + DotProduct(forward, ent->velocity), 0.6, 0);
            grenade->classname = "foamgrenade";
            grenade->enttype = ENT_GRENADE_NEW; //FIXME: own enttype
            grenade->touch = foam_touch;
            grenade->think = foam_explode;
            gi.sound(ent, CHAN_AUTO, SoundIndex(weapons_grenlf1a), 1, ATTN_NORM, 0);
            
            ent->client->grenade_time = level.time+1;
          }
        }
        if (rocketmode == 3 || rocketmode == 4) {
          if (ent->client->resp.inventory[ITEM_AMMO_MINIMISSILES] > 0 && (ent->client->buttons & BUTTON_USE && ent->wait+6.5f < level.time || ent->flags & FL_LATCHED)) {
            int i;
            
            if (!(ent->flags & FL_LATCHED)) {
              VectorSet(ent->movedir, crandom()*8, crandom()*3, crandom()*16);
              if (rocketmode == 3)
                ent->newcount = 8;
              else
                ent->newcount = 16;

              if (ent->newcount > ent->client->resp.inventory[ITEM_AMMO_MINIMISSILES])
                ent->newcount = ent->client->resp.inventory[ITEM_AMMO_MINIMISSILES];
            }
            if (ent->newcount)
              ent->flags |= FL_LATCHED;
            else
              ent->flags &= ~FL_LATCHED;

            ent->wait = level.time+1.0f; //also lock from shooting

            vec3_t dir;
            float d;
            VectorSubtract(ent->s.origin, ent->pos3, dir);
            if ((d = VectorLength(dir)) > 2) {
              vec3_t a = {dir[0], dir[1]};
              float scale = 1.0f;
              
              float s = sin(ent->client->v_angle[1]*M_PI/180);
              float c = cos(ent->client->v_angle[1]*M_PI/180);
              dir[0] = c*a[0] + s*a[1];  //forward movement
              dir[1] = c*a[1] - s*a[0];  //side movement

              ent->movedir[0] += crandom()*dir[0]*scale;
              ent->movedir[1] += dir[1]*scale + crandom()*dir[0]*scale*0.5f;
              ent->movedir[2] += crandom()*d*scale;
            }
                      
            i = 5*abs(ent->pos2[0]-ent->health) + 2*abs(ent->pos2[1] - ent->client->resp.inventory[ITEM_INDEX(ent->client->armor)]) + abs(dir[2]);
            if (i > 0) {
              float d = i/2.5f;
              for (i = 0; i < 4; i++)
                ent->movedir[i] += crandom()*d;
            }

            for (i = 0; i < 4; i++)
              ent->movedir[i] += fmod((5+2.5*crandom())*(ent->pos1[i] - ent->s.angles[i]), 360);

            if (ent->random <= level.time) {
              vec3_t angles, forward, right, up, offset, start;
              float h = (ent->flags & FL_RANDOM)?+8:-8;

              ent->client->resp.inventory[ITEM_AMMO_MINIMISSILES]--;
              ent->newcount--;
              ent->random = level.time+0.2f;

              VectorAdd(ent->client->v_angle, ent->client->kick_angles, angles);
              AngleVectors (angles, forward, right, up);
              VectorSet(offset, 0, 0, 58);
              P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
              if (rocketmode == 3)
                VectorMA(start, 2*h*(abs((ent->client->resp.inventory[ITEM_AMMO_MINIMISSILES]&7)-4)-1.5f), right, start);
              else
                VectorMA(start, h*(abs((ent->client->resp.inventory[ITEM_AMMO_MINIMISSILES]&15)-8)-3.5f), right, start);
              
              fire_bay_missile(ent, start, forward, ent->movedir, 75, 100);
            }
          } else
          if (!ent->newcount) {
            ent->flags &= ~FL_LATCHED;
            ent->client->resp.selected_item = ITEM_AMMO_MINIMISSILES;
            if (ent->wait+5.5f < level.time) {
              ent->newcount = -1;
              if (ent->client->invincible_framenum < level.framenum && ent->client->resp.inventory[ITEM_AMMO_MINIMISSILES] > 0) {
                gi.sound(ent, CHAN_AUTO, gi.soundindex("gunner/gunidle1.wav"), 1, ATTN_IDLE, 0);
                ent->client->bonus_alpha = 0.025;
              } else
                ent->newcount = -2;
            }
          } else
          if (ent->newcount == -1 && ent->wait+6.5f < level.time) {
            ent->client->bonus_alpha = 0.075;
            ent->newcount = -2;
          }

          VectorCopy(ent->s.angles, ent->pos1);
          ent->pos2[0] = ent->health;
          ent->pos2[1] = ent->client->resp.inventory[ITEM_INDEX(ent->client->armor)];
          ent->pos2[2] = 0;
          VectorCopy(ent->s.origin, ent->pos3);
        } else
        if (rocketmode == 1) {
          if (ent->client->resp.inventory[ITEM_AMMO_MINIMISSILES] <= 0) {
            if (level.time > ent->random) {
              ent->client->resp.inventory[ITEM_AMMO_MINIMISSILES] = 16;
              ent->random = 0;

              unicastSound(ent, SoundIndex(misc_ar1_pkup), 1.0);
              ent->client->ps.stats[STAT_PICKUP_ICON] = imagecache[i_missile];
              ent->client->ps.stats[STAT_PICKUP_STRING] = CS_GENERAL+game.maxclients+1;
              ent->client->pickup_msg_time = level.time + 3.0f;
              ent->client->bonus_alpha = 0.1;
            }
          } else
          if (ent->client->buttons & BUTTON_USE) {
            if (!ent->random || VectorLength(ent->velocity) > 8) {
              ent->random = level.time+3;
              ent->newcount = level.framenum;
            } else
            if (level.time > ent->random) {
              vec3_t angles, forward, right, up, offset, start;
              int count = SETTING_COUNT(mech_rockets, 1);
              int speed = 200;
              int j;

              if (!count) count = 5;

              ent->random = level.time+120;
              ent->client->resp.inventory[ITEM_AMMO_MINIMISSILES] = 0;

              VectorAdd(ent->client->v_angle, ent->client->kick_angles, angles);
              AngleVectors (angles, forward, right, up);
              VectorSet(offset, 0, 0, 58);
              P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
              VectorScale(right, -16.0, right);
              VectorAdd(start, right, start);
              VectorScale(right, -2.0/count, right);

              for (j = 0; j < count; j++) {
                speed += crandom() * 50;
                fire_drunk_missile(ent, start, forward, 70, speed, 100, 190);
                VectorAdd(start, right, start);
              }
            } else
            if ((level.framenum - ent->newcount) % 7 == 0)
              gi.sound(ent, CHAN_AUTO, SoundIndex(weapons_mechbeep), 1, ATTN_NORM, 0);
          } else
            ent->random = ent->newcount = 0;
        } else
        if (rocketmode == 2 &&   //Grim mech missiles chargeable and cost 10 normal rounds per missile
          ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE) && (ent->wait < level.time) &&
            ent->random < 30 &&
            ent->random <= ent->client->resp.inventory[ent->client->ammo_index] >> 1 &&
            ent->client->resp.inventory[ent->client->ammo_index] > 9) 
        {
                if (ent->wait > level.time)
                        return;

                if(ent->random > 0 && (int)(ent->random*100)%500 == 0)
                        gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_mechbeep), 1, ATTN_NORM, 0);

                ent->random += 0.5f;//charge them up
        } 
        else if (rocketmode == 2 && ent->random >= 5)
        {
                        vec3_t angles, forward, right, up, offset, start;
                        int j;
                        int speed = 700;
                        int howmany;

                        howmany=ent->random/5;

                        speed += crandom() * 100;
                        
                        if(howmany>5)
                                howmany=5;

                        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
                        AngleVectors (angles, forward, right, up);

                        VectorSet(offset, 0, 0, 58);
                        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
//                      VectorScale(right, 16.0, right);
//                      VectorAdd(start, right, start);

                        for (j = 0; j < howmany; j++) 
                        {
                                if(ent->client->resp.inventory[ent->client->ammo_index]>=10)
                                {
                                        fire_grim_missile (ent, start, forward, 70, speed, 100, 250);
                                        ent->client->resp.inventory[ent->client->ammo_index]-=10;
                                }
                        }

                        ent->random = 0;
                        
                        ent->wait = level.time+1.0f; //lock a bit

//                      speed = 200;
/*
                        VectorClear (angles);
                        VectorClear (right);
                        VectorClear (forward);
                        VectorClear (up);
                        VectorClear (start);

*/
        }
        else 
        {
                ent->random = 0;
        }

        // if repaired by engi, don't allow attacking
        if (ent->client->anim_priority < ANIM_ATTACK && ent->wait < level.time)
        {
                ent->client->ps.gunframe=5;

                if (ent->client->buttons & BUTTON_ATTACK)
                {
                        if (ent->client->anim_end == MECH_WALK_E)
                        {
                                // We're moving
                                ent->s.frame = MECH_WALKSHOOT_S;
                                ent->client->anim_end = MECH_WALKSHOOT_E;
                                ent->client->anim_priority = ANIM_ATTACK;
                        }
                        else
                        {
                                ent->s.frame = MECH_SHOOT_S; //1;
                                ent->client->anim_end = MECH_SHOOT_E;
                                ent->client->anim_priority = ANIM_ATTACK;
                        }
                }
        }
        else if (ent->client->anim_priority == ANIM_ATTACK)
        {
                ent->client->ps.gunframe++;

                //fire
                if(ent->client->ps.gunframe == 6)
                        MechFire(ent, 1);
                else if(ent->client->ps.gunframe == 8 && ent->client->buttons & BUTTON_ATTACK)
                        MechFire(ent, -1);

                // walk+shoot anim is longer so we need to sync gunanim to it
                // HACK: sometimes causes lonely shot when stopping, due to gunframe triggering first MechFire
                if (ent->client->anim_end == MECH_WALKSHOOT_E && ent->client->ps.gunframe > 10) {
                        ent->client->ps.gunframe = 5;
                        ent->client->anim_priority = ANIM_BASIC;
                }
        }
}

void biotech_heal (edict_t *self, float dist);
void Cmd_Discharge_f (edict_t *ent);
void Think_Weapon (edict_t *ent)
{
        if (!deathmatch->value)
                return;

        if (!ent->client->pers.connected)
                return;

        // FIXME: move this off
        if (ent->client->resp.class_type == CLASS_OBSERVER) {
                if (ent->client->resp.adminmode) {
                        if ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) {
                                if (!ent->client->trace_object) {
                                        edict_t *t;
                                        trace_t tr;
                                        vec3_t forward;

                                        AngleVectors(ent->client->v_angle, forward, NULL, NULL);
                                        VectorScale(forward, 4096, forward);
                                        VectorAdd(ent->s.origin, forward, forward);
                                        tr = gi.trace (ent->s.origin, tv(-16,-16,-16), tv(16,16,16), forward, ent, -1);                                 
                                        
                                        t = tr.ent;
                                        if (t && t != world && t->solid != SOLID_BSP) {
                                                gi.cprintf (ent, PRINT_HIGH, "Locked onto a %s [%d]\n", t->classname, t->s.number);
                                                ent->client->trace_object = tr.ent;
                                                ent->gravity = 0.001;
                                                VectorCopy (tr.ent->s.origin, ent->s.origin);
                                        }
                                } else {
                                        if (ent->client->trace_object->inuse) {
                                                VectorCopy (ent->s.origin, ent->client->trace_object->s.origin);
                                                ent->client->trace_object->gravity = 0.001;
                                                VectorClear(ent->client->trace_object->velocity);
                                                gi.linkentity (ent->client->trace_object);
                                        } else {
                                                ent->client->trace_object = NULL;
                                        }
                                }
                        } else {
                                if (ent->client->trace_object) {
                                        vec3_t down = {0,0,8};
                                        gi.linkentity (ent->client->trace_object);
                                        ent->client->trace_object->groundentity = NULL;
                                        ent->client->trace_object->gravity = 1;
                                        VectorAdd(ent->velocity, down, ent->client->trace_object->velocity);
                                        ent->client->trace_object = NULL;
                                }
                        }
                } else if (ent->client->trace_object && (ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) {
                        VectorCopy (ent->s.angles, ent->client->trace_object->s.angles);
                }
        }

        if(ent->client->resp.team == TEAM_ALIEN || ent->client->resp.class_type == CLASS_BREEDER)
        {
                // aliens don't have normal weapons
                AlienWeaponThink(ent);
        }
        else if (ent->client->resp.team == TEAM_HUMAN) 
        {
                if(ent->client->resp.class_type == CLASS_MECH)
                {
                        MechWeaponThink(ent);
                }
                else if(ent->client->resp.class_type == CLASS_ENGINEER)
                {
                        if (ent->client->buttons & BUTTON_ATTACK) {

                                if (ent->groundentity && ent->groundentity->client && ent->groundentity->client->resp.team == TEAM_HUMAN && ent->groundentity->health > 0) {
                                        vec3_t  forward;
                                        AngleVectors (ent->s.angles, forward, NULL, NULL);
                                        VectorScale(forward, 300, forward);
                                        forward[2] = 20;
                                        VectorAdd (ent->groundentity->velocity, forward, ent->groundentity->velocity);
                                }

                                if(ent->client->anim_priority > ANIM_JUMP)
                                        return;

                                engineer_fix (ent, 96);
                        }
                }
                else if ( ent->client->buttons & BUTTON_USE) 
                {
                        if (ent->client->resp.turret) 
                        {
                                ent->client->resp.turret = NULL;
                        } 
                        else if(ent->client->resp.class_type == CLASS_EXTERM) 
                        {
                                ent->random+=2;
                                if(ent->client->last_move_time>=level.time)
                                ent->random=0;
                                if(ent->random >= ent->client->resp.inventory[ent->client->ammo_index]) {
                                  ent->random=ent->client->resp.inventory[ent->client->ammo_index];
                                  if (ent->client->resp.inventory[ent->client->ammo_index] > 4) Cmd_Discharge_f (ent);
                                }
                        } 
                        else if(ent->client->resp.class_type == CLASS_BIO) 
                        {
                                biotech_heal(ent,100);
                                if (ent->client->resp.bioScore >= 1500) 
                                {
                                        ent->client->resp.total_score += 10;
                                        ent->client->resp.score ++;
                                        if (ent->client->resp.score > MAX_SCORE->value) 
                                        {
                                                ent->client->resp.score = MAX_SCORE->value;
                                                ent->client->resp.total_score += 5;
                                        }
                                        ent->client->resp.bioScore = 0;
                                }
                        }
                }
                else if(ent->client->resp.class_type == CLASS_EXTERM) 
                {
                        if(ent->random&&ent->client->last_move_time<level.time&&ent->client->resp.inventory[ent->client->ammo_index]>0)
                        Cmd_Discharge_f (ent);
                        else
                                ent->random=0;
                } 

                
/*              if (ent->client->resp.class_type == CLASS_BIO)
                        biotech_magic_thingy (ent);*/

                //handled elsewhere (huh?)
                // turret think handles firing from the owner..
                if (ent->client->resp.turret)
                        return;
        }

        // call active weapon think routine
        if (ent->client->weapon && ent->client->weapon->weaponthink)
                ent->client->weapon->weaponthink (ent);
}

/*
Use_Weapon

Make the weapon ready if there is ammo
*/
void Use_Weapon (edict_t *ent, const gitem_t *item)
{
        int                     found = 0, i;
        const gitem_t           *it;

        if (ent->client->resp.team != TEAM_HUMAN)               // aliens can't use guns
                return;

        // see if we're already using it
        if (item == ent->client->weapon && ent->client->weaponstate != WEAPON_RELOADING)
                return;
                
        // FIXME: change selected_item to clips on use weapon

        // this condition looks suspicious -ank
        if (!g_select_empty->value && !(item->flags & IT_AMMO))
        {
                // find existing ammo for this type of weapon
                it = itemlist;

                for (i=0 ; i<game.num_items ; i++, it++)
                {
                        // is a ammo for this item?
                        if (it->flags & IT_AMMO && it->clip_type == item->clip_type && ent->client->resp.inventory[i]) {
                                found = 1;
                                break;
                        }

                        // is a clip for this item?
                        if (it->flags & IT_CLIP && it->clip_type == item->clip_type && ent->client->resp.inventory[i]) {
                                found = 1;
                                break;
                        }
                }

                if(!found)
                {
                        gi.cprintf (ent, PRINT_HIGH, "You don't have any ammo for the %s.\n", item->pickup_name);
                        return;
                }
        }

        // change to this weapon when down
        ent->client->newweapon = (item != ent->client->weapon)?item:NULL;
}

/*
Weapon_Generic

A generic function to handle the basics of weapon thinking
*/
#define FRAME_FIRE_FIRST                (FRAME_ACTIVATE_LAST + 1)
#define FRAME_RELOAD_FIRST              (FRAME_FIRE_LAST + 1)
#define FRAME_IDLE_FIRST                (FRAME_RELOAD_LAST + 1)
#define FRAME_DEACTIVATE_FIRST  (FRAME_IDLE_LAST + 1)

static void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_RELOAD_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
        int             n;

        // ### Hentai ### BEGIN
        if(ent->health < 1 || ent->s.modelindex != 255) // VWep animations screw up corpses
        {
                return;
        }
        // ### Hentai ### END

        //gi.bprintf (PRINT_HIGH, "recoil = %d\n", ent->client->machinegun_shots);

        if (ent->client->weaponstate == WEAPON_RELOADING)
        {
                if(ent->client->ps.gunframe > FRAME_RELOAD_LAST || ent->client->ps.gunframe < FRAME_RELOAD_FIRST)
                        ent->client->ps.gunframe = FRAME_RELOAD_FIRST;
                else if (ent->client->ps.gunframe == FRAME_RELOAD_LAST)
                {
                        ent->client->machinegun_shots = 0;
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = FRAME_IDLE_FIRST;
                }

                ent->client->ps.gunframe++;
                return;
        }

        if (ent->client->weaponstate == WEAPON_DROPPING)
        {
                if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
                {
                        ChangeWeapon (ent);
                        return;
                }

                // hack for ht, dropping of empty rocket launcher
                if (ent->client->ps.gunframe == 27 && !Q_stricmp (ent->client->weapon->pickup_name, "Rocket Launcher"))
                {
                        ChangeWeapon (ent);
                        return;
                }

                ent->client->ps.gunframe++;
                return;
        }

        if (ent->client->weaponstate == WEAPON_ACTIVATING)
        {
                if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
                {
                        //if(ent->client->resp.inventory[ent->client->ammo_index] <= 0 && FindAmmoForGun(ent) != -1)

                        // no ammo on activated weapon? try reloading.. (g_select_empty code?)
                        if(ent->client->resp.inventory[ent->client->ammo_index] <= 0)
                        {
                                Auto_Reload(ent);
                                if (ent->client->weaponstate == WEAPON_RELOADING) {
                                        ent->client->ps.gunframe = FRAME_RELOAD_FIRST;
                                        return;
                                }
                        }

                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe = FRAME_IDLE_FIRST;
                        return;
                }

                ent->client->ps.gunframe++;
                return;
        }

        if (ent->client->machinegun_shots > 25)
                ent->client->machinegun_shots = 25;

        if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
        {
                ent->client->weaponstate = WEAPON_DROPPING;
                ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
                return;
        }

        if (ent->client->weaponstate == WEAPON_READY)
        {
                if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
                {
                        //if (ent->client->resp.class_type == CLASS_SHOCK && ent->client->last_reload_time + 1.6 > level.time)
                        //      return;
                        
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;

                        if (ent->client->ammo_index)
                        {
                                ent->client->ps.gunframe = FRAME_FIRE_FIRST;
                                ent->client->weaponstate = WEAPON_FIRING;

                                // start the animation
                                ent->client->anim_priority = ANIM_ATTACK;
                                if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                                {
                                        ent->s.frame = FrameReference(ent, FRAME_ATTACK2_S)-1;
                                        ent->client->anim_end = FrameReference(ent, FRAME_ATTACK2_E);
                                }
                                else
                                {
                                        ent->s.frame = FrameReference(ent, FRAME_ATTACK1_S)-1;
                                        ent->client->anim_end = FrameReference(ent, FRAME_ATTACK1_E);
                                }
                        }
                        /*else
                        //{
                                //if (level.time >= ent->pain_debounce_time)
                                //{
                                        //gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_noammo), 1, ATTN_NORM, 0);
                                        //ent->pain_debounce_time = level.time + 1;
                                //}
                                //NoAmmoWeaponChange (ent);
                        }*/
                }
                else
                {
                        if (ent->client->machinegun_shots)
                        {
                                if (ent->client->resp.class_type == CLASS_COMMANDO)
                                        ent->client->machinegun_shots -= 4;
                                else
                                        ent->client->machinegun_shots -= 2;
                                if (ent->client->machinegun_shots < 0)
                                        ent->client->machinegun_shots = 0;
                        }
                        
                        if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
                        {
                                ent->client->ps.gunframe = FRAME_IDLE_FIRST;
                                return;
                        }

                        if (pause_frames)
                        {
                                for (n = 0; pause_frames[n]; n++)
                                {
                                        if (ent->client->ps.gunframe == pause_frames[n])
                                        {
                                                if (randomMT()&15)
                                                        return;
                                        }
                                }
                        }

                        ent->client->ps.gunframe++;
                        return;
                }
        }

        if (ent->client->weaponstate == WEAPON_FIRING)
        {
                for (n = 0; fire_frames[n]; n++)
                {
                        if (ent->client->ps.gunframe == fire_frames[n])
                        {
                        	int oldammo;
                                //if (ent->client->quad_framenum > level.framenum)
                                        //gi.sound(ent, CHAN_AUTO, SoundIndex (items_damage3), 1, ATTN_NORM, 0);

                                if (ent->client->invincible_framenum > level.framenum)
                                        ent->client->invincible_framenum = level.framenum;
                                
                                
                                if (ent->client->ammo_index)
                                  oldammo = ent->client->resp.inventory[ent->client->ammo_index];
                                practice_target_hits = 0;

                                fire (ent);

                                if (ent->client->ammo_index && oldammo > ent->client->resp.inventory[ent->client->ammo_index]) {
                                  ent->client->resp.target_shots++;
                                  if (practice_target_hits)
                                    ent->client->resp.target_hits++;
                                }

                                break;
                        }
                }

                if (!fire_frames[n])
                {
                        if (ent->client->machinegun_shots)
                        {
                                ent->client->machinegun_shots -= 3;
                                if (ent->client->machinegun_shots < 0)
                                        ent->client->machinegun_shots = 0;
                        }
                        ent->client->ps.gunframe++;
                }

                if (ent->client->ps.gunframe == FRAME_RELOAD_FIRST && ent->client->weaponstate == WEAPON_FIRING)
                {
                        ent->client->ps.gunframe = FRAME_IDLE_FIRST;
                        ent->client->weaponstate = WEAPON_READY;
                }
        }
}

void debris_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

static void ThrowCasing (edict_t *self, char *modelname, vec3_t veloc, vec3_t origin)
{
        edict_t *chunk;

        if(bandwidth_mode->value < 2.0)
                return;

        chunk = G_Spawn();
        VectorCopy (origin, chunk->s.origin);
        gi.setmodel (chunk, modelname);
        chunk->movetype = MOVETYPE_BOUNCE;
        chunk->solid = SOLID_NOT;
        VectorCopy(veloc, chunk->velocity);
        chunk->velocity[2] *= (random()*0.1f + 0.9f);
        VectorCopy(self->s.angles, chunk->s.angles);
        chunk->think = G_FreeEdict;
        chunk->nextthink = level.time + 0.7f;
        chunk->s.frame = 0;
        chunk->flags = 0;
        chunk->classname = "debris";
        chunk->takedamage = DAMAGE_YES;
        chunk->die = debris_die;
        chunk->gravity = 0.5;
        gi.linkentity (chunk);
}

/*void StingerFire (edict_t *ent)
{
        vec3_t          start;
        vec3_t          forward, right;
        vec3_t          offset;
        int                     damage = 25;

        if(ent->client->resp.inventory[ent->client->ammo_index] <= 0)
        {
                return;
        }
        ent->client->resp.inventory[ent->client->ammo_index]--;

        // limit sounds
        if (ent->fly_sound_debounce_time < level.framenum) {
                gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_flame), 1, ATTN_IDLE, 0);
                ent->fly_sound_debounce_time = level.framenum + 4;
        }

        AngleVectors (ent->client->v_angle, forward, right, NULL);
//40
        VectorSet(offset, 20, 16,  ent->viewheight+4);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        forward[0] += crandom()*0.1f;
        forward[1] += crandom()*0.1f;
        forward[2] += crandom()*0.1f;

        fire_fire(ent, start, forward, damage);

//      PlayerNoise(ent, start, PNOISE_WEAPON);
}*/

void StingerFire (edict_t *ent)
{
        vec3_t          start;
        vec3_t          forward, right;
        vec3_t          offset;
        int                     damage = 20;

        if(ent->client->resp.inventory[ent->client->ammo_index] <= 0)
        {
                return;
        }

        // limit sounds
        if (ent->fly_sound_debounce_time < level.framenum) {
                gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_flame), 1, ATTN_IDLE, 0);
                ent->fly_sound_debounce_time = level.framenum + 4;
        }

        ent->client->resp.inventory[ent->client->ammo_index]--;

        AngleVectors (ent->client->v_angle, forward, right, NULL);
//40
        VectorSet(offset, 20, 16,  ent->viewheight+4);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        fire_fire(ent, start, forward, damage);

//      PlayerNoise(ent, start, PNOISE_WEAPON);
}

//static void InfestBody (edict_t *e, edict_t *infester);
void InfestGasThink(edict_t *self)
{
        // if owner isn't valid or player isn't guardian, infest becomes independant
        if (self->owner && (!self->owner->inuse || (self->owner->client && self->owner->client->resp.class_type != CLASS_GUARDIAN)))
        {
                self->owner = self;
        }

        //r1ch - reduced gas damage (Was 15)
        if ((self->s.frame & 1) == 0) {
                T_InfestDamage (self, self->owner, 24.0, self, 118.0, MOD_INFEST, 0);
        }

        //r1: game spawned
        if (self->spawnflags & 1024)
        {
                //r1: expire after 20 mins
                if (self->count++ == 12000)
                {
                        gi.sound (self, CHAN_AUTO, SoundIndex (misc_udeath), 0.5, ATTN_IDLE, 0);
                        //IntelligentThrowGib (self, sm_meat_index, 100, GIB_ORGANIC, 2, 5);
                        ThrowGib (self, sm_meat_index, 100, GIB_ORGANIC);
                        ThrowGib (self, sm_meat_index, 100, GIB_ORGANIC);
                        SpawnDamage (TE_BLOOD, self->s.origin, vec3_origin, 100);
                        G_FreeEdict (self);
                        return;
                }
        }

        //r1: unneeded
        /*if (level.framenum % 300 == 0) {
                edict_t *e=NULL;
                while ((e = findradius (e, self->s.origin, 200)) != NULL) {
                        if (e->enttype == ENT_HUMAN_BODY) {
                                if (random() < 0.2) {
                                        InfestBody (e, self->owner);
                                }
                        }
                }
        }*/

        if (self->spawnflags & 4)
                if(++self->s.frame > 6)
                        self->s.frame = 0;

        self->nextthink = level.time + 0.1f;
}

void infest_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        gi.sound (self, CHAN_AUTO, SoundIndex (misc_udeath), 1, ATTN_IDLE, 0);
        IntelligentThrowGib (self,self->headgib, damage, GIB_ORGANIC, 1, 4);
        //ThrowGib (self, self->headgib, damage, GIB_ORGANIC);

        if (bandwidth_mode->value)
                ThrowGib (self, self->gib1, damage, GIB_ORGANIC);
        if (bandwidth_mode->value==2)
                ThrowGib (self, self->gib2, damage, GIB_ORGANIC);

        if (attacker && attacker->client && attacker->client->resp.team == TEAM_HUMAN)
                attacker->client->resp.total_score++;

        G_FreeEdict (self);
}

void InfestCreateThink (edict_t *ent)
{
   if (ent->count++ > 4)
        {
                gi.sound (ent, CHAN_AUTO, SoundIndex (misc_udeath), 1, ATTN_IDLE, 0);
                ent->health = INFEST_HEALTH;
                ent->count = 0;
                ent->damage_absorb = 8;
                ent->think = InfestGasThink;
                ent->nextthink = level.time + 0.1f;
                ent->spawnflags |= 1024;
                //IntelligentThrowGib (ent, ent->headgib, 50, GIB_ORGANIC, 2, 4);
                ThrowGib (ent, ent->gib1, 50, GIB_ORGANIC);
                ThrowGib (ent, ent->headgib, 50, GIB_ORGANIC);
                ThrowGib (ent, ent->headgib, 50, GIB_ORGANIC);
                ThrowGib (ent, ent->gib2, 50, GIB_ORGANIC);
                ent->s.frame =  ent->s.skinnum = 0;
                ent->classname = "infest";
                ent->s.modelindex = gi.modelindex ("models/objects/infested/tris.md2");
                
                return;
        }
        IntelligentThrowGib (ent, ent->headgib, 50, GIB_ORGANIC, 1, 2);
        ent->nextthink = level.time + 1.0f + random() * 0.4f;   
}

void CorruptedMineThink (edict_t *self)
{
        spawn_spark(self);
        // Make the tripwire appear to disconnect now and then
        if (self->owner->s.frame > 0) {
            self->owner->s.frame = 0;
            self->nextthink = level.time + 0.1f;
        }
        else {
            self->owner->s.frame = 4;
            self->nextthink = level.time + 3.5f + random() * 0.4f;
        }
}

void CorruptMine (edict_t *mine, edict_t *infester)
{
    if (infester->upgrade_time < level.time) {
        // Some effects
        gi.sound (infester, CHAN_AUTO, SoundIndex (weapons_acid), 0.6, ATTN_IDLE, 0);
        gi.sound (infester, CHAN_AUTO, gi.soundindex("parasite/Paratck3.wav"), 0.6, ATTN_NORM, 0);
        temp_splash (TE_SPLASH, 24, mine->s.origin, mine->owner->movedir, SPLASH_SLIME);
        spawn_spark(mine);

        // Make the corrupted mine to appear red in IR vision (bio and aliens)
        mine->s.renderfx |= RF_IR_VISIBLE;
        mine->spawnflags |= SPAWNFLAG_CORRUPTED_MINE;
        mine->health = 60; // Reduced health
        mine->think = CorruptedMineThink;
        mine->nextthink = level.time + 4.0f + random() * 0.4f;
        gi.linkentity(mine);
        infester->upgrade_time = level.time + 8;
    }
    else gi.cprintf (infester,PRINT_MEDIUM,"You need to rest %.1f seconds before corrupting another mine.\n", infester->upgrade_time - level.time);
}

void InfestBody (edict_t *body, edict_t *infester)
{
        // FIXME: uncomment this in next client update!!
        // FIXME: make misc_infestation to have such selector

        //if (body->enttype == ENT_FEMALE_BODY) 
        //      body->s.modelindex = gi.modelindex ("models/objects/infested2/tris.md2");
        //else
                //body->s.modelindex = gi.modelindex ("models/objects/infested/tris.md2");

        gi.sound (body, CHAN_AUTO, SoundIndex (organ_growegg), 1.0, ATTN_STATIC, 0);

        body->s.effects = EF_FLIES;

        //body->health = INFEST_HEALTH;
        //body->damage_absorb = 8;

        body->owner = infester;

        body->enttype = ENT_INFEST;

        body->solid = SOLID_BBOX;
        body->movetype = MOVETYPE_TOSS;
        body->svflags = SVF_DEADMONSTER;
        body->spawnflags = 4;

        body->die = infest_die;
        body->takedamage = DAMAGE_YES;
        body->think = InfestCreateThink;
        body->nextthink = level.time + 1.0f;

        body->s.renderfx |= RF_IR_VISIBLE;

        IntelligentThrowGib (body, body->gib1, 50, GIB_ORGANIC, 1, 2);
        IntelligentThrowGib (body, body->gib2, 50, GIB_ORGANIC, 0, 2);

        gi.linkentity (body);
}

void C4_Timer(edict_t *self);
void GuardianInfest(edict_t *self)
{
        edict_t *e;
        e = NULL;
        int d;

        self->s.modelindex = 255;
        self->client->last_move_time = level.time;

        while ((e = findradius(e, self->s.origin, 70)) != NULL)
        {
        	if (((int)dmflags->value & DF_PARASITE) && e->client && e->health > 0) {
        	  switch (e->client->resp.class_type) {
        	    case CLASS_GRUNT:
        	    case CLASS_SHOCK:
        	    case CLASS_HEAVY:
        	    case CLASS_COMMANDO:
        	    case CLASS_BIO:
        	      T_Damage(e, self, self, vec3_origin, self->s.origin, vec3_origin, 2, 0, DAMAGE_NO_ARMOR, MOD_PARASITE);
        	      if (e->client->resp.class_type != CLASS_BIO && !e->client->resp.parasited)
        	        e->client->resp.parasited = self-g_edicts;
        	  }
        	} else
                switch (e->enttype) {
                        case ENT_HUMAN_BODY:
                        case ENT_FEMALE_BODY:
                                self->client->resp.total_score++;
                                InfestBody (e, self);
                                break;
                        case ENT_TRIPWIRE_BOMB:
                                if ((corrupted_tripwires->value) && !(e->spawnflags & SPAWNFLAG_CORRUPTED_MINE)) {
                                     // Calculate distance
                                     d = pow((e->s.origin[0]-self->s.origin[0]),2) + pow((e->s.origin[1]-self->s.origin[1]),2) +
                                         pow((e->s.origin[2]-self->s.origin[2]),2);
                                     // If close enough, complete the evil deeds
                                     if (d < 1400) {
                                         self->client->resp.total_score++;
                                         CorruptMine (e, self);
                                     }
                                }
                                break;
                        case ENT_C4:
                                //for the thought.
                                self->client->resp.total_score += 15;

                                //we want this before we return...
                                gi.sound(self, CHAN_AUTO, SoundIndex (weapons_chomp), 1, ATTN_NORM, 0);

                                //c4 revenge can give alien tk as human frags if overrides are enabled                                
                                if (!self->client->acid_attacker || !self->client->acid_attacker->inuse || !self->client->acid_attacker->client || self->client->acid_attacker->client->resp.team != TEAM_HUMAN)
                                  self->client->acid_attacker = e->owner;

                                //r1: just made eating it stop it. toasty was fun for a while, but...
                                self->client->resp.score ++;

                                if (self->client->resp.score > MAX_SCORE->value)
                                        self->client->resp.score = MAX_SCORE->value;

                                self->dmg = 180;
                                self->radius_dmg = 180;

                                //kamikaze_explode (self, MOD_EAT_C4, 0);
                                
                                //G_FreeEdict(e->teamchain);
                                //G_FreeEdict(e);
                                
                                //return;

                                //eating multiple doesn't skew timer
                                //if (!self->client->grenade_blow_up)
                                self->client->grenade_time = level.time + e->count + 1;

                                G_FreeEdict(e->teamchain);
                                G_FreeEdict(e);

                                self->client->grenade_blow_up++;

                                //self->dmg = 300 + random()*100;
                                //self->radius_dmg = 400;
                                break;
                }
        }
}

void StalkerSpike (edict_t *ent)
{
        int     i;
        vec3_t          start, end;
        vec3_t          forward, right, up;
        vec3_t          angles;
        int                     damage = 75;
//      int                     kick = 8;
        vec3_t          offset;
        trace_t         tr;

        if(ent->client->resp.inventory[ent->client->ammo_index] <= 1)
        {
                return;
        }
        ent->client->resp.inventory[ent->client->ammo_index]-=2;

        for (i = 0; i < 2; i++)
        {
                ent->client->kick_origin[i] = crandom() * 0.35f;
                ent->client->kick_angles[i] = crandom() * 0.7f;
        }
        // get start / end positions
        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        AngleVectors (angles, forward, right, up);
        VectorSet(offset, 0, 0, ent->viewheight+8);             //ent->viewheight-8

        G_ProjectSource (ent->s.origin, offset, forward, right, start);
        VectorCopy(start, end);
        VectorSet(offset, 2048, 0, 0);
        G_ProjectSource (start, offset, forward, right, end);

        tr=gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

        VectorScale(right, 16, right);
        VectorAdd(start, right, start);

        VectorSubtract(tr.endpos, start, forward);
        fire_spike (ent, start, forward, damage, 1000);

        VectorSubtract(start, right, start);
        VectorSubtract(start, right, start);
        VectorSubtract(tr.endpos, start, forward);
        fire_spike (ent, start, forward, damage, 1000);
}

void MechFire (edict_t *ent, float off)
{
        int     i;
        vec3_t          start;
        vec3_t          forward, right, up;
        vec3_t          angles;
        int                     damage = 40;
//      int                     kick = 8;
        vec3_t          offset;

        if (ent->wait > level.time)
                return;

        if(ent->client->resp.inventory[ent->client->ammo_index] <= 0)
        {
                gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_noammo), 1, ATTN_IDLE, 0);
                return;
        }
        ent->client->resp.inventory[ent->client->ammo_index]--;

        for (i=0 ; i < 2 ; i++)
        {
                ent->client->kick_origin[i] = crandom() * 0.35f;
                ent->client->kick_angles[i] = crandom() * 0.7f;
        }
        // get start / end positions
        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        AngleVectors (angles, forward, right, up);

        VectorSet(offset, 0, 0, 38);

        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        VectorScale(right, 16*off, right);
        VectorAdd(start, right, start);

	if (SETTING_INT(mech_rockets) != 5) {
	  fire_blaster(ent,start, forward, damage, 1800, EF_BLASTER, ENT_BLASTER);
	} else {
          edict_t*bolt = fire_projectile(ent, start, forward, ENT_BLASTER, 1800, mech_blaster_touch);
          bolt->classname = "mechblaster";
          bolt->s.modelindex = gi.modelindex("models/objects/laser/tris.md2");
          bolt->s.sound = SoundIndex(misc_lasfly);
          bolt->s.effects = EF_BLASTER;
	}

        //fire_drunk_missile (ent, start, forward, damage, 1500, 130, 90);
#ifdef CACHE_CLIENTSOUNDS
        gi.sound(ent, CHAN_WEAPON, SoundIndex(mech_shot), 1, ATTN_IDLE, 0);
#else
        gi.sound(ent, CHAN_WEAPON, gi.soundindex("*shot.wav"), 1, ATTN_IDLE, 0);
#endif
}

void kamikaze_explode(edict_t *self, int mod, int dflags)
{
        //prevent so-called double kami bug
        if (self->health < 1) {
                gi.dprintf ("warning, kami tried to explode when dead!\n");
                return;
        }

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_EXPLOSION1);
        gi.WritePosition (self->s.origin);
        gi.multicast (self->s.origin, MULTICAST_PVS);

        //we need to be really dead before triggering mines etc
        //T_Damage (self, self, self, vec3_origin, self->s.origin, vec3_origin, 1000, 0, DAMAGE_NO_PROTECTION | dflags, MOD_CHANGECLASS); //Cannot use MOD_KAMIKAZE since that requires radius explosion if overriden
        self->health = 0;
        meansOfDeath = MOD_KAMIKAZE;
        self->die(self, self, self, 1000, self->s.origin);

        T_RadiusDamage(self, self, self->dmg, NULL, self->dmg_radius, mod, dflags);

        //r1: double check, we should be dead
        if (self->health > 0)
        {
                gi.dprintf ("warning, a kami didn't die after exploding. health = %d\n", self->health);
                T_Damage (self, self, self, vec3_origin, self->s.origin, vec3_origin, 1000, 0, DAMAGE_NO_PROTECTION | dflags, MOD_CHANGECLASS);
        }
}

void DroneWeb(edict_t *ent)
{
        vec3_t          angles, forward;

        if(ent->client->resp.inventory[ent->client->ammo_index] > 0)
                ent->client->resp.inventory[ent->client->ammo_index]--;
        else 
                return;

        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        AngleVectors (angles, forward, NULL, NULL);

        //was 550
        //if (ent->client->resp.upgrades & UPGRADE_MUCUS_GLANDS)
        //      fire_web (ent, ent->s.origin, forward, 800);
        //else
        fire_web (ent, ent->s.origin, forward, 800);

        gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_webshot1), 1, ATTN_IDLE, 0);

}
// BRIAN: Wraith also uses this, so does stalker and stinger?
void DroneSlash(edict_t *ent, int damage, int kick, int range)
{
        vec3_t          angles, forward, start;

        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        VectorCopy (ent->s.origin, start);
        start[2] += ent->viewheight;
        AngleVectors (angles, forward, NULL, NULL);

        if(fire_melee(ent, start, forward, damage, kick, range))
                gi.sound(ent, CHAN_AUTO, SoundIndex (mutant_mutatck2), 1, ATTN_NORM, 0);
        else
                gi.sound(ent, CHAN_AUTO, SoundIndex (mutant_mutatck1), 1, ATTN_NORM, 0);
}

void WraithSpit(edict_t *ent)
{
        vec3_t          angles, forward, start;

        if (ent->client->resp.inventory[ent->client->ammo_index]<=0)

                return;
        else
                ent->client->resp.inventory[ent->client->ammo_index]--;

        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        AngleVectors (angles, forward, NULL, NULL);     
        VectorCopy (ent->s.origin, start);
        //start[2]+=ent->viewheight;

        gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_webshot1), 1, ATTN_IDLE, 0);
        //fire_wraith_acid (ent, start, forward, 55);
	forward[2] += 0.004f;
        fire_acid (ent, start, forward, 650);
}

void GuardianSlash(edict_t *ent, int damage, float range, int offset)
{
        vec3_t          angles, forward, start, right;

        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        VectorCopy (ent->s.origin, start);


        start[2] += ent->viewheight;

        AngleVectors (angles, NULL, right, NULL);

        angles[1] += 15;
        AngleVectors (angles, forward, NULL, NULL);

        VectorScale(right, (float)offset, right);
        VectorAdd(start, right, start);


        if(fire_melee(ent, start, forward, damage, damage, range))
                gi.sound(ent, CHAN_AUTO, SoundIndex (mutant_mutatck2), 1, ATTN_NORM, 0);
        else
                gi.sound(ent, CHAN_AUTO, SoundIndex (mutant_mutatck1), 1, ATTN_NORM, 0);

        angles[1] -= 30;
        AngleVectors (angles, forward, NULL, NULL);

        VectorScale(right, -2, right);
        VectorAdd(start, right, start);

        if(fire_melee(ent, start, forward, damage, damage, range))
                gi.sound(ent, CHAN_AUTO, SoundIndex (mutant_mutatck2), 1, ATTN_NORM, 0);
        else
                gi.sound(ent, CHAN_AUTO, SoundIndex (mutant_mutatck1), 1, ATTN_NORM, 0);
}

static void Weapon_RocketLauncher_Fire (edict_t *ent)
{
        vec3_t  offset, start;
        vec3_t  forward, right, recoil;
        int             damage;
        float           damage_radius;
        int             radius_damage;
        float           xyspeed = sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

        // run reload or drop anim after firing
        if(ent->client->ps.gunframe == 19)
        {
                if(ent->client->resp.inventory[ent->client->ammo_index] <= 0)
                {
                        ent->client->weaponstate = WEAPON_DROPPING;
                        // hacked because of model issues, look at weapon_generic

                        ent->client->ps.gunframe = 20; // end at 27
                        ent->client->newweapon = FindItem ("Pistol");

                } else {
                        // play reload anim
                        ent->client->weaponstate = WEAPON_RELOADING;

                        // play reload sound
            gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_misload), 0.8, ATTN_IDLE, 0);
                }

                return;
        }

        if (xyspeed > 150) {
                ent->client->missile_target = 0;
                return;
        }

        if (ent->client->missile_target < 9)
        {
                if (!(ent->client->buttons & BUTTON_ATTACK))
                {
                        ent->client->weaponstate = WEAPON_READY;
                        ent->client->ps.gunframe=60;
                        ent->client->missile_target=0;
                        return;
                }

                ent->client->missile_target++;

                //if((float)(ent->client->missile_target/3) == (float)ent->client->missile_target/3)
                if (ent->client->missile_target % 3 == 0)
                {
                        edict_t *cl;
                        vec3_t  dest;
                        trace_t tr;
                        
                        //match rocket shot trace size
                        vec3_t mins = {-1, -1, -1};
                        vec3_t maxs = {+1, +1, +1};


                        AngleVectors (ent->client->v_angle, forward, right, NULL);

                        //VectorScale (forward, -2, ent->client->kick_origin);

                        VectorSet(offset, 8, 8, ent->viewheight-8);
                        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
                        VectorMA (start, 2048, forward, dest);

                        tr = gi.trace (start, mins, maxs, dest, ent, MASK_SOLID);

                        if (!(ent->client->resp.upgrades & UPGRADE_STEALTH_PLATE))
                                gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_targbeep), 1, ATTN_NORM, 0);

                        for (cl = globals.edicts + 1; cl < globals.edicts + 1 + game.maxclients; cl++)
                        {
                                if (!cl->inuse)
                                        continue;

                                if (cl == ent) {
//                                  gi.WriteByte(svc_temp_entity);
//                                  gi.WriteByte(TE_HEATBEAM_SPARKS);
//                                  gi.WriteByte(192);
//                                  gi.WritePosition(start);
//                                  gi.WritePosition(tr.endpos);
//                                  gi.WriteDir(tr.plane.normal);
//                                  gi.WriteByte(192);
//                                  gi.WriteShort(1);
                                  gi.unicast(cl, false);
                                  continue;
                                }

                                if (cl->client->resp.team == ent->client->resp.team || cl->client->resp.team == TEAM_NONE)
                                {
                                        gi.WriteByte (svc_temp_entity);
                                        gi.WriteByte (TE_BFG_LASER);
                                        //gi.WriteShort (g_edicts - ent);
                                        gi.WritePosition (start);
                                        gi.WritePosition (tr.endpos);
                                        gi.unicast (cl, false);
                                }
                        }
                        //else
                                //unicastSound (ent, SoundIndex (weapons_targbeep), 1);
                }

                        // Stay on first attack frame
                if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                        ent->s.frame = FrameReference(ent, FRAME_ATTACK2_S);
                else
                        ent->s.frame = FrameReference(ent, FRAME_ATTACK1_S);

                return;
        }

        damage = 375;
        radius_damage = 380;
        damage_radius = 250;

        AngleVectors (ent->client->v_angle, forward, right, NULL);

        VectorScale (forward, -2, ent->client->kick_origin);
        ent->client->kick_angles[0] = -15;
        ent->client->kick_angles[1] = -15;

        VectorSet(offset, 8, 8, ent->viewheight-8);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        if (!(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
        {
                T_Damage(ent ,ent, ent, ent->velocity, ent->s.origin, vec3_origin, random()*30+70, 5, 0, MOD_R_SPLASH);
                if (!ent->groundentity)
                        T_Damage(ent, ent, ent, ent->velocity, ent->s.origin, vec3_origin, random()*40+80, 5, 0, MOD_R_SPLASH);
                ent->client->fall_value = 40;
                ent->client->fall_time = level.time + .5f;
                VectorScale(forward, -500, recoil);
                recoil[2] += 300;
                VectorAdd(ent->velocity, recoil, ent->velocity);
                forward[0] += (crandom()*0.35f);
                forward[1] += (crandom()*0.35f);
                forward[2] += (crandom()*0.35f);
                VectorNormalize(forward);
        }

        // send muzzle flash
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_ROCKET);
        gi.multicast (ent->s.origin, MULTICAST_PVS);



        ent->client->ps.gunframe++;

//      PlayerNoise(ent, start, PNOISE_WEAPON);

        ent->client->missile_target = 0;

        ent->client->resp.inventory[ent->client->ammo_index]--;

        fire_rocket (ent, start, forward, damage, 750, damage_radius, radius_damage);
}

void Weapon_RocketLauncher (edict_t *ent)
{
        static int      pause_frames[]  = {60, 67, 0};
        static int      fire_frames[]   = {10, 19, 0};

        Weapon_Generic (ent, 9, 19, 59, 77, 87, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}


static void Autogun_Fire (edict_t *ent)
{
        int     i;
        vec3_t          start;
        vec3_t          forward, right, up, case_start;
        vec3_t          angles;
        int                     spread;
        int                     damage = 20;
        int                     kick = 8;
        vec3_t          offset;

        //if (level.time - ent->client->next_drown_time < 0)
        //      return;

        if(ent->client->resp.inventory[ent->client->ammo_index] <= 0)
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe=57;

                ent->target_ent = NULL;

                //gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_noammo), 1, ATTN_NORM, 0);

                Auto_Reload(ent);
                return;
        }

        if (!(ent->client->buttons & BUTTON_ATTACK) || (ent->client->resp.class_type == CLASS_GRUNT && (ent->client->resp.upgrades & UPGRADE_STIMPACKS)))
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe=57;

                if (!(ent->client->resp.class_type == CLASS_GRUNT && (ent->client->resp.upgrades & UPGRADE_STIMPACKS)))
                {
                        if (ent->client->machinegun_shots < 3)
                                        return;
                }
                //ent->client->next_drown_time = level.time + .1;
        }

        ent->client->ps.gunframe++;
        
        /*ent->client->machinegun_shots++;

        for (i=0 ; i<3 ; i++)
        {
                ent->client->kick_origin[i] = crandom() * 0.35;

                if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                        ent->client->kick_angles[i] = crandom() * (float)ent->client->machinegun_shots * 0.2f;
                else
                        ent->client->kick_angles[i] = crandom() * (float)ent->client->machinegun_shots * 0.8f;

                if (ent->client->machinegun_shots <= 5)
                        ent->client->kick_angles[i] *= 0.66f;
        }*/

        for (i=1 ; i<3 ; i++)
        {
                ent->client->kick_origin[i] = crandom() * 0.35f;
        }
        
        if (recoil->value)
        {
                ent->client->kick_angles[0] = -random() * pow(ent->client->machinegun_shots / 4.0f, 2) / 3;
                ent->client->kick_angles[1] = crandom() * pow(ent->client->machinegun_shots / 4.0f, 2) / 14;
                ent->client->kick_angles[2] = crandom() * 0.7f;

                // raise the gun as it is firing
                ent->client->machinegun_shots += 2;
                if (ent->client->machinegun_shots > 24)
                        ent->client->machinegun_shots = 24;
                spread = 50 + 4 * pow(ent->client->machinegun_shots / 4, 2);
        }
        else
        {
                spread = 50;
        }

        /*ent->client->machinegun_shots++;
        if (ent->client->machinegun_shots == 5) {
                ent->client->machinegun_shots = 0;
                ent->client->next_drown_time = level.time + 0.3;
        }*/
        ent->client->kick_origin[0] = crandom() * 0.35f;
//      ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

        // get start / end positions
        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        AngleVectors (angles, forward, right, up);
        VectorSet(offset, 0, 8, ent->viewheight-8);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        VectorScale(right, -8.0, right);        // use right as trajectory for casings
        VectorScale(up, 3.0, up);
        if(bandwidth_mode->value >= 2) {
                VectorScale(forward, 10.0, case_start);
                VectorAdd(case_start, start, case_start);
                VectorAdd(case_start, right, case_start);
                VectorAdd(case_start, up, case_start);
                VectorScale(right, 8.0, right);
                VectorScale(up, 26.0, up);
                VectorAdd(up, right, right);

                ThrowCasing (ent, "models/weapons/casings/bullets.md2", right, case_start);
        }

        fire_piercing (ent, start, forward, TE_GUNSHOT, DEFAULT_AUTOGUN_HSPREAD + spread, DEFAULT_AUTOGUN_VSPREAD + spread, damage, kick, MOD_AUTOGUN);

        //r1ch - lag fix //
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_MACHINEGUN);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
        //r1ch - lag fix (dont work)
        //ent->s.effects = EF_HYPERBLASTER;

//      PlayerNoise(ent, start, PNOISE_WEAPON);

        // ### Hentai ### BEGIN

        ent->client->anim_priority = ANIM_ATTACK;
        if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        {
                ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
                ent->client->anim_end = FRAME_crattak9;
        }
        else
        {
                ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
                ent->client->anim_end = FRAME_attack8;
        }

        // ### Hentai ### END

        if (ent->client->resp.upgrades & UPGRADE_SMARTGUN)
        {
                if (!ent->target_ent)
                {
                        vec3_t  forward;
                        trace_t tr;
                        vec3_t start;
                        vec3_t mins = {-4,-4,-4};
                        vec3_t maxs = {4,4,4};

                        VectorCopy (ent->s.origin,start);
                        start[2] += ent->viewheight;

                        AngleVectors(ent->client->v_angle, forward, NULL, NULL);

                        VectorScale(forward, 512, forward);
                        VectorAdd(ent->s.origin, forward, forward);

                        tr = gi.trace (start, mins, maxs, forward, ent, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

                        // we hit something
                        if (tr.ent && tr.ent->health > 0 && tr.ent != ent)
                        {
                                if (tr.ent->client)
                                {
                                        if (tr.ent->client->resp.team == TEAM_ALIEN)
                                                ent->target_ent = tr.ent;
                                }
                                //else
                                //      ent->target_ent = tr.ent;
                        }
                }
                else
                {
                vec3_t          dir;

                if (!ent->target_ent->inuse || ent->target_ent->health <= 0 || ent->target_ent->deadflag)
                {
                        ent->target_ent = NULL;
                }
                else
                {
                        VectorSubtract (ent->target_ent->s.origin, ent->s.origin, dir);
                        if (VectorLength (dir) > 550)
                        {
                                ent->target_ent = NULL;
                        }
                        else
                        {
                                vectoangles (dir, ent->client->ps.viewangles);
                                vectoangles (dir, ent->client->v_angle);

                                ent->client->ps.pmove.delta_angles[0] = ANGLE2SHORT(ent->client->v_angle[0] - ent->client->resp.cmd_angles[0]);
                                ent->client->ps.pmove.delta_angles[1] = ANGLE2SHORT(ent->client->v_angle[1] - ent->client->resp.cmd_angles[1]);
                                ent->client->ps.pmove.delta_angles[2] = ANGLE2SHORT(ent->client->v_angle[2] - ent->client->resp.cmd_angles[2]);
                        }
                }
        }
        }


        ent->client->resp.inventory[ent->client->ammo_index]--;
}

void Weapon_Autogun (edict_t *ent)
{
        static int      pause_frames[]  = {56, 67, 91, 0};
        static int      fire_frames[]   = {4, 5, 6, 7, 8, 0};

        if (ent->target_ent && !((ent->client->buttons | ent->client->latched_buttons) & BUTTON_ATTACK))
                ent->target_ent = NULL;

        Weapon_Generic (ent, 3, 10, 55, 95, 99, pause_frames, fire_frames, Autogun_Fire);
}

static void Subgun_Fire (edict_t *ent)
{
        int     i;
        vec3_t          start;
        vec3_t          forward, right, up, case_start;
        vec3_t          angles;
        int                     spread;
        int                     damage = 18+random()*9;
        int                     kick = 8;
        vec3_t          offset;
        //gitem_t               *clip;

        if (!(ent->client->buttons & BUTTON_ATTACK))
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe=45;
        }

        if(ent->client->resp.inventory[ent->client->ammo_index] <= 0)
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe=45;

                //gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_noammo), 1, ATTN_NORM, 0);

                Auto_Reload(ent);
                return;
        }

        if (ent->client->resp.primed)
                return;

        ent->client->ps.gunframe++;

        if(ent->client->ps.gunframe == 7)
                ent->client->ps.gunframe = 5;


        /*for (i=0 ; i<3 ; i++)
        {
                ent->client->kick_origin[i] = crandom() * 0.35;
                ent->client->kick_angles[i] = crandom() * 0.7;
                if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                        ent->client->kick_angles[i] = crandom() * (float)ent->client->machinegun_shots * 0.10;
                else
                        ent->client->kick_angles[i] = crandom() * (float)ent->client->machinegun_shots * 0.45;
                if (ent->client->machinegun_shots <= 10)
                        ent->client->kick_angles[i] *= 0.66;    
        }*/

        for (i=1 ; i<3 ; i++)
        {
                ent->client->kick_origin[i] = crandom() * 0.35f;
        }

        if (recoil->value)
        {
                ent->client->kick_angles[0] = -random() * pow(ent->client->machinegun_shots / 8.0f, 2) / 3;
                ent->client->kick_angles[1] = crandom() * pow(ent->client->machinegun_shots / 8.0f, 2) / 14;
                ent->client->kick_angles[2] = crandom() * 0.7f;

                // raise the gun as it is firing
                ent->client->machinegun_shots += 1;
                if (ent->client->machinegun_shots > 48)
                        ent->client->machinegun_shots = 48;
                spread = 300 + 4 * pow(ent->client->machinegun_shots / 7, 2);
        }
        else
        {
                spread = 300;
        }

        //ent->client->kick_origin[0] = crandom() * 0.35;
        //ent->client->kick_angles[1] = 0;
//      ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

        // get start / end positions
        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        AngleVectors (angles, forward, right, up);
        VectorSet(offset, 0, 8, ent->viewheight-8);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);


        if(bandwidth_mode->value >= 2) {
                VectorScale(right, -8.0, right);        // use right as trajectory for casings
                VectorScale(up, 3.0, up);
                VectorScale(forward, 10.0, case_start);
                VectorAdd(case_start, start, case_start);
                VectorAdd(case_start, right, case_start);
                VectorAdd(case_start, up, case_start);
                VectorScale(right, 8.0, right);
                VectorScale(up, 26.0, up);
                VectorAdd(up, right, right);

                ThrowCasing (ent, "models/weapons/casings/bullets.md2", right, case_start);
        }

        //fire_bullet (ent, start, forward, damage, kick, 300, 300, MOD_MACHINEGUN);
        fire_lead (ent, start, forward, damage, kick, TE_GUNSHOT, spread, spread, MOD_MACHINEGUN, 0);
        gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_subfire), 0.05, ATTN_NORM, 0);

        //gi.WriteByte (svc_muzzleflash);
        //gi.WriteShort (ent-g_edicts);
        //gi.WriteByte (MZ_MACHINEGUN | is_silenced);
        //gi.multicast (ent->s.origin, MULTICAST_PVS);

        // ### Hentai ### BEGIN

        //ent->s.renderfx |= RF_IR_VISIBLE;
        //ent->fly_sound_debounce_time = level.framenum + 3;

        ent->client->anim_priority = ANIM_ATTACK;
        if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        {
                ent->s.frame = FrameReference(ent, FRAME_ATTACK2_S) - (int) (random()+0.25);
                ent->client->anim_end = FrameReference(ent, FRAME_ATTACK2_E);
        }
        else
        {
                ent->s.frame = FrameReference(ent, FRAME_ATTACK1_S) - (int) (random()+0.25);
                ent->client->anim_end = FrameReference(ent, FRAME_ATTACK1_E);
        }

        // ### Hentai ### END
//      PlayerNoise(ent, start, PNOISE_WEAPON);

        ent->client->resp.inventory[ent->client->ammo_index]--;
}

void Weapon_Subgun (edict_t *ent)
{
        static int      pause_frames[]  = {50, 57, 76, 0};
        static int      fire_frames[]   = {5, 6, 0};

        Weapon_Generic (ent, 4, 6, 43, 80, 85, pause_frames, fire_frames, Subgun_Fire);
}

/*

MACHINEGUN / CHAINGUN / pistol

*/

static void Pistol_Fire (edict_t *ent)
{
        int     i;
        vec3_t          start, case_start;
        vec3_t          forward, right, up;
        vec3_t          angles;
        int                     damage = 20;
        int                     kick = 8;
        vec3_t          offset;
//      gitem_t         *clip;

        if (!(ent->client->buttons & BUTTON_ATTACK))
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe=38;

                if(ent->client->resp.inventory[ent->client->ammo_index] <= 0)
                {
                        Auto_Reload(ent);
                }

                return;
        }
        else if(ent->client->ps.gunframe == 13)
        {
                ent->client->ps.gunframe=10;
                return;
        }
        else if(ent->client->ps.gunframe == 16)
        {
                gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_noammo), 1, ATTN_IDLE, 0);
                ent->client->ps.gunframe=14;
                return;
        }

        ent->client->ps.gunframe++;

        if (ent->client->resp.class_type == CLASS_GRUNT && (ent->client->resp.upgrades & UPGRADE_STIMPACKS))
                ent->client->ps.gunframe += 2;

        if (ent->client->resp.inventory[ent->client->ammo_index] < 1)
        {
                ent->client->ps.gunframe = 15;
                gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_noammo), 1, ATTN_IDLE, 0);
                return;
        }

        for (i=1 ; i<3 ; i++)
        {
                ent->client->kick_origin[i] = crandom() * 0.35f;
                ent->client->kick_angles[i] = crandom() * 0.7f;
        }
        // get start / end positions
        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        AngleVectors (angles, forward, right, up);
        VectorSet(offset, 0, 0, ent->viewheight);               //ent->viewheight-8
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        if (bandwidth_mode->value >= 2) {
                //VectorScale(right, 8.0, right);       // use right as trajectory for casings
                VectorScale(up, -6.0, up);
                VectorScale(forward, 10, case_start);
                VectorAdd(case_start, start, case_start);
                VectorAdd(case_start, right, case_start);
                VectorAdd(case_start, up, case_start);
                VectorScale(right, 32.0, right);
                VectorScale(up, -12, up);
                VectorAdd(up, right, right);

//              clip = GetItemByIndex(ent->client->resp.shot_type_pistol);
                ThrowCasing (ent, "models/weapons/casings/bullets.md2", right, case_start);
        }

        fire_bullet (ent, start, forward, damage, kick, 0, 0, MOD_PISTOL);
        gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_pistgf1), 1, ATTN_IDLE, 0);


//      PlayerNoise(ent, start, PNOISE_WEAPON);

        ent->client->resp.inventory[ent->client->ammo_index]--;
}

void Weapon_Pistol (edict_t *ent)
{
        static int      pause_frames[]  = {9, 0};
        static int      fire_frames[]   = {10, 13, 16, 0};

        Weapon_Generic (ent, 9, 16, 37, 57, 60, pause_frames, fire_frames, Pistol_Fire);
}


/*

SHOTGUN / SUPERSHOTGUN

*/

static void weapon_shotgun_fire (edict_t *ent)
{
        vec3_t          start;
        vec3_t          forward, right;
        vec3_t          offset;
        int             spread=SHOT_SHELL_SPREAD;
        int                     damage = SHOT_SHELL_DAMAGE;
        int                     kick = SHELL_KICK;
        const gitem_t *clip;

        if(ent->client->resp.inventory[ent->client->ammo_index] <= 0)
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe=55;

                Auto_Reload(ent);
                return;
        }

        AngleVectors (ent->client->v_angle, forward, right, NULL);

        VectorScale (forward, -2, ent->client->kick_origin);
        ent->client->kick_angles[0] = -2;

        VectorSet(offset, 0, 8,  ent->viewheight-8);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        //clip = GetItemByIndex(ent->client->resp.shot_type);
        clip = &itemlist[ent->client->resp.shot_type];

        if (clip->tag == AMMO_EXSHELLS)
        {
                //int i = 0;
                kick = 10;
                //if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                        //spread = EX_SHELL_CROUCH_SPREAD;
                //else
                //      spread = EX_SHELL_DEFAULT_SPREAD;

                spread = 150;

                if (!ent->groundentity)
                        spread *= 2;

                //ent->client->last_reload_time = level.time;

                /*for (i=1 ; i<3 ; i++)
                {
                        ent->client->kick_origin[i] = crandom() * 4.0f;
                        ent->client->kick_angles[i] = crandom() * 4.0f;
                }*/

                //ent->client->kick_origin[0] = crandom() * 0.25;

                fire_explosive (ent, start, forward, EX_SHELL_DAMAGE, kick, TE_EXPLOSION1, spread, spread, MOD_EXSHELL);
        }
        else if (clip->tag == AMMO_SHELLS)
                fire_shotgun (ent, start, forward, damage, kick, 600, 600, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);

        // send muzzle flash
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_SHOTGUN);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

        ent->client->ps.gunframe++;
//      PlayerNoise(ent, start, PNOISE_WEAPON);

        ent->client->resp.inventory[ent->client->ammo_index]--;
}

void Weapon_Shotgun (edict_t *ent)
{
        static int      pause_frames[]  = {55, 68, 0};
        static int      fire_frames[]   = {8, 0};

        Weapon_Generic (ent, 7, 18, 54, 72, 75, pause_frames, fire_frames, weapon_shotgun_fire);
}

static void weapon_supershotgun_fire (edict_t *ent)
{
        vec3_t          start;
        vec3_t          forward, right;
        vec3_t          offset;
        vec3_t          v;
        int                     damage = 8;
        int                     kick = 30;

        if(ent->client->resp.inventory[ent->client->ammo_index] <= 0)
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe=34;

                Auto_Reload(ent);
                return;
        }

        ent->client->resp.inventory[ent->client->ammo_index] -= 2;

        AngleVectors (ent->client->v_angle, forward, right, NULL);

        VectorScale (forward, -2, ent->client->kick_origin);
        ent->client->kick_angles[0] = -2;

        VectorSet(offset, 0, 8,  ent->viewheight-8);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);


        v[PITCH] = ent->client->v_angle[PITCH]-1;
        v[YAW]   = ent->client->v_angle[YAW];
        v[ROLL]  = ent->client->v_angle[ROLL];
        AngleVectors (v, forward, NULL, NULL);
        fire_shotgun (ent, start, forward, damage, kick, 90, 100, 7, MOD_SSHOTGUN);
        v[PITCH] = ent->client->v_angle[PITCH]+2;
        AngleVectors (v, forward, NULL, NULL);
        fire_shotgun (ent, start, forward, damage, kick, 90, 100, 7, MOD_SSHOTGUN);

        // send muzzle flash
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_SSHOTGUN);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

        ent->client->ps.gunframe++;

        if (ent->client->resp.inventory[ent->client->ammo_index] >= 2)
                ent->client->weaponstate = WEAPON_RELOADING;
}

void Weapon_SuperShotgun (edict_t *ent)
{
        static int      pause_frames[]  = {33, 42, 57, 0};
        static int      fire_frames[]   = {7, 0};

        Weapon_Generic (ent, 6, 14, 33, 66, 68, pause_frames, fire_frames, weapon_supershotgun_fire);
}


static void weapon_pulselaser_fire (edict_t *ent)
{
        vec3_t          start, ang;
        vec3_t          forward, right;
        vec3_t          offset;

        if(ent->client->last_move_time > level.time)
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe=10;
                return;
        }

        ent->client->ps.gunframe++;

        if((ent->client->resp.inventory[ent->client->ammo_index] < 1) || !(ent->client->buttons & BUTTON_ATTACK))
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe=10;

                return;
        }

        if(ent->client->ps.gunframe > 9)
                ent->client->ps.gunframe = 8;

//      ent->random += 1.40;
        
        ent->client->resp.inventory[ent->client->ammo_index]-= 1;
        if (ent->client->resp.inventory[ent->client->ammo_index] < 0)
                ent->client->resp.inventory[ent->client->ammo_index] = 0;

//      ent->random -= (int)ent->random;

        //ent->client->last_reload_time++;

        VectorCopy(ent->client->v_angle, ang);
        //ang[1] += 10;
        AngleVectors (ang, forward, right, NULL);

        VectorSet(offset, 20, 13, ent->viewheight-6);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        fire_lead (ent, start, forward, 19, 15, TE_ELECTRIC_SPARKS, 150, 150, MOD_PULSELASER, 0);
        //fire_rail (ent, start, forward, 14, 10);
        gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_pulsfire), 1, ATTN_NORM, 0);

        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_RAILGUN | MZ_SILENCED);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

//      PlayerNoise(ent, start, PNOISE_WEAPON);

        ent->client->anim_priority = ANIM_ATTACK;
        if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        {
                ent->s.frame = FrameReference(ent, FRAME_ATTACK2_S) - (int) (random()+ent->client->ps.gunframe-7.75);
                ent->client->anim_end = FrameReference(ent, FRAME_ATTACK2_E);
        }
        else
        {
                ent->s.frame = FrameReference(ent, FRAME_ATTACK1_S) - (int) (random()+ent->client->ps.gunframe-7.75);
                ent->client->anim_end = FrameReference(ent, FRAME_ATTACK1_E);
        }
}

void Weapon_PulseLaser (edict_t *ent)
{
        static int      pause_frames[]  = {18, 39, 0};
        static int      fire_frames[]   = {8, 9, 0};

        Weapon_Generic (ent, 7, 9, 9, 39, 43, pause_frames, fire_frames, weapon_pulselaser_fire);
}

void weapon_pulselaser_fire_discharge (edict_t *ent)
{
        vec3_t          start, ang;
        vec3_t          forward, right;
        vec3_t          offset, recoil;
//      int                     damage;
        float           xyspeed = sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);
        float  time = 21;


//      ent->random++;

//        if(ent->client->buttons & BUTTON_USE)
//                return;

/*
        if(ent->client->buttons & BUTTON_ATTACK)
        {
                ent->random=0;
                return;
        }
*/
//      if(xyspeed > 100)
//      {
//              ent->client->weaponstate = WEAPON_READY;
//              ent->client->ps.gunframe=10;
//              return;
//      }



        if (ent->client->last_move_time >= level.time) return;


        ent->client->ps.gunframe++;

        if((ent->client->resp.inventory[ent->client->ammo_index] < 2)) // || !(ent->client->buttons & BUTTON_ATTACK))
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe=10;

                return;
        }

        if(ent->client->ps.gunframe > 9)
                ent->client->ps.gunframe = 8;

        VectorCopy(ent->client->v_angle, ang);
        //ang[1] += 10;
        AngleVectors (ang, forward, right, NULL);

        VectorSet(offset, 20, 13, ent->viewheight-6);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        if (ent->flags & FL_POWER_ARMOR)
        {
                gi.sound(ent, CHAN_AUTO, SoundIndex (misc_power2), 1, ATTN_IDLE, 0);
                ent->flags &= ~FL_POWER_ARMOR;
                time += 5;
                if (random() < .5)
                {
                        ent->client->resp.shot_type -= random() * 3;
                        if (--ent->client->resp.shot_type < 5)
                                ent->client->resp.shot_type = 5;
                }
        }
        
        if (ent->client->resp.upgrades & UPGRADE_COOLANT) time -= 7.9;

        //fire_lead (ent, start, forward, 16+random()*2, 10+random()*5, TE_ELECTRIC_SPARKS, 150, 150, MOD_PULSELASER);
        if(ent->random<5) ent->random = ent->client->resp.inventory[ent->client->ammo_index];

        VectorScale (forward, ent->random*-5, recoil);
        //recoil[2] += 250;
        VectorAdd (ent->velocity, recoil, ent->velocity);


        ent->newcount = ent->random;
        fire_rail (ent, start, forward, ent->random * 4, ent->random*(500.0f/60), false);
        ent->client->resp.inventory[ent->client->ammo_index] -= ent->random;

        time = time*(0.25f+0.75f*ent->random/60.0f); //default 21 sec pulse
        time = SETTING_FLT(exterm_pulse_overheat)*time/21; //now scale by new cvar
        if (time > 0)
          ent->client->last_move_time = level.time + time;
        ent->random=0;

        ent->client->blind_time = 0;
        ent->client->blinded_alpha = 0.5;
        ent->client->blind_time_alpha = 0;
        //gi.sound(ent, CHAN_AUTO, SoundIndex (berserk_attack), 1, ATTN_NORM, 0);

        if(ent->client->resp.inventory[ent->client->ammo_index]<0)
        ent->client->resp.inventory[ent->client->ammo_index] = 0;


        gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_railgf1a), 1, ATTN_NORM, 0);

        /*if (random()*8 < 1) {
                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_EXPLOSION1);
                gi.WritePosition (ent->s.origin);
                gi.multicast (ent->s.origin, MULTICAST_PVS);
                ent->client->blind_time = 0.5;
                ent->client->blind_time_alpha = 1;
                ent->client->resp.shot_type -= random()*12+8;
                if (ent->client->resp.shot_type < 5)
                        ent->client->resp.shot_type = 5;
                gi.cprintf (ent,PRINT_HIGH,"Your cell pack was damaged during discharge!\n");
        }*/

        if ((rand()&5==4))
        {
          int n = (ent->client->resp.upgrades & UPGRADE_COOLANT)?2:4;
                ent->client->resp.shot_type -= randomMT()%(1+n);
                if (ent->client->resp.shot_type < 5)
                        ent->client->resp.shot_type = 5;
                gi.cprintf (ent,PRINT_HIGH,"Your cell pack was damaged during discharge!\n");
        }

        ent->client->anim_priority = ANIM_ATTACK;
        if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        {
                ent->s.frame = FrameReference(ent, FRAME_ATTACK2_S) - (int) (random()+ent->client->ps.gunframe-7.75);
                ent->client->anim_end = FrameReference(ent, FRAME_ATTACK2_E);
        }
        else
        {
                ent->s.frame = FrameReference(ent, FRAME_ATTACK1_S) - (int) (random()+ent->client->ps.gunframe-7.75);
                ent->client->anim_end = FrameReference(ent, FRAME_ATTACK1_E);
        }

        //r1: fire_rail could have killed them (nuking guard, etc)
        if (ent->health > 0) {
          int dmg = ceil(time*0.95);
                T_Damage (ent, ent, ent, start, start, start, dmg, dmg, DAMAGE_PIERCING, MOD_DISCHARGE);
        }
}

static void Magnum_Fire (edict_t *ent)
{
        int     i;
        vec3_t          start;
        vec3_t          forward, right, up, case_start;
        vec3_t          angles;
        int                     damage = 80;

        int                     kick = 8;
        vec3_t          offset;

        if(ent->client->resp.inventory[ent->client->ammo_index] <= 0)
        {
                ent->client->weaponstate = WEAPON_READY;
                ent->client->ps.gunframe=32;

                //gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_noammo), 1, ATTN_NORM, 0);

                Auto_Reload(ent);
                return;
        }

        ent->client->ps.gunframe++;

        for (i=0 ; i<3 ; i++)
        {
                ent->client->kick_origin[i] = crandom() * 5;
                ent->client->kick_angles[i] = crandom() * 5;
        }

        ent->velocity[0] /= 2;
        ent->velocity[1] /= 2;
        //ent->velocity[2] /= 2;

        // get start / end positions
        VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
        AngleVectors (angles, forward, right, up);
        VectorSet(offset, 0, 8, ent->viewheight-8);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        if (bandwidth_mode->value >= 2) {
                VectorScale(right, -8.0, right);        // use right as trajectory for casings
                VectorScale(up, 3.0, up);
                VectorScale(forward, 10.0, case_start);
                VectorAdd(case_start, start, case_start);
                VectorAdd(case_start, right, case_start);
                VectorAdd(case_start, up, case_start);
                VectorScale(right, 8.0, right);
                VectorScale(up, 26.0, up);
                VectorAdd(up, right, right);
                ThrowCasing (ent, "models/weapons/casings/bullets.md2", right, case_start);
        }


        if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        {
                forward[0] += crandom()/20;
                forward[1] += crandom()/20;
                forward[2] += crandom()/20;
        }
        else
        {
                forward[0] += crandom()/12;
                forward[1] += crandom()/12;
                forward[2] += crandom()/12;
        }

        if (!ent->groundentity) {
                forward[0] += crandom()/4;
                forward[1] += crandom()/4;
                forward[2] += crandom()/4;
        }


        fire_piercing (ent, start, forward, TE_GUNSHOT,0, 0, damage, kick, MOD_MAGNUM);

        gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_magshot), 1, ATTN_IDLE, 0);

//      PlayerNoise(ent, start, PNOISE_WEAPON);

        ent->client->resp.inventory[ent->client->ammo_index] -= 1;
}

void Weapon_Magnum (edict_t *ent)
{
        static int      pause_frames[]  = {36, 49, 0};
        static int      fire_frames[]   = {5, 0};

        Weapon_Generic (ent, 4, 13, 31, 56, 61, pause_frames, fire_frames, Magnum_Fire);
}
