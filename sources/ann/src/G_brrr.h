
#include "q_devels.h"

/* *** ANNIHILATION VERSTION *** */
#define BRRR_VERSION "1.23-Final-r2.6"
/* *** */

/* *** Development mode *** */
/* This was used for debugging, etc while the game was being developed */
#undef  DEVMODE_PSW			/* It is undefined, and therefore disabled */
/* *** Development mode *** */

/* *** DM FLAGS *** */
#define DF_ANN_CTF_BASESTART   1048576

/* *** COMMON *** */
#define TIMER_DELAY                  5
#define WELCOME_TIME                20
void nothing();
void brrr_welcome(edict_t *ent);
void brrr_memos(edict_t *ent);
void single_only(edict_t *ent);
void disarm(edict_t *self);
void Cmd_Shotguntype_f(edict_t *ent);
void Cmd_Grenadetype_f(edict_t *ent);
void Cmd_Rockettype_f(edict_t *ent);
void Cmd_Glaunchertype_f(edict_t *ent);
void Cmd_HyperBlastertype_f(edict_t *ent);
void Cmd_Railtype_f(edict_t *ent);
void Cmd_Bfgtype_f(edict_t *ent);
void Grenade_Explode(edict_t *ent);
void status_prehash(edict_t *ent);
void calcbs(edict_t *ent);
/* qboolean ctf_map (void); */

/* *** BRRR STATUSBAR *** */
#define STAT_BRRR_WEAPON_ICON       16  // CTF uses up to 27, but skips 16
#define STAT_BRRR_ROCKETS           28
#define STAT_BRRR_ROCKETS_ICON      29
#define STAT_BRRR_CELLS             30
#define STAT_BRRR_CELLS_ICON        31  // Waaa! Out of stat fields now!

/* *** FAKE DEATH *** */
void player_fake_die(edict_t *self);
/* *** */

/* *** SNIPER ZOOM *** */
void ZoomIn(edict_t *ent);
void ZoomOut(edict_t *ent);
/* *** */

/* *** KAMIKAZE MODE *** */
#define KAMIKAZE_DAMAGE         100000  // Damage
#define KAMIKAZE_DAMAGE_RADIUS     512  // Blast radius - Quake units
#define KAMIKAZE_BLOW_TIME           5  // Countdown time (seconds)
#define KAMIKAZE_HEALTH             40  // Minimum health
#define KAMIKAZE_ROCKETS            10  // Rockets needed

/* *** CLOAKING *** */
#define CLOAK_ACTIVATE_TIME        1.5  // Cloak after 1.5 seconds
#define CLOAK_DRAIN                  8  // Every CLOAK_DRAIN frames...
#define CLOAK_AMMO                   1  // ...drain CLOAK_AMMO cells
void    Cmd_Cloak_f(edict_t *ent);

/* *** CLOAKING JAMMER *** */
#define JAMMER_RADIUS              256
#define JAMMER_DRAIN                20  // Every JAMMER_DRAIN frames...
#define JAMMER_AMMO                  1  // ...drain JAMMER_AMMO cells
void    Cmd_Jammer_f(edict_t *ent);
void    jammerthink(edict_t *ent);

/* *** PORTABLE TELEPORTER *** */
#define TELEPORT_AMMO               40  // Cells used to teleport
#define TELEPORT_HEALTH             40  // Minimum health

void    Cmd_Store_Teleport_f (edict_t *ent);
void    Cmd_Load_Teleport_f (edict_t *ent);

/* *** ANTI-GRAVITY BOOTS *** */
#define FL_BOOTS            0x00002000  // Anti-gravity boots flag
#define BOOTS_ROCKETS                1  // Rockets used each antigrav jump

/* *** HOMING MISSILES *** */
#define HOMING_ROCKETS               3  // Rockets needed for a homing missile
void homing_think (edict_t *ent);
void home_ent (edict_t *ent, int count);

/* *** DEFENSE LASER *** */
#define LASER_ARMING_TIME            3
#define CELLS_FOR_LASER             20
#define LASER_DAMAGE               200
#define LASER_MOUNT_DAMAGE          50
#define LASER_MOUNT_DAMAGE_RADIUS   64

void    PlaceLaser (edict_t *ent);

void    target_laser_use (edict_t *self, edict_t *other, edict_t *activator);
void    target_laser_think (edict_t *self);
void    target_laser_on (edict_t *self);
void    target_laser_off (edict_t *self);
void    killlaser (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void    removelaser (edict_t *self);
void    disarmlaser (edict_t *self);

/* *** KAMIKAZE MODE *** */
void        Start_Kamikaze_Mode(edict_t *the_doomed_one); // setup and start self destruct mode
qboolean    Kamikaze_Active(edict_t *the_doomed_one);
void        Kamikaze_Explode(edict_t *the_doomed_one);
void        Kamikaze_Cancel(edict_t *the_spared_one);

/* Funky Functions */
qboolean fileexists(char *filename);
void trimstring(char *buffer);

/* *** X-RADIO *** */
extern cvar_t      *radio_power;
void X_Radio_f(edict_t *self, char channel, char *msg, char *person);

/* *** REXEC *** */
void RExec(edict_t *self, char type, char *rcmd, char *person);
void Admin_Who_f(edict_t *self);

void Stealth(edict_t *ent);

/* *** PIPEBOMBS *** */
#define PIPEBOMB_SEEK_RADIUS      1536
#define PIPEBOMB_DAMAGE            250
#define PIPEBOMB_DAMAGE_RADIUS     250
#define PIPEBOMB_GRENADES            3

void Cmd_Detpipes_f(edict_t *ent);
void killpipe(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void removepipe(edict_t *self);
void PipeBomb_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void invalidpipe(edict_t *ent);
int  numpipes (edict_t *ent);

/* *** PROXIMITY MINES *** */
#define PROXIMITY_DAMAGE           100
#define PROXIMITY_DAMAGE_RADIUS     96
#define PROXIMITY_GRENADES           3
void Proximity_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void proximity_think(edict_t *ent);
void killprox (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void invalidprox(edict_t *ent);
void nonstaticprox(edict_t *ent);
void removeprox (edict_t *self);
int  numprox (edict_t *ent);

/* *** BFG ARMOUR *** */
#define BFGARMOUR_RANGE            512
#define BFGARMOUR_TIME               3

void bfg_armor_think (edict_t *self);
void Use_BFGArmor (edict_t *self);

/* *** NUKES *** */
#define NUKE_ROCKETS                20
#define NUKE_DAMAGE_RADIUS        1024
#define NUKE_DAMAGE             100000
#define NUKE_SPEED           speed/2.2
#define NUKE_BLINDTIME              40
void Nuke_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);

/* *** CLUSTERS *** */
#define CLUSTER_GRENADES             2
void Cluster_Explode (edict_t *ent);

/* *** FLARES *** */
#define FLARE_DAMAGE                 0
#define FLARE_DAMAGE_RADIUS          0
#define FLARE_TIME                  30
void killflare (edict_t *self);

/* *** AIRSTRIKE *** */
#define AIRSTRIKE_TIME               7  // Airstrike delay will be 7 +/- 2 seconds
#define AIRSTRIKE_PLUSMINUS          2
#define AIRSTRIKE_PAYLOAD           10
#define AIRSTRIKE_CALLSIGN      "PS33"
#define AIRSTRIKE_ROCKETS           15
void Cmd_Airstrike_f (edict_t *ent);
void killmarker (edict_t *ent);

/* *** JETPACK *** */
#define JET_MAXFUEL                 45  // Max fuel in seconds

/* *** PICKUP FUNCTIONS *** */
void Pickup_BFGSuit (edict_t *thing, edict_t *ent);
void Pickup_Jetpack (edict_t *thing, edict_t *ent);

/* *** SNIPER RIFLE *** */
#define SNIPER_DAMAGE              500
#define SNIPER_KICK                500
#define SNIPER_SLUGS                 4

/* *** NUKE *** */
void nuke_die(edict_t *ent);

/* *** BOUNCE ROCKETS *** */
#define BOUNCE_ROCKETS               2
#define BOUNCE_STAYALIVE             5

/* *** GUIDED ROCKETS *** */
#define GUIDED_ROCKETS               5
#define GUIDED_SPEED           speed/2
void guideThink (edict_t *self);

/* *** PHASER *** */
#define PHASER_TIME                  3
#define PHASER_CELLS               100
void fire_phaser (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);
void phaser_think (edict_t *ent);

/* *** FREEZER *** */
#define FREEZER_DAMAGE               5  // Damage to do if can't freeze
#define FREEZER_MINHEALTH           30  // Minimum health to actually freeze player
#define FREEZER_TIME                60  // 10ths of a second - probably
void freezer_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

/* *** LASERDRONE *** */
#define DRONE_CELLS                 50
#define DRONE_FRAGS                  4
void Cmd_LaserDrone_f(edict_t *ent);

/* *** AUTO SHOTGUN *** */
#define AUTO_SHOTS                   5

/* *** ADDED ITEMS *** */
qboolean spawnitembyclassname(vec3_t spot, char *classname, qboolean real);
void activateitem(edict_t *item);
void Cmd_Clearitems_f(edict_t *ent);
void Cmd_Undoitem_f(edict_t *ent);
void Touch_NewItem(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void Cmd_Listitems_f(edict_t *ent);
void Cmd_Saveitems_f(edict_t *ent);
qboolean saveitemdata(edict_t *ent);
qboolean loaditemdata(void);

typedef struct newitem_s
{
     edict_t *ent;
     struct  newitem_s *next;
} newitem_t;

extern newitem_t    *newitem_head;
extern qboolean     erasing;

/* *** CONVERTING *** */
void Cmd_Convert_f(edict_t *ent, char *arg1, char *arg2);
void convert(edict_t *ent);

/* *** TIME *** */
char *tomtime(void);

/* *** RESPAWN PROTECTION *** */
extern cvar_t       *respawn_protect;
/* *** */

/* *** RADAR *** */
#define RADAR_RANGE               4096
#define RADAR_CELLS                  1
#define RADAR_FRAMES                 2
void Cmd_Radar_f(edict_t *ent);
void Cmd_Radarzoomin_f(edict_t *ent);
void Cmd_Radarzoomout_f(edict_t *ent);
void radar_on(edict_t *ent);
void radar_off(edict_t *ent);
void radar_update(edict_t *ent);
/* *** */

/* *** TRIG HELPER FUNCTIONS *** */
#define PI               3.14159265359
#define degsin(in)      sin(in*PI/180)
//float degsin(float in);
/* *** */

/* *** DISRUPTOR RIFLE *** */
#define DISRUPTOR_DAMAGE            40
#define DISRUPTOR_CELLS             20
#define DISRUPTOR_STAYALIVE        4.5
void disruptor_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void disruptor_think(edict_t *ent);

/* *** FLASHLIGHT *** */
#define FLASHLIGHT_CELLS             1
#define FLASHLIGHT_FRAMES           20

/* *** IR GOGGLES *** */
#define RF_IR_VISIBLE       0x00008000      // 32768
#define RDF_IRGOGGLES       4
#define IR_CELLS                     1
#define IR_FRAMES                    5
void Cmd_Goggles_f (edict_t *ent);
void draingoggles(edict_t *ent);

/* *** REGEN CELLS *** */
#define REGENCELLS_FRAMES           15

/* *** MAP ROTATION *** */
#define MR_SEQUENTIAL   1
#define MR_RANDOM       2

typedef struct
{
    int         maxindex;
    int         mapindex;
    char        *mapname;
    qboolean    *mapused;
} maplist_t;

void initrotate(void);
void listmaps(void);
void firstmap(void);
void nextmap(void);
void reloadmaplist(void);
edict_t *maprotate(qboolean reset);
qboolean maperr(void);

extern  maplist_t   maplist;
extern  cvar_t      *rotate;
extern  cvar_t      *rotate_filename;
/* *** */

/* *** LIMITS *** */
extern  cvar_t      *laser_limit;
extern  cvar_t      *prox_limit;
extern  cvar_t      *pipe_limit;
/* *** */

void adminnotify(edict_t *ent, char *string1, char *string2);

