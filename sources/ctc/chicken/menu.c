#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "g_local.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


//
// ----------------------------------------------------------------------------------------
//
//	Local Global Variables
//
// ----------------------------------------------------------------------------------------
//
static Menu			*menuList		= NULL;
//
// ----------------------------------------------------------------------------------------
//
//	Prototypes
//
// ----------------------------------------------------------------------------------------
//
static Menu		*MenuFind					(int	menuId);
static MenuItem *MenuItemFindFirstOption	(Menu	*menu);


//
// ----------------------------------------------------------------------------------------
//
static MenuItem *MenuItemFindFirstOption(Menu *menu)
{
	MenuItem *first = NULL;
	if (menu)
	{	
		first = menu->menuItem;
		while (first && first->itemType == OPT_FILLER)
			first = first->nextItem;
	}

	return (first);
}

//
// ----------------------------------------------------------------------------------------
//
int	Chicken_MenuSelect(void *data, int	menuId)
{
	edict_t *ent = (edict_t *)data;

	if ((ent->client->currentMenu = MenuFind(menuId)) != NULL)
		ent->client->currentItem = MenuItemFindFirstOption(ent->client->currentMenu);

	return (ent->client->currentMenu?1:0);
}

//
// ----------------------------------------------------------------------------------------
//
int Chicken_MenuInsert(int	menuId)
{
	Menu	*newMenu	= NULL;
	int		retSta		= 0;

	if ((newMenu = (Menu *)gi.TagMalloc(sizeof(*newMenu), TAG_GAME)) == NULL)
		exit(-1);
	else
	{
		memset(newMenu, 0, sizeof(*newMenu));
		newMenu->menuId = menuId;

		if (menuList == NULL)
			menuList = newMenu;
		else
		{
			Menu	*ptr	= menuList;
			Menu    *prev   = NULL;

			while (ptr->menuId < newMenu->menuId && ptr->nextMenu)
			{
				prev = ptr;
				ptr = ptr->nextMenu;
			}

			if (ptr->menuId < newMenu->menuId)
			{
				newMenu->nextMenu = ptr->nextMenu;
				ptr->nextMenu  = newMenu;
			}
			else
			{
				if (prev)
				{
					newMenu->nextMenu = menuList;
					menuList = newMenu;
				}
				else
				{
					newMenu->nextMenu = prev->nextMenu;
					prev->nextMenu = newMenu;
				}
			}
		}
	}

	return (retSta);
}

//
// ----------------------------------------------------------------------------------------
//
static Menu *MenuFind(int	menuId)
{
	Menu	*menu	= menuList;

	while (menu && menu->menuId != menuId)
		menu = menu->nextMenu;

	return (menu);
}

//
// ----------------------------------------------------------------------------------------
//
int Chicken_MenuItemInsert(int	menuId, int itemId, char *text, ItemColour colour, ItemAlign align, OptionType itemType, ...)
{
	Menu		*menu		= MenuFind(menuId);
	MenuItem	*newItem	= NULL;
	int			retSta		= 0;

	if (menu && itemId < MAX_MENU_ITEMS)
	{
		if ((newItem = (MenuItem *)gi.TagMalloc(sizeof(*newItem), TAG_GAME)) == NULL)
			exit(-1);
		else
		{
			memset(newItem, 0, sizeof(*newItem));
			newItem->itemId	= itemId;

			newItem->text = text;

			newItem->colour		= colour;
			newItem->align		= align;
			newItem->itemType	= itemType;

			{	
				va_list marker;

				va_start( marker, itemType );     /* Initialize variable arguments. */

				switch(itemType)
				{
					case OPT_TOGGLE:	
						{
							char *text;

							newItem->type->toggle->toggle = va_arg( marker,int *);

							text = va_arg( marker, char *);
							newItem->type->toggle->selectedText = text;
						}
						break;

					case OPT_OPTION:
						{
							int i;

							newItem->type->option->maxOptions = 0;
							newItem->type->option->option = va_arg( marker,int *);

							i = va_arg( marker, int);
							while( i != -1 )
							{
								newItem->type->option->maxOptions++;
								if ((newItem->type->option->optionList = realloc( newItem->type->option->optionList, 
													newItem->type->option->maxOptions * sizeof( int )) ) ==  NULL )
									exit(-1);

								newItem->type->option->optionList[newItem->type->option->maxOptions - 1] = i;
								
								i = va_arg( marker, int);
							}
							*newItem->type->option->option = newItem->type->option->optionList[newItem->type->option->curOption];

						}
						break;

					case OPT_MENU:
							newItem->type->menu->menuId = va_arg( marker, int);
						break;

					case OPT_SELECT:
							newItem->type->select->func = (void *)(va_arg( marker, void *));
						break;

					case OPT_FILLER:
						break;

					default:
						exit(-1);
				}
				va_end( marker );              /* Reset variable arguments.      */
			}

			if (menu->menuItem == NULL)
				menu->menuItem = newItem;
			else
			{
				MenuItem	*ptr	= menu->menuItem;

				while (ptr->itemId < newItem->itemId  && ptr->nextItem)
					ptr = ptr->nextItem;

				if (ptr->itemId < newItem->itemId)
				{
					newItem->nextItem = ptr->nextItem;
					newItem->prevItem = ptr;

					if (ptr->nextItem)
						ptr->nextItem->prevItem = newItem;

					ptr->nextItem  = newItem;
				}
				else
				{
					if (ptr->prevItem)
					{
						newItem->nextItem			= menu->menuItem;
						menu->menuItem->prevItem	= newItem;
						menu->menuItem				= newItem;
					}
					else
					{
						ptr->prevItem->nextItem = newItem;
						newItem->prevItem = ptr;
						newItem->nextItem = ptr->nextItem;
						ptr->prevItem = newItem;
					}
				}
			}
		}
	}

	return (retSta);
}

//
// ----------------------------------------------------------------------------------------
//
void Chicken_MenuItemNext(void *data)
{
	edict_t		*ent = (edict_t *)data;
	MenuItem	*next = NULL;

	if (ent->client->currentItem)
	{	
		int oneLoop = FALSE;
		next = ent->client->currentItem->nextItem;

		if (!next)
		{
			oneLoop = TRUE;
			next = ent->client->currentMenu->menuItem;
		}

		while (next && next->itemType == OPT_FILLER)
		{
			next = next->nextItem;
			if (!next)
			{
				oneLoop = TRUE;
				next = ent->client->currentMenu->menuItem;
			}
		}

		ent->client->currentItem = next;
	}
}

//
// ----------------------------------------------------------------------------------------
//
void Chicken_MenuItemPrev(void *data)
{
	edict_t		*ent = (edict_t *)data;
	MenuItem	*prev = NULL;

	if (ent->client->currentItem)
	{	
		int oneLoop = FALSE;
		prev = ent->client->currentItem->prevItem;

		if (!prev)
		{
			oneLoop = TRUE;
			prev = ent->client->currentMenu->menuItem;
			while(prev->nextItem)
				prev = prev->nextItem;
		}

		while (prev && prev->itemType == OPT_FILLER)
		{
			prev = prev->prevItem;
			if (!prev)
			{
				oneLoop = TRUE;
				prev = ent->client->currentMenu->menuItem;
				while(prev->nextItem)
					prev = prev->nextItem;
			}
		}
		
		ent->client->currentItem = prev;
	}
}

//
// ----------------------------------------------------------------------------------------
//
int Chicken_MenuItemSelect(void *data, int itemId)
{
	int		retSta = 0;
	char	message[64];
	edict_t *ent = (edict_t *)data;

	switch(ent->client->currentItem->itemType)
	{
		case OPT_TOGGLE:	
			{
				if (*ent->client->currentItem->type->toggle->toggle)
				{
					*ent->client->currentItem->type->toggle->toggle = FALSE;
					sprintf(message, "%s turned %s off\n", 
						ent->client?ent->client->pers.netname:"someone", ent->client->currentItem->text);
				}
				else
				{
					*ent->client->currentItem->type->toggle->toggle = TRUE;
					sprintf(message, "%s turned %s on\n", 
						ent->client?ent->client->pers.netname:"someone", ent->client->currentItem->text);
				}

				gi.bprintf(PRINT_HIGH, message);

			}
			break;

		case OPT_OPTION:
			ent->client->currentItem->type->option->curOption++;
			ent->client->currentItem->type->option->curOption %= ent->client->currentItem->type->option->maxOptions;
			*ent->client->currentItem->type->option->option = ent->client->currentItem->type->option->optionList[ent->client->currentItem->type->option->curOption];
			sprintf(message, "%s changed %s to %d\n", 
				ent->client?ent->client->pers.netname:"someone", ent->client->currentItem->text, *ent->client->currentItem->type->option->option);
			gi.bprintf(PRINT_HIGH, message);
			break;

		case OPT_MENU:
			Chicken_MenuSelect(ent, ent->client->currentItem->type->menu->menuId);
			break;

		case OPT_SELECT:
			retSta = ent->client->currentItem->type->select->func(ent, itemId);
			break;

		case OPT_FILLER:
			break;

		default:
			exit(-1);
	}

	return (retSta);
}

//
// ----------------------------------------------------------------------------------------
//
void Chicken_MenuDisplay(void *data)
{
	edict_t *ent = (edict_t *)data;

	if (ent->client->currentMenu)
	{
		char		string[1400];
		int			itemCount = 0;
		MenuItem	*curItem = ent->client->currentMenu->menuItem;

		sprintf(string, "xv 32 yv 12 picn chicken_options ");

		while(curItem)
		{
			int len = (curItem->text?strlen(curItem->text):0);

			sprintf(string + strlen(string), "yv %d xv %d ", 32 + curItem->itemId * 8, 50);

			switch(curItem->itemType)
			{
				case OPT_FILLER:
					switch (curItem->align)
					{
						case CENTER:
							sprintf(string + strlen(string), "%s \"%-*.*s%-*.*s\" ", 
										curItem->colour == WHITE ? "string "   : "string2", 
										(MENU_ITEM_LINE_LEN-len)/2, (MENU_ITEM_LINE_LEN-len)/2, " ",
										len, len, 
										curItem->text ? curItem->text: " ");
							break;
						case LEFT:
							sprintf(string + strlen(string), "%s \"%-*.*s\" ", 
										curItem->colour == WHITE ? "string "   : "string2", 
										MENU_ITEM_LINE_LEN, MENU_ITEM_LINE_LEN,
										curItem->text ? curItem->text: " ");
							break;
						case RIGHT:
							sprintf(string + strlen(string), "%s \"%*.*s\" ", 
										curItem->colour == WHITE ? "string "   : "string2", 
										MENU_ITEM_LINE_LEN, MENU_ITEM_LINE_LEN,
										curItem->text ? curItem->text: " ");
							break;
					}
					break;

				case OPT_TOGGLE:
				case OPT_SELECT:
				case OPT_MENU:
					switch(curItem->align)
					{
						case CENTER:
							sprintf(string + strlen(string), "%s \"%-1s%-*.*s%-*.*s%-*.*s %3s\" ", 
									
									curItem->colour == WHITE ? "string "   : "string2", 

									curItem == ent->client->currentItem	? "\x0d"		   : " ",

									(MENU_ITEM_LINE_LEN-5-len)/2, (MENU_ITEM_LINE_LEN-5-len)/2, " ",
									len, len, 
									(curItem->itemType == OPT_TOGGLE ?
										(*curItem->type->toggle->toggle ? 
											(curItem->type->toggle->selectedText ? curItem->type->toggle->selectedText:
												(curItem->text ? curItem->text: " ")):
											(curItem->text ? curItem->text: " ")):
										(curItem->text ? curItem->text: " ")),

									(MENU_ITEM_LINE_LEN-5-len)/2, (MENU_ITEM_LINE_LEN-5-len)/2, " ",
									(curItem->itemType == OPT_TOGGLE ?
										(*curItem->type->toggle->toggle ? "on" : "off"):
										" "));
							break;
						case LEFT:
							sprintf(string + strlen(string), "%s \"%-1s%-*.*s %3s\" ", 
									
									curItem->colour == WHITE ? "string "   : "string2", 

									curItem == ent->client->currentItem	? "\x0d"		   : " ",

									MENU_ITEM_LINE_LEN-5, MENU_ITEM_LINE_LEN-5,

									(curItem->itemType == OPT_TOGGLE ?
										(*curItem->type->toggle->toggle ? 
											(curItem->type->toggle->selectedText ? curItem->type->toggle->selectedText:
												(curItem->text ? curItem->text: " ")):
											(curItem->text ? curItem->text: " ")):
										(curItem->text ? curItem->text: " ")),

									(curItem->itemType == OPT_TOGGLE ?
										(*curItem->type->toggle->toggle ? "on" : "off"):
										" "));
							break;
						case RIGHT:
							sprintf(string + strlen(string), "%s \"%-1s%*.*s %3s\" ", 
									
									curItem->colour == WHITE ? "string "   : "string2", 

									curItem == ent->client->currentItem	? "\x0d"		   : " ",

									MENU_ITEM_LINE_LEN-5, MENU_ITEM_LINE_LEN-5,

									(curItem->itemType == OPT_TOGGLE ?
										(*curItem->type->toggle->toggle ? 
											(curItem->type->toggle->selectedText ? curItem->type->toggle->selectedText:
												(curItem->text ? curItem->text: " ")):
											(curItem->text ? curItem->text: " ")):
										(curItem->text ? curItem->text: " ")),

									(curItem->itemType == OPT_TOGGLE ?
										(*curItem->type->toggle->toggle ? "on" : "off"):
										" "));
							break;
					}
					break;

				case OPT_OPTION:
					switch(curItem->align)
					{
						case CENTER:
							sprintf(string + strlen(string), "%s \"%-1s%-*.*s%-*.*s%-*.*s %3d\" ", 
									
									curItem->colour == WHITE ? "string "   : "string2", 

									curItem == ent->client->currentItem	? "\x0d"		   : " ",

									(MENU_ITEM_LINE_LEN-5-len)/2, (MENU_ITEM_LINE_LEN-5-len)/2, " ",
									len, len, curItem->text,
									(MENU_ITEM_LINE_LEN-5-len)/2, (MENU_ITEM_LINE_LEN-5-len)/2, " ",
									*curItem->type->option->option);
							break;
						case LEFT:
							sprintf(string + strlen(string), "%s \"%-1s%-*.*s %3d\" ", 
									curItem->colour == WHITE ? "string "   : "string2", 

									curItem == ent->client->currentItem	? "\x0d"		   : " ",

									MENU_ITEM_LINE_LEN-5, MENU_ITEM_LINE_LEN-5,
									curItem->text,
									*curItem->type->option->option);
							break;
						case RIGHT:
							sprintf(string + strlen(string), "%s \"%-1s%*.*s %3d\" ", 
									curItem->colour == WHITE ? "string "   : "string2", 

									curItem == ent->client->currentItem	? "\x0d"		   : " ",

									MENU_ITEM_LINE_LEN-5, MENU_ITEM_LINE_LEN-5,
									curItem->text,
									*curItem->type->option->option);
							break;
					}
					break;

			}

			itemCount++;
			curItem = curItem->nextItem;
		}


		gi.WriteByte (svc_layout);
		gi.WriteString (string);
		gi.unicast (ent, false);
	}
}
