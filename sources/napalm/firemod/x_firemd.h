/*==========================================================================
//  x_firemd.h -- by Patrick Martin             Last updated:  2-27-1999
//--------------------------------------------------------------------------
//  This file contains definitions for the data in my Napalm2 fire model.
//
//  Frames:
//     0 -  2  Small flame expansion (used for fireball creation)
//     3 - 14  Small fire (used for fireballs and the like)
//    15 - 20  Large flame expansion (used for burning entities)
//    21 - 32  Large fire (used for burning entities)
//    33 - 38  Huge flare (NOT used)
//         39  Tiny spark (near invisible; used for smoke)
//         40  Base frame (NOT used)
//
//  Skins:
//     0  Orange fire
//
//  WARNING:  Do NOT modify!!!
//========================================================================*/

/***************/
/*  CONSTANTS  */
/***************/

/* File path to Napalm2 flame model. */
#define MD2_FIRE        gi.modelindex("models/fire/tris.md2")

/* Skins in Napalm2 flame model. */
#define SKIN_FIRE_skin  0

/* Frames in Napalm2 flame model. */
#define FRAME_FIRE_ignite1      0
#define FRAME_FIRE_ignite2      1
#define FRAME_FIRE_ignite3      2
#define FRAME_FIRE_fire1        3
#define FRAME_FIRE_fire2        4
#define FRAME_FIRE_fire3        5
#define FRAME_FIRE_fire4        6
#define FRAME_FIRE_fire5        7
#define FRAME_FIRE_fire6        8
#define FRAME_FIRE_fire7        9
#define FRAME_FIRE_fire8       10
#define FRAME_FIRE_fire9       11
#define FRAME_FIRE_fire10      12
#define FRAME_FIRE_fire11      13
#define FRAME_FIRE_fire12      14
#define FRAME_FIRE_igniteb1    15
#define FRAME_FIRE_igniteb2    16
#define FRAME_FIRE_igniteb3    17
#define FRAME_FIRE_igniteb4    18
#define FRAME_FIRE_igniteb5    19
#define FRAME_FIRE_igniteb6    20
#define FRAME_FIRE_fireb1      21
#define FRAME_FIRE_fireb2      22
#define FRAME_FIRE_fireb3      23
#define FRAME_FIRE_fireb4      24
#define FRAME_FIRE_fireb5      25
#define FRAME_FIRE_fireb6      26
#define FRAME_FIRE_fireb7      27
#define FRAME_FIRE_fireb8      28
#define FRAME_FIRE_fireb9      29
#define FRAME_FIRE_fireb10     30
#define FRAME_FIRE_fireb11     31
#define FRAME_FIRE_fireb12     32
#define FRAME_FIRE_flare1      33
#define FRAME_FIRE_flare2      34
#define FRAME_FIRE_flare3      35
#define FRAME_FIRE_flare4      36
#define FRAME_FIRE_flare5      37
#define FRAME_FIRE_flare6      38
#define FRAME_FIRE_nil1        39
#define FRAME_FIRE_base        40


/*===========================/  END OF FILE  /===========================*/
