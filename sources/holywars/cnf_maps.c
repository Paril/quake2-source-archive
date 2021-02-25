// HOLY WARS: maps loader

// #include "g_local.h"
#include "q_shared.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "cnf_maps.h"


// -- defined in CNF_DEFS.c
const char  *CNF_MAP_file_map_load_name = "holywars/config/map_load.ini";

#define MAX_MAPS 100
//      MAX_QPATH       is defined in q_shared.h (actually == 64)
typedef struct
        {
        char    name[MAX_QPATH];        // map name
        int     clients_range[2];       // clients range        [0] = min [1] = max
        int     ping_range[2];          // client's ping mean value range  [0] = min [1] = max
        } CNF_MAP_load_type;

        // space required for MAX_MAPS structs CNF_MAP_load_type
        //      = MAX_MAPS*sizeof(CNF_MAP_load_type)  = 100 * 80 = 8000 bytes
CNF_MAP_load_type CNF_MAP_desc[MAX_MAPS];

        // tech note: change to dinamic memory allocation !


//--------------------------------------------
// func. char * CNF_MAP_GetRange(char *buf, int range[2])
// -------
// loads from buf the range limits
// ------- 1/4/98 --------------------------
// param        : buf           the line
//              : range         array of 2 int  [0]=min [1]=max
// returns      : the char not processed as "range descriptor"
// R static var.: none
// W static var.: none
// extrn func.  : none
//-------------------------------------------
char *CNF_MAP_GetRange(char *buf, int range[2])
 {
     char *open_par;            // pointer at (
     char *close_par;           // pointer at )
     int  min, max;             // min e max readed from buf

     open_par  = strchr(buf, '(');      // it search  (
     close_par = strchr(buf, ')');      // it search  )

                // if the buffer does NOT contain the sequence (...)
                //      THEN it returns the current position
     if (open_par == NULL || close_par == NULL || close_par <= open_par)
        return buf;

                // if after the ( it cannot load 2 integers separated by a comma
                //      THEN it returns the current position
     if(sscanf(open_par+1, "%d, %d", &min, &max) != 2)
        return buf;

                // it stores min and max with lower and upper saturation (0 .. 1000)
     range[0] = (min <= 0)? 0: (min >= 10000)? 10000: min;
     range[1] = (max <= 0)? 0: (max >= 10000)? 10000: max;

                // it returns the byte after the )
     return close_par++;
 }


//--------------------------------------------
// func. int CNF_MAP_LoadMapList(void)
// -------
// loads map rotation definitions
// ------- 1/4/98 --------------------------
// param        : none
// returns      : > 0                   number of map loaded
//              : = 0                   error or no map found
// R static var.: CNF_MAP_file_map_load_name
// W static var.: CNF_MAP_desc
// extrn func.  : CNF_MAP_GetRange
//-------------------------------------------

int CNF_MAP_LoadMapList(void)
  {
  int    map_index = 0;
  FILE   *f;
  char   buff[512], name_found[MAX_QPATH];

 if((f=fopen(CNF_MAP_file_map_load_name, "r"))==NULL)    // no file found
        return 0;

 // loop for each text line until end of file is reached OR map_index >= MAX_MAPS
 while ( map_index < MAX_MAPS            // index doesn't exceed the array limits
      && fgets( buff, 511, f) != NULL)   // loads a line of text into buff
        {
        int i, j;
        char *pnt;      // pnt employed to scan the line

        pnt = strstr(buff, "//");  // looks for comments (//)
        if(pnt != NULL) *pnt = '\0';     // if it finds comments, it cuts the string

        pnt = strchr(buff, '\"');        // it looks for map name initial char (")
        if (pnt == NULL)
                continue;               // if name not found ignore the line

                // -- init map name founded -- start name scan
        pnt++;  // it point the 1st char of the name
        for(i = 0; i<MAX_QPATH-1; i++, pnt++)
                {
// HWv2.1
                char ch = *pnt;
                    // I don't know the chars allowed by id for map names
                    // I see '_' and '$' into the map names
                    // so ... take all chars between ' '(ASCII 32) and '~' (ASCII 126)
                    //          but NOT '\"' ! (this marks the end of the string !)
                if(ch >= ' ' && ch <= '~' && ch != '\"' )
                        name_found[i] = tolower(ch);
// end HWv2.1
                else
                        break;          // exit from the inner for
                }
        name_found[i] = 0;
        if(i >= MAX_QPATH-1 || name_found[0] == 0)
                {       // bad name -> it ignores this line and process the next one
                continue;
                }

                //-- name ok, check if an identical name was loaded
        for(j = 0; j<map_index; j++)
                {
                if(strcmp(CNF_MAP_desc[j].name, name_found) == 0)
                      {       // found !
                      break;
                      }
                 }
         if( j < map_index )
                {               // strcmp has found the map name in the struct ->
                continue;       // it ignores this line and process the next one
                }

                // name ok and unique - it can be stored in struct

        strncpy(CNF_MAP_desc[map_index].name, name_found, MAX_QPATH-1);
        CNF_MAP_desc[map_index].name[MAX_QPATH-1] = 0;     // paranoia

                // reset ranges
        CNF_MAP_desc[map_index].clients_range[0] = 0;
        CNF_MAP_desc[map_index].clients_range[1] = 0;
        CNF_MAP_desc[map_index].ping_range[0]    = 0;
        CNF_MAP_desc[map_index].ping_range[1]    = 0;

                // pnt point at the char after the name
                // scan looking for 'p' or 'c'
        while(*pnt != 0)
                {
                switch(toupper(*pnt))
                        {
                case 'C':       // client range
                        pnt = CNF_MAP_GetRange(pnt+1, CNF_MAP_desc[map_index].clients_range);
                        break;
                case 'P':       // mean value of client's ping range
                        pnt = CNF_MAP_GetRange(pnt+1, CNF_MAP_desc[map_index].ping_range);
                        break;
                default:
                        pnt++;
                        }
                }
        map_index++;    // entry ok and stored
        }
        // file scanning is over
 fclose(f);
 return map_index;
}

//--------------------------------------------
// func. char *CNF_MAP_LoadNextMapName(char *current_map, int client_num, int mean_ping, int n_map)
// -------
// search into the loaded struct for the next map
// ------- 1/4/98 --------------------------
// param        : current_map   string name of the current map
//              : client_num    the number of client connected (< 0 to disable checks)
//              : mean_ping     mean ping of the connected clients (<0 to disable checks)
//              : n_map         number of maps loaded into CNF_MAP_desc[]
// returns      : NULL if it cannot find the next map
//              : next map name (Warning ! It's a pointer to a static array. DO NOT FREE IT !)
// R static var.: CNF_MAP_desc[]
// W static var.: none
// extrn func.  : none
//-------------------------------------------

char *CNF_MAP_LoadNextMapName(char *current_map, int client_num, int mean_ping, int n_map)
 {
 int current_indx, processed;

        // it searches the current map
 for(current_indx = 0; current_indx < n_map; current_indx ++)
        {
        if( stricmp(CNF_MAP_desc[current_indx].name, current_map) == 0)
                {       // found !
                break;
                }
        }

 if(current_indx >= n_map) return NULL;         // map not found

        // starting from the index following the current map
        // it scans the structure array CNF_MAP_desc[] looking for
        // the first map that passes all the enabled checks.
        // If this search process n_map-1 names: -> cannot find next map.
 for(processed = 0; processed < n_map-1; processed++)
        {
            // index has to point to the next map
            //   (the first map follows the last-> circular buffer)
        current_indx++;
        if(current_indx >= n_map)
                current_indx = 0;
           // current_indx point at the next map

                // IF   clients checks are enabled
                //      AND min. clients check is enable
                //      AND clients num is out of range (min > actual)
                // THEN ignore this map (continue scan loop)
        if ( client_num >= 0
          && CNF_MAP_desc[current_indx].clients_range[0] > 0
          && CNF_MAP_desc[current_indx].clients_range[0] > client_num)
                {
                continue;       // ignore this map (continue with the next map)
                }

                // IF   clients checks are enabled
                //      AND max. clients check is enable
                //      AND clients num is out of range  (max < actual)
                // THEN ignore this map (continue scan loop)
        if ( client_num >= 0
          && CNF_MAP_desc[current_indx].clients_range[1] > 0
          && CNF_MAP_desc[current_indx].clients_range[1] < client_num)
                {
                continue;       // ignore this map (continue with the next map)
                }

                // IF   ping checks are enabled
                //      AND min. ping check is enable
                //      AND actual mean ping is out of range  (min > actual)
                // THEN ignore this map (continue scan loop)
        if ( mean_ping >= 0
          && CNF_MAP_desc[current_indx].ping_range[0] > 0
          && CNF_MAP_desc[current_indx].ping_range[0] > mean_ping)
                {
                continue;       // ignore this map (continue with the next map)
                }

                // IF   ping checks are enabled
                //      AND max. ping check is enable
                //      AND actual mean ping is out of range  (max < actual)
                // THEN ignore this map (continue scan loop)
        if ( mean_ping >= 0
          && CNF_MAP_desc[current_indx].ping_range[0] > 0
          && CNF_MAP_desc[current_indx].ping_range[1] > mean_ping)
                {
                continue;       // ignore this map (continue with the next map)
                }

         // ------ if the program reaches this point the map is OK and can be loaded
        return CNF_MAP_desc[current_indx].name;
        }

  // --- if the program reachs this point then NO MAPS are available for loading
  return NULL;
 }


//--------------------------------------------
// func. char *CNF_MAP_LoadNextMapName(char *current_map, int client_num, int mean_ping, int n_map)
// -------
// Load serch struct and search the next map
// ------- 1/4/98 --------------------------
// param        : current_map   string name of the current map
//              : client_num    the number of client connected (< 0 to disable checks)
//              : mean_ping     mean ping of the connected clients (<0 to disable checks)
// returns      : NULL if it cannot find the next map
//              : next map name (Warning ! It's a pointer to a static array. DO NOT FREE IT !)
// R static var.: CNF_MAP_desc[]
// W static var.: CNF_MAP_desc[]
// extrn func.  : CNF_MAP_LoadMapList() and CNF_MAP_LoadNextMapName()
//-------------------------------------------

char *CNF_MAP_LoadMap(char *current_map, int client_num, int mean_ping)
  {
  int n_map;

        // it loads the struct from the configuration file
  n_map = CNF_MAP_LoadMapList();
        // if no name was found in the file
  if( n_map  <= 0)
        return NULL;    // return NO name
  else
        return CNF_MAP_LoadNextMapName(current_map, client_num, mean_ping, n_map);
  }
