/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_scriptio.c
  Description: .DAT script loader. Script manager
               /Memory manager 

\**********************************************************/

#include "g_local.h"
#include "b_script.h"

int script_exists(edict_t *ent);

void load_script(edict_t *ent)
{
	FILE	*f;
   int commands = 0, i, cmd, intercepts = 0;
	char st1, st2;
	int scr_size = 0;
   script_t script, *scr;
   if (ent->scriptfile)
      {
      if (script_exists(ent))
         return;
      }
   else
      return;

   memset (&script_file[script_index], 0, sizeof(script_file[script_index]));
	f = fopen (ent->scriptfile, "rb+");
	if (!f)
      {
		gi.error ("Couldn't open script file %s", ent->scriptfile);
      return;
      }
   
   fread(&commands, sizeof(commands), 1, f);
   fread(&intercepts, sizeof(intercepts), 1, f);   
   script_file[script_index].commands = gi.TagMalloc (commands * sizeof(script), TAG_LEVEL);
   script_file[script_index].numcommands = commands;
   script_file[script_index].intercepts = intercepts;
   strcpy(script_file[script_index].filename, ent->scriptfile);
   for (i = 0; i < commands; i++)
      {      
      fread(&script.command, sizeof(script.command), 1, f);
      fread(&script.ParamInt[0], sizeof(script.ParamInt[0]), 2, f);
      fread(&script.ParamFloat[0], sizeof(script.ParamFloat[0]), 2, f);
		fread(&st1, 1, 1, f);
		fread(&st2, 1, 1, f);
		if (st1 > 0)
			{
			script.ParamString[0] = gi.TagMalloc (st1 + 1, TAG_LEVEL);
			memset(script.ParamString[0], 0, st1);
			fread(script.ParamString[0], st1, 1, f);
			}
		else 
			script.ParamString[0] = " ";
		if (st2 > 0)
			{
			script.ParamString[1] = gi.TagMalloc (st2 + 1, TAG_LEVEL);
			memset(script.ParamString[1], 0, st2);
			fread(script.ParamString[1], st2, 1, f);
			}
		else 
			script.ParamString[1] = " ";
      fread(&script.ParamVector, sizeof(script.ParamVector), 1, f);
      memcpy(&script_file[script_index].commands[i], &script, sizeof(script));   
		scr_size += sizeof(script.command) +  sizeof(script.ParamInt[0]) * 2
			+ sizeof(script.ParamFloat[0]) * 2 + st1 + st2 + sizeof(script.ParamVector);
      }
	fclose (f);      
   script_file[script_index].on_see = -1;
   script_file[script_index].on_touch = -1;
   script_file[script_index].on_pain = -1;
   script_file[script_index].on_block = -1;
   for (i = 0; i < commands; i++)
      {
      scr = &script_file[script_index].commands[i];
      cmd = scr->command;
      if (cmd == SCR_ON_BLOCK && script_file[script_index].on_block < 0)
         {   
         script_file[script_index].on_block = scr->ParamInt[1];
         script_file[script_index].block_cmd = i;
         }     
      else if (cmd == SCR_ON_SEE && script_file[script_index].on_see < 0)
         {
         script_file[script_index].on_see = scr->ParamInt[1];
         script_file[script_index].see_cmd = i;
         }
      else if (cmd == SCR_ON_TOUCH && script_file[script_index].on_touch < 0)
         {                                       
         script_file[script_index].on_touch = scr->ParamInt[1];
         script_file[script_index].touch_cmd = i;                     
         }
      else if (cmd == SCR_ON_PAIN && script_file[script_index].on_pain < 0)
         {
         script_file[script_index].on_pain = scr->ParamInt[1];    
         script_file[script_index].pain_cmd = i;
         }
      }
   script_index++;
   if (!script_exists(ent))
      gi.error ("Couldn't load script file %s", ent->scriptfile);
	else
      gi.dprintf ("Script file '%s:%d bytes' added to database\n", ent->scriptfile, scr_size);		
}

int script_exists(edict_t *ent)
{
   int i;
   ent->scr.on_see = -1;
   ent->scr.on_touch = -1;
   ent->scr.on_pain = -1;
   ent->scr.on_block = -1;
   ent->scr.index = -1;
   for (i = 0; i < script_index; i++)
      { 
      if (!strcmpi(script_file[i].filename, ent->scriptfile))
         {
         intercept_t x;
         int j;
         ent->scr.num_intercepts = script_file[i].intercepts;
         ent->scr.ivec = gi.TagMalloc (ent->scr.num_intercepts * sizeof(x), TAG_LEVEL);
         for (j = 0; j < ent->scr.num_intercepts; j++)
            {
            memset(ent->scr.ivec[j].activator, 0, 255);
            ent->scr.ivec[j].index = -1;
            }
         ent->scr.numcommands = script_file[i].numcommands;
         ent->scr.commands = &script_file[i].commands[0];
         ent->scr.on_see = script_file[i].on_see;
         ent->scr.on_touch = script_file[i].on_touch;
         ent->scr.on_pain = script_file[i].on_pain;
         ent->scr.on_block = script_file[i].on_block;
         ent->scr.see_cmd = script_file[i].see_cmd;
         ent->scr.touch_cmd = script_file[i].touch_cmd;
         ent->scr.pain_cmd = script_file[i].pain_cmd;
         ent->scr.block_cmd = script_file[i].block_cmd;
         ent->scr.see_time = level.time + 2 + (rand() % 2);
         ent->scr.save = -1;
         ent->scr.index = 0;
         ent->scr.cmd = ent->scr.commands;
         //ent->scr.current = script_file[i].start;
         return 1;
         }
      }
   return 0;
}
