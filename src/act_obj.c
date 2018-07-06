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
#include <string.h>
#include <stdlib.h>
#include "include.h"

/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA
bool	remove_obj	args( (CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
CD *	find_keeper	args( (CHAR_DATA *ch ) );
int	get_cost	args( (CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
void 	obj_to_keeper	args( (OBJ_DATA *obj, CHAR_DATA *ch ) );
OD *	get_obj_keeper	args( (CHAR_DATA *ch,CHAR_DATA *keeper,char *argument));
bool    in_donation_room args( (OBJ_DATA *obj) ); 
extern AFFECT_DATA *affect_free;

 
DECLARE_SPELL_FUN( spell_null);

#undef OD
#undef	CD


/* RT part of the corpse looting code */

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
{
    CHAR_DATA *owner, *wch;

    if (IS_IMMORTAL(ch))
	return TRUE;

    if (!obj->owner || obj->owner == NULL)
	return TRUE;

    owner = NULL;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
        if (!str_cmp(wch->name,obj->owner))
            owner = wch;

    if (owner == NULL)
	return TRUE;

    if (!str_cmp(ch->name,owner->name))
	return TRUE;

    if (!IS_NPC(owner) && IS_SET(owner->act,PLR_CANLOOT))
	return TRUE;

    if (is_same_group(ch,owner))
	return TRUE;

    return FALSE;
}


void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	send_to_char( "You can't take that.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( "$d: you can't carry that many items.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if (IS_QUESTOR(ch) && ch->pcdata->questobj > 0)
    {
	if (ch->pcdata->questobj == obj->pIndexData->vnum)
	{
   	    send_to_char("{RYou have almost completed your QUEST!{x\n\r", ch);
      	    send_to_char("{RReturn to the questmaster before your time runs out!{x\n\r", ch);
	}
    }

    if ((!obj->in_obj || obj->in_obj->carried_by != ch)
    &&  (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)))
    {
	act( "$d: you can't carry that much weight.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if (!can_loot(ch,obj))
    {
	act("Corpse looting is not permitted.",ch,NULL,NULL,TO_CHAR );
	return;
    }

    if (obj->in_room != NULL)
    {
	for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	    if (gch->on == obj)
	    {
		act("$N appears to be using $p.",
		    ch,obj,gch,TO_CHAR);
		return;
	    }
    }

    if (in_donation_room(obj))
    {
        if (get_trust(ch) < obj->level - 2)
        {
            send_to_char("You are not powerful enough to use that.\n\r",ch);
            return;
        }
        else if (!IS_OBJ_STAT(obj, ITEM_HAD_TIMER))
            obj->timer = 0;
 
        REMOVE_BIT(obj->extra_flags,ITEM_HAD_TIMER);
    }

    if ( container != NULL )
    {
	act( "You get $p from $P.", ch, obj, container, TO_CHAR );
	act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
	obj_from_obj( obj );
    }
    else
    {
	act( "You get $p.", ch, obj, container, TO_CHAR );
	act( "$n gets $p.", ch, obj, container, TO_ROOM );
	obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY)
    {
	ch->silver += obj->value[0];
	ch->gold += obj->value[1];
        if (IS_SET(ch->act,PLR_AUTOSPLIT))
        { /* AUTOSPLIT code */
    	  members = 0;
    	  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	  {
            if (!IS_AFFECTED(gch,AFF_CHARM) && is_same_group( gch, ch ) )
              members++;
    	  }

	  if ( members > 1 && (obj->value[0] > 1 || obj->value[1]))
	  {
	    sprintf(buffer,"%d %d",obj->value[0],obj->value[1]);
	    do_function(ch, &do_split, buffer);	
	  }
        }
 
	extract_obj( obj );
    }
    else
    {
	obj_to_char( obj, ch );
        if ( HAS_TRIGGER_OBJ( obj, TRIG_GET ) )
            p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_GET );
        if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_GET ) )
            p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_GET );
    }

    return;
}

void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2);

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Get what?\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );
	    if ( obj == NULL )
	    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

	    get_obj( ch, obj, NULL );
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		if(!IS_NPC(ch) && (IN_MINE(ch) || OBJ_IN_MINE(obj) ) && !OBJ_SAME_SHAFT(ch,obj) )
		    continue;
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
  
                    if (IS_SET(ch->in_room->room_flags, ROOM_DONATION) && !IS_IMMORTAL(ch))
                    {
                        send_to_char("Don't be so greedy!\n\r",ch);
                        return;
                    }

		    get_obj( ch, obj, NULL );
		}
	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "I see nothing here.\n\r", ch );
		else
		    act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }
    else
    {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, NULL, arg2 ) ) == NULL )
	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    send_to_char( "That's not a container.\n\r", ch );
	    return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	    break;

	case ITEM_CORPSE_PC:
	    {

		if (!can_loot(ch,container))
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
	    }
	}

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );
	    if ( obj == NULL )
	    {
		act( "I see nothing like that in the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act( "I see nothing in the $T.",
			ch, NULL, arg2, TO_CHAR );
		else
		    act( "I see nothing like that in the $T.",
			ch, NULL, arg2, TO_CHAR );
	    }
	}
    }

    return;
}



void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"in") || !str_cmp(arg2,"on"))
	argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Put what in what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, NULL, arg2 ) ) == NULL )
    {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
    }

    if ( container->item_type != ITEM_CONTAINER )
    {
	send_to_char( "That's not a container.\n\r", ch );
	return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
	act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "You can't fold it into itself.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

    	if (WEIGHT_MULT(obj) != 100)
    	{
           send_to_char("You have a feeling that would be a bad idea.\n\r",ch);
           return;
        }

        if (IS_OBJ_STAT(obj, ITEM_QUEST) && !IS_OBJ_STAT(container, ITEM_QUEST))
        {
           send_to_char("You can't put a quest item in something.\n\r", ch);
           return;
        }         

	if (get_obj_weight( obj ) + get_true_weight( container )
	     > (container->value[0] * 10) 
	||  get_obj_weight(obj) > (container->value[3] * 10))
	{
	    send_to_char( "It won't fit.\n\r", ch );
            if ( ch->sex == 1 )
                sound( "wontfit.wav", ch );
            else
                sound( "wontfitf.wav", ch );
	    return;
	}
	
	obj_from_char( obj );
	obj_to_obj( obj, container );

	if (IS_SET(container->value[1],CONT_PUT_ON))
	{
	    act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
	    act("You put $p on $P.",ch,obj,container, TO_CHAR);
	}
	else
	{
	    act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	    act( "You put $p in $P.", ch, obj, container, TO_CHAR );
	}
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   WEIGHT_MULT(obj) == 100
	    &&   obj->wear_loc == WEAR_NONE
	    &&   obj != container
	    &&   can_drop_obj( ch, obj )
	    &&   get_obj_weight( obj ) + get_true_weight( container )
		 <= (container->value[0] * 10) 
	    &&   get_obj_weight(obj) <= (container->value[3] * 10))
	    {
		obj_from_char( obj );
		obj_to_obj( obj, container );

        	if (IS_SET(container->value[1],CONT_PUT_ON))
        	{
            	    act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
            	    act("You put $p on $P.",ch,obj,container, TO_CHAR);
        	}
		else
		{
		    act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
		    act( "You put $p in $P.", ch, obj, container, TO_CHAR );
		}
	    }
	}
    }

    return;
}



void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drop what?\n\r", ch );
	return;
    }

    if ( is_number( arg ) )
    {
	/* 'drop NNNN coins' */
	int amount, gold = 0, silver = 0;

	amount   = atoi(arg);
	argument = one_argument( argument, arg );
	if ( amount <= 0
	|| ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) && 
	     str_cmp( arg, "gold"  ) && str_cmp( arg, "silver") ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	if ( !str_cmp( arg, "coins") || !str_cmp(arg,"coin") 
	||   !str_cmp( arg, "silver"))
	{
	    if (ch->silver < amount)
	    {
		send_to_char("You don't have that much silver.\n\r",ch);
		return;
	    }

	    ch->silver -= amount;
	    silver = amount;
	}

	else
	{
	    if (ch->gold < amount)
	    {
		send_to_char("You don't have that much gold.\n\r",ch);
		return;
	    }

	    ch->gold -= amount;
  	    gold = amount;
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if( obj->pIndexData->vnum == OBJ_VNUM_SILVER_ONE )
	    {	silver += 1;
		extract_obj(obj);
	    }
	    else if( obj->pIndexData->vnum ==  OBJ_VNUM_GOLD_ONE )
	    {	gold += 1;
		extract_obj( obj );
	    }
	    else if( obj->pIndexData->vnum ==  OBJ_VNUM_SILVER_SOME )
	    {	silver += obj->value[0];
		extract_obj(obj);
	    }
	    else if( obj->pIndexData->vnum ==  OBJ_VNUM_GOLD_SOME )
	    {	gold += obj->value[1];
		extract_obj( obj );
	    }
	    else if( obj->pIndexData->vnum ==  OBJ_VNUM_COINS )
	    {	silver += obj->value[0];
		gold += obj->value[1];
		extract_obj(obj);
	    }
	}
	obj = create_money(gold, silver );
	obj->carried_by = ch;
	obj_to_room( obj, ch->in_room );
	act( "$n drops some coins.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "OK.\n\r", ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

        if (IS_SET(ch->in_room->room_flags, ROOM_DONATION))
        {
            if (obj->timer)
                SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
            else
                obj->timer = number_range(100,200);
        }

	obj_from_char( obj );
	obj->carried_by = ch;
	obj_to_room( obj, ch->in_room );
	act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
	act( "You drop $p.", ch, obj, NULL, TO_CHAR );
   
     if ( HAS_TRIGGER_OBJ( obj, TRIG_DROP ) )
            p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_DROP );
        if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_DROP ) )
            p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_DROP );

        if ( obj && IS_OBJ_STAT(obj,ITEM_MELT_DROP))
        {
            act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
            act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
            extract_obj(obj);
        }       
    }
    else
    {
	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj )
	    &&   obj->belted_vnum == 0
	    &&   obj->strap_loc == -1) 
	    {
		found = TRUE;

                if (IS_SET(ch->in_room->room_flags, ROOM_DONATION))
                {
                    if (obj->timer)
                        SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
                    else
                        obj->timer = number_range(100,200);
                }

		obj_from_char( obj );
		obj->carried_by = ch;
		obj_to_room( obj, ch->in_room );
		act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
		act( "You drop $p.", ch, obj, NULL, TO_CHAR );
                
                if ( HAS_TRIGGER_OBJ( obj, TRIG_DROP ) )
                 p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_DROP );
                if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_DROP ) )
                 p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_DROP );

                if ( obj && IS_OBJ_STAT(obj,ITEM_MELT_DROP))
                {
                act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
                act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
                extract_obj(obj);
                }        
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Give what to whom?\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount;
	bool silver;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) && 
	     str_cmp( arg2, "gold"  ) && str_cmp( arg2, "silver")) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	silver = str_cmp(arg2,"gold");

	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Give what to whom?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( (!silver && ch->gold < amount) || (silver && ch->silver < amount) )
	{
	    send_to_char( "You haven't got that much.\n\r", ch );
	    return;
	}

	if (silver)
	{
	    ch->silver		-= amount;
	    victim->silver 	+= amount;
	}
	else
	{
	    ch->gold		-= amount;
	    victim->gold	+= amount;
	}

	sprintf(buf,"$n gives you %d %s.",amount, silver ? "silver" : "gold");
	act( buf, ch, NULL, victim, TO_VICT    );
	act( "$n gives $N some coins.",  ch, NULL, victim, TO_NOTVICT );
	sprintf(buf,"You give $N %d %s.",amount, silver ? "silver" : "gold");
	act( buf, ch, NULL, victim, TO_CHAR    );

        if ( IS_NPC(victim) && HAS_TRIGGER_MOB( victim, TRIG_BRIBE ) )
            p_bribe_trigger( victim, ch, silver ? amount : amount * 100 );    

	if (IS_NPC(victim) && IS_SET(victim->act,ACT_IS_CHANGER))
	{
	    int change;

	    change = (silver ? 95 * amount / 100 / 100 
		 	     : 95 * amount);


	    if (!silver && change > victim->silver)
	    	victim->silver += change;

	    if (silver && change > victim->gold)
		victim->gold += change;

	    if (change < 1 && can_see(victim,ch))
	    {
		act(
	"$n tells you 'I'm sorry, you did not give me enough to change.'"
		    ,victim,NULL,ch,TO_VICT);
		ch->reply = victim;
		sprintf(buf,"%d %s %s", 
			amount, silver ? "silver" : "gold",ch->name);
		do_function(victim, &do_give, buf);
	    }
	    else if (can_see(victim,ch))
	    {
		sprintf(buf,"%d %s %s", 
			change, silver ? "gold" : "silver",ch->name);
		do_function(victim, &do_give, buf);
		if (silver)
		{
		    sprintf(buf,"%d silver %s", 
			(95 * amount / 100 - change * 100),ch->name);
		    do_function(victim, &do_give, buf);
		}
		act("$n tells you 'Thank you, come again.'",
		    victim,NULL,ch,TO_VICT);
		ch->reply = victim;
	    }
	}
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    {
	act("$N tells you 'Sorry, you'll have to sell that.'",
	    ch,NULL,victim,TO_CHAR);
	ch->reply = victim;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
        if ( ch->sex == 1 )
        sound( "armsfull.wav", ch );
        else
        sound( "armsfullf.wav", ch );
	act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) )
    {
	act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (IS_OBJ_STAT(obj, ITEM_QUEST) && ch->level <= HERO)
    {
	send_to_char("You can't give quest items.\n\r", ch);
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    MOBtrigger = FALSE;
    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );
    MOBtrigger = TRUE;

    if ( HAS_TRIGGER_OBJ( obj, TRIG_GIVE ) )
        p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_GIVE );

    if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_GIVE ) )
        p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_GIVE );        

    if ( IS_NPC(victim) && HAS_TRIGGER_MOB( victim, TRIG_GIVE ) )
        p_give_trigger( victim, NULL, NULL, ch, obj, TRIG_GIVE ); 

    return;
}


/* for poisoning weapons and food/drink */
void do_envenom(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA af;
    int percent,skill;

    /* find out what */
    if (argument[0] == '\0')
    {
	send_to_char("Envenom what item?\n\r",ch);
	return;
    }

    obj =  get_obj_list(ch,argument,ch->carrying);

    if (obj== NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if ((skill = get_skill(ch,skill_lookup("envenom"))) < 1)
    {
	send_to_char("Are you crazy? You'd poison yourself!\n\r",ch);
	return;
    }

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
	if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	{
	    act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (number_percent() < skill)  /* success! */
	{
	    act("$n treats $p with deadly poison.",ch,obj,NULL,TO_ROOM);
	    act("You treat $p with deadly poison.",ch,obj,NULL,TO_CHAR);
	    if (!obj->value[3])
	    {
		obj->value[3] = 1;
		check_improve(ch,skill_lookup("envenom"),TRUE,4);
	    }
	    WAIT_STATE(ch,skill_table[skill_lookup("envenom")].beats);
	    return;
	}

	act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
	if (!obj->value[3])
	    check_improve(ch,skill_lookup("envenom"),FALSE,4);
	WAIT_STATE(ch,skill_table[skill_lookup("envenom")].beats);
	return;
     }

    if (obj->item_type == ITEM_WEAPON)
    {
        if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
        ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
        ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
        ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
        ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
        {
            act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
            return;
        }

	if (obj->value[3] < 0 
	||  attack_table[obj->value[3]].damage == DAM_BASH)
	{
	    send_to_char("You can only envenom edged weapons.\n\r",ch);
	    return;
	}

        if (IS_WEAPON_STAT(obj,WEAPON_POISON))
        {
            act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
            return;
        }

	percent = number_percent();
	if (percent < skill)
	{
 
            af.where     = TO_WEAPON;
            af.type      = skill_lookup("poison");
            af.level     = ch->level * percent / 100;
            af.duration  = ch->level/2 * percent / 100;
            af.location  = 0;
            af.modifier  = ch->level/10;
            af.bitvector = WEAPON_POISON;
            affect_to_obj(obj,&af);
 
            act("$n coats $p with deadly venom.",ch,obj,NULL,TO_ROOM);
	    act("You coat $p with venom.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,skill_lookup("envenom"),TRUE,3);
	    WAIT_STATE(ch,skill_table[skill_lookup("envenom")].beats);
            return;
        }
	else
	{
	    act("You fail to envenom $p.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,skill_lookup("envenom"),FALSE,3);
	    WAIT_STATE(ch,skill_table[skill_lookup("envenom")].beats);
	    return;
	}
    }
 
    act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
    return;
}

void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    found = FALSE;
    for ( fountain = ch->in_room->contents; fountain != NULL;
	fountain = fountain->next_content )
    {
	if ( fountain->item_type == ITEM_FOUNTAIN )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "There is no fountain here!\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You can't fill that.\n\r", ch );
	return;
    }

    if ( obj->value[1] != 0 && obj->value[2] != fountain->value[2] )
    {
	send_to_char( "There is already another liquid in it.\n\r", ch );
	return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
	send_to_char( "Your container is full.\n\r", ch );
	return;
    }

    if(!str_cmp(liq_table[fountain->value[2]].liq_name, "magic potion"))
    {
        sprintf(buf, "Strangly, you can't fill $p with %s from $P.", liq_table[fountain->value[2]].liq_name);
        act( buf, ch, obj, fountain, TO_CHAR);
        return;
    }

    sprintf(buf,"You fill $p with %s from $P.",
	liq_table[fountain->value[2]].liq_name);
    act( buf, ch, obj,fountain, TO_CHAR );
    sprintf(buf,"$n fills $p with %s from $P.",
	liq_table[fountain->value[2]].liq_name);
    act(buf,ch,obj,fountain,TO_ROOM);
    obj->value[2] = fountain->value[2];
    obj->value[1] = obj->value[0];
    return;
}

void do_pour (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
    OBJ_DATA *out, *in;
    CHAR_DATA *vch = NULL;
    int amount;

    argument = one_argument(argument,arg);
    
    if (arg[0] == '\0' || argument[0] == '\0')
    {
	send_to_char("Pour what into what?\n\r",ch);
	return;
    }
    

    if ((out = get_obj_carry(ch,arg, ch)) == NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if (out->item_type != ITEM_DRINK_CON)
    {
	send_to_char("That's not a drink container.\n\r",ch);
	return;
    }

    if (!str_cmp(argument,"out"))
    {
	if (out->value[1] == 0)
	{
	    send_to_char("It's already empty.\n\r",ch);
	    return;
	}

	out->value[1] = 0;
	out->value[3] = 0;
	sprintf(buf,"You invert $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_CHAR);
	
	sprintf(buf,"$n inverts $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_ROOM);
	return;
    }

    if ((in = get_obj_here(ch, NULL, argument)) == NULL)
    {
	vch = get_char_room(ch, NULL, argument);

	if (vch == NULL)
	{
	    send_to_char("Pour into what?\n\r",ch);
	    return;
	}

	in = get_eq_char(vch,WEAR_HOLD);

	if (in == NULL)
	{
	    send_to_char("They aren't holding anything.",ch);
 	    return;
	}
    }

    if (in->item_type != ITEM_DRINK_CON)
    {
	send_to_char("You can only pour into other drink containers.\n\r",ch);
	return;
    }
    
    if (in == out)
    {
	send_to_char("You cannot change the laws of physics!\n\r",ch);
	return;
    }

    if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
	send_to_char("They don't hold the same liquid.\n\r",ch);
	return;
    }

    if (out->value[1] == 0)
    {
	act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR);
	return;
    }

    if (in->value[1] >= in->value[0])
    {
	act("$p is already filled to the top.",ch,in,NULL,TO_CHAR);
	return;
    }

    amount = UMIN(out->value[1],in->value[0] - in->value[1]);

    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];
    
    if (vch == NULL)
    {
    	sprintf(buf,"You pour %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_CHAR);
    	sprintf(buf,"$n pours %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_ROOM);
    }
    else
    {
        sprintf(buf,"You pour some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_CHAR);
	sprintf(buf,"$n pours you some %s.",
	    liq_table[out->value[2]].liq_name);
	act(buf,ch,NULL,vch,TO_VICT);
        sprintf(buf,"$n pours some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_NOTVICT);
	
    }
}

void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
	    if ( obj->item_type == ITEM_FOUNTAIN )
		break;
	}

	if ( obj == NULL )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, NULL, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	send_to_char( "You can't drink from that.\n\r", ch );
	return;

    case ITEM_FOUNTAIN:
        if ( ( liquid = obj->value[2] )  < 0 )
        {
            bug( "Do_drink: bad liquid number %d.", liquid );
            liquid = obj->value[2] = 0;
        }
	amount = liq_table[liquid].liq_affect[4] * 3;
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] )  < 0 )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

        amount = liq_table[liquid].liq_affect[4];
        amount = UMIN(amount, obj->value[1]);
	break;
     }
    if (!IS_NPC(ch) && !IS_IMMORTAL(ch) 
    &&  ch->pcdata->condition[COND_FULL] > 45)
    {
        if ( ch->sex == 1 )
            sound( "nothirst.wav", ch );
        else
            sound( "nothirstf.wav", ch );
	send_to_char("You're too full to drink more.\n\r",ch);
	return;
    }

    if ( !str_cmp ( liq_table[liquid].liq_name, "magic potion" ) )
    {
        spell_random(0, ch->level, ch, ch, TAR_IGNORE);
        WAIT_STATE(ch, 5);
    }

    act( "$n drinks $T from $p.",
	ch, obj, liq_table[liquid].liq_name, TO_ROOM );
    act( "You drink $T from $p.",
	ch, obj, liq_table[liquid].liq_name, TO_CHAR );
    sound( "drink.wav", ch );

    gain_condition( ch, COND_DRUNK,
	amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36 );
    gain_condition( ch, COND_FULL,
	amount * liq_table[liquid].liq_affect[COND_FULL] / 4 );
    gain_condition( ch, COND_THIRST,
	amount * liq_table[liquid].liq_affect[COND_THIRST] / 10 );
    gain_condition(ch, COND_HUNGER,
	amount * liq_table[liquid].liq_affect[COND_HUNGER] / 2 );

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	send_to_char( "You feel drunk.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
	send_to_char( "You are full.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
	send_to_char( "Your thirst is quenched.\n\r", ch );
	
    if ( obj->value[3] != 0 )
    {
	/* The drink was poisoned ! */
	AFFECT_DATA af;

	act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You choke and gag.\n\r", ch );
	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("poison");
	af.level	 = number_fuzzy(amount); 
	af.duration  = 3 * amount;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_POISON;
	affect_join( ch, &af );
    }
	
    if (obj->value[0] > 0)
        obj->value[1] -= amount;

    return;
}



void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) )
    {
        if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL && obj->item_type != ITEM_TOKEN )
	{
	    send_to_char( "That's not edible.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 )
	{   
	    send_to_char( "You are too full to eat more.\n\r", ch );
	    return;
	}
    }

    act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );
    act( "You eat $p.", ch, obj, NULL, TO_CHAR );
    sound( "eat.wav", ch );

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_HUNGER];
	    gain_condition( ch, COND_FULL, obj->value[0] );
	    gain_condition( ch, COND_HUNGER, obj->value[1]);
	    if ( condition == 0 && ch->pcdata->condition[COND_HUNGER] > 0 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( ch->pcdata->condition[COND_FULL] > 40 )
		send_to_char( "You are full.\n\r", ch );
	}

	if ( obj->value[3] != 0 )
	{
	    /* The food was poisoned! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );

	    af.where	 = TO_AFFECTS;
	    af.type      = skill_lookup("poison");
	    af.level 	 = number_fuzzy(obj->value[0]);
	    af.duration  = 2 * obj->value[0];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}
	break;

    case ITEM_PILL:
	obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	break;

    case ITEM_TOKEN:
        value = obj->value[0];
        ch->pcdata->questpoints += value;
        break;
    }

    extract_obj( obj );
    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj, *temp;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
	act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

    unequip_char( ch, obj );
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
	if ( obj->item_type == ITEM_BELT )
	{
		for ( temp = ch->carrying; temp; temp = temp->next_content )
		{
			if ( temp->belted_vnum == obj->pIndexData->vnum)
				temp->belted_vnum = 0;
		}
	}
	if( obj->item_type == ITEM_SHEATH )
	{
		for( temp = ch->carrying; temp; temp = temp->next_content )
		{
			if(temp->sheathed_in == obj)
			    	temp->sheathed_in = NULL;
		}
	}

    if( obj->wear_loc == WEAR_HOOD )
    {
	if(IS_SET(ch->mask,HOOD_RAISED) )
	{
	    send_to_char("You lower, and remove your hood.\n\r",ch);
	    act("$n lowers, and removes $s hood", ch, NULL,NULL, TO_ROOM );
	    REMOVE_BIT(ch->mask,HOOD_RAISED);
            SET_BIT(ch->mask,HOOD_REMOVED); 
	}
    }
	return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->level < obj->level && number_classes(ch) <= 1 ) 
    {
	sprintf( buf, "You must be level %d to use this object.\n\r",
	    obj->level );
	send_to_char( buf, ch );
	act( "$n tries to use $p, but is too inexperienced.",
	    ch, obj, NULL, TO_ROOM );
        if ( ch->sex == 1 )
            sound( "nskilled.wav", ch );
        else
            sound( "nskilledf.wav", ch );
	return;
    }

    if ( ch->level <  obj->level -5 && number_classes(ch) == 2 ) 
    {
        sprintf( buf, "You must be level %d to use this object.\n\r", obj->level - 5 );
        send_to_char( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.", ch, obj, NULL, TO_ROOM );
        if ( ch->sex == 1 )
            sound( "nskilled.wav", ch );
        else
            sound( "nskilledf.wav", ch );
        return;
    }

    if ( ch->level < obj->level -10 && number_classes(ch) == 3 )
    {
        sprintf( buf, "You must be level %d to use this object.\n\r", obj->level -10 );
        send_to_char( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.", ch, obj, NULL, TO_ROOM );
        if ( ch->sex == 1 )
            sound( "nskilled.wav", ch );
        else
            sound( "nskilledf.wav", ch );
        return;
    }
   
    if ( ch->level < obj->level -15 && number_classes(ch) == 4 )
    {
        sprintf( buf, "You must be level %d to use this object.\n\r", obj->level -15 );
        send_to_char( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.", ch, obj, NULL, TO_ROOM );
        if ( ch->sex == 1 )
            sound( "nskilled.wav", ch );
        else
            sound( "nskilledf.wav", ch );
        return;
    }

    if ( obj->item_type == ITEM_LIGHT )
    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
	    return;
	act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
	act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LIGHT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
	    act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
            sound( "ring.wav", ch );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
	    act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
            sound( "ring.wav", ch );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char( "You already wear two rings.\n\r", ch );
	return;
    }

    if (!wear_obj_size(ch, obj))
        return;  

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "You already wear two neck items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	    return;
	act( "$n wears $p on $s torso.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your torso.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	    return;
	act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	    return;
	act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	    return;
	act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	    return;
	act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WEDDING ) )
    {
        if ( !remove_obj( ch, WEAR_WEDDING, fReplace ) )
            return;
        act( "$n wears $p on $s finger.", ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your finger.", ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_WEDDING );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	    return;
	act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHOULDER ) )
    {
        if ( !remove_obj( ch, WEAR_SHOULDER, fReplace ) )
            return;
        act( "$n wears $p on $s shoulder.",   ch, obj, NULL, TO_ROOM );
        act( "You wear $p on your shoulder.", ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_SHOULDER );
        return;
    }
    if ( CAN_WEAR( obj, ITEM_WEAR_BACK ) )
    {
	if ( !remove_obj( ch, WEAR_BACK, fReplace ) )
	    return;
	act( "$n places $p on thier back.",   ch, obj, NULL, TO_ROOM );
	act( "You place $p on your back.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BACK );
	return;
    }
    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	    return;
	act( "$n wears $p about $s torso.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your torso.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	    return;
	act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    act( "$n wears $p around $s left wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your left wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    act( "$n wears $p around $s right wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your right wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char( "You already wear two wrist items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
	OBJ_DATA *weapon;

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
	    return;

	weapon = get_eq_char(ch,WEAR_WIELD);
	if (weapon != NULL && ch->size < SIZE_LARGE 
	&&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS))
	{
	    send_to_char("Your hands are tied up with your weapon!\n\r",ch);
	    return;
	}

        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("You cannot use a shield while using 2 weapons.\n\r",ch);
            return;
        }

	act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
	act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHIELD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	int sn,skill;

	if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
	    return;

	if ( !IS_NPC(ch) 
	&& get_obj_weight(obj) > (str_app[get_curr_stat(ch,STAT_STR)].wield  
		* 10))
	{
	    send_to_char( "It is too heavy for you to wield.\n\r", ch );
	    return;
	}

	if (!IS_NPC(ch) && ch->size < SIZE_LARGE 
	&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
 	&&  get_eq_char(ch,WEAR_SHIELD) != NULL)
	{
	    send_to_char("You need two hands free for that weapon.\n\r",ch);
	    return;
	}

	act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
	act( "You wield $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WIELD );

        sn = get_weapon_sn(ch);

	if (sn == skill_lookup("hand to hand"))
	   return;

        skill = get_weapon_skill(ch,sn);
 
        if (skill >= 100)
            act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
        else if (skill > 85)
            act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 70)
            act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 50)
            act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
        else if (skill > 25)
            act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
        else if (skill > 1)
            act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
        else
            act("You don't even know which end is up on $p.",
                ch,obj,NULL,TO_CHAR);

	return;
    }
    if ( CAN_WEAR( obj, ITEM_WEAR_CLOAK) )
    {
	if(get_eq_char(ch, WEAR_CLOAK) != NULL)
	{
	    send_to_char("You are already wearing a cloak!\n\r",ch);
	    return;
	}
	if ( !remove_obj( ch, WEAR_CLOAK, fReplace ) )
	    return;
	act( "$n wears $p about $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_CLOAK );
	return;
    }
    if ( CAN_WEAR( obj, ITEM_LAYER_ARMS) )
    {
	if(get_eq_char(ch, WEAR_LAYER_ARMS) != NULL)
	{
	    send_to_char("You already have something wrapped about your arms!\n\r",ch);
	    return;
	}
	if ( !remove_obj( ch, WEAR_LAYER_ARMS, fReplace ) )
	    return;
	act( "$n wears $p around $s arms.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p around your arms.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LAYER_ARMS );
	return;	
    }
    if ( CAN_WEAR( obj, ITEM_LAYER_LEGS) )
    {
	if(get_eq_char(ch, WEAR_LAYER_LEGS) != NULL)
	{
	    send_to_char("You already have something wrapped about your legs!\n\r",ch);
	    return;
	}
	if ( !remove_obj( ch, WEAR_LAYER_LEGS, fReplace ) )
	    return;
	act( "$n wears $p around $s legs.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p around your legs.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LAYER_LEGS );
	return;	
    }
    if ( CAN_WEAR( obj, ITEM_LAYER_BODY) )
    {
	if(get_eq_char(ch, WEAR_LAYER_BODY) != NULL)
	{
	    send_to_char("You already have something wrapped about your body!\n\r",ch);
	    return;
	}
	if ( !remove_obj( ch, WEAR_LAYER_BODY, fReplace ) )
	    return;
	act( "$n wears $p around $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p around your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LAYER_BODY );
	return;	
    }
    if ( CAN_WEAR( obj, ITEM_WEAR_HOOD) )
    {
	if(get_eq_char(ch, WEAR_HOOD) != NULL)
	{
	    send_to_char("You are already wearing a hood!\n\r",ch);
	    return;
	}
	if ( !remove_obj( ch, WEAR_HOOD, fReplace ) )
	    return;
	act( "$n wears $p as $s hood.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p as a hood.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HOOD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
	if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
	    return;
	act( "$n holds $p in $s hand.",   ch, obj, NULL, TO_ROOM );
	act( "You hold $p in your hand.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HOLD );
	return;
    }

    if ( CAN_WEAR(obj,ITEM_WEAR_FLOAT) )
    {
	if (!remove_obj(ch,WEAR_FLOAT, fReplace) )
	    return;
	act("$n releases $p to float next to $m.",ch,obj,NULL,TO_ROOM);
	act("You release $p and it floats next to you.",ch,obj,NULL,TO_CHAR);
	equip_char(ch,obj,WEAR_FLOAT);
	return;
    }

    if ( fReplace )
	send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
            {
                if (obj->condition != 0)  
     	        {
                    wear_obj( ch, obj, FALSE );

                    if ( obj->item_type == ITEM_WEAPON && chance( 10 ) &&  IS_OBJ2_STAT( obj, ITEM_RELIC ) )
                    {
                        printf_to_char( ch, "{c%s whispers ok lets rock!{x\n\r", obj->name );
                    }
                }

                else
                    act ("$p is too damaged to wear!", ch, obj, NULL, TO_CHAR);
            } 
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

        if (obj->condition == 0)
            act("$p is too damaged to wear!", ch, obj, NULL, TO_CHAR);
        else
        {
            wear_obj( ch, obj, TRUE );    
     
            if ( obj->item_type == ITEM_WEAPON && chance( 10 ) && IS_OBJ2_STAT( obj, ITEM_RELIC ) )
            {
                printf_to_char( ch, "%s whispers ok lets rock!\n\r", obj->name );
            }
        }

    }

    return;
}



void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *tmp, *temp;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove what?\n\r", ch );
	return;
    }

    if (!str_cmp(arg, "all"))
    {
        for (tmp = ch->carrying ; tmp != NULL ; tmp = tmp->next_content)
        {
             if (tmp->wear_loc != WEAR_NONE)
                 remove_obj(ch, tmp->wear_loc, TRUE);
        }
                
        return;
    }            

    if ( ( obj = get_obj_wear( ch, arg, TRUE ) ) == NULL )
    {
			if ( ( obj = get_obj_carry( ch, arg, ch ) ) && obj->belted_vnum > 0 )
		{
			printf_to_char( ch, "You carefully unstrap %s from your belt.\n\r", obj->short_descr );
			sprintf( arg, "$n carefully unstraps %s from $s belt.", obj->short_descr );
			act( arg, ch, NULL, NULL, TO_ROOM );
			obj->belted_vnum = 0;
			return;
		}

		if ( ( obj = get_obj_carry( ch, arg, ch ) ) && obj->sheathed_in != NULL )
		{
		           if ( get_eq_char( ch, WEAR_WIELD ) != NULL )
			   {
				send_to_char("You are already wielding a weapon. Remove it then unsheath this blade\n\r",ch);   
				return;
			   }
			printf_to_char( ch, "You unsheath %s and wield it.\n\r", obj->short_descr );
			sprintf( arg, "$n unsheaths %s and wields it.", obj->short_descr );
                        rnd_sound( 2, ch, "unsheath.wav", "unsheath2.wav" );
			act( arg, ch, NULL, NULL, TO_ROOM );
                        equip_char( ch, obj, WEAR_WIELD ); 
			obj->sheathed_in = NULL;
			return;
		}
		if( (obj = get_obj_carry( ch, arg, ch) ) && obj->strap_loc > -1 )
		{
			printf_to_char(ch, "You unstrap %s, and remove it.\n\r",obj->short_descr);
			obj->strap_loc = -1;
                	for( temp = ch->carrying; temp; temp = temp->next_content )
                	{
                            if(temp->sheathed_in == obj)
                            temp->sheathed_in = NULL;
                	} 
			return;
	      }

	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}



void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int silver;
    
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];


    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
        /* It says ShadowStorm may accept it later ... =) */
        if (number_percent() == 1)
        {
            act("$n offers $mself to Someone, who graciously accepts!", ch,NULL,NULL,TO_ROOM);
            act("Shadowstorm appreciates your offer and accepts it!",ch,NULL,NULL,TO_CHAR);
            ch->mana = 1;
            ch->move = 1;
            ch->hit  = 1;
            return;
        }

	act( "$n offers $mself to Someone, who graciously declines.",ch, NULL, NULL, TO_ROOM );
        act( "ShadowStorm appreciates your offer and may accept it later.\n\r", ch, NULL, NULL, TO_CHAR );
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_DONATION))
    {
        send_to_char("You cannot sacrifice in a donation room.\n\r",ch);
        return;
    }

    if (!str_cmp("all",arg) || !str_prefix("all.",arg))
    {
        OBJ_DATA *obj_next;
        bool found = FALSE;

        for (obj = ch->in_room->contents;obj;obj = obj_next)
        { 
             obj_next = obj->next_content;

             if (arg[3] != '\0' && !is_name(&arg[4],obj->name))
                 continue;

             if (  (!CAN_WEAR(obj,ITEM_TAKE) || CAN_WEAR(obj,ITEM_NO_SAC))
                 || (obj->item_type == ITEM_CORPSE_PC && obj->contains))
                 continue;
           
             silver = UMAX(1,obj->level * 3);

             if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
                 silver = UMIN(silver,obj->cost);

             found = TRUE;

 /*            if(!IS_NPC(ch) && ch->pcdata->religion != NULL )
	     {
          	religious_sacrifice(ch, obj );
	        continue;
	     } */

             printf_to_char( ch, "{CShadowStorm gives you {Y%d{c silver for your sacrifice of {w%s{c.{x\n\r", silver, obj->short_descr );
             act( "$n sacrifices $p to ShadowStorm.", ch, obj, NULL, TO_ROOM );
             ch->silver += silver;
             extract_obj( obj );

             if (IS_SET(ch->act,PLR_AUTOSPLIT))
             { 
                 members = 0;
  
                 for (gch = ch->in_room->people;gch;gch = gch->next_in_room)
                      if (is_same_group(ch,gch))
                          members++;

                      if (members > 1 && silver > 1)
                      { 
                          sprintf(buf,"%d",silver);
                          do_function( ch, &do_split, buf); 
                      }
             }
        }

        if (found)
            wiznet("$N sends up everything in that room as a burnt offering.",ch,obj,WIZ_SACCING,0,0);

        else
            send_to_char("There is nothing sacrificable in this room.\n\r",ch);
            return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    
    if ( obj == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }

    if ( obj->item_type == ITEM_CORPSE_PC )
    {
	if (obj->contains)
        {
           act( "ShadowStorm wouldn't like that.\n\r", ch, NULL, NULL, TO_CHAR );
	   return;
        }
    }

    if ( !CAN_WEAR(obj, ITEM_TAKE) || CAN_WEAR(obj, ITEM_NO_SAC))
    {
	act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
	return;
    }

    if (obj->in_room != NULL)
    {
	for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	    if (gch->on == obj)
	    {
		act("$N appears to be using $p.",
		    ch,obj,gch,TO_CHAR);
		return;
	    }
    }
		
    silver = UMAX(1,obj->level * 3);

    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    	silver = UMIN(silver,obj->cost);
	
    /*if(!IS_NPC(ch) && HAS_RELIGION(ch) )
    {	
       religious_sacrifice(ch, obj );
       return;
    }*/

    if (silver == 1)
        act( "ShadowStorm gives you one silver coin for your sacrifice.\n\r", ch,NULL, NULL, TO_CHAR );
    else
    {
	sprintf(buf,"ShadowStorm gives you %d silver coins for your sacrifice.\n\r",silver);
	send_to_char(buf,ch);
    }
    
    ch->silver += silver;
    
    if (IS_SET(ch->act,PLR_AUTOSPLIT) )
    { /* AUTOSPLIT code */
    	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	{
    	    if ( is_same_group( gch, ch ) )
            members++;
    	}

	if ( members > 1 && silver > 1)
	{
	    sprintf(buffer,"%d",silver);
	    do_function(ch, &do_split, buffer);	
	}
    }

    act( "$n sacrifices $p to ShadowStorm", ch, obj, NULL,  TO_ROOM );
    wiznet("$N sends up $p as a burnt offering.",
	   ch,obj,WIZ_SACCING,0,0);
    extract_obj( obj );
    return;
}



void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }

    /* Stop quaff of healing potions in arena */
    if (IS_SET(ch->in_room->room_flags, ROOM_ARENA))
    {
    act("You try to drink from $p, but feel to many eyes watching you.", ch, NULL, NULL, TO_CHAR);
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that potion.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char( "You can quaff only potions.\n\r", ch );
	return;
    }

    if (ch->level < obj->level)
    {
	send_to_char("This liquid is too powerful for you to drink.\n\r",ch);
	return;
    }

    else if (ch->level < obj->level - 5 && number_classes( ch ) == 2 )
    {
        send_to_char("This liquid is too powerful for you to drink.\n\r", ch );
        return;
    }

    else if (ch->level < obj->level - 10 && number_classes( ch ) == 3 )
    {
        send_to_char("This liquid is too powerful for you to drink.\n\r", ch );
        return;
    }

    else if (ch->level < obj->level - 15 && number_classes( ch ) == 4 )
    {
        send_to_char("This liquid is too powerful for you to drink.\n\r", ch );
        return;
    }
         
    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );
    sound( "quaff.wav", ch );

    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );

    extract_obj( obj );
    return;
}



void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "You do not have that scroll.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char( "You can recite only scrolls.\n\r", ch );
	return;
    }

    if ( ch->level < scroll->level)
    {
	send_to_char(
		"This scroll is too complex for you to comprehend.\n\r",ch);
	return;
    }

    else if ( ch->level < scroll->level - 5 && number_classes( ch ) == 2 )
    {
        send_to_char(
                "This scroll is too complex for you to comprehend.\n\r", ch );
        return;
    }

    else if ( ch->level < scroll->level - 10 && number_classes( ch ) == 3 )
    {
        send_to_char(
                "This scroll is too complex for you to comprehend.\n\r", ch );
        return;
    }
        
    else if ( ch->level < scroll->level - 15 && number_classes( ch ) == 4 )
    {
        send_to_char(
                "This scroll is too complex for you to comprehend.\n\r", ch );
        return;
    }

    obj = NULL;
    if ( arg2[0] == '\0' )
    {
	victim = ch;
    }
    else
    {
	if ( ( victim = get_char_room ( ch, NULL, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, NULL, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR );
    sound( "scroll.wav", ch );

    if (number_percent() >= 20 + get_skill(ch,skill_lookup("scrolls")) * 4/5)
    {
	send_to_char("You mispronounce a syllable.\n\r",ch);
	check_improve(ch,skill_lookup("scrolls"),FALSE,2);
    }

    else
    {
    	obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
    	obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
    	obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
	check_improve(ch,skill_lookup("scrolls"),TRUE,2);
    }

    extract_obj( scroll );
    return;
}



void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn;

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "You can brandish only with a staff.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= MAX_SKILL
    ||   skill_table[sn].spell_fun == 0 )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
	act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
	act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
	if ( ch->level < staff->level 
	||   number_percent() >= 20 + get_skill(ch,skill_lookup("staves")) * 4/5)
 	{
	    act ("You fail to invoke $p.",ch,staff,NULL,TO_CHAR);
	    act ("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
	    check_improve(ch,skill_lookup("staves"),FALSE,2);
	}
	
	else for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next	= vch->next_in_room;

	    switch ( skill_table[sn].target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %d.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;
		
	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
	    check_improve(ch,skill_lookup("staves"),TRUE,2);
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
	act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
	extract_obj( staff );
    }

    return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Zap whom or what?\n\r", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "You can zap only with a wand.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char( "Zap whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, NULL, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
	if ( victim != NULL )
	{
	    act( "$n zaps $N with $p.", ch, wand, victim, TO_NOTVICT );
	    act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
	    act( "$n zaps you with $p.",ch, wand, victim, TO_VICT );
	}
	else
	{
	    act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
	    act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
	}

 	if (ch->level < wand->level 
	||  number_percent() >= 20 + get_skill(ch,skill_lookup("wands")) * 4/5) 
	{
	    act( "Your efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_CHAR);
	    act( "$n's efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_ROOM);
	    check_improve(ch,skill_lookup("wands"),FALSE,2);
	}
	else
	{
	    obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
	    check_improve(ch,skill_lookup("wands"),TRUE,2);
	}
    }

    if ( --wand->value[2] <= 0 )
    {
	act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
	act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
	extract_obj( wand );
    }

    return;
}



void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Steal what from whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_SET(victim->in_room->room_flags, ROOM_ARENA))
    {
	send_to_char("You are here to do battle, not to steal!\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) 
	  && victim->position == POS_FIGHTING)
    {
	send_to_char(  "Kill stealing is not permitted.\n\r"
		       "You'd better not -- you might get hit.\n\r",ch);
	return;
    }

    WAIT_STATE( ch, skill_table[skill_lookup("steal")].beats );
    percent  = number_percent();

    if (!IS_AWAKE(victim))
    	percent -= 10;
    else if (!can_see(victim,ch))
    	percent += 25;
    else 
	percent += 50;

    if ( ((ch->level + 7 < victim->level || ch->level -7 > victim->level) 
    && !IS_NPC(victim) && !IS_NPC(ch) )
    || ( !IS_NPC(ch) && percent > get_skill(ch,skill_lookup("steal")))
    || ( !IS_NPC(ch) && !is_clan(ch)) )
    {
	/*
	 * Failure.
	 */
	send_to_char( "Oops.\n\r", ch );
	affect_strip(ch,skill_lookup("sneak"));
	REMOVE_BIT(ch->affected_by,AFF_SNEAK);

	act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT    );
	act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );
	switch(number_range(0,3))
	{
	case 0 :
	   sprintf( buf, "%s is a lousy thief!", ch->name );
	   break;
        case 1 :
	   sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
		    ch->name,(ch->sex == 2) ? "her" : "his");
	   break;
	case 2 :
	    sprintf( buf,"%s tried to rob me!",ch->name );
	    break;
	case 3 :
	    sprintf(buf,"Keep your hands out of there, %s!",ch->name);
	    break;
        }
        if (!IS_AWAKE(victim))
            do_function(victim, &do_wake, "");
	if (IS_AWAKE(victim))
	    do_function(victim, &do_yell, buf );
	if ( !IS_NPC(ch) )
	{
	    if ( IS_NPC(victim) )
	    {
	        check_improve(ch,skill_lookup("steal"),FALSE,2);
		multi_hit( victim, ch, TYPE_UNDEFINED );
	    }
	    else
	    {
		sprintf(buf,"$N tried to steal from %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
		if ( !IS_SET(ch->act, PLR_THIEF) )
		{
		    SET_BIT(ch->act, PLR_THIEF);
		    send_to_char( "*** You are now a THIEF!! ***\n\r", ch );
		    save_char_obj( ch );
		}
	    }
	}

	return;
    }

    if ( !str_cmp( arg1, "coin"  )
    ||   !str_cmp( arg1, "coins" )
    ||   !str_cmp( arg1, "gold"  ) 
    ||	 !str_cmp( arg1, "silver"))
    {
	int gold, silver;

	gold = victim->gold * number_range(1, ch->level) / MAX_LEVEL;
	silver = victim->silver * number_range(1,ch->level) / MAX_LEVEL;
	if ( gold <= 0 && silver <= 0 )
	{
	    send_to_char( "You couldn't get any coins.\n\r", ch );
	    return;
	}

	ch->gold     	+= gold;
	ch->silver   	+= silver;
	victim->silver 	-= silver;
	victim->gold 	-= gold;
	if (silver <= 0)
	    sprintf( buf, "Bingo!  You got %d gold coins.\n\r", gold );
	else if (gold <= 0)
	    sprintf( buf, "Bingo!  You got %d silver coins.\n\r",silver);
	else
	    sprintf(buf, "Bingo!  You got %d silver and %d gold coins.\n\r",
		    silver,gold);

	send_to_char( buf, ch );
	check_improve(ch,skill_lookup("steal"),TRUE,2);
	return;
    }

    if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }
	
    if ( !can_drop_obj( ch, obj )
    ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
    ||   obj->level > ch->level )
    {
	send_to_char( "You can't pry it away.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	send_to_char( "You have your hands full.\n\r", ch );
	return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	send_to_char( "You can't carry that much weight.\n\r", ch );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    act("You pocket $p.",ch,obj,NULL,TO_CHAR);
    check_improve(ch,skill_lookup("steal"),TRUE,2);
    send_to_char( "Got it!\n\r", ch );
    return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    /*char buf[MAX_STRING_LENGTH];*/
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
	if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
	    break;
    }

    if ( pShop == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return NULL;
    }

    /*
     * Undesirables.
     *
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) )
    {
	do_function(keeper, &do_say, "Killers are not welcome!");
	sprintf(buf, "%s the KILLER is over here!\n\r", ch->name);
	do_function(keeper, &do_yell, buf );
	return NULL;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_THIEF) )
    {
	do_function(keeper, &do_say, "Thieves are not welcome!");
	sprintf(buf, "%s the THIEF is over here!\n\r", ch->name);
	do_function(keeper, &do_yell, buf );
	return NULL;
    }
	*/
    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour )
    {
	do_function(keeper, &do_say, "Sorry, I am closed. Come back later.");
	return NULL;
    }
    
    if ( time_info.hour > pShop->close_hour )
    {
	do_function(keeper, &do_say, "Sorry, I am closed. Come back tomorrow.");
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	do_function(keeper, &do_say, "I don't trade with folks I can't see.");
	return NULL;
    }

    return keeper;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper( OBJ_DATA *obj, CHAR_DATA *ch )
{
    OBJ_DATA *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
    {
	t_obj_next = t_obj->next_content;

	if (obj->pIndexData == t_obj->pIndexData 
	&&  !str_cmp(obj->short_descr,t_obj->short_descr))
	{
	    /* if this is an unlimited item, destroy the new one */
	    if (IS_OBJ_STAT(t_obj,ITEM_INVENTORY))
	    {
		extract_obj(obj);
		return;
	    }
	    obj->cost = t_obj->cost; /* keep it standard */
	    break;
	}
    }

    if (t_obj == NULL)
    {
	obj->next_content = ch->carrying;
	ch->carrying = obj;
    }
    else
    {
	obj->next_content = t_obj->next_content;
	t_obj->next_content = obj;
    }

    obj->carried_by      = ch;
    obj->in_room         = NULL;
    obj->in_obj          = NULL;
    ch->carry_number    += get_obj_number( obj );
    ch->carry_weight    += get_obj_weight( obj );
}

/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper( CHAR_DATA *ch, CHAR_DATA *keeper, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;
 
    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content )
    {
        if (obj->wear_loc == WEAR_NONE
        &&  can_see_obj( keeper, obj )
	&&  can_see_obj(ch,obj)
        &&  is_name( arg, obj->name ) )
        {
            if ( ++count == number )
                return obj;
	
	    /* skip other objects of the same name */
	    while (obj->next_content != NULL
	    && obj->pIndexData == obj->next_content->pIndexData
	    && !str_cmp(obj->short_descr,obj->next_content->short_descr))
		obj = obj->next_content;
        }
    }
 
    return NULL;
}

int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int cost;

    if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
	return 0;

    if ( fBuy )
    {
	cost = obj->cost * pShop->profit_buy  / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	int itype;

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
	    for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	    {
	    	if ( obj->pIndexData == obj2->pIndexData
		&&   !str_cmp(obj->short_descr,obj2->short_descr) )
                {
	 	    if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
			cost /= 2;
		    else
                    	cost = cost * 3 / 4;
                }
	    }
    }

    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
    {
	if (obj->value[1] == 0)
	    cost /= 4;
	else
	    cost = cost * obj->value[2] / obj->value[1];
    }

    return cost;
}



void do_buy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cost,roll;
    

    if ( argument[0] == '\0' )
    {
	send_to_char( "Buy what?\n\r", ch );
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_MOUNT_SHOP) )
    {
        do_buy_mount( ch, argument );
    	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP))
    {
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	smash_tilde(argument);

	if ( IS_NPC(ch) )
	    return;

	argument = one_argument(argument,arg);

	/* hack to make new thalos pets work */
	if (ch->in_room->vnum == 9621)
	    pRoomIndexNext = get_room_index(9706);
	else
	    pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet = get_char_room( ch, NULL, arg );
	ch->in_room = in_room;

	if ( pet == NULL || !IS_SET(pet->act, ACT_PET) )
	{
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}
        pet->hometown = ch->hometown;

	if ( ch->pet != NULL )
	{
	    send_to_char("You already own a pet.\n\r",ch);
	    return;
	}

 	cost = 10 * pet->level * pet->level;

	if ( (ch->silver + 100 * ch->gold) < cost )
	{
	    send_to_char( "You can't afford it.\n\r", ch );
	    return;
	}

	if ( ch->level < pet->level )
	{
	    send_to_char(
		"You're not powerful enough to master this pet.\n\r", ch );
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (roll < get_skill(ch,skill_lookup("haggle")))
	{
	    cost -= cost / 2 * roll / 100;
	    sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
	    send_to_char(buf,ch);
	    check_improve(ch,skill_lookup("haggle"),TRUE,4);
	
	}

	deduct_cost(ch,cost);
	pet			= create_mobile( pet->pIndexData );
	SET_BIT(pet->act, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;

	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' )
	{
	    sprintf( buf, "%s %s", pet->name, arg );
	    free_string( pet->name );
	    pet->name = str_dup( buf );
	}

	sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
	    pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );

	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	pet->leader = ch;
	ch->pet = pet;
        free_string( ch->powner );
        pet->powner = str_dup( ch->name );
        if ( pet->xp_tolevel == 0 )
        pet->xp_tolevel = 1000;
	send_to_char( "Enjoy your pet.\n\r", ch );
	act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
        rnd_sound( 2, ch, "buy.wav", "coins.wav" );
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj,*t_obj;
	char arg[MAX_INPUT_LENGTH];
	int number, count = 1;

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

	number = mult_argument(argument,arg);
	obj  = get_obj_keeper( ch,keeper, arg );
	cost = get_cost( keeper, obj, TRUE );

	if (number < 1 || number > 99)
	{
	    act("$n tells you 'Get real!",keeper,NULL,ch,TO_VICT);
	    return;
	}

	if ( cost <= 0 || !can_see_obj( ch, obj ) )
	{
	    act( "$n tells you 'I don't sell that -- try 'list''.",
		keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if (!IS_OBJ_STAT(obj,ITEM_INVENTORY))
	{
	    for (t_obj = obj->next_content;
	     	 count < number && t_obj != NULL; 
	     	 t_obj = t_obj->next_content) 
	    {
	    	if (t_obj->pIndexData == obj->pIndexData
	    	&&  !str_cmp(t_obj->short_descr,obj->short_descr))
		    count++;
	    	else
		    break;
	    }

	    if (count < number)
	    {
	    	act("$n tells you 'I don't have that many in stock.",
		    keeper,NULL,ch,TO_VICT);
	    	ch->reply = keeper;
	    	return;
	    }
	}

	if ( (ch->silver + ch->gold * 100) < cost * number )
	{
	    if (number > 1)
		act("$n tells you 'You can't afford to buy that many.",
		    keeper,obj,ch,TO_VICT);
	    else
	    	act( "$n tells you 'You can't afford to buy $p'.",
		    keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}
	
	if ( ch->level < obj->level && number_classes( ch ) <= 1 )
	{
	    act( "$n tells you 'You can't use $p yet'.",
		keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
            if ( ch->sex == 1 )
            sound( "nskilled.wav", ch );
            else
            sound( "nskilledf.wav", ch );
	    return;
	}

        else  if ( ch->level < obj->level - 5 && number_classes( ch ) == 2 )
        {
            act( "$n tells you 'You can't use $p yet'.",
                keeper, obj, ch, TO_VICT );
            ch->reply = keeper;
            if ( ch->sex == 1 )
            sound( "nskilled.wav", ch );
            else
            sound( "nskilledf.wav", ch );
            return;
        }      

        else  if ( ch->level < obj->level - 10 && number_classes( ch ) == 3 )
        {
            act( "$n tells you 'You can't use $p yet'.",
                keeper, obj, ch, TO_VICT );
            ch->reply = keeper;
            if ( ch->sex == 1 )
            sound( "nskilled.wav", ch );
            else
            sound( "nskilledf.wav", ch );
            return;
        }    

        else  if ( ch->level < obj->level - 15 && number_classes( ch ) == 4 )
        {
            act( "$n tells you 'You can't use $p yet'.",
                keeper, obj, ch, TO_VICT );
            ch->reply = keeper;
            if ( ch->sex == 1 )
            sound( "nskilled.wav", ch );
            else
            sound( "nskilledf.wav", ch );
            return;
        }    

	if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch))
	{
	    send_to_char( "You can't carry that many items.\n\r", ch );
            if ( ch->sex == 1 )
            sound( "armsfull.wav", ch );
            else
            sound( "armsfullf.wav", ch );
	    return;
	}

	if ( ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch))
	{
	    send_to_char( "You can't carry that much weight.\n\r", ch );
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) 
	&& roll < get_skill(ch,skill_lookup("haggle")))
	{
	    cost -= obj->cost / 2 * roll / 100;
	    act("You haggle with $N.",ch,NULL,keeper,TO_CHAR);
	    check_improve(ch,skill_lookup("haggle"),TRUE,4);
	}

	if (number > 1)
	{
            rnd_sound( 2, ch, "buy.wav", "coins.wav" );
	    sprintf(buf,"$n buys $p[%d].",number);
	    act(buf,ch,obj,NULL,TO_ROOM);
	    sprintf(buf,"You buy $p[%d] for %d silver.",number,cost * number);
	    act(buf,ch,obj,NULL,TO_CHAR);
	}
	else
	{
            rnd_sound( 2, ch, "buy.wav", "coins.wav" );
	    act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
	    sprintf(buf,"You buy $p for %d silver.",cost);
	    act( buf, ch, obj, NULL, TO_CHAR );
	}
	deduct_cost(ch,cost * number);
	keeper->gold += cost * number/100;
	keeper->silver += cost * number - (cost * number/100) * 100;

	for (count = 0; count < number; count++)
	{
	    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    	t_obj = create_object( obj->pIndexData, obj->level );
	    else
	    {
		t_obj = obj;
		obj = obj->next_content;
	    	obj_from_char( t_obj );
	    }

	    if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj,ITEM_HAD_TIMER))
	    	t_obj->timer = 0;
	    REMOVE_BIT(t_obj->extra_flags,ITEM_HAD_TIMER);
	    obj_to_char( t_obj, ch );
	    if (cost < t_obj->cost)
	    	t_obj->cost = cost;
                t_obj->size = ch->size;
	}
    }
}



void do_list( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	ROOM_INDEX_DATA *pRoomIndexNext;
	CHAR_DATA *pet;
	bool found;

        /* hack to make new thalos pets work */
        if (ch->in_room->vnum == 9621)
            pRoomIndexNext = get_room_index(9706);
        else
            pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );

	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	found = FALSE;
	for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
	{
	    if ( IS_SET(pet->act, ACT_PET) 
		|| IS_SET(pet->act, ACT_MOUNT) )
	    {
		if ( !found )
		{
		    found = TRUE;
		    if (IS_SET(pet->act, ACT_PET))
		        send_to_char( "Pets for sale:\n\r\n\r", ch );
		    else if (IS_SET(pet->act, ACT_MOUNT))
		        send_to_char( "Mounts for sale:\n\r\n\r", ch );

		sprintf( buf, "LVL      Price      Pet type     Exp to Level\n\r");
		send_to_char( buf, ch );
                send_to_char( "{B============================================={x\n\r", ch );

		}

                if ( pet->xp_tolevel == 0 )
                pet->xp_tolevel = 1000;
    
                if ( IS_MXP( ch ) )
                {
		sprintf( buf, MXP_SECURE "<send href=\" buy %s\" hint=\"Buy|buy\">[%3d] %6d   %14s      %4d</Send>\n\r" MXP_LLOCK,
		    pet->name, pet->level,
		    10 * pet->level * pet->level,
		    pet->short_descr, pet->xp_tolevel );
		send_to_char( buf, ch );
                }
                else
                {
		sprintf( buf, "[%3d] %6d   %14s      %4d\n\r",
		    pet->level,
		    10 * pet->level * pet->level,
		    pet->short_descr, pet->xp_tolevel );
		send_to_char( buf, ch );
                }
	    }
	}
	if ( !found )
        {
       if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
           send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
           
       else
           send_to_char( "Sorry, we're out of mounts right now.\n\r", ch );
  	   return;
        }
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost,count;
	bool found;
	char arg[MAX_INPUT_LENGTH];

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;
        one_argument(argument,arg);

	found = FALSE;
	for ( obj = keeper->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj( ch, obj )
	    &&   ( cost = get_cost( keeper, obj, TRUE ) ) > 0 
	    &&   ( arg[0] == '\0'  
 	       ||  is_name(arg,obj->name) ))
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "Lv  Price Qty  Item\n\r", ch );
                    send_to_char( "{G====================================={x\n\r", ch );
		}

		if (IS_OBJ_STAT(obj,ITEM_INVENTORY))
                {
                    if ( IS_MXP( ch ) )
		    sprintf(buf,MXP_SECURE "<send href=\"buy %s\" hint=\"Buy|buy\">[%2d %5d -- ] %s</Send>\n\r" MXP_LLOCK,
			obj->name, obj->level,cost,obj->short_descr);
                    else
		    sprintf(buf,"[%2d %5d -- ] %s\n\r",
			obj->level,cost,obj->short_descr);
                }
		else
		{
		    count = 1;

		    while (obj->next_content != NULL 
		    && obj->pIndexData == obj->next_content->pIndexData
		    && !str_cmp(obj->short_descr,
			        obj->next_content->short_descr))
		    {
			obj = obj->next_content;
			count++;
		    }
      
                    if ( IS_MXP( ch ) )
		    sprintf(buf,MXP_SECURE"<send href=\"buy %s\" hint=\"Buy|buy\">[%2d %5d %2d ] %s</Send>\n\r" MXP_LLOCK,
			obj->name, obj->level,cost,count,obj->short_descr);
                    else
                    sprintf(buf,"[%2d %5d %2d ] %s\n\r",
			obj->level,cost,count,obj->short_descr);
		}
		send_to_char( buf, ch );
	    }
	}

	if ( !found )
	    send_to_char( "You can't buy anything here.\n\r", ch );
	return;
    }
}



void do_sell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost,roll;
    OBJ_DATA *obj_next;
    int tcost = 0, count = 0;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Sell what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;
   
  if (!str_cmp(arg, "all")) 
  { 
     
    for (obj = ch->carrying; obj != NULL; obj = obj_next) 
    { 
        obj_next = obj->next_content; 
         
        if (can_drop_obj(ch, obj) 
        && obj->wear_loc == WEAR_NONE 
        && can_see_obj(keeper, obj) 
        && ((cost = get_cost(keeper, obj, FALSE)) > 0) 
        && (cost < keeper->gold)) 
        {

        tcost += cost; 
        count++;  
        ch->gold += cost; 
        deduct_cost(keeper, cost); 
         
        if (keeper->gold < 0) 
            keeper->gold = 0; 
             
        if (obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT)) 
        { 
            extract_obj(obj);  
        } 

        else 
        { 
            obj_from_char(obj); 
            if (obj->timer) 
            SET_BIT(obj->extra_flags, ITEM_HAD_TIMER); 
            else 
            obj->timer = number_range(50, 100); 
            obj_to_keeper(obj, keeper); 
        }    
         
        }           // salable item ifcheck                        
         
    }           // for loop 
     
    sprintf(buf, "You sell %d items for %d gold.\r\n", count, tcost); 
    send_to_char(buf, ch); 
    return; 
    }       

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }
    if ( cost > (keeper-> silver + 100 * keeper->gold) )
    {
	act("$n tells you 'I'm afraid I don't have enough wealth to buy $p.",
	    keeper,obj,ch,TO_VICT);
	return;
    }

    if (IS_OBJ_STAT(obj, ITEM_QUEST))
    {
	send_to_char("You should sell that to the questor instead!\n\r", ch);
	return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
    /* haggle */
    roll = number_percent();
    if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) && roll < get_skill(ch,skill_lookup("haggle")))
    {
        send_to_char("You haggle with the shopkeeper.\n\r",ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
	cost = UMIN(cost,(keeper->silver + 100 * keeper->gold));
        check_improve(ch,skill_lookup("haggle"),TRUE,4);
    }
    sprintf( buf, "You sell $p for %d silver and %d gold piece%s.",
	cost - (cost/100) * 100, cost/100, cost == 1 ? "" : "s" );
    act( buf, ch, obj, NULL, TO_CHAR );
    ch->gold     += cost/100;
    ch->silver 	 += cost - (cost/100) * 100;
    deduct_cost(keeper,cost);
    if ( keeper->gold < 0 )
	keeper->gold = 0;
    if ( keeper->silver< 0)
	keeper->silver = 0;

    if ( obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
    {
	extract_obj( obj );
    }
    else
    {
	obj_from_char( obj );
	if (obj->timer)
	    SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	else
	    obj->timer = number_range(50,100);
	obj_to_keeper( obj, keeper );
    }

    return;
}



void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Value what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    sprintf( buf, 
	"$n tells you 'I'll give you %d silver and %d gold coins for $p'.", 
	cost - (cost/100) * 100, cost/100 );
    act( buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}

void do_second (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH]; /* overkill, but what the heck */

    if ( !IS_NPC(ch)
    &&  !can_use_skpell( ch, skill_lookup("dual wield") ) )
    {
        send_to_char(
            "You don't know how to dual wield.\n\r", ch );
        return;
    }

    if (argument[0] == '\0') /* empty */
    {
        send_to_char ("Wear which weapon in your off-hand?\n\r",ch);
        return;
    }

    obj = get_obj_carry (ch, argument, ch); 

    if (obj == NULL)
    {
        send_to_char ("You have no such thing in your backpack.\n\r",ch);
        return;
    }
 
    if (!wear_obj_size(ch, obj))
        return;       

   if ( obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC )
   {
       send_to_char ("You can't dual wield a corpse.\n\r", ch );
       return;
   }

   if ( (get_eq_char (ch,WEAR_SHIELD) != NULL) ||
         (get_eq_char (ch,WEAR_HOLD)   != NULL) )
   {
        send_to_char ("You cannot use a secondary weapon while using a shieldor holding an item\n\r",ch);
        return;
   }

   if ( ch->level < obj->level && number_classes(ch) <= 1 )
   {
        sprintf( buf, "You must be level %d to use this object.\n\r",obj->level );
        send_to_char( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.",
            ch, obj, NULL, TO_ROOM );
        return;
   }

   if ( ch->level < obj->level -5 && number_classes(ch) == 2 )
   {
        sprintf( buf, "You must be level %d to use this object.\n\r",obj->level - 5 );
        send_to_char( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.",
            ch, obj, NULL, TO_ROOM );
        return;
   }           

   if ( ch->level < obj->level -10 && number_classes(ch) == 3 )
   {
        sprintf( buf, "You must be level %d to use this object.\n\r",obj->level - 10 );
        send_to_char( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.",
            ch, obj, NULL, TO_ROOM );
        return;
   }    

   if ( ch->level < obj->level -15 && number_classes(ch) == 4 )
   {
        sprintf( buf, "You must be level %d to use this object.\n\r",obj->level - 15 );
        send_to_char( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.",
            ch, obj, NULL, TO_ROOM );
        return;
   }        

    if (get_eq_char (ch, WEAR_WIELD) == NULL) 
    {
        send_to_char ("You need to wield a primary weapon, before using a secondary one!\n\r",ch);
        return;
    }

    if (!remove_obj(ch, WEAR_SECONDARY, TRUE)) 
        return;                                

    if ( get_skill(ch,skill_lookup("dual wield")) > number_percent())   
    {
        if ( chance( 10 ) && IS_OBJ2_STAT( obj, ITEM_RELIC ) )
        {                                       
            printf_to_char( ch, "%s whispers Ok lets rock!\n\r", obj->name ); 
        }
      check_improve(ch,skill_lookup("dual wield"),TRUE,3);   
      act ("$n wields $p in $s off-hand.",ch,obj,NULL,TO_ROOM);
      act ("You wield $p in your off-hand.",ch,obj,NULL,TO_CHAR);
      equip_char ( ch, obj, WEAR_SECONDARY);
      return;
    }
  
    else
    {
      send_to_char("You fail to dual wield.\n\r",ch);
      check_improve(ch,skill_lookup("dual wield"),FALSE,3);
    }            
 
    return;
}

void do_study( CHAR_DATA *ch, char *argument ) /* study by Absalom */
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn = 0;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Study what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_STAFF && obj->item_type != ITEM_WAND &&
	obj->item_type != ITEM_SCROLL )
    {
	send_to_char( "You can only study scrolls, wands, and staves.\n\r", ch );
	return;
    }

    act( "$n studies $p.", ch, obj, NULL, TO_ROOM );
    act( "You study $p.", ch, obj, NULL, TO_CHAR );

    if (ch->level < obj->level)
	{
	send_to_char("You cannot glean any knowledge from it.\n\r",ch);
	act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
	extract_obj( obj );
	}

    if (obj->item_type == ITEM_STAFF)
	{
	sn = obj->value[3];
	if ( sn < 0 || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
	    {
	    bug( "Do_study: bad sn %d.", sn );
	    return;
	    }
	if ( number_percent() >= 20 + get_skill(ch,skill_lookup("staves")) * 4/5)
	    {
	    send_to_char("You cannot glean any knowledge from it.\n\r",ch);
	    check_improve(ch,skill_lookup("staves"),FALSE,2);
	    act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
	    extract_obj( obj );
	    return;
	    }
	if ( ch->pcdata->learned[sn])
	    {
	    send_to_char("You already know that spell!\n\r",ch);
	    return;
	    }
	ch->pcdata->learned[sn] = 1;
	act("You have learned the art of $t!",
	ch,skill_table[sn].name,NULL,TO_CHAR);
	check_improve(ch,skill_lookup("staves"),TRUE,2);
	act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
	extract_obj( obj );
	return;
	}

    if (obj->item_type == ITEM_WAND)
	{
	sn = obj->value[3];
	if ( sn < 0 || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
	    {
	    bug( "Do_study: bad sn %d.", sn );
	    return;
	    }
	if ( number_percent() >= 20 + get_skill(ch,skill_lookup("wands")) * 4/5)
	    {
	    send_to_char("You cannot glean any knowledge from it.\n\r",ch);
	    check_improve(ch,skill_lookup("wands"),FALSE,2);
	    act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
	    extract_obj( obj );
	    return;
	    }
	if ( ch->pcdata->learned[sn])
	    {
	    send_to_char("You already know that spell!\n\r",ch);
	    return;
	    }
	ch->pcdata->learned[sn] = 1;
	act("You have learned the art of $t!",
	ch,skill_table[sn].name,NULL,TO_CHAR);
	check_improve(ch,skill_lookup("wands"),TRUE,2);
	act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
	extract_obj( obj );
	return;
	}

    if (obj->item_type == ITEM_SCROLL)
	{
	sn = obj->value[1];
	if ( sn < 0 || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
	    {
	    bug( "Do_study: bad sn %d.", sn );
	    return;
	    }
	if ( number_percent() >= 20 + get_skill(ch,skill_lookup("scrolls")) * 4/5)
	    {
	    send_to_char("You cannot glean any knowledge from it.\n\r",ch);
	    check_improve(ch,skill_lookup("scrolls"),FALSE,2);
	    act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
	    extract_obj( obj );
	    return;
	    }
	if ( ch->pcdata->learned[sn])
	    {
	    send_to_char("You already know that spell!\n\r",ch);
	    return;
	    }
	ch->pcdata->learned[sn] = 1;
	act("You have learned the art of $t!",
	ch,skill_table[sn].name,NULL,TO_CHAR);
	check_improve(ch,skill_lookup("scrolls"),TRUE,2);
	act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
	extract_obj( obj );
	return;
	}
}


void do_donate( CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *obj;
   char arg[MAX_INPUT_LENGTH];
   
   argument = one_argument(argument, arg);
   
   if (arg[0] == '\0' )
   {
      send_to_char("Donate what?\n\r",ch);
      return;
   }
   
   if (ch->position == POS_FIGHTING)
   {
      send_to_char("You're fighting!\n\r",ch);
      return;
   }
   
   if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
   {
      send_to_char("You do not have that!\n\r",ch);
      return;
   }
   
   if (!can_drop_obj(ch, obj))
   {
      send_to_char("It's stuck to you.\n\r",ch);
      return;
   }
   
   if (obj->item_type == ITEM_CORPSE_NPC ||
       obj->item_type == ITEM_CORPSE_PC  ||
       obj->owner     != NULL            ||
       IS_OBJ_STAT(obj,ITEM_MELT_DROP))
   {
      send_to_char("You cannot donate that!\n\r",ch);
      return;
   }
   
   if (IS_SET(ch->in_room->room_flags, ROOM_DONATION))
   {
      send_to_char("You're already here, just drop it.\n\r",ch);
      return;
   }
   
   act("$n donates $p.",ch,obj,NULL,TO_ROOM);
   act("You donate $p.",ch,obj,NULL,TO_CHAR);
   
   if (obj->timer)
      SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
   else
      obj->timer = number_range(100,200);

   obj->cost = 0;
   
   obj_from_char(obj);

   obj_to_room(obj, get_room_index(hometown_table[ch->hometown].donation));

   return;
}


void do_use( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH]; 
	OBJ_DATA  *obj;
	int to_hp;
	int to_mn;
	int to_mv;

	argument = one_argument( argument, arg );

	if ( ( arg[0] == '\0' ) )
	{
		send_to_char( "Syntax:  use (item)\n\r", ch );
		return;
	}

	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    	{
		send_to_char( "You do not have that item.\n\r", ch );
		return;
    	}
	
	if ( obj->item_type != ITEM_SOURCE )
	{
		send_to_char( "That isn't a source.\n\r", ch );
		return;
	}

	to_hp = obj->value[0];
	to_mn = obj->value[1];
	to_mv = obj->value[2];

        ch->max_hit  = ( to_hp + ch->max_hit );
        ch->max_mana = ( to_mn + ch->max_mana );
        ch->max_move = ( to_mv + ch->max_move );

	sprintf( buf, "%s brightly glows green and disappears.\n\r", obj->short_descr );	
	send_to_char( buf, ch );
	extract_obj( obj );
}


void do_insert(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int number = number_range( 1, 100 );
    int num;
   
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Insert what where??\n\r", ch );
        act( "$n looks at the slot machine with lust!", ch, NULL, NULL, TO_ROOM);
        return;
    }

    if (!str_prefix( arg, "normal" ) )
    {
       if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
       {
           send_to_char( "You don't have anything like that.\n\r", ch);
           act( "$n scans the floor for a {Ytoken{0.", ch, NULL, NULL, TO_ROOM);
           return;
       }

       if (!IS_SET( ch->in_room->room_flags, ROOM_CASINO ) )
       {
           send_to_char( "You need a slot machine to play.\n\r", ch);
           act( "$n scans the area for a Casino.", ch, NULL, NULL, TO_ROOM);
           return;
       }

       obj_from_char( obj );
       extract_obj( obj );
       WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
       
       if (chance(1))
       {
          int pracreward, expreward, trainrew, goldrew;

          switch( number_range(0,1))
          {
             case 0:
             pracreward = number_range(2,8);
             expreward = number_range(300,500);
             trainrew = number_range(1,4);
             goldrew = number_range(1000,2000);

             send_to_char("You hit the jackpot!!\n\r", ch);
             sprintf(buf, "You win %d gold, %d practices, %d train, and gain %d experience points!\n\r", goldrew, pracreward, trainrew, expreward );
             send_to_char(buf,ch);
             info( ch, 0, "{G[INFO]:{x {R%s just hit the jackpot! It was worth %d gold, %d practices, %d trains, and %d exp!{x\n\r", ch->name, goldrew, pracreward, trainrew, expreward );
             ch->gold += goldrew;
             gain_exp( ch, expreward );
             ch->train += trainrew;
             ch->practice += pracreward;
             if (ch->exp += expreward <= 0 )
             {
                 gain_exp( ch, 1 );
                 advance_level( ch, TRUE);
                 return;
             }
             break;

             case 1:
             send_to_char("Omigod, you just won an experience token!\n\r", ch );
             act( "$n just won an experience token!", ch, NULL, NULL, TO_ROOM);
             obj = create_object( get_obj_index( OBJ_VNUM_EXPERIENCE), 0 );
             obj_to_char( obj, ch );
             break;
          }
       }
      
       else if (chance( 1) && number <= 2) 
       {
             printf_to_char( ch, "You just won the Supreme jackpot! Here is your unique item!\n\r" );
             info( ch, 0, "{G[INFO]:{x %s just won the {cSup{wreme {cjack{wpot{x and a unique piece of equipment!\n\r", ch->name );
             num = ( number_range( 0, MAX_ITEM -1 ) );
             obj = create_object( get_obj_index(gamble_table[num].vnum ), ch->level );
             obj->level = ch->level;
             obj_to_char( obj, ch );
             printf_to_char( ch, "The innkeeper hands you %s as your prize.\n\r", obj->name );
             return;
       }

       else if (chance(10))
       {
              send_to_char( "Woohoo! You just won a practice token!\n\r", ch);
              act( "$n just won a practice token!", ch, NULL, NULL, TO_ROOM);
              obj = create_object( get_obj_index( OBJ_VNUM_PRACTICE), 0 );
              obj_to_char( obj, ch );
              return;
       }
       else if (chance(5))
       {
           switch(number_range(0,1))
           {
              case 0:
              send_to_char( "Holy cow, you just won a quest token!\n\r", ch);
              act( "$n just won a quest token!", ch, NULL, NULL, TO_ROOM);
              obj = create_object( get_obj_index( OBJ_VNUM_QUEST ), 0 );
              obj_to_char( obj, ch );            
              break;

              case 1:
              send_to_char( "Whoopie! You just won a train token!\n\r", ch);
              act( "$n just won a train token!", ch, NULL, NULL, TO_ROOM);
              obj = create_object( get_obj_index( OBJ_VNUM_TRAIN ), 0 );
              obj_to_char( obj, ch );
              break;
           }
       }
       else
       {
            int goldrew;

            switch(number_range(0,2))
            {
               case 0:
               send_to_char( "Poit! You lost.\n\r", ch );
               act( "$n kicks the slot machine!", ch, NULL, NULL, TO_ROOM);
               break;
   
               case 1:
               goldrew = number_range( 20, 500 );
               sprintf( buf, "You won %d gold!\n\r", goldrew);
               act( "$n wins some gold!", ch, NULL, NULL, TO_ROOM);
               ch->gold += goldrew;
               send_to_char( buf, ch );
               break;

               case 2: 
               send_to_char( "Poit! You lost.\n\r", ch ); 
               act( "$n kicks the slot machine!", ch, NULL, NULL, TO_ROOM);
            }
       }
    }
    else if (!str_prefix( arg, "experience" ))
    {
       if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
       {
            send_to_char( "You don't have any tokens like that.\n\r", ch );
            act( "$n scans the floor for experience tokens.", ch, NULL, NULL, TO_ROOM);
            return;
       }

       if (!IS_SET(ch->in_room->room_flags, ROOM_CASINO ) )
       {
           send_to_char( "You need a slot machine to play.\n\r", ch );
           act( "$n looks around for the Casino!", ch, NULL, NULL, TO_ROOM);
           return;
       }

       obj_from_char( obj );
       extract_obj( obj );

     if (chance(10))
     {
      int expreward;

      switch(number_range(0,1))
      {
         case 0:
         expreward = number_range(30,100);
         
         sprintf(buf,"You win %d experience points!\n\r", expreward );
         send_to_char(buf,ch);
         act( "$n just won some experience points!", ch, NULL, NULL, TO_ROOM);
         gain_exp( ch, expreward);
         if ( ch->exp += expreward <= 0)
         {
               gain_exp( ch, 1 );
               advance_level( ch, TRUE );
               return;
         }
         break;

         case 1:

         expreward = number_range(101,300);

         sprintf(buf,"You win %d experience points!\n\r", expreward );
         send_to_char(buf,ch);
         act( "$n just won experience points! Big Winner!", ch, NULL, NULL, TO_ROOM);
         gain_exp( ch, expreward );
         if (ch->exp += expreward <= 0)
         {
               gain_exp( ch, 1 );
               advance_level( ch, TRUE );
               return;
         }
         break;
      }
     }
    }
    else if (!str_prefix( arg, "practice" ) )
    {
       if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
       {
           send_to_char( "You don't have anything like that.\n\r", ch);
           act( "$n scans the floor of the Casino for a practice token!", ch, NULL, NULL, TO_ROOM);
           return;
       }

       if (!IS_SET(ch->in_room->room_flags, ROOM_CASINO) )
       {
           send_to_char("You need a slot machine to play.\n\r",ch);
           act( "$n look around for the Casino!", ch, NULL, NULL, TO_ROOM);
           return;
       }

       obj_from_char(obj);
       extract_obj(obj);

       if (chance(5))
       {
          int pracreward;

          switch( number_range(0,2))
          {
             case 0:
             pracreward = number_range(2,10);

             sprintf(buf,"You win %d practices!\n\r", pracreward);
             send_to_char(buf,ch);
             act( "$n just won some practices!", ch, NULL, NULL, TO_ROOM);
             ch->practice += pracreward;
             break;

             case 1:

             send_to_char( "Whoopie! You just won a train token!\n\r",ch);
             act( "$n just won a train token!", ch, NULL, NULL, TO_ROOM);
             obj = create_object( get_obj_index( OBJ_VNUM_TRAIN ), 0 );
             obj_to_char( obj, ch );
             break;

             case 2:

             send_to_char( "Holy cow, you just won a quest token!\n\r", ch);
             act( "$n just won a quest token!", ch, NULL, NULL, TO_ROOM);
             obj = create_object( get_obj_index( OBJ_VNUM_QUEST ), 0 );
             obj_to_char( obj, ch );
             break;

          }
       }
       else if (chance(10))
       {
         send_to_char(" Omigod, you just won an experience token!\n\r", ch);
         act( "$n just won an experience token!", ch, NULL, NULL, TO_ROOM);
         obj = create_object( get_obj_index( OBJ_VNUM_EXPERIENCE ), 0);
         obj_to_char( obj, ch );
         return;
       }
       else
       {
           switch(number_range(0,1))
           {
             case 0:

             send_to_char("Crap, you lost your practice token.\n\r",ch);
             act( "$n kicks the slot machine!", ch, NULL, NULL, TO_ROOM);
             break;

             case 1:

             send_to_char("Darn, you just won back a normal token.\n\r",ch);
             act( "$n glares at the slot machine!", ch, NULL, NULL, TO_ROOM);
             obj = create_object( get_obj_index( OBJ_VNUM_NORMAL ), 0 );
             obj_to_char( obj,ch );
             break;
           }
       }
    }
    else if(!str_prefix( arg, "train" ) )
    {
       if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
       {
          send_to_char("You don't have anything like that.\n\r", ch);
          act( "$n scans the floor of the Casino for a train token!", ch, NULL, NULL, TO_ROOM);
          return;
       }

       if (!IS_SET(ch->in_room->room_flags, ROOM_CASINO ) )
       {
           send_to_char( "You need a slot machine to play.\n\r", ch);
           act( "$n looks around for the Casino!", ch, NULL, NULL, TO_ROOM);
           return;
       }

       obj_from_char( obj );
       extract_obj(obj);

       if (chance(5))
       {
          int trainrew;

          switch( number_range(0,1))
          {
             case 0:
             trainrew = number_range(1,5);
             sprintf(buf,"You win %d training sessions.\n\r", trainrew );
             send_to_char(buf,ch);
             act( "$n just won some trains!", ch, NULL, NULL, TO_ROOM);
             ch->train += trainrew;
             break;

             case 1:

             send_to_char("Holy cow, you just won a quest token!\n\r",ch);
             act( "$n just won a quest token!", ch, NULL, NULL, TO_ROOM);
             obj = create_object( get_obj_index( OBJ_VNUM_QUEST ), 0 );
             obj_to_char( obj,ch);
             break;
          }
       }
       else if (chance(10))
       {
           send_to_char("Omigod, you just won an experience token!\n\r",ch);
           act( "$n just won an experience token!", ch, NULL, NULL, TO_ROOM);
           obj = create_object( get_obj_index( OBJ_VNUM_EXPERIENCE ),0);
           obj_to_char( obj, ch );
           return;
       }
       else
       {
          switch(number_range(0,2))
          {
            case 0:

            send_to_char("Damn, you lost your train token.\n\r",ch);
            act( "$n just lost a train token!", ch, NULL, NULL, TO_ROOM);
            break;

            case 1:

            send_to_char("Darn, you just won back a normal token.\n\r",ch);
            act( "$n glares at the slot machine.", ch, NULL, NULL, TO_ROOM);
            obj = create_object( get_obj_index( OBJ_VNUM_NORMAL ),0);
            obj_to_char( obj,ch);
            break;

            case 2:

            send_to_char("Hrm, bummer, you just won back a practice token.\n\r",ch);
            act( "$n glares at the slot machine. A practice token??", ch, NULL, NULL, TO_ROOM);
            obj = create_object( get_obj_index( OBJ_VNUM_PRACTICE), 0 );
            obj_to_char( obj,ch);
            break;
          }
       }
    }
    else if(!str_prefix( arg, "quest" ))
    {
       if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
       {
          send_to_char( "You don't have anything like that.\n\r",ch);
          act( "$n scans the floor of the casino for a quest token!", ch, NULL, NULL, TO_ROOM);
          return;
       }

       if (!IS_SET( ch->in_room->room_flags, ROOM_CASINO ) )
       {
          send_to_char( "You need a slot machine to play.\n\r",ch);
          act( "$n looks around for the Casino.", ch, NULL, NULL, TO_ROOM);
          return;
       }

       obj_from_char(obj);
       extract_obj(obj);

       if (chance(5))
       {
          int questreward;

          switch(number_range(0,1))
          {
             case 0:
             questreward = number_range(2,80);

             sprintf(buf,"You win %d questpoints.\n\r", questreward);
             send_to_char(buf,ch);
             act( "$n won some quest points!", ch, NULL, NULL, TO_ROOM);
             ch->pcdata->questpoints += questreward;
             break;

             case 1:

             send_to_char("Omigod, you just won an experience token!\n\r", ch);
             act( "$n won an experience token!", ch, NULL, NULL, TO_ROOM);
             obj = create_object( get_obj_index( OBJ_VNUM_EXPERIENCE), 0 );
             obj_to_char( obj, ch );
             break;
          }
       }
       else
       {
          switch(number_range(0,3))
          {
             case 0:

             send_to_char("You just lost your quest token! DRAT!!\n\r",ch);
             act( "$n kicks the slot machine.", ch, NULL, NULL, TO_ROOM);
             break;

             case 1:

             send_to_char("Darn, you just won back a normal token.\n\r",ch);
             act( "$n glares at the slot machine.", ch, NULL, NULL, TO_ROOM);
             obj = create_object( get_obj_index( OBJ_VNUM_NORMAL ), 0);
             obj_to_char( obj, ch );
             break;

             case 2:

             send_to_char("Hrm, bummer, you just won back a practice token.\n\r",ch);
             act( "$n glares at the slot machine.", ch, NULL, NULL, TO_ROOM);
             obj = create_object( get_obj_index( OBJ_VNUM_PRACTICE ), 0);
             obj_to_char( obj, ch );
             break;

             case 3:

             send_to_char("Ick, you won back a train token.\n\r",ch);
             act( "$n glares at the slot machine.", ch, NULL, NULL, TO_ROOM);
             obj = create_object( get_obj_index( OBJ_VNUM_TRAIN ), 0);
             obj_to_char( obj, ch );
             break;
          }   
       }      
    }       
}


void do_purchase(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA  *obj;
    char arg [MAX_INPUT_LENGTH];
    char arg1 [MIL];
    char buf [MSL];
    int cost = 1000;
    int number;
    int count = 1;

    argument = one_argument(argument, arg);
    one_argument( argument, arg1 );

    if ( IS_SET(ch->comm,COMM_AFK) )
    {
        send_to_char("Not while AFK.\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' || arg1[0] == '\0' )
    {
        send_to_char( "Syntax: purchase token <amount>.", ch );
        return;
    }

    if (!IS_SET( ch->in_room->room_flags, ROOM_CASINO ) )
    {
           send_to_char( "You may only purchase tokens at the Casino.\n\r", ch);
           return;
    }

    if (!str_cmp( arg, "token" ) )
    {
        number = atoi( arg1 );
        obj = create_object( get_obj_index( OBJ_VNUM_NORMAL ), 0 );  
 
        if ( number < 1 || number > 99 )
        {
            printf_to_char( ch, "I don't think so.\n\r" );
            return;
        }
        
        if ( (ch->silver + ch->gold * 100) < cost * number )
        {
              send_to_char( "You can't afford to buy that many!\n\r", ch );
              return;
        }       

        if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch))
        {
            send_to_char( "You can't carry that many items.\n\r", ch );
            return;
        }    
        
        if ( ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch))
        {
            send_to_char( "You can't carry that much weight.\n\r", ch );
            return;
        }        
    
        if (number > 1)
        {
            sprintf( buf,"$n buys [%d] tokens.", number );
            act(buf,ch,NULL,NULL,TO_ROOM);
            sprintf(buf,"You buy [%d] tokens for %d silver.", number, cost * number);
            rnd_sound( 2, ch, "coins.wav", "buy.wav" );
            act(buf,ch,NULL,NULL,TO_CHAR);
        }
        else
        {
            send_to_char( "You have purchased a normal token for gambling.\n\r", ch );
            act( "$n purchases a normal token.", ch, NULL, NULL, TO_ROOM);
            ch->gold -= 10;      
            obj = create_object( get_obj_index( OBJ_VNUM_NORMAL ), 0 );
            obj_to_char( obj, ch );
            rnd_sound( 2, ch, "coins.wav", "buy.wav" );
            return;
        }      

        for (count = 0; count < number; count++)
        {                                              
             obj = create_object( get_obj_index( OBJ_VNUM_NORMAL ), 0 ); 
             obj_to_char( obj, ch );
        } 

        deduct_cost(ch,cost * number);   
    }
    else
    {
        send_to_char( "Syntax: purchase token <amount>.", ch );
        return;
    }

    return;
}

void do_brew ( CHAR_DATA *ch, char *argument )
{

   OBJ_DATA *obj=NULL;   
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_STRING_LENGTH];
   int chance;
   int sn;
   int value;

   one_argument( argument, arg );

   if ( IS_NPC( ch ) )
       return;

    if ( (chance = get_skill(ch,skill_lookup("brew"))) == 0
       || !can_use_skpell(ch, skill_lookup("brew")))       
    {
        send_to_char( "You don't know how to brew potions.\n\r", ch );
        return;
    }
  
  if (arg[0] == '\0')
   {
        send_to_char("What spell do you want in your potion?\n\r", ch);
        return;
   }
  
 if ((sn = find_spell(ch,arg)) < 1  
     || (skill_table[sn].spell_fun == spell_null)
     || (!IS_NPC(ch) && (!can_use_skpell( ch, sn )
     ||                  ch->pcdata->learned[sn] == 0)))
    {
        send_to_char( "You cannot brew a potion with a spell you do not know.\n\r", ch );
        return;           
   }         

 if (  get_skill(ch,skill_lookup("brew"))  <  (number_range(0, 100))  )
        {
                send_to_char("You failed to create a brewed potion.\n",ch);
                check_improve(ch,skill_lookup("brew"),FALSE,1);
                return;
        }

                
                obj = create_object(get_obj_index(OBJ_VNUM_BREW), 0);       
                sprintf( buf, obj->short_descr,arg );
                free_string( obj->short_descr );
                obj->short_descr = str_dup( buf );
                sprintf( buf, obj->description, arg );
                free_string( obj->description );
                obj->description = str_dup( buf );
            value   =     atoi( arg );

           obj->value[0]   =    ch->level;

            obj->value[1] = skill_lookup( argument );

           obj_to_char(obj,ch);
           act( "Through careful manipulation you brew a potion!", ch, NULL, NULL,TO_CHAR);
           act( "Through careful manipulation $n brews a potion!", ch, NULL, NULL, TO_ROOM);
           check_improve(ch,skill_lookup("brew"),TRUE,1);           
           sound( "brew.wav", ch );  
   return;
}


bool wear_obj_size (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (obj->size == SIZE_UNKNOWN)
        return TRUE;

    if (IS_IMMORTAL(ch))
        return TRUE;

    if (ch->level <= 20)
    {
        obj->size = ch->size;
        return TRUE;
    }
     
    if (obj->size < ch->size)
    {
        send_to_char ("That object is too small for you!\n\r",ch);
        act( "$n tries to use $p, but it is too small for them.", ch, obj, NULL, TO_ROOM );
        return FALSE;
    }                        
  
    if (obj->size > ch->size)
    {
        send_to_char ("That object is too large for you!\n\r",ch);
        act( "$n tries to use $p, but it is too large for them.", ch, obj, NULL, TO_ROOM );
        return FALSE;
    }
        
    return TRUE;
}   


void do_resize( CHAR_DATA *ch, char *argument )
{
    char *remainder;
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;
    int min = 10;

    remainder = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Resize what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
    {
	send_to_char ("There is no shopkeeper here.\n\r",ch);
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
	return;
    }
    
    if  (( cost = get_cost( keeper, obj, FALSE ) <= 0 ) ) 
    cost = min;
	
    if (obj->size == ch->size)
    {
	act( "$n that item is already fit to your size.'", keeper, NULL, ch, TO_VICT );
	return;
    }
		
    cost = (abs(ch->size - obj->size) * .1) * obj->pIndexData->cost / 8;
    remainder = one_argument (remainder, arg);
	
    if (!str_cmp (arg, "cost"))
    {
	char outbuf[128];
	sprintf (outbuf, "$n tells you 'Resizing that will cost %d gold.'",cost);
	act (outbuf, keeper, NULL, ch, TO_VICT );
	return;
    }	
	
    if (cost > ch->gold)
    {
	char outbuf[128];
	sprintf (outbuf, "$n tells you 'Resizing that will cost %d gold.'",cost);
	act (outbuf, keeper, NULL, ch, TO_VICT );
	act( "$n tells you 'You don't have enough money'.", keeper, NULL, ch, TO_VICT );
	return;
    }
	
    act( "$n takes $p and resizes it for you.", keeper, obj, NULL, TO_ROOM );
    obj->size = ch->size;
    WAIT_STATE (ch, 2*PULSE_VIOLENCE);

    ch->gold     -= cost;
    keeper->gold += cost;
    return;
}


void do_search( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA *obj;

   if(IS_NPC(ch)) return;

   if(ch->pcdata->learned[skill_lookup("search")] < 1) 
   {
   send_to_char("You search around clumsily.\n\r",ch);
   return;
   }

   if(number_percent() < ch->pcdata->learned[skill_lookup("search")] )
   {
    check_improve(ch,skill_lookup("search"),TRUE,4);
    send_to_char("You search the room..\n\r",ch); 

    for(obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
    {
    if(IS_SET(obj->extra_flags,ITEM_HIDDEN) )
    {
    printf_to_char(ch,"You reveal %s\n\r",obj->short_descr);
     REMOVE_BIT(obj->extra_flags,ITEM_HIDDEN);     
       }
      }
       send_to_char("You have searched everywhere.\n\r",ch);
       WAIT_STATE(ch,24);
      }
      else
      send_to_char("You didn't uncover anything unusual.\n\r",ch);
     check_improve(ch,skill_lookup("search"),FALSE,4);
    WAIT_STATE(ch,24);
   return;
}


void do_ohide( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj; 
  char arg[MAX_INPUT_LENGTH];  

   argument = one_argument(argument, arg );

   if(arg[0] != '\0')
    {
     if(( obj = get_obj_carry(ch, arg, ch)) == NULL)
     {
     send_to_char("You don't have that item.\n\r",ch);
     return;
     }

   if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE) )
	{
	  send_to_char( "You can't let go of it.\n\r", ch );
	  return;
	}

    obj_from_char(obj);
    SET_BIT(obj->extra_flags,ITEM_HIDDEN);
    obj->carried_by = ch;
    obj_to_room(obj,ch->in_room);
    act("$n hides $p.",ch,obj,NULL,TO_ROOM);
    act("You hide $p.",ch,obj,NULL,TO_CHAR);
    return;
   }
 else

    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[skill_lookup("hide")] )
    {
	SET_BIT(ch->affected_by, AFF_HIDE);
	check_improve(ch,skill_lookup("hide"),TRUE,3);
    }
    else
	check_improve(ch,skill_lookup("hide"),FALSE,3);

    return;
}

void do_newforge( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_STRING_LENGTH];
        char arg2[MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	OBJ_DATA  *obj;
	OBJ_DATA  *obj2;
    	AFFECT_DATA *pAf;
	int to_hit;
	int to_dam;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

	if ( ( arg[0] == '\0' ) || ( arg2[0] == '\0' ) )
	{
		send_to_char( "Syntax:  nforge (item) (weapon)\n\r", ch );
		return;
	}

	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    	{
		send_to_char( "You do not have that item.\n\r", ch );
		return;
    	}
	
	if ( ( obj2 = get_obj_carry( ch, arg2, ch ) ) == NULL )
    	{
		send_to_char( "You do not have that item.\n\r", ch );
		return;
    	}

	if ( obj->item_type != ITEM_SOCKET )
	{
		send_to_char( "That isn't a socket.\n\r", ch );
		return;
	}

	if ( obj2->item_type != ITEM_WEAPON )
	{
		send_to_char( "That is not a weapon!\n\r", ch );
		return;
	}

	to_hit = obj->value[0];
	to_dam = obj->value[1];

	extract_obj( obj );
	
	if ( to_hit != 0 )
	{
    		pAf             =   new_affect();
    		pAf->location   =   APPLY_HITROLL;
    		pAf->modifier   =   to_hit;
	    	pAf->where	=   TO_OBJECT;
    		pAf->type	=   0;
	    	pAf->duration   =   -1;
    		pAf->bitvector  =   0;
	    	pAf->level      =   obj2->level;
    		pAf->next       =   obj2->affected;
	    	obj2->affected  =   pAf;
	}

	if ( to_dam != 0 )
	{
		pAf		=  new_affect();
		pAf->location   =  APPLY_DAMROLL;
		pAf->modifier	=  to_dam;
		pAf->where	=  TO_OBJECT;
		pAf->type	=  0;
		pAf->duration	=  -1;
		pAf->bitvector  =  0;
		pAf->level	=  obj2->level;
		pAf->next	=  obj2->affected;
		obj2->affected  =  pAf;
	}

	sprintf( buf1, "{G[{W*{G]{x %s", obj2->short_descr );
        free_string( obj2->short_descr );
        obj2->short_descr = str_dup( buf1 );
	       	
	send_to_char( "You have inserted the socket into your weapon, it glows and humms for an instant and now seems much more powerful.\n\r", ch);
	return;
}


void do_draw( CHAR_DATA *ch, char *argument )
{  
    OBJ_DATA *quiver;
	OBJ_DATA *arrow;
    int hand_count= 0;
	
    if ( ( quiver = get_eq_char( ch, WEAR_SHOULDER ) ) == NULL )
    {
	send_to_char( "{WYou aren't wearing a quiver where you can get to it.{x\n\r", ch );
	return;
    }

    if ( quiver->item_type != ITEM_QUIVER )
    {
	send_to_char( "{WYou can only draw arrows from a quiver.{x\n\r", ch );
	return;
    }

   	if (get_eq_char(ch,WEAR_SHIELD)!= NULL) hand_count++; 
   	if (get_eq_char(ch,WEAR_HOLD)  != NULL) hand_count++;  
   	if (get_eq_char(ch,WEAR_WIELD) != NULL) hand_count++;

	if ( hand_count > 1)
	{
		send_to_char (	"{WYou need a free hand to draw an arrow.{x\n\r", ch );
		return;
	}
	
	if ( get_eq_char(ch, WEAR_HOLD) != NULL)
	{
	       send_to_char ( "{WYour hand is not empty!{x\n\r", ch );
	       return;
	}

    if ( quiver->value[0] > 0 )
    {
	WAIT_STATE( ch, PULSE_VIOLENCE );	
	act( "{W$n draws an arrow from $p{W.{x", ch, quiver, NULL, TO_ROOM );
	act( "{WYou draw an arrow from $p{W.{x", ch, quiver, NULL, TO_CHAR );
	
	arrow = create_object(get_obj_index(OBJ_VNUM_ARROW), 0);
	arrow->value[1] = quiver->value[1];
	arrow->value[2] = quiver->value[2];
	arrow->level    = quiver->level;
	obj_to_char(arrow,ch);
	wear_obj( ch,arrow,TRUE );
	quiver->value[0] -= quiver->value[1];
	
	
    if ( quiver->value[0] <= 0 )
    {
	act( "{WYour $p {Wis now out of arrows, you need to find another one.{x", ch, quiver, NULL, TO_CHAR );
	extract_obj(quiver);
    }

    return;
    }
}


/* Bowfire code -- Used to dislodge an arrow already lodged */
void do_dislodge( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA * arrow = NULL;
	int dam = 0;
	
    if (argument[0] == '\0') /* empty */
    {
        send_to_char ("{WDislodge what?{x\n\r",ch);
        return;
    }	
	
	if ( get_eq_char(ch, WEAR_LODGE_RIB) != NULL)
	{
	arrow = get_eq_char( ch, WEAR_LODGE_RIB );
	act( "{WWith a wrenching pull, you dislodge $p {Wfrom your chest.{x", ch, arrow, NULL, TO_CHAR );
	unequip_char( ch, arrow );
	arrow->extra_flags = arrow->extra_flags - 134217728;
	dam  =  dice((3 * arrow->value[1]), (3 * arrow->value[2]));
	damage( ch, ch, dam, skill_lookup("bow"), DAM_SLASH, TRUE );
	return;
	}	
	
	else
	if (get_eq_char(ch,WEAR_LODGE_ARM) != NULL)
	{
	arrow = get_eq_char( ch, WEAR_LODGE_ARM );
	act( "{WWith a tug you dislodge $p {Wfrom your arm.{x", ch, arrow, NULL, TO_CHAR );
	unequip_char( ch, arrow );
	arrow->extra_flags = arrow->extra_flags - 134217728;
	dam      =  dice((3 * arrow->value[1]), (2 * arrow->value[2]));
	damage( ch, ch, dam, skill_lookup("bow"), DAM_SLASH, TRUE );
	return;
	}	
	
	else
	if (get_eq_char(ch,WEAR_LODGE_LEG) != NULL)  
	{
	arrow = get_eq_char( ch, WEAR_LODGE_LEG );
	act( "{WWith a tug you dislodge $p {Wfrom your leg.{x", ch, arrow, NULL, TO_CHAR );
	unequip_char( ch, arrow );
	arrow->extra_flags = arrow->extra_flags - 134217728;
	dam      =  dice((2 * arrow->value[1]), (2 * arrow->value[2]));
	damage( ch, ch, dam, skill_lookup("bow"), DAM_SLASH, TRUE );
	return;
	}
	else
	{	
	send_to_char("{WYou have nothing lodged in your body.{x\n\r", ch);
	return;
	}
}	


void do_belt( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *belt = NULL, *item, *temp;
	int i, weight = 0; 
	char buf[MSL], arg[MSL];

	if (argument[0] == '\0')
	{
		send_to_char( "What do you wish to belt to your side?\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );

/* Instead of grabbing the obj from their waist and checking to see if it's a belt, we're going
 * to cycle through all of their eq and check for item type belt. This will allow for garters and
 * such later. In the future, add a second arg to this command to let them choose which belt they're
 * using - otherwise, it'll take the first available one. For now, it works.
 *
 * Further note - you'll have to change how belted items are flagged to make it possible to have
 * belts in places other than your waist. I can explain more later, if you want.
 */

	if ( argument[0] == '\0' )
	{
		for ( i = 0; i < MAX_WEAR; i++ )
		{
			if (( belt = get_eq_char( ch, i ) ) && belt->item_type == ITEM_BELT )
				break;
		}
	}
	else
	{
		if ( ( belt = get_obj_wear( ch, argument, TRUE ) ) == NULL )
		{
			printf_to_char( ch, "You don't seem to be wearing anything called '%s'.\n\r", argument );
			return;
		}
		if ( belt->item_type != ITEM_BELT )
		{
			printf_to_char( ch, "You cannot strap anything to %s.\n\r", belt->short_descr );
			return;
		}
	}

	if ( !belt || belt->item_type != ITEM_BELT )
	{
		send_to_char( "You are wearing nothing that you can belt an item to.\n\r", ch );
		return;
	}

	if (!(item = get_obj_carry( ch, arg, ch )))
	{
		printf_to_char( ch, "You don't seem to be carrying '%s'.\n\r", argument );
		return;
	}

	if (item->belted_vnum > 0)
	{
		printf_to_char( ch, "%s is already belted.\n\r", capitalize(item->short_descr) );
		return;
	}

	for ( temp = ch->carrying; temp; temp = temp->next_content )
	{
		if (temp->belted_vnum == belt->pIndexData->vnum)
			weight += temp->weight;
	}

	if (belt->value[0] <= weight)
	{
		printf_to_char( ch, "%s is at its weight capacity - you can belt nothing else to it.\n\r", capitalize( belt->short_descr ) );
		return;
	}

	printf_to_char( ch, "You reach down, carefully belting %s onto %s.\n\r", item->short_descr, belt->short_descr );
	sprintf( buf, "$n reaches down, belting %s to %s.", item->short_descr, belt->short_descr );
	act( buf, ch, NULL, NULL, TO_ROOM );
	item->belted_vnum = belt->pIndexData->vnum;
	return;
}

void do_sheath(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *sheath = NULL, *item, *temp, *belt = NULL, *obj;
 	int i, weight = 0; 
	int count;
	char buf[MSL], arg[MSL], arg2[MSL];
        bool found = FALSE;
	if (argument[0] == '\0')
	{
		send_to_char( "What do you wish to sheath?\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2);
	if ( arg2[0] == '\0' )
	{
	for( i = 0; i < MAX_WEAR; i++ )
		{
			if (( belt = get_eq_char( ch, i ) ) && belt->item_type == ITEM_BELT )
			{
				found = TRUE;
				break;	
			}
		}
	for( obj = ch->carrying; obj; obj = obj->next_content )
	{
		if(obj->item_type == ITEM_SHEATH && obj->strap_loc > -1)
		{
			found = TRUE;
			break;
		}
	}
      if(found)
	 {
	      for ( temp = ch->carrying; temp; temp = temp->next_content )
		{
			if (temp->item_type == ITEM_SHEATH && (temp->belted_vnum > 0 || temp->strap_loc > -1 ) )
			{
				sheath = temp;
  				break;
			}
		}
       }
	}
	else
	{
		if ( ( sheath = get_obj_carry( ch, arg2, ch ) ) == NULL )
		{
			printf_to_char( ch, "You don't seem to have sheath named '%s'.\n\r", arg2 );
			return;
		}
		if(sheath->item_type != ITEM_SHEATH )
		{
			printf_to_char( ch, "You cannot sheath a weapon in %s.\n\r", sheath->short_descr );
			return;
		}
	}

	if ( !sheath || sheath->item_type != ITEM_SHEATH )
	{
		send_to_char( "You are wearing nothing that you can sheath your weapon in.\n\r", ch );
		return;
	}

	if( (item = get_obj_wear(ch, arg, TRUE) ) == NULL )
	{
		if (!(item = get_obj_carry( ch, arg, ch )))
		{
		    printf_to_char( ch, "You don't seem to be carrying '%s'.\n\r", argument );
		    return;
		}
	}
	else
	{
		if(item == get_eq_char(ch, WEAR_WIELD) )
		{
           	   item = get_eq_char(ch, WEAR_WIELD); 
	   	   unequip_char(ch,item);
		}
		else if(item == get_eq_char(ch, WEAR_SECONDARY) )
		{
	    	    item = get_eq_char(ch, WEAR_SECONDARY); 	
	    	    unequip_char(ch,item);
   		}
	}
        if(item->item_type != ITEM_WEAPON)
	{
	    send_to_char("You can only sheath weapons!\n\r",ch);
	    return;
	}
	    unequip_char( ch, item );
	

	if (item->sheathed_in != NULL)
	{
		printf_to_char( ch, "%s is already sheathed.\n\r", capitalize(item->short_descr) );
		return;
	}
	count = 0;
	for ( temp = ch->carrying; temp; temp = temp->next_content )
	{
		if(temp->sheathed_in == sheath)
		  count++;
	}


		if(sheath->value[2] == SHEATH_NO && count == 1)
		{
	    	    send_to_char("You cannot sheath more than one weapon in that sheath.\n\r",ch);
	    	    return;
   		}

		if( count >= 2 )
		{
		    send_to_char("You cannot sheath more than two weapons in that!\n\r",ch);
		    return;
		}



	for ( temp = ch->carrying; temp; temp = temp->next_content )
	{
		if (temp->sheathed_in == sheath)
			weight += temp->weight;
	}

        obj_from_char( item );
        obj_to_obj( item, sheath );

	send_to_char("You sheath your weapon neatly into its holster.\n\r",ch);
	sprintf( buf, "$n sheaths $s weapon neatly into its holster.");
	act( buf, ch, NULL, NULL, TO_ROOM );
	return;
}

void do_unsheath(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], arg2[MSL];
    OBJ_DATA *obj, *sheath;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2);
    if ( arg[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r\tunsheath <object> <sheath>\n\r", ch );
	return;
    }

    if ( ( sheath = get_obj_carry( ch, arg2, ch ) ) == NULL )
    {
		send_to_char("Where's that sheath strapped!?\n\r",ch);
		return;
    } 

    if ( (obj = sheath->contains ) == NULL)
	{
		send_to_char("There is no weapon in that sheath.\n\r",ch);
		return;
	}

	if (obj != get_obj_sheath(ch, arg, sheath) )
	{
		printf_to_char(ch, "There is no weapon of that sort in %s\n\r", sheath->name);
		return;
	}

    if ( get_eq_char( ch, WEAR_WIELD ) != NULL && get_eq_char(ch, WEAR_SECONDARY) != NULL)
    {
		send_to_char("You do not have three arms - you can only wield two weapons!\n\r",ch);   
		return;
    }

    if( get_eq_char( ch, WEAR_WIELD ) != NULL && get_eq_char(ch, WEAR_SECONDARY) == NULL)
	{
		printf_to_char( ch, "You unsheath %s and wield it in your off hand.\n\r",obj->short_descr );
		sprintf( arg, "$n unsheaths %s and wields it in $s off hand.\n\r",obj->short_descr );
		act( arg, ch, NULL, NULL, TO_ROOM );	
		get_obj( ch, obj, obj->in_obj );
		equip_char( ch, obj, WEAR_SECONDARY);
		obj->sheathed_in = NULL;
		return;
	}				
			
	printf_to_char( ch, "You unsheath %s and wield it.\n\r", obj->short_descr );
	sprintf( arg, "$n unsheaths %s and wields it.", obj->short_descr );
	act( arg, ch, NULL, NULL, TO_ROOM );
	obj_from_obj(  obj );
    equip_char( ch, obj, WEAR_WIELD ); 
	return;
	
    
}

void do_strap( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *strap = NULL, *temp;
	int location; 
	char buf[MSL], arg[MSL], arg2[MSL];
	location = -1;
	if (argument[0] == '\0')
	{
		send_to_char( "What do you wish to strap to your body?\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

	if ( arg[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char("Syntax: strap <object> <location>\n\r",ch);
		return;
	}
	else
	{
		if (!(strap = get_obj_carry( ch, arg, ch )))
		{
			printf_to_char( ch, "You don't seem to be carrying '%s'.\n\r", argument );
			return;
		}
		if ( strap->item_type != ITEM_SHEATH )
		{
			send_to_char("You can only strap sheaths to your body.\n\r",ch);
			return;
		}
		if ( strap->value[3] != SHEATH_YES )
		{
			send_to_char("It doesn't have a strap; therefore, you cannot strap it to anything.\n\r",ch);
			return;
		}
	}

	if ( !strap || strap->item_type != ITEM_SHEATH )
	{
		send_to_char( "You are not wearing anything you can strap to your body.\n\r", ch );
		return;
	}
	if ( (location = strap_lookup(arg2) ) == -1  )
	{
	    send_to_char("You can't strap anything there!\n\r",ch);
	    return;
	}
	
	if(strap->strap_loc > -1)
	{
	    send_to_char("It is already strapped to your body.\n\r",ch);
	    return;
	}
	for ( temp = ch->carrying; temp; temp = temp->next_content )
	{
		if(temp->strap_loc == location)
		{
		    send_to_char("You already have something strapped to that location.\n\r",ch);
		    return;
		}
	}

	printf_to_char( ch, "You strap %s to your body.\n\r", strap->short_descr);
	sprintf( buf, "$n straps %s to $s body.", strap->short_descr );
	act( buf, ch, NULL, NULL, TO_ROOM );
	strap->strap_loc = location;
	return;
}

void show_strapped(CHAR_DATA *ch, CHAR_DATA *victim, int location)
{
	OBJ_DATA *temp, *sheathed;

	if(location == WEAR_BACK)
      {
	      for ( temp = victim->carrying; temp; temp =temp->next_content )
            {
                 if(temp->strap_loc == STRAP_SHOULDER)
                 {
                     send_to_char("        {y<{gstrapped to shoulders{y>{x",ch );
                     printf_to_char(ch, "    %s\n\r",temp->short_descr);
			   for( sheathed = temp->contains; sheathed; sheathed = sheathed->next_content )
			   {
				    send_to_char("            {y<{gheld in sheath{y>{x",ch);
				    printf_to_char(ch, "    %s\n\r",sheathed->short_descr);				    continue;
			   }
			   return;
                 }
            }  
	}
	if(location == WEAR_WRIST_L )
      {
	      for ( temp = victim->carrying; temp; temp =temp->next_content )
            {
                 if(temp->strap_loc == STRAP_WRIST_L)
                 {
                     send_to_char("        {y<{gstrapped to wrist{y>{x",ch );
                     printf_to_char(ch, "    %s\n\r",temp->short_descr);
			   for( sheathed = temp->contains; sheathed; sheathed = sheathed->next_content )
			   {
				    send_to_char("            {y<{gheld in sheath{y>{x",ch);
				    printf_to_char(ch, "    %s\n\r",sheathed->short_descr);				    continue;
			   }
			   return;
                 }
            }  
	}
	if(location == WEAR_WRIST_R)
      {
	      for ( temp = victim->carrying; temp; temp =temp->next_content )
            {
                 if(temp->strap_loc == STRAP_WRIST_R)
                 {
                     send_to_char("        {y<{gstrapped to wrist{y>{x",ch );
                     printf_to_char(ch, "    %s\n\r",temp->short_descr);
			   for( sheathed = temp->contains; sheathed; sheathed = sheathed->next_content )
			   {
				    send_to_char("            {y<{gheld in sheath{y>{x",ch);
				    printf_to_char(ch, "    %s\n\r",sheathed->short_descr);				    continue;
			   }
			   return;
                 }
            }  
	}
	if(location == WEAR_LEGS)
      {
	      for ( temp = victim->carrying; temp; temp =temp->next_content )
            {
                 if(temp->strap_loc == STRAP_THIGH)
                 {
                     send_to_char("        {y<{gstrapped to thigh{y>{x",ch );
                     printf_to_char(ch, "    %s\n\r",temp->short_descr);

			   for( sheathed = temp->contains; sheathed; sheathed = sheathed->next_content )
			   {
				    send_to_char("            {y<{gheld in sheath{y>{x",ch);
				    printf_to_char(ch, "    %s\n\r",sheathed->short_descr);				    continue;
			   }
                     return;
                 }
            }  
	}
	if(location == WEAR_FEET)
      {
	      for ( temp = victim->carrying; temp; temp =temp->next_content )
            {
                 if(temp->strap_loc == STRAP_ANKLE)
                 {
                     send_to_char("        {y<{gstrapped to ankle{y>{x",ch );
                     printf_to_char(ch, "    %s\n\r",temp->short_descr);

			   for( sheathed = temp->contains; sheathed; sheathed = sheathed->next_content )
			   {
				    send_to_char("            {y<{gheld in sheath{y>{x",ch);
				    printf_to_char(ch, "    %s\n\r",sheathed->short_descr);
				    continue;
			   }
                     return;
                 }
            }  
	}

	if(location == WEAR_LEGS)
      {
	      for ( temp = victim->carrying; temp; temp =temp->next_content )
            {
                 if(temp->strap_loc == STRAP_THIGH)
                 {
                     send_to_char("        {y<{gstrapped to thigh{y>{x",ch );
                     printf_to_char(ch, "    %s\n\r",temp->short_descr);
			   for( sheathed = temp->contains; sheathed; sheathed = sheathed->next_content )
			   {
				    send_to_char("            {y<{gheld in sheath{y>{x",ch);
				    printf_to_char(ch, "    %s\n\r",sheathed->short_descr);
				    continue;
			   }
                     return;
                 }
            }  
	}
	return;
}

void do_demand(CHAR_DATA *ch,char *argument)
{
    CHAR_DATA  *victim;
    OBJ_DATA *obj;
    char i_name[MAX_INPUT_LENGTH];
    char m_name[MAX_INPUT_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char *target_name;
    int chance, vlevel;

    target_name = one_argument(argument,i_name);
    one_argument(target_name,m_name);

    chance = ch->pcdata->learned[skill_lookup("demand")];
    if (chance == 0
    || !can_use_skpell( ch, skill_lookup("demand") ) )
    {
        send_to_char("You are hardly intimidating enough to demand off others.\n\r",ch);
        return;
    }

    if (IS_NPC(ch))
        return;

    if ((victim = get_char_room(ch, NULL, m_name)) == NULL )
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }
    if (!IS_NPC(victim))
    {
        send_to_char("Why not just threaten them in person?\n\r",ch);
        return;
    }
    vlevel = victim->level;
    chance /= 2;
    chance += (3*ch->level);
    chance -= 2*vlevel;

    if (IS_GOOD(victim))
        chance -= 4*vlevel;
    else if (IS_EVIL(victim))
        chance -= 2*vlevel;
    else
        chance -= 3*vlevel;

    vlevel += 8;

    if ((obj = get_obj_list(victim, i_name, victim->carrying)) == NULL)
    {
        send_to_char("They do not have that object.\n\r",ch);
        return;
    }
    if (!can_see_obj(ch,obj) )
    {
        send_to_char("They do not have that object.\n\r",ch);
        return;
    }

    if (vlevel > ch->level
    || number_percent() > chance)
    {
        check_improve(ch,skill_lookup("demand"),FALSE,2);
        sprintf(buf1,"I don't think I'd give my belongings to one as weak as you!");
        sprintf(buf2,"Help! I'm being attacked by %s!",victim->short_descr);
        do_say(victim,buf1);
        do_yell(ch,buf2);
        multi_hit(victim,ch,TYPE_UNDEFINED);
        return;
    }

    if (!can_see(victim,ch))
    {
        act("$N tells you, 'I can't give to those I can't see.'",ch,0,victim,TO_CHAR);
        return;
    }

    if (!can_see_obj(victim,obj))
    {
        act("$N tells you, 'I can't see such an object.'",ch,0,victim,TO_CHAR);
        return;
    }

    if (obj->level > ch->level + 8)
    {
        do_say(victim, "That item is far to precious to hand over to scum like you!");
        sprintf(buf1,"Help! I'm being attacked by %s!",victim->short_descr);
        do_yell(ch,buf1);
        multi_hit(victim,ch,TYPE_UNDEFINED);
        return;
    }

    if (ch->move < obj->level)
    {
        act("$N tells you, 'Hah! You couldn't even get away if I chased you!'.",ch,0,victim,TO_CHAR);
        sprintf(buf1,"Help! I'm being attacked by %s!",victim->short_descr);
        do_yell(ch,buf1);
        multi_hit(victim,ch,TYPE_UNDEFINED);
        return;
    }

    if (ch->hit < (ch->max_hit*3/7))
    {
        do_say(victim,"Hah! You look weak enough that even I could kill you!");
        sprintf(buf1,"Help! I'm being attacked by %s!",victim->short_descr);
        do_yell(ch,buf1);
        multi_hit(victim,ch,TYPE_UNDEFINED);
        return;
    }

    if (ch->mana < 30)
    {
        send_to_char("You don't have the mana.\n\r",ch);
        return;
    }

    if ( ch->hit <= 30  )
    {
        send_to_char("You don't have enough hit points.\n\r", ch );
        return;
    }

    if ((obj->wear_loc != WEAR_NONE) && IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
        act("$N tells you, 'I'm unable to release $p'.",ch,obj,victim,TO_CHAR);
        act("$N cowers back from you in fright.",ch,0,victim,TO_CHAR);
        act("$N cowers back from $n in fright.",ch,0,victim,TO_NOTVICT);
        return;
    }
    if (IS_SET(obj->extra_flags,ITEM_NODROP) )
    {
        act("$N tells you, 'I'm unable to release $p'.",ch,obj,victim,TO_CHAR);
        act("$N cowers back from you in fright.",ch,0,victim,TO_CHAR);
        act("$N cowers back from $n in fright.",ch,0,victim,TO_NOTVICT);
        return;
    }

    if ( ch->carry_weight + get_obj_weight(obj)  > can_carry_w(ch) )
    {
        act("$N tells you, 'You can't carry the weight $n.'",ch,0,victim,TO_CHAR);
        return;
    }
    if ( ch->carry_number + 1 > can_carry_n(ch) )
    {
        act("$N tells you, 'You can't carry that many items $n.'",ch,0,victim,TO_CHAR);
        return;
    }

    act("$N caves in to the bully tactics of $n.",ch,0,victim,TO_NOTVICT);
    act("$N shivers in fright and caves in to your bully tactics.",ch,0,victim,TO_CHAR);

    if (obj->wear_loc != WEAR_NONE)
    {
        act("$n stops using $p.",victim,obj,NULL,TO_ROOM);
        act("You stop using $p.",victim,obj,NULL,TO_CHAR);
    }
    act("$N gives $p to $n.",ch,obj,victim,TO_NOTVICT);
    act("$N gives you $p.",ch,obj,victim,TO_CHAR);
    check_improve(ch,skill_lookup("demand"),TRUE,2);
    WAIT_STATE(ch,24);
    obj_from_char(obj);
    obj_to_char(obj,ch);
    ch->mana -= 30;
    ch->hit -= 30;
    return;
}

OBJ_DATA *get_obj_sheath(CHAR_DATA *ch, char *argument, OBJ_DATA *sheath)
{
	OBJ_DATA *obj;

	obj = sheath->contains;

	if(is_name( argument, obj->name ) )
		return obj;
        obj = obj->next_content;
	if(is_name( argument, obj->name) )
		return obj;

	return NULL;
}

void do_boxrent( CHAR_DATA *ch , char *argument)
{   
    ROOM_INDEX_DATA *to_room;
    CHAR_DATA 	     *renter;
    char            arg[MSL];

    if ( IS_NPC(ch))
	return;

    one_argument(argument,arg);

    if ( arg[0] == '\0' )
    {
	if (ch->in_room->vnum != ROOM_VNUM_RENTER)
	{
	    send_to_char("You can't rent a box here.\n\r",ch);
	    return;
	}
	
    	renter = NULL;

	for ( renter = ch->next_in_room; renter; renter = renter->next_in_room )
	{
	    if ( IS_NPC( renter ) )
		if ( renter->pIndexData->vnum == MOB_VNUM_RENTER )
		    break;
	}

	if ( !renter ) 
	{
	    send_to_char("The renter is currently not availible.\n\r", ch);
	    return;
	}

	if ( ch->pcdata->boxrented == 0 )
	{
	    long cost = 500000;

	    if ( ( ch->silver + ( ch->gold * 100 ) ) < cost )
	    {
		send_to_char( "You can't afford to rent a box.\n\r", ch );
		return;
	    }

	    else			  
	    {
		deduct_cost( ch, cost );
		send_to_char( "You have rented your very own box.\n\r", ch );
		ch->pcdata->boxrented = 1;
		act( "$N takes you to your very own box", ch, NULL, renter, TO_CHAR );
		act( "$N takes $n to $s box."   , ch, NULL, renter, TO_ROOM );
		to_room = get_room_index( ROOM_VNUM_RENTBOX );
		char_from_room( ch );
		char_to_room( ch, to_room );
		return;
	    }
	}

	else
	{
            act( "$N takes you to your box.", ch, NULL, renter, TO_CHAR );
            act( "$N takes $n to $s box."   , ch, NULL, renter, TO_ROOM );
	    to_room = get_room_index( ROOM_VNUM_RENTBOX);
	    char_from_room(ch);
	    char_to_room( ch, to_room );
	    return;
	}
    }

    else
    {
	send_to_char("Just type boxrent no argument.\n\r", ch);
	return;
    }
}

void do_store( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
	return;

    if ( argument[0] == '\0' )
    {
	   send_to_char( "Store what in where?\n\r", ch );
	   return;
    }

    if (ch->in_room->vnum == ROOM_VNUM_RENTBOX )
    {
	OBJ_DATA *obj;

	if ( ch->pcdata->item_box >= MAX_STORAGE )
	{
	   send_to_char( "How do you expect that to fit in there?\n\r", ch );
	   return;
	}

	if ( ( obj = get_obj_carry( ch, argument, ch ) ) == NULL )
	{
	   send_to_char( "You do not have that item.\n\r", ch );
	   return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	   sprintf( buf, "You can't let go of it.\n\r" );
	   send_to_char( buf, ch );
	   return;
	}

	obj_from_char( obj );
	obj_to_box( obj , ch );
	act( "$N puts $p in his box.", ch, obj, NULL, TO_ROOM );
	act( "You put $p in your box.", ch, obj, NULL, TO_CHAR );
    }
 
    return;
}

void do_retrieve( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch))
	return;

    if ( ch->in_room->vnum == ROOM_VNUM_RENTBOX )
    {
	OBJ_DATA *obj;
	
	if ( argument[0] == '\0' )
	{
	    send_to_char( "Retrieve what from your box?\n\r", ch );
	    return;
	}

	obj = get_obj_list( ch, argument, ch->pcdata->box );

	if ( obj == NULL )
	{
	    act( "I see no $T here.", ch, NULL, argument, TO_CHAR );
	    return;
	}

	obj_from_box( obj );
	obj_to_char( obj, ch );

	act( "$N gets $p from his box.", ch, obj, NULL, TO_ROOM );
	act( "You get $p from your box.", ch, obj , NULL, TO_CHAR );
    }

    return;
}


void do_smithing(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    char buf[MSL];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found = FALSE;

    if ( IS_NPC(ch)
    ||   !can_use_skpell( ch, skill_lookup("smithing") ) )
    {
	send_to_char("You do not know how to smith.\n\r", ch );
	return;
    }
 
    if ( ch->fighting )
    {
        send_to_char( "Wait until the fight finishes.\n\r", ch );
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        send_to_char( "To repair one item type: smithing <item>\n\rTo repair all your items type: smithing <all>\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {                            
        if ( number_percent() > get_skill(ch,skill_lookup("smithing")) )    
        {
            check_improve(ch,skill_lookup("smithing"),FALSE,8);
            send_to_char( "You failed to repair anything.\n\r", ch );
            return; 
        }

        for (obj = ch->carrying; obj; obj = obj_next)
        {
             obj_next = obj->next_content;

             if (IS_SET(obj->extra_flags, ITEM_NOREPAIR))
                 continue;       
            
             if ( obj->condition <= 50 && obj->condition > 0 )
             {
                 found = TRUE;
                 set_obj_condition( obj, 100 );
             }
        }

        if ( !found )
            send_to_char( "You don't have anything that needs repairing.\n\r", ch );
          
        WAIT_STATE(ch,2 * PULSE_VIOLENCE);
        send_to_char( "You painstakingly repair all of the items in your inventory.\n\r", ch );  
        check_improve(ch,skill_lookup("smithing"),TRUE,4);
        ch->move *= .60;
        sound( "hammer.wav", ch );
        return; 
    }         

    if (( obj = get_obj_carry(ch, arg, ch)) == NULL)
    {
	send_to_char("You are not carrying that.\n\r",ch);
	return;
    }

    if (obj->condition >= 100)
    {
	send_to_char("But that item is not broken.\n\r",ch);
	return;
    }

    if (IS_SET(obj->extra_flags, ITEM_NOREPAIR)) 
    {
        send_to_char( "This object can not be repaired.\n\r", ch );
        return;
    }

    WAIT_STATE(ch,2 * PULSE_VIOLENCE);

    if ( number_percent() > get_skill(ch,skill_lookup("smithing")) )    
    {
       check_improve(ch,skill_lookup("smithing"),FALSE,8);
       sprintf(buf, "$n tries to repair %s. But $n fails.", obj->short_descr); 
       act(buf,ch,NULL,NULL,TO_ROOM);
       sprintf(buf, "You failed to repair %s.\n\r", obj->short_descr);
       send_to_char(buf, ch);
    }

    else
    {
       check_improve(ch,skill_lookup("smithing"),TRUE,4);
       sprintf(buf, "$n painstakingly repairs %s.", obj->short_descr); 
       act(buf,ch,NULL,NULL,TO_ROOM);
       sprintf(buf, "You painstakingly repair %s.\n\r", obj->short_descr);
       send_to_char(buf, ch);
       set_obj_condition( obj, UMAX( 100, obj->condition + ( get_skill(ch, skill_lookup("smithing")) / 2 ) ) ); 
       sound( "hammer.wav", ch );
       return;
    }

    return;
}


void do_butcher(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  OBJ_DATA *tmp_obj;
  OBJ_DATA *tmp_next;

  if (IS_NPC(ch))
    return;

  one_argument(argument,arg);

  if ( arg[0]== '\0' )
  {
    send_to_char( "Butcher what?\n\r", ch );
    return;
  }

  if ( (obj = get_obj_here(ch,NULL,arg)) == NULL )
  {
      send_to_char("You do not see that here.\n\r",ch);
      return;
  }

  if (obj->item_type != ITEM_CORPSE_PC && obj->item_type != ITEM_CORPSE_NPC)
  {
      send_to_char("You can't butcher that.\n\r",ch);
      return;
  }

  if (obj->carried_by != NULL)
  {
      send_to_char("Put it down first.\n\r",ch);
      return;
  }

  if ( !IS_NPC(ch) && get_skill(ch,skill_lookup("butchering")) < 1)
  {
      send_to_char("You don't know how to butcher.\n\r", ch);
      return;
  }

  obj_from_room(obj);
  
  for (tmp_obj = obj->contains;tmp_obj != NULL;
       tmp_obj = tmp_next)
  {
      tmp_next = tmp_obj->next_content;
      obj_from_obj(tmp_obj);
      obj->carried_by = ch;
      obj_to_room(tmp_obj,ch->in_room);
  }
  
  if (IS_NPC(ch) || number_percent() < get_skill(ch,skill_lookup("butchering")))
  {
      int numsteaks;
      int i;
      OBJ_DATA *steak;
        
      numsteaks = number_bits(2) + 1; 
      
      if (numsteaks > 1)
      {
	  sprintf(buf, "$n butchers $p and creates %i steaks.",numsteaks);
	  act(buf,ch,obj,NULL,TO_ROOM);

	  sprintf(buf, "You butcher $p and create %i steaks.",numsteaks);
	  act(buf,ch,obj,NULL,TO_CHAR);
      }

      else 
      {
	  act("$n butchers $p and creates a steak."
	      ,ch,obj,NULL,TO_ROOM);

	  act("You butcher $p and create a steak."
	      ,ch,obj,NULL,TO_CHAR);
      }

      check_improve(ch,skill_lookup("butchering"),TRUE,1);

      for (i=0; i < numsteaks; i++)
      {
	  steak = create_object(get_obj_index(OBJ_VNUM_STEAK),0);
	  sprintf( buf, steak->short_descr, obj->short_descr);
	  free_string( steak->short_descr );
	  steak->short_descr = str_dup( buf );

	  sprintf( buf, steak->description, obj->short_descr );
	  free_string( steak->description );
	  steak->description = str_dup( buf );
	  obj->carried_by = ch;
	  obj_to_room(steak,ch->in_room);
      }
  }
	
  else 
  {
      act("You fail and destroy $p.",ch,obj,NULL,TO_CHAR);
      act("$n fails to butcher $p and destroys it.",
	  ch,obj,NULL,TO_ROOM);
      check_improve(ch,skill_lookup("butchering"),FALSE,1);
  }

  extract_obj(obj);
  return;

}


/* Creodin for Acasaid 2001 */
void do_purify (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;
  int skill;

  /* find out what */
    if (argument[0] == '\0')
    {
        send_to_char ("Purify what food or drink?\n\r", ch);
        return;
    }

    obj = get_obj_list (ch, argument, ch->carrying);

    if (obj == NULL)
    {
        send_to_char ("You don't have that item.\n\r", ch);
        return;
    }

    if ( !can_use_skpell( ch, skill_lookup("smithing") ) ) 
    {
        send_to_char( "You do not have that skill!\n\r", ch );
        return;
    }
    skill = get_skill(ch, skill_lookup("purify") );
    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {

        if (number_percent () < skill)
        {
          act ("$n calls upon $s deity to purify $p.", ch, obj, NULL, TO_ROOM);
          act ("You call upon your deity to purify $p.", ch, obj, NULL, TO_CHAR);
          obj->value[3] = 0;
          check_improve (ch, skill_lookup( "purify" ), TRUE, 4);
          WAIT_STATE (ch, skill_table[skill_lookup( "purify" )].beats);
          ch->mana -= 10;
          return;

        } else {

          act ("You fail to purify $p.", ch, obj, NULL, TO_CHAR);
          check_improve (ch, skill_lookup( "purify" ), FALSE, 3);
          WAIT_STATE (ch, skill_table[skill_lookup( "purify" )].beats);
          ch->mana -= 10;
          return;
        }
    }
}

void do_pcforge (CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *chunk;
   OBJ_DATA *item;
   int ore_weight = 0;
   int typ;
   int ore;
   char buf[MSL];
   char arg1[MSL];
   char arg2[MSL];
   bool excess = FALSE;
   int percent;
   int chance = number_percent();

   struct forge_type
   {
      char *   name;
      sh_int   type;
      int      wear_loc;
      sh_int   weap_typ;
      int      ore_min;
   };

   static const struct forge_type forge_table[] = {
   { "breastplate",  ITEM_ARMOR,  ITEM_WEAR_BODY, 0, 20 },
   { "gauntlets",  ITEM_ARMOR,  ITEM_WEAR_HANDS, 0, 10 },
   { "boots",  ITEM_ARMOR,  ITEM_WEAR_FEET, 0, 10 },
   { "bracer",  ITEM_ARMOR,  ITEM_WEAR_WRIST, 0, 5 },
   { "sleeves",  ITEM_ARMOR,  ITEM_WEAR_ARMS, 0, 10 },
   { "shield",  ITEM_ARMOR,  ITEM_WEAR_SHIELD, 0, 15 },
   { "leggings",  ITEM_ARMOR,  ITEM_WEAR_LEGS, 0, 15 },
   { "sword", ITEM_WEAPON, ITEM_WIELD, WEAPON_SWORD, 10 },
   { "dagger", ITEM_WEAPON, ITEM_WIELD, WEAPON_DAGGER, 5 },
   { "spear", ITEM_WEAPON, ITEM_WIELD, WEAPON_SPEAR, 5 },
   { "mace", ITEM_WEAPON, ITEM_WIELD, WEAPON_MACE, 8 },
   { "axe", ITEM_WEAPON, ITEM_WIELD, WEAPON_AXE, 12 },
   { "flail", ITEM_WEAPON, ITEM_WIELD, WEAPON_FLAIL, 12 },
   { "polearm", ITEM_WEAPON, ITEM_WIELD, WEAPON_POLEARM, 15 },
   { NULL, 0, 0, 0, 0 }
   };

   if ((percent = get_skill (ch, skill_lookup ("forge"))) <= 1)
   {
      send_to_char ("You might hurt yourself!\n\r", ch);
      return;
   }
 
   if (argument[0] == '\0')
   {
      send_to_char ("Syntax: forge <item type> <mineral type>.\n\r", ch);
      return;
   }

   argument = one_argument (argument, arg1);
   argument = one_argument (argument, arg2);

   /* Validate arg1 first */  

   for (typ = 0; forge_table[typ].name != NULL; typ++)
   {
      if (!str_cmp (arg1, forge_table[typ].name))
         break;
   }

   if (forge_table[typ].name == NULL)
   {
      int col = 0;

      send_to_char ("Invalid item, available choices are:\n\r\n\r", ch);
  
      for (typ = 0; forge_table[typ].name != NULL; typ++)
      {
         sprintf (buf, "%s ", forge_table[typ].name);
         send_to_char (buf, ch);

         if (++col == 6)
         {
            send_to_char ("\n\r", ch);
            col = 0;
         }
      }
      
      send_to_char ("\n\r", ch);

      return;
   }             

   /* Validate arg2 */

   for (ore = 0; mineral_table[ore].name != NULL; ore++)
   {
      if (!str_cmp (arg2, mineral_table[ore].name))
         break;
   }
 
   if (mineral_table[ore].name == NULL)
   {
      send_to_char ("That is not a forgeable material.\n\r", ch);
      return;
   }
   
   for (chunk = ch->carrying; chunk != NULL; chunk = chunk->next)
   {
      if (chunk->item_type != ITEM_ORE)
         continue;

      if (ore == chunk->value[0])
         ore_weight += chunk->weight;
   }

   if (ore_weight < forge_table[typ].ore_min)
   {
      sprintf (buf, "You lack the %s to forge a %s.\n\r", mineral_table[ore].name, forge_table[typ].name);
      send_to_char (buf, ch);
      return;
   }
   
   /* Now that we've determined we have enough, extract the ore... */

   ore_weight = 0;

   for (chunk = ch->carrying; chunk != NULL; chunk = chunk->next)
   {
      if (chunk->item_type != ITEM_ORE)
         continue;

      if (ore == chunk->value[0])
      {
         ore_weight += chunk->weight;
         extract_obj (chunk);
      }
     
      if (ore_weight >= forge_table[typ].ore_min)
      {
         if (ore_weight > forge_table[typ].ore_min) /* No wastage! */
         {
            OBJ_DATA *left;
            int leftover;
 
            leftover = ore_weight - forge_table[typ].ore_min;
 
            left = create_object (get_obj_index (OBJ_VNUM_ORE), 0);
            free_string (left->name);
            free_string (left->short_descr);
            free_string (left->description);
            sprintf (buf, "chunk ore %s", mineral_table[ore].name);
            left->name = str_dup (buf);
            sprintf (buf, "a chunk of %s", mineral_table[ore].name);
            left->short_descr = str_dup (buf);
            sprintf (buf, "A chunk of %s sits here.", mineral_table[ore].name);
            left->description = str_dup (buf);
            left->item_type = ITEM_ORE;
            left->value[0] = ore;
            left->weight = leftover;

            obj_to_char (left, ch);
            excess = TRUE;
         }
          break;
      }
   }

   /* Now the REAL fun begins */
   
   sprintf (buf, "You begin to forge the %s.\n\r", forge_table[typ].name);
   act (buf, ch, NULL, NULL, TO_CHAR );
   act ("$n begins to forge something.", ch, NULL, NULL, TO_ROOM);

   if (percent < chance - 40) /* Kablooey! */
   {
      send_to_char ("The forge blows up in your face!\n\r", ch);
      spell_fireball (skill_lookup ("fireball"), ch->level, ch, (void *) ch, TARGET_CHAR);
      return;
   }

   act ("You continue to hammer away at your creation...", ch, NULL, NULL, TO_CHAR );
   act ("$n hammers away.", ch, NULL, NULL, TO_ROOM);

   if (percent < chance - 30) /* Not quite... */
   {
      send_to_char ("Your hammer strike splits your piece in two!\n\r", ch);
      return;
   }

   sprintf (buf, "The %s is now taking shape...", forge_table[typ].name);  
   act (buf, ch, NULL, NULL, TO_CHAR );
   act ("$n continues to work.", ch, NULL, NULL, TO_ROOM);

   if (percent < chance - 20) /* Oops... */
   {
      sprintf (buf, "The %s appears to be too flimsy to use.", forge_table[typ].name); 
      send_to_char (buf, ch);
      send_to_char ("Frustratedly, you discard the piece.\n\r", ch);
      return;
   }

   sprintf (buf, "You put the final touches on the %s...", forge_table[typ].name);  
   act (buf, ch, NULL, NULL, TO_CHAR );
   act ("$n continues to work.", ch, NULL, NULL, TO_ROOM);

   if (percent < chance - 10) /* No Cigar */
   {
      sprintf (buf, "You slip and damage the %s beyond repair!", forge_table[typ].name);
      send_to_char (buf, ch);
      send_to_char ("Frustratedly, you discard the piece.\n\r", ch);
      return;
   }

   /* Good job! */

   item = create_object (get_obj_index (OBJ_VNUM_BLANK), 0);
  
   free_string (item->name);
   free_string (item->short_descr);
   free_string (item->description);

   sprintf (buf, "%s %s", forge_table[typ].name, mineral_table[ore].name);
   
   item->name = str_dup (buf);

   switch (forge_table[typ].wear_loc)
   {
      case ITEM_WEAR_LEGS:
      case ITEM_WEAR_FEET:
      case ITEM_WEAR_HANDS:
      case ITEM_WEAR_ARMS:
      { 
         sprintf (buf, "a pair of %s %s", mineral_table[ore].name, forge_table[typ].name);
         item->short_descr = str_dup (buf);
         sprintf (buf, "A pair of %s %s lies here.", mineral_table[ore].name, forge_table[typ].name);
         item->description = str_dup (buf);
      
         break;
      }

      default:
      {
         const char *article;

         if (mineral_table[ore].name[0] == 'a' ||
             mineral_table[ore].name[0] == 'e' ||
             mineral_table[ore].name[0] == 'i' ||
             mineral_table[ore].name[0] == 'o' ||
             mineral_table[ore].name[0] == 'u')
            article = "an";
         else
            article = "a";

         sprintf (buf, "%s %s %s", article, mineral_table[ore].name, forge_table[typ].name);
         item->short_descr = str_dup (buf);
         sprintf (buf, "%s %s %s lies here.", capitalize(article), mineral_table[ore].name, forge_table[typ].name);
         item->description = str_dup (buf);
         break;
      }
   }

   item->item_type = forge_table[typ].type;
   TOGGLE_BIT (item->wear_flags, forge_table[typ].wear_loc);

   switch (item->item_type)
   {
      case ITEM_ARMOR:
      {
         int number;
   
         number = UMAX(20, ch->level);
 
         item->value[0] = number;
         item->value[1] = number;
         item->value[2] = number;
         item->value[3] = (5 * UMAX(20, ch->level)) / 6;
 
         break;
      }

      case ITEM_WEAPON:
      {
     
         item->value[0] = forge_table[typ].weap_typ;
         item->value[1] = UMAX (15, (ch->level + 20) / 5);
         item->value[2] = ch->level < 80 ? 4 : 5;
         
         switch (item->value[0])
         {
            case WEAPON_SWORD:
               item->value[3] = attack_lookup ("slash");
               break;
            case WEAPON_DAGGER:
            case WEAPON_SPEAR:
               item->value[3] = attack_lookup ("stab");
               break;
            case WEAPON_MACE:
            case WEAPON_FLAIL:
            case WEAPON_POLEARM:
               item->value[3] = attack_lookup ("pound");
               break;
            case WEAPON_AXE:
               item->value[3] = attack_lookup ("chop");
               break;
            default:
               item->value[3] = attack_lookup ("blast");
               break;
         }
      
         break;
      }
      default:
         return;
   }

   sprintf (buf, "You have successfully forged %s!\n\r", item->short_descr);
   send_to_char (buf, ch);
   sprintf (buf, "$n has successfully forged %s!\n\r", item->short_descr);
   act (buf, ch, NULL, NULL, TO_ROOM);
  
   obj_to_char (item, ch);

   if (excess)
   {
      sprintf (buf, "You managed to save some %s.\n\r", mineral_table[ore].name);
      send_to_char (buf, ch);
   }

   return;
}

void do_inlay( CHAR_DATA *ch, char *argument ) 
{ 
     OBJ_DATA *sockets; 
     AFFECT_DATA *paf; 
     OBJ_DATA *obj; 
     char arg1[MAX_INPUT_LENGTH]; 
     char arg2[MAX_INPUT_LENGTH]; 
     int value; 

     argument = one_argument( argument, arg1 ); 
     argument = one_argument( argument, arg2 ); 

     obj = get_obj_carry(ch, arg2, ch); 
     sockets = get_obj_carry(ch, arg1, ch); 

     if( sockets  == '\0' ) 
     { 
         send_to_char( "The inlaid object must be a settable one.\n\r", ch ); 
         return; 
     } 

     if( obj  == '\0' ) 
     { 
         send_to_char( "What would you like to set this object into?\n\r",  ch ); 
         return; 
     } 

     if( sockets->item_type != ITEM_SOCKETS ) 
     { 
         send_to_char( "You can only use settable items to inlay.\n\r",ch); 
         return; 
     } 

     if( (obj = get_obj_carry( ch, arg2, ch )) == NULL ) 
     { 
         send_to_char( "You don't have that.\n\r", ch ); 
         return; 
     } 

     if( IS_SET(obj->extra2_flags,ITEM_INLAY2) ) 
     { 
         send_to_char( "You cannot fit another item into that.\n\r", ch); 
         return; 
     } 

     if( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR) 
     { 
         send_to_char( "You may only set gems into weapons and armor.\n\r",ch); 
         return; 
     } 

     value = sockets->value[1]; 

     extract_obj( sockets );
 
     if( IS_SET(obj->extra2_flags,ITEM_INLAY1) ) 
     { 
         REMOVE_BIT(obj->extra2_flags,ITEM_INLAY1); 
         SET_BIT(obj->extra2_flags,ITEM_INLAY2); 
     } 
     else 
     SET_BIT(obj->extra2_flags,ITEM_INLAY1); 
     switch ( sockets->value[0] ) 
     { 
             case 1: 
                 send_to_char( "You carefully set a sapphire into the spot " 
                               "provided and it stays nicely.\n\r",ch); 
                 if (affect_free == NULL) 
                     paf = (AFFECT_DATA *)alloc_perm(sizeof(*paf)); 
                 else 
                 { 
                     paf = affect_free; 
                     affect_free = affect_free->next; 
                 } 
                 paf                         = new_affect();
                 paf->location               = APPLY_MANA;               
                 paf->where                  = TO_OBJECT; 
                 paf->modifier               = 75 + (value * 30);
                 paf->type                   = 0; 
                 paf->duration               = -1; 
                 paf->bitvector              = 0; 
                 paf->next = obj->affected; 
                 obj->affected = paf; 
                 break; 
             case 2: 
                 send_to_char( "You carefully set a ruby into the spot " 
                               "provided and it stays nicely.\n\r",ch); 
                 if (affect_free == NULL) 
                     paf = (AFFECT_DATA *) alloc_perm(sizeof(*paf)); 
                 else 
                 { 
                     paf = affect_free; 
                     affect_free = affect_free->next; 
                 } 
                 paf                         = new_affect(); 
                 paf->where                  = TO_OBJECT; 
                 paf->duration               = -1;
                 paf->type                   = 0;
                 paf->location               = APPLY_HIT; 
                 paf->modifier               = 75 + (value * 30); 
                 paf->bitvector              = 0; 
                 paf->next = obj->affected; 
                 obj->affected = paf; 
                 break; 
             case 3: 
                 send_to_char( "You carefully set an emerald into the "                                "spot provided and it stays nicely.\n\r",ch); 
                 if (affect_free == NULL) 
                     paf = (AFFECT_DATA *)alloc_perm(sizeof(*paf)); 
                 else 
                 { 
                     paf = affect_free; 
                     affect_free = affect_free->next; 
                 } 
                 paf                         = new_affect(); 
                 paf->where                  = TO_OBJECT; 
                 paf->type                   = 0; 
                 paf->duration               = -1; 
                 paf->location               = APPLY_MOVE; 
                 paf->modifier               = 75 + (value * 30); 
                 paf->bitvector              = 0; 
                 paf->next = obj->affected; 
                 obj->affected = paf; 
                 break; 
              case 4: 
                 send_to_char( "You carefully set a diamond into the " 
                               "spot provided and it stays nicely.\n\r",ch); 
                 if (affect_free == NULL) 
                     paf = (AFFECT_DATA *)alloc_perm(sizeof(*paf)); 
                 else 
                 { 
                     paf = affect_free; 
                     affect_free = affect_free->next; 
                 } 
                 paf                         = new_affect(); 
                 paf->where                  = TO_AFFECTS; 
                 paf->type                   = 0; 
                 paf->duration               = -1; 
                 paf->location               = APPLY_HITROLL; 
                 paf->modifier               = 10 + (value * 5); 
                 paf->bitvector              = 0; 
                 paf->next = obj->affected; 
                 obj->affected = paf; 
                 break; 
              case 5: 
                 send_to_char( "You carefully set a topaz into the " 
                               "spot provied and it stays nicely.\n\r", ch); 
                 if (affect_free == NULL) 
                     paf = (AFFECT_DATA *)alloc_perm(sizeof(*paf)); 
                 else 
                 { 
                     paf = affect_free; 
                     affect_free = affect_free->next; 
                 } 
                 paf                         = new_affect(); 
                 paf->where                  = TO_OBJECT; 
                 paf->type                   = 0; 
                 paf->duration               = -1; 
                 paf->location               = APPLY_AC; 
                 paf->modifier               = -125 - (value * 50); 
                 paf->bitvector              = 0; 
                 paf->next = obj->affected; 
                 obj->affected = paf; 
                 break; 
              case 6: 
                 send_to_char( "You manage to set the tiny skull into the " 
                               "spot provided and it stays nicely.\n\r", ch); 
                 if (affect_free == NULL ) 
                     paf = (AFFECT_DATA *)alloc_perm(sizeof(*paf)); 
                 else 
                 { 
                     paf = affect_free; 
                     affect_free = affect_free->next; 
                 } 
                 paf                         = new_affect(); 
                 paf->where                  = TO_OBJECT; 
                 paf->type                   = 0; 
                 paf->duration               = -1; 
                 paf->location               = APPLY_DAMROLL; 
                 paf->modifier               = 10 + (value * 5); 
                 paf->bitvector              = 0; 
                 paf->next = obj->affected; 
                 obj->affected = paf; 
                 break; 
              default: 
                 send_to_char( "Are you sure that's something you can " 
                               "inset?.\n\r", ch ); 
                 return; 
             } 
     return; 
} 

void show_trade( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;
    char buf[MIL];
    int i;
    const char *kw;

    sprintf( buf, "Gold: %d coins.\n\r", GET_TRADE( victim ).gold );
    send_to_char( buf, ch );
    for( i = 0; i < MAX_TRADE_OBJ; ++i )
    {
	obj = GET_TRADE( victim ).objs[i];
	if( !obj )
	    continue;
	sprintf( buf, "Object: %s\n\r", obj->short_descr );
	send_to_char( buf, ch );
    }
    if( ch == victim )
	kw = "have";
    else
	kw = "has";

    if( IS_SET( GET_TRADE( victim ).flags, TRADE_AGREED ) )
	act( "$N $t agreed to the trade.", ch, kw, victim, TO_CHAR );
    else if( IS_SET( GET_TRADE( victim ).flags, TRADE_LOCKED ) )
	act( "$N $t set a lock on the trade.", ch, kw, victim, TO_CHAR );
    return;
}

/*
 * Checks that the items offered in trade still exist.
 */
bool check_trade( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    int i;

    for( i = 0; i < MAX_TRADE_OBJ; ++i )
    {
	obj = GET_TRADE( ch ).objs[i];
	if( !obj )
	    continue;
	if( obj->carried_by != ch )
	    return FALSE;
    }
    return TRUE;
}

/*
 * Move items in trade from victim to ch.
 * Note that for security reasons greater than mundane weight considerations,
 * I have not considered weight important.
 */
void affect_trade( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;
    int i;

    for( i = 0; i < MAX_TRADE_OBJ; ++i )
    {
	obj = GET_TRADE( victim ).objs[i];
	if( !obj )
	    continue;

	act( "You give $p to $N.", victim, obj, ch, TO_CHAR );
	act( "$n gives $p to you.", victim, obj, ch, TO_VICT );
	act( "$n gives $p to $N.", victim, obj, ch, TO_NOTVICT );

	obj_from_char( obj );
	obj_to_char( obj, ch );
    }
    if( GET_TRADE( victim ).gold > 0 )
    {
	act( "You give some gold to $N.", victim, NULL, ch, TO_CHAR );
	act( "$n gives some gold to you.", victim, NULL, ch, TO_VICT );
	act( "$n gives some gold to $N.", victim, NULL, ch, TO_NOTVICT );
    }
    ch->gold += GET_TRADE( victim ).gold;
    return;
}

/*
 * Finds a trade object.
 */
OBJ_DATA *find_trade_obj( CHAR_DATA *ch, CHAR_DATA *victim, char *name )
{
    OBJ_DATA *obj;
    int i;

    for( i = 0; i < MAX_TRADE_OBJ; ++i )
    {
	obj = GET_TRADE( victim ).objs[i];
	if( obj && is_name( name, obj->name ) )
	    return obj;
    }
    return NULL;
}

/*
 * Symposium's secure trade interface.
 */
void do_trade( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    int i;
    int amount = 0;

    if( IS_NPC( ch ) )
	return;

    argument = one_argument( argument, arg );

    if( !ch->pcdata->trade )
    {
	if( ch->mana < 100 + ch->level * 2 )
	{
	    send_to_char( "You need some more mana in order to initiate a trade.\n\r", ch );
	    return;
	}

	if( !str_cmp( arg, "offer" ) )
	{
	    ch->mana -= 100 + ch->level * 2;

	    ch->pcdata->trade = (TRADE_DATA *)alloc_mem( sizeof( TRADE_DATA ) );
	    ch->pcdata->trade->first.ch = ch;
	    ch->pcdata->trade->first.flags = 0;
	    ch->pcdata->trade->first.gold = 0;

	    if( ( victim = get_char_room( ch, NULL, argument ) )
		&& victim != ch && !IS_NPC( victim ) )
		ch->pcdata->trade->second.ch = victim;
	    else
		ch->pcdata->trade->second.ch = NULL;
	    ch->pcdata->trade->second.flags = 0;
	    ch->pcdata->trade->second.gold = 0;

	    for( i = 0; i < MAX_TRADE_OBJ; ++i )
	    {
		ch->pcdata->trade->first.objs[i] = NULL;
		ch->pcdata->trade->second.objs[i] = NULL;
	    }

	    if( ch->pcdata->trade->second.ch )
		act( "$n opens an offer for a trade session with $N.", ch, NULL, victim, TO_ALL );
	    else
		act( "$n opens an offer for a trade session.", ch, NULL, NULL, TO_ALL );
	    return;
	}

	if( !str_cmp( arg, "join" ) )
	{
	    if( !( victim = get_char_room( ch, NULL, argument ) ) || IS_NPC( victim ) )
	    {
		send_to_char( "That person is not here.\n\r", ch );
		return;
	    }

	    if( victim->pcdata->trade == NULL )
	    {
		send_to_char( "They have not offered to trade.\n\r", ch );
		return;
	    }

	    if( victim->pcdata->trade->second.ch != NULL
		&& victim->pcdata->trade->second.ch != ch )
	    {
		act( "$N has restricted the trade so you can't join.", ch, NULL, victim, TO_CHAR );
		return;
	    }

	    ch->mana -= 100 + ch->level * 2;

	    ch->pcdata->trade = victim->pcdata->trade;
	    ch->pcdata->trade->second.ch = ch;
	    act( "$n joins a trade session with $N.", ch, NULL, victim, TO_ALL );
	    return;
	}
	send_to_char( "Type HELP TRADE to get more information on trading securely.\n\r", ch );
	return;
    }

    /* From here down a trade exists */

    victim = (ch == ch->pcdata->trade->first.ch )
	? ch->pcdata->trade->second.ch : ch->pcdata->trade->first.ch;

    if( !str_prefix( arg, "examine" ) )
    {
	if( argument[0] == '\0' )
	{
	    send_to_char( "Your offer is as follows:\n\r", ch );
	    show_trade( ch, ch );
	    act( "$N's offer is as follows:", ch, NULL, victim, TO_CHAR );
	    show_trade( ch, victim );
	    return;
	}

	if( !( obj = find_trade_obj( ch, victim, argument ) )
	    && !( obj = find_trade_obj( ch, ch, argument ) ) )
	{
	    send_to_char( "You can't find that object.\n\r", ch );
	    return;
	}
 
        if ( ch->gold < amount )
        {
            printf_to_char( ch, "You must have at least %d gold to examine that item.\n\r", amount );
            return;
        }

	send_to_char( "You examine the object to determine its worth...n\n\r", ch );
        printf_to_char( ch, "It has cost you %d gold to examine the trade...\n\r", amount );
        ch->gold -= amount;
	spell_identify( skill_lookup( "identify" ), ch->level, ch, obj, 0 );
	return;
    }
    
    if( !str_prefix( arg, "add" ) )
    {
	if( IS_TRADE_LOCKED( ch ) )
	{
	    send_to_char( "You cannot make a change to a locked trade.\n\r", ch );
	    return;
	}
	
	if( is_number( argument ) )
	{
	    i = atoi( argument );
	    if( i <= 0 || i > ch->gold )
	    {
		send_to_char( "You have insufficient funds for that.\n\r", ch );
		return;
	    }
	    if( ch == ch->pcdata->trade->first.ch )
		ch->pcdata->trade->first.gold += i;
	    else
		ch->pcdata->trade->second.gold += i;
	    ch->gold -= i;
	    sprintf( arg, "You add %d gold to your trade.\n\r", i );
	    send_to_char( arg, ch );
	    sprintf( arg, "$n adds %d to $s trade.", i );
	    act( arg, ch, NULL, victim, TO_VICT );
	    return;
	}
	
	if( !( obj = get_obj_carry( ch, argument, ch ) ) )
	{
	    send_to_char( "You can't find that item.\n\r", ch );
	    return;
	}

	if( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You cannot offer that item.\n\r", ch );
	    return;
	}
	
	for( i = 0; i < MAX_TRADE_OBJ; ++i )
	{
	    if( GET_TRADE( ch ).objs[i] == NULL )
	    {
		GET_TRADE( ch ).objs[i] = obj;
		act( "You add $p to the trade.", ch, obj, victim, TO_CHAR );
		act( "$n adds $p to the trade.", ch, obj, victim, TO_VICT );	
		return;
	    }
	}

	send_to_char( "You have used all of your slots for items, sorry.\n\r", ch );
	return;
    }
    
    if( !str_prefix( arg, "remove" ) )
    {
	if( IS_TRADE_LOCKED( ch ) )
	{
	    send_to_char( "You cannot make a change to a locked trade.\n\r", ch );
	    return;
	}
	
	if( is_number( argument ) )
	{
	    i = atoi( argument );
	    if( i <= 0 || i > GET_TRADE( ch ).gold )
	    {
		send_to_char( "You cannot remove that amount!\n\r", ch );
		return;
	    }
	    if( ch == ch->pcdata->trade->first.ch )
		ch->pcdata->trade->first.gold -= i;
	    else
		ch->pcdata->trade->second.gold -= i;
	    ch->gold += i;
	    sprintf( arg, "You remove %d gold from your trade.\n\r", i );
	    send_to_char( arg, ch );
	    sprintf( arg, "$n removes %d from $s trade.", i );
	    act( arg, ch, NULL, victim, TO_VICT );
	    return;
	}
	
	if( !( obj = find_trade_obj( ch, ch, argument ) ) )
	{
	    send_to_char( "You can't find that item.\n\r", ch );
	    return;
	}
	
	for( i = 0; i < MAX_TRADE_OBJ; ++i )
	{
	    if( obj == GET_TRADE( ch ).objs[i] )
		GET_TRADE( ch ).objs[i] = NULL;
	}

	act( "You remove $p from the trade.", ch, obj, victim, TO_CHAR );
	act( "$n removes $p from the trade.", ch, obj, victim, TO_VICT );
	return;
    }
    
    if( !str_cmp( arg, "withdraw" ) || !str_cmp( arg, "leave" ) )
    {
	send_to_char( "You withdraw from the trade.\n\r", ch );
	act( "$n withdraws from the trade.", ch, NULL, victim, TO_VICT );

	ch->gold += GET_TRADE( ch ).gold;
	victim->gold += GET_TRADE( victim ).gold;
	free_mem( ch->pcdata->trade, sizeof( TRADE_DATA ) );
	ch->pcdata->trade = NULL;
	victim->pcdata->trade = NULL;
	return;
    }
    
    if( !str_cmp( arg, "lock" ) )
    {
	if( IS_SET( GET_TRADE( ch ).flags, TRADE_LOCKED ) )
	{
	    if( ch == ch->pcdata->trade->first.ch )
	    {
		REMOVE_BIT( ch->pcdata->trade->first.flags, TRADE_LOCKED );
		REMOVE_BIT( ch->pcdata->trade->first.flags, TRADE_AGREED );
		REMOVE_BIT( ch->pcdata->trade->second.flags, TRADE_AGREED );
	    }
	    else
	    {
		REMOVE_BIT( ch->pcdata->trade->second.flags, TRADE_LOCKED );
		REMOVE_BIT( ch->pcdata->trade->second.flags, TRADE_AGREED );
		REMOVE_BIT( ch->pcdata->trade->first.flags, TRADE_AGREED );
	    }
	    send_to_char( "You unlock the trade.\n\r", ch );
	    act( "$n unlocks the trade.", ch, NULL, victim, TO_VICT );
	    return;
	}
	
	send_to_char( "You lock the trade.\n\r", ch );
	act( "$n locks the trade.", ch, NULL, victim, TO_VICT );
	if( ch == ch->pcdata->trade->first.ch )
	    SET_BIT( ch->pcdata->trade->first.flags, TRADE_LOCKED );
	else
	    SET_BIT( ch->pcdata->trade->second.flags, TRADE_LOCKED );
	return;
    }
    
    if( !str_cmp( arg, "agree" ) || !str_cmp( arg, "accept" ) )
    {
	if( !IS_SET( GET_TRADE( ch ).flags, TRADE_LOCKED )
	    || !IS_SET( GET_TRADE( victim ).flags, TRADE_LOCKED ) )
	{
	    send_to_char( "Both parties must lock the trade first.\n\r", ch );
	    return;
	}
	
	if( IS_SET( GET_TRADE( ch ).flags, TRADE_AGREED ) )
	{
	    if( ch == ch->pcdata->trade->first.ch )
		REMOVE_BIT( ch->pcdata->trade->first.flags, TRADE_AGREED );
	    else
		REMOVE_BIT( ch->pcdata->trade->second.flags, TRADE_AGREED );
	    send_to_char( "You withdraw your agreement to the trade.\n\r", ch );
	    act( "$n removes $s agreement to the trade.", ch, NULL, victim, TO_VICT );
	    return;
	}

	if( !IS_SET( GET_TRADE( victim ).flags, TRADE_AGREED ) )
	{
	    if( ch == ch->pcdata->trade->first.ch )
		SET_BIT( ch->pcdata->trade->first.flags, TRADE_AGREED );
	    else
		SET_BIT( ch->pcdata->trade->second.flags, TRADE_AGREED );
	    send_to_char( "You signify your agreement for this trade.\n\r", ch );
	    act( "$n has agreed to the trade.", ch, NULL, victim, TO_VICT );
	    return;
	}
	
	if( !check_trade( ch ) || !check_trade( victim ) )
	{
	    send_to_char( "An item is missing, the trade must be cancelled.\n\r", ch );
	    send_to_char( "An item is missing, the trade must be cancelled.\n\r", victim );
	    ch->gold += GET_TRADE( ch ).gold;
	    victim->gold += GET_TRADE( victim ).gold;
	    free_mem( ch->pcdata->trade, sizeof( TRADE_DATA ) );
	    ch->pcdata->trade = NULL;
	    victim->pcdata->trade = NULL;
	    return;
	}
	
	affect_trade( ch, victim );
	affect_trade( victim, ch );
	free_mem( ch->pcdata->trade, sizeof( TRADE_DATA ) );
	ch->pcdata->trade = NULL;
	victim->pcdata->trade = NULL;

	act( "You shake $N's hand and seal the deal.", ch, NULL, victim, TO_CHAR );
	act( "$n closes the deal, you shake $s hand and seal the deal.", ch, NULL, victim, TO_VICT );
	act( "$n has just closed the deal with $N.", ch, NULL, victim, TO_NOTVICT );
	return;
    }

    send_to_char( "Type HELP TRADE to get more information on trading securely.\n\r", ch );
    return;
}

