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

/***********************************************************************
 * Award  was written by Robbert of The Inquisition MUD.  Use of this  *
 * code is held to the same licensing agreements of ROM, Diku, et al.  *
 * If you use this code, either in its entirety or as a basis for      *
 * something of your own, this header must be included with the file.  *
 ***********************************************************************/   

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "include.h"


/*
 * Local functions.
 */
ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );
bool    write_to_descriptor     args( ( int desc, char *txt, int length ) );
bool    write_to_descriptor_2   args( ( int desc, char *txt, int length ) );
int     close           args( ( int fd ) );   
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );                               
bool    check_parse_name        args( ( char *name ) ); 
bool    double_exp = FALSE;
bool    double_qp = FALSE;
bool    quad_damage = FALSE;
bool    timed_copyover = FALSE;
bool    treboot = FALSE;
bool    tshutdown = FALSE;
extern AFFECT_DATA * affect_free;
CHAR_DATA *copyover_person;
void    mobile_update   args( ( void ) );
void    weather_update  args( ( void ) );
void    char_update     args( ( void ) );
void    obj_update      args( ( void ) );
void    aggr_update     args( ( void ) );
void    quest_update    args( ( void ) );
void    auction_update  args( ( void ) );
extern  void    fread_char      args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fwrite_obj      args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
                            FILE *fp, int iNest ) );
void    fread_obj       args( ( CHAR_DATA *ch,  FILE *fp ) );   
void load_copyover_obj args ( (void) );
sh_int  max_unique;
bool    unique_set = FALSE;
bool    isignal = TRUE;
bool    quiet = FALSE;
bool    pshow = FALSE;

DECLARE_DO_FUN( do_wset		);

CHAR_DATA *get_char( CHAR_DATA *ch )
{
    if ( !ch->pcdata )
        return ch->desc->original;
    else
        return ch;
}

void do_wiznet( CHAR_DATA *ch, char *argument )
{
   int flag;
   char buf[MAX_STRING_LENGTH];
   int col = 0;
                
   if ( argument[0] == '\0' )
   {
        send_to_char("{GWELCOME TO WIZNET!!!{x\n\r", ch);
        send_to_char("   Option      Status\n\r",ch);
        send_to_char("---------------------\n\r",ch);
        /* list of all wiznet options */
        buf[0] = '\0';
        
        for (flag = 0; wiznet_table[flag].name != NULL; flag++)
        {
            if (wiznet_table[flag].level <= get_trust(ch))
            {
                sprintf( buf, "%-14s %s\t", wiznet_table[flag].name,
IS_SET(ch->wiznet,wiznet_table[flag].flag) ? "{RON{V" : "OFF" );
                send_to_char(buf, ch);   
                col++;
                if (col==3)
                {
                  send_to_char("\n\r",ch);
                  col=0;
                }
            }
        }
/* To avoid color bleeding */
     send_to_char("{x",ch);
     return;
   }    
 
   if (!str_prefix(argument,"on"))
   {     
        send_to_char("{VWelcome to Wiznet!{x\n\r",ch);
        SET_BIT(ch->wiznet,WIZ_ON);
        return;
   }
                
   if (!str_prefix(argument,"off"))
   {
        send_to_char("{VSigning off of Wiznet.{x\n\r",ch);
        REMOVE_BIT(ch->wiznet,WIZ_ON);
        return;
   }
        
   flag = wiznet_lookup(argument);
        
   if (flag == -1 || get_trust(ch) < wiznet_table[flag].level) 
   {
        send_to_char("{VNo such option.{x\n\r",ch);
        return;
   }

   if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
   {
        sprintf(buf,"{VYou will no longer see %s on wiznet.{x\n\r",
                wiznet_table[flag].name);
        send_to_char(buf,ch);
        REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
        return;
   }
   else  
   {
        sprintf(buf,"{VYou will now see %s on wiznet.{x\n\r",
                wiznet_table[flag].name);
        send_to_char(buf,ch);
        SET_BIT(ch->wiznet,wiznet_table[flag].flag);
        return;
   }
}

void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	    long flag, long flag_skip, int min_level) 
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->connected == CON_PLAYING
	&&  IS_IMMORTAL(d->character) 
	&&  IS_SET(d->character->wiznet,WIZ_ON) 
	&&  (!flag || IS_SET(d->character->wiznet,flag))
	&&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  d->character != ch )
        {
	    if (IS_SET(d->character->wiznet,WIZ_PREFIX))
	  	send_to_char("{Z--> ",d->character);
	    else
	  	send_to_char( "{Z", d->character );
            act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD);
	    send_to_char( "{x", d->character );
        }
    }
 
    return;
}

void do_wiznetall (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Sorry mobs don't need wiznet!\n\r", ch );
        return;
    }

    if (arg[0] == '\0')
    {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  Wiznetall <on/off>\n\r", ch);
        send_to_char ("  On turns all Wiznet Options on\n\r", ch);
        send_to_char ("  Off turns all Wiznet Options off\n\r", ch);
        return;
    }

    if (!str_cmp (arg, "on") )
    {
	SET_BIT(ch->wiznet,WIZ_ON);
	SET_BIT(ch->wiznet,WIZ_PREFIX);
	SET_BIT(ch->wiznet,WIZ_TICKS);
	SET_BIT(ch->wiznet,WIZ_LOGINS);
	SET_BIT(ch->wiznet,WIZ_SITES);
	SET_BIT(ch->wiznet,WIZ_LINKS);
	SET_BIT(ch->wiznet,WIZ_NEWBIE);
	SET_BIT(ch->wiznet,WIZ_SPAM);
	SET_BIT(ch->wiznet,WIZ_DEATHS);
	SET_BIT(ch->wiznet,WIZ_FLAGS);
	SET_BIT(ch->wiznet,WIZ_PENALTIES);
	SET_BIT(ch->wiznet,WIZ_LEVELS);
	SET_BIT(ch->wiznet,WIZ_LOAD);
	SET_BIT(ch->wiznet,WIZ_RESTORE);
	SET_BIT(ch->wiznet,WIZ_SNOOPS);
	SET_BIT(ch->wiznet,WIZ_SWITCHES);
	SET_BIT(ch->wiznet,WIZ_SECURE);
        SET_BIT(ch->wiznet,WIZ_OLC);
        SET_BIT(ch->wiznet,WIZ_MOBDEATHS);

	send_to_char("{YAll Wiznet Options turned {Gon{x.{x\n\r",ch);

        return;
    }

    if (!str_cmp (arg, "off"))
    {

       	REMOVE_BIT(ch->wiznet,WIZ_ON);
	REMOVE_BIT(ch->wiznet,WIZ_PREFIX);
	REMOVE_BIT(ch->wiznet,WIZ_TICKS);
	REMOVE_BIT(ch->wiznet,WIZ_LOGINS);
	REMOVE_BIT(ch->wiznet,WIZ_SITES);
	REMOVE_BIT(ch->wiznet,WIZ_LINKS);
	REMOVE_BIT(ch->wiznet,WIZ_NEWBIE);
	REMOVE_BIT(ch->wiznet,WIZ_SPAM);
	REMOVE_BIT(ch->wiznet,WIZ_DEATHS);
	REMOVE_BIT(ch->wiznet,WIZ_FLAGS);
	REMOVE_BIT(ch->wiznet,WIZ_PENALTIES);
	REMOVE_BIT(ch->wiznet,WIZ_LEVELS);
	REMOVE_BIT(ch->wiznet,WIZ_LOAD);
	REMOVE_BIT(ch->wiznet,WIZ_RESTORE);
	REMOVE_BIT(ch->wiznet,WIZ_SNOOPS);
	REMOVE_BIT(ch->wiznet,WIZ_SWITCHES);
	REMOVE_BIT(ch->wiznet,WIZ_SECURE);
        REMOVE_BIT(ch->wiznet,WIZ_OLC);
        REMOVE_BIT(ch->wiznet,WIZ_MOBDEATHS);
        return;
    }

    do_function (ch, &do_wiznetall, "");

}

/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Nochannel whom?", ch );
        return;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    {
        REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "The gods have restored your channel priviliges.\n\r", 
		      victim );
        send_to_char( "NOCHANNELS removed.\n\r", ch );
	sprintf(buf,"$N restores channels to %s",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "The gods have revoked your channel priviliges.\n\r", 
		       victim );
        send_to_char( "NOCHANNELS set.\n\r", ch );
	sprintf(buf,"$N revokes %s's channels.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
 
    return;
}


void do_smote(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char *letter,*name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    size_t matches = 0;
 
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        send_to_char( "You can't show your emotions.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }
    
    if (strstr(argument,ch->name) == NULL)
    {
	send_to_char("You must include your name in an smote.\n\r",ch);
	return;
    }
   
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
 
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->desc == NULL || vch == ch)
            continue;
 
        if ((letter = strstr(argument,vch->name)) == NULL)
        {
	    send_to_char(argument,vch);
	    send_to_char("\n\r",vch);
            continue;
        }
 
        strcpy(temp,argument);
        temp[strlen(argument) - strlen(letter)] = '\0';
        last[0] = '\0';
        name = vch->name;
 
        for (; *letter != '\0'; letter++)
        {
            if (*letter == '\'' && matches == strlen(vch->name))
            {
                strcat(temp,"r");
                continue;
            }
 
            if (*letter == 's' && matches == strlen(vch->name))
            {
                matches = 0;
                continue;
            }
 
            if (matches == strlen(vch->name))
            {
                matches = 0;
            }
 
            if (*letter == *name)
            {
                matches++;
                name++;
                if (matches == strlen(vch->name))
                {
                    strcat(temp,"you");
                    last[0] = '\0';
                    name = vch->name;
                    continue;
                }
                strncat(last,letter,1);
                continue;
            }
 
            matches = 0;
            strcat(temp,last);
            strncat(temp,letter,1);
            last[0] = '\0';
            name = vch->name;
        }
 
	send_to_char(temp,vch);
	send_to_char("\n\r",vch);
    }
 
    return;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,ch->name) == NULL)
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}
	     
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

        sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->bamfin);
        send_to_char(buf,ch);
    }
    return;
}

void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
 
        if (argument[0] == '\0')
        {
            sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->bamfout);
            send_to_char(buf,ch);
            return;
        }
 
        if ( strstr(argument,ch->name) == NULL)
        {
            send_to_char("You must include your name.\n\r",ch);
            return;
        }
 
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
 
        sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->bamfout);
        send_to_char(buf,ch);
    }
    return;
}



void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    sprintf(buf,"$N denies access to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( "OK.\n\r", ch );
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
    do_function(victim, &do_quit, "" );

    return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if (is_number(arg))
    {
	int desc;

	desc = atoi(arg);
    	for ( d = descriptor_list; d != NULL; d = d->next )
    	{
            if ( d->descriptor == desc )
            {
            	close_socket( d );
            	send_to_char( "Ok.\n\r", ch );
            	return;
            }
	}
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}



void do_pardon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "killer" ) )
    {
	if ( IS_SET(victim->act, PLR_KILLER) )
	{
	    REMOVE_BIT( victim->act, PLR_KILLER );
	    send_to_char( "Killer flag removed.\n\r", ch );
	    send_to_char( "You are no longer a KILLER.\n\r", victim );
	}
	return;
    }

    if ( !str_cmp( arg2, "thief" ) )
    {
	if ( IS_SET(victim->act, PLR_THIEF) )
	{
	    REMOVE_BIT( victim->act, PLR_THIEF );
	    send_to_char( "Thief flag removed.\n\r", ch );
	    send_to_char( "You are no longer a THIEF.\n\r", victim );
	}
	return;
    }

    send_to_char( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
    return;
}



void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    if(ch)
    {
    if ( argument[0] == '\0' )
    {
	send_to_char( "Global echo what?\n\r", ch );
	return;
    }
    }
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (ch && get_trust(d->character) >= get_trust(ch))
		send_to_char( "global> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}



void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Local echo what?\n\r", ch );

	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
            if (get_trust(d->character) >= get_trust(ch))
                send_to_char( "local> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_zecho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("Zone echo what?\n\r",ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING
	&&  d->character->in_room != NULL && ch->in_room != NULL
	&&  d->character->in_room->area == ch->in_room->area)
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char("zone> ",d->character);
	    send_to_char(argument,d->character);
	    send_to_char("\n\r",d->character);
	}
    }
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
	send_to_char("Personal echo what?\n\r", ch); 
	return;
    }
   
    if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
	send_to_char("Target not found.\n\r",ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
        send_to_char( "personal> ",victim);

    send_to_char(argument,victim);
    send_to_char("\n\r",victim);
    send_to_char( "personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if(!IS_NPC(ch) && IN_MINE(ch) )
    {	send_to_char("Exit the mine if you want to trans someone.\n\r",ch);
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_function(ch, &do_transfer, buf );
	    }
	}
	return;
    }
    
    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( !is_room_owner(ch,location) && room_is_private( location ) 
	&&  get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    if(!IS_NPC(victim ) && IN_MINE(victim) )
    	exit_mine(victim);
    
    if(  ch->level < victim->level || (is_admin(victim) && !is_admin(ch)))
    {
	send_to_char("I don't think so.\n\r",ch);
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );

    if( MOUNTED(victim) )
    {
        char_from_room( MOUNTED(victim) );
        char_to_room( MOUNTED(victim), location );
        send_to_char("Your rider is being transferred, and so are you.\n\r", MOUNTED(victim));
    }

    if (ch->pet != NULL)
    {
    	char_from_room (ch->pet);
	    char_to_room (ch->pet, location);
    }    
    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_function(victim, &do_look, "auto" );
    send_to_char( "Ok.\n\r", ch );
}



void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && room_is_private( location ) 
    &&  get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    ch->on = on;
	    break;
	}
    }

    return;
}



void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    char arg[MSL];
    int count = 0;

    if ( argument[0] == '\0' && ch->gpoint <= 0 )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }

    if ( ( argument[0] == '\0' ) && ch->gpoint >= 0 )
        sprintf(arg, "%d", ch->gpoint);
    else
        sprintf(arg, "%s", argument);

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    count = 0;
    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
        count++;

    if (!is_room_owner(ch,location) && room_is_private(location) 
    &&  (count > 1 || get_trust(ch) < MAX_LEVEL))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }

    char_from_room( ch );
    char_to_room( ch, location );

    if (ch->pet != NULL)
    {
    	char_from_room (ch->pet);
	char_to_room (ch->pet, location);
    }

    if( MOUNTED(ch) )
    {
        char_from_room( MOUNTED(ch) );
        char_to_room( MOUNTED(ch), location );
        send_to_char("Your rider is a god, and did a goto. You went along for the ride.\n\r", MOUNTED(ch));
    }   


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    do_function(ch, &do_look, "auto" );
    return;
}

void do_violate( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Goto where?\n\r", ch );
        return;
    }
 
    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if (!room_is_private( location ))
    {
        send_to_char( "That room isn't private, use goto.\n\r", ch );
        return;
    }
 
    if ( ch->fighting != NULL )
        stop_fighting( ch, TRUE );
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
            else
                act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    char_from_room( ch );
    char_to_room( ch, location );
 
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    do_function(ch, &do_look, "auto" );
    return;
}

/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
 	send_to_char("  stat room <number>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_function(ch, &do_rstat, string);
	return;
   }
  
   if (!str_cmp(arg,"obj"))
   {
	do_function(ch, &do_ostat, string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_function(ch, &do_mstat, string);
	return;
   }
   
   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != NULL)
   {

     do_function(ch, &do_ostat, argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_function(ch, &do_mstat, argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_function(ch, &do_rstat, argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;


    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private( location ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: '%s'\n\rArea: '%s'\n\r",
	location->name,
	location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
	location->vnum,
	location->sector_type,
	location->light,
	location->heal_rate,
	location->mana_rate );
    send_to_char( buf, ch );

    sprintf( buf,
	"Room flags: %d.\n\rDescription:\n\r%s",
	location->room_flags,
	location->description );
    send_to_char( buf, ch );

    if (ch->in_room->affected_by)
    {
        sprintf(buf, "Affected by %s\n\r",
            raffect_bit_name(ch->in_room->affected_by));
        send_to_char(buf,ch);
    }  

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (can_see(ch,rch))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	    	pexit->key,
	    	pexit->exit_info,
	    	pexit->keyword,
	    	pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }

    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    sprintf( buf, "Name(s): %s Count: %d\n\r",
	obj->name, obj->pIndexData->count );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d Size: %s\n\r",
	obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	item_name(obj->item_type),  obj->pIndexData->reset_num, (obj->size >=0 && obj->size <= 5) ? size_table[obj->size].name : "unknown");
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\rExtra2 bits: %s\n\r",
	wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ), extra2_bit_name( obj->extra2_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ),get_true_weight(obj) );
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
	obj->level, obj->cost, obj->condition, obj->timer );
    send_to_char( buf, ch );

    sprintf( buf,
	"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : 
	    can_see(ch,obj->carried_by) ? obj->carried_by->name
				 	: "someone",
	obj->wear_loc );
    send_to_char( buf, ch );

    sprintf( buf, "Orig Values: %d %d %d %d %d\n\r",
        obj->valueorig[0], obj->valueorig[1], obj->valueorig[2], obj->valueorig[3],
        obj->valueorig[4] );
    send_to_char( buf, ch );     
    
    sprintf( buf, "Values: %d %d %d %d %d\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4] );
    send_to_char( buf, ch );
    
	if ( IS_OBJ2_STAT(obj, ITEM_RELIC))
    {	printf_to_char(ch, "Exp TNL: %d\n\r", obj->xp_tolevel );
		printf_to_char(ch, "Exp: %d\n\r", obj->exp );
	}
    /* now give out vital statistics as per identify */
    
    switch ( obj->item_type )
    {
    	case ITEM_SCROLL: 
    	case ITEM_POTION:
    	case ITEM_PILL:
	    sprintf( buf, "Level %d spells of:", obj->value[0] );
	    send_to_char( buf, ch );

	    if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[1]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[2]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	    {
		send_to_char(" '",ch);
		send_to_char(skill_table[obj->value[4]].name,ch);
		send_to_char("'",ch);
	    }

	    send_to_char( ".\n\r", ch );
	break;

    	case ITEM_WAND: 
    	case ITEM_STAFF: 
	    sprintf( buf, "Has %d(%d) charges of level %d",
	    	obj->value[1], obj->value[2], obj->value[0] );
	    send_to_char( buf, ch );
      
	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    send_to_char( ".\n\r", ch );
	break;

	case ITEM_DRINK_CON:
	    sprintf(buf,"It holds %s-colored %s.\n\r",
		liq_table[obj->value[2]].liq_color,
		liq_table[obj->value[2]].liq_name);
	    send_to_char(buf,ch);
	    break;
		
      
    	case ITEM_WEAPON:
 	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0])
	    {
	    	case(WEAPON_EXOTIC): 
		    send_to_char("exotic\n\r",ch);
		    break;
	    	case(WEAPON_SWORD): 
		    send_to_char("sword\n\r",ch);
		    break;	
	    	case(WEAPON_DAGGER): 
		    send_to_char("dagger\n\r",ch);
		    break;
	    	case(WEAPON_SPEAR):
		    send_to_char("spear/staff\n\r",ch);
		    break;
	    	case(WEAPON_MACE): 
		    send_to_char("mace/club\n\r",ch);	
		    break;
	   	case(WEAPON_AXE): 
		    send_to_char("axe\n\r",ch);	
		    break;
	    	case(WEAPON_FLAIL): 
		    send_to_char("flail\n\r",ch);
		    break;
	    	case(WEAPON_WHIP): 
		    send_to_char("whip\n\r",ch);
		    break;
	    	case(WEAPON_POLEARM): 
		    send_to_char("polearm\n\r",ch);
		    break;
                case(WEAPON_BOW):
                    send_to_char("bow\n\r",ch);
                    break;
	    	default: 
		    send_to_char("unknown\n\r",ch);
		    break;
 	    }
	    if (obj->pIndexData->new_format)
            {
	    	sprintf(buf,"Damage is %dd%d (average %d)\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
                send_to_char( buf, ch );
             
                sprintf( buf, "Min damage: %d Max damage: %d\n\r",
                obj->value[1], obj->value[1] * obj->value[2] );
            }

	    else
	    	sprintf( buf, "Damage is %d to %d (average %d)\n\r",
	    	    obj->value[1], obj->value[2],
	    	    ( obj->value[1] + obj->value[2] ) / 2 );
	    send_to_char( buf, ch );

	    sprintf(buf,"Damage noun is %s.\n\r",
		(obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ?
		    attack_table[obj->value[3]].noun : "undefined");
	    send_to_char(buf,ch);
	    
	    if (obj->value[4])  /* weapon flags */
	    {
	        sprintf(buf,"Weapons flags: %s\n\r",
		    weapon_bit_name(obj->value[4]));
	        send_to_char(buf,ch);
            }
	break;

    	case ITEM_ARMOR:
	    sprintf( buf, 
	    "Armor klass is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
	        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    send_to_char( buf, ch );
	break;

        case ITEM_QUIVER:
		sprintf(buf,"{WIt holds {R%d %d{Wd{R%d {Warrows.{x\n\r",
		obj->value[0], obj->value[1], obj->value[2]);
		send_to_char(buf,ch);
		break;
	case ITEM_ARROW:
		sprintf(buf,"{WThis arrow will do {R%d{Wd{R%d {Wdamage for an average of {R%d{W.{x\n\r",
		obj->value[1], obj->value[2], ( obj->value[1] + obj->value[2] ) / 2 );
		send_to_char(buf,ch);
		break;

        case ITEM_CONTAINER:
            sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
            send_to_char(buf,ch);
            if (obj->value[4] != 100)
            {
                sprintf(buf,"Weight multiplier: %d%%\n\r",
		    obj->value[4]);
                send_to_char(buf,ch);
            }
        break;

       case ITEM_SOCKETS: 
     send_to_char("Gem type is: ",ch); 
     switch (obj->value[0]) 
     { 
        case 0 : send_to_char("none.  Tell an immortal.\n\r",ch);    
break; 
        case 1 : send_to_char("sapphire.\n\r",ch);   break; 
        case 2 : send_to_char("ruby.\n\r",ch);   break; 
        case 3 : send_to_char("emerald.\n\r",ch);    break; 
        case 4 : send_to_char("diamond.\n\r",ch);    break; 
        case 5 : send_to_char("topaz.\n\r",ch);  break; 
        case 6 : send_to_char("skull.\n\r",ch);  break; 
        default : send_to_char("Unknown.  Something is wrong, fix this object.\n\r",ch);    break; 
     } 
     send_to_char("Gem value is: ",ch); 
     switch (obj->value[1]) 
         { 
         case 0 : send_to_char("chip.\n\r",ch);    break; 
         case 1 : send_to_char("flawed.\n\r",ch);   break; 
         case 2 : send_to_char("flawless.\n\r",ch);   break; 
         case 3 : send_to_char("perfect.\n\r",ch);    break; 
         default : send_to_char("Unknown.  Something is wrong, fix this  object.\n\r",ch);   break; 
         } 
     break; 

    }


    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
	    	send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char(buf,ch);
	if ( paf->duration > -1)
	    sprintf(buf,", %d hours.\n\r",paf->duration);
	else
	    sprintf(buf,".\n\r");
	send_to_char( buf, ch );
	if (paf->bitvector)
	{
	    switch(paf->where)
	    {
		case TO_AFFECTS:
		    sprintf(buf,"Adds %s affect.\n",
			affect_bit_name(paf->bitvector));
		    break;
                case TO_WEAPON:
                    sprintf(buf,"Adds %s weapon flags.\n",
                        weapon_bit_name(paf->bitvector));
		    break;
		case TO_OBJECT:
		    sprintf(buf,"Adds %s object flag.\n",
			extra_bit_name(paf->bitvector));
		    break;
		case TO_IMMUNE:
		    sprintf(buf,"Adds immunity to %s.\n",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_RESIST:
		    sprintf(buf,"Adds resistance to %s.\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_VULN:
		    sprintf(buf,"Adds vulnerability to %s.\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		default:
		    sprintf(buf,"Unknown bit %d: %d\n\r",
			paf->where,paf->bitvector);
		    break;
	    }
	    send_to_char(buf,ch);
	}
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
        if (paf->bitvector)
        {
            switch(paf->where)
            {
                case TO_AFFECTS:
                    sprintf(buf,"Adds %s affect.\n",
                        affect_bit_name(paf->bitvector));
                    break;
                case TO_OBJECT:
                    sprintf(buf,"Adds %s object flag.\n",
                        extra_bit_name(paf->bitvector));
                    break;
                case TO_IMMUNE:
                    sprintf(buf,"Adds immunity to %s.\n",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_RESIST:
                    sprintf(buf,"Adds resistance to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_VULN:
                    sprintf(buf,"Adds vulnerability to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                default:
                    sprintf(buf,"Unknown bit %d: %d\n\r",
                        paf->where,paf->bitvector);
                    break;
            }
            send_to_char(buf,ch);
        }
    }

    return;
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: %s\n\r",
	victim->name);
    send_to_char( buf, ch );

    sprintf( buf, 
	"Vnum: %d  Format: %s  Race: %s  Group: %d  Sex: %s  Room: %d\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
	race_table[victim->race].name,
	IS_NPC(victim) ? victim->group : 0, sex_table[victim->sex].name,
	victim->in_room == NULL    ?        0 : victim->in_room->vnum
	);
    send_to_char( buf, ch );
   
    if (is_clan(victim))
    {
       sprintf(buf, "Clan: %-25s  Rank: %s\n\r",
               player_clan(victim), player_rank(victim));
       send_to_char(buf, ch);
    }

    if (IS_NPC(victim))
    {
	sprintf(buf,"Count: %d  Killed: %d\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed);
	send_to_char(buf,ch);
    }

    sprintf( buf, 
   	"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON) );
    send_to_char( buf, ch );

    sprintf( buf, "Hp: %ld/%ld  Mana: %ld/%ld  Move: %ld/%ld  Practices: %d\n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move,
	IS_NPC(victim) ? 0 : victim->practice );
    send_to_char( buf, ch );
	
    sprintf( buf,
	"Lv: %d  Class: %s  Align: %d  Gold: %ld  Silver: %ld  Exp: %d\n\r",
	victim->level,       
	IS_NPC(victim) ? "mobile" : class_short(victim),            
	victim->alignment,
	victim->gold, victim->silver, victim->exp );
    send_to_char( buf, ch );

    sprintf(buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    send_to_char(buf,ch);

    sprintf( buf, 
	"Hit: %d  Dam: %d Saves: %d  Size: %s  Position: %s  Wimpy: %d\n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	size_table[victim->size].name, position_table[victim->position].name,
	victim->wimpy );
    send_to_char( buf, ch );

    sprintf( buf,
        "Clevel: %d          Cability: %d     Aggression: %d\n\r", victim->cast_level, victim->cast_ability, victim->aggression );
    send_to_char( buf, ch );

    if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PET ) )
        printf_to_char( ch, "They need %d Exp for each level!\n\r", victim->xp_tolevel); 

    if (!IS_NPC(victim) && IS_SET(victim->act, PLR_QUESTOR))
    {
      OBJ_INDEX_DATA *questinfoobj;
      MOB_INDEX_DATA *questinfo;
      ROOM_INDEX_DATA *questroom = get_room_index(victim->pcdata->questloc);

        if (victim->pcdata->questmob == -1 || has_questobj(victim))
        {
	    sprintf(buf, "{g%s's Quest is {WALMOST{g complete.{x\n\r", victim->name);
            send_to_char(buf, ch);
        }
	else if (victim->pcdata->questobj > 0)
	{
	    if ((questinfoobj = get_obj_index(victim->pcdata->questobj)) != NULL)
            {
		sprintf(buf, "{gThey are on a quest to recover {W%s{g. ", questinfoobj->short_descr);
                send_to_char(buf, ch);
        
                if(questroom != NULL)
                {
                   sprintf(buf, "(Room: %d, Area: %s){x\n\r", questroom->vnum, questroom->area->name);
                   send_to_char(buf, ch);
                }
                else
                   send_to_char("\n\r", ch);
            }
	    
            else
	        send_to_char("{gBUG: player has NULL quest obj.{x\n\r", ch);
	} else if (victim->pcdata->questmob > 0)
	{
	    if ((questinfo = get_mob_index(victim->pcdata->questmob)) != NULL)
	    {
		sprintf(buf, "{gThey are on a quest to slay %s{g. ", questinfo->short_descr);
        send_to_char(buf, ch);
        if(questroom != NULL)
        {
        sprintf(buf, "(Room: %d, Area: %s){x\n\r", questroom->vnum, questroom->area->name);
        send_to_char(buf, ch);
        }
        else
        send_to_char("\n\r", ch);
	    } else
		send_to_char("{gBUG: player has NULL quest mob.{x\n\r", ch);
	}
    }

    if (!IS_NPC(victim))
    {
	sprintf(buf,"{GMobile deaths/kills {W%5d{G/{W%-5d{x\n\r{GPC deaths/kills {W%5d{G/{W%-5d{x\n\r",victim->pcdata->deaths_mob, victim->pcdata->kills_mob,victim->pcdata->deaths_pc,  victim->pcdata->kills_pc );
	send_to_char(buf,ch);
    }

    if (IS_NPC(victim) && victim->pIndexData->new_format)
    {
	sprintf(buf, "Damage: %dd%d  Message:  %s\n\r",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->dam_type].noun);
	send_to_char(buf,ch);
    }
    sprintf( buf, "Fighting: %s\n\r",
	victim->fighting ? victim->fighting->name : "(none)" );
    send_to_char( buf, ch );

    if ( !IS_NPC(victim) )
    {
	sprintf( buf,
	    "Thirst: %d  Hunger: %d  Full: %d  Drunk: %d Tired: %d\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_HUNGER],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK],
            victim->pcdata->condition[COND_TIRED] );
	send_to_char( buf, ch );
    }

    sprintf( buf, "Carry number: %d  Carry weight: %ld\n\r",
	victim->carry_number, get_carry_weight(victim) / 10 );
    send_to_char( buf, ch );


    if (!IS_NPC(victim))
    {
    	sprintf( buf, 
	    "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
	    get_age(victim), 
	    (int) (victim->played + current_time - victim->logon) / 3600, 
	    victim->pcdata->last_level, 
	    victim->timer );
    	send_to_char( buf, ch );
    }

    sprintf(buf, "Act: %s\n\r",act_bit_name(victim->act));
    send_to_char(buf,ch);
    
    if (victim->comm || victim->comm2)
    {
    	sprintf(buf,"Comm: %s Comm2: %s\n\r",comm_bit_name(victim->comm),comm2_bit_name(victim->comm2) );
    	send_to_char(buf,ch);
    }

    if (IS_NPC(victim) && victim->off_flags)
    {
    	sprintf(buf, "Offense: %s\n\r",off_bit_name(victim->off_flags));
	send_to_char(buf,ch);
    }

    if (victim->imm_flags)
    {
	sprintf(buf, "Immune: %s\n\r",imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }
 
    if (victim->res_flags)
    {
	sprintf(buf, "Resist: %s\n\r", imm_bit_name(victim->res_flags));
	send_to_char(buf,ch);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "Vulnerable: %s\n\r", imm_bit_name(victim->vuln_flags));
	send_to_char(buf,ch);
    }

    sprintf(buf, "Form: %s\n\rParts: %s\n\r", 
	form_bit_name(victim->form), part_bit_name(victim->parts));
    send_to_char(buf,ch);

    if (victim->affected_by)
    {
	sprintf(buf, "Affected by %s\n\r", 
	    affect_bit_name(victim->affected_by));
	send_to_char(buf,ch);
    }

    if (victim->affected2_by)
    {
         sprintf(buf, "Also affected by %s\n\r",
            affect2_bit_name(victim->affected2_by) );
         send_to_char(buf,ch);
    }

    sprintf( buf, "Master: %s  Leader: %s  Pet: %s  Horse: %s\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet 	    ? victim->pet->name	     : "(none)",
	victim->mount 	    ? victim->mount->name    : "(none)");
    send_to_char( buf, ch );

    if ( !IS_NPC(victim) )
    {
        sprintf( buf, "Security: %d.\n\r", victim->pcdata->security );	/* OLC */
        send_to_char( buf, ch );					/* OLC */
    }

    sprintf( buf, "Short description: %s\n\rLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    send_to_char( buf, ch );

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
    {
	sprintf(buf,"Mobile has special procedure %s.\n\r",
		spec_name(victim->spec_fun));
	send_to_char(buf,ch);
    }

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf,
	    "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
	    skill_table[(int) paf->type].name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    affect_bit_name( paf->bitvector ),
	    paf->level
	    );
	send_to_char( buf, ch );
    }

    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj <name>\n\r",ch);
	send_to_char("  vnum mob <name>\n\r",ch);
	send_to_char("  vnum skill <skill or spell>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_function(ch, &do_ofind, string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
	do_function(ch, &do_mfind, string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_function (ch, &do_slookup, string);
	return;
    }
    /* do both */
    do_function(ch, &do_mfind, argument);
    do_function(ch, &do_ofind, argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find whom?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, pMobIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No mobiles by that name.\n\r", ch );

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find what?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pObjIndex->name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pObjIndex->vnum, pObjIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No objects by that name.\n\r", ch );

    return;
}


void do_owhere(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();

    if (argument[0] == '\0')
    {
	send_to_char("Find what?\n\r",ch);
	return;
    }
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || !is_name( argument, obj->name )
        ||   ch->level < obj->level)
            continue;
 
        found = TRUE;
        number++;
 
        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;
 
        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
	&&   in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
                number, obj->short_descr,PERS(in_obj->carried_by, ch),
		in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
            sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
                number, obj->short_descr,in_obj->in_room->name, 
	   	in_obj->in_room->vnum);
	else
            sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);
 
        buf[0] = UPPER(buf[0]);
        add_buf(buffer,buf);
 
        if (number >= max_found)
            break;
    }
 
    if ( !found )
        send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}

void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    found = FALSE;
    buffer = new_buf();

    if ( arg1[0] == '\0' )
    {
	DESCRIPTOR_DATA *d;

	/* show characters logged */

	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->character != NULL && d->connected == CON_PLAYING
	    &&  d->character->in_room != NULL && can_see(ch,d->character)
	    &&  can_see_room(ch,d->character->in_room))
	    {
		victim = d->character;
		count++;
		if (d->original != NULL)
		    sprintf(buf,"%3d) %s (in the body of %s) is in %s [%d]\n\r",
			count, d->original->name,victim->short_descr,
			victim->in_room->name,victim->in_room->vnum);
		else
		    sprintf(buf,"%3d) %s is in %s [%d]\n\r",
			count, victim->name,victim->in_room->name,
			victim->in_room->vnum);
		add_buf(buffer,buf);
	    }
	}

        page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
	return;
    }
    else if(arg2[0] == '\0' )
    {
      for ( victim = char_list; victim != NULL; victim = victim->next )
      {
	if ( victim->in_room != NULL
	&&   is_name( arg1, victim->name ) )
	{
	    found = TRUE;
	    count++;
	    sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->short_descr : victim->name,
		victim->in_room->vnum,
		victim->in_room->name );
	    add_buf(buffer,buf);
	}
      }
    }
    else if(!str_cmp(arg1,"all") && !str_cmp(arg2,"area"))
    {
      found = FALSE;
      buffer = new_buf();
      for ( victim = char_list; victim != NULL; victim = victim->next )
      {
	if ( victim->in_room != NULL
        &&   victim->in_room->area == ch->in_room->area
	&&   IS_NPC(victim) )
	{
	    found = TRUE;
	    count++;
	    sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->short_descr : victim->name,
		victim->in_room->vnum,
		victim->in_room->name );
	    add_buf(buffer,buf);
	}
      }
    }
    else if(!str_cmp(arg2,"area"))
    {
      found = FALSE;
      buffer = new_buf();
      for ( victim = char_list; victim != NULL; victim = victim->next )
      {
	if ( victim->in_room != NULL
        &&   victim->in_room->area == ch->in_room->area
	&&   is_name( arg1, victim->name ) )
	{
	    found = TRUE;
	    count++;
	    sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->short_descr : victim->name,
		victim->in_room->vnum,
		victim->in_room->name );
	    add_buf(buffer,buf);
	}
      }
    }

    if ( !found )
    {     
      if ( !str_cmp(arg2,"area") )
         act("You didn't find any $T in this area.", ch, NULL, arg1, TO_CHAR );
      else
	 act("You didn't find any $T.", ch, NULL, arg1, TO_CHAR );
    }
    else
    {
    	page_to_char(buf_string(buffer),ch);
    }
    free_buf(buffer);

    return;
}


void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    if ( timed_copyover )
    {
        send_to_char( "Not while timed coypover is turned on.\n\r", ch );
        return;
    }

    if ( treboot )
    {
        send_to_char( "Not while timed reboot is turned on.\n\r", ch );
        return;
    }

    if ( tshutdown )
    {
        send_to_char( "Not while timed shutdown is turned on.\n\r", ch );
        return;
    }

    if (ch->invis_level < LEVEL_HERO)
    {
    	sprintf( buf, "Reboot by %s.", ch->name );
    	do_function(ch, &do_echo, buf );
    }

    while (auction_list != NULL)
      reset_auc (auction_list, TRUE);
    
    save_gquest_data();  

    if (war_info.iswar != WAR_OFF)
	end_war();

    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
	    save_char_obj(vch);
    	close_socket(d);
    }
    
    return;
}

void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}

void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    if ( timed_copyover )
    {
        send_to_char( "Not while timed coypover is turned on.\n\r", ch );
        return;
    }

    if ( treboot )
    {
        send_to_char( "Not while timed reboot is turned on.\n\r", ch );
        return;

    }

    if ( tshutdown )
    {
        send_to_char( "Not while timed shutdown is turned on.\n\r", ch );
        return;
    }

    if (ch->invis_level < LEVEL_HERO)
    sprintf( buf, "Shutdown by %s.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    if (ch->invis_level < LEVEL_HERO)
    {
    	do_function(ch, &do_echo, buf );
    }
    
    while (auction_list != NULL)
      reset_auc (auction_list, TRUE);

    save_gquest_data();

    merc_down = TRUE;
    if (war_info.iswar != WAR_OFF)
	end_war();
    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
	    save_char_obj(vch);
	close_socket(d);
    }
    return;
}

void do_protect( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Protect whom from snooping?\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {
	send_to_char("You can't find them.\n\r",ch);
	return;
    }

    if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
	act_new("$N is no longer snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
	send_to_char("Your snoop-proofing was just removed.\n\r",victim);
	REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
    else
    {
	act_new("$N is now snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
	send_to_char("You are now immune to snooping.\n\r",victim);
	SET_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
}
  


void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	wiznet("$N stops being such a snoop.",
		ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        send_to_char("That character is in a private room.\n\r",ch);
        return;
    }

   
    if (( get_trust( victim ) >= get_trust( ch )  && !is_admin( ch ) )
    ||  ( IS_SET(victim->comm,COMM_SNOOP_PROOF) || is_admin( victim ) ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    sprintf(buf,"$N starts snooping on %s",
	(IS_NPC(ch) ? victim->short_descr : victim->name));
    wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char("That character is in a private room.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    sprintf(buf,"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup(ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    send_to_char( 
"You return to your original body. Type replay to see any missed tells.\n\r", 
	ch );
    if (ch->prompt != NULL)
    {
	free_string(ch->prompt);
	ch->prompt = NULL;
    }

    sprintf(buf,"$N returns from %s.",ch->short_descr);
    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GOD)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
	|| (IS_TRUSTED(ch,DEMI)	    && obj->level <= 10 && obj->cost <= 500)
	|| (IS_TRUSTED(ch,ANGEL)    && obj->level <=  5 && obj->cost <= 250)
	|| (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch, NULL, rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch, NULL, rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch, NULL, argument);
	obj = get_obj_here(ch, NULL, argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_object(obj->pIndexData,0); 
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	{   obj->carried_by = ch;
	    obj_to_room(clone,ch->in_room);
	}
 	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	act("You clone $p.",ch,clone,NULL,TO_CHAR);
	wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;
	char buf[MAX_STRING_LENGTH];

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ((mob->level > 20 && !IS_TRUSTED(ch,GOD))
	||  (mob->level > 10 && !IS_TRUSTED(ch,IMMORTAL))
	||  (mob->level >  5 && !IS_TRUSTED(ch,DEMI))
	||  (mob->level >  0 && !IS_TRUSTED(ch,ANGEL))
	||  !IS_TRUSTED(ch,AVATAR))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,0);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,clone,TO_ROOM);
        act("You clone $N.",ch,NULL,clone,TO_CHAR);
	sprintf(buf,"$N clones %s.",clone->short_descr);
	wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
        return;
    }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_function(ch, &do_mload, argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_function(ch, &do_oload, argument);
	return;
    }
    /* echo syntax */
    do_function(ch, &do_load, "");
}


void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    sprintf(buf,"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
	return;
    }
    
    level = get_trust(ch); /* default */
  
    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
        {
	  send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	  return;
	}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
  	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    obj->size = ch->size;       

    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
    {   obj->carried_by = ch;
	obj_to_room( obj, ch->in_room );
    }
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE) 
	    &&   victim != ch /* safety precaution */ )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
	      extract_obj( obj );
	}
	
        SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {

	if (ch == victim)
	{
	  send_to_char("Ho ho ho.\n\r",ch);
	  return;
	}

	if (get_trust(ch) <= get_trust(victim))
	{
	  send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	  sprintf(buf,"%s tried to purge you!\n\r",ch->name);
	  send_to_char(buf,victim);
	  return;
	}

	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);

    	if (victim->level > 1)
	    save_char_obj( victim );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )
          close_socket( d );

	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( is_admin( victim ) && !is_admin(ch ) ) 
    {
        send_to_char("I'm sorry you can't change their level!\n\r", ch );
        printf_to_char( victim, "%s is trying to demote you!\n\r", ch->name );
        return;
    }
       
    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
    {
	sprintf(buf,"Level must be 1 to %d.\n\r", MAX_LEVEL);
	send_to_char(buf, ch);
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    if ( is_admin( victim ) && !is_admin( ch ) )
    {
        send_to_char( "You shouldn't try to demote one of the admins!\n\r", ch );
        return;
    }      

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->level )
    {
        int temp_prac;

	send_to_char( "Lowering a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
        if ((victim->level > HERO) || (level > HERO))
	{
	    update_wizlist(victim, level);
	}
	temp_prac = victim->practice;
	victim->level    = 1;
	victim->exp      = exp_per_level(victim,victim->pcdata->points);
	victim->max_hit  = 10;
	victim->max_mana = 100;
	victim->max_move = 100;
	victim->practice = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;
	advance_level( victim, TRUE );
	victim->practice = temp_prac;
    }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
        if ((victim->level > HERO) || (level > HERO))
	{
	    update_wizlist(victim, level);
	}
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	victim->level += 1;
	advance_level( victim,TRUE);
    }
    sprintf(buf,"You are now level %d.\n\r",victim->level);
    send_to_char(buf,victim);
    victim->exp   = exp_per_level(victim,victim->pcdata->points) 
		  * UMAX( 1, victim->level );
    victim->trust = 0;
    save_char_obj(victim);
    return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
	sprintf(buf, "Level must be 0 (reset) or 1 to %d.\n\r",MAX_LEVEL);
	send_to_char(buf, ch);
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */
    	
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            affect_strip(vch,skill_lookup("plague"));
            affect_strip(vch,skill_lookup("poison"));
            affect_strip(vch,skill_lookup("blindness"));
            affect_strip(vch,skill_lookup("sleep"));
            affect_strip(vch,skill_lookup("curse"));
            
            vch->hit 	= vch->max_hit;
            vch->mana	= vch->max_mana;
            vch->move	= vch->max_move;
            update_pos( vch);
            act("$n has restored you.",ch,NULL,vch,TO_VICT);
            sound( "restore.wav", vch );
        }

        sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
        
        send_to_char("Room restored.\n\r",ch);
        return;

    }
    
    if ( get_trust(ch) >=  MAX_LEVEL - 1 && !str_cmp(arg,"all"))
    {
    /* cure all */
    	
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;
                
            affect_strip(victim,skill_lookup("plague"));
            affect_strip(victim,skill_lookup("poison"));
            affect_strip(victim,skill_lookup("blindness"));
            affect_strip(victim,skill_lookup("sleep"));
            affect_strip(victim,skill_lookup("curse"));
            
            victim->hit 	= victim->max_hit;
            victim->mana	= victim->max_mana;
            victim->move	= victim->max_move;
            update_pos( victim);
	    if (victim->in_room != NULL)
            {
                act("$n has restored you.",ch,NULL,victim,TO_VICT);
                sound( "restore.wav", victim );
            }
        }
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    affect_strip(victim,skill_lookup("plague"));
    affect_strip(victim,skill_lookup("poison"));
    affect_strip(victim,skill_lookup("blindness"));
    affect_strip(victim,skill_lookup("sleep"));
    affect_strip(victim,skill_lookup("curse"));
    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );
    act( "$n has restored you.", ch, NULL, victim, TO_VICT );
    sprintf(buf,"$N restored %s",
	IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}

 	
void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
	sprintf(buf,"$N thaws %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
	sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NOEMOTE removed.\n\r", ch );
	sprintf(buf,"$N restores emotes to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NOEMOTE set.\n\r", ch );
	sprintf(buf,"$N revokes %s's emotes.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noshout whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
	REMOVE_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can shout again.\n\r", victim );
	send_to_char( "NOSHOUT removed.\n\r", ch );
	sprintf(buf,"$N restores shouts to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can't shout!\n\r", victim );
	send_to_char( "NOSHOUT set.\n\r", ch );
	sprintf(buf,"$N revokes %s's shouts.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NOTELL removed.\n\r", ch );
	sprintf(buf,"$N restores tells to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NOTELL set.\n\r", ch );
	sprintf(buf,"$N revokes %s's tells.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
    {
	wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	send_to_char( "Game wizlocked.\n\r", ch );
    }
    else
    {
	wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	send_to_char( "Game un-wizlocked.\n\r", ch );
    }

    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;
 
    if ( newlock )
    {
	wiznet("$N locks out new characters.",ch,NULL,0,0,0);
        send_to_char( "New characters have been locked out.\n\r", ch );
    }
    else
    {
	wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
        send_to_char( "Newlock removed.\n\r", ch );
    }
 
    return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name );
	    send_to_char( buf, ch );
	}
    }
    else
    {
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	    return;
	}

	sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name );
	send_to_char( buf, ch );
    }

    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set mob   <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
        send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_function(ch, &do_mset, argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_function(ch, &do_sset, argument);
	return;

    }

    if (!str_prefix(arg,"weather"))
    {
        do_function(ch, &do_wset, argument);
        return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_function(ch, &do_oset, argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_function(ch, &do_rset, argument);
	return;
    }
    /* echo syntax */
    do_function(ch, &do_set, "");
}


void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->pcdata->learned[sn]	= value;
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
    }

    return;
}


void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    int value;
    EMAIL_DATA *pMail;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char <name> <field> <value>\n\r",ch); 
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    str int wis dex con sex klass level\n\r",	ch );
	send_to_char( "    race group gold silver hp mana move prac\n\r",ch);
	send_to_char( "    align train thirst hunger drunk full\n\r", ch );
        send_to_char( "    questpoints trivia pckill pcdeath mobkill\n\r", ch );
        send_to_char( "    mobdeath email iqp ttrivia waypoint", ch );
	send_to_char("\n\r",ch);
	return;
    }
    
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /* clear zones for mobs */
    victim->zone = NULL;

    /*
     * Snarf the value (which need not be numeric).
     */
  
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;
    if(!str_cmp(arg2, "email" ) )
    {
	free_string(victim->pcdata->email );
	victim->pcdata->email = str_dup(arg3);
	send_to_char("Email set",ch);
        printf_to_char(victim, "Your email has been set to %s",victim->pcdata->email);
	if( ( pMail = email_lookup(victim->name ) ) == NULL )
	{
		pMail = new_email();
		free_string(pMail->name);
		pMail->name = str_dup(victim->name);
	}
	free_string(pMail->email);
	pMail->email = str_dup(victim->pcdata->email);
	return;
    }	

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "str" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_STR) )
	{
	    sprintf(buf,
		"Strength range is 3 to %d\n\r.",
		get_max_train(victim,STAT_STR));
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
        if ( value < 3 || value > get_max_train(victim,STAT_INT) )
        {
            sprintf(buf,
		"Intelligence range is 3 to %d.\n\r",
		get_max_train(victim,STAT_INT));
            send_to_char(buf,ch);
            return;
        }
 
        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
	{
	    sprintf(buf,
		"Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
	{
	    sprintf(buf,
		"Dexterity range is 3 to %d.\n\r",
		get_max_train(victim,STAT_DEX));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_CON) )
	{
	    sprintf(buf,
		"Constitution range is 3 to %d.\n\r",
		get_max_train(victim,STAT_CON));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if (!str_cmp(arg2, "class"))
    {
	char arg4[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int slot,
	 value,
	 iClass;

	argument = one_argument(argument, arg3);
	strcpy(arg4, argument);
	slot = (is_number(arg3)) ? atoi(arg3) : -1;
	value = (is_number(arg4)) ? atoi(arg4) : -1;
	if (value < -1)
	    value = -1;
	if (slot < 1 || slot > 8)
	{
	    send_to_char("Invalid slot.  Valid slot: 1-8.\n\r", ch);
	    return;
	}

	for (iClass = 0; iClass < MAX_CLASS; iClass++)
	{
	    if (!str_prefix(arg4, class_table[iClass].name) || !str_cmp(arg4, class_table[iClass].who_name))
	    {
		value = iClass;
		break;
	    }
	}

	if (slot == 1 && value == -1)
	{
	    send_to_char("Cannot turn off 1st Class.\n\r", ch);
	    return;
	}

	if ((slot == 2 && value == -1 && victim->klass[2] != -1) 
	|| (slot == 3 && value == -1 && victim->klass[3] != -1) 
	|| (slot == 4 && value == -1 && victim->klass[4] != -1) 
        || (slot == 5 && value == -1 && victim->klass[5] != -1)
        || (slot == 6 && value == -1 && victim->klass[6] != -1)
        || (slot == 7 && value == -1 && victim->klass[7] != -1)
        || (slot == 8 && value == -1 && victim->klass[8] != -1) )
	{
	    send_to_char("You must turn off next to last klass first.\n\r", ch);
	    return;
	}
	if ((slot == 3 && victim->klass[1] == -1) || (slot == 4 && victim->klass[2] == -1) ) 
	{
	    act("Cannot set that Class for $N.  Player does not have a Previous Class.", ch, NULL, victim, TO_CHAR);
	    return;
	}
	if (value < -1 || value >= MAX_CLASS)
	{
	    sprintf(buf, "Class range is 0 to %d.\n\r", MAX_CLASS - 1);
	    send_to_char(buf, ch);
	    for (iClass = 0; iClass < MAX_CLASS; iClass++)
	    {
		sprintf(buf, " %2d = %s\n\r", iClass, class_table[iClass].name);
		send_to_char(buf, ch);
	    }
	    send_to_char(" -1 = Turns off class.\n\r", ch);
	    return;
	}
	if (value != -1 && is_class(victim, value))
	{
	    sprintf(buf, "$N is already %s %s.", (victim->klass[1] != -1) ? "part" : "a", class_table[value].name);
	    act(buf, ch, NULL, victim, TO_CHAR);
	    return;
	}
	victim->klass[slot - 1] = value;
	if (slot == 9)
	    victim->klass[slot] = -1;
	send_to_char("Ok.\n\r", ch);
	return;
    }

    if (!str_prefix(arg2, "trivia"))
    {
        if (value < -10 || value > 10)
        {
            send_to_char("Trivia point range is -10 to 10.\n\r", ch);
            return;
        }

        victim->pcdata->trivia += value;
        printf_to_char(ch, "{wAdjusting %s's trivia points by {W%s{x\n\r", victim->name, value );
        printf_to_char(victim, "{wYour trivia points have been modified by {W%s{w.{x\n\r", ch->name );
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > MAX_LEVEL )
	{
	    sprintf(buf, "Level range is 0 to %d.\n\r", MAX_LEVEL);
	    send_to_char(buf, ch);
	    return;
	}
	victim->level = value;
	return;
    }

    if ( !str_prefix( arg2, "gold" ) )
    {
	victim->gold = value;
	return;
    }

    if ( !str_prefix(arg2, "silver" ) )
    {
	victim->silver = value;
	return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
	if ( value < -10 || value > 40000 )
	{
	    send_to_char( "Hp range is -10 to 40,000 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_hit = value;
	return;
    }

    if ( !str_prefix( arg2, "mana" ) )
    {
	if ( value < 0 || value > 40000 )
	{
	    send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_mana = value;
	return;
    }

    if ( !str_prefix( arg2, "move" ) )
    {
	if ( value < 0 || value > 40000 )
	{
	    send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_move = value;
	return;
    }

    if ( !str_prefix( arg2, "practice" ) )
    {
	if ( value < 0 || value > 500 )
	{
	    send_to_char( "Practice range is 0 to 500 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !str_prefix( arg2, "train" ))
    {
	if (value < 0 || value > 100 )
	{
	    send_to_char("Training session range is 0 to 100 sessions.\n\r",ch);
	    return;
	}
	victim->train = value;
	return;
    }

    if ( !str_prefix( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Thirst range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_prefix( arg2, "tired" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < -1 || value > 48 )
        {
            send_to_char( "Tired range is -1 to 48.\n\r", ch );
            return;
        }
        victim->pcdata->condition[COND_TIRED] = value;
        return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !str_prefix( arg2, "hunger" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
 
        if ( value < -1 || value > 100 )
        {
            send_to_char( "Full range is -1 to 100.\n\r", ch );
            return;
        }
 
        victim->pcdata->condition[COND_HUNGER] = value;
        return;
    }

   
    if ( !str_prefix( arg2, "pckill" ) )
    {
		if ( IS_NPC( victim ) )
		{
			send_to_char( "{RNot on NPC's.{x\n\r", ch );
			return;
		}
		if ( !is_number( arg3 ) )
		{
			send_to_char( "{RValue must be numeric.{x\n\r", ch );
			return;
		}
		value = atoi( arg3 );

		if ( value < 0 || value > 30000 )
		{
		    send_to_char( "{RPC Kills value must be 0 to 30,000!{x\n\r", ch );
		    return;
		}

		victim->pcdata->kills_pc = value;
		return;
    }

    if ( !str_prefix( arg2, "pcdeaths" ) )
    {
		if ( IS_NPC( victim ) )
		{
			send_to_char( "{RNot on NPC's.{x\n\r", ch );
			return;
		}
		if ( !is_number( arg3 ) )
		{
			send_to_char( "{RValue must be numeric.{x\n\r", ch );
			return;
		}
		value = atoi( arg3 );

		if ( value < 0 || value > 30000 )
		{
		    send_to_char( "{RPC Deaths value must be 0 to 30,000!{x\n\r", ch );
		    return;
		}

		victim->pcdata->deaths_pc = value;
		return;
    }

    if ( !str_prefix( arg2, "mobkill" ) )
    {
		if ( IS_NPC( victim ) )
		{
			send_to_char( "{RNot on NPC's.{x\n\r", ch );
			return;
		}
		if ( !is_number( arg3 ) )
		{
			send_to_char( "{RValue must be numeric.{x\n\r", ch );
			return;
		}
		value = atoi( arg3 );

		if ( value < 0 || value > 30000 )
		{
		    send_to_char( "{RMob Kills value must be 0 to 30,000!{x\n\r", ch );
		    return;
		}

		victim->pcdata->kills_mob = value;
		return;
    }

    if ( !str_prefix( arg2, "mobdeaths" ) )
    {
		if ( IS_NPC( victim ) )
		{
			send_to_char( "{RNot on NPC's.{x\n\r", ch );
			return;
		}
		if ( !is_number( arg3 ) )
		{
			send_to_char( "{RValue must be numeric.{x\n\r", ch );
			return;
		}
		value = atoi( arg3 );

		if ( value < 0 || value > 30000 )
		{
		    send_to_char( "{RMob Deaths value must be 0 to 30,000!{x\n\r", ch );
		    return;
		}

		victim->pcdata->deaths_mob = value;
		return;
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->race = race;
	return;
    }
   
    if (!str_prefix(arg2,"group"))
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("Only on NPCs.\n\r",ch);
	    return;
	}
	victim->group = value;
	return;
    }

    if (!str_prefix(arg2, "questpoints"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not on NPC's.\n\r", ch);
	    return;
	}
	victim->pcdata->questpoints = value;
	return;
    }

    if (!str_prefix(arg2, "iqp"))
    {
        if (IS_NPC(victim))
        {
            send_to_char("Not on NPC's.\n\r",ch );
            return;
        }

        victim->pcdata->qps = value;
        send_to_char( "Ok setting their immortal questpoints.\n\r", ch );
        printf_to_char( victim, "Your immortal questpoints are now %d\n\r", value );
        return;
    }

    if (!str_prefix(arg2, "ttrivia"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not on NPC's.\n\r", ch);
	    return;
	}

	victim->pcdata->totaltpoints = value;
        if ( ch != victim )
        printf_to_char( ch, "Changing %s's totalpoints to %d.\n\r", victim->name, value );

        printf_to_char( victim, "Your totalpoints have been changed to %d.\n\r", value );
	return;
    }

        if(!str_prefix(arg2, "waypoint" ) )
        {       ROOM_INDEX_DATA *pR;
                if(! ( pR = get_room_index(value) ) )
                {       send_to_char("That isn't a room!\r\n",ch);
                        return;
                }
                if(IS_NPC(victim) )
                {       send_to_char("Not on mobiles.\r\n",ch);
                        return;
                }
                victim->pcdata->map_point = pR;
                send_to_char("Waypoint set.\r\n",ch);
                return;
        }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_mset, "" );
    return;
}

void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long desc title spec",ch);
	if(is_admin(ch) )
	   send_to_char(" pretitle",ch);
	send_to_char("\n\r",ch);
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended\n\r",ch);
	return;
    }
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
    	}

	/* clear zone for mobs */
	victim->zone = NULL;

	/* string something */

     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }
	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}
    	
    	if ( !str_prefix( arg2, "description" ) )
    	{
    	    free_string(victim->description);
    	    victim->description = str_dup(arg3);
    	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
    	}
        if( !str_prefix(arg2, "pretitle" ) )
	{   if(!is_admin(ch) )
 	    {   do_function(ch, &do_string, "" );
		return;
	    }
	    if(IS_NPC(victim) )
	    {   send_to_char("Not on Mobiles!\n\r",ch);
		return;
	    }
	    if(arg3[0] == '\0' )
	    {   send_to_char("Pretitle nulled!\n\r",ch);
	        free_string( victim->pcdata->pretitle );
		victim->pcdata->pretitle = str_dup("");
	    }
		
	    free_string( victim->pcdata->pretitle );
	    strcat(arg3, " " );
	    victim->pcdata->pretitle = str_dup(arg3);
	    return;
	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "spec" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }

	    if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	    {
	    	send_to_char( "No such spec fun.\n\r", ch );
	    	return;
	    }

	    return;
    	}
    }
    
    if (!str_prefix(type,"object"))
    {
    	/* string an obj */
    	
   	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	    return;
    	}
    	
        if ( !str_prefix( arg2, "name" ) )
    	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
	    	return;
	    }

 	    strcat(argument,"\n\r");

	    ed = new_extra_descr();

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}
    }
    
    	
    /* echo bad use message */
    do_function(ch, &do_string, "");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch );
	send_to_char("    extra wear level weight cost timer size\n\r",		ch );
        send_to_char("    extra2 condition\n\r",ch);
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if( !str_cmp(arg2, "condition" ) )
    {
	obj->condition = UMIN(100, value );
	return;
    }
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = UMIN(50,value);
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
        if ( value > 20 )
        {
            send_to_char( "Sorry you can't set v1 over 20.\n\r", ch );
            return;
        }
       
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
        if ( value > 20 )
        {
            send_to_char( "Sorry you can't set v2 over 20.\n\r", ch );
            return;
        }

	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[4] = value;
	return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
	obj->extra_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "extra2" ) )
    {
	if( flag_lookup(arg3, extra2_flags ) == NO_FLAG )
	{   send_to_char("What type of flag is that!?\n\r",ch);
	    return;
        }

	if(IS_SET(obj->extra2_flags, flag_lookup(arg3, extra2_flags) ) )
	    REMOVE_BIT(obj->extra2_flags, flag_lookup(arg3, extra2_flags) );
	else
	    SET_BIT(obj->extra2_flags, flag_lookup(arg3, extra2_flags) );
	send_to_char("Ok\n\r",ch);
        return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	obj->level = value;
	return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }

    if (!str_prefix( arg2, "size" ) )
    {
        char buf[128];
        
        if (value < 0 || value > 5)
        {
            send_to_char ("Size range is 0-5.\n\r",ch);
            return;
        }

        obj->size = value;
        sprintf (buf, "Size set to %s.\n\r", osize_table[value]);
        send_to_char (buf, ch);
        return;
    }     
	
    /*
     * Generate usage message.
     */
    do_function(ch, &do_oset, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    flags sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        send_to_char("That room is private right now.\n\r",ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	location->room_flags	= value;
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_rset, "");
    return;
}

void do_sockets( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *vch;
    DESCRIPTOR_DATA *d;
    char            buf  [ MAX_STRING_LENGTH ];
    char            buf2 [ MAX_STRING_LENGTH ];
    int             count;
    char *          st;
    char            s[100];
    char            idle[10];

    count       = 0;
    buf[0]      = '\0';
    buf2[0]     = '\0';

    strcat( buf2, "\n\r[{WNum{x {GConnected_State{x {cLogin@{x Idl] {BPlayer Name{x  Host\n\r" );
    strcat( buf2,"--------------------------------------------------------------------------\n\r");  
    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->character && can_see( ch, d->character ) )
        {
	    /* NB: You may need to edit the CON_ values */
	    switch( d->connected )
	    {
	    case CON_PLAYING:              st = "    PLAYING    "; break;
	    case CON_GET_NAME:             st = "   Get Name    "; break;
	    case CON_GET_OLD_PASSWORD:     st = "Get Old Passwd "; break;
	    case CON_CONFIRM_NEW_NAME:     st = " Confirm Name  "; break;
	    case CON_GET_NEW_PASSWORD:     st = "Get New Passwd "; break;
	    case CON_CONFIRM_NEW_PASSWORD: st = "Confirm Passwd "; break;
	    case CON_GET_NEW_RACE:         st = "  Get New Race "; break;
	    case CON_GET_NEW_SEX:          st = "  Get New Sex  "; break;
	    case CON_GET_NEW_CLASS:        st = " Get New Class "; break;
	    case CON_GET_ALIGNMENT:   	   st = " Get New Align "; break;
            case CON_GET_GOD:              st = " Choosing a God"; break;
	    case CON_DEFAULT_CHOICE:	   st = " Choosing Cust "; break;
	    case CON_GEN_GROUPS:           st = " Customization "; break;
	    case CON_PICK_WEAPON:	   st = " Picking Weapon"; break;
	    case CON_READ_IMOTD:	   st = " Reading IMOTD "; break;
	    case CON_BREAK_CONNECT:	   st = "   LINKDEAD    "; break;
	    case CON_READ_MOTD:            st = "  Reading MOTD "; break;
            case CON_NOTE_TO:              st = "    Note to    "; break;
            case CON_NOTE_SUBJECT:         st = "  Note subject "; break;
            case CON_NOTE_EXPIRE:          st = "  Note expire  "; break;
            case CON_NOTE_TEXT:            st = "  Note text    "; break;
            case CON_NOTE_FINISH:          st = "  Note finish  "; break;
            case CON_MENU:                 st = "  Menu screen  "; break;
            case CON_GET_HOMETOWN:         st = "  Hometown     "; break;
	    case CON_GET_HAIR:		   st = "     Hair      "; break;
	    case CON_GET_WEIGHT:	   st = "    Weight     "; break;
	    case CON_GET_HEIGHT:	   st = "    Height     "; break;
	    case CON_GET_EYE:              st = "     Eyes      "; break;
            case CON_ANSI:                 st = "     Ansi 	"; break;
 	    case CON_SET_SHORT:		   st = "  Short Descr  "; break;
	    case CON_CREATION_MENU:	   st = " Creation Menu "; break;
 	    case CON_GET_EMAIL:		   st = "  Get E-Mail   "; break;
	    default:                       st = "   !UNKNOWN!   "; break;
	    }
	    count++;
           
	    /* Format "login" value... */
	    vch = d->original ? d->original : d->character;
	    strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );
           
	    if ( vch->timer > 0 )
		sprintf( idle, "%-2d", vch->timer );
	    else
		sprintf( idle, "  " );
           
	    sprintf( buf, "[{W%3d{x {G%s{x {c%7s{x %2s] {B%-12s{x {W%-32.32s{x\n\r",
		    d->descriptor,
		    st,
		    s,
		    idle,
		    ( d->original ) ? d->original->name
		    : ( d->character )  ? d->character->name
		    : "(None!)",
		    d->host );
              
	    strcat( buf2, buf );

        }
    }

    sprintf( buf, "\n\r%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat( buf2, buf );
    send_to_char( buf2, ch );
    return;
}


/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob"))     
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    sprintf( buf, "$n forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )
    {
        DESCRIPTOR_DATA *desc,*desc_next;

        if (get_trust(ch) < MAX_LEVEL - 3) 
        {
	send_to_char("Not at your level!\n\r",ch);
	return;
        }

        for ( desc = descriptor_list; desc != NULL; desc = desc_next ) 
        {
	    desc_next = desc->next;

   	    if (desc->connected==CON_PLAYING &&
	    get_trust( desc->character ) < get_trust( ch ) ) 
            {
	    act( buf, ch, NULL, desc->character, TO_VICT );
	    interpret( desc->character, argument );
	    }
        }
    }

    else if (!str_cmp(arg,"players"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) 
	    &&	 vch->level < LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"gods"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
            &&   vch->level >= LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

    	if (!is_room_owner(ch,victim->in_room) 
	&&  ch->in_room != victim->in_room 
        &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    	{
            send_to_char("That character is in a private room.\n\r",ch);
            return;
        }

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	act( buf, ch, NULL, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' ) 
    /* take the default path */

      if ( ch->invis_level)
      {
	  ch->invis_level = 0;
          ch->fquit = 0;
	  act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly fade back into existence.\n\r", ch );
      }
      else
      {
	  ch->invis_level = ch->level;
	  act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    else
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
	send_to_char("Invis level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
	  ch->reply = NULL;
          ch->invis_level = level;
          act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    }

    return;
}


void do_incognito( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
 
    /* RT code for taking a level argument */
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    /* take the default path */
 
      if ( ch->incog_level)
      {
          ch->incog_level = 0;
          act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You are no longer cloaked.\n\r", ch );
      }
      else
      {
	  if(is_admin(ch) )
          ch->incog_level = get_trust(ch);
	  else
	  ch->incog_level = LEVEL_IMMORTAL;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      if(!is_admin(ch))
      {
	send_to_char("Using this function is restricted to admins only. Please use this command with no argument.\n\r",ch);
	return;
      }
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Incog level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          ch->incog_level = level;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    }
 
    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, char *argument)
{
    send_to_char("You cannot abbreviate the prefix command.\r\n",ch);
    return;
}

void do_prefix (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (argument[0] == '\0')
    {
	if (ch->prefix[0] == '\0')
	{
	    send_to_char("You have no prefix to clear.\r\n",ch);
	    return;
	}

	send_to_char("Prefix removed.\r\n",ch);
	free_string(ch->prefix);
	ch->prefix = str_dup("");
	return;
    }

    if (ch->prefix[0] != '\0')
    {
	sprintf(buf,"Prefix changed to %s.\r\n",argument);
	free_string(ch->prefix);
    }
    else
    {
	sprintf(buf,"Prefix set to %s.\r\n",argument);
    }

    ch->prefix = str_dup(argument);
}

#define CH(descriptor)  ((descriptor)->original ? \
(descriptor)->original : (descriptor)->character)

/*  Copyover - Original idea: Fusion of MUD++
 *  Adapted to Diku by Erwin S. Andreasen, <erwin@andreasen.org>
 *  http://www.andreasen.org
 *  Changed into a ROM patch after seeing the 100th request for it :)
 */
void do_copyover (CHAR_DATA *ch, char * argument)
{
	FILE *fp = NULL, *fpObj, *fpBoot;
	DESCRIPTOR_DATA *d, *d_next;
	char buf [100], buf2[100];
	extern int port,control; /* db.c */
	bool asave = TRUE, disregard = FALSE, has_reset = FALSE;
	char arg[MSL];
	CHAR_DATA *wch;
        CHAR_DATA *gch;
	OBJ_DATA *obj;
        RESET_DATA *pReset;
	arg[0] = '\0';

	argument = one_argument(argument,arg);

	if(!str_cmp(arg, "no_asave") )
	    asave = FALSE;
	if(!str_cmp(arg, "disregard") )
	    disregard = TRUE;
 
	fp = file_open(COPYOVER_FILE, "w");

        while (auction_list != NULL)
           reset_auc (auction_list, TRUE);

        save_gquest_data();


        if ( copyover_person == NULL )
        {
             sprintf( log_buf, "Can't do a copyover. Copyover person is null. " );      
             log_string( log_buf );
        }  

        for ( d = descriptor_list; d; d = d->next )
        {
             if ( d->character!= NULL )
                 gch = d->character;
             else 
                 continue;
#ifdef     MCCP_ENABLED
           if ( gch->desc->out_compress )
           {
               if ( !compressEnd( gch->desc ) )
                   send_to_char( "Could not disable compression you'll have to reconnect in five seconds!\n\r", gch );
           }
#endif
        }
         
        for(d = descriptor_list; d; d = d->next )
	{
	   wch = CH(d);
            
	   if( d->pString && !disregard )
	   {
              if (!str_cmp( arg, "timer" ) )
              {
		  sprintf(buf, "%s is currently in a string editor.\r\n", copyover_person->name);
                  send_to_char( buf, copyover_person );
              }
              else
              {
		  sprintf(buf, "%s is currently in a string editor.\r\n", wch->name);
	  	  send_to_char(buf, ch);
		  return;
              }
	   }
	}
   
        if ( copyover_person == NULL )
        {

        if ( timed_copyover )
        {
           send_to_char( "Not while timed copyover is turned on.\n\r", copyover_person );
           return;
        }
        if ( treboot )
        {
            send_to_char( "Not while timed reboot is turned on.\n\r", copyover_person ); 
            return;
        }
        if ( tshutdown )
        {
            send_to_char( "Not while timed shutdown is turned on.\n\r", copyover_person );
            return;
        }
        }
            
        else if ( ch != NULL && copyover_person == NULL )
        {

        if ( timed_copyover )
        {
           send_to_char( "Not while timed copyover is turned on.\n\r", ch );
           return;
        }
        if ( treboot )
        {
            send_to_char( "Not while timed reboot is turned on.\n\r", ch ); 
            return;
        }
        if ( tshutdown )
        {
            send_to_char( "Not while timed shutdown is turned on.\n\r", ch );
            return;
        }
        }

	/* Consider changing all saved areas here, if you use OLC */
	if(asave)
        do_function(NULL, &do_asave, "changed" );   

        if (!str_cmp( arg, "timer" ) )
            sprintf( buf, "\n\r *** TIMED COPYOVER by %s - please remain seated!\n\r", copyover_person->name );

        else
 	    sprintf (buf, "\n\r *** COPYOVER by %s - please remain seated!\n\r", ch->name);

	/* For each playing descriptor, save its state */
	for (d = descriptor_list; d ; d = d_next)
	{
		CHAR_DATA * och = CH (d);
		d_next = d->next; /* We delete from the list , so need to save this */
		
		if ( d->connected > CON_PLAYING && !disregard) /* drop those logging on */
		{
                    if (!str_cmp( arg, "timer" ) )
                    send_to_char("Nuh uh! No timed copyover while people aren't playing. Check your sockets.\n\r", copyover_person ); 
                    else
	 	    send_to_char("Nuh uh! No copying over while people aren't playing. Check your sockets!",ch);
		}
		else
		{	if( d->connected != CON_PLAYING)
			{
                           write_to_buffer(d, "Sorry, we're running a timed Copyover. Be back soon!\n\r",0);
			   close(d->descriptor);
			   continue;
			}
			fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);

#if 0			 /* This is not necessary for ROM */
			if (och->level == 1)
			{
				write_to_descriptor (d->descriptor, "Since you are level one, and level one characters do not save, you gain a free level!\n\r", 0);
				advance_level (och);
				och->level++; /* Advance_level doesn't do that */
			}
#endif			
			save_char_obj (och);
#ifdef MCCP_ENABLED
			write_to_descriptor_2 (d->descriptor, buf, 0);
#else
			write_to_descriptor (d->descriptor, buf, 0);
#endif
		}
	}
	
	fprintf (fp, "-1\n");
	file_close(fp);
	
	/* Close reserve and other always-open files and release other resources */

        /*Save boot time*/
        if( (fpBoot = file_open(BOOT_FILE, "w" ) ) )
            fprintf(fpBoot, "%d\n\r", first_boot_time );
	fclose(fpBoot);   

	fpObj = file_open("../area/objcopy.txt", "w" );

    for ( obj = object_list; obj != NULL; obj = obj->next )
	{   has_reset = FALSE;
		if(obj->carried_by )
			continue;
		if(obj->item_type != ITEM_CORPSE_PC )
			continue;
		for ( pReset = obj->in_room->reset_first; pReset; pReset = pReset->next )
		{   if(( (pReset->command == 'O' || pReset->command == 'P') && pReset->arg3 == obj->pIndexData->vnum ) ||
				( pReset->command == 'E' && pReset->arg1 == obj->pIndexData->vnum ) )
			{	has_reset = TRUE;
				break;
			}
		}
		if(has_reset)
			continue;
		fwrite_obj(NULL, obj, fpObj, 0 );
	}
	fprintf(fpObj, "#END\n");
	file_close(fpObj);
	/* exec - descriptors are inherited */
	
	sprintf (buf, "%d", port);
	sprintf (buf2, "%d", control);
	execl (EXE_FILE, "shadow", buf, "copyover", buf2, (char *) NULL);

	/* Failed - sucessful exec will not return */
	
	perror ("do_copyover: execl");
       
        if ( !str_cmp( arg, "timer" ) )
	send_to_char ("Timed copyover FAILED!\n\r",copyover_person);

        else
	send_to_char ("Copyover FAILED!\n\r",ch);
}

void load_copyover_obj(void)
{	FILE *fpObj;
	char letter;
    char *word;
	
	if(!file_exists("../area/objcopy.txt" ) )
		return;
	fpObj = file_open("../area/objcopy.txt", "r" );
	unlink("../area/objcopy.txt"); //To prevent from char's doubling corpses.
	logf2("Loading pc corpses.");
	for(;;)
	{	

        letter = fread_letter( fpObj );
        if ( letter == '*' )
        {	fread_to_eol( fpObj );
            continue;
        }
	    if ( letter != '#' )
        {	bug( "Load_char_obj: # not found.", 0 );
            break;
        }
		logf2("Freading word.");
        word = fread_word( fpObj );
			 if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( NULL, fpObj );
        else if ( !str_cmp( word, "O"      ) ) fread_obj  ( NULL, fpObj );
        else if ( !str_cmp( word, "END"    ) ) break;
        else
        {	bug( "Load_copyover_obj: bad section.", 0 );
            break;
        }
	}
	file_close(fpObj);
	return;
}

void do_avatar( CHAR_DATA *ch, char *argument ) 
{  
    char buf[MAX_STRING_LENGTH];	/* buf */
    char arg1[MAX_INPUT_LENGTH];	/* arg1 */
    OBJ_DATA *obj_next;			/* obj data which is a pointer */
    OBJ_DATA *obj;			/* obj */
    int level;				/* level */
    int iLevel;				/* ilevel */

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        send_to_char( "Syntax: avatar <level>.\n\r", ch );
        return;
    }

    if ( IS_NPC(ch) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( ( level = atoi( arg1 ) ) < 1 || level > MAX_LEVEL )
    {
       sprintf(buf, "Level must be 1 to %d.\n\r", MAX_LEVEL );
       send_to_char( buf, ch );
       return;
    }

    if ( level > get_trust( ch ) )
    {
        send_to_char( "Limited to your trust level.\n\r", ch );
        sprintf(buf, "Your Trust is %d.\n\r",ch->trust);
        send_to_char(buf,ch);
        return;
    }

    if(ch->trust == 0) 
    {
       ch->trust = ch->level;
    }

    if ( level <= ch->level )
    {
       int temp_prac;

        send_to_char( "Lowering a player's level!\n\r", ch );
        send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r",ch );
        temp_prac = ch->practice;
        ch->level    = 1;
        ch->exp      = exp_per_level(ch,ch->pcdata->points);
        ch->max_hit  = 20;
        ch->max_mana = 100;
        ch->max_move = 100;
        ch->practice = 0;
        ch->hit      = ch->max_hit;
        ch->mana     = ch->max_mana;
        ch->move     = ch->max_move;
        advance_level( ch, TRUE );
        ch->practice = temp_prac;
    }
    else
    {
        send_to_char( "Raising a player's level!\n\r", ch );
        send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", ch);
    }

    for ( iLevel = ch->level ; iLevel < level; iLevel++ )
    {
        ch->level += 1;
        advance_level( ch,TRUE);
    }

    sprintf(buf,"You are now level %d.\n\r",ch->level);
    send_to_char(buf,ch);
    ch->exp   = exp_per_level(ch,ch->pcdata->points)
                 * UMAX( 1, ch->level );

    if ( ch->level < LEVEL_IMMORTAL )
    {
        for ( obj = ch->carrying; obj; obj = obj_next )       
        {
             obj_next = obj->next_content;
   
             if (obj->wear_loc != WEAR_NONE && can_see_obj (ch, obj))
                 remove_obj (ch, obj->wear_loc, TRUE);
        }
    }

    save_char_obj(ch); /* save character */
    return;
}

void do_fvlist (CHAR_DATA *ch, char *argument)
{
  int i;
  char arg[MAX_INPUT_LENGTH];
  char *string;

  string = one_argument(argument,arg);
 
  if (arg[0] == '\0')
  {
      send_to_char("Syntax:\n\r",ch);
      send_to_char("  fvlist obj\n\r",ch);
      send_to_char("  fvlist mob\n\r",ch);
      send_to_char("  fvlist room\n\r",ch);
      return;
  }
 
  if (!str_cmp(arg,"obj"))
  {
      printf_to_char(ch,"{WFree {C%s{W vnum listing for area {C%s{x\n\r",arg,
		     ch->in_room->area->name);
      
printf_to_char(ch,"{Y=============================================================================={C\n\r");
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) 
      {
 	   if (get_obj_index(i) == NULL) 
           {
	       printf_to_char(ch,"%8d, ",i);
	 
	   }
      }
      send_to_char("{x\n\r",ch);
      return;
  }

  if (!str_cmp(arg,"mob"))
  { 
      printf_to_char(ch,"{WFree {C%s {Wvnum listing for area {C%s{x\n\r",arg,
		     ch->in_room->area->name);
      
printf_to_char(ch,"{Y=============================================================================={C\n\r");
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) 
      {
	if (get_mob_index(i) == NULL) 
        {
	  printf_to_char(ch,"%8d, ",i);
	}
      }
      send_to_char("{x\n\r",ch);
      return;
  }
  if (!str_cmp(arg,"room"))
  { 
      printf_to_char(ch,"{WFree {C%s {Wvnum listing for area {C%s{x\n\r",arg,
		     ch->in_room->area->name);
      
printf_to_char(ch,"{Y=============================================================================={C\n\r");
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) 
      {
	if (get_room_index(i) == NULL) 
        {
	  printf_to_char(ch,"%8d, ",i);
	}
      }
      send_to_char("{x\n\r",ch);
      return;
  }
  send_to_char("WHAT??? \n\r",ch);
  send_to_char("Syntax:\n\r",ch);
  send_to_char("  fvlist obj\n\r",ch);
  send_to_char("  fvlist mob\n\r",ch);
  send_to_char("  fvlist room\n\r",ch);

}


void do_ifollow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC(ch))
    {
      send_to_char("Sorry, NPC's can't do this.\n\r",ch);
      return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "Follow whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if (IS_NPC(victim))
    {
        send_to_char( "NPC's cannot be tracked\n\r", ch);
        return;
    }

    if (ch->pcdata->tracking != NULL)
    {
      if( ch->pcdata->tracking == victim)
      {
	send_to_char("You are already tracking them.\n\r",ch);
	return;
      }
      sprintf(buf,"You stop tracking %s.\n\r", ch->pcdata->tracking->name );
      send_to_char(buf,ch);
      ch->pcdata->tracking->pcdata->track_count--;
      ch->pcdata->tracking = NULL;
      if(victim == ch) return;

    }
    else if(victim == ch)
    {
      send_to_char("You can't follow yourself.\n\r",ch);
      return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    victim->pcdata->track_count++;
    ch->pcdata->tracking = victim;
    sprintf(buf,"You start tracking %s\n\r", victim->name );
    send_to_char( buf, ch );
    return;
}

void do_rename(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char strsave[MAX_INPUT_LENGTH];
  char *name;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char playerfile[MAX_INPUT_LENGTH];


  if (!IS_IMMORTAL(ch))
  {
    send_to_char("You don't have the power to do that.\n\r",ch);
    return;
  }

  argument = one_argument(argument,arg1);
  argument = one_argument(argument,arg2);

  if (arg1[0] == '\0')
  {
    send_to_char("Rename who?\n\r",ch);
    return;
  }

  if (arg2[0] == '\0')
  {
    send_to_char("What should their new name be?\n\r",ch);
    return;
  }

  arg2[0] = UPPER(arg2[0]);

  if ((victim = get_char_world(ch,arg1)) == NULL)
  {
    send_to_char("They aren't connected.\n\r",ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Use string for NPC's.\n\r",ch);
    return;
  }

  if (!check_parse_name(arg2))
  {
    sprintf(buf,"The name {c%s{x is {Rnot allowed{x.\n\r",arg2);
    send_to_char(buf,ch);
    return;
  }

  sprintf(playerfile, "%s%s", PLAYER_DIR, capitalize(arg2));

  if (file_exists(playerfile))
  {
    sprintf(buf,"There is already someone named %s.\n\r",capitalize(arg2));
    send_to_char(buf,ch);
    return;
  }

  if ((victim->level >= ch->level) && (victim->level >= ch->trust)
    &&((ch->level != IMPLEMENTOR) || (ch->trust != IMPLEMENTOR))
    &&(ch != victim))
  {
    send_to_char("I don't think that's a good idea.\n\r",ch);
    return;
  }

  if (victim->position == POS_FIGHTING)
  {
    send_to_char("They are fighting right now.\n\r",ch);
    return;
  }

  name = str_dup(victim->name);
  sprintf( strsave, "%s%s", PLAYER_DIR, capitalize(victim->name));
  arg2[0] = UPPER(arg2[0]);
  free_string(victim->name);
  victim->name = str_dup(arg2);
  save_char_obj(victim);
  unlink(strsave);
#if defined(unix)
  if (IS_IMMORTAL(victim))
  {
    sprintf(strsave,"%s%s", GOD_DIR, capitalize(name));
    unlink(strsave);
  }
#endif
  if (victim != ch)
  {
    sprintf(buf,"{YNOTICE: {xYou have been renamed to {c%s{x.\n\r",arg2);
    send_to_char(buf,victim);
  }

  send_to_char("Done.\n\r",ch);

  return;
}
int days_since_last_file_mod( char *filename )
{
    int days;
    struct stat buf;
    extern time_t current_time;

    if (!stat(filename, &buf))
    {
	days = (current_time - buf.st_mtime)/86400;
    }
    else
	days = 0;

    return days;
}

void do_pwipe (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];	
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char strsave[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d = NULL;    
    CHAR_DATA *victim;
    bool isChar = FALSE;
    int i;

  argument = one_argument( argument, arg1 );
  one_argument( argument, arg2 );
  
  if ( arg1[0] == '\0' )
  {
	send_to_char( "Syntax: pwipe [person] [text]\n\r", ch );
	return;
  }

  if(is_exact_name(arg1, "Davion" ) && !is_admin(ch) )
  {
	send_to_char("Only Admins can Pwipe them!\n\r",ch);
	return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  {
 	MEMBER_DATA *pMem, *temp;
        isChar = load_char_obj(d, arg1);

	if (!isChar)
	{ /*No pfile? Oops. */
	
		send_to_char("Who are you talking about?\n\r", ch);
		return;
	}

	for( i = 0 ; i < MAX_CLAN ; i++ )
 	{    if( ( pMem = member_lookup(&clan_table[i], arg1 ) ) == NULL )
		continue;
	     else
		break;
	}

	if(pMem )
	{    if(pMem == clan_table[i].member_first )
	     {   clan_table[i].member_first = pMem->next;
		 free_member(pMem);
	     }
	     else
	     for(temp = clan_table[i].member_first ; temp ; temp = temp->next )
	     {   if(temp->next == pMem )
		 {   temp->next = pMem->next;
		     free_member(pMem);
		 }
	     }
	}

        if (arg2[0] == '\0' )
        {    
            if (!pshow)
            {   
                pshow = FALSE;
                sprintf( buf, "{G[INFO]:{x %s has been cleansed from the hard drive.\n\r", capitalize (arg1) );
                do_echo( ch, buf );
            }
        }
	else
	{
        sprintf( buf, "{G[INFO]:{x %s has been cleansed from the hard drive.\n\rReason: %s\n\r", capitalize (arg1), argument );
        do_echo( ch, buf );
        }
        
        for ( d = descriptor_list; d; d = d->next )
        {
	     if ( d->connected == CON_PLAYING )
	     {
	         send_to_char( buf, d->character );
	     }
        }

        sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( arg1 ) );
 	unlink(strsave);
        sound_to_room( "evil-laf.wav", ch, TRUE );
        return;
  }

    if (IS_NPC(victim))
    {
      send_to_char("You can't wipe mobs.\n\r", ch);
      return;
    }

    if (!IS_NPC(victim) && get_trust(victim) >= get_trust(ch))
    {
      send_to_char("I don't think so.\n\r", ch);
      printf_to_char( victim, "%s just tried to pwipe you!\n\r", ch->name );
      return;
    }

    if ( ch == victim )
    {
      send_to_char("Umm, ok, retard.\n\r", ch);
      return;
    }

    if (arg2[0] == '\0' )
    {
        sprintf( buf, "{G[INFO]:{x %s has been cleansed from the hard drive.\n\r", victim->name );
        do_echo( ch, buf );
    }
    else
    {
        sprintf( buf, "{G[INFO]:{x %s has been cleansed from the hard drive.\n\rReason: %s\n\r", victim->name, argument );    
        do_echo( ch, buf );
    }

    for ( d = descriptor_list; d; d = d->next )
    {
  	 if ( d->connected == CON_PLAYING )
	 {
	     send_to_char( buf, d->character );
	 }
    }

    if (victim->level > HERO)
    {
        update_wizlist(victim, 1);
    } 

    sound( "evil-laf.wav", victim);
    sound( "evil-laf.wav", ch );   
    do_quit( victim, "" );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( arg1) );
    unlink(strsave);
}

void do_omni( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int immmatch;
    int mortmatch;
    int hptemp;
 
 /*
 * Initalize Variables.
 */

    immmatch = 0;
	mortmatch = 0;
    buf[0] = '\0';
    output = new_buf();

 /*
 * Count and output the IMMs.
 */

	sprintf( buf, " ----Immortals:----\n\r");
	add_buf(output,buf);
	sprintf( buf, "Name          Level   Wiz   Incog   [ Vnum]\n\r");
	add_buf(output,buf);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;

        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
 
        wch   = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch)  || wch->level < LEVEL_IMMORTAL)
	    continue;
 
        immmatch++;

	sprintf( buf, "%-14s %d     %-2d     %-2d     [%5d]\n\r",
			wch->name,
			wch->level,
			wch->invis_level,
			wch->incog_level,
			wch->in_room->vnum);
			add_buf(output,buf);
    }
    
    
 /*
 * Count and output the Morts.
 */
	sprintf( buf, " \n\r ----Mortals:----\n\r");
	add_buf(output,buf);
	sprintf( buf, "Name           Race/Class   Position        Lev  %%hps  [ vnum]\n\r");
	add_buf(output,buf);
	hptemp = 0;
	
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
        char const *klass;
        
        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
 
        wch   = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch) || wch->level > ch->level || wch->level > LEVEL_IMMORTAL)
	    continue;
 
        mortmatch++;
 	if ((wch->max_hit != wch->hit) && (wch->hit > 0))
 		hptemp = (wch->hit*100)/wch->max_hit;
 	else if (wch->max_hit == wch->hit)
 		hptemp = 100;
 	else if (wch->hit < 0)
 		hptemp = 0;
 		
	klass = class_short( wch );

	sprintf( buf, "%-14s %5s/%3s    %-15s %-2d   %3d%%   [%5d]\n\r",
		wch->name,
		wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name : "     ",
	    klass,
		capitalize( position_table[wch->position].name) , 
		wch->level,
		hptemp,
		wch->in_room->vnum);
	add_buf(output,buf);
    }

/*
 * Tally the counts and send the whole list out.
 */
    sprintf( buf2, "\n\rIMMs found: %d\n\r", immmatch );
    add_buf(output,buf2);
    sprintf( buf2, "Morts found: %d\n\r", mortmatch );
    add_buf(output,buf2);
    page_to_char( buf_string(output), ch );
    free_buf(output);
    return;
}

void do_implag(CHAR_DATA *ch, char *argument)
{
   char      arg[MAX_INPUT_LENGTH];
   char      buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
       send_to_char("Who you wanna lag?\n\r", ch);
       return;
   }

   if ((victim = get_char_world( ch, arg ) )==NULL )
   {
       send_to_char("They aren't playing you dufus!\n\r", ch);
       return;
   }

      
   if (IS_NPC(victim))
   {
       send_to_char("Not on NPC's!\n\r", ch);
       return;
   }
      
   else if (IS_SET(victim->act, PLR_IMPLAG))
   {
       REMOVE_BIT(victim->act, PLR_IMPLAG);
       sprintf(buf, "You make %s's life better.\n\r", victim->name);
       send_to_char(buf, ch);
       sprintf(buf, "You suddenly look at the world normally.\n\r");
       send_to_char(buf, victim); 
       return;
   }
      
   else
   {
         SET_BIT(victim->act, PLR_IMPLAG);
         sprintf(buf, "You make %s's life totally suck.\n\r", victim->name);
         send_to_char(buf, ch);
         return;
   }
}


void do_unrestore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            vch->hit  = vch->max_hit - vch->max_hit + 1;
            vch->mana = vch->max_mana - vch->max_mana + 1;
            vch->move = vch->max_move - vch->max_move + 1;
            update_pos( vch);
            act("{D*{c*{C*{W* $n has unrestored you *{C*{c*{D*{x",ch,NULL,vch,TO_VICT);
        }

        sprintf(buf,"$N unrestored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
        
        send_to_char("Room unrestored.\n\r",ch);
        return;

    }
    
    if ( get_trust(ch) >=  MAX_LEVEL - 1 && !str_cmp(arg,"all"))
    {
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;
                
           victim->hit  = victim->max_hit - victim->max_hit + 1;
           victim->mana = victim->max_mana - victim->max_mana + 1;
           victim->move = victim->max_move - victim->max_move + 1;
            update_pos( victim);
	    if (victim->in_room != NULL)
                act("{D*{c*{C*{W* $n has unrestored you *{C*{c*{D*{x",ch,NULL,victim,TO_VICT);
        }
	send_to_char("All active players unrestored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    victim->hit  = victim->max_hit - victim->max_hit + 1;
    victim->mana = victim->max_mana - victim->max_mana + 1;
    victim->move = victim->max_move - victim->max_move + 1;
    update_pos( victim );
    act( "{D*{c*{C*{W* $n has unrestored you *{C*{c*{D*{x", ch, NULL, victim, TO_VICT );
    sprintf(buf,"$N unrestored %s",
	IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_scatter( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  ROOM_INDEX_DATA *pRoomIndex;

  for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next ) 
  {
    obj_next = obj->next_content;
    for ( ; ; )
    {
        pRoomIndex = get_room_index( number_range( 0, 65535 ) );
        if ( pRoomIndex != NULL )
        if ( CAN_ACCESS(pRoomIndex) )
            break;                
    }
    act("$p vanishes in a puff of smoke!",ch,obj,NULL,TO_ROOM);
    obj_from_room(obj);
    obj_to_room(obj, pRoomIndex);
  }
  return;
}       

void do_pload( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA d;
  bool isChar = FALSE;
  char name[MAX_INPUT_LENGTH];

  if (argument[0] == '\0')
  {
    send_to_char("Load who?\n\r", ch);
    return;
  }

  if(!IS_NPC(ch) && IN_MINE(ch) )
  { 
     send_to_char("Don't pload in a mine!\n\r",ch);
     return;
  }

  argument[0] = UPPER(argument[0]);
  argument = one_argument(argument, name);

  /* Dont want to load a second copy of a player who's allready online! */
  if ( get_char_world( ch, name ) != NULL )
  {
    send_to_char( "That person is allready connected!\n\r", ch );
    return;
  }

  isChar = load_char_obj(&d, name); /* char pfile exists? */

  if (!isChar)
  {
    send_to_char("Load Who? Are you sure? I cant seem to find them.\n\r", ch);
    return;
  }

  d.character->desc     = NULL;
  d.character->next     = char_list;
  char_list             = d.character;
  d.connected           = CON_PLAYING;
  reset_char(d.character);

  /* bring player to imm */
  if ( d.character->in_room != NULL )
  {
    char_to_room( d.character, ch->in_room); /* put in room imm is in */
  }

  act( "You have pulled $N from the pattern!", ch, NULL, d.character, TO_CHAR );
  act( "$n has pulled $N from the pattern!",
        ch, NULL, d.character, TO_ROOM );

  if (d.character->pet != NULL)
   {
     char_to_room(d.character->pet,d.character->in_room);
     act("$n has entered the game.",d.character->pet,NULL,NULL,TO_ROOM);
   }

}

void do_punload( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char who[MAX_INPUT_LENGTH];

  argument = one_argument(argument, who);

  if ( ( victim = get_char_world( ch, who ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  /** Person is legitametly logged on... was not ploaded.
   */
  if (victim->desc != NULL)
  {
    send_to_char("I dont think that would be a good idea...\n\r", ch);
    return;
  }

  if (victim->was_in_room != NULL) 
  {
    char_to_room(victim, victim->was_in_room);
    if (victim->pet != NULL)
      char_to_room(victim->pet, victim->was_in_room);
  }

  save_char_obj(victim);
  do_quit(victim,"");

  act("$n has released $N back to the Pattern.",
       ch, NULL, victim, TO_ROOM);
}

void do_wpeace(CHAR_DATA *ch, char *argument )
{
     CHAR_DATA *rch;
     char buf[MAX_STRING_LENGTH];

     rch = get_char( ch );

     for ( rch = char_list; rch; rch = rch->next )
     {
	if ( ch->desc == NULL || ch->desc->connected != CON_PLAYING )
	   continue;

	if ( rch->fighting )
        {
 	   sprintf( buf, "%s has declared World Peace.\n\r", ch->name );
	   send_to_char( buf, rch );
	   stop_fighting( rch, TRUE );
        }
      }

      send_to_char( "You have declared World Peace.\n\r", ch );
      return;
}

void do_skillstat(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer;
    char arg[MAX_INPUT_LENGTH];
    char skill_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
    char skill_columns[LEVEL_HERO + 1];
    int sn, level, min_lev = 1, max_lev = LEVEL_HERO;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if (IS_NPC(ch))
    return;
    if ( arg[0] == '\0' )
    {
        send_to_char( "List skills for whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if (IS_NPC(victim))
    {
        send_to_char( "Use this for skills on players.\n\r", ch );
        return;
    }

    /* initialize data */
    for (level = 0; level < LEVEL_HERO + 1; level++)
    {
        skill_columns[level] = 0;
        skill_list[level][0] = '\0';
    }

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL )
        break;

        if (( level = skill_level( victim, sn ) < LEVEL_HERO+ 1 )
        &&  level >= min_lev && level <= max_lev
        &&  (skill_table[sn].spell_fun == spell_null)
        &&  victim->pcdata->learned[sn] > 0 )
        {
            found = TRUE;
            level = skill_level( victim, sn );
            if (victim->level < level)
                sprintf(buf,"%-18s n/a      ", skill_table[sn].name);
            else
                sprintf(buf,"%-18s %3d%%      ",skill_table[sn].name,
                victim->pcdata->learned[sn]);

            if (skill_list[level][0] == '\0')
                sprintf(skill_list[level],"\n\rLevel %2d: %s",level,buf);
            else /* append */
            {
                if ( ++skill_columns[level] % 2 == 0)
                    strcat(skill_list[level],"\n\r          ");
                    strcat(skill_list[level],buf);
            }
        }
    }

    /* return results */

    if (!found)
    {
        send_to_char("No skills found.\n\r",ch);
        return;
    }

    buffer = new_buf();
    for (level = 0; level < LEVEL_HERO + 1; level++)
        if (skill_list[level][0] != '\0')
            add_buf(buffer,skill_list[level]);
            add_buf(buffer,"\n\r");
            page_to_char(buf_string(buffer),ch);
            free_buf(buffer);
}

void do_spellstat(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer;
    char buff[100];
    char arg[MAX_INPUT_LENGTH];
    char spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
    char spell_columns[LEVEL_HERO + 1];
    int sn, gn, col, level, min_lev = 1, max_lev = LEVEL_HERO, mana;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
  
    argument = one_argument( argument, arg );

    if (IS_NPC(ch))
    return;

    if ( arg[0] == '\0' )
    {
        send_to_char( "List spells for whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if (IS_NPC(victim))
    {
        send_to_char( "Use this for skills on players.\n\r", ch );
        return;
    }

    /* groups */

    col = 0;

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
        if (group_table[gn].name == NULL)
        break;
        if (victim->pcdata->group_known[gn])
        {
            sprintf(buff,"%-20s ",group_table[gn].name);
            send_to_char(buff,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
    {
        send_to_char( "\n\r", ch );
        sprintf(buff,"Creation points: %d\n\r",victim->pcdata->points);
        send_to_char(buff,ch);
    }
    
    /* initialize data */
    for (level = 0; level < LEVEL_HERO + 1; level++)
    {
        spell_columns[level] = 0;
        spell_list[level][0] = '\0';
    }

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL )
        break;

        if ((skill_level( victim, sn )  < LEVEL_HERO+ 1
        &&  level >= min_lev && level <= max_lev
        &&  skill_table[sn].spell_fun != spell_null
        &&  victim->pcdata->learned[sn] > 0) )
        {
            found = TRUE;
            level = skill_level( victim, sn );

            if (victim->level < level)
                sprintf(buf,"%-18s n/a      ", skill_table[sn].name);
            else
            {
                mana = UMAX(skill_table[sn].min_mana,
                100/(2 + victim->level - level));
                sprintf(buf,"%-18s  %3d mana  ",skill_table[sn].name,mana);
            }

            if (spell_list[level][0] == '\0')
                sprintf(spell_list[level],"\n\rLevel %2d: %s",level,buf);
            else /* append */
            {
                if ( ++spell_columns[level] % 2 == 0)
                strcat(spell_list[level],"\n\r          ");
                strcat(spell_list[level],buf);
            }
        }
    }

    /* return results */

    if (!found)
    {
        send_to_char("No spells found.\n\r",ch);
        return;
    }

    buffer = new_buf();
    for (level = 0; level < LEVEL_HERO + 1; level++)
        if (spell_list[level][0] != '\0')
            add_buf(buffer,spell_list[level]);
            add_buf(buffer,"\n\r");
            page_to_char(buf_string(buffer),ch);
            free_buf(buffer);
}

void do_slist(CHAR_DATA *ch,char *argument) 
{
   char arg1[MIL],group[MSL];
   int skill,klass = 0,cl = 0;
   int sn = 0,col = 0,level = 0, sp,gr;
   int skCount = 0,spCount = 0, class_name = 0, gn = 0;
   bool found = FALSE;

   argument = one_argument(argument,arg1);

   if (arg1[0] == '\0') 
   {
        printf_to_char(ch,"Enter the name of the spell or class.\n\r");
        return;
   }
        if (((skill = skill_lookup(arg1)) == -1) && (cl = class_lookup(arg1)) == -1) 
        {
        printf_to_char(ch,"That is not a valid klass or skill.\n\r");
        return;
    }

    if (((skill = skill_lookup(arg1)) == -1) && (cl = class_lookup(arg1)) != -1) {
      for (class_name = 0; class_name < MAX_CLASS; class_name++) {
          if (!str_cmp(arg1, class_table[class_name].name))
            break;
      }
      if (class_name == MAX_CLASS) {
        send_to_char( "Please spell out the full klass name.\n\r",ch);
        return;
      } else {
          printf_to_char(ch,"{CAviable skill/spells for the class{c: {W%s{x\n\r", class_table[cl].name);
          printf_to_char(ch,"{cSpells{W:{x\n\r");
          for (level = 1 ;level <= 101;level++) {
              for (sn = 0; sn < MAX_SKILL; sn++) {
                  if (skill_table[sn].skill_level[cl] > 100)
                      continue;
                  if (skill_table[sn].skill_level[cl] != level)
                      continue;
                  if (skill_table[sn].spell_fun == spell_null)
                      continue;
                  printf_to_char(ch, "{B[{C%3d{B] {W%-18s{x",
                  level,
                  skill_table[sn].name);
                  spCount++;
                  if (++col % 3 == 0)
                      send_to_char("\n\r",ch);
              }
          }
          send_to_char("\n\r",ch); col = 0;
          printf_to_char(ch, "{CTotal spells found{g: {W%d{x\n\r",spCount);
          printf_to_char(ch, "{cSkills{W:{x\n\r");
          for (level = 1 ;level <= 101;level++) {
              for (sn = 0; sn < MAX_SKILL; sn++) {
                  if (skill_table[sn].skill_level[cl] > 100)
                      continue;
                  if (skill_table[sn].skill_level[cl] != level)
                      continue;
                  if (skill_table[sn].spell_fun != spell_null)
                      continue;
                  printf_to_char(ch, "{B[{C%3d{B] {W%-18s{x",
                  level,
                  skill_table[sn].name);
                  skCount++;
                  if (++col % 3 == 0)
                      send_to_char("\n\r",ch);
              }
          }
          send_to_char("\n\r",ch);
          printf_to_char(ch, "{cBase Group{W: {W%s{x\n\r",class_table[cl].base_group);
          gn = 0; col = 0; level = 1;
          gn = group_lookup(class_table[klass].base_group);
          printf_to_char(ch, "{CSkills included with {W%s{x.{x\n\r",class_table[cl].base_group);
              for (sn = 0; sn < MAX_IN_GROUP; sn++) {
                  if (group_table[gn].spells[sn] == NULL)
                      continue;
                  
printf_to_char(ch,"{W%-18s{x",group_table[gn].spells[sn]);
                  if (++col % 3 == 0)
                      send_to_char("\n\r",ch);
              }
          send_to_char("\n\r",ch);
          printf_to_char(ch, "{cDefault Group{W: {W%s{x\n\r",class_table[cl].default_group);
          gn = 0; col = 0; level = 1;
          gn = group_lookup(class_table[cl].default_group);
          printf_to_char(ch, "{CSkills included with {W%s{x.{x\n\r",class_table[cl].default_group);
              for (sn = 0; sn < MAX_IN_GROUP; sn++) {
                  if (group_table[gn].spells[sn] == NULL)
                      continue;
                  
printf_to_char(ch,"{W%-18s{x",group_table[gn].spells[sn]);
                  if (++col % 3 == 0)
                      send_to_char("\n\r",ch);
              }
          send_to_char("\n\r",ch);
          printf_to_char(ch, "{CTotal skills found{g: {W%d{x\n\r",skCount);
          printf_to_char(ch, "{WTotal skills/spells found: {B%d{x\n\r",(spCount + skCount));
          return;
      }
    }
    if (((skill = skill_lookup(arg1)) != -1) && (cl = class_lookup(arg1)) == -1) {
        for (gr = 1; gr < MAX_GROUP; gr++) {
          if (group_table[gr].name == NULL) {
            continue;
          }
          for (sp = 0; sp < MAX_IN_GROUP; sp++) {
            if (group_table[gr].spells[sp] == NULL) {
              continue;
            }
            if(group_table[gr].spells[sp] == skill_table[skill].name) {
              found = TRUE;
              sprintf(group,"%s",capitalize(group_table[gr].name));
              break;
            }
          }
        }
        printf_to_char(ch,"{CSettings for skill/spell{c: {W%s{x\n\r",skill_table[skill].name);
        if (found)
          printf_to_char(ch,"{CIn the group: {W%s{C.{x\n\r",group);
        else
          printf_to_char(ch,"{CIn the group: {WUnknown{C!{x\n\r");
        send_to_char("{cClass       Level Gain Wait-Time Min-Mana Rating{x\n\r",ch);
        send_to_char("{W----------- ----- ---- --------- -------- ------{x\n\r",ch);
        for (klass = 0;klass < MAX_CLASS;klass++) {
        if (skill_table[skill].skill_level[klass] > 100)
            continue;
        printf_to_char(ch,"{G%-11s {g%5d %4d %9d %8d %6d\n\r",
        class_table[klass].name,
        skill_table[skill].skill_level[klass],
        skill_table[skill].rating[klass],
        skill_table[skill].beats,
        skill_table[skill].min_mana,
        skill_table[skill].rating[klass]);
        }
        return;
    }
}

void do_olevel(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];
    char level[MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();

    argument = one_argument(argument, level);
    if (level[0] == '\0')
    {
        send_to_char("Syntax: olevel <level>\n\r",ch);
        send_to_char("        olevel <level> <name>\n\r",ch);
        return;
    }
 
    argument = one_argument(argument, name);
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( obj->level != atoi(level) )
            continue;

	if ( name[0] != '\0' && !is_name(name, obj->name) )
	    continue;

        found = TRUE;
        number++;
 
        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );
 
        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
        &&   in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
                number, obj->short_descr,PERS(in_obj->carried_by, ch),
                in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
            sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
                number, obj->short_descr,in_obj->in_room->name, 
                in_obj->in_room->vnum);
        else
            sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);
 
        buf[0] = UPPER(buf[0]);
        add_buf(buffer,buf);
 
        if (number >= max_found)
            break;
    }
 
    if ( !found )
        send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}

void do_mlevel( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	send_to_char("Syntax: mlevel <level>\n\r",ch);
	return;
    }

    found = FALSE;
    buffer = new_buf();
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
        if ( victim->in_room != NULL
        &&   atoi(argument) == victim->level )
        {
            found = TRUE;
            count++;
            sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
                IS_NPC(victim) ? victim->pIndexData->vnum : 0,
                IS_NPC(victim) ? victim->short_descr : victim->name,
                victim->in_room->vnum,
                victim->in_room->name );
            add_buf(buffer,buf);
        }
    }

    if ( !found )
        act( "You didn't find any mob of level $T.", ch, NULL, argument, TO_CHAR );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}

void do_divine( CHAR_DATA *ch, char *argument )
{
    char arg[MSL];
    char buf[MSL];
    CHAR_DATA *victim;
    AFFECT_DATA af;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;
    int sn = 0;
    bool followers = FALSE;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: divine <player>\n\rSyntax: divine all\n\rSyntax: divine followers <Gods only>\r\n", ch );
        return;
    }
   
    if ( !str_cmp( arg, "all" ) || !str_cmp( arg, "followers" ) )
    {

        for (d = descriptor_list; d != NULL; d = d->next)
        {
            victim = d->character;

            obj = get_eq_char( victim, WEAR_WIELD );    

            af.where     = TO_AFFECTS;
            af.level	 = ch->level;
            af.duration	 = (ch->level + 3) / 2;
            af.bitvector = 0;
            af.modifier  = (ch->level / 30);
            af.location  = APPLY_AC;
            af.type      = skill_lookup( "armor" );
            affect_to_char(victim,&af);     
            af.modifier  = 8 * (ch->level / 24);
            af.location  = APPLY_HITROLL;
            af.type      = skill_lookup( "bless" );
            affect_to_char(victim,&af);
            af.location  = APPLY_DAMROLL;
            af.type      = skill_lookup( "bless" );
            affect_to_char(victim,&af);
            af.location  = APPLY_STR;
            af.type      = skill_lookup( "giant" );
            affect_to_char(victim,&af);
            af.location  = APPLY_DEX;
            af.type      = skill_lookup( "haste" );
            affect_to_char(victim,&af);
            af.modifier  = 0;
            af.duration  = ( ch->level + 50 ) /8;         
            af.location  = APPLY_NONE;
            af.bitvector = AFF_SANCTUARY;
            af.type      = skill_lookup( "sanctuary" );
            affect_to_char(victim,&af);
            if ( obj != NULL )
            {
            af.where     = TO_OBJECT;
            af.type      = sn;
            af.level     =  ch->level;
            af.duration  = ( ch->level + 100 ) /8;
            af.bitvector = ITEM_BLESS;
            af.location  = APPLY_HITROLL;
            af.modifier  = 15;
            affect_to_obj(obj,&af);
            af.location  = APPLY_DAMROLL;
            af.modifier  = 15;
            affect_to_obj(obj,&af);
            act("$p glows a brilliant gold!",ch,obj,NULL,TO_VICT); 
            }
            act("$n has given you divine blessing!",ch,NULL,victim,TO_VICT);     
        }
	if( !followers )
	{
        send_to_char("You have given the entire mud divine grace!\n\r", ch ); 
        info( ch, 0,"{G[INFO]{x {R%s has blessed the mud with their divine grace!{x\n\r", ch->name );
	}
	else
	{
	    send_to_char( "You have given your followers divine grace!\r\n", ch );
	    act("Your deity, $n, has blessed you with divine grace!\r\n",ch,NULL,victim,TO_VICT);
	    act("$n has been given divine grace!",victim,NULL,NULL,TO_ROOM);
	}
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim ) ) 
    {
        send_to_char( "You can't give a mob divine blessing!\n\r", ch );
        return;
    }

    obj = get_eq_char( victim, WEAR_WIELD );    

    af.where     = TO_AFFECTS;
    af.level     = ch->level;
    af.duration  = (ch->level + 3) / 2;
    af.bitvector = 0;    
    af.modifier  = ch->level / 30; 
    af.location  = APPLY_AC;
    af.type      = skill_lookup("armor");
    affect_to_char(victim,&af);
    af.modifier  = 10 * (ch->level / 24); 
    af.location  = APPLY_HITROLL;
    af.type      = skill_lookup( "bless" );
    affect_to_char(victim,&af);
    af.location  = APPLY_DAMROLL;
    af.type      = skill_lookup( "bless" );
    affect_to_char(victim,&af);
    af.location  = APPLY_STR;
    af.type      = skill_lookup("giant");
    affect_to_char(victim,&af);
    af.location  = APPLY_DEX;
    af.type      = skill_lookup("haste");
    affect_to_char(victim,&af);    
    af.modifier  = 0;
    af.duration  = ( ch->level + 50 ) /8;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SANCTUARY;
    af.type      = skill_lookup( "sanctuary" );
    affect_to_char(victim,&af);                
   if ( obj != NULL )
   { 
    af.where     = TO_OBJECT;     
    af.type      = sn;
    af.level     =  ch->level;
    af.duration  = ( ch->level + 100 ) /8;
    af.bitvector = ITEM_BLESS;
    af.location  = APPLY_HITROLL;
    af.modifier  = 15;
    affect_to_obj(obj,&af);
    af.location  = APPLY_DAMROLL;
    af.modifier  = 15;
    affect_to_obj(obj,&af);
    act("$p glows a brilliant gold!",ch,obj,NULL,TO_VICT);         
   } 
    send_to_char("You have been given divine grace!\n\r",victim);
    act("$n has been given divine grace!",victim,NULL,NULL,TO_ROOM);
    sprintf( buf, "You have given %s your divine grace!\n\r", victim->name );
    send_to_char( buf, ch );

    return;
}

void do_astrip(CHAR_DATA *ch,char *argument)
{
    CHAR_DATA *victim;
    char arg[MSL];
    DESCRIPTOR_DATA *d;

    one_argument(argument,arg);

    if (arg[0] == '\0')
        victim = ch;
    else
        victim = get_char_world(ch,arg);      

    if ( !str_cmp( arg, "all" ) )
    {
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            victim = d->character;

            if (victim == NULL || IS_NPC(victim) )
                continue;                                       
         
            while (victim->affected)
                affect_remove(victim,victim->affected);     
        }
   
        send_to_char( "You have removed everyones spell affects.\n\r", ch );
        info( ch, 0, "{R[INFO]:{x %s has just removed everyones spell affects!\n\r", ch->name );        
        return;
    }                                                                                

    while (victim->affected)
           affect_remove(victim,victim->affected);

    if (victim != ch)
    {
        act("All affects stripped from $N.",ch,0,victim,TO_CHAR);
        send_to_char( "All of your spell affects have been removed!\n\r", victim );
    }
    else
        send_to_char("All affects stripped from yourself.\n\r",ch);

    return;        
}


void do_arealinks(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer;
    AREA_DATA *parea;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    ROOM_INDEX_DATA *from_room;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    int vnum = 0;
    int iHash, door;
    bool found = FALSE;

    static char * const dir_name[] = {"north","east","south","west","up","down"};

    argument = one_argument(argument, arg1);

    /* First, the 'all' option */
    if (!str_cmp(arg1,"all"))
    {
	buffer = new_buf();

	for (parea = area_first; parea != NULL; parea = parea->next)
	{
	    /* First things, add area name  and vnums to the buffer */
	    sprintf(buf, "*** %s (%d to %d) ***\n\r",
			 parea->name, parea->min_vnum, parea->max_vnum);

	    found = FALSE;
	    for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	    {
		for( from_room = room_index_hash[iHash];
		     from_room != NULL;
		     from_room = from_room->next )
		{
		    if ( from_room->vnum < parea->min_vnum
		    ||   from_room->vnum > parea->max_vnum )
			continue;

		    for (door = 0; door < 6; door++)
		    {
			/* Does an exit exist in this direction? */
			if( (pexit = from_room->exit[door]) != NULL )
			{
			    to_room = pexit->u1.to_room;

			    if( to_room != NULL
			    &&  (to_room->vnum < parea->min_vnum
			    ||   to_room->vnum > parea->max_vnum) )
			    {
				found = TRUE;
				sprintf(buf, "    (%d) links %s to %s (%d)\n\r",
				    from_room->vnum, dir_name[door],
				    to_room->area->name, to_room->vnum);

				    add_buf(buffer, buf);
			    }
			}
		    }
		}
	    }

	    if (!found)
		add_buf(buffer, "    No links to other areas found.\n\r");
	}

	    page_to_char(buf_string(buffer), ch);
	    free_buf(buffer);

	return;
    }

    if(arg1[0] == '\0')
    {
	parea = ch->in_room ? ch->in_room->area : NULL;

	if (parea == NULL)
	{
	    send_to_char("You aren't in an area right now, funky.\n\r",ch);
	    return;
	}
    }

    else if(is_number(arg1))
    {
	vnum = atoi(arg1);

	if (vnum <= 0 || vnum > 65536)
	{
	    send_to_char("The vnum must be between 1 and 65536.\n\r",ch);
	    return;
	}

	for (parea = area_first; parea != NULL; parea = parea->next)
	{
	    if(vnum >= parea->min_vnum && vnum <= parea->max_vnum)
		break;
	}

	if (parea == NULL)
	{
	    send_to_char("There is no area containing that vnum.\n\r",ch);
	    return;
	}
    }

    else
    {
	for(parea = area_first; parea != NULL; parea = parea->next)
	{
	    if(!str_prefix(arg1, parea->name))
		break;
	}

	if (parea == NULL)
	{
	    send_to_char("There is no such area.\n\r",ch);
	    return;
	}
    }

    for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
	for( from_room = room_index_hash[iHash];
	     from_room != NULL;
	     from_room = from_room->next )
	{
	    if ( from_room->vnum < parea->min_vnum
	    ||   from_room->vnum > parea->max_vnum )
		continue;

	    for (door = 0; door < 6; door++)
	    {
		if( (pexit = from_room->exit[door]) != NULL )
		{
		    to_room = pexit->u1.to_room;

		    if( to_room != NULL
		    &&  (to_room->vnum < parea->min_vnum
		    ||   to_room->vnum > parea->max_vnum) )
		    {
			found = TRUE;
			sprintf(buf, "%s (%d) links %s to %s (%d)\n\r",
				    parea->name, from_room->vnum, dir_name[door],
				    to_room->area->name, to_room->vnum);
			send_to_char(buf, ch);
		    }
		}
	    }
	}
    }

    if(!found)
    {
	send_to_char("No links to other areas found.\n\r",ch);
	return;
    }

}

void do_fremove( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj_next;
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );  

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: fremove <char>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
          send_to_char( "That player is not here.\n\r", ch);
          return;
    }

    if ( victim->level > get_trust( ch) )
    {
        send_to_char( "Limited to your trust level.\n\r", ch );
        return;
    }         

    for (obj = victim->carrying; obj; obj = obj_next)
    {
         obj_next = obj->next_content;
           
         if (obj->wear_loc != WEAR_NONE)
         {
             remove_obj (victim, obj->wear_loc, TRUE);
             obj_from_char(obj);
             obj_to_char(obj, ch);
         }
    }
  
    printf_to_char( ch, "You confiscate all of %s's items!\n\r", victim->name );
    printf_to_char( victim, "%s has confiscated all of your items!\n\r", ch->name );
    save_char_obj(victim);
    return;
}                     
         

void do_security( CHAR_DATA *ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[100];
   CHAR_DATA *victim;
   int value;
   
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char("Syntax:\n\r",ch);
      send_to_char("Security <char name> <value>\n\r",ch);
      send_to_char("Security value 0 >> Player\n\r",ch);      
      send_to_char("Security value 5 >> Builder\n\r",ch);
      send_to_char("Security value 7 >> Head Builder\n\r",ch);
      send_to_char("Security value 9 >> Implementor\n\r",ch);
      return;
   }

   if( (victim = get_char_world(ch, arg1)) == NULL )
   {
      send_to_char("They aren't here.\n\r",ch);
      return;
   }

   victim->zone = NULL;
   value = is_number( arg2 ) ? atoi( arg2 ) : 0;

   if( IS_NPC(victim) )
   {
      send_to_char("NPC's lack security now!",ch);
      return;
   }

   if(victim->level > ch->level)       
   {
      send_to_char("At your level try again!\n\r",ch);
      return;
   }

   if( value > ch->pcdata->security && ch->level < IMPLEMENTOR )  
   {
      sprintf(buf, "Your level %d security isn't high enough\n\r" ,ch->pcdata->security);
      send_to_char( buf, ch );
      return;             
   }

   if ( value > 9 )
   {
       printf_to_char( ch, "You may not set someones security to %d.\n\r", value );
       return;
   }

   else
   {
      victim->pcdata->security = value;
      send_to_char("Your security rating has been changed.\n\r", victim);	
      sprintf(buf,"You changed %s's security rating to %d.\n\r",victim->name, value );
      send_to_char( buf, ch );
   }

   return;
    
}


void do_repeat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int count = 0;

    argument = one_argument(argument, buf);

    if (strlen(buf) < 1 || ( count = atoi(buf) ) < 1 || strlen(argument) < 1 )
    {
        send_to_char("Syntax:  repeat <count> <command string>\r\n", ch);
        return;
    }

    for ( ; count > 0 ; count-- )
        interpret(ch, argument);   

    return;
}


void do_repop( CHAR_DATA *ch, char *argument )
{
   AREA_DATA *pArea;
   char       buf[MAX_STRING_LENGTH];
   bool       found_area = FALSE;
   int        vnum;      

   if ( argument[0] == 0 )
   {
      reset_area(ch->in_room->area);
      send_to_char( "Area repop!\n\r", ch );
   }

   if ( is_number(argument) )
   {
      vnum = atoi(argument);

      for (pArea = area_first; pArea; pArea = pArea->next )
      {
         if (pArea->vnum == vnum)
         {
            sprintf(buf, "%s has been repoped!\n\r", &pArea->name[8]);
            send_to_char(buf, ch);
            found_area=TRUE;
            reset_area(pArea);
         }
      }        
      if (!found_area) send_to_char("No such area!\n\r",ch);
   }  

   if ( !strcmp(argument, "all") )
   {
      for (pArea = area_first; pArea; pArea = pArea->next )
      {
         reset_area(pArea);
      }
      send_to_char("World repop!\n\r", ch);
   }

   return;
}

  
void do_reward(CHAR_DATA *ch, char *argument)
/*
 *Written by Robbert for The Inquisition!
 *webmaster@theinquisition.net
 *modified by Synon 
 *synch23@titan.kyndig.com 
 */

{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char buf  [MSL];
   char arg1 [MSL];
   char arg2 [MSL];
   char arg3 [MSL];
   int value;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) || arg3[0] == '\0' )
   {
     send_to_char( "Syntax: award <char | all> <amount> <type>.\n\r", ch);
     send_to_char( "Valid types are: qp | exp\n\r", ch );
     return;
   }
 
   if ( !str_cmp( arg1, "all" ) )
   {

       value = atoi( arg2 );
       
       if ( value < -1000 || value > 1000 )
       {
           send_to_char( "Award range is -1000 to 1000.\n\r", ch );
           return;
       }
 
       if ( value == 0 )
       {
           send_to_char( "Value cannot be 0.\n\r", ch );
           return;
       }         
    
      if ( !str_cmp( arg3, "qp" ) )
      {

       for (d = descriptor_list; d != NULL; d = d->next)
       {
            victim = d->character;
            
            if ( victim == NULL || IS_NPC(victim) || victim == ch || victim->level >= LEVEL_IMMORTAL ) 
                continue;
             
            if ( value > 0 )
            {
                victim->pcdata->questpoints += value;
                printf_to_char( victim, "{BYou have been awarded {Y%d{x Quest Points by %s!\n\r", value, ch->name );
            }
            
            else
            {
                victim->pcdata->questpoints += value;
                sprintf( buf, "{RYou have been penalized {Y%d{x Quest Points by %s!\n\r", value, ch->name );
                send_to_char( buf, victim );
            }
       }        
       
       if ( value > 0 )
       {
       info( ch, 0, "{G[INFO]:{x {B%s has awarded everyone with %d quest points!{x\n\r", ch->name, value );   
       printf_to_char( ch, "You just awared everyone %d quest points!\n\r", value );
       }
       else
       {
       printf_to_char( ch, "You just penalized everyone for %d quest points!\n\r", value );
       info( ch, 0, "{G[INFO]:{x {R%s has penalized everyone for %d quest points!{x\n\r", ch->name, value ); 
       }
      
      return; 

      }   
  
      if ( !str_cmp( arg3, "exp" ) )
      {
       
       for (d = descriptor_list; d != NULL; d = d->next)
       {
            victim = d->character;
            
            if ( victim == NULL || IS_NPC(victim) || victim == ch || victim->level >= LEVEL_IMMORTAL ) 
                continue;
       
            gain_exp(victim, value);                        
      
            if ( value > 0 )
            {
                printf_to_char( victim, "{BYou have been awarded {Y%d{x Exp Points by %s!\n\r", value, ch->name );
            }
            
            else
            {
                sprintf( buf, "{RYou have been penalized {Y%d{x Exp Points by %s!\n\r", value, ch->name );
                send_to_char( buf, victim );
            }

       }        
       
       if ( value > 0 )
       {
       printf_to_char( ch, "You just awarded everyone with %d exp points!\n\r", value );
       info( ch, 0, "{G[INFO]:{x {B%s has awarded everyone with %d exp points!{x\n\r", ch->name, value );   
       }
       else
       {
       printf_to_char( ch, "You just penalized everyone for %d exp points!\n\r", value );
       info( ch, 0, "{G[INFO]:{x {R%s has penalized everyone for %d exp points!{x\n\r", ch->name, value ); 
       }
       
      }   
    
   return;

   }

   if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
   {
     send_to_char( "That player is not here.\n\r", ch);
     return;
   }

   if ( IS_NPC( victim ) )
   {
     send_to_char( "Not on NPC's!\n\r", ch);
     return;   
   }

   if ( ch == victim )
   {
     send_to_char( "You may not award yourself!", ch );
     return;
   }

   if (IS_IMMORTAL(victim) || victim->level >= LEVEL_IMMORTAL)
   {
     send_to_char( "You cannot award an immortal!\n\r", ch );
     return;
   }

   value = atoi( arg2 );

   if ( value < -5000 || value > 5000 )
   {
     send_to_char( "Award range is -5000 to 5000.\n\r", ch );
     return;
   }

   if ( value == 0 )
   {
     send_to_char( "Value cannot be 0.\n\r", ch );
     return;
   }

   if ( !str_cmp( arg3, "qp" ) )
   {

       sprintf( buf, "{wYou have awarded %s {Y%d{w Quest Points!\n\r", victim->name, value );
       send_to_char(buf, ch);
                             
       if ( value > 0 )
       {
           victim->pcdata->questpoints += value;
           sprintf( buf, "{wYou have been awarded {Y%d{w Quest Points by  %s!\n\r", value, ch->name );
           send_to_char( buf, victim );
           do_save(victim, "");
           sprintf( buf, "{R$N awards %d Quest Points to %s{x", value, victim->name);
           wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
           return;
       }
       else
       {
           victim->pcdata->questpoints += value;
           sprintf( buf, "{wYou have been penalized {Y%d{w Quest Points by %s!\n\r", value, ch->name );
           send_to_char( buf, victim );
           do_save(victim, "");
           sprintf( buf, "$N penalizes %s %d Quest Points.", victim->name, value );
           wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
           return;
       }
   }
    
   else if ( !str_cmp( arg3, "exp" ) )
   {
       printf_to_char( ch, "You have awarded %s {G%d{x Exp Points!\n\r", victim->name, value );
       gain_exp(victim, value);
       
       if ( value > 0 )
       {
           sprintf( buf,"You have been bonused %d experience points by %s!\n\r", value, ch->name );
           send_to_char( buf, victim );
           return;
       }
      
       else
       {
           sprintf( buf,"You have been penalized %d experience points by %s!\n\r", value, ch->name );
           send_to_char( buf, victim );
           return;
       }
   }

   send_to_char( "Syntax: award <char | all> <amount> <type>\n\r", ch);
   send_to_char( "Valid types are: exp | qp\n\r", ch );
   return;
}     


void do_check( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    int count = 1;
    
    one_argument( argument, arg );
    
    if (arg[0] == '\0'|| !str_prefix(arg,"stats"))
    {
        buffer = new_buf();
    	for (victim = char_list; victim != NULL; victim = victim->next)
    	{
    	    if (IS_NPC(victim) || !can_see(ch,victim)) 
    	    	continue;
    	    	
	    if (victim->desc == NULL)
	    {
	    	sprintf(buf,"%3d) %s is linkdead.\n\r", count, victim->name);
	    	add_buf(buffer, buf);
	    	count++;
	    	continue;	    	
	    }
	    
	    if (victim->desc->connected >= CON_GET_NEW_RACE
	     && victim->desc->connected <= CON_PICK_WEAPON)
	    {
	    	sprintf(buf,"%3d) %s is being created.\n\r",
 	    	    count, victim->name);
	    	add_buf(buffer, buf);
	    	count++;
	    	continue;
	    }
	    
	    if ( (victim->desc->connected == CON_GET_OLD_PASSWORD
	       || victim->desc->connected >= CON_READ_IMOTD)
	     && get_trust(victim) <= get_trust(ch) )
	    {
	    	sprintf(buf,"%3d) %s is connecting.\n\r",
 	    	    count, victim->name);
	    	add_buf(buffer, buf);
	    	count++;
	    	continue; 	    		 
	    }
	    
	    if (victim->desc->connected == CON_PLAYING)
	    {
	        if (get_trust(victim) > get_trust(ch))
	            sprintf(buf,"%3d) %s.\n\r", count, victim->name);
	        else
	        {
		    sprintf(buf,"%3d) %s, Level %d connected since %d hours (%d total hours)\n\r",
		    	count, victim->name,victim->level,
                        ((int)(current_time - victim->logon)) /3600, 
		    	(victim->played + (int)(current_time - victim->logon)) /3600 );
		    add_buf(buffer, buf);
		    if (arg[0]!='\0' && !str_prefix(arg,"stats"))
		    {
		      sprintf(buf,"  %ld HP %ld Mana (%d %d %d %d %d) %ld golds %d Tr %d Pr %d Qpts.\n\r",
		    	victim->max_hit, victim->max_mana,victim->perm_stat[STAT_STR],
		    	victim->perm_stat[STAT_INT],victim->perm_stat[STAT_WIS],
		    	victim->perm_stat[STAT_DEX],victim->perm_stat[STAT_CON],
		    	victim->gold + victim->silver/100,
		    	victim->train, victim->practice, victim->pcdata->questpoints);
		      add_buf(buffer, buf);
		    }
		    count++;
		}
	        continue;
	    }
	    
	    sprintf(buf,"%3d) bug (oops)...please report to Loran: %s %d\n\r",
	    	count, victim->name, victim->desc->connected);
	    add_buf(buffer, buf);
	    count++;   
    	}
    	page_to_char(buf_string(buffer),ch);
    	free_buf(buffer);
    	return;
    }
    
    if (!str_prefix(arg,"eq"))
    {
        buffer = new_buf();
    	for (victim = char_list; victim != NULL; victim = victim->next)
    	{
    	    if (IS_NPC(victim) 
    	     || victim->desc->connected != CON_PLAYING
    	     || !can_see(ch,victim)
    	     || get_trust(victim) > get_trust(ch) )
    	    	continue;
    	    	
    	    sprintf(buf,"%3d) %s, %d items (weight %d) Hit:%d Dam:%d Save:%d AC:%d %d %d %d.\n\r",
    	    	count, victim->name, victim->carry_number, victim->carry_weight, 
    	    	victim->hitroll, victim->damroll, victim->saving_throw,
    	    	victim->armor[AC_PIERCE], victim->armor[AC_BASH],
    	    	victim->armor[AC_SLASH], victim->armor[AC_EXOTIC]);
    	    add_buf(buffer, buf);
    	    count++;  
    	}
    	page_to_char(buf_string(buffer),ch);
    	free_buf(buffer);    	
    	return;
    }

    if (!str_prefix(arg,"snoop")) /* this part by jerome */
    {
        char bufsnoop [100];

        if(ch->level < MAX_LEVEL )
        {
            send_to_char("You can't use this check option.\n\r",ch);
            return;
        }
        buffer = new_buf();

        for (victim = char_list; victim != NULL; victim = victim->next)
        {
            if (IS_NPC(victim)
             || victim->desc->connected != CON_PLAYING
             || !can_see(ch,victim)
             || get_trust(victim) > get_trust(ch) )
                continue;

            if(victim->desc->snoop_by != NULL)
              sprintf(bufsnoop," %15s .",victim->desc->snoop_by->character->name);
            else
              sprintf(bufsnoop,"     (none)      ." );

            sprintf(buf,"%3d %15s : %s \n\r",count,victim->name, bufsnoop);
            add_buf(buffer, buf);
            count++;
        }
        page_to_char(buf_string(buffer),ch);
        free_buf(buffer);
        return;
    }

    send_to_char("Syntax: 'check'       display info about players\n\r",ch);
    send_to_char("        'check stats' display info and resume stats\n\r",ch);
    send_to_char("        'check eq'    resume eq of all players\n\r",ch);
    send_to_char("        'check snoop' displays a list of all the snoops\n\r",ch );
    send_to_char("Use the stat command in case of doubt about someone...\n\r",ch);
    return;
}

void do_ftick(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_STRING_LENGTH];

     one_argument(argument, arg);

     if ( arg[0] == '\0')
     {
        send_to_char("Syntax: ftick weather|char|obj|area|quest|mob|violence|aggr|olc|auction|all\n\r", ch );
        return;
     }

     if ( !str_prefix(arg, "area") )
        area_update     ( );

     if ( !str_prefix(arg, "quest") )
        quest_update    ( );

     if ( !str_prefix(arg, "mob") )
        mobile_update   ( );

     if ( !str_prefix(arg, "violence") )
        violence_update ( );

     if ( !str_prefix(arg, "aggr") )
        aggr_update     ( );

     if ( !str_prefix(arg, "auction") )
        auction_update  ( );

     if ( !str_prefix(arg, "weather") )
        weather_update  ( );

     if ( !str_prefix(arg, "all") )
     {        
        char_update     ( );
        obj_update      ( );
        area_update     ( );
        quest_update    ( );
        mobile_update   ( );
        violence_update ( );
        aggr_update     ( );
        auction_update  ( );
        weather_update  ( );
      }

      send_to_char("Tick!\n\r",ch);
      return;
} 
        
void do_openvlist( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    int loop = 1, x, lowvnum = 0;
    bool found = FALSE;
    bool havehighvnum = FALSE;
    AREA_DATA *pArea;

    buffer = new_buf();

    for (x = 1; x <= MAX_VNUM ; x++)
    {
        int count = 0;
        havehighvnum = FALSE;

        for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
        {
           if (x <= pArea->max_vnum && x >= pArea->min_vnum)
           {
               count++;                 /* count adds 1 if vnum in area    */
               havehighvnum = TRUE;     /* Might be the high vnum, trap it */
           }
        }

        if (x == MAX_VNUM && count == 0)  /* Lame hack to catch last vnum  */
            havehighvnum = TRUE;

        if (count == 0 && lowvnum == 0)   /* no areas found in and no low  */
            lowvnum = x;                  /* vnum yet, assign this one     */

        if (lowvnum > 0 && havehighvnum)  /* Have low and high vnum now    */
        {
            sprintf(buf, "[%d] Unassigned block of vnums from %d to %d\n\r"
                    ,loop, lowvnum, x - 1);
            add_buf(buffer,buf);
            lowvnum = 0;                  /* Reset low vnum                */
            loop++;
            found = TRUE;
        }
    }

    if (!found)
        send_to_char("No free range of vnums was found.\n\r",ch);
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

  return;

}

/* Used Vnum listings - Xander */

void do_rlist(CHAR_DATA *ch, char *argument)
{
	char	buf[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *room;
	int	lvnum, uvnum, i;
	

	sprintf(buf,"Rooms in area %s.\n\r",ch->in_room->area->name);
	send_to_char(buf,ch);

	lvnum = ch->in_room->area->min_vnum;
	uvnum = ch->in_room->area->max_vnum;

	if((uvnum - lvnum) > 100)
	{
	  send_to_char("Too big to loop through sorry.\n\r",ch);
	  send_to_char("Still workin on it - Xander :).\n\r",ch);
	}

	for(i = lvnum; i <= uvnum; i++)
	{
	  if((room = get_room_index(i)) == NULL) continue;
	  sprintf(buf,"[ %5d ] %s\n\r",i,room->name);
	  send_to_char(buf,ch);
	}
	return;
}

void do_olist(CHAR_DATA *ch, char *argument)
{
	char	buf[MAX_STRING_LENGTH];
	OBJ_INDEX_DATA *obj;
	int	lvnum, uvnum, i;
	

	sprintf(buf,"Objects in area %s.\n\n\r",ch->in_room->area->name);
	send_to_char(buf,ch);

	lvnum = ch->in_room->area->min_vnum;
	uvnum = ch->in_room->area->max_vnum;

	if((uvnum - lvnum) > 500)
	{
	  send_to_char("Too big to loop through sorry.\n\r",ch);
	  send_to_char("Still workin on it - Xander :).\n\r",ch);
	}

	for(i = lvnum; i <= uvnum; i++)
	{
	  if((obj = get_obj_index(i)) == NULL) continue;
	  sprintf(buf,"[ %5d ] %s\n\r",i,obj->name);
	  send_to_char(buf,ch);
	}
	return;
}

void do_mlist(CHAR_DATA *ch, char *argument)
{
	char	buf[MAX_STRING_LENGTH];
	MOB_INDEX_DATA *mob;
	int	lvnum, uvnum, i;
	
	sprintf(buf,"Mobs in area %s.\n\n\r",ch->in_room->area->name);
	send_to_char(buf,ch);

	lvnum = ch->in_room->area->min_vnum;
	uvnum = ch->in_room->area->max_vnum;

	if((uvnum - lvnum) > 100)
	{
	  send_to_char("Too big to loop through sorry.\n\r",ch);
	  send_to_char("Still workin on it - Xander :).\n\r",ch);
	}

	for(i = lvnum; i <= uvnum; i++)
	{
	  if((mob = get_mob_index(i)) == NULL) continue;
	  sprintf(buf,"[ %5d ] %s\n\r",i,mob->short_descr);
	  send_to_char(buf,ch);
	}
	return;
}

/* End of used vnum listings - Xander */

void do_updatechar(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    FILE *fp;
    char arg[MSL], buf[MSL];
    AFFECT_DATA *paf, *paf_next;
    NICK_DATA *pNick, *pNick_next;
     
    arg[0] = '\0';
    argument = one_argument(argument, arg);

    if(arg[0] == '\0')
    {
	send_to_char("Whom would you like to update?\n\r",ch);
	return;
    }
    if( ( victim = get_char_world(ch, arg) ) == NULL )
    {
	send_to_char("They aren't around!\n\r",ch);
	return;
    }
    paf = ch->affected;
    ch->affected = NULL;
    
    for ( ; paf != NULL; paf = paf_next )
    {	
	paf_next = paf->next;
        free_affect(paf);
    }
    pNick = ch->pcdata->nick_first;
    ch->pcdata->nick_first = NULL;

    for ( ; pNick ; pNick = pNick_next )
    {
	pNick_next = pNick->next;
        free_nick(pNick);
    }
    sprintf(buf, "%s%s", PLAYER_DIR, victim->name);
    if (file_exists(buf))
    {
	fp = file_open(buf, "r");
	fread_char(victim, fp);
	printf_to_char(ch, "%s has been updated.\n\r", victim->name);
	file_close(fp);
	return;
    }
    printf_to_char(ch, "%s's pfile does not exist. Advise a save!\n\r", victim->name);
    return;
}


void do_otype(CHAR_DATA *ch, char *argument)
{
    int type;
    int type2;
    int vnum=1;
    char buf[MAX_STRING_LENGTH];
    char buffer[12 * MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *item;      
    OBJ_INDEX_DATA *obj;
    bool found;

    item = one_argument(argument, arg1);
    one_argument ( item , arg2);

    found = FALSE;
    buffer [0] = '\0';

    if (arg1[0] == '\0')     
    {
        send_to_char( "Syntax: otype <type> <wear|weapon>\n\r", ch );
        send_to_char( "For example: otype light, otype weapon axe, otype armor hands\n\r", ch );
        return;
    }

    type2 = 0;

    if ((!str_cmp(arg1,"armor") || !str_cmp(arg1,"weapon"))
        && arg2[0] == '\0' )
    {        
        send_to_char("Type 'Help Otype' for proper usage.\n\r",ch);
        return;
    }

    else if (!str_cmp(arg1,"armor"))
    {
        type = flag_value(type_flags,arg1);

        if ((type2 = flag_value(wear_flags,arg2)) == NO_FLAG)
        {
             send_to_char("No such armor type.\n\r",ch);
             return;      
        }
    }

    else if (!str_cmp(arg1,"weapon"))
    {
        type = flag_value(type_flags,arg1);

        if ((type2 = flag_value(weapon_class,arg2)) == NO_FLAG)
        {
             send_to_char("No such weapon type.\n\r",ch);
             return;
        }            
    }

    else
    {
        if((type = flag_value(type_flags,arg1)) == NO_FLAG)
        {
            send_to_char("Unknown Type.\n\r", ch);
            return;
        }
    }        

    for(;vnum <= top_vnum_obj; vnum++)
    {
        if((obj=get_obj_index(vnum)) != NULL)
        {
            if((obj->item_type == type && type2 == 0
                && str_cmp(arg1,"weapon") && str_cmp(arg1,"armor"))
            || (obj->item_type == type && obj->value[0] == type2
                && str_cmp(arg1,"armor"))
            || (obj->item_type == type && IS_SET(obj->wear_flags,type2)
                && str_cmp(arg1,"weapon")))    
            {
                sprintf(buf, "%35s - %5d - Area [%d]\n\r", obj->short_descr, vnum, obj->area->vnum);   
                found = TRUE;
                strcat(buffer,buf);
            }
        }
    }

    if (!found)
        send_to_char("No objects of that type exist\n\r",ch);

    else            
        if (ch->lines)
            page_to_char(buffer,ch);
        else
            send_to_char(buffer,ch);
}   


void do_grantbp( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MSL];
    char arg2[MSL];
    char buf[MSL];
    int award;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
 

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("Syntax: grantbp <character> <amount>\n\r", ch);
	return;
    }

    if ( (victim = get_char_world( ch , arg1)) == NULL)  
    {
	 send_to_char("They must be playing!\n\r",ch);
	 return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("NPC's can't gain blessing points!\n\r", ch);
	return;
    }

    award = atoi(arg2);
 
    if (award == 0)
    {
	send_to_char("You must choose an amount greater than 0.\n\r", ch);
	return;
    }
	
    if ((award > 10) || (award < -10) )
    {
	send_to_char("You must choose an amount between 10 and -10.\n\r", ch);
	return;
    }

    victim->bp_points += award;	
 
    if (award > 0)
    {
        sprintf(buf, "You award %d blessing points to %s!\n\r", award, victim->name);
        send_to_char(buf, ch);
        sprintf(buf, "%s has awarded you %d blessing points!", ch->name, award);
        send_to_char(buf, victim);
        return;
    }
   
    else
    {
	 sprintf(buf, "You remove %d blessing points from %s!\n\r", award, victim->name);
	 send_to_char(buf, ch);

	 sprintf(buf, "%s has taken %d blessing points away from you!", ch->name, award);
	 send_to_char(buf, victim);
         return;
    }
 
}


void do_xpreset( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: xpreset <name>\n\r", ch );
        return;
    }

    if ( !(victim = get_char_world( ch, arg )))  
    {
        send_to_char("They are not currently on.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ))
    {
        send_to_char("Not on a NPC.\n\r", ch );
        return;
    }

    ch->exp = exp_per_level(ch,ch->pcdata->points) * ch->level;
    send_to_char( "Their exp to level has been reset.\n\r", ch );
    send_to_char( "Your exp to level has been reset.\n\r", victim );
    save_char_obj( ch );
    return;
}                                

void do_silence( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Silence whom?", ch );
        return;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
 
if ((IS_SET(victim->comm, COMM_NOCHANNELS)) && (IS_SET(victim->comm, COMM_NOTELL)) && (IS_SET(victim->comm, COMM_NOEMOTE)) && (IS_SET(victim->comm, COMM_NOSHOUT)))
	{
		REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
		REMOVE_BIT(victim->comm, COMM_NOTELL);
		REMOVE_BIT(victim->comm, COMM_NOEMOTE);
		REMOVE_BIT(victim->comm, COMM_NOSHOUT);
		send_to_char( "The Immortals remove your restriction to says.\n\r", victim );
		send_to_char( "SILENCE removed.\n\r", ch );
		sprintf(buf,"$N decides %s has been silenced long enough.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}
	else
	{
		SET_BIT(victim->comm, COMM_NOCHANNELS);
		SET_BIT(victim->comm, COMM_NOTELL);
		SET_BIT(victim->comm, COMM_NOEMOTE);
		SET_BIT(victim->comm, COMM_NOSHOUT);
		send_to_char( "The Immortals restrict you to says.\n\r", victim );
		send_to_char( "SILENCE set.\n\r", ch );
		sprintf(buf,"$N silences %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}


    return;
}

void do_munique( CHAR_DATA *ch, char *argument )
{
   char arg[MSL];
   int amount = 0;

   argument = one_argument( argument, arg );

   if ( arg[0] == '\0' || !is_number( arg ) )
   {
       send_to_char( "Syntax: munique <amount>\n\r", ch );
       send_to_char( "Note the default for this is 25 per boot.\n\rOnly change it if you want a different amount.\n\r", ch );
       return;
   }

   amount = atoi( arg );
   
   if ( amount < 1 || amount > 50 )
   {
       send_to_char("Please choose an amount between 1 and 50.\n\r", ch );
       return;
   }

   if ( unique_set )
   {
       send_to_char( "The max amount of unique objects has been set for this reboot!\n\rTry again after the next one!\n\r", ch );
       return;
   }

   unique_set = TRUE;
   max_unique = amount;
   printf_to_char( ch, "The amount of unique objects that can load mud wide is now %d!\n\r", amount );
   return;
}

/*
 * Seize command by Chilalin. Allows you to forcibly take an
 * item from a player, bypassing all checks such as noremove
 * nodrop.
 */
void do_seize(CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    bool found;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("Seize what from whom?\n\r", ch);
        return;
    }

    if (!(victim = get_char_world(ch, arg2)))
    {
        send_to_char("They aren't in the mud.\n\r", ch);
        return;
    }

    if (get_trust(victim) >= get_trust(ch))
    {
        send_to_char("You cannot.\n\r", ch);
        return;
    }

    found = FALSE;
    for (;;)
    {
        if (!(obj = get_obj_carry(victim, arg1, ch)) &&
            !(obj = get_obj_wear(victim, arg1, TRUE)))
        {
            break;
        }
        found = TRUE;
        obj_from_char(obj);
        obj_to_char(obj, ch);
        save_char_obj(victim);
        act("You seize $p from $N.", ch, obj, victim, TO_CHAR);
    }

    if (!found)
    {
        send_to_char("You can't find it.\n\r", ch);
    }
    return;
}


void do_prand( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *pRoomIndex;

    one_argument( argument, arg );    

    if ( arg[0] == '\0' )
    {
        send_to_char("Syntax: prand <name>\n\r",ch);
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }     

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }  
   
    if ( victim->level >= ch->level )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    pRoomIndex = get_random_room(victim);       

    printf_to_char( victim, "%s uses their divine power to send you to a far away location!\n\r", ch->name );
    printf_to_char( ch, "Using your divine power you send %s to a far away place!\n\r", victim->name );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n materalizes before your very eyes!", victim, NULL, NULL, TO_ROOM );     
    do_look( ch, "auto" );

    return;
}

void do_gpoint( CHAR_DATA *ch, char *argument )
{
   int amount;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mobs don't need goto points!\n\r", ch );
        return;
    }

    if (  argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Your goto point has been cleared!\n\r", ch );
        ch->gpoint = 0;
        return;
    }

    amount = atoi( argument );

    if ( amount <= 0 )
    {
        send_to_char( "Please set your goto point to a number greater than 0.\n\r", ch );
        return;
    }

    ch->gpoint = amount; 
    printf_to_char( ch, "Your goto point is set to %d\n\r", ch->gpoint );
    
    return;
}
    
void do_timeout( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;
    char arg[MSL];
    char arg1[MSL];
    char buf[MSL];
    int amount;
   
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );
  
    location = get_room_index( ROOM_VNUM_TIMEOUT );

    if ( arg[0] == '\0' || arg1[0] == '\0' || !is_number( arg1 ) )
    {
	send_to_char( "Syntax: timeout <name> <amount>\n\r", ch );
	return;
    }
    amount = atoi(arg1);
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( amount <= 0 || amount < 100 )
    {
        send_to_char( "Please choose an amount larger than 0 or less than 100 for a timeout!\n\r", ch );
        return;
    }

    amount = atoi( arg1 );

    victim->pcdata->timeout = amount;
    sprintf(buf,"$N has given %s a timeout!",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    printf_to_char( ch, "Ok %s has been given a timeout for %d ticks!\n\r", victim->name, amount );
    info( ch, 0, "{G[INFO]:{x {C%s {chas been given a timeout for not following the rules!{x\n\r", victim->name );
    SET_BIT( victim->act, PLR_TIMEOUT );
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
    char_from_room( victim );
    char_to_room( victim, location );
    return;
}

void do_auto_shutdown()
{
   FILE *fp, *fpObj, *fpCrash, *fpBoot;
   DESCRIPTOR_DATA *d, *d_next;
   CHAR_DATA *gch;
   char buf [100], buf2[100];
	OBJ_DATA * obj;
	bool has_reset=FALSE;
	RESET_DATA *pReset;

   while (auction_list != NULL)
          reset_auc (auction_list, TRUE);

        for ( d = descriptor_list; d; d = d->next )
        {
             if ( d->character!= NULL )
                 gch = d->character;
             else 
                 continue;
#ifdef     MCCP_ENABLED
           if ( gch->desc->out_compress )
           {
               if ( !compressEnd( gch->desc ) )
                   send_to_char( "Could not disable compression you'll have to reconnect in five seconds!\n\r", gch );
           }
#endif
        }
 
   save_gquest_data();    
	fpObj = file_open("../area/objcopy.txt", "w" );

    for ( obj = object_list; obj != NULL; obj = obj->next )
	{   has_reset = FALSE;
		if(obj->carried_by )
			continue;
		if(obj->item_type != ITEM_CORPSE_PC )
			continue;
		for ( pReset = obj->in_room->reset_first; pReset; pReset = pReset->next )
		{   if(( (pReset->command == 'O' || pReset->command == 'P') && pReset->arg3 == obj->pIndexData->vnum ) ||
				( pReset->command == 'E' && pReset->arg1 == obj->pIndexData->vnum ) )
			{	has_reset = TRUE;
				break;
			}
		}
		if(has_reset)
			continue;
		fwrite_obj(NULL, obj, fpObj, 0 );
	}
	fprintf(fpObj, "#END\n");
	file_close(fpObj);
   fpCrash = file_open("../data/crash.time", "w" );
   fprintf(fpCrash, "%d", (int) current_time );
   file_close(fpCrash);
   if( (fpBoot = file_open(BOOT_FILE, "w" ) ) )
       fprintf(fpBoot, "%d\n\r", first_boot_time );
   fclose(fpBoot);


   fp = fopen (COPYOVER_FILE, "w");
   if (!fp)
   {
       for (d = descriptor_list;d != NULL;d = d_next)
       {
           if(d->character)
           {
              do_function( d->character, &do_save, "" );
              send_to_char("Ok I tried but we're crashing anyway sorry!\n\r",d->character);
           }

           d_next=d->next;
           close_socket(d);
       }
         
       exit(1);
   }
	
   do_function( NULL, &do_asave, "changed" );

   sprintf(buf,"\n\rShadowStorm is Crashing Attempting a copyover now!\n\r");
   
   for (d = descriptor_list; d ; d = d_next)
   {
	CHAR_DATA * och = CH (d);
	d_next = d->next; /* We delete from the list , so need to save this */
		
	if (!d->character || d->connected > CON_PLAYING) 
	{
#ifdef MCCP_ENABLED
	    write_to_descriptor_2(d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
#else
	    write_to_descriptor(d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
	    close_socket (d); /* throw'em out */
#endif
	}
		
        else
	{
	    fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
	    save_char_obj (och);
#ifdef MCCP_ENABLED
			write_to_descriptor_2 (d->descriptor, buf, 0);
#else
			write_to_descriptor (d->descriptor, buf, 0);
#endif
	}
   }
	
   fprintf (fp, "-1\n");
   fclose(fp);
   fclose (fpReserve);
   sprintf (buf, "%d", port);
   sprintf (buf2, "%d", control);
   execl (EXE_FILE, "shadow", buf, "copyover", buf2, (char *) NULL);   
   exit(1);
}


void do_ibeep ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    argument = one_argument( argument, arg );

    if  ( arg[0] == '\0' )
    {
        send_to_char( "Beep who?\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
        send_to_char( "They are not here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim))
    {
        send_to_char( "They are not beepable.\n\r", ch );
        return;
    }

    sprintf( buf, "\aYou beep %s.\n\r", victim->name );
    send_to_char( buf, ch );

sprintf( buf, "{$BBBBBB    {^EEEEEEE {#EEEEEEE  {MPPPPP{0\n\r");
    send_to_char( buf, victim );
sprintf( buf, "{$BB   BB   {^EE      {#EE       {MPP  PP{0\n\r");
    send_to_char( buf, victim );
sprintf( buf, "{$BB    BB  {^EE      {#EE       {MPP   PP{0\n\r");
    send_to_char( buf, victim );
sprintf( buf, "{$BB    BB  {^EE      {#EE       {MPP   PP{0\n\r");
    send_to_char( buf, victim );
sprintf( buf, "{$BB   BB   {^EE      {#EE       {MPP  PP{0\n\r");
    send_to_char( buf, victim );
sprintf( buf, "{$BBBBBB    {^EEEEE   {#EEEEE    {MPPPPP{0\n\r");
    send_to_char( buf, victim );
sprintf( buf, "{$BB   BB   {^EE      {#EE       {MPP{0\n\r");
    send_to_char( buf, victim );
sprintf( buf, "{$BB    BB  {^EE      {#EE       {MPP{0\n\r");
    send_to_char( buf, victim );
sprintf( buf, "{$BB    BB  {^EE      {#EE       {MPP{0\n\r");
    send_to_char( buf, victim );
sprintf( buf, "{$BB   BB   {^EE      {#EE       {MPP{0\n\r");
    send_to_char( buf, victim );
sprintf( buf, "\a{$BBBBBB    {^EEEEEEE {#EEEEEEE  {MPP{0\n\r");
    send_to_char( buf, victim );
    sprintf( buf, "from %s.\n\r", ch->name );
    send_to_char( buf, victim );
    return;
}


void do_wset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL];
    argument = one_argument( argument, arg1 );
    

    if ( arg1[0] == '\0' )
    {
        send_to_char("Syntax:\n\r",ch);
        send_to_char("  wset <condition> \n\r", ch);
        send_to_char("  Condition can be :\n\r",ch);
        send_to_char("  (H)ail (F)og (I)cestorm (B)lizzard (S)nowing (R)ain\n\r",ch);
        send_to_char("  (L)ightning (T)hunderstorm (C)loudless cl(O)udy br(E)eze (W)ind\n\r",ch);
        return;
    }

    switch ( LOWER(arg1[0]) )
    {
        default:
        send_to_char("Syntax:\n\r",ch);
        send_to_char("  wset <condition> \n\r", ch);
        send_to_char("  Condition can be :\n\r",ch);
        send_to_char("  (H)ail (F)og (I)cestorm (B)lizzard (S)nowing (R)ain\n\r",ch);
        send_to_char("  (L)ightning (T)hunderstorm (C)loudless cl(O)udy br(E)eze (W)ind\n\r",ch);
        break;

        case 'c':
            send_to_char( "You wave your hands and in reverence to you, the clouds dissapear.\n\r", ch );
            weather_info.sky = SKY_CLOUDLESS;
            act( "$n makes a strange movement with their hands and the clouds part.", ch, NULL, NULL, TO_ROOM);
            break;

        case 'f':
            send_to_char( "You wave your hands and in reverence to you, a mist vapors the horizon.\n\r", ch );            
            weather_info.sky = SKY_FOGGY;
            act( "$n makes a strange movement with their hands and a mist vapors the horizon.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_FOGGY;
            break;

        case 'h':
            send_to_char( "You wave your hands and in reverence to you, hailstones fall from the sky \n\r", ch);
            act( "$n makes a strange movement with their hands and hailstones fall from the sky.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_HAILSTORM;
            break;        

        case 'i':
            send_to_char( "You wave your hands and in reverence to you, it starts raining ice\n\r", ch);
            act( "$n makes a strange movement with their hands and it starts raining ice.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_ICESTORM;
            break;

        case 'b':
            send_to_char( "You wave your hands and in reverence to you, snowflakes cover the horizon making it impossible to see.\n\r", ch);
            weather_info.sky = SKY_BLIZZARD;
            act( "$n makes a strange movement with their hands and snowflakes cover the horizon making it impossibile to see", ch, NULL, NULL, TO_ROOM);
            break;
          
        case 's':
            send_to_char( "You wave your hands and in reverence to you, snowflakes fall from the sky..\n\r", ch);
            weather_info.sky = SKY_SNOWING;
            act( "$n makes a strange movement with their hands and snowflakes fall from the sky", ch, NULL, NULL, TO_ROOM);
            break;
        
        case 'r':
            send_to_char( "You wave your hands and in reverence to you, a warm rain starts to fall.\n\r", ch);
            weather_info.sky = SKY_RAINING;
            act( "$n makes a strange movement with their hands and a warm rain starts to fall", ch, NULL, NULL, TO_ROOM);
            break;

        case 'l':
            send_to_char( "You wave your hands and in reverence to you, lightning pierces the sky. \n\r", ch);
            weather_info.sky = SKY_LIGHTNING;
            act( "$n makes a strange movement with their hands and lightning pierces the sky", ch, NULL, NULL, TO_ROOM);
            break;
       
        case 't':
            send_to_char( "You wave your hands and in reverence to you, The clouds clap in thunder.\n\r", ch);
            weather_info.sky = SKY_THUNDERSTORM;
            act( "$n makes a strange movement with their hands and the clouds clap in thunder", ch, NULL, NULL, TO_ROOM);
            break;

        case 'o':
            send_to_char( "You wave your hands and in reverence to you, clouds cover the horizon, threatening rain.\n\r", ch);
            weather_info.sky = SKY_CLOUDY;
            act( "$n makes a strange movement with their hands and clouds cover the horizon, threatening rain", ch, NULL, NULL, TO_ROOM);
            break;

        case 'e':
            send_to_char( "You wave your hands and a cool breeze begins to sweep across the lands!\n\r", ch );
            weather_info.sky = SKY_BREEZE;
            act( "$n makes a strange movement, and a cool breeze begins to blow across the land.", ch, NULL, NULL, TO_ROOM );
            break;
       
        case 'w':
            send_to_char( "You wave your hands and a strong wind begins to sweep across the land.\n\r", ch );
            weather_info.sky = SKY_WIND;
            act( "$n makes a strange movement, and a strong wind begins to sweep across the land.", ch, NULL, NULL, TO_ROOM );
            break;
    }
}

void do_isignal( CHAR_DATA *ch, char *argument )
{
   char arg[MSL];
 
   argument = one_argument( argument, arg );

   if ( arg[0] == '\0' )
   {
       send_to_char( "Syntax: isignal <ON|OFF>\n\r", ch );
       return;
   }

   if ( !str_cmp( arg, "on" ) )
   {
       if ( isignal )
       {
           send_to_char( "The signal handler is already on!\n\r", ch );
           return;
       }

       isignal = TRUE;
       send_to_char( "You have turned the signal handler on.\n\r", ch );
       return;
   }

   if ( !str_cmp( arg, "off" ) )
   {
       if ( !isignal )
       {
           send_to_char( "The signal handler is already off!\n\r", ch );
           return;
       }

       isignal = FALSE;
       send_to_char( "You have turned the signal handler off.\n\r", ch );
       return;
   }

   return;
}

    
void do_fquit( CHAR_DATA *ch, char *argument )
{

    if ( IS_NPC( ch ) ) 
        return;

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "No way! You are fighting.\n\r", ch );
        return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
        send_to_char( "You're not DEAD yet.\n\r", ch );
        return;
    }        

    if ( ch->fquit == 1 )
    {
        send_to_char( "You already used fquit once, to remove it type invis.\n\r", ch );
        return;
    }

    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
    wiznet("$N rejoins the real world.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));
    info( ch, 0, "{G[INFO]:{x {R%s has rejoined the real world!{x\n\r", ch->name );
    ch->invis_level = ch->level;
    ch->fquit = 1;
    send_to_char( "You have successfully tricked those of a lower level into thinking you logged off.\n\r", ch );
    return;
}

void do_constval(CHAR_DATA *ch, char *argument )
{
	char arg[MSL], arg2[MSL];
	int iConst, value, col = 0;

	arg[0] = '\0';
	arg2[0] = arg[0];

	argument = one_argument(argument,arg);
	argument = one_argument(argument,arg2);
	
	if(arg[0] == '\0' )
	{	send_to_char("Syntax: constval list\n\r"
					 "        constval <constant> <new value>\n\r",ch);
		return;
	}
	
	if(!str_cmp(arg, "list" ) )
	{
           	printf_to_char(ch, "{r[{R%-30s{r:{R %-5s{r] {R| {r[{R%-30s{r:{R %-5s{r]{x\n\r", "Constant Name", "Value", "Constant Name", "Value" );
	
          	for(iConst = 0; const_table[iConst].name != NULL ; iConst++ )
		{	printf_to_char(ch, "{r[{D%-30s{r:{D %-5d{r]{x", const_table[iConst].name, *const_table[iConst].value );
			if( ++col % 2 == 0 )
				send_to_char("\n\r",ch);
			else
				send_to_char(" {R| ",ch);
		}	
		return;
	}
	if(arg2[0] == '\0' )
	{	do_function(ch, &do_constval, "" );
		return;
	}
	if( ( iConst = const_lookup(arg) ) == -1 )
	{	send_to_char("No such constant. Use the list argument to see valid constants.\n\r",ch);
		return;
	}

	if(!is_number(arg2 ))
	{	send_to_char("You must give it a value... as in number!\n\r",ch);
		return;
	}
	value = atoi(arg2);

	printf_to_char(ch, "%s's value changed to %d.\n\r",const_table[iConst].name, value );
	*const_table[iConst].value = value;
	do_function(ch, &do_saveconst, "");
	return;
}

void do_checkvnum(CHAR_DATA *ch, char *argument )
{
    int i;

    for (i = 0; vnum_table[i].vnum != NULL; i++)
    {
	if(vnum_table[i].type == VNUM_MOB )
	{    if( get_mob_index(*vnum_table[i].vnum ) == NULL )
		printf_to_char(ch, "Mob vnum not existant: %d\n\r",*vnum_table[i].vnum); 	}
	if(vnum_table[i].type == VNUM_OBJ )
	{   if(get_obj_index(*vnum_table[i].vnum) == NULL )
		printf_to_char(ch, "Obj vnum not existant: %d\n\r", *vnum_table[i].vnum );
	}
	if(vnum_table[i].type == VNUM_ROOM )
	{    if(get_room_index(*vnum_table[i].vnum ) == NULL )
		printf_to_char(ch, "Room vnum not existant: %d\n\r",*vnum_table[i].vnum );
	}
    }
    return;
}

void do_mrelic (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    int i = 1500;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Make a relic item of what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, argument, ch ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }                                  

    if (IS_OBJ2_STAT(obj,ITEM_RELIC))
    {
        REMOVE_BIT(obj->extra2_flags,ITEM_RELIC);
        act("$p is no longer a relic item.",ch,obj,NULL,TO_CHAR);
    }

    else
    {
        SET_BIT(obj->extra2_flags,ITEM_RELIC);
        if ( obj->xp_tolevel <= 0 )
        obj->xp_tolevel = i;
        act("$p is now a relic item.",ch,obj,NULL,TO_CHAR);
    }

    return;
}           

void do_pack ( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *pack;
    OBJ_DATA *obj;
    int i;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Who would you like to send a pack too.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( (victim->level >= 10) )
    {
	send_to_char("They don't need one at thier level.\n\r", ch);
	return;
    }

    if (!can_pack(victim) )
    {
	send_to_char("They already have a survival pack.\n\r",ch);
	return;
    }

    pack = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SATCHEL), 0 );

    pack->size = victim->size;

    obj = create_object( get_obj_index(OBJ_VNUM_BEGINING), 0 );
    obj_to_obj( obj, pack );

    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIRT), 0 );
    obj_to_obj( obj, pack );
    
    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_PANTS), 0 );
    obj_to_obj( obj, pack );
       
    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SANDLES), 0 );
    obj_to_obj( obj, pack );

    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_WATER_SKIN), 0 );
    obj_to_obj( obj, pack );

    obj = create_object( get_obj_index(OBJ_VNUM_GLOVES), 0 );
    obj_to_obj( obj, pack );
 
    obj = create_object( get_obj_index(OBJ_VNUM_HELMET), 0 );
    obj_to_obj( obj, pack );

    for ( i = 0; i < 2; i++ )
    {
         obj = create_object( get_obj_index(OBJ_VNUM_RING), 0 );
         obj_to_obj( obj, pack );
    }

    for ( i = 0; i < 2; i++ )
    {
         obj = create_object( get_obj_index(OBJ_VNUM_BRACELET), 0 );
         obj_to_obj( obj, pack );
    }

    for ( i = 0; i < 2; i++ )
    {
         obj = create_object( get_obj_index(OBJ_VNUM_NECKLACE), 0 );
         obj_to_obj( obj, pack );
    }

    for ( i = 0; i < 2; i++ )
    {
         obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SWORD), 0 );
         obj_to_obj( obj, pack );
    }

    for ( i = 0; i < 2; i++ )
    {
         obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_DAGGER), 0 );
         obj_to_obj( obj, pack );
    }

    for ( i = 0; i < 2; i++ )
    {
         obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SPEAR), 0 );
         obj_to_obj( obj, pack );
    }
    
    for ( i = 0; i < 2; i++ )
    {
         obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_MACE), 0 );
         obj_to_obj( obj, pack );
    }

    for ( i = 0; i < 2; i++ )
    {
         obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_AXE), 0 );
         obj_to_obj( obj, pack );
    }

    for ( i = 0; i < 2; i++ )
    {
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_FLAIL), 0 );
        obj_to_obj( obj, pack );
    }

    for ( i = 0; i < 2; i++ )
    {
         obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_WHIP), 0 );
         obj_to_obj( obj, pack );
    }

    for ( i = 0; i < 2; i++ )
    {
         obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_POLEARM), 0 );
         obj_to_obj( obj, pack );
    }

    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
    obj_to_obj( obj, pack );

    for ( i = 0; i < 10; i ++ )
    {
         obj = create_object( get_obj_index(OBJ_VNUM_BREAD), 0 );
         obj_to_obj( obj, pack );
    }

    obj_to_char( pack, victim );
    send_to_char("Ok.\n\r", ch);
    printf_to_char( ch, "%s appears in your inventory.\n\r", pack->short_descr );
    return;
}

void do_frecall( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "{RForce Recall whom?{x\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "{RThey aren't here.{x\n\r", ch );
		return;
	}

	if ( IS_NPC(victim) )
	{
		send_to_char( "{RNot on NPC's.{x\n\r", ch );
		return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "{RYou failed.{x\n\r", ch );
		return;
	}

        if ( victim->gold < 250 || victim->pcdata->gold_bank < 250 )
        {
            printf_to_char( ch, "%s does not have enough gold.\n\r", victim->name );
            return;
        }

	sprintf(buf,"$N force recalled %s",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	send_to_char( "Force Recall Done.\n\r", ch );

	stop_fighting(victim,TRUE);
	sprintf( buf, "%s %d", victim->name, ROOM_VNUM_TEMPLE );

	do_transfer( ch, buf);

	return;
}

void do_testfunc (CHAR_DATA *ch, char *argument)
{
   TRIVIA_SCORE *score;

   if (!str_cmp (argument, "poop"))
   {
      score = new_score();

      strcpy (score->name, "Larry");
      score->points = 5;
      score->totalpoints = 20;

      score = new_score();

      strcpy (score->name, "Jed");
      score->points = 12;
      score->totalpoints = 25;

      score = new_score();

      strcpy (score->name, "Gary");
      score->points = 8;
      score->totalpoints = 2;

      score = new_score();

      strcpy (score->name, "Spedro");
      score->points = 19;
      score->totalpoints = 108;
   
      return;
    }

    else if (!str_cmp (argument, "sped"))
       do_trivia_scores (ch, "Spedro");

    else
      do_trivia_scores (ch, "");

    return;
}

void do_unlink( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *linkdead;
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    bool found = FALSE;
    int i = 0;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Unlink <all> or Unlink <player>\n\r", ch );
     
        for ( wch = char_list; wch != NULL; wch = wch_next )
        {
             wch_next = wch->next;
            
             if (IS_NPC( wch ) )
                 continue;        

             if ( wch->desc == NULL )
             {
                 printf_to_char( ch, "\n\rLinkdead characters: %s\n\r", wch->name );
                 i++;
             }     
        }

        send_to_char( "\n\r{C================================================{x\n\r\n\r", ch );
        printf_to_char( ch, "The number of linkdead people on the mud is %d.\n\r", i );
        return;    
    }

    if  ( !str_cmp( argument, "all" ) )
    {
        for ( wch = char_list;  wch != NULL; wch = wch_next )
        {
              wch_next = wch->next;

              if ( IS_NPC( wch ) )
                  continue;

              if ( wch->desc == NULL )
              {
                  found = TRUE;
                  do_function( wch, &do_quit, "" );
              }
        } 

        if ( found )
            send_to_char( "Extracting all linkdead players.\n\r", ch );

        if ( !found )
        send_to_char( "No linkdead players were found!\n\r", ch );
        return;
    }

    if ((linkdead = get_char_world(ch,argument)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }
    
    if ( linkdead->desc != NULL )
    {
        send_to_char( "They aren't linkdead just use disconnect instead.\n\r", ch );
        return;
    }

    else
    {
        do_function( linkdead, &do_quit, "" );
        send_to_char("They have been booted.\n\r", ch);
        return;
    }

}

void do_plist( CHAR_DATA *ch, char *argument )
{
    DIR *dp;
    struct dirent *ep;
    char buf[80];
    char buffer[MAX_STRING_LENGTH*4];
    int days;
    bool fAll = TRUE, fImmortal = FALSE;

    buffer[0] = '\0';

    if ( argument[0] == '\0' || !str_cmp( argument, "all" ) )
    {
	fAll = TRUE;
    }

    else
    if ( !str_prefix( argument, "immortal" ) )
    {
	fImmortal = TRUE;
    }
    else
    {
	send_to_char( "Syntax: PLIST [ALL/IMMORTAL]\n\r", ch );
	return;
    }

    if ( fImmortal )
    {
	/*
	 * change the string literals to GODS_DIR or where
	 * you keep your gods folder
	 */
	dp = opendir ("../gods");
    }
    else
    {
	/*
	 * change the string literals to PLAYER_DIR or where
	 * you keep your gods folder
	 */
	dp = opendir ("../player");
    }

    if (dp != NULL)
    {
 	while ( (ep = readdir (dp)) )
      	{
	    if ( ep->d_name[0] == '.' )
	    	continue;

	/*
	 * change the string literals to fit your needs like above
	 */
	    sprintf( buf, "%s%s", fImmortal ? "../gods/" : "../player/", ep->d_name );

	    days = days_since_last_file_mod( buf );

	/*
	 * color coding using Lopes comaptable color to highlight
	 * inactivity. green = active, red = innactive
	 * Just remove the colors if they cause problems.
	 */
	    sprintf( buf, "%-15s %s%-3d{x days\n\r",
		ep->d_name, days > 40 ? "{r" : days > 20 ? "{Y" : days > 10 ? "{g" : "{G", days );
	    strcat( buffer, buf );

            if ( ep->d_name && days > 40 )
            {
                do_pwipe( ch, ep->d_name );
                pshow = TRUE;
            }
        }
      	closedir (dp);
    }
    else
    	perror ("Couldn't open the directory");

    page_to_char( buffer, ch );

    if ( pshow )
    send_to_char( "\n\rAll player files over 40 days old have been deleted.\n\r", ch );

  return;
}

void do_double( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char arg1[MIL];
    char arg2[MIL];
    int amount;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1);
               one_argument( argument, arg2);

    if ( arg[0] == '\0'|| arg1[0] == '\0' )
    {
        send_to_char("Syntax: all <on|off> ticks.\n\r",ch);
        send_to_char("Syntax: <type> <on|off> ticks.\n\r", ch );
        send_to_char("Valid types are: exp | quest | quad\n\r", ch );
        return;
    }

    if (!str_cmp(arg, "all"))
    {
        if (!str_cmp(arg1, "on"))
        {
            if ( arg2[0] == '\0' || !is_number( arg2 ) )
            {
                send_to_char("You need to apply the number of ticks.\n\r", ch );
                return;
            }

            if (double_exp || double_qp || quad_damage )
            {
                send_to_char("One of the types is already in affect! Please turn it off in order to use the all option.\n\r",ch);
                return;
            }

            amount = atoi( arg2 );

            if ( amount < 0 || amount > 500 )
            {
                send_to_char( "Please choose an amount between 0 and 500.\n\r", ch );
                return;
            }
 
            global_exp = amount;
            double_exp = TRUE;
            global_qp = amount;
            double_qp = TRUE;
            global_quad = amount;
            quad_damage = TRUE;
            info( NULL, 0, "{G[INFO]:{x {R%s has declared %d ticks of double exp, double qp, and quad damage for everyone!{x\n\r", ch->name, amount );
            return;
        }                
 
        if (!str_cmp(arg1, "off"))
        {
            if (!double_exp && !double_qp && !quad_damage)
            {
                send_to_char("All of the double types are off. Turn them on first.\n\r",ch);
                return;
            }

            double_exp = FALSE;
            global_exp = 0;
            global_qp = 0;
            double_qp = FALSE;
            quad_damage = FALSE;
            global_quad = 0;
            info( NULL, 0, "{G[INFO]:{x {R%s has removed all of the double types!{x\n\r", ch->name);
            return;             
        }
    }

    if (!str_cmp( arg, "exp"))
    {
        if (!str_cmp(arg1, "on"))
        {
            if ( arg2[0] == '\0' || !is_number( arg2 ) )
            {
                send_to_char("You need to apply the number of ticks.\n\r", ch );
                return;
            }

            if (double_exp)
            {
                send_to_char("Double exp is already in affect!\n\r",ch);
                return;
            }

            amount = atoi( arg2 ); 
           
            if ( amount < 0 || amount > 500 )
            {
                send_to_char( "Please choose an amount between 0 and 500.\n\r", ch );
                return;
            }

            global_exp = amount;
            double_exp = TRUE;  
            
            info( NULL, 0, "{G[INFO]:{x {R%s has declared %d ticks of double exp for everyone!{x\n\r", ch->name, amount );
            return;      
        }

        if (!str_cmp(arg, "off"))
        {
            if (!double_exp)
            {
                send_to_char("Double exp is not on please turn it on first!\n\r",ch);
                return;
            }
        
            double_exp = FALSE;
            global_exp = 0;
            info( NULL, 0, "{G[INFO]:{x {R%s has removed double experience!{x\n\r",ch->name);
            return;
        }         
    }

    if (!str_cmp( arg, "quest"))
    {
        if (!str_cmp(arg1, "on"))
        {
            if ( arg2[0] == '\0' || !is_number( arg2 ) )
            {
                send_to_char("You need to apply the number of ticks.\n\r", ch );
                return;
            }

            if (double_qp)
            {
                send_to_char("Double questpoints is already in affect!\n\r",ch);
                return;
            }

            amount = atoi( arg2 ); 
           
            if ( amount < 0 || amount > 500 )
            {
                send_to_char( "Please choose an amount between 0 and 500.\n\r", ch );
                return;
            }

            global_qp = amount;
            double_qp = TRUE;  
            
            info( NULL, 0, "{G[INFO]:{x {R%s has declared %d ticks of double questpoints for everyone!{x\n\r", ch->name, amount );
            return;      
        }

        if (!str_cmp(arg, "off"))
        {
            if (!double_qp)
            {
                send_to_char("Double questpoints is not on please turn it on first!\n\r",ch);
                return;
            }
        
            double_qp = FALSE;
            global_qp = 0;
            info( NULL, 0, "{G[INFO]:{x {R%s has removed double questpoints!{x\n\r",ch->name);
            return;
        }         
    }

    if (!str_cmp( arg, "quad"))
    {
        if (!str_cmp(arg1, "on"))
        {
            if ( arg2[0] == '\0' || !is_number( arg2 ) )
            {
                send_to_char("You need to apply the number of ticks.\n\r", ch );
                return;
            }

            if (quad_damage)
            {
                send_to_char("Quad damage is already in affect!\n\r",ch);
                return;
            }

            amount = atoi( arg2 ); 
           
            if ( amount < 0 || amount > 500 )
            {
                send_to_char( "Please choose an amount between 0 and 500.\n\r", ch );
                return;
            }

            global_quad  = amount;
            quad_damage = TRUE;  
            
            info( NULL, 0, "{G[INFO]:{x {R%s has declared %d ticks of quad damage for everyone!{x\n\r", ch->name, amount );
            return;      
        }

        if (!str_cmp(arg, "off"))
        {
            if (!quad_damage)
            {
                send_to_char("Quad damage is not on please turn it on first!\n\r",ch);
                return;
            }
        
            quad_damage = FALSE;
            global_quad = 0;
            info( NULL, 0, "{G[INFO]:{x {R%s has removed quad damage!{x\n\r",ch->name);
            return;
        }         
    }

    send_to_char("Syntax: all <on|off> ticks.\n\r",ch);
    send_to_char("Syntax: <type > <on|off> ticks.\n\r", ch );
    send_to_char("Valid types are: exp | quest | quad\n\r", ch );
}

void do_timer( CHAR_DATA *ch, char *argument )
{
    char arg[MSL];
    char arg1[MSL];
    char arg2[MSL];
    int amount;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );
    copyover_person = ch;

    if (arg[0] == '\0' || arg1[0] == '\0' )
    {
        send_to_char( "<Type> <on|off> ticks.\n\r", ch );
        send_to_char( "Valid types are: copyover | reboot | shutdown\n\r", ch );
    }
 

    if (!str_cmp( arg, "copyover" ) )
    {
        if (!str_cmp( arg1, "on" ) )
        {
            if ( arg2[0] == '\0' || !is_number( arg2 ) )
            {
                send_to_char( "You need to apply the number of ticks.\n\r", ch );  
                return;
            }

            if (timed_copyover)
            {
                send_to_char("The copyover timer is already counting down.\n\r", ch );
                return;
            }

            amount = atoi(arg2);

            if ( amount < 2 || amount > 10 )
            {
                send_to_char( "Please choose an amount between 2 and 10.\n\r", ch );
                return;
            }

            timed_copyover = TRUE;
            copyovert = amount;

            info( NULL, 0, "{G[INFO]:{x {R%s has declared an automatic copyover in %d ticks!{x\n\r", ch->name, amount );
            return;      
        }

        if (!str_cmp( arg1, "off" ) )
        {
            if (!timed_copyover)
            {
                send_to_char( "Timed copyover is not on please turn it on first.\n\r", ch );
                return;
            }
 
            timed_copyover = FALSE;
            copyovert = 0;

            info( NULL, 0, "{G[INFO]:{x {R%s has stopped the automatic copyover.{x\n\r",ch->name);
            return;
        }         
    }

    if (!str_cmp( arg, "reboot"))
    {
        if (!str_cmp(arg1, "on"))
        {
            if ( arg2[0] == '\0' || !is_number( arg2 ) )
            {
                send_to_char("You need to apply the number of ticks.\n\r", ch );
                return;
            }

            if (treboot)
            {
                send_to_char("The reboot timer is already counting down!\n\r",ch);
                return;
            }
           
            amount = atoi( arg2 ); 
           
            if ( amount < 2 || amount > 10 )
            {
                send_to_char( "Please choose an amount between 2 and 10.\n\r", ch );
                return;
            }

            reboott = amount;
            treboot = TRUE;  
            
            info( NULL, 0, "{G[INFO]:{x {R%s has declared a reboot in %d ticks!{x\n\r", ch->name, amount );
            return;      
        }

        if (!str_cmp(arg1, "off"))
        {
            if (!treboot)
            {
                send_to_char("Timed reboot is not on turn it on first!\n\r",ch);
                return;
            }
        
            treboot = FALSE;
            reboott = 0;
            info( NULL, 0, "{G[INFO]:{x {R%s has stopped the timed reboot!{x\n\r",ch->name);
            return;
        }         
    }
   
    if (!str_cmp( arg, "shutdown"))
    {
        if (!str_cmp(arg1, "on"))
        {
            if ( arg2[0] == '\0' || !is_number( arg2 ) )
            {
                send_to_char("You need to apply the number of ticks.\n\r", ch );
                return;
            }

            if (tshutdown)
            {
                send_to_char("Timed shutdown is already counting down!\n\r",ch);
                return;
            }

            amount = atoi( arg2 ); 
           
            if ( amount < 2 || amount > 10 )
            {
                send_to_char( "Please choose an amount between 2 and 10.\n\r", ch );
                return;
            }

            shutdownt  = amount;
            tshutdown = TRUE;  
            
            info( NULL, 0, "{G[INFO]:{x {R%s has declared a timed shutdown in %d ticks!{x\n\r", ch->name, amount );
            return;      
        }

        if (!str_cmp(arg, "off"))
        {
            if (!tshutdown)
            {
                send_to_char("Timed shutdown is not on please turn it on first!\n\r",ch);
                return;
            }
        
            tshutdown = FALSE;
            shutdownt = 0;
            info( NULL, 0, "{G[INFO]:{x {R%s has stopped the timed shutdown!{x\n\r",ch->name);
            return;
        }         
    }
}

void reboot( )
{
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    info( NULL, 0, "{G[INFO]{x Automatic reboot happening now. Come back in a minute.\n\r" );

    while (auction_list != NULL)
      reset_auc (auction_list, TRUE);
    
    save_gquest_data();  

    if (war_info.iswar != WAR_OFF)
	end_war();

    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
	    save_char_obj(vch);
    	close_socket(d);
    }
    
    return;
}

void shutdown( )
{
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    info( NULL, 0, "{G[INFO]:{x Automatic shutdown happening now. Come back in a second!\n\r" );
    
    while (auction_list != NULL)
      reset_auc (auction_list, TRUE);

    save_gquest_data();

    merc_down = TRUE;
    if (war_info.iswar != WAR_OFF)
	end_war();
    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
	    save_char_obj(vch);
	close_socket(d);
    }
    return;
}
