/*
Copyright (C) 1997-2001 Id Software, Inc.

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

#include "client.h"

image_t		*i_conback;
//image_t		*i_inventory;
image_t		*i_net;
image_t		*i_pause;
image_t		*i_loading;
image_t		*i_backtile;
// jitmenu:
image_t		*i_slider1l;
image_t		*i_slider1r;
image_t		*i_slider1lh;
image_t		*i_slider1rh;
image_t		*i_slider1ls;
image_t		*i_slider1rs;
image_t		*i_slider1t;
image_t		*i_slider1th;
image_t		*i_slider1b;
image_t		*i_slider1bh;
image_t		*i_slider1bs;
image_t		*i_checkbox1u;
image_t		*i_checkbox1uh;
image_t		*i_checkbox1us;
image_t		*i_checkbox1c;
image_t		*i_checkbox1ch;
image_t		*i_checkbox1cs;
image_t		*i_field1l;
image_t		*i_field1lh;
image_t		*i_field1ls;
image_t		*i_field1m;
image_t		*i_field1mh;
image_t		*i_field1ms;
image_t		*i_field1r;
image_t		*i_field1rh;
image_t		*i_field1rs;
image_t		*i_cursor;
image_t		*i_cursor_text;

bordered_pic_data_t		bpdata_popup1;
bordered_pic_data_t		bpdata_button1;
bordered_pic_data_t		bpdata_button1_hover;
bordered_pic_data_t		bpdata_button1_select;

hash_table_t			g_bpic_hash;

void CL_LoadBorderedPic (bordered_pic_data_t *bpdata, const char *name)
{
	if (bpdata)
	{
		char filename[MAX_QPATH];
		char *filedata;
		float xoffset = 0.0f, yoffset = 0.0f;

		Com_sprintf(filename, sizeof(filename), "pics/%s.bpic", name);

		if (FS_LoadFileZ(filename, (void **)&filedata) > 0)
		{
			char *buf = filedata;
			char *token = COM_Parse(&buf);
			int index = 0;
			int style;
			float imageWidth = 100.0f, imageHeight = 100.0f;
			float xbase = 0.0f, ybase = 0.0f;

			if (Q_strcasecmp(token, "dpborderedpic1.0"))
			{
				Com_Printf("First line of %s should be \"dpborderedpic1.0\"\n", filename);
				goto FreeBorderPic;
			}

			token = COM_Parse(&buf);

			while (token && *token)
			{
				if (Q_strcaseeq(token, "style"))
				{
					token = COM_Parse(&buf);
					style = atoi(token);
					
					if (style != 2)
					{
						Com_Printf("Style %s not supported (only 2 supported now) in %s\n", token, filename);
						goto FreeBorderPic;
					}
				}
				else if (Q_strcaseeq(token, "image") || Q_strcaseeq(token, "pic"))
				{
					token = COM_Parse(&buf);
					bpdata->image = re.DrawFindPic(token);

					if (bpdata->image)
					{
						imageWidth = bpdata->image->width;
						imageHeight = bpdata->image->height;
					}
				}
				else if (Q_strcaseeq(token, "xoffset"))
				{
					token = COM_Parse(&buf);
					xoffset = (float)atof(token);
				}
				else if (Q_strcaseeq(token, "yoffset"))
				{
					token = COM_Parse(&buf);
					yoffset = (float)atof(token);
				}
				else if (Q_strcaseeq(token, "import"))
				{
					token = COM_Parse(&buf);
					Com_sprintf(filename, sizeof(filename), "pics/%s.bpic", token);
					FS_FreeFile(filedata);

					if (FS_LoadFileZ(filename, (void **)&filedata) <= 0)
					{
						Com_Printf("Could not load imported bpic: %s\n", filename);
						return;
					}

					buf = filedata;
					token = COM_Parse(&buf);

					if (Q_strcasecmp(token, "dpborderedpic1.0"))
					{
						Com_Printf("First line of imported %s should be \"dpborderedpic1.0\"\n", filename);
						goto FreeBorderPic;
					}
				}
				else
				{
					const char *s = strchr(token, ',');

					if (s)
					{
						if (index < BORDERED_PIC_COORD_COUNT)
						{
							int subindex;
							s = token;

							for (subindex = 0; subindex < 4 && s > (const char *)1; ++subindex)
							{
								float f = (float)atof(s);
								float divideBy = imageWidth;
								float base = xbase;

								// Evens are X coords, odds are Y coords, so divide by witdh and height accordingly to get a texture coord from 0 to 1.
								if (subindex & 1)
								{
									divideBy = imageHeight;
									base = ybase;
									f += yoffset;
								}
								else
								{
									f += xoffset;
								}

								// Use the first entry as the base offset for everything else (in case we don't start at 0,0)
								if (index == 0)
								{
									if (subindex == 0)
										base = xbase = f;
									else if (subindex == 1)
										base = ybase = f;
								}

								bpdata->screencoords[index][subindex] = f - base;
								bpdata->texcoords[index][subindex] = f / divideBy;
								s = strchr(s, ',') + 1;
							}
							
							++index;
						}
						else
						{
							Com_Printf("More than %d sets of coordinates in %s", BORDERED_PIC_COORD_COUNT, filename);
							goto FreeBorderPic;
						}
					}
					else
					{
						Com_Printf("Unknown line %s in %s\n", token, filename);
					}
				}

				token = COM_Parse(&buf);
			}

FreeBorderPic:
			FS_FreeFile(filedata);
		}
	}
}

void CL_InitImages (void)
{
	i_conback = re.DrawFindPic("conback");
	//i_inventory = re.DrawFindPic("inventory");
	i_net = re.DrawFindPic("net");
	i_pause = re.DrawFindPic("pause");
	i_loading = re.DrawFindPic("loading");
	i_backtile = re.DrawFindPic("backtile");

	// jitmenu:
	i_slider1l = re.DrawFindPic("slider1l");
	i_slider1lh = re.DrawFindPic("slider1lh");
	i_slider1ls = re.DrawFindPic("slider1ls");
	i_slider1r = re.DrawFindPic("slider1r");
	i_slider1rh = re.DrawFindPic("slider1rh");
	i_slider1rs = re.DrawFindPic("slider1rs");
	i_slider1t = re.DrawFindPic("slider1t");
	i_slider1th = re.DrawFindPic("slider1th");
	i_slider1b = re.DrawFindPic("slider1b");
	i_slider1bh = re.DrawFindPic("slider1bh");
	i_slider1bs = re.DrawFindPic("slider1bs");
	i_checkbox1u = re.DrawFindPic("checkbox1u");
	i_checkbox1uh = re.DrawFindPic("checkbox1uh");
	i_checkbox1us = re.DrawFindPic("checkbox1us");
	i_checkbox1c = re.DrawFindPic("checkbox1c");
	i_checkbox1ch = re.DrawFindPic("checkbox1ch");
	i_checkbox1cs = re.DrawFindPic("checkbox1cs");
	i_field1l  = re.DrawFindPic("field1l");
	i_field1lh = re.DrawFindPic("field1lh");
	i_field1ls = re.DrawFindPic("field1ls");
	i_field1m  = re.DrawFindPic("field1m");
	i_field1mh = re.DrawFindPic("field1mh");
	i_field1ms = re.DrawFindPic("field1ms");
	i_field1r  = re.DrawFindPic("field1r");
	i_field1rh = re.DrawFindPic("field1rh");
	i_field1rs = re.DrawFindPic("field1rs");

	i_cursor = re.DrawFindPic("cursor");
	i_cursor_text = re.DrawFindPic("cursor_text");

	// jit - borderpic stuff:
	CL_LoadBorderedPic(&bpdata_popup1, "popup1");
	CL_LoadBorderedPic(&bpdata_button1, "button1");
	CL_LoadBorderedPic(&bpdata_button1_hover, "button1_hover");
	CL_LoadBorderedPic(&bpdata_button1_select, "button1_select");

	// This function is called every time there is a vid_restart, so we need to make sure any already-loaded bpics get freed up.
	hash_table_free(&g_bpic_hash);
	hash_table_init(&g_bpic_hash, 0x20, Z_Free);
}


void CL_ShutdownImages (void)
{
	hash_table_free(&g_bpic_hash);
}


bordered_pic_data_t *CL_FindBPic (const char *name)
{
	if (name)
	{
		bordered_pic_data_t *bpdata = hash_get(&g_bpic_hash, name);

		if (!bpdata)
		{
			bpdata = Z_Malloc(sizeof(bordered_pic_data_t));

			if (bpdata)
			{
				CL_LoadBorderedPic(bpdata, name);
				hash_add(&g_bpic_hash, name, bpdata);
			}
		}

		return bpdata;
	}

	return NULL;
}
