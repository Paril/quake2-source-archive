//*************************************************************************************************************
//*************************************************************************************************************
// File: shared.h
//*************************************************************************************************************
//*************************************************************************************************************

#define GDS_IP "134.193.111.9"
//#define GDS_IP "10.10.5.80"


#define GDS_PORT     "6501"
#define GDS_PASSWORD "kotsv525"

#define KOTS_CHECKIN_PLAYER 12 //players are saved every 10 kills/deaths

#define	LINELEN	1024
#define	WSVERS	MAKEWORD(2, 0)

#define ADMIN_TEMPLATE "\\orbit\\player_admin.html" 
#define ARMORY_TEMPLATE "\\orbit\\player_purchase.html" 
#define STAT_TEMPLATE "\\orbit\\player_stats.html" 
#define WTRAIN_TEMPLATE "\\orbit\\player_wtrain.html" 
#define PTRAIN_TEMPLATE "\\orbit\\player_ptrain.html" 

//action codes:
#define CHECKIP		       1
#define CHECKOUT	       2
#define CHECKIN		       3
#define UPDATE_WEB_USER	 4
#define UPDATE_WTRAINING 5
#define UPDATE_PTRAINING 6
#define CHANGE_NAME      7
#define CHANGE_PASSWORD  8
#define ADMIN_FUNCS      9

#define KOTS_SUCCESS			  0
#define KOTS_ERROR				  1
#define KOTS_NOFILE				  2
#define KOTS_INVALID_SERVER 3
#define KOTS_INVALID_S_PASS 4   // Server error
#define KOTS_INVALID_P_PASS 5
#define KOTS_BAD_FORMAT		  6
#define KOTS_SERVNOTREG			7		// Server error
#define KOTS_NOACCESS			  8
#define KOTS_BADFILEIO		  9
#define KOTS_LOADING		    10
#define KOTS_NEW      	    11
#define KOTS_INGAME         12
#define KOTS_NO_MONEY				13
#define KOTS_BANNED 				14
#define KOTS_NAME_EXISTS		15
#define KOTS_BAD_CONFIRM		16
#define KOTS_WARN_POINTS		17
#define KOTS_DELETED    		18
#define KOTS_RESTORE    		19

//define characters:
#define SEPARATOR				'\b'	//backspace character
#define SSEPARATOR			"\b"	//backspace character

#define KOTS_HEADSIZE 9

//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************

