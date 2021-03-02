#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Basically just a string(max length 64 chars) and a pointer to another object.
// Use to build linked lists, d'oh.
typedef struct string64_node_t {
   char strMap[64];
   char strGravity[5];
   char strTimelimit[5];
   char strFraglimit[5];
   int strRepeat;
   int strMasterRepeat; // used to restore strRepeat after count reaches 0
   struct string64_node_t * next;
};

/*Suck up the level cycle.txt file from lcycle dir. Build a linked lists containing all the
lines of the file, and return a pointer to the first node.
*/
struct string64_node_t * J_ImportLevelCycle();
