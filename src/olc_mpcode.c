/* The following code is based on ILAB OLC by Jason Dinkel */
/* Mobprogram code by Lordrom for Nevermore Mud */

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
#include "tables.h"
#include "olc.h"
#include "recycle.h"
#include "const.h"
void save_area args ( ( AREA_DATA *pArea ) ); 

#define MPEDIT( fun )           bool fun(CHAR_DATA *ch, char*argument)
#define OPEDIT( fun )		bool fun(CHAR_DATA *ch, char*argument)
#define RPEDIT( fun )		bool fun(CHAR_DATA *ch, char*argument)

const struct olc_cmd_type mpedit_table[] =
{
/*	{	command		function	}, */

	{	"commands",	show_commands	},
	{	"create",	mpedit_create	},
	{	"code",		mpedit_code	},
	{	"show",		mpedit_show	},
	{	"list",		mpedit_list	},
        {       "delete",       mpedit_delete   },
	{	"?",		show_help	},

	{	NULL,		0		}
};

const struct olc_cmd_type opedit_table[] =
{
/*	{	command		function	}, */

	{	"commands",	show_commands	},
	{	"create",	opedit_create	},
	{	"code",		opedit_code	},
	{	"show",		opedit_show	},
	{	"list",		opedit_list	},
        {       "delete",       opedit_delete   },
	{	"?",		show_help	},

	{	NULL,		0		}
};

const struct olc_cmd_type rpedit_table[] =
{
/*	{	command		function	}, */

	{	"commands",	show_commands	},
	{	"create",	rpedit_create	},
	{	"code",		rpedit_code	},
	{	"show",		rpedit_show	},
	{	"list",		rpedit_list	},
        {       "delete",       rpedit_delete   },
	{	"?",		show_help	},

	{	NULL,		0		}
};

void mpedit( CHAR_DATA *ch, char *argument)
{
    PROG_CODE *pMcode;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;
    AREA_DATA *ad;

    smash_tilde(argument);
    strcpy(arg, argument);
    argument = one_argument( argument, command);

    EDIT_MPCODE(ch, pMcode);

    if (pMcode)
    {
	ad = get_vnum_area( pMcode->vnum );

	if ( ad == NULL ) /* ??? */
	{
		edit_done(ch);
		return;
	}

	if ( !IS_BUILDER(ch, ad) )
	{
		send_to_char("MPEdit: Insufficient security to modify code.\n\r", ch);
		edit_done(ch);
		return;
	}
    }

    if (command[0] == '\0')
    {
        mpedit_show(ch, argument);
        return;
    }

    if (!str_cmp(command, "done") )
    {
        edit_done(ch);
        return;
    }

    for (cmd = 0; mpedit_table[cmd].name != NULL; cmd++)
    {
	if (!str_prefix(command, mpedit_table[cmd].name) )
	{
		if ((*mpedit_table[cmd].olc_fun) (ch, argument) && pMcode)
			if ((ad = get_vnum_area(pMcode->vnum)) != NULL)
				SET_BIT(ad->area_flags, AREA_CHANGED);
		return;
	}
    }

    interpret(ch, arg);

    return;
}

void opedit( CHAR_DATA *ch, char *argument)
{
    PROG_CODE *pOcode;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;
    AREA_DATA *ad;

    smash_tilde(argument);
    strcpy(arg, argument);
    argument = one_argument( argument, command);

    EDIT_OPCODE(ch, pOcode);

    if (pOcode)
    {
	ad = get_vnum_area( pOcode->vnum );

	if ( ad == NULL ) /* ??? */
	{
		edit_done(ch);
		return;
	}

	if ( !IS_BUILDER(ch, ad) )
	{
		send_to_char("OPEdit: Insufficient security to modify code.\n\r", ch);
		edit_done(ch);
		return;
	}
    }

    if (command[0] == '\0')
    {
        opedit_show(ch, argument);
        return;
    }

    if (!str_cmp(command, "done") )
    {
        edit_done(ch);
        return;
    }

    for (cmd = 0; opedit_table[cmd].name != NULL; cmd++)
    {
	if (!str_prefix(command, opedit_table[cmd].name) )
	{
		if ((*opedit_table[cmd].olc_fun) (ch, argument) && pOcode)
			if ((ad = get_vnum_area(pOcode->vnum)) != NULL)
				SET_BIT(ad->area_flags, AREA_CHANGED);
		return;
	}
    }

    interpret(ch, arg);

    return;
}

void rpedit( CHAR_DATA *ch, char *argument)
{
    PROG_CODE *pRcode;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;
    AREA_DATA *ad;

    smash_tilde(argument);
    strcpy(arg, argument);
    argument = one_argument( argument, command);

    EDIT_RPCODE(ch, pRcode);

    if (pRcode)
    {
	ad = get_vnum_area( pRcode->vnum );

	if ( ad == NULL ) /* ??? */
	{
		edit_done(ch);
		return;
	}

	if ( !IS_BUILDER(ch, ad) )
	{
		send_to_char("RPEdit: Insufficient security to modify code.\n\r", ch);
		edit_done(ch);
		return;
	}
    }

    if (command[0] == '\0')
    {
        rpedit_show(ch, argument);
        return;
    }

    if (!str_cmp(command, "done") )
    {
        edit_done(ch);
        return;
    }

    for (cmd = 0; rpedit_table[cmd].name != NULL; cmd++)
    {
	if (!str_prefix(command, rpedit_table[cmd].name) )
	{
		if ((*rpedit_table[cmd].olc_fun) (ch, argument) && pRcode)
			if ((ad = get_vnum_area(pRcode->vnum)) != NULL)
				SET_BIT(ad->area_flags, AREA_CHANGED);
		return;
	}
    }

    interpret(ch, arg);

    return;
}

void do_mpedit(CHAR_DATA *ch, char *argument)
{
    PROG_CODE *pMcode;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument(argument, command);

    if( is_number(command) )
    {
	int vnum = atoi(command);
	AREA_DATA *ad;

	if ( (pMcode = get_prog_index(vnum,PRG_MPROG)) == NULL )
	{
		send_to_char("MPEdit : That vnum does not exist.\n\r",ch);
		return;
	}

	ad = get_vnum_area(vnum);

	if ( ad == NULL )
	{
		send_to_char( "MPEdit : Vnum is not assigned an area.\n\r", ch );
		return;
	}

	if ( !IS_BUILDER(ch, ad) )
	{
		send_to_char("MPEdit : Insufficient security to modify area.\n\r", ch );
		return;
	}

	ch->desc->pEdit		= (void *)pMcode;
	ch->desc->editor	= ED_MPCODE;

	return;
    }

    if ( !str_cmp(command, "create") )
    {
	if (argument[0] == '\0')
	{
		send_to_char( "Syntax : mpedit create [vnum]\n\r", ch );
		return;
	}

	mpedit_create(ch, argument);
	return;
    }

    if (!str_cmp(command, "delete") )
    {
	if(argument[0] == '\0')
	{
	   send_to_char("Syntax: mpedit delete [vnum]\n\r", ch);
	   return;
	}
        sprintf ( log_buf, "$N deleting mprog '%s'.", argument );
        wiznet ( log_buf, ch, NULL, WIZ_OLC, WIZ_SECURE, get_trust ( ch ) );
        mpedit_delete ( ch, argument );
        return;
    }

    send_to_char( "Syntax : mpedit [vnum]\n\r", ch );
    send_to_char( "         mpedit create [vnum]\n\r", ch );

    return;
}

void do_opedit(CHAR_DATA *ch, char *argument)
{
    PROG_CODE *pOcode;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument(argument, command);

    if( is_number(command) )
    {
	int vnum = atoi(command);
	AREA_DATA *ad;

	if ( (pOcode = get_prog_index(vnum,PRG_OPROG)) == NULL )
	{
		send_to_char("OPEdit : That vnum does not exist.\n\r",ch);
		return;
	}

	ad = get_vnum_area(vnum);

	if ( ad == NULL )
	{
		send_to_char( "OPEdit : Vnum is not assigned an area.\n\r", ch );
		return;
	}

	if ( !IS_BUILDER(ch, ad) )
	{
		send_to_char("OPEdit : Insufficient security to modify area.\n\r", ch );
		return;
	}

	ch->desc->pEdit		= (void *)pOcode;
	ch->desc->editor	= ED_OPCODE;

	return;
    }

    if ( !str_cmp(command, "create") )
    {
	if (argument[0] == '\0')
	{
		send_to_char( "Syntax : opedit create [vnum]\n\r", ch );
		return;
	}

	opedit_create(ch, argument);
	return;
    }

    if (!str_cmp(command, "delete") )
    {
	if(argument[0] == '\0')
	{
  	   send_to_char("Syntax: opedit delete [vnum]\n\r", ch);
	   return;
	}
        sprintf ( log_buf, "$N deleting oprog '%s'.", argument );
        wiznet ( log_buf, ch, NULL, WIZ_OLC, WIZ_SECURE, get_trust ( ch ) );
        opedit_create ( ch, argument );
        return;
    }

    send_to_char( "Syntax : opedit [vnum]\n\r", ch );
    send_to_char( "         opedit create [vnum]\n\r", ch );

    return;
}

void do_rpedit(CHAR_DATA *ch, char *argument)
{
    PROG_CODE *pRcode;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument(argument, command);

    if( is_number(command) )
    {
	int vnum = atoi(command);
	AREA_DATA *ad;

	if ( (pRcode = get_prog_index(vnum,PRG_RPROG)) == NULL )
	{
		send_to_char("RPEdit : That vnum does not exist.\n\r",ch);
		return;
	}

	ad = get_vnum_area(vnum);

	if ( ad == NULL )
	{
		send_to_char( "RPEdit : Vnum is not assigned an area.\n\r", ch );
		return;
	}

	if ( !IS_BUILDER(ch, ad) )
	{
		send_to_char("RPEdit : Insufficient security to modify area.\n\r", ch );
		return;
	}

	ch->desc->pEdit		= (void *)pRcode;
	ch->desc->editor	= ED_RPCODE;

	return;
    }

    if ( !str_cmp(command, "create") )
    {
	if (argument[0] == '\0')
	{
		send_to_char( "Syntax : rpedit create [vnum]\n\r", ch );
		return;
	}

	rpedit_create(ch, argument);
	return;
    }

    if (!str_cmp(command, "delete") )
    {
	if(argument[0] == '\0')
	{
	   send_to_char("Syntax: rpedit delete [vnum]\n\r", ch);
	   return;
	}
        sprintf ( log_buf, "$N deleting rprog '%s'.", argument );
        wiznet ( log_buf, ch, NULL, WIZ_OLC, WIZ_SECURE, get_trust ( ch ) );
        rpedit_delete ( ch, argument );
        return;
    }

    send_to_char( "Syntax : rpedit [vnum]\n\r", ch );
    send_to_char( "         rpedit create [vnum]\n\r", ch );

    return;
}

MPEDIT (mpedit_create)
{
    PROG_CODE *pMcode;
    int value = atoi(argument);
    AREA_DATA *ad;

    if (IS_NULLSTR(argument) || value < 1)
    {
	send_to_char( "Syntax : mpedit create [vnum]\n\r", ch );
	return FALSE;
    }

    ad = get_vnum_area(value);

    if ( ad == NULL )
    {
    	send_to_char( "MPEdit : Vnum is not assigned an area.\n\r", ch );
    	return FALSE;
    }
    
    if ( !IS_BUILDER(ch, ad) )
    {
        send_to_char("MPEdit : Insufficient security to create MobProgs.\n\r", ch);
        return FALSE;
    }

    if ( get_prog_index(value,PRG_MPROG) )
    {
	send_to_char("MPEdit: Code vnum already exists.\n\r",ch);
	return FALSE;
    }

    pMcode			= new_mpcode();
    pMcode->vnum		= value;
    pMcode->next		= mprog_list;
    mprog_list			= pMcode;
    ch->desc->pEdit		= (void *)pMcode;
    ch->desc->editor		= ED_MPCODE;

    send_to_char("MobProgram Code Created.\n\r",ch);

    return TRUE;
}

OPEDIT (opedit_create)
{
    PROG_CODE *pOcode;
    int value = atoi(argument);
    AREA_DATA *ad;

    if (IS_NULLSTR(argument) || value < 1)
    {
	send_to_char( "Syntax : opedit create [vnum]\n\r", ch );
	return FALSE;
    }

    ad = get_vnum_area(value);

    if ( ad == NULL )
    {
    	send_to_char( "OPEdit : Vnum is not assigned an area.\n\r", ch );
    	return FALSE;
    }
    
    if ( !IS_BUILDER(ch, ad) )
    {
        send_to_char("OPEdit : Insufficient security to create ObjProgs.\n\r", ch);
        return FALSE;
    }

    if ( get_prog_index(value,PRG_OPROG) )
    {
	send_to_char("OPEdit: Code vnum already exists.\n\r",ch);
	return FALSE;
    }

    pOcode			= new_opcode();
    pOcode->vnum		= value;
    pOcode->next		= oprog_list;
    oprog_list			= pOcode;
    ch->desc->pEdit		= (void *)pOcode;
    ch->desc->editor		= ED_OPCODE;

    send_to_char("ObjProgram Code Created.\n\r",ch);

    return TRUE;
}

RPEDIT (rpedit_create)
{
    PROG_CODE *pRcode;
    int value = atoi(argument);
    AREA_DATA *ad;

    if (IS_NULLSTR(argument) || value < 1)
    {
	send_to_char( "Syntax : rpedit create [vnum]\n\r", ch );
	return FALSE;
    }

    ad = get_vnum_area(value);

    if ( ad == NULL )
    {
    	send_to_char( "RPEdit : Vnum is not assigned an area.\n\r", ch );
    	return FALSE;
    }
    
    if ( !IS_BUILDER(ch, ad) )
    {
        send_to_char("RPEdit : Insufficient security to create RoomProgs.\n\r", ch);
        return FALSE;
    }

    if ( get_prog_index(value,PRG_RPROG) )
    {
	send_to_char("RPEdit: Code vnum already exists.\n\r",ch);
	return FALSE;
    }

    pRcode			= new_rpcode();
    pRcode->vnum		= value;
    pRcode->next		= rprog_list;
    rprog_list			= pRcode;
    ch->desc->pEdit		= (void *)pRcode;
    ch->desc->editor		= ED_RPCODE;

    send_to_char("RoomProgram Code Created.\n\r",ch);

    return TRUE;
}

MPEDIT(mpedit_show)
{
    PROG_CODE *pMcode;
    char buf[MAX_STRING_LENGTH];

    EDIT_MPCODE(ch,pMcode);

    sprintf(buf,
           "Vnum:       [%d]\n\r"
           "Code:\n\r%s\n\r",
           pMcode->vnum, pMcode->code);
    send_to_char(buf, ch);

    return FALSE;
}

OPEDIT(opedit_show)
{
    PROG_CODE *pOcode;
    char buf[MAX_STRING_LENGTH];

    EDIT_OPCODE(ch,pOcode);

    sprintf(buf,
           "Vnum:       [%d]\n\r"
           "Code:\n\r%s\n\r",
           pOcode->vnum, pOcode->code);
    send_to_char(buf, ch);

    return FALSE;
}

RPEDIT(rpedit_show)
{
    PROG_CODE *pRcode;
    char buf[MAX_STRING_LENGTH];

    EDIT_RPCODE(ch,pRcode);

    sprintf(buf,
           "Vnum:       [%d]\n\r"
           "Code:\n\r%s\n\r",
           pRcode->vnum, pRcode->code);
    send_to_char(buf, ch);

    return FALSE;
}

MPEDIT(mpedit_code)
{
    PROG_CODE *pMcode;
    EDIT_MPCODE(ch, pMcode);

    if (argument[0] =='\0')
    {
       string_append(ch, &pMcode->code);
       return TRUE;
    }

    send_to_char("Syntax: code\n\r",ch);
    return FALSE;
}

OPEDIT(opedit_code)
{
    PROG_CODE *pOcode;
    EDIT_OPCODE(ch, pOcode);

    if (argument[0] =='\0')
    {
       string_append(ch, &pOcode->code);
       return TRUE;
    }

    send_to_char("Syntax: code\n\r",ch);
    return FALSE;
}

RPEDIT(rpedit_code)
{
    PROG_CODE *pRcode;
    EDIT_RPCODE(ch, pRcode);

    if (argument[0] =='\0')
    {
       string_append(ch, &pRcode->code);
       return TRUE;
    }

    send_to_char("Syntax: code\n\r",ch);
    return FALSE;
}

MPEDIT( mpedit_list )
{
    int count = 1;
    PROG_CODE *mprg;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    bool fAll = !str_cmp(argument, "all");
    char blah;
    AREA_DATA *ad;

    buffer = new_buf();

    for (mprg = mprog_list; mprg !=NULL; mprg = mprg->next)
	if ( fAll || ENTRE(ch->in_room->area->min_vnum, mprg->vnum, ch->in_room->area->max_vnum) )
	{
		ad = get_vnum_area(mprg->vnum);

		if ( ad == NULL )
			blah = '?';
		else
		if ( IS_BUILDER(ch, ad) )
			blah = '*';
		else
			blah = ' ';

		sprintf(buf, "[%3d] (%c) %5d\n\r", count, blah, mprg->vnum );
		add_buf(buffer, buf);

		count++;
	}

    if ( count == 1 )
    {
    	if ( fAll )
    		add_buf( buffer, "No existing MobPrograms.\n\r" );
    	else
    		add_buf( buffer, "No existing MobPrograms in this area.\n\r" );
    }

    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);

    return FALSE;
}

OPEDIT( opedit_list )
{
    int count = 1;
    PROG_CODE *oprg;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    bool fAll = !str_cmp(argument, "all");
    char blah;
    AREA_DATA *ad;

    buffer = new_buf();

    for (oprg = oprog_list; oprg !=NULL; oprg = oprg->next)
	if ( fAll || ENTRE(ch->in_room->area->min_vnum, oprg->vnum, ch->in_room->area->max_vnum) )
	{
		ad = get_vnum_area(oprg->vnum);

		if ( ad == NULL )
			blah = '?';
		else
		if ( IS_BUILDER(ch, ad) )
			blah = '*';
		else
			blah = ' ';

		sprintf(buf, "[%3d] (%c) %5d\n\r", count, blah, oprg->vnum );
		add_buf(buffer, buf);

		count++;
	}

    if ( count == 1 )
    {
    	if ( fAll )
    		add_buf( buffer, "No existing ObjPrograms.\n\r" );
    	else
    		add_buf( buffer, "No existing ObjPrograms in this area.\n\r" );
    }

    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);

    return FALSE;
}

RPEDIT( rpedit_list )
{
    int count = 1;
    PROG_CODE *rprg;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    bool fAll = !str_cmp(argument, "all");
    char blah;
    AREA_DATA *ad;

    buffer = new_buf();

    for (rprg = rprog_list; rprg !=NULL; rprg = rprg->next)
	if ( fAll || ENTRE(ch->in_room->area->min_vnum, rprg->vnum, ch->in_room->area->max_vnum) )
	{
		ad = get_vnum_area(rprg->vnum);

		if ( ad == NULL )
			blah = '?';
		else
		if ( IS_BUILDER(ch, ad) )
			blah = '*';
		else
			blah = ' ';

		sprintf(buf, "[%3d] (%c) %5d\n\r", count, blah, rprg->vnum );
		add_buf(buffer, buf);

		count++;
	}

    if ( count == 1 )
    {
    	if ( fAll )
    		add_buf( buffer, "No existing RoomPrograms.\n\r" );
    	else
    		add_buf( buffer, "No existing RoomPrograms in this area.\n\r" );
    }

    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);

    return FALSE;
}


void unlink_mprog(long pnum)
{
	PROG_LIST *list, *list_next;
	MOB_INDEX_DATA *pMob;
	long vnum = 0;
	int nMatch = 0, count, pos = 0;

	for(vnum = 0 ; nMatch < top_mob_index; vnum++)
	{
		if((pMob = get_mob_index(vnum)) == NULL)
			continue;

		nMatch++;
		count = -1;
		for ( list = pMob->mprogs; list != NULL; list = list_next )
		{
			list_next = list->next;
			count++;

			/* Important line */
			if(list->vnum != pnum)
				continue;

			/* Next part taken from medit_delprog */
			if(count == 0)
			{
				REMOVE_BIT ( pMob->mprog_flags, pMob->mprogs->trig_type );
				list = pMob->mprogs;
				pMob->mprogs = list->next;
				free_mprog ( list );
			}
			else
			{
				while ( ( list_next = list->next ) && ( ++pos < count ) )
					list = list_next;

				if ( list_next )
				{
					REMOVE_BIT ( pMob->mprog_flags, list_next->trig_type );
					list->next = list_next->next;
					free_mprog ( list_next );
				}
			}
		}
	}
}

void unlink_oprog(long pnum)
{
	PROG_LIST *list, *list_next;
	OBJ_INDEX_DATA *pObj;
	long vnum = 0;
	int nMatch = 0, count, pos = 0;

	for(vnum = 0 ; nMatch < top_obj_index; vnum++)
	{
		if((pObj = get_obj_index(vnum)) == NULL)
			continue;

		nMatch++;
		count = -1;
		for ( list = pObj->oprogs; list != NULL; list = list_next )
		{
			list_next = list->next;
			count++;

			if(list->vnum != pnum)
				continue;

			if(count == 0)
			{
				REMOVE_BIT ( pObj->oprog_flags, pObj->oprogs->trig_type );
				list = pObj->oprogs;
				pObj->oprogs = list->next;
				free_oprog ( list );
			}
			else
			{
				while ( ( list_next = list->next ) && ( ++pos < count ) )
					list = list_next;

				if ( list_next )
				{
					REMOVE_BIT ( pObj->oprog_flags, list_next->trig_type );
					list->next = list_next->next;
					free_oprog ( list_next );
				}
			}
		}
	}
}

void unlink_rprog(long pnum)
{
	PROG_LIST *list, *list_next;
	ROOM_INDEX_DATA *pRoom;
	long vnum = 0;
	int nMatch = 0, count, pos = 0;

	for(vnum = 0 ; nMatch < top_room; vnum++)
	{
		if((pRoom = get_room_index(vnum)) == NULL)
			continue;

		nMatch++;
		count = -1;
		for ( list = pRoom->rprogs; list != NULL; list = list_next )
		{
			list_next = list->next;
			count++;

			if(list->vnum != pnum)
				continue;

			if(count == 0)
			{
				REMOVE_BIT ( pRoom->rprog_flags, pRoom->rprogs->trig_type );
				list = pRoom->rprogs;
				pRoom->rprogs = list->next;
				free_rprog ( list );
			}
			else
			{
				while ( ( list_next = list->next ) && ( ++pos < count ) )
					list = list_next;

				if ( list_next )
				{
					REMOVE_BIT ( pRoom->rprog_flags, list_next->trig_type );
					list->next = list_next->next;
					free_rprog ( list_next );
				}
			}
		}
	}
}


MPEDIT ( mpedit_delete )
{
    PROG_CODE *curr, *prev, *temp;
    long value = atol ( argument );
	AREA_DATA *ad;

    if ( IS_NULLSTR ( argument ) || value < 1 )
    {
        send_to_char( "Syntax : mpedit create [vnum]\n\r", ch );
        return FALSE;
    }
	
    if ( get_prog_index ( value, PRG_MPROG ) == NULL )
    {
        send_to_char( "MPEdit : Mob program not found.\n\r", ch );
        return FALSE;
    }

	ad = get_vnum_area(value);
	if(ad == NULL)
	{
		send_to_char("MPEdit : Mob program not assigned to an area.\n\r", ch);
		return FALSE;
	}

    if (ch && !IS_BUILDER ( ch, ad ) )
    {
        send_to_char( "MPEdit : Insufficient security to create MobProgs.\n\r",
                 ch );
        return FALSE;
    }

	unlink_mprog(value);
    prev = NULL;
    for ( curr = mprog_list; curr != NULL; prev = curr, curr = temp )
    {
		temp = curr->next;
		if(curr->vnum != value)
			continue;
		if ( prev == NULL )
			mprog_list = mprog_list->next;
		else
			prev->next = curr->next;

		free_mpcode ( curr );
		save_area(ad);
	}
   if(ch)
    send_to_char( "MobProgram Code Deleted.\n\r", ch );

    return TRUE;
}

OPEDIT ( opedit_delete )
{
    PROG_CODE *curr, *prev, *temp;
    long value = atol ( argument );
	AREA_DATA *ad;

    if ( IS_NULLSTR ( argument ) || value < 1 )
    {
        send_to_char( "Syntax : opedit create [vnum]\n\r", ch );
        return FALSE;
    }

    if ( get_prog_index ( value, PRG_OPROG ) == NULL )
    {
        send_to_char( "OPEdit : Obj program not found.\n\r", ch );
        return FALSE;
    }

	ad = get_vnum_area(value);
	if(ad == NULL)
	{
		send_to_char("OPEdit : Obj program not assigned to an area.\n\r", ch);
		return FALSE;
	}

    if ( ch && !IS_BUILDER ( ch, ad ) )
    {
        send_to_char( "OPEdit : Insufficient security to create ObjProgs.\n\r",
                 ch );
        return FALSE;
    }

	unlink_oprog(value);
    prev = NULL;
    for ( curr = oprog_list; curr != NULL; prev = curr, curr = temp )
    {
		temp = curr->next;
		if(curr->vnum != value)
			continue;

		if ( prev == NULL )
			oprog_list = oprog_list->next;
		else
			prev->next = curr->next;

		free_mpcode ( curr );
		save_area(ad);
	}
    if(ch)
    send_to_char( "Obj Program Code Deleted.\n\r", ch );

    return TRUE;
}

RPEDIT ( rpedit_delete )
{
    PROG_CODE *curr, *prev, *temp;
    long value = atol ( argument );
	AREA_DATA *ad;

    if ( IS_NULLSTR ( argument ) || value < 1 )
    {
        send_to_char( "Syntax : rpedit create [vnum]\n\r", ch );
        return FALSE;
    }

    if ( get_prog_index ( value, PRG_RPROG ) == NULL )
    {
        send_to_char( "RPEdit : Room program not found.\n\r", ch );
        return FALSE;
    }

	ad = get_vnum_area(value);
	if(ad == NULL)
	{    
            send_to_char("RPEdit : Room program not assigned to an area.\n\r", ch);
   	    return FALSE;
	}

    if (ch && !IS_BUILDER ( ch, ad ) )
    {
        send_to_char( "RPEdit : Insufficient security to create RoomProgs.\n\r",
                 ch );
        return FALSE;
    }

	unlink_rprog(value);
    prev = NULL;
    for ( curr = rprog_list; curr != NULL; prev = curr, curr = temp )
    {
		temp = curr->next;
		if(curr->vnum != value)
			continue;
		if ( prev == NULL )
			rprog_list = rprog_list->next;
		else
			prev->next = curr->next;

		free_mpcode ( curr );
		save_area(ad);
	}
	if(ch)
        send_to_char( "Room Program Code Deleted.\n\r", ch );

    return TRUE;
}



