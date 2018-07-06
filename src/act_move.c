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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "include.h"

char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down"
};

int find_path(ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to, int max_depth);
extern char * const dir_name[];

const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4
};

const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 2
};

char *  const   rev_move        []              =
{
    "the south", "the west", "the north", "the east", "below", "above"
};

/*
 * Local functions.
 */
int	find_door	args( ( CHAR_DATA *ch, char *arg ) );
bool	has_key		args( ( CHAR_DATA *ch, int key ) );
void    blood_trail     args( ( CHAR_DATA *ch, int direction ) );
void    aggr_update     args( ( void ) );

DECLARE_DO_FUN( do_look );


void move_char( CHAR_DATA *ch, int door, bool follow )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    char buf[MAX_STRING_LENGTH];

    if ( door < 0 || door > 5 )
    {
	bug( "Do_move: bad door %d.", door );
	return;
    }

    if ( !IS_NPC(ch)
      && (p_exit_trigger( ch, door, PRG_MPROG )
      ||  p_exit_trigger( ch, door, PRG_OPROG )
      ||  p_exit_trigger( ch, door, PRG_RPROG )) )
        return;    
    
    in_room = ch->in_room;
   
    if( !IS_NPC(ch) && in_room->exit[door] && in_room->exit[door]->to_shaft 
	&& !in_room->exit[door]->u1.to_room 
	&& !ch->pcdata->in_mine
	&& door != DIR_UP)
    {
        put_char_mine( ch, in_room->exit[door] );
	return;
    }

    if(!IS_NPC(ch) && IN_MINE(ch) && ch->pcdata->in_shaft == ch->pcdata->in_mine->shaft_first && ch->pcdata->in_shaft->exit[door] && ch->pcdata->in_shaft->exit[door]->u1.to_room )
    {   
        char_from_mine(ch);
	return;
    }

    if(!IS_NPC(ch) && IN_MINE(ch)  )
    {   
        move_char_mine(ch, door);
	return;
    }
 
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||	 !can_see_room(ch,pexit->u1.to_room))
    {
	send_to_char( "Alas, you cannot go that way.\n\r", ch );
        sound( "yocant.wav", ch );
	return;
    }

    if( pexit->u1.to_room != NULL && pexit->to_shaft != NULL )
    {    logf2("BUG: Move_char with both shaft and room. %s\n\r",in_room->name );
	 return;
    }

    weather_move( ch, to_room, pexit );

    if ((to_room->sector_type != SECT_INSIDE ) && (to_room->room_flags != ROOM_INDOORS) && !IS_NPC( ch ) )
    weather_check( ch );
 
    if ( IS_SET(to_room->room_flags, ROOM_RENTED))
    {
        if ( !IS_IMMORTAL(ch))
        {
            send_to_char( "You can't go in there.\n\r", ch );
            return;
        }         

    }

    if (IS_SET(pexit->exit_info, EX_CLOSED)
    &&  (!IS_AFFECTED(ch, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS))
    &&   !IS_TRUSTED(ch,ANGEL))
    {
	act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return;
    }

    if ( !is_room_owner(ch,to_room) && room_is_private( to_room ) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( MOUNTED(ch) )
    {
	if (MOUNTED(ch)->position < POS_FIGHTING)
        {
            send_to_char("Your mount must be standing.\n\r", ch);
            return; 
        }

	if (!mount_success(ch, MOUNTED(ch), FALSE))
        {
            send_to_char("Your mount stubbornly refuses to go that way.\n\r", ch);
            return;
        }
    }


    if ( !IS_NPC(ch) )
    {
	int move;

	/*for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    for ( iGuild = 0; iGuild < MAX_GUILD; iGuild ++)	
	    {
                if ( is_class( ch, iClass )        
	    	&&   to_room->vnum == class_table[iClass].guild[iGuild] ) 
	    	{
		    send_to_char( "You aren't allowed in there.\n\r", ch );
		    return;
		}
	    }
	}*/

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR )
	{
	    if ( MOUNTED(ch) )
	    {
    		if( !IS_AFFECTED(MOUNTED(ch), AFF_FLYING) )
	    	{
		        send_to_char( "Your mount can't fly.\n\r", ch );
                return;
            }
	    }
	    else
	        {
		    if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch))
		    {
		        send_to_char( "You can't fly.\n\r", ch );
		        return;
		    }
	    }
	}

	if ( (in_room->sector_type == SECT_WATER_NOSWIM
	    ||    to_room->sector_type == SECT_WATER_NOSWIM )
	    &&    (MOUNTED(ch) && IS_AFFECTED(MOUNTED(ch), AFF_FLYING) ))
        {
            sprintf( buf,"You can't take your mount there.\n\r");
            send_to_char(buf, ch);
            return;
        } 


	if (( in_room->sector_type == SECT_WATER_NOSWIM
	||    to_room->sector_type == SECT_WATER_NOSWIM )
  	&&    !IS_AFFECTED(ch,AFF_FLYING))
	{
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		send_to_char( "You need a boat to go there.\n\r", ch );
		return;
	    }
	}

	move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
	     ;

        move /= 2;  /* i.e. the average */


	if ( !MOUNTED(ch) )
	{
	    /* conditional effects */
	    if (IS_AFFECTED(ch,AFF_FLYING) || IS_AFFECTED(ch,AFF_HASTE))
		move /= 2;

	    if (IS_AFFECTED(ch,AFF_SLOW))
		move *= 2;

	    if ( ch->move < move )
	    {
		send_to_char( "You are too exhausted.\n\r", ch );
                sound( "exhaust.wav", ch );
		return;
	    }
	}
	else
	{
    if (IS_AFFECTED( MOUNTED(ch), AFF_FLYING)
    || IS_AFFECTED( MOUNTED(ch), AFF_HASTE))
                move /= 2;
            if (IS_AFFECTED( MOUNTED(ch), AFF_SLOW))
                move *= 2;

	    if ( MOUNTED(ch)->move < move )
	    {
		send_to_char( "Your mount is too exhausted.\n\r", ch );
		return;
            }
        }

	WAIT_STATE( ch, 1 );

	if (!MOUNTED(ch))
            ch->move -= move;
        else
            MOUNTED(ch)->move -= move;
    }

    if (RIDDEN(ch))
    {
	CHAR_DATA *rch;
	rch = RIDDEN(ch);

    	if (!mount_success(rch, ch, FALSE))
	    {
	        act( "Your mount escapes your control, and leaves $T.", rch, NULL, dir_name[door], TO_CHAR );
	        if (RIDDEN(ch))
		    ch = RIDDEN(ch);
	    }
	    else
	    {
	        send_to_char("You steady your mount.\n\r", rch);
	        return;
    	}

    }

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    &&   ch->invis_level < LEVEL_HERO)
    {

	if (MOUNTED(ch))
	{
	    if( !IS_AFFECTED(MOUNTED(ch), AFF_FLYING) )
		sprintf(buf, "$n leaves $T, riding on %s.", MOUNTED(ch)->short_descr);
            else
               sprintf(buf, "$n soars $T, on %s.", MOUNTED(ch)->short_descr);
            act( buf, ch, NULL, dir_name[door], TO_ROOM );
        }
	else
	{
            if ( is_affected( ch, skill_lookup("doorbash") ) )
                act("$n goes crashing through the door $T.",ch,0,dir_name[door],TO_ROOM);
            else
	    act( "$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM );

            if (ch->hit < (ch->max_hit / 2)) 
                blood_trail( ch, door );
        }
    }

    char_from_room( ch );
    char_to_room( ch, to_room );

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    &&   ch->invis_level < LEVEL_HERO)
    {
        if (is_affected(ch,skill_lookup("doorbash")))
        act("The $T door bursts open and $n comes crashing in!",ch,0,dir_name[door],TO_ROOM);
	if(!MOUNTED(ch))
	{
	    act( "$n has arrived.", ch, NULL, NULL, TO_ROOM );
	}
	else
	{
	    if ( !IS_AFFECTED(MOUNTED(ch), AFF_FLYING) )
		act( "$n has arrived, riding on $N.", ch, NULL, MOUNTED(ch), TO_ROOM );
	    else
		act( "$n soars in, riding on $N.", ch, NULL, MOUNTED(ch), TO_ROOM );
        }
    }

    if (MOUNTED(ch))
    {
        char_from_room( MOUNTED(ch) );
        char_to_room( MOUNTED(ch), to_room );
    }
    
    do_function(ch, &do_look, "auto" );

    if (in_room == to_room) /* no circular follows */
	return;

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;

	if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM) 
	&&   fch->position < POS_STANDING)
	    do_function(fch, &do_stand, "");

	if ( fch->master == ch && fch->position == POS_STANDING 
	&&   can_see_room(fch,to_room))
	{

	    if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
	    &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
	    {
		act("You can't bring $N into the city.",
		    ch,NULL,fch,TO_CHAR);
		act("You aren't allowed in the city.",
		    fch,NULL,NULL,TO_CHAR);
		continue;
	    }

	    act( "You follow $N.", fch, NULL, ch, TO_CHAR );
	    move_char( fch, door, TRUE );
	}
    }

    if ( IS_NPC( ch ) && HAS_TRIGGER_MOB( ch, TRIG_ENTRY ) )
        p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_ENTRY );  
    
    if ( !IS_NPC( ch ) )
    {
        p_greet_trigger( ch, PRG_MPROG );
        p_greet_trigger( ch, PRG_OPROG );
        p_greet_trigger( ch, PRG_RPROG );
    }   

}



void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTH, FALSE );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_EAST, FALSE );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTH, FALSE );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_WEST, FALSE );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_UP, FALSE );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_DOWN, FALSE );
    return;
}



int find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
		return door;
	}
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}



void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
 	/* open portal */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1], EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1], EX_CLOSED))
	    {
		send_to_char("It's already open.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1], EX_LOCKED))
	    {
		send_to_char("It's locked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1], EX_CLOSED);
	    act("You open $p.",ch,obj,NULL,TO_CHAR);
	    act("$n opens $p.",ch,obj,NULL,TO_ROOM);
	    return;
 	}

	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER)
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	act("You open $p.",ch,obj,NULL,TO_CHAR);
	act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
        sound( "dooropen.wav", ch );
	printf_to_char(ch,  "You open the %s.\n\r", pexit->keyword );

	/* open the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }



    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{

	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||   IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's already closed.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_CLOSED);
	    act("You close $p.",ch,obj,NULL,TO_CHAR);
	    act("$n closes $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }

	SET_BIT(obj->value[1], CONT_CLOSED);
	act("You close $p.",ch,obj,NULL,TO_CHAR);
	act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	printf_to_char(ch, "You close the %s.\n\r", pexit->keyword );
        sound( "doorclose.wav", ch );
	/* close the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }


    if (MOUNTED(ch))
    {
        send_to_char("You can't reach the lock from your mount.\n\r", ch);
        return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||  IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }
	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
	 	return;
	    }

	    if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK))
	    {
		send_to_char("It can't be locked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already locked.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_LOCKED);
	    act("You lock $p.",ch,obj,NULL,TO_CHAR);
	    act("$n locks $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	act("You lock $p.",ch,obj,NULL,TO_CHAR);
	act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if (MOUNTED(ch))
    {
        send_to_char("You can't reach the lock from your mount.\n\r", ch);
        return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
 	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already unlocked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("You unlock $p.",ch,obj,NULL,TO_CHAR);
	    act("$n unlocks $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	act("You unlock $p.",ch,obj,NULL,TO_CHAR);
	act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    if (MOUNTED(ch))
    {
        send_to_char("You can't pick locks while mounted.\n\r", ch);
        return;
    }

    WAIT_STATE( ch, skill_table[skill_lookup("pick lock")].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

    if ( !IS_NPC(ch) && number_percent( ) > get_skill(ch,skill_lookup("pick lock")))
    {
	send_to_char( "You failed.\n\r", ch);
	check_improve(ch,skill_lookup("pick lock"),FALSE,2);
	return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {	
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_PICKPROOF))
	    {
		send_to_char("You failed.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
	    check_improve(ch,skill_lookup("pick lock"),TRUE,2);
	    return;
	}

	    


	
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
        act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
        act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
	check_improve(ch,skill_lookup("pick lock"),TRUE,2);
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 && !IS_IMMORTAL(ch))
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	check_improve(ch,skill_lookup("pick lock"),TRUE,2);

	/* pick the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}




void do_stand( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (argument[0] != '\0')
    {
	if (ch->position == POS_FIGHTING)
	{
	    send_to_char("Maybe you should finish fighting first?\n\r",ch);
	    return;
	}
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	if (obj->item_type != ITEM_FURNITURE
	||  (!IS_SET(obj->value[2],STAND_AT)
	&&   !IS_SET(obj->value[2],STAND_ON)
	&&   !IS_SET(obj->value[2],STAND_IN)))
	{
	    send_to_char("You can't seem to find a place to stand.\n\r",ch);
	    return;
	}
	if (ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act_new("There's no room to stand on $p.",
		ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}
 	ch->on = obj;

        if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
         p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );      
    }
    
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }
	
	if (obj == NULL)
	{
	    send_to_char( "You wake and stand up.\n\r", ch );
	    act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	   act_new("You wake and stand at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	   act("$n wakes and stands at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act_new("You wake and stand on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    act("$n wakes and stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	else 
	{
	    act_new("You wake and stand in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    act("$n wakes and stands in $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_STANDING;
	do_function(ch, &do_look, "auto");
	break;

    case POS_RESTING: case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char( "You stand up.\n\r", ch );
	    act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	    act("You stand at $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act("You stand on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	else
	{
	    act("You stand in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (MOUNTED(ch))
    {
        send_to_char("You can't rest while mounted.\n\r", ch);
        return;
    }

    if (RIDDEN(ch))
    {
        send_to_char("You can't rest while being ridden.\n\r", ch);
        return;
    }


    if (ch->position == POS_FIGHTING)
    {
	send_to_char("You are already fighting!\n\r",ch);
	return;
    }

    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)
    {
        if (obj->item_type != ITEM_FURNITURE
    	||  (!IS_SET(obj->value[2],REST_ON)
    	&&   !IS_SET(obj->value[2],REST_IN)
    	&&   !IS_SET(obj->value[2],REST_AT)))
    	{
	    send_to_char("You can't rest on that.\n\r",ch);
	    return;
    	}

        if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
        {
	    act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
    	}
	
	ch->on = obj;
       
        if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
         p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	if (IS_AFFECTED(ch,AFF_SLEEP))
	{
	    send_to_char("You can't wake up!\n\r",ch);
	    return;
	}

	if (obj == NULL)
	{
	    send_to_char( "You wake up and start resting.\n\r", ch );
	    act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],REST_AT))
	{
	    act_new("You wake up and rest at $p.",
		    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
	    act("$n wakes up and rests at $p.",ch,obj,NULL,TO_ROOM);
	}
        else if (IS_SET(obj->value[2],REST_ON))
        {
            act_new("You wake up and rest on $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
            act_new("You wake up and rest in $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests in $p.",ch,obj,NULL,TO_ROOM);
        }
	ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	if (obj == NULL)
	{
	    send_to_char( "You rest.\n\r", ch );
	    act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM );
	}
        else if (IS_SET(obj->value[2],REST_AT))
        {
	    act("You sit down at $p and rest.",ch,obj,NULL,TO_CHAR);
	    act("$n sits down at $p and rests.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
	    act("You sit on $p and rest.",ch,obj,NULL,TO_CHAR);
	    act("$n sits on $p and rests.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
	    act("You rest in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
        }
	ch->position = POS_RESTING;
	break;

    case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char("You rest.\n\r",ch);
	    act("$n rests.",ch,NULL,NULL,TO_ROOM);
	}
        else if (IS_SET(obj->value[2],REST_AT))
        {
	    act("You rest at $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests at $p.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
	    act("You rest on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
	    act("You rest in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_RESTING;
	break;
    }


    return;
}


void do_sit (CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (MOUNTED(ch))
    {
        send_to_char("You can't sit while mounted.\n\r", ch);
        return;
    }

    if (RIDDEN(ch))
    {
        send_to_char("You can't sit while being ridden.\n\r", ch);
        return;
    }


    if (ch->position == POS_FIGHTING)
    {
	send_to_char("Maybe you should finish this fight first?\n\r",ch);
	return;
    }

    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)                                                              
    {
	if (obj->item_type != ITEM_FURNITURE
	||  (!IS_SET(obj->value[2],SIT_ON)
	&&   !IS_SET(obj->value[2],SIT_IN)
	&&   !IS_SET(obj->value[2],SIT_AT)))
	{
	    send_to_char("You can't sit on that.\n\r",ch);
	    return;
	}

	if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}

	ch->on = obj;
     
        if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
         p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
    }

    if (MOUNTED(ch))
    {
        send_to_char("You can't sleep while mounted.\n\r", ch);
        return;
    }

    if (RIDDEN(ch))
    {
        send_to_char("You can't sleep while being ridden.\n\r", ch);
        return;
    }
    
    
    switch (ch->position)
    {
	case POS_SLEEPING:
	    if (IS_AFFECTED(ch,AFF_SLEEP))
	    {
		send_to_char("You can't wake up!\n\r",ch);
		return;
	    }

            if (obj == NULL)
            {
            	send_to_char( "You wake and sit up.\n\r", ch );
            	act( "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM );
            }
            else if (IS_SET(obj->value[2],SIT_AT))
            {
            	act_new("You wake and sit at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SIT_ON))
            {
            	act_new("You wake and sit on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
            }
            else
            {
            	act_new("You wake and sit in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits in $p.",ch,obj,NULL,TO_ROOM);
            }

	    ch->position = POS_SITTING;
	    break;
	case POS_RESTING:
	    if (obj == NULL)
		send_to_char("You stop resting.\n\r",ch);
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("You sit at $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits at $p.",ch,obj,NULL,TO_ROOM);
	    }

	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("You sit on $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SITTING;
	    break;
	case POS_SITTING:
	    send_to_char("You are already sitting down.\n\r",ch);
	    break;
	case POS_STANDING:
	    if (obj == NULL)
    	    {
		send_to_char("You sit down.\n\r",ch);
    	        act("$n sits down on the ground.",ch,NULL,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("You sit down at $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits down at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("You sit on $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act("You sit down in $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits down in $p.",ch,obj,NULL,TO_ROOM);
	    }
    	    ch->position = POS_SITTING;
    	    break;
    }
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING: 
	if (argument[0] == '\0' && ch->on == NULL)
	{
	    send_to_char( "You go to sleep.\n\r", ch );
            sound( "sleep.wav", ch );
	    act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	    ch->position = POS_SLEEPING;
	}
	else  /* find an object and sleep on it */
	{
	    if (argument[0] == '\0')
		obj = ch->on;
	    else
	    	obj = get_obj_list( ch, argument,  ch->in_room->contents );

	    if (obj == NULL)
	    {
		send_to_char("You don't see that here.\n\r",ch);
		return;
	    }
	    if (obj->item_type != ITEM_FURNITURE
	    ||  (!IS_SET(obj->value[2],SLEEP_ON) 
	    &&   !IS_SET(obj->value[2],SLEEP_IN)
	    &&	 !IS_SET(obj->value[2],SLEEP_AT)))
	    {
		send_to_char("You can't sleep on that!\n\r",ch);
		return;
	    }

	    if (ch->on != obj && count_users(obj) >= obj->value[0])
	    {
		act_new("There is no room on $p for you.",
		    ch,obj,NULL,TO_CHAR,POS_DEAD);
		return;
	    }

	    ch->on = obj;

            if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
             p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );

	    if (IS_SET(obj->value[2],SLEEP_AT))
	    {
		act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR);
                sound( "sleep.wav", ch );
		act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SLEEP_ON))
	    {
	        act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR);
	        act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR);
                sound( "sleep.wav", ch );
		act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SLEEPING;
	}
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_function(ch, &do_stand, ""); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }

    act_new( "$n wakes you.", ch, NULL, victim, TO_VICT,POS_SLEEPING );
    do_function(ch, &do_stand, "");
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;


    if ( !IS_NPC(ch)
    &&  !can_use_skpell( ch, skill_lookup("sneak") ) )
    {
	send_to_char(
	    "You are too clumsy to sneak.\n\r", ch );
	return;
    }

    if (MOUNTED(ch))
    {
        send_to_char("You can't sneak while riding.\n\r", ch);
        return;
    }


    if (IS_AFFECTED(ch,AFF_SNEAK))
    {
   	affect_strip( ch, skill_lookup("sneak") );
        REMOVE_BIT( ch->affected_by, AFF_SNEAK);
        send_to_char( "You no longer move silently.\n\r", ch );
        return;
    }

    if ( number_percent( ) < get_skill(ch,skill_lookup("sneak")))
    {
	check_improve(ch,skill_lookup("sneak"),TRUE,3);
	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("sneak");
	af.level     = ch->level; 
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );
        send_to_char( "You begin to move silently.\n\r", ch );
	
    }
    else
    {
	check_improve(ch,skill_lookup("sneak"),FALSE,3);
	send_to_char( "You fail to move silently.\n\r", ch );
    }
    return;
}


void do_hide( CHAR_DATA *ch, char *argument )
{

    if (MOUNTED(ch))
    {
        send_to_char("You can't hide while riding.\n\r", ch);
        return;
    }

    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( number_percent( ) < get_skill(ch,skill_lookup("hide")))
    {
	SET_BIT(ch->affected_by, AFF_HIDE);
	check_improve(ch,skill_lookup("hide"),TRUE,3);
    }
    else
	check_improve(ch,skill_lookup("hide"),FALSE,3);

    return;
}



/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, skill_lookup("invis")			);
    affect_strip ( ch, skill_lookup("mass invis")			);
    affect_strip ( ch, skill_lookup("sneak")			);
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE	);
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_recall( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

    if (IS_NPC(ch) && !IS_SET(ch->act,ACT_PET) && !ch->mount )
    {
	send_to_char("Only players can recall.\n\r",ch);
	return;
    }

    if( IN_MINE(ch) )
    {	send_to_char("You can't recall from a mine!\n\r",ch);
	return;
    }  

    if (IS_SET(ch->in_room->room_flags, ROOM_ARENA))
    {
	send_to_char("You can't recall while in the arena!\n\r", ch);
	return;
    }

    act( "$n prays for transportation!", ch, 0, 0, TO_ROOM );

    if ( ( location = get_room_index( hometown_table[ch->hometown].recall )) == NULL )   
    {
	send_to_char( "You are completely lost.\n\r", ch );
	return;
    }

    if ( ch->in_room == location )
	return;

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_AFFECTED(ch, AFF_CURSE))
    {
        act( "ShadowStorm has forsaken you.\n\r", ch, NULL, 0, TO_CHAR );
	return;
    }

    if ( ( victim = ch->fighting ) != NULL )
    {
	int lose,skill;

	skill = get_skill(ch,skill_lookup("recall"));

	if ( number_percent() < 80 * skill / 100 )
	{
	    check_improve(ch,skill_lookup("recall"),FALSE,6);
	    WAIT_STATE( ch, 4 );
	    sprintf( buf, "You failed!.\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	lose = (ch->desc != NULL) ? 25 : 50;
	gain_exp( ch, 0 - lose );
	check_improve(ch,skill_lookup("recall"),TRUE,4);
	sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	send_to_char( buf, ch );
	stop_fighting( ch, TRUE );
	
    }

    ch->move *= .80;
    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    sound( "recall.wav", ch );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_function(ch, &do_look, "auto" );
    
    if (ch->pet != NULL)
    {
        char_from_room( ch->pet );
        char_to_room( ch->pet, ch->in_room );
        do_function(ch->pet, &do_look, "auto" );
        return;
    }              

    if (ch->mount != NULL)
    {
        char_from_room( ch->mount );
        char_to_room( ch->mount, ch->in_room );
        do_function(ch->mount, &do_look, "auto" );
        return;
    }   

    return;
}



void do_train( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    sh_int stat = - 1;
    char *pOutput = NULL;
    int cost, number;
    char arg[MSL];
    char arg2[MSL];

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( IS_NPC(ch) )
	return;

    /*
     * Check for trainer.
     */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0'  )
    {
	sprintf( buf, "You have %d training sessions.\n\r", ch->train );
	send_to_char( buf, ch );
     
        if (is_number( arg ) )
        {
           send_to_char("syntax: for the train command is train [number] type.\n\rThe number is optional\n\r", ch );
           return;
        }

        strcpy( arg2, arg );
    }

    number = is_number(arg) ? atoi(arg) : 1;

    if ( number < 2 )
    cost = 1;

    else if ( !str_cmp( arg2, "str" ) || !str_cmp( arg2, "wis" ) ||
    !str_cmp( arg2, "dex" ) || !str_cmp( arg2, "con" ) || !str_cmp( arg2, "int" ) ) 
    cost = number;

    if ( !str_cmp( arg2, "str" ) )
    {
	if ( is_prime_stat(ch, STAT_STR) )
	    cost    = 1;
	stat        = STAT_STR;
	pOutput     = "strength";
    }

    else if ( !str_cmp( arg2, "int" ) )
    {
	if ( is_prime_stat(ch, STAT_INT) )
	    cost    = 1;
	stat	    = STAT_INT;
	pOutput     = "intelligence";
    }

    else if ( !str_cmp( arg2, "wis" ) )
    {
	if ( is_prime_stat(ch, STAT_WIS) )
	    cost    = 1;
	stat	    = STAT_WIS;
	pOutput     = "wisdom";
    }

    else if ( !str_cmp( arg2, "dex" ) )
    {
	if ( is_prime_stat(ch, STAT_DEX) )
	    cost    = 1;
	stat  	    = STAT_DEX;
	pOutput     = "dexterity";
    }

    else if ( !str_cmp( arg2, "con" ) )
    {
	if ( is_prime_stat(ch, STAT_CON) )
	    cost    = 1;
	stat	    = STAT_CON;
	pOutput     = "constitution";
    }

    else if ( !str_cmp(arg2, "hp" ) )
    {
        if ( number < 2 )
        cost = 1;
        else
	cost = number;
    }

    else if ( !str_cmp(arg2, "mana" ) )
    {
        if ( number < 2 )
	cost = 1;
        else 
        cost = number;
    }

    else if ( !str_cmp(arg2, "move" ) )
    {
        if ( number < 2 )
        cost = 1;
        else
        cost = number;
    }
    
    else
    {
	strcpy( buf, "You can train:" );
	if ( ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
	    strcat( buf, " str" );
	if ( ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))  
	    strcat( buf, " int" );
	if ( ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
	    strcat( buf, " wis" );
	if ( ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))  
	    strcat( buf, " dex" );
	if ( ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))  
	    strcat( buf, " con" );
	strcat( buf, " hp mana move");

	if ( buf[strlen(buf)-1] != ':' )
	{
	    strcat( buf, ".\n\r" );
	    send_to_char( buf, ch );
	}
	else
	{
	    /*
	     * This message dedicated to Jordan ... you big stud!
	     */
	    act( "You have nothing left to train, you $T!",
		ch, NULL,
		ch->sex == SEX_MALE   ? "big stud" :
		ch->sex == SEX_FEMALE ? "hot babe" :
					"wild thing",
		TO_CHAR );
	}

	return;
    }

    if (!str_cmp("hp",arg2))
    {
    	if ( cost > ch->train )
    	{
       	    send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }

	ch->train -= cost;
        ch->pcdata->perm_hit += 10 * cost;
        ch->max_hit += 10 * cost;
        ch->hit += 10 * cost;
        if ( number < 2 )
        {
        act( "Your durability increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's durability increases!",ch,NULL,NULL,TO_ROOM);
        return;
        }
        else
        {
        printf_to_char( ch, "Your durability increases %d times!\n\r", cost );
        act( "$n's durability increases!\n\r", ch, NULL, NULL, TO_ROOM );
        return;
        }
    }
 
    if (!str_cmp("mana",arg2))
    {
        if ( cost > ch->train )
        {
            send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }

	ch->train -= cost;
        ch->pcdata->perm_mana += 10 * cost;
        ch->max_mana += 10 * cost;
        ch->mana += 10 * cost;
        if ( number < 2 )
        {
        act( "Your power increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's power increases!",ch,NULL,NULL,TO_ROOM);
        return;
        }
        else
        {
        printf_to_char( ch, "Your power increases %d times!", number );
        act( "$n's power increases!", ch, NULL, NULL, TO_ROOM ); 
        return;
        }
    }

    if ( !str_cmp( arg2, "move" ) )
    {
        if ( cost >= ch->train )
        {
            send_to_char( "You don't have enough training sessions!\n\r", ch );
            return;
        }

        ch->train -= cost;
        ch->pcdata->perm_move += cost * 10;
        ch->max_move += cost * 10;
        ch->move += cost * 10;
          
        if ( number < 2 )
        {
        act( "Your stamina increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's stamina increases!",ch,NULL,NULL,TO_ROOM);
        return;
        }
        else
        {
        printf_to_char( ch, "Your power increases %d times!", number );
        act( "$n's stamina increases!", ch, NULL, NULL, TO_ROOM );
        return;         
        }
    }

    if ( ch->perm_stat[stat]  >= get_max_train(ch,stat) )
    {
	act( "Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR );
	return;
    }

    if ( cost > ch->train )
    {
	send_to_char( "You don't have enough training sessions.\n\r", ch );
	return;
    }

    ch->train		-= cost;
  
    ch->perm_stat[stat]		+= 1;
    act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
    act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
    return;
}


void do_push_drag( CHAR_DATA *ch, char *argument, char *verb )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    OBJ_DATA *obj;
    int door;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    victim = get_char_room(ch, NULL, arg1);
    obj = get_obj_list( ch, arg1, ch->in_room->contents );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	sprintf( buf, "%s whom or what where?\n\r", capitalize(verb));
	send_to_char( buf, ch );
	return;
    }

    if ( (!victim || !can_see(ch,victim))
    && (!obj || !can_see_obj(ch,obj)) )
    {
	sprintf(buf,"%s whom or what where?\n\r", capitalize(verb));
	send_to_char( buf, ch );
        return;
    }

         if ( !str_cmp( arg2, "n" ) || !str_cmp( arg2, "north" ) ) door = 0;
    else if ( !str_cmp( arg2, "e" ) || !str_cmp( arg2, "east"  ) ) door = 1;
    else if ( !str_cmp( arg2, "s" ) || !str_cmp( arg2, "south" ) ) door = 2;
    else if ( !str_cmp( arg2, "w" ) || !str_cmp( arg2, "west"  ) ) door = 3;
    else if ( !str_cmp( arg2, "u" ) || !str_cmp( arg2, "up"    ) ) door = 4;
    else if ( !str_cmp( arg2, "d" ) || !str_cmp( arg2, "down"  ) ) door = 5;
    else
    {
      sprintf( buf, "Alas, you cannot %s in that direction.\n\r", verb );
      send_to_char( buf, ch );
      return;
    }

  if ( obj )
  {
    in_room = obj->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||	 !can_see_room(ch,pexit->u1.to_room))
    {
        sprintf( buf, "Alas, you cannot %s in that direction.\n\r", verb );
        send_to_char( buf, ch );
	return;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    || IS_SET(pexit->exit_info,EX_NOPASS) )
    {
	act( "You cannot $t it through the $d.", ch, verb, pexit->keyword, TO_CHAR );
	act( "$n decides to $t $P around!", ch, verb, obj, TO_ROOM );
	return;
    }

    act( "You attempt to $T $p out of the room.", ch, obj, verb, TO_CHAR );
    act( "$n is attempting to $T $p out of the room.", ch, obj, verb, TO_ROOM );

    if ( obj->weight >  (2 * can_carry_w (ch)) )
    {
      act( "$p is too heavy to $T.\n\r", ch, obj, verb, TO_CHAR);
      act( "$n attempts to $T $p, but it is too heavy.\n\r", ch, obj, verb, TO_ROOM);
      return;
    }
    if 	 ( !IS_IMMORTAL(ch)
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) )
    {
	send_to_char( "It won't budge.\n\r", ch );
	return;
    }

    if ( ch->move > 10 )
    {
	ch->move -= 10;
	send_to_char( "You succeed!\n\r", ch );
	act( "$n succeeds!", ch, NULL, NULL, TO_ROOM );
	if (!str_cmp( verb, "drag" ))
	{
	act( "$n drags $p $T!", ch, obj, dir_name[door], TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, pexit->u1.to_room );
	do_look( ch, "auto" );
	obj_from_room( obj );
	obj->carried_by = ch;
	obj_to_room( obj, to_room );
	act( "$n drags $p into the room.", ch, obj, dir_name[door], TO_ROOM );
	}
	else if (!str_cmp( verb, "push" ))
	{
	act( "$p {Wflies{x $T!", ch, obj, dir_name[door], TO_ROOM );
	act( "$p {Wflies{x $T!", ch, obj, dir_name[door], TO_CHAR );
	char_from_room( ch );
	char_to_room( ch, pexit->u1.to_room );
	act( "You notice movement from nearby to the $T.",
		ch, NULL, rev_move[door], TO_ROOM );
	act( "$p {Wflies{x into the room!", ch, obj, dir_name[door], TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, in_room );
	obj_from_room( obj );
	obj->carried_by = ch;
	obj_to_room( obj, to_room );
	}
    }
    else
    {
      sprintf( buf, "You are too tired to %s anything around!\n\r", verb );
      send_to_char( buf, ch );
    }
  }
  else
  {
    if ( ch == victim )
    {
	act( "You $t yourself about the room and look very silly.", ch, verb, NULL, TO_CHAR );
	act( "$n decides to be silly and $t $mself about the room.", ch, verb, NULL, TO_ROOM );
	return;
    }

    in_room = victim->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||	 !can_see_room(victim,pexit->u1.to_room))
    {
        sprintf( buf, "Alas, you cannot %s them that way.\n\r", verb );
        send_to_char( buf, ch );
	return;
    }

    if (IS_SET(pexit->exit_info, EX_CLOSED)
    &&  (!IS_AFFECTED(victim, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS)))
    {
	act( "You try to $t them through the $d.", ch, verb, pexit->keyword, TO_CHAR );
	act( "$n decides to $t you around!", ch, verb, victim, TO_VICT );
	act( "$n decides to $t $N around!", ch, verb, victim, TO_NOTVICT );
	return;
    }

    act( "You attempt to $t $N out of the room.", ch, verb, victim, TO_CHAR );
    act( "$n is attempting to $t you out of the room!", ch, verb, victim, TO_VICT );
    act( "$n is attempting to $t $N out of the room.", ch, verb, victim, TO_NOTVICT );

    if 	 ( !IS_IMMORTAL(ch)
    ||   (IS_NPC(victim)
    &&	 (IS_SET(victim->act,ACT_TRAIN)
    ||	 IS_SET(victim->act,ACT_PRACTICE)
    ||	 IS_SET(victim->act,ACT_IS_HEALER)
    ||	 IS_SET(victim->act,ACT_IS_CHANGER)
    ||	 IS_SET(victim->imm_flags,IMM_SUMMON)
    ||	 victim->pIndexData->pShop ))
    ||   victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   (!str_cmp( verb, "push" ) && victim->position != POS_STANDING)
    || 	 is_safe(ch,victim)
    || 	 (number_percent() > 90)
    ||   (victim->max_hit > (ch->max_hit + (get_curr_stat(ch,STAT_STR)*20))) )
    {
	send_to_char( "They won't budge.\n\r", ch );
	return;
    }

    if ( ch->move > 10 )
    {
	ch->move -= 10;
	send_to_char( "You succeed!\n\r", ch );
	act( "$n succeeds!", ch, NULL, NULL, TO_ROOM );
	if (!str_cmp( verb, "drag" ))
	{
	move_char( ch, door, FALSE );
	act( "$n is dragged $T!", victim, NULL, dir_name[door], TO_ROOM );
	act( "You are dragged $T!\n\r", victim, NULL, dir_name[door], TO_CHAR );
	char_from_room( victim );
	char_to_room( victim, pexit->u1.to_room );
	do_look( victim, "auto" );
	act( "$N drags $n into the room.", victim, NULL, ch, TO_NOTVICT );
	}
	else if (!str_cmp( verb, "push" ))
	{
	act( "$n {Wflies{x $T!", victim, NULL, dir_name[door], TO_ROOM );
	act( "You {Wfly{x $T!\n\r", victim, NULL, dir_name[door], TO_CHAR );
	char_from_room( victim );
	char_to_room( victim, pexit->u1.to_room );
	do_look( victim, "auto" );
	act( "You notice movement from nearby to the $T.",
		victim, NULL, rev_move[door], TO_ROOM );
	act( "$n {Wflies{x into the room!", victim, NULL, NULL, TO_ROOM );
	}
    }
    else
    {
      sprintf( buf, "You are too tired to %s anybody around!\n\r", verb );
      send_to_char( buf, ch );
    }
  }

  return;
}
               
void do_push( CHAR_DATA *ch, char *argument )
{
    do_push_drag( ch, argument, "push" );
    return;
}

void do_drag( CHAR_DATA *ch, char *argument )
{
    do_push_drag( ch, argument, "drag" );
    return;
}

void do_rental( CHAR_DATA *ch, char *argument )
{
    int cost = 10;

    if ( IS_SET(ch->in_room->room_flags, ROOM_RENTAL) )
    {

     if ( IS_NPC(ch))
       return;
    
     if ( IS_SET(ch->act, PLR_TENNANT))
     {
       send_to_char("You haven't checked out from last time, yet!\n\r", ch);
       return;
     }
   
     if ( ch->gold < cost )
     {
         send_to_char("You need at least 10 gold to rent a room!\n\r", ch );
         return;
     }

     send_to_char("You rent the room.\n\r", ch);
     REMOVE_BIT(ch->in_room->room_flags, ROOM_RENTAL);
     SET_BIT(ch->in_room->room_flags, ROOM_PRIVATE);
     SET_BIT(ch->in_room->room_flags, ROOM_RENTED);
     SET_BIT(ch->act, PLR_TENNANT);
     ch->gold -= cost;
     return;

    }

    else
    {
         send_to_char("You can't rent this room.\n\r", ch);
         return;
    }
}

void do_checkout( CHAR_DATA *ch, char *argument )
{
    if ( IS_SET(ch->in_room->room_flags, ROOM_RENTED) )
    {
	
      if ( IS_NPC(ch))
        return;
    
      if ( IS_SET(ch->act, PLR_TENNANT) && ch->pcdata->rentroom == ch->in_room->vnum)
      {
       send_to_char("You check out of the room.\n\r", ch);
       REMOVE_BIT(ch->act, PLR_TENNANT);
       REMOVE_BIT(ch->in_room->room_flags, ROOM_RENTED);
       SET_BIT(ch->in_room->room_flags, ROOM_RENTAL);
       ch->pcdata->rentroom = 0;
       return;
      }
      else
      {
       send_to_char("This is not your room.\n\r", ch);
       return;
      }
    }
}

int find_path(ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to, int max_depth)
{
  int bitvector[PATH_MAX_VNUM/BITS_PER_INT];
  ROOM_INDEX_DATA *rlist;
  ROOM_INDEX_DATA *track_room_list;
  int i, depth;

  bzero(bitvector, sizeof(bitvector));
  PATH_SET_FLAG(bitvector, from->vnum);
  track_room_list = from;
  track_room_list->next_track = NULL;
  for (depth = 0; depth < max_depth; depth++)
  {
    rlist = track_room_list;
    track_room_list = NULL;
    for (; rlist; rlist = rlist->next_track)
    {
      for (i = 0; i < PATH_MAX_DIR; i++)
      {
        if (!rlist->exit[i] || !rlist->exit[i]->u1.to_room ||
          PATH_IS_FLAG(bitvector, rlist->exit[i]->u1.to_room->vnum))
          continue;
        PATH_SET_FLAG(bitvector, rlist->exit[i]->u1.to_room->vnum);
        rlist->exit[i]->u1.to_room->track_came_from = rlist;
        if (rlist->exit[i]->u1.to_room == to)
        {
          if (rlist == from)
            return i;
//  if you need access to the entire path, this is the place to get it.
//  basically it's back-tracking how it got to the destination.
//  Also a good place to hinder track based on sector, weather, etc.
          while (rlist->track_came_from != from)
            rlist = rlist->track_came_from;
          for (i = 0; i < PATH_MAX_DIR; i++)
            if (from->exit[i] && from->exit[i]->u1.to_room == rlist)
               return i;
          return -1;
        }
        else
        {
          rlist->exit[i]->u1.to_room->next_track = track_room_list;
          track_room_list = rlist->exit[i]->u1.to_room;
        }
      }
    }
  }
  return -1;
}


void do_track( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int direction;
  bool fArea;
  int skill = 100;

  skill /= 4;
  skill += ch->level/5;
  
  one_argument( argument, arg );

  if (IS_NPC(ch)
  ||  !can_use_skpell( ch, skill_lookup("hunt")) )
  {
	send_to_char("Who do you want to track?\n\r",ch);
	return;
  }

  if( arg[0] == '\0' )
    {
      send_to_char( "Whom are you trying to hunt?\n\r", ch );
      return;
    }

  /* only imps can hunt to different areas */
  fArea = ( get_trust(ch) < MAX_LEVEL );

  if( fArea )
    victim = get_char_area( ch, arg );
  else
    victim = get_char_world( ch, arg );

  if( victim == NULL )
    {
      send_to_char("No-one around by that name.\n\r", ch );
      return;
    }

  if( ch->in_room == victim->in_room )
  {
      act( "$N is here!", ch, NULL, victim, TO_CHAR );
      return;
  }

  /*
   * Deduct some movement.
   */
  if( ch->move > 2 )
    ch->move -= 3;
  else
  {
      send_to_char( "You're too exhausted to hunt anyone!\n\r", ch );
      return;
  }

  act( "$n carefully sniffs the air.", ch, NULL, NULL, TO_ROOM );
  WAIT_STATE( ch, skill_table[skill_lookup("hunt")].beats );

  direction = find_path(ch->in_room, victim->in_room, skill);

  if( direction == -1 )
  {
      act( "You couldn't find a path to $N from here.",
	  ch, NULL, victim, TO_CHAR );
      return;
  }

  if( direction < 0 || direction > 5 )
  {
      send_to_char( "Hmm... Something seems to be wrong.\n\r", ch );
      return;
  }

  /*
   * Give a random direction if the player misses the die roll.
   */
  if( ( IS_NPC (ch) && number_percent () > 75)        /* NPC @ 25% */
     || (!IS_NPC (ch) && number_percent () >          /* PC @ norm */
	 ch->pcdata->learned[skill_lookup("hunt")] ) )
    {
      do
	{
	  direction = number_door();
	}
      while( ( ch->in_room->exit[direction] == NULL )
	    || ( ch->in_room->exit[direction]->u1.to_room == NULL) );
    }

  /*
   * Display the results of the search.
   */
  sprintf( buf, "$N is %s from here.", dir_name[direction] );
  act( buf, ch, NULL, victim, TO_CHAR );
  check_improve(ch,skill_lookup("hunt"),TRUE,1);
  return;
}

void blood_trail ( CHAR_DATA *ch, int direction )
{
        OBJ_DATA *trail;
        OBJ_DATA *obj;
        char buf[MAX_STRING_LENGTH];
	if(!ch || !ch->in_room)
 	  return;

        trail = create_object(get_obj_index(OBJ_VNUM_BLOOD_TRAIL), 0);
        trail->timer = number_range( 8, 15 );
        obj = get_obj_list(ch, "btrail", ch->in_room->contents);

        if ( obj )
                obj_from_room( obj );

        sprintf(buf, trail->short_descr, dir_name[direction]);
        free_string(trail->short_descr);
        trail->short_descr = str_dup(buf);

        sprintf(buf, trail->description, dir_name[direction]);
        free_string(trail->description);
        trail->description = str_dup(buf);
	trail->carried_by = ch;
        obj_to_room(trail, ch->in_room);
        return;
}

void do_wpoint( CHAR_DATA *ch, char *argument )  
{
   char buf[MSL];
   char arg[MSL];
   int  a;
   ROOM_INDEX_DATA *location;

   if( ch->in_room == NULL ) 
   {
      return;
   }

   if ( IS_NPC( ch ) )
   {
       send_to_char( "Mobs don't need way points!\n\r", ch );
       return;
   }

   if( IN_MINE(ch) )
   {	send_to_char("You can't set a wpoint in a mine!\n\r",ch);
	return;
   }

   argument = one_argument( argument, arg );

   if (!IS_SET(ch->in_room->room_flags,ROOM_WAYPOINT) )
   {
       send_to_char("You are not at a waypoint!\n\r", ch );
       return;
   }

   if( arg[0] == '\0' ) 
   {
      send_to_char( "         {CLocation name{x\n\r" , ch );
      send_to_char( "  {G==========================={x\n\r" , ch );

      for( a = 0; a < MAX_WAYPOINT; a++ ) 
      {
         location = get_room_index( ch->wpoint[a] );

         sprintf( buf, " %2d:  %s\n\r", a,
            ( location == NULL ? "(No way point set yet)" : location->name ) );
         send_to_char( buf, ch );
         location = NULL;
      }
      return;
   }

   if( !str_cmp( "set", arg ) ) 
   {
      argument = one_argument( argument, arg );

      if( arg[0] == '\0' ) 
      {
         send_to_char("Syntx: wpoint set (number)\n\r", ch );
         return;
      }

      a = atoi(arg);

      if( a < 0 || a >= MAX_WAYPOINT ) 
      {
         printf_to_char( ch, "Please choose a number between 1 and %d.\n\r", MAX_WAYPOINT );
         return;
      }

      else 
      {
         ch->wpoint[a] = ch->in_room->vnum;
         send_to_char("You memorize your current location.\n\r", ch );
         return;
      }
   }

   if( !str_cmp( "recall", arg ) ) 
   {
      argument = one_argument( argument, arg );

      if( arg[0] == '\0' ) 
      {
         send_to_char("To visit another waypoint type wpoint recall (number).\n\r", ch );
         return;
      }

      a = atoi( arg );

      if( a < 0 || a >= MAX_WAYPOINT ) 
      {
         printf_to_char(ch, "Please choose a number between 1 and %d.\n\r", MAX_WAYPOINT );
         return;
      }

      else 
      {
         location = get_room_index( ch->wpoint[a] );

         if ( ch->in_room == location )
         {
             send_to_char( "You are already there!\n\r", ch );
             return;        
         }

         if( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) ) 
         {
            send_to_char("The room glows for a moment, then fades.\n\r", ch);
            return;
         }

         if( location == NULL ) 
         {
            send_to_char("You remember no such location.\n\r", ch );
            ch->wpoint[a] = 0;
            return;
         }
            
         if (!IS_SET(ch->in_room->room_flags,ROOM_WAYPOINT) )
         {
             send_to_char("You are not at a waypoint!\n\r", ch );
             return;
         }
     
         act("$n disappears in a blinding flash of light!",ch,NULL,NULL,TO_ROOM);
         char_from_room( ch );
         char_to_room( ch, location );
         act("$n appears suddenly from a puff of smoke!",ch,NULL,NULL,TO_ROOM);
         do_function( ch, &do_look, "auto" );
         return;

         if (ch->pet != NULL)
         {
             char_from_room( ch->pet );
             char_to_room( ch->pet, ch->in_room );
             do_function(ch->pet, &do_look, "auto" );
             return;
         }

         if (ch->mount != NULL)
         {
             char_from_room( ch->mount );
             char_to_room( ch->mount, ch->in_room );
             do_function(ch->mount, &do_look, "auto" );
             return;
         }            

      }
   }

   return;
} 

void do_familiar (CHAR_DATA *ch, char *argument)
{
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *mount;
    int i, chance;

   if ((chance = get_skill(ch,skill_lookup("familiar"))) == 0
   ||  (!IS_NPC(ch)
   &&   !can_use_skpell(ch, skill_lookup("familiar"))))    
   {
      send_to_char("You can't summon a familiar you don't know how.\n\r",ch);
      return;
   }

   if ( ch->pet != NULL )
   {
       send_to_char("You already have a pet.\n\r",ch);
       return;
   }

   if(ch->position == POS_FIGHTING)
   {
      send_to_char("You can't summon a familiar while your fighting!\n\r",ch);
      return;
   }
    
   if ( ( pMobIndex = get_mob_index(MOB_VNUM_FAMILIAR) ) == NULL )
   {
       send_to_char( "The familiar mob doesn't exist.\n\r", ch );
       return;
   }

   if(ch->in_room->sector_type == SECT_INSIDE
   || ch->in_room->sector_type == SECT_WATER_SWIM 
   || ch->in_room->sector_type == SECT_WATER_NOSWIM
   || ch->in_room->sector_type == SECT_AIR )
   {
      send_to_char("You are feeling too sick to concentrate.\n\r",ch);
      return;
   }
     	
   mount = create_mobile( pMobIndex );
    
   mount->level = number_fuzzy(ch->level / 2);
   mount->mana = mount->max_mana = number_fuzzy(ch->max_mana / 2);
   mount->hit = mount->max_hit = number_fuzzy(ch->max_hit / 2);
   mount->move = mount->max_move = number_fuzzy(ch->max_move / 2); 
   for(i = 0; i < 4; i++)
       mount->armor[i] = number_fuzzy(ch->armor[i] - 20);
   mount->hitroll = number_fuzzy(ch->level / 10);
   mount->damroll = number_fuzzy(ch->level / 10);
    
    /* free up the old mob names */ 
   free_string(mount->description);
   free_string(mount->name);
   free_string(mount->short_descr);
   free_string(mount->long_descr);
    
    /* terrain */
   switch(ch->in_room->sector_type)
   {
	case(SECT_CITY): /* rat */
	case(SECT_FIELD):
            mount->description = str_dup( "You see a large furry rat." );
	    mount->short_descr = str_dup("large rat");
	    mount->long_descr = str_dup("A large rodent is here looking for something to eat.");
	    mount->name = str_dup("familiar rat");
	    mount->dam_type = 22; /* scratch */
            
	    break;
	case(SECT_FOREST):  /* falcon */
	case(SECT_HILLS):
	    mount->description = str_dup("You see a large falcon.");
	    mount->short_descr = str_dup("large falcon");
	    mount->long_descr = str_dup("A large falcon screams here.\n\r");
	    mount->name = str_dup("familiar falcon");
	    mount->dam_type = 5; /* claw */
	    break;
	case(SECT_MOUNTAIN): /* mountain lion */
	    mount->description = 
	          str_dup("You see a very large mountain lion.  One wrong look and it could\n\r"
	                  "have your head lying at your feet.  You should think better than\n\r"
	                  "cross this beast especial if you have a weapon in your hand.\n\r");
	    mount->short_descr = str_dup("large mountain lion");
	    mount->long_descr = str_dup("A large mountain lion claws the ground here.\n\r");
	    mount->name = str_dup("familiar mountain lion");
	    mount->dam_type = 10; /* bite */
	    break;
	case(SECT_DESERT): /* sandworm */
	    mount->description = 
	          str_dup("You see a large white sandworm wiggling in the light.\n\r"
	                  "A red spot on one end makes you guess it is a mouth.\n\r"
	                  "A loud moan comes from the direction of that red spot.\n\r");
	    mount->short_descr = str_dup("sandworm");
	    mount->long_descr = str_dup("A white sandworm wiggles on the ground here.\n\r");
	    mount->name = str_dup("familiar sandworm");
	    mount->dam_type = 12; /* suction */
	    break;
    }

    do_sit( ch, "" );
    char_to_room( mount, ch->in_room );
    act( "You begin your summoning spell and a $N appears before you!.",ch,NULL,mount,TO_CHAR);
    act( "$n begins to chant and calls to a $N!", ch, NULL, mount, TO_ROOM );
    WAIT_STATE(ch, 2 * PULSE_MOBILE);
    add_follower( mount, ch );
    mount->leader = ch;
    ch->pet = mount;
    do_stand(ch,"");

    SET_BIT(mount->act, ACT_PET);
    SET_BIT(mount->affected_by, AFF_CHARM);
    ch->move -= (mount->level / 2);  
    check_improve(ch,skill_lookup("familiar"),TRUE,6);
    return;
}

void do_doorbash(CHAR_DATA *ch,char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    int door;
    int chance;
    AFFECT_DATA af;

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        send_to_char("Bash door which door?\n\r",ch);
        return;
    }

    chance = get_skill(ch,skill_lookup("doorbash"));
    if (chance == 0
    || !can_use_skpell( ch, skill_lookup("doorbash") ) )
    {
        send_to_char("You'd hurt yourself doing that.\n\r",ch);
        return;
    }

    if (ch->move < 5)
    {
        send_to_char("You are too exhausted.\n\r",ch);
	return;
    }

    if ( (door = find_door(ch,arg) ) >= 0)
    {
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev;
        pexit = ch->in_room->exit[door];

    if (!IS_SET(pexit->exit_info,EX_CLOSED))
    {
        send_to_char("It's not closed.\n\r",ch);
        return;
    }

    if (!IS_SET(pexit->exit_info,EX_LOCKED))
    {
        send_to_char("It's already unlocked, why not just use the knob?\n\r",ch);
        return;
    }

    chance /= 4;
    chance += ch->level/5;
    chance += get_curr_stat(ch,STAT_STR)/2;

    if (!str_cmp(pc_race_table[ch->race].name,"giant"))
	chance += 15;

    do_visible(ch,"");

    if (number_percent() > chance || IS_SET(pexit->exit_info,EX_NOBASH) )
    {
    act("$n flies into the $T door and rebounds with a great lack of dignity!",ch,0,dir_name[door],TO_ROOM);
    act("You fly into the door $T but simply bounce off it like a lump of rock!",ch,0,dir_name[door],TO_CHAR);
    damage(ch,ch,dice(3,5),skill_lookup("doorbash"),DAM_BASH,TRUE);
    damage(ch,ch,dice(3,5),skill_lookup("doorbash"),DAM_BASH,TRUE);
    check_improve(ch,skill_lookup("doorbash"),FALSE,1);
    WAIT_STATE(ch,12);
    return;
    }

    act("$n slams into the $T door and throws it open with a mighty crash!",ch,0,dir_name[door],TO_ROOM);
    act("You slam into the $T door and it cracks open with a deafening sound!",ch,0,dir_name[door],TO_CHAR);
    check_improve(ch,skill_lookup("doorbash"),TRUE,1);
    WAIT_STATE(ch,12);
    REMOVE_BIT(pexit->exit_info,EX_LOCKED);
    REMOVE_BIT(pexit->exit_info,EX_CLOSED);

    if ( ( (to_room = pexit->u1.to_room) != NULL)
    && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
    && pexit_rev->u1.to_room == ch->in_room)
    {
        REMOVE_BIT(pexit_rev->exit_info,EX_LOCKED);
        REMOVE_BIT(pexit_rev->exit_info,EX_CLOSED);
    }

    if (number_percent() < get_curr_stat(ch,STAT_DEX))
        return;

    af.where = TO_AFFECTS;
    af.type = skill_lookup("doorbash");
    af.location = 0;
    af.modifier = 0;
    af.duration = -1;
    af.bitvector = 0;
    af.level = ch->level;
    affect_to_char(ch,&af);

    move_char(ch,door,FALSE);
    affect_strip(ch,skill_lookup("doorbash"));
    }

    return;

}

void do_sstep( CHAR_DATA *ch, char *argument )
{
    char buf[MIL];
    char buf2[MIL];
    char arg[MIL];
    ROOM_INDEX_DATA *in_room, *start_room;
    AFFECT_DATA af;
    int chance;
    int count;
    char *c, last_c, *num, *p;
    int door = -1, outtop = 0;
    bool dFound = FALSE;
   
    chance = get_skill(ch,skill_lookup("shadowstep"));

    if (chance == 0
    || !can_use_skpell( ch, skill_lookup("shadowstep") ) )
    {
        send_to_char("You'd hurt yourself doing that.\n\r",ch);
        return;
    }
              
    if( argument[0] == '\0' )
    {
       send_to_char("Shadowstep where?\n\r",ch);
       return;
    }
	
    if (!ch->desc || *argument == '\0')
    {
	send_to_char("You run in place!\n\r",ch);
	return;
    }
	
    buf[0] = '\0';
	
    while( *argument != '\0')
    {
 	  argument = one_argument(argument,arg);
	  strcat(buf,arg);
    }
	
    for( p = buf + strlen(buf)-1; p >= buf; p--)
    {
	if (!isdigit(*p))
	{
	    switch( *p )
	    {
	 	   case 'n':
		   case 's':
		   case 'e':
		   case 'w':
		   case 'u':
		   case 'd': 
		     dFound = TRUE;
                     break;
			
                  case 'o': 
		     break;
		     default: send_to_char("Invalid direction!\n\r",ch);
		     return;
	    }
		
        }
		
        else if (!dFound) 
            *p = '\0';
	
    }
	
    if( !dFound )
    {
       send_to_char("No directions specified!\n\r",ch); 
       return;
    }

    send_to_char("You step into the {Dshadows{x.\n\r\n\r",ch); 
    act("$n steps into the {Dshadows{x.",ch,NULL,NULL,TO_ROOM);
    check_improve(ch,skill_lookup("shadowstep"),TRUE,2);

    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("shadowstep");
    af.level     = ch->level;
    af.duration  = 0;
    af.location  = APPLY_NONE;
    af.modifier  = 0;

    if( !IS_AFFECTED(ch, AFF_PASS_DOOR)) 
    {
       af.bitvector = AFF_PASS_DOOR;
       affect_to_char(ch, &af);
    }

    if( !IS_AFFECTED(ch, AFF_SNEAK)) 
    {
       af.bitvector = AFF_SNEAK;
       affect_to_char(ch, &af);
    }

    if( !IS_AFFECTED(ch, AFF_FLYING)) 
    {
       af.bitvector = AFF_FLYING;
       affect_to_char(ch, &af);
    }

    if( ch->desc )
       outtop = ch->desc->outtop;

    start_room = ch->in_room;
    last_c = '\0';
    count = 1;
    last_c = *buf;
	
    for( c = buf; *c != '\0'; ++c )
    {
        count = 1;
	
	if( isdigit(*c))
	{
	   char nbuf[100];
	   num = nbuf;
	   *num = *c;
		
           while( isdigit(*c)) 
           {
		 *num = *c;
		 *(num+1) = '\0';
		 ++c;
		 ++num;
	   }
			
           count = atoi(nbuf);
	}
		
        while( *c != '\0' && count )
	{
	      door = -1;
	
	      if( *c == 'o' )
	      {
		 ++c;
		 sprintf(buf2, "open ");
			
                 switch(*c)
		 {
			case 'n' :	strcat(buf, "north");	door = DIR_NORTH; break;
			case 's' :	strcat(buf, "south");	door = DIR_SOUTH; break;
			case 'e' :	strcat(buf, "east");	door = DIR_EAST;  break;
			case 'w' :	strcat(buf, "west");	door = DIR_WEST;  break;
			case 'u' :	strcat(buf, "up" );		door = DIR_UP;	  break;
			case 'd' :	strcat(buf, "down");	door = DIR_DOWN;  break;
			default:	break;
		 }

                 if( door != -1
		     && ch->in_room->exit[door]
		     && IS_SET(ch->in_room->exit[door]->exit_info, EX_CLOSED)
		     && IS_SET(ch->in_room->exit[door]->exit_info, EX_NOPASS))
		 {
		     interpret(ch,buf);
			
	             if( IS_SET(ch->in_room->exit[door]->exit_info, EX_CLOSED))
		     {
		        if( ch->desc )
	                   ch->desc->outtop = outtop;
                           
                        send_to_char("\n\rYou are thrown from the {Dshadows{x.\n\r",ch);
		        act("$n appears from the {Dshadows{x.",ch,NULL,NULL,TO_ROOM);
	                affect_strip(ch,skill_lookup("shadowstep"));
		        return; 
		     }
				
                 }

                 if( ch->desc )
		    ch->desc->outtop = outtop;

		    continue;
	} 

	switch(*c)
	{
	       case 'n' :	door = DIR_NORTH; break;
	       case 's' :	door = DIR_SOUTH; break;
	       case 'e' :	door = DIR_EAST;  break;
	       case 'w' :	door = DIR_WEST;  break;
	       case 'u' :	door = DIR_UP;	  break;
	       case 'd' :	door = DIR_DOWN;  break;
	       default:	break;
	}

	if( door == -1 )
	{
	   affect_strip(ch,skill_lookup("shadowstep"));
	   send_to_char("You are thrown from the {Dshadows{x.\n\r",ch);

	   if( ch->in_room != start_room )
	      act("$n appears from the {Dshadows{x.",ch,NULL,NULL,TO_ROOM);
	     
           return;
	}

	in_room = ch->in_room;
	sprintf(buf2,"%s",dir_name[door]);
	interpret(ch,buf2);

	if( ch->in_room == in_room )
	{
	   affect_strip(ch,skill_lookup("shadowstep"));
	   send_to_char("You are thrown from the {Dshadows{x.\n\r",ch);
	
     	   if( ch->in_room != start_room )
	      act("$n appears from the {Dshadows{x.",ch,NULL,NULL,TO_ROOM);
	      return;
	}
		
	if( in_room->people && in_room != start_room && *(c+1) != '\0')
	   act("A dark {Dshadow{x passes over you.",in_room->people,NULL,NULL,TO_ALL);

	if( ch->desc )
	   ch->desc->outtop = outtop;

	aggr_update( ); 
  
     } 
  }

  affect_strip(ch, skill_lookup( "shadowstep_shadowstep"));
  do_function( ch, &do_look, "auto" );
  act("$n steps out of the {Dshadows{x.",ch,NULL,NULL,TO_ROOM);
  return;

}

//Check to see if a vnum has been explored
bool explored_vnum(CHAR_DATA *ch, int vnum)
{       int mask = vnum / 32; //Get which bucket the bit is in
        unsigned int bit = vnum % 32; //Get which bit in the bucket we're playing with
        EXPLORE_HOLDER *pExp; //The buckets bucket.

        if(bit == 0 ) // % 32 will return 0 if vnum == 32, instead make it the last bit of the previous mask
        {       mask--;
                bit = 32;
        }

        for(pExp = ch->pcdata->explored->bits ; pExp ; pExp = pExp->next ) //Iterate through the buckets
        {       if(pExp->mask != mask)
                        continue;
                //Found the right bucket, might be explored.
                if(IS_SET(pExp->bits, ( 1 << bit ) ) ) //Convert bit to 2^(bit-1) and see if it's set.
                        return TRUE;
                return FALSE; //Return immediately. This value wont be in any other bucket.
        }
        return FALSE;
}
//Explore a vnum. Assume it's not explored and just set it.
void explore_vnum(CHAR_DATA *ch, int vnum )
{       int mask = vnum / 32; //Get which bucket it will be in
        unsigned int bit = vnum % 32; // Get which bit to set
        EXPLORE_HOLDER *pExp; //The buckets bucket.

        if(bit == 0 ) // % 32 will return 0 if vnum is a multiple 32, instead make it the last bit of the previous mask
        {       mask--;
                bit = 32;
        }

        //Find the bucket.
        for(pExp = ch->pcdata->explored->bits ; pExp ; pExp = pExp->next )
                if(pExp->mask == mask)
                        break;

        if(!pExp) //If it's null, bucket not found, we'll whip one up.
        {       pExp = (EXPLORE_HOLDER *)calloc(sizeof(*pExp), 1); //Alloc and zero
                pExp->mask = mask;
                pExp->next = ch->pcdata->explored->bits; //Add to
                ch->pcdata->explored->bits = pExp;       //the list
        }

        SET_BIT(pExp->bits, ( 1 << bit ) ); //Convert bit to 2^(bit-1) and set
        ch->pcdata->explored->set++; //Tell how many rooms we've explored
}
        

//Explore a vnum.
void check_explore( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom )
{       if(IS_NPC(ch) ) return;

        
        if(explored_vnum(ch, pRoom->vnum) )
                return;

        explore_vnum(ch, pRoom->vnum);
}

void do_explored(CHAR_DATA *ch, char *argument )
{       char buf[MAX_STRING_LENGTH];
        sprintf(buf, "You have explored %d room%s!{x\r\n", ch->pcdata->explored->set, (ch->pcdata->explored->set == 1 ? "" : "s") );
        send_to_char(buf,ch);

//This shows all the rooms they've explored. Probably don't need mortals seeing this, and for immortals... it'd get really spammy. Mostly an example.
/*      for(pExp = ch->pcdata->explored->bits ; pExp ; pExp = pExp->next )
        {       for(bit = 1 ; bit <= 32 ; ++bit )
                {       if(IS_SET(pExp->bits, (1 << bit) ) )
                        {       sprintf(buf, "[%-5d]", (pExp->mask * 32 + bit) );
                                send_to_char(buf,ch);
                        }
                }
                send_to_char("\r\n",ch);
        }
*/
        return;
}

