/* 
 * a_radio.h
 * 
 * Include file for use with radio stuff
 * -Fireblade
 */

typedef struct radio_msg_s 
{
        char *msg;  // the msg name 
        int length; // length in server frames (ie tenths of a second), rounded up
} radio_msg_t;

#define RADIO_CLICK                     "radio/click.wav"
#define RADIO_CLICK_LEN                 2
#define RADIO_DEATH_MALE                "radio/male/rdeath.wav"
#define RADIO_DEATH_MALE_LEN            27
#define RADIO_DEATH_FEMALE              "radio/female/rdeath.wav"
#define RADIO_DEATH_FEMALE_LEN          30

#define MAX_SOUNDFILE_PATH_LEN          32 // max length of a sound file path
#define MAX_RADIO_MSG_QUEUE_SIZE        4
#define MAX_RADIO_QUEUE_SIZE            6  // this must be at least 2 greater than the above

extern radio_msg_t male_radio_msgs[];
extern radio_msg_t female_radio_msgs[];

typedef struct radio_queue_entry_s
{
        char soundfile[MAX_SOUNDFILE_PATH_LEN];
        edict_t *from_player;
        int from_gender; // true if female
        qboolean now_playing;
        int length;
        qboolean click;
} radio_queue_entry_t;

void RadioThink(edict_t *);
void Cmd_Radio_f(edict_t *);
void Cmd_Radiogender_f(edict_t *);
void Cmd_Radio_power_f(edict_t *);
void Cmd_Radiopartner_f(edict_t *);
void Cmd_Radioteam_f(edict_t *);
void Cmd_Channel_f(edict_t *);
void Cmd_Say_partner_f(edict_t *);
void Cmd_Partner_f(edict_t *);
void Cmd_Deny_f(edict_t *);
void Cmd_Unpartner_f(edict_t *);
void PrecacheRadioSounds();
