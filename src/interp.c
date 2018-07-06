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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "merc.h"
#include "interp.h"
#include "olc.h"
#include "recycle.h"
#include "const.h"
bool	check_social	args( ( CHAR_DATA *ch, char *command,
			    char *argument ) );
bool 	check_disabled (CMD_DATA *command);
struct  wizcommand_type *wizcommand_table[9];

#define END_MARKER	"END" /* for load_disabled() and save_disabled() */

void gen_wiz_table (void);

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2



/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;




/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    char *allowed_commands = "say inv who score quit";
    int trust;
    bool found;
    CMD_DATA *pCmd;
    OBJ_DATA *pObj;

    /*
     * Strip leading spaces.
     */
    while ( isspace(*argument) )
	argument++;
    if ( argument[0] == '\0' )
	return;

    /*
     * No hiding.
     */
    REMOVE_BIT( ch->affected_by, AFF_HIDE );

    if (IS_SET( ch->comm2, COMM_AUTO_AFK ) )
    set_auto_afk( ch );

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
	send_to_char( "You're totally frozen!\n\r", ch );
	return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    if(!IS_NPC(ch) )
	ch->pcdata->last_typed = current_time;

    strcpy( logline, argument );
    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace(*argument) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }

    if (!is_full_name(command, allowed_commands) && IS_SET( ch->act, PLR_TIMEOUT ) )
    {
	printf_to_char(ch, "You can only execute following commands: %s.\n", allowed_commands);
        return;
    }

    /*
     * Look for command in command table.
     */
    if(HAS_TRIGGER_ROOM(ch->in_room,TRIG_ALIAS ) )
    {    if( p_alias_trigger(ch, command, NULL, PRG_RPROG, argument) )
	     return;
    }

    for( pObj = ch->in_room->contents ; pObj ; pObj = pObj->next_content )
    {
	if(HAS_TRIGGER_OBJ(pObj, TRIG_ALIAS ) )
	    if( p_alias_trigger(ch, command, pObj, PRG_OPROG, argument ) )
		return;
    }

    for ( pObj = ch->carrying; pObj; pObj = pObj->next_content )
    {
	if(HAS_TRIGGER_OBJ(pObj, TRIG_ALIAS ) )
            if( p_alias_trigger(ch, command, pObj, PRG_OPROG, argument ) )
                return;
    }

    found = FALSE;
    trust = get_trust( ch );
    for ( pCmd = cmd_first; pCmd ; pCmd = pCmd->next )
    {
	if ( command[0] == pCmd->name[0]
	&&   !str_prefix( command, pCmd->name )
	&&   pCmd->level <= trust )
	{
	    found = TRUE;
	    break;
	}
    }
    if ( !found )
    {
	/*
	 * Look for command in socials table.
	 */
	if ( !check_social( ch, command, argument ) )
	    send_to_char( "That command doesn't exist type help for a list of valid commands.\n\r", ch );
  	    return;
    }

    /*
     * Log and snoop.
     */
    if ( pCmd->log == LOG_NEVER )
	strcpy( logline, "" );

    if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
    ||   fLogAll
    ||   pCmd->log == LOG_ALWAYS )
    {
	sprintf( log_buf, "Log %s: %s", ch->name, logline );
	wiznet(log_buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
	log_string( log_buf );
    }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    if ( IS_SET(ch->act, PLR_IMPLAG ) )
    {
        WAIT_STATE(ch,2*PULSE_VIOLENCE);
    }


    else /* a normal valid command.. check if it is disabled */
     	if (check_disabled (pCmd))
     	{
     		send_to_char ("This command has been temporarily disabled.\n\r",ch);
     		return;
     	}
  
    /*
     * Character not in position for command?
     */
    if ( ch->position < pCmd->position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "Lie still; you are DEAD.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "Better stand up first.\n\r",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	}
	return;
    }

    /*
     * Dispatch the command.
     */
    (*pCmd->do_fun) ( ch, argument );

    tail_chain( );
    return;
}

/* function to keep argument safe in all commands -- no static strings */
void do_function (CHAR_DATA *ch, DO_FUN *do_fun, char *argument)
{
    char *command_string;
    
    /* copy the string */
    command_string = str_dup(argument);
    
    /* dispatch the command */
    (*do_fun) (ch, command_string);
    
    /* free the string */
    free_string(command_string);
}
    
bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You are anti-social!\n\r", ch );
	return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
	send_to_char( "In your dreams, or what?\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
	act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim) 
	&&   victim->desc == NULL)
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( social_table[cmd].others_found,
		    victim, NULL, ch, TO_NOTVICT );
		act( social_table[cmd].char_found,
		    victim, NULL, ch, TO_CHAR    );
		act( social_table[cmd].vict_found,
		    victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( char *arg )
{
 
    if ( *arg == '\0' )
        return FALSE;
 
    if ( *arg == '+' || *arg == '-' )
        arg++;
 
    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }
 
    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(char *argument, char *arg)
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '*' )
        {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '*';
            strcpy( arg, pdot+1 );
            return number;
        }
    }
 
    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Contributed by Alander.
 * Modified to sort by category by Atlas of One Percent MUD. 1mud.org port 9000
 * Please read and follow the DIKU license, as this modification is a derivative work!
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    CMD_DATA * cmd;
    int col;
    int category;
 
    col = 0;
	output = new_buf();

 /* I had to categorize the immortal commands, but not print them to mortals, 
  * so immortal commands are category 0, and I started the loop with category
  * 1 commands. There's probably a better way to do this, but hey, it works!
  * I also have the commands that I felt were most important in the higher 
  * categories, which print out last. The more useless commands are in lower 
  * categories so they might scroll off the screen.

  * Wondering who made it so that nothing will scroll off the screen? Why, it
  * was Laeseah! Sorry, but it was driving me nuts. ;)
  */
    for( category = 1; category < 8; category++ )
    {
      switch(category)
      {
      case 1:
              add_buf(output, "*** {CConfiguration Commands{x ***\n\r");  
	      break;							
      case 2: 							
	      add_buf(output, "*** {BCommon Commands{x ***\n\r"); 		 
	      break;								 
      case 3:
	      add_buf(output, "*** {WCommunication Commands{x ***\n\r");
	      break;
      case 4:
	      add_buf(output, "*** {rInformational Commands{x ***\n\r");
	      break;
      case 5:
	      add_buf(output, "*** {YObject Manipulation Commands{x ***\n\r");
	      break;
      case 6:
	      add_buf(output, "*** {mMovement Commands{x ***\n\r");
	      break;
      case 7:
	      add_buf(output, "*** {rCombat Commands{x ***\n\r");
	      break;
      }
       for ( cmd = cmd_first ; cmd ; cmd = cmd->next )
       {
           if ( cmd->level <  LEVEL_HERO
           &&   cmd->level <= get_trust( ch ) 
	   &&   cmd->show
	   &&   cmd->cat == category) /* <--- this ensures the category equals the current category loop value */
	   {
	       sprintf( buf, " [{c%-11s{x]", cmd->name );
	       add_buf(output, buf );
	       if ( ++col % 5 == 0 )
		  add_buf(output, "\n\r");
	   }
       }
       if (col % 5 != 0 )
       {
           add_buf(output, "\n\r");
           col = 0;
       }
    }
 
    if ( col % 5 != 0 )
	add_buf(output, "\n\r");
	page_to_char(buf_string(output), ch);
	free_buf(output);
    return;
}


void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int col;
    int counter;
    int level;
    int v_level; 
    struct wizcommand_type *tmp;

    v_level = UMAX (ch->level, get_trust(ch));
	
    for (counter = MAX_LEVEL-v_level; counter < 9; counter++)
    {
	 level = MAX_LEVEL-counter;
    	 
         col = 0;
	
         if (level % 2 == 0)
	     sprintf (buf, "{BLevel %3d:{x  ", level);

         if (col % 5) send_to_char("\n\r",ch);            

	 else
	     sprintf (buf, "{CLevel %3d:{x  ", level);
       	     send_to_char(buf, ch);	

         if (col % 5) send_to_char("\n\r",ch);
		
         for (tmp = wizcommand_table[counter]; tmp != NULL ; tmp = tmp->next)
	 {
	      sprintf( buf, "%-13s", tmp->name );
	      send_to_char( buf, ch );
	    
              if ( ++col % 5 == 0 )
	 	  send_to_char( "\n\r            ", ch );
	 }

         send_to_char ("\n\r",ch);
    }

    return;
} 

void gen_wiz_table (void)
{
    CMD_DATA * cmd;
    int level;
    struct wizcommand_type *tmp;
    bool show;
	
    for ( cmd = cmd_first; cmd; cmd = cmd->next)
    {
 	 level = cmd->level;
	 show = cmd->show == 1 ? TRUE : FALSE;
	 
         if (level >= HE && show) 
         {
	     if (wizcommand_table[MAX_LEVEL-level] == NULL)
	     {
	 	 tmp = (struct wizcommand_type *) malloc (sizeof (struct wizcommand_type));
		 tmp->name = strdup (cmd->name); 
 	 	 tmp->next = NULL;	
	 	 wizcommand_table[MAX_LEVEL-level] = tmp;
	     }
	 else
	 {
		 for (tmp = wizcommand_table[MAX_LEVEL-level] ; tmp->next != NULL ; tmp = tmp->next);
  	 	      tmp->next = (struct wizcommand_type *) malloc (sizeof (struct wizcommand_type));
	   	      tmp->next->name = strdup(cmd->name); 
		      tmp->next->next = NULL;	
	 }
	 }
    }
}			


void do_disable (CHAR_DATA *ch, char *argument)
{	CMD_DATA *pCmd;
	
	if(argument[0] == '\0' )
	{	send_to_char("{DDisabled Commands{r:\n\r",ch);
		for(pCmd = cmd_first ; pCmd ; pCmd = pCmd->next )
			if(pCmd->disabled)
				printf_to_char(ch, "\t{W%s{x\n\r", pCmd->name );
		return;
	}

	if( ( pCmd = cmd_lookup(argument ) ) == NULL )
	{	send_to_char("You cannot disable a command that doesn't exist!\n\r",ch);
		return;
	}

	if(pCmd->disabled )
	{	send_to_char("Command enabled!\n\r",ch);
		pCmd->disabled = FALSE;
		return;
	}
	else
	{	send_to_char("Command disabled!\n\r",ch);
		pCmd->disabled = TRUE;
		return;
	}
	return;
}

bool check_disabled (CMD_DATA *command)
{
	return command->disabled;
}

