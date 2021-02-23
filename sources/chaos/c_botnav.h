// basic definitions
#define		MAX_NODES			512
#define		NODE_INFINITY		65535
#define		noPredecessor		NODE_INFINITY

#define		NO_PATH				0
#define		VALID_PATH			1

// node type
#define	ALL_NODES		-1
#define	NORMAL_NODE		0
#define	PLAT_NODE		1
#define BUTTON_NODE		2
#define TELEPORT_NODE	3
#define	TRAIN_NODE		4
#define	REDFLAG_NODE	5
#define BLUEFLAG_NODE	6
#define LADDER_NODE		7
#define	INAIR_NODE		8

// node states
enum nodestate {permanent, tentative};
typedef enum	nodestate	nodestate_t;


// nodeinfo structure
typedef struct
{
	int				predecessor;
	double			dist;
	nodestate_t		state;
} nodeinfo_t ;


// node structure
typedef struct
{
	vec3_t		origin;				//Node location			
	int			flag;				//Flags for special nodes
	int			duckflag;
	double		dist[MAX_NODES];	//Distances to surrounding nodes
} nodes_t;


extern nodes_t		nodes[MAX_NODES];
extern nodeinfo_t	nodeinfo[MAX_NODES];


// functions
qboolean	Bot_FindNode(edict_t *self, float radius, int flag);
int			Bot_FindNodeAtEnt(vec3_t spot);
int RecalculateCurrentNode(edict_t *ent);
void		Bot_PlaceNode(vec3_t spot, int flag, int duckflag);
void		Bot_CalcNode(edict_t *self,int nindex);
int			Bot_ShortestPath (int source, int target);

