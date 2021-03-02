#include "g_local.h"
#include "g_brrr.h"

#define DEBRIS1_MODEL "models/objects/debris1/tris.md2"
#define DEBRIS2_MODEL "models/objects/debris2/tris.md2"
#define DEBRIS3_MODEL "models/objects/debris3/tris.md2"

qboolean        entinrange(vec3_t start, edict_t * player);
qboolean        entinrange2(vec3_t start, edict_t * player);
                                // ======================================================
                                // ========== PARTICLE TORTURE CHAMBER ==================
                                // ======================================================
#define EF_NONE 0x00000000      // No effects
#define BURNING1_SOUND gi.soundindex("player/burn1.wav")
#define PAIN50_1_SOUND gi.soundindex("*pain50_1.wav")
#define PAIN75_1_SOUND gi.soundindex("*pain75_1.wav")
#define PAIN100_1_SOUND gi.soundindex("*pain100_1.wav")
#define FRYING_SOUND gi.soundindex("player/fry.wav")
#define TELEPORT_SOUND gi.soundindex("misc/tele1.wav")
#define MASK_OPAQUE (CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define MASK_LIQUID (CONTENTS_SLIME|CONTENTS_LAVA)
#define ITEM_IN_ENTS_INVENTORY ent->client->pers.inventory[index]
void            G_Spawn_Models(int type, short rec_no, vec3_t start, vec3_t endpos, vec3_t offset, vec3_t origin);
void            G_Spawn_Splash(int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin);
void            G_Spawn_Trails(int type, vec3_t start, vec3_t endpos, vec3_t origin);
void            G_Spawn_Sparks(int type, vec3_t start, vec3_t movdir, vec3_t origin);
void            G_Spawn_Explosion(int type, vec3_t start, vec3_t origin);
qboolean        G_ClientInGame(edict_t * ent);
qboolean        G_EntExists(edict_t * ent);
qboolean        G_ClientNotDead(edict_t * ent);
qboolean        Clear_Path(vec3_t spot1, vec3_t spot2);
void            Search_For_Player(edict_t * particle);
                                // ======================================================
                                // Returns true if clear path from spot1 to spot2
                                // ======================================================
                                /* qboolean Clear_Path(vec3_t spot1, vec3_t
                                 spot2) { trace_t tr; tr=gi.trace(spot1,
                                 NULL, NULL, spot2, NULL, MASK_OPAQUE);
                                 return (tr.fraction == 1.0); } */
                                // ======================================================
                                // Need dummy touch because BBOX hits world surfaces..
                                // ======================================================
void            Dummy_Touch(edict_t * a, edict_t * b, cplane_t * c, csurface_t * d)
{
}
                                // ======================================================
                                // Explode the chamber at particle's origin and
                                // then free up all the chamber entities..
                                // ======================================================
void            Chamber_Explode(edict_t * particle)
{
    edict_t        *item;
    // Destroy the Chamber in a big fireball.
    G_Spawn_Explosion(TE_EXPLOSION2, particle->s.origin, particle->s.origin);
    particle->owner->client->chamber = 0;
    particle->owner->client->chamberent = 0;
    // Throw chamber debris all over the place...
    ThrowDebris(particle, DEBRIS3_MODEL, 3.50, particle->s.origin);
    ThrowDebris(particle, DEBRIS3_MODEL, 2.50, particle->s.origin);
    ThrowDebris(particle, DEBRIS3_MODEL, 1.50, particle->s.origin);
    ThrowDebris(particle, DEBRIS3_MODEL, 4.50, particle->s.origin);
    ThrowDebris(particle, DEBRIS3_MODEL, 3.75, particle->s.origin);
    ThrowDebris(particle, DEBRIS3_MODEL, 2.30, particle->s.origin);
    ThrowDebris(particle, DEBRIS3_MODEL, 1.00, particle->s.origin);
    // Assign any Radius Damage frags to the particle's owner..
    T_RadiusDamage(particle, particle->owner, particle->dmg, NULL, particle->dmg_radius, MOD_SPLASH);
    // *** TOM - Scan for lost chamber parts (from collisions with BSP objects)
    item = NULL;
    while (1)
    {
        item = G_Find(item, FOFS(classname), "ChamberPipe");
        if (item == NULL)
            break;
        if (item->parent == particle->activator)
            G_FreeEdict(item);
    }
    item = NULL;
    while (1)
    {
        item = G_Find(item, FOFS(classname), "ChamberLaser");
        if (item == NULL)
            break;
        if (item->parent == particle->activator)
            G_FreeEdict(item);
    }
    item = NULL;
    while (1)
    {
        item = G_Find(item, FOFS(classname), "ChamberTop");
        if (item == NULL)
            break;
        if (item->parent == particle->activator)
            G_FreeEdict(item);
    }
    // Free these in reverse order.
    if (particle->activator)
        G_FreeEdict(particle->activator);   // Bot Respawn Pad
    G_FreeEdict(particle);      // Particle Entity
}
                                // ==========================================================
                                // Player is linked to Particle as particle->goalentity.
                                // Chamber is linked to Particle as particle->activator.
                                // ==========================================================
void            Torture_The_Bastard(edict_t * particle)
{
    float           dmg = 20.0;
    int             sound_index;
    vec3_t          zvec = {0, 0, 0};
    // Player died? or Player has escaped!! (by self teleport)
    if ((particle->goalentity->health <= 0)
    // || (!G_Within_Radius(particle->s.origin,particle->goalentity->s.origin,10))) {
            || (!entinrange2(particle->s.origin, particle->goalentity)))
    {
        if (particle->goalentity->client)
            particle->goalentity->client->inchamber = 0;
        particle->goalentity = NULL;
        // NOTE: Chamber won't self destruct with player inside!
        if (particle->delay < level.time)
            particle->think = Chamber_Explode;
        else
            // Go search for another victim!
            particle->think = Search_For_Player;
        particle->nextthink = level.time + 0.1; // On next frame..
        return;
    }
    // If last torture cycle, then
    // Use with exploding body parts tutorial!
    if (particle->goalentity->health - dmg <= 0)
        dmg *= 3;
    else
    {
        // Pick a painful sound to play
        switch ((((int) (random() * 10 + 0.5)) % 6) + 1)
        {
        case 1:
            sound_index = PAIN100_1_SOUND;
            break;
        case 2:
            sound_index = PAIN50_1_SOUND;
            break;
        case 3:
            sound_index = PAIN75_1_SOUND;
            break;
        default:
            sound_index = BURNING1_SOUND;
        }
        // Play randomly selected pain sound!
        gi.sound(particle->goalentity, CHAN_VOICE, sound_index, 1, ATTN_NORM, 0);
    }
    // Damage this player in decrements of 'dmg' units per cycle.
    T_Damage(particle->goalentity, particle, particle->owner, zvec, particle->s.origin, NULL, dmg, 0, 0, MOD_CHAMBER);
    if (particle->goalentity->health <= 0)  // dead :)
        particle->count++;
    // Cycle every 1 sec until player dead.
    // Need time for scream sound to finish..
    particle->think = Torture_The_Bastard;
    particle->nextthink = level.time + 1.0; // 1 sec intervals.
    /* *** TOM *** */
    if (particle->goalentity->client)
    {
        particle->goalentity->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        particle->goalentity->client->ps.pmove.pm_time = 100;
    }
    /* *** */
}
                                // ===============================================================
                                // Teleport particle->goalentity into Particle Chamber..
                                // ===============================================================
void            Beam_Player_Into_Chamber(edict_t * particle)
{
    vec3_t          start, zvec = {0, 0, 0};
    // Turn OFF particle effect.
    particle->s.effects = EF_NONE;
    // Tell Pipe to spurt Red sparks..
    particle->activator->chain->noise_index2 = 0;
    // Play teleport sound effects.
    gi.sound(particle->goalentity, CHAN_VOICE, TELEPORT_SOUND, 1, ATTN_NORM, 0);
    // Temp storage of player's previous location.
    VectorCopy(particle->goalentity->s.origin, start);
    // Physically relocate this player into Chamber.
    VectorCopy(particle->s.origin, particle->goalentity->s.origin);
    // Large colored particle effect at player's old spot.
    G_Spawn_Splash(TE_LASER_SPARKS, 24, 0xd0d1d2d3, start, zvec, start);
    // Maximize health to make'em suffer longer!
    // particle->goalentity->health = particle->goalentity->max_health;
    // TOM - no, don't
    // Start Torture effect on next frame
    particle->think = Torture_The_Bastard;
    particle->nextthink = level.time + 0.1;
    /* *** TOM *** */
    if (particle->goalentity->client)
    {
        particle->goalentity->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        particle->goalentity->client->ps.pmove.pm_time = 100;
        particle->goalentity->client->inchamber = 1;
    }
    /* *** */
}
                                // ==========================================================
                                // Search for first visible player with targeted radius units
                                // then teleport player into Particle Chamber..
                                // ==========================================================
void            Search_For_Player(edict_t * particle)
{
    int             i = 0;
    trace_t         tr;
    edict_t        *player = NULL;
    vec3_t          start;
    edict_t        *ent = NULL;

    if (particle->activator == NULL || particle->activator->chain==NULL || particle->activator->chain->health <= 0)
    {
        Chamber_Explode(particle);
        return;
    }
    /* *** TOM *** */
    particle->owner->client->chamberent = 0;
    // Time to self destruct? OR, Owner has died?
    if ((particle->delay < level.time)
            || (particle->owner->client->chamber == 0))
    {
        /* *** TOM *** */
        particle->owner->client->chamber = 0;
        /* *** */
        particle->think = Chamber_Explode;  // Self destruct..
        particle->nextthink = level.time + 0.1; // On next frame.
        return;
    }
    // Chamber makes a frying/hissing sound!!
    gi.sound(particle, CHAN_VOICE, FRYING_SOUND, 1, ATTN_STATIC, 0);
    // Turn ON particle effect when nobody in chamber.
    particle->s.effects = EF_TELEPORTER;
    // Tell Pipe to spurt Green sparks (nobody in chamber)..
    particle->activator->chain->noise_index2 = 1;
    // Get starting vector at very top of pipe
    VectorCopy(particle->activator->chain->s.origin, start);
    // start[2] += (65+12); // Beam starts from top of pipe!!
    start[2] += 40;
    // Is top of pipe in solid?
    if (gi.pointcontents(start) & MASK_SHOT)
        VectorCopy(particle->s.origin, start);  // Make an adjustment
    // Grab each visible player in 1000 unit radius!
    if (deathmatch->value)
    {
        for (i = 0; i < game.maxclients; i++)
        {
            player = g_edicts + i + 1;
            if (!G_ClientInGame(player))
                continue;
            /* *** TOM *** */
            if ((int) ctf->value && player->client)
            {
                if ((particle->tomvar == 1 && player->client->resp.ctf_team == CTF_TEAM1) ||
                        (particle->tomvar == 2 && player->client->resp.ctf_team == CTF_TEAM2))
                    continue;
            }
            else if (OnSameTeam(player, particle->owner) || player == particle->owner)
                continue;
            /* *** */
            if (!player->takedamage)
                continue;
            if (!Clear_Path(start, player->s.origin))
                continue;
            /* *** TOM: Changed this bit to use my routine *** */
            if (!entinrange(start, player))
                continue;
            /* *** */
            // Laser Beam reaches out and teleports player into chamber.
            tr = gi.trace(start, NULL, NULL, player->s.origin, particle->activator, MASK_SHOT);
            G_Spawn_Trails(TE_BFG_LASER, start, tr.endpos, tr.endpos);
            particle->goalentity = player;  // Linked to particle entity
            particle->think = Beam_Player_Into_Chamber;
            particle->nextthink = level.time + 0.2; // Give time to teleport.
            return;
        }                       // end for
        // Continue searching on next frame.
        particle->think = Search_For_Player;
        particle->nextthink = level.time + 0.1;
    }
    else                        // *** TOM *** Single Player
        //
        while ((player = findradius(player, particle->s.origin, 512)) != NULL)
        {
            if (!(player->svflags & SVF_MONSTER))
                continue;
            if (!player->takedamage)
                continue;
            if (player->health <= 0)
                continue;
            if (!Clear_Path(start, player->s.origin))
                continue;
            // Laser Beam reaches out and teleports player into chamber.
            tr = gi.trace(start, NULL, NULL, player->s.origin, particle->activator, MASK_SHOT);
            G_Spawn_Trails(TE_BFG_LASER, start, tr.endpos, tr.endpos);
            particle->goalentity = player;  // Linked to particle entity
            particle->think = Beam_Player_Into_Chamber;
            particle->nextthink = level.time + 0.2; // Give time to teleport.
            return;
        }                       // end for
    // Continue searching on next frame.
    particle->think = Search_For_Player;
    particle->nextthink = level.time + 0.1;
}
                                /* *** TOM *** */
qboolean        entinrange(vec3_t start, edict_t * player)
{
    edict_t        *ent = NULL;
    while ((ent = findradius(ent, start, 512)) != NULL)
    {
        if (ent == player)
            return true;
    }
    return false;
}
qboolean        entinrange2(vec3_t start, edict_t * player)
{
    edict_t        *ent = NULL;
    while ((ent = findradius(ent, start, 10)) != NULL)
    {
        if (ent == player)
            return true;
    }
    return false;
}
                                /* *** */
                                // ==========================================================
                                // Particle communicates to 'Stove Pipe' thru noise_index2
                                // ==========================================================
void            StovePipe_Think(edict_t * pipe)
{
    vec3_t          start;
    // Place effect at end of pipe.
    VectorCopy(pipe->s.origin, start);
    start[2] += 12;
    if (pipe->noise_index2 > 0)
        // Green sparks coming out of Top Pipe - Nobody in Chamber
        G_Spawn_Splash(TE_LASER_SPARKS, 12, 0xd0d1d2d3, start, NULL, start);
    else
        // Red blood spurting out of Top Pipe - Chamber Occupied
        G_Spawn_Splash(TE_LASER_SPARKS, 12, 0xf2f2f0f0, start, NULL, start);
    // More effects every other frame.
    pipe->nextthink = level.time + 0.2; // No need every frame.
}
                                // ==========================================================
                                // Keeps the laser beam and top particle effect active..
                                // ==========================================================
void            Laser_Think(edict_t * laser)
{
    trace_t         tr;
    vec3_t          end = {0, 0, 0};
    if (laser->parent->health == 0)
    {
        Chamber_Explode(laser);
        return;
    }
    VectorMA(laser->s.origin, 70, laser->movedir, end);
    tr = gi.trace(laser->s.origin, NULL, NULL, end, laser, MASK_ALL);
    VectorCopy(tr.endpos, laser->s.old_origin);
    laser->nextthink = level.time + 0.1;    // Every frame else it flashes!!
}
                                // ==========================================================
                                // Place the 'stove pipe' on top of Particle Chamber.
                                // ONLY way to kill chamber is to 'Smoke' its Stack!
                                // ==========================================================
void            Create_StovePipe(edict_t * botpad)
{
    edict_t        *pipe = NULL;
    // ------------------------------------------
    // -------- Stove Pipe Entity ---------------
    // ------------------------------------------
    pipe = G_Spawn();
    botpad->chain = pipe;       // botpad's link to pipe
    pipe->owner = botpad->owner;// For killing!!
    pipe->parent = botpad;
    pipe->classname = "ChamberPipe";
    pipe->takedamage = DAMAGE_YES;
    pipe->health = 200;
    pipe->max_health = 200;
    pipe->noise_index2 = 1;     // 1=GREEN, 0=RED
    pipe->deadflag = DEAD_DEAD; // So, Not in game..
    pipe->clipmask = MASK_SHOT;
    VectorClear(pipe->velocity);
    pipe->movetype = MOVETYPE_NONE;
    pipe->solid = SOLID_BBOX;
    pipe->s.effects = EF_NONE;
    VectorSet(pipe->mins, -2, -2, -8);  // Size of BBOX
    VectorSet(pipe->maxs, 2, 2, 8); // Size of BBOX
    pipe->s.modelindex = gi.modelindex("models/objects/minelite/light1/tris.md2");
    pipe->s.modelindex2 = 0;
    // ------ Position Relative to BotPad -------
    VectorCopy(botpad->s.origin, pipe->s.origin);
    pipe->s.origin[2] += 65;
    VectorCopy(pipe->s.origin, pipe->s.old_origin);
    // ------------------------------------------
    pipe->touch = Dummy_Touch;
    pipe->think = StovePipe_Think;
    pipe->die=(void *)nothing;
    pipe->nextthink = level.time + 0.5; // Wait to start sparks effect.
    gi.linkentity(pipe);
}
                                // ======================================================
                                // Create the Laser Beam as separate entity.
                                // ======================================================
void            Create_Laser_Entity(edict_t * botpad)
{
    edict_t        *laser = NULL;
    // ------------------------------------------
    // -------- Laser Beam Entity ---------------
    // ------------------------------------------
    laser = G_Spawn();
    botpad->mynoise = laser;    // botpad's link to Laser
    laser->owner = botpad->owner;   // For killing!!
    laser->parent = botpad;
    laser->classname = "ChamberLaser";
    laser->movetype = MOVETYPE_NONE;
    laser->solid = SOLID_BBOX;
    laser->deadflag = DEAD_DEAD;// So not in game..
    laser->health = 0;          // Can take damage!
    laser->takedamage = DAMAGE_NO;
    laser->clipmask = MASK_SHOT;
    laser->s.renderfx |= RF_BEAM | RF_TRANSLUCENT | RF_GLOW;
    laser->s.modelindex = 1;    // must be non-zero
    laser->s.frame = 54;        // beam diameter in units
    if (ctf->value)
    {
        if (botpad->owner->client->resp.ctf_team == CTF_TEAM1)  /* RED TEAM */
            laser->s.skinnum = 0xf2f2f0f0;
        else if (botpad->owner->client->resp.ctf_team == CTF_TEAM2) /* BLUE
                                 TEAM */
            laser->s.skinnum = 0xf3f3f1f1;
        else
            laser->s.skinnum = 0xd0d1d2d3;  /* UNKNOWN */
    }
    else
        laser->s.skinnum = 0xd0d1d2d3;  /* NOT CTF */
    VectorClear(laser->velocity);   // No Movement
    VectorClear(laser->s.angles);
    VectorSet(laser->movedir, 0, 0, 1);
    VectorSet(laser->mins, -20, -20, -60);  // Size of BBOX for Touch()
    VectorSet(laser->maxs, 20, 20, 60); // Size of BBOX for Touch()
    // ------ Position Relative to BotPad -------
    VectorCopy(botpad->s.origin, laser->s.origin);
    laser->s.origin[2] += -15;
    VectorCopy(laser->s.origin, laser->s.old_origin);
    // ------------------------------------------
    laser->touch = Dummy_Touch;
    laser->think = Laser_Think;
    laser->nextthink = level.time + 0.1;
    gi.linkentity(laser);
}
                                // ======================================================
                                // Create the Particle Effect as separate entity.
                                // ======================================================
void            Create_Particle_Effect(edict_t * botpad)
{
    edict_t        *particle = NULL;
    // ------------------------------------------
    // ------- Particle Effect Entity -----------
    // ------------------------------------------
    particle = G_Spawn();
    if (ctf->value)
    {
        if (botpad->owner->client->resp.ctf_team == CTF_TEAM1)
            particle->tomvar = 1;
        else if (botpad->owner->client->resp.ctf_team == CTF_TEAM2)
            particle->tomvar = 2;
        else
            particle->tomvar = 0;
    }
    else
        particle->tomvar = 0;
    botpad->movetarget = particle;  // botpad's link to particle
    particle->goalentity = NULL;// Linked to Captive player..
    particle->activator = botpad;   // particle's link to botpad
    particle->owner = botpad->owner;    // Frags go to owner..
    botpad->owner = NULL;       // Turn this OFF
    particle->movetype = MOVETYPE_NONE;
    particle->solid = SOLID_NOT;// Laser passes thru ent..
    particle->takedamage = DAMAGE_NO;
    particle->health = 0;       // So not In Game..
    particle->deadflag = DEAD_DEAD; // So not in game..
    particle->s.effects = EF_TELEPORTER;
    particle->s.modelindex = 0;
    particle->s.modelindex2 = 0;
    particle->dmg = 80;         // for Detonation Damage
    particle->dmg_radius = 300;
    VectorClear(particle->mins);
    VectorClear(particle->maxs);
    VectorClear(particle->velocity);
    VectorClear(particle->s.angles);
    // ------ Position Relative to BotPad -------
    VectorCopy(botpad->s.origin, particle->s.origin);
    particle->s.origin[2] += 7;
    VectorCopy(particle->s.origin, particle->s.old_origin);
    // ------------------------------------------
    particle->delay = level.time + 120; // Set Self Destruct timer.
    particle->think = Search_For_Player;
    particle->nextthink = level.time + 5.0; // Activate search..
    particle->count = 0;        // frags
    gi.linkentity(particle);
}
                                // ======================================================
                                // Called from SV_Physics_Bounce(), SV_Physics_Toss()
                                // Moves top spawn pad in sync with bottom spawn pad.
                                // ======================================================
void            Link_All_Together(edict_t * botpad)
{
    // Move Top Respawn Pad in sync with Bottom Respawn Pad.
    VectorCopy(botpad->s.origin, botpad->mynoise2->s.origin);
    botpad->mynoise2->s.origin[2] += 32;
    // Immediately update the world..
    gi.linkentity(botpad->mynoise2);
    // Chamber has stopped moving, so..
    if (VectorLength(botpad->velocity) <= 1 && botpad->tomvar == 0)
    {
        botpad->tomvar = 1;
        Create_StovePipe(botpad);   // Put Pipe on top of Chamber.
        Create_Laser_Entity(botpad);    // Activate the Laser Field.
        Create_Particle_Effect(botpad);
    }                           // Initiate Particle Effect
}
                                // ======================================================
                                // Create Bottom Pad, Top Pad and Particle Entity
                                // Also works for:
                                // MOVETYPE_TOSS
                                // MOVETYPE_FLY
                                // MOVETYPE_FLYMISSILE
                                // MOVETYPE_NONE
                                // Best: MOVEYTPE_BOUNCE with Improve Bounce Physics!
                                // ======================================================
void            Make_Particle_Chamber(edict_t * owner,  // Owner of Chamber
                                      vec3_t origin,    // Owner's origin
                                      vec3_t v_angles,  // Owner's View Angles
                                      int mtype,    // MOVETYPE_*
                                      int preset)   // 1 if preset origin
{
    edict_t        *botpad = NULL, *toppad = NULL;
    vec3_t          torigin;
    vec3_t          forward = {0, 0, 0};
    vec3_t          right = {0, 0, 0};
    vec3_t          up = {0, 0, 0};
    vec3_t          end = {0, 0, 0};
    vec3_t          down = {0, 0, -1};
    // Don't want to effect original origin..
    VectorCopy(origin, torigin);
    // If origin not pre-defined then...
    if (!preset)
    {
        AngleVectors(v_angles, forward, right, up);
        VectorMA(torigin, 100, forward, end);   // forward 100 units.
        if (gi.pointcontents(end) & MASK_SHOT)
        {
            gi.cprintf(owner, PRINT_HIGH, "Cannot project into solid!\n");
            owner->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] += 200;
            return;
        }
        VectorMA(torigin, 80, forward, end);    // back off 20 units.
        VectorCopy(end, torigin);
    }                           // Use this projected origin..
    // Ensure valid MOVETYPE_ flag!
    switch (mtype)
    {
    case MOVETYPE_TOSS:
        break;
    case MOVETYPE_FLY:
        break;
    case MOVETYPE_FLYMISSILE:
        break;
    case MOVETYPE_BOUNCE:
        break;
    default:
        mtype = MOVETYPE_NONE;
    }                           // end switch
    // ------------------------------------------
    // ------ Bottom Respawn Pad Entity ---------
    // ------------------------------------------
    botpad = G_Spawn();
    botpad->owner = owner;
    botpad->clipmask = MASK_SHOT;
    botpad->movetype = mtype;   // Only assignment here!
    botpad->solid = SOLID_BBOX;
    botpad->takedamage = DAMAGE_NO;
    botpad->s.effects = EF_NONE;
    botpad->health = 100;
    botpad->deadflag = DEAD_DEAD;   // So not in game..
    botpad->s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
    botpad->s.modelindex2 = 0;
    botpad->classname = "Chamber";
    VectorSet(botpad->mins, -20, -20, -30); // Size of BBOX for Touch()
    VectorSet(botpad->maxs, 20, 20, 10);    // Size of BBOX for Touch()
    VectorClear(botpad->velocity);
    if (!preset)
    {
        VectorScale(forward, 600, botpad->velocity);
        VectorMA(botpad->velocity, 200 + crandom() * 10.0, up, botpad->velocity);
        VectorMA(botpad->velocity, crandom() * 10.0, right, botpad->velocity);
    }
    VectorClear(botpad->avelocity);
    VectorClear(botpad->s.angles);
    // ------------------------------------------
    VectorCopy(torigin, botpad->s.origin);
    VectorCopy(botpad->s.origin, botpad->s.old_origin);
    // ------------------------------------------
    botpad->touch = Dummy_Touch;
    botpad->think = NULL;
    botpad->nextthink = 0;
    // ------------------------------------------
    // ------- Top Respawn Pad Entity -----------
    // ------------------------------------------
    toppad = G_Spawn();
    botpad->mynoise2 = toppad;  // botpad's link to toppad
    toppad->owner = NULL;
    toppad->clipmask = MASK_SHOT;
    toppad->movetype = MOVETYPE_NONE;
    toppad->solid = SOLID_TRIGGER;
    toppad->takedamage = DAMAGE_NO;
    toppad->s.effects = EF_NONE;
    toppad->health = 0;         // So not In Game..
    toppad->deadflag = DEAD_DEAD;   // So not in game..
    toppad->s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
    toppad->s.modelindex2 = 0;
    toppad->classname = "ChamberTop";
    toppad->parent = botpad;
    VectorSet(toppad->mins, -20, -20, -5);  // Size of BBOX for Touch()
    VectorSet(toppad->maxs, 20, 20, 5); // Size of BBOX for Touch()
    VectorClear(toppad->velocity);
    VectorClear(toppad->s.angles);
    toppad->s.angles[2] += 180; // flip upside down..
    // ------ Position Relative to BotPad -------
    VectorCopy(botpad->s.origin, toppad->s.origin);
    toppad->s.origin[2] += 32;
    VectorCopy(toppad->s.origin, toppad->s.old_origin);
    // ------------------------------------------
    toppad->touch = NULL;
    toppad->think = NULL;
    toppad->nextthink = 0;
    gi.linkentity(botpad);
    gi.linkentity(toppad);
    owner->client->chamber = 1; // Flag as ON
    /* *** TOM *** */
    owner->client->chamberent = 1;
}
#if 0
                                // =================================================================
                                // The following should be considered examples only...
                                //
                                // Put these two lines at the top of ClientThink and move around to
                                // get the vector position which you like for your particular Q2DM
                                // map then put it in the array..
                                //
                                // ent->movetype = MOVETYPE_NOCLIP;
                                // gi.dprintf("Vector: %s\n",vtos(ent->s.origin));
                                // =================================================================
qboolean        MakePresetChamber(edict_t * ent)
{
    vec3_t          mapvec[] = {{26, 878, 810}, // Q2DM1
    {863, -550, 263},          // Q2DM2
    {332, 8, 221},             // Q2DM3
    {-17, 73, 327},            // Q2DM4
    {223, 852, 436},           // Q2DM5
    {715, 1397, 63},           // Q2DM6
    {1114, 487, 128},          // Q2DM7
    {-468, 731, 92},};         // Q2DM8
    char            mapstr[10];
    int             i;
    for (i = 1; i <= 8; i++)
    {
        sprintf(mapstr, "q2dm%d", i);   // only q2dm maps!!
        if (Q_stricmp(level.mapname, mapstr) == 0)
        {
            gi.dprintf("Map index: %d\n", i);
            Make_Particle_Chamber(ent, mapvec[i - 1], NULL, MOVETYPE_BOUNCE, 1);
            return true;
        }
    }
    gi.dprintf("MAPS ARE NOT Q2DM MAPS!\n");
    return false;
}
#endif
                                // ======================================================
                                // Pay the Piper for the Chamber
                                // ======================================================
void            Cmd_Chamber_f(edict_t * ent)
{
    int             index;
    float           temp = 0;
    // Don't allow dead/respawning players to have Chamber!
    if (!G_ClientInGame(ent))
        return;
    /* *** TOM *** */
    if (ent->client->chamberent)
        return;
    // debug:
    // if (1) { Make_Particle_Chamber(ent, ent->s.origin, ent->client->v_angle, MOVETYPE_BOUNCE, 0); return; }
    // if (1) { MakePresetChamber(ent); return; }
    if (!ent->client->chamber)
    {
        if (!((unsigned long) ann_allow2->value & ANN_ALLOW2_TORTURECHAMBER))
        {
            gi.cprintf(ent, PRINT_HIGH, "Torture chambers are disallowed on this server!\n");
            return;
        }
        // Check if ent has any powercells at all..
        index = ITEM_INDEX(FindItem("cells"));
        // If not enough cells or not enough frags then notify ent..
        if (ITEM_IN_ENTS_INVENTORY < 200)
        {
            gi.centerprintf(ent, "You need 200 cells for a torture chamber!");
            return;
        }
        // Try to deduct the cells first!
        if (ITEM_IN_ENTS_INVENTORY >= 200)
        {
            // Deduct 200 cells from inventory
            ITEM_IN_ENTS_INVENTORY -= 200;
            // And, activate the Particle Chamber..
            if (ent->client->v_angle[0] > 0)
            {
                temp = ent->client->v_angle[0];
                ent->client->v_angle[0] = 0;
            }
            Make_Particle_Chamber(ent, ent->s.origin, ent->client->v_angle, MOVETYPE_BOUNCE, 0);
            if (temp)
                ent->client->v_angle[0] = temp;
            return;
        }                       // endif
    }                           // endif
    else
        // Turn OFF - Activate Self Destruct.
        ent->client->chamber = 0;
    gi.centerprintf(ent, "Torture chamber deactivated...");
}
