#include "readmaps.h" // for error messages and the gi object
#include "s_readconfig.h"

struct string64_node_t *curLevInCycle;

struct string64_node_t * J_ImportLevelCycle() {
      
      FILE *f;
	  char garbage;
      char instr[64];
  	  char temp[64];
	  int count=0;
	  int x=0;
	  int y=0;
	  
      struct string64_node_t *node, *newnode;
 	  int lcount = 0;
	  unsigned int i;


	  if (strlen(cycleloc) < 1) {
		  if ((f = fopen(cycleloc, "r")) == NULL) {
			printf("You did not define a map cycle file to use.\n");
			return NULL;
		  }
	  } else {
		  if ((f = fopen(cycleloc, "r")) == NULL) {
			printf("Unable to open map cycle file, [%s]", cycleloc);
			return NULL;
		  }
	  }


			for (y=0; y<64; y++) {
			    instr[y] = '\0';
				temp[y] = '\0';
			}

	  
        //grab the first level name / gravity / time and frag limits
        if ((node = malloc(sizeof(struct string64_node_t))) == NULL) {
                printf("Unable to allocate memory for cycle node.");
                return NULL;
        } else {
                if (!fgets(instr, 64, f)) { // no names in file, default to base1
                        strcpy(node->strMap, "base1");
						strcpy(node->strGravity, GlobalGravity);
					    strcpy(node->strTimelimit,GlobalTimeLimit);
	                    strcpy(node->strFraglimit, GlobalFragLimit);
				        node->strRepeat=0;
						node->strMasterRepeat=0;
				        node->next = node; // As in, a cycle of one.
                } else {
				// Establish priming read.
					for (i=0; i < strlen(instr); i++) {
							// Strips out the commas
						if (instr[i] != ',' && instr[i] != ' ' && instr[i] != '\n' && i != strlen(instr)-1) {
							temp[x] = instr[i]; // copy into temp variable
							x++;
						} else {
							
							// This allows a line without a \n to be used						
							if (i == strlen(instr)-1) {
								temp[x]=instr[i];
								temp[x+1]='\0';
							} else

							temp[x]='\0'; // make sure you terminate the string
							
							x=0; // reset x back to zero
							garbage = instr[i]; // assign comma or space or \n to garbage
							count++; // Increment comma count so we know where to put things
							switch (count) {
							case 1:
						    	// Make sure strMap doesn't have embedded spaces
								if (temp[strlen(temp) -1] == '\n')
					                temp[strlen(temp) - 1] = '\0';
								strcpy(node->strMap, temp);
								
								if (i==strlen(instr)-1) { // If only map was entered, assume defaults
									strcpy(node->strGravity, GlobalGravity);
								    strcpy(node->strTimelimit,GlobalTimeLimit);
									strcpy(node->strFraglimit, GlobalFragLimit);
									node->strRepeat=0;
									node->strMasterRepeat=0;
								}
								break;
							case 2:
								strcpy(node->strGravity,temp);
							break;
							case 3:
								strcpy(node->strTimelimit,temp);
							break;
							case 4:
								strcpy(node->strFraglimit,temp);
							break;
							case 5:
								node->strRepeat=atoi(temp);
								node->strMasterRepeat=atoi(temp);
							break;
							} // end switch


						} // end else		
					} // end for
					printf("Map:%4s   Gravity:%4s   Timelimit:%3s   Fraglimit:%3s   Repeat:%2d\n", node->strMap, node->strGravity, node->strTimelimit, node->strFraglimit, node->strRepeat);
					node->next = node; // Assume this is the last node just in case.
	            	lcount++;
					} // end else
					
       } // end else
		
		count=0; // reset counter back to zero
		x=0; // reset x back to zero
		garbage = '\0';
	    
	     while (fgets(instr, 64, f)) { // keep on suckin'
                
				if ((newnode = malloc(sizeof(struct string64_node_t))) == NULL) {
                        printf("Unable to allocate memory for cycle node.");
                        return NULL; // FIX ME: Remember to treat this as an error when calling
                } // else
        
				for (i=0; i < strlen(instr); i++) {
							// Strips out the commas
						if (instr[i] != ',' && instr[i] != ' ' && instr[i] != '\n' && i != strlen(instr)-1) {
							temp[x] = instr[i]; // copy into temp variable
							x++;
						} else {

							if (i == strlen(instr)-1) {
								temp[x]=instr[i];
								temp[x+1]='\0';
							} else
							temp[x]='\0'; // make sure you terminate the string
							
														
							x=0; // reset x back to zero
							garbage = instr[i]; // assign comma or space or \n to garbage
							count++; // Increment comma count so we know where to put things
							switch (count) {
							case 1:
								// Make sure strMap doesn't have embedded spaces
								if (temp[strlen(temp) -1] == '\n')
					                temp[strlen(temp) - 1] = '\0';
								strcpy(newnode->strMap, temp);
								if (i==strlen(instr)-1) { // If only map was entered, assume defaults
									strcpy(newnode->strGravity, GlobalGravity);
								    strcpy(newnode->strTimelimit,GlobalTimeLimit);
									strcpy(newnode->strFraglimit, GlobalFragLimit);
									newnode->strRepeat=0;
									newnode->strMasterRepeat=0;
								}
								break;
							case 2:
								strcpy(newnode->strGravity,temp);
							break;
							case 3:
								strcpy(newnode->strTimelimit,temp);
							break;
							case 4:
								strcpy(newnode->strFraglimit,temp);
							break;
							case 5:
								newnode->strRepeat=atoi(temp);
								newnode->strMasterRepeat=atoi(temp);
							break;
							} // end switch
							
							} // end else		
					} // end for
				printf("Map:%4s   Gravity:%4s   Timelimit:%3s   Fraglimit:%3s   Repeat:%2d\n", newnode->strMap, newnode->strGravity, newnode->strTimelimit, newnode->strFraglimit, newnode->strRepeat);
				// Fix to read blank lines properly... Thanks again to Joel Gluth!
				if(strlen(newnode->strMap)) {
				newnode->next = node->next; // All nodes assume they are the last until told.
                node->next = newnode;  // Here, for example.
                node = newnode;        // Shift the whole assembly along one node, keep going.
        		lcount++;
				}
			count=0; // reset counter back to zero
			x=0; // reset x back to zero
		} // endwhile

		printf("%i levels imported.\n", lcount);
        fclose(f);
						
										
        return newnode->next; // pointer to the first node
}
