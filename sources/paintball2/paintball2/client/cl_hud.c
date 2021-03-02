/*
Copyright (C) 2004 Nathan 'jitspoe' Wulf / Digital Paint

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// cl_hud.c -- client-side HUD (heads-up display)

#include "client.h"
void SCR_DrawField (int x, int y, int color, int width, int value);


#define MASK_X		viddef.width/2 - (160 - (116))*hudscale
#define MASK_Y		viddef.height + (-25) * hudscale
#define CO2_12G_X	viddef.width/2 - (160 - (1))*hudscale
#define CO2_12G_Y	viddef.height + (-32) * hudscale
#define CO2BAR_X	viddef.width/2 - (160 - (0))*hudscale
#define CO2BAR_Y	viddef.height + (-10) * hudscale
#define GUNTEMP_Y	viddef.height + (-5) * hudscale
#define BARREL_Y	viddef.height + (-25) * hudscale
#define BARREL_X	viddef.width/2 - (160 - (140))*hudscale
#define ALIVE_Y		viddef.height + (-8*teamcount-4*(2/teamcount)) * hudscale
#define ALIVE_X		viddef.width/2 - (160 - (246))*hudscale
#define TEAMICON_X	viddef.width + (-25)*hudscale;
#define TEAMSCORE_X	viddef.width + (-80)*hudscale;
#define TEAMICON_Y	viddef.height + (-60)*hudscale;
#define KILLS_X		viddef.width + (-50)*hudscale;
#define KILLS_Y		(1)*hudscale;
#define ID_X		viddef.width/2
#define ID_Y		viddef.height/2 + (-32) * hudscale
#define TIMELEFT_X	viddef.width - (43)*hudscale;
#define TIMELEFT_Y	viddef.height - (28)*hudscale;

void SCR_DrawPaintballHUD1 (void) // todo version check
{
	int pic, value, x, x2, y;
	char *picname;
	static int teamcount = 2;
	static int lastidtime;
	static char id_name[64];
	int newcount = 0;
	char teamcolor = 'w';
	qboolean has_co2 = false;

	// HUD Mask image:
	if (pic = cl.frame.playerstate.stats[STAT_PB_MASK])
	{
		picname = cl.configstrings[CS_IMAGES + pic];
		re.DrawPic(MASK_X, MASK_Y, picname);
		
		// Only draw the other hud stuff if the player
		// isn't observing:
		if (picname && picname[5] != 'w') // !mask_w
		{
			teamcolor = picname[5];

			x = CO2_12G_X;
			y = CO2_12G_Y;

			// 12g CO2
			if (pic = cl.frame.playerstate.stats[STAT_CO2_12G])
			{
				has_co2 = true;
				re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
				x += 11 * hudscale;
			}
			
			// 7oz CO2
			if (pic = cl.frame.playerstate.stats[STAT_CO2_7OZ])
			{
				has_co2 = true;
				re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
				x += 13 * hudscale;
			}
			
			// 12oz CO2
			if (pic = cl.frame.playerstate.stats[STAT_CO2_12OZ])
			{
				has_co2 = true;
				re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
				x += 13 * hudscale;
			}

			// 20oz CO2
			if (pic = cl.frame.playerstate.stats[STAT_CO2_20OZ])
			{
				has_co2 = true;
				re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
				x += 13 * hudscale;
			}

			// CO2 bar
			x = CO2BAR_X;

			if (pic = cl.frame.playerstate.stats[STAT_CO2BAR])
				re.DrawPic(x, CO2BAR_Y, cl.configstrings[CS_IMAGES + pic]);

			// Gun temperature bar
			if (value = cl.frame.playerstate.stats[STAT_GUNTEMP])
			{
				static int oldvalue = 0;

				value = (value + oldvalue) / 2;
				if (value > 100)
					value = 100;

				y = GUNTEMP_Y;
				x -= 1;

				if (value > 98)
					re.DrawStretchPic(x+2*hudscale, y+hudscale,
						value*110*hudscale/100, 2*hudscale, "guntemp_flash");
				else
					re.DrawStretchPic(x+2*hudscale, y+hudscale,
						value*110*hudscale/100, 2*hudscale, "guntemp_fill");

				re.DrawStretchPic(x, y, 128*hudscale, 4*hudscale, "guntemp_border");
				oldvalue = value;
			}

			// Barrel
			y = BARREL_Y;
			x = BARREL_X;

			if (pic = cl.frame.playerstate.stats[STAT_PB_BARREL])
				re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);

			// Hopper
			if (pic = cl.frame.playerstate.stats[STAT_PB_HOPPER])
				re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);

			// Paintball ammo count
			x += hudscale*(22);

			if ((value = cl.frame.playerstate.stats[STAT_PB_PAINTCOUNT]) || has_co2)
				SCR_DrawField(x, y, 0, 3, value);

			// Grenade
			if (pic = cl.frame.playerstate.stats[STAT_PB_GRENADE])
			{
				x += hudscale*(54);
				re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
			}

			if (pic = cl.frame.playerstate.stats[STAT_OLD_PB_SMOKEGREN]) // jitodo -- check server version here.
			{
				x += hudscale*(54);
				re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
			}
		}
	}

	// Player counts:
	x = ALIVE_X;
	y = ALIVE_Y;

	if (pic = cl.frame.playerstate.stats[STAT_ALIVE_TEAM4])
	{
		re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
		y += 8*hudscale;
		newcount++;
	}

	if (pic = cl.frame.playerstate.stats[STAT_ALIVE_TEAM3])
	{
		re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
		y += 8*hudscale;
		newcount++;
	}

	if (pic = cl.frame.playerstate.stats[STAT_ALIVE_TEAM2])
	{
		re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
		y += 8*hudscale;
		newcount++;
	}

	if (pic = cl.frame.playerstate.stats[STAT_ALIVE_TEAM1])
	{
		re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
		y += 8*hudscale;
		newcount++;
	}

	if (newcount > 0)
		teamcount = newcount;

	// Team scores
	x = TEAMICON_X;
	x2 = TEAMSCORE_X;
	y = TEAMICON_Y;
	if (pic = cl.frame.playerstate.stats[STAT_PB_TEAM1])
	{
		re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
		SCR_DrawField(x2, y, 0, 3, cl.frame.playerstate.stats[STAT_SCORE_TEAM1]);
		y -= (30*hudscale);
	}

	if (pic = cl.frame.playerstate.stats[STAT_PB_TEAM2])
	{
		re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
		SCR_DrawField(x2, y, 0, 3, cl.frame.playerstate.stats[STAT_SCORE_TEAM2]);
		y -= (30*hudscale);
	}

	if (pic = cl.frame.playerstate.stats[STAT_PB_TEAM3])
	{
		re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
		SCR_DrawField(x2, y, 0, 3, cl.frame.playerstate.stats[STAT_SCORE_TEAM3]);
		y -= (30*hudscale);
	}

	if (pic = cl.frame.playerstate.stats[STAT_PB_TEAM4])
	{
		re.DrawPic(x, y, cl.configstrings[CS_IMAGES + pic]);
		SCR_DrawField(x2, y, 0, 3, cl.frame.playerstate.stats[STAT_SCORE_TEAM4]);
		y -= (30*hudscale);
	}

	// Kills / streak
	if ((value = cl.frame.playerstate.stats[STAT_FRAGS]) || teamcolor != 'w')
	{
		x = KILLS_X;
		y = KILLS_Y;
		re.DrawString(x, y, "Kills");
		SCR_DrawField(x-11*hudscale, y+8*hudscale, 0, 3, cl.frame.playerstate.stats[STAT_FRAGS]);

		re.DrawString(x, y+30*hudscale, "Streak");
		SCR_DrawField(x-11*hudscale, y+38*hudscale, 0, 3, cl.frame.playerstate.stats[STAT_STREAK]);
	}

	// Player ID
  	if (lastidtime > cl.time) // map has changed
  	{
  		lastidtime = cl.time - 500;
  	}

	if ((value = cl.frame.playerstate.stats[STAT_ID]))
	{
		if (value >= 0 && value < MAX_CONFIGSTRINGS)
		{
			char *s;
			int i;
			
			for (i=0, s=cl.configstrings[value]; i<32 && *s != '\\'; i++, s++)
				id_name[i] = *s; // copy just the name out of the string

			id_name[i] = 0; // terminate string
			re.DrawString(ID_X - i*4*hudscale, ID_Y, id_name);
			lastidtime = cl.time;
		}
	}
	else if (cl.time - lastidtime < 500) // 1/2 second fade
	{
		re.DrawStringAlpha(ID_X - strlen(id_name)*4*hudscale, ID_Y, id_name, (500 - cl.time + lastidtime)/500.0f);
	}

	// Time remaining
	if ((value = cl.frame.playerstate.stats[STAT_TIMELEFT]))
	{
		x = TIMELEFT_X;
		y = TIMELEFT_Y;
		re.DrawString(x, y, va("%c%cTime:", SCHAR_COLOR, ']'));
		
		if (value >= 0 && value < MAX_CONFIGSTRINGS)
			re.DrawString(x, y+8*hudscale, cl.configstrings[value]);
	}
}


void SCR_DrawHUD (void)
{
	int hudstyle;

#ifdef QUAKE2
	hudstyle = Cvar_Get("cl_hudstyle", "0", CVAR_ARCHIVE)->value;
#else
	hudstyle = Cvar_Get("cl_hudstyle", "1", CVAR_ARCHIVE)->value;
#endif

	switch(hudstyle)
	{
	case 0: // original style.
        SCR_ExecuteLayoutString(cl.configstrings[CS_STATUSBAR]);
		break;
	case 1:
	default:
		SCR_DrawPaintballHUD1();
		break;
	}
}