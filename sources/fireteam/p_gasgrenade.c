#include "g_local.h"

//======================================================
void Gas_Touch(edict_t *gas, edict_t *target, cplane_t *plane, csurface_t *surf) {
	vec3_t dir={0,0,0};

    gas->enemy=target;

    if (target->takedamage)
	    T_Damage(target, gas, gas->owner, dir, gas->s.origin, plane->normal, 5, 0, 0, MOD_GAS_GRENADE );

    G_FreeEdict(gas); // free up this cloud upon touch..
}

//======================================================
void Generate_Grenade_Gas(edict_t *ent, vec3_t last_angles) {
	edict_t    *gas;
    int x,y;

    gas = G_Spawn();

    gas->classname = "gas";
    gas->owner = ent; // owner is the grenade..
              // player is grenade's owner!

    VectorCopy (ent->s.origin, gas->s.origin);
    x = (random()>0.5?-1:1);
    y = (random()>0.5?-1:1);
    gas->s.origin[0] += (random()*20+1)*x;
    gas->s.origin[1] += (random()*20+1)*y;
    gas->s.origin[2] += 8;
    VectorCopy (ent->s.old_origin, gas->s.old_origin);
    VectorClear(gas->s.angles);
    gas->velocity[2] = (random()*40)+40;
    gas->velocity[1] = ((int)((random()*40)+20+last_angles[1])%60)*y;
    gas->velocity[0] = ((int)((random()*40)+20+last_angles[0])%60)*x;
    VectorCopy(gas->velocity, last_angles);

    gas->movetype = MOVETYPE_FLY; // clouds float gently around..

    gas->solid = SOLID_BBOX; // enable touch detection..

    VectorSet(gas->mins, -10, -10, -10); // size of bbox for touch
    VectorSet(gas->maxs, 10, 10, 10); // size of bbox for touch

    gas->s.modelindex = gi.modelindex("sprites/s_gas.sp2");

    gas->touch=Gas_Touch; // Touch detection function.

    gas->nextthink = level.time+10;
    gas->think=G_FreeEdict; // kill gas cloud in 10 secs if not touched..

    gi.linkentity (gas);
}

//======================================================
void Gas_Grenade(edict_t *ent) {

	Generate_Grenade_Gas(ent, ent->move_angles);

    if (ent->gas_timer > level.time) {
	    ent->nextthink = level.time + 0.1;
        ent->think = Gas_Grenade;
        return;    }

    G_FreeEdict(ent);
}
