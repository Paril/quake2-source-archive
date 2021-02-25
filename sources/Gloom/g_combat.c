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

int GetArmor(edict_t *ent) {
    if (!ent->client)
        return 0;
    else if (!ent->client->armor)
        return 0;

    return ent->client->resp.inventory[ITEM_INDEX(ent->client->armor)];
}

typedef struct healthstate_s {
    int health;
    int armour;
    int powershield;
    int damage; //Raw unscaled damage points
} healthstate_t;


/*
  Damage logs
 */

static DEBUGITEM(damagelog);

static void LogDamage(edict_t*ent, edict_t*attacker, vec3_t knockback, int totaldamage, int meansofdeath) {
    float world_extend = 0.8; //How long can non-player damages keep the player's damage "hot"
    float player_time = 2.4; //How long until victim must stay safe to avoid giving frags away

    client_respawn_t*r;
    dmglog_t*d;
    vec3_t kd;
    float kl = VectorNormalize2(knockback, kd);

    if (!ent->client || totaldamage <= 0 && kl <= 0) return;

    r = &ent->client->resp;

    if (attacker->client && attacker != ent && (kl > (ent->groundentity ? 224 : 80) || kl > r->pushfraction)) {
        float d = DotProduct(kd, r->pushvel);
        float a = (kl - 50)*0.75f;

        if (a > 75) a = 75;

        if (r->pusher != attacker || d <= 0 || r->pushfraction <= a) {
            r->pusher = attacker;
            VectorCopy(ent->s.origin, r->pushpoint);
            VectorCopy(knockback, r->pushvel);
        } else {
            kl *= r->pushfraction - 75;
            if (kl < 0) kl = 0;
            VectorMA(knockback, kl, r->pushvel, r->pushvel);
        }

        d = (VectorLength(r->pushvel) - 50)*0.75f;
        if (d > 75) d = 75;
        r->pushfraction = VectorLength(knockback) + d;
        r->pushtime = level.time;
    }

    if (attacker->client && ent->max_health > 50) {
        d = r->playerdmg + attacker->s.number - 1;
        r->playerdmgtime = level.time + player_time;
    } else {
        //Hatchies and kami are sure free frag after first poke/nade/bullet, so they're excluded due to low hp
        d = &r->worlddmg;
        if (r->playerdmgtime >= level.time && r->playerdmgtime - player_time + world_extend <= level.time)
            r->playerdmgtime = level.time + player_time;
    }

    d->lastmod = meansofdeath;
    d->damage += totaldamage;

    if (Debug(damagelog))
        gi.dprintf("Logged %d of %d for %s[%d]  k=%.1f\n", totaldamage, d->damage, attacker->classname, attacker->s.number, kl);
}

void LogHeal(edict_t*ent, int heal) {
    client_respawn_t*r;
    int a, max_a, i, sum;

    if (!ent->client || heal < 0) return;

    r = &ent->client->resp;

    a = GetArmor(ent);
    max_a = classlist[r->class_type].armorcount;

    if (ent->health >= ent->max_health && a >= max_a) {
        //Clear rounding errors of code below
        memset(r->playerdmg, 0, sizeof (r->playerdmg));
        memset(&r->worlddmg, 0, sizeof (r->worlddmg));

        //Dont allow world to extend when there arent actually any players either
        r->playerdmgtime = 0;

        if (Debug(damagelog))
            gi.dprintf("Logged full healing for %d\n", ent->s.number);


        return;
    }

    sum = r->worlddmg.damage;
    for (i = 0; i < LENGTH(r->playerdmg); i++)
        sum += r->playerdmg[i].damage;

    if (sum) {
        int d;

        for (i = 0; i < LENGTH(r->playerdmg); i++) {
            d = heal * r->playerdmg[i].damage / sum;
            if (r->playerdmg[i].damage > d)
                r->playerdmg[i].damage -= d;
            else
                r->playerdmg[i].damage = 0;
        }

        d = heal * r->worlddmg.damage / sum;
        if (r->worlddmg.damage > d)
            r->worlddmg.damage -= d;
        else
            r->worlddmg.damage = 0;
    }

    if (Debug(damagelog))
        gi.dprintf("Logged %d healing for %d\n", heal, ent->s.number);
}

void LogRemoveDamages(edict_t*ent) {
    int i, num = ent->s.number - 1;

    if (num < 0 || num >= MAX_CLIENTS) return;

    for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
        if (ent->inuse && ent->client)
            ent->client->resp.playerdmg[num].damage = 0;

    if (Debug(damagelog))
        gi.dprintf("Removed player %d from damage logs\n", num + 1);
}

static DEBUGITEM(dmgpoints);

static void RewardDamage(edict_t *targ, edict_t *inflictor, edict_t *attacker, healthstate_t oldhs, healthstate_t newhs, int mod, float scale) {
    float points, health_dmg_ratio, armor_dmg_ratio = 0;
    int score_point_ratio;
    rewardinfo_t r = GetCombatReward(attacker, inflictor, targ, mod);

    health_dmg_ratio = (oldhs.health - newhs.health) / (float) newhs.damage;
    armor_dmg_ratio = (oldhs.armour - newhs.armour) / (float) newhs.damage;
    points = (r.instant_a_points * armor_dmg_ratio + r.instant_h_points * health_dmg_ratio) * scale + r.coop_points + r.kill_points;
    score_point_ratio = (attacker->client->resp.dmg_points + (int)points)/SCORE_REWARD_THRESHOLD;

    if (score_point_ratio > attacker->client->resp.dmg_points/SCORE_REWARD_THRESHOLD)
        attacker->client->resp.total_score++;
    attacker->client->resp.dmg_points += (int) points;
    
    if (attacker->client->resp.dmg_points >= FRAG_REWARD_THRESHOLD) {
        if (attacker->client->resp.score < MAX_SCORE->value) {
                attacker->client->resp.score++;
                gi.cprintf(attacker, PRINT_HIGH, "%d damage points earned you 1 frag.\n", FRAG_REWARD_THRESHOLD);
        } else {
            gi.cprintf(attacker, PRINT_HIGH, "Frag reward for %d damage points not given, maximum frags already reached.\n", FRAG_REWARD_THRESHOLD);
        }
        attacker->client->resp.dmg_points %= FRAG_REWARD_THRESHOLD;
    }
    if (Debug(dmgpoints))
        gi.cprintf(attacker, PRINT_HIGH, "Dmg points = %d, Total dmg points = %d\n", (int) points, attacker->client->resp.dmg_points);
}

static void AnyKilled(edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t point, healthstate_t*oldhs, healthstate_t*newhs) {
    targ->die(targ, inflictor, attacker, newhs->damage - oldhs->damage, point);
}

static void PlayerKilled(edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t point, healthstate_t*oldhs, healthstate_t*newhs) {
    if (targ->health < -999) targ->health = -999;
    targ->enemy = attacker;

    /*if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE)
    {       // doors, triggers, etc
            targ->die (targ, inflictor, attacker, damage, point);
            return;
    }*/

    if (targ->client && !attacker->client) {
        client_respawn_t*r = &targ->client->resp;
        qboolean pushed = false;

        if (r->pushfraction > 0 && r->pusher->inuse) { //Pushed into something?
            float dist, movespeed, pushspeed;
            vec3_t dir;
            vec3_t pushdir = {r->pushvel[0], r->pushvel[1], 0};
            pushspeed = VectorNormalize(pushdir);

            VectorSubtract(point, r->pushpoint, dir);
            dir[2] = 0;
            dist = VectorNormalize(dir);


            if (level.time > r->pushtime)
                movespeed = dist / (level.time - r->pushtime);
            else
                movespeed = dist;

            if (movespeed < pushspeed * 2 || pushspeed > 150) {
                if (dist > 32)
                    pushed = DotProduct(dir, pushdir) >= 0.5; //Died at around 60° from where was pushed
                else
                    pushed = DotProduct(dir, pushdir) >= 0; //Too close for proper throwing towards death, check up to 90° for confusing player to walk into death
            }
        }

        if (pushed) {
            attacker = r->pusher;
            meansOfDeath = (attacker->client->resp.team == TEAM_ALIEN) ? MOD_PUSHED : MOD_SHOT_AWAY;
        } else
            if (r->playerdmgtime >= level.time) { //Most damage done by a player?
            int i;
            float most = -1;

            int sum = 0;

            if (targ->client->acid_duration > 0 && targ->client->acid_attacker) {
                int hits = targ->client->acid_duration / 5;
                if (hits > 0)
                    LogDamage(targ, targ->client->acid_attacker, vec3_origin, targ->client->acid_damage * hits, MOD_POISON);
            }


            for (i = 0; i < LENGTH(r->playerdmg); i++)
                sum += r->playerdmg[i].damage;

            if (sum > r->worlddmg.damage * 2) { //Players must do more than 66.7% of total to get a lastdamage frag
                for (i = 0; i < LENGTH(r->playerdmg); i++) {
                    edict_t*p = g_edicts + i + 1;
                    if (p->inuse && r->playerdmg[i].damage) {
                        float d = r->playerdmg[i].damage //Damage done
                                - p->client->resp.score * 1.15f //In case of tie or near-tie, prefer player with less frags
                                + (rand()&127) / 1100.0f; //Never allow a tie

                        if (d > most) {
                            most = d;
                            meansOfDeath = r->playerdmg[i].lastmod;
                            attacker = p;
                        }
                    }
                }
            }
        }

        if (attacker->client && (!inflictor || inflictor->owner != attacker)) inflictor = attacker;
    }

    //we just "died" into something else (this stops eggs, spikers, etc from keeping their
    //enttype after dying and probably some other stuff i don't care about)
    //gi.dprintf("%s->die(%s(%d), %s(%d), %s(%d), %d, %p); MOD=%d\n", targ->classname, targ->classname, targ->s.number, inflictor?inflictor->classname:"NULL", inflictor?inflictor->s.number:"-", attacker->classname, attacker->s.number, damage, point, meansOfDeath);


    AnyKilled(targ, inflictor, attacker, point, oldhs, newhs);
}






/*
--------
 */











void Toss_C4(edict_t *ent, const gitem_t *item);

const double pislash180 = (M_PI / 180.0);

/*
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
 */
qboolean CanDamage(edict_t *targ, edict_t *inflictor) {
    vec3_t dest;
    trace_t trace;

    // bmodels need special checking because their origin is 0,0,0
    if (targ->movetype == MOVETYPE_PUSH) {
        VectorAdd(targ->absmin, targ->absmax, dest);
        VectorScale(dest, 0.5f, dest);
        trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
        if (trace.fraction == 1.0f)
            return true;
        if (trace.ent == targ)
            return true;
        return false;
    }

    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    VectorCopy(targ->s.origin, dest);
    dest[0] += 15.0f;
    dest[1] += 15.0f;
    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0f)
        return true;

    VectorCopy(targ->s.origin, dest);
    dest[0] += 15.0f;
    dest[1] -= 15.0f;
    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    VectorCopy(targ->s.origin, dest);
    dest[0] -= 15.0f;
    dest[1] += 15.0f;
    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0f)
        return true;

    VectorCopy(targ->s.origin, dest);
    dest[0] -= 15.0f;
    dest[1] -= 15.0f;
    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0f)
        return true;


    return false;
}

/*
SpawnDamage
 */
void SpawnDamage(int type, vec3_t origin, vec3_t normal, int damage) {
    if (damage > 255)
        damage = 255;
    gi.WriteByte(svc_temp_entity);
    gi.WriteByte(type);
    //      gi.WriteByte (damage);
    gi.WritePosition(origin);
    gi.WriteDir(normal);
    gi.multicast(origin, MULTICAST_PVS);
}

/*
SpawnDamage
 */
void SpawnDamage2(int type, vec3_t origin, vec3_t normal, int count, int color) {
    gi.WriteByte(svc_temp_entity);
    gi.WriteByte(type);
    gi.WriteByte(count);
    gi.WritePosition(origin);
    gi.WriteDir(normal);
    gi.WriteByte(color);
    gi.multicast(origin, MULTICAST_PVS);
}

typedef struct {
    uint16_t flags;
    uint8_t subtract_or_lookup;
    float hscale;
    float ascale;
} dmgscale_t;

typedef struct {
    dmgscale_t*classes;
    dmgscale_t*entities;
} dmgs_t;

enum {
    DS_REDIR = 1
};

static void ApplyDamageScale(edict_t*ent, int *take, int*dflags, dmgs_t*scales) {
    dmgscale_t*dmgbase;
    dmgscale_t*dmg;
    int old = *take;

    if (ent->client) {
        dmgbase = scales->classes;
        dmg = scales->classes + ent->client->resp.class_type + 1;
    } else {
        if (ent->enttype == ENT_TURRETBASE) ent = ent->target_ent;
        dmgbase = scales->entities;
        dmg = scales->entities + ent->enttype + 1;
    }

    while (dmg->flags & DS_REDIR)
        dmg = dmgbase + dmg->subtract_or_lookup + 1;

    *take -= dmg->subtract_or_lookup + dmgbase->subtract_or_lookup;
    if (*take > 0) {
        if (ent->client && ent->client->armor && ent->client->resp.inventory[ITEM_INDEX(ent->client->armor)] > 1)
            *take *= dmg->ascale * dmgbase->ascale;
        else
            *take *= dmg->hscale * dmgbase->hscale;
    } else
        *take = 0;
    //gi.dprintf("%d -> %d\n", old, *take);
}



static dmgscale_t corrode_class[1 + NUMCLASSES + 1] = {
    /*ALL                 */
    {0, 0, 0.5f, 0.5f},
    /*CLASS_GRUNT		*/
    {0, 0, 0.75f, 0.75f},
    /*CLASS_HATCHLING	*/
    {0, 0, 1.0f, 1.0f},
    /*CLASS_HEAVY		*/
    {DS_REDIR, CLASS_GRUNT},
    /*CLASS_COMMANDO	*/
    {DS_REDIR, CLASS_GRUNT},
    /*CLASS_DRONE		*/
    {DS_REDIR, CLASS_HATCHLING},
    /*CLASS_MECH		*/
    {0, 0, 0.0f, 0.0f},
    /*CLASS_SHOCK		*/
    {0, 0, 0.63f, 0.7f},
    /*CLASS_STALKER	*/
    {0, 0, 0.5f, 0.5f},
    /*CLASS_BREEDER	*/
    {DS_REDIR, CLASS_HATCHLING},
    /*CLASS_ENGINEER	*/
    {0, 0, 0.33f, 0.33f},
    /*CLASS_GUARDIAN	*/
    {DS_REDIR, CLASS_STALKER},
    /*CLASS_KAMIKAZE	*/
    {DS_REDIR, CLASS_HATCHLING},
    /*CLASS_EXTERM	*/
    {0, 0, 0.8f, 0.1f},
    /*CLASS_STINGER	*/
    {DS_REDIR, CLASS_STALKER},
    /*CLASS_WRAITH	*/
    {DS_REDIR, CLASS_HATCHLING},
    /*CLASS_BIO		*/
    {DS_REDIR, CLASS_GRUNT},
    /*CLASS_OBSERVER	*/
    {0}
};


static dmgscale_t corrode_entity[1 + ENT_COUNT + 1] = {
    /*ALL                 */
    {0, 0, 1.00f},
    /*ENT_UNKNOWN		*/
    {0, 0, 1.00f},
    /*ENT_DRONEWEB	*/
    {0, 0, 1.00f},
    /*ENT_SPIKE		*/
    {0, 0, 1.00f},
    /*ENT_TURRET		*/
    {0, 0, 0.33f},
    /*ENT_MGTURRET	*/
    {0, 0, 0.33f},
    /*ENT_TURRETBASE	*/
    {0, 0, 0.33f},
    /*ENT_DETECTOR	*/
    {0, 0, 0.25f},
    /*ENT_TRIPWIRE_BOMB	*/
    {0, 0, 1.33f},
    /*ENT_TRIPWIRE_LASER	*/
    {0, 0, 1.00f},
    /*ENT_AMMO_DEPOT	*/
    {0, 0, 0.33f},
    /*ENT_TELEPORTER_U	*/
    {0, 0, 0.33f},
    /*ENT_TELEPORTER_D	*/
    {0, 0, 0.33f},
    /*ENT_TELEPORTER	*/
    {0, 0, 0.33f},
    /*ENT_COCOON_U	*/
    {0, 0, 1.00f},
    /*ENT_COCOON_D	*/
    {0, 0, 1.00f},
    /*ENT_COCOON		*/
    {0, 0, 1.00f},
    /*ENT_SPIKER		*/
    {0, 0, 1.50f},
    /*ENT_OBSTACLE	*/
    {0, 0, 2.00f},
    /*ENT_GASSER		*/
    {0, 0, 1.50f},
    /*ENT_HEALER		*/
    {0, 0, 1.00f},
    /*ENT_WRAITHBOMB	*/
    {0, 0, 1.00f},
    /*ENT_ROCKET		*/
    {0, 0, 1.00f},
    /*ENT_GRENADE		*/
    {0, 0, 1.00f},
    /*ENT_FLASH_GRENADE	*/
    {0, 0, 1.00f},
    /*ENT_SMOKE_GRENADE	*/
    {0, 0, 1.00f},
    /*ENT_C4		*/
    {0, 0, 1.00f},
    /*ENT_GAS_SPORE	*/
    {0, 0, 1.00f},
    /*ENT_SPIKE_SPORE	*/
    {0, 0, 1.00f},
    /*ENT_PROXY_SPORE	*/
    {0, 0, 1.00f},
    /*ENT_FLARE		*/
    {0, 0, 1.00f},
    /*ENT_INFEST		*/
    {0, 0, 0.25f},
    /*ENT_CORPSE		*/
    {0, 0, 0.25f},
    /*ENT_HUMAN_BODY	*/
    {0, 0, 0.25f},
    /*ENT_FEMALE_BODY	*/
    {0, 0, 0.25f},
    /*ENT_FLAMES		*/
    {0, 0, 1.00f},
    /*ENT_GASSER_GAS	*/
    {0, 0, 1.00f},
    /*ENT_BLASTER		*/
    {0, 0, 1.00f},
    /*ENT_NOBLOOD_BODY	*/
    {0, 0, 1.00f},
    /*ENT_AREAPORTAL	*/
    {0, 0, 1.00f},
    /*ENT_FUNC_DOOR	*/
    {0, 0, 1.00f},
    /*ENT_TARGET_BLASTER	*/
    {0, 0, 1.00f},
    /*ENT_PROXYSPIKE	*/
    {0, 0, 1.00f},
    /*ENT_FUNC_EXPLOSIVE	*/
    {0, 0, 1.00f},
    /*ENT_FUNC_WALL	*/
    {0, 0, 1.00f},
    /*ENT_EVIL_HEALER	*/
    {0, 0, 1.00f},
    /*ENT_GRENADE_NEW	*/
    {0, 0, 1.00f},
    /*ENT_GRENADE		*/
    {0, 0, 1.00f},
    /*ENT_TURRET_BLOB	*/
    {0, 0, 1.00f},
    /*ENT_FAKE		*/
    {0, 0, 1.00f},
    /*ENT_SD_CAUSER	*/
    {0, 0, 1.00f}
};

static dmgs_t dmg_corroded = {
    /*classes */ corrode_class,
    /*entities*/ corrode_entity
};

/*
T_Damage

targ            entity that is being damaged
inflictor       entity that is causing the damage
attacker        entity that caused the inflictor to damage targ
        example: targ=monster, inflictor=rocket, attacker=player

dir                     direction of the attack
point           point at which the damage is being inflicted
normal          normal vector from that point
damage          amount of damage being inflicted
knockback       force to be applied against targ as a result of the damage

dflags          these flags are used to control how T_Damage works
        DAMAGE_RADIUS                   damage was indirect (from a nearby explosion)
        DAMAGE_NO_ARMOR                 armor does not protect from this damage
        DAMAGE_ENERGY                   damage is from an energy based weapon
        DAMAGE_NO_KNOCKBACK             do not affect velocity, just view angles
        DAMAGE_BULLET                   damage is from a bullet (used for ricochets)
        DAMAGE_NO_PROTECTION    kills godmode, armor, everything

static int CheckPowerArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int dflags)
{
        gclient_t       *client;
        int                     save;
        int                     power_armor_type;
        int                     index;
        int                     damagePerCell;
        int                     pa_te_type;
        int                     power;
        int                     power_used;

        if (!damage)
                return 0;

        if (dflags & DAMAGE_NO_ARMOR)
                return 0;

        client = ent->client;

        power_armor_type = PowerArmorType (ent);

        if (power_armor_type == POWER_ARMOR_NONE)
                return 0;

        index = ITEM_INDEX(FindItem("Cells"));
        power = client->resp.inventory[index];

        if (!power)
                return 0;

        if (power_armor_type == POWER_ARMOR_SCREEN)
        {
                vec3_t          vec;
                float           dot;
                vec3_t          forward;

                // only works if damage point is in front
                AngleVectors (ent->s.angles, forward, NULL, NULL);
                VectorSubtract (point, ent->s.origin, vec);
                VectorNormalize (vec);
                dot = DotProduct (vec, forward);
                if (dot <= 0.3)
                        return 0;

                damagePerCell = 1;
                pa_te_type = TE_SCREEN_SPARKS;
                damage = damage / 3;
        }
        else
        {
                damagePerCell = 2;
                pa_te_type = TE_SCREEN_SPARKS;//TE_SHIELD_SPARKS;
                damage = (2 * damage) / 3;
        }

        save = power * damagePerCell;
        if (!save)
                return 0;
        if (save > damage)
                save = damage;

        //SpawnDamage (pa_te_type, point, normal, save);
        SpawnDamage (pa_te_type, ent->s.origin, normal, save);

        power_used = save / damagePerCell;

        ent->client->powerarmor_time = level.time + 0.2f;
        client->resp.inventory[index] -= power_used;

        return save;
}*/


static int CheckArmor(edict_t *ent, edict_t *attacker, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags, int mod, vec3_t dir) {
    gclient_t *client;
    int save;
    int remsave = 0;
    int index;
    const gitem_t *armor;

    if (!damage)
        return 0;

    if (dflags & DAMAGE_NO_ARMOR)
        return 0;

    client = ent->client;

    if (!client || !client->armor)
        return 0;

    index = ITEM_INDEX(client->armor);

    /*if (dflags & DAMAGE_FRIENDLY_FIRE) {
            damage -= ent->damage_absorb;
            damage *= 0.25;
            if (damage <= 0)
                    return 0;
    }*/

    save = damage;

    // limit armor taken damage to armor health
    if (save >= client->resp.inventory[index])
        save = client->resp.inventory[index];

    // if no armor, just deal the damage
    if (!save)
        return 0;

    remsave = save;

    // mech armor absorbs more
    if (client->armor->tag == ARMOR_FULL) {
        if (!attacker->client)
            remsave *= 0.35f;
        else
            remsave *= 0.6f;
    }

    // combat armor halves damage done
    //if (index == combat_armor_index && remsave > 2)
    if (client->armor->tag == ARMOR_COMBAT && remsave > 2)
        remsave /= 2;

    if (remsave < 1)
        remsave = 1;

    if (dflags & DAMAGE_FRIENDLY_FIRE && attacker->client && classlist[client->resp.class_type].frags_needed > 0) {
        if (dflags & DAMAGE_RADIUS && teamreflectradiusdamage->value) {
            if (teamreflectradiusdamage->value == 2) {
                //full reflect
                T_Damage(attacker, attacker, attacker, dir, point, normal, remsave, 0, DAMAGE_NO_KNOCKBACK, mod);
            } else if (teamreflectradiusdamage->value == 1) {
                //half reflect
                T_Damage(attacker, attacker, attacker, dir, point, normal, (int) (remsave * 0.5), 0, DAMAGE_NO_KNOCKBACK, mod);
            }
        }

        if (teamreflectarmordamage->value) {
            if (teamreflectarmordamage->value == 1)
                T_Damage(attacker, attacker, attacker, point, point, normal, (int) (remsave * 0.5), 0, DAMAGE_NO_KNOCKBACK, mod);
            else if (teamreflectarmordamage->value == 2)
                T_Damage(attacker, attacker, attacker, point, point, normal, remsave, 0, DAMAGE_NO_KNOCKBACK, mod);
        }
    }

    if (!(dflags & DAMAGE_FRIENDLY_FIRE) || dflags & DAMAGE_RADIUS || (teamarmordamage->value)) {
        client->resp.inventory[index] -= remsave;
        SpawnDamage(te_sparks, point, normal, save);
    }

    if (dflags & DAMAGE_PIERCING) {
        armor = &itemlist[index];

        return ceil(((gitem_armor_t *) armor->info)->normal_protection * save); // piercing causes damage to health
    } else
        return save; // all damage taken by armor
}


//same as above, but customizable scaling, may be in a future piercing scale too

static int ArmourProtect(edict_t *ent, edict_t *attacker, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags, int mod, vec3_t dir, float ascale) {
    gclient_t *client;
    int save;
    int remsave = 0;
    int index;
    const gitem_t *armor;

    if (!damage)
        return 0;

    if (dflags & DAMAGE_NO_ARMOR || damage < 1)
        return 0;

    client = ent->client;

    if (!client || !client->armor)
        return 0;

    index = ITEM_INDEX(client->armor);

    save = damage;
    // limit armor taken damage to armor health
    if (save >= client->resp.inventory[index])
        save = client->resp.inventory[index];

    // if no armor, just deal the damage
    if (!save)
        return 0;

    remsave = save*ascale;

    if (remsave < 1)
        remsave = 1;

    if (dflags & DAMAGE_FRIENDLY_FIRE && attacker->client && classlist[client->resp.class_type].frags_needed > 0) {
        if (dflags & DAMAGE_RADIUS && teamreflectradiusdamage->value) {
            if (teamreflectradiusdamage->value == 2) {
                //full reflect
                T_Damage(attacker, attacker, attacker, dir, point, normal, remsave, 0, DAMAGE_NO_KNOCKBACK, mod);
            } else if (teamreflectradiusdamage->value == 1) {
                //half reflect
                T_Damage(attacker, attacker, attacker, dir, point, normal, (int) (remsave * 0.5), 0, DAMAGE_NO_KNOCKBACK, mod);
            }
        }

        if (teamreflectarmordamage->value) {
            if (teamreflectarmordamage->value == 1)
                T_Damage(attacker, attacker, attacker, point, point, normal, (int) (remsave * 0.5), 0, DAMAGE_NO_KNOCKBACK, mod);
            else if (teamreflectarmordamage->value == 2)
                T_Damage(attacker, attacker, attacker, point, point, normal, remsave, 0, DAMAGE_NO_KNOCKBACK, mod);
        }
    }

    if (!(dflags & DAMAGE_FRIENDLY_FIRE) || dflags & DAMAGE_RADIUS || (teamarmordamage->value)) {
        client->resp.inventory[index] -= remsave;
        SpawnDamage(te_sparks, point, normal, save);
    }

    if (dflags & DAMAGE_PIERCING) {
        armor = &itemlist[index];

        return ceil(((gitem_armor_t *) armor->info)->normal_protection * save); // piercing causes damage to health
    } else
        return save; // all damage taken by armor
}

qboolean CheckTeamDamage(edict_t *targ, edict_t *attacker) {
    //FIXME make the next line real and uncomment this block
    // if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
    if ((!targ || !attacker) || (!targ->client && !attacker->client))
        return false;

    if (teamdamage->value == 2)
        return false;

    if (teamdamage->value == 1 && classlist[attacker->client->resp.class_type].frags_needed > 0)
        return false;

    if (targ->client->resp.team == attacker->client->resp.team)
        return true;

    return false;
}

#define HURTCLASS_SPARKS \
        if ((!(targ->flags & FL_NO_HURTSPARKS))) { \
                if (targ->hurtflags & WC_NO_SPARKS) \
                        SpawnDamage (te_sparks, point, normal, take); \
                else \
                        SpawnDamage2 (TE_LASER_SPARKS, point, normal, 5, 2); \
        } \
        return false;

qboolean Damage(damage_t*d) {
    gclient_t *client;
    int take;
    int save = 0;
    int asave = 0;
    int psave = 0;
    int te_sparks = TE_SPARKS;
    vec3_t dir, point, normal;
    dmgoverride_t dmg = {0};
    int tk = 0;
    uint8_t damage_health = true;
    uint8_t damage_armour = true;
    int armour_restore = -1;
    int cells_restore;
    qboolean fixknockback = true;
    qboolean overriden = false;
    qboolean canheal = false;

    //old single args
    edict_t *targ = d->targ;
    edict_t *inflictor = d->inflictor;
    edict_t *attacker = d->attacker;
    int damage = d->damage * d->scale;
    int knockback = d->knockback * d->scale;
    int dflags = d->dflags;
    int mod = d->mod;
    float oscale = 1.0f / (1 + 0.5f * ((dflags & DAMAGE_PASS_MASK) >> DAMAGE_PASS_SHIFT)); //override passthrough scaling
    float rewardscale = 1.0f;

    //Damage tracking
    healthstate_t oldhs, newhs;

    //Debugging
    static DEBUGITEM(damages);

    //we shouldn't really be here ?
    if (!targ->takedamage)
        return false;

    //can't kill stuff after the game is over.
    if (level.intermissiontime && (level.intermissiontime < level.framenum))
        return false;

    //old vector args
    VectorCopy(d->dir, dir);
    VectorNormalize(dir);
    VectorCopy(d->point, point);
    VectorCopy(d->normal, normal);


    //shortcut
    client = targ->client;

    // copy original damage
    take = damage;

    if (targ->client && targ->client->resp.class_type == CLASS_OBSERVER) {
#warning "FIXME: Why observers die from time to time?"
        gi.dprintf("BAD-DAMAGE: ent %d attacked observer %d with a %s\n", attacker - g_edicts, targ - g_edicts, inflictor ? inflictor->classname : "NULL");
        return false;
    }

    //Track damage changes
    oldhs.health = targ->health;
    oldhs.armour = GetArmor(targ);
    oldhs.powershield = oldhs.damage = 0;
    if (targ->flags & FL_POWER_ARMOR) oldhs.powershield = client->resp.inventory[ITEM_AMMO_CELLS];

    //teamkill redirection
    if (attacker->client && inflictor && inflictor->tkredirect) {
        int ateam = attacker->client->resp.team;

        if (targ->client && targ->client->resp.team == ateam)
            attacker = inflictor->tkredirect; //can be same or different team
        else
            if (ateam == TEAM_ALIEN) {
            if (targ->enttype == ENT_COCOON || targ->enttype == ENT_COCOON_U || targ->enttype == ENT_COCOON_D ||
                    targ->enttype == ENT_OBSTACLE || targ->enttype == ENT_SPIKER || targ->enttype == ENT_GASSER ||
                    targ->enttype == ENT_HEALER)
                attacker = inflictor->tkredirect;
        } else
            if (ateam == TEAM_HUMAN)
            if (targ->enttype == ENT_TELEPORTER || targ->enttype == ENT_TELEPORTER_U || targ->enttype == ENT_TELEPORTER_D ||
                    targ->enttype == ENT_TRIPWIRE_BOMB || targ->enttype == ENT_TURRET || targ->enttype == ENT_MGTURRET ||
                    targ->enttype == ENT_TURRETBASE || targ->enttype == ENT_AMMO_DEPOT || targ->enttype == ENT_DETECTOR)
                attacker = inflictor->tkredirect;
    }



    if (mod == MOD_CORRODED)
        ApplyDamageScale(targ, &take, &dflags, &dmg_corroded);
    else
        take = take * 1.2; //grim's damage increase?


    dmg.weaponindex = 0;
    dmg.ascale = -1;
    if (dmg_override && mod != MOD_CHANGECLASS) {
        dmg.hitpoints = take;
        overriden = OverrideDamage(&dmg, targ, inflictor, attacker, dflags, mod);
    }


    // normalize dir, used below anyway
    // if target was a player
    if (client) {
        if (VectorLength(dir) < 0.5f) { //no valid dir given to T_Damage?
            VectorSubtract(targ->s.origin, attacker->s.origin, dir);
            VectorNormalize(dir);
        }
        VectorCopy(dir, client->damage_dir);

        //damage specified not to hurt clients
        if (dflags & DAMAGE_IGNORE_CLIENTS)
            return false;

        if (dflags & DAMAGE_GAS) {
            //immune to gas.
            if (client->resp.team == TEAM_ALIEN)
                return false;

            //add greenish tint
            if (inflictor->enttype == ENT_GAS_SPORE)
                client->damage_parmor += (damage / 2);
        }

        //r1: prevent wraithspit/fire from tking
        if (mod == MOD_ACID || mod == MOD_BURNED)
            dflags &= ~DAMAGE_RADIUS;

        if (attacker == world && client->invincible_framenum > level.framenum && mod != MOD_FALLING)
            client->invincible_framenum = level.framenum - 10; //also disable refund

        if (targ->deadflag == DEAD_DYING)
            take *= 0.5f;

        //gi.dprintf ("before resistance code: %d\n", take);

        /*if (level.framenum == client->damage_last_frame && (mod != MOD_SHOTGUN && mod != MOD_SSHOTGUN) && attacker->client && attacker->client->resp.team == TEAM_ALIEN) {
                gi.bprintf (PRINT_HIGH, "multiframe damage (%u:%u): new damage %d (from %d)\n", level.framenum, client->damage_last_frame, (int)(take * 1.5), take);
                take *= 1.5;
        }*/

        if (!(dflags & DAMAGE_RADIUS) && attacker->client) {
            client->damage_last_frame = level.framenum;
            client->resp.can_respawn = false;
        }

        // if ignore_res, we want to inflict pure damage
        if (!(dflags & DAMAGE_IGNORE_RESISTANCES))

            switch (client->resp.class_type) {

                case CLASS_GRUNT:
                    if (mod == MOD_BURNED)
                        take *= 1.5f; //last 1.0
                    if (mod == MOD_SPIKE)
                        take *= 1.5f;

                    if (attacker->client && !attacker->client->resp.class_type == CLASS_STINGER)
                        take *= 0.8f; //last 1.0
                    break;

                case CLASS_HATCHLING:
                    //                              if (mod == MOD_AUTOGUN || mod == MOD_SHOTGUN || mod == MOD_MACHINEGUN)
                    //                                      take *= 0.5f;
                    if (mod == MOD_PULSELASER)
                        take *= 0.5f;
                    break;

                case CLASS_ENGINEER:
                    if (mod == MOD_GAS_SPORE)
                        return false;
                    else if (mod == MOD_BURNED)
                        take *= 1.15f;
                    if (mod == MOD_SPIKE)
                        take *= 1.5f;
                    break;

                case CLASS_WRAITH:
                    if (mod == MOD_AUTOGUN)
                        take *= 0.5f;
                    break;

                case CLASS_GUARDIAN:
                    if (client->glow_time >= level.time)
                        take *= 1.5f;

                    if (mod == MOD_GRENADE || mod == MOD_ROCKET || mod == MOD_R_SPLASH || mod == MOD_KAMIKAZE || mod == MOD_C4 || mod == MOD_DRUNK_MISSILE || mod == MOD_GRIM_ROCKET || mod == MOD_MECH_MISSILE)
                        take *= 0.25f;
                    else if (mod == MOD_EXSHELL)
                        take *= 0.2f;
                        //else if (mod == MOD_TARGET_LASER)
                        //      take *= 2;
                    else if (mod == MOD_HYPERBLASTER)
                        take *= 0.7f;
                    else if (mod == MOD_MELEE)
                        take *= 0.3f;
                    else if (mod == MOD_RAIL)
                        take *= 0.65f; //was 0.9
                    else if (mod == MOD_PULSELASER || mod == MOD_MAGNUM)
                        take *= 0.8f;

                    if (GetArmor(targ) == 0) {
                        switch (mod) {
                            case MOD_AUTOGUN:
                            case MOD_PISTOL:
                            case MOD_SHOTGUN:
                            case MOD_SSHOTGUN:
                                take *= 0.4f;
                                break;
                            case MOD_HYPERBLASTER:
                                take *= 0.7f;
                                break;
                            case MOD_MACHINEGUN:
                                take *= 0.25f;
                                break;
                        }
                    } else {
                        switch (mod) {
                            case MOD_SSHOTGUN:
                                take *= 0.16f;
                                break;

                            case MOD_AUTOGUN:
                                take *= 0.18f;
                                break;
                            case MOD_PISTOL:
                            case MOD_SHOTGUN:
                                take *= 0.34f;
                                break;
                            case MOD_MACHINEGUN:
                                take *= 0.28f;
                                break;
                            case MOD_EXSHELL:
                            case MOD_HYPERBLASTER:
                                take *= 0.32f;
                                break;
                            case MOD_PULSELASER:
                                take *= 0.30f;
                                break;
                            case MOD_RAIL:
                                take *= 0.65f; //was 0.25f;
                                break;
                        }

                        //if (dflags & DAMAGE_BULLET)
                        //      take *= 0.8;
                    }

                    break;
                case CLASS_STALKER:
                    if (client->glow_time >= level.time)
                        take *= 1.1f;

                    if (mod == MOD_TARGET_LASER)
                        take *= 0.45;
                    else if (mod == MOD_MELEE)
                        take *= 0.3f;
                    else if (mod == MOD_EXSHELL)
                        take *= 1.2f;
                    else if (mod == MOD_FALLING)
                        take *= 0.2f;
                    else if (mod == MOD_RAIL)
                        take *= 0.5f;
                    //else if (dflags & DAMAGE_ENERGY)
                    //      take += 11 + random();

                    if (GetArmor(targ) == 0) {
                        switch (mod) {
                            case MOD_PISTOL:
                            case MOD_MACHINEGUN:
                            case MOD_AUTOGUN:
                            case MOD_MGTURRET:
                                take *= 0.31f;
                                break;

                            case MOD_SSHOTGUN:
                            case MOD_SHOTGUN:
                                take *= 0.5f;
                                break;
                            case MOD_DRUNK_MISSILE:
                            case MOD_GRIM_ROCKET:
                            case MOD_MECH_MISSILE:
                            case MOD_HYPERBLASTER:
                                take *= 0.45f;
                                break;
                        }

                        //if (dflags & DAMAGE_BULLET)
                        //      take *= 1.7;
                    } else {
                        switch (mod) {
                            case MOD_PISTOL:
                            case MOD_MACHINEGUN:
                                take *= 0.19f;
                                break;
                            case MOD_AUTOGUN:
                                take *= 0.12f;
                                break;
                            case MOD_SHOTGUN:
                            case MOD_SSHOTGUN:
                                take *= 0.3f;
                                break;
                            case MOD_MAGNUM:
                            case MOD_EXSHELL:
                                take *= 0.7f;
                                break;
                            case MOD_DRUNK_MISSILE:
                            case MOD_GRIM_ROCKET:
                            case MOD_MECH_MISSILE:
                            case MOD_HYPERBLASTER:
                                take *= 0.40f;
                                break;
                            case MOD_PULSELASER:
                                take *= 0.3f;
                                break;
                            case MOD_RAIL:
                                take *= 0.7f;
                                break;
                        }
                    }

                    break;

                case CLASS_DRONE:

                    if (GetArmor(targ) != 0) {
                        switch (mod) {
                            case MOD_EXSHELL:
                                take += 10;
                                break;
                        }
                        /*
                                case MOD_PISTOL:
                                case MOD_MACHINEGUN:
                                case MOD_AUTOGUN:
                                case MOD_MGTURRET:
                                        //take += random() * 3 + random() + 3;
                                        take *= 0.4;
                                        break;

                                case MOD_SSHOTGUN:
                                case MOD_SHOTGUN:
                                        //take += (random() * 4) + 2;
                                        take *= 0.6;
                                        break;

                                case MOD_PULSELASER:
                                        //take = random()*3+2;
                                        break;

                                case MOD_RAIL:
                                        take /= 2.01;
                                        break;
                        }*/

                        //if (dflags & DAMAGE_BULLET)
                        //      take *= 1.7;
                        //} else {
                        switch (mod) {
                            case MOD_PISTOL:
                            case MOD_MACHINEGUN:
                            case MOD_AUTOGUN:
                            case MOD_MAGNUM:
                                take *= 0.5f;
                                break;
                            case MOD_SHOTGUN:
                            case MOD_SSHOTGUN:
                                take *= 0.9f;
                                break;
                            case MOD_EXSHELL:
                                take *= 0.9f;
                                break;
                                /*case MOD_PULSELASER:
                                        take *= 0.6;
                                        break;*/
                        }
                    } else {
                        switch (mod) {
                            case MOD_PISTOL:
                            case MOD_AUTOGUN:
                                take *= 0.55f;
                                break;
                            case MOD_DRUNK_MISSILE:
                            case MOD_GRIM_ROCKET:
                            case MOD_MECH_MISSILE:
                            case MOD_HYPERBLASTER:
                                take *= 1.20f;
                                break;
                        }
                    }
                    break;

                case CLASS_KAMIKAZE:
                    if (mod == MOD_GRENADE || mod == MOD_ROCKET || mod == MOD_R_SPLASH || mod == MOD_KAMIKAZE || mod == MOD_C4 || mod == MOD_EXSHELL)
                        take *= 0.1f;
                    break;

                case CLASS_SHOCK:
                    if (mod == MOD_BURNED)
                        take *= 0.2f;
                    else if (mod == MOD_ACID)
                        take /= 2.5;
                    else if (dflags & DAMAGE_GAS)
                        return false;

                    break;

                case CLASS_COMMANDO:
                    if (mod == MOD_SPIKE && !attacker->client)
                        take *= 0.9f;
                    else if (attacker->client && attacker->client->resp.class_type == CLASS_HATCHLING)
                        take *= 0.80f;
                    break;

                case CLASS_MECH:
                    if (mod == MOD_BURNED || dflags & DAMAGE_GAS)
                        return false;
                    else if (mod == MOD_ACID)
                        if (GetArmor(targ) > 0)
                            take *= 0.4f;
                        else
                            take *= 0.8f;
                    else if (mod == MOD_SPIKE)
                        take *= 0.88f;
                        //else if (mod == MOD_MELEE && attacker->client && attacker->client->resp.class_type == CLASS_STINGER)
                        //      take *= 1.15;
                    else if (mod == MOD_SLIME && GetArmor(targ) > 0)
                        return false;
                    break;

                case CLASS_STINGER:

                    switch (mod) {

                        case MOD_R_SPLASH:
                            take *= 0.80f;
                            break;
                        case MOD_PULSELASER:
                            take = 0;
                            break;
                        case MOD_MGTURRET:
                            take *= 0.5f;
                            break;
                        case MOD_PISTOL:
                        case MOD_AUTOGUN:
                            take *= 0.3f;
                            break;
                        case MOD_SSHOTGUN:
                            take *= 0.25f;
                            break;
                        case MOD_MACHINEGUN:
                            take *= 0.3f; //0.4 last
                            break;
                        case MOD_DRUNK_MISSILE:
                        case MOD_GRIM_ROCKET:
                        case MOD_MECH_MISSILE:
                        case MOD_HYPERBLASTER:
                            take *= 0.45f;
                            break;
                        case MOD_SHOTGUN:
                            take *= 0.38f;
                            break;
                        case MOD_TARGET_LASER:
                            take *= 0.50f;
                            break;
                        case MOD_MAGNUM:
                            take *= 0.70f;
                            break;
                        case MOD_EXSHELL:
                            take *= 0.80f;
                            break;
                        case MOD_RAIL:
                            take *= 0.62f;
                            break;
                    }
                    break;

                case CLASS_EXTERM:

                    if (mod == MOD_MELEE && attacker->client) {
                        if (attacker->client->resp.class_type == CLASS_DRONE)
                            take /= 1.5f;
                        else if (attacker->client->resp.class_type == CLASS_HATCHLING)
                            take *= 0.30f;
                        else if (attacker->client->resp.class_type == CLASS_STINGER)
                            take *= 0.8f;
                    } else if (mod == MOD_SPIKE && !attacker->client)
                        take *= 0.8f;
                    else if (mod == MOD_ACID)
                        take *= 0.8f;
                    else if (dflags & DAMAGE_GAS)
                        return false;

                    break;

                case CLASS_BREEDER:
                    if (mod == MOD_EXSHELL)
                        take += 50;
                    break;

                case CLASS_BIO:
                    if (dflags & DAMAGE_GAS || mod == MOD_ACID) //bio is resistant
                        take *= 0.6f;
                    break;
                default:
                    break;
            }

        //gi.dprintf ("after resistance code: %d\n", take);

        // area damage code
        //R1: update: falling on people as 'big' classes damage bonuses !
        if (attacker->client && (mod == MOD_RUN_OVER || mod == MOD_JUMPATTACK || mod == MOD_OW_MY_HEAD_HURT || mod == MOD_MECHSQUISH)) {
            //int height = point[2] - targ->s.origin[2] + targ->viewheight;
            int height; // = attacker->absmax[2] -  targ->absmax[2];

            height = attacker->s.old_origin[2] - targ->s.origin[2];

            if (mod == MOD_MECHSQUISH && height > 10 && (targ->client->resp.class_type == CLASS_HATCHLING || targ->client->resp.class_type == CLASS_KAMIKAZE))
                mod = MOD_MECHSQUISH_SQUISH;

            if ((attacker->client->resp.class_type == CLASS_STINGER || attacker->client->resp.class_type == CLASS_DRONE) && height > 75) { //drone based fall on head dmg
                int fallspeed = -attacker->client->oldvelocity[2];

                if (fallspeed > 6)
                    take += fallspeed / 6;
                //gi.bprintf (PRINT_HIGH,"drone fallspeed additional damage: %d",(int)(fallspeed / 6));

            } else if ((attacker->client->resp.class_type == CLASS_GUARDIAN || attacker->client->resp.class_type == CLASS_STALKER || attacker->client->resp.class_type == CLASS_MECH) && height > 50) {
                int fallspeed = -attacker->client->oldvelocity[2];

                if (fallspeed > 0)
                    take += (fallspeed * 2);

                //gi.bprintf (PRINT_HIGH,"fallspeed %d (additional damage: %d)\n",fallspeed, (int)(fallspeed * 2));

                if (fallspeed > 100) //likely to kill, update MoD
                    mod = MOD_OW_MY_HEAD_HURT;

            } else if (attacker->client->resp.class_type == CLASS_HATCHLING || attacker->client->resp.class_type == CLASS_KAMIKAZE) {
                //different height calc.
                height = point[2] - targ->s.origin[2] + targ->viewheight;
                //if they're crouched it's always the head (+hack)
                if (client->ps.pmove.pm_flags & PMF_DUCKED) {
                    height = 75;
                }

                if (height > 66) {
                    take *= 1.275f; //head hit - note no multiplier to grunt since we want a body to infest.
                    //if (classlist[targ->client->resp.class_type].frags_needed < 5 && targ->client->resp.class_type != CLASS_GRUNT)
                    //      take *= 2;
                    //gi.bprintf (PRINT_HIGH, "height %d = head\n", height);
                } else if (height > 50) {
                    take *= 1.08f; //neck hit
                    //gi.bprintf (PRINT_HIGH, "height %d = neck\n", height);
                } else if (height > 22) {
                    take *= 1.02f; //body hit
                    //gi.bprintf (PRINT_HIGH, "height %d = body\n", height);
                } else if (height > 7) {
                    take *= 0.92f; //legs hit
                    //gi.bprintf (PRINT_HIGH, "height %d = legs\n", height);
                } else {
                    take *= 0.85f; //feet hit
                    //gi.bprintf (PRINT_HIGH, "height %d = feet\n", height);
                }
                //gi.bprintf (PRINT_HIGH, "final damage = %d\n", take);
            }
        }

        // if teammate attacked, do tk checking
        if (attacker->client && client->resp.team == attacker->client->resp.team &&
                !(dflags & (DAMAGE_NO_PROTECTION | DAMAGE_FRIENDLY_FIRE)) && targ != attacker) {
            //stop tking continuing
            if (attacker->client->resp.teamkills == -1)
                return false;

            // FIXME: change the reflect to reflect any damage done, not just splash
            // allow team splash damage
            dflags |= DAMAGE_FRIENDLY_FIRE;
            // splash damage?
            if (mod == MOD_GRENADE || mod == MOD_ROCKET || mod == MOD_R_SPLASH || mod == MOD_C4 || mod == MOD_KAMIKAZE || mod == MOD_EXSHELL ||
                    mod == MOD_GRIM_ROCKET || mod == MOD_DRUNK_MISSILE || mod == MOD_MECH_MISSILE/* || mod == MOD_MAGNUM*/) {
                dflags |= DAMAGE_FRIENDLY_SPLASH;
                // allow splash damage?
                if (teamsplashdamage->value) {

                    //if we hurt someone, check for reflect damage
                    if (teamreflectradiusdamage->value == 2) {
                        //full reflect
                        T_Damage(attacker, attacker, attacker, dir, point, normal, take, 0, DAMAGE_IGNORE_RESISTANCES | DAMAGE_FRIENDLY_FIRE | DAMAGE_NO_KNOCKBACK, mod);
                    } else if (teamreflectradiusdamage->value == 1) {
                        //half reflect
                        T_Damage(attacker, attacker, attacker, dir, point, normal, (int) (take * 0.5), 0, DAMAGE_IGNORE_RESISTANCES | DAMAGE_FRIENDLY_FIRE | DAMAGE_NO_KNOCKBACK, mod);
                    }
                } else {
                    take = 0;
                }
            }
        }

        /* entirely disabled until ridiculous broken cases are fixed
                        // save last info?
                        if (attacker->client &&
                                attacker != targ &&
                                (mod == MOD_SHOTGUN ||
                                mod == MOD_SSHOTGUN ||
                                mod == MOD_MELEE ||
                                mod == MOD_GRENADE ||
                                mod == MOD_R_SPLASH ||
                                mod == MOD_KAMIKAZE ||
                                mod == MOD_RAIL ||
                                mod == MOD_SPIKESPORE ||
                                mod == MOD_C4 ||
                                mod == MOD_EAT_C4 ||
                                mod == MOD_POISON ||
                                mod == MOD_SPIKE ||
                                mod == MOD_JUMPATTACK
                                || mod == MOD_ACID)) {
                                        if (!(attacker->client->resp.team == client->resp.team && (mod == MOD_JUMPATTACK || mod == MOD_ACID || mod == MOD_SPIKE || mod == MOD_MELEE)))
                                        {
                                                client->last_attacker = attacker;
                                                client->last_damage = level.time;
                                                client->last_mod = mod;
                                        }
                        }
         */

        //select type of temp_ent
        if (dflags & DAMAGE_BULLET)
            te_sparks = TE_BULLET_SPARKS;

        //R1: NOTICE! this code is only run when hurting clients!
        //if hurting ent, te_sparks = UNINIT! sending uninit in tempent = crashed clients
        //moving the te_sparks = TE_SPARKS; to the declaration.

        // check any savings, if dmg_no_prot isn't on
        if (!(dflags & DAMAGE_NO_PROTECTION) && take) {
            // check for godmode
            if ((targ->flags & FL_GODMODE)) {
                take = 0;
                save = damage;
                //SpawnDamage (te_sparks, point, normal, save);
            }

            // check for invincibility
            if (client->invincible_framenum > level.framenum && (attacker != targ)) {
                //noise
                /*if (targ->pain_debounce_time < level.time) {
                        gi.sound(targ, CHAN_AUTO, SoundIndex (items_protect4), 1, ATTN_NORM, 0);
                        targ->pain_debounce_time = level.time + 2;
                }*/
                take = 0;
                save = damage;
            }

            // check if this goes thru armor
            if (!(dflags & DAMAGE_NO_ARMOR) && take && !overriden) {
                //power armor
                // unnecessary check for those without power armor
                //psave = CheckPowerArmor (targ, point, normal, take, dflags);
                cells_restore = client->resp.inventory[ITEM_AMMO_CELLS];
                armour_restore = GetArmor(targ);
                if (targ->flags & FL_POWER_ARMOR && (client->resp.class_type == CLASS_ENGINEER || !attacker->client) && attacker->enttype != ENT_OBSTACLE) {
                    int oldtake = take;
                    //int index = ITEM_INDEX(FindItem("Cells"));

                    //gi.bprintf (PRINT_HIGH, "old dmg: %d\n", take);
                    take *= 1.0f - ((float) client->resp.inventory[ITEM_AMMO_CELLS]) / 100.0f;
                    //gi.bprintf (PRINT_HIGH, "new dmg: %d for %d cells\n", take, (int)((oldtake - take)*0.5f));
                    client->resp.inventory[ITEM_AMMO_CELLS] -= (oldtake - take)*0.5f;
                    if (client->resp.class_type == CLASS_EXTERM && client->resp.inventory[ITEM_AMMO_CELLS] < 0) {
#warning "FIXME: overwtfing when real damage doesnt take all cells away"
                        client->resp.inventory[ITEM_AMMO_CELLS] = 0;
                        gi.cprintf(targ, PRINT_HIGH, "Power cells overheated!\n");
                        gi.sound(targ, CHAN_AUTO, SoundIndex(weapons_pulseout), 1, ATTN_NORM, 0);
                        targ->client->last_move_time = level.time + 5;
                        if (targ->client->resp.upgrades & UPGRADE_COOLANT)
                            targ->client->last_move_time -= 2.5f;
                    }
                    client->damage_parmor += (oldtake - take)*0.33f;
                    SpawnDamage(TE_SCREEN_SPARKS, point, normal, (int) (take * 0.33f));

                    if (client->resp.class_type == CLASS_ENGINEER) {
                        client->resp.inventory[ITEM_AMMO_CELLS] -= (take * 0.5f);
                        if (client->resp.inventory[ITEM_AMMO_CELLS] < 0)
                            client->resp.inventory[ITEM_AMMO_CELLS] = 0;
                    } else if (attacker->client && (attacker->client->resp.class_type == CLASS_HATCHLING || attacker->client->resp.class_type == CLASS_KAMIKAZE)) {
                        vec3_t forward;
                        VectorCopy(dir, forward);
                        VectorScale(forward, -300, forward);
                        VectorCopy(forward, attacker->velocity);
                    }
                }

                // normal armor
                asave = CheckArmor(targ, attacker, point, normal, take, te_sparks, dflags, mod, dir);
                take -= asave;
            }

            // allow any teamdamage?
            if (teamdamage->value == 0 && (dflags & DAMAGE_FRIENDLY_FIRE) && !(dflags & DAMAGE_FRIENDLY_SPLASH))
                take = 0;
                //return false;
            else if (teamdamage->value == 1 && attacker->client && classlist[attacker->client->resp.class_type].frags_needed == 0
                    && (dflags & DAMAGE_FRIENDLY_FIRE) && !(dflags & DAMAGE_FRIENDLY_SPLASH))
                // allow teamdamage from 1+ frag classes?
                take = 0;
            //return false;
            // teamdamage > 1, allow all teamdamage

        }

    } else {
        // hurting non-client
        // STRUCTURE STUFF

        //damage flags specify can't hurt non-clients
        if (dflags & DAMAGE_IGNORE_STRUCTS)
            return false;

        //gas doesn't hurt spider structures
        if (targ->svflags & SVF_MONSTER) {
            if (dflags & DAMAGE_GAS && mod != MOD_INFEST)
                return false;
        }

        if (mod == MOD_ACID)
            take *= 0.4f;

        if (targ->enttype == ENT_TRIPWIRE_BOMB) {
            take *= 1.5f; //was 1.5 in 1.4t0.6
            if (mod == MOD_BURNED) take *= 0.75f;
            else
                if (mod == MOD_MINE) take *= 1.8f; //old was 50hp*0.9scale, compensate as mine has 150hp now with a take *= 1.5f;
        }

        //gas doesn't damage structures
        //if (targ->flags & FL_CLIPPING && dflags & DAMAGE_GAS)
        //      return false;

        //              if (targ->svflags & SVF_MONSTER)
        //              {
        if (targ->enttype == ENT_COCOON) {
            if (mod == MOD_PULSELASER) {
                take *= 0.35f;
            } else
                if (mod == MOD_RAIL) {
                take *= 0.4f;
            } else
                if (mod != MOD_MAGNUM && mod != MOD_EGG && !(dflags & DAMAGE_RADIUS) && mod != MOD_ROCKET && mod != MOD_EXSHELL && mod != MOD_HYPERBLASTER && mod != MOD_DRUNK_MISSILE && mod != MOD_GRIM_ROCKET && mod != MOD_MECH_MISSILE) {
                take *= 0.1;
            }
        }
        //                      else if (mod == MOD_GRENADE || mod == MOD_KAMIKAZE)
        //                      {
        //                              take *= 0.5;
        //                      }
        //              }

        //tripwires reduce dmg to other tripwires, can't be set off by fire.

        if (targ->enttype == ENT_TRIPWIRE_BOMB) {
            if (mod == MOD_MINE)
                take /= 8;
            else if (mod == MOD_BURNED) {
                if (randomMT() & 1)
                    take += 8;
            }
        }
        //spikes on proxy spores won't kill tele v. easily
        if (targ->enttype == ENT_TELEPORTER) {
            if (mod == MOD_ACID || mod == MOD_MACHINEGUN)
                return false;
            if (dflags & DAMAGE_RADIUS && mod == MOD_SPIKE)
                take = 35;
            if (mod == MOD_BURNED)
                take += 22;
        }

        // direct attack from client
        if (attacker && attacker->client) {
            //stop tking continuing
            if (attacker->client->resp.teamkills == -1)
                return false;

            if (targ->enttype == ENT_SPIKER) take *= 0.8f;

            if (mod == MOD_AUTOGUN && targ->enttype == ENT_SPIKER)
                take *= 0.8f;

            if (targ->enttype == ENT_TURRET || targ->enttype == ENT_TURRETBASE) {
                if (mod == MOD_ACID)
                    take *= 1.25f;
                else if (mod == MOD_BURNED)
                    take *= 1.7f;
                if (mod == MOD_MELEE)
                    take *= 1.2f;
                else if (mod == MOD_SPIKE)
                    take *= 1.7f;
            }

            if (targ->enttype == ENT_PROXYSPIKE && attacker->client->resp.team == TEAM_ALIEN) {
                if (dflags & DAMAGE_RADIUS)
                    return false;
                take *= 0.66f;
            }

            if (mod == MOD_MELEE)

                switch (targ->enttype) {

                    case ENT_TURRET:
                    case ENT_MGTURRET:
                    case ENT_TURRETBASE:
                        switch (attacker->client->resp.class_type) {

                            case CLASS_GUARDIAN:
                            case CLASS_WRAITH:
                                //guard shouldn't be hitting turrets!
                                take /= 3.5f;
                                break;
                            case CLASS_HATCHLING:
                            case CLASS_KAMIKAZE:
                                take = 40;
                                break;
                            case CLASS_STINGER:
                                take *= 0.6f;
                                break;
                            default:
                                break;
                        }
                        break;
                    case ENT_TELEPORTER:
                        switch (attacker->client->resp.class_type) {

                            case CLASS_WRAITH:
                            case CLASS_HATCHLING:
                            case CLASS_KAMIKAZE:
                                take = 0;
                                break;
                            case CLASS_STINGER:
                                take *= 0.7f;
                                break;
                            case CLASS_DRONE:
                                // punch up drone tele dmg
                                take += 17;
                                break;
                            case CLASS_GUARDIAN:
                                take *= 0.8f; //and fix again, making 4dmg here for guard yields 0 dmg somehow, leaving 34 dmg here does actual 4 dmg to tele
                                break;
                            default:
                                break;
                        }
                        if (mod == MOD_GRENADE || mod == MOD_R_SPLASH || mod == MOD_C4)
                            take *= 0.67f; //tele is no hatchie, shouldnt die as easy as a hatchie from explosions
                        break;
                    default:
                        break;
                }

            // toggle for teamspawndamage
            if (teamspawndamage->value == 0) {
                if (targ->enttype == ENT_TELEPORTER && attacker->client->resp.team == TEAM_HUMAN)
                    take = 0;
                else if (targ->enttype == ENT_COCOON && attacker->client->resp.team == TEAM_ALIEN)
                    take = 0;
            }
        }

        // xmin checks
        if (!(dflags & DAMAGE_NO_PROTECTION)) {
            if (targ->enttype == ENT_TELEPORTER && (targ->flags & FL_MAP_SPAWNED) && level.time < (60 * xmins->value))
                take = 0;
            else if (targ->enttype == ENT_COCOON && (targ->flags & FL_MAP_SPAWNED) && level.time < (60 * xmins->value))
                take = 0;
        }
    }

    if (overriden) {
        damage_health = !(dmg.flags & do_no_health_dmg);
        damage_armour = !(dmg.flags & do_no_armour_dmg);

        if (armour_restore >= 0) {
            client->resp.inventory[ITEM_AMMO_CELLS] = cells_restore;
            if (targ->client->armor)
                targ->client->resp.inventory[ITEM_INDEX(targ->client->armor)] = armour_restore;
        }

        if (!(dmg.flags & do_ignore_armour))
            dflags &= ~DAMAGE_NO_ARMOR;
        else
            dflags |= DAMAGE_NO_ARMOR;

        if (dmg.radius > 1)
            d->radius = dmg.radius;

        if (d->distance < 0) { //not an explosion
            if (dmg.uflags & dou_damage_set) {
                dflags |= DAMAGE_IGNORE_RESISTANCES;
                take = dmg.hitpoints * d->scale*oscale;
                rewardscale = d->scale*oscale;
                canheal = dmg.hitpoints < 0 && targ->health > 0;
            }
            if (dmg.uflags & dou_knockback_set) {
                knockback = dmg.knockback * d->scale*oscale;
                fixknockback = false;
            }

            if (dmg.flags & do_force_explosion) { //but damage is for one
                static edict_t fake_ent[4] = {
                    {0}
                };
                static int fake_num = 0;

                int hold = targ->health;
                int aold = (targ->client && targ->client->armor) ? targ->client->resp.inventory[ITEM_INDEX(targ->client->armor)] : 0;
                int eold = targ->enttype;
                int dold = targ->deadflag;

                fake_ent[fake_num].classname = "explosion";
                fake_ent[fake_num].owner = attacker;
                fake_ent[fake_num].enttype = ENT_FAKE;
                fake_ent[fake_num].count = dmg.weaponindex;
                if (inflictor == attacker || !inflictor) inflictor = &fake_ent[fake_num];
                VectorCopy(point, inflictor->s.origin);

                fake_num = (fake_num + 1) % (sizeof (fake_ent) / sizeof (fake_ent[0]));

                T_RadiusDamage(inflictor, attacker, dmg.hitpoints, inflictor, d->radius, mod, dflags);

                return (hold != targ->health) || ((targ->client && targ->client->armor) ? aold != targ->client->resp.inventory[ITEM_INDEX(targ->client->armor)] : 0) || (eold != targ->enttype) || (dold != targ->deadflag);
            }

        } else { //its an explosion
            dflags |= DAMAGE_RADIUS | ((dmg.flags & do_original_explosion) ? 0 : DAMAGE_LINEAR);

            if (dmg.uflags & dou_damage_set) {
                float a;
                if (!(dmg.flags & do_original_explosion)) {
                    a = d->distance / d->radius;
                    if (a < 0) a = 0;
                    take = dmg.hitpoints * (1 - a) * d->scale*oscale; //do_linear_explosion
                    rewardscale = (1 - a) * d->scale*oscale;
                } else {
                    a = dmg.hitpoints - d->distance;
                    if (a < 0) a = 0;
                    take = a * d->scale*oscale; //points = damage - VectorLength(v);
                    rewardscale = dmg.hitpoints / take;
                }
                dflags |= DAMAGE_IGNORE_RESISTANCES;
                canheal = dmg.hitpoints < 0 && targ->health > 0;
            }

            if (dmg.uflags & dou_knockback_set) {
                if (!(dmg.flags & do_original_explosion))
                    knockback = dmg.knockback * (1 - d->distance / d->radius) * d->scale * oscale; //do_linear_explosion
                else
                    knockback = (dmg.knockback - d->distance) * d->scale*oscale; //knockback = 1.0f*(points = damage - VectorLength(v));
                if (knockback * dmg.knockback < 0) knockback = 0; //dont allow to invert sign, but allow negative knockbacks
                fixknockback = false;
            }
        }

        if (!(dflags & DAMAGE_RADIUS) && !(dmg.flags & do_no_locations) && targ->client) { //locations
            int ct = targ->client->resp.class_type;
            const gclass_t *c = classlist + ct;
            int py = point[2] - targ->s.origin[2] - c->mins[2];

            static DEBUGITEM(damage_locations);

            if ((dflags & DAMAGE_TOUCH) && (inflictor->s.origin[2] + inflictor->maxs[2] <= targ->s.origin[2] + targ->mins[2])) {
                if (Debug(damage_locations)) {
                    if (attacker->client) gi.cprintf(attacker, PRINT_HIGH, "Location: Step over (%d%%)\n", (int) (dmg.lsstep * 100));
                    if (targ->client) gi.cprintf(targ, PRINT_HIGH, "Location: Step over (%d%%)\n", (int) (dmg.lsstep * 100));
                }
                take *= rewardscale = dmg.lsstep;
            } else
                if (c->team == TEAM_ALIEN) {
                if (Debug(damage_locations)) {
                    if (attacker->client) gi.cprintf(attacker, PRINT_HIGH, "Location: Torso (%d%%A)\n", (int) (dmg.lstorso * 100));
                    if (targ->client) gi.cprintf(targ, PRINT_HIGH, "Location: Torso (%d%%A)\n", (int) (dmg.lstorso * 100));
                }
                take *= rewardscale = dmg.lstorso;
            } else { //human
                int hhh = c->maxs[2] - c->viewheight;
                //Workaround for hatchie/kami
                int offset = 12;
                if (attacker->client && (attacker->client->resp.class_type == CLASS_HATCHLING || attacker->client->resp.class_type ==
                        CLASS_KAMIKAZE)) offset = 2;

                if (py > (int) (c->maxs[2] - c->mins[2]) - offset) {
                    if (Debug(damage_locations)) {
                        if (attacker->client) gi.cprintf(attacker, PRINT_HIGH, "Location: Head (%d%%H)\n", (int) (dmg.lshead * 100));
                        if (targ->client) gi.cprintf(targ, PRINT_HIGH, "Location: Head (%d%%H)\n", (int) (dmg.lshead * 100));
                    }
                    take *= rewardscale = dmg.lshead;
                } else
                    if (!(client->ps.pmove.pm_flags & PMF_DUCKED)) {
                    if (py < (int) (c->maxs[2] - c->mins[2]) >> 1) {
                        if (Debug(damage_locations)) {
                            if (attacker->client) gi.cprintf(attacker, PRINT_HIGH, "Location: Legs (%d%%H)\n", (int) (dmg.lslegs * 100));
                            if (targ->client) gi.cprintf(targ, PRINT_HIGH, "Location: Legs (%d%%H)\n", (int) (dmg.lslegs * 100));
                        }
                        take *= rewardscale = dmg.lslegs;
                    } else {
                        if (Debug(damage_locations)) {
                            if (attacker->client) gi.cprintf(attacker, PRINT_HIGH, "Location: Torso (%d%%H)\n", (int) (dmg.lstorso * 100));
                            if (targ->client) gi.cprintf(targ, PRINT_HIGH, "Location: Torso (%d%%H)\n", (int) (dmg.lstorso * 100));
                        }
                        take *= rewardscale = dmg.lstorso;
                    }
                } else {
                    if (Debug(damage_locations)) {
                        if (attacker->client) gi.cprintf(attacker, PRINT_HIGH, "Location: Crouch (%.1f%%H)\n", (dmg.lstorso + dmg.lslegs)*50);
                        if (targ->client) gi.cprintf(targ, PRINT_HIGH, "Location: Crouch (%.1f%%H)\n", (dmg.lstorso + dmg.lslegs)*50);
                    }

                    take *= rewardscale = (dmg.lstorso + dmg.lslegs)*0.5f;
                }
            } //human locations
        } // locations

        if (dmg.flags & do_mass_scale) {
            float s = 1;
            if (inflictor == attacker && attacker->mass > 0) s *= attacker->mass;
            if (targ->mass > 0) s /= targ->mass;

            take *= s;
        }

        if (dmg.flags & do_volume_scale) {
            float s = 0.5f / 72; //grunt size
            vec3_t box;
            VectorSubtract(targ->maxs, targ->mins, box);
            float len = VectorLength(box);
            if (len > 1)
                take *= (0.5f + len * s);
        }

        if (dmg.flags & do_piercing)
            dflags |= DAMAGE_PIERCING;
        else
            dflags &= ~DAMAGE_PIERCING;

        int tkdmg = 0;

        int attackerteam = TEAM_NONE;

        if (take < 0 && canheal) { //ugly hack for wraith spit healing
            if (targ->client && ((int) dmflags->value & DF_SLOW_HEALTH_PACK)) {
                if (targ->health + targ->client->resp.health_regen < targ->max_health) {
                    targ->client->resp.health_regen -= take;
                    if (targ->health + targ->client->resp.health_regen > targ->max_health)
                        targ->client->resp.health_regen = targ->max_health - targ->health;
                }
            } else
                if (targ->health < targ->max_health) {
                targ->health -= take;
                if (targ->health > targ->max_health)
                    targ->health = targ->max_health;
            }

            if (Debug(damages))
                gi.dprintf("T_Damage: %s(%s) vs %s: od=%d heal=%d(->%d) type=%s distance=%.2f/%.2f %s\n",
                    attacker->classname, inflictor ? inflictor->classname : "", targ->classname,
                    overriden, -take, targ->health + (targ->client ? targ->client->resp.health_regen : 0),
                    GetWeaponName(dmg.weaponindex), d->distance, d->radius,
                    (d->distance < 0) ? "Hit" : (dflags & DAMAGE_NO_FALLOFF) ? "NoF" : (dflags & DAMAGE_LINEAR) ? "Linear" : "Old"
                    );

            return -1;
        }

        if (attacker->client)
            attackerteam = attacker->client->resp.team;
        else
            if (attacker->die) {
            if (attacker->svflags & SVF_MONSTER) attackerteam = TEAM_ALIEN;
            else
                if (attacker->flags & FL_CLIPPING) attackerteam = TEAM_HUMAN;
        }

        if (targ == attacker) {
            tk = 0;
            tkdmg = 1;
            if (dmg.flags & do_no_self_dmg)
                take = 0;
        } else
            if (targ->client) {
            if (targ->client->resp.team == attackerteam) {
                //Special handing: C4 revenge converts alien tks into human frags
                if (dmg.flags & do_revenge && attacker->client && attacker->client->acid_attacker && attacker->client->acid_attacker->inuse && attacker->client->acid_attacker->client && attacker->client->acid_attacker->client->resp.team && attacker->client->acid_attacker->client->resp.team != attackerteam)
                    return T_Damage(targ, inflictor, attacker->client->acid_attacker, dir, point, normal, damage, knockback, dflags, mod);

                tk = 1;
                tkdmg = teamdamage->value;
            }
            if (dmg.flags & do_no_tk_players) tkdmg = 0;
            if (dmg.flags & do_tk_players) tkdmg = 1;

            if (!tkdmg && tk) take = 0;
        } else
            if (targ->die) {
            int team = TEAM_NONE;
            if (targ->svflags & SVF_MONSTER) team = TEAM_ALIEN;
            else
                if (targ->flags & FL_CLIPPING) team = TEAM_HUMAN;

            if (team == attackerteam) {

                //Special handing: C4 revenge converts alien tks into human frags
                if (dmg.flags & do_revenge && attacker->client && attacker->client->acid_attacker && attacker->client->acid_attacker->inuse && attacker->client->acid_attacker->client && attacker->client->acid_attacker->client->resp.team && attacker->client->acid_attacker->client->resp.team != attackerteam)
                    return T_Damage(targ, inflictor, attacker->client->acid_attacker, dir, point, normal, damage, knockback, dflags, mod);

                tk = 1;
                tkdmg = teamstructdamage->value;
            }

            if (dmg.flags & do_no_tk_structures) tkdmg = 0;
            if (dmg.flags & do_tk_structures) tkdmg = 1;

            if (!tkdmg && tk) take = 0;
        }

        if (targ->flags & FL_GODMODE || targ->client && targ->client->invincible_framenum > level.framenum) {
            save = take;
            take = 0;
        }


        if (!(dflags & DAMAGE_NO_ARMOR) && take > 0) {
            if (targ->flags & FL_POWER_ARMOR && (client->resp.class_type == CLASS_ENGINEER || !attacker->client) && attacker->enttype != ENT_OBSTACLE) {
                int oldtake = take;
                take *= 1.0f - ((float) client->resp.inventory[ITEM_AMMO_CELLS]) / 100.0f;
                client->resp.inventory[ITEM_AMMO_CELLS] -= (oldtake - take)*0.5f;
                if (client->resp.class_type == CLASS_EXTERM && client->resp.inventory[ITEM_AMMO_CELLS] < 0) {
                    client->resp.inventory[ITEM_AMMO_CELLS] = 0;
                    gi.cprintf(targ, PRINT_HIGH, "Power cells overheated!\n");
                    gi.sound(targ, CHAN_AUTO, SoundIndex(weapons_pulseout), 1, ATTN_NORM, 0);
                    targ->client->last_move_time = level.time + 5;
                    if (targ->client->resp.upgrades & UPGRADE_COOLANT)
                        targ->client->last_move_time -= 2.5f;
                }
                client->damage_parmor += (oldtake - take)*0.33f;
                SpawnDamage(TE_SCREEN_SPARKS, point, normal, (int) (take * 0.33f));

                if (client->resp.class_type == CLASS_ENGINEER) {
                    client->resp.inventory[ITEM_AMMO_CELLS] -= (take * 0.5f);
                    if (client->resp.inventory[ITEM_AMMO_CELLS] < 0)
                        client->resp.inventory[ITEM_AMMO_CELLS] = 0;
                } else
                    if (attacker->client && (attacker->client->resp.class_type == CLASS_HATCHLING || attacker->client->resp.class_type == CLASS_KAMIKAZE)) {
                    vec3_t forward;
                    VectorCopy(dir, forward);
                    VectorScale(forward, -300, forward);
                    VectorCopy(forward, attacker->velocity);
                }
            }

            asave = 0;
            if (targ->client && targ->client->armor) {
                if (damage_armour || !targ->client || !targ->client->armor) {
                    if (dmg.ascale >= 0)
                        asave = ArmourProtect(targ, attacker, point, normal, take, te_sparks, dflags, mod, dir, dmg.ascale);
                    else
                        asave = CheckArmor(targ, attacker, point, normal, take, te_sparks, dflags, mod, dir);
                } else {
                    int acount = targ->client->resp.inventory[ITEM_INDEX(targ->client->armor)];
                    if (dmg.ascale >= 0)
                        asave = ArmourProtect(targ, attacker, point, normal, take, te_sparks, dflags, mod, dir, dmg.ascale);
                    else
                        asave = CheckArmor(targ, attacker, point, normal, take, te_sparks, dflags, mod, dir);
                    targ->client->resp.inventory[ITEM_INDEX(targ->client->armor)] = acount;
                }
                take -= asave;
            }
        }
    } //dmg_override && mod != MOD_CHANGECLASS


    //entity class damage
    if (targ->hurtflags) {
        if (!attacker->client)
            return false;

        switch (mod) {
            case MOD_JUMPATTACK:
            case MOD_MECHSQUISH:
            case MOD_MECHSQUISH_SQUISH:
            case MOD_RUN_OVER:
                if (!(targ->hurtflags & WC_JUMPATTACK)) {
                    return false;
                }
                break;
            case MOD_MELEE:
                if (!(targ->hurtflags & WC_MELEE)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_AUTOGUN:
                if (!(targ->hurtflags & WC_AUTOGUN)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_POKED:
                if (!(targ->hurtflags & WC_POKE)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_PISTOL:
                if (!(targ->hurtflags & WC_PISTOL)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_SHOTGUN:
                if (!(targ->hurtflags & WC_SHOTGUN)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_SSHOTGUN:
                if (!(targ->hurtflags & WC_SCATTER)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_MACHINEGUN:
                if (!(targ->hurtflags & WC_SMG)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_MAGNUM:
                if (!(targ->hurtflags & WC_MAGNUM)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_EXSHELL:
                if (!(targ->hurtflags & WC_EXSHELL)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_R_SPLASH:
            case MOD_ROCKET:
            case MOD_DRUNK_MISSILE:
            case MOD_GRIM_ROCKET:
            case MOD_MECH_MISSILE:
                if (!(targ->hurtflags & WC_ROCKET)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_GRENADE:
                if (!(targ->hurtflags & WC_GRENADE)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_C4:
                if (!(targ->hurtflags & WC_C4)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_PULSELASER:
                if (!(targ->hurtflags & WC_PULSE)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_RAIL:
                if (!(targ->hurtflags & WC_DISCHARGE)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_HYPERBLASTER:
                if (!(targ->hurtflags & WC_AUTOCANNON)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_FLASH:
                if (!(targ->hurtflags & WC_FLASHGRENADE)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_SPIKESPORE:
                if (!(targ->hurtflags & WC_SPIKESPORE)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_SPIKE:
                if (!(targ->hurtflags & WC_SPIKES)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_GAS_SPORE:
                if (!(targ->hurtflags & WC_GAS)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_BURNED:
                if (!(targ->hurtflags & WC_FIRE)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_ACID:
                if (!(targ->hurtflags & WC_WRAITHSPIT)) {
                    HURTCLASS_SPARKS
                }
                break;
            case MOD_KAMIKAZE:
                if (!(targ->hurtflags & WC_KAMI)) {
                    HURTCLASS_SPARKS
                }
                break;
            default:
                HURTCLASS_SPARKS
        }
    }

    //classes field on things like func_explosive
    if (targ->style) {
        //can only be killed by players
        if (!attacker->client)
            return false;

        //draw "youcan'thurtme" sparks
        if ((!(targ->flags & FL_NO_HURTSPARKS)) && ((targ->style) & classtypebit[attacker->client->resp.class_type])) {
            SpawnDamage2(TE_LASER_SPARKS, point, normal, 5, 3);
            return false;
        }
    }

    if (targ->flags & FL_NO_KNOCKBACK) {
        //they aren't supposed to be knockedback
        knockback = 0;
    } else {
        // figure momentum add
        if (!(dflags & DAMAGE_NO_KNOCKBACK)) {
            if ((knockback) && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) && (targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP)) {
                vec3_t kvel;
                float mass;

                if (targ->mass < 50)
                    mass = 50;
                else
                    mass = targ->mass;

                // ???
                //if (client)
                //      mass = 250;]

                if (fixknockback) {
                    //r1: fix for overly powerful greande knock
                    if (mod == MOD_GRENADE || mod == MOD_R_SPLASH)
                        knockback *= 0.25f;

                    if (mod == MOD_SSHOTGUN && targ->client)
                        knockback /= (1 + (classlist[targ->client->resp.class_type].frags_needed)*0.2f);

                    if (mod == MOD_JUMPATTACK)
                        knockback *= 0.5f;
                }

                //gi.bprintf (PRINT_HIGH, "knockback: %d (dir %s)\n", knockback, vtos(dir));
                VectorScale(dir, 500 * (float) knockback / mass, kvel);
                VectorAdd(targ->velocity, kvel, targ->velocity);
            }
        }
    }

    //they lose invuln if they are attacking
    if (attacker->client && mod != MOD_CHANGECLASS) {
        if (attacker->client->invincible_framenum > level.framenum)
            attacker->client->invincible_framenum = level.framenum;

        attacker->client->resp.can_respawn = false;
    }

    //ignore resistances flag (water/lava/slime/some others)
    // if ignore_res is on, all damage should be applied without any resistance calcs, eg. pure damage
    if (targ->damage_absorb && !(dflags & DAMAGE_IGNORE_RESISTANCES))
        take -= targ->damage_absorb;

    /*if (targ->client && targ->client->resp.class_type == CLASS_EXTERM) {
            client->resp.inventory[client->ammo_index] -= damage * 0.2f;
            if (client->resp.inventory[client->ammo_index] < 0)
                    client->resp.inventory[client->ammo_index] = 0;
    }*/

    meansOfDeath = mod;

    { //New lastdamage logging
        vec3_t k;
        int dmg = take;
        VectorNormalize2(dir, k);
        VectorScale(k, knockback, k);
        if (dmg > targ->health) dmg = targ->health;
        if (dmg < 0) dmg = 0;
        LogDamage(targ, attacker, k, asave + dmg, mod);
    }


    //don't want to be giving them health...
    if (take <= 0) {
        take = 0;
        if (!asave && !save) return false; //nothing to do
    }

    if (Debug(damages))
        gi.dprintf("T_Damage: %s(%s) vs %s: od=%d dmg=%d(%da) kick=%d type=%s distance=%.2f/%.2f %s\n",
            attacker->classname, inflictor ? inflictor->classname : "", targ->classname,
            overriden, take, asave, knockback,
            GetWeaponName(dmg.weaponindex), d->distance, d->radius,
            (d->distance < 0) ? "Hit" : (dflags & DAMAGE_NO_FALLOFF) ? "NoF" : (dflags & DAMAGE_LINEAR) ? "Linear" : "Old"
            );

    if (targ->client && (asave > 0 || take > 0)) {
        if (targ->client->resp.health_regen_step > 0) {
            int n = take / 5;
            if (n < targ->client->resp.health_regen_step)
                targ->client->resp.health_regen_step -= n;
            else
                targ->client->resp.health_regen_step = 0;
            if (n && !targ->client->resp.health_regen_accel)
                targ->client->resp.health_regen_accel = 1;
        }
        targ->client->resp.health_boost_step = 0;
        if (targ->client->resp.class_type == CLASS_HEAVY)
            targ->client->missile_target = 0;
    }

    //gi.bprintf (PRINT_HIGH, "final dmg = %d\n", take);

    //treat cheat/powerup savings the same as armor
    asave += save;

    if (take) {
        if (client) {

            switch (client->resp.class_type) {

                case CLASS_EXTERM:
                case CLASS_MECH:
                case CLASS_ENGINEER:
                    //spawn "oil"
                    gi.WriteByte(svc_temp_entity);
                    gi.WriteByte(TE_LASER_SPARKS);
                    gi.WriteByte(20);
                    gi.WritePosition(targ->s.origin);
                    gi.WriteDir(normal);
                    gi.WriteByte(0);
                    gi.multicast(targ->s.origin, MULTICAST_PVS);

                    break;

                case CLASS_GUARDIAN:

                    if (targ->health > 0) {
                        //make guardian visible
                        targ->s.modelindex = 255;
                        targ->fly_sound_debounce_time = level.framenum + 5;
                    }
                    SpawnDamage(TE_BLOOD, point, normal, take);
                    break;

                case CLASS_COMMANDO:

                    if (client->resp.primed) {
                        //make command invuln. die
                        if (client->invincible_framenum > level.framenum)
                            client->invincible_framenum = level.framenum;

                        //ouch! damnit i dropped the c4!
                        if (mod == MOD_FALLING && take > 10) {
                            targ->dmg = 300;
                            targ->dmg_radius = 200;
                            client->resp.primed = 0;
                            kamikaze_explode(targ, MOD_R_SPLASH, 0);
                        } else if (take > 0) {
                            const gitem_t *item;
                            item = FindItem("C4 Explosive");
                            Toss_C4(targ, item);
                        }
                    }
                    SpawnDamage(TE_BLOOD, point, normal, take);
                    break;

                default:
                    //bleed
                    SpawnDamage(TE_BLOOD, point, normal, take);
                    break;
            }

        } else {

            if (targ->svflags & SVF_MONSTER || targ->enttype == ENT_CORPSE || targ->enttype == ENT_HUMAN_BODY || targ->enttype == ENT_FEMALE_BODY) {
                //bleed
                SpawnDamage(TE_BLOOD, point, normal, take);
            } else {
                //armor sparks
                SpawnDamage(te_sparks, point, normal, take);
            }
        }

#ifndef ONECALL
        //gi.dprintf("damage inflicted: %d\n", take);
#endif
        // inflict damage
        if (damage_health)
            targ->health -= take;

        meansOfDeath = mod;
        newhs.health = targ->health;
        newhs.armour = GetArmor(targ);
        newhs.powershield = newhs.damage = 0;
        if (targ->flags & FL_POWER_ARMOR) newhs.powershield = client->resp.inventory[ITEM_AMMO_CELLS];
        newhs.damage = oldhs.health - newhs.health + oldhs.armour - newhs.armour + oldhs.powershield - newhs.powershield;
        //newhs.damage = take;

        //did we kill them?
        if (targ->health <= 0) {
            int wasdead = targ->health + take <= 0;

            if (targ->client && targ->health < 0 && !wasdead && mod != MOD_TELEFRAG && mod != MOD_FALLING) { //split damage in 2 so damage vs corpses uses proper scaling and gib flags
                int extra = -targ->health;
                targ->health = 0;

                if (targ->die) PlayerKilled(targ, inflictor, attacker, point, &oldhs, &newhs); //kill player

                if (targ->client->resp.body_entnum) d->targ = targ = g_edicts + targ->client->resp.body_entnum;

                if (targ->takedamage) { //apply the rest to the corpse
                    d->scale *= extra * 1.0f / take;
                    Damage(d);
                }

                if (targ->health < -999)
                    targ->health = -999;
            } else
                if (targ->die)
                AnyKilled(targ, inflictor, attacker, point, &oldhs, &newhs);

            if (score_override && attacker->client && !wasdead && targ->health <= 0) {
                gclient_t*a = attacker->client;
                if (targ != attacker && SETTING_INT(fragmode) != FM_DPOINTS) {
                    a->resp.score += dmg.frags;
                    a->resp.total_score += dmg.score;
                }
                if (tk) {
                    if (targ != attacker) {
                        int extra = a->resp.lasttkcounter * dmg.fragsper10tkpoint * 0.1f;
                        if (targ->client && extra > classlist[targ->client->resp.class_type].frags_needed)
                            extra = classlist[targ->client->resp.class_type].frags_needed;
                        if (SETTING_INT(fragmode) != FM_DPOINTS)
                            a->resp.score -= extra;
                    } else
                        if (SETTING_INT(fragmode) != FM_DPOINTS)
                        a->resp.score--;
                }
                a->resp.teamkills += dmg.tkcounter;
                if (a->resp.teamkills < 0) a->resp.teamkills = 0;
                if (dmg.tkcounter > 0) CheckTKs(attacker);
            }
            if (SETTING_INT(fragmode) & FM_DPOINTS && attacker->client)
                RewardDamage(targ, inflictor, attacker, oldhs, newhs, mod, rewardscale);
            return true;
        } else
            if (targ->pain && !(targ->flags & FL_GODMODE)) {
            currentinflictor = inflictor; //eek
            targ->pain(targ, attacker, knockback, take);
        }
    }

    // add to the damage inflicted on a player this frame
    // the total will be turned into screen blends and view angle kicks
    // at the end of the frame
    if (client && (!(!take && dflags & (DAMAGE_FRIENDLY_FIRE)) || (dflags & DAMAGE_FRIENDLY_SPLASH))) {
        //if (client->resp.class_type == CLASS_BREEDER && targ->client->build_timer - level.time < 1)
        //      targ->client->build_timer = level.time + 1;

        client->damage_parmor += psave;
        client->damage_armor += asave;
        client->damage_blood += take;
        client->damage_knockback += knockback;
        VectorCopy(point, client->damage_from);
        client->healthinc_wait = level.framenum + 20;
    }
    if (SETTING_INT(fragmode) & FM_DPOINTS && attacker->client) {
        newhs.health = targ->health;
        newhs.armour = GetArmor(targ);
        newhs.powershield = 0;
        if (targ->flags & FL_POWER_ARMOR) newhs.powershield = client->resp.inventory[ITEM_AMMO_CELLS];
        newhs.damage = oldhs.health - newhs.health + oldhs.armour - newhs.armour + oldhs.powershield - newhs.powershield;
        RewardDamage(targ, inflictor, attacker, oldhs, newhs, mod, rewardscale);
    }
    //we did the damage
    return true;
}

qboolean T_Damage(edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod) {
    damage_t d;

    d.targ = targ;
    d.inflictor = inflictor;
    d.attacker = attacker;
    VectorCopy(dir, d.dir);
    VectorCopy(point, d.point);
    if (normal)
        VectorCopy(normal, d.normal);
    else {
        float s = VectorLength(dir);
        if (s > 0) s = -1.0f / s;
        VectorScale(dir, s, d.normal);
    }
    d.damage = damage;
    d.scale = 1.0f;
    d.distance = -1;
    d.radius = -1;
    d.knockback = knockback;
    d.dflags = dflags;
    d.mod = mod;

    DAMAGE_FIELDS(13); //valid fields set above to make sure if some new is added its not left uninitialized

    return Damage(&d);
}

/*
T_RadiusDamage
 */
void T_RadiusDamage(edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod, int dflags) {
    float prod, splash; //for blending
    int points;
    edict_t *ent = NULL;
    vec3_t v;
    float knockback;
    vec3_t dir, normal;
    explosioninfo_t ei;
    qboolean od = false;
    vec3_t icentre; //inflictor's centre (instead of model origin)
    float extra = 0;
    vec3_t origin;

    static DEBUGITEM(radius_damage); //debug the call itself
    static DEBUGITEM(radius_damages); //debugs the damages produced

    damage_t d;

    od = (dmg_override && GetExplosionInfo(&ei, inflictor, attacker, mod));
    if (od) {
        radius = ei.radius;
        extra = 64; //since explosion is to closest point, not to centre, need some extra margin for findradius call
    }

    if (Debug(radius_damage))
        gi.dprintf("T_RadiusDamage(i=%d<%s>, a=%d<%s>, d=%.1f, i=%s, r=%.1f, m=%d, f=%d) mode=%s\n",
            inflictor - g_edicts, inflictor->classname,
            attacker - g_edicts, attacker->classname,
            damage,
            ignore ? va("%d<%s>", ignore - g_edicts, ignore->classname) : "NULL",
            radius, mod, dflags,
            (dflags & DAMAGE_NO_FALLOFF) ? "NoF" : (dflags & DAMAGE_LINEAR) ? "Linear" : "Old"
            );

    dflags |= DAMAGE_RADIUS;

    VectorAdd(inflictor->mins, inflictor->maxs, v);
    VectorMA(inflictor->s.origin, 0.5f, v, icentre);

    //r1: small optimz.
    if (dflags & DAMAGE_IGNORE_CLIENTS)
        ent = g_edicts + game.maxclients;

    VectorCopy(inflictor->s.origin, origin);

    // FIXME: slow, maybe should use gi.pvs?
    while ((ent = findradius(ent, origin, radius + extra)) != NULL) {
        if (ent == ignore)
            continue;
        if (!ent->takedamage)
            continue;
        if (dflags & DAMAGE_IGNORE_STRUCTS && !ent->client)
            continue;

        if (!(dflags & DAMAGE_NO_FALLOFF)) {
            vec3_t s, c;
            float ds; //distance scale
            VectorAdd(ent->mins, ent->maxs, s); //size
            VectorMA(ent->s.origin, 0.5f, s, c); //centre

            VectorSubtract(icentre, c, v);

            d.distance = VectorLength(v) - VectorLength(s)*0.5f;
            if (d.distance < 0) d.distance = 0;

            if ((dflags & DAMAGE_LINEAR)) {
                points = damage * (1.0f - d.distance / radius);
            } else {
                VectorAdd(ent->mins, ent->maxs, v);
                VectorMA(ent->s.origin, 0.5f, v, v);
                VectorSubtract(origin, v, v);

                points = damage - VectorLength(v);
            }
            if (points < 0) points = 0;
            knockback = points * 1.0f;
            splash = 10 * points / damage;
        } else {
            points = damage;
            knockback = points * 1.0f;
            splash = 10 * points / damage;
            d.distance = 0;
        }


        // the friendly fire flag tells radiusdamage to not hurt (used by engi destroying mines)
        if (dflags & DAMAGE_FRIENDLY_FIRE)
            points = 0;

        if (Debug(radius_damages))
            gi.dprintf("T_RadiusDamage: %s(%s) vs %s: dmg=%d of %d, dist=%d of %d\n",
                attacker->classname, inflictor ? inflictor->classname : "", ent->classname,
                (int) points, (int) damage,
                (int) d.distance, (int) radius
                );

        // if distance to ent too great, skip it (fixes alpha blending on no damage)
        if (points <= 0 && fabs(knockback < 1))
            continue;


        VectorSubtract(ent->s.origin, origin, dir);
        if (CanDamage(ent, inflictor)) {
            if (ent->client) {
                if (mod == MOD_R_SPLASH) {
                    prod = -DotProduct(dir, ent->client->v_angle);
                    if (prod > 0) {
                        ent->client->blinded_alpha += prod * splash;
                        if (ent->client->blinded_alpha > 1.0)
                            ent->client->blinded_alpha = 1.0;
                        if (ent->client->blinded_alpha < 0.3)
                            ent->client->blinded_alpha = 0.3;
                    }
                } else if (inflictor->enttype == ENT_FLASH_GRENADE) {
                    ent->client->blind_time = (5 - (classlist[ent->client->resp.class_type].frags_needed / 2)) / 2;
                    ent->client->blind_time_alpha = 1;
                    if (ent->client->resp.team == TEAM_HUMAN) {
                        ent->client->blind_time /= 2;
                        ent->client->blind_time_alpha = 0.75;
                    } else {
                        int i;
                        float dot;
                        vec3_t forward, vec, viewpos;

                        if (inflictor->owner && inflictor->owner->client && (inflictor->owner->client->resp.upgrades & UPGRADE_GLOW_GRENADE)) {
                            ent->client->glow_time = level.time + 150;
                        }



                        //ent->client->frozenmode =  PMF_NO_PREDICTION;
                        //ent->client->frozentime += 1.25;
                        VectorCopy(ent->s.origin, viewpos);
                        viewpos[2] += ent->viewheight;
                        AngleVectors(ent->s.angles, forward, NULL, NULL);
                        VectorSubtract(origin, viewpos, vec);
                        VectorNormalize(vec);
                        dot = DotProduct(vec, forward);
                        if (dot > 0.5f) {
                            for (i = 0; i < 2; i++)
                                ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(crandom()*(180 / (classlist[ent->client->resp.class_type].frags_needed + 1)) - ent->client->resp.cmd_angles[i]);
                        }
                    }

                    knockback = 0;
                    if (!od) points = 10;
                }
            } else {
                //r1: stop flash grenades being micro nukes to structures
                if (!od && inflictor->enttype == ENT_FLASH_GRENADE)
                    points = 20;
            }


            VectorNormalize(dir);
            normal[0] = -dir[0];
            normal[1] = -dir[1];
            normal[2] = -dir[2];

            d.targ = ent;
            VectorCopy(dir, d.dir);
            d.point[0] = ent->s.origin[0] + ((d.dir[0] > 0) ? ent->maxs[0] : -ent->mins[0]) * dir[0];
            d.point[1] = ent->s.origin[1] + ((d.dir[1] > 0) ? ent->maxs[1] : -ent->mins[1]) * dir[1];
            d.point[2] = ent->s.origin[2] + ((d.dir[2] > 0) ? ent->maxs[2] : -ent->mins[2]) * dir[2];
            VectorScale(dir, -1, d.normal);
            d.damage = points;
            d.knockback = knockback;

            d.inflictor = inflictor;
            d.attacker = attacker;
            d.scale = 1;
            d.radius = radius;
            d.dflags = dflags;
            d.mod = mod;
            DAMAGE_FIELDS(13); //valid fields set above to make sure if some new is added its not left uninitialized

            Damage(&d);
            //T_Damage (ent, inflictor, attacker, dir, ent->s.origin, normal, points, (int)knockback, dflags, mod);
        }
    }
}

void T_GasDamage(edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod, int dflags) {
    edict_t *ent = NULL;
    vec3_t dir, v;
    int points;

    if (damage == 0)
        return;

    if (dmg_override && GetExplosionInfo(NULL, inflictor, attacker, mod)) {
        T_RadiusDamage(inflictor, attacker, damage, ignore, radius, mod, dflags | DAMAGE_LINEAR);
        return;
    }

    while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL) {
        if (ent == ignore)
            continue;
        if (!ent->takedamage)
            continue;
        if (!ent->client && !attacker->client)
            continue;
        //if (ent->enttype == ENT_NOBLOOD_BODY || ent->enttype == ENT_FEMALE_BODY || ent->enttype == ENT_HUMAN_BODY || ent->enttype == ENT_CORPSE)
        //      continue;

        points = damage;

        VectorAdd(ent->mins, ent->maxs, v);
        VectorMA(ent->s.origin, 0.5f, v, v);
        VectorSubtract(inflictor->s.origin, v, v);

        points *= 1 - (VectorLength(v) / radius);
        //gi.bprintf (PRINT_HIGH, "inflicting %s, radius = %f, dmg = %d\n", ent->classname, VectorLength (v), points);
        // if distance to ent too great, skip it (fixes alpha blending on no damage)
        if (points <= 0)
            continue;



        if (CanDamage(ent, inflictor)) {
            if (ent->enttype == ENT_TRIPWIRE_BOMB)
                points *= 0.65f;
            else if (ent->enttype == ENT_TURRET || ent->enttype == ENT_TURRETBASE)
                points *= 0.7f;
            else if (ent->enttype == ENT_AMMO_DEPOT)
                points *= 0.75f;
            else if (ent->enttype == ENT_MGTURRET)
                points *= 0.18f;

            if (ent->enttype == ENT_SPIKE)
                continue;

            if (ent->enttype == ENT_TRIPWIRE_BOMB)
                points += 11;

            //r1: set dir! uninit memory otherwise... eek
            VectorSubtract(ent->s.origin, inflictor->s.origin, dir);

            T_Damage(ent, inflictor, attacker, dir, ent->s.origin, vec3_origin, points, damage, DAMAGE_RADIUS | DAMAGE_GAS | DAMAGE_NO_KNOCKBACK | DAMAGE_NO_ARMOR | dflags, mod);
        }
    }
}

void T_InfestDamage(edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod, int dflags) {
    edict_t *ent = NULL;
    vec3_t dir, v;
    float points;

    /*if(damage == 0.0)
            return;*/

    if (dmg_override && GetExplosionInfo(NULL, inflictor, attacker, mod)) {
        T_RadiusDamage(inflictor, attacker, damage, ignore, radius * 2, mod, dflags | DAMAGE_LINEAR);
        return;
    }

    while ((ent = findradius_c(ent, inflictor, radius * 2)) != NULL) {
        if (ent == ignore)
            continue;

        if (!ent->takedamage)
            continue;

        if (ent->client->resp.team != TEAM_HUMAN)
            continue;

        //points = damage;

        VectorAdd(ent->mins, ent->maxs, v);
        VectorMA(ent->s.origin, 0.5f, v, v);
        VectorSubtract(inflictor->s.origin, v, v);

        points = ((radius * 2) - VectorLength(v)) / (radius * 2);

        VectorSubtract(ent->s.origin, inflictor->s.origin, dir);

        if (CanDamage(ent, inflictor))
            T_Damage(ent, inflictor, attacker, dir, ent->s.origin, vec3_origin, damage * points, damage * points, DAMAGE_RADIUS | DAMAGE_GAS | DAMAGE_NO_KNOCKBACK | dflags, mod);
    }
}

int BruteForceBoxEntities(vec3_t mins, vec3_t maxs, edict_t**ents, int count) {
    int maxcount = count;
    int i;
    edict_t*e;

    for (i = 0; count && i < globals.num_edicts; i++) {
        e = g_edicts + i;
        if (!e->inuse) continue;

        if (e->absmax[0] >= mins[0] && e->absmin[0] <= maxs[0] &&
                e->absmax[1] >= mins[1] && e->absmin[1] <= maxs[1] &&
                e->absmax[2] >= mins[2] && e->absmin[2] <= maxs[2]) {
            *(ents++) = e;
            count--;
        }
    }

    return maxcount - count;
}




//Some gloom++ code preview ;)

float LineClosestPoint(vec3_t start, vec3_t end, vec3_t point, vec3_t out_point, qboolean clampsegment) {
    float f = (point[0] - start[0])*(end[0] - start[0]) + (point[1] - start[1])*(end[1] - start[1]) + (point[2] - start[2])*(end[2] - start[2]);
    f /= (end[0] - start[0])*(end[0] - start[0]) + (end[1] - start[1])*(end[1] - start[1]) + (end[2] - start[2])*(end[2] - start[2]);

    if (clampsegment) {
        if (f < 0) f = 0;
        else
            if (f > 1) f = 1;
    }

    out_point[0] = start[0]*(1 - f) + end[0] * f;
    out_point[1] = start[1]*(1 - f) + end[1] * f;
    out_point[2] = start[2]*(1 - f) + end[2] * f;

    return f;
}

static float SRD_PointFraction(vec3_t point, float radius, vec3_t centre, vec3_t size) {
    vec3_t d;
    int i;

    //distance to the other's ellipsoid surface from the point
    VectorSubtract(centre, point, d);
    float distance = VectorNormalize(d);
    for (i = 0; i < 4; i++)
        distance -= size[i] * d[i];

    //Corresponding fraction
    if (distance < 0) return 1.0f;
    if (distance >= radius) return 0.0f;
    return 1 - distance / radius;
}

int SegmentRadiusDamage(vec3_t start, vec3_t end, edict_t*inflictor, edict_t*attacker, float damage, float knockback, edict_t*ignore, float radius, int mod, int dflags) {
    vec3_t mins, maxs, s0, s1, normal;
    int i, count, oth;
    edict_t * others[1024];
    float s1q, fraction;
    int result = 0;
    damage_t d;

    static DEBUGITEM(segmentdmg);
    static DEBUGITEM(segmentlines);

    dflags |= DAMAGE_RADIUS | DAMAGE_LINEAR;

    //q2gloom damage overrides
    explosioninfo_t ei;
    qboolean od = (dmg_override && GetExplosionInfo(&ei, inflictor, attacker, mod));
    if (od) radius = ei.radius;

    //BoxEntiteis area
    for (i = 0; i < 3; i++) {
        if (start[i] < end[i]) {
            mins[i] = start[i] - radius;
            maxs[i] = end[i] + radius;
        } else {
            mins[i] = end[i] - radius;
            maxs[i] = start[i] + radius;
        }
    }

    if (Debug(segmentlines)) {
        vec3_t a = {start[0] - 2, start[1] - 2, start[2] - 2};
        vec3_t b = {start[0] + 2, start[1] + 2, start[2] + 2};
        DebugTrail(start, end);
        DebugBox(a, b);
    }


    //Segment
    VectorSubtract(end, start, s1);
    s1q = s1[0] * s1[0] + s1[1] * s1[1] + s1[2] * s1[2];
    if (s1q < 1) s1q = 1;

    //Check entities in the box area
    count = gi.BoxEdicts(mins, maxs, others, MAX_EDICTS, AREA_SOLID);
    for (oth = 0; oth < count; oth++) {
        edict_t*other = others[oth];
        vec3_t centre, point, size, hitpoint;
        float distance;

        if (!other->takedamage || other->solid == SOLID_NOT || other == ignore) continue;

        //central point of other
        VectorAdd(other->mins, other->maxs, centre);
        VectorMA(other->s.origin, 0.5f, centre, centre);

        //closest point in the source line
        VectorSubtract(centre, start, s0);
        fraction = (s0[0] * s0[0] + s0[1] * s0[1] + s0[2] * s0[2]) / s1q;
        if (fraction < 0) fraction = 0;
        else
            if (fraction > 1) fraction = 1;
        VectorMA(start, fraction, s1, point);

        //Fraction corresponding to the distance to the point
        distance = VectorNormalize(s0);
        for (i = 0; i < 3; i++) {
            size[i] = other->maxs[i] - other->mins[i];
            distance -= fabs(size[i] * s0[i]);
            hitpoint[i] = centre[i]/* - size[i]*s0[i]*/;
        }

        if (distance >= 0.0f) {
            fraction = 1 - distance / radius;
            if (fraction <= 0.00001f) {
                if (Debug(segmentdmg) && attacker->client)
                    gi.cprintf(attacker, PRINT_HIGH, "SegmentDmg(%s): Main fraction discard. d=%.1f/%.1f\n", other->classname, distance, radius);
                continue; //would result in 0 damage, skip it
            }
        } else
            fraction = 1.0f;


        if (dflags & DAMAGE_NO_START) {
            fraction -= SRD_PointFraction(start, radius, centre, size);
            if (fraction <= 0.00001f) {
                if (Debug(segmentdmg) && attacker->client)
                    gi.cprintf(attacker, PRINT_HIGH, "SegmentDmg(%s): Start fraction discard. d=%.1f\n", other->classname, distance);
                continue; //would result in 0 damage, skip it
            }
        }

        if (dflags & DAMAGE_NO_END) {
            fraction -= SRD_PointFraction(end, radius, centre, size);
            if (fraction <= 0.00001f) {
                if (Debug(segmentdmg) && attacker->client)
                    gi.cprintf(attacker, PRINT_HIGH, "SegmentDmg(%s): End fraction discard. d=%.1f\n", other->classname, distance);
                continue; //would result in 0 damage, skip it
            }
        }

        if (fraction >= 1.0f) continue;

        if (Debug(segmentdmg) && attacker->client)
            gi.cprintf(attacker, PRINT_HIGH, "SegmentDmg(%s): dmg=%.1f f=%.3f d=%.1f\n", other->classname, damage, 1 - fraction, distance);

        if (Debug(segmentlines)) DebugTrail(point, hitpoint);

        d.inflictor = inflictor;
        d.attacker = attacker;
        d.scale = 1;
        d.dflags = dflags;
        d.mod = mod;
        d.radius = radius;
        DAMAGE_FIELDS(13); //valid fields set above to make sure if some new is added its not left uninitialized

        d.targ = other;
        VectorCopy(s0, d.dir);
        VectorCopy(hitpoint, d.point);
        VectorScale(s0, -1, d.normal);
        d.damage = damage * (1 - fraction);
        d.distance = radius * (1 - fraction);
        d.knockback = damage * (1 - fraction);

        if (Damage(&d)) //T_Damage(other, inflictor, attacker, s0, hitpoint, normal, fraction*damage, fraction*knockback, dflags, mod)
            result++;


    }

    return result;
}

