// 
// maplist.h 
// 

typedef struct
{    
	char   filename[MAX_QPATH];    //8
//	char   mapname[MAX_QPATH]; //32
	int	   game;
	int	   type;
//	int    min;    
//	int    max;    
	int    timesVisited;
} mapinfo;



void InitMapList(void);
char * GenNextMap(void);
qboolean GenMapListAlive(void);

void PrintMapList(void);
char * FindCurrentMap();







