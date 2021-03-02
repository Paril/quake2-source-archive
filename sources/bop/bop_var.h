const float n_laghelp_str = 0.50; // Our game constants, for the purpose of easy modification
const float n_laghelp_rst = 0.50;
const int n_max_delta_ping = 1000;
const int n_min_delta_ping = 100;

const float x_laghelp_str = 0.07; // Game constants for Expert Mode
const float x_laghelp_rst = 0.15;
const int x_max_delta_ping = 1000;
const int x_min_delta_ping = 225;

const float t_laghelp_str = 0;	// Game constants for Tournament Mode
const float t_laghelp_rst = 0;
const int t_max_delta_ping = 500;
const int t_min_delta_ping = 500;	// doesn't matter, but 0 might cause trouble

#define BOP_VERSION 0.92
#define BOP_STAGE	2	// 0 = Alpha, 1 = Beta, 2 = Public

#define NINETYDEGS 1.5708	// PI divided by 2

int max_delta_ping = 1000;
int min_delta_ping = 100;
float laghelp_str = 0.15;	// laghelp strength
float laghelp_rst = 0.15;	// laghelp resistance

unsigned char game_mode = 0;	// 0 = Normal, 1 = Xpert, 2 = Tourny

const float bop_flareup = 5;
float bop_deadtime = 600;
float bop_getawaytime = 40;
float lagsicktime = 550;
int lag_threshold = 1000;
int lagged_out_detect = 30;	// give them 3 seconds for a fluctuation
const float msg_interval = 20;
const float short_msg_interval = 10;
qboolean no_suicide_protect = 1;
qboolean no_slag_protect = 1;
qboolean bop_verbose = 0;
qboolean bop_invisible = 1;
qboolean nobophelp = 0;
qboolean bopsickremove = 1;
qboolean bopmanageobservers = 1; // 0 if you run into observer trouble

char bop_twoarg[81];	// this is admittedly a hack.. a global variable
			// used to store all gi.argv[2] information for
			// the purpose of using a bop.cfg file and parsing
			// stuff from it independantly
			// Only one server operator, so we can get away with this
