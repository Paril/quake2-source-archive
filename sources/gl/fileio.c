  // 
  // fileio.c -- file access routines 
  // 
  // 
  // 4/98 - L. Allan Campbell (Geist) 
  // 
  // INCLUDES ///////////////////////////////////////////////// 
  #include "g_local.h" 
    
  // FUNCTIONS //////////////////////////////////////////////// 
  // 
  // OpenFile 
  // 
  // Opens a file for reading.  This function will probably need 
  // a major overhaul in future versions so that it will handle 
  // writing, appending, etc. 
  // 
  // Args: 
  //   filename - name of file to open. 
  // 
  // Return: file handle of open file stream. 
  //         Returns NULL if file could not be opened. 
  // 
  FILE *OpenFile(char *filename) 
  { 
     FILE *fp = NULL; 
     if ((fp = fopen(filename, "r")) == NULL)        
     { 
        // file did not load 
        gi.dprintf ("Could not open file \"%s\".\n", filename); 
        return NULL; 
     } 
     return fp; 
  } 
    
  // 
  // CloseFile 
  // 
  // Closes a file that was previously opened with OpenFile(). 
  // 
  // Args: 
  //   fp  - file handle of file stream to close. 
  // 
  // Return: (none) 
  // 
  void CloseFile(FILE *fp) 
  { 
     if (fp)        // if the file is open 
     { 
        fclose(fp); 
     } 
     else    // no file is opened 
        gi.dprintf ("ERROR -- CloseFile() exception.\n"); 
  }
