/*
==============================================================================

jawa3  - Trader

==============================================================================
*/

#include "g_local.h"
#include "m_jawa3.h"

static int	sound_agree;
static int	sound_cry1;
static int	sound_cry2;
static int	sound_disagree;
static int	sound_lift1;
static int	sound_lift2;
static int	sound_scare;
static int	sound_talk1;
static int	sound_talk2;
static int	sound_talk3;
static int	sound_talk4;
static int	sound_talk5;
static int	sound_talk6;
static int	sound_work;

// STAND

void jawa3_stand (edict_t *self);

mframe_t jawa3_frames_stand2 [] =
{
	ai_stand, 0, NULL,
};
mmove_t jawa3_move_stand2 = {FRAME_stand1, FRAME_stand1, jawa3_frames_stand2, jawa3_stand};

mframe_t jawa3_frames_stand1 [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t jawa3_move_stand1 = {FRAME_stand1, FRAME_stand15, jawa3_frames_stand1, jawa3_stand};

mframe_t jawa3_frames_idle1 [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t jawa3_move_idle1 = {FRAME_idle1, FRAME_idle6, jawa3_frames_idle1, jawa3_stand};

//************
//HEAD SHAKING
//************

mframe_t jawa3_frames_agree1 [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t jawa3_move_agree1 = {FRAME_agree1, FRAME_agree4, jawa3_frames_agree1, jawa3_stand};

mframe_t jawa3_frames_disagre1 [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t jawa3_move_disagre1 = {FRAME_disagre1, FRAME_disagre4, jawa3_frames_disagre1, jawa3_stand};

void jawa3_stand (edict_t *self)
{
	if(random() > 0.01)
		self->monsterinfo.currentmove = &jawa3_move_stand2;
	else
		self->monsterinfo.currentmove = &jawa3_move_stand1;
}

//
// WALK
//

mframe_t jawa3_frames_walk1 [] =
{
	ai_walk, 3,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 6,  NULL,

	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL
};
mmove_t jawa3_move_walk1 = {FRAME_walk1, FRAME_walk12, jawa3_frames_walk1, NULL};

void jawa3_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &jawa3_move_walk1;
}

//
// RUN
//

void jawa3_run (edict_t *self);

mframe_t jawa3_frames_run1 [] =
{
	ai_run, 10, NULL,
	ai_run, 11, NULL,
	ai_run, 11, NULL,
	ai_run, 16, NULL,
	ai_run, 10, NULL,
	ai_run, 15, NULL
};
mmove_t jawa3_move_run1 = {FRAME_run1, FRAME_run6, jawa3_frames_run1, NULL};

void jawa3_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &jawa3_move_stand1;
	else
	{
		self->monsterinfo.currentmove = &jawa3_move_run1;
	}
}

//
// PAIN
//

mframe_t jawa3_frames_pain1 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL
};
mmove_t jawa3_move_pain1 = {FRAME_pain1, FRAME_pain3, jawa3_frames_pain1, jawa3_run};

//void Trade_Menu (edict_t *trader, edict_t *ent, int num_options);

void jawa3_pain (edict_t *self, edict_t *other, float kick, int damage)
{
//	int i;

//	if (self->health < (self->max_health / 2))
//			self->s.skinnum |= 1;

	if (level.time < self->pain_debounce_time)
	{
		return;
	}

/*	if(self->message)
	{
		for(i=0; i<16; i++)
		{
			if(self->monsterinfo.itemnum[i] == 0)
				break;
		}
		Trade_Menu (self, other, i);
	}*/

	self->pain_debounce_time = level.time + 6;

	if(random() >= 0.5)
		gi.sound (self, CHAN_VOICE, sound_cry1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_cry2, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	self->monsterinfo.currentmove = &jawa3_move_pain1;
}


//
// ATTACK
//

void jawa3_attack(edict_t *self)
{
}


//
// SIGHT
//

void jawa3_sight(edict_t *self, edict_t *other)
{
}

//
// DEATH
//

void jawa3_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t jawa3_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t jawa3_move_death1 = {FRAME_drama1, FRAME_drama19, jawa3_frames_death1, jawa3_dead};

mframe_t jawa3_frames_death2 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t jawa3_move_death2 = {FRAME_nodrama1, FRAME_nodrama9, jawa3_frames_death2, jawa3_dead};

void give_items(edict_t *self, edict_t *other);

void jawa3_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	float		n;
//	int i, j;

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 3; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

/*	for(i=0; i<16; i++)
	{
		if(self->monsterinfo.itemnum[i] == 0)
			break;
	}
//	Trade_Menu (self, attacker, i);
	for(j=0; j<i; j++)
	{
		if (self->monsterinfo.item_names[j] != NULL)
			free(self->monsterinfo.item_names[j]);
	}
*/
	n = random();
	if (n >= 0.5)
		gi.sound (self, CHAN_VOICE, sound_cry1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_cry2, 1, ATTN_NORM, 0);

	n = random();
	if (n >= 0.5)
		self->monsterinfo.currentmove = &jawa3_move_death1;
	else
		self->monsterinfo.currentmove = &jawa3_move_death2;
}

//
// TRADING
//
/*
void trade_item(edict_t *ent, int choice);

void Trade_Menu (edict_t *trader, edict_t *ent, int num_options)
{
	char tmp[80];
	gitem_t *item;
	int i;

	if (ent->client->showscores || ent->client->showinventory ||
		ent->client->showmenu || ent->client->showmsg)
	{
		return;
	}

//	Menu_Clear(ent);

	Menu_Title(ent,"Welcome");

	for(i=0; i<num_options; i++)
	{
		if(trader->monsterinfo.itemnum[i] == 0)
			break;
		item = GetItemByIndex(trader->monsterinfo.itemnum[i]);
		sprintf(tmp,"%s %i", trader->monsterinfo.item_names[i], trader->monsterinfo.inventory[ITEM_INDEX(item)]);
//		sprintf(tmp,"%s %i", item->classname, trader->monsterinfo.inventory[ITEM_INDEX(item)]);
		Menu_Add(ent, tmp);
	}
	Menu_Add(ent, "Exit");
	ent->target_ent = trader;
	ent->client->usr_menu_sel = trade_item;
	Menu_Open(ent);
	ent->client->showscores = 4;
	ent->client->showmenu = 4;
}

void Trade_Menu_Sel(edict_t *ent)
{
	int sel;

	sel = ent->client->menu_choice;
	trade_item(ent, sel);
}

void trade_item(edict_t *ent, int choice)
{
	gitem_t *item;
	edict_t *trader;
	int i;

	if(choice == (ent->client->menu_count-1))
	{
		Menu_Close(ent);
		return;
	}

//	gi.dprintf("choice=%i\n, menu_count=%i\n, showmenu=%i\n", choice, ent->client->menu_count, ent->client->showmenu);

	trader = ent->target_ent;

	item = GetItemByIndex(trader->monsterinfo.itemnum[choice]);
	if(trader->monsterinfo.inventory[ITEM_INDEX(item)] == 0)
		return;

	trader->monsterinfo.inventory[ITEM_INDEX(item)]--;
	if(item->quantity)
		ent->client->pers.inventory[ITEM_INDEX(item)] += item->quantity;
	else
		ent->client->pers.inventory[ITEM_INDEX(item)]++;

	for(i=0; i<16; i++)
	{
		if(trader->monsterinfo.itemnum[i] == 0)
			break;
	}
	Menu_Close(ent);
	Trade_Menu(trader, ent, i);
}

void give_items(edict_t *self, edict_t *other)
{
	gitem_t *dropitem;
	int i, j;

	for(i=0; i<15; i++)
	{
		if(self->monsterinfo.itemnum[i] == 0)
			break;
		for(j=0; j<256; j++)
		{
			dropitem = GetItemByIndex(self->monsterinfo.itemnum[i]);
			self->monsterinfo.inventory[ITEM_INDEX(dropitem)]--;
			other->client->pers.inventory[ITEM_INDEX(dropitem)]++;
			if(self->monsterinfo.inventory[ITEM_INDEX(dropitem)] == 0)
				break;
		}
	}
}

//
// SPAWN
//

void Jawa_LoadInventory (edict_t *ent, char *invfile)
{
	int count_int;
	char count[8], num_char[8];
	char item[80];
	static char text[18];
      int i, j, num_int;
	FILE *f;
	char			file_name[256];
	char			game_dir_name[80];
	cvar_t			*game_dir, *basedir;
	int			index;
	gitem_t *tradeitem;

	game_dir = gi.cvar ("game", "", 0);
	basedir = gi.cvar ("basedir", ".", 0);

	if (!Q_stricmp (game_dir->string, ""))
		sprintf (game_dir_name, "baseq2");
	else 
		sprintf (game_dir_name, "%s", game_dir->string);

	sprintf (file_name, "%s\\%s\\data\\%s.tdr", basedir->string, game_dir_name, invfile);

	f = fopen (file_name, "rt");
	if (!f)
	{
		gi.dprintf("no inventory file\n");
		return;
	}
	else
		gi.dprintf("%s loaded\n", invfile);

	fseek(f, 0, SEEK_SET);

	for(i=0; i<15; i++)
	{
		ent->monsterinfo.itemnum[i] = 0;
	}

	fscanf(f,"%s",num_char);
	num_int = atoi(num_char);

	for(i=0; i<num_int; i++)
	{
		for(j=0; j<18; j++)
		{
			text[j] = fgetc(f);
			if(text[j] == '\n' && j!=0)
			{
				text[j] = '\0';
				break;
			}
			else if(text[j] == '\n')
			{
				text[j] = ' ';
				j--;
				continue;
			}
		}
		fscanf(f,"%s",item);
		fscanf(f,"%s",count);
		count_int = atoi(count);
		if(item == "END\n" || count_int == 0)
		{
//			gi.dprintf("end of file\n");
			break;
		}
//		gi.dprintf("%s: %s=%i\n", text, item, count_int);

		tradeitem = FindItem (item);
		index = ITEM_INDEX(tradeitem);
		ent->monsterinfo.inventory[index] = count_int;
		ent->monsterinfo.itemnum[i] = index;
		ent->monsterinfo.item_names[i] = text;

		ent->monsterinfo.item_names[i] = malloc(18);

		if (ent->monsterinfo.item_names[i] == NULL)
			return;
		sprintf(ent->monsterinfo.item_names[i],"%-17s",text);
		gi.dprintf("*%s*\n", ent->monsterinfo.item_names[i]);
	}

	fclose(f);
}
*/
void SP_monster_jawa3 (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_agree = gi.soundindex ("jawa/j_agree.wav");
	sound_cry1 = gi.soundindex ("jawa/j_cry1.wav");
	sound_cry2 = gi.soundindex ("jawa/j_cry2.wav");
	sound_disagree = gi.soundindex ("jawa/j_disagr.wav");
	sound_lift1 = gi.soundindex ("jawa/j_lift1.wav");
	sound_lift2 = gi.soundindex ("jawa/j_lift2.wav");
	sound_scare = gi.soundindex ("jawa/j_scare.wav");
	sound_talk1 = gi.soundindex ("jawa/j_talk1.wav");
	sound_talk2 = gi.soundindex ("jawa/j_talk2.wav");
	sound_talk3 = gi.soundindex ("jawa/j_talk3.wav");
	sound_talk4 = gi.soundindex ("jawa/j_talk4.wav");
	sound_talk5 = gi.soundindex ("jawa/j_talk5.wav");
	sound_talk6 = gi.soundindex ("jawa/j_talk6.wav");
	sound_work = gi.soundindex ("jawa/j_work.wav");

	self->s.skinnum = 0;
	self->health = 30+(random() * 15);
	self->max_health = self->health;
	self->gib_health = -40;

	self->s.modelindex = gi.modelindex ("models/monsters/jawa/trader.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->yaw_speed = 45;

	self->mass = 50;

	self->pain = jawa3_pain;
	self->die = jawa3_die;

	self->monsterinfo.stand = jawa3_stand;
	self->monsterinfo.walk = jawa3_walk;
	self->monsterinfo.run = jawa3_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = jawa3_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = jawa3_sight;

	self->monsterinfo.aiflags |= AI_GOOD_GUY;

//	if(!self->message)
//		self->message = "defaultinv";
//	else
//	if(self->message)
//		Jawa_LoadInventory (self, self->message);

	gi.linkentity (self);

	self->monsterinfo.stand (self);

	walkmonster_start (self);
}

