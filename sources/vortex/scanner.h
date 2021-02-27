// scanner consts & macros

#define	SCANNER_RANGE                  1024
#define	SCANNER_UPDATE_FREQ            2
#define	PIC_SCANNER                    "pics/scanner/scanner.pcx"
#define	PIC_SCANNER_TAG                "scanner/scanner"
#define	PIC_DOT                        "pics/scanner/dot.pcx"
#define	PIC_DOT_TAG                    "scanner/dot"
#define	PIC_ACIDDOT                    "pics/scanner/aciddot.pcx"
#define	PIC_ACIDDOT_TAG                "scanner/aciddot"
#define	PIC_INVDOT                     "pics/scanner/invdot.pcx"
#define	PIC_INVDOT_TAG                 "scanner/invdot"
#define	PIC_QUADDOT                    "pics/scanner/quaddot.pcx"
#define	PIC_QUADDOT_TAG                "scanner/quaddot"
#define	PIC_DOWN                       "pics/scanner/down.pcx"
#define	PIC_DOWN_TAG                   "scanner/down"
#define	PIC_UP                         "pics/scanner/up.pcx"
#define	PIC_UP_TAG                     "scanner/up"
#define	PIC_SCANNER_ICON               "pics/scanner/scanicon.pcx"
#define	PIC_SCANNER_ICON_TAG           "scanner/scanicon"
#define	SAFE_STRCAT(org,add,maxlen)    if ((strlen(org) + strlen(add)) < maxlen)    strcat(org,add);
#define	LAYOUT_MAX_LENGTH              1400

// scanner functions

void		Toggle_Scanner (edict_t *ent);
void		ShowScanner(edict_t *ent,char *layout);
void		ClearScanner(gclient_t *client);
qboolean	Pickup_Scanner (edict_t *ent, edict_t *other);