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
 **************************************************************************/

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
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "include.h"

/* Local variables */
void do_quotes  args( ( CHAR_DATA *ch ) );

/* RT code to delete yourself */

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];

   if (IS_NPC(ch))
	return;
  
   if (ch->pcdata->confirm_delete)
   {    MEMBER_DATA *pMem, *temp;
	if (argument[0] != '\0')
	{
	    send_to_char("Delete status removed.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
    	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
	    stop_fighting(ch,TRUE);
       	    if (ch->level > HERO)
	    {
		update_wizlist(ch, 1);
	    }
	   if(is_clan(ch) )
           {   pMem = member_lookup(&clan_table[ch->clan], ch->name );
        	
               if(pMem )
               {    if(pMem == clan_table[ch->clan].member_first )
                    {   clan_table[ch->clan].member_first = pMem->next;
                        free_member(pMem);
                    }
                    else
                        for(temp = clan_table[ch->clan].member_first ; temp ; temp = temp->next )
                        {   if(temp->next == pMem )
                            {   temp->next = pMem->next;
                                free_member(pMem);
                            }
                        }
               }
           }

	    do_function(ch, &do_quit, "");
	    unlink(strsave);
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type delete. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible.\n\r",ch);
    send_to_char("Typing delete with an argument will undo delete status.\n\r",
	ch);
    ch->pcdata->confirm_delete = TRUE;
    wiznet("$N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));
}
	    

/* RT code to display channel status */

void do_channels( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    /* lists all channels and their status */

/*    send_to_char("   channel     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
 
    send_to_char("{dgossip{x         ",ch);
    if (!IS_SET(ch->comm,COMM_NOGOSSIP))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("{aauction{x        ",ch);
    if (!IS_SET(ch->comm,COMM_NOAUCTION))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("{emusic{x          ",ch);
    if (!IS_SET(ch->comm,COMM_NOMUSIC))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("{qQ{x/{fA{x            ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUESTION))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("{hQuote{x          ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUOTE))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("{tgrats{x          ",ch);
    if (!IS_SET(ch->comm,COMM_NOGRATS))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    if (IS_IMMORTAL(ch))
    {
      send_to_char("{igod channel{x    ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ))
        send_to_char("ON\n\r",ch);
      else
        send_to_char("OFF\n\r",ch);
    }

    send_to_char("{tshouts{x         ",ch);
    if (!IS_SET(ch->comm,COMM_SHOUTSOFF))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("{bpray{x           ",ch);
    if (!IS_SET(ch->comm,COMM_NOPRAY))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);
    
    send_to_char("{Brace channel{x   ",ch);
    if (!IS_SET(ch->comm, COMM_NORACE))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    if ( IS_HERO( ch ) )
    {
    send_to_char("{Rhero channel{x   ",ch);
    if (!IS_SET(ch->comm,COMM_NOHERO))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch); 
    }

    send_to_char("{Gooc channel{x    ",ch);
    if (!IS_SET(ch->comm,COMM_NOOOC))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("hint        ",ch);
    if (!IS_SET(ch->comm,COMM_NOHINT))
      send_to_char("   ON\n\r",ch);
    else
      send_to_char("   OFF\n\r",ch);
   
    send_to_char("newbie      ",ch);
    if (!IS_SET(ch->comm2,COMM_NONEWBIE))
      send_to_char("   ON\n\r",ch);
    else
      send_to_char("   OFF\n\r",ch);

    send_to_char("info        ",ch);
    if (!IS_SET(ch->comm, COMM_NOINFO) )
      send_to_char("   ON\n\r",ch);
    else
      send_to_char("   OFF\n\r",ch);
     
    send_to_char("announce    ",ch);
    if (!IS_SET(ch->comm2, COMM_ANNOUNCE ) )
      send_to_char("   ON\n\r",ch);
    else
      send_to_char("   OFF\n\r",ch);
    if (!IS_SET(
    send_to_char("{ktells{x          ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("{tquiet mode{x     ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

     send_to_char("{c{tPlayer Entry{c    ",ch);
     if (IS_SET(ch->comm,COMM_PENTER))
       send_to_char("{cON{c\n\r",ch);
     else
       send_to_char("{cOFF{c\n\r",ch);

    if (IS_SET(ch->comm,COMM_AFK))
	send_to_char("You are AFK.\n\r",ch);

    if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
	send_to_char("You are immune to snooping.\n\r",ch);
   
    if (ch->lines != PAGELEN)
    {
	if (ch->lines)
	{
	    sprintf(buf,"You display %d lines of scroll.\n\r",ch->lines+2);
	    send_to_char(buf,ch);
 	}
	else
	    send_to_char("Scroll buffering is off.\n\r",ch);
    }

    if (ch->prompt != NULL)
    {
	sprintf(buf,"Your current prompt is: %s\n\r",ch->prompt);
	send_to_char(buf,ch);
    }

    if (IS_SET(ch->comm,COMM_NOSHOUT))
      send_to_char("You cannot shout.\n\r",ch);
  
    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("You cannot use tell.\n\r",ch);
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
     send_to_char("You cannot use channels.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("You cannot show emotions.\n\r",ch);
*/



send_to_char("{cO--------------------------------------------------------------------O\n\r",ch);
send_to_char("{c| {CGLOBAL CHANNELS:                                                   {c|\n\r",ch);
send_to_char("{c| {yGossip         {X: ",ch);
 if (!IS_SET(ch->comm,COMM_NOGOSSIP))
      send_to_char("{GEnabled ",ch);
    else
      send_to_char("{RDisabled",ch);
send_to_char("            {yOOC         {X: ",ch);
    if (!IS_SET(ch->comm,COMM_NOOOC))
        send_to_char("{GEnabled ",ch);
    else
        send_to_char("{RDisabled",ch);
send_to_char("        {c|\n\r",ch);
send_to_char("{c| {yAuction        {X: ",ch);
    if (!IS_SET(ch->comm,COMM_NOAUCTION))
      send_to_char("{GEnabled ",ch);
    else
      send_to_char("{RDisabled",ch);
send_to_char("            {yMusic       {X: ",ch);
    if (!IS_SET(ch->comm,COMM_NOMUSIC))
      send_to_char("{GEnabled ",ch);
    else
      send_to_char("{RDisabled",ch);
send_to_char("        {c|\n\r",ch);
send_to_char("{c| {yQ/A            {X: ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUESTION))
      send_to_char("{GEnabled ",ch);
    else
      send_to_char("{RDisabled",ch);
send_to_char("            {yQuote       {X: ",ch);
     if (!IS_SET(ch->comm,COMM_NOQUOTE))
	send_to_char("{GEnabled ",ch);
    else
	send_to_char("{RDisabled",ch);
send_to_char("        {c|\n\r",ch);
send_to_char("{c| {yGrats          {X: ",ch);
    if (!IS_SET(ch->comm,COMM_NOGRATS))
      send_to_char("{GEnabled ",ch);
    else
      send_to_char("{RDisabled",ch);
send_to_char("            {yShouts      {X: ",ch);
    if (!IS_SET(ch->comm,COMM_SHOUTSOFF))
      send_to_char("{GEnabled ",ch);
    else
      send_to_char("{RDisabled",ch);
send_to_char("        {c|\n\r",ch);
send_to_char("{c| {yHint           {X: ",ch);
    if (!IS_SET(ch->comm,COMM_NOHINT))
      send_to_char("{GEnabled ",ch);
    else
      send_to_char("{RDisabled",ch);
send_to_char("            {yNewbie      {X: ",ch);
    if (!IS_SET(ch->comm2,COMM_NONEWBIE))
      send_to_char("{GEnabled ",ch);
    else
      send_to_char("{RDisabled",ch);
send_to_char("        {c|\n\r",ch);
send_to_char("{c| {yPlayer Entry   {X: ",ch);
     if (IS_SET(ch->comm,COMM_PENTER))
       send_to_char("{GEnabled ",ch);
     else
       send_to_char("{RDisabled",ch);
send_to_char("            {yAnnounce    {X: ",ch);
    if (!IS_SET(ch->comm2, COMM_ANNOUNCE ) )
      send_to_char("{GEnabled ",ch);
    else
       send_to_char("{RDisabled",ch);
send_to_char("        {c|\n\r",ch);
send_to_char("{c| {yInfo           {X: ",ch);
    if (!IS_SET(ch->comm, COMM_NOINFO) )
      send_to_char("{GEnabled ",ch);
    else
      send_to_char("{RDisabled",ch);
send_to_char("                                          {c|\n\r",ch);
send_to_char("{c|                                                                    {c|\n\r",ch);
send_to_char("{c| {COTHER CHANNELS:                                                    {c|\n\r",ch);
send_to_char("{c| {yRace           {X: ",ch);
    if (!IS_SET(ch->comm, COMM_NORACE))
	send_to_char("{GEnabled ",ch);
    else
	send_to_char("{RDisabled",ch);
send_to_char("            {yPray        {X: ",ch);
    if (!IS_SET(ch->comm,COMM_NOPRAY))
      send_to_char("{GEnabled ",ch);
    else
      send_to_char("{RDisabled",ch);
send_to_char("        {c|\n\r",ch);
    if ( IS_HERO( ch ) )
    {
	send_to_char("{c| {yHero           {X: ",ch);
	    if (!IS_SET(ch->comm,COMM_NOHERO))
      	  send_to_char("{GEnabled ",ch);
	    else
       	 send_to_char("{RDisabled",ch); 
if (IS_IMMORTAL(ch))
    {
	send_to_char("            {yImmortal    {X: ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ))
        send_to_char("{GEnabled ",ch);
      else
        send_to_char("{RDisabled",ch);
    }
	else
	send_to_char("                                  {X: ",ch);
 
send_to_char("        {c|\n\r",ch);
}
send_to_char("{c|                                                                    |\n\r",ch);
send_to_char("{c| {COTHER CHANNEL INFORMATION:                                        {c |\n\r",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
	send_to_char("{c| {GYou are currently allowing tells.                                  {c|\n\r",ch);
    else
	send_to_char("{c| {RYou are currently refusing tells.                                  {c|\n\r",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
        send_to_char("{c| {RYou are in QUIET mode.                                             {c|\n\r",ch);
        if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
	send_to_char("{c| {GYou are immune to snooping.                                        {c|\n\r",ch);
	if (ch->lines)
	{
	 sprintf(buf,"{c| {XYou are currently scroling %2d lines per page.                     {c |\n\r",ch->lines+2);
	    send_to_char(buf,ch);
 	}
	else
        send_to_char("{c| {XYou are currently not making use of the scroll buffer.          {c   |\n\r",ch);
 if (IS_SET(ch->comm,COMM_AFK))
	send_to_char("{c| {RCertain channels are blocked due to your AFK status.               {c|\n\r",ch);
    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("{c| {RThe Immortals have revoked your tell priveleges.                  {c |\n\r",ch);
    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("{c| {RThe Immortals have revoked your emote priveleges.                  {c|\n\r",ch);
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
      send_to_char("{c| {RThe Immortals have revoked your channel priveleges.                {c|\n\r",ch);
    if (IS_SET(ch->comm,COMM_NOSHOUT))
      send_to_char("{c| {RThe Immortals have revoked your shouting priveleges.              {c |\n\r",ch);
      send_to_char("{c|                                                                    |\n\r",ch);
      send_to_char("{cO--------------------------------------------------------------------O\n\r",ch);
sprintf(buf,"{c| {XYour current PROMPT is {X: %s \n\r",ch->prompt);
	send_to_char(buf,ch);
      send_to_char("{cO--------------------------------------------------------------------O\n\r",ch);

}

/* RT deaf blocks out all shouts */

void do_deaf( CHAR_DATA *ch, char *argument)
{
    
   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("You can now hear tells again.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else 
   {
     send_to_char("From now on, you won't hear tells.\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

/* RT quiet blocks out all communication */

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("Quiet mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }
}

/* afk command */

void do_afk ( CHAR_DATA *ch, char * argument)
{
    char buf[MSL];

    smash_tilde (argument);

    if (argument[0] != '\0')  
    {
        strcpy(buf, argument);
        free_string( ch->pcdata->afk );
        ch->pcdata->afk = str_dup(buf);
        printf_to_char( ch, "Away message set to: %s\n\r", ch->pcdata->afk ); 
 	return;
    }

    if (IS_SET(ch->comm,COMM_AFK))
    {
        if ( ch->pcdata->title2 != NULL )
        {
            set_title( ch, ch->pcdata->title2 );
        }

        send_to_char("AFK mode removed. Type 'replay' to see tells.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_AFK);
    }

    else
    {
        if ( ch->pcdata->afk != NULL )
        {
            free_string( ch->pcdata->title2 );
            ch->pcdata->title2 = str_dup(ch->pcdata->title);
            set_title( ch, ch->pcdata->afk );
        }

        send_to_char("You are now in AFK mode.\n\r",ch);
        SET_BIT(ch->comm,COMM_AFK);
    }
}

void do_replay (CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
	send_to_char("You can't replay.\n\r",ch);
	return;
    }

    if (buf_string(ch->pcdata->buffer)[0] == '\0')
    {
	send_to_char("You have no tells to replay.\n\r",ch);
	return;
    }

    page_to_char(buf_string(ch->pcdata->buffer),ch);
    clear_buf(ch->pcdata->buffer);
}

/* RT chat replaced with ROM gossip */

/* RT answer channel - uses same line as questions */


void do_say( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj, *obj_next;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    if (!IS_NPC(ch) &&( ch->pcdata->condition[COND_DRUNK] > 10 || ch->pcdata->condition[COND_DRUNK] == -1) )
	argument = makedrunk(argument,ch);
    act( "{6$n says '{7$T{6'{x", ch, NULL, argument, TO_ROOM );
    if (!IS_NPC(ch) &&( ch->pcdata->condition[COND_DRUNK] > 10|| ch->pcdata->condition[COND_DRUNK] == -1) )
	argument = makedrunk(argument,ch);
    act( "{6You say '{7$T{6'{x", ch, NULL, argument, TO_CHAR );
    
    if ( !IS_NPC(ch) )
    {
        CHAR_DATA *mob, *mob_next;

        for ( mob = ch->in_room->people; mob != NULL; mob = mob_next )
        {
            mob_next = mob->next_in_room;
            if ( IS_NPC(mob) && HAS_TRIGGER_MOB( mob, TRIG_SPEECH )
            &&   mob->position == mob->pIndexData->default_pos )
            p_act_trigger( argument, mob, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH );
          
            for ( obj = mob->carrying; obj; obj = obj_next )
            {
                obj_next = obj->next_content;
                if ( HAS_TRIGGER_OBJ( obj, TRIG_SPEECH ) )
                p_act_trigger( argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_SPEECH );
            }          
        }
    }    

    for ( obj = ch->in_room->contents; obj ; obj = obj_next )
    {    
	 obj_next = obj->next_content;
         if ( HAS_TRIGGER_OBJ( obj, TRIG_SPEECH ) )
             p_act_trigger( argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_SPEECH );
    }

    if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_SPEECH ) )
        p_act_trigger( argument, NULL, NULL, ch->in_room, ch, NULL, NULL, TRIG_SPEECH );
                 
    return;
}



void do_shout( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0' )
    {
      	if (IS_SET(ch->comm,COMM_SHOUTSOFF))
      	{
            send_to_char("You can hear shouts again.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);
      	}
      	else
      	{
            send_to_char("You will no longer hear shouts.\n\r",ch);
            SET_BIT(ch->comm,COMM_SHOUTSOFF);
      	}
      	return;
    }

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't shout.\n\r", ch );
        return;
    }
 
    REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);

    WAIT_STATE( ch, 12 );

    act( "You shout '$T'", ch, NULL, argument, TO_CHAR );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm, COMM_SHOUTSOFF) &&
	     !IS_SET(victim->comm, COMM_QUIET) ) 
	{
	    act("$n shouts '$t'",ch,argument,d->character,TO_VICT);
	}
    }

    return;
}



void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    if (IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char("You must turn off deaf mode first.\n\r",ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
        sprintf(buf,"{k%s tells you '{K%s{k'{x\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	return;
    }

    if ( !(IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }
  
    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  	return;
    }

    if (IS_SET(victim->comm,COMM_AFK) || IS_SET(victim->comm2, COMM_AUTO_AFK) )
    {
	if (IS_NPC(victim))
	{
	    act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR);
	    return;
	}

	act("$E is AFK, but your tell will go through when $E returns.",
	    ch,NULL,victim,TO_CHAR);

        act("AFK Message: $t ", ch, victim->pcdata->afk, NULL, TO_CHAR);

	sprintf(buf,"{B%s tells you '{C%s{B'{x\n\r",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	return;
    }

    act( "{BYou tell $N '{C$t{B'{x", ch, argument, victim, TO_CHAR );
    act_new("{B$n tells you '{C$t{B'{x",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER_MOB(victim,TRIG_SPEECH))
        p_act_trigger( argument, victim , NULL, NULL, ch, NULL, NULL, TRIG_ACT );

    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
        act("$N seems to have misplaced $S link...try again later.",
            ch,NULL,victim,TO_CHAR);
        sprintf(buf,"{k%s tells you '{K%s{k'{x\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    &&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
    {
        act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,POS_DEAD);
        return;
    }

    if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch))
    {
	send_to_char( "In your dreams, or what?\n\r", ch );
	return;
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
        if (IS_NPC(victim))
        {
            act_new("$E is AFK, and not receiving tells.",
		ch,NULL,victim,TO_CHAR,POS_DEAD);
            return;
        }
 
        act_new("$E is AFK, but your tell will go through when $E returns.",
            ch,NULL,victim,TO_CHAR,POS_DEAD);
        act("AFK Message: $t ", ch, victim->pcdata->afk, NULL, TO_CHAR);
        sprintf(buf,"{B%s tells you '{C%s{B'{x\n\r",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    act_new("{BYou tell $N '{C$t{B'{x",ch,argument,victim,TO_CHAR,POS_DEAD);
    act_new("{B$n tells you '{C$t{B'{x",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    return;
}



void do_yell( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't yell.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
	send_to_char( "Yell what?\n\r", ch );
	return;
    }


    act("You yell '$t'",ch,argument,NULL,TO_CHAR);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   d->character->in_room->area == ch->in_room->area 
        &&   !IS_SET(d->character->comm,COMM_QUIET) )
	{
	    act("$n yells '$t'",ch,argument,d->character,TO_VICT);
	}
    }

    return;
}


void do_emote( CHAR_DATA *ch, char *argument )
{
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

    MOBtrigger = FALSE; 
    if(*argument == '\'')
    {
    	act( "$n$T", ch, NULL, argument, TO_ROOM );
    	act( "$n$T", ch, NULL, argument, TO_CHAR );
    }
    else
    {
	act( "$n $T", ch, NULL, argument, TO_ROOM );
    	act( "$n $T", ch, NULL, argument, TO_CHAR );
    }
    MOBtrigger = TRUE;
    return;
}


void do_pmote( CHAR_DATA *ch, char *argument )
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
 
    act( "$n $t", ch, argument, NULL, TO_CHAR );

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->desc == NULL || vch == ch)
	    continue;

	if ((letter = strstr(argument,vch->name)) == NULL)
	{
            MOBtrigger = FALSE;
	    act("$N $t",vch,argument,ch,TO_CHAR);
	    continue;
            MOBtrigger = TRUE;
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

        MOBtrigger = FALSE;
	act("$N $t",vch,temp,ch,TO_CHAR);
        MOBtrigger = TRUE;
    }
	
    return;
}

void do_whisper ( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *victim;

    argument = one_argument ( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char ( "Whisper to who?\n\r", ch );
        return;
    }
    if ( argument[0] == '\0' )
    {
        send_to_char ( "Whisper what?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
    {
        send_to_char ( "They aren't here.\n\r", ch );
        return;
    }

    act ( "$n whispers to you '$t'", ch, argument, victim, TO_VICT );
    act ( "You whisper to $N '$t'", ch, argument, victim, TO_CHAR );
    act ( "$n whispers something to $N.", ch, NULL, victim, TO_NOTVICT );
    return;
}


/*
 * All the posing stuff.
 */
struct	pose_table_type
{
    char *	message[2*MAX_CLASS];
};

const	struct	pose_table_type	pose_table	[]	=
{
    {
	{
	    "You sizzle with energy.",
	    "$n sizzles with energy.",
	    "You feel very holy.",
	    "$n looks very holy.",
	    "You perform a small card trick.",
	    "$n performs a small card trick.",
	    "You show your bulging muscles.",
	    "$n shows $s bulging muscles."
	}
    },

    {
	{
	    "You turn into a butterfly, then return to your normal shape.",
	    "$n turns into a butterfly, then returns to $s normal shape.",
	    "You nonchalantly turn wine into water.",
	    "$n nonchalantly turns wine into water.",
	    "You wiggle your ears alternately.",
	    "$n wiggles $s ears alternately.",
	    "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers."
	}
    },

    {
	{
	    "Blue sparks fly from your fingers.",
	    "Blue sparks fly from $n's fingers.",
	    "A halo appears over your head.",
	    "A halo appears over $n's head.",
	    "You nimbly tie yourself into a knot.",
	    "$n nimbly ties $mself into a knot.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean."
	}
    },

    {
	{
	    "Little red lights dance in your eyes.",
	    "Little red lights dance in $n's eyes.",
	    "You recite words of wisdom.",
	    "$n recites words of wisdom.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll."
	}
    },

    {
	{
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "Deep in prayer, you levitate.",
	    "Deep in prayer, $n levitates.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle."
	}
    },

    {
	{
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
	    "An angel consults you.",
	    "An angel consults $n.",
	    "The dice roll ... and you win again.",
	    "The dice roll ... and $n wins again.",
	    "... 98, 99, 100 ... you do pushups.",
	    "... 98, 99, 100 ... $n does pushups."
	}
    },

    {
	{
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
	    "Your body glows with an unearthly light.",
	    "$n's body glows with an unearthly light.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it.",
	    "Arnold Schwarzenegger admires your physique.",
	    "Arnold Schwarzenegger admires $n's physique."
	}
    },

    {
	{
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "A spot light hits you.",
	    "A spot light hits $n.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders."
	}
    },

    {
	{
	    "The light flickers as you rap in magical languages.",
	    "The light flickers as $n raps in magical languages.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder."
	}
    },

    {
	{
	    "Your head disappears.",
	    "$n's head disappears.",
	    "A cool breeze refreshes you.",
	    "A cool breeze refreshes $n.",
	    "You step behind your shadow.",
	    "$n steps behind $s shadow.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear."
	}
    },

    {
	{
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug."
	}
    },

    {
	{
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree."
	}
    },

    {
	{
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews."
	}
    },

    {
	{
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown."
	}
    },

    {
	{
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "An eye in a pyramid winks at you.",
	    "An eye in a pyramid winks at $n.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding."
	}
    },

    {
	{
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "Valentine Michael Smith offers you a glass of water.",
	    "Valentine Michael Smith offers $n a glass of water.",
	    "Where did you go?",
	    "Where did $n go?",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot."
	}
    },

    {
	{
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The great god $g gives you a staff.",
	    "The great god $g gives $n a staff.",
	    "Click.",
	    "Click.",
	    "Atlas asks you to relieve him.",
	    "Atlas asks $n to relieve him."
	}
    }
};



void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC(ch) )
	return;

    level = UMIN( ch->level, (int)(sizeof(pose_table) / sizeof(pose_table[0])) - 1 );
    pose  = number_range(0, level);

    act( pose_table[pose].message[2*ch->klass[0]+0], ch, NULL, NULL, TO_CHAR );
    act( pose_table[pose].message[2*ch->klass[0]+1], ch, NULL, NULL, TO_ROOM );

    return;
}



void do_bug( CHAR_DATA *ch, char *argument )
{
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}

void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}


void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d,*d_next;
    int id;
    MEMBER_DATA *pMem;

    if ( IS_NPC(ch) )
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

    if(has_auction(ch))
    {
       printf_to_char(ch,"You can not leave while you have an auction running.\n\r");
       return;
    }

    do_quotes( ch );

    if ( ch->fquit == 0 )
    {
    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
    wiznet("$N rejoins the real world.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));
    
    info( ch, 0, "{G[INFO]:{x {R%s has rejoined the real world!{x\n\r", ch->name );     
    }

   if(is_clan(ch) ) 
   {
	if( (pMem = member_lookup(&clan_table[ch->clan], ch->name) ) != NULL )
	   pMem->laston = current_time;
   }
	
    /*
     * After extract_char the ch is no longer valid!
     */
    save_char_obj( ch );

    /* Free note that might be there somehow */
    if (ch->pcdata->in_progress)
	free_note (ch->pcdata->in_progress);

    id = ch->id;
    d = ch->desc;

    if (ch->desc != NULL) 
    {
 	d->host = str_dup(ch->pcdata->host);
    }

    extract_char( ch, TRUE );
    if ( d != NULL )
	close_socket( d );

    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next)
    {
	CHAR_DATA *tch;

	d_next = d->next;
	tch = d->original ? d->original : d->character;
	if (tch && tch->id == id)
	{
	    extract_char(tch,TRUE);
	    close_socket(d);
	} 
    }

    return;
}


void update_statlist args((CHAR_DATA * ch, bool));

void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    save_char_obj( ch );
    update_statlist(ch, FALSE);
    send_to_char("Saving. Remember that ShadowStorm has automatic saving now.\n\r", ch);
    return;
}



void do_follow( CHAR_DATA *ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_IMMORTAL(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
             ch,NULL,victim, TO_CHAR);
        return;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, skill_lookup("charm person") );
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }
    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{    
    CHAR_DATA *pet;

    if (IS_NPC(ch))
        return;


    if ((pet = ch->pet) != NULL)
    {
    	stop_follower(pet);
    	if (pet->in_room != NULL)
    	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
    	extract_char(pet,TRUE);
    }
    ch->pet = NULL;

    if ( ch->mount
	&& (ch->mount->in_room == ch->in_room || ch->mount->in_room==NULL) )
    {
	pet = ch->mount;
	do_dismount(ch, "");
	if (pet->in_room != NULL)
	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
	else
	    log_string("void nuke_pets: Extracting null pet");
        ch->mount = NULL;
        ch->riding = FALSE;            
	extract_char(pet, TRUE);
    }
    else if (ch->mount)
    {
	ch->mount->mount = NULL;
	ch->mount->riding = FALSE;
    }
    ch->mount = NULL;

    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
    	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete") || !str_cmp(arg2,"mob"))  
    {
        send_to_char("That will NOT be done.\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;

        if (!strcmp(arg, "mount"))
        {
            if (!ch->mount)
            {
                send_to_char("Your don't have a mount.\n\r", ch);
                return;
            }

            if (ch->mount->in_room != ch->in_room)
            {
                send_to_char("Your mount isn't here!\n\r", ch);
                return;
            }
            else
            {
                victim = ch->mount;
            }
        }
        else if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )

	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

        if ( victim->mount == ch )
	{ 
	    if ( !mount_success(ch, victim, FALSE) )
	    {
		act("$N ignores your orders.", ch, NULL, victim, TO_CHAR);
                return;
	    }
	    else
	    {
		sprintf(buf,"%s orders you to \'%s\'.", ch->name, argument);
		send_to_char(buf, victim);
	    	interpret( victim, argument );
                return;
            }                
        }
	else if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
	||  (IS_IMMORTAL(victim) && victim->trust >= ch->trust))
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	    sprintf( buf, "$n orders you to '%s'.", argument );
	    act( buf, ch, NULL, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
    {
	WAIT_STATE(ch,PULSE_VIOLENCE);
	send_to_char( "Ok.\n\r", ch );
    }
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "%s's group:\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"[%2d %s] %-16s %4ld/%4ld hp %4ld/%4ld mana %4ld/%4ld mv %5d xp\n\r",
		    gch->level,
		    IS_NPC(gch) ? "Mob" : class_who(gch),
		    capitalize( PERS(gch, ch) ),
		    gch->hit,   gch->max_hit,
		    gch->mana,  gch->max_mana,
		    gch->move,  gch->max_move,
		    gch->exp    );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act_new("$N isn't following you.",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_CHARM))
    {
        send_to_char("You can't remove charmed mobs from your group.\n\r",ch);
        return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM))
    {
    	act_new("You like your master too much to leave $m!",
	    ch,NULL,victim,TO_VICT,POS_SLEEPING);
    	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act_new("$n removes $N from $s group.",
	    ch,NULL,victim,TO_NOTVICT,POS_RESTING);
	act_new("$n removes you from $s group.",
	    ch,NULL,victim,TO_VICT,POS_SLEEPING);
	act_new("You remove $N from your group.",
	    ch,NULL,victim,TO_CHAR,POS_SLEEPING);
	return;
    }

    victim->leader = ch;
    act_new("$N joins $n's group.",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
    act_new("You join $n's group.",ch,NULL,victim,TO_VICT,POS_SLEEPING);
    act_new("$N joins your group.",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount_gold = 0, amount_silver = 0;
    int share_gold, share_silver;
    int extra_gold, extra_silver;

    argument = one_argument( argument, arg1 );
	       one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount_silver = atoi( arg1 );

    if (arg2[0] != '\0')
	amount_gold = atoi(arg2);

    if ( amount_gold < 0 || amount_silver < 0)
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount_gold == 0 && amount_silver == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold <  amount_gold || ch->silver < amount_silver)
    {
	send_to_char( "You don't have that much to split.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    share_gold   = amount_gold / members;
    extra_gold   = amount_gold % members;

    if ( share_gold == 0 && share_silver == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->silver	-= amount_silver;
    ch->silver	+= share_silver + extra_silver;
    ch->gold 	-= amount_gold;
    ch->gold 	+= share_gold + extra_gold;

    if (share_silver > 0)
    {
	sprintf(buf,
	    "You split %d silver coins. Your share is %d silver.\n\r",
 	    amount_silver,share_silver + extra_silver);
	send_to_char(buf,ch);
    }

    if (share_gold > 0)
    {
	sprintf(buf,
	    "You split %d gold coins. Your share is %d gold.\n\r",
	     amount_gold,share_gold + extra_gold);
	send_to_char(buf,ch);
    }

    if (share_gold == 0)
    {
	sprintf(buf,"$n splits %d silver coins. Your share is %d silver.",
		amount_silver,share_silver);
    }
    else if (share_silver == 0)
    {
	sprintf(buf,"$n splits %d gold coins. Your share is %d gold.",
		amount_gold,share_gold);
    }
    else
    {
	sprintf(buf,
"$n splits %d silver and %d gold coins, giving you %d silver and %d gold.\n\r",
	 amount_silver,amount_gold,share_silver,share_gold);
    }

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share_gold;
	    gch->silver += share_silver;
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    printf_to_char( ch, "{GYou tell the group '%s'{x", argument );
    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	    act_new("{G$n tells the group '$t'{x",
		ch,argument,gch,TO_VICT,POS_SLEEPING);
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach == NULL || bch == NULL)
	return FALSE;

    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}

/*
 * ColoUr setting and unsetting, way cool, Ant Oct 94
 *        revised to include config colour, Ant Feb 95
 */
void do_colour( CHAR_DATA *ch, char *argument )
{
    char 	arg[ MAX_STRING_LENGTH ];

    if( IS_NPC( ch ) )
    {
	send_to_char_bw( "ColoUr is not ON, Way Moron!\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if( !*arg )
    {
	if( !IS_SET( ch->act, PLR_COLOUR ) )
	{
	    SET_BIT( ch->act, PLR_COLOUR );
	    send_to_char( "ColoUr is now ON, Way Cool!\n\r"
		"Further syntax:\n\r   colour {c<{xfield{c> <{xcolour{c>{x\n\r"
		"   colour {c<{xfield{c>{x {cbeep{x|{cnobeep{x\n\r"
		"Type help {ccolour{x and {ccolour2{x for details.\n\r"
		"ColoUr is brought to you by Lope, ant@solace.mh.se.\n\r", ch );
	}
	else
	{
	    send_to_char_bw( "ColoUr is now OFF, <sigh>\n\r", ch );
	    REMOVE_BIT( ch->act, PLR_COLOUR );
	}
	return;
    }

    if( !str_cmp( arg, "default" ) )
    {
	default_colour( ch );
	send_to_char_bw( "ColoUr setting set to default values.\n\r", ch );
	return;
    }

    if( !str_cmp( arg, "all" ) )
    {
	all_colour( ch, argument );
	return;
    }

    /*
     * Yes, I know this is ugly and unnessessary repetition, but its old
     * and I can't justify the time to make it pretty. -Lope
     */
    if( !str_cmp( arg, "text" ) )
    {
	ALTER_COLOUR( text )
    }
    else if( !str_cmp( arg, "auction" ) )
    {
	ALTER_COLOUR( auction )
    }
    else if( !str_cmp( arg, "auction_text" ) )
    {
	ALTER_COLOUR( auction_text )
    }
    else if( !str_cmp( arg, "gossip" ) )
    {
	ALTER_COLOUR( gossip )
    }
    else if( !str_cmp( arg, "gossip_text" ) )
    {
	ALTER_COLOUR( gossip_text )
    }
    else if( !str_cmp( arg, "music" ) )
    {
	ALTER_COLOUR( music )
    }
    else if( !str_cmp( arg, "music_text" ) )
    {
	ALTER_COLOUR( music_text )
    }
    else if( !str_cmp( arg, "question" ) )
    {
	ALTER_COLOUR( question )
    }
    else if( !str_cmp( arg, "question_text" ) )
    {
	ALTER_COLOUR( question_text )
    }
    else if( !str_cmp( arg, "answer" ) )
    {
	ALTER_COLOUR( answer )
    }
    else if( !str_cmp( arg, "answer_text" ) )
    {
	ALTER_COLOUR( answer_text )
    }
    else if( !str_cmp( arg, "quote" ) )
    {
	ALTER_COLOUR( quote )
    }
    else if( !str_cmp( arg, "quote_text" ) )
    {
	ALTER_COLOUR( quote_text )
    }
    else if( !str_cmp( arg, "immtalk_text" ) )
    {
	ALTER_COLOUR( immtalk_text )
    }
    else if( !str_cmp( arg, "immtalk_type" ) )
    {
	ALTER_COLOUR( immtalk_type )
    }
    else if( !str_cmp( arg, "info" ) )
    {
	ALTER_COLOUR( info )
    }
    else if( !str_cmp( arg, "say" ) )
    {
	ALTER_COLOUR( say )
    }
    else if( !str_cmp( arg, "say_text" ) )
    {
	ALTER_COLOUR( say_text )
    }
    else if( !str_cmp( arg, "tell" ) )
    {
	ALTER_COLOUR( tell )
    }
    else if( !str_cmp( arg, "tell_text" ) )
    {
	ALTER_COLOUR( tell_text )
    }
    else if( !str_cmp( arg, "reply" ) )
    {
	ALTER_COLOUR( reply )
    }
    else if( !str_cmp( arg, "reply_text" ) )
    {
	ALTER_COLOUR( reply_text )
    }
    else if( !str_cmp( arg, "gtell_text" ) )
    {
	ALTER_COLOUR( gtell_text )
    }
    else if( !str_cmp( arg, "gtell_type" ) )
    {
	ALTER_COLOUR( gtell_type )
    }
    else if( !str_cmp( arg, "wiznet" ) )
    {
	ALTER_COLOUR( wiznet )
    }
    else if( !str_cmp( arg, "room_title" ) )
    {
	ALTER_COLOUR( room_title )
    }
    else if( !str_cmp( arg, "room_text" ) )
    {
	ALTER_COLOUR( room_text )
    }
    else if( !str_cmp( arg, "room_exits" ) )
    {
	ALTER_COLOUR( room_exits )
    }
    else if( !str_cmp( arg, "room_things" ) )
    {
	ALTER_COLOUR( room_things )
    }
    else if( !str_cmp( arg, "prompt" ) )
    {
	ALTER_COLOUR( prompt )
    }
    else if( !str_cmp( arg, "fight_death" ) )
    {
	ALTER_COLOUR( fight_death )
    }
    else if( !str_cmp( arg, "fight_yhit" ) )
    {
	ALTER_COLOUR( fight_yhit )
    }
    else if( !str_cmp( arg, "fight_ohit" ) )
    {
	ALTER_COLOUR( fight_ohit )
    }
    else if( !str_cmp( arg, "fight_thit" ) )
    {
	ALTER_COLOUR( fight_thit )
    }
    else if( !str_cmp( arg, "fight_skill" ) )
    {
	ALTER_COLOUR( fight_skill )
    }
    else
    {
	send_to_char_bw( "Unrecognised Colour Parameter Not Set.\n\r", ch );
	return;
    }

    send_to_char_bw( "New Colour Parameter Set.\n\r", ch );
    return;
}

void announce( CHAR_DATA * ch, int level, char * message, ...)
{
    char buf[MSL];
    DESCRIPTOR_DATA * d;
    va_list args;

    va_start(args,message);
    vsnprintf(buf, MAX_STRING_LENGTH, message, args);
    va_end(args);

    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->connected == CON_PLAYING && 
             get_trust(d->character) >= level )
        {
           send_to_char(buf, d->character);
           sound( "announce.wav", d->character );
        }
    }
}

void do_announce(CHAR_DATA * ch, char * argument)
{
    char buf[MSL];
    buf[0] = '\0';
    strcat(buf, "{G[ANNOUNCE]:{x ");
    strcat(buf, argument);
    strcat(buf, "\n\r" );
    if(buf[0] == '\0' || !IS_IMMORTAL(ch) )
    {
    	if (IS_SET(ch->comm2,COMM_ANNOUNCE))
    	{
            send_to_char("Announce channel is now ON.\n\r",ch);
            REMOVE_BIT(ch->comm2,COMM_ANNOUNCE);
            return;
    	}
    	else
    	{
            send_to_char("Announce channel is now OFF.\n\r",ch);
            SET_BIT(ch->comm2,COMM_ANNOUNCE);
	    return;
    	}
    }
    else
    {
	announce(ch, 0, buf);
 	return;
    } 
}


void do_pray( CHAR_DATA *ch, char *argument )
 {
     char buf[MAX_STRING_LENGTH];
     DESCRIPTOR_DATA *d;
 
     if ( argument[0] == '\0' )
     {
       if (IS_SET(ch->comm,COMM_NOPRAY))
       {
         send_to_char("Pray channel is now ON\n\r",ch);
         REMOVE_BIT(ch->comm,COMM_NOPRAY);
       }
       else
       {
         send_to_char("Pray channel is now OFF\n\r",ch);
         SET_BIT(ch->comm,COMM_NOPRAY);
       }
       return;
     }
 
     REMOVE_BIT(ch->comm,COMM_NOPRAY);
 
     sprintf( buf, "{y$n:{c %s{x", argument );
     act_new("You [pray] '{B$t{x'",ch,argument,NULL,TO_CHAR,POS_DEAD);
     for ( d = descriptor_list; d != NULL; d = d->next )
     {
         CHAR_DATA *victim;
         victim = d->original ? d->original : d->character;
     
         if ( d->connected == CON_PLAYING
            && IS_IMMORTAL(victim) 
            && !IS_SET(d->character->comm,COMM_NOPRAY)
            && !IS_SET(d->character->comm,COMM_QUIET) )
         {
             act_new("$n [pray] '{B$t'{x",ch,argument,d->character,TO_VICT,POS_DEAD);
         }
     }
 
     return;
}


void do_beep ( CHAR_DATA *ch, char *argument )
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

    sprintf( buf, "\a%s has beeped you.\n\r", ch->name );
    send_to_char( buf, victim );
    return;
}


void do_racetalk( CHAR_DATA *ch, char *argument ) 
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (IS_NPC(ch))
	return;

    if (argument[0] == '\0')
    {
	if (IS_SET(ch->comm, COMM_NORACE))
	{
	    send_to_char("Race channel is now ON.\n\r", ch);
	    REMOVE_BIT(ch->comm, COMM_NORACE);
	}
	else
	{
	    send_to_char("Race channel is now OFF.\n\r", ch);
	    SET_BIT(ch->comm, COMM_NORACE);
	}
    }
    else
    {
	if (IS_SET(ch->comm, COMM_QUIET))
	{
	    send_to_char("You must turn off quiet mode first.\n\r", ch);
	    return;
	}

	if (IS_SET(ch->comm, COMM_NOCHANNELS))
	{
	    send_to_char("The gods have revoked your channels priviliges.\n\r",		ch);
	    return;
	}

	REMOVE_BIT(ch->comm, COMM_NORACE);

	sprintf(buf, "[ %s ] $n '$t'", pc_race_table[ch->race].who_name);
	act_new(buf, ch, argument, NULL, TO_CHAR, POS_DEAD);
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    CHAR_DATA *victim;

	    victim = d->original ? d->original : d->character;

	    if (d->connected == CON_PLAYING
	    &&	victim != ch
	    &&	victim->race == ch->race
	    &&  !IS_NPC(victim) && !IS_SET(victim->comm, COMM_NORACE)
	    &&	!IS_SET(victim->comm, COMM_QUIET))
	    {
		act_new(buf, ch, argument, d->character, TO_VICT,
		    POS_SLEEPING);
	    }
	}
    }
}

void do_clear (CHAR_DATA * ch, char *argument)
{
    send_to_char ("\x01B[2J", ch);
}



char * makedrunk (char *string, CHAR_DATA * ch)
{
  struct struckdrunk drunk[] =
  {
    {3, 10,
     {"a", "a", "a", "A", "aa", "ah", "ah", "ao", "aw", "a", "ah"}},
    {8, 5,
     {"b", "b", "b", "B", "B", "b"}},
    {3, 5,
     {"c", "c", "C", "cj", "c", "c"}},
    {5, 2,
     {"d", "d", "D"}},
    {3, 3,
     {"e", "e", "eh", "E"}},
    {4, 5,
     {"f", "f", "ff", "ff", "Ff", "F"}},
    {8, 2,
     {"g", "g", "G"}},
    {9, 6,
     {"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
    {7, 6,
     {"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
    {9, 5,
     {"j", "j", "jj", "Jj", "jJ", "J"}},
    {7, 2,
     {"k", "k", "K"}},
    {3, 2,
     {"l", "l", "L"}},
    {5, 8,
     {"m", "m", "mm", "m", "m", "mm", "MmM", "mM", "M"}},
    {6, 6,
     {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
    {3, 6,
     {"o", "o", "ooo", "ao", "oo", "Ooo", "Oo"}},
    {3, 2,
     {"p", "p", "P"}},
    {5, 5,
     {"q", "q", "Q", "q", "q", "q"}},
    {4, 2,
     {"r", "r", "R"}},
    {2, 5,
     {"s", "ss", "s", "ssS", "sS", "Sss"}},
    {5, 2,
     {"t", "t", "T"}},
    {3, 4,
     {"u", "u", "uh", "Uh", "Uhh", }},
    {4, 2,
     {"v", "v", "V"}},
    {4, 2,
     {"w", "w", "W"}},
    {5, 6,
     {"x", "x", "X", "ks", "iks", "kz", "xz"}},
    {3, 2,
     {"y", "y", "Y"}},
    {2, 9,
     {"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
  };

  char buf[MSL];
  char temp;
  int pos = 0;
  int drunklevel;
  int randomnum;

  if (IS_NPC(ch))
        drunklevel = 0;
  else
  {     if(ch->pcdata->condition[COND_DRUNK] == -1 )
	      drunklevel = 100;
	else
	      drunklevel = ch->pcdata->condition[COND_DRUNK];
  }
  if (drunklevel > 0)
    {
      do
        {
          temp = toupper (*string);
          if ((temp >= 'A') && (temp <= 'Z'))
            {
              if (drunklevel > drunk[temp - 'A'].min_drunk_level)
                {
                  randomnum = number_range (0, drunk[temp - 'A'].number_of_rep);
                  strcpy (&buf[pos], drunk[temp - 'A'].replacement[randomnum]);
                  pos += strlen (drunk[temp - 'A'].replacement[randomnum]);
                }
              else
                buf[pos++] = *string;
            }
          else
            {
              if ((temp >= '0') && (temp <= '9'))
                {
                  temp = '0' + number_range (0, 9);
                  buf[pos++] = temp;
                }
              else
                buf[pos++] = *string;
            }
        }
      while (*string++);
      buf[pos] = '\0';          /* Mark end of the string... */
      strcpy(string, buf);
      return(string);
    }
  return (string);
}

void do_hint( CHAR_DATA *ch, char *argument)
{
if(IS_NPC(ch))
  return;

if(IS_SET(ch->comm,COMM_NOHINT))
  {
  REMOVE_BIT(ch->comm, COMM_NOHINT);
  send_to_char("Hint channel is now ON.\n\r",ch);
  return;
  }
else
  {
  SET_BIT(ch->comm, COMM_NOHINT);
  send_to_char("Hint channel is now OFF.\n\r",ch);
  return;
  }
}

void do_penter ( CHAR_DATA *ch, char * argument)
{
   if (IS_SET(ch->comm,COMM_PENTER))
   {
      send_to_char("{cPlayer Entry message removed.{c\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_PENTER);
   }
   else
   {
     send_to_char("{cFrom now on, you will see Player Entries.{c\n\r",ch);
     SET_BIT(ch->comm,COMM_PENTER);
   }
}

void do_newchan( CHAR_DATA *ch, char *argument, int channel)
{	char arg[MSL];
	char arg2[MSL];
	char *talk;
	char *pre;
	char emote[2*MSL];
	char buf[MAX_STRING_LENGTH];
	char soc[MSL];
        DESCRIPTOR_DATA *d;
	pre = get_pre(ch, channel);
	if (channel == CHAN_CLAN)
	{
		if (IS_NPC(ch))
		{
		send_to_char("You aren't in a clan.\n\r",ch);
		return;
		}
	}
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,chan_table[channel].bit))
      {
		sprintf( buf, "The %s {xchannel is now ON.\n\r", chan_table[channel].chan_pre );
		send_to_char( buf, ch );
		REMOVE_BIT(ch->comm,chan_table[channel].bit);
		return;
      }
      else
      {
		sprintf( buf, "The %s {xchannel is now OFF.\n\r", chan_table[channel].chan_pre );
		send_to_char( buf, ch );
		SET_BIT(ch->comm,chan_table[channel].bit);
		return;
      }
    }
    else  /* message sent, toggle channel on/off */
	{
		if (IS_SET(ch->comm,COMM_QUIET))
		{
		  send_to_char("You must turn off quiet mode first.\n\r",ch);
		  return;
		}

		if (IS_SET(ch->comm,COMM_NOCHANNELS))
		{
		send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
		return;
		}

		if (IS_SET(ch->comm,chan_table[channel].bit))
		{
		send_to_char("Try turning the channel on?\n\r",ch);
		return;
		}
    }
  
     if( check_color_string(argument) )
	send_to_char("{RFinal {{ removed. Do not do this again.{x\r\n",ch);

    arg2[0] = '\0';
    talk = argument;
    one_argument(argument, arg);
    argument = one_argument(argument,arg);
	sprintf( emote, "%s", argument );
    argument = one_argument(argument,arg2);

    if(islower(*talk) )
       if(strlen(arg) > 2 )
         if(check_chansocial(ch,arg,arg2,channel))
	     return;

	if(!strcmp(arg, "emote") || !strcmp(arg, "!") )
    {
		argument = one_argument(argument,arg);

		sprintf( buf, "%s {%s$n%s%s{x", pre, chan_table[channel].chan_name,  emote[0] == '\'' ? "" : " ", emote);  
		act_chan(buf,ch,argument,NULL,TO_CHAR,POS_DEAD);	
		
		for ( d = descriptor_list; d != NULL; d = d->next )
		{
			CHAR_DATA *vch;
        		vch = d->character; 

			if ( d->connected == CON_PLAYING &&
				 vch != ch && 
				 !IS_SET(vch->comm,chan_table[channel].bit) &&
				 !IS_SET(vch->comm,COMM_QUIET) ) 
				 {
			           	if(can_be_sent(ch,vch,channel) )
					{
			                     act_chan(buf,ch,argument,vch,TO_VICT,POS_DEAD);
					}
			                else
            				     continue;
				}	
			}
		 return;
    }
    if(!strcmp(arg, "smote"))
    {
		if(smote_check(ch, arg, emote, channel))
			return;
    }   

	if(!strcmp(arg, "echo"))
    {
		if(echo_check(ch,arg,emote,channel))
			return;
    }

    sprintf( soc, "%s", get_emote(ch, talk ) );

    if(*talk == '#')
	sprintf(talk, "%s", emote);

      if(channel == CHAN_GOSSIP)
      {
      if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	talk = makedrunk(talk,ch);
      }
    
	   sprintf( buf, "%s {%s%s %s '{%s%s{%s'{x\n\r", 
		pre,
		chan_table[channel].chan_name,ch->name, 
		soc,
		chan_table[channel].chan_say, 
		talk, 
		chan_table[channel].chan_name );
		send_to_char(buf,ch);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *vch;
        vch = ( d->original != NULL ) ? d->original : d->character; 

		if ( d->connected == CON_PLAYING &&
			 !IS_SET(vch->comm,chan_table[channel].bit) &&
			 !IS_SET(vch->comm,COMM_QUIET) )
		{
           if(can_be_sent(ch,vch,channel) )
	   {
	    sprintf( buf, "%s {%s%s %s '{%s%s{%s'{x", 
	  	pre,
		chan_table[channel].chan_name,
		 PERS_CHAN(ch, vch ),
		soc,
		chan_table[channel].chan_say,
		talk,
		chan_table[channel].chan_name);
            act_new(buf,ch,argument,vch,TO_VICT,POS_DEAD);
 	     
	   }
           else
            continue;
	}
   }
}

char *get_emote( CHAR_DATA *ch, char *argument)
{ 
    char arg[MSL], buf[MSL];
    int cmd, len, i, y;
    bool found = FALSE;

    arg[0] = '\0';
    buf[0] = '\0';

    argument = one_argument(argument, arg);
    i = 0;
    if( arg[0] == '#' )
    {	
		len = strlen_color(arg);
		for (i=y=0; i < len; i++)
		{
			if(arg[i] == '#' )
				i++;
			
			buf[y] = arg[i];
			y++;
			buf[y] = '\0';

		}
		
		if( ( cmd = social_lookup(buf) ) < 0)
			return "says";
		else
			found = TRUE;

		if(found)
			return social_table[cmd].chan_emote;
	}   if(!found)
   {
    len = strlen_color(argument);
    len -= 1;
    if(argument[len] == ')' 
    || argument[len] == 'D'
    || argument[len] == ']')
    {
        if(argument[len-1] == ';')
	   return "winks, saying";
        if(argument[len-1] == ':'
        || argument[len-1] == '=')
           return "smiles, saying";
     }
     if( (argument[len] == '('
     || argument[len] == '[')
     && argument[len-1] == ':')
        return "sobs, saying";
     if(argument[len] == 'P'
     && argument[len-1] == ':')
        return "teases, saying";
     if(argument[len] == 'S'
     && argument[len-1] == ':')
        return "says uncomfortably";
     if(argument[len] == '/'
	 && argument[len-1] == ':')
	    return "quirks, saying";
   }
   return "says";
}

char *get_pre(CHAR_DATA *ch, int channel)
{
	if(channel == CHAN_CLAN)
		return clan_table[ch->clan].who_name;
	else 
		return chan_table[channel].chan_pre;
}


bool smote_check(CHAR_DATA *ch, char *argument, char *emote, int channel )
{
    CHAR_DATA *vch;
    char *letter,*name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    size_t matches = 0;
    char *pre;
    char buf[MSL];
    DESCRIPTOR_DATA *d; 
 	
    pre = get_pre(ch, channel);
	
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	return FALSE;
    }
 
    if ( argument[0] == '\0' )
    {
	return FALSE;
    }
    
    if (strstr(emote,ch->name) == NULL)
    {
	send_to_char("You must include your name in an smote.\n\r",ch);
	return TRUE;
    }

    sprintf( buf, "%s {%s%s{x", pre, chan_table[channel].chan_name, emote); 

    send_to_char(buf,ch);
    send_to_char("\n\r",ch);
 
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        vch = ( d->original != NULL ) ? d->original : d->character; 
		if ( d->connected == CON_PLAYING &&
			 vch != ch &&
			 !IS_SET(vch->comm,chan_table[channel].bit) &&
			 !IS_SET(vch->comm,COMM_QUIET) )
		{

		if(!can_be_sent(ch,vch,channel) )
		    continue;
        if (vch->desc == NULL || vch == ch)
            continue;
 
        if ((letter = strstr(argument,vch->name)) == NULL)
        {
		act_chan(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
            continue;
        }
 
        strcpy(temp,emote);
        temp[strlen(emote) - strlen(letter)] = '\0';
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
 
		act_chan(buf,ch,NULL,vch,TO_VICT,POS_DEAD);


    }
  } 
		return TRUE;
}
bool echo_check(CHAR_DATA *ch, char *argument, char *emote, int channel)
{
    DESCRIPTOR_DATA *d;
    char *pre;
	char buf[MSL];
    buf[0] = '\0';
	pre = get_pre(ch, channel);

    if ( argument[0] == '\0' )
    {
	return FALSE;
    }
    if(!IS_IMMORTAL(ch))
	return FALSE;

    sprintf( buf, "%s {%s%s{x", pre, chan_table[channel].chan_name, emote); 
	send_to_char(buf,ch);
	send_to_char("\n\r",ch);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *vch;
        vch = ( d->original != NULL ) ? d->original : d->character;; 
		if ( d->connected == CON_PLAYING &&
			 vch != ch &&
			 !IS_SET(vch->comm,chan_table[channel].bit) &&
			 !IS_SET(vch->comm,COMM_QUIET) )
			{
           if(can_be_sent(ch, vch,channel) )
            act_chan(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
           else
            continue;
   		}
    }
	return TRUE;
}

bool check_chansocial( CHAR_DATA *ch, char *command, char *argument, int channel )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;
    char buf[MSL];
    int counter;
    int count;
    char buf2[MSL];        
    char buf3[MSL];
	char *pre;
    DESCRIPTOR_DATA *d;

	pre = get_pre(ch, channel);


    buf[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';
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

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
      sprintf( buf, "%s {%s{%s%s{%s{x", 
		pre, 
		chan_table[channel].chan_name,
		chan_table[channel].chan_say, 
		social_table[cmd].others_no_arg,
		chan_table[channel].chan_name);

      sprintf( buf2, "%s {%s{%s%s{%s{x\n\r",
		pre,
		chan_table[channel].chan_name,
		chan_table[channel].chan_say,
		social_table[cmd].char_no_arg,
		chan_table[channel].chan_name);

	if(social_table[cmd].char_no_arg[0] == '\0' || social_table[cmd].others_no_arg[0] == '\0')
	{	send_to_char("That part of the social doesn't exist... IDIOT!\n\r",ch);
		return TRUE;
	}

	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    CHAR_DATA *vch;
	    vch = ( d->original != NULL ) ? d->original : d->character;
	    if (d->connected == CON_PLAYING &&
		  vch != ch &&
		  !IS_SET(vch->comm,chan_table[channel].bit) &&
	 	  !IS_SET(vch->comm,COMM_QUIET))
	    {
           if(can_be_sent(ch,vch,channel) )
            act_chan(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
           else
            continue;
          }
       }
	send_to_char(buf2,ch);	
    }
    else if ( ( victim = get_char_world( ch, arg ) ) == NULL || IS_NPC(victim))
    {
	return FALSE;
    }	
    else if(IS_SET(victim->comm,chan_table[channel].bit))
    	{
	    send_to_char("They cannot see this channel, therefor you cannot do that to them.\n\r",ch);
    	    return TRUE;
      }
    else if ( victim == ch )
    {
      sprintf( buf, "%s {%s{%s%s{%s{x",
		pre,
		chan_table[channel].chan_name,
		chan_table[channel].chan_say,
		social_table[cmd].others_auto,
		chan_table[channel].chan_name);

      sprintf( buf2, "%s {%s{%s%s{%s{x\n\r",
		pre,
		chan_table[channel].chan_name,
		chan_table[channel].chan_say,
		social_table[cmd].char_auto,
		chan_table[channel].chan_name);	
	if(social_table[cmd].char_auto[0] == '\0' || social_table[cmd].others_auto[0] == '\0')
	{	send_to_char("That part of the social doesn't exist... IDIOT!\n\r",ch);
		return TRUE;
	}

	send_to_char(buf2,ch);


      for ( d = descriptor_list; d != NULL; d = d->next )
      {
		CHAR_DATA *vch;
        	vch = ( d->original != NULL ) ? d->original : d->character;
           if(!can_be_sent(ch, vch,channel) )
            continue;
 
        if ( d->connected == CON_PLAYING &&
             vch != ch &&
		 victim->name != arg &&
             !IS_SET(vch->comm,chan_table[channel].bit) &&
             !IS_SET(vch->comm,COMM_QUIET) )
        {
	    act_chan(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
        }
      }
    }
    else
    {
      sprintf( buf, "%s {%s{%s%s{%s{x",
		pre,
		chan_table[channel].chan_name,
		chan_table[channel].chan_say,
		social_table[cmd].others_found,
		chan_table[channel].chan_name);

      sprintf( buf2, "%s {%s{%s%s{%s{x",
		pre,
		chan_table[channel].chan_name,
		chan_table[channel].chan_say,
		social_table[cmd].char_found,
		chan_table[channel].chan_name);

      sprintf( buf3, "%s {%s{%s%s{%s{x",
		pre,
		chan_table[channel].chan_name,
		chan_table[channel].chan_say,
		social_table[cmd].vict_found,
		chan_table[channel].chan_name);

	if(social_table[cmd].vict_found[0] == '\0' || social_table[cmd].char_found[0] == '\0' || social_table[cmd].others_found[0] == '\0' )
	{	send_to_char("That part of the social doesn't exist... IDIOT!\n\r",ch);
		return TRUE;
	}

	  act_chan(buf3,ch,NULL,victim,TO_VICT,POS_DEAD);
	  act_chan(buf2,ch,NULL,victim,TO_CHAR,POS_DEAD);
        for (counter = 0; buf[counter+1] != '\0'; counter++)
        {
            if (buf[counter] == '$' && buf[counter + 1] == 'N')
            {
                strcpy(buf2,buf);
                buf2[counter] = '\0';
                strcat(buf2,victim->name);
                for (count = 0; buf[count] != '\0'; count++)
                {
                    buf[count] = buf[count+counter+2];
                }
                strcat(buf2,buf);
                strcpy(buf,buf2);

            }
            else if (buf[counter] == '$' && buf[counter + 1] == 'E')
            {
                switch (victim->sex)
                {
                default:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"it");
                    for (count = 0; buf[count] != '\0'; count ++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                case 1:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"it");
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                case 2:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"it");
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                }
            }
            else if (buf[counter] == '$' && buf[counter + 1] == 'M')
            {
                buf[counter] = '%';
                buf[counter + 1] = 's';
                switch (victim->sex)
                {
                default:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"it");
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                case 1:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"him");
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                case 2:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"her");
                    for (count = 0; buf[count] != '\0'; count++);
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                }
            }
            else if (buf[counter] == '$' && buf[counter + 1] == 'S')
            {
                switch (victim->sex)
                {
                default:
                strcpy(buf2,buf);
                buf2[counter] = '\0';
                strcat(buf2,"its");
                for (count = 0;buf[count] != '\0'; count++)
                {
                    buf[count] = buf[count+counter+2];
                }
                strcat(buf2,buf);
                strcpy(buf,buf2);
                break;
                case 1:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"his");
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                case 2:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"hers");
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                }
            }

        }

      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	  CHAR_DATA *vch;
        vch = ( d->original != NULL ) ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             vch != ch &&
			  victim != vch &&
             !IS_SET(vch->comm,chan_table[channel].bit) &&
             !IS_SET(vch->comm,COMM_QUIET) )
        {
	   if(can_be_sent(ch,vch,channel) )
	    act_chan(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
	   else
	    continue;
        }
      }	
    }

    return TRUE;
}

void do_ooc( CHAR_DATA *ch, char *argument)	    
{
	do_newchan( ch, argument, CHAN_OOC );
	return;
}
void do_gossip( CHAR_DATA *ch, char *argument)	    
{
	do_newchan( ch, argument, CHAN_GOSSIP );
	return;
}
void do_godchat( CHAR_DATA *ch, char *argument )
{ 	do_newchan(ch, argument, CHAN_GOD );
	return;
}

void do_quote( CHAR_DATA *ch, char *argument)	    
{
	do_newchan( ch, argument, CHAN_QUOTE );
	return;
}
void do_grats( CHAR_DATA *ch, char *argument)	    
{
	do_newchan( ch, argument, CHAN_GRATS );
	return;
}
void do_question( CHAR_DATA *ch, char *argument)	    
{
	do_newchan( ch, argument, CHAN_QA );
        sound( "ask.wav", ch );
	return;
}
void do_answer( CHAR_DATA *ch, char *argument )
{
	do_newchan(ch, argument, CHAN_QA );
	return;
}

void do_music( CHAR_DATA *ch, char *argument)	    
{
	do_newchan( ch, argument, CHAN_MUSIC );
	return;
}
void do_immtalk( CHAR_DATA *ch, char *argument)	    
{
	do_newchan( ch, argument, CHAN_IMM );
	return;
}
void do_clantalk( CHAR_DATA *ch, char *argument)	    
{
	do_newchan( ch, argument, CHAN_CLAN );
	return;
}
void do_imp( CHAR_DATA *ch, char *argument )
{
	do_newchan( ch, argument, CHAN_IMP );
	return;
}
void do_admin( CHAR_DATA *ch, char *argument )
{
        do_newchan( ch, argument, CHAN_ADMIN );
        return;
}
void do_hero( CHAR_DATA *ch, char *argument )
{
	do_newchan( ch, argument, CHAN_HERO );
	return;
}      
void do_newbie( CHAR_DATA *ch, char *argument )
{

        do_newchan( ch, argument, CHAN_NEWBIE );
        return;
}

bool can_be_sent(CHAR_DATA *ch, CHAR_DATA *vch, int channel )
{
    if ((channel == CHAN_IMM) && (!IS_IMMORTAL(vch)))
    	return FALSE;
    if (channel == CHAN_CLAN &&  ( !is_same_clan(ch,vch) && !is_admin(vch)) )
        return FALSE;
    if (channel == CHAN_IMP && !IS_IMP(vch) )
	return FALSE; 
    if (channel == CHAN_ADMIN && !is_admin(vch ) )
        return FALSE;
    if (channel == CHAN_HERO && !IS_HERO(vch) && number_classes(vch) <= 1)
	return FALSE;
    if ( channel == CHAN_GOD && vch->level < 157 )
	return FALSE;
    return TRUE;   
}
    
void set_auto_afk(CHAR_DATA *ch)
{
  
    if ( IS_SET(ch->comm, COMM_AFK ))
        return;

    if (!IS_SET(ch->comm2,COMM_AUTO_AFK))
    {
      send_to_char("You have been idle for 12 ticks Auto AFK is now on.\n\r",ch);
      send_to_char("To remove auto afk mode type in any command.\n\r", ch );
      SET_BIT(ch->comm2,COMM_AUTO_AFK);
      return;
    }

    else
    {
      send_to_char("Auto AFK removed. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm2,COMM_AUTO_AFK);
      return;
    }  
}     

char *PERS_CHAN(CHAR_DATA *ch, CHAR_DATA *looker)
{
    if(can_see(looker,ch))
	return ch->name;

    return "someone";

}

void info( CHAR_DATA * ch, int level, char * message, ...)
{
    char buf[MSL];
    DESCRIPTOR_DATA * d;
    va_list args;

    va_start(args,message);
    vsnprintf(buf, MAX_STRING_LENGTH, message, args);
    va_end(args);

    for ( d = descriptor_list; d; d = d->next )
    {
        CHAR_DATA *vch;
        vch = ( d->original != NULL ) ? d->original : d->character;         

	if( d && d->character != NULL )
	{
	    if ((d->connected == CON_PLAYING) &&
		(d->character != ch) &&
		(get_trust(d->character) >= level ) && !IS_SET( vch->comm, COMM_NOINFO ) )
	    {
		send_to_char(buf, d->character);
		send_to_char("\r\n",d->character);
	    }
	}
    }
}         

struct quote_type
{
    char * 	text;
};


const struct quote_type quote_table [MAX_QUOTES] =
{
    { "Fine be that way go ahead and leave."                  }, 
    { "Alas all good things must come to an end!"             },
    { "Just when I was about to kill you...poof...you leave!" },
    { "Since your leaving do you mind if I borrow your eq?"   },
    { "Did I say you could leave? I think not! Log back on"   },
    { "Don't follow that bright light!!! It's a Trick!!"      },
    { "Sure you'll be back... that's what they all say:P!"    },
    { "Pardon me, can I get your measurements for a coffin?"  },
    { "Me dragon...you food...stand still so I can eat you!"  },
    { "You can run but you can't hide! I'll find you!"        }
};


/*
 * The Routine
 */

void do_quotes( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    int quote = 0;

    buf[0]='\0';

    quote = number_range( 0, MAX_QUOTES-1);
                                                       
    if (quote_table[quote].text == NULL)
    {
        sprintf(buf,"DO_QUOTE: Null Quote %d",quote);
        log_string(buf);
        return;
    }

    sprintf ( buf, "\n\r{C%s{x\n\r",
              quote_table[quote].text );
    send_to_char ( buf, ch );
    return;
}               

void do_info( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
        if (IS_SET(ch->comm,COMM_NOINFO))
        {
            send_to_char("The info channel is now ON.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_NOINFO);
        }

        else
        {
            send_to_char("The info channel is now OFF.\n\r",ch);
            SET_BIT(ch->comm,COMM_NOINFO);
        }

        return;
    }
}                   
