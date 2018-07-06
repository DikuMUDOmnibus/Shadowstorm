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

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define DECLARE_OBJ_FUN( fun )		void fun( )
#define DECLARE_ROOM_FUN( fun )		void fun( )
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	void fun( )
#define DECLARE_SONG_FUN( fun )         void fun( int songnum, int level, CHAR_DATA *ch )
#define DECLARE_OBJ_FUN( fun )		OBJ_FUN	  fun
#define DECLARE_ROOM_FUN( fun )		ROOM_FUN  fun
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#define DECLARE_SONG_FUN( fun )         void fun( int songnum, int level, CHAR_DATA *ch )
#define DECLARE_OBJ_FUN( fun )		OBJ_FUN	  fun
#define DECLARE_ROOM_FUN( fun )		ROOM_FUN  fun
#endif

#define SPELL(fun)                      void fun (int sn,int level,CHAR_DATA *ch,void *vo,int target)

#define MCCP_ENABLED


/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#define unix
#else
typedef short   int			sh_int;
#endif

#include "mccp.h"

#define DEBUG

#define COMPRESS_BUF_SIZE 16384

/*
 * Structure types.
 */
typedef struct	affect_data			AFFECT_DATA;
typedef struct	area_data			AREA_DATA;
typedef struct	ban_data			BAN_DATA;
typedef struct 	buf_type	 		BUFFER;
typedef struct	char_data			CHAR_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data			EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data			HELP_DATA;
typedef struct	kill_data			KILL_DATA;
typedef struct	mem_data			MEM_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data			NOTE_DATA;
typedef struct	obj_data			OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data				PC_DATA;
typedef struct  gen_data			GEN_DATA;
typedef struct	reset_data			RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data	    SHOP_DATA;
typedef struct	time_info_data	    TIME_INFO_DATA;
typedef struct	weather_data	    WEATHER_DATA;
typedef struct  bounty_data         BOUNTY_DATA;
typedef struct	colour_data	    COLOUR_DATA;
typedef struct  prog_list           PROG_LIST;
typedef struct  prog_code           PROG_CODE;
typedef struct  auction_data        AUCTION_DATA;
typedef struct  wiz_data            WIZ_DATA;
typedef struct  gquest_data         GQUEST;
typedef struct  clan_type           CLAN_DATA;
typedef struct  war_data            WAR_DATA; 
typedef struct  nick_data	    NICK_DATA;
typedef struct  event_data	    EVENT_DATA;  /* timed events */
typedef struct  member_data	    MEMBER_DATA;
typedef struct  email_type	    EMAIL_DATA;
typedef struct  stat_data	    STAT_DATA;
typedef struct  command_type	    CMD_DATA;
typedef struct  mine_type	    MINE_DATA;
typedef struct  shaft_type          SHAFT_DATA;
typedef struct  pet_data            PET_DATA;
typedef struct	religion_type       RELIGION;
typedef struct	wearloc_type        WEAR_LOC;
typedef struct  trivia_data         TRIVIA_DATA;  
typedef struct  house_data          HOUSE_DATA;
typedef struct  trivia_score_data   TRIVIA_SCORE;
typedef struct  mail_data           MAIL_DATA;
typedef struct	trade_data          TRADE_DATA;


/*
 * Function types.
 */
typedef	void DO_FUN	args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef void SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo,
				int target ) );
typedef void SONG_FUN   args( ( int songnum, int level, CHAR_DATA *ch ) );
typedef void OBJ_FUN	args( ( OBJ_DATA *obj, char *argument ) );
typedef void ROOM_FUN	args( ( ROOM_INDEX_DATA *room, char *argument ) );




/*
 * String and memory management parameters.
 */
#define	MAX_KEY_HASH		1024
#define MAX_STRING_LENGTH	20000
#define MAX_INPUT_LENGTH	 5000
#define PAGELEN			   22
#define MAX_MEM_LIST            11
#define MXP_SAFE                1
#define MXP_ALL                 2
#define MXP_NONE                3
#define MAX_TRADE_OBJ		3
#define TRADE_LOCKED		1
#define TRADE_AGREED		2


/* I am lazy :) */
#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH
#define MAX_EXITS               4
#define MONEY                   "Silver"
/* max shares per level */
#define SHARE_MAX               5
#define AUCTION_LENGTH          3
#define MAX_GQUEST_MOB          25
#define MAX_SONG	        5
#define MAX_VNUM                32768
#define MAX_STORAGE             15
#define MAX_QUOTES              10
#define MAX_WAYPOINT            10
#define MAX_ADMIN               3
#define LI3                     MAX_LEVEL/6
#define LI2                     MAX_LEVEL/3
#define LI1                     MAX_LEVEL 
#define MIN_TEMP                -40
#define MAX_TEMP                120
#define MOBJLEVEL               10
#define MAX_ITEM                1
#define MIN_XP                  2000
#define MIN_RAIN                0
#define MUDINFO_UPDATED      0
#define MUDINFO_MCCP_USERS   1
#define MUDINFO_OTHER_USERS  2
#define MUDINFO_PEAK_USERS   3
#define MUDINFO_MBYTE        4
#define MUDINFO_BYTE         5
#define MUDINFO_DATA_PEAK    6
#define MUDINFO_MBYTE_S      7
#define MUDINFO_BYTE_S       8
#define MUDINFO_MAX          9

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
extern int MAX_SKILL;
#define MAX_GROUP		   35
#define MAX_IN_GROUP		   20
#define MAX_ALIAS		    5
#define MAX_CLASS		    8
#define MAX_MCLASS                 (MAX_CLASS + 7)
#define MAX_REMORT                  8
#define MAX_PC_RACE		    8
#define MAX_CLAN		   17
#define MAX_RANK                    6 /* guild.c */
#define MAX_DAMAGE_MESSAGE	   47
#define MAX_LEVEL		   159
#define MAX_MINERALS		   12
#define LEVEL_HERO		   (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL		   (MAX_LEVEL - 8)

#define PULSE_PER_SECOND	    4
#define PULSE_VIOLENCE		  ( 3 * PULSE_PER_SECOND)
#define PULSE_MOBILE		  ( 4 * PULSE_PER_SECOND)
#define PULSE_MUSIC		  ( 6 * PULSE_PER_SECOND)
#define PULSE_TICK		  (60 * PULSE_PER_SECOND)
#define PULSE_AREA		  (120 * PULSE_PER_SECOND)
#define PULSE_AUCTION             (30 * PULSE_PER_SECOND)
#define PULSE_HINT                (30 * PULSE_PER_SECOND)
#define PULSE_EVENT               ( 2 * PULSE_PER_SECOND)
#define PULSE_TRIVIA              ( 5 * PULSE_PER_SECOND)


#define IMPLEMENTOR		MAX_LEVEL
#define	CREATOR			(MAX_LEVEL - 1)
#define SUPREME			(MAX_LEVEL - 2)
#define DEITY			(MAX_LEVEL - 3)
#define GOD			(MAX_LEVEL - 4)
#define IMMORTAL		(MAX_LEVEL - 5)
#define DEMI			(MAX_LEVEL - 6)
#define ANGEL			(MAX_LEVEL - 7)
#define AVATAR			(MAX_LEVEL - 8)
#define HERO			LEVEL_HERO

/*Align's for Races */
#define ALIGN_GOOD 	750
#define ALIGN_EVIL 	-750
#define ALIGN_NEUTRAL 	0

/*
 * ColoUr stuff v2.0, by Lope.
 */
#define CLEAR		"\e[0m"		/* Resets Colour	*/
#define C_RED		"\e[0;31m"	/* Normal Colours	*/
#define C_GREEN		"\e[0;32m"
#define C_YELLOW	"\e[0;33m"
#define C_BLUE		"\e[0;34m"
#define C_MAGENTA	"\e[0;35m"
#define C_CYAN		"\e[0;36m"
#define C_WHITE		"\e[0;37m"
#define C_D_GREY	"\e[1;30m"  	/* Light Colors		*/
#define C_B_RED		"\e[1;31m"
#define C_B_GREEN	"\e[1;32m"
#define C_B_YELLOW	"\e[1;33m"
#define C_B_BLUE	"\e[1;34m"
#define C_B_MAGENTA	"\e[1;35m"
#define C_B_CYAN	"\e[1;36m"
#define C_B_WHITE	"\e[1;37m"
#define C_UNDERSCORE    "\e[4m"

#define BLINKING        9
#define UNDERSCORE      8
#define COLOUR_NONE	7		/* White, hmm...	*/
#define RED		1		/* Normal Colours	*/
#define GREEN		2
#define YELLOW		3
#define BLUE		4
#define MAGENTA		5
#define CYAN		6
#define WHITE		7
#define BLACK		0

#define NORMAL		0		/* Bright/Normal colours */
#define BRIGHT		1

#define ALTER_COLOUR( type )	if( !str_prefix( argument, "red" ) )		\
				{						\
				    ch->pcdata->type[0] = NORMAL;		\
				    ch->pcdata->type[1] = RED;			\
				}						\
				else if( !str_prefix( argument, "hi-red" ) )	\
				{						\
				    ch->pcdata->type[0] = BRIGHT;		\
				    ch->pcdata->type[1] = RED;			\
				}						\
				else if( !str_prefix( argument, "green" ) )	\
				{						\
				    ch->pcdata->type[0] = NORMAL;		\
				    ch->pcdata->type[1] = GREEN;		\
				}						\
				else if( !str_prefix( argument, "hi-green" ) )	\
				{						\
				    ch->pcdata->type[0] = BRIGHT;		\
				    ch->pcdata->type[1] = GREEN;		\
				}						\
				else if( !str_prefix( argument, "yellow" ) )	\
				{						\
				    ch->pcdata->type[0] = NORMAL;		\
				    ch->pcdata->type[1] = YELLOW;		\
				}						\
				else if( !str_prefix( argument, "hi-yellow" ) )	\
				{						\
				    ch->pcdata->type[0] = BRIGHT;		\
				    ch->pcdata->type[1] = YELLOW;		\
				}						\
				else if( !str_prefix( argument, "blue" ) )	\
				{						\
				    ch->pcdata->type[0] = NORMAL;		\
				    ch->pcdata->type[1] = BLUE;		\
				}						\
				else if( !str_prefix( argument, "hi-blue" ) )	\
				{						\
				    ch->pcdata->type[0] = BRIGHT;		\
				    ch->pcdata->type[1] = BLUE;		\
				}						\
				else if( !str_prefix( argument, "magenta" ) )	\
				{						\
				    ch->pcdata->type[0] = NORMAL;		\
				    ch->pcdata->type[1] = MAGENTA;		\
				}						\
				else if( !str_prefix( argument, "hi-magenta" ) ) \
				{						\
				    ch->pcdata->type[0] = BRIGHT;		\
				    ch->pcdata->type[1] = MAGENTA;		\
				}						\
				else if( !str_prefix( argument, "cyan" ) )	\
				{						\
				    ch->pcdata->type[0] = NORMAL;		\
				    ch->pcdata->type[1] = CYAN;			\
				}						\
				else if( !str_prefix( argument, "hi-cyan" ) )	\
				{						\
				    ch->pcdata->type[0] = BRIGHT;		\
				    ch->pcdata->type[1] = CYAN;			\
				}						\
				else if( !str_prefix( argument, "white" ) )	\
				{						\
				    ch->pcdata->type[0] = NORMAL;		\
				    ch->pcdata->type[1] = WHITE;		\
				}						\
				else if( !str_prefix( argument, "hi-white" ) )	\
				{						\
				    ch->pcdata->type[0] = BRIGHT;		\
				    ch->pcdata->type[1] = WHITE;		\
				}						\
				else if( !str_prefix( argument, "grey" ) )	\
				{						\
				    ch->pcdata->type[0] = BRIGHT;		\
				    ch->pcdata->type[1] = BLACK;		\
				}						\
				else if( !str_prefix( argument, "beep" ) )	\
				{						\
				    ch->pcdata->type[2] = 1;			\
				}						\
				else if( !str_prefix( argument, "nobeep" ) )	\
				{						\
				    ch->pcdata->type[2] = 0;			\
				}						\
				else						\
				{						\
		send_to_char_bw( "Unrecognised colour, unchanged.\n\r", ch );	\
				    return;					\
				}

#define LOAD_COLOUR( field )	ch->pcdata->field[1] = fread_number( fp );	\
				if( ch->pcdata->field[1] > 100 )		\
				{						\
				    ch->pcdata->field[1] -= 100;		\
				    ch->pcdata->field[2] = 1;			\
				}						\
				else						\
				{						\
				    ch->pcdata->field[2] = 0;			\
				}						\
				if( ch->pcdata->field[1] > 10 )			\
				{						\
				    ch->pcdata->field[1] -= 10;			\
				    ch->pcdata->field[0] = 1;			\
				}						\
				else						\
				{						\
				    ch->pcdata->field[0] = 0;			\
				}


#define CREATE(result, type, number)                    \
do                                        \
{                                        \
   if (!((result) = (type *) calloc ((number), sizeof(type))))     \
     { perror("malloc failure"); abort(); }               \
} while(0)


# define MXP_OPEN      "[0z"      /* Open Line, allows "open" category MXP commands */
# define MXP_SECURE    "[1z"      /* Secure Line, allows only Secure Commands (until next newline )*/
# define MXP_LOCKED    "[2z"      /* Locked Line, no parsing!, until next newline. */
// These are in v0.4 of the spec:
# define MXP_RESET     "[3z"      /* Reset. Close all open tags, Mode Open, text/color to default. */
# define MXP_STAG      "[4z"      /* Temp Secure, next tag ONLY is secure, must set again to close tag. */
# define MXP_LOPEN     "[5z"      /* Open mode until changed. */
# define MXP_LSECURE   "[6z"      /* Secure mode until changed. */
# define MXP_LLOCK     "[7z"      /* No parsing until changed. */
// v0.3 of the spec ( zMud ):
# define MXP_RNAME     "[10z"     /* Room Name - Not used here, we use an entity */
# define MXP_RDESC     "[11z"     /* Room Description - ditto */
# define MXP_REXIT     "[12z"     /* Room Exits - ditto */
# define MXP_WELCOME   "[19z"     /* Welcome text... seems to be used only for relocate, don't need it. */


/*
 * Site ban structure.
 */

#define BAN_SUFFIX		A
#define BAN_PREFIX		B
#define BAN_NEWBIES		C
#define BAN_ALL			D	
#define BAN_PERMIT		E
#define BAN_PERMANENT		F

/*
 * Clan flags
 */

#define GUILD_DELETED		A
#define GUILD_CHANGED		B
#define GUILD_INDEPENDENT 	C /* a "loner" guild */
#define GUILD_IMMORTAL		E /* immortal only clan */

//Sheath yes no stuff
#define SHEATH_YES 	0
#define SHEATH_NO	1


struct trivia_score_data
{
   TRIVIA_SCORE *next;
   char *name;
   int   points;
   int   totalpoints;
};


struct	ban_data
{
    BAN_DATA *	next;
    bool	valid;
    sh_int	ban_flags;
    sh_int	level;
    char *	name;
};

struct  wiz_data
{
    WIZ_DATA *  next;
    bool        valid;
    sh_int      level;
    char *      name;
};     

struct buf_type
{
    BUFFER *    next;
    bool        valid;
    sh_int      state;  /* error state of the buffer */
    sh_int      size;   /* size in k */
    char *      string; /* buffer's string */
};

/*
 * Bounty stuff -- by Gomi no sensei
 */

struct bounty_data
{
    BOUNTY_DATA *next;
    bool        valid;
    char *      name;
    int         amount;
};

struct wizcommand_type
{
    char *name;
    struct wizcommand_type *next;
};           

struct	hometown_type
{
	char	*name;
	int	recall;
	int	school;
	int	morgue;
	int	death;
        int     donation;
        int     postal;
};

struct auction_data 
{
   AUCTION_DATA *next;
   OBJ_DATA     *item;
   CHAR_DATA    *owner;
   CHAR_DATA    *high_bidder;
   CHAR_DATA    *house;
   sh_int       status;
   sh_int       number;
   long         bid;
   bool         valid;
   bool         ishouse;
};

struct pet_data
{
    PET_DATA            *next;
    char *              name;
    char *              cname;
    char *              short_desc;
    char *              long_desc;
    int                 cost;
    int                 vnum;
    int                 level;
    int                 max_hit;
    int                 max_mana;
    int                 max_move;
    int                 exp;
    int                 xp_tolevel;
    sh_int              levelflux;
    int                 practice;
    int                 train;
    int                 damroll;
    int                 hitroll;
    int                 armor[4];
    int                 gold;
    int                 silver;
    bool                valid;
};                               


struct house_data
{
    HOUSE_DATA *        next;
    char *		oname;
    char *              objname;
    char *              mobname;
    int                 ovalue;
    int                 mvalue;
    bool		valid;
};
    
struct mail_data
{
    MAIL_DATA * 	next;
    OBJ_DATA *          item;
    int                 box;
    char *              sender;
    char *              to_who;
    char *              subject;
    char *              text;
    char *              date;
    time_t              date_stamp;
    time_t              expire;
    sh_int              id;
    bool                valid;
};
    
struct song_type
{
     char *      name;                   /* Name of song */
     char *      listname;               /* name that shows on do_songs */
     sh_int      level;                  /* Average of player vs instrument */
     SONG_FUN *  song_fun;               /* function called for song */
     sh_int      minimum_position;       /* min position to play song */
     sh_int      min_mana;               /* min mana song takes */
     sh_int      beats;                  /* Waiting time after use */
};


#define		GQUEST_OFF              0
#define		GQUEST_WAITING		1
#define		GQUEST_RUNNING		2


struct gquest_data
{
    int mobs[MAX_GQUEST_MOB];
    char *who;
    int mob_count;
    int timer;
    int involved;
    int qpoints;
    int gold;
    int minlevel;
    int maxlevel;
    int running;
    int next;
};           


/*
 * Drunk struct
 */
struct struckdrunk
{
	int	min_drunk_level;
	int	number_of_rep;
	char	*replacement[11];
};

 
struct material_type
{
        char *name;
        sh_int type;
        long vuln_flag;     
}; 

struct unique_attrib_table
{
  char *descriptive;
};

struct weather_type 
{
   char * description;
};

struct cloud_type
{ 
  char *description;
};

struct rain_type
{
  char *description;
};

struct cloudy_description
{
  char *description;
};

struct hailstorm_description
{
  char *description;
};

struct thunderstorm_description
{ 
  char *description;
};

struct icestorm_description
{
  char *description;
};

struct snow_description
{ 
  char *description;
};

struct blizzard_description
{ 
  char *description;
};

struct foggy_description
{ 
  char *description;
};

struct lightning_description
{
  char *description;
};

struct breeze_description
{
  char *description;
};

struct wind_description
{
  char *description;
};

/* for statlist command*/
#define PK_KILLS         0
#define MOB_KILLS        1
#define PK_DEATHS        2
#define MOB_DEATHS       3
#define MAX_GAMESTAT     4

struct stat_data 
{
    STAT_DATA *next;
    bool valid;
    char *name;                     // name of character
    long gamestat[MAX_GAMESTAT];    // stat data
};

struct smith_type
{
    char * type;
    int cost;
};

struct gamble_item
{
  int vnum;
};      

struct house_item
{
  char *name;
  int cost;
  int vnum;
  int type;
};    

/*
 * Time and weather stuff.
 */
#define SUN_DARK		    0
#define SUN_RISE		    1
#define SUN_LIGHT		    2
#define SUN_SET			    3

#define SKY_CLOUDLESS		    0
#define SKY_CLOUDY		    1
#define SKY_RAINING		    2
#define SKY_LIGHTNING		    3
#define SKY_SNOWING                 4
#define SKY_BLIZZARD                5
#define SKY_FOGGY                   6
#define SKY_HAILSTORM               7
#define SKY_THUNDERSTORM            8
#define SKY_ICESTORM                9
#define SKY_BREEZE                  10
#define SKY_WIND                    11

#define MAX_RAIN                    7
#define MAX_CLOUD                   2
#define MAX_CLOUDY                  2
#define MAX_HAILSTORM               2
#define MAX_THUNDERSTORM            3
#define MAX_ICESTORM                2
#define MAX_SNOW                    2
#define MAX_BLIZZARD                2
#define MAX_FOGGY                   2
#define MAX_LIGHTNING               2
#define MAX_BREEZE                  2
#define MAX_WIND                    2
#define MAX_WEATHER                30

#define SPRING                      0
#define SUMMER                      1
#define FALL                        2
#define WINTER                      3

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
};

struct	weather_data
{
    int		change;
    int		sky;
    int		sunlight;
    int         season;
};

struct  trivia_data
{
        bool   running;
        char  *question;
        char  *answer;
        sh_int reward;
        sh_int qnumber;
        sh_int timer;
};       

struct trade_half
{
    CHAR_DATA *ch;
    int flags;
    OBJ_DATA *objs[MAX_TRADE_OBJ];
    int gold;
};

struct trade_data
{
    struct trade_half first;
    struct trade_half second;
};

/*
 * Connected state for a channel.
 */
#define CON_PLAYING			 0
#define CON_GET_NAME			 1
#define CON_GET_OLD_PASSWORD		 2
#define CON_CONFIRM_NEW_NAME		 3
#define CON_GET_NEW_PASSWORD		 4
#define CON_CONFIRM_NEW_PASSWORD	 5
#define CON_GET_NEW_RACE		 6
#define CON_GET_NEW_SEX			 7
#define CON_GET_NEW_CLASS		 8
#define CON_GET_HAIR                     9
#define CON_GET_WEIGHT                  10
#define CON_GET_HEIGHT                  11
#define CON_GET_EYE                     12
#define CON_GET_ALIGNMENT		13
#define CON_GET_GOD                     14
#define CON_DEFAULT_CHOICE		15 
#define CON_GEN_GROUPS			16 
#define CON_PICK_WEAPON			17
#define CON_READ_IMOTD			18
#define CON_READ_MOTD			19
#define CON_BREAK_CONNECT		20
#define CON_COPYOVER_RECOVER            21
#define CON_NOTE_TO                     22
#define CON_NOTE_SUBJECT                23
#define CON_NOTE_EXPIRE                 24
#define CON_NOTE_TEXT                   25
#define CON_NOTE_FINISH                 26   
#define CON_MENU                        27
#define CON_GET_HOMETOWN                28
#define CON_ANSI			29
#define CON_SET_SHORT			30
#define CON_CREATION_MENU		31
#define CON_GET_EMAIL			32



/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    bool		valid;
    bool                walkabout;
    char *		host;
    sh_int		descriptor;
    sh_int		connected;
    bool		fcommand;
    char		inbuf		[4 * MAX_INPUT_LENGTH];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int			repeat;
    char *		outbuf;
    int			outsize;
    int			outtop;
    char *		showstr_head;
    char *		showstr_point;
    void *              pEdit;		/* OLC */
    char **             pString;	/* OLC */
    int			editor;		/* OLC */
    bool		ansi;
    bool                msp;
    bool                mxp;
    char *              client;
    char *              support; 
#ifdef MCCP_ENABLED
    z_stream           *out_compress;      
    unsigned char      *out_compress_buf;      
#endif
};


/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    sh_int	tohit;
    sh_int	todam;
    sh_int	carry;
    sh_int	wield;
};

struct	int_app_type
{
    sh_int	learn;
};

struct	wis_app_type
{
    sh_int	practice;
};

struct	dex_app_type
{
    sh_int	defensive;
};

struct	con_app_type
{
    sh_int	hitp;
    sh_int	shock;
};



/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_ALL		    4

/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    HELP_DATA * next_area;
    sh_int	level;
    char *	keyword;
    char *	text;
    bool	changed;

};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    sh_int	keeper;			/* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		/* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
};



/*
 * Per-class stuff.
 */

#define MAX_GUILD 	2
#define MAX_STATS 	5
#define STAT_STR 	0
#define STAT_INT	1
#define STAT_WIS	2
#define STAT_DEX	3
#define STAT_CON	4

struct	class_type
{
    char *	name;			/* the full name of the class */
    char 	who_name	[4];	/* Three-letter name for 'who'	*/
    sh_int	attr_prime;		/* Prime attribute		*/
    int   *	weapon;			/* First weapon			*/
    sh_int	guild[MAX_GUILD];	/* Vnum of guild rooms		*/
    sh_int	skill_adept;		/* Maximum skill level		*/
    sh_int	thac0_00;		/* Thac0 for level  0		*/
    sh_int	thac0_32;		/* Thac0 for level 32		*/
    sh_int	hp_min;			/* Min hp gained on leveling	*/
    sh_int	hp_max;			/* Max hp gained on leveling	*/
    bool	fMana;			/* Class gains mana on level	*/
    char *	base_group;		/* base skills gained		*/
    char *	default_group;		/* default skills gained	*/
};

struct item_type
{
    int		type;
    char *	name;
};

struct weapon_type
{
    char *	name;
    int	*	vnum;
    sh_int	type;
};

struct wiznet_type
{
    char *	name;
    long 	flag;
    int		level;
};

struct attack_type
{
    char *	name;			/* name */
    char *	noun;			/* message */
    int   	damage;			/* damage class */
};

struct weight_type
{
    char * name;
};

struct hair_type
{
    char * name;
};

struct height_type
{
    char * name;
};

struct eye_type
{
    char * name;
}; 

struct race_type
{
    char *	name;			/* call name of the race */
    bool	pc_race;		/* can be chosen by pcs */
    long	act;			/* act bits for the race */
    long	aff;			/* aff bits for the race */
    long	off;			/* off bits for the race */
    long	imm;			/* imm bits for the race */
    long        res;			/* res bits for the race */
    long	vuln;			/* vuln bits for the race */
    long	form;			/* default form flag for the race */
    long	parts;			/* default parts for the race */
};


struct pc_race_type  /* additional data for pc races */
{
    char *	name;			/* MUST be in race_type */
    char 	who_name[6];
    sh_int	points;			/* cost in points of the race */
    sh_int	class_mult[MAX_CLASS];	/* exp multiplier for class, * 100 */
    char *	skills[5];		/* bonus skills for the race */
    sh_int 	stats[MAX_STATS];	/* starting stats */
    sh_int	max_stats[MAX_STATS];	/* maximum stats */
    sh_int	size;			/* aff bits for the race */
    int		align;			//Race Align.
};


struct spec_type
{
    char * 	name;			/* special function name */
    SPEC_FUN *	function;		/* the function */
};



/*
 * Data structure for notes.
 */

struct	note_data
{
    NOTE_DATA *	next;
    bool 	valid;
    sh_int	type;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    char *	text;
    time_t  	date_stamp;
    time_t	expire;
};

struct summoner_data
{
    char *name;
    int cost;
    int vnum;
};


/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    bool		valid;
    sh_int		where;
    sh_int		type;
    sh_int		level;
    sh_int		duration;
    sh_int		location;
    sh_int		modifier;
    int			bitvector;
};

/* where definitions */
#define TO_AFFECTS	0
#define TO_OBJECT	1
#define TO_IMMUNE	2
#define TO_RESIST	3
#define TO_VULN		4
#define TO_WEAPON	5
#define TO_AFFECTS2     6
#define TO_OBJECT2      7

/*
 * A kill structure (indexed by level).
 */
struct	kill_data
{
    sh_int		number;
    sh_int		killed;
    sh_int              kills;
};


struct	nick_data
{
	char * nickname;
	char * realname;
	char * shortd;
	bool valid;
	NICK_DATA *next;
};

struct event_data
{
   int        delay;
   CHAR_DATA    *to;
   int       action;
   DO_FUN   *do_fun;
   char    *args[5];
   void    *argv[5];
   int      argi[5];
   EVENT_DATA *next;
   bool       valid;
};

extern EVENT_DATA *events;

/* timed event action */
#define ACTION_PRINT      1
#define ACTION_FUNCTION   2
#define ACTION_WAIT       3
#define ACTION_ACT        4

#define MOON_FULL 	4
#define MOON_THREE	3
#define MOON_TWO	2
#define MOON_ONE	1
#define MOON_GONE	0

/*Continent variables */
#define CONT_CALNON    0
#define CONT_BEALOTH   1
#define CONT_PEKSOR    2
#define CONT_HIRROT    3
#define CONT_NONE      4
#define MAX_CONT       5

struct email_type
{
     EMAIL_DATA *next;
     char *name;
     char *email;
     bool valid;
};

extern EMAIL_DATA * email_first;

struct  command_type
{
    CMD_DATA	*	prev;
    CMD_DATA 	*	next;
    CMD_DATA	*	char_next;
    bool			valid;
    char * 	        name;
    DO_FUN *        do_fun;
    sh_int          position;
    sh_int          level;
    sh_int          log;
    sh_int          show;
    sh_int          cat;
	bool			disabled;
};
extern CMD_DATA * cmd_first;
extern CMD_DATA * cmd_last;
#define MAX_MINE_X   25
#define MAX_MINE_Y   25
#define MAX_MINE_Z   5

struct mine_type
{   
    MINE_DATA  	*	next;
    char 	*	name;
    SHAFT_DATA  *	shaft_first;  
    SHAFT_DATA  *	shaft_last;
    ROOM_INDEX_DATA *   entrance;
    bool		valid;
    int                 mineral [MAX_MINERALS];
    int			map[MAX_MINE_X][MAX_MINE_Y][MAX_MINE_Z];
};


struct  shaft_type
{
    bool valid;
    SHAFT_DATA 	*	next;
    long 		parts;
    char 	*       desc;
    int 		mineral[MAX_MINERALS];
    EXIT_DATA   *       exit[6];
    int			cord[3];
};
#define RANK_INITIATE		0
#define RANK_ACOLYTE		1
#define RANK_DISCIPLE		2
#define RANK_BISHOP			3
#define RANK_PRIEST			4
#define RANK_DEITY			5
#define RANK_GOD			6
#define MAX_RELG_RANK			7

#define FACTION_ONE			0
#define FACTION_TWO			1
#define MAX_FACTION			2

struct religion_type
{
	char *		name;
	char *		rank[MAX_CLASS][MAX_RELG_RANK];
	bool		isfaction; // TRUE if faction.
	RELIGION *	main;    // For Factions only.
	char *		deity;   // For Factions only.
	char *		god;    // Main Religion Only.
	RELIGION *	next;
	bool		valid;
	char *		skpell[MAX_CLASS][MAX_RELG_RANK];    
	RELIGION *  faction[MAX_FACTION];
	AREA_DATA * temple;
	int			donation_vnum;
	char *		sac_msg;
	char *		pmt_msg;
	char *		dmt_msg;
	char *		chan_name;
};


struct wearloc_type
{	WEAR_LOC		*		next;
	WEAR_LOC		*		prev;
	WEAR_LOC		*		layer;
	OBJ_DATA		*		belt_list;
	OBJ_DATA		*		sheath;
	char			*		name;
	char			*		eqmsg;
	sh_int					wear_loc;
	bool					belt;
	bool					sheathed;
	bool					valid;
	bool					race[MAX_PC_RACE];
};

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */

/* RT ASCII conversions -- used so we can have letters in this file */

#define A		  	1
#define B			2
#define C			4
#define D			8
#define E			16
#define F			32
#define G			64
#define H			128

#define I			256
#define J			512
#define K		        1024
#define L		 	2048
#define M			4096
#define N		 	8192
#define O			16384
#define P			32768

#define Q			65536
#define R			131072
#define S			262144
#define T			524288
#define U			1048576
#define V			2097152
#define W			4194304
#define X			8388608

#define Y			16777216
#define Z			33554432
#define aa			67108864 	/* doubled due to conflicts */
#define bb			134217728
#define cc			268435456    
#define dd			536870912
#define ee			1073741824

/*
 * CLOAK_ bits for character mask.
 */
#define CLOAK_WEAR		(A)
#define CLOAK_REMOVE		(B)

/*
 * HOOD_ bits for character mask.
 */
#define HOOD_RAISED		(A)
#define HOOD_REMOVED		(B)


/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		(A)		/* Auto set for mobs	*/
#define ACT_SENTINEL	    	(B)		/* Stays in one room	*/
#define ACT_SCAVENGER	      	(C)		/* Picks up objects	*/
#define ACT_FORGER              (D)             /* Forger set           */
#define ACT_BOUNTY              (E)             /* Bounty set           */
#define ACT_AGGRESSIVE		(F)    		/* Attacks PC's		*/
#define ACT_STAY_AREA		(G)		/* Won't leave area	*/
#define ACT_WIMPY		(H)
#define ACT_PET			(I)		/* Auto set for pets	*/
#define ACT_TRAIN		(J)		/* Can train PC's	*/
#define ACT_PRACTICE		(K)		/* Can practice PC's	*/
#define ACT_MOUNT		(L)
#define ACT_UNDEAD		(O)	
#define ACT_CLERIC		(Q)
#define ACT_MAGE		(R)
#define ACT_THIEF		(S)
#define ACT_WARRIOR		(T)
#define ACT_NOALIGN		(U)
#define ACT_NOPURGE		(V)
#define ACT_OUTDOORS		(W)
#define ACT_INDOORS		(Y)
#define ACT_IS_HEALER		(aa)
#define ACT_GAIN		(bb)
#define ACT_UPDATE_ALWAYS	(cc)
#define ACT_IS_CHANGER		(dd)
#define ACT_LEVELER             (ee)

/* damage classes */
#define DAM_NONE                0
#define DAM_BASH                1
#define DAM_PIERCE              2
#define DAM_SLASH               3
#define DAM_FIRE                4
#define DAM_COLD                5
#define DAM_LIGHTNING           6
#define DAM_ACID                7
#define DAM_POISON              8
#define DAM_NEGATIVE            9
#define DAM_HOLY                10
#define DAM_ENERGY              11
#define DAM_MENTAL              12
#define DAM_DISEASE             13
#define DAM_DROWNING            14
#define DAM_LIGHT		15
#define DAM_OTHER               16
#define DAM_HARM		17
#define DAM_CHARM		18
#define DAM_SOUND		19
#define DAM_SILVER              20
#define DAM_WIND                21

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK         (A)
#define OFF_BACKSTAB            (B)
#define OFF_BASH                (C)
#define OFF_BERSERK             (D)
#define OFF_DISARM              (E)
#define OFF_DODGE               (F)
#define OFF_FADE                (G)
#define OFF_FAST                (H)
#define OFF_KICK                (I)
#define OFF_KICK_DIRT           (J)
#define OFF_PARRY               (K)
#define OFF_RESCUE              (L)
#define OFF_TAIL                (M)
#define OFF_TRIP                (N)
#define OFF_CRUSH		(O)
#define OFF_BLOODLUST           (V)
#define ASSIST_ALL       	(P)
#define ASSIST_ALIGN	        (Q)
#define ASSIST_RACE    	     	(R)
#define ASSIST_PLAYERS      	(S)
#define ASSIST_GUARD        	(T)
#define ASSIST_VNUM		(U)
#define OFF_FEED                (V)

/* return values for check_imm */
#define IS_NORMAL		0
#define IS_IMMUNE		1
#define IS_RESISTANT		2
#define IS_VULNERABLE		3

/* IMM bits for mobs */
#define IMM_SUMMON              (A)
#define IMM_CHARM               (B)
#define IMM_MAGIC               (C)
#define IMM_WEAPON              (D)
#define IMM_BASH                (E)
#define IMM_PIERCE              (F)
#define IMM_SLASH               (G)
#define IMM_FIRE                (H)
#define IMM_COLD                (I)
#define IMM_LIGHTNING           (J)
#define IMM_ACID                (K)
#define IMM_POISON              (L)
#define IMM_NEGATIVE            (M)
#define IMM_HOLY                (N)
#define IMM_ENERGY              (O)
#define IMM_MENTAL              (P)
#define IMM_DISEASE             (Q)
#define IMM_DROWNING            (R)
#define IMM_LIGHT		(S)
#define IMM_SOUND		(T)
#define IMM_WOOD                (X)
#define IMM_SILVER              (Y)
#define IMM_IRON                (Z)
 
/* RES bits for mobs */
#define RES_SUMMON		(A)
#define RES_CHARM		(B)
#define RES_MAGIC               (C)
#define RES_WEAPON              (D)
#define RES_BASH                (E)
#define RES_PIERCE              (F)
#define RES_SLASH               (G)
#define RES_FIRE                (H)
#define RES_COLD                (I)
#define RES_LIGHTNING           (J)
#define RES_ACID                (K)
#define RES_POISON              (L)
#define RES_NEGATIVE            (M)
#define RES_HOLY                (N)
#define RES_ENERGY              (O)
#define RES_MENTAL              (P)
#define RES_DISEASE             (Q)
#define RES_DROWNING            (R)
#define RES_LIGHT		(S)
#define RES_SOUND		(T)
#define RES_WOOD                (X)
#define RES_SILVER              (Y)
#define RES_IRON                (Z)
 
/* VULN bits for mobs */
#define VULN_SUMMON		(A)
#define VULN_CHARM		(B)
#define VULN_MAGIC              (C)
#define VULN_WEAPON             (D)
#define VULN_BASH               (E)
#define VULN_PIERCE             (F)
#define VULN_SLASH              (G)
#define VULN_FIRE               (H)
#define VULN_COLD               (I)
#define VULN_LIGHTNING          (J)
#define VULN_ACID               (K)
#define VULN_POISON             (L)
#define VULN_NEGATIVE           (M)
#define VULN_HOLY               (N)
#define VULN_ENERGY             (O)
#define VULN_MENTAL             (P)
#define VULN_DISEASE            (Q)
#define VULN_DROWNING           (R)
#define VULN_LIGHT		(S)
#define VULN_SOUND		(T)
#define VULN_WOOD               (X)
#define VULN_SILVER             (Y)
#define VULN_IRON		(Z)
#define VULN_STEEL              (aa)
#define VULN_MITHRIL            (bb)
#define VULN_ADAMANTITE         (cc)
 
/* body form */
#define FORM_EDIBLE             (A)
#define FORM_POISON             (B)
#define FORM_MAGICAL            (C)
#define FORM_INSTANT_DECAY      (D)
#define FORM_OTHER              (E)  /* defined by material bit */
 
/* actual form */
#define FORM_ANIMAL             (G)
#define FORM_SENTIENT           (H)
#define FORM_UNDEAD             (I)
#define FORM_CONSTRUCT          (J)
#define FORM_MIST               (K)
#define FORM_INTANGIBLE         (L)
 
#define FORM_BIPED              (M)
#define FORM_CENTAUR            (N)
#define FORM_INSECT             (O)
#define FORM_SPIDER             (P)
#define FORM_CRUSTACEAN         (Q)
#define FORM_WORM               (R)
#define FORM_BLOB		(S)
 
#define FORM_MAMMAL             (V)
#define FORM_BIRD               (W)
#define FORM_REPTILE            (X)
#define FORM_SNAKE              (Y)
#define FORM_DRAGON             (Z)
#define FORM_AMPHIBIAN          (aa)
#define FORM_FISH               (bb)
#define FORM_COLD_BLOOD		(cc)	
 
/* body parts */
#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE		(K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
/* for combat */
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS		(Y)


/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		(A)
#define AFF_INVISIBLE		(B)
#define AFF_DETECT_EVIL		(C)
#define AFF_DETECT_INVIS	(D)
#define AFF_DETECT_MAGIC	(E)
#define AFF_DETECT_HIDDEN	(F)
#define AFF_DETECT_GOOD		(G)
#define AFF_SANCTUARY		(H)
#define AFF_FAERIE_FIRE		(I)
#define AFF_INFRARED		(J)
#define AFF_CURSE		(K)
#define AFF_UNUSED_FLAG		(L)	/* unused */
#define AFF_POISON		(M)
#define AFF_PROTECT_EVIL	(N)
#define AFF_PROTECT_GOOD	(O)
#define AFF_SNEAK		(P)
#define AFF_HIDE		(Q)
#define AFF_SLEEP		(R)
#define AFF_CHARM		(S)
#define AFF_FLYING		(T)
#define AFF_PASS_DOOR		(U)
#define AFF_HASTE		(V)
#define AFF_CALM		(W)
#define AFF_PLAGUE		(X)
#define AFF_WEAKEN		(Y)
#define AFF_DARK_VISION		(Z)
#define AFF_BERSERK		(aa)
#define AFF_SWIM		(bb)
#define AFF_REGENERATION        (cc)
#define AFF_SLOW		(dd)
#define AFF_VICEGRIP            (ee)

/*
 * Bits for 'affected2_by'.
 * Used in #MOBILES.
 */
#define AFF_STEEL_SKIN          (A)
#define AFF_TURNING             (B)
#define AFF_BLOODLUST           (C)
#define AFF_DIVINE_PROTECTION   (D)
#define AFF_FORCE_SHIELD        (E)
#define AFF_STATIC_SHIELD       (F)
#define AFF_FLAME_SHIELD        (G)
#define AFF_LIFEFORCE           (H)
#define AFF_DARKSHIELD		(I)
#define AFF_MIRROR_SHIELD       (J)

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2

// Stances by Majik
#define STANCE_NEUTRAL  0
#define STANCE_AGG      1
#define STANCE_DEF      2

/* AC types */
#define AC_PIERCE			0
#define AC_BASH				1
#define AC_SLASH			2
#define AC_EXOTIC			3

/* dice */
#define DICE_NUMBER			0
#define DICE_TYPE			1
#define DICE_BONUS			2

/* size */
#define SIZE_TINY			0
#define SIZE_SMALL			1
#define SIZE_MEDIUM			2
#define SIZE_LARGE			3
#define SIZE_HUGE			4
#define SIZE_GIANT			5
#define SIZE_UNKNOWN                    6


/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		      1
#define ITEM_SCROLL		      2
#define ITEM_WAND		      3
#define ITEM_STAFF		      4
#define ITEM_WEAPON		      5
#define ITEM_TREASURE		      8
#define ITEM_ARMOR		      9
#define ITEM_POTION		     10
#define ITEM_CLOTHING		     11
#define ITEM_FURNITURE		     12
#define ITEM_TRASH		     13
#define ITEM_CONTAINER		     15
#define ITEM_DRINK_CON		     17
#define ITEM_KEY		     18
#define ITEM_FOOD		     19
#define ITEM_MONEY		     20
#define ITEM_BOAT		     22
#define ITEM_CORPSE_NPC		     23
#define ITEM_CORPSE_PC		     24
#define ITEM_FOUNTAIN		     25
#define ITEM_PILL		     26
#define ITEM_PROTECT		     27
#define ITEM_MAP		     28
#define ITEM_PORTAL		     29
#define ITEM_WARP_STONE		     30
#define ITEM_ROOM_KEY		     31
#define ITEM_GEM		     32
#define ITEM_JEWELRY		     33
#define ITEM_JUKEBOX		     34
#define ITEM_TOKEN                   35
#define ITEM_SOURCE                  36
#define ITEM_INSTRUMENT              37
#define ITEM_SOCKET                  38
#define ITEM_QUIVER                  39
#define ITEM_ARROW	             40
#define ITEM_BELT		     41
#define ITEM_SHEATH		     42
#define ITEM_SKELETON                43
#define ITEM_ORE		     44
#define ITEM_SOCKETS                 45

/*
 * Material types
 */
#define	MAT_STEEL	1
#define	MAT_STONE	2
#define	MAT_BRASS	3
#define	MAT_BONE	4
#define	MAT_ENERGY	5
#define	MAT_MITHRIL	6
#define	MAT_COPPER	7
#define	MAT_SILK	8
#define	MAT_MARBLE	9
#define	MAT_GLASS	10
#define	MAT_WATER	11
#define	MAT_FLESH	12
#define	MAT_PLATINUM	13
#define	MAT_GRANITE	14
#define	MAT_LEATHER	15
#define	MAT_CLOTH	16
#define	MAT_GEMSTONE	17
#define	MAT_GOLD	18
#define	MAT_PORCELAIN	19
#define	MAT_OBSIDIAN	20
#define	MAT_DRAGONSCALE	21
#define	MAT_EBONY	22
#define	MAT_BRONZE	23
#define	MAT_WOOD	24
#define	MAT_SILVER	25
#define	MAT_IRON	26
#define	MAT_BLOODSTONE	27
#define	MAT_FOOD	28
#define	MAT_LEAD	29
#define	MAT_WAX		30
#define MAT_DIAMOND     31
#define MAT_CRYSTAL     32
#define MAT_VELLUM      33
#define MAT_PAPER       34
#define MAT_MEAT        35
#define MAT_PILL        36
#define MAT_LIQUID      37
#define MAT_ADAMANTITE  38
#define MAT_UNIQUE      39
#define MAT_UNKNOWN     40


/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		(A)
#define ITEM_HUM		(B)
#define ITEM_DARK		(C)
#define ITEM_LOCK		(D)
#define ITEM_EVIL		(E)
#define ITEM_INVIS		(F)
#define ITEM_MAGIC		(G)
#define ITEM_NODROP		(H)
#define ITEM_BLESS		(I)
#define ITEM_ANTI_GOOD		(J)
#define ITEM_ANTI_EVIL		(K)
#define ITEM_ANTI_NEUTRAL	(L)
#define ITEM_NOREMOVE		(M)
#define ITEM_INVENTORY		(N)
#define ITEM_NOPURGE		(O)
#define ITEM_ROT_DEATH		(P)
#define ITEM_VIS_DEATH		(Q)
#define ITEM_NOREPAIR           (R)
#define ITEM_NONMETAL		(S)
#define ITEM_NOLOCATE		(T)
#define ITEM_MELT_DROP		(U)
#define ITEM_HAD_TIMER		(V)
#define ITEM_SELL_EXTRACT	(W)
#define ITEM_QUEST              (X)
#define ITEM_BURN_PROOF		(Y)
#define ITEM_NOUNCURSE		(Z)
#define ITEM_NO_AUCTION         (aa)
#define ITEM_AUCTIONED          (bb)
#define ITEM_HIDDEN             (cc)
#define ITEM_LODGED             (dd)
#define ITEM_NOCOND             (ee)

/*
 * Second set of extra2 flags
 * Used in #OBJECTS 
 */
#define ITEM_UNIQUE             (A)
#define CORPSE_NO_ANIMATE       (B)
#define ITEM_SANC		(C)
#define ITEM_RELIC              (D)
#define ITEM_INLAY1             (E)
#define ITEM_INLAY2             (F)

/*
 * Wear flags.
 * Used in #OBJECTS.
 */

#define ITEM_TAKE		(A)
#define ITEM_WEAR_FINGER	(B)
#define ITEM_WEAR_NECK		(C)
#define ITEM_WEAR_BODY		(D)
#define ITEM_WEAR_HEAD		(E)
#define ITEM_WEAR_LEGS		(F)
#define ITEM_WEAR_FEET		(G)
#define ITEM_WEAR_HANDS		(H)
#define ITEM_WEAR_ARMS		(I)
#define ITEM_WEAR_SHIELD	(J)
#define ITEM_WEAR_ABOUT		(K)
#define ITEM_WEAR_WAIST		(L)
#define ITEM_WEAR_WRIST		(M)
#define ITEM_WIELD		(N)
#define ITEM_HOLD		(O)
#define ITEM_NO_SAC		(P)
#define ITEM_WEAR_FLOAT		(Q)
#define ITEM_WEAR_SHOULDER      (R)
#define ITEM_WEAR_CLOAK		(S)
#define ITEM_LAYER_ARMS		(T)
#define ITEM_LAYER_BODY		(U)
#define ITEM_LAYER_LEGS		(V)
#define ITEM_WEAR_BACK		(W)
#define ITEM_WEAR_HOOD		(X)
#define ITEM_WEAR_WEDDING       (Y)

/* weapon class */
#define WEAPON_EXOTIC		0
#define WEAPON_SWORD		1
#define WEAPON_DAGGER		2
#define WEAPON_SPEAR		3
#define WEAPON_MACE		4
#define WEAPON_AXE		5
#define WEAPON_FLAIL		6
#define WEAPON_WHIP		7	
#define WEAPON_POLEARM		8
#define WEAPON_BOW              9

/* weapon types */
#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_VAMPIRIC		(C)
#define WEAPON_MANA_DRAIN       (D)
#define WEAPON_VORPAL		(E)
#define WEAPON_TWO_HANDS	(F)
#define WEAPON_SHOCKING		(G)
#define WEAPON_POISON		(H)
#define WEAPON_SHARP		(I)
#define WEAPON_ACIDIC           (J)

/* gate flags */
#define GATE_NORMAL_EXIT	(A)
#define GATE_NOCURSE		(B)
#define GATE_GOWITH		(C)
#define GATE_BUGGY		(D)
#define GATE_RANDOM		(E)

/* furniture flags */
#define STAND_AT		(A)
#define STAND_ON		(B)
#define STAND_IN		(C)
#define SIT_AT			(D)
#define SIT_ON			(E)
#define SIT_IN			(F)
#define REST_AT			(G)
#define REST_ON			(H)
#define REST_IN			(I)
#define SLEEP_AT		(J)
#define SLEEP_ON		(K)
#define SLEEP_IN		(L)
#define PUT_AT			(M)
#define PUT_ON			(N)
#define PUT_IN			(O)
#define PUT_INSIDE		(P)




/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_INT		      3
#define APPLY_WIS		      4
#define APPLY_CON		      5
#define APPLY_SEX		      6
#define APPLY_CLASS		      7
#define APPLY_LEVEL		      8
#define APPLY_AGE		      9
#define APPLY_HEIGHT		     10
#define APPLY_WEIGHT		     11
#define APPLY_MANA		     12
#define APPLY_HIT		     13
#define APPLY_MOVE		     14
#define APPLY_GOLD		     15
#define APPLY_EXP		     16
#define APPLY_AC		     17
#define APPLY_HITROLL		     18
#define APPLY_DAMROLL		     19
#define APPLY_SAVES		     20
#define APPLY_SAVING_PARA	     20
#define APPLY_SAVING_ROD	     21
#define APPLY_SAVING_PETRI	     22
#define APPLY_SAVING_BREATH	     23
#define APPLY_SAVING_SPELL	     24
#define APPLY_SPELL_AFFECT	     25
#define APPLY_CAST_LEVEL             26
#define APPLY_CAST_ABILITY           27

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8
#define CONT_PUT_ON		     16

/* 
 * Values for socketing items (value[0]). 
 * Used in #OBJECTS. 
 */ 
#define SOC_SAPPHIRE         1 
#define SOC_RUBY             2
#define SOC_EMERALD          3
#define SOC_DIAMOND          4
#define SOC_TOPAZ            5
#define SOC_SKULL            6

/* 
 * Values for socketing items (value[1]). 
 * used in #OBJECTS 
 */ 
#define GEM_CHIPPED	  0 
#define GEM_FLAWED        1 
#define GEM_FLAWLESS      2 
#define GEM_PERFECT       3 

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		(A)
#define ROOM_DONATION           (B)
#define ROOM_NO_MOB		(C)
#define ROOM_INDOORS		(D)
#define ROOM_NOMAGIC            (E)
#define ROOM_MOUNT_SHOP		(F)
#define ROOM_BANK               (G)
#define ROOM_PRIVATE		(J)
#define ROOM_SAFE		(K)
#define ROOM_SOLITARY		(L)
#define ROOM_PET_SHOP		(M)
#define ROOM_NO_RECALL		(N)
#define ROOM_IMP_ONLY		(O)
#define ROOM_GODS_ONLY		(P)
#define ROOM_HEROES_ONLY	(Q)
#define ROOM_NEWBIES_ONLY	(R)
#define ROOM_LAW		(S)
#define ROOM_NOWHERE		(T)
#define ROOM_RENTAL             (U)
#define ROOM_RENTED             (W)
#define ROOM_CASINO             (X)
#define ROOM_ARENA              (Y)
#define ROOM_WAYPOINT           (Z)
#define ROOM_MAPPED		(aa)
#define ROOM_MINE		(bb)
#define ROOM_POSTAL             (cc)

/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5



/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		      (A)
#define EX_CLOSED		      (B)
#define EX_LOCKED		      (C)
#define EX_PICKPROOF		      (F)
#define EX_NOPASS		      (G)
#define EX_EASY			      (H)
#define EX_HARD			      (I)
#define EX_INFURIATING		      (J)
#define EX_NOCLOSE		      (K)
#define EX_NOLOCK		      (L)
#define EX_NOBASH                     (M)



/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		      0
#define SECT_CITY		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM		      6
#define SECT_WATER_NOSWIM	      7
#define SECT_UNUSED		      8
#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_FARM		     11
#define SECT_MAX		     12


/*
 * Sheath Strap locations.
 */
#define STRAP_NONE		-1
#define STRAP_THIGH		 0
#define STRAP_ANKLE		 1
#define STRAP_SHOULDER		 2
#define STRAP_WRIST_R		 3
#define STRAP_WRIST_L	 	 4
#define STRAP_MAX		 5

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */

#define WEAR_NONE		 			-1
#define WEAR_LIGHT					0
#define WEAR_HOOD					1
#define WEAR_HEAD					2
#define WEAR_CLOAK					3
#define WEAR_NECK_1					4
#define WEAR_NECK_2					5
#define WEAR_ABOUT					6
#define WEAR_BACK					7
#define WEAR_SHOULDER					8
#define WEAR_LAYER_ARMS					9
#define WEAR_ARMS					10
#define WEAR_WRIST_L					11
#define WEAR_WRIST_R					12
#define WEAR_HANDS					13
#define WEAR_FINGER_L					14
#define WEAR_FINGER_R					15
#define WEAR_WEDDING                                    16
#define WEAR_LAYER_BODY					17
#define WEAR_BODY					18
#define WEAR_WAIST					19
#define WEAR_LAYER_LEGS					20
#define WEAR_LEGS					21
#define WEAR_FEET					22
#define WEAR_SHIELD					23
#define WEAR_WIELD					24
#define WEAR_SECONDARY					25
#define WEAR_HOLD					26
#define WEAR_FLOAT					27
#define WEAR_LODGE_LEG			            	28
#define WEAR_LODGE_ARM  		                29
#define WEAR_LODGE_RIB             			30
#define MAX_WEAR					31

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2
#define COND_HUNGER		      3
#define COND_TIRED                    4


/*
 * Positions.
 */
#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
#define POS_STUNNED		      3
#define POS_SLEEPING		      4
#define POS_RESTING		      5
#define POS_SITTING		      6
#define POS_FIGHTING		      7
#define POS_STANDING		      8


/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		(A)		/* Don't EVER set.	*/

/* RT auto flags */
#define PLR_GQUEST              (B)
#define PLR_AUTOASSIST		(C)
#define PLR_AUTODAMAGE		(D)
#define PLR_AUTOEXIT		(E)
#define PLR_AUTOLOOT		(F)
#define PLR_AUTOSAC             (G)
#define PLR_AUTOGOLD		(H)
#define PLR_AUTOSPLIT		(I)
#define PLR_AUTOMAP             (S)

#define PLR_QUESTOR             (J)
#define PLR_REMORT              (K)
#define PLR_MORTAL_LEADER       (L)
#define PLR_FREQUENT            (M)
#define PLR_HOLYLIGHT		(N)
#define PLR_TIMEOUT		(O)
#define PLR_CANLOOT		(P)

/* RT personal flags */
#define PLR_NOSUMMON		(Q)
#define PLR_NOFOLLOW		(R)

#define PLR_COLOUR		(T)

/* 1 bit reserved, S */

/* penalty flags */
#define PLR_PERMIT		(U)
#define PLR_CONSENT             (V)
#define PLR_LOG			(W)
#define PLR_DENY		(X)
#define PLR_FREEZE		(Y)
#define PLR_THIEF		(Z)
#define PLR_KILLER		(aa)
#define PLR_IMPLAG              (bb)
#define PLR_TENNANT             (cc)
#define	PLR_WAR                 (dd)
#define PLR_AUTOWEATHER         (ee)

/* weight values */

#define WEIGHT_SLENDER 	0
#define WEIGHT_SLIM    	1
#define WEIGHT_AVERAGE 	2
#define WEIGHT_STOCKY  	3
#define WEIGHT_HEAVY   	4
#define MAX_WEIGHT      5
/* Eye color values */

#define EYE_BLUE  	0
#define EYE_GREEN 	1
#define EYE_BROWN 	2
#define EYE_BLACK 	3
#define EYE_WHITE 	4
#define EYE_CLEAR 	5
#define EYE_RED   	6
#define MAX_EYE		7

/* Height values */

#define HEIGHT_SHORT   0
#define HEIGHT_MIDGET  1
#define HEIGHT_AVERAGE 2
#define HEIGHT_TALL    3
#define HEIGHT_GIANT   4
#define MAX_HEIGHT     5

/* Hair color values */

#define HAIR_BLOND  0
#define HAIR_RED    1
#define HAIR_BLACK  2
#define HAIR_BROWN  3
#define HAIR_WHITE  4
#define HAIR_BLUE   5
#define MAX_HAIR    6

// Chan flags - for Aethaellyn's new channel system.
#define CHAN_OOC                0
#define CHAN_GOSSIP             1
#define CHAN_QUOTE              2
#define CHAN_MUSIC              3
#define CHAN_QA                 4
#define CHAN_AUCT               5
#define CHAN_GRATS              6
#define CHAN_IMM                7
#define CHAN_CLAN               8
#define CHAN_IMP		9                                       
#define CHAN_HERO		10
#define CHAN_NEWBIE             11
#define CHAN_ADMIN              12
#define CHAN_GOD		13

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (A)
#define COMM_DEAF            	(B)
#define COMM_NOWIZ              (C)
#define COMM_NOAUCTION          (D)
#define COMM_NOGOSSIP           (E)
#define COMM_NOQUESTION         (F)
#define COMM_NOMUSIC            (G)
#define COMM_NOCLAN		(H)
#define COMM_NOQUOTE		(I)
#define COMM_SHOUTSOFF		(J)
#define COMM_NOINFO             (K)

/* display flags */
#define COMM_COMPACT		(L)
#define COMM_BRIEF		(M)
#define COMM_PROMPT		(N)
#define COMM_COMBINE		(O)
#define COMM_TELNET_GA		(P)
#define COMM_SHOW_AFFECTS	(Q)
#define COMM_NOGRATS		(R)
#define COMM_NOENTER            (S)
#define COMM_NOPRAY             (aa)
#define COMM_NORACE             (bb)
#define COMM_NOHERO             (cc)
#define COMM_NOHINT             (dd)
#define COMM_PENTER             (ee)
#define COMM_NOOOC              (X)

/* penalties */
#define COMM_NOEMOTE		(T)
#define COMM_NOSHOUT		(U)
#define COMM_NOTELL		(V)
#define COMM_NOCHANNELS		(W) 
#define COMM_SNOOP_PROOF	(Y)
#define COMM_AFK		(Z)

/* More comm flags */
#define COMM_NOIMP              (A)
#define COMM_SHOW_ARMOR         (B)
#define COMM_NONEWBIE           (C)
#define COMM_AUTO_AFK           (D)
#define COMM_ANNOUNCE           (E)
#define COMM_AUTOLEVEL          (F)
#define COMM_PETLEVEL           (G)
#define COMM_NOADMIN            (H)
#define COMM_NOQUEST            (I)

/* WIZnet flags */
#define WIZ_ON			(A)
#define WIZ_TICKS		(B)
#define WIZ_LOGINS		(C)
#define WIZ_SITES		(D)
#define WIZ_LINKS		(E)
#define WIZ_DEATHS		(F)
#define WIZ_RESETS		(G)
#define WIZ_MOBDEATHS		(H)
#define WIZ_FLAGS		(I)
#define WIZ_PENALTIES		(J)
#define WIZ_SACCING		(K)
#define WIZ_LEVELS		(L)
#define WIZ_SECURE		(M)
#define WIZ_SWITCHES		(N)
#define WIZ_SNOOPS		(O)
#define WIZ_RESTORE		(P)
#define WIZ_LOAD		(Q)
#define WIZ_NEWBIE		(R)
#define WIZ_PREFIX		(S)
#define WIZ_SPAM		(T)
#define WIZ_OLC                 (U)

#define SUPPRESS_FLAGS          (A)
#define SUPPRESS_FLAG_EFFECTS   (B)
#define SUPPRESS_DODGE          (C)
#define SUPPRESS_STD            (D)
#define SUPPRESS_BURN           (E)
#define SUPPRESS_SHIELD         (F)
#define SUPPRESS_SHIELD_COMP    (G)          

typedef struct guild_rank       rankdata;

struct guild_rank
{
  char *rankname;               /* name of rank                         */
  char *skillname;              /* name of skill earned at this rank    */
};

/* mortal leader rights are as follows;
 ml[0] = can_guild
 ml[1] = can_deguild
 ml[2] = can_promote
 ml[3] = can_demote
*/

struct member_data
{
  MEMBER_DATA *next;
  char *name;
  int rank;
  int level;
  int flevel;
  bool valid;
  bool ldr;
  int laston;
};

struct clan_type
{
  MEMBER_DATA * member_first;
  long flags;                   /* flags for guild                      */
  char *name;			/* name of guild                        */
  char *who_name;		/* name sent for "who" command		*/
  int room[3];                  /* hall/morgue/temple rooms             */
  rankdata rank[MAX_RANK];	/* guilds rank system                   */
  int ml[4];			/* mortal leader rights 		*/
};

#define	WAR_OFF     0
#define	WAR_WAITING 1
#define	WAR_RUNNING 2

struct war_data
{
    char *who;
    int min_level;
    int max_level;
    int inwar;
    int wartype;
    int timer;
    int iswar;
    int next;
};

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    AREA_DATA *		area;		/* OLC */
    PROG_LIST *         mprogs;
    MOB_INDEX_DATA *	next;
    MOB_INDEX_DATA *    in_room;
    SPEC_FUN *		spec_fun;
    SHOP_DATA *		pShop;
    sh_int		vnum;
    sh_int		group;
    bool		new_format;
    sh_int		count;
    sh_int		killed;
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    long		act;
    long		affected_by;
    sh_int		alignment;
    sh_int		level;
    sh_int		hitroll;
    int                 aggression;
    long		hit[3];
    long		mana[3];
    sh_int		damage[3];
    sh_int		ac[4];
    sh_int 		dam_type;
    sh_int              cast_level;
    sh_int              cast_ability;
    sh_int              kills;
    long		off_flags;
    long		imm_flags;
    long		res_flags;
    long		vuln_flags;
    sh_int		start_pos;
    sh_int		default_pos;
    sh_int		sex;
    sh_int		race;
    long		wealth;
    long		form;
    long		parts;
    sh_int		size;
    char *		material;
    long                mprog_flags;
    long                affected2_by;
    int                 xp_tolevel;
};



/* memory settings */
#define MEM_CUSTOMER	A	
#define MEM_SELLER	B
#define MEM_HOSTILE	C
#define MEM_AFRAID	D

/* memory for mobs */
struct mem_data
{
    MEM_DATA 	*next;
    bool	valid;
    int		id; 	
    int 	reaction;
    time_t 	when;
};
   

#include "board.h"
#include "buffer.h"


/*
 * One character (PC or NPC).
 */
struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    CHAR_DATA *		fighting;
    CHAR_DATA *		reply;
    CHAR_DATA *		pet;
    CHAR_DATA *         mprog_target;
    MEM_DATA *		memory;
    SPEC_FUN *		spec_fun;
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	affected;
/*    NOTE_DATA *		pnote; */
    OBJ_DATA *		carrying;
    OBJ_DATA *		on;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room;
    AREA_DATA *		zone;
    PC_DATA *		pcdata;
    GEN_DATA *		gen_data;
    int			mineral [MAX_MINERALS];
    int                 aggression;
    bool		valid;
    char *		name;
    long		id;
    sh_int		version;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    char *		prompt;
    char *		prefix;
    sh_int		group;
    sh_int		clan;
    sh_int              rank;
    sh_int		sex;
    sh_int              stance;
    sh_int              stance_ac_bonus;
    sh_int              stance_hit_bonus;    
    sh_int              klass[MAX_MCLASS];
    sh_int		race;
    sh_int		level;
    sh_int		trust;
    int			played;
    int			lines;  /* for the pager */
    int                 subtype;
    int                 gpoint;
    time_t		logon;
    time_t              llogoff;
    sh_int		timer;
    sh_int		wait;
    sh_int		daze;
    long		hit;
    long  	        max_hit;
    long		mana;
    long		max_mana;
    long		move;
    long		max_move;
    sh_int              cast_level;
    sh_int              cast_ability;
    long		gold;
    long		silver;
    int			exp;
    long		act;
    long		comm;   /* RT added to pad the vector */
    long                comm2;
    long		wiznet; /* wiz stuff */
    long		imm_flags;
    long		res_flags;
    long		vuln_flags;
    sh_int		invis_level;
    sh_int		incog_level;
    long		affected_by;
    long                affected2_by;
    sh_int		position;
    sh_int		practice;
    sh_int		train;
    sh_int		carry_weight;
    sh_int		carry_number;
    sh_int		saving_throw;
    sh_int		alignment;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		armor[4];
    sh_int		wimpy;
    /* stats */
    sh_int		perm_stat[MAX_STATS];
    sh_int		mod_stat[MAX_STATS];
    /* parts stuff */
    long		form;
    long		parts;
    sh_int		size;
    char*		material;
    /* mobile stuff */
    long		off_flags;
    sh_int		damage[3];
    sh_int		dam_type;
    sh_int		start_pos;
    sh_int		default_pos;
    CHAR_DATA *		mount;
    bool		riding;    
    sh_int              hometown;
    sh_int              mprog_delay;
    /* cloak stuff */
    long		cloak;
    long		mask;
    int                 bp_points;
    int                 xp_tolevel;
    //Lycanthropy
    int 		bites;
    int 		bloodthirst;
    bool 		isvamp;
    bool		iswere;
    int			wereform;
    bool		turned;
    sh_int              wpoint[MAX_WAYPOINT];
    int                 fquit;
    int                 levelflux;
    char *              powner;
};

#define CHOSE_RACE 0
#define CHOSE_CLASS 1
#define CHOSE_GENDER 2
#define CHOSE_HOMETOWN 3
#define CHOSE_GOD 4
#define CHOSE_SHORT 5
#define CHOSE_ATTRIBUTES 6
#define MAX_CBOOL 7


//Exploration holder.
//Basically just a linked list of int's where we only use the bits.
typedef struct explore_holder
{       struct explore_holder *next;
        unsigned int bits;
        int mask;
} EXPLORE_HOLDER;
typedef struct exploration_data
{       EXPLORE_HOLDER *bits;
        int set;
} EXPLORE_DATA;



/*
 * Data which only PC's have.
 */
struct	pc_data
{
    int 		security;	/* OLC */ /* Builder security */
    PC_DATA *		next;
    BUFFER * 		buffer;
    COLOUR_DATA *	code;		/* Data for coloUr configuration	*/
    NICK_DATA 	*	nick_first;
    MINE_DATA   *    	in_mine;
    SHAFT_DATA  *	in_shaft;
    RELIGION	*	religion;
    TRADE_DATA  *       trade;
    sh_int		rank;
    bool		valid;
    char *		pwd;
    char *		bamfin;
    char *		bamfout;
    char *		title;
    char *		title2;
    char *              host;
    char *              spouse;
    char *              mplaying;
    long		perm_hit;
    long		perm_mana;
    long		perm_move;
    sh_int		true_sex;
    int                 weight;
    int                 height;
    int                 hair;
    int                 eye;
    sh_int              track_count;
    sh_int              nextquest;
    sh_int              countdown;
    int                 questobj;
    int                 questmob;
    int                 questgiver;
    int                 questloc;
    int                 rentroom;
    sh_int              questpoints;
    sh_int              qps;
    sh_int              trivia;
    sh_int              totaltpoints;
    CHAR_DATA *         tracking;
    int			last_level;
    long		current_exp;
    sh_int		condition	[5];
    sh_int		*learned	;
    bool		group_known	[MAX_GROUP];
    sh_int		points;
    bool              	confirm_delete;
    char *		alias[MAX_ALIAS];
    char * 		alias_sub[MAX_ALIAS];
    bool                confirm_remort;
    bool                stay_race;
    long		gold_bank;
    long		silver_bank;
    sh_int		shares;
    sh_int		share_level;
    sh_int     		shares_bought;
    sh_int		duration;
    int                 kills_mob;
    int                 kills_pc;
    int                 deaths_mob;
    int                 deaths_pc;
    BOARD_DATA *        board;
    time_t              last_note[MAX_BOARD]; 
    NOTE_DATA *         in_progress;
    int		idle_time;
    /*
     * Colour data stuff for config.
     */
    int			text[3];		/* {t */
    int			auction[3];		/* {a */
    int			auction_text[3];	/* {A */
    int			gossip[3];		/* {d */
    int			gossip_text[3];		/* {9 */
    int			music[3];		/* {e */
    int			music_text[3];		/* {E */
    int			question[3];		/* {q */
    int			question_text[3];	/* {Q */
    int			answer[3];		/* {f */
    int			answer_text[3];		/* {F */
    int			quote[3];		/* {h */
    int			quote_text[3];		/* {H */
    int			immtalk_text[3];	/* {i */
    int			immtalk_type[3];	/* {I */
    int			info[3];		/* {j */
    int			say[3];			/* {6 */
    int			say_text[3];		/* {7 */
    int			tell[3];		/* {k */
    int			tell_text[3];		/* {K */
    int			reply[3];		/* {l */
    int			reply_text[3];		/* {L */
    int			gtell_text[3];		/* {n */
    int			gtell_type[3];		/* {N */
    int			wiznet[3];		/* {B */
    int			room_title[3];		/* {s */
    int			room_text[3];		/* {S */
    int			room_exits[3];		/* {o */
    int			room_things[3];		/* {O */
    int			prompt[3];		/* {p */
    int			fight_death[3];		/* {1 */
    int			fight_yhit[3];		/* {2 */
    int			fight_ohit[3];		/* {3 */
    int			fight_thit[3];		/* {4 */
    int			fight_skill[3];		/* {5 */
    sh_int              mvolume;
    sh_int              svolume;
    sh_int              timeout;  
    long                gq_mobs[MAX_GQUEST_MOB];
    OBJ_DATA *          box;
    bool                boxrented;
    sh_int              item_box;
    char *              immtitle;
    //Creation boolines
    bool		creation[MAX_CBOOL];
    char *		email;
    long                gamestat[MAX_GAMESTAT];
    //Mine locations
    int 		cordinate[3];
    //Idle thingy.
    long		last_typed;
    char    *		pretitle;
    sh_int              dblq;
    //Stuff for religion;
    int			bless;
    int                 suppress;
    int                 h_vnum;
    int                 hinvite;
    sh_int              horesets;
    sh_int              hmresets;
    bool                hchange;        
    char                *afk;
    sh_int              bcount;
    int                 mailid;
    int                 hdam;
    int                 mdam;
    int                 msp;
    int                 mxp;
    int detail_map_x;
    int detail_map_y;
    EXPLORE_DATA *explored;
    ROOM_INDEX_DATA *map_point;
};

#define CORD_X 	0
#define CORD_Y  1
#define CORD_Z  2


/* Data for generating characters -- only used during generation */
struct gen_data
{
    GEN_DATA	*next;
    bool	valid;
    bool	*skill_chosen;
    bool	group_chosen[MAX_GROUP];
    int		points_chosen;
};



/*
 * Liquids.
 */
#define LIQ_WATER        0

struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    sh_int	liq_affect[5];
};



/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                     */
    bool valid;
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    AREA_DATA *		area;	/* OLC */
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    bool		new_format;
    char *		name;
    char *		short_descr;
    char *		description;
    sh_int		vnum;
    sh_int		reset_num;
    char *		material;
    sh_int		item_type;
    int			extra_flags;
    int                 extra2_flags;
    int			wear_flags;
    sh_int		level;
    sh_int 		condition;
    sh_int		count;
    sh_int		weight;
    sh_int              size;
    int			cost;
    int			value[5];
    int                 valueorig[5];
    PROG_LIST *		oprogs;
    long		oprog_flags;
    sh_int              plevel;
    int                 xp_tolevel;
    int                 exp;
};



/*
 * One object.
 */
struct	obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		next_content;
    OBJ_DATA *		contains;
    OBJ_DATA *		in_obj;
    OBJ_DATA *		on;
    CHAR_DATA *		carried_by;
    CHAR_DATA *	oprog_target;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    bool		valid;
    bool		enchanted;
    char *	        owner;
    char *		name;
    char *		short_descr;
    char *		description;
    sh_int		item_type;
    int			extra_flags;
    int                 extra2_flags;
    int			wear_flags;
    sh_int		wear_loc;
    sh_int		weight;
    int			cost;
    sh_int		level;
    sh_int 		condition;
    char *		material;
    sh_int		timer;
    int			value	[5];
    int                 valueorig [5];
    sh_int		oprog_delay;
    sh_int              size;
    int			belted_vnum;	// Vnum of the object this item is belted to.
    OBJ_DATA 		*sheathed_in;   // Vnum of the object that is sheathed.
    SHAFT_DATA	* 	in_shaft;
    int			strap_loc;    //Strap location.
    bool		part;
    int                 exp;
    sh_int              plevel;
    int                 xp_tolevel;
};

/*
 * Exit data.
 */
struct	exit_data
{
    EXIT_DATA *		next;		/* OLC */
    int			rs_flags;	/* OLC */
    int			orig_door;	/* OLC */
    MINE_DATA *		mine;
    SHAFT_DATA  *       to_shaft;
    int 		cord[3];
    union
    {
	
        ROOM_INDEX_DATA *	to_room;
	sh_int			vnum;
    } u1;
    sh_int		exit_info;
    sh_int		key;
    char *		keyword;
    char *		description;
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    RESET_DATA *	prev;
    char		command;
    sh_int		arg1;
    sh_int		arg2;
    sh_int		arg3;
    sh_int		arg4;
};


/* New area_data for OLC */
struct	area_data
{
    AREA_DATA *		next;		/*
    AREA_DATA *		next_on_cont;
    RESET_DATA *	reset_first;     *  Removed for OLC reset system.
    RESET_DATA *	reset_last;	 */
    char *		name;
    char *		credits;
    int 		age;
    int 		nplayer;
    bool                empty;          /* ROM OLC */
    char *		file_name;	/* OLC */
    char *		builders;	/* OLC */ /* Listing of */
    char *		repop_msg;
    char *              soundfile;
    int			security;	/* OLC */ /* Value 1-9  */
    sh_int              min_vnum;
    sh_int              max_vnum;
    int			vnum;		/* OLC */ /* Area vnum  */
    int			area_flags;	/* OLC */
    int			continent;
    int			min_lev;
    int			max_lev;
    int                 min_temp;
    int                 max_temp;
    int                 temperature;
};

/*
 * Room type.
 */
struct	room_index_data
{
    RESET_DATA *	reset_first;	/* OLC */
    RESET_DATA *	reset_last;	/* OLC */
    RESET_DATA *	last_mob_reset;
    RESET_DATA *	last_obj_reset;
    ROOM_INDEX_DATA *	next;
    ROOM_INDEX_DATA *   map_next;
    CHAR_DATA *		people;
    OBJ_DATA *		contents;
    EXTRA_DESCR_DATA *	extra_descr;
    AREA_DATA *		area;
    EXIT_DATA *		exit	[6];
    ROOM_INDEX_DATA *   next_track;
    ROOM_INDEX_DATA *   track_came_from;
    int		 	mineral [MAX_MINERALS];
    char *		name;
    char *		description;
    char *		owner;
    sh_int		vnum;
    int			room_flags;
    sh_int		light;
    sh_int		sector_type;
    sh_int		heal_rate;
    sh_int 		mana_rate;
    sh_int		clan; 
    PROG_LIST *		rprogs;
    CHAR_DATA *		rprog_target;
    long		rprog_flags;
    sh_int		rprog_delay;
    AFFECT_DATA         *affected;
    long                affected_by;
    int                 moisture;
};


/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000
#define TYPE_MIRROR                  2000


// Mineral Types
#define MIN_STEEL		0
#define MIN_IRON		1
#define MIN_COPPER		2
#define MIN_ADAMANTIUM		3
#define MIN_PLATINUM		4
#define MIN_BRONZE		5
#define MIN_SILVER		6
#define MIN_MARBLE		7
#define MIN_BRASS		8
#define MIN_GRANITE		9
#define MIN_ELECTRUM		10

/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4
#define TAR_OBJ_CHAR_DEF	    5
#define TAR_OBJ_CHAR_OFF	    6

#define TARGET_CHAR		    0
#define TARGET_OBJ		    1
#define TARGET_ROOM		    2
#define TARGET_NONE		    3



/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
    char *	name;			/* Name of skill		*/
    sh_int	skill_level[MAX_CLASS];	/* Level needed by class	*/
    sh_int	rating[MAX_CLASS];	/* How hard it is to learn	*/	
    SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
    sh_int	target;			/* Legal targets		*/
    sh_int	minimum_position;	/* Position for caster / user	*/
    sh_int	slot;			/* Slot for #OBJECT loading	*/
    sh_int	min_mana;		/* Minimum mana used		*/
    sh_int	beats;			/* Waiting time after use	*/
    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
    char *	msg_obj;		/* Wear off message for obects	*/
};

struct  group_type
{
    char *	name;
    sh_int	rating[MAX_CLASS];
    char *	spells[MAX_IN_GROUP];
};


/*
 * MOBprog definitions
 */                   
#define TRIG_ACT	(A)
#define TRIG_BRIBE	(B)
#define TRIG_DEATH	(C)
#define TRIG_ENTRY	(D)
#define TRIG_FIGHT	(E)
#define TRIG_GIVE	(F)
#define TRIG_GREET	(G)
#define TRIG_GRALL	(H)
#define TRIG_KILL	(I)
#define TRIG_HPCNT	(J)
#define TRIG_RANDOM	(K)
#define TRIG_SPEECH	(L)
#define TRIG_EXIT	(M)
#define TRIG_EXALL	(N)
#define TRIG_DELAY	(O)
#define TRIG_SURR	(P)
#define TRIG_GET        (Q)
#define TRIG_DROP	(R)
#define TRIG_SIT	(S)
#define TRIG_ALIAS	(T)

/*
 * Prog types
 */
#define PRG_MPROG	0
#define PRG_OPROG	1
#define PRG_RPROG	2

struct prog_list
{
    int                 trig_type;
    char *              trig_phrase;
    sh_int              vnum;
    char *              code;
    PROG_LIST *         next;
    bool                valid;
};

struct prog_code
{
    sh_int              vnum;
    char *              code;
    PROG_CODE *         next;
}; 


/*
 * Utility macros.
 */

#define IS_VALID(data)		((data) != NULL && (data)->valid)
#define VALIDATE(data)		((data)->valid = TRUE)
#define INVALIDATE(data)	((data)->valid = FALSE)
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define CH(descriptor)  ((descriptor)->original ? \
(descriptor)->original : (descriptor)->character)     


/*
 * Character macros.
 */
#define IN_FACTION(ch)			( (ch)->pcdata->religion->isfaction )
#define HAS_RELIGION(ch)		( (ch)->pcdata->religion )
#define IS_SAME_RELIGION(ch, vct)	( (ch)->pcdata->religion == (vct)->pcdata->religion )
#define OBJ_IN_MINE(obj)		((obj)->in_shaft)
#define OBJ_SAME_SHAFT(ch, obj) 	 ( (ch)->pcdata->in_shaft == (obj)->in_shaft ) 
#define IS_SAME_SHAFT(ch, victim)        ( (ch)->pcdata->in_shaft == (victim)->pcdata->in_shaft )
#define IN_MINE(ch)        ( (ch)->pcdata->in_mine )
#define IS_WEREWOLF(ch)    ( (ch)->iswere && (ch)->wereform == MOON_FULL )
#define IS_VAMPIRE(ch)     ( (ch)->isvamp == TRUE )
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMP(ch)	        (get_trust(ch) == MAX_LEVEL)
#define IS_IMMORTAL(ch)		(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define IS_AFFECTED2(ch, sn)    (IS_SET((ch)->affected2_by, (sn)))
#define GET_AGE(ch)		((int) (17 + ((ch)->played \
				    + current_time - (ch)->logon )/72000))
#define IS_GOOD(ch)		(ch->alignment >= 350)
#define IS_EVIL(ch)		(ch->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))
#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define GET_AC(ch,type)        ((ch)->armor[type]                \
                + ch->stance_ac_bonus + ( IS_AWAKE(ch)                \
            ? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 )  - number_classes(ch))  
#define GET_HITROLL(ch)    \
        ((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit + ch->stance_hit_bonus + number_classes(ch))
#define GET_DAMROLL(ch) \
		((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam + number_classes(ch))
#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS))
#define ON_GQUEST(ch)           (!IS_NPC(ch) && IS_SET((ch)->act, PLR_GQUEST) && gquest_info.running != GQUEST_OFF)
#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))
#define get_carry_weight(ch)	((ch)->carry_weight + (ch)->silver/10 +  \
						      (ch)->gold * 2 / 5)
#define MOUNTED(ch) \
		((!IS_NPC(ch) && ch->mount && ch->riding) ? ch->mount : NULL)
#define RIDDEN(ch) \
		((IS_NPC(ch) && ch->mount && ch->riding) ? ch->mount : NULL)
#define IS_DRUNK(ch)		((ch->pcdata->condition[COND_DRUNK] > 10))
#define act(format,ch,arg1,arg2,type)\
	act_new((format),(ch),(arg1),(arg2),(type),POS_RESTING)
#define HAS_TRIGGER_MOB(ch,trig) (IS_SET((ch)->pIndexData->mprog_flags,(trig)))
#define HAS_TRIGGER_OBJ(obj,trig) (IS_SET((obj)->pIndexData->oprog_flags,(trig)))
#define HAS_TRIGGER_ROOM(room,trig) (IS_SET((room)->rprog_flags,(trig)))
#define IS_SWITCHED( ch )       ( ch->desc && ch->desc->original )
#define IS_BUILDER(ch, Area)	( !IS_NPC(ch) && !IS_SWITCHED( ch ) &&	  \
				( ch->pcdata->security >= Area->security  \
				|| strstr( Area->builders, ch->name )	  \
				|| strstr( Area->builders, "All" ) ) )
#define	IS_QUESTOR(ch)     (!IS_NPC(ch) && IS_SET((ch)->act, PLR_QUESTOR) && \
(ch)->pcdata->questgiver != 0 )
#define GET_NAME(Ch)    (IS_NPC(Ch) ? (Ch)->short_descr : (Ch)->name)
#define IS_REMORT(ch)      (!IS_NPC(ch) && ( IS_SET((ch)->act, PLR_REMORT) \
                                || number_classes(ch) > 1))   
#define IS_IN_WAR(ch)   (!IS_NPC(ch) && IS_SET((ch)->act, PLR_WAR) \
                         && IS_SET((ch)->in_room->room_flags, ROOM_ARENA))     
#define HAS_HOME(ch)    (!IS_NPC(ch) && ch->pcdata->h_vnum != 0 )
#define IS_HOME(ch)     ( ( ch->pcdata->h_vnum != 0 && (ch->in_room->vnum >= ch->pcdata->h_vnum) && (ch->in_room->vnum < ch->pcdata->h_vnum + 5 ) ) )                  
#define IS_MXP(ch)      ( ( ch->desc->mxp == TRUE ) )
#define IS_MSP(ch)      ( ( ch->desc->msp == TRUE ) )
#define IS_TRADE_LOCKED( _ch )	\
	( IS_SET( (_ch)->pcdata->trade->first.flags, TRADE_LOCKED ) \
	  || IS_SET( (_ch)->pcdata->trade->second.flags, TRADE_LOCKED ) )
#define GET_TRADE( _ch ) \
	( ( (_ch) == (_ch)->pcdata->trade->first.ch ) \
	  ? (_ch)->pcdata->trade->first \
	  : (_ch)->pcdata->trade->second )

/*
 * Object macros.
 */
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_OBJ2_STAT(obj, stat)  (IS_SET((obj)->extra2_flags, (stat))) 
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)	((obj)->item_type == ITEM_CONTAINER ? \
	(obj)->value[4] : 100)



/*
 * Description macros.
 */

/*
 * Olc macros.
 */
#define IS_NULLSTR(str)		((str) == NULL || (str)[0] == '\0')
#define ENTRE(min,num,max)	( ((min) < (num)) && ((num) < (max)) )
#define CHECK_POS(a, b, c)      {                                       \
                                (a) = (b);                              \
                                if ( (a) < 0 )                          \
                                bug( "CHECK_POS : " c " == %d < 0", a );\
                                }                                           
#define EDIT_GUILD(Ch, Clan)    ( Clan = (CLAN_DATA *)Ch->desc->pEdit )

/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    char *     name;
    char *    char_no_arg;
    char *    others_no_arg;
    char *    char_found;
    char *    others_found;
    char *    vict_found;
    char *    char_not_found;
    char *    char_auto;
    char *    others_auto;
    char *    chan_emote;
};

struct postal_type
{
    int       pcount;
    int       vnum;
};


/*
 * Global constants.
 */

extern	const	struct	str_app_type	str_app		[51];
extern	const	struct	int_app_type	int_app		[51];
extern	const	struct	wis_app_type	wis_app		[51];
extern	const	struct	dex_app_type	dex_app		[51];
extern	const	struct	con_app_type	con_app		[51];

extern	const	struct	class_type	class_table	[MAX_CLASS];
extern	const	struct	weapon_type	weapon_table	[];
extern  const   struct  item_type	item_table	[];
extern	const	struct	wiznet_type	wiznet_table	[];
extern	const	struct	attack_type	attack_table	[];
extern  const	struct  race_type	race_table	[];
extern	const	struct	pc_race_type	pc_race_table	[];
extern  const	struct	spec_type	spec_table	[];
extern	const	struct	liq_type	liq_table	[];
extern		struct	skill_type	*skill_table;
extern  const   struct  group_type      group_table	[MAX_GROUP];
extern  const   struct  eye_type        eye_table       [];
extern  const   struct  hair_type       hair_table      [];
extern  const   struct  height_type     height_table    [];
extern  const   struct  weight_type     weight_table    [];
extern          struct  social_type     *social_table;  
extern          struct  postal_type     *postal_table;
extern	const	struct	hometown_type	hometown_table	[];
extern  const   struct  song_type       bsong_table     [MAX_SONG];
extern  const   char    *osize_table                    [];
extern  const   char    *cond_table                     [];
extern  struct  unique_attrib_table     unique_table_prefix[];
extern  struct  unique_attrib_table     unique_table_weapon_suffix[];
extern  struct  unique_attrib_table     unique_table_armor_suffix[];
extern  const   char    *admin_table                    [];
extern  const   struct  smith_type      smith_table     [];
extern  const   struct  wind_description wind_table     [MAX_WIND];
extern  const   struct  breeze_description breeze_table [MAX_BREEZE];
extern  const   struct  rain_type rain_table     [MAX_RAIN];
extern  const   struct  hailstorm_description hailstorm_table [MAX_HAILSTORM];
extern  const   struct  cloud_type cloud_table   [MAX_CLOUD];
extern  const   struct  cloudy_description cloudy_table [MAX_CLOUDY];
extern  const   struct  lightning_description lightning_table [MAX_LIGHTNING];
extern  const   struct  thunderstorm_description thunderstorm_table [MAX_THUNDERSTORM];
extern  const   struct  foggy_description foggy_table [MAX_FOGGY];
extern  const   struct  blizzard_description blizzard_table [MAX_BLIZZARD];
extern  const   struct  snow_description snow_table [MAX_SNOW];
extern  const   struct  icestorm_description icestorm_table [MAX_ICESTORM];
extern  const   struct  gamble_item gamble_table [MAX_ITEM];   
extern  const   struct  house_item house_table []; 
extern  const   struct  weather_type weather_table [MAX_WEATHER];
extern  const   struct  summoner_data summoner_table [];

extern const   char    echo_off_str    [];
extern const   char    echo_on_str     [];
extern const   char    go_ahead_str    [];

extern const int          rgSizeList      []; 

/*
 * Global variables.
 */
extern 		int			first_boot_time;
extern 		int			last_crash_time;
extern		RELIGION	  *     religion_list;
extern		HELP_DATA	  *	help_first;
extern		SHOP_DATA	  *	shop_first;

extern		CHAR_DATA	  *	char_list;
extern		DESCRIPTOR_DATA   *	descriptor_list;
extern		OBJ_DATA	  *	object_list;
extern          BOUNTY_DATA       *     bounty_list;
extern          PET_DATA          *     pet_list;
extern          PROG_CODE         *     mprog_list;
extern          PROG_CODE         *     rprog_list;
extern          PROG_CODE         *     oprog_list;
extern		HOUSE_DATA	  *	house_list;;
extern          MAIL_DATA         *     mail_list;

extern		char			bug_buf		[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		KILL_DATA		kill_table	[];
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;
extern          NOTE_DATA         *     note_free;
extern          OBJ_DATA          *     obj_free;
extern          CHAR_DATA               *copyover_person;

extern          bool                    double_exp;
extern          bool                    double_qp;
extern          bool                    quad_damage;
extern          bool                    timed_copyover;
extern          bool                    treboot;
extern          bool                    tshutdown;
extern          bool                    quiet;
extern          int                     copyovert;
extern          int                     reboott;
extern          int                     shutdownt;
extern          int                     global_exp;
extern          int                     global_qp;     
extern		int     		global_quad;
extern          sh_int                  display;
extern          sh_int                  qpdisplay;
extern          sh_int                  quaddisplay;
extern          bool                    pshow;

extern          bool                    MOBtrigger;
extern          WAR_DATA                war_info;  
extern          sh_int                  count_connections;
extern          sh_int                  max_unique;
extern          bool                    unique_set;
extern          sh_int                  display;
extern          int                     port, control;
extern          bool                    isignal;
extern          TRIVIA_DATA       *     trivia;   

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
int	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(linux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(macintosh)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(MIPS_OS)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MSDOS)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(NeXT)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(sun)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
#if	defined(SYSV)
size_t	fread		args( ( void *ptr, size_t size, size_t n, 
			    FILE *stream) );
#elif !defined(__SVR4)
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
#endif
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(ultrix)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define PLAYER_DIR	""			/* Player files	*/
#define TEMP_FILE	"romtmp"
#define NULL_FILE	"proto.are"		/* To reserve one stream */
#endif

#if defined(MSDOS)
#define PLAYER_DIR	""			/* Player files */
#define TEMP_FILE	"romtmp"
#define NULL_FILE	"nul"			/* To reserve one stream */
#endif

#if defined(unix)
#define PLAYER_DIR      "../player/"        	/* Player files */
#define GOD_DIR         "../gods/"  		/* list of gods */
#define TEMP_FILE	"../player/romtmp"
#define NULL_FILE	"null.txt"		/* To reserve one stream */
#endif

#define RELG_DIR	"../data/religion/"
#define RELG_LIST	"../data/religion/religion.lst"
#define SKILL_DIR	"../data/skill/"
#define SKILLDAT	"skill.dat"
#define DATA_DIR	"../data/" /*Data dir */
#define AREA_LIST       "area.lst"  /* List of areas*/
#define BUG_FILE        "bugs.txt" /* For 'bug' and bug()*/
#define NEW_SAVE_TEST     "save_test.txt"/*testing new saves*/
#define TYPO_FILE       "typos.txt" /* For 'typo'*/
#define SHUTDOWN_FILE   "shutdown.txt"/* For 'shutdown'*/
#define BAN_FILE	"ban.txt"
#define MUSIC_FILE	"music.txt"
#define BOUNTY_FILE     "bounty.txt"
#define ILLEGAL_NAME_FILE	"illegalnames.txt"

#define QUEST_FILE      "quest.log"  /* for purchase logging. */
#define WIZ_FILE        "wizlist.txt"
#define GQUEST_FILE     "gquest.dat"
#define DATA_DIR        "../data/"
#define MAX_WHO_FILE    "../data/maxwho.txt"
#define NEW_AREA_DIR    "../new_area/"          /*new area*/
#define VERSION_FILE    "../data/version.txt"
#define COPYOVER_FILE   "copyover.data" /* copyover data */
#define EXE_FILE        "../src/shadow" /* executable file */
#define STAT_FILE       "../data/statlist.txt"
#define PET_FILE        "../data/pet.txt"
#define BOOT_FILE	"../data/boot.time"
#define HOUSE_FILE      "../data/house.txt"
#define TRIVIA_SCORE_FILE  "../data/tscore.txt"
#define MAIL_FILE       "../data/mail.txt"
#define POSTAL_FILE     "../data/postal.txt"
#define TXT_SOUNDURL    "http://shadowstorm.wolfpaw.net/sounds/"
#define TXT_IMAGEURL    "http://shadowstorm.wolfpaw.net/images/"

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define AD	AFFECT_DATA
#define PC      PROG_CODE

/* act_comm.c */
void  	check_sex	args( ( CHAR_DATA *ch) );
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void 	nuke_pets	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void 	logf2 		args( ( const char * fmt, ... ) ); 
void    info(CHAR_DATA * ch, int level, char * message, ...);
bool    can_be_sent     args( ( CHAR_DATA *ch, CHAR_DATA *vch, int channel ) );
void    set_auto_afk    args( ( CHAR_DATA *ch ) );

/* act_enter.c */
RID  *get_random_room   args ( (CHAR_DATA *ch) );

/* act_info.c */
void load_email         args( ( ) );
void save_email         args( ( ) );
void	set_title	args( ( CHAR_DATA *ch, char *title ) );
char * PERS_CHAN	args( ( CHAR_DATA *ch, CHAR_DATA *looker) );
void   display_bp       args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
#define SUP_OO(ch,flag) (!IS_SET((ch)->pcdata->suppress,(flag))?"{G({gON{G){x":"{R({rOFF{R){x")

/* act_move.c */
void	move_char	args( ( CHAR_DATA *ch, int door, bool follow ) );
bool    weather_move    args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *to_room, EXIT_DATA *pexit ) );
void    weather_check   args( ( CHAR_DATA *ch ) );
void    ice             args( ( CHAR_DATA *ch ) );
void    blizzard        args( ( CHAR_DATA *ch ) );
void    fog             args( ( CHAR_DATA *ch ) );       
#define BITS_PER_INT             32
#define PATH_IS_FLAG(flag, bit)  ((unsigned)flag[bit/BITS_PER_INT]>>bit%BITS_PER_INT&01)
#define PATH_SET_FLAG(flag, bit) (flag[bit/BITS_PER_INT] |= 1 << bit%BITS_PER_INT)
#define PATH_MAX_VNUM 32768
#define PATH_MAX_DIR  6

void check_explore args( ( CHAR_DATA *, ROOM_INDEX_DATA * ) );

void explore_vnum args( (CHAR_DATA *, int ) );
bool explored_vnum args( (CHAR_DATA *, int ) );

/* act_obj.c */
bool can_loot		args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void	wear_obj	args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace) );
void    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
                            OBJ_DATA *container ) );
bool    wear_obj_size   args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );          
void show_strapped	args( (CHAR_DATA*ch,CHAR_DATA *victim, int location) );
OBJ_DATA *get_obj_sheath args( ( CHAR_DATA *ch, char *argument, OBJ_DATA *sheath ) );

/* act_wiz.c */
void wiznet		args( (char *string, CHAR_DATA *ch, OBJ_DATA *obj,
			       long flag, long flag_skip, int min_level ) );
void copyover_recover   args( (void));
int  const_lookup       args( ( const char *name ) );
			       
/* alias.c */
void 	substitute_alias args( (DESCRIPTOR_DATA *d, char *input) );

/* ban.c */
bool	check_ban	args( ( char *site, int type) );


/* comm.c */
char *	str_str		args( (char *str1, char *str2) );
void	show_string	args( ( struct descriptor_data *d, char *input) );
void	close_socket	args( ( DESCRIPTOR_DATA *dclose ) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt,
			    int length ) );
void    send_to_desc    args( ( const char *txt, DESCRIPTOR_DATA *d ) );
void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	page_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	act		args( ( const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type ) );
void	act_new		args( ( const char *format, CHAR_DATA *ch, 
			    const void *arg1, const void *arg2, int type,
			    int min_pos) );
void	act_chan		args( ( const char *format, CHAR_DATA *ch, 
			    const void *arg1, const void *arg2, int type,
			    int min_pos) );
void    printf_to_char  args( ( CHAR_DATA *ch, char * fmt, ...));  
void	bugf		args( ( char *, ... ) );
void	send_to_desc    args( (const char *txt, DESCRIPTOR_DATA *d));
int 	colourdesc	args( ( char type, CHAR_DATA *ch, char *string ) );
void xact_new           args ( ( const char *format, CHAR_DATA *ch, const void *arg1,
                        const void *arg2, int type, int min_pos, int suppress ) );  
void xsend_to_char      args( (const char *txt, CHAR_DATA *ch, int suppress) );

/* File open/close wrappers to handle fpreserve correctly in all cases.
 * Also does some basic error checking. -Chilalin
 */
FILE *  file_open	args( ( const char *path, const char *mode ) );
void	file_close	args( ( FILE *fp ) );
bool	file_exists	args( ( const char *path ) );

/*
 * Colour stuff by Lope
 */
int     colour          args( ( char type, CHAR_DATA *ch, char *string ) );
void   colourconv      args( ( char *buffer, const char *txt, CHAR_DATA *ch ) );
void    send_to_char_bw args( ( const char *txt, CHAR_DATA *ch ) );
void    page_to_char_bw args( ( const char *txt, CHAR_DATA *ch ) );    

//cmdedit.c
void   save_cmd_table   args( ( ) );       
void   load_cmd_list    args( ( ) );

/* db.c */
void    fix_resets      args( ( void ) );
char *	print_flags	args( ( int flag ));
void	boot_db		args( ( void ) );
void	area_update	args( ( void ) );
CD *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex ) );
void	clone_mobile	args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void	clone_object	args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *	get_mob_index	args( ( int vnum ) );
OID *	get_obj_index	args( ( int vnum ) );
RID *	get_room_index	args( ( int vnum ) );
PC  *   get_prog_index  args( ( int vnum, int type ) ); 
char	fread_letter	args( ( FILE *fp ) );
int	fread_number	args( ( FILE *fp ) );
long 	fread_flag	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
char *  fread_string_eol args(( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
long	flag_convert	args( ( char letter) );
void *	alloc_mem	args( ( int sMem ) );
void *	alloc_perm	args( ( int sMem ) );
void	free_mem	args( ( void *pMem, int sMem ) );
char *	str_dup		args( ( const char *str ) );
void	free_string	args( ( char *pstr ) );
int	number_fuzzy	args( ( int number ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
long     number_mm       args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	capitalize	args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	bug		args( ( const char *str, int param ) );
void	log_string	args( ( const char *str ) );
void	tail_chain	args( ( void ) );
void 	load_helps_new  args( (  ) );
void    create_unique   args( ( void ) );
void  increase_total_output    args ((int clenght));
void  update_mudinfo           args ((void));
void  load_mudinfo             args ((void));
void  save_mudinfo             args (( void ));
void  affect_join_obj2  args( ( OBJ_DATA *obj, AFFECT_DATA * paf ) );
void  write_mudinfo_database   args (( void ));
extern int   mudinfo[MUDINFO_MAX];
extern int   total_output;

/* effect.c */
void	acid_effect	args( (void *vo, int level, int dam, int target) );
void	cold_effect	args( (void *vo, int level, int dam, int target) );
void	fire_effect	args( (void *vo, int level, int dam, int target) );
void	poison_effect	args( (void *vo, int level, int dam, int target) );
void	shock_effect	args( (void *vo, int level, int dam, int target) );
void    sand_effect     args( (void *vo, int level, int dam, int target) );

/* fight.c */
bool 	is_safe		args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool 	is_safe_spell	args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void	violence_update	args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
bool	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			        int dt, int, bool show ) );
bool    damage_old      args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
                                int dt, int, bool show ) );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim) );

/* handler.c */
int prime_class		args( (CHAR_DATA * ch) );
char *  material_name   args( ( sh_int num ) ); /* OLC */
AD  	*affect_find args( (AFFECT_DATA *paf, int sn));
void	affect_check	args( (CHAR_DATA *ch, int where, int vector) );
int	count_users	args( (OBJ_DATA *obj) );
void 	deduct_cost	args( (CHAR_DATA *ch, int cost) );
void	affect_enchant	args( (OBJ_DATA *obj) );
int 	check_immune	args( (CHAR_DATA *ch, int dam_type) );
int 	material_lookup args( ( const char *name) );
int	weapon_lookup	args( ( const char *name) );
int	weapon_type	args( ( const char *name) );
long    material_vuln   args( ( sh_int num ) );       
char 	*weapon_name	args( ( int weapon_Type) );
char	*item_name	args( ( int item_type) ); 
int	attack_lookup	args( ( const char *name) );
long	wiznet_lookup	args( ( const char *name) );
int	class_lookup	args( ( const char *name) );
bool	is_old_mob	args ( (CHAR_DATA *ch) );
int	get_skill	args( ( CHAR_DATA *ch, int sn ) );
int	get_weapon_sn	args( ( CHAR_DATA *ch ) );
int	get_weapon_skill args(( CHAR_DATA *ch, int sn ) );
int     get_age         args( ( CHAR_DATA *ch ) );
void	reset_char	args( ( CHAR_DATA *ch )  );
int	get_trust	args( ( CHAR_DATA *ch ) );
int	get_curr_stat	args( ( CHAR_DATA *ch, int stat ) );
int 	get_max_train	args( ( CHAR_DATA *ch, int stat ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( char *str, char *namelist ) );
bool    is_full_name    args( ( const char *str, char *namelist ) );
bool	is_exact_name	args( ( char *str, char *namelist ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_obj	args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove_obj args( (OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, int sn ) );
bool	is_affected	args( ( CHAR_DATA *ch, int sn ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear, int type ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
CD *    get_char_id     args( ( long id ) );     
CD *	get_char_area	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument, 
			    CHAR_DATA *viewer ) );
bool CAN_WEAR		args( (OBJ_DATA *obj, int part) );
OD *get_obj_wear	args( ( CHAR_DATA *ch, char *argument, bool character ));
OD *	get_obj_here	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	create_money	args( ( int gold, int silver ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
int	get_true_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	is_room_owner	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_see_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
const char *	affect_loc_name	args( ( int location ) );
const char *	affect_bit_name	args( ( int vector ) );
const char *  affect2_bit_name args( ( int vector ) );
const char *	extra_bit_name	args( ( int extra_flags ) );
const char *  extra2_bit_name args( ( int extra2_flags ) );
const char * 	wear_bit_name	args( ( int wear_flags ) );
const char *	act_bit_name	args( ( int act_flags ) );
const char *	off_bit_name	args( ( int off_flags ) );
const char *  imm_bit_name	args( ( int imm_flags ) );
const char * 	form_bit_name	args( ( int form_flags ) );
const char *	part_bit_name	args( ( int part_flags ) );
const char *	weapon_bit_name	args( ( int weapon_flags ) );
const char *  comm_bit_name	args( ( int comm_flags ) );
const char *  comm2_bit_name  args( ( int comm2_flags ) );
const char *	cont_bit_name	args( ( int cont_flags) );
void    affect_to_room  args( (ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) );
void	affect_remove_room args((ROOM_INDEX_DATA *room, AFFECT_DATA *paf) );
void	check_spirit	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
const char *  raffect_bit_name args( ( int vector ) ); 
bool check_color_string args( (char *string) );
void    obj_to_box      args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    obj_from_box    args( ( OBJ_DATA *obj ) );        
char *PERS		args( (CHAR_DATA *ch, CHAR_DATA *looker) );
bool has_nick_for_ch	args( (CHAR_DATA *ch, CHAR_DATA *victim) );
bool    is_admin        args( ( CHAR_DATA *ch ) );
void    timeout         args( ( CHAR_DATA *ch ) );
int     levelflux_dam   args( ( CHAR_DATA *ch ) );
MEMBER_DATA *member_lookup args( (CLAN_DATA *pClan, char *name) );
bool    can_pack        args( ( CHAR_DATA *ch ) );
CHAR_DATA *prog_char_room args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, char *argument ) );
void    dead_pet        args( ( CHAR_DATA *victim, PET_DATA *pEt ) );
void    revived_pet     args( ( CHAR_DATA *pet, PET_DATA *pEt ) );
const char *  suppress_bit_name args( ( int suppress_flags) ); 
void send_to_room       args( ( ROOM_INDEX_DATA *pRoom, char *fmt, ... ));

/*
 * Colour Config
 */
void	default_colour	args( ( CHAR_DATA *ch ) );
void	all_colour	args( ( CHAR_DATA *ch, char *argument ) );


/* interp.c */
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
int	mult_argument	args( ( char *argument, char *arg) );
char *	one_argument	args( ( char *argument, char *arg_first ) );
void    gen_wiz_table   args( (void) );   

/* magic.c */
int	find_spell	args( ( CHAR_DATA *ch, const char *name) );
int 	mana_cost 	(CHAR_DATA *ch, int min_mana, int level);
int	skill_lookup	args( ( const char *name ) );
int	slot_lookup	args( ( int slot ) );
bool	saves_spell	args( ( int level, CHAR_DATA *victim, int dam_type ) );
void	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch,
				    CHAR_DATA *victim, OBJ_DATA *obj ) );

/* multiclass.c */
bool can_use_skpell args((CHAR_DATA * ch, int sn));
bool has_spells args((CHAR_DATA * ch));
bool is_class args((CHAR_DATA * ch, int ));
bool is_same_class args((CHAR_DATA * ch, CHAR_DATA * victim));
int number_classes args((CHAR_DATA * ch));
char *class_long args((CHAR_DATA * ch));
char *class_who args((CHAR_DATA * ch));
char *class_short args((CHAR_DATA * ch));
char *class_numbers args((CHAR_DATA * ch, bool pSave));
int skill_level args((CHAR_DATA * ch, int sn));
int skill_rating args((CHAR_DATA * ch, int sn));
int group_rating args((CHAR_DATA * ch, int gn));
bool check_base_group args((CHAR_DATA * ch, int gn));
bool is_base_skill args((CHAR_DATA * ch, int sn));
int get_hp_gain args((CHAR_DATA * ch));
int get_mana_gain args((CHAR_DATA * ch));
bool is_prime_stat args((CHAR_DATA * ch, int pStat));
void add_default_groups args((CHAR_DATA * ch));
void add_base_groups args((CHAR_DATA * ch));
int get_stat_bonus args((CHAR_DATA * ch, int pStat));
int get_thac00 args((CHAR_DATA * ch));
int get_thac32 args((CHAR_DATA * ch));
int get_hp_max args((CHAR_DATA * ch));
int lvl_bonus args((CHAR_DATA * ch));
int class_mult args((CHAR_DATA * ch));
bool is_race_skill args((CHAR_DATA * ch, int sn));
int hp_max args((CHAR_DATA *ch));
//skill_save.c
void save_skills_table args( ());

//Religion.c
void religious_sacrifice args ( (CHAR_DATA *ch, OBJ_DATA *obj ) );
void save_religion  args ( () );
void load_religion  args ( () );
//recycle.c
EMAIL_DATA *  new_email		args ( ( void ) );
BUFFER *new_buf         args( () );
void free_nick		args ( (NICK_DATA *nick) );
NICK_DATA *new_nick	args( (void) );
void nick_to_char	args( ( CHAR_DATA *ch, NICK_DATA *nick ) );
MEMBER_DATA *new_member args( (void) );
void free_member	args( (MEMBER_DATA *member) );
void member_to_guild    args( ( CLAN_DATA *pClan, MEMBER_DATA *pmem ) );

/* save.c */
void	save_char_obj	args( ( CHAR_DATA *ch ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name ) );

/* skills.c */
bool 	parse_gen_groups args( ( CHAR_DATA *ch,char *argument ) );
void 	list_group_costs args( ( CHAR_DATA *ch ) );
void    list_group_known args( ( CHAR_DATA *ch ) );
int 	exp_per_level	args( ( CHAR_DATA *ch, int points ) );
void 	check_improve	args( ( CHAR_DATA *ch, int sn, bool success, 
				    int multiplier ) );
int 	group_lookup	args( (const char *name) );
void	gn_add		args( ( CHAR_DATA *ch, int gn) );
void 	gn_remove	args( ( CHAR_DATA *ch, int gn) );
void 	group_add	args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void	group_remove	args( ( CHAR_DATA *ch, const char *name) );

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );
char *	spec_name	args( ( SPEC_FUN *function ) );

/* teleport.c */
RID *	room_by_name	args( ( char *target, int level, bool error) );

/* update.c */
void	advance_level	args( ( CHAR_DATA *ch, bool hide ) );
void	gain_exp	args( ( CHAR_DATA *ch, int gain ) );
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( void ) );
void    advance_level_pet args( ( CHAR_DATA *pet, bool hide ) );
void    pet_gain_exp    args( ( CHAR_DATA *pet, int gain ) );
void    gain_object_exp args( ( CHAR_DATA *ch, OBJ_DATA *obj, int gain ) );

/* bounty.c */
bool is_bountied        args( (CHAR_DATA *ch) );
MEM_DATA *get_mem_data args((CHAR_DATA *ch, CHAR_DATA *target));  

#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef AD

/*****************************************************************************
 *                                    OLC                                    *
 *****************************************************************************/
/*
 * This structure is used in bit.c to lookup flags and stats.
 */
struct flag_type
{
    char * name;
    int  bit;
    bool settable;
};

/*
 * Object defined in limbo.are
 * Used in save.c to load objects that don't exist.
 */
#define OBJ_VNUM_DUMMY	1
#define OBJ_VNUM_BLANK  7

/*
 * Area flags.
 */
#define         AREA_NONE       0
#define         AREA_CHANGED    1	/* Area has been modified. */
#define         AREA_ADDED      2	/* Area has been added to. */
#define         AREA_LOADING    4	/* Used for counting in db.c */
#define		AREA_COMPLETE   5
#define         NO_UNIQUE       6

#define MAX_DIR	6
#define NO_FLAG -99	/* Must not be used in flags or stats. */


/*
 * Global Constants
 */
extern	char *	const	dir_name        [];
extern	const	sh_int	rev_dir         [];          /* sh_int - ROM OLC */
extern	const	struct	spec_type	spec_table	[];
extern  char *  const   rev_move        [];
extern  const   struct material_type    material_table  [];


/*
 * Global variables
 */
extern          AREA_DATA *             area_first;
extern          AREA_DATA *             area_last;
extern  	SHOP_DATA *             shop_last;

extern          int                     top_affect;
extern          int                     top_area;
extern          int                     top_ed;
extern          int                     top_exit;
extern          int                     top_help;
extern          int                     top_mob_index;
extern          int                     top_obj_index;
extern          int                     top_reset;
extern          int                     top_room;
extern          int                     top_shop;
extern          int                     top_vnum_mob;
extern          int                     top_vnum_obj;
extern          int                     top_vnum_room;

extern          char                    str_empty       [1];

extern  MOB_INDEX_DATA *        mob_index_hash  [MAX_KEY_HASH];
extern  OBJ_INDEX_DATA *        obj_index_hash  [MAX_KEY_HASH];
extern  ROOM_INDEX_DATA *       room_index_hash [MAX_KEY_HASH];
extern  AUCTION_DATA *          auction_list;
extern  GQUEST                  gquest_info;
extern  TRIVIA_SCORE *          trivia_score_hash [MAX_KEY_HASH];



/* act_wiz.c */
/*
ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg );
*/
/* db.c */
void	reset_area      args( ( AREA_DATA * pArea ) );
void	reset_room	args( ( ROOM_INDEX_DATA *pRoom ) );

/* string.c */
void	string_edit	args( ( CHAR_DATA *ch, char **pString ) );
void    string_append   args( ( CHAR_DATA *ch, char **pString ) );
char *	string_replace	args( ( char * orig, char * old, char *  ) );
void    string_add      args( ( CHAR_DATA *ch, char *argument ) );
char *  format_string   args( ( char *oldstring /*, bool fSpace */ ) );
char *  first_arg       args( ( char *argument, char *arg_first, bool fCase ) );
char *	string_unpad	args( ( char * argument ) );
char *	string_proper	args( ( char * argument ) );
char *  itos            args( ( int num ) );

/* olc.c */
bool	run_olc_editor	args( ( DESCRIPTOR_DATA *d ) );
char	*olc_ed_name	args( ( CHAR_DATA *ch ) );
char	*olc_ed_vnum	args( ( CHAR_DATA *ch ) );
CLAN_DATA *get_clan_data args( ( int clan ) );
int     qksort          args( ( void *data, int size, int esize, int i, int k, int (*compare)
           (const void *key1, const void *key2 ) ) );           
int     get_area_list_size args( ( void ) );

/* olc_save.c */
void    save_guilds     args( ( CHAR_DATA *ch, char *argument ) );
char 	*fwrite_flag	args( ( long flags, char buf[] ) );
char 	*fix_string	args( ( const char *str ) );
void	save_specials	args( ( FILE *fp, AREA_DATA *pArea ) );
void	save_resets		args( ( FILE *fp, AREA_DATA *pArea ) );
void	save_shops		args( ( FILE *fp, AREA_DATA *pArea ) );
void	save_mobprogs	args( ( FILE *fp, AREA_DATA *pArea ) );
void	save_objprogs	args( ( FILE *fp, AREA_DATA *pArea ) );
void	save_roomprogs	args( ( FILE *fp, AREA_DATA *pArea ) );
void	save_help_new	args( () );

/* special.c */
char *	spec_string	args( ( SPEC_FUN *fun ) );	/* OLC */

/* bit.c */
extern const struct flag_type 	area_flags[];
extern const struct flag_type	sex_flags[];
extern const struct flag_type	exit_flags[];
extern const struct flag_type	door_resets[];
extern const struct flag_type	room_flags[];
extern const struct flag_type	sector_flags[];
extern const struct flag_type	type_flags[];
extern const struct flag_type	extra_flags[];
extern const struct flag_type   extra2_flags[];
extern const struct flag_type	wear_flags[];
extern const struct flag_type	act_flags[];
extern const struct flag_type	affect_flags[];
extern const struct flag_type   affect2_flags[];
extern const struct flag_type	apply_flags[];
extern const struct flag_type	wear_loc_strings[];
extern const struct flag_type	wear_loc_flags[];
extern const struct flag_type	weapon_flags[];
extern const struct flag_type	container_flags[];
extern const struct flag_type	liquid_flags[];

/* ROM OLC: */

extern const struct flag_type   material_type[];
extern const struct flag_type   form_flags[];
extern const struct flag_type   part_flags[];
extern const struct flag_type   ac_type[];
extern const struct flag_type   size_flags[];
extern const struct flag_type   off_flags[];
extern const struct flag_type   imm_flags[];
extern const struct flag_type   res_flags[];
extern const struct flag_type   vuln_flags[];
extern const struct flag_type   position_flags[];
extern const struct flag_type   weapon_class[];
extern const struct flag_type   weapon_type_olc[];

/*olc_save_new.c*/
void	fwrite_mobiles	args( ( FILE *fp, AREA_DATA *pArea ) );
void	fwrite_rooms	args( ( FILE *fp, AREA_DATA *pArea ) );
void	fwrite_objects	args( ( FILE *fp, AREA_DATA *pArea ) );
void	fwrite_area		args( ( AREA_DATA *pArea ) );
void load_const		args ( () );

/*olc_load_new.c*/
void	fread_one_object		args( ( FILE *fp ) );
void	fread_affects		args( ( FILE *fp, OBJ_INDEX_DATA *pObjIndex ) );
void	append_new_save		args( ( char *file, char *str ) );
void	fread_objects			args( ( FILE *fp ) );
/*olc_load_rooms.c*/
void	fread_one_room		args( ( FILE *fp ) );
void	fread_rooms			args( ( FILE *fp ) );

/*olc_load_rooms.c*/
void	fread_one_mobile		args( ( FILE *fp ) );
void	fread_mobiles			args( ( FILE *fp ) );



/*****************************************************************************
 *                                 OLC END                                   *
 *****************************************************************************/



void spell_check args((CHAR_DATA * ch, char *string));       


#define CAN_ACCESS(room)     ( !IS_SET(room->room_flags, ROOM_PRIVATE) \
                             &&   !IS_SET(room->room_flags, ROOM_SOLITARY) \
                             &&   !IS_SET(room->room_flags, ROOM_NEWBIES_ONLY) \
                             &&   !IS_SET(room->room_flags, ROOM_GODS_ONLY) \
                             &&   !IS_SET(room->room_flags, ROOM_IMP_ONLY) )

int	get_hometown	args( ( char *argument ) );

/* mob_prog.c */
void	program_flow	args( ( sh_int vnum, char *source, CHAR_DATA *mob, 
				OBJ_DATA *obj, ROOM_INDEX_DATA *room,
				CHAR_DATA *ch, const void *arg1,
				const void *arg2 ) );
void	p_act_trigger	args( ( char *argument, CHAR_DATA *mob, 
				OBJ_DATA *obj, ROOM_INDEX_DATA *room,
				CHAR_DATA *ch, const void *arg1,
				const void *arg2, int type ) );
bool	p_percent_trigger args( ( CHAR_DATA *mob, OBJ_DATA *obj,
				ROOM_INDEX_DATA *room, CHAR_DATA *ch, 
				const void *arg1, const void *arg2, int type ) );
void	p_bribe_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch, int amount ) );
bool	p_exit_trigger   args( ( CHAR_DATA *ch, int dir, int type ) );
void	p_give_trigger   args( ( CHAR_DATA *mob, OBJ_DATA *obj, 
				ROOM_INDEX_DATA *room, CHAR_DATA *ch,
				OBJ_DATA *dropped, int type ) );
void 	p_greet_trigger  args( ( CHAR_DATA *ch, int type ) );
void	p_hprct_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch ) );
bool    p_alias_trigger  args ( ( CHAR_DATA *ch, char *cmd, OBJ_DATA *pObj, int type, char *argument ) );

/* mob_cmds.c */
void	mob_interpret	args( ( CHAR_DATA *ch, char *argument ) );
void	obj_interpret	args( ( OBJ_DATA *obj, char *argument ) );
void    room_interpret	args( ( ROOM_INDEX_DATA *room, char *argument ) );

/* lookup.c */
int	race_lookup	args( ( const char *name) );
int	item_lookup	args( ( const char *name) );
int	liq_lookup	args( ( const char *name) );


/* quest.c */
bool    chance args((int num));
void    do_mob_tell args((CHAR_DATA * ch, CHAR_DATA * victim, char *argument));
void    generate_quest args((CHAR_DATA * ch, CHAR_DATA * questman));
void    quest_update args((void));
bool    quest_level_diff args((CHAR_DATA * ch, CHAR_DATA * mob));
void    end_quest args((CHAR_DATA * ch, int time));
OBJ_DATA *has_questobj args((CHAR_DATA * ch));
void    update_questobjs args((CHAR_DATA * ch, OBJ_DATA * obj));
int     qobj_cost args((OBJ_DATA * obj));
int     is_qobj args((OBJ_DATA *obj));
void    add_apply args((OBJ_DATA * obj, int loc, int mod, int where, int type, int dur,int vector, int level));
void    do_mob_tell args((CHAR_DATA * ch, CHAR_DATA * victim, char *argument));
void    unfinished_quest args((CHAR_DATA *ch));
void    update_all_qobjs args((CHAR_DATA * ch));


/* auction.c */
void    reset_auc args ((AUCTION_DATA * auc, bool forced));
int     count_auc args ((CHAR_DATA * ch));
void    auction_update args ((void));
long    advatoi args ((const char *s));
long    parsebet args ((const long currentbet, const char *argument));
AUCTION_DATA *auction_lookup args((sh_int num));
int     get_auc_id args((void));
bool    has_auction args((CHAR_DATA *ch));
void    auction_channel args ( ( CHAR_DATA *ch, char * message ) );

/* wizlist.c */
void	update_wizlist	args( ( CHAR_DATA *ch, int level ) );

/* Gquest.c */
bool    load_gquest_data args((void));
bool    save_gquest_data args((void));
void    auto_gquest args((void));
bool    start_gquest args((CHAR_DATA * ch, char *argument));
void    end_gquest args((void));
void    gquest_update args((void));
void    gquest_wait args((void));
bool    generate_gquest args((CHAR_DATA * who));
int     count_gqmobs args((CHAR_DATA * ch));
int     is_gqmob args((CHAR_DATA * ch, int vnum));
void    reset_gqmob args((CHAR_DATA * ch, int value));
bool    is_random_gqmob args((int vnum));

/* drunk.c */
char	*makedrunk	args( (char *string, CHAR_DATA *ch) );
void spell_random args( (int sn, int level, CHAR_DATA * ch, void *vo, int target ) );


/* mount.c */
int     mount_success	args( ( CHAR_DATA *ch, CHAR_DATA *mount, int canattack ) );
void	do_buy_mount	args( ( CHAR_DATA *ch, char *argument ) );

//mine.c
SHAFT_DATA *get_shaft_cord	args( (MINE_DATA *pMine, int new_x, int new_y, int new_z ) );
void exit_mine args( ( CHAR_DATA *ch ) );
void put_char_mine args( ( CHAR_DATA *ch, EXIT_DATA *pexit ) );
void move_char_mine args ( (CHAR_DATA *ch, int door ) );
void char_from_mine args ( ( CHAR_DATA *ch ) );
MINE_DATA *new_mine args( (void) );
void free_mine	args( (MINE_DATA *mine) );
SHAFT_DATA *new_shaft args( (void) );
void free_shaft	args( (SHAFT_DATA *shaft) );
void look_mine	args ( ( CHAR_DATA *ch ) );
void fwrite_mine args ( (EXIT_DATA *pExit, FILE *fp ) );
void fread_mine args ( ( ROOM_INDEX_DATA *pRoom, FILE *fp) );

/* channels */
char *get_pre args( (CHAR_DATA *ch, int channel) );
void do_newchan args ( ( CHAR_DATA *ch, char *argument, int channel) );
bool smote_check args ( (CHAR_DATA *ch, char *argument, char *emote, int channel ) );
bool echo_check args ( (CHAR_DATA *ch, char *argument, char *emote, int channel ) );
bool check_chansocial args ( ( CHAR_DATA *ch, char *command, char *argument, int channel ) );
char *get_emote args ( ( CHAR_DATA *ch, char *argument) );
int  social_lookup args ( (const char *name) ); 
int  strlen_color args ( (char *argument) );

/* guild.c */
extern struct	clan_type	clan_table[];
const char *  guild_bit_name  args( ( int guild_flags ) );
bool    is_clan         args( (CHAR_DATA *ch) );
bool    is_same_clan    args( (CHAR_DATA *ch, CHAR_DATA *victim) );
int 	clan_lookup	args( (const char *name) );
char 	*player_rank	args( (CHAR_DATA *ch) );
char	*player_clan	args( (CHAR_DATA *ch) );
bool	can_guild	args( (CHAR_DATA *ch) );
bool    can_deguild     args( (CHAR_DATA *ch) );
bool    can_promote     args( (CHAR_DATA *ch) );
bool    can_demote      args( (CHAR_DATA *ch) );

/* war.c */
void war_channel args((CHAR_DATA *ch, char *message));
void war_update args((void));
void auto_war args((void));
void check_war args((CHAR_DATA * ch, CHAR_DATA * victim));
bool is_safe_war args((CHAR_DATA * ch, CHAR_DATA * wch));
void war_talk args((CHAR_DATA * ch, char *argument));
void end_war args((void));
bool abort_race_war args((void));
bool abort_class_war args((void));
bool abort_clan_war args((void));
void extract_war args(( CHAR_DATA * ch ));
                                           

/* object_cond.c */
void show_obj_cond     args ( (CHAR_DATA *ch, OBJ_DATA *obj) );  
void check_damage_obj  args ( (CHAR_DATA *ch, OBJ_DATA *obj, int chance) );  
void damage_obj        args ( (CHAR_DATA *ch, OBJ_DATA *obj, int damage) );  
void set_obj_condition args ( (OBJ_DATA *obj, int condition) );


/* timer.c */
char *   nsprintf       args( (char *, char *, ...) );
void  wait_wait         args( (CHAR_DATA *, int, int) );
void  wait_printf       args( (CHAR_DATA *, int, char *, ...) );
void  wait_act          args( (int, char *, void *, void *, void *, int) );
void  wait_function     args( (CHAR_DATA *ch,int delay, DO_FUN *do_fun, char *argument) );
EVENT_DATA *create_event(int, char *);

/* weather.c */
bool  number_chance     args( ( int num ) );
void  show_weather      args( ( CHAR_DATA *ch ) );
bool  IS_OUTDOORS       args( ( CHAR_DATA *ch ) );
int   temperature_change args( ( CHAR_DATA *ch ) );

/* act_mob.c */
void  save_pets         args( ( ) );


CMD_DATA *cmd_lookup args( ( const char *name ) );   


/* Trivia.c */
void do_trivia args( ( CHAR_DATA *ch, char*argument ) ); 
void load_trivia_scores args ((void));
void save_trivia_scores args ((void));
TRIVIA_SCORE *new_score args ((void));
TRIVIA_SCORE *trivia_score_lookup args ((char *argument));

void do_trivia_scores args ( ( CHAR_DATA *ch, char *argument) );

/* Socials */

void load_social_table();
void save_social_table(); 

/* Postal */

void load_postal_table();
void save_social_table();
void new_postal	args ( ( CHAR_DATA *ch ) );

/* Client */
void sound      args ( ( const char *fname, CHAR_DATA * ch ) );
void music      args ( ( const char *fname, CHAR_DATA *ch, bool repeat ) );
void stop_music args ( ( DESCRIPTOR_DATA *d ) );
void rnd_sound(int max, CHAR_DATA *ch, ...);
void mxp_init   args ( ( DESCRIPTOR_DATA *d ) );
char *click_cmd args ( ( DESCRIPTOR_DATA *d, char *text, char *command, char *mouseover ) );
size_t strlcat  args ( ( char *dst, const char *src, size_t siz ) );
void mxp_to_char args ( ( CHAR_DATA *ch, char *txt, int mxp_style ) );
void sound_to_room args ( ( const char *fname, CHAR_DATA *ch, bool to_world ) );

# if defined ( SNP )
#  undef SNP
# endif
# define SNP(var,args...)   snprintf ( (var), sizeof((var)) -1, ##args )

# if defined ( SLCAT )
#  undef SLCAT
# endif
# define SLCAT(str,append)  strlcat ( (str), (append), sizeof ( str ) )

/* mccp.c */
#ifdef MCCP_ENABLED
bool compressStart(DESCRIPTOR_DATA *desc);
bool compressEnd(DESCRIPTOR_DATA *desc);
bool processCompressed(DESCRIPTOR_DATA *desc);
bool writeCompressed(DESCRIPTOR_DATA *desc, char *txt, int length);
#endif
