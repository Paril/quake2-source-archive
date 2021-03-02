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

// ===[
// jitmenu

#ifndef _MENU_H_
#define _MENU_H_

#include "client.h"

#ifdef WIN32
///* ACT
// For socket stuff
#include "winsock.h"

// From net_wins.c
int NET_TCPSocket (int port);	
int NET_TCPConnect(SOCKET sockfd, char *net_remote_address, int port);
char *NET_ErrorString (void);

#endif

extern cvar_t *serverlist_source; // jitserverlist
extern cvar_t *serverlist_source2; // jitserverlist
extern cvar_t *serverlist_source3; // jitserverlist
extern cvar_t *serverlist_blacklist;
extern cvar_t *serverlist_udp_source1; // jitserverlist

#define TEXT_WIDTH_UNSCALED		CHARWIDTH
#define TEXT_HEIGHT_UNSCALED	CHARHEIGHT
#define BUTTON_V_PADDING		4
#define BUTTON_H_PADDING		10

#define TEXT_WIDTH		(TEXT_WIDTH_UNSCALED*scale)
#define TEXT_HEIGHT		(TEXT_HEIGHT_UNSCALED*scale)

#define SLIDER_BUTTON_WIDTH_UNSCALED	8
#define SLIDER_BUTTON_HEIGHT_UNSCALED	8
#define SLIDER_TRAY_WIDTH_UNSCALED		32
#define SLIDER_TRAY_HEIGHT_UNSCALED		8
#define SLIDER_KNOB_WIDTH_UNSCALED		8
#define SLIDER_KNOB_HEIGHT_UNSCALED		8
#define SLIDER_TOTAL_WIDTH_UNSCALED		((SLIDER_BUTTON_WIDTH_UNSCALED*2)+SLIDER_TRAY_WIDTH_UNSCALED)
#define SLIDER_TOTAL_HEIGHT_UNSCALED	8

#define SLIDER_BUTTON_WIDTH		(SLIDER_BUTTON_WIDTH_UNSCALED	*scale)
#define SLIDER_BUTTON_HEIGHT	(SLIDER_BUTTON_HEIGHT_UNSCALED	*scale)
#define SLIDER_TRAY_WIDTH		(SLIDER_TRAY_WIDTH_UNSCALED		*scale)
#define SLIDER_TRAY_HEIGHT		(SLIDER_TRAY_HEIGHT_UNSCALED	*scale)
#define SLIDER_KNOB_WIDTH		(SLIDER_KNOB_WIDTH_UNSCALED		*scale)
#define SLIDER_KNOB_HEIGHT		(SLIDER_KNOB_HEIGHT_UNSCALED	*scale)
#define SLIDER_TOTAL_WIDTH		(SLIDER_TOTAL_WIDTH_UNSCALED	*scale)
#define SLIDER_TOTAL_HEIGHT		(SLIDER_TOTAL_HEIGHT_UNSCALED	*scale)

#define CHECKBOX_WIDTH_UNSCALED			8
#define CHECKBOX_HEIGHT_UNSCALED		8

#define CHECKBOX_WIDTH			(CHECKBOX_WIDTH_UNSCALED	*scale)
#define CHECKBOX_HEIGHT			(CHECKBOX_HEIGHT_UNSCALED	*scale)

#define CURSOR_HEIGHT	(16*scale)
#define CURSOR_WIDTH	(16*scale)

#define FIELD_HEIGHT	(12*scale)
#define FIELD_LWIDTH	((2+TEXT_WIDTH_UNSCALED)*scale)
#define FIELD_RWIDTH	((2+TEXT_WIDTH_UNSCALED)*scale)

#define SELECT_HSPACING_UNSCALED	1
#define SELECT_VSPACING_UNSCALED	2

#define SELECT_HSPACING		(SELECT_HSPACING_UNSCALED*scale)
#define SELECT_VSPACING		(SELECT_VSPACING_UNSCALED*scale)

#define SELECT_BACKGROUND_SIZE		4

#define LISTVIEW_VSPACING_UNSCALED	2
#define LISTVIEW_VSPACING			(LISTVIEW_VSPACING_UNSCALED*scale)

#define SCROLL_ARROW_WIDTH_UNSCALED	8
#define SCROLL_ARROW_HEIGHT_UNSCALED 8



// widget flags
#define WIDGET_FLAG_NONE		0
#define WIDGET_FLAG_USEMAP		2
#define WIDGET_FLAG_VSCROLLBAR	4
#define WIDGET_FLAG_HSCROLLBAR	8
#define WIDGET_FLAG_DROPPEDDOWN	16
#define WIDGET_FLAG_NOAPPLY		32 // don't apply changes immediately (jitodo)
#define WIDGET_FLAG_FLOAT		64
#define WIDGET_FLAG_INT			128
#define WIDGET_FLAG_FILELIST	256
#define WIDGET_FLAG_LISTSOURCE	512
#define WIDGET_FLAG_NOBG		1024 // no background for select widget OR no button image for text widget
#define WIDGET_FLAG_BIND		2048 // key binding
#define WIDGET_FLAG_PASSWORD	4096
#define WIDGET_FLAG_NOSPACE		8192 // No spaces allowed in this edit box


typedef enum {
	WIDGET_TYPE_UNKNOWN	= 0,
	WIDGET_TYPE_BUTTON,
	WIDGET_TYPE_SLIDER,
	WIDGET_TYPE_CHECKBOX,
	WIDGET_TYPE_SELECT,
	WIDGET_TYPE_TEXT,
	WIDGET_TYPE_PIC,
	WIDGET_TYPE_FIELD,
	WIDGET_TYPE_VSCROLL,
	WIDGET_TYPE_HSCROLL,
	WIDGET_TYPE_LISTVIEW //xrichardx: multi column list view
} WIDGET_TYPE;

typedef enum {
	MENU_TYPE_DEFAULT = 0,
	MENU_TYPE_DIALOG = 1
} MENU_TYPE;

// Horizontal alignment of widget
typedef enum {
	WIDGET_HALIGN_LEFT		= 0,
	WIDGET_HALIGN_CENTER	= 1,
	WIDGET_HALIGN_RIGHT		= 2
} WIDGET_HALIGN;

// Vertical alignment of widget
typedef enum {
	WIDGET_VALIGN_TOP		= 0,
	WIDGET_VALIGN_MIDDLE	= 1,
	WIDGET_VALIGN_BOTTOM	= 2
} WIDGET_VALIGN;

typedef enum {
	SLIDER_SELECTED_NONE = 0,
	SLIDER_SELECTED_TRAY,
	SLIDER_SELECTED_LEFTARROW,
	SLIDER_SELECTED_RIGHTARROW,
	SLIDER_SELECTED_KNOB
} SLIDER_SELECTED;

typedef struct POINT_S {
	float x;
	float y;
} point_t;

typedef struct RECT_S {
	float bottom;
	float right;
	float left;
	float top;
} rect_t;

typedef struct MENU_MOUSE_S {
	float x;
	float y;
	image_t *cursorpic;
	qboolean button_down[8];
} menu_mouse_t;

typedef enum {
	M_ACTION_NONE = 0,
	M_ACTION_HILIGHT,
	M_ACTION_SELECT,
	M_ACTION_EXECUTE,
	M_ACTION_DOUBLECLICK,
	M_ACTION_SCROLLUP,
	M_ACTION_SCROLLDOWN,
	M_ACTION_DRAG
} MENU_ACTION;

typedef enum { //xrichardx: for ascending and descending sorting.
	LISTVIEW_ASCENDING = 0,
	LISTVIEW_DESCENDING,
} LISTVIEW_SORTORDERS;

typedef struct SELECT_MAP_LIST_S {
	char *cvar_string;
	char *string;
	struct SELECT_MAP_LIST_S *next;
} select_map_list_t;

//xrichardx: multi column list view.
typedef struct LISTVIEW_ROW_LIST_S {
	int itemcount; //items in that row
	char *cvar_string; //the assigned internal value for the row
	char **items; //array of strings, each item = one string.
	struct LISTVIEW_ROW_LIST_S *next;
} listview_row_list_t;

typedef struct MENU_WIDGET_S {
	WIDGET_TYPE type;
	int flags;			// for things like numbersonly
	qboolean modified;	// drawing information changed
	qboolean dynamic;	// contents can change / variables in string
	char *name;			// so you can reference a certain widget by name
	char *command;		// command executed when widget activated
	char *doubleclick;	// command exceuted on double click.
	char *cvar;			// cvar widget reads and/or modifies
	char *cvar_default;	// set cvar to this value if unset
	float x;			// position from 0 (left) to 320 (right)
	float y;			// position from 0 (top) to 240 (bottom)
	int limit;			// max number of characters for edit fields (0 for no limit)
	WIDGET_HALIGN halign;	// horizontal alignment relative to x, y coords
	WIDGET_VALIGN valign;	// vertical alignment relative to x, y coords
	char *text;			// text displayed by widget
	char *hovertext;	// text when mouse over widget
	char *selectedtext;	// text when mouse clicked on widget
	image_t *pic;		// image displayed by widget
	image_t *missingpic;// optional pic to display if the pic can't be found (ex: "No Preview" for mapshots).
	bordered_pic_data_t *bpic; // Bordered pic (for buttons)
	char *picname;		// text name of pic
	image_t *hoverpic;	// image displayed when mouse over widget
	bordered_pic_data_t *hoverbpic;
	char *hoverpicname;
	int hoverpicwidth;
	int hoverpicheight;
	image_t *selectedpic;// image displayed when mouse clicked on widget
	bordered_pic_data_t *selectedbpic;
	char *selectedpicname;
	qboolean enabled;	// for greying out widgets
	qboolean hover;		// mouse is over widget
	qboolean selected;	// widget has 'focus'
	SLIDER_SELECTED slider_hover; // which part of the slider is the mouse over
	SLIDER_SELECTED slider_selected; // which part of the slider is the mouse clicked on?

	float	slider_min;
	float	slider_max;
	float	slider_inc;

	union {
		int	field_cursorpos;
		int select_pos;
		int listview_pos;
		int scrollbar_pos;
		float slider_pos;
	};
	union {
		int	field_width;
		int	select_width;
		int picwidth;		// width to scale image to
		int scrollbar_width;
		int text_width;
		int listview_column_separator_padding;
	};
	union {
		int	field_start;
		int select_vstart; // start point for vertical scroll
		int listview_vstart;
		int scrollbar_visible;
	};
	union {
		int listview_visible_rows; //displayed rows (set by menu file)
		int select_rows;
		int picheight;		// height to scale image to
		int scrollbar_height;
	};
	union {
		int select_totalitems;
		int scrollbar_total;
		int listview_rowcount;
	};
	
	union {
		char *listsource; //select widget list source
		char *listview_source; //listview widget list source
	};

	union {
		char **select_map;
		char **listview_map;
	};

	char	**select_list;
	int		select_hstart; // start point for horizontal scroll.
	
	int		listview_columncount;
	int		listview_totalwidth;
	int		listview_pos_x; //xrichardx: multicolumn listview: contains the column the widget is in.
	int		*listview_sortorders; //xrichardx: ascending/descending. Will be initialized in select_begin_list()
	int		listview_sortedcolumn; //xrichardx: needed so listsources can insert new items at the right location.

	char	***listview_list; //pointer to rows; each row is an array of the entries; each entry is a char array.
	char	***listview_source_list;
	char	**listview_source_map;
	int		listview_source_rowcount;
	int		listview_source_column_count;

	// Drawing Information
	point_t widgetCorner;
	point_t widgetSize;
	point_t textCorner;
	point_t textSize;
	rect_t mouseBoundaries;

	// callback function
	void (*callback)(struct MENU_WIDGET_S *widget);
	void (*callback_doubleclick)(struct MENU_WIDGET_S *widget);
	void (*callback_drag)(struct MENU_WIDGET_S *widget);

	struct MENU_WIDGET_S *parent;
	struct MENU_WIDGET_S *subwidget; // child
	struct MENU_WIDGET_S *next;
} menu_widget_t;

typedef struct MENU_SCREEN_S {
	char			*name;
	char			*command;
	image_t			*background;
	qboolean		use_temporary_background; //temporary background if there is nothing rendered behind so avoid artifacts
	MENU_TYPE		type;
	menu_widget_t	*widget;
	menu_widget_t	*selected_widget;
	menu_widget_t	*hover_widget;
	qboolean		allow_game_input; // if true, input will go to the game while this menu is up
	qboolean		from_file;
	struct MENU_SCREEN_S *next;
} menu_screen_t;

typedef struct M_SERVERLIST_SERVER_S {
	char *servername;
	char *mapname;
	int players;
	int maxplayers;
	int ping;
	int ping_request_time; // time (ms) at which ping was requested.
	int remap;
	netadr_t adr;
} m_serverlist_server_t;

typedef struct M_SERVERLIST_S {
	int numservers;
	int nummapped;
	int actualsize;
	m_serverlist_server_t *server;
	char **ips;
	char **info;
	char ***listview_info; //contains list view table like content (array of rows, each row is an array of elements)
	int listview_info_column_count;
	qboolean sortthisframe;
} m_serverlist_t;

extern cvar_t *cl_hudscale;
extern cvar_t *cl_menuback;

#define MENU_SOUND_FLAG_OPEN	(1 << 0)
#define MENU_SOUND_FLAG_SELECT	(1 << 1)
#define MENU_SOUND_FLAG_CLOSE	(1 << 2)
#define MENU_SOUND_FLAG_SLIDER	(1 << 3)

#define MENU_SOUND_OPEN		(m_menu_sound_flags |= MENU_SOUND_FLAG_OPEN);
#define MENU_SOUND_SELECT	(m_menu_sound_flags |= MENU_SOUND_FLAG_SELECT);
#define MENU_SOUND_CLOSE	(m_menu_sound_flags |= MENU_SOUND_FLAG_CLOSE);
#define MENU_SOUND_SLIDER	(m_menu_sound_flags |= MENU_SOUND_FLAG_SLIDER);

void *free_string_array(char *array[], int size);
char *text_copy(const char *in);

static menu_widget_t *find_widget_under_cursor (menu_widget_t *widget);

extern pthread_mutex_t m_mut_widgets;

#endif

// ]===

