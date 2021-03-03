/*
Copyright (C) 2003-2013 Nathan Wulf (jitspoe)

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

// ===
// jitmenu

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "menu.h"

#define MAX_MENU_SCREENS 32

// Local globals
static int				m_menudepth	= 0;
static menu_screen_t	*root_menu	= NULL;
static menu_screen_t	*m_menu_screens[MAX_MENU_SCREENS];
static menu_mouse_t		m_mouse; // jitmouse
static float			scale;
static float			oldscale = 0.0f;
static menu_widget_t	*m_active_bind_widget = NULL;
static char				*m_active_bind_command = NULL;
static menu_screen_t	*m_current_menu;
static hash_table_t		named_widgets_hash;
static qboolean			m_initialized = false;
static qboolean			m_vscrollbar_tray_selected = false;
static int				m_menu_sound_flags = 0;
static int				m_stored_menu_depth = 0;
static menu_screen_t	*m_stored_menu_screens[MAX_MENU_SCREENS];
static image_t			*m_temporary_background = NULL;

// Globals
pthread_mutex_t			m_mut_widgets;

extern m_serverlist_t	m_serverlist;

static void M_UpdateDrawingInformation (menu_widget_t *widget);
char *Cmd_MacroExpandString (const char *text);
static void widget_complete (menu_widget_t *widget);
static int listview_compare(char* stra, char* strb, qboolean ascending); //xrichardx: multi column listview compare function for sorting
static void listview_sort(menu_widget_t *widget, int column, qboolean ascending)
{
	int i, j;
	char** buffer;
	char* buffer2;
	
	char** map = widget->listview_source_map;
	char*** list = widget->listview_source_list;
	int start_index = 0;
	int end_index = widget->listview_source_rowcount;

	if(!widget->listview_source)
	{
		start_index = 2;
		end_index = widget->listview_rowcount;
		map = widget->listview_map;
		list = widget->listview_list;
	}

	// TODO: This is currently O(n^2). If at any point we reach a significant number of servers again, we might need to change that, but since we need
	// to keep list and map in the same order here, using qsort is not trivial.
	for (i = start_index; i < end_index - 1; i++)
	{
		for (j = (i + 1); j < end_index; j++)
		{
			if (listview_compare(list[i][column], list[j][column], ascending) > 0)
			{
				buffer = list[i]; //swap the whole lines by swapping the pointers.
				list[i] = list[j];
				list[j] = buffer;

				buffer2 = map[i];
				map[i] = map[j];
				map[j] = buffer2;
			}
		}
	}
}

// same thing as strdup, only uses Z_Malloc - todo: replace with CopyString, which is exactly the same thing.
char *text_copy (const char *in)
{
	return CopyString(in);
}

// these two functions are for the "listsource"
// key for the select widget.
static void list_source (menu_widget_t *widget)
{
	extern char **cl_scores_nums; // jitodo - put these in a header or something
	extern char **cl_scores_info;
	extern int cl_scores_count;
	qboolean cl_scores_prep_select_widget (void);

	if (!widget || !widget->listsource)
		return;

	if (Q_streq(widget->listsource, "serverlist"))
	{
		widget->select_list = m_serverlist.info;
		widget->select_map = m_serverlist.ips;
		widget->select_totalitems = m_serverlist.nummapped;
	}
	else if (Q_streq(widget->listsource, "scores"))
	{
		cl_scores_prep_select_widget();
		widget->select_list = cl_scores_info;
		widget->select_map = cl_scores_nums;
		widget->select_totalitems = cl_scores_count;
	}
	else if (Q_streq(widget->listsource, "maplist"))
	{
		widget->select_list = cl_maplist_info;
		widget->select_map = cl_maplist_names;
		widget->select_totalitems = cl_maplist_count;
		//pthread_mutex_unlock(&m_mut_widgets); // hack to keep mutexes from locking up
		M_RefreshWidget("vote_map_mode", false);// -- todo, might be necessary to force widget refresh?
		//pthread_mutex_lock(&m_mut_widgets); // hack to keep semaphores from locking up
	}
	else if (Q_streq(widget->listsource, "maplist_modes"))
	{
		CL_UpdateMaplistModes(); // todo - does this happen every frame?
		widget->select_list = cl_maplist_modes;
		widget->select_map = cl_maplist_modes;
		widget->select_totalitems = cl_maplist_modes_count;
	}
}

static char*** copy_listview_info(char*** source, int column_count, int row_count)
{
	int row, column;

	char*** copy = Z_Malloc(sizeof(char**) * row_count);

	for(row = 0; row < row_count; ++row) {
		copy[row] = Z_Malloc(sizeof(char*) * column_count);

		for(column = 0; column < column_count; ++column) {
			copy[row][column] = CopyString(source[row][column]);
		}
	}

	return copy;
}

static char** copy_listview_map(char** source, int row_count)
{
	int row;
	char** copy = Z_Malloc(sizeof(char*) * row_count);

	for(row = 0; row < row_count; ++row) {
		copy[row] = CopyString(source[row]);
	}

	return copy;
}

void free_listview_info_entry(char** listview_info_entry, int column_count)
{
	int i;
	for(i = 0; i < column_count; ++i)
	{
		Z_Free(listview_info_entry[i]);
	}

	Z_Free(listview_info_entry);
}

static void free_listview_info(char*** listview_info, int row_count, int column_count)
{
	int row;
	for(row = 0; row < row_count; ++row) {
		free_listview_info_entry(listview_info[row], column_count);
	}
	Z_Free(listview_info);
}

static void free_listview_map(char** listview_map, int row_count)
{
	int row;
	for(row = 0; row < row_count; ++row) {
		Z_Free(listview_map[row]);
	}
	Z_Free(listview_map);
}

static void listview_source (menu_widget_t *widget)
{
	extern char **cl_scores_nums; // jitodo - put these in a header or something
	extern char ***cl_scores_listview_info;
	extern int cl_scores_count;
	extern int cl_scores_listview_column_count;
	qboolean cl_scores_prep_listview_widget (void);

	if (!widget || !widget->listview_source)
		return;

	if(widget->listview_source_list) {
		free_listview_info(widget->listview_source_list, widget->listview_source_rowcount, widget->listview_source_column_count);
		widget->listview_source_list = NULL;
	}

	if(widget->listview_source_map) {
		free_listview_map(widget->listview_source_map, widget->listview_source_rowcount);
		widget->listview_source_map = NULL;
	}

	if (Q_streq(widget->listview_source, "serverlist"))
	{
		widget->listview_source_list = copy_listview_info(m_serverlist.listview_info, m_serverlist.listview_info_column_count, m_serverlist.nummapped);
		widget->listview_source_map = copy_listview_map(m_serverlist.ips, m_serverlist.nummapped);
		widget->listview_source_rowcount = m_serverlist.nummapped;
		widget->listview_source_column_count = m_serverlist.listview_info_column_count;
	}
	else if (Q_streq(widget->listsource, "scores"))
	{
		cl_scores_prep_listview_widget();
		widget->listview_source_list = copy_listview_info(cl_scores_listview_info, cl_scores_listview_column_count, cl_scores_count);
		widget->listview_source_map = copy_listview_map(cl_scores_nums, cl_scores_count);
		widget->listview_source_rowcount = cl_scores_count;
		widget->listview_source_column_count = cl_scores_listview_column_count;
	}
}

static void add_named_widget (menu_widget_t *widget)
{
	hash_add(&named_widgets_hash, widget->name, widget);
}

static void remove_named_widget (menu_widget_t *widget)
{
	hash_delete(&named_widgets_hash, widget->name);
}

static qboolean widget_is_selectable (menu_widget_t *widget)
{
	return (widget->enabled && (widget->cvar || widget->command || widget->callback));
}

static void M_DrawBackground (image_t *background)
{
	char version[64];

	re.DrawStretchPic2(0, 0, viddef.width, viddef.height, background);
	SCR_AddDirtyPoint(0, 0);
	SCR_AddDirtyPoint(viddef.width - 1, viddef.height - 1);
	Com_sprintf(version, sizeof(version), "%c]v%4.2f Alpha (build %d)", SCHAR_COLOR, VERSION, BUILD); // jit 
	re.DrawStringAlpha(viddef.width - 176 * hudscale, viddef.height - 12 * hudscale, version, 1.0f);
}


void M_ForceMenuOff (void)
{
	m_menudepth = 0;
	cls.key_dest = key_game;
	Key_ClearStates();
	Cvar_Set("paused", "0");
	m_active_bind_widget = NULL;
	m_active_bind_command = NULL;

	if (oldscale && (oldscale != cl_hudscale->value))
		Cvar_SetValue("cl_hudscale", oldscale);
}

void M_Menu_Main_f (void)
{
	if (cls.state == ca_active)
		Cbuf_AddText("menu main_ingame\n");
	else
		Cbuf_AddText("menu main\n");
}

/*
   strlen_noformat
   Returns the length of strings ignoring formatting (underlines, colors, etc)
*/
int strlen_noformat (const char *s)
{
	int count = 0;

	if (!s)
		return 0;

	while (*s)
	{
		if (*s != SCHAR_UNDERLINE && *s != SCHAR_ITALICS && *s != SCHAR_ENDFORMAT)
		{
			if (*s == SCHAR_COLOR && *(s+1))
				s++; // skip two characters.
			else
				count++;
		}

		s++;
	}

	return count;
}


// Returns the actual character pos for a given visible position,
// EX: ("##abc", 1) would return 3 if # was a formatting character.
// If the input pos is > than the length of the string, will return the length of the string.
int str_actual_format_pos (const char *s, int visible_pos)
{
	int count = 0;
	const char *s_start = s;

	if (!s)
		return 0;

	while (*s && count < visible_pos)
	{
		if (*s != SCHAR_UNDERLINE && *s != SCHAR_ITALICS && *s != SCHAR_ENDFORMAT)
		{
			if (*s == SCHAR_COLOR && *(s + 1))
				++s; // skip two characters.
			else
				++count;
		}

		++s;
	}

	return s - s_start;
}


// Returns the position visible to the user.
// EX ("##abc", 3) would return 1 if # was a formatting character.
// If the input pos is > than the length of the string, will return the length of the string.
int str_visible_pos (const char *s, int actual_pos)
{
	int visible_pos = 0;
	const char *s_start = s;

	if (!s)
		return 0;

	while (*s && s - s_start < actual_pos)
	{
		if (*s != SCHAR_UNDERLINE && *s != SCHAR_ITALICS && *s != SCHAR_ENDFORMAT)
		{
			if (*s == SCHAR_COLOR && *(s + 1))
				++s; // skip two characters.
			else
				++visible_pos;
		}

		++s;
	}

	return visible_pos;
}


// Returns the position in the string, counting back from a given position ignoring formatting characters
// Returns 0 if the number of characters back is more than the visible length of the string.
int str_visible_pos_back (const char *s, int from_pos, int num_visible_chars_back)
{
	register int num_visible_chars = 0;
	const char *s_start = s;
	s += from_pos;

	while (s > s_start && num_visible_chars < num_visible_chars_back)
	{
		switch (*s)
		{
		case SCHAR_COLOR:
		case SCHAR_UNDERLINE:
		case SCHAR_ENDFORMAT:
		case SCHAR_ITALICS:
			break;
		default:
			// Current character might be a color, so don't count it if the previous character is a color code.
			if (s > s_start && *(s - 1) != SCHAR_COLOR)
			{
				++num_visible_chars;
			}
		}
		--s;
	}

	return s - s_start;
}

static void M_FindKeysForCommand (char *command, int *twokeys)
{
	int		count;
	int		j;
	int		l;
	char	*b;

	twokeys[0] = twokeys[1] = -1;
	l = strlen(command);
	count = 0;

	for (j=0; j<256; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (Q_streq(b, command))
		{
			twokeys[count] = j;
			count++;
			if (count == 2)
				break;
		}
	}
}

void *free_string_array (char *array[], int size)
{
	int i;

	if (array)
	{
		for(i=0; i<size; i++)
			Z_Free(array[i]);

		Z_Free(array);
	}

	return NULL;
}


// free the widget passed as well as all of its child widgets
// and widgets following it in the list, return NULL.
static menu_widget_t *free_widgets (menu_widget_t *widget)
{
	if (!widget)
		return NULL;

	// recursively free widgets in list
	if (widget->next)
		widget->next = free_widgets(widget->next);

	if (widget->subwidget)
		widget->subwidget = free_widgets(widget->subwidget);

	// free text/string stuff:
	if (widget->name)
	{
		remove_named_widget(widget);
		Z_Free(widget->name);
	}

	if (widget->picname)
		Z_Free(widget->picname);

	if (widget->command)
		Z_Free(widget->command);

	if (widget->doubleclick)
		Z_Free(widget->doubleclick);

	if (widget->cvar)
		Z_Free(widget->cvar);

	if (widget->cvar_default)
		Z_Free(widget->cvar_default);

	if (widget->hovertext)
		Z_Free(widget->hovertext);

	if (widget->text)
		Z_Free(widget->text);

	if (widget->selectedtext)
		Z_Free(widget->selectedtext);

	if (widget->listsource)
		Z_Free(widget->listsource);

	// free lists
	if (!(widget->flags & WIDGET_FLAG_LISTSOURCE)) // don't free hardcoded lists!
	{
		if (widget->type == WIDGET_TYPE_SELECT && widget->select_map)
			free_string_array(widget->select_map, widget->select_totalitems);

		if (widget->type == WIDGET_TYPE_SELECT && widget->select_list)
		{
			if (widget->flags & WIDGET_FLAG_FILELIST)
				FS_FreeFileList(widget->select_list, widget->select_totalitems+1);
			else
				free_string_array(widget->select_list, widget->select_totalitems);
		}
		
		if (widget->type == WIDGET_TYPE_LISTVIEW && widget->listview_list) // xrichardx: multicolumn list view
		{
			int i;
			for (i = 0; i < widget->listview_rowcount; i++)
				free_string_array(widget->listview_list[i], widget->listview_columncount);
		}

		if (widget->type == WIDGET_TYPE_LISTVIEW && widget->listview_map && *widget->listview_map)
		{
			if (widget->flags & WIDGET_FLAG_FILELIST)
				FS_FreeFileList(widget->listview_map, widget->listview_rowcount + 1);
			else
				free_string_array(widget->listview_map, widget->listview_rowcount);
		}
	}

	Z_Free(widget);

	return NULL;
}


static menu_widget_t *M_GetNewBlankMenuWidget (void)
{
	menu_widget_t *widget;

	widget = Z_Malloc(sizeof(menu_widget_t));
	memset(widget, 0, sizeof(menu_widget_t));
	widget->enabled = true;
	widget->modified = true;

	return widget;
}


static menu_widget_t *M_GetNewMenuWidget (WIDGET_TYPE type, const char *text, const char *cvar, 
								  const char *cmd, int x, int y, qboolean enabled, qboolean parse_vars)
{
	menu_widget_t *widget;

	widget = M_GetNewBlankMenuWidget();
	widget->type = type;

	if (text)
	{
		if (parse_vars && strchr(text, '$'))
			widget->dynamic = true;

		widget->text = text_copy(text);
	}

	if (cvar)
	{
		if (parse_vars && strchr(cvar, '$'))
			widget->dynamic = true;

		widget->cvar = text_copy(cvar);
	}

	if (cmd)
		widget->command = text_copy(cmd);

	widget->x = x;
	widget->y = y;
	widget->enabled = enabled;
	widget->modified = true;

	return widget;
}


static void callback_vscrollbar_tray (menu_widget_t *widget)
{
	widget->hover = true;
	widget->selected = false;
}


static void callback_vscrollbar_tray_drag (menu_widget_t *widget)
{
	register float pos = ((float)(m_mouse.y - widget->mouseBoundaries.top) + 0.5f) /
		(float)(widget->mouseBoundaries.bottom - widget->mouseBoundaries.top);
	
	widget->parent->scrollbar_pos = (int)(pos * (float)widget->parent->scrollbar_total) -
		widget->parent->scrollbar_visible / 2;

	if (widget->parent->scrollbar_pos < 0)
		widget->parent->scrollbar_pos = 0;

	if (widget->parent->scrollbar_pos > widget->parent->scrollbar_total - widget->parent->scrollbar_visible)
		widget->parent->scrollbar_pos = widget->parent->scrollbar_total - widget->parent->scrollbar_visible;
	
	if (widget->parent->parent)
	{
		switch (widget->parent->parent->type)
		{
		case WIDGET_TYPE_SELECT:
		case WIDGET_TYPE_LISTVIEW:
			if (widget->parent->scrollbar_pos != widget->parent->parent->select_vstart)
			{
				widget->parent->parent->select_vstart = widget->parent->scrollbar_pos;
				widget->parent->parent->modified = true;
			}
			break;
		}
	}

	m_vscrollbar_tray_selected = true;
}


static menu_widget_t *M_CreateScrollbar (WIDGET_TYPE type, int x, int y, int length,
										 int total, int visible, int offset,
										 qboolean selected, qboolean enabled)
{
	menu_widget_t *widget = M_GetNewBlankMenuWidget();
	menu_widget_t *subwidget1, *subwidget2;

	widget->x = x;
	widget->y = y;	
	widget->scrollbar_total = total;
	widget->scrollbar_visible = visible;
	widget->enabled = enabled;

	subwidget1 = M_GetNewBlankMenuWidget();
	subwidget1->selected = selected;
	subwidget2 = M_GetNewBlankMenuWidget();
	subwidget2->x = subwidget1->x = x;
	subwidget2->y = subwidget1->y = y;

	// todo -- support for horizontal scroll type.
	switch (type)
	{
	case WIDGET_TYPE_VSCROLL:
	default:
		widget->type = WIDGET_TYPE_VSCROLL;
		widget->scrollbar_height = length;
		widget->scrollbar_width = SCROLL_ARROW_WIDTH_UNSCALED;
		subwidget2->picwidth = subwidget1->picwidth = SCROLL_ARROW_WIDTH_UNSCALED;
		subwidget1->picheight = length;
		subwidget2->picheight = (int)((float)(length * visible) / (float)total + 0.5f);
		subwidget2->y += (int)((float)(offset * length) / (float)total + 0.5f);
		subwidget1->pic = re.DrawFindPic("scroll1v");
		subwidget1->hoverpic = re.DrawFindPic("scroll1vh");
		subwidget1->selectedpic = re.DrawFindPic("scroll1vs");
		subwidget2->pic = re.DrawFindPic("scroll1vb");
		subwidget2->hoverpic = re.DrawFindPic("scroll1vbs");
		subwidget2->selectedpic = re.DrawFindPic("scroll1vbs");
		subwidget1->callback = callback_vscrollbar_tray;
		subwidget1->callback_drag = callback_vscrollbar_tray_drag;
	}

	subwidget1->next = subwidget2;
	widget->subwidget = subwidget1;
	subwidget1->parent = subwidget2->parent = widget;

	return widget;
}


static void callback_select_item (menu_widget_t *widget)
{
	if (widget->flags & WIDGET_FLAG_BIND)
	{
		m_active_bind_widget = widget;
		m_active_bind_command = widget->parent->select_map[widget->select_pos];
	}
	else
	{
		// update the position of the selected item in the list
		if (widget->parent->select_pos != widget->select_pos)
		{
			widget->parent->modified = true;

			widget->parent->select_pos = widget->select_pos;

			// update the widget's cvar
			if (!(widget->parent->flags & WIDGET_FLAG_NOAPPLY) && widget->parent->cvar)
			{
				if (widget->parent->flags & WIDGET_FLAG_USEMAP)
					Cvar_Set(widget->parent->cvar, widget->parent->select_map[widget->select_pos]);
				else
					Cvar_Set(widget->parent->cvar, widget->parent->select_list[widget->select_pos]);
			}
		}
	}

	if (widget->parent && widget->parent->command)
	{
		Cbuf_AddText(Cmd_MacroExpandString(widget->parent->command));
		Cbuf_AddText("\n");
	}
}


// will also be used as listview doubleclick callback:
static void callback_doubleclick_item (menu_widget_t *widget)
{
	if (widget->parent->doubleclick)
	{
		Cbuf_AddText(Cmd_MacroExpandString(widget->parent->doubleclick));
		Cbuf_AddText("\n");
	}
}

// will also do scrolling for listviews:
static void callback_select_scrollup (menu_widget_t *widget)
{
	if (widget->parent->select_vstart > 0)
	{
		widget->parent->select_vstart--;
		widget->parent->modified = true;
	}
}


static qboolean listview_scrolldown_allowed(menu_widget_t *widget)
{
	if (widget->listview_source)
	{
		//compensate for the two rows that are given otherwise
		if (widget->listview_source_rowcount - widget->listview_vstart - widget->listview_visible_rows > -1)
		{
			return true;
		}
	}
	else
	{
		if (widget->listview_rowcount - widget->listview_vstart - widget->listview_visible_rows > 1)
		{
			return true;
		}
	}
	return false;
}

// will also do scrolling for listviews:
static void callback_select_scrolldown (menu_widget_t *widget)
{
	if (widget->parent->type == WIDGET_TYPE_SELECT
		&& widget->parent->select_totalitems - widget->parent->select_vstart - widget->parent->select_rows > 0)
	{
		widget->parent->select_vstart++;
		widget->parent->modified = true;
	}
	else if (widget->parent->type == WIDGET_TYPE_LISTVIEW && listview_scrolldown_allowed(widget->parent))
	{
		widget->parent->listview_vstart++;
		widget->parent->modified = true;
	}
}

static void callback_listview_item (menu_widget_t *widget)
{
	if (widget->listview_pos == 1)
	{
		int column = widget->listview_pos_x;
		
		if (widget->parent->listview_sortedcolumn == column)
		{
			widget->parent->listview_sortorders[column] =
				(widget->parent->listview_sortorders[column] == LISTVIEW_ASCENDING)
				? LISTVIEW_DESCENDING : LISTVIEW_ASCENDING;
		}
		widget->parent->listview_sortedcolumn = column;

		listview_sort(widget->parent, column, widget->parent->listview_sortorders[column] == LISTVIEW_ASCENDING);

		widget->parent->modified = true;
	}
	// update the position of the selected item in the list
	else if (widget->parent->listview_pos != widget->listview_pos)
	{
		widget->parent->modified = true;
		widget->parent->listview_pos = widget->listview_pos;

		// update the listview's cvar
		if (!(widget->parent->flags & WIDGET_FLAG_NOAPPLY) && widget->parent->cvar)
		{
			if (widget->parent->listview_source)
			{
				//compensate for missing column widths and headers:
				Cvar_Set(widget->parent->cvar, widget->parent->listview_source_map[widget->listview_pos - 2]);
			}
			else
			{
				Cvar_Set(widget->parent->cvar, widget->parent->listview_map[widget->listview_pos]);
			}
		}

		if (widget->parent && widget->parent->command) //only execute command if the header was not clicked
		{
			Cbuf_AddText(Cmd_MacroExpandString(widget->parent->command));
			Cbuf_AddText("\n");
		}
	}
}


static menu_widget_t *create_background (int x, int y, int w, int h, menu_widget_t *next)
{
	menu_widget_t *start = next;
	menu_widget_t *new_widget;

	// top left corner:
	new_widget = M_GetNewBlankMenuWidget();
	new_widget->picwidth = SELECT_BACKGROUND_SIZE;
	new_widget->picheight = SELECT_BACKGROUND_SIZE;
	new_widget->pic = re.DrawFindPic("select1btl");
	new_widget->x = x;
	new_widget->y = y;
	new_widget->next = start;
	start = new_widget;

	// top edge
	new_widget = M_GetNewBlankMenuWidget();
	new_widget->picwidth = w - SELECT_BACKGROUND_SIZE*2;
	new_widget->picheight = SELECT_BACKGROUND_SIZE;
	new_widget->pic = re.DrawFindPic("select1bt");
	new_widget->x = x + SELECT_BACKGROUND_SIZE;
	new_widget->y = y;
	new_widget->next = start;
	start = new_widget;

	// top right corner:
	new_widget = M_GetNewBlankMenuWidget();
	new_widget->picwidth = SELECT_BACKGROUND_SIZE;
	new_widget->picheight = SELECT_BACKGROUND_SIZE;
	new_widget->pic = re.DrawFindPic("select1btr");
	new_widget->x = x + w - SELECT_BACKGROUND_SIZE;
	new_widget->y = y;
	new_widget->next = start;
	start = new_widget;

	// left edge:
	new_widget = M_GetNewBlankMenuWidget();
	new_widget->picwidth = SELECT_BACKGROUND_SIZE;
	new_widget->picheight = h - SELECT_BACKGROUND_SIZE*2;
	new_widget->pic = re.DrawFindPic("select1bl");
	new_widget->x = x;
	new_widget->y = y + SELECT_BACKGROUND_SIZE;
	new_widget->next = start;
	start = new_widget;

	// right edge:
	new_widget = M_GetNewBlankMenuWidget();
	new_widget->picwidth = SELECT_BACKGROUND_SIZE;
	new_widget->picheight = h - SELECT_BACKGROUND_SIZE*2;
	new_widget->pic = re.DrawFindPic("select1br");
	new_widget->x = x + w - SELECT_BACKGROUND_SIZE;
	new_widget->y = y + SELECT_BACKGROUND_SIZE;
	new_widget->next = start;
	start = new_widget;

	// middle:
	new_widget = M_GetNewBlankMenuWidget();
	new_widget->picwidth = w - SELECT_BACKGROUND_SIZE*2;
	new_widget->picheight = h - SELECT_BACKGROUND_SIZE*2;
	new_widget->pic = re.DrawFindPic("select1bm");
	new_widget->x = x + SELECT_BACKGROUND_SIZE;
	new_widget->y = y + SELECT_BACKGROUND_SIZE;
	new_widget->next = start;
	start = new_widget;

	// bottom edge:
	new_widget = M_GetNewBlankMenuWidget();
	new_widget->picwidth = w - SELECT_BACKGROUND_SIZE*2;
	new_widget->picheight = SELECT_BACKGROUND_SIZE;
	new_widget->pic = re.DrawFindPic("select1bb");
	new_widget->x = x + SELECT_BACKGROUND_SIZE;
	new_widget->y = y + h - SELECT_BACKGROUND_SIZE;
	new_widget->next = start;
	start = new_widget;

	// bottom left corner:
	new_widget = M_GetNewBlankMenuWidget();
	new_widget->picwidth = SELECT_BACKGROUND_SIZE;
	new_widget->picheight = SELECT_BACKGROUND_SIZE;
	new_widget->pic = re.DrawFindPic("select1bbl");
	new_widget->x = x;
	new_widget->y = y + h - SELECT_BACKGROUND_SIZE;
	new_widget->next = start;
	start = new_widget;

	// bottom right corner:
	new_widget = M_GetNewBlankMenuWidget();
	new_widget->picwidth = SELECT_BACKGROUND_SIZE;
	new_widget->picheight = SELECT_BACKGROUND_SIZE;
	new_widget->pic = re.DrawFindPic("select1bbr");
	new_widget->x = x + w - SELECT_BACKGROUND_SIZE;
	new_widget->y = y + h - SELECT_BACKGROUND_SIZE;
	new_widget->next = start;
	start = new_widget;

	return start;
}

static void select_widget_center_pos (menu_widget_t *widget)
{
	if (widget->select_pos > 0)
	{
		// put the selection in the middle of the widget (for long lists)
		widget->select_vstart = widget->select_pos - (widget->select_rows / 2);

		// don't go past end
		if (widget->select_totalitems - widget->select_vstart < widget->select_rows)
			widget->select_vstart = widget->select_totalitems - widget->select_rows;

		// don't go past start
		if (widget->select_vstart < 0)
			widget->select_vstart = 0;
	}
}

static void M_UnbindCommand (char *command)
{
	int		j;
	int		l;
	char	*b;

	l = strlen(command);

	for (j = 0; j < 256; ++j)
	{
		b = keybindings[j];

		if (!b)
			continue;

		if (!strncmp(b, command, l))
			Key_SetBinding(j, "");
	}
}

char *string_for_bind (char *bind)
{
	int keys[2];
	static char str[64];

	M_FindKeysForCommand(bind, keys);

	if (keys[0] == -1)
	{
		sprintf(str, "%cNot Bound%c", SCHAR_ITALICS, SCHAR_ITALICS);
	}
	else
	{
		strcpy(str, Key_KeynumToString(keys[0]));

		if (keys[1] != -1)
		{
			strcat(str, " or ");
			strcat(str, Key_KeynumToString(keys[1]));
		}
	}

	return str;
}


static void M_UpdateWidgetPosition (menu_widget_t *widget)
{
	int xcenteradj, ycenteradj;
	const char *text = NULL;
	image_t *pic = NULL;

	scale = cl_hudscale->value;
	// we want the menu to be drawn in the center of the screen
	// if it's too small to fill it.
	xcenteradj = (viddef.width - 320 * scale) / 2;
	ycenteradj = (viddef.height - 240 * scale) / 2;
	widget->widgetCorner.x = widget->textCorner.x = widget->x * scale + xcenteradj;
	widget->widgetCorner.y = widget->textCorner.y = widget->y * scale + ycenteradj;

	if (widget->enabled)
	{
		if (widget->text)
		{
			if (widget->dynamic)
				text = Cmd_MacroExpandString(widget->text);
			else
				text = widget->text;
		}

		if (widget->pic)
		{
			if (widget->dynamic && widget->picname)
				pic = re.DrawFindPic(Cmd_MacroExpandString(widget->picname));
			else
				pic = widget->pic;
		}

		// Update text/pic for hovering/selection
		if (widget->hover || widget->selected)
		{
			if (widget->hovertext)
			{
				if (widget->dynamic)
					text = Cmd_MacroExpandString(widget->hovertext);
				else
					text = widget->hovertext;
			}

			if (widget->hoverpic)
			{
				if (widget->dynamic && widget->hoverpicname)
					pic = re.DrawFindPic(Cmd_MacroExpandString(widget->hoverpicname));
				else
					pic = widget->hoverpic;
			}
		}

		// Calculate picture size
		if (pic || (widget->picwidth && widget->picheight))
		{
			widget->widgetSize.x = (widget->picwidth) ? widget->picwidth : pic->width;
			widget->widgetSize.y = (widget->picheight) ? widget->picheight : pic->height;

			if (widget->hover || widget->selected)
			{
				if (widget->hoverpicwidth)
					widget->widgetSize.x = widget->hoverpicwidth;

				if (widget->hoverpicheight)
					widget->widgetSize.y = widget->hoverpicheight;			
			}

			widget->widgetSize.x *= scale;
			widget->widgetSize.y *= scale;
		}

		switch(widget->type)
		{
		case WIDGET_TYPE_SLIDER:
			widget->widgetSize.x = SLIDER_TOTAL_WIDTH;
			widget->widgetSize.y = SLIDER_TOTAL_HEIGHT;
			break;
		case WIDGET_TYPE_CHECKBOX:
			widget->widgetSize.x = CHECKBOX_WIDTH;
			widget->widgetSize.y = CHECKBOX_HEIGHT;
			break;
		case WIDGET_TYPE_FIELD:
			{
				int width;

				width = widget->field_width - 2;

				if (width < 1)
					width = 1;

				widget->widgetSize.x = FIELD_LWIDTH + TEXT_WIDTH*width + FIELD_RWIDTH;
				widget->widgetSize.y = FIELD_HEIGHT;

				if (widget->valign != WIDGET_VALIGN_MIDDLE) // center text by field vertically
					widget->textCorner.y += (FIELD_HEIGHT-TEXT_HEIGHT)/2;
			}
			break;
		case WIDGET_TYPE_SELECT:
			if (widget->select_rows < 2) // jitodo -- allow for dropdowns later
				widget->select_rows = 2;

			if (widget->select_width < 2)
				widget->select_width = 2;

			widget->widgetSize.x = widget->select_width * TEXT_WIDTH + SELECT_HSPACING*2;
			widget->widgetSize.y = widget->select_rows * 
				(TEXT_HEIGHT+SELECT_VSPACING) + SELECT_VSPACING;
			break;

		case WIDGET_TYPE_LISTVIEW:
			widget->listview_totalwidth = 0;

			{
				int i;
				for (i = 0; i < widget->listview_columncount; i++)
					widget->listview_totalwidth += atoi(widget->listview_list[0][i]);
			}

			widget->widgetSize.x = widget->listview_totalwidth * TEXT_WIDTH
				+ widget->listview_column_separator_padding * (widget->listview_columncount) * scale
				+ SELECT_HSPACING * widget->listview_columncount
				+ SCROLL_ARROW_WIDTH_UNSCALED * scale; // TODO: Should only take this into account when there is a scroll bar present (or always show the scroll bar?)

			widget->widgetSize.y = widget->listview_rowcount * 
				(TEXT_HEIGHT+LISTVIEW_VSPACING) + LISTVIEW_VSPACING;
			break;
		}

		switch(widget->halign)
		{
		case WIDGET_HALIGN_CENTER:
			widget->widgetCorner.x -= widget->widgetSize.x * 0.5f;
			widget->textCorner.x -= strlen_noformat(text) * CHARWIDTH * scale * 0.5f;
			break;
		case WIDGET_HALIGN_RIGHT:
			widget->widgetCorner.x -= widget->widgetSize.x;
			widget->textCorner.x -= strlen_noformat(text) * CHARWIDTH * scale;
			switch(widget->type)
			{
			case WIDGET_TYPE_SLIDER:
			case WIDGET_TYPE_CHECKBOX:
			case WIDGET_TYPE_FIELD:
			case WIDGET_TYPE_SELECT:
			case WIDGET_TYPE_LISTVIEW:
				widget->textCorner.x -= CHARWIDTH * scale + widget->widgetSize.x;
				break;
			default:
				break;
			}
			break;
		case WIDGET_HALIGN_LEFT:
		default:
			switch(widget->type)
			{
			case WIDGET_TYPE_SLIDER:
			case WIDGET_TYPE_CHECKBOX:
			case WIDGET_TYPE_FIELD:
			case WIDGET_TYPE_SELECT:
			case WIDGET_TYPE_LISTVIEW:
				widget->textCorner.x += CHARWIDTH * scale + widget->widgetSize.x;
				break;
			default:
				break;
			}
			break;
		}

		switch(widget->valign)
		{
		case WIDGET_VALIGN_MIDDLE:
			widget->widgetCorner.y -= widget->widgetSize.y * 0.5f;
			widget->textCorner.y -= TEXT_HEIGHT * 0.5f;
			break;
		case WIDGET_VALIGN_BOTTOM:
			widget->widgetCorner.y -= widget->widgetSize.y;
			widget->textCorner.y -= TEXT_HEIGHT;
			break;
		default:
			break;
		}

		widget->mouseBoundaries.bottom = -0x0FFFFFFF;
		widget->mouseBoundaries.left = 0x0FFFFFFF;
		widget->mouseBoundaries.right = -0x0FFFFFFF;
		widget->mouseBoundaries.top = 0x0FFFFFFF;

		if (pic || widget->type == WIDGET_TYPE_CHECKBOX || (widget->picwidth && widget->picheight))
		{
			widget->mouseBoundaries.left = widget->widgetCorner.x;
			widget->mouseBoundaries.right = widget->widgetCorner.x + widget->widgetSize.x;
			widget->mouseBoundaries.top = widget->widgetCorner.y;
			widget->mouseBoundaries.bottom = widget->widgetCorner.y + widget->widgetSize.y;
		}

		if (text)
		{
			widget->textSize.x = strlen_noformat(text) * TEXT_HEIGHT;
			widget->textSize.y = TEXT_HEIGHT;

			switch(widget->type)
			{
			case WIDGET_TYPE_CHECKBOX:
				widget->textCorner.y = widget->widgetCorner.y + (CHECKBOX_HEIGHT - TEXT_HEIGHT) * 0.5f;
				break;
			case WIDGET_TYPE_SLIDER:
				widget->textCorner.y = widget->widgetCorner.y + (SLIDER_TOTAL_HEIGHT - TEXT_HEIGHT) * 0.5f;
				break;
			}

			if (widget->parent && widget->parent->type == WIDGET_TYPE_LISTVIEW)
			{
				widget->textCorner.x += widget->parent->listview_column_separator_padding * scale * 0.5f;
			}

			if (widget->mouseBoundaries.left > widget->textCorner.x)
				widget->mouseBoundaries.left = widget->textCorner.x;

			if (widget->mouseBoundaries.right < widget->textCorner.x + widget->textSize.x)
				widget->mouseBoundaries.right = widget->textCorner.x + widget->textSize.x;

			if (widget->mouseBoundaries.top > widget->textCorner.y)
				widget->mouseBoundaries.top = widget->textCorner.y;

			if (widget->mouseBoundaries.bottom < widget->textCorner.y + widget->textSize.y)
				widget->mouseBoundaries.bottom = widget->textCorner.y + widget->textSize.y;
		}

		if (widget->type == WIDGET_TYPE_SLIDER || widget->type == WIDGET_TYPE_FIELD)
		{
			widget->mouseBoundaries.left = widget->widgetCorner.x;
			widget->mouseBoundaries.right = widget->widgetCorner.x + widget->widgetSize.x;
			widget->mouseBoundaries.top = widget->widgetCorner.y;
			widget->mouseBoundaries.bottom = widget->widgetCorner.y + widget->widgetSize.y;
		}

		if (widget->type == WIDGET_TYPE_BUTTON)
		{
			widget->mouseBoundaries.left -= BUTTON_H_PADDING * scale;
			widget->mouseBoundaries.right += BUTTON_H_PADDING * scale;
			widget->mouseBoundaries.top -= BUTTON_V_PADDING * scale;
			widget->mouseBoundaries.bottom += BUTTON_V_PADDING * scale;
		}
	}
}


static void update_select_subwidgets (menu_widget_t *widget)
{
	int i;
	char *nullpos;
	char *s;
	menu_widget_t *new_widget;
	char temp;
	int width, x, y;
	char *widget_text;

	M_UpdateWidgetPosition(widget);

	//pthread_mutex_lock(&m_mut_widgets); // jitmultithreading
	if (widget->flags & WIDGET_FLAG_LISTSOURCE)
	{
		list_source(widget);
		widget->flags |= WIDGET_FLAG_USEMAP;
	}

	// find which position should be selected:
	if (widget->cvar)
	{
		s = Cvar_Get(widget->cvar, widget->cvar_default, CVAR_ARCHIVE)->string;

		widget->select_pos = -1; // nothing selected;

		for (i = 0; i < widget->select_totalitems; i++)
		{
			if (widget->flags & WIDGET_FLAG_USEMAP)
			{
				if (Q_streq(s, widget->select_map[i]))
				{
					widget->select_pos = i;
					break;
				}
			}
			else
			{
				if (Q_streq(s, widget->select_list[i]))
				{
					widget->select_pos = i;
					break;
				}
			}
		}
	}

	if (widget->subwidget)
		widget->subwidget = free_widgets(widget->subwidget);

	x = (widget->widgetCorner.x - (viddef.width - 320 * scale) / 2) / scale;
	y = (widget->widgetCorner.y - (viddef.height - 240 * scale) / 2) / scale;	

	width = widget->select_width;

	if (width < 3)
		width = 3;

	// create the vertical scroll buttons:
	if (widget->select_totalitems > widget->select_rows)
	{
		// Up arrow
		new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC, NULL, NULL, NULL,
			x + width*TEXT_WIDTH_UNSCALED - 
			SCROLL_ARROW_WIDTH_UNSCALED + SELECT_HSPACING_UNSCALED*2,
			y, true, false);

		new_widget->callback = callback_select_scrollup;
		new_widget->picwidth = SCROLL_ARROW_WIDTH_UNSCALED;
		new_widget->picheight = SCROLL_ARROW_HEIGHT_UNSCALED;
		new_widget->pic = re.DrawFindPic("select1u");
		new_widget->hoverpic = re.DrawFindPic("select1uh");
		new_widget->selectedpic = re.DrawFindPic("select1us");
		new_widget->parent = widget;
		new_widget->next = widget->subwidget;
		widget->subwidget = new_widget;

		// Down arrow
		new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC, NULL, NULL, NULL,
			x + width*TEXT_WIDTH_UNSCALED - 
			SCROLL_ARROW_WIDTH_UNSCALED + SELECT_HSPACING_UNSCALED*2,
			y + widget->select_rows*
			(TEXT_HEIGHT_UNSCALED+SELECT_VSPACING_UNSCALED) + SELECT_VSPACING_UNSCALED - 
			SCROLL_ARROW_HEIGHT_UNSCALED, true, false);

		new_widget->callback = callback_select_scrolldown;
		new_widget->picwidth = SCROLL_ARROW_WIDTH_UNSCALED;
		new_widget->picheight = SCROLL_ARROW_HEIGHT_UNSCALED;
		new_widget->pic = re.DrawFindPic("select1d");
		new_widget->hoverpic = re.DrawFindPic("select1dh");
		new_widget->selectedpic = re.DrawFindPic("select1ds");
		new_widget->parent = widget;
		new_widget->next = widget->subwidget;
		widget->subwidget = new_widget;

		// Scroll bar
		new_widget = M_CreateScrollbar(WIDGET_TYPE_VSCROLL,
			x + width*TEXT_WIDTH_UNSCALED - SCROLL_ARROW_WIDTH_UNSCALED +
			SELECT_HSPACING_UNSCALED*2,	y + SCROLL_ARROW_HEIGHT_UNSCALED,
			widget->select_rows *
			(TEXT_HEIGHT_UNSCALED+SELECT_VSPACING_UNSCALED) + SELECT_VSPACING_UNSCALED - 
			SCROLL_ARROW_HEIGHT_UNSCALED * 2,
			widget->select_totalitems, widget->select_rows, widget->select_vstart,
			m_vscrollbar_tray_selected, true);

		new_widget->parent = widget;
		new_widget->next = widget->subwidget;
		widget->subwidget = new_widget;
		
		width -= (SCROLL_ARROW_WIDTH_UNSCALED/TEXT_WIDTH_UNSCALED); // make text not overlap scrollbar
	}
	else
	{
		widget->select_vstart = 0;
	}

	// create a widget for each item visible in the select widget
	for (i = widget->select_vstart; i < widget->select_totalitems
		&& i < widget->select_rows + widget->select_vstart;	i++)
	{
		if (widget->flags & WIDGET_FLAG_BIND)
		{
			// add bind info to end:
			widget_text = va("%s%s", widget->select_list[i],
					string_for_bind(widget->select_map[i]));
		}
		else
		{
			widget_text = widget->select_list[i];
		}

		// jitodo -- should make sure strlen(widget_text) > select_hstart, otherwise bad mem!
		// (once horizontal scrolling added)
		
		// truncate the text if too wide to fit:
		if (strlen_noformat(widget->select_list[i] + widget->select_hstart) > width)
		{
			nullpos = widget->select_list[i] + str_actual_format_pos(widget->select_list[i], widget->select_hstart + width);
			temp = *nullpos;
			*nullpos = '\0';

			new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC, 
				widget_text + widget->select_hstart, NULL, NULL, 
				x+SELECT_HSPACING_UNSCALED, 
				y + (i - widget->select_vstart) * 
				(TEXT_HEIGHT_UNSCALED + SELECT_VSPACING_UNSCALED) +
				SELECT_VSPACING_UNSCALED, true, false);

			*nullpos = temp;
		}
		else
		{
			new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC, 
				widget_text + widget->select_hstart, NULL, NULL, 
				x+SELECT_HSPACING_UNSCALED, 
				y + (i - widget->select_vstart) * 
				(TEXT_HEIGHT_UNSCALED + SELECT_VSPACING_UNSCALED) +
				SELECT_VSPACING_UNSCALED, true, false);
		}

		// set images for background
		if (i == widget->select_pos)
			new_widget->pic = re.DrawFindPic("select1bs");
		else
			new_widget->pic = NULL; //re.DrawFindPic("blank");

		new_widget->hoverpic = re.DrawFindPic("select1bh");
		new_widget->picheight = TEXT_HEIGHT_UNSCALED + SELECT_VSPACING_UNSCALED;
		new_widget->valign = WIDGET_VALIGN_MIDDLE;
		new_widget->y += SELECT_VSPACING_UNSCALED*2;
		new_widget->picwidth = width * TEXT_WIDTH_UNSCALED;
		new_widget->select_pos = i;
		new_widget->parent = widget;
		new_widget->callback = callback_select_item;
		new_widget->callback_doubleclick = callback_doubleclick_item;
		new_widget->flags = widget->flags; // inherit flags from parent
		new_widget->next = widget->subwidget;
		widget->subwidget = new_widget;
	}

	// create the background:
	if (!(widget->flags & WIDGET_FLAG_NOBG))
	{
		widget->subwidget = create_background(x, y, 
			widget->select_width * TEXT_WIDTH_UNSCALED + SELECT_HSPACING_UNSCALED * 2, 
			widget->select_rows * (TEXT_HEIGHT_UNSCALED + SELECT_VSPACING_UNSCALED) + 
			SELECT_VSPACING_UNSCALED, widget->subwidget);
	}
	//pthread_mutex_unlock(&m_mut_widgets);
}

// xrichardx: multi column list view widgets
static void update_listview_subwidgets (menu_widget_t *widget)
{
	int i, j;
	char *nullpos;
	char *s;
	menu_widget_t *new_widget;
	char temp;
	int width, x, y, hpadding;
	char *widget_text;
	//image_t *image;

	if (widget->listview_rowcount < 2)
		return; //we need at least the column widths and headers...

	M_UpdateWidgetPosition(widget);

	//pthread_mutex_lock(&m_mut_widgets); // jitmultithreading
	if (widget->flags & WIDGET_FLAG_LISTSOURCE)
	{
		listview_source(widget);

		if(widget->listview_sortedcolumn >= 0)
		{
			// The source probably does not provide the data sorted in the way we want it -> sort manually.
			listview_sort(widget, widget->listview_sortedcolumn, widget->listview_sortorders[widget->listview_sortedcolumn] == LISTVIEW_ASCENDING);
		}
	}

	// find which position should be selected:
	if (widget->cvar)
	{
		s = Cvar_Get(widget->cvar, widget->cvar_default, CVAR_ARCHIVE)->string;

		widget->listview_pos = -1; // nothing selected;

		if (widget->listview_source)
		{
			for (i = 0; i < widget->listview_source_rowcount; i++)
			{
				if (Q_streq(s, widget->listview_source_map[i]))
				{
					widget->listview_pos = i + 2; //compensate for missing column widths and headers
					break;
				}
			}
		}
		else
		{
			for (i = 0; i < widget->listview_rowcount; i++)
			{
				if (Q_streq(s, widget->listview_map[i]))
				{
					widget->listview_pos = i;
					break;
				}
			}
		}
	}

	if (widget->subwidget)
		widget->subwidget = free_widgets(widget->subwidget);
	
	hpadding = widget->listview_column_separator_padding;
	if (hpadding < 1)
		hpadding = 1;

	x = (widget->widgetCorner.x - (viddef.width - 320 * scale) / 2) / scale;
	y = (widget->widgetCorner.y - (viddef.height - 240 * scale) / 2) / scale;

	width = widget->listview_totalwidth * TEXT_WIDTH_UNSCALED
		+ hpadding * (widget->listview_columncount);

	// create vertical scroll bar:
	if (widget->listview_source)
	{
		if (widget->listview_visible_rows < (widget->listview_source_rowcount - 1))
		{
			// Up arrow
			new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC, NULL, NULL, NULL,
				x + width,
				y, true, false);

			new_widget->callback = callback_select_scrollup;
			new_widget->picwidth = SCROLL_ARROW_WIDTH_UNSCALED;
			new_widget->picheight = SCROLL_ARROW_HEIGHT_UNSCALED;
			new_widget->pic = re.DrawFindPic("select1u");
			new_widget->hoverpic = re.DrawFindPic("select1uh");
			new_widget->selectedpic = re.DrawFindPic("select1us");
			new_widget->parent = widget;
			new_widget->next = widget->subwidget;
			widget->subwidget = new_widget;

			// Down arrow
			new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC, NULL, NULL, NULL,
				x + width,
				y + widget->listview_visible_rows*
				(TEXT_HEIGHT_UNSCALED+LISTVIEW_VSPACING_UNSCALED) + LISTVIEW_VSPACING_UNSCALED*2 - 
				SCROLL_ARROW_HEIGHT_UNSCALED, true, false);

			new_widget->callback = callback_select_scrolldown;
			new_widget->picwidth = SCROLL_ARROW_WIDTH_UNSCALED;
			new_widget->picheight = SCROLL_ARROW_HEIGHT_UNSCALED;
			new_widget->pic = re.DrawFindPic("select1d");
			new_widget->hoverpic = re.DrawFindPic("select1dh");
			new_widget->selectedpic = re.DrawFindPic("select1ds");
			new_widget->parent = widget;
			new_widget->next = widget->subwidget;
			widget->subwidget = new_widget;

			// Scroll bar
			new_widget = M_CreateScrollbar(WIDGET_TYPE_VSCROLL,
				x + width,
				y + SCROLL_ARROW_HEIGHT_UNSCALED,
				widget->listview_visible_rows *
				(TEXT_HEIGHT_UNSCALED+LISTVIEW_VSPACING_UNSCALED) + LISTVIEW_VSPACING_UNSCALED*2 - 
				SCROLL_ARROW_HEIGHT_UNSCALED * 2,
				widget->listview_source_rowcount + 1, widget->listview_visible_rows, widget->listview_vstart,
				m_vscrollbar_tray_selected, true);

			new_widget->parent = widget;
			new_widget->next = widget->subwidget;
			widget->subwidget = new_widget;
		}
		else
		{
			widget->listview_vstart = 0;
		}
	}
	else
	{
		if (widget->listview_visible_rows < (widget->listview_rowcount - 1))
		{
			// Up arrow
			new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC, NULL, NULL, NULL,
				x + width,
				y, true, false);

			new_widget->callback = callback_select_scrollup;
			new_widget->picwidth = SCROLL_ARROW_WIDTH_UNSCALED;
			new_widget->picheight = SCROLL_ARROW_HEIGHT_UNSCALED;
			new_widget->pic = re.DrawFindPic("select1u");
			new_widget->hoverpic = re.DrawFindPic("select1uh");
			new_widget->selectedpic = re.DrawFindPic("select1us");
			new_widget->parent = widget;
			new_widget->next = widget->subwidget;
			widget->subwidget = new_widget;

			// Down arrow
			new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC, NULL, NULL, NULL,
				x + width,
				y + widget->listview_visible_rows*
				(TEXT_HEIGHT_UNSCALED+LISTVIEW_VSPACING_UNSCALED) + LISTVIEW_VSPACING_UNSCALED*2 - 
				SCROLL_ARROW_HEIGHT_UNSCALED, true, false);

			new_widget->callback = callback_select_scrolldown;
			new_widget->picwidth = SCROLL_ARROW_WIDTH_UNSCALED;
			new_widget->picheight = SCROLL_ARROW_HEIGHT_UNSCALED;
			new_widget->pic = re.DrawFindPic("select1d");
			new_widget->hoverpic = re.DrawFindPic("select1dh");
			new_widget->selectedpic = re.DrawFindPic("select1ds");
			new_widget->parent = widget;
			new_widget->next = widget->subwidget;
			widget->subwidget = new_widget;

			// Scroll bar
			new_widget = M_CreateScrollbar(WIDGET_TYPE_VSCROLL,
				x + width,
				y + SCROLL_ARROW_HEIGHT_UNSCALED,
				widget->listview_visible_rows *
				(TEXT_HEIGHT_UNSCALED+LISTVIEW_VSPACING_UNSCALED) + LISTVIEW_VSPACING_UNSCALED*2 - 
				SCROLL_ARROW_HEIGHT_UNSCALED * 2,
				widget->listview_rowcount - 1, widget->listview_visible_rows, widget->listview_vstart,
				m_vscrollbar_tray_selected, true);

			new_widget->parent = widget;
			new_widget->next = widget->subwidget;
			widget->subwidget = new_widget;
		}
		else
		{
			widget->listview_vstart = 0;
		}
	}
	
	//create the header widgets:
	x = (widget->widgetCorner.x - (viddef.width - 320 * scale) / 2) / scale + 1;
	for (i = 0; i < widget->listview_columncount; i++)
	{
		widget_text = widget->listview_list[1][i];
		width = atoi(widget->listview_list[0][i]);
		

		if (strlen_noformat(widget->listview_list[1][i]) > width)
		{
			nullpos = widget->listview_list[1][i] + str_actual_format_pos(widget->listview_list[1][i], width);
			temp = *nullpos;
			*nullpos = '\0';

			new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC,
				widget_text, NULL, NULL,
				x,
				y + LISTVIEW_VSPACING_UNSCALED,
				true,
				false);

			*nullpos = temp;
		}
		else
		{
			new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC,
				widget_text, NULL, NULL,
				x,
				y + LISTVIEW_VSPACING_UNSCALED,
				true,
				false);
		}
		
		//Problem: stretching and hilighting when hovered and selected
		/*image = re.DrawFindPic(widget_text);
		if (strcmp(image->name, "***r_notexture***"))
		{
			new_widget->pic = image;
			if (strlen(new_widget->text) > 0)
				new_widget->text[0] = '\0';
		}*/

		new_widget->pic = re.DrawFindPic("select1bs");
		new_widget->picheight = TEXT_HEIGHT_UNSCALED + LISTVIEW_VSPACING_UNSCALED;
		new_widget->valign = WIDGET_VALIGN_MIDDLE;
		new_widget->y += LISTVIEW_VSPACING_UNSCALED*2;
		new_widget->picwidth = atoi(widget->listview_list[0][i]) * TEXT_WIDTH_UNSCALED + hpadding - 1;
		new_widget->listview_pos = 1;
		new_widget->listview_pos_x = i;
		new_widget->parent = widget;
		new_widget->callback = callback_listview_item;
		new_widget->flags = widget->flags; // inherit flags from parent
		new_widget->next = widget->subwidget;
		new_widget->textCorner.x += 10;
		widget->subwidget = new_widget;

		x += atoi(widget->listview_list[0][i]) * TEXT_WIDTH_UNSCALED + hpadding;
	}

	y += TEXT_HEIGHT_UNSCALED + LISTVIEW_VSPACING_UNSCALED * 2;
	// create a widget for each row visible from listview_list
	if (!widget->listview_source)
	{
		for (j = (widget->listview_vstart + 2);
			j < (widget->listview_vstart + widget->listview_visible_rows + 1)
			&& j < widget->listview_rowcount;
			j++)
		{
			x = (widget->widgetCorner.x - (viddef.width - 320 * scale) / 2) / scale + 1;

			for (i = 0; i < widget->listview_columncount; i++)
			{
				widget_text = widget->listview_list[j][i];
				width = atoi(widget->listview_list[0][i]);

				if (strlen_noformat(widget_text) > width)
				{
					int truncate_pos = str_actual_format_pos(widget_text, width);
					nullpos = widget_text + truncate_pos;
					temp = *nullpos;
					*nullpos = '\0';

					new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC,
						widget_text, NULL, NULL,
						x,
						y + LISTVIEW_VSPACING_UNSCALED,
						true,
						false);

					*nullpos = temp;
				}
				else
				{
					new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC,
						widget_text, NULL, NULL,
						x,
						y + LISTVIEW_VSPACING_UNSCALED,
						true,
						false);
				}
				
				if (j == widget->listview_pos) //for selected column
				{
					new_widget->pic = re.DrawFindPic("select1bs");
					new_widget->hoverpic = re.DrawFindPic("select1bh");
				}
				else
				{
					new_widget->pic = NULL;
					new_widget->hoverpic = re.DrawFindPic("select1bh");
				}

				new_widget->picheight = TEXT_HEIGHT_UNSCALED + LISTVIEW_VSPACING_UNSCALED;
				new_widget->valign = WIDGET_VALIGN_MIDDLE;
				new_widget->y += LISTVIEW_VSPACING_UNSCALED*2;
				new_widget->picwidth = atoi(widget->listview_list[0][i]) * TEXT_WIDTH_UNSCALED + hpadding - 1;
				new_widget->listview_pos = j;
				new_widget->listview_pos_x = i;
				new_widget->parent = widget;
				new_widget->callback = callback_listview_item;
				new_widget->callback_doubleclick = callback_doubleclick_item;
				new_widget->flags = widget->flags; // inherit flags from parent
				new_widget->next = widget->subwidget;
				widget->subwidget = new_widget;

				x += atoi(widget->listview_list[0][i]) * TEXT_WIDTH_UNSCALED + hpadding;
			}

			y += TEXT_HEIGHT_UNSCALED + LISTVIEW_VSPACING_UNSCALED;
		}
	}
	//create a widget for every item in listview_source_list;
	else
	{
		for (j = widget->listview_vstart;
			j < (widget->listview_vstart + widget->listview_visible_rows - 1)
			&& j < widget->listview_source_rowcount;
			j++)
		{
			x = (widget->widgetCorner.x - (viddef.width - 320 * scale) / 2) / scale + 1;

			for (i = 0; i < widget->listview_columncount; i++)
			{
				widget_text = widget->listview_source_list[j][i];
				width = atoi(widget->listview_list[0][i]);

				if (strlen_noformat(widget_text) > width)
				{
					int truncate_pos = str_actual_format_pos(widget_text, width);
					nullpos = widget_text + truncate_pos;
					temp = *nullpos;
					*nullpos = '\0';

					new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC,
						widget_text, NULL, NULL,
						x,
						y + LISTVIEW_VSPACING_UNSCALED,
						true,
						false);

					*nullpos = temp;
				}
				else
				{
					new_widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC,
						widget_text, NULL, NULL,
						x,
						y + LISTVIEW_VSPACING_UNSCALED,
						true,
						false);
				}

				if (j+2 == widget->listview_pos) //for selected column (missing first two array entried compensated)
				{
					new_widget->pic = re.DrawFindPic("select1bs");
					new_widget->hoverpic = re.DrawFindPic("select1bh");
				}
				else
				{
					new_widget->pic = NULL;
					new_widget->hoverpic = re.DrawFindPic("select1bh");
				}

				new_widget->picheight = TEXT_HEIGHT_UNSCALED + LISTVIEW_VSPACING_UNSCALED;
				new_widget->valign = WIDGET_VALIGN_MIDDLE;
				new_widget->y += LISTVIEW_VSPACING_UNSCALED*2;
				new_widget->picwidth = atoi(widget->listview_list[0][i]) * TEXT_WIDTH_UNSCALED + hpadding - 1;
				new_widget->listview_pos = j + 2; //compensate for missing column width and header caption at the beginning
				new_widget->listview_pos_x = i;
				new_widget->parent = widget;
				new_widget->callback = callback_listview_item;
				new_widget->callback_doubleclick = callback_doubleclick_item;
				new_widget->flags = widget->flags; // inherit flags from parent
				new_widget->next = widget->subwidget;
				widget->subwidget = new_widget;

				x += atoi(widget->listview_list[0][i]) * TEXT_WIDTH_UNSCALED + hpadding;
			}

			y += TEXT_HEIGHT_UNSCALED + LISTVIEW_VSPACING_UNSCALED;
		}
	}

	
	//create the background:
	if (!(widget->flags & WIDGET_FLAG_NOBG))
	{
		//header background:
		x = (widget->widgetCorner.x - (viddef.width - 320 * scale) / 2) / scale;
		y = (widget->widgetCorner.y - (viddef.height - 240 * scale) / 2) / scale;

		for (i = 0; i < widget->listview_columncount; i++)
		{
			widget->subwidget = create_background(x, y, 
				atoi(widget->listview_list[0][i]) * TEXT_WIDTH_UNSCALED + hpadding + 1,
				TEXT_HEIGHT_UNSCALED + LISTVIEW_VSPACING_UNSCALED*2,
				widget->subwidget);

			x += atoi(widget->listview_list[0][i]) * TEXT_WIDTH_UNSCALED + hpadding;
		}
		
		//column background:
		x = (widget->widgetCorner.x - (viddef.width - 320 * scale) / 2) / scale;

		for (i = 0; i < widget->listview_columncount; i++)
		{
			widget->subwidget = create_background(x,
				y + TEXT_HEIGHT_UNSCALED + LISTVIEW_VSPACING_UNSCALED*2, 
				atoi(widget->listview_list[0][i]) * TEXT_WIDTH_UNSCALED + hpadding + 1, 
				(widget->listview_visible_rows - 1) * (TEXT_HEIGHT_UNSCALED + LISTVIEW_VSPACING_UNSCALED) + LISTVIEW_VSPACING_UNSCALED,
				widget->subwidget);

			x += atoi(widget->listview_list[0][i]) * TEXT_WIDTH_UNSCALED + hpadding;
		}
	}
	//pthread_mutex_unlock(&m_mut_widgets);
}


// ++ ARTHUR [9/04/03]
static void M_UpdateDrawingInformation (menu_widget_t *widget)
{
	menu_widget_t* under_cursor;

	// only update if the widget or the hudscale has changed
	if (!(widget->modified || cl_hudscale->modified || widget->dynamic))
		return;

	M_UpdateWidgetPosition(widget);

	if (widget->select_list)
		update_select_subwidgets(widget);

	if (widget->listview_list)
		update_listview_subwidgets(widget);

	under_cursor = find_widget_under_cursor(widget);
	if(under_cursor)
	{
		under_cursor->hover = true;
		M_UpdateWidgetPosition(under_cursor);
	}

	widget->modified = false;
}


static void M_DeselectWidget (menu_widget_t *current)
{
	// jitodo - apply changes to field, call this func during keyboard arrow selection
	current->slider_hover = SLIDER_SELECTED_NONE;
	current->slider_selected = SLIDER_SELECTED_NONE;
	current->selected = false;
	current->hover = false;
}


static void M_UnfocusWidget (menu_widget_t *current)
{
	current->hover = false;
}


static menu_widget_t *find_widget_under_cursor (menu_widget_t *widget)
{
	menu_widget_t *selected = NULL;
	menu_widget_t *sub_selected;

	while (widget)
	{
		if (widget_is_selectable(widget) &&
			m_mouse.x > widget->mouseBoundaries.left &&
			m_mouse.x < widget->mouseBoundaries.right &&
			m_mouse.y < widget->mouseBoundaries.bottom &&
			m_mouse.y > widget->mouseBoundaries.top)
		{
			selected = widget;
		}
		else
		{
			// clear selections
			M_UnfocusWidget(widget);
		}

		if (widget->subwidget)
		{
			sub_selected = find_widget_under_cursor(widget->subwidget);

			if (sub_selected)
				selected = sub_selected;
		}

		widget = widget->next;
	}

	return selected;
}


// figure out what portion of the slider to highlight
// (buttons, knob, or bar)
static void M_HilightSlider (menu_widget_t *widget, qboolean selected)
{
	widget->slider_hover = SLIDER_SELECTED_NONE;
	widget->slider_selected = SLIDER_SELECTED_NONE;

	if (widget->type != WIDGET_TYPE_SLIDER)
		return;

	// left button:
	if (m_mouse.x < widget->mouseBoundaries.left + SLIDER_BUTTON_WIDTH)
	{
		if (selected)
			widget->slider_selected = SLIDER_SELECTED_LEFTARROW;
		else
			widget->slider_hover = SLIDER_SELECTED_LEFTARROW;
	}
	// tray area and knob:
	else if (m_mouse.x < widget->mouseBoundaries.left + SLIDER_BUTTON_WIDTH + SLIDER_TRAY_WIDTH)
	{
		if (selected)
			widget->slider_selected = SLIDER_SELECTED_TRAY;
		else
			widget->slider_hover = SLIDER_SELECTED_TRAY;
	}
	// right button:
	else
	{
		if (selected)
			widget->slider_selected = SLIDER_SELECTED_RIGHTARROW;
		else
			widget->slider_hover = SLIDER_SELECTED_RIGHTARROW;
	}
}

// update slider depending on where the user clicked
static void M_UpdateSlider (menu_widget_t *widget)
{
	float value;

	if (widget->cvar)
		value = Cvar_Get(widget->cvar, widget->cvar_default, CVAR_ARCHIVE)->value;
	else
		return;

	switch (widget->slider_selected)
	{
	case SLIDER_SELECTED_LEFTARROW:
		MENU_SOUND_SLIDER;
		value -= widget->slider_inc;
		break;
	case SLIDER_SELECTED_RIGHTARROW:
		MENU_SOUND_SLIDER;
		value += widget->slider_inc;
		break;
	case SLIDER_SELECTED_TRAY:
		value = (float)(m_mouse.x - (widget->mouseBoundaries.left + SLIDER_BUTTON_WIDTH + SLIDER_KNOB_WIDTH/2))/((float)(SLIDER_TRAY_WIDTH-SLIDER_KNOB_WIDTH));
		value = value * (widget->slider_max - widget->slider_min) + widget->slider_min;
		break;
	case SLIDER_SELECTED_KNOB:

		break;
	default:
		break;
	}

	// we can have the min > max for things like vid_gamma, where it makes
	// more sense for the user to have it backwards
	if (widget->slider_min > widget->slider_max)
	{
		if (value > widget->slider_min)
			value = widget->slider_min;
		else if (value < widget->slider_max)
			value = widget->slider_max;
	}
	else
	{
		if (value > widget->slider_max)
			value = widget->slider_max;
		if (value < widget->slider_min)
			value = widget->slider_min;
	}

	Cvar_SetValue(widget->cvar, value);
	// todo: may not want to set value immediately (video mode change) -- need temp variables.
}

static void toggle_checkbox (menu_widget_t *widget)
{
	if (widget->cvar)
	{
		if (Cvar_Get(widget->cvar, widget->cvar_default, CVAR_ARCHIVE)->value)
			Cvar_SetValue(widget->cvar, 0.0f);
		else
			Cvar_SetValue(widget->cvar, 1.0f);	
	}
}


static void M_HilightNextWidget (menu_screen_t *menu)
{
	menu_widget_t *widget;

	if (!menu)
		return;

	widget = menu->widget;

	// find the hilighted widget
	while (widget && !widget->selected && !widget->hover)
		widget = widget->next;

	// nothing hilighted, start with the first one on the list
	// that's selectable
	if (!widget)
	{
		widget = menu->widget;

		while (widget && !widget_is_selectable(widget))
			widget = widget->next;

		if (widget)
		{
			widget->hover = true;
			MENU_SOUND_SELECT;
		}
	}
	else
	{
		widget->hover = false;
		widget->selected = false;
		widget = widget->next;

		if (!widget)
			widget = menu->widget;

		while (!widget_is_selectable(widget))
		{
			widget = widget->next;
			if (!widget)
				widget = menu->widget;
		}

		widget->hover = true;
		MENU_SOUND_SELECT;
	}
}


static void M_HilightPreviousWidget (menu_screen_t *menu)
{
	menu_widget_t *widget, *oldwidget, *prevwidget = NULL;

	widget = menu->widget;

	// find the hilighted widget
	while (widget && !widget->selected && !widget->hover)
		widget = widget->next;

	// nothing hilighted, start with the first one on the list
	// that's selectable
	if (!widget)
	{
		widget = menu->widget;

		while(widget && !widget_is_selectable(widget))
			widget = widget->next;

		if (widget)
		{
			MENU_SOUND_SELECT;
			widget->hover = true;
		}
	}
	// otherwise find the previous selectable widget
	else
	{
		widget->hover = false;
		widget->selected = false;
		oldwidget = widget;
		widget = widget->next;

		if (!widget)
			widget = menu->widget;

		while (widget != oldwidget)
		{
			if (widget_is_selectable(widget))
				prevwidget = widget;

			widget = widget->next;

			if (!widget)
				widget = menu->widget;
		}

		if (!prevwidget)
			prevwidget = widget;

		prevwidget->hover = true;
		MENU_SOUND_SELECT;
	}
}


static void field_adjustCursor (menu_widget_t *widget)
{
	int pos = widget->field_cursorpos;
	int string_len = 0;
	const char *str = "";

	if (widget->cvar)
		str = Cvar_Get(widget->cvar, widget->cvar_default, CVAR_ARCHIVE)->string;

	string_len = strlen(str);

	if (pos < 0)
		pos = 0;

	if (pos < widget->field_start)
		widget->field_start = pos;

	if (pos > string_len)
		pos = string_len;

	if (widget->field_start > string_len) // shouldn't happen, but just in case something wierd goes on and the string is shortened?
	{
		assert(0);
		widget->field_start = string_len;
	}

	if (str_visible_pos(str + widget->field_start, pos) >= widget->field_width)
	{
		//widget->field_start = pos - widget->field_width + 1;
		widget->field_start = str_visible_pos_back(str, pos, widget->field_width - 1);
	}

	widget->field_cursorpos = pos;
}


static void M_AdjustWidget (menu_screen_t *menu, int direction, qboolean keydown)
{
	menu_widget_t *widget;
	menu_widget_t *selected = NULL;

	// find an active widget, if there is one:
	for (widget = menu->widget; widget != NULL; widget = widget->next)
	{
		if (widget->selected)
		{
			selected = widget;
			break;
		}

        if (widget->hover)
		{
			if (!selected) // use the hover widget only if there is no selected one.
				selected = widget;
		}
	}

	widget = selected;

	if (widget)
	{
		switch (widget->type)
		{
		case WIDGET_TYPE_SLIDER:
			if (keydown)
			{
				if (direction < 0)
					widget->slider_selected = SLIDER_SELECTED_LEFTARROW;
				else
					widget->slider_selected = SLIDER_SELECTED_RIGHTARROW;
			}
			else
			{
				widget->slider_selected = SLIDER_SELECTED_NONE;
			}

			M_UpdateSlider(widget);
			break;
		case WIDGET_TYPE_FIELD:
			if (keydown)
			{
				widget->field_cursorpos += direction;
				field_adjustCursor(widget);
			}
			break;
		case WIDGET_TYPE_SELECT:
			if (keydown)
			{
				widget->select_pos += direction;

				if (widget->select_pos < 0)
					widget->select_pos = 0;

				if (widget->select_pos >= widget->select_totalitems)
					widget->select_pos = widget->select_totalitems - 1;

				widget->modified = true;

				if (widget->cvar)
				{
					if (widget->flags & WIDGET_FLAG_USEMAP)
						Cvar_Set(widget->cvar, widget->select_map[widget->select_pos]);
					else
						Cvar_Set(widget->cvar, widget->select_list[widget->select_pos]);
				}

				select_widget_center_pos(widget);
			}
			break;
		default:
			break;
		}
	}
}


static void field_activate (menu_widget_t *widget)
{
	widget->field_cursorpos = (m_mouse.x - widget->widgetCorner.x)/(TEXT_WIDTH) + widget->field_start;
	field_adjustCursor(widget);
}


static void M_DeselectAllWidgets (menu_widget_t *widget)
{
	while (widget)
	{
		M_DeselectWidget(widget);

		if (widget->subwidget)
			M_DeselectAllWidgets(widget->subwidget);

		widget = widget->next;
	}
}


static qboolean widget_execute (menu_widget_t *widget, qboolean doubleclick)
{
	if (widget->selected || widget->hover)
	{
		if (doubleclick)
		{
			if (widget->callback_doubleclick)
				widget->callback_doubleclick(widget);

			if (widget->doubleclick)
			{
				Cbuf_AddText(Cmd_MacroExpandString(widget->doubleclick));
				Cbuf_AddText("\n");
			}
		}
		else
		{
			if (widget->callback)
				widget->callback(widget);

			if (widget->command)
			{
				Cbuf_AddText(Cmd_MacroExpandString(widget->command));
				Cbuf_AddText("\n");
			}
		}

		switch (widget->type)
		{
		case WIDGET_TYPE_SLIDER:
			M_UpdateSlider(widget);
			widget->selected = false;
			break;
		case WIDGET_TYPE_CHECKBOX:
			toggle_checkbox(widget);
			widget->selected = false;
			break;
		case WIDGET_TYPE_FIELD:
			widget->selected = true;
			field_activate(widget);
			break;
		default:
			widget->selected = false;
			break;
		}

		widget->hover = true;

		while (widget->parent)
		{
			widget = widget->parent;
			widget->hover = true;
		}

		return true;
	}
	else
	{
		return false;
	}
}


static void M_KBAction (menu_screen_t *menu, MENU_ACTION action)
{
	menu_widget_t *widget;

	if (!menu)
		return;

	widget = menu->widget;

	while (widget && !widget->hover && !widget->selected)
		widget = widget->next;

	if (widget)
	{
		switch (action)
		{
		case M_ACTION_SELECT:
			M_DeselectAllWidgets(menu->widget);
			widget->selected = true;
			break;
		case M_ACTION_EXECUTE:
			widget_execute(widget, false);
			break;
		default:
			break;
		}
	}
}

// Returns true if the widget or one of its parents is a select widget.  The parent
// select widget is returned in widget_out.
static qboolean widget_is_select (menu_widget_t *widget, menu_widget_t **widget_out)
{
	if (widget)
	{
		if (widget->type == WIDGET_TYPE_SELECT)
		{
			*widget_out = widget;
			return true;
		}
		else if (widget->parent)
		{
			return widget_is_select(widget->parent, widget_out);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

static qboolean widget_is_listview (menu_widget_t *widget, menu_widget_t **widget_out)
{
	if (widget)
	{
		if (widget->type == WIDGET_TYPE_LISTVIEW)
		{
			*widget_out = widget;
			return true;
		}
		else if (widget->parent)
		{
			return widget_is_listview(widget->parent, widget_out);
		}
	}
	return false;
}

static qboolean M_MouseAction (menu_screen_t *menu, MENU_ACTION action)
{
	menu_widget_t *newSelection = NULL;
	menu_widget_t *widget = NULL;
	m_mouse.cursorpic = i_cursor;
	m_vscrollbar_tray_selected = false;

	if (!menu)
		return false;

	// If we're actually over something
	if ((newSelection = find_widget_under_cursor(menu->widget))) 
	{
		newSelection->modified = true;

		switch (action)
		{
		case M_ACTION_HILIGHT:
			switch (newSelection->type)
			{
			case WIDGET_TYPE_SLIDER:
				M_HilightSlider(newSelection, newSelection->selected);

				if (newSelection->slider_selected == SLIDER_SELECTED_TRAY)
					M_UpdateSlider(newSelection);

				break;
			case WIDGET_TYPE_FIELD:
				m_mouse.cursorpic = i_cursor_text;
				break;
			default:
				break;
			}
			
			newSelection->hover = true;
			break;
		case M_ACTION_SELECT:
			M_DeselectAllWidgets(menu->widget);

			switch (newSelection->type)
			{
			case WIDGET_TYPE_SLIDER:
				M_HilightSlider(newSelection, true);
				if (newSelection->slider_selected == SLIDER_SELECTED_TRAY)
					M_UpdateSlider(newSelection);
				break;
			case WIDGET_TYPE_FIELD:
				m_mouse.cursorpic = i_cursor_text;
				break;
			default:
				break;
			}

			newSelection->selected = true;
			break;
		case M_ACTION_EXECUTE:
			if (!widget_execute(newSelection, false))
				M_DeselectAllWidgets(menu->widget);
			break;
		case M_ACTION_DOUBLECLICK:
			if (!widget_execute(newSelection, true))
				M_DeselectAllWidgets(menu->widget);
			break;
		case M_ACTION_SCROLLUP:
			if (widget_is_select(newSelection, &widget) || widget_is_listview(newSelection, &widget))
			{
				if (widget->select_vstart > 0)
				{
					widget->select_vstart--;
					widget->modified = true;
				}
			}
			else if (newSelection->type == WIDGET_TYPE_SLIDER)
			{
				M_AdjustWidget(menu, 1, true);
			}
			break;
		case M_ACTION_SCROLLDOWN:
			if (widget_is_select(newSelection, &widget))
			{
				if (widget->select_totalitems - widget->select_vstart - widget->select_rows > 0)
				{
					widget->select_vstart++; // todo - can we just use M_AdjustWidget?
					widget->modified = true;
				}
			}
			else if (widget_is_listview(newSelection, &widget) && listview_scrolldown_allowed(widget))
			{
				widget->select_vstart++; // todo - can we just use M_AdjustWidget?
				widget->modified = true;
			}
			else if (newSelection->type == WIDGET_TYPE_SLIDER)
			{
				M_AdjustWidget(menu, -1, true);
			}
			break;
		case M_ACTION_DRAG:
			if (newSelection->callback_drag)
				newSelection->callback_drag(newSelection);
			break;
		case M_ACTION_NONE:
		default:
			break;
		}
	}
	else
	{
		// If we're not over anything when we release the mouse button, deselect everything
		if (action == M_ACTION_EXECUTE)
			M_DeselectAllWidgets(menu->widget);
	}

	return true;
}


// flag widget and all of its children as modified
static void refresh_menu_widget (menu_widget_t *widget)
{
	widget->modified = true;
	widget = widget->subwidget;

	while (widget)
	{
		refresh_menu_widget(widget);
		widget = widget->next;
	}
}


// flag widgets as modified:
static void refresh_menu_screen (menu_screen_t *menu)
{
	menu_widget_t *widget;

	if (!menu)
		return;

	//pthread_mutex_lock(&m_mut_widgets); // jitmultithreading
	widget = menu->widget;

	while (widget)
	{
		refresh_menu_widget(widget);
		widget = widget->next;
	}
	//pthread_mutex_unlock(&m_mut_widgets);
}


static void M_PushMenuScreen (menu_screen_t *menu, qboolean samelevel)
{
	MENU_SOUND_OPEN;

	if (m_menudepth < MAX_MENU_SCREENS)
	{
		refresh_menu_screen(menu); // screen size may have changed or something, refresh it.

		if (samelevel)
		{
			m_menu_screens[m_menudepth-1] = menu;
		}
		else
		{
			m_menu_screens[m_menudepth] = menu;
			m_menudepth++;
		}

		cls.key_dest = key_menu;
	}
}


void M_PopMenu (const char *sMenuName)
{
	menu_widget_t *widget;
	menu_screen_t *menu;

	if (m_menudepth < 1)
	{
		// Don't think this ever gets executed, but just in case.
		M_ForceMenuOff();
		return;
	}

	menu = m_menu_screens[m_menudepth - 1];

	if (!menu)
	{
		M_ForceMenuOff();
		return;
	}

	if (sMenuName && *sMenuName && !Q_streq(sMenuName, menu->name))
		return;

	MENU_SOUND_CLOSE;
	--m_menudepth;
	widget = menu->widget;

	while (widget)
	{
		if (widget->flags & WIDGET_FLAG_PASSWORD)
			if (widget->cvar)
				Cbuf_AddText(va("unset %s\n", widget->cvar));

		widget = widget->next;
	}

	if (oldscale && (oldscale != cl_hudscale->value))
		Cvar_SetValue("cl_hudscale", oldscale);

	if (!m_menudepth)
		M_ForceMenuOff();
}

// find the first widget that is selected or hilighted and return it
// otherwise return null.  Selected widgets have higher priority.
static menu_widget_t *M_GetActiveWidget (menu_screen_t *menu)
{
	menu_widget_t *widget;
	menu_widget_t *active = NULL;

	if (!menu)
		menu = m_current_menu;

	if (!menu)
		return NULL;

	for (widget = menu->widget; widget; widget = widget->next)
	{
		if (widget->selected)
			return widget;

		if (widget->hover)
			active = widget;
	}

	return active;
}


// returns true if key was handled here (user inputed data into field)
// returns false if no field was active.
static qboolean M_InsertField (int key)
{
	char s[256];
	menu_widget_t *widget;
	int cursorpos, maxlength;
	int len;

	widget = M_GetActiveWidget(NULL);

	if (!widget || widget->type != WIDGET_TYPE_FIELD || !widget->cvar)
		return false;

	strcpy(s, Cvar_Get(widget->cvar, widget->cvar_default, CVAR_ARCHIVE)->string);
	len = strlen(s);
	cursorpos = widget->field_cursorpos;

	if (cursorpos > len)
		cursorpos = len;

	maxlength = widget->field_width;
	key = KeyPadKey(key);

	if ((toupper(key) == 'V' && keydown[K_CTRL]) ||
		 (((key == K_INS) || (key == K_KP_INS)) && keydown[K_SHIFT]))
	{
		char *cbd;
		
		if ((cbd = Sys_GetClipboardData()) != 0)
		{
			int i;

			strtok(cbd, "\n\r\b");

			i = strlen(cbd);
			if (i + strlen(s) >= sizeof(s)-1)
				i= sizeof(s) - 1 - strlen(s);

			if (i > 0)
			{
				cbd[i] = 0;
				strcat(s, cbd);
				cursorpos += i;
			}
			free(cbd);
		}
	}
	else if ((key == K_BACKSPACE))
	{
		if (cursorpos > 0)
		{
			// skip to the end of color sequence

			strcpy (s + cursorpos - 1, s + cursorpos);
			cursorpos--;
			if (cursorpos <= widget->field_start)
				widget->field_start -= ((widget->field_width+1) / 2);
			if (cursorpos < 0)
				cursorpos = 0;
			if (widget->field_start < 0)
				widget->field_start = 0;
		}
	}
	else if (key == K_DEL)
	{
		if (cursorpos < strlen(s))
			strcpy(s + cursorpos, s + cursorpos + 1);
	}
	else if (key == K_INS)
	{
		key_insert = !key_insert;
	}
	else if (key == K_HOME)
	{
		cursorpos = 0;
	}
	else if (key == K_END)
	{
		cursorpos = strlen(s);
	}
	else if (key == K_SPACE && (widget->flags & WIDGET_FLAG_NOSPACE))
	{
		return false;
	}
	else if (key >= 32 && key < 127)
	{
		// color codes
		if (keydown[K_CTRL]) // jitconsole / jittext
		{
			if (toupper(key) == 'K')
				key = SCHAR_COLOR;
			else if (toupper(key) == 'U')
				key = SCHAR_UNDERLINE;
			else if (toupper(key) == 'I')
				key = SCHAR_ITALICS;
			else if (toupper(key) == 'O')
				key = SCHAR_ENDFORMAT;
		}

		// normal text
		if (cursorpos < sizeof(s)-1)
		{
			int i;

			// check insert mode
			if (key_insert)
			{
				// can't do strcpy to move string to right
				i = strlen(s);

				if (i == 254) 
					i--;

				for (; i >= cursorpos; i--)
					s[i + 1] = s[i];
			}

			// only null terminate if at the end
			i = s[cursorpos];
			s[cursorpos] = key;
			cursorpos++;

			if (!i)
				s[cursorpos] = 0;	
		}
	}

	// put updated values in widget's cvar
	widget->field_cursorpos = cursorpos;

	if (widget->limit > 0)
	{
		if (cursorpos >= widget->limit)
			cursorpos = widget->limit - 1;

		if (strlen(s) > widget->limit)
			s[widget->limit] = 0;
	}

	Cvar_Set(widget->cvar, s);
	field_adjustCursor(widget);
	return true;
}

extern qboolean gamekeydown[256];

qboolean M_Keyup (int key)
{
	static int old_mouse_x, old_mouse_y, old_clicktime;

	if (gamekeydown[key])
		return false;

	if (m_active_bind_command)
	{
		if (key != K_MOUSE1) // linux hack
		{
			m_active_bind_widget = NULL;
			m_active_bind_command = NULL;
			return true;
		}
	}

	switch (key) 
	{
	case K_ENTER:
	case K_KP_ENTER:
		M_KBAction(m_current_menu, M_ACTION_EXECUTE);
		break;
	case K_MOUSE1:
		m_mouse.button_down[1] = false;
		M_MouseAction(m_current_menu, M_ACTION_EXECUTE);

		if (old_mouse_x == m_mouse.x && old_mouse_y == m_mouse.x
			&& curtime - old_clicktime < m_doubleclickspeed->value)
		{
			M_MouseAction(m_current_menu, M_ACTION_DOUBLECLICK);
		}

		old_mouse_x = m_mouse.x;
		old_mouse_y = m_mouse.x;
		old_clicktime = curtime;
		break;
	case K_RIGHTARROW:
		M_AdjustWidget(m_current_menu, 1, false);
		break;
	case K_LEFTARROW:
		M_AdjustWidget(m_current_menu, -1, false);
		break;
	default:
		return false;
		break;
	};

	return true;
}

// returns true if key was handled by the menu
// returns false if not
qboolean M_Keydown (int key)
{
	if (!m_current_menu)
		return false;

	if (m_active_bind_command)
	{
		if (key == K_ESCAPE || key == '`') // jitodo -- is console toggled before this?
		{
			m_active_bind_widget = NULL;
			m_active_bind_command = NULL;
		}
		else
		{
			if (key != K_MOUSEMOVE) // don't try to bind mouse movements!
			{
				int keys[2];

				M_FindKeysForCommand(m_active_bind_command, keys);

				if (keys[1] != -1) // 2 or more binds, so clear them out.
					M_UnbindCommand(m_active_bind_command);

				Key_SetBinding(key, m_active_bind_command);
				m_active_bind_widget->parent->modified = true;
				m_active_bind_widget = NULL;
				m_active_bind_command = NULL;
			}
		}
	}
	else
	{
		menu_widget_t *widget;

		switch (key) 
		{
		case K_ESCAPE:
			M_PopMenu(NULL);
			break;
		case K_ENTER:
		case K_KP_ENTER:
			M_KBAction(m_current_menu, M_ACTION_SELECT);
			break;
		case K_MOUSE1:
			m_mouse.button_down[1] = true;
			M_MouseAction(m_current_menu, M_ACTION_SELECT);
			key = K_ENTER;
			break;
		case K_MOUSEMOVE:
			if (m_mouse.button_down[1])
				M_MouseAction(m_current_menu, M_ACTION_DRAG);
			else
				M_MouseAction(m_current_menu, M_ACTION_HILIGHT);
			break;
		case K_MWHEELUP:
			M_MouseAction(m_current_menu, M_ACTION_SCROLLUP);
			break;
		case K_MWHEELDOWN:
			M_MouseAction(m_current_menu, M_ACTION_SCROLLDOWN);
			break;
		case K_TAB:
			if (keydown[K_SHIFT])
				M_HilightPreviousWidget(m_current_menu);
			else
				M_HilightNextWidget(m_current_menu);
			break;
		case K_UPARROW:
			widget = M_GetActiveWidget(m_current_menu);

			if (widget && widget->type == WIDGET_TYPE_SELECT)
				M_AdjustWidget(m_current_menu, -1, true);
			else
				M_HilightPreviousWidget(m_current_menu);
			break;
		case K_DOWNARROW:
			widget = M_GetActiveWidget(m_current_menu);

			if (widget && widget->type == WIDGET_TYPE_SELECT)
				M_AdjustWidget(m_current_menu, 1, true);
			else
				M_HilightNextWidget(m_current_menu);
			break;
		case K_RIGHTARROW:
			M_AdjustWidget(m_current_menu, 1, true);
			break;
		case K_LEFTARROW:
			M_AdjustWidget(m_current_menu, -1, true);
			break;
		default:
			// insert letter into field, if one is active
			if (!M_InsertField(key))
				return false;
			break;
		}
	}

	return !m_current_menu->allow_game_input;
}



//xrichardx: multi column listview. Gets the elements in a row without the cvar at the beginning
static char** listview_get_row_items (int columncount, char **buf)
{
	int element;
	char *token;
	char **items;
	
	items = Z_Malloc(sizeof(char*) * columncount);
	memset(items, 0, sizeof(char*) * columncount);

	for (element = 0; element < columncount; element++)
	{
		token = COM_Parse(buf); //xricharx TODO: tokens like "" wont be returned, so there is no easy way to get empty space

		if (!token || !*token || Q_streq(token, "end")) //should not happen
		{
			int i = 0;
			for (i = 0; i < element; i++)
			{
				/*if (items[i])*/ Z_Free(items[i]);
			}
			/*if (items)*/ Z_Free(items);
			return NULL;
		}

		items[element] = (char*)(Z_Malloc((strlen(token) + 1) * sizeof(char)));
		strcpy(items[element], token);
	}
	return items;
}

//xrichardx: compare function for the listview content:
static int listview_compare(char* stra, char* strb, qboolean ascending)
{
	int result;
	int strlena = strlen(stra);
	int strlenb = strlen(strb);
	char * stripped_a = (char*) Z_Malloc(sizeof(char) * (strlena + 1));
	char * stripped_b = (char*) Z_Malloc(sizeof(char) * (strlenb + 1));
	strip_garbage(stripped_a, stra, strlena + 1);
	strip_garbage(stripped_b, strb, strlenb + 1);

	if (strlen(stripped_a) > 0
		&& strlen(stripped_b) > 0
		&& isdigit((unsigned char)stripped_a[0])
		&& isdigit((unsigned char)stripped_b[0]))
	{
		result = atoi(stripped_a) - atoi(stripped_b);
	}
	else
	{
		result = Q_strcasecmp(stripped_a, stripped_b);
	}

	Z_Free(stripped_a);
	Z_Free(stripped_b);
	return ascending ? result : -result;
}

static select_map_list_t *get_new_select_map_list (char *cvar_string, char *string)
{
	select_map_list_t *new_map;

	new_map = Z_Malloc(sizeof(select_map_list_t));
	memset(new_map, 0, sizeof(select_map_list_t));
	
	if (cvar_string)
	{
		new_map->cvar_string = Z_Malloc(sizeof(char)*(strlen(cvar_string)+1));
		strcpy(new_map->cvar_string, cvar_string);
	}

	if (string)
	{
		new_map->string = Z_Malloc(sizeof(char)*(strlen(string)+1));
		strcpy(new_map->string, string);
	}

	return new_map;
}

// get the list from the file, then store it in an array on the widget
static void select_begin_list (menu_widget_t *widget, char **buf)
{
	char *token;
	char cvar_string[MAX_TOKEN_CHARS];
	int count=0, i;

	select_map_list_t *new_map;
	select_map_list_t *list_start = NULL;
	select_map_list_t *finger;

	if (widget->select_list) //might occur if a "begin pairs ... end" block exists and the "file" attribute is set
	{
		return; //just keep the content of whatever is first in the menu file
	}

	token = COM_Parse(buf);

	if (strstr(token, "pair") || strstr(token, "map") || strstr(token, "bind"))
	{
		widget->flags |= WIDGET_FLAG_USEMAP;

		if (strstr(token, "bind"))
			widget->flags |= WIDGET_FLAG_BIND;

		token = COM_Parse(buf);
	
		// read in map pair
		while (token && *token && !Q_streq(token, "end"))
		{
			strcpy(cvar_string, token);
			token = COM_Parse(buf);
			new_map = get_new_select_map_list(cvar_string, token);
			new_map->next = list_start;
			list_start = new_map;
			count ++;
			token = COM_Parse(buf);
		}

		widget->select_totalitems = count;
		widget->select_map = Z_Malloc(sizeof(char*)*count);
		widget->select_list = Z_Malloc(sizeof(char*)*count);

		if (count > widget->select_rows)
			widget->flags |= WIDGET_FLAG_VSCROLLBAR;

		// put the list into the widget's array, and free the list.
		for (i = count-1; i >= 0; i--, finger = finger->next)
		{
			finger = list_start;
			widget->select_map[i] = finger->cvar_string;
			widget->select_list[i] = finger->string;
			list_start = finger->next;
			Z_Free(finger);
		}
	}
	else if (strstr(token, "single") || strstr(token, "list"))
	{	
		// read in list
		token = COM_Parse(buf);

		while (token && *token && !Q_streq(token, "end"))
		{	
			new_map = get_new_select_map_list(NULL, token);
			new_map->next = list_start;
			list_start = new_map;
			count ++;
			token = COM_Parse(buf);
		}

		widget->select_totalitems = count;
		widget->select_map = NULL;
		widget->select_list = Z_Malloc(sizeof(char*)*count);

		if (count > widget->select_rows)
			widget->flags |= WIDGET_FLAG_VSCROLLBAR;

		// put the list into the widget's array, and free the list.
		for (i = count-1; i >= 0; i--, finger = finger->next)
		{
			finger = list_start;
			widget->select_list[i] = finger->string;
			list_start = finger->next;
			Z_Free(finger);
		}
	}
	else if (strstr(token, "multicolumn")) //xrichardx: multi column listview widget
	{
		/*
		begin multi 5					// 5 columns
		-2 10  10  10  10  10  30		// each column's width
		-1 "1" "2" "3" "4" "5" "6000"	// each column's header
		0  "a" "b" "c" "d" "e" "fooo"	// row text (1. row)
		1  "g" "h" "i" "j" "k" "lmao"	// row text (2. row)
		end
		*/
		listview_row_list_t *start;		//struct containing all cvar_values and mapped rows
		listview_row_list_t *curr_row;
		listview_row_list_t *next_row;
		char** items;					//items in one row. Each string is one element
		int columncount = 0, count = 0, i;
		
		curr_row = (listview_row_list_t*)(Z_Malloc(sizeof(listview_row_list_t)));
		memset(curr_row, 0, sizeof(listview_row_list_t));

		start = curr_row;

		next_row = (listview_row_list_t*)(Z_Malloc(sizeof(listview_row_list_t)));
		memset(next_row, 0, sizeof(listview_row_list_t));

		token = COM_Parse(buf);
		columncount = atoi(token);
		
		token = COM_Parse(buf);

		while (token && *token && !Q_streq(token, "end"))
		{
			next_row->itemcount = 0;

			strcpy(cvar_string, token);

			items = listview_get_row_items(columncount, buf);

			if (!items)
			{
				Z_Free(curr_row);
				Z_Free(next_row);
				return;
			}

			next_row->cvar_string = (char*)(Z_Malloc((strlen(cvar_string) + 1) * sizeof(char)));
			strcpy(next_row->cvar_string, cvar_string);
			next_row->items = (char**)(Z_Malloc (sizeof(char*) * columncount));

			for (i = 0; i < columncount; i++)
			{
				next_row->items[i] = Z_Malloc((strlen(items[i]) + 1) * sizeof(char));
				strcpy(next_row->items[i], items[i]);
				next_row->itemcount += 1;
				Z_Free(items[i]);
			}
			Z_Free(items);

			if (!curr_row || !curr_row->cvar_string)
			{
				curr_row->cvar_string = next_row->cvar_string;
				curr_row->items = next_row->items;
				curr_row->itemcount = next_row->itemcount;
			}
			else
			{
				curr_row->next = next_row;
				curr_row = next_row;
				next_row = (listview_row_list_t*)(Z_Malloc(sizeof(listview_row_list_t)));
				memset(next_row, 0, sizeof(listview_row_list_t));
			}

			count++;

			token = COM_Parse(buf);
		}
		
		widget->listview_rowcount = count;
		widget->listview_columncount = columncount;
		widget->listview_sortorders = (int*)(Z_Malloc(sizeof(int) * columncount));
		memset(widget->listview_sortorders, 0, sizeof(int) * columncount);
		widget->listview_sortedcolumn = -1;
		widget->listview_map = (char**)(Z_Malloc(sizeof(char*)*count));
		widget->listview_list = (char***)(Z_Malloc(sizeof(char**)*count));

		for (i = 0; i < count; i++)
		{
			curr_row = start;
			widget->listview_list[i] = curr_row->items;
			widget->listview_map[i] = curr_row->cvar_string;

			start = curr_row->next;
			Z_Free(curr_row);
		}

	}
}


static void select_strip_from_list (menu_widget_t *widget, const char *striptext_in)
{
	int i, len;
	char *textpos;
	const char *striptext;

	striptext = Cmd_MacroExpandString(striptext_in); // for variables in menu widgets
	len = strlen(striptext);

	for (i = 0; i < widget->select_totalitems; i++)
		if (textpos = strstr(widget->select_list[i], striptext))
			strcpy(textpos, textpos + len);
}


static void select_begin_file_list (menu_widget_t *widget, char *findname)
{
	if (widget->select_list) //might occur if a "begin pairs ... end" block exists and the "file" attribute is set
	{
		return; //just keep the content of whatever is first in the menu file
	}

	widget->select_list = FS_ListFiles(Cmd_MacroExpandString(findname), &widget->select_totalitems, 0, 0, true);
	widget->select_totalitems--;
	widget->flags |= WIDGET_FLAG_FILELIST;
}


static menu_screen_t* M_GetNewMenuScreen (const char *menu_name, const char *background)
{
	menu_screen_t* menu;
	
	menu = Z_Malloc(sizeof(menu_screen_t));
	memset(menu, 0, sizeof(menu_screen_t));
	menu->name = text_copy(menu_name);
	menu->background = background ? re.DrawFindPic(background) : NULL;
	menu->next = root_menu;
	root_menu = menu;

	return menu;
}


static void M_ErrorMenu (menu_screen_t* menu, const char *text)
{
	char err[16];
	sprintf(err, "%c%c%cERROR:", SCHAR_UNDERLINE, SCHAR_COLOR, 'A');
	menu->widget = M_GetNewMenuWidget(WIDGET_TYPE_TEXT, err,
			NULL, NULL, 60, 116, true, false);
	menu->widget->next = M_GetNewMenuWidget(WIDGET_TYPE_TEXT, text,
			NULL, NULL, 60, 124, true, false);
	menu->widget->next->next = M_GetNewMenuWidget(WIDGET_TYPE_TEXT, "Back",
			NULL, "menu pop", 8, 224, true, false);
}


#define DIALOG_WIDTH 260
#define DIALOG_X_PAD 10 // space between border and text
#define DIALOG_Y_PAD 12

static void M_DialogBox (menu_screen_t* menu, const char *text)
{
	char text_wrapped[1024];
	menu_widget_t *widget;
	int linecount = SCR_WordWrapText(text, (DIALOG_WIDTH - (DIALOG_X_PAD * 2)) * hudscale, text_wrapped, sizeof(text_wrapped));

	assert(menu->widget == NULL); // Only a blank menu should be passed in here.

	menu->type = MENU_TYPE_DIALOG;
	widget = menu->widget = M_GetNewMenuWidget(WIDGET_TYPE_PIC, NULL, NULL, NULL, 160, 120, true, false);
	widget->bpic = &bpdata_popup1;
	widget->picwidth = DIALOG_WIDTH;
	widget->picheight = (linecount + 2) * CHARHEIGHT + (DIALOG_Y_PAD * 2); // + 3 to compensate for the OK button
	widget->halign = WIDGET_HALIGN_CENTER;
	widget->valign = WIDGET_VALIGN_MIDDLE;
	widget->next = M_GetNewMenuWidget(WIDGET_TYPE_TEXT, text_wrapped, NULL, NULL, 160 - (DIALOG_WIDTH / 2) + DIALOG_X_PAD, 120 - (CHARHEIGHT * (linecount + 2)) / 2, true, false); // + 3 to compensate for the OK button
	widget = widget->next;
	widget->next = M_GetNewMenuWidget(WIDGET_TYPE_BUTTON, "OK", NULL, "menu pop", 160, 120 + (CHARHEIGHT * (linecount + 2)) / 2 - (CHARHEIGHT * 1), true, false);
	widget = widget->next;
	widget->halign = WIDGET_HALIGN_CENTER;
	widget_complete(widget); // initialize the button bpic.
	widget->hover = true;
}


static void M_DialogBox_f (void)
{
	M_PrintDialog(Cmd_Args());
}


void M_PrintDialog (const char *text)
{
	char translated_text[1024];
	menu_screen_t *menu;
	menu = M_GetNewMenuScreen("dialog", "black_50");
	translate_string(translated_text, sizeof(translated_text), text);
	M_DialogBox(menu, translated_text);
	M_PushMenuScreen(menu, false);
}


static int M_WidgetGetType (const char *s)
{
	if (Q_streq(s, "text"))
		return WIDGET_TYPE_TEXT;
	if (Q_streq(s, "button"))
		return WIDGET_TYPE_BUTTON;
	if (Q_streq(s, "slider"))
		return WIDGET_TYPE_SLIDER;
	if (Q_streq(s, "checkbox"))
		return WIDGET_TYPE_CHECKBOX;
	if (Q_streq(s, "dropdown") || strstr(s, "select"))
		return WIDGET_TYPE_SELECT;
	if (Q_streq(s, "editbox") || Q_streq(s, "field"))
		return WIDGET_TYPE_FIELD;
	if (Q_streq(s, "listview")) //xrichardx: multi colum listview
		return WIDGET_TYPE_LISTVIEW;

	return WIDGET_TYPE_UNKNOWN;
}

static MENU_TYPE M_MenuGetType (const char *s)
{
	if (Q_streq(s, "dialog"))
		return MENU_TYPE_DIALOG;
	else
		return MENU_TYPE_DEFAULT;
}

static int M_WidgetGetAlign (const char *s)
{
	if (Q_streq(s, "left"))
		return WIDGET_HALIGN_LEFT;
	if (Q_streq(s, "center"))
		return WIDGET_HALIGN_CENTER; // Note: Sometimes "center" is used for valign, so WIDGET_HALIGN_CENTER should always == WIDGET_VALIGN_MIDDLE.
	if (Q_streq(s, "right"))
		return WIDGET_HALIGN_RIGHT;

	if (Q_streq(s, "top"))
		return WIDGET_VALIGN_TOP;
	if (Q_streq(s, "middle"))
		return WIDGET_VALIGN_MIDDLE;
	if (Q_streq(s, "bottom"))
		return WIDGET_VALIGN_BOTTOM;

	return WIDGET_HALIGN_LEFT; // default top/left
}


// Finished reading in a widget.  Do whatever we need to do
// to initialize it.
static void widget_complete (menu_widget_t *widget)
{
//	pthread_mutex_lock(&m_mut_widgets); // jitmultithreading
	if (widget->cvar && !widget->cvar_default)
		widget->cvar_default = text_copy("");

	switch(widget->type)
	{
	case WIDGET_TYPE_SLIDER:
		if (!widget->slider_min && !widget->slider_max)
		{
			widget->slider_min = 0.0f;
			widget->slider_max = 1.0f;
		}
		if (!widget->slider_inc)
			widget->slider_inc = (widget->slider_max - widget->slider_min)/24.0;
		break;
	case WIDGET_TYPE_SELECT:
		widget->select_pos = -1;
		//pthread_mutex_unlock(&m_mut_widgets);
		update_select_subwidgets(widget);
		//pthread_mutex_lock(&m_mut_widgets); // jitmultithreading
		select_widget_center_pos(widget);
		//pthread_mutex_unlock(&m_mut_widgets);
		break;
	case WIDGET_TYPE_LISTVIEW: //xrichardx: multi column listview
		widget->select_pos = -1;
		update_listview_subwidgets(widget);
		break;
	case WIDGET_TYPE_FIELD:
		if (widget->field_width < 3)
			widget->field_width = 3; // can't have fields shorter than this!
		break;
	case WIDGET_TYPE_UNKNOWN:
	case WIDGET_TYPE_BUTTON:
	case WIDGET_TYPE_TEXT:
		
		if (widget->text_width && widget->text)
		{
			char wrapped_text[1024];
			char *old_text = widget->text;

			SCR_WordWrapText(widget->text, widget->field_width * hudscale, wrapped_text, sizeof(wrapped_text));
			widget->text = text_copy(wrapped_text);
			Z_Free(old_text);
		}

		if (widget->text && widget_is_selectable(widget) && !widget->pic && !widget->hoverpic && !widget->selectedpic && !(widget->flags & WIDGET_FLAG_NOBG))
		{
#if 1
			// Buttons.
			widget->bpic = &bpdata_button1;
			widget->hoverbpic = &bpdata_button1_hover;
			widget->selectedbpic = &bpdata_button1_select;

			if (!widget->picwidth)
				widget->picwidth = strlen_noformat(widget->text) * TEXT_WIDTH_UNSCALED; // TODO: Handle variable-width font.

			if (!widget->picheight)
				widget->picheight = TEXT_HEIGHT_UNSCALED;

			widget->type = WIDGET_TYPE_BUTTON;
#else
			// Old text hover behavior
			widget->selectedpic = re.DrawFindPic("text1bg");
			widget->hoverpic = re.DrawFindPic("text1bgh");
			widget->picwidth = strlen_noformat(widget->text) * TEXT_WIDTH_UNSCALED;
			widget->picheight = TEXT_HEIGHT_UNSCALED;
#endif
		}
		break;
	}
	//pthread_mutex_unlock(&m_mut_widgets);
}


static void menu_from_file (menu_screen_t *menu, qboolean include, const char *loadname)
{
	char menu_filename[MAX_QPATH];
	char *filebuf;
	int file_len;
	extern cvar_t *cl_menu;
	extern cvar_t *cl_language;
	const char *name = menu->name;

	menu->from_file = true;

	if (loadname)
		name = loadname;

	scale = cl_hudscale->value;

	Com_sprintf(menu_filename, sizeof(menu_filename), "menus/%s/%s/%s.txt", cl_menu->string, cl_language->string, name);
	file_len = FS_LoadFileZ(menu_filename, (void **)&filebuf);

	if (file_len < 0)
	{
		Com_sprintf(menu_filename, sizeof(menu_filename), "menus/%s/%s.txt", cl_menu->string, name);
		file_len = FS_LoadFileZ(menu_filename, (void **)&filebuf);

		if (file_len < 0)
		{
			Com_sprintf(menu_filename, sizeof(menu_filename), "menus/%s.txt", name);
			file_len = FS_LoadFileZ(menu_filename, (void **)&filebuf);
		}
	}

	if (file_len != -1)
	{
		char *token;
		char *buf = filebuf;

		// check for header: "pb2menu 1"
		token = COM_Parse(&buf); // "pb2menu"

		if (Q_streq(token, "pb2menu"))
		{
			token = COM_Parse(&buf); // "1"

			if (atoi(token) == 1)
			{
				menu_widget_t *widget = NULL;
				int x = 0, y = 0;

				if (include && menu->widget) // if this is included/imported, pick up where we left off.
				{
					widget = menu->widget;

					while (widget->next)
						widget = widget->next;
				}

				token = COM_Parse(&buf); 

				while (*token)
				{
					// Background props:
					if (Q_streq(token, "background") && !widget)
					{
						token = COM_Parse(&buf);

						if (Q_streq(token, "none"))
							menu->background = NULL;
						else
							menu->background = re.DrawFindPic(token);
					}

					// Should the game have input while this menu is up?
					if (Q_streq(token, "allowgameinput") && !widget)
					{
						menu->allow_game_input = true;
					}

					// new widget:
					if (Q_streq(token, "widget"))
					{
						if (!widget)
						{
							widget = menu->widget = M_GetNewBlankMenuWidget();
						}
						else
						{
							//pthread_mutex_unlock(&m_mut_widgets);
							widget_complete(widget); // semaphore handled internally here
							//pthread_mutex_lock(&m_mut_widgets); // jitmultithreading
							widget = widget->next = M_GetNewBlankMenuWidget();
						}
						widget->y = y;
						widget->x = x;
						//y += 8;
					}

					if (Q_streq(token, "include") || Q_streq(token, "import"))
					{
						menu_from_file(menu, true, COM_Parse(&buf));

						// We may have added new widgets, so move to the end of the list.
						if (widget)
						{
							while (widget->next)
								widget = widget->next;
						}
						else
						{
							widget = menu->widget;
						}
					}

					// widget properties:
					else if (Q_streq(token, "type"))
					{
						if (widget)
							widget->type = M_WidgetGetType(COM_Parse(&buf));
						else
							menu->type = M_MenuGetType(COM_Parse(&buf));
					}
					else if (Q_streq(token, "password") && widget)
						widget->flags |= WIDGET_FLAG_PASSWORD;
					else if (Q_streq(token, "selected") && widget)
						widget->selected = true;
					else if (Q_streq(token, "nospace") && widget)
						widget->flags |= WIDGET_FLAG_NOSPACE;
					else if (Q_streq(token, "limit"))
					{
						char *text = COM_Parse(&buf);

						if (widget)
							widget->limit = atoi(text);
					}
					else if (Q_streq(token, "text") && widget)
					{
						char *text = COM_Parse(&buf);
						char translated_text[1024];

						if (strchr(text, '$'))
							widget->dynamic = true;

						translate_string(translated_text, sizeof(translated_text), text);
						widget->text = text_copy(translated_text);
					}
					else if (Q_streq(token, "hovertext") && widget)
					{
						char *text = COM_Parse(&buf);

						if (strchr(text, '$'))
							widget->dynamic = true;

						widget->hovertext = text_copy(text);
					}
					else if (Q_streq(token, "selectedtext") && widget)
					{
						char *text = COM_Parse(&buf);

						if (strchr(text, '$'))
							widget->dynamic = true;

						widget->selectedtext = text_copy(text);
					}
					else if (Q_streq(token, "cvar") && widget)
					{
						char *text = COM_Parse(&buf);

						if (strchr(text, '$'))
							widget->dynamic = true;

						widget->cvar = text_copy(text);
					}
					else if (Q_streq(token, "cvar_default") && widget)
					{
						char *text = COM_Parse(&buf);

						if (strchr(text, '$'))
							widget->dynamic = true;

						widget->cvar_default = text_copy(text);
					}
					else if (Q_streq(token, "command") || Q_streq(token, "cmd"))
					{
						if (widget)
							widget->command = text_copy(COM_Parse(&buf));
						else
							menu->command = text_copy(COM_Parse(&buf));
					}
					else if (Q_streq(token, "doubleclick") && widget)
					{
						widget->doubleclick = text_copy(COM_Parse(&buf));
					}
					else if (Q_streq(token, "pic") && widget)
					{
						char *picname = COM_Parse(&buf);

						if (strchr(picname, '$'))
						{
							widget->dynamic = true;
							widget->picname = text_copy(picname);
						}

						widget->pic = re.DrawFindPic(Cmd_MacroExpandString(picname));
						
						// default to double resolution, since it's too blocky otherwise.
						if (!widget->picwidth)
							widget->picwidth = widget->pic->width / 2.0f;

						if (!widget->picheight)
							widget->picheight = widget->pic->height / 2.0f;
					}
					else if (Q_streq(token, "missingpic") && widget)
					{
						char *picname = COM_Parse(&buf);
						widget->missingpic = re.DrawFindPic(Cmd_MacroExpandString(picname));
					}
					else if ((Q_streq(token, "bpic") || Q_streq(token, "borderpic")) && widget)
					{
						char picname[256];
						Q_strncpyz(picname, COM_Parse(&buf), sizeof(picname)); // we have to copy this here, because COM_Parse gets used later and screws up this string.
						widget->bpic = CL_FindBPic(picname);
					}
					else if ((Q_streq(token, "hoverbpic") || Q_streq(token, "hoverborderpic")) && widget)
					{
						char picname[256];
						Q_strncpyz(picname, COM_Parse(&buf), sizeof(picname)); // we have to copy this here, because COM_Parse gets used later and screws up this string.
						widget->hoverbpic = CL_FindBPic(picname);
					}
					else if ((Q_streq(token, "selectedbpic") || Q_streq(token, "selectedborderpic")) && widget)
					{
						char picname[256];
						Q_strncpyz(picname, COM_Parse(&buf), sizeof(picname)); // we have to copy this here, because COM_Parse gets used later and screws up this string.
						widget->selectedbpic = CL_FindBPic(picname);
					}
					else if (Q_streq(token, "picwidth") && widget)
					{
						widget->picwidth = atoi(COM_Parse(&buf));
					}
					else if (Q_streq(token, "picheight") && widget)
					{
						widget->picheight = atoi(COM_Parse(&buf));
					}
					else if (Q_streq(token, "hoverpic") && widget)
					{
						char *picname = COM_Parse(&buf);

						if (strchr(picname, '$'))
						{
							widget->dynamic = true;
							widget->hoverpicname = text_copy(picname);
						}

						widget->hoverpic = re.DrawFindPic(Cmd_MacroExpandString(picname));

						// default to double resolution, since it's too blocky otherwise.
						if (!widget->hoverpicwidth)
							widget->hoverpicwidth = widget->hoverpic->width / 2.0f;

						if (!widget->hoverpicheight)
							widget->hoverpicheight = widget->hoverpic->height / 2.0f;
					}
					else if (Q_streq(token, "hoverpicwidth") && widget)
					{
						widget->hoverpicwidth = atoi(COM_Parse(&buf));
					}
					else if (Q_streq(token, "hoverpicheight") && widget)
					{
						widget->hoverpicheight = atoi(COM_Parse(&buf));
					}
					else if (Q_streq(token, "selectedpic") && widget)
					{
						char *picname = COM_Parse(&buf);

						if (strchr(picname, '$'))
						{
							widget->dynamic = true;
							widget->selectedpicname = text_copy(picname);
						}

						widget->selectedpic = re.DrawFindPic(Cmd_MacroExpandString(picname));
					}
					else if ((Q_streq(token, "xabs") || Q_streq(token, "xleft") || Q_streq(token, "x")) && widget)
						x = widget->x = atoi(COM_Parse(&buf));
					else if ((Q_streq(token, "yabs") || Q_streq(token, "ytop") || Q_streq(token, "y")) && widget)
						y = widget->y = atoi(COM_Parse(&buf));
					else if (strstr(token, "xcent") && widget)
						x = widget->x = 160 + atoi(COM_Parse(&buf));
					else if (strstr(token, "ycent") && widget)
						y = widget->y = 120 + atoi(COM_Parse(&buf));
					else if (Q_streq(token, "xright") && widget)
						x = widget->x = 320 + atoi(COM_Parse(&buf));
					else if (strstr(token, "ybot") && widget)
						y = widget->y = 240 + atoi(COM_Parse(&buf));
					else if (strstr(token, "xrel") && widget)
						x = widget->x += atoi(COM_Parse(&buf));
					else if (strstr(token, "yrel") && widget)
						y = widget->y += atoi(COM_Parse(&buf));
					else if (Q_streq(token, "halign") && widget)
						widget->halign = M_WidgetGetAlign(COM_Parse(&buf));
					else if (Q_streq(token, "valign") && widget)
						widget->valign = M_WidgetGetAlign(COM_Parse(&buf));
					// slider cvar min, max, and increment
					else if (strstr(token, "min") && widget)
						widget->slider_min = atof(COM_Parse(&buf));
					else if (strstr(token, "max") && widget)
						widget->slider_max = atof(COM_Parse(&buf));
					else if (strstr(token, "inc") && widget)
						widget->slider_inc = atof(COM_Parse(&buf));
					// editbox/field options
					else if ((strstr(token, "width") || strstr(token, "cols")) && widget)
						widget->field_width = atoi(COM_Parse(&buf));
					else if (Q_streq(token, "int") && widget)
						widget->flags |= WIDGET_FLAG_INT; // jitodo
					else if (Q_streq(token, "float") && widget)
						widget->flags |= WIDGET_FLAG_FLOAT; // jitodo
					// select/dropdown options
					else if ((strstr(token, "size") || strstr(token, "rows") || strstr(token, "height")) && widget)
						widget->select_rows = atoi(COM_Parse(&buf)); // will also set listview_visible_rows because they are a union.
					else if ((strstr(token, "lvcolseppadding") || strstr(token, "lvpadding")) && widget)
						widget->listview_column_separator_padding = atoi(COM_Parse(&buf));
					else if (strstr(token, "begin") && widget)
						select_begin_list(widget, &buf); //also parses listview multicolumn blocks
					else if (strstr(token, "file") && widget) // "filedir"
						select_begin_file_list(widget, COM_Parse(&buf));
					else if (Q_streq(token, "serverlist") && widget) // for backwards compatibility
					{
						widget->flags |= WIDGET_FLAG_LISTSOURCE;
						widget->listsource = text_copy("serverlist");
					}
					else if ((Q_streq(token, "listsource") || Q_streq(token, "listsrc")) && widget)
					{
						widget->flags |= WIDGET_FLAG_LISTSOURCE;
						widget->listsource = text_copy(COM_Parse(&buf));
					}
					else if ((Q_streq(token, "name") || Q_streq(token, "id")) && widget)
					{
						widget->name = text_copy(COM_Parse(&buf));
						add_named_widget(widget);
					}
					else if (strstr(token, "strip") && widget)
						select_strip_from_list(widget, COM_Parse(&buf));
					else if ((Q_streq(token, "nobg") || Q_streq(token, "nobackground") || Q_streq(token, "nobutton")) && widget)
						widget->flags |= WIDGET_FLAG_NOBG;

					token = COM_Parse(&buf);
				}

				if (widget)
				{
					//pthread_mutex_unlock(&m_mut_widgets);
					widget_complete(widget); // semaphore handled internally here.
					//pthread_mutex_lock(&m_mut_widgets); // jitmultithreading
				}
			}
			else
			{
				M_ErrorMenu(menu, "Invalid menu version.");
			}
		}
		else
		{
			M_ErrorMenu(menu, "Invalid menu file.");
		}

		FS_FreeFile(filebuf);
	}
	else
	{
		char notfoundtext[MAX_QPATH+10];

		sprintf(notfoundtext, "%s not found.", menu_filename);
		M_ErrorMenu(menu, notfoundtext);
	}
}

static menu_screen_t* M_LoadMenuScreen (const char *menu_name)
{
	menu_screen_t *menu;

	menu = M_GetNewMenuScreen(menu_name, cl_menuback->string);
	menu_from_file(menu, false, NULL);

	return menu;
}

static menu_screen_t* M_FindMenuScreen (const char *menu_name)
{
	menu_screen_t *menu;

	menu = root_menu;

	// look through "cached" menus
	while (menu)
	{
		if (Q_streq(menu_name, menu->name))
			return menu;

		menu = menu->next;
	}

	// not found, load from file:
	return M_LoadMenuScreen(menu_name);
}


static void reload_menu_screen (menu_screen_t *menu)
{
	if (!menu)
		return;

	if (menu->from_file)
	{
		//pthread_mutex_lock(&m_mut_widgets); // jitmultithreading
		menu->widget = free_widgets(menu->widget);
		menu->background = re.DrawFindPic(cl_menuback->string);
		menu_from_file(menu, false, NULL); // reload data from file
		//pthread_mutex_unlock(&m_mut_widgets);
	}
}


// Flag all widgets as modified so they update
void M_RefreshMenu (void)
{
	if (m_initialized)
	{
		menu_screen_t *menu;

		pthread_mutex_lock(&m_mut_widgets); // jitmultithreading

		menu = root_menu;

		while (menu)
		{
			refresh_menu_screen(menu);
			menu = menu->next;
		}

		pthread_mutex_unlock(&m_mut_widgets);
	}
}

void M_RefreshWidget (const char *name, qboolean lock)
{
	menu_widget_t *widget;

	if (lock)
		pthread_mutex_lock(&m_mut_widgets); // jitmultithreading -- jitodo - locks up here if refreshwidget called while another widget refreshing.
	if (widget = hash_get(&named_widgets_hash, name))
		widget->modified = true;

	if (lock)
		pthread_mutex_unlock(&m_mut_widgets);
}

void M_RefreshActiveMenu (void)
{
	pthread_mutex_lock(&m_mut_widgets); // jitmultithreading

	if (m_menudepth)
		refresh_menu_screen(m_menu_screens[m_menudepth-1]);

	pthread_mutex_unlock(&m_mut_widgets);
}

// Load menu scripts back from disk
void M_ReloadMenu (void)
{
	if (m_initialized)
	{
		menu_screen_t *menu;
		
		pthread_mutex_lock(&m_mut_widgets); // jitmultithreading
		m_mouse.cursorpic = i_cursor;
		menu = root_menu;

		while (menu)
		{
			reload_menu_screen(menu);
			menu = menu->next;
		}

		pthread_mutex_unlock(&m_mut_widgets);
	}
}


void M_Menu_f (void)
{
	char *menuname;
	qboolean samelevel = false;

	menuname = Cmd_Argv(1);
	
	if (Q_streq(menuname, "samelevel"))
	{
		menuname = Cmd_Argv(2);
		samelevel = true;
	}

	pthread_mutex_lock(&m_mut_widgets); // jitmultithreading

	if (Q_streq(menuname, "pop") || Q_streq(menuname, "back"))
	{
		M_PopMenu(Cmd_Argv(2));
	}
	else if (Q_streq(menuname, "off") || Q_streq(menuname, "close"))
	{
		M_ForceMenuOff();
	}
	else
	{
		menu_screen_t *menu;
		menu = M_FindMenuScreen(menuname);

		if (menu)
		{
			if (menu->command)
			{
				Cbuf_AddText(Cmd_MacroExpandString(menu->command));
				Cbuf_AddText("\n");
			}

			// hardcoded hack so gamma image is correct:
			if (Q_streq(menuname, "setup_gamma"))
			{
				oldscale = cl_hudscale->value;
				Cvar_Set("cl_hudscale", "2");
			}
			else
			{
				oldscale = 0;
			}

			M_PushMenuScreen(menu, samelevel);
		}
	}

	pthread_mutex_unlock(&m_mut_widgets);
}


// Save the current menu state so it can be restored later (used for tutorial map)
void M_MenuStore_f (void)
{
	m_stored_menu_depth = m_menudepth;
	memcpy(m_stored_menu_screens, m_menu_screens, sizeof(m_stored_menu_screens));
}


// Restore stored menu state
void M_MenuRestore_f (void)
{
	m_menudepth = m_stored_menu_depth;
	// Note: This may be unsafe if something happens to the menu screens that are being pointed to.
	// They should currently stay cached, though.
	memcpy(m_menu_screens, m_stored_menu_screens, sizeof(m_menu_screens));
	cls.key_dest = key_menu;

	if (!m_menudepth) // if there's no menu to restore, just default to the main menu.
	{
		Cbuf_AddText("menu main\n");
	}
}


void M_CreateTemporaryBackground()
{
	m_temporary_background = re.DrawFindPic(cl_menuback->string);
}


void M_Init (void)
{
	memset(&m_mouse, 0, sizeof(m_mouse));
	m_mouse.cursorpic = i_cursor;
	// jitodo - mutex for widget updating
	pthread_mutex_init(&m_mut_widgets, NULL);

	// Init hash table:
	hash_table_init(&named_widgets_hash, 0x40, NULL);

    // Add commands
	Cmd_AddCommand("menu", M_Menu_f);
	Cmd_AddCommand("menu_reload", M_ReloadMenu); // Probably not a good reason to use this, but leaving it for backward compatibility
	Cmd_AddCommand("menu_refresh", M_RefreshActiveMenu);
	Cmd_AddCommand("dialog", M_DialogBox_f);
	Cmd_AddCommand("menu_store", M_MenuStore_f);
	Cmd_AddCommand("menu_restore", M_MenuRestore_f);
	m_initialized = true;
}


static void M_DrawSlider (int x, int y, float pos, SLIDER_SELECTED slider_hover, SLIDER_SELECTED slider_selected)
{
	int xorig;

	xorig = x;

	// left arrow:
	if (slider_selected == SLIDER_SELECTED_LEFTARROW)
		re.DrawStretchPic2(x, y, SLIDER_BUTTON_WIDTH, SLIDER_BUTTON_HEIGHT, i_slider1ls);
	else if (slider_hover == SLIDER_SELECTED_LEFTARROW)
		re.DrawStretchPic2(x, y, SLIDER_BUTTON_WIDTH, SLIDER_BUTTON_HEIGHT, i_slider1lh);
	else
		re.DrawStretchPic2(x, y, SLIDER_BUTTON_WIDTH, SLIDER_BUTTON_HEIGHT, i_slider1l);

	x += SLIDER_BUTTON_WIDTH;

	// tray:
	if (slider_hover == SLIDER_SELECTED_TRAY)
		re.DrawStretchPic2(x, y, SLIDER_TRAY_WIDTH, SLIDER_TRAY_HEIGHT, i_slider1th);
	else
		re.DrawStretchPic2(x, y, SLIDER_TRAY_WIDTH, SLIDER_TRAY_HEIGHT, i_slider1t);

	x += SLIDER_TRAY_WIDTH;

	// right arrow
	if (slider_selected == SLIDER_SELECTED_RIGHTARROW)
		re.DrawStretchPic2(x, y, SLIDER_BUTTON_WIDTH, SLIDER_BUTTON_HEIGHT, i_slider1rs);
	else if (slider_hover == SLIDER_SELECTED_RIGHTARROW)
		re.DrawStretchPic2(x, y, SLIDER_BUTTON_WIDTH, SLIDER_BUTTON_HEIGHT, i_slider1rh);
	else
		re.DrawStretchPic2(x, y, SLIDER_BUTTON_WIDTH, SLIDER_BUTTON_HEIGHT, i_slider1r);

	// knob:
	x = xorig + SLIDER_BUTTON_WIDTH + pos*(SLIDER_TRAY_WIDTH-SLIDER_KNOB_WIDTH);

	if (slider_selected == SLIDER_SELECTED_TRAY ||
		slider_hover == SLIDER_SELECTED_KNOB ||	slider_selected == SLIDER_SELECTED_KNOB)
		re.DrawStretchPic2(x, y, SLIDER_KNOB_WIDTH, SLIDER_KNOB_HEIGHT, i_slider1bs);
	else if (slider_hover == SLIDER_SELECTED_TRAY || slider_hover == SLIDER_SELECTED_KNOB)
		re.DrawStretchPic2(x, y, SLIDER_KNOB_WIDTH, SLIDER_KNOB_HEIGHT, i_slider1bh);
	else
		re.DrawStretchPic2(x, y, SLIDER_KNOB_WIDTH, SLIDER_KNOB_HEIGHT, i_slider1b);
}


static float M_SliderGetPos(menu_widget_t *widget)
{
	float sliderdiff;
	float retval = 0.0f;

	// if they forgot to set the slider max
	if (widget->slider_max == widget->slider_min)
		widget->slider_max++;

	sliderdiff = widget->slider_max - widget->slider_min;

	if (widget->cvar)
		retval = Cvar_Get(widget->cvar, widget->cvar_default, CVAR_ARCHIVE)->value;
	else
		retval = 0;

	if (sliderdiff)
		retval = (retval - widget->slider_min) / sliderdiff;

	if (retval > 1.0f)
		retval = 1.0f;

	if (retval < 0.0f)
		retval = 0.0f;

	return retval;
}


static void M_DrawCheckbox (int x, int y, qboolean checked, qboolean hover, qboolean selected)
{
	if (checked)
	{
		if (selected)
			re.DrawStretchPic2(x, y, CHECKBOX_WIDTH, CHECKBOX_HEIGHT, i_checkbox1us);
		else if (hover)
			re.DrawStretchPic2(x, y, CHECKBOX_WIDTH, CHECKBOX_HEIGHT, i_checkbox1ch);
		else
			re.DrawStretchPic2(x, y, CHECKBOX_WIDTH, CHECKBOX_HEIGHT, i_checkbox1c);
	}
	else
	{
		if (selected)
			re.DrawStretchPic2(x, y, CHECKBOX_WIDTH, CHECKBOX_HEIGHT, i_checkbox1cs);
		else if (hover)
			re.DrawStretchPic2(x, y, CHECKBOX_WIDTH, CHECKBOX_HEIGHT, i_checkbox1uh);
		else
			re.DrawStretchPic2(x, y, CHECKBOX_WIDTH, CHECKBOX_HEIGHT, i_checkbox1u);
	}
}


static void M_DrawField (menu_widget_t *widget)
{
	//M_DrawField(widget->widgetCorner.x, widget->widgetCorner.y,
			//	widget->field_width, cvar_val, widget->hover, widget->selected);
	int width, x, y;
	char *cvar_string;
	char temp;
	int nullpos;
	char pass_str[64] = "***************************************************************";
	int start_pos = 0;

	if (widget->cvar)
		cvar_string = Cvar_Get(widget->cvar, widget->cvar_default, CVAR_ARCHIVE)->string;
	else
		cvar_string = "";

	if (widget->flags & WIDGET_FLAG_PASSWORD)
	{
		int len = strlen(cvar_string);

		if (len > 63)
			len = 63;

		pass_str[len] = 0;
		cvar_string = pass_str;
	}

	width = widget->field_width;
	x = widget->widgetCorner.x;
	y = widget->widgetCorner.y;

	if (width > 2)
		width -= 2;
	else
		width = 1;

	if (widget->selected)
	{
		re.DrawStretchPic2(x, y, FIELD_LWIDTH, FIELD_HEIGHT, i_field1ls);
		re.DrawStretchPic2(x+FIELD_LWIDTH, y, TEXT_WIDTH*width, FIELD_HEIGHT, i_field1ms);
		re.DrawStretchPic2(x+FIELD_LWIDTH+TEXT_WIDTH*width, y, FIELD_LWIDTH, FIELD_HEIGHT, i_field1rs);
	}
	else if (widget->hover)
	{
		re.DrawStretchPic2(x, y, FIELD_LWIDTH, FIELD_HEIGHT, i_field1lh);
		re.DrawStretchPic2(x+FIELD_LWIDTH, y, TEXT_WIDTH*width, FIELD_HEIGHT, i_field1mh);
		re.DrawStretchPic2(x+FIELD_LWIDTH+TEXT_WIDTH*width, y, FIELD_LWIDTH, FIELD_HEIGHT, i_field1rh);
	}
	else
	{
		re.DrawStretchPic2(x, y, FIELD_LWIDTH, FIELD_HEIGHT, i_field1l);
		re.DrawStretchPic2(x + FIELD_LWIDTH, y, TEXT_WIDTH * width, FIELD_HEIGHT, i_field1m);
		re.DrawStretchPic2(x + FIELD_LWIDTH + TEXT_WIDTH * width, y, FIELD_LWIDTH, FIELD_HEIGHT, i_field1r);
	}

	// draw only the portion of the string that fits within the field:
	//if (strlen_noformat(cvar_string) > widget->field_start + widget->field_width)
	{
		// TODO: Add color code back in if it was truncated.
		start_pos = str_actual_format_pos(cvar_string, widget->field_start);
		nullpos = str_actual_format_pos(cvar_string + start_pos, widget->field_width) + start_pos;
		temp = cvar_string[nullpos];
		cvar_string[nullpos] = 0;
		re.DrawString(x + (FIELD_LWIDTH-TEXT_WIDTH), y + (FIELD_HEIGHT - TEXT_HEIGHT) / 2,
			cvar_string + start_pos);
		cvar_string[nullpos] = temp;
	}
	/*else
	{
		re.DrawString(x+(FIELD_LWIDTH-TEXT_WIDTH), y+(FIELD_HEIGHT-TEXT_HEIGHT)/2,
			cvar_string+widget->field_start);
	}*/

	if (widget->selected || widget->hover)
	{
		Con_DrawCursor(x + (FIELD_LWIDTH - TEXT_WIDTH) +
			str_visible_pos(cvar_string, (widget->field_cursorpos - start_pos)) * TEXT_WIDTH, y + (FIELD_HEIGHT - TEXT_HEIGHT) / 2);
	}
}


static void M_DrawWidget (menu_widget_t *widget)
{
	const char *text = NULL;
	char *cvar_val = "";
	image_t *pic = NULL;
	bordered_pic_data_t *bpic = NULL;
	qboolean checkbox_checked;
	char wordwrappedtext[1024];

	M_UpdateDrawingInformation(widget);

	if (widget->enabled)
	{
		if (widget->pic)
		{
			if (widget->dynamic && widget->picname)
				pic = re.DrawFindPic(Cmd_MacroExpandString(widget->picname));
			else
				pic = widget->pic;

			if (widget->missingpic)
			{
				if (pic == re.DrawFindPic("***r_notexture***"))
					pic = widget->missingpic;
			}
		}

		if (widget->bpic)
			bpic = widget->bpic;

		if (widget->text)
		{
			if (widget->dynamic)
			{
				text = Cmd_MacroExpandString(widget->text);

				if (widget->text_width)
				{
					SCR_WordWrapText(text, (float)widget->text_width * hudscale, wordwrappedtext, sizeof(wordwrappedtext));
					text = wordwrappedtext;
				}
			}
			else
			{
				text = widget->text;
			}
		}

		// Update text/pic for hovering/selection
		if (widget->selected)
		{
			if (widget->selectedtext)
			{
				if (widget->dynamic)
					text = Cmd_MacroExpandString(widget->selectedtext);
				else
					text = widget->selectedtext;
			}
			else if (widget->text)
			{
				if (widget->dynamic)
					text = va("%c%c%s", SCHAR_COLOR, 214, Cmd_MacroExpandString(widget->text));
				else
					text = va("%c%c%s", SCHAR_COLOR, 214, widget->text);
			}

			if (widget->selectedpic)
			{
				if (widget->dynamic && widget->selectedpicname)
					pic = re.DrawFindPic(Cmd_MacroExpandString(widget->selectedpicname));
				else
					pic = widget->selectedpic;
			}
			else if (widget->hoverpic)
			{
				if (widget->dynamic && widget->hoverpicname)
					pic = re.DrawFindPic(Cmd_MacroExpandString(widget->hoverpicname));
				else
					pic = widget->hoverpic;
			}
			else if (widget->pic)
			{
				if (widget->dynamic && widget->picname)
					pic = re.DrawFindPic(Cmd_MacroExpandString(widget->picname));
				else
					pic = widget->pic;
			}

			if (widget->selectedbpic)
				bpic = widget->selectedbpic;
			else if (widget->hoverbpic)
				bpic = widget->hoverbpic;
		}
		else if (widget->hover)
		{
			if (widget->hovertext)
			{
				if (widget->dynamic)
					text = Cmd_MacroExpandString(widget->hovertext);
				else
					text = widget->hovertext;
			}
			else if (widget->text)
			{
				if (widget->dynamic)
					text = va("%c%c%s", SCHAR_COLOR, 218, Cmd_MacroExpandString(widget->text));
				else
					text = va("%c%c%s", SCHAR_COLOR, 218, widget->text);
			}
			
			if (widget->hoverpic)
			{
				if (widget->dynamic && widget->hoverpicname)
					pic = re.DrawFindPic(Cmd_MacroExpandString(widget->hoverpicname));
				else
					pic = widget->hoverpic;
			}
			else if (widget->pic)
			{
				if (widget->dynamic && widget->picname)
					pic = re.DrawFindPic(Cmd_MacroExpandString(widget->picname));
				else
					pic = widget->pic;
			}

			if (widget->hoverbpic)
				bpic = widget->hoverbpic;
		}

		switch (widget->type) 
		{
		case WIDGET_TYPE_SLIDER:
			M_DrawSlider(widget->widgetCorner.x, widget->widgetCorner.y, M_SliderGetPos(widget),
				widget->slider_hover, widget->slider_selected);
			break;
		case WIDGET_TYPE_CHECKBOX:
			if (widget->cvar && Cvar_Get(widget->cvar, widget->cvar_default, CVAR_ARCHIVE)->value)
				checkbox_checked = true;
			else
				checkbox_checked = false;

			M_DrawCheckbox(widget->widgetCorner.x, widget->widgetCorner.y, 
				checkbox_checked, widget->hover, widget->selected);
			break;
		case WIDGET_TYPE_FIELD:
			M_DrawField(widget);
			break;
		case WIDGET_TYPE_SELECT:
		case WIDGET_TYPE_LISTVIEW:
			break;
		default:
			break;
		}

		if (pic)
			re.DrawStretchPic2(widget->widgetCorner.x,
								widget->widgetCorner.y,
								widget->widgetSize.x,
								widget->widgetSize.y,
								pic);

		if (bpic)
		{
			if (widget->type == WIDGET_TYPE_BUTTON)
			{
				re.DrawBorderedPic(bpic, widget->widgetCorner.x - BUTTON_H_PADDING * scale, widget->widgetCorner.y - BUTTON_V_PADDING * scale,
					widget->widgetSize.x + BUTTON_H_PADDING * scale * 2.0f, widget->widgetSize.y + BUTTON_V_PADDING * scale * 2.0f, scale / 2.0f, 1.0f);
			}
			else
			{
				re.DrawBorderedPic(bpic, widget->widgetCorner.x, widget->widgetCorner.y,
					widget->widgetSize.x, widget->widgetSize.y, scale / 2.0f, 1.0f);
			}
		}

		if (text)
			re.DrawString(widget->textCorner.x, widget->textCorner.y, text);
	}

	// Draw subwidgets
	if (widget->subwidget)
	{
		menu_widget_t *subwidget;

		subwidget = widget->subwidget;

		while (subwidget)
		{
			M_DrawWidget(subwidget);
			subwidget = subwidget->next;
		}
	}
}

static void M_DrawCursor (void)
{
	re.DrawStretchPic2(m_mouse.x-CURSOR_WIDTH/2, m_mouse.y-CURSOR_HEIGHT/2,
		CURSOR_WIDTH, CURSOR_HEIGHT, m_mouse.cursorpic);
}

static void draw_menu_screen (menu_screen_t *menu)
{
	menu_widget_t *widget;

	m_current_menu = menu; // set the global pointer

	if (menu->background)
		M_DrawBackground(menu->background);
	else if (menu->use_temporary_background)
		M_DrawBackground(m_temporary_background);

	widget = menu->widget;

	while(widget)
	{
		M_DrawWidget(widget);
		widget = widget->next;
	}

	// waiting for user to press bind:
	if (m_active_bind_command)
	{
		re.DrawFadeScreen ();
		M_DrawWidget(m_active_bind_widget);
	}

	M_DrawCursor();
}

static void draw_menu_at_depth (int depth)
{
	menu_screen_t *menu;

	if (depth > 0 && depth <= m_menudepth)
	{
		menu = m_menu_screens[depth-1];

		// if a dialog, draw what's behind it first:
		if (menu->type == MENU_TYPE_DIALOG && depth > 1)
		{
			draw_menu_at_depth(depth-1);
		}
		else
		{
			//Now, we've reached the first non-dialog menu
			if (cls.state != ca_active && (menu->background == NULL || Q_streq(menu->background->name, "***r_notexture***")))
			{
				menu->use_temporary_background = true;
			}
			else
			{
				menu->use_temporary_background = false;
			}
		}
		draw_menu_screen(menu);
	}
}

void M_Draw (void)
{
	//if (cls.key_dest == key_menu && m_menudepth)
	if (m_menudepth)
	{
		SCR_DirtyScreen();
		draw_menu_at_depth(m_menudepth);
	}
}


void M_MouseSet (int mx, int my)
{
	m_mouse.x = mx;
	m_mouse.y = my;

	if (m_mouse.x < 0)
		m_mouse.x = 0;
	if (m_mouse.y < 0)
		m_mouse.y = 0;

	if (m_mouse.x > viddef.width)
		m_mouse.x = viddef.width;
	if (m_mouse.y > viddef.height)
		m_mouse.y = viddef.height;
}


void M_MouseMove (int mx, int my)
{
	M_MouseSet(m_mouse.x + mx, m_mouse.y + my);
}


qboolean M_MenuActive()
{
	return (m_menudepth != 0);
}


void M_PlayMenuSounds ()
{
	static qboolean first_frame = true;

	//  Don't play a menu sound on the first frame (since it's always going to open a menu on init)
	if (first_frame)
	{
		first_frame = false;
	}
	else
	{
		if (m_menu_sound_flags & MENU_SOUND_FLAG_OPEN)
			S_StartLocalSound("misc/menu1.wav");

		if (m_menu_sound_flags & MENU_SOUND_FLAG_SELECT)
			S_StartLocalSound("misc/menu2.wav");

		if (m_menu_sound_flags & MENU_SOUND_FLAG_CLOSE)
			S_StartLocalSound("misc/menu3.wav");

		if (m_menu_sound_flags & MENU_SOUND_FLAG_SLIDER)
			S_StartLocalSound("misc/menu4.wav");
	}

	m_menu_sound_flags = 0;
}

// jitmenu
// ===
