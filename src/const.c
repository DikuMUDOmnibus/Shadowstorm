
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@hypercube.org)				   *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "include.h"


/* item type list */
const struct item_type		item_table	[]	=
{
    {	ITEM_LIGHT,	"light"		},
    {	ITEM_SCROLL,	"scroll"	},
    {	ITEM_WAND,	"wand"		},
    {   ITEM_STAFF,	"staff"		},
    {   ITEM_WEAPON,	"weapon"	},
    {   ITEM_TREASURE,	"treasure"	},
    {   ITEM_ARMOR,	"armor"		},
    {	ITEM_POTION,	"potion"	},
    {	ITEM_CLOTHING,	"clothing"	},
    {   ITEM_FURNITURE,	"furniture"	},
    {	ITEM_TRASH,	"trash"		},
    {	ITEM_CONTAINER,	"container"	},
    {	ITEM_DRINK_CON, "drink"		},
    {	ITEM_KEY,	"key"		},
    {	ITEM_FOOD,	"food"		},
    {	ITEM_MONEY,	"money"		},
    {	ITEM_BOAT,	"boat"		},
    {	ITEM_CORPSE_NPC,"npc_corpse"	},
    {	ITEM_CORPSE_PC,	"pc_corpse"	},
    {   ITEM_FOUNTAIN,	"fountain"	},
    {	ITEM_PILL,	"pill"		},
    {	ITEM_PROTECT,	"protect"	},
    {	ITEM_MAP,	"map"		},
    {	ITEM_PORTAL,	"portal"	},
    {	ITEM_WARP_STONE,"warp_stone"	},
    {	ITEM_ROOM_KEY,	"room_key"	},
    {	ITEM_GEM,	"gem"		},
    {	ITEM_JEWELRY,	"jewelry"	},
    {   ITEM_JUKEBOX,	"jukebox"	},
    {   ITEM_TOKEN,     "token"         },
    {   ITEM_SOURCE,    "source"        },
    {   ITEM_INSTRUMENT,"instrument"    },
    {   ITEM_SOCKET,    "socket",       },
    {	ITEM_QUIVER,	"quiver"	},
    {	ITEM_ARROW,     "arrow",	},
    {   ITEM_SHEATH,    "sheath",	},
    {   ITEM_SKELETON,  "skeleton",     },
    {   ITEM_ORE,	"ore",		},
    {   ITEM_SOCKETS,   "sockets",      },
    {   0,		NULL		}
};


/* weapon selection table */
const	struct	weapon_type	weapon_table	[]	=
{
   { "sword",	&OBJ_VNUM_SCHOOL_SWORD,	WEAPON_SWORD	},
   { "mace",	&OBJ_VNUM_SCHOOL_MACE,	WEAPON_MACE 	},
   { "dagger",	&OBJ_VNUM_SCHOOL_DAGGER,	WEAPON_DAGGER	},
   { "axe",	&OBJ_VNUM_SCHOOL_AXE,	WEAPON_AXE	},
   { "flail",	&OBJ_VNUM_SCHOOL_FLAIL,	WEAPON_FLAIL	},
   { "whip",	&OBJ_VNUM_SCHOOL_WHIP,	WEAPON_WHIP	},
   { "polearm", &OBJ_VNUM_SCHOOL_POLEARM,WEAPON_POLEARM	},
   { "bow",     &OBJ_VNUM_BOW,           WEAPON_BOW      },
   { NULL,	0,				0	}
};

/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",           WIZ_ON,         IM },
   {    "prefix",	WIZ_PREFIX,	IM },
   {    "ticks",        WIZ_TICKS,      IM },
   {    "logins",       WIZ_LOGINS,     IM },
   {    "sites",        WIZ_SITES,      L4 },
   {    "links",        WIZ_LINKS,      L7 },
   {	"newbies",	WIZ_NEWBIE,	IM },
   {	"spam",		WIZ_SPAM,	L5 },
   {    "deaths",       WIZ_DEATHS,     IM },
   {    "resets",       WIZ_RESETS,     L4 },
   {    "mobdeaths",    WIZ_MOBDEATHS,  L4 },
   {    "flags",	WIZ_FLAGS,	L5 },
   {	"penalties",	WIZ_PENALTIES,	L5 },
   {	"saccing",	WIZ_SACCING,	L5 },
   {	"levels",	WIZ_LEVELS,	IM },
   {	"load",		WIZ_LOAD,	L2 },
   {	"restore",	WIZ_RESTORE,	L2 },
   {	"snoops",	WIZ_SNOOPS,	L2 },
   {	"switches",	WIZ_SWITCHES,	L2 },
   {	"secure",	WIZ_SECURE,	L1 },
   {    "olc",          WIZ_OLC,        L4 },
   {	NULL,		0,		0  }
};

/* attack table  -- not very organized :( */
const 	struct attack_type	attack_table	[MAX_DAMAGE_MESSAGE]	=
{
    { 	"none",		"hit",		-1},  /*  0 */
    {	"slice",	"slice", 	DAM_SLASH	},	
    {   "stab",		"stab",		DAM_PIERCE	},
    {	"slash",	"slash",	DAM_SLASH	},
    {	"whip",		"whip",		DAM_SLASH	},
    {   "claw",		"claw",		DAM_SLASH	},  /*  5 */
    {	"blast",	"blast",	DAM_BASH	},
    {   "pound",	"pound",	DAM_BASH	},
    {	"crush",	"crush",	DAM_BASH	},
    {   "grep",		"grep",		DAM_SLASH	},
    {	"bite",		"bite",		DAM_PIERCE	},  /* 10 */
    {   "pierce",	"pierce",	DAM_PIERCE	},
    {   "suction",	"suction",	DAM_BASH	},
    {	"beating",	"beating",	DAM_BASH	},
    {   "digestion",	"digestion",	DAM_ACID	},
    {	"charge",	"charge",	DAM_BASH	},  /* 15 */
    { 	"slap",		"slap",		DAM_BASH	},
    {	"punch",	"punch",	DAM_BASH	},
    {	"wrath",	"wrath",	DAM_ENERGY	},
    {	"magic",	"magic",	DAM_ENERGY	},
    {   "divine",	"divine power",	DAM_HOLY	},  /* 20 */
    {	"cleave",	"cleave",	DAM_SLASH	},
    {	"scratch",	"scratch",	DAM_PIERCE	},
    {   "peck",		"peck",		DAM_PIERCE	},
    {   "peckb",	"peck",		DAM_BASH	},
    {   "chop",		"chop",		DAM_SLASH	},  /* 25 */
    {   "sting",	"sting",	DAM_PIERCE	},
    {   "smash",	 "smash",	DAM_BASH	},
    {   "shbite",	"shocking bite",DAM_LIGHTNING	},
    {	"flbite",	"flaming bite", DAM_FIRE	},
    {	"frbite",	"freezing bite", DAM_COLD	},  /* 30 */
    {	"acbite",	"acidic bite", 	DAM_ACID	},
    {	"chomp",	"chomp",	DAM_PIERCE	},
    {  	"drain",	"life drain",	DAM_NEGATIVE	},
    {   "thrust",	"thrust",	DAM_PIERCE	},
    {   "slime",	"slime",	DAM_ACID	},
    {	"shock",	"shock",	DAM_LIGHTNING	},
    {   "thwack",	"thwack",	DAM_BASH	},
    {   "flame",	"flame",	DAM_FIRE	},
    {   "chill",	"chill",	DAM_COLD	},
    {   "silver",       "silver",       DAM_SILVER      },
    {   "wind",         "wind",         DAM_WIND        },
    {   NULL,		NULL,		0		}
};

/* race table */
const 	struct	race_type	race_table	[]		=
{
/*
    {
	name,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts 
    },
*/
    { "unused",		FALSE, 0, 0, 0, 0, 0, 0, 0, 0 },

    { 
	"human",		TRUE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"elf",			TRUE,
	0,		AFF_INFRARED,	0,
	0,		RES_CHARM,	VULN_IRON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"dwarf",		TRUE,
	0,		AFF_INFRARED,	0,
	0,		RES_POISON|RES_DISEASE, VULN_DROWNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"giant",		TRUE,
	0,		0,		0,
	0,		RES_FIRE|RES_COLD,	VULN_MENTAL|VULN_LIGHTNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
        "drow",                 TRUE,
        0,              0,              0,
        0,              RES_NEGATIVE|RES_COLD|RES_CHARM,  VULN_HOLY|VULN_LIGHT,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },   

    {
        "duergar",              TRUE,
        0,              AFF_INFRARED,      0,
        0,              RES_PIERCE|RES_NEGATIVE,      VULN_LIGHT|VULN_DROWNING,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
        "orc",                  TRUE,
        0,              AFF_INFRARED,   0,
        0,              RES_DISEASE,    VULN_LIGHT,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },                                                                         



    {
	"bat",			FALSE,
	0,		AFF_FLYING|AFF_DARK_VISION,	OFF_DODGE|OFF_FAST,
	0,		0,		VULN_LIGHT,
	A|G|V,		A|C|D|E|F|H|J|K|P
    },

    {
	"bear",			FALSE,
	0,		0,		OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
	0,		RES_BASH|RES_COLD,	0,
	A|G|V,		A|B|C|D|E|F|H|J|K|U|V
    },

    {
	"cat",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V
    },

    {
	"centipede",		FALSE,
	0,		AFF_DARK_VISION,	0,
	0,		RES_PIERCE|RES_COLD,	VULN_BASH,
 	A|B|G|O,		A|C|K	
    },

    {
	"dog",			FALSE,
	0,		0,		OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|U|V
    },

    {
	"doll",			FALSE,
	0,		0,		0,
	IMM_COLD|IMM_POISON|IMM_HOLY|IMM_NEGATIVE|IMM_MENTAL|IMM_DISEASE
	|IMM_DROWNING,	RES_BASH|RES_LIGHT,
	VULN_SLASH|VULN_FIRE|VULN_ACID|VULN_LIGHTNING|VULN_ENERGY,
	E|J|M|cc,	A|B|C|G|H|K
    },

    { 	"dragon", 		FALSE, 
	0, 			AFF_INFRARED|AFF_FLYING,	0,
	0,			RES_FIRE|RES_BASH|RES_CHARM, 
	VULN_PIERCE|VULN_COLD,
	A|H|Z,		A|C|D|E|F|G|H|I|J|K|P|Q|U|V|X
    },

    {
	"fido",			FALSE,
	0,		0,		OFF_DODGE|ASSIST_RACE,
	0,		0,			VULN_MAGIC,
	A|B|G|V,	A|C|D|E|F|H|J|K|Q|V
    },		
   
    {
	"fox",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|V
    },

    {
	"hobgoblin",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE|RES_POISON,	0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|Y
    },

    {
	"lizard",		FALSE,
	0,		0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|cc,	A|C|D|E|F|H|K|Q|V
    },

    {
	"modron",		FALSE,
	0,		AFF_INFRARED,		ASSIST_RACE|ASSIST_ALIGN,
	IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
			RES_FIRE|RES_COLD|RES_ACID,	0,
	H,		A|B|C|G|H|J|K
    },

    {
	"pig",			FALSE,
	0,		0,		0,
	0,		0,		0,
	A|G|V,	 	A|C|D|E|F|H|J|K
    },	

    {
	"rabbit",		FALSE,
	0,		0,		OFF_DODGE|OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K
    },
    
    {
	"school monster",	FALSE,
	ACT_NOALIGN,		0,		0,
	IMM_CHARM|IMM_SUMMON,	0,		VULN_MAGIC,
	A|M|V,		A|B|C|D|E|F|H|J|K|Q|U
    },	

    {
	"snake",		FALSE,
	0,		0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|Y|cc,	A|D|E|F|K|L|Q|V|X
    },
 
    {
	"song bird",		FALSE,
	0,		AFF_FLYING,		OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },

    {
	"water fowl",		FALSE,
	0,		AFF_SWIM|AFF_FLYING,	0,
	0,		RES_DROWNING,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },		
  
    {
	"wolf",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|J|K|Q|V
    },

    {
	"wyvern",		FALSE,
	0,		AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,
	OFF_BASH|OFF_FAST|OFF_DODGE,
	IMM_POISON,	0,	VULN_LIGHT,
	A|B|G|Z,		A|C|D|E|F|H|J|K|Q|V|X
    },

    {
        "rat",                  FALSE,
        0,                      0,
        OFF_FAST,               0,
        0,                      VULN_POISON,
        A|G,                    A|C|D|E|F|H|J|K|Q
    },

    {
	"griffin",		FALSE,
	0,			0,
	0,			0,
	0,			0,
	A|B|G|Z,		A|C|D|E|F|H|J|K|Q|V|X
    },

    {
        "hawk",                 FALSE,
        0,			0,
    	0,			0,
 	0,			0,
        A|B|G|Z,		A|C|D|E||F|H|J|K|Q|V|X
    },

    {
	"unique",		FALSE,
	0,		0,		0,
	0,		0,		0,		
	0,		0
    },


    {
	NULL, 0, 0, 0, 0, 0, 0
    }
};

const	struct	pc_race_type	pc_race_table	[]	=
{
    { "null race", "", 0, { 100, 100, 100, 100 },
      { "" }, { 13, 13, 13, 13, 13 }, { 18, 18, 18, 18, 18 }, 0, 1 },
 
/*
    {
	"race name", 	short name, 	points,	{ klass multipliers },
	{ bonus skills },
	{ base stats },		{ max stats },		size 
    },
*/
    {
	"human",	"Human",	0,	
        { 100, 100, 100, 100, 100, 100, 100, 100 },
		{ "" },
		{ 13, 13, 13, 13, 13 },	{ 18, 18, 18, 18, 18 },	SIZE_MEDIUM, ALIGN_NEUTRAL
    },

    { 	
		"elf",		" Elf ",	5,	
	    { 100, 125,  100, 120, 100, 125, 100, 120 }, 
		{ "sneak", "hide" },
		{ 12, 14, 13, 15, 11 },	{ 16, 20, 18, 21, 15 }, SIZE_SMALL, ALIGN_GOOD
    },

    {
		"dwarf",	"Dwarf",	8,	
        { 150, 100, 125, 100, 150, 100, 125, 100 },
		{ "berserk" },
		{ 14, 12, 14, 10, 15 },	{ 20, 16, 19, 14, 21 }, SIZE_MEDIUM, ALIGN_GOOD
    },

    {
		"giant",	"Giant",	6,	
        { 200, 150, 150, 105, 200, 150, 150, 105 },
		{ "bash", "fast healing" },
		{ 16, 11, 13, 11, 14 },	{ 22, 15, 18, 15, 20 }, SIZE_LARGE, ALIGN_GOOD
    },

    {
        "drow",         "Drow",         6,    
        { 150, 100, 125, 100, 150, 100, 125, 100 },
        { "hide", "sneak" },     
        { 15, 18, 15, 18, 15 }, { 18, 20, 18, 20, 17 }, SIZE_SMALL, ALIGN_EVIL
    },

    {
        "duergar",      "Duer",      6,      
        { 150, 100, 125, 100, 150, 100, 125, 100 },
        { "hide", "sneak" },            
        { 17, 14, 20, 14, 14 }, { 19, 16, 22, 16, 16 }, SIZE_MEDIUM, ALIGN_EVIL
    },
   
    {
        "orc",          "Orc",      6,     
        { 200, 150, 150, 105, 200, 150, 150, 105 },
        { "bash" },
        { 17, 15, 17, 14, 13 }, { 19, 17, 19, 16, 15 }, SIZE_LARGE, ALIGN_EVIL
    },

};

	
      	

/*
 * Class table.
 */
const	struct	class_type	class_table	[MAX_CLASS]	=
{
    {
	"mage", "Mag",  STAT_INT,  &OBJ_VNUM_SCHOOL_DAGGER,
	{ 3018, 9618 },  75,  20, 6,  6,  8, TRUE,
	"mage basics", "mage default"
    },

    {
	"cleric", "Cle",  STAT_WIS,  &OBJ_VNUM_SCHOOL_MACE,
	{ 3003, 9619 },  75,  20, 2,  7, 10, TRUE,
	"cleric basics", "cleric default"
    },

    {
	"thief", "Thi",  STAT_DEX,  &OBJ_VNUM_SCHOOL_DAGGER,
	{ 3028, 9639 },  75,  20,  -4,  8, 13, FALSE,
	"thief basics", "thief default"
    },

    {
	"warrior", "War",  STAT_STR,  &OBJ_VNUM_SCHOOL_SWORD,
	{ 3022, 9633 },  75,  20,  -10,  11, 15, FALSE,
	"warrior basics", "warrior default"
    },

    {
        "wizard", "Wiz", STAT_INT, &OBJ_VNUM_SCHOOL_DAGGER,
        { 3018, 9618 }, 75, 20, 2, 7, 10, TRUE,
        "wizard basics", "wizard default"
    },

    {     
        "druid", "Drd", STAT_STR, &OBJ_VNUM_SCHOOL_MACE,
        { 3003, 9619 }, 75, 18, -2, 9, 12, TRUE,
        "druid basics", "druid default"
    },

    {
        "assassin", "Asn", STAT_DEX, &OBJ_VNUM_SCHOOL_DAGGER,
       	{ 3028, 9639 },  75,  17,  -8,  10, 15, FALSE, 
        "assassin basics", "assassin default"
    },

    {
        "barbarian", "Bar", STAT_STR, &OBJ_VNUM_SCHOOL_SWORD,
       	{ 3022, 9633 },  75,  15,  -12,  13, 17, FALSE,
        "barbarian basics", "barbarian default"
    }
    
};

/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[51]		=
{
    { -5, -4,   0,  0  },  /* 0  */
    { -5, -4,   3,  1  },  /* 1  */
    { -3, -2,   3,  2  },
    { -3, -1,  10,  3  },  /* 3  */
    { -2, -1,  25,  4  },
    { -2, -1,  55,  5  },  /* 5  */
    { -1,  0,  80,  6  },
    { -1,  0,  90,  7  },
    {  0,  0, 100,  8  },
    {  0,  0, 100,  9  },
    {  0,  0, 115, 10  }, /* 10  */
    {  0,  0, 115, 11  },
    {  0,  0, 130, 12  },
    {  0,  0, 130, 13  }, /* 13  */
    {  0,  1, 140, 14  },
    {  1,  1, 150, 15  }, /* 15  */
    {  1,  2, 165, 16  },
    {  2,  3, 180, 22  },
    {  2,  3, 200, 25  }, /* 18  */
    {  3,  4, 225, 30  },
    {  3,  5, 250, 35  }, /* 20  */
    {  4,  6, 300, 40  },
    {  4,  6, 350, 45  },
    {  5,  7, 400, 50  },
    {  5,  8, 425, 55  },
    {  6,  9, 440, 60  }, /* 25   */
    {  6,  9, 450, 63  },
    {  7, 10, 460, 66  },
    {  7, 10, 470, 69  },
    {  8, 11, 480, 72  },
    {  8, 11, 490, 75  }, /* 30	 */
    {  9, 12, 500, 78  },  
    {  9, 13, 510, 81  },
    { 10, 13, 520, 84  },
    { 11, 14, 530, 87  },
    { 11, 14, 540, 90  }, /* 35  */
    { 13, 15, 550, 93  },
    { 13, 15, 560, 96  },    
    { 14, 16, 570, 99  },
    { 14, 16, 580, 102 },
    { 15, 17, 590, 105 }, /* 40  */
    { 16, 18, 600, 108 },
    { 16, 19, 610, 111 },
    { 17, 19, 620, 114 },
    { 17, 20, 630, 117 },
    { 18, 20, 640, 120 }, /* 45  */
    { 18, 21, 650, 123 },
    { 19, 21, 660, 126 },
    { 19, 22, 670, 129 },
    { 20, 22, 680, 132 },
    { 21, 23, 690, 135 }  /* 50 */   
};



const	struct	int_app_type	int_app		[51]		=
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 30 },	/* 15 */
    { 32 },
    { 34 },
    { 36 },	/* 18 */
    { 38 },
    { 40 },	/* 20 */
    { 42 },
    { 44 },
    { 46 },
    { 48 },
    { 50 },	/* 25 */
    { 51 },
    { 52 },
    { 53 },
    { 54 },
    { 55 },     /* 30 */
    { 56 },
    { 58 },
    { 59 },
    { 60 },
    { 61 },     /* 35 */
    { 62 },
    { 63 },
    { 64 },
    { 65 },
    { 66 },     /* 40 */
    { 67 },
    { 69 },
    { 71 },
    { 73 },
    { 75 },     /* 45 */
    { 77 },
    { 79 },
    { 81 },
    { 83 },
    { 85 }      /* 50 */
};



const	struct	wis_app_type	wis_app		[51]		=
{
    { 0  },	/*  0 */
    { 0  },	/*  1 */
    { 0  },
    { 0  },	/*  3 */
    { 0  },
    { 1  },	/*  5 */
    { 1  },
    { 1  },
    { 1  },
    { 1  },
    { 1  },	/* 10 */
    { 1  },
    { 1  },
    { 1  },
    { 1  },
    { 2  },	/* 15 */
    { 2  },
    { 2  },
    { 3  },	/* 18 */
    { 3  },
    { 3  },	/* 20 */
    { 3  },
    { 4  },
    { 4  },
    { 4  },
    { 5  },	/* 25 */
    { 5  },
    { 5  },
    { 5  },
    { 6  },    
    { 6  },     /* 30 */
    { 6  },
    { 6  },
    { 7  },
    { 7  },
    { 7  },     /* 35 */
    { 8  },
    { 8  },
    { 8  },
    { 8  },
    { 9  },     /* 40 */
    { 9  },
    { 9  },
    { 9  },
    { 10 },
    { 10 },     /* 45 */
    { 10 },
    { 11 },
    { 11 },
    { 11 },
    { 12 }      /* 50 */
};



const	struct	dex_app_type	dex_app		[51]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 },   /* 25 */
    { -125 },   
    { -130 },
    { -135 },
    { -140 },
    { -145 },   /* 30 */
    { -150 },
    { -155 },
    { -160 },
    { -165 },
    { -170 },   /* 35 */
    { -175 },    
    { -180 },  
    { -185 },
    { -190 },
    { -195 },   /* 40 */
    { -200 },
    { -205 },
    { -210 },
    { -215 },
    { -220 },   /* 45 */
    { -225 },
    { -230 },
    { -235 },
    { -240 },
    { -245 }    /* 50 */
};


const	struct	con_app_type	con_app		[51]		=
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },	  /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  6, 99 },
    {  7, 99 },
    {  8, 99 },    /* 25 */
    {  8, 99 },
    {  8, 99 },
    {  9, 99 },
    {  9, 99 },
    { 10, 99 },    /* 30 */
    { 10, 99 },
    { 10, 99 },
    { 11, 99 },
    { 11, 99 },
    { 12, 99 },    /* 35 */
    { 12, 99 },
    { 12, 99 },
    { 13, 99 },
    { 13, 99 },
    { 14, 99 },    /* 40 */
    { 14, 99 },
    { 14, 99 }, 
    { 15, 99 },
    { 15, 99 },
    { 16, 99 },    /* 45 */
    { 16, 99 },
    { 16, 99 },
    { 17, 99 },    
    { 17, 99 },
    { 17, 99 }     /* 50 */
};



/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[]	=
{
/*    name			color	proof, full, thirst, food, ssize */
    { "water",			"clear",	{   0, 1, 10, 0, 16 }	},
    { "beer",			"amber",	{  12, 1,  8, 1, 12 }	},
    { "red wine",		"burgundy",	{  30, 1,  8, 1,  5 }	},
    { "ale",			"brown",	{  15, 1,  8, 1, 12 }	},
    { "dark ale",		"dark",		{  16, 1,  8, 1, 12 }	},

    { "whisky",			"golden",	{ 120, 1,  5, 0,  2 }	},
    { "lemonade",		"pink",		{   0, 1,  9, 2, 12 }	},
    { "firebreather",		"boiling",	{ 190, 0,  4, 0,  2 }	},
    { "local specialty",	"clear",	{ 151, 1,  3, 0,  2 }	},
    { "slime mold juice",	"green",	{   0, 2, -8, 1,  2 }	},

    { "milk",			"white",	{   0, 2,  9, 3, 12 }	},
    { "tea",			"tan",		{   0, 1,  8, 0,  6 }	},
    { "coffee",			"black",	{   0, 1,  8, 0,  6 }	},
    { "blood",			"red",		{   0, 2, -1, 2,  6 }	},
    { "salt water",		"clear",	{   0, 1, -2, 0,  1 }	},

    { "coke",			"brown",	{   0, 2,  9, 2, 12 }	}, 
    { "root beer",		"brown",	{   0, 2,  9, 2, 12 }   },
    { "elvish wine",		"green",	{  35, 2,  8, 1,  5 }   },
    { "white wine",		"golden",	{  28, 1,  8, 1,  5 }   },
    { "champagne",		"golden",	{  32, 1,  8, 1,  5 }   },

    { "mead",			"honey-colored",{  34, 2,  8, 2, 12 }   },
    { "rose wine",		"pink",		{  26, 1,  8, 1,  5 }	},
    { "benedictine wine",	"burgundy",	{  40, 1,  8, 1,  5 }   },
    { "vodka",			"clear",	{ 130, 1,  5, 0,  2 }   },
    { "cranberry juice",	"red",		{   0, 1,  9, 2, 12 }	},

    { "orange juice",		"orange",	{   0, 2,  9, 3, 12 }   }, 
    { "absinthe",		"green",	{ 200, 1,  4, 0,  2 }	},
    { "brandy",			"golden",	{  80, 1,  5, 0,  4 }	},
    { "aquavit",		"clear",	{ 140, 1,  5, 0,  2 }	},
    { "schnapps",		"clear",	{  90, 1,  5, 0,  2 }   },

    { "icewine",		"purple",	{  50, 2,  6, 1,  5 }	},
    { "amontillado",		"burgundy",	{  35, 2,  8, 1,  5 }	},
    { "sherry",			"red",		{  38, 2,  7, 1,  5 }   },	
    { "framboise",		"red",		{  50, 1,  7, 1,  5 }   },
    { "rum",			"amber",	{ 151, 1,  4, 0,  2 }	},

    { "cordial",		"clear",	{ 100, 1,  5, 0,  2 }   },
    { "magic potion",         "multi-coloured", { 0,   1, 10, 0, 16 }   }, 
    { NULL,			NULL,		{   0, 0,  0, 0,  0 }	}
};

const   struct  group_type      group_table     [MAX_GROUP]     =
{

    {
	"rom basics",		{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "scrolls", "staves", "wands", "recall" }
    },

    {
	"mage basics",		{ 0, -1, -1, -1, -1, -1, -1, -1 },
	{ "dagger" }
    },

    {
	"cleric basics",	{ -1, 0, -1, -1, -1, -1, -1, -1 },
	{ "mace" }
    },
   
    {
	"thief basics",		{ -1, -1, 0, -1, -1, -1, -1, -1 },
	{ "dagger", "steal" }
    },

    {
	"warrior basics",	{ -1, -1, -1, 0, -1, -1, -1, -1 },
	{ "sword", "second attack" }
    },

   
    {
	"wizard basics",	{ -1, -1, -1, -1, 0, -1, -1, -1 },
	{ "dagger" }
    },

    {
	"druid basics",	        { -1, -1, -1, -1, -1, 0, -1, -1 },
	{ "mace" }
    },
   
    {
	"assassin basics",	{ -1, -1, -1, -1, -1, -1, 0, -1 },
	{ "dagger", "steal" }
    },

    {
	"barbarian basics",	{ -1, -1, -1, -1, -1, -1, -1, 0 },
	{ "sword", "second attack" }
    },

    {
	"mage default",		{ 40, -1, -1, -1, -1, -1, -1, -1 },
	{ "lore", "beguiling", "combat", "detection", "enhancement", "illusion",
	  "maladictions", "protective", "transportation", "weather" }
    },

    {
	"cleric default",	{ -1, 40, -1, -1, -1, -1, -1, -1 },
	{ "flail", "attack", "creation", "curative",  "benedictions", 
	  "detection", "healing", "maladictions", "protective", "shield block", 
	  "transportation", "weather" }
    },
 
    {
	"thief default",	{ -1, -1, 40, -1, -1, -1, -1, -1 },
	{ "mace", "sword", "backstab", "disarm", "dodge", "second attack",
	  "trip", "hide", "peek", "pick lock", "sneak" }
    },

    {
	"warrior default",	{ -1, -1, -1, 40, -1, -1, -1, -1 },
	{ "weaponsmaster", "shield block", "bash", "disarm", "enhanced damage", 
	  "parry", "rescue", "third attack" }
    },

    {
	"wizard default",	{ -1, -1, -1, -1, 40, -1, -1, -1 },
	{ "lore", "beguiling", "combat", "detection", "enhancement", "illusion",
	  "maladictions", "protective", "transportation", "weather" }
    },

    {
	"druid default",	{ -1, -1, -1, -1, -1, 40, -1, -1 },
	{ "flail", "attack", "creation", "curative",  "benedictions", 
	  "detection", "healing", "maladictions", "protective", "shield block", 
	  "transportation", "weather" }
    },
 
    {
	"assassin default",	{ -1, -1, -1, -1, -1, -1, 40, -1 },
	{ "mace", "sword", "backstab", "disarm", "dodge", "second attack",
	  "trip", "hide", "peek", "pick lock", "sneak" }
    },

    {
	"barbarian default",	{ -1, -1, -1, -1, -1, -1, -1, 40 },
	{ "weaponsmaster", "shield block", "bash", "disarm", "enhanced damage", 
	  "parry", "rescue", "third attack" }
    },

    {
	"weaponsmaster",	{ 40, 40, 40, 20, 40, 40, 40, 20 },
	{ "axe", "dagger", "flail", "mace", "polearm", "spear", "sword", "whip",
          "bow" }
    },

    {
	"attack",		{ -1, 5, -1, 8, -1, 4, -1, 7 },
	{ "demonfire", "dispel evil", "dispel good", "earthquake", 
	  "flamestrike", "heat metal", "ray of truth" }
    },

    {
	"beguiling",		{ 4, -1, 6, -1, 3, -1, 5, -1 },
	{ "calm", "charm person", "sleep" }
    },

    {
	"benedictions",		{ -1, 4, -1, -1, -1, 3, -1, -1 },
	{ "bless", "calm", "frenzy", "holy word", "remove curse" }
    },

    {
	"combat",		{ 6, -1, 10, 9, 5, -1, 9, 8 },
	{ "acid blast", "burning hands", "chain lightning", "chill touch",
	  "colour spray", "fireball", "lightning bolt", "magic missile",
	  "shocking grasp"  }
    },

    {
	"creation",		{ 4, 4, 8, 8, 3, 3, 7, 7 },
	{ "continual light", "create food", "create spring", "create water",
	  "create rose", "floating disc" }
    },

    {
	"curative",		{ -1, 4, -1, 8, -1, 3, -1, 7 },
	{ "cure blindness", "cure disease", "cure poison" }
    }, 

    {
	"detection",		{ 4, 3, 6, -1, 3, 2, 5, -1,  },
 	{ "detect evil", "detect good", "detect hidden", "detect invis", 
	  "detect magic", "detect poison", "farsight", "identify", 
	  "know alignment", "locate object" } 
    },

    {
	"draconian",		{ 8, -1, -1, -1, 7, -1, -1, -1 },
	{ "acid breath", "fire breath", "frost breath", "gas breath",
	  "lightning breath"  }
    },

    {
	"enchantment",		{ 6, -1, -1, -1, 5, -1, -1, -1 },
	{ "enchant armor", "enchant weapon", "fireproof", "recharge" }
    },

    { 
	"enhancement",		{ 5, -1, 9, 9, 4, -1, 8, 8,},
	{ "giant strength", "haste", "infravision", "refresh" }
    },

    {
	"harmful",		{ -1, 3, -1, -1, -1, 2, -1, -1  },
	{ "cause critical", "cause light", "cause serious", "harm" }
    },

    {   
	"healing",		{ -1, 3, -1, -1, 2, -1, -1  },
 	{ "cure critical", "cure light", "cure serious", "heal", 
	  "mass healing", "refresh" }
    },

    {
	"illusion",		{ 4, -1, 7, -1, 3, -1, 6, -1 },
	{ "invis", "mass invis", "ventriloquate" }
    },
  
    {
	"maladictions",		{ 5, 4, -1, -1, 4, 3, -1, -1 },
	{ "blindness", "change sex", "curse", "energy drain", "plague", 
	  "poison", "slow", "weaken" }
    },

    { 
	"protective",		{ 4, 4, -1, -1, 3, 3, -1, -1 },
	{ "armor", "cancellation", "dispel magic", "fireproof",
	  "protection evil", "protection good", "sanctuary", "shield", 
	  "stone skin" }
    },

    {
	"transportation",	{ 4, 4, 8, 9, 3, 3, 7, 8 },
	{ "fly", "gate", "nexus", "pass door", "portal", "summon", "teleport", 
	  "word of recall" }
    },
   
    {
	"weather",		{ 4, 4, -1, -1, 3, 3, -1, -1 },
	{ "call lightning", "control weather", "faerie fire", "faerie fog",
	  "lightning bolt" }
    }
	
   

};


const struct weight_type weight_table [ ] =
{
    { "slender" },
    { "slim"    },
    { "average" },
    { "stocky"  },
    { "heavy"   },
    { NULL      }
};                   

const struct eye_type eye_table [] =
{
    { "blue"   },
    { "green"  },
    { "brown"  },
    { "black"  },
    { "white"  },
    { "clear"  },
    { "red"    },
    { NULL     }
};
      
const struct height_type height_table [] =
{
    { "short"    },
    { "midget"   },
    { "average"  },
    { "tall"     },
    { "giant"    },
    { NULL       }
};     


const struct hair_type hair_table [] =
{
    { "blond"   },
    { "red"     },
    { "black"   },
    { "brown"   },
    { "white"   },
    { "blue"    },
    { NULL      }
};       

