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
#include "merc.h"
#include "magic.h"
#include "tables.h"
#include "const.h"

/* for position */
const struct position_type position_table[] =
{
    {	"dead",			"dead"	},
    {	"mortally wounded",	"mort"	},
    {	"incapacitated",	"incap"	},
    {	"stunned",		"stun"	},
    {	"sleeping",		"sleep"	},
    {	"resting",		"rest"	},
    {   "sitting",		"sit"   },
    {	"fighting",		"fight"	},
    {	"standing",		"stand"	},
    {	NULL,			NULL	}
};

/* for sex */
const struct sex_type sex_table[] =
{
   {	"none"		},
   {	"male"		},
   {	"female"	},
   {	"either"	},
   {	NULL		}
};

//Stances by Majik
const struct stance_type stance_table[] = 
{
    { "neutral"		},
    { "aggressive"	},
    { "defensive"	},
    {NULL}
};

/* for sizes */
const struct size_type size_table[] =
{ 
    {	"tiny"		},
    {	"small" 	},
    {	"medium"	},
    {	"large"		},
    {	"huge", 	},
    {	"giant" 	},
    {	NULL		}
};

/* various flag tables */
const struct flag_type act_flags[] =
{
    {   "forger",               D,      TRUE    },
    {	"npc",			A,	FALSE	},
    {	"sentinel",		B,	TRUE	},
    {	"scavenger",		C,	TRUE	},
    {   "bounty",               E,      TRUE    },
    {	"aggressive",		F,	TRUE	},    
    {	"stay_area",		G,	TRUE	},
    {	"wimpy",		H,	TRUE	},
    {	"pet",			I,	TRUE	},
    {	"train",		J,	TRUE	},
    {	"practice",		K,	TRUE	},
    {	"mount",		L,	TRUE	},    
    {	"undead",		O,	TRUE	},
    {	"cleric",		Q,	TRUE	},
    {	"mage",			R,	TRUE	},
    {	"thief",		S,	TRUE	},
    {	"warrior",		T,	TRUE	},
    {	"noalign",		U,	TRUE	},
    {	"nopurge",		V,	TRUE	},
    {	"outdoors",		W,	TRUE	},
    {	"indoors",		Y,	TRUE	},
    {	"healer",		aa,	TRUE	},
    {	"gain",			bb,	TRUE	},
    {	"update_always",	cc,	TRUE	},
    {	"changer",		dd,	TRUE	},
    {   "leveler",              ee,     TRUE    },
    {	NULL,			0,	FALSE	}
};

const struct flag_type socket_flags[] =
{
    {   "sapphire",	SOC_SAPPHIRE,	TRUE	},
    {   "ruby",         SOC_RUBY,	TRUE	},
    {   "emerald", 	SOC_EMERALD,	TRUE	},
    {	"diamond",  	SOC_DIAMOND,	TRUE	},
    {   "topaz", 	SOC_TOPAZ,	TRUE	},
    { 	"skull",	SOC_SKULL,	TRUE	},
    {   NULL,		0,		0 	},
};

const struct flag_type socket_values[] =
{
    {	"chipped",      GEM_CHIPPED,    TRUE	},
    {	"flawed",	GEM_FLAWED, 	TRUE	},
    {	"flawless",	GEM_FLAWLESS, 	TRUE	},
    {	"perfect",	GEM_PERFECT,	TRUE	},
    {	NULL,		0,		0	},
};

const struct flag_type plr_flags[] =
{
    {	"npc",			A,	FALSE	},
    {	"autoassist",		C,	FALSE	},
    {   "autodamage",		D,	FALSE	},
    {	"autoexit",		E,	FALSE	},
    {	"autoloot",		F,	FALSE	},
    {	"autosac",		G,	FALSE	},
    {	"autogold",		H,	FALSE	},
    {	"autosplit",		I,	FALSE	},
    {	"holylight",		N,	FALSE	},
    {	"can_loot",		P,	FALSE	},
    {	"nosummon",		Q,	FALSE	},
    {	"nofollow",		R,	FALSE	},
    {	"colour",		T,	FALSE	},
    {	"permit",		U,	TRUE	},
    {   "consent",              V,      FALSE   },
    {	"log",			W,	FALSE	},
    {	"deny",			X,	FALSE	},
    {	"freeze",		Y,	FALSE	},
    {	"thief",		Z,	FALSE	},
    {	"killer",		aa,	FALSE	},
    {   "questor",              J,      FALSE   },
    {   "gquest",               B,      FALSE   },
    {   "war",                  dd,     FALSE   },
    {   "frequent",             M,      FALSE   },
    {   "timeout",              O,      FALSE   },
    {   "autoweather",          ee,     FALSE   },
    {	NULL,			0,	0	}
};

const struct flag_type affect_flags[] =
{
    {	"blind",		A,	TRUE	},
    {	"invisible",		B,	TRUE	},
    {	"detect_evil",		C,	TRUE	},
    {	"detect_invis",		D,	TRUE	},
    {	"detect_magic",		E,	TRUE	},
    {	"detect_hidden",	F,	TRUE	},
    {	"detect_good",		G,	TRUE	},
    {	"sanctuary",		H,	TRUE	},
    {	"faerie_fire",		I,	TRUE	},
    {	"infrared",		J,	TRUE	},
    {	"curse",		K,	TRUE	},
    {	"poison",		M,	TRUE	},
    {	"protect_evil",		N,	TRUE	},
    {	"protect_good",		O,	TRUE	},
    {	"sneak",		P,	TRUE	},
    {	"hide",			Q,	TRUE	},
    {	"sleep",		R,	TRUE	},
    {	"charm",		S,	TRUE	},
    {	"flying",		T,	TRUE	},
    {	"pass_door",		U,	TRUE	},
    {	"haste",		V,	TRUE	},
    {	"calm",			W,	TRUE	},
    {	"plague",		X,	TRUE	},
    {	"weaken",		Y,	TRUE	},
    {	"dark_vision",		Z,	TRUE	},
    {	"berserk",		aa,	TRUE	},
    {	"swim",			bb,	TRUE	},
    {	"regeneration",		cc,	TRUE	},
    {	"slow",			dd,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type affect2_flags[] =     
{
    {   "steel_skin",           A,      TRUE    },
    {   "turning",              B,      TRUE    },
    {   "bloodlust",            C,      TRUE    },
    {   "divine_protection",    D,      TRUE    },
    {   "force_shield",         E,      TRUE    },
    {   "static_shield",        F,      TRUE    },
    {   "flame_shield",         G,      TRUE    },
    {   "lifeforce",            H,      TRUE    },
    {   NULL,                   0,      0       }
};

const struct flag_type off_flags[] =
{
    {	"area_attack",		A,	TRUE	},
    {	"backstab",		B,	TRUE	},
    {	"bash",			C,	TRUE	},
    {	"berserk",		D,	TRUE	},
    {	"disarm",		E,	TRUE	},
    {	"dodge",		F,	TRUE	},
    {	"fade",			G,	TRUE	},
    {	"fast",			H,	TRUE	},
    {	"kick",			I,	TRUE	},
    {	"dirt_kick",		J,	TRUE	},
    {	"parry",		K,	TRUE	},
    {	"rescue",		L,	TRUE	},
    {	"tail",			M,	TRUE	},
    {	"trip",			N,	TRUE	},
    {	"crush",		O,	TRUE	},
    {	"assist_all",		P,	TRUE	},
    {	"assist_align",		Q,	TRUE	},
    {	"assist_race",		R,	TRUE	},
    {	"assist_players",	S,	TRUE	},
    {	"assist_guard",		T,	TRUE	},
    {	"assist_vnum",		U,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type imm_flags[] =
{
    {	"summon",		A,	TRUE	},
    {	"charm",		B,	TRUE	},
    {	"magic",		C,	TRUE	},
    {	"weapon",		D,	TRUE	},
    {	"bash",			E,	TRUE	},
    {	"pierce",		F,	TRUE	},
    {	"slash",		G,	TRUE	},
    {	"fire",			H,	TRUE	},
    {	"cold",			I,	TRUE	},
    {	"lightning",		J,	TRUE	},
    {	"acid",			K,	TRUE	},
    {	"poison",		L,	TRUE	},
    {	"negative",		M,	TRUE	},
    {	"holy",			N,	TRUE	},
    {	"energy",		O,	TRUE	},
    {	"mental",		P,	TRUE	},
    {	"disease",		Q,	TRUE	},
    {	"drowning",		R,	TRUE	},
    {	"light",		S,	TRUE	},
    {	"sound",		T,	TRUE	},
    {	"wood",			X,	TRUE	},
    {	"silver",		Y,	TRUE	},
    {	"iron",			Z,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type form_flags[] =
{
    {	"edible",		FORM_EDIBLE,		TRUE	},
    {	"poison",		FORM_POISON,		TRUE	},
    {	"magical",		FORM_MAGICAL,		TRUE	},
    {	"instant_decay",	FORM_INSTANT_DECAY,	TRUE	},
    {	"other",		FORM_OTHER,		TRUE	},
    {	"animal",		FORM_ANIMAL,		TRUE	},
    {	"sentient",		FORM_SENTIENT,		TRUE	},
    {	"undead",		FORM_UNDEAD,		TRUE	},
    {	"construct",		FORM_CONSTRUCT,		TRUE	},
    {	"mist",			FORM_MIST,		TRUE	},
    {	"intangible",		FORM_INTANGIBLE,	TRUE	},
    {	"biped",		FORM_BIPED,		TRUE	},
    {	"centaur",		FORM_CENTAUR,		TRUE	},
    {	"insect",		FORM_INSECT,		TRUE	},
    {	"spider",		FORM_SPIDER,		TRUE	},
    {	"crustacean",		FORM_CRUSTACEAN,	TRUE	},
    {	"worm",			FORM_WORM,		TRUE	},
    {	"blob",			FORM_BLOB,		TRUE	},
    {	"mammal",		FORM_MAMMAL,		TRUE	},
    {	"bird",			FORM_BIRD,		TRUE	},
    {	"reptile",		FORM_REPTILE,		TRUE	},
    {	"snake",		FORM_SNAKE,		TRUE	},
    {	"dragon",		FORM_DRAGON,		TRUE	},
    {	"amphibian",		FORM_AMPHIBIAN,		TRUE	},
    {	"fish",			FORM_FISH ,		TRUE	},
    {	"cold_blood",		FORM_COLD_BLOOD,	TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type part_flags[] =
{
    {	"head",			PART_HEAD,		TRUE	},
    {	"arms",			PART_ARMS,		TRUE	},
    {	"legs",			PART_LEGS,		TRUE	},
    {	"heart",		PART_HEART,		TRUE	},
    {	"brains",		PART_BRAINS,		TRUE	},
    {	"guts",			PART_GUTS,		TRUE	},
    {	"hands",		PART_HANDS,		TRUE	},
    {	"feet",			PART_FEET,		TRUE	},
    {	"fingers",		PART_FINGERS,		TRUE	},
    {	"ear",			PART_EAR,		TRUE	},
    {	"eye",			PART_EYE,		TRUE	},
    {	"long_tongue",		PART_LONG_TONGUE,	TRUE	},
    {	"eyestalks",		PART_EYESTALKS,		TRUE	},
    {	"tentacles",		PART_TENTACLES,		TRUE	},
    {	"fins",			PART_FINS,		TRUE	},
    {	"wings",		PART_WINGS,		TRUE	},
    {	"tail",			PART_TAIL,		TRUE	},
    {	"claws",		PART_CLAWS,		TRUE	},
    {	"fangs",		PART_FANGS,		TRUE	},
    {	"horns",		PART_HORNS,		TRUE	},
    {	"scales",		PART_SCALES,		TRUE	},
    {	"tusks",		PART_TUSKS,		TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type comm_flags[] =
{
    {	"quiet",		COMM_QUIET,		TRUE	},
    {   "deaf",			COMM_DEAF,		TRUE	},
    {   "nowiz",		COMM_NOWIZ,		TRUE	},
    {   "noclangossip",		COMM_NOAUCTION,		TRUE	},
    {   "nogossip",		COMM_NOGOSSIP,		TRUE	},
    {   "noquestion",		COMM_NOQUESTION,	TRUE	},
    {   "nomusic",		COMM_NOMUSIC,		TRUE	},
    {   "noclan",		COMM_NOCLAN,		TRUE	},
    {   "noquote",		COMM_NOQUOTE,		TRUE	},
    {   "shoutsoff",		COMM_SHOUTSOFF,		TRUE	},
    {   "compact",		COMM_COMPACT,		TRUE	},
    {   "brief",		COMM_BRIEF,		TRUE	},
    {   "prompt",		COMM_PROMPT,		TRUE	},
    {   "combine",		COMM_COMBINE,		TRUE	},
    {   "telnet_ga",		COMM_TELNET_GA,		TRUE	},
    {   "show_affects",		COMM_SHOW_AFFECTS,	TRUE	},
    {   "nograts",		COMM_NOGRATS,		TRUE	},
    {   "noemote",		COMM_NOEMOTE,		FALSE	},
    {   "noshout",		COMM_NOSHOUT,		FALSE	},
    {   "notell",		COMM_NOTELL,		FALSE	},
    {   "nochannels",		COMM_NOCHANNELS,	FALSE	},
    {   "snoop_proof",		COMM_SNOOP_PROOF,	FALSE	},
    {   "afk",			COMM_AFK,		TRUE	},
    {   "nopray",               COMM_NOPRAY,            TRUE    },
    {   "noinfo",               COMM_NOINFO,            TRUE    },
    {	NULL,			0,			0	}
};

const struct flag_type comm2_flags[] =
{ 
/*  {   "test",                 COMM_TEST,              TRUE    } */
    {   "imp",                  COMM_NOIMP,             TRUE    },
    {   "show_armor",           COMM_SHOW_ARMOR,        TRUE    },
    {   "newbie",               COMM_NONEWBIE,          TRUE    },
    {   "auto_afk",             COMM_AUTO_AFK,          TRUE    },
    {   "announce",             COMM_ANNOUNCE,          TRUE    },
    {   "autolevel",            COMM_AUTOLEVEL,         TRUE    },
    {   "petlevel",             COMM_PETLEVEL,          TRUE    },
    {   "noadmin",              COMM_NOADMIN,           TRUE    },
    {   "noquest",              COMM_NOQUEST,           TRUE    },
    {   NULL,                   0,                      0       }
};                                                                   

const	struct	hometown_type	hometown_table [] =
{
/*  {	"name",		recall,	school,	morgue,	death, donation, postal	},	*/
    {	"Midgaard",	3001,	3700,	3000,	3054,  3037,	 3037	},
    {   "Bridgewater",  5600,	3700,   5619,   5752,  5737,     5738   },
    {	NULL,		0,	0,	0,	0,     0,	 0      }
};


const struct flag_type mprog_flags[] =
{
    {	"act",			TRIG_ACT,		TRUE	},
    {	"bribe",		TRIG_BRIBE,		TRUE 	},
    {	"death",		TRIG_DEATH,		TRUE    },
    {	"entry",		TRIG_ENTRY,		TRUE	},
    {	"fight",		TRIG_FIGHT,		TRUE	},
    {	"give",			TRIG_GIVE,		TRUE	},
    {	"greet",		TRIG_GREET,		TRUE    },
    {	"grall",		TRIG_GRALL,		TRUE	},
    {	"kill",			TRIG_KILL,		TRUE	},
    {	"hpcnt",		TRIG_HPCNT,		TRUE    },
    {	"random",		TRIG_RANDOM,		TRUE	},
    {	"speech",		TRIG_SPEECH,		TRUE	},
    {	"exit",			TRIG_EXIT,		TRUE    },
    {	"exall",		TRIG_EXALL,		TRUE    },
    {	"delay",		TRIG_DELAY,		TRUE    },
    {	"surr",			TRIG_SURR,		TRUE    },
    {	NULL,			0,			TRUE	}
};

const struct flag_type area_flags[] =
{
    {	"none",			AREA_NONE,		FALSE	},
    {	"changed",		AREA_CHANGED,		TRUE	},
    {	"added",		AREA_ADDED,		TRUE	},
    {	"loading",		AREA_LOADING,		FALSE	},
    {   "complete",		AREA_COMPLETE,		TRUE    },
    {   "unique",               NO_UNIQUE,              TRUE    },
    {	NULL,			0,			0	}
};


const struct flag_type sex_flags[] =
{
    {	"male",			SEX_MALE,		TRUE	},
    {	"female",		SEX_FEMALE,		TRUE	},
    {	"neutral",		SEX_NEUTRAL,		TRUE	},
    {   "random",               3,                      TRUE    },   /* ROM */
    {	"none",			SEX_NEUTRAL,		TRUE	},
    {	NULL,			0,			0	}
};


const struct flag_type exit_flags[] =
{
    {   "door",			EX_ISDOOR,		TRUE    },
    {	"closed",		EX_CLOSED,		TRUE	},
    {	"locked",		EX_LOCKED,		TRUE	},
    {	"pickproof",		EX_PICKPROOF,		TRUE	},
    {   "nopass",		EX_NOPASS,		TRUE	},
    {   "easy",			EX_EASY,		TRUE	},
    {   "hard",			EX_HARD,		TRUE	},
    {	"infuriating",		EX_INFURIATING,		TRUE	},
    {	"noclose",		EX_NOCLOSE,		TRUE	},
    {	"nolock",		EX_NOLOCK,		TRUE	},
    {   "nobash",               EX_NOBASH,              TRUE    },
    {	NULL,			0,			0	}
};



const struct flag_type door_resets[] =
{
    {	"open and unlocked",	0,		TRUE	},
    {	"closed and unlocked",	1,		TRUE	},
    {	"closed and locked",	2,		TRUE	},
    {	NULL,			0,		0	}
};



const struct flag_type room_flags[] =
{
    {	"dark",			ROOM_DARK,		TRUE	},
    {	"no_mob",		ROOM_NO_MOB,		TRUE	},
    {	"indoors",		ROOM_INDOORS,		TRUE	},
    {	"private",		ROOM_PRIVATE,		TRUE    },
    {	"safe",			ROOM_SAFE,		TRUE	},
    {	"solitary",		ROOM_SOLITARY,		TRUE	},
    {	"pet_shop",		ROOM_PET_SHOP,		TRUE	},
    {	"mount_shop",		ROOM_MOUNT_SHOP,		TRUE	},    
    {	"no_recall",		ROOM_NO_RECALL,		TRUE	},
    {	"imp_only",		ROOM_IMP_ONLY,		TRUE    },
    {	"gods_only",	        ROOM_GODS_ONLY,		TRUE    },
    {	"heroes_only",		ROOM_HEROES_ONLY,	TRUE	},
    {	"newbies_only",		ROOM_NEWBIES_ONLY,	TRUE	},
    {	"law",			ROOM_LAW,		TRUE	},
    {   "nowhere",		ROOM_NOWHERE,		TRUE	},
    {   "donation",             ROOM_DONATION,          TRUE    },
    {   "rental",               ROOM_RENTAL,            TRUE    },
    {   "casino",               ROOM_CASINO,            TRUE    },
    {   "nomagic",              ROOM_NOMAGIC,           TRUE    },
    {   "arena",                ROOM_ARENA,             TRUE    },
    {   "waypoint",             ROOM_WAYPOINT,          TRUE    },
    {   "mapped",		ROOM_MAPPED,		FALSE   },
    {   "mine",			ROOM_MINE,		TRUE    },
    {   "bank",                 ROOM_BANK,              TRUE    },
    {   "postal",               ROOM_POSTAL,            TRUE    },
    {	NULL,			0,			0	}
};



const struct flag_type sector_flags[] =
{
    {	"inside",	SECT_INSIDE,		TRUE	},
    {	"city",		SECT_CITY,		TRUE	},
    {	"field",	SECT_FIELD,		TRUE	},
    {	"forest",	SECT_FOREST,		TRUE	},
    {	"hills",	SECT_HILLS,		TRUE	},
    {	"mountain",	SECT_MOUNTAIN,		TRUE	},
    {	"swim",		SECT_WATER_SWIM,	TRUE	},
    {	"noswim",	SECT_WATER_NOSWIM,	TRUE	},
    {   "unused",	SECT_UNUSED,		TRUE	},
    {	"air",		SECT_AIR,		TRUE	},
    {	"desert",	SECT_DESERT,		TRUE	},
    {   "farmland",	SECT_FARM,		TRUE	},
    {	NULL,		0,			0	}
};



const struct flag_type type_flags[] =
{
    {	"light",		ITEM_LIGHT,		TRUE	},
    {	"scroll",		ITEM_SCROLL,		TRUE	},
    {	"wand",			ITEM_WAND,		TRUE	},
    {	"staff",		ITEM_STAFF,		TRUE	},
    {	"weapon",		ITEM_WEAPON,		TRUE	},
    {	"treasure",		ITEM_TREASURE,		TRUE	},
    {	"armor",		ITEM_ARMOR,		TRUE	},
    {	"potion",		ITEM_POTION,		TRUE	},
    {	"furniture",		ITEM_FURNITURE,		TRUE	},
    {	"trash",		ITEM_TRASH,		TRUE	},
    {	"container",		ITEM_CONTAINER,		TRUE	},
    {	"drinkcontainer",	ITEM_DRINK_CON,		TRUE	},
    {	"key",			ITEM_KEY,		TRUE	},
    {	"food",			ITEM_FOOD,		TRUE	},
    {	"money",		ITEM_MONEY,		TRUE	},
    {	"boat",			ITEM_BOAT,		TRUE	},
    {	"npccorpse",		ITEM_CORPSE_NPC,	TRUE	},
    {	"pc corpse",		ITEM_CORPSE_PC,		FALSE	},
    {	"fountain",		ITEM_FOUNTAIN,		TRUE	},
    {	"pill",			ITEM_PILL,		TRUE	},
    {	"protect",		ITEM_PROTECT,		TRUE	},
    {	"map",			ITEM_MAP,		TRUE	},
    {   "portal",		ITEM_PORTAL,		TRUE	},
    {   "warpstone",		ITEM_WARP_STONE,	TRUE	},
    {	"roomkey",		ITEM_ROOM_KEY,		TRUE	},
    { 	"gem",			ITEM_GEM,		TRUE	},
    {	"jewelry",		ITEM_JEWELRY,		TRUE	},
    {	"jukebox",		ITEM_JUKEBOX,		TRUE	},
    {   "token",                ITEM_TOKEN,             TRUE    },
    {   "source",		ITEM_SOURCE,		TRUE	},
    {   "socket",               ITEM_SOCKET,            TRUE    },
    {   "instrument",           ITEM_INSTRUMENT,        TRUE    },
    {   "quiver",               ITEM_QUIVER,            TRUE    },
    {   "arrow",                ITEM_ARROW,             TRUE    },
    {	"sheath",		ITEM_SHEATH,		TRUE	},
    {	"belt",			ITEM_BELT, 		TRUE	},
    {   "skeleton",             ITEM_SKELETON,          TRUE    },
    {   "ore",			ITEM_ORE,		TRUE    },
    {   "sockets",              ITEM_SOCKETS,           TRUE    },
    { 	NULL, 			0, 			0 	}
};


const struct flag_type extra_flags[] =
{
    {	"glow",			ITEM_GLOW,		TRUE	},
    {	"hum",			ITEM_HUM,		TRUE	},
    {	"dark",			ITEM_DARK,		TRUE	},
    {	"lock",			ITEM_LOCK,		TRUE	},
    {	"evil",			ITEM_EVIL,		TRUE	},
    {	"invis",		ITEM_INVIS,		TRUE	},
    {	"magic",		ITEM_MAGIC,		TRUE	},
    {	"nodrop",		ITEM_NODROP,		TRUE	},
    {	"bless",		ITEM_BLESS,		TRUE	},
    {	"antigood",		ITEM_ANTI_GOOD,		TRUE	},
    {	"antievil",		ITEM_ANTI_EVIL,		TRUE	},
    {	"antineutral",		ITEM_ANTI_NEUTRAL,	TRUE	},
    {	"noremove",		ITEM_NOREMOVE,		TRUE	},
    {	"inventory",		ITEM_INVENTORY,		TRUE	},
    {	"nopurge",		ITEM_NOPURGE,		TRUE	},
    {	"rotdeath",		ITEM_ROT_DEATH,		TRUE	},
    {	"visdeath",		ITEM_VIS_DEATH,		TRUE	},
    {   "nonmetal",		ITEM_NONMETAL,		TRUE	},
    {	"meltdrop",		ITEM_MELT_DROP,		TRUE	},
    {	"hadtimer",		ITEM_HAD_TIMER,		TRUE	},
    {	"sellextract",		ITEM_SELL_EXTRACT,	TRUE	},
    {	"burnproof",		ITEM_BURN_PROOF,	TRUE	},
    {   "quest",                ITEM_QUEST,             TRUE    },
    {	"nouncurse",		ITEM_NOUNCURSE,		TRUE	},
    {   "auctioned",            ITEM_AUCTIONED,         FALSE   },
    {   "no_auction",           ITEM_NO_AUCTION,        TRUE    },
    {	"lodged",		ITEM_LODGED,		TRUE	},
    {   "nocond",               ITEM_NOCOND,            TRUE    },
    {   "norepair",             ITEM_NOREPAIR,          TRUE    },
    {	NULL,			0,			0	}
};

const struct flag_type extra2_flags[] =
{  
    {   "unique",               ITEM_UNIQUE,            TRUE    },
    {   "no_animate",           CORPSE_NO_ANIMATE,      TRUE    },
    {   "sanctuary",		ITEM_SANC,		TRUE	},        
    {   "relic",                ITEM_RELIC,             TRUE    },
    {   "inlay1",               ITEM_INLAY1,            TRUE    },
    {   "inlay2",               ITEM_INLAY2,            TRUE    },
    {   NULL,                   0,                      0       }
}; 

const struct flag_type wear_flags[] =
{
    {	"take",			ITEM_TAKE,		TRUE	},
    {	"finger",		ITEM_WEAR_FINGER,	TRUE	},
    {	"neck",			ITEM_WEAR_NECK,		TRUE	},
    {	"body",			ITEM_WEAR_BODY,		TRUE	},
    {	"head",			ITEM_WEAR_HEAD,		TRUE	},
    {	"legs",			ITEM_WEAR_LEGS,		TRUE	},
    {	"feet",			ITEM_WEAR_FEET,		TRUE	},
    {	"hands",		ITEM_WEAR_HANDS,	TRUE	},
    {	"arms",			ITEM_WEAR_ARMS,		TRUE	},
    {	"shield",		ITEM_WEAR_SHIELD,	TRUE	},
    {	"about",		ITEM_WEAR_ABOUT,	TRUE	},
    {	"waist",		ITEM_WEAR_WAIST,	TRUE	},
    {	"wrist",		ITEM_WEAR_WRIST,	TRUE	},
    {	"wield",		ITEM_WIELD,		TRUE	},
    {	"hold",			ITEM_HOLD,		TRUE	},
    {   "nosac",		ITEM_NO_SAC,		TRUE	},
    {   "shoulder",             ITEM_WEAR_SHOULDER,     TRUE    },
    {	"wearfloat",		ITEM_WEAR_FLOAT,	TRUE	},
    {   "hood",			ITEM_WEAR_HOOD,		TRUE	},
    {   "cloak",		ITEM_WEAR_CLOAK,	TRUE	},
    {   "armlayer",		ITEM_LAYER_ARMS,	TRUE    },
    {   "bodylayer",		ITEM_LAYER_BODY,	TRUE	},
    {   "leglayer",		ITEM_LAYER_LEGS,	TRUE	},
    {   "wedding",              ITEM_WEAR_WEDDING,      TRUE    },
    {   "back",			ITEM_WEAR_BACK,		TRUE	},
    {	NULL,			0,			0	}
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
    {	"none",			APPLY_NONE,		TRUE	},
    {	"strength",		APPLY_STR,		TRUE	},
    {	"dexterity",		APPLY_DEX,		TRUE	},
    {	"intelligence",		APPLY_INT,		TRUE	},
    {	"wisdom",		APPLY_WIS,		TRUE	},
    {	"constitution",		APPLY_CON,		TRUE	},
    {	"sex",			APPLY_SEX,		TRUE	},
    {	"class",		APPLY_CLASS,		TRUE	},
    {	"level",		APPLY_LEVEL,		TRUE	},
    {	"age",			APPLY_AGE,		TRUE	},
    {	"height",		APPLY_HEIGHT,		TRUE	},
    {	"weight",		APPLY_WEIGHT,		TRUE	},
    {	"mana",			APPLY_MANA,		TRUE	},
    {	"hp",			APPLY_HIT,		TRUE	},
    {	"move",			APPLY_MOVE,		TRUE	},
    {	"gold",			APPLY_GOLD,		TRUE	},
    {	"experience",		APPLY_EXP,		TRUE	},
    {	"ac",			APPLY_AC,		TRUE	},
    {	"hitroll",		APPLY_HITROLL,		TRUE	},
    {	"damroll",		APPLY_DAMROLL,		TRUE	},
    {	"saves",		APPLY_SAVES,		TRUE	},
    {	"savingpara",		APPLY_SAVING_PARA,	TRUE	},
    {	"savingrod",		APPLY_SAVING_ROD,	TRUE	},
    {	"savingpetri",		APPLY_SAVING_PETRI,	TRUE	},
    {	"savingbreath",		APPLY_SAVING_BREATH,	TRUE	},
    {	"savingspell",		APPLY_SAVING_SPELL,	TRUE	},
    {   "clevel",               APPLY_CAST_LEVEL,       TRUE    },
    {   "cability",             APPLY_CAST_ABILITY,     TRUE    },
    {	"spellaffect",		APPLY_SPELL_AFFECT,	FALSE	},
    {	"quiver",		ITEM_QUIVER,		TRUE	},
    {	"arrow",		ITEM_ARROW,		TRUE	},
    {	NULL,			0,			0	}
};


/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
    {	"in the inventory",	WEAR_NONE,	TRUE	},
    {	"as a light",		WEAR_LIGHT,	TRUE	},
    {	"on the left finger",	WEAR_FINGER_L,	TRUE	},
    {	"on the right finger",	WEAR_FINGER_R,	TRUE	},
    {	"around the neck (1)",	WEAR_NECK_1,	TRUE	},
    {	"around the neck (2)",	WEAR_NECK_2,	TRUE	},
    {	"on the body",		WEAR_BODY,	TRUE	},
    {	"over the head",	WEAR_HEAD,	TRUE	},
    {	"on the legs",		WEAR_LEGS,	TRUE	}, 
    {	"on the feet",		WEAR_FEET,	TRUE	},
    {	"on the hands",		WEAR_HANDS,	TRUE	},
    {	"on the arms",		WEAR_ARMS,	TRUE	},
    {	"as a shield",		WEAR_SHIELD,	TRUE	},
    {	"about the shoulders",	WEAR_ABOUT,	TRUE	},
    {	"around the waist",	WEAR_WAIST,	TRUE	},
    {	"on the left wrist",	WEAR_WRIST_L,	TRUE	},
    {	"on the right wrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",		WEAR_WIELD,	TRUE	},
    {	"held in the hands",	WEAR_HOLD,	TRUE	},
    {	"floating nearby",	WEAR_FLOAT,	TRUE	},
    {   "on your shoulder",     WEAR_SHOULDER,  TRUE    },
    {   "worn as hood",         WEAR_HOOD,      TRUE    },
    {   "worn as cloak",        WEAR_CLOAK,     TRUE    },
    {   "worn around arms",     WEAR_LAYER_ARMS,TRUE    },
    {   "worn around body",     WEAR_LAYER_BODY,TRUE    },
    {   "worn around legs",     WEAR_LAYER_LEGS,TRUE    },
    {   "worn on back",         WEAR_BACK,      TRUE    },
    {	NULL,			0	      , 0	}
};


const struct flag_type wear_loc_flags[] =
{
    {	"none",		WEAR_NONE,	TRUE	},
    {	"light",	WEAR_LIGHT,	TRUE	},
    {   "hood",         WEAR_HOOD,      TRUE    },
    {	"lfinger",	WEAR_FINGER_L,	TRUE	},
    {	"rfinger",	WEAR_FINGER_R,	TRUE	},
    {	"neck1",	WEAR_NECK_1,	TRUE	},
    {	"neck2",	WEAR_NECK_2,	TRUE	},
    {   "laybody",      WEAR_LAYER_BODY,TRUE    },
    {	"body",		WEAR_BODY,	TRUE	},
    {	"head",		WEAR_HEAD,	TRUE	},
    {   "cloak",        WEAR_CLOAK,     TRUE    },
    {   "llegs",        WEAR_LAYER_LEGS,TRUE    },
    {	"legs",		WEAR_LEGS,	TRUE	},
    {	"feet",		WEAR_FEET,	TRUE	},
    {	"hands",	WEAR_HANDS,	TRUE	},
    {   "laarms",       WEAR_LAYER_ARMS,TRUE    },
    {	"arms",		WEAR_ARMS,	TRUE	},
    {	"shield",	WEAR_SHIELD,	TRUE	},
    {	"about",	WEAR_ABOUT,	TRUE	},
    {   "back",         WEAR_BACK,      TRUE    },
    {	"waist",	WEAR_WAIST,	TRUE	},
    {	"lwrist",	WEAR_WRIST_L,	TRUE	},
    {	"rwrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",	WEAR_WIELD,	TRUE	},
    {	"hold",		WEAR_HOLD,	TRUE	},
    {	"floating",	WEAR_FLOAT,	TRUE	},
    {   "shoulder",     WEAR_SHOULDER,  TRUE    },
    {	"lodge_leg",	WEAR_LODGE_LEG,	TRUE	},
    {	"lodge_arm",	WEAR_LODGE_ARM,	TRUE	},
    {	"lodge_rib",	WEAR_LODGE_RIB,	TRUE	},
    {	NULL,		0,		0	}
};

const struct flag_type container_flags[] =
{
    {	"closeable",		1,		TRUE	},
    {	"pickproof",		2,		TRUE	},
    {	"closed",		4,		TRUE	},
    {	"locked",		8,		TRUE	},
    {	"puton",		16,		TRUE	},
    {	NULL,			0,		0	}
};


/*****************************************************************************
                      ROM - specific tables:
 
****************************************************************************/




const struct flag_type ac_type[] =
{
    {   "pierce",        AC_PIERCE,            TRUE    },
    {   "bash",          AC_BASH,              TRUE    },
    {   "slash",         AC_SLASH,             TRUE    },
    {   "exotic",        AC_EXOTIC,            TRUE    },
    {   NULL,              0,                    0       }
};


const struct flag_type size_flags[] =
{
    {   "tiny",          SIZE_TINY,            TRUE    },
    {   "small",         SIZE_SMALL,           TRUE    },
    {   "medium",        SIZE_MEDIUM,          TRUE    },
    {   "large",         SIZE_LARGE,           TRUE    },
    {   "huge",          SIZE_HUGE,            TRUE    },
    {   "giant",         SIZE_GIANT,           TRUE    },
    {   NULL,              0,                    0       },
};


const struct flag_type weapon_class[] =
{
    {   "exotic",	WEAPON_EXOTIC,		TRUE    },
    {   "sword",	WEAPON_SWORD,		TRUE    },
    {   "dagger",	WEAPON_DAGGER,		TRUE    },
    {   "spear",	WEAPON_SPEAR,		TRUE    },
    {   "mace",		WEAPON_MACE,		TRUE    },
    {   "axe",		WEAPON_AXE,		TRUE    },
    {   "flail",	WEAPON_FLAIL,		TRUE    },
    {   "whip",		WEAPON_WHIP,		TRUE    },
    {   "polearm",	WEAPON_POLEARM,		TRUE    },
    {   "bow",          WEAPON_BOW,             TRUE    },
    {   NULL,		0,			0       }
};


const struct flag_type weapon_type2[] =
{
    {   "flaming",       WEAPON_FLAMING,       TRUE    },
    {   "frost",         WEAPON_FROST,         TRUE    },
    {   "vampiric",      WEAPON_VAMPIRIC,      TRUE    },
    {   "sharp",         WEAPON_SHARP,         TRUE    },
    {   "vorpal",        WEAPON_VORPAL,        TRUE    },
    {   "twohands",      WEAPON_TWO_HANDS,     TRUE    },
    {	"shocking",	 WEAPON_SHOCKING,      TRUE    },
    {	"poison",	 WEAPON_POISON,	       TRUE    },
    {   NULL,            0,                    0       }
};

const struct flag_type res_flags[] =
{
    {	"summon",	 RES_SUMMON,	       TRUE    },
    {   "charm",         RES_CHARM,            TRUE    },
    {   "magic",         RES_MAGIC,            TRUE    },
    {   "weapon",        RES_WEAPON,           TRUE    },
    {   "bash",          RES_BASH,             TRUE    },
    {   "pierce",        RES_PIERCE,           TRUE    },
    {   "slash",         RES_SLASH,            TRUE    },
    {   "fire",          RES_FIRE,             TRUE    },
    {   "cold",          RES_COLD,             TRUE    },
    {   "lightning",     RES_LIGHTNING,        TRUE    },
    {   "acid",          RES_ACID,             TRUE    },
    {   "poison",        RES_POISON,           TRUE    },
    {   "negative",      RES_NEGATIVE,         TRUE    },
    {   "holy",          RES_HOLY,             TRUE    },
    {   "energy",        RES_ENERGY,           TRUE    },
    {   "mental",        RES_MENTAL,           TRUE    },
    {   "disease",       RES_DISEASE,          TRUE    },
    {   "drowning",      RES_DROWNING,         TRUE    },
    {   "light",         RES_LIGHT,            TRUE    },
    {	"sound",	 RES_SOUND,	       TRUE    },
    {	"wood",		 RES_WOOD,	       TRUE    },
    {	"silver",	 RES_SILVER,	       TRUE    },
    {	"iron",		 RES_IRON,	       TRUE    },
    {   NULL,            0,                    0       }
};


const struct flag_type vuln_flags[] =
{
    {	"summon",	 VULN_SUMMON,          TRUE    },
    {	"charm",	 VULN_CHARM,	       TRUE    },
    {   "magic",         VULN_MAGIC,           TRUE    },
    {   "weapon",        VULN_WEAPON,          TRUE    },
    {   "bash",          VULN_BASH,            TRUE    },
    {   "pierce",        VULN_PIERCE,          TRUE    },
    {   "slash",         VULN_SLASH,           TRUE    },
    {   "fire",          VULN_FIRE,            TRUE    },
    {   "cold",          VULN_COLD,            TRUE    },
    {   "lightning",     VULN_LIGHTNING,       TRUE    },
    {   "acid",          VULN_ACID,            TRUE    },
    {   "poison",        VULN_POISON,          TRUE    },
    {   "negative",      VULN_NEGATIVE,        TRUE    },
    {   "holy",          VULN_HOLY,            TRUE    },
    {   "energy",        VULN_ENERGY,          TRUE    },
    {   "mental",        VULN_MENTAL,          TRUE    },
    {   "disease",       VULN_DISEASE,         TRUE    },
    {   "drowning",      VULN_DROWNING,        TRUE    },
    {   "light",         VULN_LIGHT,           TRUE    },
    {	"sound",	 VULN_SOUND,	       TRUE    },
    {   "wood",          VULN_WOOD,            TRUE    },
    {   "silver",        VULN_SILVER,          TRUE    },
    {   "iron",          VULN_IRON,            TRUE    },
    {   "steel",         VULN_STEEL,           TRUE    },
    {   "mithril",       VULN_MITHRIL,         TRUE    },
    {   "adamantite",    VULN_ADAMANTITE,      TRUE    },
    {   NULL,              0,                    0     }
};

const struct flag_type position_flags[] =
{
    {   "dead",           POS_DEAD,            FALSE   },
    {   "mortal",         POS_MORTAL,          FALSE   },
    {   "incap",          POS_INCAP,           FALSE   },
    {   "stunned",        POS_STUNNED,         FALSE   },
    {   "sleeping",       POS_SLEEPING,        TRUE    },
    {   "resting",        POS_RESTING,         TRUE    },
    {   "sitting",        POS_SITTING,         TRUE    },
    {   "fighting",       POS_FIGHTING,        FALSE   },
    {   "standing",       POS_STANDING,        TRUE    },
    {   NULL,              0,                    0     }
};

const struct flag_type portal_flags[]=
{
    {   "normal_exit",	  GATE_NORMAL_EXIT,	TRUE	},
    {	"no_curse",	  GATE_NOCURSE,		TRUE	},
    {   "go_with",	  GATE_GOWITH,		TRUE	},
    {   "buggy",	  GATE_BUGGY,		TRUE	},
    {	"random",	  GATE_RANDOM,		TRUE	},
    {   NULL,		  0,			0	}
};

const struct flag_type furniture_flags[]=
{
    {   "stand_at",	  STAND_AT,		TRUE	},
    {	"stand_on",	  STAND_ON,		TRUE	},
    {	"stand_in",	  STAND_IN,		TRUE	},
    {	"sit_at",	  SIT_AT,		TRUE	},
    {	"sit_on",	  SIT_ON,		TRUE	},
    {	"sit_in",	  SIT_IN,		TRUE	},
    {	"rest_at",	  REST_AT,		TRUE	},
    {	"rest_on",	  REST_ON,		TRUE	},
    {	"rest_in",	  REST_IN,		TRUE	},
    {	"sleep_at",	  SLEEP_AT,		TRUE	},
    {	"sleep_on",	  SLEEP_ON,		TRUE	},
    {	"sleep_in",	  SLEEP_IN,		TRUE	},
    {	"put_at",	  PUT_AT,		TRUE	},
    {	"put_on",	  PUT_ON,		TRUE	},
    {	"put_in",	  PUT_IN,		TRUE	},
    {	"put_inside",	  PUT_INSIDE,		TRUE	},
    {	NULL,		  0,			0	}
};

const	struct	flag_type	apply_types	[]	=
{
	{	"affects",	TO_AFFECTS,	TRUE	},
	{	"object",	TO_OBJECT,	TRUE	},
	{	"immune",	TO_IMMUNE,	TRUE	},
	{	"resist",	TO_RESIST,	TRUE	},
	{	"vuln",		TO_VULN,	TRUE	},
	{	"weapon",	TO_WEAPON,	TRUE	},
        {       "affects2",     TO_AFFECTS2,    TRUE    },
        {       "object2",      TO_OBJECT2,     TRUE    },
	{	NULL,		0,		TRUE	}
};

const	struct	bit_type	bitvector_type	[]	=
{
	{	affect_flags,	"affect"	},
        {       affect2_flags,  "affect2"       },
	{	apply_flags,	"apply"		},
	{	imm_flags,	"imm"		},
	{	res_flags,	"res"		},
	{	vuln_flags,	"vuln"		},
	{	weapon_type2,	"weapon"	}
};

const struct flag_type material_type[] =   
{
    {   "none",         0,              TRUE    },
    {   "steel",        MAT_STEEL,      TRUE    },
    {   "stone",        MAT_STONE,      TRUE    },
    {   "brass",        MAT_BRASS,      TRUE    },
    {   "bone",         MAT_BONE,       TRUE    },
    {   "energy",       MAT_ENERGY,     TRUE    },
    {   "mithril",      MAT_MITHRIL,    TRUE    },
    {   "copper",       MAT_COPPER,     TRUE    },        
    {   "silk",         MAT_SILK,       TRUE    },
    {   "marble",       MAT_MARBLE,     TRUE    },
    {   "glass",        MAT_GLASS,      TRUE    },
    {   "water",        MAT_WATER,      TRUE    },
    {   "flesh",        MAT_FLESH,      TRUE    },
    {   "platinum",     MAT_PLATINUM,   TRUE    },
    {   "granite",      MAT_GRANITE,    TRUE    },
    {   "leather",      MAT_LEATHER,    TRUE    },
    {   "cloth",        MAT_CLOTH,      TRUE    },
    {   "gemstone",     MAT_GEMSTONE,   TRUE    },    
    {   "gold",         MAT_GOLD,       TRUE    },
    {   "porcelain",    MAT_PORCELAIN,  TRUE    },
    {   "obsidian",     MAT_OBSIDIAN,   TRUE    },
    {   "dragonscal",   MAT_DRAGONSCALE,TRUE    },
    {   "ebony",        MAT_EBONY,      TRUE    },
    {   "bronze",       MAT_BRONZE,     TRUE    },
    {   "wood",         MAT_WOOD,       TRUE    },
    {   "silver",       MAT_SILVER,     TRUE    },
    {   "iron",         MAT_IRON,       TRUE    },
    {   "bloodstone",   MAT_BLOODSTONE, TRUE    }, 
    {   "food",         MAT_FOOD,       TRUE    },
    {   "lead",         MAT_LEAD,       TRUE    },
    {   "wax",          MAT_WAX,        TRUE    },
    {   "diamond",      MAT_DIAMOND,    TRUE    },
    {   "crystal",      MAT_CRYSTAL,    TRUE    },
    {   "vellum",       MAT_VELLUM,     TRUE    },
    {   "paper",        MAT_PAPER,      TRUE    },
    {   "meat",         MAT_MEAT,       TRUE    },
    {   "pill",         MAT_PILL,       TRUE    },
    {   "liquid",       MAT_LIQUID,     TRUE    },
    {   "adamantite",   MAT_ADAMANTITE, TRUE    },
    {   "unique",       MAT_UNIQUE,     TRUE    },
    {   "unknown",      MAT_UNKNOWN,    TRUE    },
    {   NULL,            0,             0       }
};                                     


const struct flag_type oprog_flags[] =
{
    {	"act",			TRIG_ACT,		TRUE	},
    {	"fight",		TRIG_FIGHT,		TRUE	},
    {	"give",			TRIG_GIVE,		TRUE	},
    {   "greet",		TRIG_GRALL,		TRUE	},
    {	"random",		TRIG_RANDOM,		TRUE	},
    {   "speech",		TRIG_SPEECH,		TRUE	},
    {	"exall",		TRIG_EXALL,		TRUE	},
    {	"delay",		TRIG_DELAY,		TRUE	},
    {	"drop",		TRIG_DROP,		TRUE	},
    {	"get",		TRIG_GET,		TRUE	},
    {	"sit",		TRIG_SIT,		TRUE	},
    {   "alias",		TRIG_ALIAS,		TRUE    },
    {	NULL,			0,			TRUE	},
};

const struct flag_type rprog_flags[] =
{
    {	"act",			TRIG_ACT,		TRUE	},
    {	"fight",		TRIG_FIGHT,		TRUE	},
    {	"drop",			TRIG_DROP,		TRUE	},
    {	"greet",		TRIG_GRALL,		TRUE	},
    {	"random",		TRIG_RANDOM,		TRUE	},
    {	"speech",		TRIG_SPEECH,		TRUE	},
    {	"exall",		TRIG_EXALL,		TRUE	},
    {	"delay",		TRIG_DELAY,		TRUE	},
    {   "alias",		TRIG_ALIAS,		TRUE    },
    {	NULL,			0,			TRUE	},
};

const char *osize_table []   =
{
        "tiny",
        "small",
        "medium",
        "large",
        "huge",
        "giant"
}; 

const char *cond_table []   =
{
    "Perfect",
    "Excellent",
    "Slightly damaged",
    "Moderately damaged",
    "Heavily damaged",
    "Badly damaged",
    "Barely usable",
    "Worthless"
};   

const struct material_type material_table [] =
{
/*	{       material name  	        ,material_type,		vuln_flag    } */
	{       "steel",		MAT_STEEL,	        VULN_STEEL   },
	{	"stone",		MAT_STONE,		0	     },
	{	"brass",		MAT_BRASS,		0	     },
	{  	"bone",			MAT_BONE,		0	     },
        {       "energy",               MAT_ENERGY,             VULN_ENERGY  },
	{	"mithril",		MAT_MITHRIL,            VULN_MITHRIL },
        {       "copper",               MAT_COPPER,             0            },
        {       "silk",                 MAT_SILK,               0            },
        {       "marble",               MAT_MARBLE,             0            },
	{	"glass",		MAT_GLASS,		0	     },
        {       "water",                MAT_WATER,              0            },
	{  	"flesh",		MAT_FLESH,		0	     },
        {       "platinum",             MAT_PLATINUM,           0            },
        {       "granite",              MAT_GRANITE,            0            },
	{	"leather",		MAT_LEATHER,	        0	     },
	{	"cloth",		MAT_CLOTH,		0	     },
        {       "gemstone",             MAT_GEMSTONE,           0            },
	{	"gold",			MAT_GOLD,		0	     },
        {       "porcelain",            MAT_PORCELAIN,          0            },
        {       "obsidian",             MAT_OBSIDIAN,           0            },
        {       "dragonscal",           MAT_DRAGONSCALE,        0            },
        {       "ebony",                MAT_EBONY,              0            },
	{	"bronze",		MAT_BRONZE,	0	             },
	{	"wood",	 		MAT_WOOD, 		VULN_WOOD    },
	{	"silver",		MAT_SILVER,	        VULN_SILVER  },
	{  	"iron",			MAT_IRON, 		VULN_IRON    },
        {       "bloodstone",           MAT_BLOODSTONE,         0            },
	{	"food",			MAT_FOOD,		0	     },
        {       "lead",                 MAT_LEAD,               0            },
        {       "wax",                  MAT_WAX,                0            },
	{	"diamond",		MAT_DIAMOND,	        0	     },
	{	"crystal",		MAT_CRYSTAL,	        0	     },
	{	"vellum",		MAT_VELLUM,	        0	     },
	{	"paper",		MAT_PAPER,		0	     },
	{	"meat",			MAT_MEAT,		0	     },
	{	"adamantite",	        MAT_ADAMANTITE,      VULN_ADAMANTITE },
	{	"pill",			MAT_PILL,		0	     },
	{	"liquid",		MAT_LIQUID,	        0	     },
	{	"unique",		MAT_UNIQUE,	        0	     },
	{	"unknown",		0,			0	     }
};
 
const	struct	chan_type chan_table[] =
{
	{	"{w[{WOOC{w]",    	"W",	"W",	COMM_NOOOC},
	{	"{D[{yGossip{D]",	"D",	"C",	COMM_NOGOSSIP},
	{	"{w[Quote]",		"w",	"w",	COMM_NOQUOTE},
	{	"{C[{cMusic{C]",	"c",	"C",	COMM_NOMUSIC},
	{	"{g[{GQA{g]",		"g",	"g",    COMM_NOQUESTION},
	{	"{r[{RAuction{r]",	"R",	"R",	COMM_NOAUCTION},
	{	"{M[{mGrats{M]",	"M",	"m",	COMM_NOGRATS},
	{	"{W[{DImmChat{W]",	"W",	"W",	COMM_NOWIZ},
	{	"{y[{YClan{y]",		"y",	"y",	COMM_NOCLAN},
	{	"{B[{yIMP{B]",		"B",	"c",	COMM_NOIMP},
        {       "{B[{GHERO{B]",		"B",    "c",	COMM_NOHERO},
        {       "{B[{RNewbie{B]",       "B",    "c",    COMM_NONEWBIE},
        {       "{r[{DADMIN{r]",        "w",    "D",    COMM_NOADMIN},
        {       "{g[{GGodChat{g]",		"g",	"y",    COMM_NOWIZ },
	{       "{D[{YCommChat{D]",	"w",    "D",	COMM_NOOOC },
	{	NULL,			" ",	" ",	0            }
};

const struct	flag_type strap_table  	[] 	=
{
    { "none",	STRAP_NONE,	   TRUE },
    { "thigh",	STRAP_THIGH, 	   TRUE },
    { "ankle",	STRAP_ANKLE,	   TRUE },
    { "shoulder", STRAP_SHOULDER,  TRUE },
    { "rwrist",	STRAP_WRIST_R,	   TRUE },
    { "lwrist",	STRAP_WRIST_L,	   TRUE },
    {	NULL,	0,		   0    }
}; 

const struct flag_type guild_flags[] =
{
   {    "Independent",		GUILD_INDEPENDENT,	TRUE 	},
   {	"Modified",		GUILD_CHANGED,		FALSE	},
   {	"Delete",		GUILD_DELETED,		TRUE	},
   {    "Immortal",		GUILD_IMMORTAL,		TRUE	},
   {	NULL,			0,			FALSE   }
};
const struct flag_type auctn_table[] =
{
    {  "once", 0, 0 },
    {  "twice", 0, 0 },
    {  "three times", 0, 0 }
};


const char *admin_table[] =
{
        "Synon",
        "Davion",
        "Kera",
        ""
};
      
const struct vnum_type vnum_table[] = 
{
	{	&MOB_VNUM_FIDO,				VNUM_MOB},
	{	&MOB_VNUM_CITYGUARD,			VNUM_MOB},
	{	&MOB_VNUM_REGISTAR,			VNUM_MOB},
	{	&MOB_VNUM_WARMASTER,			VNUM_MOB},
	{	&MOB_VNUM_GGOLEM,			VNUM_MOB},
	{	&MOB_VNUM_STONE_GOLEM,		VNUM_MOB},
	{	&MOB_VNUM_IRON_GOLEM,		VNUM_MOB},
	{	&MOB_VNUM_ADAMANTITE_GOLEM,	VNUM_MOB},
	{	&MOB_VNUM_L_GOLEM,			VNUM_MOB},
	{	&MOB_VNUM_WOLF,				VNUM_MOB},
	{	&MOB_VNUM_ZOMBIE,			VNUM_MOB},
	{	&MOB_VNUM_SLAYER,			VNUM_MOB},
	{	&MOB_VNUM_MUMMY,				VNUM_MOB},
	{	&MOB_VNUM_SKELETON,			VNUM_MOB},
	{	&MOB_VNUM_RENTER,			VNUM_MOB},
	{	&MOB_VNUM_FAMILIAR,			VNUM_MOB},
//        {       &MOB_VNUM_ANIMATE,                       VNUM_MOB},
     	{	&ROOM_VNUM_LIMBO,			VNUM_ROOM },
	{	&ROOM_VNUM_CHAT,				VNUM_ROOM },
	{	&ROOM_VNUM_TEMPLE,			VNUM_ROOM },
	{	&ROOM_VNUM_ALTAR,			VNUM_ROOM },
	{	&ROOM_VNUM_SCHOOL,			VNUM_ROOM },
	{	&ROOM_VNUM_BALANCE,			VNUM_ROOM },
	{	&ROOM_VNUM_CIRCLE,			VNUM_ROOM },
	{	&ROOM_VNUM_DEMISE,			VNUM_ROOM },
	{	&ROOM_VNUM_HONOR,			VNUM_ROOM },
	{	&ROOM_VNUM_MORGUE,			VNUM_ROOM },
	{	&ROOM_VNUM_DONATION,			VNUM_ROOM },
	{	&ROOM_VNUM_BANK,				VNUM_ROOM },
	{	&ROOM_VNUM_WAITROOM,			VNUM_ROOM },
	{	&ROOM_VNUM_RENTBOX,			VNUM_ROOM },
	{	&ROOM_VNUM_RENTER,			VNUM_ROOM },
        {       &ROOM_VNUM_TIMEOUT,                     VNUM_ROOM },
	{	&OBJ_FRUIT_DUMMY,			VNUM_OBJ },
	{	&OBJ_VNUM_SBLADE,			VNUM_OBJ },
	{	&OBJ_VNUM_SILVER_ONE,		VNUM_OBJ },
	{	&OBJ_VNUM_GOLD_ONE,			VNUM_OBJ },
	{	&OBJ_VNUM_GOLD_SOME,			VNUM_OBJ },
	{	&OBJ_VNUM_SILVER_SOME,		VNUM_OBJ },
	{	&OBJ_VNUM_COINS,				VNUM_OBJ },
	{	&OBJ_VNUM_CORPSE_NPC,		VNUM_OBJ },
	{	&OBJ_VNUM_CORPSE_PC,			VNUM_OBJ },
	{	&OBJ_VNUM_SEVERED_HEAD,		VNUM_OBJ },
	{	&OBJ_VNUM_TORN_HEART,		VNUM_OBJ },
	{	&OBJ_VNUM_SLICED_ARM,		VNUM_OBJ },
	{	&OBJ_VNUM_SLICED_LEG,		VNUM_OBJ },
	{	&OBJ_VNUM_GUTS,				VNUM_OBJ },
	{	&OBJ_VNUM_BRAINS,			VNUM_OBJ },
	{	&OBJ_VNUM_MUSHROOM,			VNUM_OBJ },
	{	&OBJ_VNUM_LIGHT_BALL,		VNUM_OBJ },
	{	&OBJ_VNUM_SPRING,			VNUM_OBJ },
	{	&OBJ_VNUM_DISC,				VNUM_OBJ },
	{	&OBJ_VNUM_PORTAL,			VNUM_OBJ },
	{	&OBJ_VNUM_ROSE,				VNUM_OBJ },
	{	&OBJ_VNUM_SCHOOL_MACE,		VNUM_OBJ },
	{	&OBJ_VNUM_SCHOOL_DAGGER,		VNUM_OBJ },
	{	&OBJ_VNUM_SCHOOL_SWORD,		VNUM_OBJ },
	{	&OBJ_VNUM_SCHOOL_SPEAR,		VNUM_OBJ },
	{	&OBJ_VNUM_SCHOOL_AXE,		VNUM_OBJ },
	{	&OBJ_VNUM_SCHOOL_FLAIL,		VNUM_OBJ },
	{	&OBJ_VNUM_SCHOOL_WHIP,		VNUM_OBJ },
	{	&OBJ_VNUM_SCHOOL_POLEARM,	VNUM_OBJ },
	{	&OBJ_VNUM_SCHOOL_SHIELD,		VNUM_OBJ },
	{	&OBJ_VNUM_BOW,				VNUM_OBJ },
	{	&OBJ_VNUM_WHISTLE,			VNUM_OBJ },
	{	&OBJ_VNUM_TOKEN,				VNUM_OBJ },
	{	&OBJ_VNUM_NORMAL,			VNUM_OBJ },
	{	&OBJ_VNUM_EXPERIENCE,		VNUM_OBJ },
	{	&OBJ_VNUM_PRACTICE,			VNUM_OBJ },
	{	&OBJ_VNUM_QUEST,				VNUM_OBJ },
	{	&OBJ_VNUM_TRAIN,				VNUM_OBJ },
	{	&OBJ_VNUM_BREW,				VNUM_OBJ },
	{	&OBJ_VNUM_ARROW,				VNUM_OBJ },
	{	&OBJ_VNUM_BLOOD_TRAIL,	      	        VNUM_OBJ },
	{	&OBJ_UNIQUE_DUMMY,			VNUM_OBJ },
	{	&OBJ_VNUM_SKELETON,			VNUM_OBJ },
	{	&OBJ_VNUM_MAGIC_PLATE,		        VNUM_OBJ },
	{       &OBJ_VNUM_ORE,				VNUM_OBJ },
	{ 	NULL , -1 						}
};

const struct size_type cont_table[] =
{
    {   "Lurin"  },
    {	"Acire"  },
    {	"Ishani"  },
    {	"Dhriag"   },
    {   "none"	   }
};
const struct spellfun_type spellfun_table[] =
{
	{		"spell_null",			spell_null				},
	{		"spell_acid_blast",		spell_acid_blast		},
	{		"spell_armor",			spell_armor				},
	{		"spell_bless",			spell_bless				},
	{		"spell_blindness",		spell_blindness			},
	{		"spell_burning_hands",	spell_burning_hands		},
	{		"spell_call_lightning",	spell_call_lightning	},
	{		"spell_calm",			spell_calm				},
	{		"spell_cancellation",	spell_cancellation		},
	{		"spell_cause_critical",	spell_cause_critical	},
	{		"spell_cause_light",	spell_cause_light		},
	{		"spell_cause_serious",	spell_cause_serious		},
	{		"spell_change_sex",		spell_change_sex		},
	{		"spell_chain_lightning",spell_chain_lightning   },
	{		"spell_charm_person",	spell_charm_person		},
	{		"spell_chill_touch",	spell_chill_touch		},
	{		"spell_colour_spray",	spell_colour_spray		},
	{		"spell_continual_light",spell_continual_light	},
	{		"spell_control_weather",spell_control_weather	},
	{		"spell_create_food",	spell_create_food		},
	{		"spell_create_rose",	spell_create_rose		},
	{		"spell_create_spring",	spell_create_spring		},
	{		"spell_create_water",	spell_create_water		},
	{		"spell_cure_blindness",	spell_cure_blindness	},
	{		"spell_cure_critical",	spell_cure_critical		},
	{		"spell_cure_disease",	spell_cure_disease		},
	{		"spell_cure_light",		spell_cure_light		},
	{		"spell_cure_poison",	spell_cure_poison		},
	{		"spell_cure_serious",	spell_cure_serious		},
	{		"spell_curse",			spell_curse		},
        {               "spell_dancing_sword",  spell_dancing_sword             },
	{		"spell_demonfire",		spell_demonfire			},
	{		"spell_detect_evil",	spell_detect_evil		},
	{		"spell_detect_good",	spell_detect_good		},
	{		"spell_detect_hidden",	spell_detect_hidden		},
	{		"spell_detect_invis",	spell_detect_invis		},
	{		"spell_detect_magic",	spell_detect_magic		},
	{		"spell_detect_poison",	spell_detect_poison		},
	{		"spell_dispel_evil",	spell_dispel_evil		},
	{		"spell_dispel_good",	spell_dispel_good       },
	{		"spell_dispel_magic",	spell_dispel_magic		},
	{		"spell_earthquake",		spell_earthquake		},
	{		"spell_enchant_armor",	spell_enchant_armor		},
	{		"spell_enchant_weapon",	spell_enchant_weapon	},
	{		"spell_energy_drain",	spell_energy_drain		},
	{		"spell_faerie_fire",	spell_faerie_fire		},
	{		"spell_faerie_fog",		spell_faerie_fog		},
	{		"spell_farsight",		spell_farsight			},
	{		"spell_fireball",		spell_fireball			},
	{		"spell_fireproof",		spell_fireproof			},
	{		"spell_flamestrike",	spell_flamestrike		},
	{		"spell_floating_disc",	spell_floating_disc		},
	{		"spell_fly",			spell_fly				},
	{		"spell_frenzy",			spell_frenzy			},
	{		"spell_gate",			spell_gate				},
	{		"spell_giant_strength",	spell_giant_strength	},
	{		"spell_harm",			spell_harm				},
	{		"spell_haste",			spell_haste				},
	{		"spell_heal",			spell_heal				},
	{		"spell_heat_metal",		spell_heat_metal		},
	{		"spell_holy_word",		spell_holy_word			},
	{		"spell_identify",		spell_identify			},
	{		"spell_infravision",	spell_infravision		},
	{		"spell_invis",			spell_invis				},
	{		"spell_know_alignment",	spell_know_alignment	},
	{		"spell_lightning_bolt",	spell_lightning_bolt	},
	{		"spell_locate_object",	spell_locate_object		},
	{		"spell_energy_spike",	spell_energy_spike		},
	{		"spell_mass_healing",	spell_mass_healing		},
	{		"spell_mass_invis",		spell_mass_invis		},
        {               "spell_mirror_shield",  spell_mirror_shield                     },
	{		"spell_nexus",			spell_nexus				},
	{		"spell_pass_door",		spell_pass_door			},
	{		"spell_plague",			spell_plague			},
	{		"spell_poison",			spell_poison			},
	{		"spell_portal",			spell_portal			},
	{		"spell_protection_evil",spell_protection_evil	},
	{		"spell_protection_good",spell_protection_good	},
	{		"spell_ray_of_truth",	spell_ray_of_truth		},
	{		"spell_recharge",		spell_recharge			},
	{		"spell_refresh",		spell_refresh			},
	{		"spell_remove_curse",	spell_remove_curse		},
	{		"spell_sanctuary",		spell_sanctuary			},
	{		"spell_shocking_grasp",	spell_shocking_grasp	},
	{		"spell_shield",			spell_shield			},
	{		"spell_sleep",			spell_sleep				},
	{		"spell_slow",			spell_slow				},
	{		"spell_stone_skin",		spell_stone_skin		},
	{		"spell_summon",			spell_summon			},
	{		"spell_teleport",		spell_teleport			},
	{		"spell_ventriloquate",	spell_ventriloquate		},
	{		"spell_weaken",			spell_weaken			},
	{		"spell_word_of_recall",	spell_word_of_recall	},
	{		"spell_acid_breath",	spell_acid_breath		},
	{		"spell_fire_breath",	spell_fire_breath		},
	{		"spell_frost_breath",	spell_frost_breath		},
	{		"spell_gas_breath",		spell_gas_breath		},
	{		"spell_lightning_breath",spell_lightning_breath	},
	{		"spell_general_purpose",spell_general_purpose	},
	{		"spell_high_explosive",	spell_high_explosive	},
	{		"spell_trivia_pill",	spell_trivia_pill       },
	{		"spell_vicegrip",		spell_vicegrip          },
	{		"spell_hurricane",		spell_hurricane         },
	{		"spell_greater_heal",	spell_greater_heal      },
	{		"spell_super_heal",		spell_super_heal        },
	{		"spell_ultimate_heal",	spell_ultimate_heal     },
	{		"spell_group_heal",		spell_group_heal        },
	{		"spell_bark_skin",		spell_bark_skin         },
	{		"spell_knock",			spell_knock             },
	{		"spell_group_defense",	spell_group_defense     },
	{		"spell_wizards_fire",	spell_wizards_fire      },
	{		"spell_meteor_swarm",	spell_meteor_swarm      },
	{		"spell_group_sanctuary",spell_group_sanctuary   },
	{		"spell_holyblast",		spell_holyblast         },
	{		"spell_lightningblast",	spell_lightningblast    },
	{		"spell_electric_blast",	spell_electric_blast    },
	{		"spell_gasblast",		spell_gasblast          },
	{		"spell_iceblast",		spell_iceblast          },
	{		"spell_holy_bolt",		spell_holy_bolt         },
	{		"spell_acid_bolt",		spell_acid_bolt         },
	{		"spell_gas_bolt",		spell_gas_bolt          },
	{		"spell_fire_bolt",		spell_fire_bolt         },
	{		"spell_ice_bolt",		spell_ice_bolt          },
	{		"spell_diamond_skin",	spell_diamond_skin      },
	{		"spell_mud_skin",		spell_mud_skin          },
	{		"spell_moss_skin",		spell_moss_skin         },
	{		"spell_sate",			spell_sate              },
	{		"spell_quench",			spell_quench            },
	{		"spell_cone_of_cold",	spell_cone_of_cold      },
	{		"spell_synon_aura",		spell_synon_aura        },
	{		"spell_drain_blade",	spell_drain_blade       },
	{		"spell_shocking_blade",	spell_shocking_blade    },
	{		"spell_flame_blade",	spell_flame_blade       },
	{		"spell_frost_blade",	spell_frost_blade       },
	{		"spell_sharp_blade",	spell_sharp_blade       },
	{		"spell_vorpal_blade",	spell_vorpal_blade      },
	{		"spell_summon_ggolem",	spell_summon_ggolem     },
	{		"spell_fire_and_ice",	spell_fire_and_ice      },
	{		"spell_preserve",		spell_preserve          },
	{		"spell_sunbolt",		spell_sunbolt           },
	{		"spell_heroes_feast",	spell_heroes_feast      },
	{		"spell_mind_shatter",	spell_mind_shatter      },
	{		"spell_wizards_firestorm", spell_wizards_firestorm },
	{		"spell_prismatic_spray", spell_prismatic_spray   },
	{		"spell_revitalize",		spell_revitalize        },
	{		"spell_sturning",		spell_sturning          },
	{		"spell_steel_skin",		spell_steel_skin        },
	{		"spell_tsunami",		spell_tsunami           },    
	{		"spell_shadow_shield",	spell_shadow_shield     },
	{		"spell_weapon_bless",	spell_weapon_bless      },
	{		"spell_dark_blessing",	spell_dark_blessing     },
	{		"spell_divine_protection",	spell_divine_protection },
	{		"spell_moonblade",		spell_moonblade         },
	{		"spell_tornado",		spell_tornado           },
	{		"spell_wizards_fury",	spell_wizards_fury      },
	{		"spell_glacier",		spell_glacier           },
	{		"spell_thunderstorm",	spell_thunderstorm      },
	{		"spell_mystic_armor",	spell_mystic_armor      },
	{		"spell_desert_fist",	spell_desert_fist       },
	{		"spell_resilience",		spell_resilience        },
	{		"spell_magic_resistance", spell_magic_resistance  },
	{		"spell_sandstorm",		spell_sandstorm         },
	{		"spell_fire_blade",		spell_fire_blade        },
	{		"spell_call_wolf",		spell_call_wolf         },
	{		"spell_random",			spell_random            },
	{		"spell_banshee_scream",	spell_banshee_scream    },
	{		"spell_resurrect",		spell_resurrect         },
	{		"spell_raise_skeleton",	spell_raise_skeleton    },
	{		"spell_iron_golem",		spell_iron_golem        },
	{		"spell_stone_golem",	spell_stone_golem       },
	{		"spell_adamantite_golem", spell_adamantite_golem  },
	{		"spell_raise_dead",		spell_raise_dead        },
	{		"spell_magic_plate",	spell_magic_plate       },
	{		"spell_decay_corpse",	spell_decay_corpse      },
	{		"spell_create_mummy",	spell_create_mummy      },
	{		"spell_call_slayer",	spell_call_slayer       },
	{		"spell_lesser_golem",	spell_lesser_golem      },
	{		"spell_forceshield",	spell_forceshield		},
	{		"spell_staticshield",	spell_staticshield		},
	{		"spell_flameshield",	spell_flameshield		},
	{		"spell_powerstorm",		spell_powerstorm        },
	{		"spell_animal_instinct",spell_animal_instinct   },
	{		"spell_earthdrain",		spell_earthdrain        },
	{		"spell_chaos_flare",	spell_chaos_flare       },
	{		"spell_mantle",			spell_mantle            },
	{		"spell_arcane_magic",	spell_arcane_magic      },
	{		"spell_animate",		spell_animate           },
        {               "spell_essence_of_mist",        spell_essence_of_mist },
        {               "spell_essence_of_life",        spell_essence_of_life },
        {               "spell_essence_of_magic",       spell_essence_of_magic },
        {               "spell_lifeforce",              spell_lifeforce },
        {               "spell_silver_fire",            spell_silver_fire },
        {               "spell_soulburn",               spell_soulburn },
        {               "spell_icelance",               spell_icelance },
	{		"spell_shield_darkness",	spell_shield_darkness },
	{		NULL,					NULL					}
};

const struct flag_type target_table[] =
{
 { "ignore", TAR_IGNORE, TRUE },
 { "char_offensive",  TAR_CHAR_OFFENSIVE,    TRUE },
 { "char_defensive",  TAR_CHAR_DEFENSIVE,    TRUE },
 { "char_self", TAR_CHAR_SELF, TRUE },
 { "obj_inventory", TAR_OBJ_INV, TRUE },
 { "obj_char_defensive", TAR_OBJ_CHAR_DEF, TRUE },
 { "obj_char_ofensive", TAR_OBJ_CHAR_OFF, TRUE },
 { NULL, 0, 0 },
};


const struct smith_type smith_table[] =
{
     /* Name */             /* cost */
    { "Hitroll",		10      },
    { "Damroll",                10	},
    { "Dex",                    2       },
    { "Int",                    2	},
    { "Wis",                    2	},
    { "Con",                    2	},
    { "Str",                    2	},
    { "Hp",                     4	},
    { "Mana",                   4	},
    { "Move",                   4 	},
    { "Ac",                     1 	},
    { "Saves",                  10 	},
    { "Clevel",                 10	},
    { "Cability",               10 	},
    { "Invisible",              15 	},
    { "Detect_evil",            12 	},
    { "Detect_invis",           14 	},
    { "Detect_magic",           12 	},
    { "Detect_hidden",          13 	},
    { "Detect_good",            14	},
    { "Sanctuary",              20 	},
    { "Protect_evil",           15 	},
    { "Protect_good",           12 	},
    { "Sneak",                  13 	},
    { "Hide",                   10 	},
    { "Infrared",               12 	},
    { "Flying",                 11 	},
    { "Haste",                  15 	},
    { "Pass_door",              13 	},
    { "Swim",                   14 	},
    { "Regeneration",           18 	},
    { "Berserk",                17 	},
    { NULL, 0 },
};

const struct rain_type rain_table[ MAX_RAIN ] =
{
   { "There is a cold rain trickling down.",  		},
   { "There is a warm rain trickling down.",  		},
   { "Giant drops of rain are pouring from the sky.", 	},
   { "Ring sized drops of rain are falling from the sky.",    },
   { "Soft warm rain is slowly coming down.", 	 	},
   { "Hand sized drops of rain are falling from the sky.",    },
   { "Boot sized drops of rain are falling from the sky.",    }
};


const struct cloud_type cloud_table[ MAX_CLOUD ] =
{
   { "An overcast sheet of clouds heavily blankets the sky.", },
   { "Wisps of clouds delicately curl and drift above.", }
};

const struct cloudy_description cloudy_table[ MAX_CLOUDY ] =
{
   { "A clear azure sky blankets the land.",	},
   { "The pristine blue of the sky above spreads, unmarred by a cloud or a bird." } 
};

const struct hailstorm_description hailstorm_table[ MAX_HAILSTORM ] =
{
   { "Small pellets of ice fall from the sky and bounce as they hit the ground.",		},
   { "Balls of hail blanket the ground covering the landscape.", }
};

const struct thunderstorm_description thunderstorm_table[ MAX_THUNDERSTORM ] =
{
   { "A large thunderstorm approaches you from a distance.",	},
   { "A deep roar of thunder causes the very earth to tremble.", 	},
   { "Flashes of light stab and illuminate the sky.",               }
};

const struct icestorm_description icestorm_table[ MAX_ICESTORM ] =
{
   { "Waves of ice fall continually from the sky.",	},
   { "Shards of ice mercilessly batter the land.", }
};

const struct snow_description snow_table[ MAX_SNOW ] =
{
   { "Soft flakes of snow drift from above, covering the land in a wash of white.", },
   { "Large snowflakes swirl and drift, obscuring the nearby landscape.", }
};

const struct blizzard_description blizzard_table[ MAX_BLIZZARD ] =
{
   { "Large drifts of snow cover the ground blown out by driving winds.", },
   { "Blinding snow drives into the land, carried on howling gusts." }
};

const struct foggy_description foggy_table[ MAX_FOGGY ] =
{
   { "A misty haze covers the horizon.",	},
   { "Rolling fog claims the land clinging to the ground.", }
};

const struct lightning_description lightning_table[ MAX_LIGHTNING ] =
{
   { "A bolt of lightning pierces the sky as a large storm approaches.",  },
   { "Violent flashes of light rent the dark clouds.",			    }
};

const struct breeze_description breeze_table[ MAX_BREEZE ] =
{
   { "A warm breeze flows through the land blanketing it with heat.",		},
   { "A cold breeze flows through the land blanketing it with frost.",		}
};

const struct wind_description wind_table[ MAX_WIND ] =
{
   { "Gusts of wind sweep across the ground, pushing against any resistance.", },
   { "Pressing winds push against the land, sighing mounfully with their passage.", }
};

const struct size_type mineral_table[] =
{
   { "steel" 		},
   { "iron"  		},
   { "copper" 		},
   { "adamantium" 	},
   { "platinum" 	},
   { "bronze" 		},
   { "silver" 		},
   { "marble" 		},
   { "brass" 		},
   { "granite" 		},
   { "electrum" 	},
   { NULL 		}
};

const struct const_type const_table[] =
{
	{   "MOB_VNUM_FIDO",				&MOB_VNUM_FIDO				},
	{   "MOB_VNUM_CITYGUARD",			&MOB_VNUM_CITYGUARD			},
	{   "MOB_VNUM_REGISTAR",			&MOB_VNUM_REGISTAR			},
	{   "MOB_VNUM_WARMASTER",			&MOB_VNUM_WARMASTER			},
	{   "MOB_VNUM_GGOLEM",				&MOB_VNUM_GGOLEM			},
	{   "MOB_VNUM_STONE_GOLEM",			&MOB_VNUM_STONE_GOLEM			},
	{   "MOB_VNUM_IRON_GOLEM",			&MOB_VNUM_IRON_GOLEM			},
	{   "MOB_VNUM_ADAMANTITE_GOLEM",		&MOB_VNUM_ADAMANTITE_GOLEM		},
	{   "MOB_VNUM_L_GOLEM",				&MOB_VNUM_L_GOLEM			},
	{   "MOB_VNUM_WOLF",				&MOB_VNUM_WOLF				},
	{   "MOB_VNUM_ZOMBIE",				&MOB_VNUM_ZOMBIE			},
	{   "MOB_VNUM_SLAYER",				&MOB_VNUM_SLAYER			},
	{   "MOB_VNUM_MUMMY",				&MOB_VNUM_MUMMY				},
	{   "MOB_VNUM_SKELETON",			&MOB_VNUM_SKELETON			},
	{   "MOB_VNUM_RENTER",				&MOB_VNUM_RENTER			},
	{   "MOB_VNUM_FAMILIAR",			&MOB_VNUM_FAMILIAR			},
	{   "MOB_VNUM_ANIMATE",				&MOB_VNUM_ANIMATE			},
	{   "OBJ_FRUIT_DUMMY",				&OBJ_FRUIT_DUMMY			},
	{   "OBJ_VNUM_SBLADE",				&OBJ_VNUM_SBLADE			},
	{   "OBJ_VNUM_SILVER_ONE",			&OBJ_VNUM_SILVER_ONE			},
	{   "OBJ_VNUM_GOLD_ONE",			&OBJ_VNUM_GOLD_ONE			},
	{   "OBJ_VNUM_GOLD_SOME",			&OBJ_VNUM_GOLD_SOME			},
	{   "OBJ_VNUM_SILVER_SOME",			&OBJ_VNUM_SILVER_SOME			},
	{   "OBJ_VNUM_COINS",				&OBJ_VNUM_COINS				},
	{   "OBJ_VNUM_CORPSE_NPC",			&OBJ_VNUM_CORPSE_NPC			},
	{   "OBJ_VNUM_CORPSE_PC",			&OBJ_VNUM_CORPSE_PC			},
	{   "OBJ_VNUM_SEVERED_HEAD",			&OBJ_VNUM_SEVERED_HEAD			},
	{   "OBJ_VNUM_TORN_HEART",			&OBJ_VNUM_TORN_HEART			},
	{   "OBJ_VNUM_SLICED_ARM",			&OBJ_VNUM_SLICED_ARM			},
	{   "OBJ_VNUM_SLICED_LEG",			&OBJ_VNUM_SLICED_LEG			},
	{   "OBJ_VNUM_GUTS",				&OBJ_VNUM_GUTS				},
	{   "OBJ_VNUM_BRAINS",				&OBJ_VNUM_BRAINS			},
	{   "OBJ_VNUM_MUSHROOM",			&OBJ_VNUM_MUSHROOM			},
	{   "OBJ_VNUM_LIGHT_BALL",			&OBJ_VNUM_LIGHT_BALL			},
	{   "OBJ_VNUM_SPRING",				&OBJ_VNUM_SPRING			},
	{   "OBJ_VNUM_DISC",				&OBJ_VNUM_DISC				},
	{   "OBJ_VNUM_PORTAL",				&OBJ_VNUM_PORTAL			},
	{   "OBJ_VNUM_ROSE",				&OBJ_VNUM_ROSE				},
	{   "OBJ_VNUM_SCHOOL_MACE",			&OBJ_VNUM_SCHOOL_MACE			},
	{   "OBJ_VNUM_SCHOOL_DAGGER",			&OBJ_VNUM_SCHOOL_DAGGER			},
	{   "OBJ_VNUM_SCHOOL_SWORD",			&OBJ_VNUM_SCHOOL_SWORD			},
	{   "OBJ_VNUM_SCHOOL_SPEAR",			&OBJ_VNUM_SCHOOL_SPEAR			},
	{   "OBJ_VNUM_SCHOOL_AXE",			&OBJ_VNUM_SCHOOL_AXE			},
	{   "OBJ_VNUM_SCHOOL_FLAIL",			&OBJ_VNUM_SCHOOL_FLAIL			},
	{   "OBJ_VNUM_SCHOOL_WHIP",			&OBJ_VNUM_SCHOOL_WHIP			},
	{   "OBJ_VNUM_SCHOOL_POLEARM",			&OBJ_VNUM_SCHOOL_POLEARM		},
	{   "OBJ_VNUM_SCHOOL_SHIELD",			&OBJ_VNUM_SCHOOL_SHIELD			},
        {   "OBJ_VNUM_RING",                            &OBJ_VNUM_RING			 },
        {   "OBJ_VNUM_BRACELET",                        &OBJ_VNUM_BRACELET			 },
        {   "OBJ_VNUM_GLOVES",                          &OBJ_VNUM_GLOVES		},
        {   "OBJ_VNUM_HELMET",                          &OBJ_VNUM_HELMET             },
        {   "OBJ_VNUM_NECKLACE",                        &OBJ_VNUM_NECKLACE          },
        {   "OBJ_VNUM_SYMBOL_NOONE",                    &OBJ_VNUM_SYMBOL_NOONE			},
        {   "OBJ_VNUM_SCHOOL_PANTS",                    &OBJ_VNUM_SCHOOL_PANTS			},
        {   "OBJ_VNUM_BEGINING",                        &OBJ_VNUM_BEGINING			},
        {   "OBJ_VNUM_SCHOOL_SATCHEL",                  &OBJ_VNUM_SCHOOL_SATCHEL		},
        {   "OBJ_VNUM_SCHOOL_SANDLES",                  &OBJ_VNUM_SCHOOL_SANDLES		},
        {   "OBJ_VNUM_SCHOOL_SHIRT",                    &OBJ_VNUM_SCHOOL_SHIRT			},
        {   "OBJ_VNUM_SCHOOL_WATER_SKIN",               &OBJ_VNUM_SCHOOL_WATER_SKIN		},
        {   "OBJ_VNUM_BREAD",                           &OBJ_VNUM_BREAD		},
	{   "OBJ_VNUM_BOW",				&OBJ_VNUM_BOW				},
	{   "OBJ_VNUM_ORE",				&OBJ_VNUM_ORE				},
	{   "OBJ_VNUM_WHISTLE",				&OBJ_VNUM_WHISTLE			},
	{   "OBJ_VNUM_TOKEN",				&OBJ_VNUM_TOKEN				},
	{   "OBJ_VNUM_NORMAL",				&OBJ_VNUM_NORMAL			},
	{   "OBJ_VNUM_EXPERIENCE",			&OBJ_VNUM_EXPERIENCE			},
	{   "OBJ_VNUM_PRACTICE",			&OBJ_VNUM_PRACTICE			},
	{   "OBJ_VNUM_QUEST",				&OBJ_VNUM_QUEST				},
	{   "OBJ_VNUM_TRAIN",				&OBJ_VNUM_TRAIN				},
	{   "OBJ_VNUM_BREW",				&OBJ_VNUM_BREW				},
	{   "OBJ_VNUM_ARROW",				&OBJ_VNUM_ARROW				},
	{   "OBJ_VNUM_BLOOD_TRAIL",			&OBJ_VNUM_BLOOD_TRAIL			},
	{   "OBJ_UNIQUE_DUMMY",				&OBJ_UNIQUE_DUMMY			},
	{   "OBJ_VNUM_SKELETON",			&OBJ_VNUM_SKELETON			},
	{   "OBJ_VNUM_MAGIC_PLATE",			&OBJ_VNUM_MAGIC_PLATE			},
	{   "OBJ_VNUM_STEAK",				&OBJ_VNUM_STEAK				},
	{   "ROOM_VNUM_LIMBO",				&ROOM_VNUM_LIMBO			},
	{   "ROOM_VNUM_CHAT",				&ROOM_VNUM_CHAT				},
	{   "ROOM_VNUM_TEMPLE",				&ROOM_VNUM_TEMPLE			},
	{   "ROOM_VNUM_ALTAR",				&ROOM_VNUM_ALTAR			},
	{   "ROOM_VNUM_SCHOOL",				&ROOM_VNUM_SCHOOL			},
	{   "ROOM_VNUM_BALANCE",			&ROOM_VNUM_BALANCE			},
	{   "ROOM_VNUM_CIRCLE",				&ROOM_VNUM_CIRCLE			},
	{   "ROOM_VNUM_DEMISE",				&ROOM_VNUM_DEMISE			},
	{   "ROOM_VNUM_HONOR",				&ROOM_VNUM_HONOR			},
	{   "ROOM_VNUM_MORGUE",				&ROOM_VNUM_MORGUE			},
	{   "ROOM_VNUM_DONATION",			&ROOM_VNUM_DONATION			},
	{   "ROOM_VNUM_BANK",				&ROOM_VNUM_BANK				},
	{   "ROOM_VNUM_WAITROOM",			&ROOM_VNUM_WAITROOM			},
	{   "ROOM_VNUM_RENTBOX",			&ROOM_VNUM_RENTBOX			},
	{   "ROOM_VNUM_RENTER",				&ROOM_VNUM_RENTER			},
	{   "ROOM_VNUM_TIMEOUT",			&ROOM_VNUM_TIMEOUT			},
	{   "VNUM_START",				&VNUM_START	},
	{   "VNUM_STOP",			        &VNUM_STOP	},
        {   "OBJ_VNUM_DIAMOND_RING",                    &OBJ_VNUM_DIAMOND_RING },
        {   "OBJ_VNUM_WEDDING_BAND",                    &OBJ_VNUM_WEDDING_BAND	},
        {   "MIN_LEVEL_MARRY",				&MIN_LEVEL_MARRY },
	{	NULL,							0							}
};

const struct gamble_item gamble_table[MAX_ITEM] =
{
  /* Vnum */
  { 20209 },
};
        
const struct flag_type suppress_flags[] =
{
  {      "flags",                SUPPRESS_FLAGS,         TRUE    },
  {      "flageffects",          SUPPRESS_FLAG_EFFECTS,  TRUE    },
  {      "dodge",                SUPPRESS_DODGE,         TRUE    },
  {      "burn",                 SUPPRESS_BURN,          TRUE    },
  {      "std",                  SUPPRESS_STD,           TRUE    },
  {      "shield",               SUPPRESS_SHIELD,        TRUE    },
  {      "shieldcomp",           SUPPRESS_SHIELD_COMP,   TRUE    },
  {      NULL,                   0,                      0       }
};         

const struct  house_item  house_table [] =
{
    /* Name */   /* Cost */  /* Vnum */    /* Type */
  {   "Couch",      200,        20706,      OBJ_VNUM    },
  {   "Healer",     500,        20701,      MOB_VNUM    },
  {   NULL,          0,         0,          0,          }
};

 
const struct weather_type weather_table[MAX_WEATHER] =
{
   { "There is a cold rain trickling down.",						},
   { "There is a warm rain trickling down.",						},
   { "Giant drops of rain are pouring from the sky.",					},
   { "Ring sized drops of rain are falling from the sky.",				},
   { "Soft warm rain is slowly coming down.",                   			},
   { "Hand sized drops of rain are falling from the sky.",      			},
   { "Boot sized drops of rain are falling from the sky.",      			},
   { "A clear azure sky blankets the land.",                    	        	},
   { "The pristine blue of the sky above spreads, unmarred by a cloud or a bird.", 	},
   { "An overcast sheet of clouds heavily blankets the sky.",   			},
   { "Wisps of clouds delicately curl and drift above.",		    		},
   { "Small pellets of ice fall from the sky and bounce as they hit the ground.",	},
   { "Balls of hail blanket the ground covering the landscape.",			},
   { "A large thunderstorm approaches you from a distance.",   				},
   { "A deep roar of thunder causes the very earth to tremble.",			},
   { "Flashes of light stab and illuminate the sky.",                   		},
   { "Waves of ice fall conintually from the sky.", 					},
   { "Shards of ice mercilessly batter the land.", 					},
   { "Soft flakes of snow drift from above, covering the land in a wash of white.",	},
   { "Large snowflakes swirl and drift, obscuring the nearby landscape.", 		},
   { "Large drifts of snow cover the ground blown out by driving winds.", 		},
   { "Blinding snow drives into the land, carried on howling gusts.",			},
   { "A misty haze covers the horizon.",						},
   { "Rolling fog claims the land clinging to the ground.",				},
   { "A bolt of lightning pierces the sky as a large storm approaches.", 		},
   { "Violent flashes of light rent the dark clouds.",					},
   { "A warm breeze flows through the land blanketing it with heat.",   		},
   { "A cold breeze flows through the land blanketing it with frost.",			},
   { "Gusts of wind sweep across the ground, pushing against any resistance.",		},
   { "Pressing winds push against the land, sighing mournfully with their passage.",	}, 
};   

const struct summoner_data summoner_table[] =
{ /* Name */        /* Cost */ /* Vnum */
  {"Ofcol",             50,     637    },
  {"New Thalos",        75,     3900   },
  {"Miden'nir",         200,    3500   },
  {"Hell",              500,    10416  },
  {"Pyramid",           75,     8700   },
  {"Olympus",           100,    900    },
  {"Drow City",         200,    5269   },
  {NULL,0,0 } /* End of list - must always end with this! - Kharas */
};                           
