/*  Amount of Damage caused */
#define	KAMIKAZE_DAMAGE			300

/* Radius of blast */
#define	KAMIKAZE_DAMAGE_RADUIS	400 
// Quake Units

/* Count down time */
//Gregg-Make it longer to match 10 second countdown
//#define KAMIKAZE_BLOW_TIME     30 // 1/10 seconds
#define KAMIKAZE_BLOW_TIME     110 // 1/10 seconds

void		Start_Kamikaze_Mode(edict_t *the_doomed_one);
 // setup and start self destruct mode
qboolean		Kamikaze_Active(edict_t *the_doomed_one);
void		Kamikaze_Explode(edict_t *the_doomed_one);
void		Kamikaze_Cancel(edict_t *the_spared_one);
