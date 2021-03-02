/*
Copyright (C) 2003-2004 Nathan Wulf (jitspoe)

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

// === jitmenu / jitscores
// encode_bin_str.c -- Functions to encode/decode binary data stored as a string

#include "client.h"

#define MARKER_CHAR 255
#define MARKER_CHAR_0 1
#define MARKER_CHAR_SHORT 254
#define MARKER_CHAR_LONG 253

#define item_from_index(a) cl.configstrings[CS_ITEMS+(a)]

static unsigned char *outpos;
static const unsigned char *decode_ptr;
static unsigned int decode_char_unsigned (void);

static void encode_addmarker (const unsigned char marker)
{
	*outpos = MARKER_CHAR;
	outpos++;
	*outpos = marker;
	outpos++;
}

static void encode_addchar (unsigned int c)
{
	c &= 255;

	if (c == 0)
	{
		*outpos = MARKER_CHAR;
		outpos++;
		*outpos = MARKER_CHAR_0;
	}
	else if (c == MARKER_CHAR)
	{
		*outpos = MARKER_CHAR;
		outpos++;
		*outpos = MARKER_CHAR;
	}
	else
	{
		*outpos = c;
	}

	outpos++;
}

static void encode_end (void)
{
	*outpos = '\0';
}

static unsigned int decode_short_unsigned (void)
{
	return (decode_char_unsigned() << 8) | decode_char_unsigned();
}

static unsigned int decode_long_unsigned (void)
{
	return (decode_short_unsigned() << 16) | decode_short_unsigned();
}

static unsigned int decode_char_unsigned (void)
{
	unsigned char c;
	
	c = *decode_ptr;
	decode_ptr++;

	if (c == MARKER_CHAR)
	{
		c = *decode_ptr;
		decode_ptr++;

		switch (c)
		{
			case MARKER_CHAR:
				return MARKER_CHAR;
				break;
			case MARKER_CHAR_0:
				return 0;
				break;
			case MARKER_CHAR_SHORT:
				return decode_short_unsigned();
				break;
			case MARKER_CHAR_LONG:
				return decode_long_unsigned();
				break;
			default:
				Com_Printf("ERROR: Bad encode!\n");
				return 0;
				break;
		}
	}
	else
	{
		return c;
	}
}

void encode_unsigned (unsigned int count, unsigned int *in, unsigned char *out)
{
	unsigned int i, out_i = 0;
	unsigned int j;
	unsigned char c;
	unsigned short s;

	outpos = out;

	for (i = 0; i < count; i++)
	{
		j = in[i] + 1; // since 0 requires an extra char and is common, we want to avoid it.
		s = (short)j;
		c = (char)j;

		if (c == j) // j < 256, just need 1 char to store it.
		{
			encode_addchar(c);
		}
		else if (s == j) // can fit in 2 bytes (short)
		{
			encode_addmarker(MARKER_CHAR_SHORT);
			encode_addchar(s>>8);
			encode_addchar(s);
		}
		else // full 32bit integer (4 bytes)
		{
			encode_addmarker(MARKER_CHAR_LONG);
			encode_addchar(j>>24);
			encode_addchar(j>>16);
			encode_addchar(j>>8);
			encode_addchar(j);
		}
	}
	encode_end();
}

// in: string encoded with encode_unsigned
// out: integer array
// max: max number of elements in int array.
// return: number of integers decoded
int decode_unsigned (const unsigned char *in, unsigned int *out, int max)
{
	int i = 0;

	if (!in || !out || !max)
		return 0;

	decode_ptr = in;

	while (*decode_ptr && i < max)
	{
		out[i] = decode_char_unsigned() - 1; // - 1 is to compensate for 1 added in encode_unsigned
		i++;
	}

	return i;
}


#define MAX_DECODE_ARRAY 128
static int index_array[MAX_DECODE_ARRAY];
static int num_elements;
static int current_element;

int format_event_string (char *out_str, int max_len, const char *in_str)
{
	char *out_str_start = out_str;
	const char *in_str_start = in_str;
	int temp_len;

	// Translate string...
	// %s = index to event string
	// %i = index to item string
	// %n = index to name string
	// %t = index to name string, inlclude team splat.
	// %% = %
	// %c = single charcter (like in printf)
	// %d = decimal number
	while (*in_str)
	{
		while (*in_str && *in_str != '%' && max_len > 1)
		{
			*out_str = *in_str;
			in_str++;
			out_str++;
			max_len--;
		}

		if (*in_str)
		{
			in_str++;

			if (current_element < num_elements)
			{
				switch(*in_str)
				{
				case 's': // event string
					max_len -= format_event_string(out_str, max_len, cl.configstrings[CS_EVENTS + index_array[current_element++]]);
					break;
				case 'i': // item
					Q_strncpyz(out_str, item_from_index(index_array[current_element++]), max_len);
					temp_len = strlen(out_str);
					out_str += temp_len;
					max_len -= temp_len;
					break;
				case 't': // team splat + name
					*out_str++ = cl_scores_get_team_splat(index_array[current_element]);
					max_len--;
					// 'n' MUST follow this:
				case 'n': // name
					Q_strncpyz(out_str, name_from_index(index_array[current_element++]), max_len);
					temp_len = strlen(out_str);
					out_str += temp_len;
					max_len -= temp_len;
					break;
				case '%': // %
					*out_str = '%';
					out_str++;
					max_len--;
					break;
				case 'c': // character
					*out_str = index_array[current_element++];
					out_str++;
					max_len--;
					break;
				case 'd': // decimal
					Com_sprintf(out_str, max_len, "%d", index_array[current_element++]);
					temp_len = strlen(out_str);
					out_str += temp_len;
					max_len -= temp_len;
					break;
				default: // unknown? (shouldn't happen)
					*out_str = '?';
					out_str++;
					max_len--;
					break;
				}

				in_str++;
			}
		}
	}

	*out_str = 0; // terminate string
	assert(max_len > 1);
	temp_len = out_str - out_str_start;

	return temp_len;
}

#define MAX_EVENT_STRINGS 8
static char event_strings[MAX_EVENT_STRINGS][MAX_QPATH]; // displayed briefly just above hud
static int event_string_time[MAX_EVENT_STRINGS];
static int startpos = 0;

static void event_print (char *s)
{
	startpos--;

	if (startpos<0)
		startpos = MAX_EVENT_STRINGS - 1;

	strcpy(event_strings[startpos], s);
	event_string_time[startpos] = curtime;
}

void CL_DrawEventStrings (void)
{
	int i, j;
	float alpha;

	for (i=0, j=startpos; i<MAX_EVENT_STRINGS; i++, j++, j%=MAX_EVENT_STRINGS)
	{
		if (*event_strings[j])
		{
			alpha = (4000 - (curtime-event_string_time[j])) / 3000.0f;

			if (alpha > 1.0f)
				alpha = 1.0f;

			if (alpha < 0.05f)
			{
				event_strings[j][0] = '\0';
				break;
			}

			re.DrawStringAlpha(viddef.width/2-4*hudscale*strlen_noformat(event_strings[j]),
				viddef.height/2+(32+i*8)*hudscale, event_strings[j], alpha);
		}
		else
		{
			break;
		}
	}

	// set transparency back to normal so other stuff renders correctly
	re.DrawStringAlpha(0, 0, "", 1.0f);
}


extern cvar_t *cl_timestamp;
extern char timestamp[24];
void CL_ParsePrintEvent (const char *str) // jitevents
{
	unsigned int event;
	char event_text[128];

	num_elements = decode_unsigned(str, index_array, MAX_DECODE_ARRAY);

	if (num_elements < 1)
		return;

	event = index_array[0];

	if (num_elements > 1)
	{
		current_element = 2;
		format_event_string(event_text, sizeof(event_text), cl.configstrings[CS_EVENTS + index_array[1]]);
	}
	else
	{
		strcpy(event_text, "(null)");
	}

	if (cl_timestamp->value)
		Com_Printf("[%s] %s\n", timestamp, event_text);
	else
		Com_Printf("%s\n", event_text);
	
	// handle special cases of events here...
	switch(event)
	{
	case EVENT_ENTER:
		if (num_elements > 2)
		{
			cl_scores_clear(index_array[2]);
			cl_scores_setinuse(index_array[2], true);

			if (num_elements > current_element)
				cl_scores_setteam(index_array[2], (char)index_array[current_element++]);

			if (num_elements > current_element)
				cl_scores_setstarttime(index_array[2], index_array[current_element++]);
		}

//		if ((g_autorecord->value == 1 && nopassword) || g_autorecord->value != 2)		// T3RR0R15T - client side autodemo --> todo: this line (get values + only on own enter event)
//		{
//			switch ((int)(cl_autorecord->value))
//			{
//			case 1:
//				Cbuf_AddText("record cl_auto\n");
//				break;
//			case 2:
//				Cbuf_AddText("arecord cl_auto\n");
//				break;
//			default:
//				break;
//			}
//		}

		S_StartLocalSound("misc/enter.wav");

		break;
	case EVENT_DISCONNECT:
		if (num_elements > 2)
			cl_scores_clear(index_array[2]);

		S_StartLocalSound("misc/disconnect.wav");
		break;
	case EVENT_JOIN:
		if (num_elements > 3)
			cl_scores_setteam(index_array[2], (char)index_array[3]);
		break;
	case EVENT_ROUNDOVER:
		event_print(event_text);
		break;
	case EVENT_OVERTIME:
		event_print(event_text);
		break;
	case EVENT_ROUNDSTART:
		cl_scores_setisalive_all(true);
		cl_scores_sethasflag_all(false);
		break;
	case EVENT_ADMINKILL: // jitodo - fix all these offsets
		if (num_elements > 2 && index_array[2] == cl.playernum)
		{
			if (num_elements > 3)
				Com_sprintf(event_text, sizeof(event_text), "Admin (%s) killed you.", name_from_index(index_array[3]));
			else
				strcpy(event_text, "Admin killed you.");

			event_print(event_text);
		}
		break;
	case EVENT_KILL: // jitodo - fix all these offsets
		// Update scoreboard:
		if (current_element < num_elements)
			cl_scores_setkills(index_array[2], index_array[current_element++]);

		if (current_element < num_elements)
		{
			cl_scores_setdeaths(index_array[4], index_array[current_element++]);
			cl_scores_setisalive(index_array[4], false);
		}

		if (num_elements < 8)
			break;
		
		if (index_array[2] == cl.playernum)
		{
			Com_sprintf(event_text, sizeof(event_text), "You eliminated %s (%s).",
				name_from_index(index_array[4]), item_from_index(index_array[5]));
			Stats_AddEvent(STATS_KILL);
		}
		else if (index_array[4] == cl.playernum)
		{
			Com_sprintf(event_text, sizeof(event_text), "%s (%s) eliminated you.",
				name_from_index(index_array[2]), item_from_index(index_array[3]));
			Stats_AddEvent(STATS_DEATH);
		}
		else
			break;

		if (cl_centerprintkills->value)
			event_print(event_text);

		break;
	case EVENT_SUICIDE: // jitodo - fix all these offsets
		if (num_elements < 3)
			break;

		// Scoreboard:
		if (current_element < num_elements)
			cl_scores_setdeaths(index_array[2], index_array[current_element++]);

		cl_scores_setisalive(index_array[2], false);

		if (index_array[2] == cl.playernum)
		{
			Com_sprintf(event_text, sizeof(event_text), "You eliminated yourself!");

			if (cl_centerprintkills->value)
				event_print(event_text);
		}

		break;
	case EVENT_FFIRE:
		if (num_elements < 4)
			break;

		cl_scores_setisalive(index_array[3], false);

		if (index_array[2] == cl.playernum)
			Com_sprintf(event_text, sizeof(event_text), "%cYou eliminated your teammate!", SCHAR_ITALICS);
		else if (index_array[3] == cl.playernum)
			Com_sprintf(event_text, sizeof(event_text), "Your teammate (%s) eliminated you.", name_from_index(index_array[2]));
		else
			break;

		event_print(event_text);
		break;
	case EVENT_RESPAWN:
		if (num_elements > 2)
			cl_scores_setisalive(index_array[2], true);
		break;
	case EVENT_GRAB:
		if (num_elements < 2)
			break;

		cl_scores_sethasflag(index_array[2], true);
		if (current_element < num_elements)
		{
			cl_scores_setgrabs(index_array[2], index_array[current_element++]);
			if (index_array[2] == cl.playernum)
				Stats_AddEvent(STATS_GRAB);
		}
		break;
	case EVENT_DROPFLAG:
		if (num_elements > 2)
			cl_scores_sethasflag(index_array[2], false);
		break;
	case EVENT_CAP:
		if (num_elements < 2)
			break;

		if (index_array[2] == cl.playernum)
			Stats_AddEvent(STATS_CAP);

		cl_scores_sethasflag(index_array[2], false);
		if (current_element < num_elements)
			cl_scores_setcaps(index_array[2], index_array[current_element++]);
		break;
	default:
		break;
	}
}

