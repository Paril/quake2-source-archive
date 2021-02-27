#include "g_local.h"

int numnodes=0;

// NOTE: This size of the pathnode array may have to change if a larger array is needed for your individual maps..

#define MAX_GRID_SIZE	10000
vec3_t pathnode[MAX_GRID_SIZE];

#define maxx 512 // 32 units/node x=[0..255]
#define maxy 512
#define maxz 512 // 16 units/node z=[0..511]

#define xevery 16 // 32=8192/maxx
#define yevery 16
#define zevery 16

#define MaxOf(x,y) ((x) > (y)?(x):(y))
#define MinOf(x,y) ((x) < (y)?(x):(y))
#define gridz(z) (int)MinOf(MaxOf(((z)+4096)*0.06250,0),511)
#define g2v0(x)  (float)MinOf(MaxOf((x)*xevery-4096+(xevery*0.5),-4096),4096)
#define g2v1(y)  (float)MinOf(MaxOf((y)*yevery-4096+(yevery*0.5),-4096),4096)
#define g2v2(z)  (float)MinOf(MaxOf((z)*zevery-4096+(zevery*0.5),-4096),4096)

//=====================================================
//================== pathfinding stuff ================
//=====================================================

// numchilds should be set based on the maximum number of expected child nodes in search pattern
#define NUMCHILDS 12

typedef struct node_s node_t;
typedef struct stack_s stack_t;

struct node_s {
  int   g; // how far we've already gone from start to here
  float h; // heuristic estimate of how far is left
  float f; // is total cost (estimated) from start to finish
  int nodenum; // index number of this node
 // vec3_t origin; // location of this node
  node_t *Child[NUMCHILDS];
  node_t *PrevNode;
  node_t *NextNode;
} ;

node_t *node;

struct stack_s {
  node_t  *NodePtr;
  stack_t *StackPtr;
};

stack_t *Stack=NULL;

//=========================================
// Push Node to front of the linked list
//=========================================
void Push(node_t *Node) {
stack_t *STK;

  STK=(stack_t *)V_Malloc(sizeof(stack_t), TAG_LEVEL);
  STK->StackPtr=Stack; // NULL at start
  STK->NodePtr=Node;   // Tie the Node
  Stack=STK;           // Set to start of Stack
}

//=========================================
// Pop Node from front of the linked list
//=========================================
node_t *Pop(void) {
node_t *tNode;
stack_t *STK;

  STK=Stack;             // Start of Stack
  tNode=Stack->NodePtr;  // Grab this Node.
  Stack=Stack->StackPtr; // Move Stack Pointer
  V_Free(STK);       // Free this node

  return (tNode);
}

node_t *OPEN=NULL;   // Start of OPEN List

node_t *CLOSED=NULL; // Start of CLOSED List

int NodeCount=0;//GHz: for debugging
node_t *NodeList[10000];//GHz: cleanup

//=====================================================
// Check the desired OPEN/CLOSED LIST for NodeNum..
//=====================================================
node_t *CheckLIST(node_t *LIST, int NodeNum) {
node_t *tNode;

  tNode=LIST->NextNode; // Start of OPEN or CLOSED

  while (tNode)
    if (tNode->nodenum == NodeNum) { // My test!!
      return (tNode); } // Found it!
    else
      tNode=tNode->NextNode;

  return NULL; // NodeNum NOT on LIST.
}

void PrintNodes (node_t *Node, qboolean reverse)
{
	int nodeNumber;
	node_t *tNode = Node;

	while (tNode)
	{
		nodeNumber = tNode->nodenum;
		if (nodeNumber < 0 || nodeNumber > numnodes)
			nodeNumber = 9999;
		gi.dprintf("%d->", nodeNumber);
		if (reverse)
			tNode = tNode->PrevNode;
		else
			tNode = tNode->NextNode;
	}
	gi.dprintf("(null)\n");
}

//=========================================
// Free allocated resources for next search
//=========================================
void FreeStack(node_t *PathNode) {
	unsigned long NumFreed=0;//GHz - for debugging
node_t *tNode;
stack_t *STK;

//GHz NOTE: numfreed is not consistent with numcount
// it looks like the OPEN list has a broken forward link
// or perhaps the child nodes are not being accounted for?
// are we counting nodes for NodeCount properly?
// maybe make a ListNodes() func for debugging to see what all the lists contain and where they are pointing?

   while (PathNode) {
    tNode=PathNode;
    PathNode=PathNode->PrevNode;
	V_Free(tNode); 
	//free(tNode);
   NumFreed++;
   }

  while (CLOSED) 
  {
    tNode=CLOSED;
    CLOSED=CLOSED->NextNode;
    V_Free(tNode);
	//free(tNode);
	NumFreed++;
  }

  while (OPEN) {
    tNode=OPEN;
	OPEN=OPEN->NextNode;
    V_Free(tNode); 
	//free(tNode);
  NumFreed++;
  }

  while (Stack) {
    STK=Stack;
    Stack=Stack->StackPtr;
    V_Free(STK); 
	//free(STK);
  NumFreed++;
  }

  //gi.dprintf("freed %d/%d nodes\n", NumFreed, NodeCount);
}

//===============================================
// Propagate Old node's values to Nodes on Stack
//===============================================
void PropagateDown(node_t *Old) {
node_t *POPNode;
int g,c;

  for (c=0;c < NUMCHILDS;c++) // parse through Old node children
    if (Old->Child[c])
      if (Old->g+1 < Old->Child[c]->g) {
        Old->Child[c]->g=g=Old->g+1;//FIXME:why is 'g' not initialized? GHZ: added 'g='
        Old->Child[c]->f=g+Old->h;
        Old->Child[c]->PrevNode=Old;
        Push(Old->Child[c]); } // Push onto Stack

  while (Stack) { // is the stack in use?
    POPNode=Pop(); // grab node from stack
    for (c=0;c < NUMCHILDS;c++) { // parse through all existing POPNOde children
      if (!POPNode->Child[c]) break; // No more valid Child nodes!
      if (POPNode->g+1 < POPNode->Child[c]->g) { // update g and f values
        POPNode->Child[c]->g=g=POPNode->g+1;
        POPNode->Child[c]->f=g+POPNode->h;
        POPNode->Child[c]->PrevNode=POPNode;
        Push(POPNode->Child[c]); } } } // Push onto Stack
}

#define vDiff(b,a) sqrt((a[0]*a[0]-b[0]*b[0])+(a[1]*a[1]-b[1]*b[1])+(a[2]*a[2]-b[2]*b[2]))

//===========================================
// Successor Nodes all pushed onto OPEN list
//===========================================
void GetSuccessorNodes(node_t *StartNode, int NodeNumS, int NodeNumD) {
node_t *Old,*Successor;
node_t *tNode1,*tNode2;
int g,c;
float h;

// NOTE: NodeNumS is the index of a node that was found by the node searching routine
  //================================
  // Has NodeNumS been Searched yet?
  //================================
	// see if this node is already on the OPEN list
	Old = CheckLIST(OPEN, NodeNumS);
	if (Old) 
	{ 
		// node was found on the OPEN list
		// this means the node was found before (as a child of another node)
		// but not yet searched (as a parent node)
		for (c = 0; c < NUMCHILDS; c++)
		{
			// break on the first available child slot of StartNode
			if (!StartNode->Child[c]) 
				break;
		}

		// if we found an empty child slot, use it, otherwise use the last one
		StartNode->Child[((c < NUMCHILDS)?c:(NUMCHILDS-1))] = Old;

		// have we gone farther with this node than StartNode?
		if (StartNode->g + 1 < Old->g) 
		{ 
			Old->g = g = StartNode->g + 1; // make node one step beyond StartNode	
			Old->f = g + Old->h; // update total cost
			Old->PrevNode = StartNode; // reverse link to StartNode
		}
		return; 
	}

  //==================================
  // Has NodeNumS been searched yet?
  //==================================
  Old=CheckLIST(CLOSED,NodeNumS);
  if (Old!=NULL) {
	  // node has been searched before
    for (c=0;c < NUMCHILDS;c++)
      if (StartNode->Child[c]==NULL) break;
    StartNode->Child[((c < NUMCHILDS)?c:(NUMCHILDS-1))]=Old;
    if (StartNode->g+1 < Old->g) {
      Old->g=g=StartNode->g+1;
      Old->f=g+Old->h;
      Old->PrevNode=StartNode;
      PropagateDown(Old); }
    return; }

  //=======================================
  // It is NOT on the OPEN or CLOSED List!!
  //=======================================
  // Make Successor a Child of StartNode
  //=======================================
  //Successor=(node_t *)malloc(sizeof(node_t));
  Successor=(node_t *)V_Malloc(sizeof(node_t), TAG_LEVEL);
  Successor->nodenum=NodeNumS;
  Successor->g=g=StartNode->g+1;

  //GHz - track node memory use so we can free this later
//	NodeList[NodeCount++] = Successor;
  NodeCount++;

// NOTE: the heuristic estimate of the remaining path from this node
// to the destination node is given by the difference between the 2
// vectors.  You can come up with your own estimate..

  Successor->h=h=distance(pathnode[NodeNumS], pathnode[NodeNumD]);//fabs(vDiff(node[NodeNumS].origin,node[NodeNumD].origin));//GHz - changed to fabs()
  Successor->f=g+h;
  Successor->PrevNode=StartNode; // reverse link to StartNode
  Successor->NextNode=NULL;
  // make all child links of new Successor node NULL
  for (c=0;c < NUMCHILDS;c++)
    Successor->Child[c]=NULL;

  for (c=0;c < NUMCHILDS;c++)
    if (StartNode->Child[c]==NULL) break; // Find first empty Child[] of StartNode
  StartNode->Child[((c < NUMCHILDS)?c:(NUMCHILDS-1))]=Successor; // make Successor a child of StartNode

  //=================================
  // Insert Successor into OPEN List
  //=================================
  tNode1=OPEN;
  tNode2=OPEN->NextNode;
  // find node in OPEN list with f-cost greater than Successor node
  while (tNode2 && (tNode2->f < Successor->f)) {
    tNode1=tNode2;
    tNode2=tNode2->NextNode; }
  Successor->NextNode=tNode2;
  tNode1->NextNode=Successor;
  //gi.dprintf("added node %d to the OPEN list\n", Successor->nodenum);
}

vec_t VectorLengthSqr(vec3_t v)

{
       int       i;
        float   length;

        length = 0.0f;

        for (i=0; i<3; i++)
               length += v[i]*v[i];

        return length;

}

// returns node index of node closest to start
int NearestNodeNumber (vec3_t start, float range, qboolean vis)
{
	int		i=0, bestNodeNum=-1;
	float	dist, best=9999;
	vec3_t	v;

	if (!numnodes)
		return -1;

	// get the nodenum for the closest node
	for ( ; i < numnodes; i++)
	{
		VectorCopy(pathnode[i], v);
		// ignore nodes we can't see
		if (vis && !G_IsClearPath(NULL, MASK_SOLID, v, start))
			continue;
		dist = distance(v, start);
		if (range && dist > range)
			continue;
		if (dist < best)
		{
			best = dist;
			bestNodeNum = i;
		}
	}

	return bestNodeNum;
}

qboolean NearestNodeLocation (vec3_t start, vec3_t node_loc, float range, qboolean vis)
{
	int		i=0, bestNodeNum=-1;
	float	dist, best=9999;
	vec3_t	v;

	if (!numnodes)
		return false;

	// get the nodenum for the closest node
	for ( ; i < numnodes; i++)
	{
		VectorCopy(pathnode[i], v);
		// ignore nodes we can't see
		if (vis && !G_IsClearPath(NULL, MASK_SOLID, v, start))
			continue;
		dist = distance(v, start);
		if (range && dist > range)
			continue;
		if (dist < best)
		{
			best = dist;
			bestNodeNum = i;
		}
	}
	
	if (bestNodeNum == -1)
		return false;

	VectorCopy(pathnode[bestNodeNum], node_loc);
	return true;

}

//=========================================
// What is nodenum of this origin, if any?
//=========================================
int GetNodeNum(vec3_t origin) {
vec3_t vtmp;
float dist;
//float best=99999, bestd=99999, d;//GHz
int i;

//gi.dprintf("GetNodeNum() is checking %d nodes\n", numnodes);

  // Search all of my node[i]'s
  for (i=0;i < numnodes;i++) {
    VectorSubtract(pathnode[i],origin,vtmp);
	  dist=VectorLengthSqr(vtmp);

	//if (dist < best)//GHz: added for debugging only
	//	best = dist;
	//d=distance(node[i].origin, origin);
	//if (d < bestd)
	//	bestd=d;

    if (dist < 1.0F)//FIXME: wtf does this mean?
     return i; }

  //gi.dprintf("best result=%f, need=%f, actual=%f\n", best, 1.0F, bestd);
  return -1;
}

//=========================================
// Pull FIRST node from OPEN, put on CLOSED
//=========================================
node_t *NextBestNode(int NodeNumS, int NodeNumD) {
node_t *Node;

  Node=OPEN->NextNode;    // Pull from FRONT of OPEN list

  if (!Node) return NULL;

  //GHz: wont this break if the OPEN list is empty?
  OPEN->NextNode=OPEN->NextNode->NextNode;

  // GHz: OK, the final node list (Best) that is used by FindPath()
  // cannot reference any nodes in other lists, thus you can't have
  // a node appear on the OPEN or CLOSED list and the Best (final) list
 // if (Node->nodenum == NodeNumD || Node->nodenum == NodeNumS)
//	  return Node; // return the final node but don't put it on the CLOSED list!!

  Node->NextNode=CLOSED->NextNode;
  CLOSED->NextNode=Node; // Put at FRONT of CLOSED list

  //gi.dprintf("moved %d to the CLOSED list\n", Node->nodenum);
  return Node; // Return Next Best Node
}

int GetVerticalNodeNum (vec3_t start, float x, float y, float max_z_range, int nodes)
{
	int		i;
	vec3_t	v;

	// copy to temp vector
	VectorCopy(start, v);

	// modify 2-d coordinates of temp vector
	v[0] += x;
	v[1] += y;

	// search for nodes
	for (i=0; i<nodes; i++)
	{
		// put temp node and pathnode on the same vertical plane
		//v[2] = pathnode[i][2];
		// is there a match on the x and y coordinates?
		//if (distance(v, pathnode[i]) > 1.0F)
		//	continue;
		if (Get2dDistance(v, pathnode[i]) > 1)
			continue;
		// is it within our specified z range?
		if (abs((int)pathnode[i][2]-(int)start[2]) > max_z_range)
			continue;
		return i;
	}

	return -1;
}

int CheckVertical (vec3_t start, float x, float y, float z, int max_steps_up, int max_steps_down)
{
	int i, n, max_steps;
	vec3_t v;

	VectorCopy(start, v);

	max_steps = max_steps_up + max_steps_down;

	v[0] += x; // left-right
	v[1] += y; // forward-backward
	
	// step up from this location until we find a node
	for (i=0; i<=max_steps; i++)
	{
		if ((n = GetNodeNum(v)) != -1)
			return n;
		if (i <= max_steps_up)
			v[2] += z;
		else
			v[2] -= z;
	}

	return -1;
}

int gridlist[MAX_GRID_SIZE];

// sets all gridlist values to -1
void ClearGridList (void)
{
	int i;

	// initialize gridlist, remove old values
	for (i=0; i<numnodes; i++)//FIXME: should we initialize up to MAX_GRID_SIZE?
		gridlist[i] = -1;
}

qboolean CheckPath1 (vec3_t start, vec3_t end)
{
	int		i;
	vec3_t	from;
	edict_t *ignore=NULL;
	trace_t	tr;

	VectorCopy(start, from);

	for (i = 0; i < 10000; i++)
	{
		tr = gi.trace(from, tv(-16,-16,0), tv(16,16,0), end, ignore, MASK_SOLID);
		// ignore doors
		if (tr.ent && tr.ent->inuse && tr.ent->mtype == FUNC_DOOR)
		{
			VectorCopy(tr.endpos, from);
			ignore = tr.ent;
		}
		// stop on anything else that is solid
		else
			break;
	}

	if (tr.fraction != 1.0 || tr.startsolid || tr.allsolid || tr.contents & MASK_SOLID)
		return false;
	return true;
}

qboolean CheckPath (vec3_t start, vec3_t end)
{
	//trace_t tr;

	return CheckPath1(start, end);
/*
	tr = gi.trace(start, tv(-16,-16,0), tv(16,16,0), end, NULL, MASK_SOLID);
	if (tr.fraction != 1.0 || tr.startsolid || tr.allsolid || tr.contents & MASK_SOLID)
		return false;
	return true;*/
}

qboolean isValidChildNode (vec3_t start, vec3_t v, int max_distance, int max_z_delta)
{
	// node should be within +/- 32 units of start on the Z axis
	if (fabs(v[2]-start[2]) > max_z_delta)
		return false;
	// distance check, next node could be anywhere between 128 - 255 units away
	if (Get2dDistance(start, v) >= max_distance)
		return false;
	// basic visibility check
	if (!gi.inPVS(start, v))
		return false;
	// advanced visibility check, make sure the path is wide enough to walk to
	if (!CheckPath(start, v))
		return false;
	return true;
}

// fills gridlist with visible nodes within +/- 32 of start on the Z axis
// returns the number of found nodes
int FillGridList (int NodeNumStart)
{
	int		i, j;
	vec3_t	start;

	// clear previous gridlist values
	ClearGridList();

	VectorCopy(pathnode[NodeNumStart], start);

	// fill gridlist with node indices
	for (i=0,j=0; i<numnodes; i++)
	{
		// we don't want the start node pointing to itself!
		if (i == NodeNumStart)
			continue;
		if (!isValidChildNode(start, pathnode[i], 256, 32))
			continue;

		// copy pathnode index to gridlist array and increment the gridlist index/nodes found
		gridlist[j++] = i;
	}

	return j;
}

//TODO: limit search pattern on X and Y axis?
int SortGridList (int NodeNumStart)
{
	int		GLindex;
	int		i, j, index, temp, list_size, childs=NUMCHILDS;
	float	best, dist;
	vec3_t	start;

	VectorCopy(pathnode[NodeNumStart], start);

	list_size = FillGridList(NodeNumStart);

	// nothing to sort!
	if (list_size < 2)
	{
		//gi.dprintf("nothing to sort\n");
		return list_size;
	}

	// we only need to sort nodes equal to the number of child nodes we want
	//if (list_size > NUMCHILDS)
	//	list_size = NUMCHILDS;

	if (list_size < childs)
	{
		//gi.dprintf("list < childs\n");
		childs = list_size;
	}

	// fill gridlist with node indices closest to start
	//for (i=0; i<list_size; i++)
	for (i=0; i<childs; i++)
	{
		GLindex = i;
		index = gridlist[i];//i;
		best = distance(pathnode[gridlist[i]], start);

		for (j = i+1; j<list_size; j++)
		{
			dist = distance(pathnode[gridlist[j]], start);
			if (dist < best)
			{
				index = gridlist[j];//j; // stored value
				GLindex = j; // list position of best value
				best = dist;
			}
		}

		// swap node index of current position with node index of closest node
		// this procedure sorts the list and keeps us from finding the same value twice
		temp = gridlist[i];
		gridlist[i] = index;
		gridlist[GLindex] = temp;
		//gridlist[index] = temp;
	}

	//return CullGridList(start, childs);
	return childs; // return maximum number of valid child nodes found

}
	
int NextNode (int i, vec3_t start)
{
	/*
	switch (i)
	{
	case 0: return CheckVertical(start, 32, 0, 16, 1, 1); // right
	case 1: return CheckVertical(start, -32, 0, 16, 1, 1); // left
	case 2: return CheckVertical(start, 0, 32, 16, 1, 1); // forward
	case 3: return CheckVertical(start, 0, -32, 16, 1, 1); // backward
	case 4: return CheckVertical(start, 32, 32, 16, 1, 1); // right-forward
	case 5: return CheckVertical(start, -32, 32, 16, 1, 1); // left-forward
	case 6: return CheckVertical(start, -32, -32, 16, 1, 1); // back-left
	case 7: return CheckVertical(start, 32, -32, 16, 1, 1); // back-right
	}*/

	switch (i)
	{
	case 0: return GetVerticalNodeNum(start, 32, 0, 32, numnodes); // right
	case 1: return GetVerticalNodeNum(start, -32, 0, 32, numnodes); // left
	case 2: return GetVerticalNodeNum(start, 0, 32, 32, numnodes); // forward
	case 3: return GetVerticalNodeNum(start, 0, -32, 32, numnodes); // backward
	//case 4: return GetVerticalNodeNum(start, 32, 32, 32); // right-forward
	//case 5: return GetVerticalNodeNum(start, -32, 32, 32); // left-forward
	//case 6: return GetVerticalNodeNum(start, -32, -32, 32); // back-left
	//case 7: return GetVerticalNodeNum(start, 32, -32, 32); // back-right
	}
	return -1;

}

/*
int NextNode(int i, vec3_t start) {
int x=32; // 32 units each x direction
int y=32; // 32 units each y direction
int z=16; // 24 units each z direction

//GHz NOTE: since our grid is always on the floor, perhaps we should be pushing down from above so as to find steps?
  switch (i)
  {
   case 0: v[0]+=x; break;//right
   case 1: v[0]-=x; break;//left
   case 2: v[1]+=y; break;//forward
   case 3: v[1]-=y; break;//backward
   case 4: v[2]+=z; break;//up
   case 5: v[2]-=z; break;//down
   } 
}
*/
//==========================================================

void ComputeSuccessors(node_t *PresentNode, int NodeNumD)
{
	int i, maxChilds, NextNodeNum;

	// create a sorted list of the closest node indices
	maxChilds = SortGridList(PresentNode->nodenum);

	for (i=0; i<maxChilds; i++)
	{
		NextNodeNum = gridlist[i];
		//if (NextNodeNum == PresentNode->nodenum)
		//	gi.dprintf("%d = %d\n", PresentNode->nodenum, NextNodeNum);
		if (NextNodeNum == -1) 
			continue;
		GetSuccessorNodes(PresentNode, NextNodeNum, NodeNumD);
	}
}
/*
void ComputeSuccessors(node_t *PresentNode, int NodeNumD) {
int c,NextNodeNum;
vec3_t vtmp;

  for (c=0;c < NUMCHILDS;c++) {
    VectorCopy(pathnode[PresentNode->nodenum],vtmp);
   NextNodeNum= NextNode(c,vtmp); // Note: vtmp is changed inside NextNode()
    //NextNodeNum=GetNodeNum(vtmp); // location in node[] array?
    if (NextNodeNum == -1) continue;   // vtmp Node is invalid
    GetSuccessorNodes(PresentNode,NextNodeNum,NodeNumD); }
}
*/
int *Waypoint=NULL; // Integer array of nodenum's along the path
int numpts=0;       // Number of nodes in the path..

int CopyWaypoints (int *wp, int max)
{
	int i, j=max;

	if (j > numpts)
		j = numpts;

	for (i = 0; i < j; i++)
		wp[i] = Waypoint[i];

	return j;
}

void GetNodePosition (int nodenum, vec3_t pos)
{
	VectorCopy(pathnode[nodenum], pos);
}

// returns the waypoint index closest to start along the path leading to our final destination
int NearestWaypointNum (vec3_t start, int *wp)
{
	int		i, bestNodeNum;
	float	dist, best = 0;

	if (!numpts)
		return -1;

	// get the nodenum for the closest node
	for (i = 0; i < numpts; i++)
	{
		dist = distance(pathnode[wp[i]], start);
		if (!best || dist < best)
		{
			best = dist;
			bestNodeNum = i;
		}
	}
	
	return bestNodeNum;
}

// copies the location of the next waypoint nearest to start
// returns -1 if we are at the end of the waypoint path
int NextWaypointLocation (vec3_t start, vec3_t loc, int *wp)
{
	int nearestWaypoint;

	// get the waypoint index closest to start
	if ((nearestWaypoint = NearestWaypointNum(start, wp)) != -1)
	{
		// we are at the end of the path
		if (nearestWaypoint == numpts - 1)
			return -1;

		//gi.dprintf("current node = %d, next node = %d\n", 
		//	wp[nearestWaypoint], wp[nearestWaypoint+1]);

		VectorCopy(pathnode[wp[nearestWaypoint+1]], loc);
		return 1;// success!
	}

	// couldn't find the closest waypoint index
	return -1;
}

void RemoveDuplicates (node_t *BestList, node_t *OtherList)
{
	node_t *bestNode, *tNode, *tNodePrev=NULL;

	bestNode = BestList;

	while (bestNode)
	{
		tNode = OtherList;

		// search OtherList for a node that also exists in BestList
		while (tNode)
		{
			// is this nodenum on the BestList?
			if (tNode == bestNode)
				break;

			// save the previous node
			tNodePrev = tNode;
			// move to the next node in OtherList
			tNode = tNode->NextNode;
		}
			
		// we found a duplicate, so pop it out of OtherList
		if (tNode)
			tNodePrev->NextNode = tNode->NextNode;

		// move to the next node in BestList
		bestNode = bestNode->PrevNode;
	}
}

//=========================================
int FindPath(vec3_t start, vec3_t destination) {
	node_t *StartNode;
	node_t *BestNode;
	node_t *tNode;
	int NodeNumD;
	int NodeNumS;
	int g, c, i;
	float h;
	vec3_t tstart, tdest;

	VectorCopy(start, tstart);
	VectorCopy(destination, tdest);

	// Get NodeNum of start vector
	NodeNumS = GetNodeNum(tstart);
	if (NodeNumS == -1) {
		//gi.dprintf("bad nodenum at start\n");
		return 0; // ERROR
	}

	// Get NodeNum of destination vector
	NodeNumD = GetNodeNum(tdest);
	if (NodeNumD == -1)
	{
		// gi.dprintf("bad nondenum at end\n");
		return 0; // ERROR
	}

	// Allocate OPEN/CLOSED list pointers..
	OPEN = (node_t *)V_Malloc(sizeof(node_t), TAG_LEVEL);
	// OPEN=(node_t *)malloc(sizeof(node_t));
	OPEN->NextNode = NULL;

	CLOSED = (node_t *)V_Malloc(sizeof(node_t), TAG_LEVEL);
	//CLOSED=(node_t *)malloc(sizeof(node_t));
	CLOSED->NextNode = NULL;

	//================================================
	// This is our very first NODE!  Our start vector
	//================================================
	StartNode = (node_t *)V_Malloc(sizeof(node_t), TAG_LEVEL);
	//StartNode=(node_t *)malloc(sizeof(node_t));
	StartNode->nodenum = NodeNumS; // starting position nodenum
	StartNode->g = g = 0; // we haven't gone anywhere yet
	StartNode->h = h = distance(start, destination);//fabs(vDiff(start,destination)); // calculate remaining distance (heuristic estimate) GHz - changed to fabs()
	StartNode->f = g + h; // total cost from start to finish
	for (c = 0; c < NUMCHILDS; c++)
		StartNode->Child[c] = NULL; // no children for search pattern yet
	StartNode->NextNode = NULL;
	StartNode->PrevNode = NULL;
	//================================================

	// next node in open list points to our starting node
	OPEN->NextNode = BestNode = StartNode; // First node on OPEN list..

	//GHz - need to free these nodes too!
	//NodeList[NodeCount++] = OPEN;
	//  NodeList[NodeCount++] = CLOSED;
	NodeCount += 2;

	for (;;) {
		tNode = BestNode; // Save last valid node
		BestNode = (node_t *)NextBestNode(NodeNumS, NodeNumD); // Get next node from OPEN list
		if (!BestNode) {
			//gi.dprintf("ran out of nodes to search\n");
			return 0;//GHz
			// BestNode=tNode; // Last valid node..
			// break;
		}

		if (BestNode->nodenum == NodeNumD) break;// we there yet?
		ComputeSuccessors(BestNode, NodeNumD);
	} // Search from here..

	//================================================

	RemoveDuplicates(BestNode, CLOSED);//FIXME: move this up before the start==end crash check

	// gi.dprintf("%d: processed %d nodes\n", level.framenum,NodeCount);
	if (BestNode == StartNode) {  // Start==End??
		FreeStack(StartNode);//FIXME: may cause crash
		//gi.dprintf("start==end\n");
		return 0;
	}




	//gi.dprintf("Start = %d End = %d\n", NodeNumS, NodeNumD);
	// gi.dprintf("Printing tNode (in reverse):\n");
	// PrintNodes(BestNode, true);
	// gi.dprintf("Printing OPEN list:\n");
	//PrintNodes(OPEN, false);
	//gi.dprintf("Printing CLOSED list:\n");
	// PrintNodes(CLOSED, false);

	BestNode->NextNode = NULL; // Must tie this off!


	// How many nodes we got?
	tNode = BestNode;
	i = 0;
	while (tNode) {
		i++; // How many nodes?
		tNode = tNode->PrevNode;
	}

	if (i <= 2) { // Only nodes are Start and End??
		FreeStack(BestNode);//FIXME: may cause crash
		//gi.dprintf("only start and end nodes\n");
		return 0;
	}

	// Let's allocate our own stuff...


	//CLOSED->NextNode = NULL;//GHz - only needs to be null if we are using freestack()
	numpts = i;

	//GHz - free old memory
	//V_Free(Waypoint);

	Waypoint = (int *)V_Malloc(numpts*sizeof(int), TAG_LEVEL);
	//Waypoint=(int *)malloc(numpts*sizeof(int));

	// Now, we have to assign the nodenum's along
	// this path in reverse order because that is
	// the way the A* algorithm finishes its search.
	// The last best node it visited was the END!
	// So, we copy them over in reverse.. No biggy..

	tNode = BestNode;
	while (BestNode) {
		Waypoint[--i] = BestNode->nodenum;//GHz: how/when is this freed?
		BestNode = BestNode->PrevNode;
	}

	// NOTE: At this point, if our numpts returned is not
	// zero, then a path has been found!  To follow this
	// path we simply follow node[Waypoint[i]].origin
	// because Waypoint array is filled with indexes into
	// our node[i] array of valid vectors in the map..
	// We did it!!  Now free the stack and exit..

	//================================================

	//++++++++++ GHz NOTES +++++++++++++
	// FreeStack() is flawed because the lists have nodes that point to nodes on other lists
	// so if you free one list, then the next list will crash when it encounters a node with
	// an invalid pointer (node was freed in last list)
	//++++++++++++++++++++++++++++++++++

	FreeStack(tNode); // Release ALL resources!!

	//GHz: cleanup test/debugging
	//for (i=0;i<NodeCount;i++)
	//{
	//	  V_Free(NodeList[i]);
	// }
	// OPEN = NULL;
	//CLOSED = NULL;
	NodeCount = 0;

	//TODO: performance... cpu usage is still very high
	//TODO: grid editor, save grid to disk
	//TODO: need some way of handling manually edited grid
	// because NextNode() only searches within a specific 32x32 pattern


	// gi.dprintf("%d: found %d\n",level.framenum,numpts);

	return (numpts);
}

//======================================================
void G_Spawn_Splash(int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin) {
  gi.WriteByte(svc_temp_entity);
  gi.WriteByte(type);
  gi.WriteByte(count);
  gi.WritePosition(start);
  gi.WriteDir(movdir);
  gi.WriteByte(color);
  gi.multicast(origin, MULTICAST_PVS);
}

//==================================================

//=====================================================
// NOTE: you may already have this function someplace
//=====================================================
void G_Spawn_Trails(int type,vec3_t start,vec3_t endpos) {
  gi.WriteByte(svc_temp_entity);
  gi.WriteByte(type);
  gi.WritePosition(start);
  gi.WritePosition(endpos);
  gi.multicast(start,MULTICAST_PVS);
}

void DrawPath (edict_t *ent)
{
	int i, j;

	if (!ent->showPathDebug)
		return;

	if (!ent->monsterinfo.numWaypoints)
		return;

	for (i = 0; i < ent->monsterinfo.numWaypoints; i++)
	{
		j = ent->monsterinfo.waypoint[i];
		G_Spawn_Splash(TE_LASER_SPARKS, 4, 0xdcdddedf, pathnode[j], vec3_origin, pathnode[j]);
	}
}
/*
void DrawPath (void) 
{
	int i, j=0;

	if (numpts < 0)
		return;

	for (i=0; i<numpts; i++)
	{
		j = Waypoint[i];
		G_Spawn_Splash(TE_LASER_SPARKS, 4, 0xdcdddedf, pathnode[j], vec3_origin, pathnode[j]);
		//G_Spawn_Trails(TE_BFG_LASER, pathnode[j], pathnode[j+1]);
	}
}
*/

//=====================================================
//================== pathfinding stuff ================
//=====================================================




//========================================================
// Put this at very top of ClientThink() so you can see
// the nodes created by CreateGrid as you walk around!
// Also, prototype this function above ClientThink()...
//========================================================

// NOTE: max search distance for child links should be set to
// gap specified in nearbyGridNode + x/yevery + min1/max1 (e.g. 128+32+3=163)
void DrawChildLinks (edict_t *ent)
{
	int		i, count, parentNode;
	float	dist, maxDist;
	vec3_t	start, end;
	trace_t	tr;

	if (ent->client->showGridDebug == 3)
	{
		parentNode = 0;

		// spawn bfg laser trails between player and nearby nodes
		VectorCopy(ent->s.origin, start);
		start[2] -= 8192;
		tr = gi.trace(ent->s.origin, NULL, NULL, start, NULL, MASK_SOLID);
		VectorCopy(tr.endpos, start);
		start[2] += 32;
	}
	else
	{
		parentNode = NearestNodeNumber(ent->s.origin, 255, true);
		VectorCopy(pathnode[parentNode], start);
	}

	for (i = count = maxDist = 0; i < numnodes; i++)
	{
		// don't link parent node to itself!
		if (parentNode && i == parentNode)
			continue;
		if (!isValidChildNode(start, pathnode[i], 256, 32))
			continue;

		// copy node position
		VectorCopy(pathnode[i], end);

		// get distance between parent node and this node
		dist = Get2dDistance(start, end);

		// update maximum child link distance
		if (!maxDist || dist > maxDist)
			maxDist = dist;

		// spawn bfg laser trails between parent and child nodes
		G_Spawn_Trails(TE_BFG_LASER, start, end);

		// keep track of the number of child links found
		count++;
	}

	if (!(level.framenum%20))
		gi.centerprintf(ent, "Node %d has %d child links @ %.0f.\n", 
			parentNode, count, maxDist);
}

void DrawNearbyGrid(edict_t *ent) {
	int i;
vec3_t v,forward;

  AngleVectors(ent->s.angles,forward,NULL,NULL);

  for (i=0;i<numnodes;i++) {
    VectorSubtract(pathnode[i],ent->s.origin,v);
    if (VectorLength(v)>=256) continue; // limit view distance to eliminate overflows
    if (DotProduct(v,forward)>0.3) { // infront?
      VectorCopy(pathnode[i],v);
      v[2]-=4; // node height
	 // NearestNodeLocation(ent->s.origin, start);
	 // gi.dprintf("%f\n", fabs(pathnode[i][2]-start[2]));
      G_Spawn_Trails(TE_BFG_LASER,pathnode[i],v); } } 
}

void DeleteNode (int nodenum)
{
	// if this isn't the last node on the list, then copy the
	// vector stored at the end of the array to current position
	if (nodenum != numnodes - 1)
		VectorCopy(pathnode[numnodes-1], pathnode[nodenum]);
	// clear the value stored at the end of the list
	VectorClear(pathnode[numnodes-1]);
	// reduce the size of the list
	numnodes--;
}

void Cmd_DeleteNode_f (edict_t *ent)
{
	int nearestNode;

	if (!ent->myskills.administrator)
		return;

	if ((nearestNode = NearestNodeNumber(ent->s.origin, 255, true)) != -1)
		DeleteNode(nearestNode);

	safe_cprintf(ent, PRINT_HIGH, "**Closest node deleted (%d nodes total).**\n", numnodes);
}

void Cmd_AddNode_f (edict_t *ent)
{
	vec3_t	start;
	trace_t	tr;

	if (!ent->myskills.administrator)
		return;

	VectorCopy(ent->s.origin, start);
	start[2] -= 8192;
	tr = gi.trace(ent->s.origin, NULL, NULL, start, NULL, MASK_SOLID);
	VectorCopy(tr.endpos, start);
	start[2] += 32;
	VectorCopy(start, pathnode[numnodes]);
	numnodes++;

	safe_cprintf(ent, PRINT_HIGH, "**Node added at current position (%d nodes total).\n**", numnodes);
}

void Cmd_DeleteAllNodes_f (edict_t *ent)
{
	if (!ent->myskills.administrator)
		return;

	memset(&pathnode, 0, numnodes*sizeof(vec3_t));
	numnodes = 0;

	safe_cprintf(ent, PRINT_HIGH, "All nodes deleted.\n");
}

void SaveGrid (void)
{
	char	filename[255];
	FILE	*fptr;

	CheckDir(va("%s/settings/grd", gamedir->string));

	Com_sprintf(filename, sizeof(filename), "%s/settings/grd/%s.grd", game_path->string, level.mapname);

	if ((fptr = fopen(filename, "wb")) != NULL)
	{
		fwrite(&pathnode[0], sizeof(vec3_t), MAX_GRID_SIZE, fptr);
		WriteInteger(fptr, numnodes);
		fclose(fptr);
		gi.dprintf("Grid successfully saved.\n");
	}
	else
		gi.dprintf("Unable to save grid file: %s\n", filename);
}

qboolean LoadGrid (void)
{
	char	filename[255];
	FILE	*fptr;

	//memset(&pathnode[0], 0, sizeof(vec3_t));
	Com_sprintf(filename, sizeof(filename), "%s/settings/grd/%s.grd", game_path->string, level.mapname);

	if ((fptr = fopen(filename, "rb")) != NULL)
	{
		fread(&pathnode[0], sizeof(vec3_t), MAX_GRID_SIZE, fptr);
		numnodes = ReadInteger(fptr);
		fclose(fptr);

		gi.dprintf("Grid successfully loaded (%d nodes).\n", numnodes);
		return true;
	}

	gi.dprintf("Grid file not found.\n");
	return false;
}

void Cmd_SaveNodes_f (edict_t *ent)
{
	if (!ent->myskills.administrator)
		return;

	SaveGrid();

	safe_cprintf(ent, PRINT_HIGH, "Saving nodes...\n", numnodes);
}

void Cmd_LoadNodes_f (edict_t *ent)
{
	if (!ent->myskills.administrator)
		return;

	LoadGrid();

	safe_cprintf(ent, PRINT_HIGH, "Loading nodes...\n", numnodes);
}

//========================================================
int AdjustDownward(edict_t *ignore,vec3_t start) {
vec3_t endpt;
trace_t tr;
  VectorSet(endpt,start[0],start[1],-8192);
  tr=gi.trace(start,tv(-1,-1,0),tv(1,1,0),endpt,ignore,CONTENTS_SOLID);
  tr.endpos[2]+=32;
  return (int)(tr.endpos[2]-start[2]); // return delta, if needed later..
}


//========================================================

int GetNumChildren (int parent_nodenum, int nodes)
{
	int		i, j;

	for (i = j = 0; i < nodes; i++)
	{
		if (i == parent_nodenum)
			continue;
		if (!isValidChildNode(pathnode[parent_nodenum], pathnode[i], 256, 32))
			continue;
		j++;
	}

	return j;
}		

// NOTE: max search distance for child links should be set to
// gap specified in nearbyGridNode + x/yevery + min1/max1 (e.g. 128+32+3=163)
qboolean NearbyGridNode (vec3_t start, int nodes)
{
	int i;
	
	for (i = 0; i < nodes; i++)
	{
		if (!isValidChildNode(start, pathnode[i], 129, 18))
			continue;
		return true;
	}
	return false;
}

int c_yes, c_no;
qboolean CheckBottom (vec3_t pos, vec3_t boxmin, vec3_t boxmax)
{
	vec3_t	mins, maxs, start, stop;
	trace_t	trace;
	int		x, y;
	float	mid, bottom;
	
	VectorAdd (pos, boxmin, mins);
	VectorAdd (pos, boxmax, maxs);

// if all of the points under the corners are solid world, don't bother
// with the tougher checks
// the corners must be within 16 of the midpoint
	start[2] = mins[2] - 8;
	for	(x=0 ; x<=1 ; x++)
		for	(y=0 ; y<=1 ; y++)
		{
			start[0] = x ? maxs[0] : mins[0];
			start[1] = y ? maxs[1] : mins[1];
			if (gi.pointcontents (start) != CONTENTS_SOLID)
				goto realcheck;
		}

	c_yes++;
	return true;		// we got out easy

realcheck:
	c_no++;
//
// check it for real...
//
	start[2] = mins[2];
	
// the midpoint must be within 16 of the bottom
	start[0] = stop[0] = (mins[0] + maxs[0])*0.5;
	start[1] = stop[1] = (mins[1] + maxs[1])*0.5;
	stop[2] = start[2] - 2*18;
	trace = gi.trace (start, vec3_origin, vec3_origin, stop, NULL,MASK_PLAYERSOLID /*MASK_MONSTERSOLID*/);

	if (trace.fraction == 1.0)
		return false;
	mid = bottom = trace.endpos[2];
	
// the corners must be within 16 of the midpoint	
	for	(x=0 ; x<=1 ; x++)
		for	(y=0 ; y<=1 ; y++)
		{
			start[0] = stop[0] = x ? maxs[0] : mins[0];
			start[1] = stop[1] = y ? maxs[1] : mins[1];
			
			trace = gi.trace (start, vec3_origin, vec3_origin, stop, NULL, MASK_PLAYERSOLID /*MASK_MONSTERSOLID*/);
			
			if (trace.fraction != 1.0 && trace.endpos[2] > bottom)
				bottom = trace.endpos[2];
			if (trace.fraction == 1.0 || mid - trace.endpos[2] > 18)
				return false;
		}

	c_yes++;
	return true;
}

void CullGrid (void)
{
	int i;

	for (i = 0; i < numnodes; i++)
	{
		// delete nodes that have no children
		if (GetNumChildren(i, numnodes) < 1)
			DeleteNode(i);
	}
}

void CreateGrid (qboolean force) {
// ====path stuff
//int i;
// ====path stuff
int x,y,z,cnt=0;
vec3_t v,endpt;
trace_t tr1,tr2;
float v0,v1,v2;

  vec3_t min1={0,0,0};  // width 6x6
  vec3_t max1={0,0,0};

  vec3_t min2={-16,-16,0};// width 32x32 (was 24x24)
  vec3_t max2={+16,+16,0};

    numnodes=0;

	if (!force && LoadGrid())
		return;

  for (x=0;x<maxx;x++) {
    v0=g2v0(x); // convert grid(x) to v[0]
    for (y=0;y<maxy;y++) {
      v1=g2v1(y); // convert grid(y) to v[1]
      for (z=maxz-1;z>=0;z--) {
        v2=g2v2(z); // convert grid(z) to v[2]
        //--------------------------------------
        VectorSet(v,v0,v1,v2);
        // Skip world locations in solid/lava/slime/window/ladder
        if (gi.pointcontents(v) & MASK_OPAQUE) { z--; continue; }
        //-----------------------------------------------
        // At this point,v(x,y,z) is a point in mid-air
        //-----------------------------------------------
        // Trace small bbox down to see what is below
        VectorSet(endpt,v[0],v[1],-8192);
        // Stop at world locations in solid/lava/slime/window/ladder
        tr1=gi.trace(v,min1,max1,endpt,NULL,MASK_OPAQUE);
        // Set for-loop index to our endpt's grid(z)
        z=gridz(tr1.endpos[2]);
        // Skip if trace endpt hit func entity.
        if (tr1.ent && (tr1.ent->use || tr1.ent->think || tr1.ent->blocked)) continue;
        // Skip if trace endpt hit lava/slime/window/ladder.
        if (tr1.contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WINDOW)) continue;
        // Skip if trace endpt hit non-walkable slope
        if (tr1.plane.normal[2]<0.7) continue;
        //----------------------------------------
        // Test vertical clearance above v(x,y,z)
        //----------------------------------------
        VectorCopy(tr1.endpos,endpt);
        //tr1.endpos[2]+=2; // set start just above surface
        endpt[2]+=32;     // endpt at approx crouch height
        tr2=gi.trace(endpt,min2,max2,tr1.endpos,NULL,MASK_OPAQUE);//GHz - push down instead of up
        // Skip if not reachable by crouched bbox - trace incomplete?
       // if (tr2.fraction != 1.0) continue;
        // Skip if linewidth inside solid - too close to adjoining surface?
		if (tr2.startsolid || tr2.allsolid) continue;

		// GHz: check final position to see if it intersects with a solid
		tr1=gi.trace(tr2.endpos,min2,max2,tr2.endpos,NULL,MASK_OPAQUE);
		if (tr1.fraction != 1.0 || tr1.startsolid || tr1.allsolid)
			continue;
		if (!CheckBottom(tr2.endpos, min2, max2))
			continue;

		VectorCopy(tr2.endpos, endpt);//GHz
		endpt[2]+=32;//GHz
		//if (tr2.allsolid) continue;
        //-------------------------------------
        // Now, adjust downward for uniformity
        //-------------------------------------
       // AdjustDownward(NULL,endpt);
        // Houston,we have a valid node!
		if (NearbyGridNode(endpt, cnt))
			continue;//GHz
        VectorCopy(endpt,pathnode[cnt]); // copy to pathnode[] array
        cnt++; } } }

  numnodes=cnt;
  CullGrid();

  gi.dprintf("%d Nodes Created\n",numnodes);

//=====================================================
//================== pathfinding stuff ================
//=====================================================
/*
  // allocate memory for node array
  node = (node_t *) V_Malloc(numnodes*sizeof(node_t), TAG_LEVEL);

  // copy all the pathnode stuff to new node array
  for (i=0;i<numnodes;i++)
  {
	  VectorCopy(pathnode[i], node[i].origin);
	  node[i].nodenum = i;
  }
*/

  if (!force)
	SaveGrid();
}

void Cmd_ComputeNodes_f (edict_t *ent)
{
	if (!ent->myskills.administrator)
		return;

	CreateGrid(true);
	safe_cprintf(ent, PRINT_HIGH, "Computing nodes...\n");
}

void Cmd_ToggleShowGrid (edict_t *ent)
{
	if (!ent->myskills.administrator)
		return;

	if (ent->client->showGridDebug == 3)
		ent->client->showGridDebug = 0;
	else
		ent->client->showGridDebug++;
}
