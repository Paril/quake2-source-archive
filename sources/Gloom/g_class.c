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
#include "m_hatchling.h"
#include "m_drone.h"
#include "m_mech.h"
#include "m_stalker.h"
#include "m_hsold.h"
#include "m_breeder.h"
#include "m_engineer.h"
#include "m_guardian.h"
#include "m_exterm.h"
#include "m_stinger.h"
#include "m_wraith.h"




//r1: added this - generic code = less mistakes (in theory)
//mainly used for repairing armor, but could be a variety of functions....
int RepairArmor (edict_t *ent, const gitem_t *item, int count, int max)
{
        gitem_armor_t   *armorinfo;
        int                             i = 0;

        if (!item) {
                item = ent->client->armor;
                armorinfo = (gitem_armor_t *)item->info;
                if (!max) {
                        if (classlist[ent->client->resp.class_type].armorcount) {
                                max = classlist[ent->client->resp.class_type].armorcount;
                        } else if (armorinfo) {
                                max = armorinfo->max_count;
                        }
                }
        } else {
                armorinfo = (gitem_armor_t *)item->info;
                if (!max) {
                        if (armorinfo)
                                max = armorinfo->max_count;
                        else
                                max = count;
                }
        }

        if (!item) {
                gi.dprintf ("RepairArmor: couldn't find armor!\n");
                return 0;
        }


        i = ITEM_INDEX(item);
        ent->client->resp.inventory[i] += count;

        if(ent->client->resp.inventory[i] >= max)
        {
                ent->client->resp.inventory[i] = max;
                return -1;
        } else {
                return ent->client->resp.inventory[i];
        }
}

void HealbyBiotech(edict_t *ent, edict_t *bio)
{
        int max_health_from_bio = 0;
        int numpacks = 0;
        int diff = 0;
        int diffpacks = 0;
        int sound = 0;

        if (ent->health <= 0)
                return;

        if (bio->wait > level.time)
                return;

        switch (ent->client->resp.class_type) {
                case CLASS_ENGINEER:
                case CLASS_MECH:
                        numpacks=0;
                        break;
                case CLASS_HEAVY:
                case CLASS_EXTERM:
                case CLASS_COMMANDO:
                        numpacks=2;
                        break;
                default:
                        numpacks=1;
                        break;
        }

        //item = FindItem("Cells");
        //cells = ITEM_INDEX(item);
        
        //i = ITEM_INDEX(item);

        diffpacks = numpacks - ent->client->resp.inventory[ITEM_HEALTH];

        if (bio->client->resp.inventory[ITEM_AMMO_CELLS] - 50 * diffpacks < 0)
                diffpacks--;

        if (bio->client->resp.inventory[ITEM_AMMO_CELLS] - 50 * diffpacks < 0)
                diffpacks--;

        if (diffpacks < 0)
                diffpacks = 0;

        //wait=(float)(numpacks-ent->client->resp.inventory[i])*20.0;
        if (ent->client->resp.inventory[ITEM_HEALTH] < numpacks && diffpacks > 0) {
                ent->client->resp.inventory[ITEM_HEALTH] += diffpacks;
                bio->client->resp.bioScore += 100 * diffpacks;
                sound = SoundIndex(items_s_health);

                // flash the screen
                ent->client->bonus_alpha = 0.25;

                // show icon and name on status bar
                ent->client->ps.stats[STAT_PICKUP_ICON] = imagecache[((const gitem_t *)(itemlist + ITEM_HEALTH))->item_id];
                ent->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(((const gitem_t *)(itemlist + ITEM_HEALTH)));

                ent->client->pickup_msg_time = level.time + 3.0f;
                bio->client->resp.total_score++;
                bio->client->resp.inventory[ITEM_AMMO_CELLS]-= diffpacks * 50;
        } else {
                if (ent->health >= ent->max_health && diffpacks > 0) {
                        gi.cprintf (bio,PRINT_HIGH,"Report: Need 50 cells to give out a health pack!\n");
                        bio->wait = level.time + 1;
                        return;
                }
        }

        if (ent->client->acid_duration)
        {
                if (bio->client->resp.inventory[ITEM_AMMO_CELLS] < 15) {
                        gi.cprintf (bio, PRINT_HIGH, "Report: Need 15 cells for poison cure!\n");
                        bio->wait = level.time + 1;
                        if (sound) gi.sound(ent,CHAN_AUTO,sound,1.0,ATTN_NORM,0);
                        return;
                }

                ent->client->acid_duration = ent->client->acid_damage = 0;
                ent->client->acid_attacker = NULL;

                bio->client->resp.bioScore += 25;
                bio->client->resp.total_score++;
                gi.cprintf (ent, PRINT_HIGH, "%s cured you of poison!\n", bio->client->pers.netname);
                gi.cprintf (bio, PRINT_HIGH, "Report: Cured %s of poison!\n", ent->client->pers.netname);
                bio->client->resp.inventory[ITEM_AMMO_CELLS] -= 15;
        }
        
        if ((int)dmflags->value & DF_SLOW_HEALTH_PACK) {
          int heal = ent->max_health - ent->health - ent->client->resp.health_regen;
          if (heal > 0) {
            if (heal > bio->client->resp.inventory[ITEM_AMMO_CELLS]*2) {
              gi.cprintf (bio,PRINT_HIGH,"Report: Not enough cells!\n");
              bio->wait = level.time + 1;
              heal = bio->client->resp.inventory[ITEM_AMMO_CELLS]*2;
            }
            if (heal > 0) {
              bio->client->resp.inventory[ITEM_AMMO_CELLS] -= heal/2;

              diff += heal;

              ent->client->resp.health_regen += heal;
              ent->client->resp.bio_effect_count += heal;
              if (ent->client->resp.health_regen_time < level.time+0.09f)
                ent->client->resp.health_regen_time = level.time+0.09f;
              else
              if (ent->client->resp.health_regen_time > level.time+0.99f)
                ent->client->resp.health_regen_time = level.time+0.99f;
            }
          }
        } else
        if (ent->health < ent->max_health) {
                diff = ent->max_health - ent->health;
                max_health_from_bio = bio->client->resp.inventory[ITEM_AMMO_CELLS] * 2;
                if (max_health_from_bio == 0) {
                        gi.cprintf (bio,PRINT_HIGH,"Report: Not enough cells!\n");
                        bio->wait = level.time + 1;
                        if (sound) gi.sound(ent,CHAN_AUTO,sound,1.0,ATTN_NORM,0);
                        return;
                }
                bio->client->resp.inventory[ITEM_AMMO_CELLS]-= diff / 2;

                if (diff > max_health_from_bio)
                        diff = max_health_from_bio;

                ent->health += diff;

                if ((ent->waterlevel && ((ent->client->resp.class_type == CLASS_ENGINEER || ent->client->resp.class_type == CLASS_MECH || ent->client->resp.class_type == CLASS_EXTERM) || ent->watertype & (CONTENTS_LAVA|CONTENTS_SLIME))) && ((bio->velocity[0] + bio->velocity[1] < 200 || ent->velocity[0] + ent->velocity[1] < 200)))
                {
                        bio->client->resp.bioScore += diff * 0.1f;

                        if (diff > 10)
                                bio->client->resp.total_score--;
                }
                else
                {

                        bio->client->resp.bioScore += diff;

                        if (diff > 10)
                                bio->client->resp.total_score++;
                }
        }

        if (bio->client->resp.upgrades & UPGRADE_GLOW_GRENADE)
        {
                if (diff == 0 && diffpacks == 0)
                {
                        if (ent->health+ent->client->resp.health_regen+ent->client->resp.health_boost == ent->max_health)
                        {
                                if ((bio->client->resp.inventory[ITEM_AMMO_CELLS]) - (ent->max_health / 2) > 0)
                                {
                                        sound = SoundIndex(items_m_health);
                                        if ((int)dmflags->value & DF_SLOW_HEALTH_PACK)
                                          ent->client->resp.health_boost = ent->max_health / 2;
                                        else
                                          ent->health += ent->max_health / 2;
                                        bio->client->resp.bioScore += ent->max_health / 2;
                                        bio->client->resp.inventory[ITEM_AMMO_CELLS] -= (ent->max_health / 2);
                                        bio->wait = level.time + 1;
                                        // flash the screen
                                        ent->client->bonus_alpha = 0.25;

                                        // show icon and name on status bar
                                        ent->client->ps.stats[STAT_PICKUP_ICON] = imagecache[((const gitem_t *)(itemlist + ITEM_HEALTH))->item_id];
                                        ent->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(((const gitem_t *)(itemlist + ITEM_HEALTH)));

                                        ent->client->pickup_msg_time = level.time + 3.0;
                                        bio->client->resp.total_score++;

                                        gi.cprintf(bio, PRINT_HIGH, "Report: %d health boost given to %s\n", ent->max_health / 2, ent->client->pers.netname);
                                        gi.cprintf(ent, PRINT_HIGH, "%s gave you a %d health boost\n",bio->client->pers.netname,ent->max_health / 2);
                                }
                                else
                                {
                                        gi.cprintf (bio,PRINT_HIGH,"Report: Need %d cells for a health boost!\n",(ent->max_health / 2));
                                        bio->wait = level.time + 1;
                                        if (sound) gi.sound(ent,CHAN_AUTO,sound,1.0,ATTN_NORM,0);
                                        return;
                                }
                        }
                }
        }


        if (diff != 0 || diffpacks != 0) {
                bio->wait = level.time + 0.5f;
                gi.cprintf(bio, PRINT_HIGH, "Report: %d health and %d health packs given to %s\n", diff, diffpacks, ent->client->pers.netname);
                gi.cprintf(ent, PRINT_HIGH, "%s gave you %d health and %d health packs\n",bio->client->pers.netname,diff, diffpacks);
        }

        if (sound) gi.sound(ent,CHAN_AUTO,sound,1.0,ATTN_NORM,0);

        if (bio->client->resp.inventory[ITEM_AMMO_CELLS] < 0)
                bio->client->resp.inventory[ITEM_AMMO_CELLS] = 0;
}

void GenericVoice(edict_t *self, int n)
{
#ifdef CACHE_CLIENTSOUNDS
        int     base;
#else
        char *s=NULL;
#endif

        switch (self->client->resp.class_type) {
        case CLASS_ENGINEER:
#ifdef CACHE_CLIENTSOUNDS
                base = engineer_spawn;
#else
                s = "eng";
#endif
                break;
        case CLASS_SHOCK:
#ifdef CACHE_CLIENTSOUNDS
                base = shock_spawn;
#else
                s = "shock";
#endif
                break;
        case CLASS_BIO:
#ifdef CACHE_CLIENTSOUNDS
                base = biotech_spawn;
#else
                s = "biotech";
#endif
                break;
        case CLASS_HEAVY:
#ifdef CACHE_CLIENTSOUNDS
                base = heavy_spawn;
#else
                s = "heavy";
#endif
                break;
        case CLASS_COMMANDO:
#ifdef CACHE_CLIENTSOUNDS
                base = commando_spawn;
#else
                s = "commando";
#endif
                break;
        case CLASS_EXTERM:
#ifdef CACHE_CLIENTSOUNDS
                base = exterm_spawn;
#else
                s = "exterm";
#endif
                break;
        case CLASS_MECH:
#ifdef CACHE_CLIENTSOUNDS
                base = mech_spawn;
#else
                s = "mech";
#endif
                break;
        }
/*
        if (n == VOICE_NOAMMO) {
          self->client->teamoverlayindex = gi.modelindex("sprites/noammo.sp2");
          self->client->teamoverlaytime = level.time+8;
        }
*/
#ifdef CACHE_CLIENTSOUNDS
        if (n==0)
                teamcastUSound(self, TEAM_HUMAN, SoundIndex(base+n), 0.7);
        else if (n==1)
                teamcastUSound(self, TEAM_HUMAN, SoundIndex(base+n), 1.0);
        else if (n>1 && n<7)
                broadcastUSound(self, SoundIndex(base+n), 1, ATTN_IDLE);
#else
        switch(n)
        {
        case VOICE_SPAWN:
                teamcastUSound(self, TEAM_HUMAN, gi.soundindex(va("voice/%s/spawn1.wav", s)), 0.7);
                break;
        case VOICE_HELP:
                teamcastUSound(self, TEAM_HUMAN, gi.soundindex(va("voice/%s/help1.wav", s)), 1.0);
                break;
        case VOICE_ORDER:
                broadcastUSound(self, gi.soundindex(va("voice/%s/order1.wav", s)), 1, ATTN_IDLE);
                break;
        case VOICE_AFFIRM:
                broadcastUSound(self, gi.soundindex(va("voice/%s/affirm1.wav", s)), 1, ATTN_IDLE);
                break;
        case VOICE_DEFEND:
                broadcastUSound(self, gi.soundindex(va("voice/%s/defend1.wav", s)), 1, ATTN_IDLE);
                break;
        case VOICE_NOAMMO:
                broadcastUSound(self, gi.soundindex(va("voice/%s/noammo1.wav", s)), 1, ATTN_IDLE);
                break;
        case VOICE_TAUNT:
                broadcastUSound(self, gi.soundindex(va("voice/%s/taunt1.wav", s)), 1, ATTN_IDLE);
                break;
        }
#endif
}

void AlienVoice (edict_t *ent, int n)
{
#ifdef CACHE_CLIENTSOUNDS
        int base;

        switch (ent->client->resp.class_type) {
        case CLASS_BREEDER:
                base = breeder_spawn;
                break;
        case CLASS_HATCHLING:
                base = hatch_spawn;
                break;
        case CLASS_DRONE:
                base = drone_spawn;
                break;
        case CLASS_WRAITH:
                base = wraith_spawn;
                break;
        case CLASS_KAMIKAZE:
                base = kamikaze_spawn;
                break;
        case CLASS_STINGER:
                base = stinger_spawn;
                break;
        case CLASS_GUARDIAN:
                base = guardian_spawn;
                break;
        case CLASS_STALKER:
                base = stalker_spawn;
                break;
        }

        if (n >= 0 && n < 7)
                broadcastUSound(ent, SoundIndex (base+n), 1, ATTN_IDLE);
#else

        if (n == 0){
                broadcastUSound(ent, gi.soundindex ("*scream.wav"), 1, ATTN_IDLE);
        } else if (n > 0 && n < 7) {
                broadcastUSound(ent, gi.soundindex (va("*scream%d.wav", n)), 1, ATTN_IDLE);
        }
#endif
}

// max. viewheight 31
// min. viewheight -31


const gclass_t classlist[] =
{
        {
                "Grunt",
                {-16, -16, -24},
                {16, 16, 32},
                22,
                100,
                -60,
                100,
                TEAM_HUMAN,
                90,
                "players/male/tris.md2",
                "male/soldier",
                "models/objects/gibs/sm_meat/tris.md2", // skull
                "models/objects/gibs/sm_meat/tris.md2",
                "models/objects/gibs/sm_meat/tris.md2",
                1.0,
                false,
                true,
                false,
                0,
                GruntStart,
                NULL,
                GruntHeal,
                GruntVoice,
                0,
                0,
                2,
                {
                        FRAME_stand01, FRAME_stand40, FRAME_run1, FRAME_run6,
                        FRAME_attack1, FRAME_attack8, FRAME_crattak1, FRAME_crattak9,
                        FRAME_pain101, FRAME_pain104, FRAME_pain201, FRAME_pain204,
                        FRAME_flip01, FRAME_flip12, FRAME_salute01, FRAME_salute11,
                        FRAME_taunt01, FRAME_taunt17, FRAME_death101, FRAME_death106,
                        FRAME_death201, FRAME_death206, FRAME_death301, FRAME_death308,
                        FRAME_crstnd01, FRAME_crstnd19, FRAME_crwalk1, FRAME_crwalk6,
                        FRAME_crpain1, FRAME_crpain4, FRAME_crdeath1, FRAME_crdeath5,
                        FRAME_pain301, FRAME_pain304, FRAME_jump1, FRAME_jump5
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        grunt_jump,
                        grunt_pain100_1, grunt_pain100_2, grunt_pain75_1, grunt_pain75_2,
                        grunt_pain50_1, grunt_pain50_2, grunt_pain25_1, grunt_pain25_2,
                        grunt_death1, grunt_death2, grunt_death3, grunt_death4,
                        grunt_fall1, grunt_fall2
                },
#endif
                ARMOR_JACKET,
                25,
                /*jumpheight*/ {272, 272, 272},
                /*jumploop*/ 0,
        },

        {
                "Hatchling",
                {-16, -16, -24},
                {16, 16, -8},
                -5,
                20,
                -20,
                100,
                TEAM_ALIEN,
                140,
                //"models/monsters/hatchling/tris.md2",
                "players/hatch/tris.md2",
                "hatch/skin",
                "models/objects/gibs/sm_meat/tris.md2",
                "models/gibs/hatchling/leg/tris.md2",
                "models/gibs/hatchling/leg/tris.md2",
                1.5,
                true,
                false,
                false,
                75,
                NULL,
                HatchlingDie,
                NULL,
                AlienVoice,
                0,
                0,
                2,
                {
                        HATCH_STAND_S, HATCH_STAND_E, HATCH_RUN_S, HATCH_RUN_E,
                        HATCH_ATTACKA_S, HATCH_ATTACKA_E, HATCH_ATTACKB_S, HATCH_ATTACKB_E,
                        HATCH_STAND_S, HATCH_STAND_S+4, HATCH_STAND_S, HATCH_STAND_S+4,
                        HATCH_STAND_S, HATCH_STAND_E, HATCH_STAND_S, HATCH_STAND_E,
                        HATCH_STAND_S, HATCH_STAND_E, HATCH_DEATHA_S, HATCH_DEATHA_E,
                        HATCH_DEATHB_S, HATCH_DEATHB_E, HATCH_DEATHC_S, HATCH_DEATHC_E,
                        HATCH_STAND_S, HATCH_STAND_E, HATCH_STAND_S, HATCH_STAND_E,
                        HATCH_STAND_S, HATCH_STAND_E, HATCH_DEATHA_S, HATCH_DEATHA_E,
                        HATCH_STAND_S, HATCH_STAND_S+2, HATCH_JUMP_S, HATCH_JUMP_E
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        hatch_jump,
                        hatch_pain100_1, hatch_pain100_2, hatch_pain75_1, hatch_pain75_2,
                        hatch_pain50_1, hatch_pain50_2, hatch_pain25_1, hatch_pain25_2,
                        hatch_death1, hatch_death2, hatch_death3, hatch_death4,
                        hatch_fall1, hatch_fall2
                },
#endif
                ARMOR_NONE,
                0,
                /*jumpheight*/ {400, 512, -180},
                /*jumploop*/ 0,
        },
        {
                "Heavy Trooper",
                {-16, -16, -24},
                {16, 16, 32},
                22,
                180,
                -70,
                200,
                TEAM_HUMAN,
                90,
                "players/hsold/tris.md2",
                "hsold/hsold",
                "models/objects/gibs/sm_meat/tris.md2", // skull
                "models/objects/gibs/sm_meat/tris.md2",
                "models/objects/gibs/sm_meat/tris.md2",
                0.8,
                false,
                true,
                false,
                0,
                HeavyStart,
                NULL,
                HeavyHeal,
                GenericVoice,
                2,
                0,
                5,
                {
/*                      FRAME_stand01, FRAME_stand40, FRAME_run1, FRAME_run6,
                        FRAME_attack1, FRAME_attack8, FRAME_crattak1, FRAME_crattak9,
                        FRAME_pain101, FRAME_pain104, FRAME_pain201, FRAME_pain204,
                        FRAME_flip01, FRAME_flip12, FRAME_salute01, FRAME_salute11,
                        FRAME_taunt01, FRAME_taunt17, FRAME_death101, FRAME_death106,
                        FRAME_death201, FRAME_death206, FRAME_death301, FRAME_death308,
                        FRAME_crstnd01, FRAME_crstnd19, FRAME_crwalk1, FRAME_crwalk6,
                        FRAME_crpain1, FRAME_crpain4, FRAME_crdeath1, FRAME_crdeath5,
                        FRAME_pain301, FRAME_pain304, FRAME_jump1, FRAME_jump6
                                                                                                                                                 */
                        HS_STAND_S, HS_STAND_E, HS_RUN_S, HS_RUN_E,
                        HS_ATTACK_S, HS_ATTACK_E, HS_CRATTACK_S, HS_CRATTACK_E,
                        HS_PAIN1_S, HS_PAIN1_E, HS_PAIN2_S, HS_PAIN2_E,
                        HS_TAUNT1_S, HS_TAUNT1_E, HS_TAUNT2_S, HS_TAUNT2_E,
                        HS_TAUNT3_S, HS_TAUNT3_E, HS_DEATH1_S, HS_DEATH1_E,
                        HS_DEATH2_S, HS_DEATH2_E, HS_DEATH3_S, HS_DEATH3_E,
                        HS_CROUCH_S, HS_CROUCH_E, HS_CRWALK_S, HS_CRWALK_E,
                        HS_CRPAIN_S, HS_CRPAIN_E, HS_CRDIE_S, HS_CRDIE_E,
                        HS_PAIN3_S, HS_PAIN3_E, HS_JUMP_S, HS_JUMP_E
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        heavy_jump,
                        heavy_pain100_1, heavy_pain100_2, heavy_pain75_1, heavy_pain75_2,
                        heavy_pain50_1, heavy_pain50_2, heavy_pain25_1, heavy_pain25_2,
                        heavy_death1, heavy_death2, heavy_death3, heavy_death4,
                        heavy_fall1, heavy_fall2
                },
#endif
                ARMOR_BODY,
                100,
                /*jumpheight*/ {208, 208, 208},
                /*jumploop*/ 0,
        },
        {
                "Commando",
                {-16, -16, -24},
                {16, 16, 32},
                22,
                200,
                -70,
                100,
                TEAM_HUMAN,
                90,
                "players/male/tris.md2",
                "male/commando",
                "models/objects/gibs/sm_meat/tris.md2",
                "models/objects/gibs/sm_meat/tris.md2",
                "models/objects/gibs/sm_meat/tris.md2",
                1.2,
                false,
                false,          // no footsteps for commando
                false,
                0,
                CommandoStart,
                NULL,
                CommandoHeal,
                GenericVoice,
                3,
                0,
                6,
                {
                        FRAME_stand01, FRAME_stand40, FRAME_run1, FRAME_run6,
                        FRAME_attack1, FRAME_attack8, FRAME_crattak1, FRAME_crattak9,
                        FRAME_pain101, FRAME_pain104, FRAME_pain201, FRAME_pain204,
                        FRAME_stand01, FRAME_stand02, FRAME_stand01, FRAME_stand02,
                        FRAME_stand01, FRAME_stand02, FRAME_death101, FRAME_death106,
                        FRAME_death201, FRAME_death206, FRAME_death301, FRAME_death308,
                        FRAME_crstnd01, FRAME_crstnd19, FRAME_crwalk1, FRAME_crwalk6,
                        FRAME_crpain1, FRAME_crpain4, FRAME_crdeath1, FRAME_crdeath5,
                        FRAME_pain301, FRAME_pain304, FRAME_jump1, FRAME_jump5
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        commando_jump,
                        commando_pain100_1, commando_pain100_2, commando_pain75_1, commando_pain75_2,
                        commando_pain50_1, commando_pain50_2, commando_pain25_1, commando_pain25_2,
                        commando_death1, commando_death2, commando_death3, commando_death4,
                        commando_fall1, commando_fall2
                },
#endif
                ARMOR_COMBAT,
                50,
                /*jumpheight*/ {320, 380, 400},
                /*jumploop*/ 0,
        },
        {
                "Drone",
                {-16, -16, -24},
                {16, 16, 32},
                16, // old 10
                100,
                -40,
                200,
                TEAM_ALIEN,
                120,
                "players/drone/tris.md2",
                "drone/skin",
                "models/objects/gibs/sm_meat/tris.md2",
                "models/gibs/hatchling/leg/tris.md2",
                "models/objects/gibs/sm_meat/tris.md2",
                1.4,
                true,
                false,
                false,
                60,
                DroneStart,
                NULL,
                DroneHeal,
                AlienVoice,
                1,
                0, // 5 ab
                3,
                {
                        DRONE_STAND_S, DRONE_STAND_E, DRONE_RUN_S, DRONE_RUN_E,
                        DRONE_ATTACK_S, DRONE_ATTACK_E, DRONE_JUMP_E-1, DRONE_JUMP_E,
                        DRONE_PAINA_S, DRONE_PAINA_E, DRONE_PAINB_S, DRONE_PAINB_E,
                        DRONE_STAND_S, DRONE_STAND_E, DRONE_STAND_S, DRONE_STAND_E,
                        DRONE_STAND_S, DRONE_STAND_E, DRONE_DIEA_S, DRONE_DIEA_E,
                        DRONE_DIEA_S, DRONE_DIEA_E, DRONE_DIEB_S, DRONE_DIEB_E,
                        DRONE_STAND_S, DRONE_STAND_E, DRONE_STAND_S, DRONE_STAND_E,
                        DRONE_STAND_S, DRONE_STAND_E, DRONE_DIEB_S, DRONE_DIEB_E,
                        DRONE_PAINA_S, DRONE_PAINA_E, DRONE_JUMP_S, DRONE_JUMP_E
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        drone_jump,
                        drone_pain100_1, drone_pain100_2, drone_pain75_1, drone_pain75_2,
                        drone_pain50_1, drone_pain50_2, drone_pain25_1, drone_pain25_2,
                        drone_death1, drone_death2, drone_death3, drone_death4,
                        drone_fall1, drone_fall2
                },
#endif
                ARMOR_COMBAT,
                25,
                /*jumpheight*/ {360, 540, 0},
                /*jumploop*/ 0,
        },

        {
                "Mech",
                {-24, -24, -24},
                {24, 24, 56},
                30, // old 32 out of bounds
                120,
                -400,
                600,
                TEAM_HUMAN,
                90,
                "players/mech/tris.md2",
                "mech/skin",
                "models/objects/debris1/tris.md2",
                "models/objects/debris1/tris.md2",
                "models/objects/debris1/tris.md2",
                0.4,
                true,
                false,
                true,
                25,
                MechStart,
                MechDie,
                MechHeal,
                GenericVoice,
                5,
                15,
                8,
                {
                        MECH_STAND_S, MECH_STAND_E, MECH_WALK_S,        MECH_WALK_E,
                        MECH_SHOOT_S, MECH_SHOOT_E, MECH_SHOOT_S, MECH_SHOOT_E,
                        MECH_STAND_S, MECH_STAND_E, MECH_STAND_S, MECH_STAND_E,
                        MECH_STAND_S, MECH_STAND_E, MECH_STAND_S, MECH_STAND_E,
                        MECH_STAND_S, MECH_STAND_E, MECH_FALL_S,        MECH_FALL_E,
                        MECH_FALL_S,  MECH_FALL_E,      MECH_FALL_S,    MECH_FALL_E,
                        MECH_STAND_S, MECH_STAND_E, MECH_STAND_S, MECH_STAND_E,
                        MECH_STAND_S, MECH_STAND_E, MECH_FALL_S,        MECH_FALL_E,
                        MECH_STAND_S, MECH_STAND_E, MECH_WALK_S,        MECH_WALK_E
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        mech_jump,
                        mech_pain100_1, mech_pain100_2, mech_pain75_1, mech_pain75_2,
                        mech_pain50_1, mech_pain50_2, mech_pain25_1, mech_pain25_2,
                        mech_death1, mech_death2, mech_death3, mech_death4,
                        mech_fall1, mech_fall2
                },
#endif
                ARMOR_FULL,
                500,
                /*jumpheight*/ {72, 68, 0},
                /*jumploop*/ 1,
        },
        {
                "Shock Trooper",
                {-16, -16, -24},
                {16, 16, 32},
                22,
                100, // 1.1e 150h
                -70,
                100,
                TEAM_HUMAN,
                90,
                "players/male/tris.md2",
                "male/shotgun",
                "models/objects/gibs/sm_meat/tris.md2", // skull
                "models/objects/gibs/sm_meat/tris.md2",
                "models/objects/gibs/sm_meat/tris.md2",
                1.0,
                false,
                true,
                false,
                0,
                ShotgunStart,
                NULL,
                ShotgunHeal,
                GenericVoice,
                1,
                0, // 1 ab
                3,
                {
                        FRAME_stand01, FRAME_stand40, FRAME_run1, FRAME_run6,
                        FRAME_attack1, FRAME_attack8, FRAME_crattak1, FRAME_crattak8,
                        FRAME_pain101, FRAME_pain104, FRAME_pain201, FRAME_pain204,
                        FRAME_stand01, FRAME_stand02, FRAME_stand01, FRAME_stand02,
                        FRAME_stand01, FRAME_stand02, FRAME_death101, FRAME_death106,
                        FRAME_death201, FRAME_death206, FRAME_death301, FRAME_death308,
                        FRAME_crstnd01, FRAME_crstnd19, FRAME_crwalk1, FRAME_crwalk6,
                        FRAME_crpain1, FRAME_crpain4, FRAME_crdeath1, FRAME_crdeath5,
                        FRAME_pain301, FRAME_pain304, FRAME_jump1, FRAME_jump5
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        shock_jump,
                        shock_pain100_1, shock_pain100_2, shock_pain75_1, shock_pain75_2,
                        shock_pain50_1, shock_pain50_2, shock_pain25_1, shock_pain25_2,
                        shock_death1, shock_death2, shock_death3, shock_death4,
                        shock_fall1, shock_fall2
                },
#endif
                ARMOR_JACKET,
                25,
                /*jumpheight*/ {272, 272, 272},
                /*jumploop*/ 0,
        },
        {
                "Stalker",
                {-32, -32, -24},
                {32, 32, 40},
                30, // old 32 out of bounds
                200, // 250h 1.1e ; was 300
                -175,
                400,
                TEAM_ALIEN,
                120,
                "players/stalker/tris.md2",
                "stalker/skin",
                "models/gibs/stalker/gib1.md2",
                "models/gibs/stalker/gib2.md2",
                "models/gibs/stalker/gib3.md2",
                0.5,
                true,
                false,
                true,
                100,
                StalkStart,
                NULL,
                StalkHeal,
                AlienVoice,
                5,
                0, // 15ab
                8,
                {
                        STALK_STAND_S, STALK_STAND_E, STALK_WALK_S, STALK_WALK_E,
                        STALK_ATTACK_S, STALK_ATTACK_E, STALK_JUMP_E-1, STALK_JUMP_E,
                        STALK_PAINA_S, STALK_PAINA_E, STALK_PAINB_S, STALK_PAINB_E,
                        STALK_SCREAM_S, STALK_SCREAM_E, 1, 1,
                        STALK_SCREAM_S, STALK_SCREAM_E, STALK_DEATHA_S, STALK_DEATHA_E,
                        STALK_DEATHA_S, STALK_DEATHA_E, STALK_DEATHB_S, STALK_DEATHB_E,
                        STALK_STAND_S, STALK_STAND_E, STALK_STAND_S, STALK_STAND_E,
                        STALK_STAND_S, STALK_STAND_E, STALK_DEATHB_S, STALK_DEATHB_E,
                        STALK_PAINA_S, STALK_PAINA_E, STALK_JUMP_S, STALK_JUMP_E
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        stalker_jump,
                        stalker_pain100_1, stalker_pain100_2, stalker_pain75_1, stalker_pain75_2,
                        stalker_pain50_1, stalker_pain50_2, stalker_pain25_1, stalker_pain25_2,
                        stalker_death1, stalker_death2, stalker_death3, stalker_death4,
                        stalker_fall1, stalker_fall2
                },
#endif
                ARMOR_JACKET,
                250,
                /*jumpheight*/ {152, 144, 0},
                /*jumploop*/ 1,
        },
        {
                "Breeder",
                {-16, -16, -24},
                {16, 16, 32},
/*
                {-42, -32, -24},
                {34, 32, 40},
*/
                2,
                150,
                -80,
                400,
                TEAM_ALIEN,
                80,
                "players/breeder/tris.md2",
                "breeder/skin",
                "models/objects/gibs/sm_meat/tris.md2",
                "models/objects/gibs/sm_meat/tris.md2",
                "models/objects/gibs/sm_meat/tris.md2",
                0.7,
                true,
                false,
                true,
                0,
                BreedStart,
                NULL,
                NULL,
                AlienVoice,
                0,
                0,
                1,
                {
                        BREED_STAND_S, BREED_STAND_E, BREED_WALK_S, BREED_WALK_E,
                        BREED_COVER_S, BREED_COVER_E, BREED_STAND_S, BREED_STAND_E,
                        BREED_PAINA_S, BREED_PAINA_E, BREED_PAINB_S, BREED_PAINB_E,
                        BREED_STAND_S, BREED_STAND_E, BREED_STAND_S, BREED_STAND_E,
                        BREED_STAND_S, BREED_STAND_E, BREED_EXPLODE_S, BREED_EXPLODE_E,
                        BREED_EXPLODE_S, BREED_EXPLODE_E, BREED_EXPLODE_S, BREED_EXPLODE_E,
                        BREED_STAND_S, BREED_STAND_E, BREED_STAND_S, BREED_STAND_E,
                        BREED_STAND_S, BREED_STAND_E, BREED_EXPLODE_S, BREED_EXPLODE_E,
                        BREED_PAINA_S, BREED_PAINA_E, BREED_JUMP_S, BREED_JUMP_E
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        breeder_jump,
                        breeder_pain100_1, breeder_pain100_2, breeder_pain75_1, breeder_pain75_2,
                        breeder_pain50_1, breeder_pain50_2, breeder_pain25_1, breeder_pain25_2,
                        breeder_death1, breeder_death2, breeder_death3, breeder_death4,
                        breeder_fall1, breeder_fall2
                },
#endif
                ARMOR_NONE,
                0,
                /*jumpheight*/ {200, 200, 200},
                /*jumploop*/ 0,
        },
        {
                "Engineer",
                {-16, -16, -24},
                {16, 16, 40},
                22,
                75,
                -40,
                100,
                TEAM_HUMAN,
                90,
                "players/engineer/tris.md2",
                "engineer/skin",
                "models/objects/debris1/tris.md2",
                "models/objects/debris2/tris.md2",
                "models/objects/debris1/tris.md2",
                1.0,
                true,
                false,
                false,
                0,
                EngineerStart,
                NULL,           // Need a special explode death.
                EngineerHeal,
                GenericVoice,
                0,
                0,
                1,
                {
                        /*ENG_STAND_S, ENG_STAND_E, ENG_MOVE_S, ENG_MOVE_E,
                        ENG_STAND_S, ENG_STAND_E, ENG_STAND_S, ENG_STAND_E,
                        ENG_PAINA_S, ENG_PAINA_E, ENG_PAINB_S, ENG_PAINB_E,
                        ENG_SRIGHT_S, ENG_SRIGHT_E, ENG_SLEFT_S, ENG_SLEFT_E,
                        ENG_WORKA_S, ENG_WORKB_E, ENG_EXPLODE_S, ENG_EXPLODE_E,
                        ENG_EXPLODE_S, ENG_EXPLODE_E, ENG_EXPLODE_S, ENG_EXPLODE_E,
                        ENG_STAND_S, ENG_STAND_E, ENG_STAND_S, ENG_STAND_E,
                        ENG_PAINA_S, ENG_PAINA_E, 
                        ENG_EXPLODE_S, ENG_EXPLODE_E,
                        ENG_TELEPORT_S, ENG_TELEPORT_E, 
                        ENG_MOVE_S, ENG_MOVE_E*/
                        ENG_STAND_S, ENG_STAND_E, ENG_MOVE_S, ENG_MOVE_E,
                        ENG_STAND_S, ENG_STAND_E, ENG_STAND_S, ENG_STAND_E,
                        ENG_PAINA_S, ENG_PAINA_E, ENG_PAINB_S, ENG_PAINB_E,
                        ENG_SRIGHT_S, ENG_SRIGHT_E, ENG_SLEFT_S, ENG_SLEFT_E,
                        ENG_WORKA_S, ENG_WORKB_E, ENG_EXPLODE_S, ENG_EXPLODE_E,
                        ENG_EXPLODE_S, ENG_EXPLODE_E, ENG_EXPLODE_S, ENG_EXPLODE_E,
                        ENG_STAND_S, ENG_STAND_E, ENG_STAND_S, ENG_STAND_E,
                        ENG_PAINA_S, ENG_PAINA_E, ENG_EXPLODE_S, ENG_EXPLODE_E,
                        ENG_PAINA_S, ENG_PAINA_E, ENG_MOVE_S, ENG_MOVE_E
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        engineer_jump,
                        engineer_pain100_1, engineer_pain100_2, engineer_pain75_1, engineer_pain75_2,
                        engineer_pain50_1, engineer_pain50_2, engineer_pain25_1, engineer_pain25_2,
                        engineer_death1, engineer_death2, engineer_death3, engineer_death4,
                        engineer_fall1, engineer_fall2
                },
#endif
                ARMOR_NONE,
                0,
                /*jumpheight*/ {256, 256, 256},
                /*jumploop*/ 0,
        },
        {
                "Guardian",
                {-32, -32, -24},
                {32, 32, 24},
                11,
                200,
                -90,
                400,
                TEAM_ALIEN,
                80,
                "players/guardian/tris.md2",
                "guardian/skin",
                "models/objects/gibs/sm_meat/tris.md2",
                "models/gibs/guardian/gib2.md2",
                "models/gibs/guardian/gib1.md2",
                1.2,
                true,
                false,
                false,
                40,
                GuardianStart,
                NULL,
                GuardianHeal,
                AlienVoice,
                4,
                0, //15 ab
                7,
                {
                        GUARD_STAND_S, GUARD_STAND_E, GUARD_WALK_S, GUARD_WALK_E,
                        GUARD_ATTACK_S, GUARD_ATTACK_E, GUARD_STAND_S, GUARD_STAND_E,
                        GUARD_PAIN1_S, GUARD_PAIN1_E, GUARD_PAIN2_S, GUARD_PAIN2_E,
                        GUARD_SCREAM_S, GUARD_SCREAM_E, GUARD_SCREAM_S, GUARD_SCREAM_E,
                        GUARD_STAND_S, GUARD_STAND_E, GUARD_DEATH1_S, GUARD_DEATH1_E,
                        GUARD_DEATH2_S, GUARD_DEATH2_E, GUARD_EXPLODE_S, GUARD_EXPLODE_E,
                        GUARD_STAND_S, GUARD_STAND_E, GUARD_STAND_S, GUARD_STAND_E,
                        GUARD_STAND_S, GUARD_STAND_E, GUARD_EXPLODE_S, GUARD_EXPLODE_E,
                        GUARD_PAIN1_S, GUARD_PAIN1_E, GUARD_JUMP_S, GUARD_JUMP_E
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        guardian_jump,
                        guardian_pain100_1, guardian_pain100_2, guardian_pain75_1, guardian_pain75_2,
                        guardian_pain50_1, guardian_pain50_2, guardian_pain25_1, guardian_pain25_2,
                        guardian_death1, guardian_death2, guardian_death3, guardian_death4,
                        guardian_fall1, guardian_fall2,
                },
#endif
#ifndef GUARDIAN_REGEN_AT_HEALER
                ARMOR_COMBAT,
                30,
#else
                ARMOR_COMBAT,
                70,
#endif
                /*jumpheight*/ {320, 400, 0},
                /*jumploop*/ 1
        },
        {
                "Kamikaze",
                {-16, -16, -24},
                {16, 16, -8},
                -5,
                20,
                -20,
                100,
                TEAM_ALIEN,
                140,
                //"models/monsters/hatchling/tris.md2",
                "players/hatch/tris.md2",
                "hatch/kam",
                "models/objects/gibs/sm_meat/tris.md2",
                "models/gibs/hatchling/leg/tris.md2",
                "models/gibs/hatchling/leg/tris.md2",
                1.4,
                true,
                false,
                false,
                50,
                NULL,
                KamikazeDie,
                NULL,
                AlienVoice,
                2,
                0,
                5,
                {
                        HATCH_STAND_S, HATCH_STAND_E, HATCH_RUN_S, HATCH_RUN_E,
                        HATCH_ATTACKA_S, HATCH_ATTACKA_E, HATCH_ATTACKB_S, HATCH_ATTACKB_E,
                        HATCH_STAND_S, HATCH_STAND_S+4, HATCH_STAND_S, HATCH_STAND_S+4,
                        HATCH_STAND_S, HATCH_STAND_E, HATCH_STAND_S, HATCH_STAND_E,
                        HATCH_STAND_S, HATCH_STAND_E, HATCH_DEATHA_S, HATCH_DEATHA_E,
                        HATCH_DEATHB_S, HATCH_DEATHB_E, HATCH_DEATHC_S, HATCH_DEATHC_E,
                        HATCH_STAND_S, HATCH_STAND_E, HATCH_STAND_S, HATCH_STAND_E,
                        HATCH_STAND_S, HATCH_STAND_E, HATCH_DEATHA_S, HATCH_DEATHA_E,
                        HATCH_STAND_S, HATCH_STAND_S+2, HATCH_JUMP_S, HATCH_JUMP_E
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        kamikaze_jump,
                        kamikaze_pain100_1, kamikaze_pain100_2, kamikaze_pain75_1, kamikaze_pain75_2,
                        kamikaze_pain50_1, kamikaze_pain50_2, kamikaze_pain25_1, kamikaze_pain25_2,
                        kamikaze_death1, kamikaze_death2, kamikaze_death3, kamikaze_death4,
                        kamikaze_fall1, kamikaze_fall2
                },
#endif
                ARMOR_JACKET,
                5,
                /*jumpheight*/ {400, 512, -180},
                /*jumploop*/ 0,
        },
        {
                "Exterminator",
                {-16, -16, -24},
                {16, 16, 32},
                26,
                200,
                -120,
                200,
                TEAM_HUMAN,
                90,
                "players/exterm/tris.md2",
                "exterm/skin",
                "models/objects/debris1/tris.md2",
                "models/objects/debris1/tris.md2",
                "models/objects/debris2/tris.md2",
                0.9,
                false,
                true,
                false,
                0,
                ExterStart,
                NULL,
                ExterHeal,
                GenericVoice,
                4,
                0,
                7,
                {
                        EXTER_STAND_S, EXTER_STAND_E, EXTER_RUN_S, EXTER_RUN_E,
                        EXTER_SHOOT_S, EXTER_SHOOT_E, EXTER_CRFIRE_S, EXTER_CRFIRE_E,
                        EXTER_PAIN_S, EXTER_PAIN_E, EXTER_PAIN_S, EXTER_PAIN_E,
                        EXTER_STAND_S, EXTER_STAND_E, EXTER_STAND_S, EXTER_STAND_E,
                        EXTER_STAND_S, EXTER_STAND_E, EXTER_DIE1_S, EXTER_DIE1_E,
                        EXTER_DIE2_S, EXTER_DIE2_E, EXTER_DIE3_S, EXTER_DIE3_E,
                        EXTER_CRSTAND_S, EXTER_CRSTAND_E, EXTER_CRWALK_S, EXTER_CRWALK_E,
                        EXTER_CRPAIN_S, EXTER_CRPAIN_E, EXTER_CRDIE_S, EXTER_CRDIE_E,
                        EXTER_PAIN_S+3, EXTER_PAIN_E, EXTER_JUMP_S, EXTER_JUMP_E
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        exterm_jump,
                        exterm_pain100_1, exterm_pain100_2, exterm_pain75_1, exterm_pain75_2,
                        exterm_pain50_1, exterm_pain50_2, exterm_pain25_1, exterm_pain25_2,
                        exterm_death1, exterm_death2, exterm_death3, exterm_death4,
                        exterm_fall1, exterm_fall2
                },
#endif
                ARMOR_COMBAT,
                100,
                /*jumpheight*/ {230, 230, 230},
                /*jumploop*/ 0,
        },
        {
                "Stinger",
                {-24, -24, -24},
                {24, 24, 24},
                8,
                200,
                -100,
                200,
                TEAM_ALIEN,
                90,
                "players/stinger/tris.md2",
                "stinger/skin",
                "models/objects/gibs/sm_meat/tris.md2",
                "models/gibs/guardian/gib2.md2",
                "models/gibs/guardian/gib1.md2",
                1.35,
                true,
                false,
                false,
                25,
                StingStart,
                NULL,
                StingHeal,
                AlienVoice,
                3,
                0, // 10ab
                6,
                {
                        STING_STAND_S, STING_STAND_E, STING_WALK_S, STING_WALK_E,
                        STING_STING_S, STING_STING_E, STING_SHOT_S, STING_SHOT_E,
                        STING_PAIN1_S, STING_PAIN1_E, STING_PAIN2_S, STING_PAIN2_E,
                        STING_STAND_S, STING_STAND_E, STING_STAND_S, STING_STAND_E,
                        STING_STAND_S, STING_STAND_E, STING_DEATH1_S, STING_DEATH1_E,
                        STING_DEATH2_S,STING_DEATH2_E, STING_DEATH1_S, STING_DEATH1_E,
                        STING_STAND_S, STING_STAND_E, STING_STAND_S, STING_STAND_E,
                        STING_STAND_S, STING_STAND_E, STING_DEATH2_S, STING_DEATH2_E,
                        STING_PAIN1_S, STING_PAIN1_E, STING_JUMP_S, STING_JUMP_E
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        stinger_jump,
                        stinger_pain100_1, stinger_pain100_2, stinger_pain75_1, stinger_pain75_2,
                        stinger_pain50_1, stinger_pain50_2, stinger_pain25_1, stinger_pain25_2,
                        stinger_death1, stinger_death2, stinger_death3, stinger_death4,
                        stinger_fall1, stinger_fall2
                },
#endif
                ARMOR_NONE,
                0,
                /*jumpheight*/ {386, 464, 496},
                /*jumploop*/ 0,
        },
        /*       Brian Added this       */
        {
                "Wraith",
                {-16, -16, -24},
                {16, 16, 16},
                8,
                75,
                -60,
                200,
                TEAM_ALIEN,
                120,
                "players/wraith/tris.md2",
                "wraith/skin",
                "models/objects/gibs/sm_meat/tris.md2",
                "models/gibs/hatchling/leg/tris.md2",
                "models/objects/gibs/sm_meat/tris.md2",
                1.4,
                true,
                false,
                false,
                50,
                WraithStart,
                NULL,
                WraithHeal,
                AlienVoice,
                1,
                0,
                WRAITH_MENU_NUM,
                {
                        WRAITH_STAND_S, WRAITH_STAND_E, WRAITH_RUN_S,   WRAITH_RUN_E,
                        WRAITH_SLASH_S, WRAITH_SLASH_E, WRAITH_FLYATT_S,WRAITH_FLYATT_E,
                        WRAITH_PAIN_S,  WRAITH_PAIN_E,  WRAITH_PAIN_S,  WRAITH_PAIN_E,
                        WRAITH_STAND_S, WRAITH_STAND_E, WRAITH_STAND_S, WRAITH_STAND_E,
                        WRAITH_STAND_S, WRAITH_STAND_E, WRAITH_DIE_S,   WRAITH_DIE_E,
                        WRAITH_DIE_S,   WRAITH_DIE_E,   WRAITH_FDIE_S,  WRAITH_FDIE_E,
                        WRAITH_STAND_S, WRAITH_STAND_E, WRAITH_FLY_S,   WRAITH_FLY_E,
                        WRAITH_FALL_S,  WRAITH_FALL_E,  WRAITH_DIE_S, WRAITH_DIE_E,
                        WRAITH_FPAIN_S, WRAITH_FPAIN_E, WRAITH_JUMP_S,  WRAITH_JUMP_E
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        wraith_jump,
                        wraith_pain100_1, wraith_pain100_2, wraith_pain75_1, wraith_pain75_2,
                        wraith_pain50_1, wraith_pain50_2, wraith_pain25_1, wraith_pain25_2,
                        wraith_death1, wraith_death2, wraith_death3, wraith_death4,
                        wraith_fall1, wraith_fall2
                },
#endif
                ARMOR_NONE,
                0,
                /*jumpheight*/ {380, 512, -204},
                /*jumploop*/ 0,
        },
        {
                "Biotech",
                {-16, -16, -24},
                {16, 16, 32},
                22,
                100,
                -60,
                100,
                TEAM_HUMAN,
                90,
                "players/female/tris.md2",
                "female/bio",
                "models/objects/gibs/sm_meat/tris.md2",
                "models/objects/gibs/sm_meat/tris.md2",
                "models/objects/gibs/sm_meat/tris.md2",
                1.0,
                false,
                true,
                false,
                0,
                BiotechStart,
                NULL,
                BiotechHeal,
                BiotechVoice,
                1,
                0,      // absorb
                4,
                {
                        FRAME_stand01, FRAME_stand40, FRAME_run1, FRAME_run6,
                        FRAME_attack1, FRAME_attack8, FRAME_crattak1, FRAME_crattak8,
                        FRAME_pain101, FRAME_pain104, FRAME_pain201, FRAME_pain204,
                        FRAME_stand01, FRAME_stand02, FRAME_stand01, FRAME_stand02,
                        FRAME_stand01, FRAME_stand02, FRAME_death101, FRAME_death106,
                        FRAME_death201, FRAME_death206, FRAME_death301, FRAME_death308,
                        FRAME_crstnd01, FRAME_crstnd19, FRAME_crwalk1, FRAME_crwalk6,
                        FRAME_crpain1, FRAME_crpain4, FRAME_crdeath1, FRAME_crdeath5,
                        FRAME_pain301, FRAME_pain304, FRAME_jump1, FRAME_jump5
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        biotech_jump,
                        biotech_pain100_1, biotech_pain100_2, biotech_pain75_1, biotech_pain75_2,
                        biotech_pain50_1, biotech_pain50_2, biotech_pain25_1, biotech_pain25_2,
                        biotech_death1, biotech_death2, biotech_death3, biotech_death4,
                        biotech_fall1, biotech_fall2
                },
#endif
                ARMOR_JACKET,
                30,
                /*jumpheight*/ {272, 272, 272},
                /*jumploop*/ 0,
        },

        {
                "Observer",
                {-8, -8, -8},
                {8, 8, 8},
                0,
                0,
                0,
                100, //Need to give mass to avoid division by zero when obs isn't MOVETYPE_NOCLIP
                TEAM_NONE,
                90,
                NULL,
                "male/obs",
                NULL,
                NULL,
                NULL,
                1.0,
                false,
                false,
                false,
                0,
                NULL,
                NULL,
                NULL,
                NULL,
                0,
                0,
                0,
                {
                        0 // we don't reference these anyway
                },
#ifdef CACHE_CLIENTSOUNDS
                {
                        0 // we don't reference these anyway
                },
#endif
                ARMOR_NONE,
                0,
                /*jumpheight*/ {1, 1, 1},
                /*jumploop*/ 0,
        },

        {NULL}
};

