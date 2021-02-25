#include <stdint.h>
#include <time.h>

typedef struct wininfo_s {
  float    time;
  uint16_t count;
} wininfo_t;

typedef struct gamepersistent_s {
  uint32_t version;
  time_t   time;
  wininfo_t wins[8][3];
} gamepersistent_t;


extern gamepersistent_t*gamepersistent;

void FreeGamePersistent();
void InitGamePersistent();
