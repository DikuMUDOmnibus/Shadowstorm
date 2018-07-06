/**************************************************************************
 *  File: olc.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/



#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h" 
#include "tables.h"
#include "olc.h"
#include "lookup.h"
#include "const.h"

/*
 * Local functions.
 */
AREA_DATA *get_area_data	args( ( int vnum ) );
DECLARE_OLC_FUN( do_asave       );


/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor( DESCRIPTOR_DATA *d )
{
    switch ( d->editor )
    {	case ED_AREA:
			aedit( d->character, d->incomm );
			break;
		case ED_ROOM:
			redit( d->character, d->incomm );
			break;
		case ED_OBJECT:
			oedit( d->character, d->incomm );
			break;
		case ED_MOBILE:
			medit( d->character, d->incomm );
			break;
		case ED_MPCODE:
			mpedit( d->character, d->incomm );
			break;
		case ED_OPCODE:
			opedit( d->character, d->incomm );
			break;
		case ED_RPCODE:
			rpedit( d->character, d->incomm );
			break;
		case ED_HELP:
    		hedit( d->character, d->incomm );
    		break;
		case ED_GUILD:
			gedit( d->character, d->incomm );
			break;
		case ED_SKILL:
			skedit(d->character, d->incomm);
			break;
		case ED_COMMAND:
			cmdedit( d->character, d->incomm);
			break;
		case ED_RELIGION:
			rlgedit(d->character, d->incomm);
			break;
		default:
			return FALSE;
    }
    return TRUE;
}



char *olc_ed_name( CHAR_DATA *ch )
{
    static char buf[10];
    
    buf[0] = '\0';
    switch (ch->desc->editor)
    {	case ED_AREA:
			sprintf( buf, "AEdit" );
			break;
		case ED_ROOM:
			sprintf( buf, "REdit" );
			break;
		case ED_OBJECT:
			sprintf( buf, "OEdit" );
			break;
		case ED_MOBILE:
			sprintf( buf, "MEdit" );
			break;
		case ED_MPCODE:
			sprintf( buf, "MPEdit" );
			break;
		case ED_OPCODE:
			sprintf( buf, "OPEdit" );
			break;
		case ED_RPCODE:
			sprintf( buf, "RPEdit" );
			break;
		case ED_HELP:
			sprintf( buf, "HEdit" );
			break;
		case ED_GUILD:
		    sprintf( buf, "GEdit" );
			break;
		case ED_SKILL:
			sprintf(buf, "SkEdit" );
			break;
		case ED_COMMAND:
			sprintf(buf, "CmdEdit" );
			break;
		case ED_RELIGION:
			sprintf(buf, "RlgEdit" );
			break;
		default:
			sprintf( buf, " " );
			break;
    }
    return buf;
}



char *olc_ed_vnum( CHAR_DATA *ch )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObj;
    MOB_INDEX_DATA *pMob;
    PROG_CODE *pMprog;
    PROG_CODE *pOprog;
    PROG_CODE *pRprog;
    HELP_DATA *pHelp;
    static char buf[MIL];
	
    buf[0] = '\0';
    switch ( ch->desc->editor )
    {
    case ED_AREA:
	pArea = (AREA_DATA *)ch->desc->pEdit;
	sprintf( buf, "%d", pArea ? pArea->vnum : 0 );
	break;
    case ED_ROOM:
	pRoom = ch->in_room;
	sprintf( buf, "%d", pRoom ? pRoom->vnum : 0 );
	break;
    case ED_OBJECT:
	pObj = (OBJ_INDEX_DATA *)ch->desc->pEdit;
	sprintf( buf, "%d", pObj ? pObj->vnum : 0 );
	break;
    case ED_MOBILE:
	pMob = (MOB_INDEX_DATA *)ch->desc->pEdit;
	sprintf( buf, "%d", pMob ? pMob->vnum : 0 );
	break;
    case ED_MPCODE:
    	pMprog = (PROG_CODE *)ch->desc->pEdit;
    	sprintf( buf, "%d", pMprog ? pMprog->vnum : 0 );
	break;
    case ED_OPCODE:
	pOprog = (PROG_CODE *)ch->desc->pEdit;
	sprintf( buf, "%d", pOprog ? pOprog->vnum : 0 );
	break;
    case ED_RPCODE:
	pRprog = (PROG_CODE *)ch->desc->pEdit;
	sprintf( buf, "%d", pRprog ? pRprog->vnum : 0 );
	break;
    case ED_HELP:
    	pHelp = (HELP_DATA *)ch->desc->pEdit;
    	sprintf( buf, "%s", pHelp ? pHelp->keyword : "" );
    	break;
    default:
	sprintf( buf, " " );
	break;
    }

    return buf;
}



/*****************************************************************************
 Name:		show_olc_cmds
 Purpose:	Format up the commands from given table.
 Called by:	show_commands(olc_act.c).
 ****************************************************************************/
void show_olc_cmds( CHAR_DATA *ch, const struct olc_cmd_type *olc_table )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  cmd;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (cmd = 0; olc_table[cmd].name != NULL; cmd++)
    {
	sprintf( buf, "%-15.15s", olc_table[cmd].name );
	strcat( buf1, buf );
	if ( ++col % 5 == 0 )
	    strcat( buf1, "\n\r" );
    }
 
    if ( col % 5 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}



/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_commands( CHAR_DATA *ch, char *argument )
{
    switch (ch->desc->editor)
    {
	case ED_AREA:
	    show_olc_cmds( ch, aedit_table );
	    break;
	case ED_ROOM:
	    show_olc_cmds( ch, redit_table );
	    break;
	case ED_OBJECT:
	    show_olc_cmds( ch, oedit_table );
	    break;
	case ED_MOBILE:
	    show_olc_cmds( ch, medit_table );
	    break;
	case ED_MPCODE:
	    show_olc_cmds( ch, mpedit_table );
	    break;
       	case ED_OPCODE:
	    show_olc_cmds( ch, opedit_table );
	    break;
	case ED_RPCODE:
	    show_olc_cmds( ch, rpedit_table );
	    break;
	case ED_HELP:
	    show_olc_cmds( ch, hedit_table );
	    break;
        case ED_GUILD:
            show_olc_cmds( ch, gedit_table );
            break;
	case ED_COMMAND:
	    show_olc_cmds( ch, cmdedit_table );
	    break;
	case ED_RELIGION:
		show_olc_cmds( ch, rlgedit_table );
		break;
	case ED_SKILL:
	    show_olc_cmds( ch, skedit_table );
    }

    return FALSE;
}



/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/
const struct olc_cmd_type aedit_table[] =
{
/*  {   command		function	}, */

    {   "age",		aedit_age	},
    {   "builder",	aedit_builder	}, /* s removed -- Hugin */
    {   "commands",	show_commands	},
    {   "create",	aedit_create	},
    {   "filename",	aedit_file	},
    {   "name",		aedit_name	},
/*  {   "recall",	aedit_recall	},   ROM OLC */
    {	"reset",	aedit_reset	},
    {   "security",	aedit_security	},
    {	"show",		aedit_show	},
    {   "vnum",		aedit_vnum	},
    {   "lvnum",	aedit_lvnum	},
    {   "uvnum",	aedit_uvnum	},
    {   "credits",	aedit_credits	},
    {   "repop",	aedit_repop_msg	},
    {   "continent",    aedit_continent },
    {   "temperature",  aedit_temperature },
    {   "complete",	aedit_complete  },
    {   "music",        aedit_music     },

    {   "?",		show_help	},
    {   "version",	show_version	},

    {	NULL,		0,		}
};



const struct olc_cmd_type redit_table[] =
{
/*  {   command		function	}, */

    {   "commands",	show_commands	},
    {   "create",	redit_create	},
    {   "desc",		redit_desc	},
    {   "ed",		redit_ed	},
    {   "format",	redit_format	},
    {   "name",		redit_name	},
    {	"show",		redit_show	},
    {   "heal",		redit_heal	},
    {	"mana",		redit_mana	},
    {   "clan",		redit_clan	},

    {   "north",	redit_north	},
    {   "south",	redit_south	},
    {   "east",		redit_east	},
    {   "west",		redit_west	},
    {   "up",		redit_up	},
    {   "down",		redit_down	},

    /* New reset commands. */
    {	"mreset",	redit_mreset	},
    {	"oreset",	redit_oreset	},
    {	"mlist",	redit_mlist	},
    {	"rlist",	redit_rlist	},
    {	"olist",	redit_olist	},
    {	"mshow",	redit_mshow	},
    {	"oshow",	redit_oshow	},
    {   "owner",	redit_owner	},
    {	"room",		redit_room	},
    {	"sector",	redit_sector	},
    {   "mineral",      redit_mineral   },
    {	"addrprog",	redit_addrprog	},
    {	"delrprog",	redit_delrprog	},
    {   "delete",       redit_delete    },
    {   "walkabout",    redit_walkabout },
    {   "?",		show_help	},
    {   "version",	show_version	},

    {	NULL,		0,		}
};



const struct olc_cmd_type oedit_table[] =
{
/*  {   command		function	}, */

    {   "addaffect",	oedit_addaffect	},
    {	"addapply",	oedit_addapply	},
    {   "addspell",     oedit_addspell  },
    {   "commands",	show_commands	},
    {   "cost",		oedit_cost	},
    {   "create",	oedit_create	},
    {   "delaffect",	oedit_delaffect	},
    {   "ed",		oedit_ed	},
    {   "long",		oedit_long	},
    {   "name",		oedit_name	},
    {   "short",	oedit_short	},
    {	"show",		oedit_show	},
    {   "v0",		oedit_value0	},
    {   "v1",		oedit_value1	},
    {   "v2",		oedit_value2	},
    {   "v3",		oedit_value3	},
    {   "v4",		oedit_value4	},  /* ROM */
    {   "weight",	oedit_weight	},

    {   "extra",        oedit_extra     },  /* ROM */
    {   "extra2",       oedit_extra2    },
    {   "wear",         oedit_wear      },  /* ROM */
    {   "type",         oedit_type      },  /* ROM */
    {   "material",     oedit_material  },  /* ROM */
    {   "level",        oedit_level     },  /* ROM */
    {   "condition",    oedit_condition },  /* ROM */
    {	"addoprog",	oedit_addoprog	},
    {	"deloprog",	oedit_deloprog	},
    {   "size",         oedit_size      },
    {   "autoweapon",   oedit_autoweapon},
    {   "autoarmor",    oedit_autoarmor },
    {   "delete",       oedit_delete    },
    {   "xptolevel",    oedit_xptolevel },
 
    {   "?",		show_help	},
    {   "version",	show_version	},

    {	NULL,		0,		}
};



const struct olc_cmd_type medit_table[] =
{
/*  {   command		function	}, */

    {   "alignment",	medit_align	},
    {   "commands",	show_commands	},
    {   "create",	medit_create	},
    {   "desc",		medit_desc	},
    {   "level",	medit_level	},
    {   "experience",   medit_experience},
    {   "long",		medit_long	},
    {   "name",		medit_name	},
    {   "shop",		medit_shop	},
    {   "short",	medit_short	},
    {	"show",		medit_show	},
    {   "spec",		medit_spec	},

    {   "sex",          medit_sex       },  /* ROM */
    {   "act",          medit_act       },  /* ROM */
    {   "affect",       medit_affect    },  /* ROM */
    {   "affect2",      medit_affect2   }, 
    {   "armor",        medit_ac        },  /* ROM */
    {   "form",         medit_form      },  /* ROM */
    {   "part",         medit_part      },  /* ROM */
    {   "imm",          medit_imm       },  /* ROM */
    {   "res",          medit_res       },  /* ROM */
    {   "vuln",         medit_vuln      },  /* ROM */
    {   "material",     medit_material  },  /* ROM */
    {   "off",          medit_off       },  /* ROM */
    {   "size",         medit_size      },  /* ROM */
    {   "hitdice",      medit_hitdice   },  /* ROM */
    {   "manadice",     medit_manadice  },  /* ROM */
    {   "damdice",      medit_damdice   },  /* ROM */
    {   "race",         medit_race      },  /* ROM */
    {   "position",     medit_position  },  /* ROM */
    {   "wealth",       medit_gold      },  /* ROM */
    {   "hitroll",      medit_hitroll   },  /* ROM */
    {	"damtype",	medit_damtype	},  /* ROM */
    {   "group",	medit_group	},  /* ROM */
    {   "addmprog",	medit_addmprog  },  /* ROM */
    {	"delmprog",	medit_delmprog	},  /* ROM */
    {   "autoset",      medit_autoset   },
    {   "clevel",       medit_clevel    },
    {   "cability",     medit_cability  },
    {   "delete",       medit_delete    },
    {   "aggression",   medit_aggression},

    {   "?",		show_help	},
    {   "version",	show_version	},

    {	NULL,		0,		}
};

/* Guild edit command table */
const struct olc_cmd_type gedit_table[] =
{
    {   "commands",	show_commands   },
    {   "create",	gedit_create	},
    {   "flag",		gedit_flags	},
    {   "list",		gedit_list	},
    {   "ml",           gedit_ml        },
    {   "name",		gedit_name	},
    {   "whoname",	gedit_whoname	},
    {   "rank",		gedit_rank	},
    {   "rooms",        gedit_rooms     },
    {   "show",		gedit_show	},
    {   "skill",	gedit_skill	},
    {   "?",		show_help	},
    {   NULL,		0		}
};

const struct olc_cmd_type skedit_table[] =
{
   {   "commands",	show_commands   },
   {   "show",		skedit_show	},
   {   "name",		skedit_name	},
   {   "level",		skedit_level	},
   {   "rating",	skedit_rating	},
   {   "spellfun",	skedit_spellfun },
   {   "target",	skedit_target	},
   {   "position",	skedit_position },
   {   "mana",		skedit_mana	},
   {   "beats",		skedit_beats	},
   {   "damage",	skedit_nound	},
   {   "msgoff",	skedit_msg_off  },
   {   "msgobj",	skedit_msg_obj  },
   {   "new",		skedit_new	},
   {   "list",		skedit_list	},
   {   NULL,		0		}
};

const struct olc_cmd_type cmdedit_table[] =
{
    { "commands",	show_commands	},
    { "show",		cmdedit_show    },
    { "name",		cmdedit_name	},
    { "catagory",	cmdedit_catagory },
    { "dofun",		cmdedit_dofun	},
    { "level",		cmdedit_level 	},
    { "cmdshow",	cmdedit_showcmd },
    { "log",		cmdedit_log	},
    { "new",		cmdedit_new	},
    { "move",		cmdedit_move	},
    { "position",	cmdedit_position },
    { "delete",		cmdedit_delete   },
    { NULL,		0		}
};

const struct olc_cmd_type rlgedit_table[] =
{
	{ "show",		rlgedit_show		},
	{ "name",		rlgedit_name		},
	{ "temple",		rlgedit_temple		},
	{ "faction",    	rlgedit_faction		},
	{ "god",		rlgedit_god		},
	{ "donation",   	rlgedit_donation	},
	{ "temple",		rlgedit_temple		},	
	{ "rank",		rlgedit_rank		},
	{ "skill",		rlgedit_skill		},
	{ "new",	    	rlgedit_new		},
	{ "list",	    	rlgedit_list		},
	{ "delete",	   	rlgedit_delete		},
	{ "command",		show_commands		},
	{ "message",		rlgedit_message		},
	{ NULL,			0					}
};
/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/



/*****************************************************************************
 Name:		get_area_data
 Purpose:	Returns pointer to area with given vnum.
 Called by:	do_aedit(olc.c).
 ****************************************************************************/

/** Function: get_clan_data
  * Descr   : Returns a pointer to the clan_table of the requested clan #
  * Returns : (CLAN_DATA *)
  * Syntax  : (n/a)
  * Written : v1.0 3/98
  * Author  : Gary McNickle <gary@dharvest.com>
  */
CLAN_DATA *get_clan_data( int clan )
{
  if ( clan <= MAX_CLAN && 
       (clan_table[clan].name != NULL && clan_table[clan].name[0] != '\0' ))
    return &clan_table[clan];

  return &clan_table[0]; /* null clan */
}

AREA_DATA *get_area_data( int vnum )
{
    AREA_DATA *pArea;

    for (pArea = area_first; pArea; pArea = pArea->next )
    {
        if (pArea->vnum == vnum)
            return pArea;
    }

    return 0;
}



/*****************************************************************************
 Name:		edit_done
 Purpose:	Resets builder information on completion.
 Called by:	aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool edit_done( CHAR_DATA *ch )
{
    ch->desc->pEdit = NULL;
    ch->desc->editor = 0;
    ch->desc->walkabout = FALSE;
    return FALSE;
}



/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/


/* Area Interpreter, called by do_aedit. */
void aedit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int  cmd;
    int  value;

    EDIT_AREA(ch, pArea);
    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "AEdit:  Insufficient security to modify area.\n\r", ch );
	edit_done( ch );
	return;
    }

    if ( !str_cmp(command, "done") )
    {
	edit_done( ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	aedit_show( ch, argument );
	return;
    }

    if ( ( value = flag_value( area_flags, command ) ) != NO_FLAG )
    {
	if(( value == AREA_COMPLETE ) )
	{
	    aedit_complete(ch, argument);
	    return;
	}
    

	TOGGLE_BIT(pArea->area_flags, value);

	send_to_char( "Flag toggled.\n\r", ch );
	return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; aedit_table[cmd].name != NULL; cmd++ )
    {
	if ( !str_prefix( command, aedit_table[cmd].name ) )
	{
	    if ( (*aedit_table[cmd].olc_fun) ( ch, argument ) )
	    {
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		return;
	    }
	    else
		return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}



/* Room Interpreter, called by do_redit. */
void redit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int  cmd;

    EDIT_ROOM(ch, pRoom);
    pArea = pRoom->area;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( !IS_BUILDER( ch, pArea ) )
    {
        send_to_char( "REdit:  Insufficient security to modify room.\n\r", ch );
	edit_done( ch );
	return;
    }

    if ( !str_cmp(command, "done") )
    {
	edit_done( ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	redit_show( ch, argument );
	return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; redit_table[cmd].name != NULL; cmd++ )
    {
	if ( !str_prefix( command, redit_table[cmd].name ) )
	{
	    if ( (*redit_table[cmd].olc_fun) ( ch, argument ) )
	    {
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		return;
	    }
	    else
		return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}



/* Object Interpreter, called by do_oedit. */
void oedit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int  cmd;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_OBJ(ch, pObj);
    pArea = pObj->area;

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "OEdit: Insufficient security to modify area.\n\r", ch );
	edit_done( ch );
	return;
    }

    if ( !str_cmp(command, "done") )
    {
	edit_done( ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	oedit_show( ch, argument );
	return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; oedit_table[cmd].name != NULL; cmd++ )
    {
	if ( !str_prefix( command, oedit_table[cmd].name ) )
	{
	    if ( (*oedit_table[cmd].olc_fun) ( ch, argument ) )
	    {
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		return;
	    }
	    else
		return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}



/* Mobile Interpreter, called by do_medit. */
void medit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int  cmd;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_MOB(ch, pMob);
    pArea = pMob->area;

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "MEdit: Insufficient security to modify area.\n\r", ch );
	edit_done( ch );
	return;
    }

    if ( !str_cmp(command, "done") )
    {
	edit_done( ch );
	return;
    }

    if ( command[0] == '\0' )
    {
        medit_show( ch, argument );
        return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; medit_table[cmd].name != NULL; cmd++ )
    {
	if ( !str_prefix( command, medit_table[cmd].name ) )
	{
	    if ( (*medit_table[cmd].olc_fun) ( ch, argument ) )
	    {
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		return;
	    }
	    else
		return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}


void skedit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int  cmd, sn;

    arg[0] = '\0';

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_SKILL(ch, sn);

    if ( !str_cmp(command, "done") )
    {
		edit_done( ch );
		return;
    }

    if ( command[0] == '\0' )
    {
	skedit_show(ch, argument);
        return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; skedit_table[cmd].name != NULL; cmd++ )
    {
		if ( !str_prefix( command, skedit_table[cmd].name ) )
		{
			skedit_table[cmd].olc_fun( ch, argument );
			return;
		}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}

void cmdedit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int  cmd;
    CMD_DATA *pCmd;

    arg[0] = '\0';

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_COMMAND(ch, pCmd);

    if ( !str_cmp(command, "done") )
    {
		edit_done( ch );
		return;
    }

    if ( command[0] == '\0' )
    {
	cmdedit_show(ch, argument);
        return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; cmdedit_table[cmd].name != NULL; cmd++ )
    {
		if ( !str_prefix( command, cmdedit_table[cmd].name ) )
		{
			cmdedit_table[cmd].olc_fun( ch, argument );
			return;
		}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}

void rlgedit(CHAR_DATA *ch, char *argument )
{	char religion[MIL];
	char arg[MSL];
	int relg;
	RELIGION *pRelg;

	arg[0] = '\0';

	smash_tilde(argument);
	strcpy(arg, argument);
	argument = one_argument(argument, religion);

	EDIT_RELIGION(ch, pRelg);

	if(!str_cmp(religion, "done") )
	{	edit_done(ch);
		return;
	}
	if(religion[0] == '\0' )
	{	rlgedit_show(ch, argument);
		return;
	}

    for ( relg = 0; rlgedit_table[relg].name != NULL; relg++ )
    {
		if ( !str_prefix( religion, rlgedit_table[relg].name ) )
		{
			rlgedit_table[relg].olc_fun( ch, argument );
			return;
		}
    }

	interpret(ch, arg);
	return;
}

const struct editor_cmd_type editor_table[] =
{
/*  {   command		function	}, */

    {   "area",		do_aedit	},
    {   "room",		do_redit	},
    {   "object",	do_oedit	},
    {   "mobile",	do_medit	},
    {	"mpcode",	do_mpedit	},
    {	"opcode",	do_opedit	},
    {	"rpcode",	do_rpedit	},
    {	"hedit",	do_hedit	},
    {   "guild",        do_gedit        },
    {   "skills",	do_skedit	},
    {   "commands",     do_cmdedit	},
	{	"religions",	do_rlgedit },
    {	NULL,		0,		}
};


/* Entry point for all editors. */
void do_olc( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    int  cmd;

    if ( IS_NPC(ch) )
    	return;

    argument = one_argument( argument, command );

    if ( command[0] == '\0' )
    {
        do_help( ch, "olc" );
        return;
    }
 
    /* Search Table and Dispatch Command. */
    for ( cmd = 0; editor_table[cmd].name != NULL; cmd++ )
    {
	if ( !str_prefix( command, editor_table[cmd].name ) )
	{
	    (*editor_table[cmd].do_fun) ( ch, argument );
	    return;
	}
    }

    /* Invalid command, send help. */
    do_help( ch, "olc" );
    return;
}

void clean_area_links (AREA_DATA * target)
{
    int vnum;
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObj;
    MOB_INDEX_DATA *pMob;
    PROG_CODE *pMpcode;
    char buf[MIL];

    for (vnum = target->min_vnum; vnum <= target->max_vnum; vnum++)
	{
	    if ((pRoom = get_room_index (vnum)) != NULL)
		{
		    sprintf (buf, "%d", vnum);
		    redit_delete (NULL, buf);
		}
	    if ((pObj = get_obj_index (vnum)) != NULL)
		{
		    sprintf (buf, "%d", vnum);
		    oedit_delete (NULL, buf);
		}
	    if ((pMob = get_mob_index (vnum)) != NULL)
		{
		    sprintf (buf, "%d", vnum);
		    medit_delete (NULL, buf);
		}
	    if ((pMpcode = get_prog_index (vnum, PRG_MPROG)) != NULL)
		{
		    sprintf (buf, "%d", vnum);
		    mpedit_delete (NULL, buf);
		}
		if ((pMpcode = get_prog_index (vnum, PRG_RPROG)) != NULL)
		{
		    sprintf (buf, "%d", vnum);
		    rpedit_delete (NULL, buf);
		}
		if ((pMpcode = get_prog_index (vnum, PRG_OPROG)) != NULL)
		{
		    sprintf (buf, "%d", vnum);
		    opedit_delete (NULL, buf);
		}
	}
}
bool vnum_OK (int lnum, int hnum)
{
    int i;

    for (i = 0; vnum_table[i].vnum != NULL; i++)
	{
	    if (*vnum_table[i].vnum >= lnum && *vnum_table[i].vnum <= hnum)
		return FALSE;
	}
    return TRUE;
}

/* Entry point for editing area_data. */
void do_aedit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    int value;
    char arg[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    	return;

    pArea	= ch->in_room->area;

    argument	= one_argument(argument,arg);

    if ( is_number( arg ) )
    {
	value = atoi( arg );
	if ( !( pArea = get_area_data( value ) ) )
	{
	    send_to_char( "That area vnum does not exist.\n\r", ch );
	    return;
	}
    }
    else
    if ( !str_cmp( arg, "create" ) )
    {
	if ( ch->pcdata->security < 9 )
	{
		send_to_char( "AEdit : Security insufficient to create area.\n\r", ch );
		return;
	}

	aedit_create( ch, "" );
	ch->desc->editor = ED_AREA;
	return;
    }

    else if (!str_cmp (arg, "delete"))
    {
	    AREA_DATA *curr, *prev;

	    if (ch->pcdata->security < 9)
		{
		    send_to_char
			("AEdit : Insufciant security to delete area.\n\r",
			 ch);
		    return;
		}
	    if (!is_number (argument) ||
		!(pArea = get_area_data (atoi (argument))))
		{
		    send_to_char ("That area vnum does not exist.\n\r", ch);
		    return;
		}

	    if (vnum_OK (pArea->min_vnum, pArea->max_vnum) == FALSE)
		{
		    send_to_char
			("That area contains hard coded vnums and cannot be deleted.\n\r",
			 ch);
		    return;
		}

	    clean_area_links (pArea);
	    unlink (pArea->file_name);
	    prev = NULL;
	    for (curr = area_first; curr != NULL;
		 prev = curr, curr = curr->next)
		{
		    if (curr == pArea)
			{
			    if (prev == NULL)
				area_first = area_first->next;
			    else
				prev->next = curr->next;

			    free_area (curr);
			    send_to_char ("Area deleted.\n\r", ch);
			    do_asave (ch, "world");
			}
		}
	    return;
	}

    if (!IS_BUILDER(ch,pArea))
    {
	send_to_char("Insuficiente seguridad para editar areas.\n\r",ch);
	return;
    }

    ch->desc->pEdit = (void *)pArea;
    ch->desc->editor = ED_AREA;
    return;
}



/* Entry point for editing room_index_data. */
void do_redit( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;
    char arg1[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    	return;

    argument = one_argument( argument, arg1 );

    pRoom = ch->in_room;

    if ( !str_cmp( arg1, "reset" ) )	/* redit reset */
    {
	if ( !IS_BUILDER( ch, pRoom->area ) )
	{
		send_to_char( "Insuficiente seguridad para modificar cuartos.\n\r" , ch );
        	return;
	}

	reset_room( pRoom );
	send_to_char( "Room reset.\n\r", ch );

	return;
    }
    else
    if ( !str_cmp( arg1, "create" ) )	/* redit create <vnum> */
    {
	if ( argument[0] == '\0' || atoi( argument ) == 0 )
	{
	    send_to_char( "Syntax:  edit room create [vnum]\n\r", ch );
	    return;
	}

	if ( redit_create( ch, argument ) ) /* pEdit == nuevo cuarto */
	{
	    ch->desc->editor = ED_ROOM;
	    char_from_room( ch );
	    char_to_room( ch, (ROOM_INDEX_DATA *)ch->desc->pEdit );
	    SET_BIT( ((ROOM_INDEX_DATA *)ch->desc->pEdit)->area->area_flags, AREA_CHANGED );
	}

	return;
    }
    else if ( !IS_NULLSTR(arg1) )	/* redit <vnum> */
    {
	pRoom = get_room_index(atoi(arg1));

	if ( !pRoom )
	{
		send_to_char( "REdit : Nonexistant room.\n\r", ch );
		return;
	}

	if ( !IS_BUILDER(ch, pRoom->area) )
	{
		send_to_char( "REdit : Insufficient security to modify room.\n\r", ch );
		return;
	}

	char_from_room( ch );
	char_to_room( ch, pRoom );
    }

    else if (!str_cmp (arg1, "delete"))
    {
	redit_delete (ch, argument);
	return;
    }

    if ( !IS_BUILDER(ch, pRoom->area) )
    {
    	send_to_char( "REdit : Insuficiente seguridad para editar cuartos.\n\r", ch );
    	return;
    }

    ch->desc->pEdit	= (void *) pRoom;
    ch->desc->editor	= ED_ROOM;

    return;
}



/* Entry point for editing obj_index_data. */
void do_oedit( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    char arg1[MAX_STRING_LENGTH];
    int value;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg1 );

    if ( is_number( arg1 ) )
    {
	value = atoi( arg1 );
	if ( !( pObj = get_obj_index( value ) ) )
	{
	    send_to_char( "OEdit:  That vnum does not exist.\n\r", ch );
	    return;
	}

	if ( !IS_BUILDER( ch, pObj->area ) )
	{
		send_to_char( "Insuficiente seguridad para modificar objetos.\n\r" , ch );
	        return;
	}

	ch->desc->pEdit = (void *)pObj;
	ch->desc->editor = ED_OBJECT;
	return;
    }
    else
    {
	if ( !str_cmp( arg1, "create" ) )
	{
	    value = atoi( argument );
	    if ( argument[0] == '\0' || value == 0 )
	    {
		send_to_char( "Syntax:  edit object create [vnum]\n\r", ch );
		return;
	    }

	    pArea = get_vnum_area( value );

	    if ( !pArea )
	    {
		send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
		return;
	    }

	    if ( !IS_BUILDER( ch, pArea ) )
	    {
		send_to_char( "Insuficiente seguridad para modificar objetos.\n\r" , ch );
	        return;
	    }

	    if ( oedit_create( ch, argument ) )
	    {
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		ch->desc->editor = ED_OBJECT;
	    }
	    return;
	}

	else if (!str_cmp (arg1, "delete"))
	{
	    oedit_delete (ch, argument);
	    return;
	}
    }

    send_to_char( "OEdit:  There is no default object to edit.\n\r", ch );
    return;
}



/* Entry point for editing mob_index_data. */
void do_medit( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int value;
    char arg1[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg1 );

    if ( IS_NPC(ch) )
    	return;

    if ( is_number( arg1 ) )
    {
	value = atoi( arg1 );
	if ( !( pMob = get_mob_index( value ) ))
	{
	    send_to_char( "MEdit:  That vnum does not exist.\n\r", ch );
	    return;
	}

	if ( !IS_BUILDER( ch, pMob->area ) )
	{
		send_to_char( "Insuficiente seguridad para modificar mobs.\n\r" , ch );
	        return;
	}

	ch->desc->pEdit = (void *)pMob;
	ch->desc->editor = ED_MOBILE;
	return;
    }
    else
    {
	if ( !str_cmp( arg1, "create" ) )
	{
	    value = atoi( argument );
	    if ( arg1[0] == '\0' || value == 0 )
	    {
		send_to_char( "Syntax:  edit mobile create [vnum]\n\r", ch );
		return;
	    }

	    pArea = get_vnum_area( value );

	    if ( !pArea )
	    {
		send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
		return;
	    }

	    if ( !IS_BUILDER( ch, pArea ) )
	    {
		send_to_char( "Insuficiente seguridad para modificar mobs.\n\r" , ch );
	        return;
	    }

	    if ( medit_create( ch, argument ) )
	    {
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		ch->desc->editor = ED_MOBILE;
	    }
	    return;
	}
         
	else if (!str_cmp (arg1, "delete"))
	{
	    medit_delete (ch, argument);
	    return;
	}
    }

    send_to_char( "MEdit:  There is no default mobile to edit.\n\r", ch );
    return;
}



void display_resets( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA	*pRoom;
    RESET_DATA		*pReset;
    MOB_INDEX_DATA	*pMob = NULL;
    char 		buf   [ MAX_STRING_LENGTH ];
    char 		final [ MAX_STRING_LENGTH ];
    int 		iReset = 0;

    EDIT_ROOM(ch, pRoom);
    final[0]  = '\0';
    
    send_to_char ( 
  " No.  Loads    Description       Location         Vnum   Mx Mn Description"
  "\n\r"
  "==== ======== ============= =================== ======== ===== ==========="
  "\n\r", ch );

    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
    {
	OBJ_INDEX_DATA  *pObj;
	MOB_INDEX_DATA  *pMobIndex;
	OBJ_INDEX_DATA  *pObjIndex;
	OBJ_INDEX_DATA  *pObjToIndex;
	ROOM_INDEX_DATA *pRoomIndex;

	final[0] = '\0';
	sprintf( final, "[%2d] ", ++iReset );

	switch ( pReset->command )
	{
	default:
	    sprintf( buf, "Bad reset command: %c.", pReset->command );
	    strcat( final, buf );
	    break;

	case 'M':
	    if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
	    {
                sprintf( buf, "Load Mobile - Bad Mob %d\n\r", pReset->arg1 );
                strcat( final, buf );
                continue;
	    }

	    if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
	    {
                sprintf( buf, "Load Mobile - Bad Room %d\n\r", pReset->arg3 );
                strcat( final, buf );
                continue;
	    }

            pMob = pMobIndex;
            sprintf( buf, "M[%5d] %-13.13s in room             R[%5d] %2d-%2d %-15.15s\n\r",
                       pReset->arg1, pMob->short_descr, pReset->arg3,
                       pReset->arg2, pReset->arg4, pRoomIndex->name );
            strcat( final, buf );

	    /*
	     * Check for pet shop.
	     * -------------------
	     */
	    {
		ROOM_INDEX_DATA *pRoomIndexPrev;

		pRoomIndexPrev = get_room_index( pRoomIndex->vnum - 1 );
		if ( pRoomIndexPrev
		    && IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
                    final[5] = 'P';
	    }

	    break;

	case 'O':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
	    {
                sprintf( buf, "Load Object - Bad Object %d\n\r",
		    pReset->arg1 );
                strcat( final, buf );
                continue;
	    }

            pObj       = pObjIndex;

	    if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
	    {
                sprintf( buf, "Load Object - Bad Room %d\n\r", pReset->arg3 );
                strcat( final, buf );
                continue;
	    }

            sprintf( buf, "O[%5d] %-13.13s in room             "
                          "R[%5d]       %-15.15s\n\r",
                          pReset->arg1, pObj->short_descr,
                          pReset->arg3, pRoomIndex->name );
            strcat( final, buf );

	    break;

	case 'P':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
	    {
                sprintf( buf, "Put Object - Bad Object %d\n\r",
                    pReset->arg1 );
                strcat( final, buf );
                continue;
	    }

            pObj       = pObjIndex;

	    if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
	    {
                sprintf( buf, "Put Object - Bad To Object %d\n\r",
                    pReset->arg3 );
                strcat( final, buf );
                continue;
	    }

	    sprintf( buf,
		"O[%5d] %-13.13s inside              O[%5d] %2d-%2d %-15.15s\n\r",
		pReset->arg1,
		pObj->short_descr,
		pReset->arg3,
		pReset->arg2,
		pReset->arg4,
		pObjToIndex->short_descr );
            strcat( final, buf );

	    break;

	case 'G':
	case 'E':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
	    {
                sprintf( buf, "Give/Equip Object - Bad Object %d\n\r",
                    pReset->arg1 );
                strcat( final, buf );
                continue;
	    }

            pObj       = pObjIndex;

	    if ( !pMob )
	    {
                sprintf( buf, "Give/Equip Object - No Previous Mobile\n\r" );
                strcat( final, buf );
                break;
	    }

	    if ( pMob->pShop )
	    {
	    sprintf( buf,
		"O[%5d] %-13.13s in the inventory of S[%5d]       %-15.15s\n\r",
		pReset->arg1,
		pObj->short_descr,                           
		pMob->vnum,
		pMob->short_descr  );
	    }
	    else
	    sprintf( buf,
		"O[%5d] %-13.13s %-19.19s M[%5d]       %-15.15s\n\r",
		pReset->arg1,
		pObj->short_descr,
		(pReset->command == 'G') ?
		    flag_string( wear_loc_strings, WEAR_NONE )
		  : flag_string( wear_loc_strings, pReset->arg3 ),
		  pMob->vnum,
		  pMob->short_descr );
	    strcat( final, buf );

	    break;

	/*
	 * Doors are set in rs_flags don't need to be displayed.
	 * If you want to display them then uncomment the new_reset
	 * line in the case 'D' in load_resets in db.c and here.
	 */
	case 'D':
	    pRoomIndex = get_room_index( pReset->arg1 );
	    sprintf( buf, "R[%5d] %s door of %-19.19s reset to %s\n\r",
		pReset->arg1,
		capitalize( dir_name[ pReset->arg2 ] ),
		pRoomIndex->name,
		flag_string( door_resets, pReset->arg3 ) );
	    strcat( final, buf );

	    break;
	/*
	 * End Doors Comment.
	 */
	case 'R':
	    if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
	    {
		sprintf( buf, "Randomize Exits - Bad Room %d\n\r",
		    pReset->arg1 );
		strcat( final, buf );
		continue;
	    }

	    sprintf( buf, "R[%5d] Exits are randomized in %s\n\r",
		pReset->arg1, pRoomIndex->name );
	    strcat( final, buf );

	    break;
	}
	send_to_char( final, ch );
    }

    return;
}



/*****************************************************************************
 Name:		add_reset
 Purpose:	Inserts a new reset in the given index slot.
 Called by:	do_resets(olc.c).
 ****************************************************************************/
void add_reset( ROOM_INDEX_DATA *room, RESET_DATA *pReset, int index )
{
    RESET_DATA *reset;
    int iReset = 0;

    if ( !room->reset_first )
    {
	room->reset_first	= pReset;
	room->reset_last	= pReset;
	pReset->next		= NULL;
	return;
    }

    index--;

    if ( index == 0 )	/* First slot (1) selected. */
    {
	pReset->next = room->reset_first;
	room->reset_first = pReset;
	return;
    }

    /*
     * If negative slot( <= 0 selected) then this will find the last.
     */
    for ( reset = room->reset_first; reset->next; reset = reset->next )
    {
	if ( ++iReset == index )
	    break;
    }

    pReset->next	= reset->next;
    reset->next		= pReset;
    if ( !pReset->next )
	room->reset_last = pReset;
    return;
}



void do_resets( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    char arg6[MAX_INPUT_LENGTH];
    char arg7[MAX_INPUT_LENGTH];
    RESET_DATA *pReset = NULL;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg5 );
    argument = one_argument( argument, arg6 );
    argument = one_argument( argument, arg7 );

    if ( !IS_BUILDER( ch, ch->in_room->area ) )
    {
	send_to_char( "Resets: Invalid security for editing this area.\n\r",
                      ch );
	return;
    }

    /*
     * Display resets in current room.
     * -------------------------------
     */
    if ( arg1[0] == '\0' )
    {
	if ( ch->in_room->reset_first )
	{
	    send_to_char(
		"Resets: M = mobile, R = room, O = object, "
		"P = pet, S = shopkeeper\n\r", ch );
	    display_resets( ch );
	}
	else
	    send_to_char( "No resets in this room.\n\r", ch );
    }


    /*
     * Take index number and search for commands.
     * ------------------------------------------
     */
    if ( is_number( arg1 ) )
    {
	ROOM_INDEX_DATA *pRoom = ch->in_room;

	/*
	 * Delete a reset.
	 * ---------------
	 */
	if ( !str_cmp( arg2, "delete" ) )
	{
	    int insert_loc = atoi( arg1 );

	    if ( !ch->in_room->reset_first )
	    {
		send_to_char( "No resets in this area.\n\r", ch );
		return;
	    }

	    if ( insert_loc-1 <= 0 )
	    {
		pReset = pRoom->reset_first;
		pRoom->reset_first = pRoom->reset_first->next;
		if ( !pRoom->reset_first )
		    pRoom->reset_last = NULL;
	    }
	    else
	    {
		int iReset = 0;
		RESET_DATA *prev = NULL;

		for ( pReset = pRoom->reset_first;
		  pReset;
		  pReset = pReset->next )
		{
		    if ( ++iReset == insert_loc )
			break;
		    prev = pReset;
		}

		if ( !pReset )
		{
		    send_to_char( "Reset not found.\n\r", ch );
		    return;
		}

		if ( prev )
		    prev->next = prev->next->next;
		else
		    pRoom->reset_first = pRoom->reset_first->next;

		for ( pRoom->reset_last = pRoom->reset_first;
		  pRoom->reset_last->next;
		  pRoom->reset_last = pRoom->reset_last->next );
	    }

	    free_reset_data( pReset );
	    send_to_char( "Reset deleted.\n\r", ch );
	}
	else
	/*
	 * Add a reset.
	 * ------------
	 */
	if ( (!str_cmp( arg2, "mob" ) && is_number( arg3 ))
	  || (!str_cmp( arg2, "obj" ) && is_number( arg3 )) )
	{
	    /*
	     * Check for Mobile reset.
	     * -----------------------
	     */
	    if ( !str_cmp( arg2, "mob" ) )
	    {
		if (get_mob_index( is_number(arg3) ? atoi( arg3 ) : 1 ) == NULL)
		  {
		    send_to_char("Mob no existe.\n\r",ch);
		    return;
		  }
		pReset = new_reset_data();
		pReset->command = 'M';
		pReset->arg1    = atoi( arg3 );
		pReset->arg2    = is_number( arg4 ) ? atoi( arg4 ) : 1; /* Max # */
		pReset->arg3    = ch->in_room->vnum;
		pReset->arg4	= is_number( arg5 ) ? atoi( arg5 ) : 1; /* Min # */
	    }
	    else
	    /*
	     * Check for Object reset.
	     * -----------------------
	     */
	    if ( !str_cmp( arg2, "obj" ) )
	    {
		pReset = new_reset_data();
		pReset->arg1    = atoi( arg3 );
		/*
		 * Inside another object.
		 * ----------------------
		 */
		if ( !str_prefix( arg4, "inside" ) )
		{
		    OBJ_INDEX_DATA *temp;

		    temp = get_obj_index(is_number(arg5) ? atoi(arg5) : 1);
		    if ( ( temp->item_type != ITEM_CONTAINER ) &&
		         ( temp->item_type != ITEM_CORPSE_NPC ) )
		     {
		       send_to_char( "The second object is not a container.\n\r", ch);
		       return;
		     }
		    pReset->command = 'P';
		    pReset->arg2    = is_number( arg6 ) ? atoi( arg6 ) : 1;
		    pReset->arg3    = is_number( arg5 ) ? atoi( arg5 ) : 1;
		    pReset->arg4    = is_number( arg7 ) ? atoi( arg7 ) : 1;
		}
		else
		/*
		 * Inside the room.
		 * ----------------
		 */
		if ( !str_cmp( arg4, "room" ) )
		{
		    if (get_obj_index(atoi(arg3)) == NULL)
		      {
		         send_to_char( "That room does dot exist.\n\r",ch);
		         return;
		      }
		    pReset->command  = 'O';
		    pReset->arg2     = 0;
		    pReset->arg3     = ch->in_room->vnum;
		    pReset->arg4     = 0;
		}
		else
		/*
		 * Into a Mobile's inventory.
		 * --------------------------
		 */
		{
		    if ( flag_value( wear_loc_flags, arg4 ) == NO_FLAG )
		    {
			send_to_char( "Resets: '? wear-loc'\n\r", ch );
			return;
		    }
		    if (get_obj_index(atoi(arg3)) == NULL)
		      {
		         send_to_char( "Vnum no existe.\n\r",ch);
		         return;
		      }
		    pReset->arg1 = atoi(arg3);
		    pReset->arg3 = flag_value( wear_loc_flags, arg4 );
		    if ( pReset->arg3 == WEAR_NONE )
			pReset->command = 'G';
		    else
			pReset->command = 'E';
		}
	    }
	    add_reset( ch->in_room, pReset, atoi( arg1 ) );
	    SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
	    send_to_char( "Reset added.\n\r", ch );
	}
	else
	if (!str_cmp( arg2, "random") && is_number(arg3))
	{
		if (atoi(arg3) < 1 || atoi(arg3) > 6)
			{
				send_to_char("Invalid argument.\n\r", ch);
				return;
			}
		pReset = new_reset_data ();
		pReset->command = 'R';
		pReset->arg1 = ch->in_room->vnum;
		pReset->arg2 = atoi(arg3);
		add_reset( ch->in_room, pReset, atoi( arg1 ) );
		SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
		send_to_char( "Random exits reset added.\n\r", ch);
	}
	else
	{
	send_to_char( "Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch );
	send_to_char( "        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\n\r", ch );
	send_to_char( "        RESET <number> OBJ <vnum> room\n\r", ch );
	send_to_char( "        RESET <number> MOB <vnum> [max #x area] [max #x room]\n\r", ch );
	send_to_char( "        RESET <number> DELETE\n\r", ch );
	send_to_char( "        RESET <number> RANDOM [#x exits]\n\r", ch);
	}
    }

    return;
}

/** Function: gedit
  * Descr   : Interprets commands sent while inside the guild editor.
  *         : Passing appropriate commands on to editor, rest to mud.
  * Returns : (N/A)
  * Syntax  : (N/A| called by do_gedit only)
  * Written : v1.0 3/98
  * Author  : Gary McNickle <gary@dharvest.com>
  */
void gedit( CHAR_DATA *ch, char *argument )
{
  CLAN_DATA *pClan;
  char command[MIL];
  char arg[MIL];
  
  int cmd;
  
  EDIT_GUILD(ch, pClan);
  smash_tilde( argument );
  strcpy( arg, argument );
  argument = one_argument( argument, command );
  
  if ( ch->level < MAX_LEVEL -2 )
  {
    send_to_char("Insuffecient security to modify guild data", ch);
    edit_done( ch );
    return;
  }

  if ( !str_cmp(command, "done") )
  {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' )
  {
    gedit_show( ch, argument );
    return;
  }
  
  /* Search Table and Dispatch Command. */
  for ( cmd = 0; gedit_table[cmd].name != NULL; cmd++ )
  {
    if ( !str_prefix( command, gedit_table[cmd].name ) )
    {
      if ( (*gedit_table[cmd].olc_fun) ( ch, argument ) )
      {
	SET_BIT( pClan->flags, GUILD_CHANGED );
	return;
      }
      else
       return;
    }
  }

  /* Default to Standard Interpreter. */
  interpret( ch, arg );
  return;
}


/** Function: do_gedit
  * Descr   : Interprets commands sent to the OLC Guild editor
  * Returns : (N/A)
  * Syntax  : gedit [(none)|done|create]
  * Written : v1.0 3/98
  * Author  : Gary McNickle <gary#dharvest,com>
  */
void do_gedit( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;
    int value;
    char arg[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    	return;

    pClan = &clan_table[1]; /* set default */

    argument	= one_argument(argument,arg);

    if ( is_number( arg ) )
    {
	value = atoi( arg );
        pClan = get_clan_data( value );
        if (pClan->name[0] == '\0')
	{
	    send_to_char( "That guild does not exist.\n\r", ch );
	    return;
	}
	ch->desc->editor = ED_GUILD;
	ch->desc->pEdit = pClan;
    }
    else
    if ( !str_cmp( arg, "create" ) )
    {
        int i = 0;

	if ( ch->pcdata->security < 9 )
	{
	  send_to_char( "GEdit : Insuffecient security to create new guilds.\n\r", ch );
  	  return;
	}

	gedit_create( ch, "" );

         /* since the clan just created will be the last clan in the
            clan_table, we search backwards until we find one with a name */

        for (i = MAX_CLAN; i > 0; i--)
         if ( clan_table[i].name != NULL ) break;

        pClan = get_clan_data( i );

	ch->desc->editor = ED_GUILD;
	return;
    }
}

/*  sorting method for alist */
static int compare_lvnum(const void *area1, const void *area2)
{
    if (((const AREA_DATA *)area1)->min_vnum >
        ((const AREA_DATA *)area2)->min_vnum)
        return 1;
    else if (((const AREA_DATA *)area1)->min_vnum <
             ((const AREA_DATA *)area2)->min_vnum)
        return -1;
    else
        return 0;
}

/*  sorting method for alist */
static int compare_name(const void *area1, const void *area2)
{
    int retval;

    if ((retval = strcmp(((const AREA_DATA *)area1)->name,
                         ((const AREA_DATA *)area2)->name)) > 0)
        return 1;
    else if (retval < 0)
        return -1;
    else
        return 0;
}

/*  sorting method for alist */
static int compare_filename(const void *area1, const void *area2)
{
    int retval;

    if ((retval = strcmp(((const AREA_DATA *)area1)->file_name,
                         ((const AREA_DATA *)area2)->file_name)) > 0)
        return 1;
    else if (retval < 0)
        return -1;
    else
        return 0;
}

/*  sorting method for alist */
static int compare_builders(const void *area1, const void *area2)
{
    int retval;

    if ((retval = strcmp(((const AREA_DATA *)area1)->builders,
                         ((const AREA_DATA *)area2)->builders)) > 0)
        return 1;
    else if (retval < 0)
        return -1;
    else
        return 0;
}


void do_alist (CHAR_DATA * ch, char *argument)
{
    char command[MAX_INPUT_LENGTH];
    int numAreas = get_area_list_size();
    int i, complete = 0, count = 0, x;
    AREA_DATA *pArea;
    AREA_DATA *aSort = (AREA_DATA *)calloc(numAreas, sizeof(AREA_DATA));
    ROOM_INDEX_DATA *pRoom;
    int top_area, desc_area;
    BUFFER1 *buf = buffer_new(3000);

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, command);


    /*  copy area list into an array for sorting */
    pArea = area_first;
    for (i = 0; i < numAreas; ++i)
    {
        aSort[i].vnum      = pArea->vnum;
        aSort[i].name      = pArea->name;
        aSort[i].min_vnum  = pArea->min_vnum;
        aSort[i].max_vnum  = pArea->max_vnum;
        aSort[i].file_name = pArea->file_name;
        aSort[i].security  = pArea->security;
        aSort[i].builders  = pArea->builders;
        aSort[i].continent = pArea->continent;
	aSort[i].area_flags = pArea->area_flags;
        pArea = pArea->next;
    }

    bprintf (buf, "{D[{w%3s{D] {W%-23s {D({w%-5s-%5s{D) {W%-10s {D[{w%3s{D] {W[{w%-10s{W] {D[{w%-5s{D] {W[{wContinent{W]{x\n\r\n\r",
             "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec",
             "Builders", "Complete");    
    send_to_char( buf->data, ch );
    buffer_clear(buf);

    /*  perform sort if argment given */
    if (!str_cmp (command, "lvnum"))
        qksort(aSort, numAreas, sizeof(AREA_DATA), 0, numAreas - 1, compare_lvnum);
    else if (!str_cmp (command, "name"))
        qksort(aSort, numAreas, sizeof(AREA_DATA), 0, numAreas - 1, compare_name);
    else if (!str_cmp (command, "filename"))
        qksort(aSort, numAreas, sizeof(AREA_DATA), 0, numAreas - 1, compare_filename);
    else if (!str_cmp (command, "builders") || !str_cmp (command, "builder"))
        qksort(aSort, numAreas, sizeof(AREA_DATA), 0, numAreas - 1, compare_builders);
    /*  generate output buffer */
    desc_area = 0; top_area = 0;

    for (i = 0; i < numAreas; ++i)
    {
           for (x = aSort[i].min_vnum; x <= aSort[i].max_vnum; x++)
               if ( (pRoom = get_room_index(x) ) != NULL && pRoom->description[0] != '\0')
	       {   desc_area++;   count++; }

	    complete = aSort[i].max_vnum - aSort[i].min_vnum;
	    top_area +=  complete+1;

            /* Temporarily removed colour.  Too much information sent to the client =( - Marquoz
            bprintf( buf,
                 "{D[{w%3d{D] {W%-19.19s {D({w%-5d-%5d{D) {W%-10.10s {D[{w%d{D] {W[{w%-9.9s{W] {D[{w%-3d/%3d{D] {W[{w%7s{W]{x\n\r",
                 aSort[i].vnum, aSort[i].name, aSort[i].min_vnum, aSort[i].max_vnum,
                 aSort[i].file_name, aSort[i].security, aSort[i].builders, count, complete+1, cont_table[aSort[i].continent].name);
            */
            bprintf( buf,
                 "[%3d] %-19.19s (%-5d-%5d) %-10.10s [%d] [%-9.9s] [%-3d/%3d] [%7s]{x\n\r",
                 aSort[i].vnum, aSort[i].name, aSort[i].min_vnum, aSort[i].max_vnum,
                 aSort[i].file_name, aSort[i].security, aSort[i].builders, count, complete+1, cont_table[aSort[i].continent].name);
            send_to_char( buf->data, ch );
            /* Data sent, clear it out now, not later.  Not clearing it was causing
               a nice little spillover - Marquoz */
            buffer_clear(buf);	   
 
            count = 0;
    }

    printf_to_char(ch, "\n\rYou have %d rooms with descriptions. Out of a possible %d.\n\r",desc_area,top_area );

    /*  free memory */
    free((AREA_DATA *)aSort);
    buffer_free( buf );
    return;
}

void stop_editing (void *OLC)
{
    DESCRIPTOR_DATA *d;

    for (d = descriptor_list; d != NULL; d = d->next)
        {
            if (d->pEdit == NULL || d->character == NULL)
                continue;

            if (d->pEdit == (void *) OLC)
                edit_done (d->character);
        }
    return;
}    


void do_skedit( CHAR_DATA *ch, char *argument )
{
    int sn;
    char arg[MAX_STRING_LENGTH];

    arg[0] = '\0';

    if ( IS_NPC(ch) )
    	return;

	argument = one_argument(argument, arg);

	if(arg[0]  == '\0')
	{
		send_to_char("Syntax: Skedit <spell/skill>\n\r",ch);
		return;
	}

    if( (sn = skill_lookup(arg) ) < 0 )
	{
		send_to_char("What spell, or skill is this?\n\r",ch);
		return;
	}

	ch->desc->pEdit = (void *) (long int)sn;
	ch->desc->editor = ED_SKILL;
	printf_to_char(ch, "Entering skeditor for %s.\n\r", skill_table[sn].name );
	

    return;
}

void do_cmdedit( CHAR_DATA *ch, char *argument )
{
    CMD_DATA *cmd;
    char arg[MAX_STRING_LENGTH];

    arg[0] = '\0';

    if ( IS_NPC(ch) )
    	return;

	argument = one_argument(argument, arg);

	if(arg[0]  == '\0')
	{
		send_to_char("Syntax: Cmdedit <command>\n\r",ch);
		return;
	}
	if(!str_cmp(arg, "new" ) )
	{
		cmdedit_new(ch, argument);
		return;
	}
	if(!str_cmp(arg, "delete" ) )
	{
		cmdedit_delete(ch, argument );
		return;
	}
    if( (cmd = cmd_lookup(arg) ) == NULL )
	{
		send_to_char("What command is this?\n\r",ch);
		return;
	}

	ch->desc->pEdit = (void *) cmd;
	ch->desc->editor = ED_COMMAND;
	printf_to_char(ch, "Entering Command Editor for %s.\n\r", cmd->name );
	

    return;
}

void do_rlgedit( CHAR_DATA *ch, char *argument )
{	RELIGION *relg;
	char arg[MSL];

	if( IS_NPC(ch) )
		return;

	argument = one_argument(argument, arg);

	if( arg[0] == '\0' )
	{	send_to_char("Syntax: Rlgedit <religion>\n\r",ch);
		return;
	}
	if( !str_cmp(arg, "new" ) )
	{	rlgedit_new(ch, argument);
		return;
	}
	if( !str_cmp(arg, "delete" ) )
	{	rlgedit_delete(ch, argument );
		return;
	}
	if( !str_cmp(arg, "list" ) )
	{	rlgedit_list(ch, argument );
		return;
	}
	if( (relg = religion_lookup(arg) )== NULL )
	{	send_to_char("There is no religion by that name.\n\r",ch);
		return;
	}

	ch->desc->pEdit = (void * ) relg;
	ch->desc->editor = ED_RELIGION;
	printf_to_char(ch, "Entering the Religion editor for %s\n\r",relg->name );
	return;
}
