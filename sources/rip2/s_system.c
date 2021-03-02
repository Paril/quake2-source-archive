#include "g_local.h"

void LoadMapCfg (void)
{
	char *l_var2;

    // first we make sure that the .ini file is open
    if (ini_file.ini_file_read)
	{
		 gi.dprintf ("%s.ini loaded.\n", level.mapname);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Name");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
		 {
             gi.configstring (CS_NAME, l_var2);
			 strncpy (level.level_name, l_var2, sizeof(level.level_name));
		 }

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Nextmap");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
             strncpy (level.nextmap, l_var2, sizeof(level.nextmap));

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Sky");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
		 {
 		     gi.configstring (CS_SKY, l_var2);
			 gi.cvar_set ("sky", l_var2);
		 }

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Dmflags");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
	         gi.cvar_set ("dmflags", l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Ripflags");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
			 gi.cvar_set ("tflags", l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Timelimit");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
			 gi.cvar_set ("timelimit", l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Fraglimit");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
			 gi.cvar_set ("fraglimit", l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Gravity");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
			 gi.cvar_set ("sv_gravity", l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Max_Velocity");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
			 gi.cvar_set ("sv_maxvelocity", l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Capture_Message");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 strncpy (level.capture_message, l_var2, sizeof(level.capture_message));

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Take_Message");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 strncpy (level.take_message, l_var2, sizeof(level.take_message));

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Capturer_Message");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 strncpy (level.capturer_message, l_var2, sizeof(level.capturer_message));

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Capturelimit");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 gi.cvar_set ("capturelimit", l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Extratime");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 gi.cvar_set ("extratime", l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Next_Maxclients");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 gi.cvar_set ("maxclients", l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Next_Eject");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 gi.cvar_set ("eject_life", l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Next_Bhole_Life");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 gi.cvar_set ("bhole_life", l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Maxspectators");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 gi.cvar_set ("maxspectators", l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Taker_Message");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 strncpy (level.taker_message, l_var2, sizeof(level.taker_message));

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Return_Message");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 strncpy (level.return_message, l_var2, sizeof(level.return_message));

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Flag_At_Base");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 strncpy (level.flag_at_base, l_var2, sizeof(level.flag_at_base));

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Flag_Own");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 strncpy (level.flag_own, l_var2, sizeof(level.flag_own));

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Flag_Lying");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 strncpy (level.flag_lying, l_var2, sizeof(level.flag_lying));

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Flag_Corrupt");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
          	 strncpy (level.flag_corrupt, l_var2, sizeof(level.flag_corrupt));
		 
         l_var2 = Ini_GetValue(&ini_file, "Parms", "Take_Frags");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
     		 level.FLAG_TAKE_BONUS = atoi(l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Capture_Frags");

	     // check that what was returned wasn't NULL
         if (l_var2 != NULL)
     		 level.FLAG_CAPTURE_BONUS = atoi(l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Recovery_Frags");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
     		 level.FLAG_RECOVERY_BONUS = atoi(l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Team_Frags");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
     		 level.FLAG_TEAM_BONUS = atoi(l_var2);

         l_var2 = Ini_GetValue(&ini_file, "Parms", "Maphelp");

         // check that what was returned wasn't NULL
         if (l_var2 != NULL)
             strncpy (level.goal, l_var2, sizeof(level.goal));
    }
	else
		gi.dprintf ("Couldn't find file %s.ini.\n", level.mapname);
}