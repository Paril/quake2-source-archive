#ifndef __MENU_H
#define __MENU_H

typedef enum 
{
	LEFT,		
	CENTER,		
	RIGHT
} ItemAlign;

typedef enum 
{
	WHITE,	
	YELLOW
} ItemColour;

#define MAX_MENU_ITEMS			21
#define MENU_ITEM_LINE_LEN		27

typedef enum 
{	
	OPT_TOGGLE,	
	OPT_OPTION,
	OPT_SELECT,
	OPT_MENU,
	OPT_FILLER
} OptionType;

typedef struct MenuItemNext
{
	char		itemId;
	char		*text;

	ItemColour	colour;
	ItemAlign   align;

	OptionType	itemType;

	union
	{
		struct
		{
			int			*toggle;
			char		*selectedText;
		} toggle[1];

		struct
		{
			int			menuId;
		} menu[1];

		struct
		{
			int			*option;
			int			*optionList;
			int			curOption;
			int			maxOptions;
		} option[1];

		struct
		{
			int (* func)(void *, int);
		} select[1];

	} type[1];

	struct MenuItemNext *nextItem;
	struct MenuItemNext *prevItem;
} MenuItem;

typedef struct MenuNext
{
	int					menuId;
	MenuItem			*menuItem;

	struct MenuNext		*nextMenu;
} Menu;

// 
//	Menu Prototypes
//
int			Chicken_MenuInsert		(int menuId);
int			Chicken_MenuItemInsert	(int menuId, int itemId, char *text, ItemColour colour, ItemAlign align, OptionType itemType, ...);
int			Chicken_MenuSelect		(void *data, int menuId);
void		Chicken_MenuDisplay		(void *data);
void		Chicken_MenuItemPrev	(void *data);
void		Chicken_MenuItemNext	(void *data);
int			Chicken_MenuItemSelect	(void *data, int itemId);

#endif