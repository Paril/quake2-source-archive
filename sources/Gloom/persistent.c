#include "persistent.h"
#include "g_local.h"

#ifdef __unix
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#define DVERSION 0x129543

static gamepersistent_t sgamepersistent[1];
gamepersistent_t*gamepersistent = sgamepersistent;
static int mapsize;


void InitGamePersistent()
{
  if (gamepersistent != sgamepersistent) return;

  char*v = GLOOMVERSIONNO;
  memset(sgamepersistent, 0, sizeof(gamepersistent_t));
  sgamepersistent->version = DVERSION | (atoi(v+4)*750) | sizeof(gamepersistent_t);
  gamepersistent = sgamepersistent;
  time(&sgamepersistent->time);

#ifdef __unix
  int fd = open(va("%s/gamepersistent.%d", gamedir->string, (int)gi.cvar("port","27910",0)->value), O_RDWR | O_CREAT, 438);
  if (fd >= 0) {
    int n = lseek(fd, 0, SEEK_END);
    int pagesize = sysconf(_SC_PAGESIZE); //getpagesize();

    if (n < sizeof(gamepersistent_t)) {
      lseek(fd, 0, SEEK_SET);
      if (write(fd, sgamepersistent, sizeof(gamepersistent_t)) != sizeof(gamepersistent_t)) {
        perror("InitGamePersistent::write");
        gi.dprintf("WARNING: Write error during persistent data intialization. Stats will be lost between server restarts.\n");
        close(fd);
        return;
      }
      n = sizeof(gamepersistent_t);
    }

    mapsize = ((n+pagesize-1)/pagesize)*pagesize;
    if (n < mapsize) {
      char z = 0;
      if (lseek(fd, mapsize-1, SEEK_SET) < 0 || write(fd, &z, 1) != 1) {
        perror("InitGamePersistent::write");
        gi.dprintf("WARNING: Write error during persistent data padding.\n");
        //Don't treat this as fatal error, as man page says mmap must be page size aligned, but in linux proved it wasnt actually needed.
        //Although if this failed, probably means disk/quota has 0 bytes free and something else might fail.
      }
      lseek(fd, 0, SEEK_SET);
    }
    
    if (!(gamepersistent = mmap(NULL, mapsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0))) {
      perror("InitGamePersistent::mmap");
      gi.dprintf("WARNING: Failed to mmap stats file. Stats will be lost between server restarts.\n");
      gamepersistent = sgamepersistent;
      close(fd);
      return;
    }
    close(fd);
    
    if (sgamepersistent->version != gamepersistent->version) {
      memset(gamepersistent, 0, sizeof(gamepersistent_t));
      gamepersistent->version = sgamepersistent->version;
      gamepersistent->time = sgamepersistent->time;
      gi.dprintf("Game persistent stats resetted.\n");
    }

    gi.dprintf("Loaded persistent data (%d of %d [%d])\n", n, mapsize, pagesize);
  }
#else
#warning "Incompatible OS: Persistent stats will not be available. Consider porting the code and sending a patch to support it"
#endif
  
  if (gamepersistent == sgamepersistent)
    gi.dprintf("WARNING: Failed to initialize persistent data. Stats will be lost between server restarts\n");
}


void FreeGamePersistent()
{
#ifdef __unix
  if (gamepersistent != sgamepersistent)
    munmap(gamepersistent, mapsize);
#endif
  gamepersistent = sgamepersistent;
}
