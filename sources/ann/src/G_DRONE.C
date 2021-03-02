#include "g_local.h"
#include "g_brrr.h"
#include "g_helper.h"
                                // ======================================================
                                // =========== AIRIAL LASERDRONE WEAPON =================
                                // ======================================================
                                // ======================================================
                                // ============ LASERDRONE WEAPON ROUTINES ==============
                                // ======================================================
#define DEBRIS1_MODEL "models/objects/debris1/tris.md2"
#define DEBRIS2_MODEL "models/objects/debris2/tris.md2"
#define DEBRIS3_MODEL "models/objects/debris3/tris.md2"
#define ITEM_IN_ENTS_INVENTORY ent->client->pers.inventory[index]
void            ThrowDebris(edict_t * self, char *modelname, float speed, vec3_t origin);
qboolean        visible(edict_t * self, edict_t * other);
#define LAUNCH_SOUND gi.soundindex("world/airhiss1.wav")
#define ACTIVATION_SOUND gi.soundindex("boss3/w_loop.wav")
#define LASER_FIRE_SOUND gi.soundindex("gladiator/railgun.wav")
                                // ======================================================
void            LaserDrone_Fire(edict_t * LaserDrone, edict_t * target)
{
    trace_t         tr;
    vec3_t          dummy = {0, 0, 0};
    vec3_t          start, end;
    VectorCopy(LaserDrone->s.origin, start);
    VectorCopy(target->s.origin, end);
    // some minor adjustments to positioning
    start[2] += 70;
    end[2] += target->viewheight;   // *** TOM: Added +viewheight
    // Find out what we hit at trace end point, if anything..
    // *** TOM: Added ignore self
    tr = gi.trace(start, target->mins, target->maxs, end, LaserDrone, MASK_SHOT);
    // tr = gi.trace(start, target->mins, target->maxs, end, NULL, MASK_SHOT);
    // Fire the laser beam with bubbletrail from start to end..
    G_Spawn_Trails(TE_BFG_LASER, start, tr.endpos, tr.endpos);
    G_Spawn_Trails(TE_BUBBLETRAIL, start, tr.endpos, tr.endpos);
    // If end point entity (tr.ent) exists and can takedamage, then...
    // if (G_EntExists(tr.ent) && (tr.ent->takedamage) && tr.ent!=LaserDrone) {
    if (G_EntExists(target) && (target->takedamage) && target != LaserDrone)
    {
        // Display a splash of blood from victim at impact point.
        G_Spawn_Sparks(TE_BLOOD, tr.endpos, tr.plane.normal, tr.endpos);
        // Apply damage to this victim and return..
        T_Damage(target, LaserDrone->owner, LaserDrone->owner, dummy, start, dummy, LaserDrone->dmg, 1, 0, MOD_LASERDRONE);
    }                           // endif
    else
        // Else, if the trace end point was NOT the sky, then ...
    if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
    {
        // Show sparks at point of impact..
        G_Spawn_Sparks(TE_SCREEN_SPARKS, tr.endpos, tr.plane.normal, tr.endpos);
        // Do a radius damage of 100 units from sparks impact point..
        T_RadiusDamage(tr.ent, LaserDrone->owner, 100, NULL, LaserDrone->dmg, MOD_SPLASH);
    }                           // endif
    // Must play sound AFTER firing else targeting accuracy affected.. (??)
    gi.sound(LaserDrone, CHAN_VOICE, LASER_FIRE_SOUND, 1, ATTN_NORM, 0);
    LaserDrone->nextthink = level.time + 1.0;   // Re-target in 1 second...
}
                                // ======================================================
void            LaserDrone_Die(edict_t * LaserDrone)
{
    /* *** TOM *** */
    // if (LaserDrone->takedamage==DAMAGE_NO)
    // return;
    /* *** */
    LaserDrone->owner->client->laserdrone = 0;  // Immediately Flag as OFF
    /* *** TOM *** */
    LaserDrone->owner->client->droneent = 0;
    /* *** */
    /* *** TOM *** */
    LaserDrone->takedamage = DAMAGE_NO;
    /* *** */
    // Blow up the LaserDrone Unit and let debris rain down...
    G_Spawn_Explosion(TE_EXPLOSION2, LaserDrone->s.origin, LaserDrone->s.origin);
    // Let it rain, rain, rain...
    ThrowDebris(LaserDrone, DEBRIS3_MODEL, 3.75, LaserDrone->s.origin);
    ThrowDebris(LaserDrone, DEBRIS3_MODEL, 2.50, LaserDrone->s.origin);
    ThrowDebris(LaserDrone, DEBRIS2_MODEL, 4.60, LaserDrone->s.origin);
    ThrowDebris(LaserDrone, DEBRIS2_MODEL, 1.50, LaserDrone->s.origin);
    ThrowDebris(LaserDrone, DEBRIS3_MODEL, 2.30, LaserDrone->s.origin);
    ThrowDebris(LaserDrone, DEBRIS3_MODEL, 4.50, LaserDrone->s.origin);
    // Do damage to all within ents within blast radius.
    if (!G_ClientInGame(LaserDrone->owner))
        // Owner has since died so don't assign damage frags to LaserDrone's owner.
        T_RadiusDamage(LaserDrone, LaserDrone, LaserDrone->dmg, NULL, LaserDrone->dmg_radius, MOD_LASERDRONE);
    else
        // Owner alive, so assign all frags to the LaserDrone's owner..
        T_RadiusDamage(LaserDrone, LaserDrone->owner, LaserDrone->dmg, NULL, LaserDrone->dmg_radius, MOD_LASERDRONE);
    G_FreeEdict(LaserDrone);    // Free the LaserDrone entity.
}
                                // ======================================================
                                // True if start and end are within radius distance..
                                // ======================================================
qboolean        Within_Radius(vec3_t start, vec3_t end, float rad)
{
    vec3_t          eorg = {0, 0, 0};
    int             j;
    for (j = 0; j < 3; j++)
        eorg[j] = abs(start[j] - end[j]);
    return (VectorLength(eorg) < rad);
}
                                // ======================================================
                                // True if ent meets Drone's targeting criteria..
                                // ======================================================
qboolean        Good_Target(edict_t * LaserDrone, edict_t * ent)
{
    // float rad;
    edict_t        *tom = NULL;
    if ((G_ClientInGame(ent)) && (ent != LaserDrone)
            && (ent->takedamage) && (visible(LaserDrone, ent)))
    {
        // Targeting radius decreases with health of Drone.
        // rad=LaserDrone->wait*(LaserDrone->health/LaserDrone->max_health);
        // Is this good ent within the targeting radius??
        // return Within_Radius(LaserDrone->s.origin, ent->s.origin, rad);
        /* *** TOM: Within_Radius() doesn't work, so using own code *** */
        while ((tom = findradius(tom, LaserDrone->s.origin, 512)) != NULL)
        {
            if (tom == ent)
                return true;
        }
    }                           // endif
    return false;
}
                                // ======================================================
                                // True if LaserDrone has been deactivated (exploded!).
                                // ======================================================
qboolean        DeActivated(edict_t * LaserDrone)
{
    // Has time expired? Or, Owner since died?
    if ((LaserDrone->delay < level.time)
            || (LaserDrone->owner->client->laserdrone == 0))
    {
        LaserDrone_Die(LaserDrone);
        return true;
    }
    else
        return false;
}
                                // ======================================================
                                // Need dummy touch because BBOX hits world surfaces..
                                // ======================================================
void            LaserDrone_Touch(edict_t * LaserDrone, edict_t * other, cplane_t * plane, csurface_t * surf)
{
}
                                // ======================================================
                                // UNUSED - Search ALL Ents, then fire at the closest one!
                                // ======================================================
#if 0
void            LaserDrone_ThinkSlow(edict_t * LaserDrone)
{
    edict_t        *ent = NULL;
    edict_t        *pBest = NULL;
    vec3_t          vDist;
    long int        i, tDist, closest = 99999;
    for (i = 0; i < game.maxclients; i++)
    {
        ent = g_edicts + i + 1;
        if (DeActivated(LaserDrone))
            return;
        if (!Good_Target(LaserDrone, ent))
            continue;
        VectorSubtract(ent->s.origin, LaserDrone->s.origin, vDist);
        tDist = VectorLength(vDist);
        if (tDist < closest)
        {
            pBest = ent;        // closest ent so far..
            closest = tDist;
        }                       // shortest distance so far.
    }                           // end for
    // Re-test because Owner may have since moved or died.
    if (Good_Target(LaserDrone, pBest))
    {
        LaserDrone_Fire(LaserDrone, pBest);
        return;
    }
    // Nobody around? so, re-target on next frame.
    LaserDrone->nextthink = level.time + 0.1;
}
#endif
                                // ==========================================================
                                // Search and Fire at the first visible ent.. (VERY FAST!)
                                // ==========================================================
qboolean        Clear_Path(vec3_t spot1, vec3_t spot2)
{
    trace_t         tr;
    tr = gi.trace(spot1, NULL, NULL, spot2, NULL, MASK_OPAQUE);
    return (tr.fraction == 1.0);
}
void            LaserDrone_ThinkFast(edict_t * LaserDrone)
{
    edict_t        *ent = NULL;
    int             i;
    // Shoot down vulnerable rockets
    while ((ent = findradius(ent, LaserDrone->s.origin, 512)) != NULL)
    {
        if ((Q_stricmp(ent->classname, "guided") != 0)
                && (Q_stricmp(ent->classname, "nuke") != 0)
                && (Q_stricmp(ent->classname, "LaserDrone") != 0))
            continue;
        if (ent == LaserDrone)
            continue;
        LaserDrone_Fire(LaserDrone, ent);
        return;
    }                           // end for
    if (deathmatch->value)
    {
        for (i = 0; i < game.maxclients; i++)
        {
            ent = g_edicts + i + 1;
            if (DeActivated(LaserDrone))
                return;         // Drone has exploded!
            // if (ent==LaserDrone->owner) continue; // Don't Attack Owner??
            if (!Good_Target(LaserDrone, ent))
                continue;
            LaserDrone_Fire(LaserDrone, ent);
            return;
        }                       // end for
    }
    else
    {
        if (DeActivated(LaserDrone))
            return;             // Drone has exploded!
        while ((ent = findradius(ent, LaserDrone->s.origin, 512)) != NULL)
        {
            if (!(ent->svflags & SVF_MONSTER))
                continue;
            if (!ent->takedamage)
                continue;
            if (ent->health <= 0)
                continue;
            if (!Clear_Path(LaserDrone->s.origin, ent->s.origin))
                continue;
            LaserDrone_Fire(LaserDrone, ent);
            return;
        }                       // end for
    }
    // Nobody around? Re-target on next frame.
    LaserDrone->nextthink = level.time + 0.1;
}
                                // ======================================================
void            Activate_LaserDrone(edict_t * LaserDrone)
{
    // Drone can take damage upon activation..
    // LaserDrone->takedamage = DAMAGE_YES;
    // Play Drone activation sound..
    gi.sound(LaserDrone, CHAN_VOICE, ACTIVATION_SOUND, 1, ATTN_NORM, 0);
    // Let Owner know that LaserDrone is now Active..
    gi.centerprintf(LaserDrone->owner, "LaserDrone is now active!\n");
    LaserDrone->think = LaserDrone_ThinkFast;   // Use the FAST TARGETING routine.
    LaserDrone->nextthink = level.time + 0.1;   // Start thinking on next frame..
    /* *** TOM *** */
    LaserDrone->owner->client->droneent = 0;
    /* *** */
}
                                // ======================================================
void            Launch_LaserDrone(edict_t * ent)
{
    edict_t        *LaserDrone;
    int             x = (random() > 0.5 ? -1 : 1);
    int             y = (random() > 0.5 ? -1 : 1);
    vec3_t          last_angles = {0, 0, 0};
    LaserDrone = G_Spawn();
    LaserDrone->classname = "LaserDrone";
    LaserDrone->owner = ent;
    VectorCopy(ent->s.origin, LaserDrone->s.origin);
    VectorCopy(ent->s.old_origin, LaserDrone->s.old_origin);
    LaserDrone->s.origin[0] += (random() * 20 + 1) * x;
    LaserDrone->s.origin[1] += (random() * 20 + 1) * y;
    LaserDrone->s.origin[2] += ent->viewheight + 8;
    VectorCopy(ent->move_angles, last_angles);
    LaserDrone->velocity[2] = (random() * 40);
    LaserDrone->velocity[1] = ((int) ((random() * 40) + 10 + last_angles[1]) % 60) * y;
    LaserDrone->velocity[0] = ((int) ((random() * 40) + 20 + last_angles[0]) % 60) * x;
    VectorCopy(LaserDrone->velocity, last_angles);
    LaserDrone->movetype = MOVETYPE_FLY;    // Float around gently.
    LaserDrone->solid = SOLID_BBOX; // Enable touch capability.
    // LaserDrone->takedamage = DAMAGE_NO; // No Damage until Activation.
    LaserDrone->takedamage = DAMAGE_YES;
    LaserDrone->clipmask = MASK_SHOT;   // Ability to be hit by weapon's fire.
    LaserDrone->s.effects = EF_ROTATE;  // Spin Drone around slowly.
    LaserDrone->s.modelindex = gi.modelindex("models/items/keys/target/tris.md2");
    LaserDrone->s.modelindex2 = 0;  // no gun model..
    VectorSet(LaserDrone->mins, -20, -20, -40); // size of bbox for touch
    VectorSet(LaserDrone->maxs, 20, 20, 60);    // size of bbox for touch
    LaserDrone->dmg = 80;       // 80 units damaging power.
    LaserDrone->dmg_radius = 300;   // 300 unit radius damage upon explosion.
    LaserDrone->health = 200;   // titanium hull makes it tough to kill!!
    LaserDrone->max_health = 200;
    LaserDrone->wait = 1200;    // Used as search radius var!!
    /* *** TOM *** */
    LaserDrone->die = (void *) LaserDrone_Die;
    /* *** */
    LaserDrone->delay = level.time + 60.0;  // This tape will self-destruct in 60 secs. Good Luck Jim!
    LaserDrone->touch = LaserDrone_Touch;   // Dummy Function..
    LaserDrone->think = Activate_LaserDrone;    // Done only once!
    LaserDrone->nextthink = level.time + 5.0;   // 5 Sec delay before Activation..
    gi.linkentity(LaserDrone);
    ent->client->laserdrone = 1;// Flag LaserDrone as ON.
    gi.centerprintf(ent, "LaserDrone launched!\n5 seconds until activation...\n");
    gi.sound(LaserDrone, CHAN_VOICE, LAUNCH_SOUND, 1, ATTN_NORM, 0);
    /* *** TOM *** */
    ent->client->droneent = 1;
    /* *** */
}
                                // ======================================================
                                // Pay the Piper to initiate the Arial LaserDrone Unit
                                // ======================================================
void            Cmd_LaserDrone_f(edict_t * ent)
{
    int             index;
    // Don't allow dead/respawning players to launch drone!
    if (!G_ClientInGame(ent))
        return;
    /* *** TOM *** */
    if (ent->client->droneent)
        return;
    // If LaserDrone not already ON, then..
    if (ent->client->laserdrone == 0)
    {
        if (!((unsigned long) ann_allow2->value & ANN_ALLOW2_LASERDRONE))
        {
            gi.cprintf(ent, PRINT_HIGH, "LaserDrones are disabled on this server!\n");
            return;
        }
        // Check if ent has any powercells at all..
        index = ITEM_INDEX(FindItem("cells"));
        // If not enough cells or not enough frags then notify ent..
        if (ITEM_IN_ENTS_INVENTORY < DRONE_CELLS)
        {
            gi.centerprintf(ent, "You need %d cells for a LaserDrone!\n", DRONE_CELLS);
            return;
        }
        // Try to deduct the cells first!
        if (ITEM_IN_ENTS_INVENTORY >= DRONE_CELLS)
        {
            // Deduct 50 cells from inventory
            ITEM_IN_ENTS_INVENTORY -= DRONE_CELLS;
            // And, activate the LaserDrone..
            Launch_LaserDrone(ent);
            return;
        }                       // endif
        // Otherwise, if ent has enough frags,
        // if (ent->client->resp.score > 5) {
        // Then, deduct frags from ent's Score
        // ent->client->resp.score -= 5;
        // And, activate the LaserDrone..
        // Launch_LaserDrone(ent);
        // return;
        // } // endif
    }                           // endif
    else
        // Turn OFF - Activate Self Destruct.
        gi.centerprintf(ent, "LaserDrone disarmed!\n");
    ent->client->laserdrone = 0;
}
