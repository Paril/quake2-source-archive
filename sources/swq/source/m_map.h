
#ifndef __m_map_h
#define __m_map_h

#define		NODEFILE_VERSION			1

//#define		INFINITY		 		65535			
#define		INFINITY				268435456
//#define		kNoPredecessor			INFINITY
#define		kNoPredecessor			65535

//#define		MAX_NODES				1024
#define		MAX_NODES				1024

#define		MAX_NODE_DIST			128
#define		MAX_LINK_DIST			1024
#define		NO_NODES				65535
#define		MAX_NODE_DIST_SQUARED	16384
#define		MAX_LINK_DIST_SQUARED	1048576

enum nodestate {SP_permanent, SP_tentative};

typedef	unsigned short	int			counter_t;
//typedef	unsigned short	int			distance_t;
typedef		int			distance_t;

typedef unsigned short int		nodeid_t;  
typedef enum nodestate			nodestate_t;

struct nodeinfo
{ 
	nodeid_t		iPredecessor;
	distance_t		iDistance;
	nodestate_t		iState;
};
typedef struct nodeinfo    nodeinfo_t;        

typedef struct navigator
{
	counter_t			last[MAX_NODES];
	distance_t			dist[MAX_NODES];
} navigator;


counter_t			node_count;

extern	 distance_t		distance_table[MAX_NODES][MAX_NODES];
extern	 nodeinfo_t		node_info[MAX_NODES];
extern	 vec3_t			node_list[MAX_NODES];
extern navigator		graph[MAX_NODES];		// needed for Navigator
extern	 int			node_flags[MAX_NODES];

void initialize_tables (void);
qboolean make_node (edict_t *self);

float	path_not_time_yet;

#endif
