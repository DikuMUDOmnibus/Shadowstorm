/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   
*                                                                         
*
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          
*  Chastain, Michael Quan, and Mitchell Tse.                              
*                                                                         
*
*  In order to use any part of this Merc Diku Mud, you must comply with   
*  both the original Diku license in 'license.doc' as well the Merc       
*  license in 'license.txt'.  In particular, you may not remove either of 
*  these copyright notices.                                               
*                                                                         
*
*  Much time and thought has gone into this software and you are          
*  benefitting.  We hope that you share your changes too.  What goes      
*  around, comes around.                                                  
*
***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1998 Russ Taylor                         
*
*       ROM has been brought to you by the ROM consortium                  
*
*           Russ Taylor (rtaylor@hypercube.org)                            
*
*           Gabrielle Taylor (gtaylor@hypercube.org)                       
*
*           Brian Moore (zump@rom.org)                                     
*
*       By using this code, you have agreed to follow the terms of the     
*
*       ROM license, in the file Rom24/doc/rom.license                     
*
***************************************************************************/

/**************************************************************************
***************************************************************************
***************************************************************************
***									
***
***	This file is for bard songs, and the music and play skill,	
***
***	written from scratch by me (Chaos) for the Realm of the lance	
***
***	MUD, copyright 1998.  It may be used as long as this header	
***
***	file remains intact, and my name is mentioned somewhere in the	
***
***	help file for music/play.  I would also appreciate an e-mail	
***
***	stating that you are using this and the address of your mud	
***
***	so I can come check it out!  Any comments or suggestions, or	
***
***	bugs may be e-mailed to chaos@lance.betterbox.net.  Enjoy!	
***
***									
***
***************************************************************************
***************************************************************************
**************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "include.h"

DECLARE_SONG_FUN(	song_of_huma		);
DECLARE_SONG_FUN(       song_of_soothing        );
DECLARE_SONG_FUN(       song_of_paralysis       );
DECLARE_SONG_FUN(       song_of_rage            );   

const   struct  song_type       bsong_table [MAX_SONG] =
{
/*  {
	"Song name", "Song list name", level, (void(*)(int, int, CHAR_DATA *))song_fun,
	min position, min mana, beats
    }
*/
    {
        "song of huma", "Song of Huma", 20, (void(*)(int, int, CHAR_DATA *))song_of_huma,
	POS_RESTING, 40, 25
    },

    {
        "song of soothing", "Song of Soothing", 48, (void(*)(int, int, CHAR_DATA *))song_of_soothing,
        POS_RESTING, 30, 5
    },

    {
        "song of paralysis", "Song of Paralysis", 50, (void(*)(int, int, CHAR_DATA *))song_of_paralysis,
	POS_RESTING, 50, 5
    },
       
    {
        "song of rage",      "Song of Rage", 40, (void (*)(int, int, CHAR_DATA *))song_of_rage, 
        POS_RESTING, 50, 5
    },
};

void do_songs(CHAR_DATA *ch, char *argument)
{
    char buf[MSL];
    int i;

    sprintf(buf, "{Y[%-18s %5s]{x\n\r", "Song Name", "Level");
    send_to_char(buf,ch);

    for (i = 0; i <= MAX_SONG -1; i++)
    {
	sprintf(buf, "{Y[{G%-20s {C%3d{Y]{x\n\r", bsong_table[i].listname, bsong_table[i].level);
	send_to_char(buf,ch);
    }
    return;
}


int song_lookup( const char *name )
{
    int songnum;

    for ( songnum = 0; songnum <= MAX_SKILL -1; songnum++ )
    {
	if ( bsong_table[songnum].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(bsong_table[songnum].name[0])
	&&   !str_cmp( name, bsong_table[songnum].name ) )
	    return songnum;
    }

    return -1;
}


void do_splay( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *instrument;
    int songnum, chance, mana, level;

    if ((chance = get_skill(ch,skill_lookup("music"))) == 0)
    {
	send_to_char("You have no clue how to play music.\n\r", ch);
	return;
    }

    if ((instrument = get_eq_char(ch, WEAR_HOLD)) == NULL)
    {
	send_to_char("You aren't carrying an instrument.\n\r", ch);
	return;
    }

    if (instrument->item_type != ITEM_INSTRUMENT)
    {
	send_to_char("You aren't carrying an instrument.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Play what?\n\r", ch);
	return;
    }

    songnum = song_lookup(argument);

    if (songnum == -1)
    {
	send_to_char("That isn't a song.\n\r", ch);
	return;
    }

    if (ch->level < bsong_table[songnum].level)
    {
	send_to_char("You haven't been able to master that song yet.\n\r", ch);
	return;
    }

    if (ch->position < bsong_table[songnum].minimum_position)
    {
	send_to_char("You need to be standing up to play that song.\n\r", ch);
	return;
    }

    mana = bsong_table[songnum].min_mana;

    if (!IS_NPC(ch) && ch->mana < mana)
    {
	send_to_char("You don't have enough mana.\n\r", ch);
	return;
    }

    act("$n plays a melody on $s $p.",ch,instrument,NULL,TO_ROOM);
    act("You play a melody on your $p.",ch,instrument,NULL,TO_CHAR);

    WAIT_STATE( ch, bsong_table[songnum].beats );

    chance = chance - (20 / (1 + ch->level - bsong_table[songnum].level));

	/* average of level of player and level of instrument */
    level = (ch->level + instrument->level) / 2;

    if (number_percent() > chance)
    {
	ch->mana -= mana / 2;
	act("$n's fingers slip and the song ends abruptly.",ch,NULL,NULL,TO_ROOM);
	send_to_char("Your fingers slip and the song ends abruptly.\n\r", ch);
	check_improve(ch,skill_lookup("music"),FALSE,1);
	return;
    }
    else /* actually start playing the song */
    {
	ch->mana -= mana;
	if (IS_NPC(ch) || has_spells( ch ) )
	    (*bsong_table[songnum].song_fun) ( songnum, level, ch );
	else
	    (*bsong_table[songnum].song_fun) ( songnum, 3 * level/4, ch );
	check_improve(ch,skill_lookup("music"),TRUE,1);
    }

    return;
}

void song_of_huma( int songnum, int level, CHAR_DATA *ch )
{
    CHAR_DATA *vch;
    AFFECT_DATA af1, af2;

    af1.where = TO_AFFECTS;
    af1.type = skill_lookup("bless");
    af1.level = level;
    af1.duration = level/2;
    af1.location = APPLY_HITROLL;
    af1.modifier = level/8;
    af1.bitvector = 0;

    af2.where = TO_AFFECTS;
    af2.type = skill_lookup("armor");
    af2.level = level;
    af2.duration = level/2;
    af2.location = APPLY_AC;
    af2.modifier = -20;
    af2.bitvector = 0;

    if (!is_affected( ch, af1.type ))
    {
	act("$n glows briefly.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You glow briefly.\n\r", ch);
	affect_to_char(ch,&af1);
    }

    if (!is_affected( ch, af2.type ))
    {
	act("$n is suddenly surrounded by a glowing suit of armor.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You are suddenly surrounded by a glowing suit of armor.\n\r", ch);
	affect_to_char(ch,&af2);
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if (ch == vch)
	    continue;

	if (!IS_NPC(vch))
	{
	    if (number_percent() <= 80 && !is_affected( vch, af1.type ))
	    {
		act("$N glows briefly.",ch,NULL,vch,TO_ROOM);
		act("$N glows briefly.",ch,NULL,vch,TO_CHAR);
		send_to_char("You glow briefly.\n\r", vch);
		affect_to_char( vch, &af1 );
	    }
	    if (number_percent() <= 80 && !is_affected( vch, af2.type ))
	    {
		act("$N is suddenly surrounded by a glowing suit of armor.",ch,NULL,vch,TO_ROOM);
		act("$N is suddenly surrounded by a glowing suit of armor.",ch,NULL,vch,TO_CHAR);
		send_to_char("You are suddenly surrounded by a glowing suit of armor.\n\r", vch);
		affect_to_char( vch, &af2 );
	    }
	}
    }

    return;
}

void song_of_soothing( int songnum, int level, CHAR_DATA *ch)
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;
    int chance;
    AFFECT_DATA af;
    int chance1;

    /* chances you put them to sleep */
    chance1 = number_range( 1,100 );             

    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->position == POS_FIGHTING)
        {
            count++;
            if (IS_NPC(vch))
              mlevel += vch->level;
            else
              mlevel += vch->level/2;
            high_level = UMAX(high_level,vch->level);
        }
    }       
                                                                   
    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL(ch)) /* always works */
      mlevel = 0;

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            if (IS_SET(vch->imm_flags,IMM_MAGIC) ||
                                IS_SET(vch->act,ACT_UNDEAD))
              return;
        
            if ( ch == vch )
                return;

            if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED( vch, AFF_SLEEP)  
            ||IS_AFFECTED(vch,AFF_BERSERK)
            ||  is_affected(vch,skill_lookup("frenzy")))
              return;

            printf_to_char( ch, "A soothing wave of calm washes over %s\n\r", vch->name );
            send_to_char("A soothing wave of calm washes over you.\n\r",vch);
            act("$n feels a soothing wave of calm wash over $m.",ch,NULL,NULL,TO_ROOM);

            if (vch->fighting || vch->position == POS_FIGHTING)
              stop_fighting(vch,FALSE);
         
            if ( IS_AWAKE(vch) && chance1 <= 25 ) 
            {
                send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", vch );
                act( "$n goes to sleep.", vch, NULL, NULL, TO_ROOM );
                vch->position = POS_SLEEPING;
            }  

            af.where = TO_AFFECTS;
            af.type = skill_lookup( "calm" );
            af.level = level;
            af.duration = level/4;
            af.location = APPLY_HITROLL;
            if (!IS_NPC(vch))
              af.modifier = -5;
            else
              af.modifier = -2;
            af.bitvector = AFF_CALM;
            affect_to_char(vch,&af);

            af.type = skill_lookup( "sleep" );
            af.location = APPLY_DAMROLL;
            af.modifier = -5;
            if ( chance1 <= 25 )
            {
            af.bitvector = AFF_SLEEP;
            affect_join( vch, &af );
            } 
        }
    }
}                                                                              
                                        
void song_of_paralysis( int songnum, int level, CHAR_DATA *ch)
{

     CHAR_DATA *vch;


     for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
     {
	if (IS_NPC(vch))
	{
	   if (vch->fighting || vch->position == POS_FIGHTING)
	      stop_fighting(vch,FALSE);
		      
           vch->position = POS_STUNNED;
           WAIT_STATE( vch, 2 * PULSE_VIOLENCE );
	   act("$n falls into a heap, stunned.", vch, NULL, NULL, TO_ROOM);
	 }
      }
}


void song_of_rage( int songnum, int level, CHAR_DATA *ch )
{
  AFFECT_DATA af1, af2;

  af1.where = TO_AFFECTS;
  af1.type = skill_lookup("bless");
  af1.level = level;
  af1.duration = level/2;
  af1.location = APPLY_HITROLL;
  af1.modifier = level/8;
  af1.bitvector = 0;

  af2.where = TO_AFFECTS;
  af2.type = skill_lookup("frenzy");
  af2.level = level;
  af2.duration = level/2;
  af2.location = APPLY_DAMROLL;
  af2.modifier = ch->level + 10/4;
  af2.bitvector = 0;

  if (is_affected(ch, af1.type))
  {
  act("$n glows briefly.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You glow briefly.\n\r",ch);
  affect_to_char(ch,&af1);
  }

  if (!is_affected(ch,af2.type))
  {
    act("$n is suddenly feels much stronger and battleworthy.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You feel stronger and more battle worthy.\n\r",ch);
    affect_to_char(ch,&af2);
  }

  return;
}


  


