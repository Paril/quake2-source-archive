#include "g_local.h"

#define DOM_EXPMULT_NORMAL 0.1
#define DOM_EXPMULT_SINGLE 0.25

void domination_point_think (edict_t *ent)
{
	ent->enemy = NULL;
}

void domination_point_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;
	if (ent->touch_debounce_time > level.time)
		return;

	if (ent->enemy) {
		if ((ent->enemy == other) && (ent->count != other->count) && (other->health > 0) && (other->count > 0)) {
			if (other->count == 1) { // Captured by red guy
				if (!ent->spawnflags)
					ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx &= ~RF_SHELL_GREEN;
				ent->s.renderfx |= RF_SHELL_RED;
				if (!ent->spawnflags) {
					level.red_team_bonus += DOM_EXPMULT_NORMAL;
					if (ent->count != 0)
						level.green_team_bonus -= DOM_EXPMULT_NORMAL;
				} else {
					level.red_team_bonus += DOM_EXPMULT_SINGLE;
					if (ent->count != 0)
						level.green_team_bonus -= DOM_EXPMULT_SINGLE;
				}
			} else if (other->count == 2) { // Captured by green guy
				if (!ent->spawnflags)
					ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx &= ~RF_SHELL_RED;
				ent->s.renderfx |= RF_SHELL_GREEN;
				if (!ent->spawnflags) {
					if (ent->count != 0)
						level.red_team_bonus -= DOM_EXPMULT_NORMAL;
					level.green_team_bonus += DOM_EXPMULT_NORMAL;
				} else {
					if (ent->count != 0)
						level.red_team_bonus -= DOM_EXPMULT_SINGLE;
					level.green_team_bonus += DOM_EXPMULT_SINGLE;
				}
			}

			ent->count = other->count;
			gi.bprintf(PRINT_HIGH, "%s captures a point for the %s team!\n", other->client->pers.netname, (other->count == 1)?"RED":"GREEN");
		}
		return;
	}
	ent->think = NULL;

	if (ent->count != other->count) {
		if ((other->count == 1) || (other->count == 2)) {
			gi.sound(ent, CHAN_AUTO|CHAN_RELIABLE, gi.soundindex("world/klaxon2.wav"), 1, ATTN_NORM, 0);
			ent->touch_debounce_time = level.time + 2;
			ent->enemy = other;
			ent->think = domination_point_think;
			ent->nextthink = level.time + 2.2;
		}
		else
			return;
	} else
		return;
}

void droptofloor (edict_t *ent);
void SP_domination_point (edict_t *ent)
{
	if (!deathmatch->value || (teams->value != 2)) {
		G_FreeEdict(ent);
		return;
	}

	ent->model = "models/items/keys/power/tris.md2";
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_TRIGGER;
	ent->touch = domination_point_touch;
	ent->s.modelindex = gi.modelindex ("models/items/keys/power/tris.md2");
	VectorSet (ent->mins, -16, -16, -16);
	VectorSet (ent->maxs, 16, 16, 16);

/*	if (!strcmp(ent->classname, "item_flag_team1")) {
		ent->count = 1;
		ent->spawnflags = 1;
	}
	if (!strcmp(ent->classname, "item_flag_team2")) {
		ent->count = 2;
		ent->spawnflags = 1;
	}*/

	if (!ent->spawnflags)
		ent->s.effects |= EF_COLOR_SHELL;
	if (ent->count == 1) {
		ent->s.renderfx |= RF_SHELL_RED;
		if (!ent->spawnflags)
			level.red_team_bonus += DOM_EXPMULT_NORMAL;
		else
			level.red_team_bonus += DOM_EXPMULT_SINGLE;
	} else if (ent->count == 2) {
		ent->s.renderfx |= RF_SHELL_GREEN;
		if (!ent->spawnflags)
			level.red_team_bonus += DOM_EXPMULT_NORMAL;
		else
			level.red_team_bonus += DOM_EXPMULT_SINGLE;
	}
	gi.linkentity (ent);
}

void SP_info_monster_spawn (edict_t *ent) {
	if (!deathmatch->value) {
		G_FreeEdict(ent);
		return;
	}

	if (ent->spawnflags == 0)
		level.monspawncount++;

	gi.linkentity (ent);
}

/*
==============
SpawnCustomEntities

Reads from a file and spawns custom domination points
==============
*/

void ED_CallSpawn (edict_t *ent);
void SpawnCustomEntities(void)
{
	FILE *file;
	edict_t *ent;
	char path[128];
	int type, count, flags, done = 0;
	vec3_t pos;
	vec3_t dir;

	if (!deathmatch->value)
		return;

	sprintf(path, "%s/%s.cdp", cdpspath->string, level.mapname);

	if ((file = fopen(path, "rb")) == NULL) {
		gi.dprintf("%s does not exist\n", path);
		return;
	}

	while(!done) {
		if (!fread(&type, sizeof(int), 1, file)) {
			done = 1;
			break;
		}
		if (!fread(&count, sizeof(int), 1, file)) {
			done = 1;
			break;
		}
		if (!fread(&flags, sizeof(int), 1, file)) {
			done = 1;
			break;
		}
		if (!fread(&pos[0], sizeof(float), 1, file)) {
			done = 1;
			break;
		}
		if (!fread(&pos[1], sizeof(float), 1, file)) {
			done = 1;
			break;
		}
		if (!fread(&pos[2], sizeof(float), 1, file)) {
			done = 1;
			break;
		}
		if (!fread(&dir[0], sizeof(float), 1, file)) {
			done = 1;
			break;
		}
		if (!fread(&dir[1], sizeof(float), 1, file)) {
			done = 1;
			break;
		}
		if (!fread(&dir[2], sizeof(float), 1, file)) {
			done = 1;
			break;
		}
//		gi.dprintf("%f, %f, %f\n", pos[0], pos[1], pos[2]);
		if (type == 0) {
			if (teams->value == 2) {
				ent=G_Spawn();
				ent->count = count;
				ent->spawnflags = flags;
				VectorCopy(pos, ent->s.origin);
				ent->classid = CI_DOM_POINT;
				ent->classname = "item_domination_point";
		//		gi.dprintf("Spawning %s at %s (count %d and spawnflags %d)\n",
		//			ent->classname, vtos(ent->s.origin), ent->count, ent->spawnflags);
				ED_CallSpawn(ent);
			}
		} else {
			ent=G_Spawn();
			ent->dmg = type;
			ent->count = count;
			ent->spawnflags = flags;
			VectorCopy(pos, ent->s.origin);
			dir[2] = 0;
			VectorCopy(dir, ent->s.angles);
			ent->classid = CI_GIEX_MONSTERSPAWN;
			ent->classname = "info_monster_spawn";
//			gi.dprintf("Spawning %s at %s\n", ent->classname, vtos(ent->s.origin));
			ED_CallSpawn(ent);
		}
	}
	fclose(file);
}

void CreateCustomEntity(int type, int c, int sf, vec3_t pos, vec3_t dir)
{
	FILE *file;
	char path[128];

	if (!sv_cheats->value)
	{
		gi.dprintf ("You must run the server with '+set cheats 1' to create custom ents.\n");
		return;
	}

	sprintf(path, "%s/%s.cdp", cdpspath->string, level.mapname);
	if ((file = fopen(path, "ab")) == NULL) {
		gi.dprintf("Error opening file, make sure the quake2/giex/cdps directory exists\n");
		return;
	}
	gi.dprintf("Creating custom entity at %s (count %d and spawnflags %d)\n",
		vtos(pos), c, sf);
	if (!fwrite(&type, sizeof(int), 1, file)) {
		gi.dprintf("Error writing to file, possibly corrupted\n");
		fclose(file);
		return;
	}
	if (!fwrite(&c, sizeof(int), 1, file)) {
		gi.dprintf("Error writing to file, possibly corrupted\n");
		fclose(file);
		return;
	}
	if (!fwrite(&sf, sizeof(int), 1, file)) {
		gi.dprintf("Error writing to file, possibly corrupted\n");
		fclose(file);
		return;
	}
	if (!fwrite(&pos[0], sizeof(float), 1, file)) {
		gi.dprintf("Error writing to file, possibly corrupted\n");
		fclose(file);
		return;
	}
	if (!fwrite(&pos[1], sizeof(float), 1, file)) {
		gi.dprintf("Error writing to file, possibly corrupted\n");
		fclose(file);
		return;
	}
	if (!fwrite(&pos[2], sizeof(float), 1, file)) {
		gi.dprintf("Error writing to file, possibly corrupted\n");
		fclose(file);
		return;
	}
	if (!fwrite(&dir[0], sizeof(float), 1, file)) {
		gi.dprintf("Error writing to file, possibly corrupted\n");
		fclose(file);
		return;
	}
	if (!fwrite(&dir[1], sizeof(float), 1, file)) {
		gi.dprintf("Error writing to file, possibly corrupted\n");
		fclose(file);
		return;
	}
	if (!fwrite(&dir[2], sizeof(float), 1, file)) {
		gi.dprintf("Error writing to file, possibly corrupted\n");
		fclose(file);
		return;
	}
	fclose(file);
}
