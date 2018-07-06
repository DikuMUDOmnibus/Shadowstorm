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
#include <string.h>
#include <time.h>
#include "include.h"

/*
 * Local functions.
 */
void	check_assist	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_shield_block     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    dam_message 	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
                            int dt, bool immune ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim, 
			    int total_levels ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	make_corpse	args( ( CHAR_DATA *ch ) );
void    one_hit         args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ) );
void    mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	raw_kill	args( ( CHAR_DATA *victim, CHAR_DATA *killer ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
extern  void            set_fighting(CHAR_DATA *ch, CHAR_DATA *victim);
bool    check_critical  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_counter   args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt ) );
void    crowd_brawl     args( ( CHAR_DATA *ch ) );
bool    check_steel_skin args( ( CHAR_DATA *ch, CHAR_DATA *victim ) ); 
bool    check_phase     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_force_shield args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_static_shield args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_flame_shield args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );


#define MAX_SPECTATORS 100

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;
    OBJ_DATA *obj, *obj_next;
    bool room_trig = FALSE;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	ch_next	= ch->next;

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
        {
            crowd_brawl(ch);
            multi_hit( ch, victim, TYPE_UNDEFINED );
        }
	else
	    stop_fighting( ch, FALSE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);
    
       	if ( IS_NPC( ch ) )
	{
	    if ( HAS_TRIGGER_MOB( ch, TRIG_FIGHT ) )
                p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_FIGHT );  
	    if ( HAS_TRIGGER_MOB( ch, TRIG_HPCNT ) )
		p_hprct_trigger( ch, victim );
 	}
  
       	for ( obj = ch->carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( obj->wear_loc != WEAR_NONE && HAS_TRIGGER_OBJ( obj, TRIG_FIGHT ) )
		p_percent_trigger( NULL, obj, NULL, victim, NULL, NULL, TRIG_FIGHT );
	}

	if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_FIGHT ) && room_trig == FALSE )
	{
	    room_trig = TRUE;
	    p_percent_trigger( NULL, NULL, ch->in_room, victim, NULL, NULL, TRIG_FIGHT );
	}
    }

    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;
	
	if (IS_AWAKE(rch) && rch->fighting == NULL && can_see(rch,victim))

	{
        if (RIDDEN(rch) == ch || MOUNTED(rch) == ch)
            multi_hit(rch,victim,TYPE_UNDEFINED);
	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch) 
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  rch->level + 6 > victim->level)
	    {
               if(!is_safe(rch,ch))
               {
            
		  do_function(rch, &do_emote, "screams and attacks!");
		  multi_hit(rch,victim,TYPE_UNDEFINED);
               }
		  continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
		||     IS_AFFECTED(rch,AFF_CHARM)) 
		&&   is_same_group(ch,rch) 
		&&   !is_safe(rch, victim))
		    multi_hit (rch,victim,TYPE_UNDEFINED);
		
		continue;
	    }
  	
	    /* now check the NPC cases */
	    
 	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
	
	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   (IS_NPC(rch) && rch->group && rch->group == ch->group)

		||   (IS_NPC(rch) && rch->race == ch->race 
		   && IS_SET(rch->off_flags,ASSIST_RACE))

		||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
		   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
		     ||  (IS_EVIL(rch)    && IS_EVIL(ch))
		     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 

		||   (rch->pIndexData == ch->pIndexData 
		   && IS_SET(rch->off_flags,ASSIST_VNUM)))

	   	{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

                    if(target != NULL && (!is_safe(rch, target)))
		    {
			do_function(rch, &do_emote, "screams and attacks!");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}	
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance;

    /* decrement the wait */
    if (ch->desc == NULL)
	ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);

    if (ch->desc == NULL)
	ch->daze = UMAX(0,ch->daze - PULSE_VIOLENCE); 


    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
	return;

    if (IS_NPC(ch))
    {
	mob_hit(ch,victim,dt);
	return;
    }

    if (MOUNTED(ch))
    {
    	if (!mount_success(ch, MOUNTED(ch), FALSE))
	    send_to_char("You fall off your mount as you start your attacks!\n\r", ch);
    }
    one_hit( ch, victim, dt, FALSE );
    if(ch->wereform == MOON_FULL && ch->iswere)
    {
	one_hit(ch, victim, dt, FALSE );
	one_hit(ch, victim, dt, FALSE );
	one_hit(ch, victim, dt, FALSE );
    }
    if (get_eq_char (ch, WEAR_SECONDARY))
    {
        one_hit( ch, victim, dt, TRUE );
        if ( ch->fighting != victim )
            return;
    }

    if (ch->fighting != victim)
	return;

    if (IS_AFFECTED(ch,AFF_HASTE))
	one_hit(ch,victim,dt, FALSE);

    if ( ch->fighting != victim || dt == skill_lookup("backstab"))
	return;

    chance = get_skill(ch,skill_lookup("second attack"))/2;

    if (IS_AFFECTED(ch,AFF_SLOW))
	chance /= 2;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve(ch,skill_lookup("second attack"),TRUE,5);
	if ( ch->fighting != victim )
	    return;
    }

    chance = get_skill(ch,skill_lookup("third attack"))/4;

    if (IS_AFFECTED(ch,AFF_SLOW))
	chance = 0;;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve(ch,skill_lookup("third attack"),TRUE,6);
	if ( ch->fighting != victim )
	    return;
    }

    chance = get_skill(ch,skill_lookup("fourth attack"))/4;

    if (IS_AFFECTED(ch,AFF_SLOW))
        chance = 0;;    

    if ( number_percent( ) < chance )
    {
        one_hit( ch, victim, dt, FALSE );
        check_improve(ch,skill_lookup("fourth attack"), TRUE,6);
        if ( ch->fighting != victim )
            return;
    }

    chance = get_skill(ch,skill_lookup("fifth attack"))/4;

    if (IS_AFFECTED(ch,AFF_SLOW))
        chance = 0;;    

    if ( number_percent( ) < chance )
    {
        one_hit( ch, victim, dt, FALSE );
        check_improve(ch,skill_lookup("fifth attack"), TRUE,6);     
        if ( ch->fighting != victim )
            return;
    }

    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int chance,number;
    CHAR_DATA *vch, *vch_next;

    one_hit(ch,victim,dt, FALSE);

    if (ch->fighting != victim)
	return;

    /* Area attack -- BALLS nasty! */
 
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next;
	    if ((vch != victim && vch->fighting == ch))
		one_hit(ch,vch,dt, FALSE);
	}
    }

    if (IS_AFFECTED(ch,AFF_HASTE) 
    ||  (IS_SET(ch->off_flags,OFF_FAST) && !IS_AFFECTED(ch,AFF_SLOW)))
	one_hit(ch,victim,dt, FALSE);

    if (ch->fighting != victim || dt == skill_lookup("backstab"))
	return;

    chance = get_skill(ch,skill_lookup("second attack"))/2;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
	chance /= 2;

    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt, FALSE);
	if (ch->fighting != victim)
	    return;
    }

    chance = get_skill(ch,skill_lookup("third attack"))/4;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
	chance = 0;

    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt, FALSE);
	if (ch->fighting != victim)
	    return;
    } 

    chance = get_skill(ch,skill_lookup("fourth attack"))/4;      

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
        chance = 0;

    if (number_percent() < chance)
    {
        one_hit(ch,victim,dt, FALSE);
        if (ch->fighting != victim)
            return;
    }

    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
	return;

    number = number_range(0,2);

    if (number == 1 && IS_SET(ch->act,ACT_MAGE))
    {
	/*  { mob_cast_mage(ch,victim); return; } */ ;
    }

    if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
    {	
	/* { mob_cast_cleric(ch,victim); return; } */ ;
    }

    /* now for the skills */

    number = number_range(0,8);

    switch(number) 
    {
    case (0) :
	if (IS_SET(ch->off_flags,OFF_BASH))
	    do_function(ch, &do_bash, "");
	break;

    case (1) :
	if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
	    do_function(ch, &do_berserk, "");
	break;


    case (2) :
	if (IS_SET(ch->off_flags,OFF_DISARM) 
	|| (get_weapon_sn(ch) != skill_lookup("hand to hand") 
	&& (IS_SET(ch->act,ACT_WARRIOR)
   	||  IS_SET(ch->act,ACT_THIEF))))
	    do_function(ch, &do_disarm, "");
	break;

    case (3) :
	if (IS_SET(ch->off_flags,OFF_KICK))
	    do_function(ch, &do_kick, "");
	break;

    case (4) :
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
	    do_function(ch, &do_dirt, "");
	break;

    case (5) :
	if (IS_SET(ch->off_flags,OFF_TAIL))
	{
	    do_function(ch, &do_tail, "");
	}
	break; 

    case (6) :
	if (IS_SET(ch->off_flags,OFF_TRIP))
	    do_function(ch, &do_trip, "");
	break;

    case (7) :
	if (IS_SET(ch->off_flags,OFF_CRUSH))
	{
	    /* do_function(ch, &do_crush, "") */ ;
	}
	break;
    case (8) :
	if (IS_SET(ch->off_flags,OFF_BACKSTAB))
	{
	    do_function(ch, &do_backstab, "");
	}
        break;
    case (9) :
        if (IS_SET(ch->off_flags,OFF_BLOODLUST))
        {
            do_function(ch, &do_bloodlust, "");
        }
    case (10) :
        if (IS_SET(ch->off_flags,OFF_FEED))
        { 
            do_function(ch, &do_feed, "");
        }
    }
}
	

/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;
    bool result;

    sn = -1;


    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    /*
     * Figure out the type of damage message.
     */

    if (!secondary)
        wield = get_eq_char( ch, WEAR_WIELD );
    else
        wield = get_eq_char( ch, WEAR_SECONDARY );

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else if(ch->wereform == MOON_FULL && ch->iswere)
  	     dt += attack_table[attack_lookup("claw")].damage;
	else
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    {
	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else if (ch->wereform == MOON_FULL && ch->iswere)
	    dam_type = attack_table[attack_lookup("claw")].damage;
	else
    	    dam_type = attack_table[ch->dam_type].damage;
    }
    else
        dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn(ch);
    skill = 20 + get_weapon_skill(ch,sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = 20;
	thac0_32 = -4;   /* as good as a thief */ 
	if (IS_SET(ch->act,ACT_WARRIOR))
	    thac0_32 = -10;
	else if (IS_SET(ch->act,ACT_THIEF))
	    thac0_32 = -4;
	else if (IS_SET(ch->act,ACT_CLERIC))
	    thac0_32 = 2;
	else if (IS_SET(ch->act,ACT_MAGE))
	    thac0_32 = 6;
    }
    else
    {
	thac0_00 = get_thac00(ch);
	thac0_32 = get_thac32(ch);
    }
    thac0  = interpolate( ch->level, thac0_00, thac0_32 );

    if (thac0 < 0)
        thac0 = thac0/2;

    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL(ch) * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == skill_lookup("backstab"))
	thac0 -= 10 * (100 - get_skill(ch,skill_lookup("backstab")));

    if (dt == skill_lookup("throatchop") )
    	thac0 -= 10 * (90 - get_skill(ch, skill_lookup("throatchop" )));

    if (dt == skill_lookup("cleave"))
        thac0 -= 10 * (100 - get_skill(ch,skill_lookup("cleave")));

    if (dt == skill_lookup("circle"))
        thac0 -= 10 * (100 - get_skill(ch,skill_lookup("circle")));

    if (dt == skill_lookup("whirlwind"))
        thac0 -= 10 * (100 - get_skill(ch,skill_lookup("whirlwind")));

    if (dt == skill_lookup("assassinate"))
        thac0 -= 10 * (100 - get_skill(ch,skill_lookup("assassinate"))); 

    if (dt == skill_lookup("spin"))
        thac0 -= 10 * (100 - get_skill(ch,skill_lookup("spin")));

    if (dt == skill_lookup("rage"))
        thac0 -= 10 * (100 - get_skill(ch,skill_lookup("rage")));

    switch(dam_type)
    {
	case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
	case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
	case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
	default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
    }; 
	
    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;
     
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;
 
    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if(number_range(1,10) > 6 && IS_WEREWOLF(ch))
	diceroll = 19;
    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	damage( ch, victim, 0, dt, dam_type, TRUE );
        rnd_sound( 6, ch, "miss.wav", "miss1.wav", "miss2.wav", "miss3.wav", "miss4.wav","miss5.wav" );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
	if (!ch->pIndexData->new_format)
	{
	    dam = number_range( ch->level / 2, ch->level * 3 / 2 );
	    if ( wield != NULL )
	    	dam += dam / 2;
	}
	else
	    dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);
	
    else
    {
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);
	if ( wield != NULL )
	{
	    if (wield->pIndexData->new_format)
		dam = dice(wield->value[1],wield->value[2]) * skill/100;
	    else
	    	dam = number_range( wield->value[1] * skill/100, 
				wield->value[2] * skill/100);

	    if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
		dam = dam * 11/10;

	    /* sharpness! */
	    if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
	    {
		int percent;

		if ((percent = number_percent()) <= (skill / 8))
		    dam = 2 * dam + (dam * 2 * percent / 100);
	    }
	}
	else if( ch->wereform == MOON_FULL && ch->iswere)
	    dam = number_range( ch->level / 2,  200 );
	else
	    dam = number_range( 1 + 4 * skill/100, 2 * ch->level/3 * skill/100);
    }

    /*
     * Bonuses.
     */
    if ( get_skill(ch,skill_lookup("enhanced damage")) > 0 )
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch,skill_lookup("enhanced damage")))
        {
            check_improve(ch,skill_lookup("enhanced damage"),TRUE,6);
            dam += 2 * ( dam * diceroll/300);
        }
    }

    if ( get_skill(ch,skill_lookup("ultra damage")) > 0 )
    {
          diceroll = number_percent();
          if (diceroll <= get_skill(ch,skill_lookup("ultra damage")))
          {
            check_improve(ch,skill_lookup("ultra damage"),TRUE,6);
            dam += dam * diceroll/100;
          }
    }         

    if ( get_skill(ch,skill_lookup("hero damage")) > 0 )
    {
          diceroll = number_percent();
          if (diceroll <= get_skill(ch,skill_lookup("hero damage")))
          {
            check_improve(ch,skill_lookup("hero damage"),TRUE,6);
            dam += dam * diceroll/80;
          }
    }          

    if ( check_critical(ch,victim) )
        dam *= 1.40;

    if ( !IS_AWAKE(victim) )
	dam *= 2;
     else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    if ( dt == skill_lookup("backstab") && wield != NULL) 
    {
    	if ( wield->value[0] != 2 )
	    dam *= 2 + (ch->level / 10); 
	else 
	    dam *= 2 + (ch->level / 8);
    }
    if ( dt == skill_lookup("throatchop") && wield != NULL )
    	dam *= 2 +(ch->level/5 );
    if ( dt == skill_lookup("cleave") && wield != NULL)
    {      
        if ( wield->value[0] != 2 )
            dam *= 2 + (ch->level / 14);
        else
            dam *= 2 + (ch->level / 10);
    }     

    if ( dt == skill_lookup("circle") && wield != NULL)
    {
        if ( wield->value[0] != 2 )
           dam *= 2 + (ch->level / 10);
        else
           dam *= 2 + (ch->level / 8);
    }

    if ( dt == skill_lookup("whirlwind") && wield != NULL)
    {
        if ( wield->value[0] != 2 )
           dam *= 2 + (ch->level / 10);
        else
           dam *= 2 + (ch->level / 8);
    }     
   
    if ( dt == skill_lookup("assassinate") && wield != NULL)
    {
        if ( wield->value[0] != 2 )
           dam *= 2 + (ch->level / 10);
        else
           dam *= 2 + (ch->level / 8);
    }

    if ( dt == skill_lookup("spin") && wield != NULL)
    {
        if ( wield->value[0] != 2 )
           dam *= 2 + (ch->level / 10);
        else
           dam *= 2 + (ch->level / 8);
    }          

    if ( dt == skill_lookup("rage") && wield != NULL)
    {
        if ( wield->value[0] != 2 )
           dam *= 2 + (ch->level / 10);
        else
           dam *= 2 + (ch->level / 8);
    }    

    dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

    if ( dam <= 0 )
	dam = 1;

    if ( !check_counter( ch, victim, dam, dt ) )
        result = damage( ch, victim, dam, dt, dam_type, TRUE );

    else return;
    
    /* but do we have a funky weapon? */
    if (result && wield != NULL)
    { 
	int dam;

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON))
	{
	    int level;
	    AFFECT_DATA *poison, af;

	    if ((poison = affect_find(wield->affected,skill_lookup("poison"))) == NULL)
		level = wield->level;
	    else
		level = poison->level;
	
	    if (!saves_spell(level / 2,victim,DAM_POISON)) 
	    {
		send_to_char("You feel poison coursing through your veins.",
		    victim);
		act("$n is poisoned by the venom on $p.",
		    victim,wield,NULL,TO_ROOM);

    		af.where     = TO_AFFECTS;
    		af.type      = skill_lookup("poison");
    		af.level     = level * 3/4;
    		af.duration  = level / 2;
    		af.location  = APPLY_STR;
    		af.modifier  = -1;
    		af.bitvector = AFF_POISON;
    		affect_join( victim, &af );
	    }

	    /* weaken the poison if it's temporary */
	    if (poison != NULL)
	    {
	    	poison->level = UMAX(0,poison->level - 2);
	    	poison->duration = UMAX(0,poison->duration - 1);
	
	    	if (poison->level == 0 || poison->duration == 0)
		    act("The poison on $p has worn off.",ch,wield,NULL,TO_CHAR);
	    }
 	}

    	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
	{
	    dam = number_range(1, wield->level / 5 + 1);
	    xact_new("$p draws life from $n.",victim,wield,NULL,TO_ROOM, POS_RESTING, SUPPRESS_FLAGS);
	    xact_new("You feel $p drawing your life away.",
		victim,wield,NULL,TO_CHAR,  POS_RESTING, SUPPRESS_FLAGS);
	    damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
	    ch->hit += dam/2;
	}

        /* Drain mana away from the victim.  Should be a pretty low rate - Marquoz
        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_MANA_DRAIN))
        {
            dam = UMIN(number_range(1, wield->level / 10 + 1), victim->mana);
            xact_new("$p drains $n's force.",victim,wield,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAGS);
            xact_new("You feel $p draining your force away.",
                victim,wield,NULL,TO_CHAR,POS_RESTING,SUPPRESS_FLAGS);
            victim->mana -= dam;
            ch->mana += dam;
        }
        */ /* Commented out pending approval */
        
        /* Is the weapon coated with acid? - Marquoz
         * Needs a better damage line =(
        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_ACIDIC))
        {
            dam = number_range(1,wield->level / 5 + 1);
            xact_new("$n is acidified by $p.",victim,wield,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAGS);
            xact_new("$p burns your flesh with acid.",victim,wield,NULL,TO_ROOM,POS_RESTING,SUPRESS_FLAGS);
            acid_effect((void *)victim,wield->level/2,dam,TARGET_CHAR);
            damage(ch,victim,dam,0,DAM_ACID,FALSE);
        }
        */ /* Commented out pending approval */
        
	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING))
	{
	    dam = number_range(1,wield->level / 4 + 1);
	    xact_new("$n is burned by $p.",victim,wield,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAGS);
	    xact_new("$p sears your flesh.",victim,wield,NULL,TO_CHAR,POS_RESTING,SUPPRESS_FLAGS);
	    fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_FIRE,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST))
	{
	    dam = number_range(1,wield->level / 6 + 2);
	    xact_new("$p freezes $n.",victim,wield,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAGS);
	    xact_new("The cold touch of $p surrounds you with ice.",
		victim,wield,NULL,TO_CHAR,POS_RESTING,SUPPRESS_FLAGS);
	    cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_COLD,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
	{
	    dam = number_range(1,wield->level/5 + 2);
	    xact_new("$n is struck by lightning from $p.",victim,wield,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAGS);
	    xact_new("You are shocked by $p.",victim,wield,NULL,TO_CHAR,POS_RESTING,SUPPRESS_FLAGS);
	    shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
	}
    }

    tail_chain( );
    return;
}


/*
 * Inflict damage from a hit.
 */
bool damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,
	    bool show) 
{
    OBJ_DATA *corpse;
    bool immune;
    OBJ_DATA *wield;
    PET_DATA *pEt;

    wield = get_eq_char( ch, WEAR_WIELD ); 

    if ( victim->position == POS_DEAD )
	return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 2000 && dt >= TYPE_HIT)
    {
	bug( "Damage: %d: more than 7950 points!", dam );
	dam = 7950;
	if (!IS_IMMORTAL(ch))
	{
	    OBJ_DATA *obj;
	    obj = get_eq_char( ch, WEAR_WIELD );
	    send_to_char("You really shouldn't cheat.\n\r",ch);
	    if (obj != NULL)
	    	extract_obj(obj);
	}

    }

    /* damage reduction */
    if ( dam > 35)
	dam = (dam - 35)/2 + 35;
    if ( dam > 80)
	dam = (dam - 80)/2 + 80; 
    if ( dam > 0 && ch->levelflux > 0 )
        dam += levelflux_dam( ch );

    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;
	check_killer( ch, victim );

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
            {
		set_fighting( victim, ch );
               	if ( IS_NPC( victim ) && HAS_TRIGGER_MOB( victim, TRIG_KILL ) )
		    p_percent_trigger( victim, NULL, NULL, ch, NULL, NULL, TRIG_KILL );
            }

	    if (victim->timer <= 4)
	    	victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, skill_lookup("invis") );
	affect_strip( ch, skill_lookup("mass invis") );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }

    /*
     * Damage modifiers.
     */

    /* Make sure the player has a mirror, and then check for the damage
       to be a spell, easier not to do a check */
    if ( dam > 1 && IS_AFFECTED2(victim,AFF_MIRROR_SHIELD) && skill_table[dt].spell_fun != NULL)
    {
        AFFECT_DATA *paf;
        if (number_percent() > 75)
        {
            act_new("$n's mirror shield reflects $t at you!",ch, skill_table[dt].name,victim,TO_CHAR,POS_RESTING);
            act_new("Your mirror shield turns $t back on $N!",ch,skill_table[dt].name,victim,TO_VICT,POS_RESTING);
            /* Lower duration of the shield due to it being hit */
            for (paf = victim->affected; paf != NULL; paf = paf->next)
            {
                if (paf->bitvector == AFF_MIRROR_SHIELD)
                    paf->duration -= ch->level / 10;
                else
                    continue;
            }
            /* Everything should be the same just directed at the caster.  Change
               the dt to MIRROR to avoid a mirrored bounce back fest =) */
            damage(ch,ch,dam,TYPE_MIRROR,dam_type,show);
            return FALSE;
        }
    }
            
    if ( dam > 1 && !IS_NPC(victim) 
    &&   victim->pcdata->condition[COND_DRUNK]  > 10 )
	dam = 9 * dam / 10;

    if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;

    if ( dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
    ||		     (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )))
	dam -= dam / 4;

    if ( dam > 1 && IS_AFFECTED2(victim, AFF_DIVINE_PROTECTION) )
        dam *= .75;

	if ( dam > 1 && IS_AFFECTED2( victim, AFF_DARKSHIELD ))	
		dam *= .75;
	
    immune = FALSE;


    /*
     * Check for parry, and dodge.
     */
    if ( dt >= TYPE_HIT && ch != victim && (number_range(1,10) < 4 && IS_WEREWOLF(ch) ) )
    {
        if ( check_parry( ch, victim ) )
        {
            OBJ_DATA *tmp1 = get_eq_char(victim, WEAR_WIELD);
            OBJ_DATA *tmp2 = get_eq_char(victim, WEAR_SECONDARY);
            if (tmp1 != NULL)
                check_damage_obj (victim, tmp1, 1);
            if (tmp2 != NULL)
                check_damage_obj (victim, tmp2, 1);
            return FALSE;
        }        
	if ( check_dodge( ch, victim ) )
	    return FALSE;
	if ( check_shield_block(ch,victim))
        {
            check_damage_obj(victim, get_eq_char(victim, WEAR_SHIELD), 2);
            check_damage_obj(ch, wield, 2); 
            return FALSE;
        }      
        if ( check_steel_skin( ch, victim ) )
	    return FALSE;
        if ( check_phase( ch, victim) )
            return FALSE;
    
        if(IS_AFFECTED2 (victim, AFF_FORCE_SHIELD) && check_force_shield(ch, victim))
 	   return FALSE;
	if(IS_AFFECTED2 (victim, AFF_STATIC_SHIELD) && check_static_shield(ch, victim))
	   return FALSE;
    }

    if(IS_AFFECTED2(victim, AFF_FLAME_SHIELD) && dam_type <= 3)
       check_flame_shield(ch, victim);

    check_damage_obj(victim, NULL, 2);
    check_damage_obj(ch, wield, 2); 

    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):	
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    if ( !IS_NPC( ch ) )
    {
        if (IS_NPC(victim) && quad_damage)
        dam = dam * 4;

        if ( dam > ch->pcdata->mdam )
        dam = ch->pcdata->mdam;

        if ( ch->pcdata->hdam < dam )
        ch->pcdata->hdam = dam;   
    }


    if (show)
    	dam_message( ch, victim, dam, dt, immune );

    if (dam == 0)
	return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */

    if(victim->mana > dam && IS_AFFECTED2(victim,AFF_LIFEFORCE))
    {
       victim->mana -= dam/2;
       victim->hit -= dam/2;
    }

    else
    {
       victim->hit -= dam;
    }

    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
	break;

    case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are incapacitated and will slowly die, if not aided.\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\n\r",
	    victim );
	break;

    case POS_DEAD:
	act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	send_to_char( "You have been KILLED!!\n\r\n\r", victim );
        sound( "killed.wav", victim );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "That really did HURT!\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "You sure are BLEEDING!\n\r", victim );
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {

        if (IS_IN_WAR(ch) && IS_IN_WAR(victim))
        {
            check_war(ch, victim);
            return TRUE;
        }

        if (!IS_NPC(ch) && !IS_NPC(victim) &&
            IS_SET(ch->in_room->room_flags, ROOM_ARENA) &&
            IS_SET(victim->in_room->room_flags, ROOM_ARENA))
        {
            stop_fighting(victim, TRUE);
            death_cry(victim);
            char_from_room(victim);
            char_to_room(victim, get_room_index(ROOM_VNUM_TEMPLE));
            victim->hit = UMAX(1, victim->hit);
            victim->mana = UMAX(1, victim->mana);
            victim->move = UMAX(1, victim->move);
            update_pos(victim);
            do_look(victim, "auto");
            if (ch->in_room->area->nplayer == 1)        
            {
                send_to_char("You emerge victorious in the arena!\n\r", ch);
                stop_fighting(ch, TRUE);
                char_from_room(ch);
                char_to_room(ch, get_room_index(ROOM_VNUM_TEMPLE));
                ch->hit = ch->max_hit;
                ch->mana = ch->max_mana;
                ch->move = ch->max_move;
                update_pos(ch);
                do_look(ch, "auto");
            }
            return TRUE;
        }                     

	group_gain( ch, victim );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		ch->in_room->vnum );
	    log_string( log_buf );

            info( victim, 0, "{B[{GINFO{B]{x: {B%s{x just got {RC{Br{YE{Ga{MM{Re{GD{x by {R%s{x at %s!",
                victim->name, (IS_NPC(ch) ? ch->short_descr : ch->name), ch->in_room->name );

	    /*
	     * Dying penalty:
	     * 2/3 way back to previous level.
	     */
         if(!IS_QUESTOR(victim) || (IS_NPC(ch) && ch->pIndexData->vnum != victim->pcdata->questmob))
         {

	    if ( victim->exp > exp_per_level(victim,victim->pcdata->points) 
			       * victim->level )
	        gain_exp( victim, (2 * (exp_per_level(victim,victim->pcdata->points)
			         * victim->level - victim->exp)/3) + 50 );
         }
	}

    if (!IS_NPC(victim))
    {
        if (IS_NPC(ch))
            victim->pcdata->gamestat[MOB_DEATHS]++;
        else
        {
            victim->pcdata->gamestat[PK_DEATHS]++;
            ch->pcdata->gamestat[PK_KILLS]++;
        }
    }
    if (IS_NPC(victim))
    {
        if (!IS_NPC(ch))
            ch->pcdata->gamestat[MOB_KILLS]++;
    }

        sprintf( log_buf, "%s got toasted by %s at %s [room %d]",
            (IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->name, ch->in_room->vnum);
 
        if (IS_NPC(victim))
            wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
        else
            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

       	if ( IS_NPC( victim ) && HAS_TRIGGER_MOB( victim, TRIG_DEATH) )
	{
	    victim->position = POS_STANDING;
	    p_percent_trigger( victim, NULL, NULL, ch, NULL, NULL, TRIG_DEATH );
	}

        /* Make the head for a bountied PC */
        if(!IS_NPC(victim) && !IS_NPC(ch)
        && is_bountied(victim))
        {
            char buf[MAX_STRING_LENGTH];
            char *name;
            OBJ_DATA *obj;

            name = str_dup(victim->name);
            obj = create_object(get_obj_index(OBJ_VNUM_SEVERED_HEAD),0);

            obj->extra_flags |= ITEM_NODROP|ITEM_NOUNCURSE;

            sprintf(buf, "%s %s", "head", name);
            free_string( obj->name );
            obj->name = str_dup( buf);

            sprintf( buf, obj->short_descr, name );
            free_string( obj->short_descr );
            obj->short_descr = str_dup( buf );

            sprintf( buf, obj->description, name );
            free_string( obj->description );
            obj->description = str_dup( buf );

            obj_to_char(obj,ch);
            free_string(name);
        }

	raw_kill( victim, ch );
 
        if (IS_NPC(ch)) /* is a mob */
	{
	    if (!IS_NPC(victim)) 
		victim->pcdata->deaths_mob += 1;
	}
        else 
	{
	    if IS_NPC(victim)
	    {
		ch->pcdata->kills_mob += 1;
	    }
            else 
	    {
	 	victim->pcdata->deaths_pc += 1;
		ch->pcdata->kills_pc += 1;
            }
	}

        if IS_NPC(victim) 
        {
           if ( !IS_SET(victim->form,FORM_UNDEAD))
   	       check_spirit(ch,victim);
        }

        if ( IS_NPC(victim ) && IS_SET( victim->act, ACT_PET ) ) 
        {
           if ( IS_SET( victim->act, ACT_PET ) ) 
           {
               for ( pEt = pet_list; pEt != NULL; pEt = pEt->next )
               {
                    if ( !str_cmp( victim->name, pEt->name ) && !str_cmp( victim->powner, pEt->cname ) )
                        break;
               }
              
               if ( pEt == NULL )
               {
                   dead_pet( victim, pEt );
               }
           }    
        } 

        /* dump the flags */
        if (ch != victim && !IS_NPC(ch) && !is_same_clan(ch,victim))
        {
            if (IS_SET(victim->act,PLR_KILLER))
                REMOVE_BIT(victim->act,PLR_KILLER);
            else
                REMOVE_BIT(victim->act,PLR_THIEF);
        }

        /* RT new auto commands */

	if (!IS_NPC(ch)
	&&  (corpse = get_obj_list(ch,"corpse",ch->in_room->contents)) != NULL
	&&  corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse))
	{
	    OBJ_DATA *coins;

	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
            {
		do_function(ch, &do_get, "all corpse");
	    }

 	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	        corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
	    {
		if ((coins = get_obj_list(ch,"gcash",corpse->contains))
		     != NULL)
		{
		    do_function(ch, &do_get, "all.gcash corpse");
	      	}
	    }
            
	    if (IS_SET(ch->act, PLR_AUTOSAC))
	    {
       	        if (IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
       	      	{
		    return TRUE;  /* leave if corpse has treasure */
	      	}
	        else
		{
		    do_function(ch, &do_sacrifice, "corpse");
		}
	    }
	}

	return TRUE;
    }

    if ( victim == ch )
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_function(victim, &do_recall, "" );
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	{
	    do_function(victim, &do_flee, "" );
	}
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
    {
	do_function (victim, &do_flee, "" );
    }

    tail_chain( );
    return TRUE;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL)
	return FALSE;

    /* killing mobiles */
    if (IS_NPC(victim))
    {

	/* safe room? */
	if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	{
	    send_to_char("Not in this room.\n\r",ch);
	    return TRUE;
	}

	if (victim->pIndexData->pShop != NULL)
	{
	    send_to_char("The shopkeeper wouldn't like that.\n\r",ch);
	    return TRUE;
	}

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	||  IS_SET(victim->act,ACT_IS_CHANGER))
	{
            printf_to_char( ch, "I don't think ShadowStorm would approve.\n\r" );
	    return TRUE;
	}

	if (!IS_NPC(ch))
	{
	    /* no pets */
	    if (IS_SET(victim->act,ACT_PET))
	    {
		act("But $N looks so cute and cuddly...",
		    ch,NULL,victim,TO_CHAR);
		return TRUE;
	    }

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
	    {
		send_to_char("You don't own that monster.\n\r",ch);
		return TRUE;
	    }
	}
    }
    /* killing players */
    else
    {
	/* NPC doing the killing */
	if (IS_NPC(ch))
	{
	    /* safe room check */
	    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    {
		send_to_char("Not in this room.\n\r",ch);
		return TRUE;
	    }

            if (IS_SET(victim->comm2, COMM_AUTO_AFK ) )
            {
                send_to_char("Not while they are afk.\n\r", ch );
                return TRUE;
            }

	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	    &&  ch->master->fighting != victim)
	    {
		send_to_char("Players are your friends!\n\r",ch);
		return TRUE;
	    }
	}
	/* player doing the killing */
	else
	{


	    if (IS_SET(victim->act,PLR_KILLER) || IS_SET(victim->act,PLR_THIEF))
		return FALSE;

	    if (!is_clan(victim))
	    {
		send_to_char("They aren't in a clan, leave them alone.\n\r",ch);
		return TRUE;
	    }

     	    if (ON_GQUEST(ch))
	    {
		send_to_char("I don't beleive they are on the target list.\n\r", ch);
		return TRUE;
	    }

	    if (ON_GQUEST(victim))
	    {
		send_to_char("They are to closey involved in something right now.\n\r", ch);
		return TRUE;
	    }

            if (IS_SET( victim->comm2, COMM_AUTO_AFK ) )
            {
                send_to_char("Not while they are auto afk.\n\r", ch );
                return TRUE;
            }

	    if (ch->level > victim->level + 8)
	    {
		send_to_char("Pick on someone your own size.\n\r",ch);
		return TRUE;
	    }
	}
    }
    return FALSE;
}
 
bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    if (victim->in_room == NULL || ch->in_room == NULL)
        return TRUE;

    if (victim == ch && area)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL && !area)
	return FALSE;

    /* killing mobiles */
    if (IS_NPC(victim))
    {
	/* safe room? */
	if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    return TRUE;

	if (victim->pIndexData->pShop != NULL)
	    return TRUE;

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	||  IS_SET(victim->act,ACT_IS_CHANGER))
	    return TRUE;

	if (!IS_NPC(ch))
	{
	    /* no pets */
	    if (IS_SET(victim->act,ACT_PET))
	   	return TRUE;

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim,AFF_CHARM) && (area || ch != victim->master))
		return TRUE;

	    /* legal kill? -- cannot hit mob fighting non-group member */
	    if (victim->fighting != NULL && !is_same_group(ch,victim->fighting))
		return TRUE;
	}
	else
	{
	    /* area effect spells do not hit other mobs */
	    if (area && !is_same_group(victim,ch->fighting))
		return TRUE;
	}
    }
    /* killing players */
    else
    {
	if (area && IS_IMMORTAL(victim) && victim->level > LEVEL_IMMORTAL)
	    return TRUE;

	/* NPC doing the killing */
	if (IS_NPC(ch))
	{
	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	    &&  ch->master->fighting != victim)
		return TRUE;
	
	    /* safe room? */
	    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
		return TRUE;

	    /* legal kill? -- mobs only hit players grouped with opponent*/
	    if (ch->fighting != NULL && !is_same_group(ch->fighting,victim))
		return TRUE;
	}

	/* player doing the killing */
	else
	{

	    if (is_safe_war(ch, victim))
	    {
		send_to_char("They're on YOUR team.\n\r", ch);
		return TRUE;
	    }

	    if (IS_SET(victim->in_room->room_flags, ROOM_ARENA))
		return FALSE;

	    if (is_safe_war(ch, victim))
		return TRUE;

	    if (IS_SET(victim->in_room->room_flags, ROOM_ARENA))
		return FALSE;

        if (!is_clan(ch))
		return TRUE;

	    if (IS_SET(victim->act,PLR_KILLER) || IS_SET(victim->act,PLR_THIEF))
		return FALSE;

	    if (!is_clan(victim))
		return TRUE;

            if (ON_GQUEST(ch) || ON_GQUEST(victim))
		return TRUE;

	    if (ch->level > victim->level + 8)
		return TRUE;
	}

    }
    return FALSE;
}
/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];

    if (IS_SET(ch->in_room->room_flags, ROOM_ARENA))
	return;

    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
	victim = victim->master;

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC(victim)
    ||   IS_SET(victim->act, PLR_KILLER)
    ||   IS_SET(victim->act, PLR_THIEF))
	return;

    /*
     * Charm-o-rama.
     */
    if ( IS_SET(ch->affected_by, AFF_CHARM) )
    {
	if ( ch->master == NULL )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, skill_lookup("charm person") );
	    REMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}
/*
	send_to_char( "*** You are now a KILLER!! ***\n\r", ch->master );
  	SET_BIT(ch->master->act, PLR_KILLER);
*/

	stop_follower( ch );
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
    ||   ch == victim
    ||   ch->level >= LEVEL_IMMORTAL
    ||   !is_clan(ch)
    ||   IS_SET(ch->act, PLR_KILLER) 
    ||	 ch->fighting  == victim)
	return;

    send_to_char( "*** You are now a KILLER!! ***\n\r", ch );
    SET_BIT(ch->act, PLR_KILLER);
    sprintf(buf,"$N is attempting to murder %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
    save_char_obj( ch );
    return;
}



/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,skill_lookup("parry")) / 2;

    if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
    {
	if (IS_NPC(victim))
	    chance /= 2;
	else
	    return FALSE;
    }

    if (!can_see(ch,victim))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    act( "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );
    act( "$N parries your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,skill_lookup("parry"),TRUE,6);
    return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
        return FALSE;


    chance = get_skill(victim,skill_lookup("shield block")) / 5 + 3;


    if ( get_eq_char( victim, WEAR_SHIELD ) == NULL )
        return FALSE;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You block $n's attack with your shield.",  ch, NULL, victim, TO_VICT    );
    act( "$N blocks your attack with a shield.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,skill_lookup("shield block"),TRUE,6);
    sound( "hitshld.wav", ch );
    sound( "hitshld.wav", victim );
    return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( MOUNTED(victim) )
	return FALSE;


    chance = get_skill(victim,skill_lookup("dodge")) / 2;

    if (!can_see(victim,ch))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    xact_new( "You dodge $n's attack.", ch, NULL, victim, TO_VICT, POS_RESTING, SUPPRESS_DODGE    );
    xact_new( "$N dodges your attack.", ch, NULL, victim, TO_CHAR, POS_RESTING, SUPPRESS_DODGE    );
    check_improve(victim,skill_lookup("dodge"),TRUE,6);
    return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= -11 )
    {
	victim->position = POS_DEAD;
	return;
    }

         if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, skill_lookup("sleep") );

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    fch->fighting	= NULL;
	    fch->position	= IS_NPC(fch) ? fch->default_pos : POS_STANDING;
	    update_pos( fch );
	}
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
    ROOM_INDEX_DATA *location;
    location = get_room_index ( hometown_table[ch->hometown].morgue );

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	if ( ch->gold > 0 )
	{
	    obj_to_obj( create_money( ch->gold, ch->silver ), corpse );
	    ch->gold = 0;
	    ch->silver = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 25, 40 );
	REMOVE_BIT(ch->act,PLR_CANLOOT);
	if (!is_clan(ch))
	    corpse->owner = str_dup(ch->name);
	else
	{
	    corpse->owner = NULL;
	    if (ch->gold > 1 || ch->silver > 1)
	    {
		obj_to_obj(create_money(ch->gold / 2, ch->silver/2), corpse);
		ch->gold -= ch->gold/2;
		ch->silver -= ch->silver/2;
	    }
	}
		
	corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	bool floating = FALSE;

	obj_next = obj->next_content;
	if (obj->wear_loc == WEAR_FLOAT && !IS_OBJ2_STAT( obj, ITEM_RELIC) )
	    floating = TRUE;
	obj_from_char( obj );
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH) && !floating)
	{
	    obj->timer = number_range(5,10);
	    REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
	}
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);

        if (IS_OBJ_STAT(obj, ITEM_AUCTIONED))
  	    continue;

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    extract_obj( obj );
	else if (floating)
	{
	    if (IS_OBJ_STAT(obj,ITEM_ROT_DEATH)) /* get rid of it! */
	    { 
		if (obj->contains != NULL)
		{
		    OBJ_DATA *in, *in_next;

		    act("$p evaporates,scattering its contents.",
			ch,obj,NULL,TO_ROOM);
		    for (in = obj->contains; in != NULL; in = in_next)
		    {
			in_next = in->next_content;
			obj_from_obj(in);
			obj_to_room(in,ch->in_room);
		    }
		 }
		 else
		    act("$p evaporates.",
			ch,obj,NULL,TO_ROOM);
		 extract_obj(obj);
	    }
	    else
	    {
		act("$p falls to the floor.",ch,obj,NULL,TO_ROOM);
		obj_to_room(obj,ch->in_room);
	    }
	}
	else
	    obj_to_obj( obj, corpse );
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_ARENA))
	return;

    if ( IS_NPC(ch) )
    {
        obj_to_room( corpse,ch->in_room );

    }
    else
    {   obj_to_room( corpse,location );  
    }

    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";

    switch ( number_bits(4))
    {
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: 
	if (ch->material == 0)
	{
	    msg  = "$n splatters blood on your armor.";		
	    break;
	}
    case  2: 							
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "$n spills $s guts all over the floor.";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "$n's severed head plops on the ground.";
	    vnum = OBJ_VNUM_SEVERED_HEAD;				
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "$n's heart is torn from $s chest.";
	    vnum = OBJ_VNUM_TORN_HEART;				
	}
	break;
    case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "$n's arm is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_ARM;				
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "$n's leg is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_LEG;				
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "$n's head is shattered, and $s brains splash all over you.";
	    vnum = OBJ_VNUM_BRAINS;
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
    {
	msg = "You hear something's death cry.";
        rnd_sound( 2, ch, "death_mob1.wav", "death_mob3.wav" );
    }
    else
    {
	msg = "You hear someone's death cry.";
        rnd_sound( 2, ch, "deathcry2.wav", "deathcry3.wav" );       
    }

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill( CHAR_DATA *victim, CHAR_DATA *killer )
{
    int i;

    stop_fighting( victim, TRUE );
    death_cry( victim );
    make_corpse( victim );

    if (!IS_NPC( victim ) )
    {
        victim->pcdata->condition[COND_HUNGER] = 24;   
        victim->pcdata->condition[COND_THIRST] = 30;
    } 

    if (!IS_NPC (victim))
    { 
      if (IS_NPC(killer))  
      {
        kill_table[URANGE (0, killer->level, MAX_LEVEL - 1)].kills++;
        killer->pIndexData->kills++;
      }
    }
    else
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    } 

    extract_char( victim, FALSE );
    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by	= race_table[victim->race].aff;
    for (i = 0; i < 4; i++)
    	victim->armor[i]= 100;
    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
    victim->move	= UMAX( 1, victim->move );
    victim->turned 	= FALSE;
    return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;
    int i;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( victim == ch )
	return;
    
    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
        {
	    members++;
	    group_levels += IS_NPC(gch) ? gch->level / 2 : gch->level;
	}
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level ;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) || (IS_NPC(gch) && !IS_SET( gch->act, ACT_PET ) ) )
	    continue;

        if (IS_QUESTOR(ch) && IS_NPC(victim))
        {
            if (ch->pcdata->questmob == victim->pIndexData->vnum)
            {
                send_to_char("{RYou have almost completed your QUEST!{x\n\r", ch);
                send_to_char("{RReturn to the questmaster before your time runs out!{x\n\r", ch);
                ch->pcdata->questmob = -1;
            }
        }

        if (IS_NPC(victim) && !IS_NPC(ch) &&
	    gquest_info.running == GQUEST_RUNNING && ON_GQUEST(ch) &&
	    (i = is_gqmob(ch, victim->pIndexData->vnum)) != -1)
	{
	    ch->pcdata->gq_mobs[i] = -1;
	    send_to_char("Congratulations, that that mob was part of your global quest!\n\r",ch);
	    send_to_char("You receive an extra 3 Quest Points", ch);
	    ch->pcdata->questpoints += 3;
	    if (chance((MAX_GQUEST_MOB - gquest_info.mob_count) / 2 + gquest_info.mob_count))
	    {
		send_to_char(" and a Trivia Point!\n\r", ch);
		ch->pcdata->trivia += 1;
	    }
	    else
		send_to_char(".\n\r", ch);
	    sprintf(buf, "$N has killed %s, a global questmob.", victim->short_descr);
	    wiznet(buf, ch, NULL, 0, 0, 0);

	    if (count_gqmobs(ch) == gquest_info.mob_count)
		send_to_char("You are now ready to complete the global quest. Type 'GQUEST COMPLETE' to finish.\n\r", ch);
	}

	xp = xp_compute( gch, victim, group_levels ); 
             
        if ( !IS_NPC(gch) && ch->level < LEVEL_HERO && !IS_SET( gch->comm2, COMM_AUTOLEVEL ) && gch->pcdata->current_exp >= exp_per_level( gch, gch->pcdata->points ) * 6 )
            send_to_char("Please go and level at the healer!\n\r", gch );

        else
	{
	    sprintf( buf, "You receive %d experience points.\n\r", xp );
	    send_to_char( buf, gch );
	    gain_exp( gch, xp );
	}

	for ( obj = gch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

            if ( IS_OBJ2_STAT(obj, ITEM_RELIC) && obj->xp_tolevel > 0 ) 
            {
                 if ( number_range(0, 1) <= 1 ) 
                 { 
                    gain_object_exp( ch, obj, xp );
                    if ( chance(10) )                    
                    printf_to_char( ch, "%s whispers your doing good %s only %d more xp until I level.\n\r", obj->name, ch->name, obj->xp_tolevel );  
                 } 
            }
  
	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(gch) )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(gch) )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(gch) ) )
	    {
		act( "You are zapped by $p.", gch, obj, NULL, TO_CHAR );
		act( "$n is zapped by $p.",   gch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
         
                if ( IS_OBJ_STAT(obj,ITEM_NODROP) )
                   obj_to_char( obj, gch );
                else
	 	   obj_to_room( obj, gch->in_room );
	    }
	}
    }

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
{
    int xp,base_exp;
    int align,level_range;
    int time_per_level;
    int bonus;

    level_range = victim->level - gch->level;
 
    /* compute the base exp */
    switch (level_range)
    {
 	default : 	base_exp =   0;		break;
	case -9 :	base_exp =   1;		break;
	case -8 :	base_exp =   2;		break;
	case -7 :	base_exp =   5;		break;
	case -6 : 	base_exp =   9;		break;
	case -5 :	base_exp =  11;		break;
	case -4 :	base_exp =  22;		break;
	case -3 :	base_exp =  33;		break;
	case -2 :	base_exp =  50;		break;
	case -1 :	base_exp =  66;		break;
	case  0 :	base_exp =  83;		break;
	case  1 :	base_exp =  99;		break;
	case  2 :	base_exp = 121;		break;
	case  3 :	base_exp = 143;		break;
	case  4 :	base_exp = 165;		break;
    } 
    
    if (level_range > 4)
	base_exp = 160 + 20 * (level_range - 4);

    if(IS_NPC(victim) )/* at max a mob with all of these is worth 5 levels above their own*/
    {
       if( IS_AFFECTED( victim, AFF_SANCTUARY ) ) 
          base_exp = (base_exp * 130) / 100;
       if( IS_AFFECTED( victim, AFF_HASTE ) )
          base_exp = (base_exp * 120) / 100;
       if( IS_SET(victim->off_flags,OFF_AREA_ATTACK) ) 
          base_exp = (base_exp * 120) / 100;
       if( IS_SET(victim->off_flags,OFF_BACKSTAB) ) 
          base_exp = (base_exp * 120) / 100;
       if( IS_SET(victim->off_flags,OFF_FAST) )
          base_exp = (base_exp * 120) / 100;
       if( IS_SET(victim->off_flags,OFF_DODGE) )
          base_exp = (base_exp * 110) / 100;
       if( IS_SET(victim->off_flags,OFF_PARRY) )
          base_exp = (base_exp * 110) / 100;

       if( victim->spec_fun != 0 )
       {
          if(   !str_cmp(spec_name(victim->spec_fun),"spec_breath_any")
             || !str_cmp(spec_name(victim->spec_fun),"spec_breath_acid")
             || !str_cmp(spec_name(victim->spec_fun),"spec_breath_fire")
             || !str_cmp(spec_name(victim->spec_fun),"spec_breath_frost")
             || !str_cmp(spec_name(victim->spec_fun),"spec_breath_gas")
             || !str_cmp(spec_name(victim->spec_fun),"spec_breath_lightning") )
             base_exp = (base_exp * 125) / 100;
           
          else if(   !str_cmp(spec_name(victim->spec_fun),"spec_cast_cleric")
                  || !str_cmp(spec_name(victim->spec_fun),"spec_cast_mage")
                  || !str_cmp(spec_name(victim->spec_fun),"spec_cast_undead") ) 
                base_exp = (base_exp * 120) / 100;
         
          else if( !str_cmp(spec_name(victim->spec_fun),"spec_poison") )
                base_exp = (base_exp * 110) / 100;
       }
    }

    /* do alignment computations */
   
    align = victim->alignment - gch->alignment;

    if (IS_SET(victim->act,ACT_NOALIGN))
    {
	/* no change */
    }

    
    /* calculate exp multiplier */
    if (IS_SET(victim->act,ACT_NOALIGN))
	xp = base_exp;

    else if (gch->alignment > 500)  /* for goodie two shoes */
    {
	if (victim->alignment < -750)
	    xp = (base_exp *4)/3;
   
 	else if (victim->alignment < -500)
	    xp = (base_exp * 5)/4;

        else if (victim->alignment > 750)
	    xp = base_exp / 4;

   	else if (victim->alignment > 500)
	    xp = base_exp / 2;

        else if (victim->alignment > 250)
	    xp = (base_exp * 3)/4; 

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -500) /* for baddies */
    {
	if (victim->alignment > 750)
	    xp = (base_exp * 5)/4;
	
  	else if (victim->alignment > 500)
	    xp = (base_exp * 11)/10; 

   	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < -500)
	    xp = (base_exp * 3)/4;

	else if (victim->alignment < -250)
	    xp = (base_exp * 9)/10;

	else
	    xp = base_exp;
    }

    else if (gch->alignment > 200)  /* a little good */
    {

	if (victim->alignment < -500)
	    xp = (base_exp * 6)/5;

 	else if (victim->alignment > 750)
	    xp = base_exp/2;

	else if (victim->alignment > 0)
	    xp = (base_exp * 3)/4; 
	
	else
	    xp = base_exp;
    }

    else if (gch->alignment < -200) /* a little bad */
    {
	if (victim->alignment > 500)
	    xp = (base_exp * 6)/5;
 
	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < 0)
	    xp = (base_exp * 3)/4;

	else
	    xp = base_exp;
    }

    else /* neutral */
    {

	if (victim->alignment > 500 || victim->alignment < -500)
	    xp = (base_exp * 4)/3;

	else if (victim->alignment < 200 && victim->alignment > -200)
	    xp = base_exp/2;

 	else
	    xp = base_exp;
    }

    /* more exp at the low levels */
    if (gch->level < 70)
    	xp = 9 * xp / (gch->level + 4);
 
    /* reduce for playing time */
    
    {
	/* compute quarter-hours per level */
	time_per_level = 4 *
			 (gch->played + (int) (current_time - gch->logon))/3600
			 / gch->level;

	time_per_level = URANGE(5,time_per_level,15);
	if (gch->level < 25)  /* make it a curve */
	    time_per_level = UMAX(time_per_level,(15 - gch->level));
	xp = xp * time_per_level / 4;
    }
   
    /* randomize the rewards */
    xp = number_range (xp * 3/4, xp * 5/4);

    /* adjust for grouping */
    xp = xp * gch->level/( UMAX(1,total_levels -1) );

    bonus = xp;

    if (double_exp)
    {
        xp += bonus;
    }

    return xp;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;

    if (ch == NULL || victim == NULL)
	return;

	 if ( dam ==   0 ) { vs = "{Wmiss{x";	vp = "{Wmisses{x";		}
    else if ( dam <=   4 ) { vs = "{Yscratch{x";vp = "{Yscratches{x";	}
    else if ( dam <=   8 ) { vs = "{Cgraze{x";	vp = "{Cgrazes{x";		}
    else if ( dam <=  12 ) { vs = "{Rhit{x";	vp = "{Rhits{x";		}
    else if ( dam <=  16 ) { vs = "{Binjure{x";	vp = "{Binjures{x";		}
    else if ( dam <=  20 ) { vs = "{Ywound{x";	vp = "{Ywounds{x";		}
    else if ( dam <=  24 ) { vs = "{Rmaul{x";   vp = "{Rmauls{x";		}
    else if ( dam <=  28 ) { vs = "{Mdecimate{x";vp = "{Mdecimates{x";	}
    else if ( dam <=  32 ) { vs = "{Bdevastate{x";vp = "{Bdevastates{x";	}
    else if ( dam <=  36 ) { vs = "{Cmaim{x";	vp = "{Cmaims{x";		}
    else if ( dam <=  44 ) { vs = "{YMUTILATE{x";vp = "{YMUTILATES{x";	}
    else if ( dam <=  48 ) { vs = "{RDISEMBOWEL{x";vp = "{RDISEMBOWELS{x";	}
    else if ( dam <=  52 ) { vs = "{BDISMEMBER{x";vp = "{BDISMEMBERS{x";	}
    else if ( dam <=  56 ) { vs = "{CMASSACRE{x";vp = "{CMASSACRES{x";	}
    else if ( dam <=  60 ) { vs = "{YMANGLE{x";	vp = "{YMANGLES{x";		}
    else if ( dam <=  65 ) { vs = "{B*** MASSACRE ***{x";                            
                             vp = "{B*** MASSACRES ***{x";                  }
    else if ( dam <=  75 ) { vs = "{B*** DEVASTATE ***{x";
			     vp = "{B*** DEVASTATES ***{x";  		}
    else if ( dam <= 100 ) { vs = "{G*** DEMOLISH ***{x";
                             vp = "{G*** DEMOLISHES ***{x";                 }   
    else if ( dam <= 125)  { vs = "{Y*** OBLITERATE ***{x";
			     vp = "{Y*** OBLITERATES ***{x";		}
    else if ( dam <= 150)  { vs = "{Y>>> ANNIHILATE <<<{x";
			     vp = "{Y>>> ANNIHILATES <<<{x";		}
    else if ( dam <= 200)  { vs = "{C<<< ERADICATE >>>{x";
			     vp = "{C<<< ERADICATES >>>{x";			}
    else if ( dam <= 300)  { vs = "{G<--> {YVAPORIZE {G<-->{x";
                             vp = "{G<--> {YVAPORIZES {G<-->{x";      }    
    else if ( dam <= 400)  { vs = "{G<*><*><*> ELIMINATE <*><*><*>{x";
                             vp = "{G<*><*><*> ELIMINATES <*><*><*>{x";     }     
    else if ( dam <= 500)  { vs = "{Y<*><*><*> DESTROY <*><*><*>{x";             
                             vp = "{Y<*><*><*> DESTROYS <*><*><*>{x";       }
    else if ( dam <= 600 ) { vs = "{C<+><+><+> DISINTEGRATE <+><+><+>{x";
                             vp = "{C<+><+><+> DISINTEGRATES <+><+><+>{x";      }     
    else if ( dam <= 700 ) { vs = "{m_{r\\{R|{r/{m__{r\\{R|{r/{m_{mE{WX{mT{WE{mR{WM{mI{WN{mA{WT{mE {m_{r\\{R|{r/{m__{r\\{R|{r/{m_{x";
                             vp = "{m_{r\\{R|{r/{m__{r\\{R|{r/{m_{mE{WX{mT{WE{mR{WM{mI{WN{mA{WT{mE{WS {m_{r\\{R|{r/{m__{r\\{R|{r/{m_{x"; }   
    else                   { vs = "do {RU{Bn{GI{Ym{BA{Rg{I{Yn{BA{Rb{BL{GE{x things to";
			     vp = "does {RU{BnI{Ym{BA{Rg{GI{Yn{BA{Rb{BLE{x things to";		}

    punct   = (dam <= 24) ? '.' : '!';

    if(dt == TYPE_UNDEFINED  )
       dt = TYPE_HIT;

    if ( dt == TYPE_HIT )
    {
	if (ch  == victim)
	{
	    sprintf( buf1, "{3$n %s $melf%c{x",vp,punct);
	    sprintf( buf2, "{2You %s yourself%c{x",vs,punct);
	}
	else
	{
	    sprintf( buf1, "{3$n %s $N%c{x",  vp, punct );
	    sprintf( buf2, "{2You %s $N%c{x", vs, punct );
	    sprintf( buf3, "{4$n %s you%c{x", vp, punct );
	}
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack	= skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt < TYPE_HIT + MAX_DAMAGE_MESSAGE) 
	    attack	= attack_table[dt - TYPE_HIT].noun;
	else
	{
	    if(dt != 1000 )
	    {   bug( "Dam_message: bad dt %d.", dt );
	    	dt  = TYPE_HIT;
	    	attack  = attack_table[0].name;
	    }
	}

	if (immune)
	{
	    if (ch == victim)
	    {
		sprintf(buf1,"{3$n is unaffected by $s own %s.{x",attack);
		sprintf(buf2,"{2Luckily, you are immune to that.{x");
	    } 
	    else
	    {
	    	sprintf(buf1,"{3$N is unaffected by $n's %s!{x",attack);
	    	sprintf(buf2,"{2$N is unaffected by your %s!{x",attack);
	    	sprintf(buf3,"{4$n's %s is powerless against you.{x",attack);
	    }
	}

	else
	{
          if ( IS_SET(ch->act, PLR_AUTODAMAGE))
          {
	    if (ch == victim )
	    {
		sprintf( buf1, "{3$n's %s %s $m%c{x {W[{B%d{W]{x",attack,vp,punct, dam);
		sprintf( buf2, "{2Your %s %s you%c{x {W[{B%d{w]{x",attack,vp,punct, dam);
	    }

            else
            {
	    	sprintf( buf1, "{3$n's %s %s $N%c{x {W[{B%d{W]{x",  attack, vp, punct, dam );
	    	sprintf( buf2, "{2Your %s %s $N%c {W[{B%d{W]{x", attack, vp, punct, dam );
	    	sprintf( buf3, "{4$n's %s %s you%c {W[{R%d{W]{x", attack, vp, punct, dam );
	    }
          }

          else 
          {
            if ( ch == victim )
            {
		sprintf( buf1, "{3$n's %s %s $m%c{x",attack,vp,punct );
		sprintf( buf2, "{2Your %s %s you%c{x",attack,vp,punct );
            }

            else
            {
	        sprintf( buf1, "{3$n's %s %s $N%c{x",  attack, vp, punct);
    	        sprintf( buf2, "{2Your %s %s $N%c{x",  attack, vp, punct);
                sprintf( buf3, "{4$n's %s %s you%c{x", attack, vp, punct);
            }
          }
        }
    }

    if (ch == victim)
    {
	act(buf1,ch,NULL,NULL,TO_ROOM);
	act(buf2,ch,NULL,NULL,TO_CHAR);
    }

    else
    {
    	act( buf1, ch, NULL, victim, TO_NOTVICT );
    	act( buf2, ch, NULL, victim, TO_CHAR );
    	act( buf3, ch, NULL, victim, TO_VICT );
    }

    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE) || IS_OBJ_STAT( obj, ITEM_RELIC ) )
    {
	act("{5$S weapon won't budge!{x",ch,NULL,victim,TO_CHAR);
	act("{5$n tries to disarm you, but your weapon won't budge!{x",
	    ch,NULL,victim,TO_VICT);
	act("{5$n tries to disarm $N, but fails.{x",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    act( "{5$n DISARMS you and sends your weapon flying!{x", 
	 ch, NULL, victim, TO_VICT    );
    act( "{5You disarm $N!{x",  ch, NULL, victim, TO_CHAR    );
    act( "{5$n disarms $N!{x",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) ||
	IS_SET(victim->in_room->room_flags, ROOM_ARENA) )
	obj_to_char( obj, victim );
    else
    {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    get_obj(victim,obj,NULL);
    }

    return;
}

void do_berserk( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if ((chance = get_skill(ch,skill_lookup("berserk"))) == 0
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    ||  (!IS_NPC(ch)
    &&   !can_use_skpell(ch, skill_lookup("berserk"))))
    {
	send_to_char("You turn red in the face, but nothing happens.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,skill_lookup("berserk"))
    ||  is_affected(ch,skill_lookup("frenzy")))
    {
	send_to_char("You get a little madder.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("You're feeling to mellow to berserk.\n\r",ch);
	return;
    }

    if (ch->mana < 50)
    {
	send_to_char("You can't get up enough energy.\n\r",ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move /= 2;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);

        sound( "berserk1.wav", ch );
	send_to_char("Your pulse races as you are consumed by rage!\n\r",ch);
	act("$n gets a wild look in $s eyes.",ch,NULL,NULL,TO_ROOM);
	check_improve(ch,skill_lookup("berserk"),TRUE,2);

	af.where	= TO_AFFECTS;
	af.type		= skill_lookup("berserk");
	af.level	= ch->level;
	af.duration	= number_fuzzy(ch->level / 8);
	af.modifier	= UMAX(1,ch->level/5);
	af.bitvector 	= AFF_BERSERK;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	= UMAX(10,10 * (ch->level/5));
	af.location	= APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move *= .80;

	send_to_char("Your pulse speeds up, but nothing happens.\n\r",ch);
	check_improve(ch,skill_lookup("berserk"),FALSE,2);
    }
}

void do_bash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);
 
    if ( (chance = get_skill(ch,skill_lookup("bash"))) == 0
    ||	 (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||	 (!IS_NPC(ch)
    &&	  !can_use_skpell(ch, skill_lookup("bash"))))
    {	
	send_to_char("Bashing? What's that?\n\r",ch);
	return;
    }
 
    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }
    if ( MOUNTED(ch) )
    {
        send_to_char("You can't bash while riding!\n\r", ch);
        return;
    }
 
    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("You try to bash your brains out, but fail.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;


    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 15;
    else
	chance += (ch->size - victim->size) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /25;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 30;

    /* level */
    chance += (ch->level - victim->level);

    if (!IS_NPC(victim) 
	&& chance < get_skill(victim,skill_lookup("dodge")) )
    {	/*
        act("{5$n tries to bash you, but you dodge it.{x",ch,NULL,victim,TO_VICT);
        act("{5$N dodges your bash, you fall flat on your face.{x",ch,NULL,victim,TO_CHAR);
        WAIT_STATE(ch,skill_table[skill_lookup("bash")].beats);
        return;*/
	chance -= 3 * (get_skill(victim,skill_lookup("dodge")) - chance);
    }

    /* now the attack */
    if (number_percent() < chance )
    {
    
	act("{5$n sends you sprawling with a powerful bash!{x",
		ch,NULL,victim,TO_VICT);
	act("{5You slam into $N, and send $M flying!{x",ch,NULL,victim,TO_CHAR);
	act("{5$n sends $N sprawling with a powerful bash.{x",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,skill_lookup("bash"),TRUE,1);

	DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[skill_lookup("bash")].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2,4 + 4 * ch->size + chance/20),skill_lookup("bash"),
	    DAM_BASH,FALSE);

        if (RIDDEN(victim))
    	{
            mount_success(RIDDEN(victim), victim, FALSE);
        }	
	
    }
    else
    {
	damage(ch,victim,0,skill_lookup("bash"),DAM_BASH,FALSE);
	act("{5You fall flat on your face!{x",
	    ch,NULL,victim,TO_CHAR);
	act("{5$n falls flat on $s face.{x",
	    ch,NULL,victim,TO_NOTVICT);
	act("{5You evade $n's bash, causing $m to fall flat on $s face.{x",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,skill_lookup("bash"),FALSE,1);
	ch->position = POS_RESTING;
	WAIT_STATE(ch,skill_table[skill_lookup("bash")].beats * 3/2); 
    }
	check_killer(ch,victim);
}

void do_dirt( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,skill_lookup("dirt"))) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    !can_use_skpell(ch, skill_lookup("dirt"))))
    {
	send_to_char("You get your feet dirty.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't dirt while riding!\n\r", ch);
        return;
    }


    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;


    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance -= 20;	break;
	case(SECT_CITY):		chance -= 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_DESERT):		chance += 10;   break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick.\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("{5$n is blinded by the dirt in $s eyes!{x",victim,NULL,NULL,TO_ROOM);
	act("{5$n kicks dirt in your eyes!{x",ch,NULL,victim,TO_VICT);
        damage(ch,victim,number_range(2,5),skill_lookup("dirt"),DAM_NONE,FALSE);
	send_to_char("{5You can't see a thing!{x\n\r",victim);
	check_improve(ch,skill_lookup("dirt"),TRUE,2);
	WAIT_STATE(ch,skill_table[skill_lookup("dirt")].beats);

	af.where	= TO_AFFECTS;
	af.type 	= skill_lookup("dirt");
	af.level 	= ch->level;
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;

	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,skill_lookup("dirt"),DAM_NONE,TRUE);
	check_improve(ch,skill_lookup("dirt"),FALSE,2);
	WAIT_STATE(ch,skill_table[skill_lookup("dirt")].beats);
    }
	check_killer(ch,victim);
}

void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,skill_lookup("trip"))) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
	  && !can_use_skpell(ch, skill_lookup("trip"))))
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't trip while riding!\n\r", ch);
        return;
    }


    if (is_safe(ch,victim))
	return;

    
    if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("{5You fall flat on your face!{x\n\r",ch);
	WAIT_STATE(ch,2 * skill_table[skill_lookup("trip")].beats);
	act("{5$n trips over $s own feet!{x",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /* now the attack */
    if (number_percent() < chance)
    {
	act("{5$n trips you and you go down!{x",ch,NULL,victim,TO_VICT);
	act("{5You trip $N and $N goes down!{x",ch,NULL,victim,TO_CHAR);
	act("{5$n trips $N, sending $M to the ground.{x",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,skill_lookup("trip"),TRUE,1);

	DAZE_STATE(victim,2 * PULSE_VIOLENCE);
        WAIT_STATE(ch,skill_table[skill_lookup("trip")].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2, 2 +  2 * victim->size),skill_lookup("trip"),
	    DAM_BASH,TRUE);
    }
    else
    {
	damage(ch,victim,0,skill_lookup("trip"),DAM_BASH,TRUE);
	WAIT_STATE(ch,skill_table[skill_lookup("trip")].beats*2/3);
	check_improve(ch,skill_lookup("trip"),FALSE,1);
    } 
	check_killer(ch,victim);
}



void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;


    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    if ( IS_NPC(victim))
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    if(IS_WEREWOLF(ch) )
	victim->turned = TRUE;
    if(IS_WEREWOLF(victim) )
	ch->turned = TRUE;
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Murder whom?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
	return;

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;


    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    if (IS_NPC(ch))
	sprintf(buf, "Help! I am being attacked by %s!",ch->short_descr);
    else
    	sprintf( buf, "Help!  I am being attacked by %s!", ch->name );
    do_function(victim, &do_yell, buf );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if (arg[0] == '\0')
    {
        send_to_char("Backstab whom?\n\r",ch);
        return;
    }

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't backstab while riding!\n\r", ch);
        return;
    }
 

    if (ch->fighting != NULL)
    {
	send_to_char("You're facing the wrong end.\n\r",ch);
	return;
    }
 
    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;


    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit / 3)
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[skill_lookup("backstab")].beats );
    if ( number_percent( ) < get_skill(ch,skill_lookup("backstab"))
    || ( get_skill(ch,skill_lookup("backstab")) >= 2 && !IS_AWAKE(victim) ) )
    {
	check_improve(ch,skill_lookup("backstab"),TRUE,1);
	multi_hit( ch, victim, skill_lookup("backstab") );
        sound( "backstab1.wav", ch );
        sound( "backstab1.wav", victim );
    }
    else
    {
	check_improve(ch,skill_lookup("backstab"),FALSE,1);
	damage( ch, victim, 0, skill_lookup("backstab"),DAM_NONE,TRUE);
    }

    return;
}



void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_ARENA))
	return;

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	||   number_range(0,ch->daze) != 0
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	move_char( ch, door, FALSE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n has fled!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch) )
	{
	    send_to_char( "You flee from combat!\n\r", ch );
            sound( "flee.wav", ch );
  	    if( is_class(ch, 2) 
	    && (number_percent() < 3*(ch->level/2) ) )
		send_to_char( "You snuck away safely.\n\r", ch);
	}

	stop_fighting( ch, TRUE );
	return;
    }

    send_to_char( "PANIC! You couldn't escape!\n\r", ch );
    return;
}



void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) && !is_same_group(ch,victim))
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }


    WAIT_STATE( ch, skill_table[skill_lookup("rescue")].beats );
    if ( number_percent( ) > get_skill(ch,skill_lookup("rescue")))
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	check_improve(ch,skill_lookup("rescue"),FALSE,1);
	return;
    }

    act( "{5You rescue $N!{x",  ch, NULL, victim, TO_CHAR    );
    act( "{5$n rescues you!{x", ch, NULL, victim, TO_VICT    );
    act( "{5$n rescues $N!{x",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,skill_lookup("rescue"),TRUE,1);

    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    check_killer( ch, fch );
    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int number = 0;

    number = number_range( 0, 5 );

    if ( !IS_NPC(ch)
    &&   !can_use_skpell(ch, skill_lookup("kick")) )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
	return;

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't kick while riding!\n\r", ch);
        return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[skill_lookup("kick")].beats );
    if ( get_skill(ch,skill_lookup("kick")) > number_percent())
    {
	damage(ch,victim,number_range( 50, ch->level + 100 / 2 ), skill_lookup("kick"),DAM_BASH,TRUE);
	check_improve(ch,skill_lookup("kick"),TRUE,1);
        if ( number <= 1 )
        {
            printf_to_char( ch, "{YYou make a series of lightning fast kicks right to %ss head!{x\n\r", victim->short_descr );
            damage(ch,victim,number_range( 150, ch->level + 200 / 2 ), skill_lookup("kick"), DAM_BASH, TRUE );
            damage(ch,victim,number_range( 150, ch->level + 200 / 2 ), skill_lookup("kick"), DAM_BASH, TRUE );
            check_improve(ch,skill_lookup("kick"),TRUE,1);
        }
    }
    else
    {
	damage( ch, victim, 0, skill_lookup("kick"),DAM_BASH,TRUE);
	check_improve(ch,skill_lookup("kick"),FALSE,1);
    }
	
    check_killer(ch,victim);
    return;
}


void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;

    hth = 0;

    if ((chance = get_skill(ch,skill_lookup("disarm"))) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL 
    &&   ((hth = get_skill(ch,skill_lookup("hand to hand"))) == 0
    ||    (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM))))
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
	return;
    }

    if (IS_AFFECTED(victim, AFF_VICEGRIP))
    {
       send_to_char ("Your opponent has an iron grip on their weapon! You have no chance!\n\r", ch);
       return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));

    /* modifiers */

    /* skill */
    if ( get_eq_char(ch,WEAR_WIELD) == NULL)
	chance = chance * hth/150;
    else
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;
 
    /* and now the attack */
    if (number_percent() < chance)
    {
    	WAIT_STATE( ch, skill_table[skill_lookup("disarm")].beats );
	disarm( ch, victim );
	check_improve(ch,skill_lookup("disarm"),TRUE,1);
    }
    else
    {
	WAIT_STATE(ch,skill_table[skill_lookup("disarm")].beats);
	act("{5You fail to disarm $N.{x",ch,NULL,victim,TO_CHAR);
	act("{5$n tries to disarm you, but fails.{x",ch,NULL,victim,TO_VICT);
	act("{5$n tries to disarm $N, but fails.{x",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,skill_lookup("disarm"),FALSE,1);
    }
    check_killer(ch,victim);
    return;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= get_trust(ch) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "{1You slay $M in cold blood!{x",  ch, NULL, victim, TO_CHAR    );
    act( "{1$n slays you in cold blood!{x", ch, NULL, victim, TO_VICT    );
    act( "{1$n slays $N in cold blood!{x",  ch, NULL, victim, TO_NOTVICT );
    raw_kill( victim, ch );
    return;
}

void do_engage(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

    /* Check for skill.  */
    if (   (get_skill(ch,skill_lookup("engage")) == 0 )
      || (    !IS_NPC(ch)
          &&  !can_use_skpell( ch, skill_lookup("engage") ) ) )
    {
      send_to_char("Engage?  You're not even dating!\n\r",ch);  /* Humor. :)  */
      return;
    }

    /* Must be fighting.  */
    if (ch->fighting == NULL)
    {
      send_to_char("You're not fighting anyone.\n\r",ch);
      return;
    }

    one_argument( argument, arg );

    /* Check for argument.  */
    if (arg[0] == '\0')
    {
      send_to_char("Engage who?\n\r",ch);
      return;
    }

    /* Check for victim.  */
    if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
      send_to_char("Shadowbox some other time.\n\r",ch);
      return;
    }

    if (victim == ch)
    {
      send_to_char("Attacking yourself in combat isn't a smart thing.\n\r",ch);
      return;
    }

    if (ch->fighting == victim)
    {
      send_to_char("You're already pummelling that target as hard as you can!\n\r",ch);
      return;
    }

    /* Check for safe.  */
    if (is_safe(ch, victim))
    return;

    /* Check for charm.  */
    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
      act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
      return;
    }

    /* This lets higher-level characters engage someone that isn't already fighting them.
     Quite powerful.  Raise level as needed.  */
    if ((victim->fighting != ch) && (ch->level < 2 ) )
    {
      send_to_char("But they're not fighting you!\n\r",ch);
      return;
    }

    /* Get chance of success, and allow max 95%.  */
    chance = get_skill(ch,skill_lookup("engage"));
    chance = UMIN(chance,95);

    if (number_percent() < chance)
    {
      /* It worked!  */
      stop_fighting(ch,FALSE);

      set_fighting(ch,victim);
      if (victim->fighting == NULL)
        set_fighting(victim,ch);

      check_improve(ch,skill_lookup("engage"),TRUE,3);
      act("$n has turned $s attacks toward you!",ch,NULL,victim,TO_VICT);
      act("You turn your attacks toward $N.",ch,NULL,victim,TO_CHAR);
      act("$n has turned $s attacks toward $N!",ch,NULL,victim,TO_NOTVICT);
    }
    else
    {
      /* It failed!  */
      send_to_char("You couldn't get your attack in.\n\r",ch);
      check_improve(ch,skill_lookup("engage"),FALSE,3);
    }
}

bool check_airmail_dir(CHAR_DATA* ch, ROOM_INDEX_DATA* to_room, EXIT_DATA* pexit)
{
    return !(IS_SET(pexit->exit_info, EX_CLOSED) && 
	   (!IS_AFFECTED(ch, AFF_PASS_DOOR) || 
    IS_SET(pexit->exit_info,EX_NOPASS))) ||
	   /* unenterable room */
    (!is_room_owner(ch,to_room) && room_is_private(to_room)) ||
	   /* elemental difficulties.. */
    ((to_room->sector_type == SECT_AIR ||
    to_room->sector_type == SECT_WATER_NOSWIM)
           && !IS_AFFECTED(ch,AFF_FLYING));
}

void airmail_damage(CHAR_DATA* ch, CHAR_DATA* victim, int num)
{
    int amount = number_range(5 * victim->size, 5 * victim->size * (num + 1));

    WAIT_STATE(ch, PULSE_VIOLENCE);
    damage(ch,victim, amount, 0, DAM_BASH, FALSE);
    check_killer(ch, victim);
}

void airmail_char(CHAR_DATA *ch, CHAR_DATA *victim, int door, CHAR_DATA* lst[], int* spectator_count, int num)
{
   ROOM_INDEX_DATA *in_room;
   ROOM_INDEX_DATA *to_room;
   EXIT_DATA *pexit;
   CHAR_DATA* target;
   int s;

   in_room = victim->in_room;

   if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL )
   {
      send_to_char( "Try another direction.\n\r", ch );
      return;
   }    

   /* End of the line */
   if(num <= 0)
   {
	   /* It didn't go far..  */
      if(in_room == ch->in_room)
      {
         act("With a mighty heave you throw $M $T.", ch, victim, dir_name[door], TO_CHAR);
	 act("With a mighty heave $n throws $N into the air!", ch, NULL, victim, TO_ROOM);
      }

      act("$n lands in an unceremonious heap.", victim, NULL, NULL, TO_ROOM);
      act("You hear a confused racket as $N completes $S erstwhile journey.", ch, NULL, victim, TO_CHAR);
      send_to_char("Your journey is suddenly arrested by the floor.\n\r", victim);

	  /* Keep the spectators informed.. */
      for(s = 0; s < *spectator_count; s++)
      {
	  act("You hear a confused racket as $N completes $S erstwhile journey.", lst[s], NULL, victim, TO_CHAR);
      }

      airmail_damage(ch, victim, num);
   }
   else if(pexit == NULL || to_room == NULL)
   {

	if(in_room == ch->in_room)
        {
	   act("With a mighty heave you throw $M $T.", ch, victim, dir_name[door], TO_CHAR);
	   act("With a mighty heave $n throws $N into the air!", ch, NULL, victim, TO_ROOM);
	}

        act("$n crashes into the wall and lands in a disgruntled heap.", victim, NULL, NULL, TO_ROOM);
        send_to_char("*CRUNCH* is the sound of you bouncing.\n\r", victim);

        if(in_room == ch->in_room)
        {
	   act("*CRUNCH* is the sound of $N bouncing.", ch, NULL, victim, TO_CHAR);
	}
        else
        {
	   act("You hear a confused racket as $N completes $S erstwhile journey.", ch, NULL, victim, TO_CHAR);

	   for(s = 0; s < *spectator_count; s++)
	   {
	       act("You hear a confused racket as $N completes $S erstwhile journey.", lst[s], NULL, victim, TO_CHAR);
           }
	}

        airmail_damage(ch, victim, num);
   }
   else if(!check_airmail_dir(ch, to_room, pexit))
   {
	act("$n gets a physics lesson from the $d and lands in a disgruntled heap.", victim, NULL, pexit->keyword, TO_ROOM);
        act("OWCH! is the sound of you missing the $d.", victim, NULL, pexit->keyword, TO_CHAR);

        if(in_room == ch->in_room)
        {
	   act("Ooooh!  With a neato crunch $N bounced!", ch, NULL, victim, TO_CHAR);
	}
	else
	{
	   act("You hear a confused racket as $N completes $S erstwhile journey.", ch, NULL, victim, TO_CHAR);

 	   for(s = 0; s < *spectator_count; s++)
	   {
	       act("You hear a confused racket as $N completes $S erstwhile journey.", lst[s], NULL, victim, TO_CHAR);
	   }
	}

        airmail_damage(ch, victim, num);
   }
   else 
   {
       if(ch->fighting != NULL)
       {
          stop_fighting(ch, TRUE);
          if(ch->position == POS_FIGHTING)
          {
            ch->position = POS_STANDING;
          }
       }

       if(in_room == ch->in_room)
       {  
	  act("With a mighty heave you throw $M $T.", ch, victim, dir_name[door], TO_CHAR);
	  act("With a mighty heave $n throws $N into the air!", ch, NULL, victim, TO_ROOM);

	  target = in_room->people;

	  while(target && *spectator_count < MAX_SPECTATORS)
	  {
	        if(target != ch && target != victim)
	        {
	   	   lst[(*spectator_count)++] = target;
   	        }

	        target = target->next_in_room;
          }
       }
       else
       {
	   act("You hear swooshing noises to the $T.", ch, victim, dir_name[door], TO_CHAR);
       }

       act("$n cuts a perfect arc as $E sails $T.", victim, NULL, dir_name[door], TO_ROOM);
       char_from_room(victim);
       char_to_room(victim, to_room);

       act("In a swoosh of air $n sails in looking quite confused.", victim, NULL, NULL, TO_ROOM);
       do_look(victim, "auto");

       target = to_room->people;

       while(target)
       {
 	   if(target == victim)
 	   {
	      target = target->next_in_room;
	      continue;
	   }

	   if((target->size + victim->size) * 10 > number_range(0, 200))
	   {
   	       act("The $n smashes headlong into $N!", victim, NULL, target, TO_ROOM);
               act("You smash into $N!", victim, NULL, target, TO_CHAR);
               act("$n crashes into you!", victim, NULL, target, TO_VICT);
	       act("There is a confused shriek as the $N crashes into someone.", ch, NULL, victim, TO_CHAR);

	       for(s = 0; s < *spectator_count; s++)
	       {
		   act("There is a confused shriek as the $N crashes into someone.", lst[s], NULL, victim, TO_CHAR);
	       }

	       airmail_damage(victim, target, num);
	       airmail_damage(target, victim, num);
	       break;
	   }
		  
	   target = target->next_in_room;
	  
          }

	  if(!target)
	  {
  		  /* Add spectators in this room */
		  target = to_room->people;
		  while(target && *spectator_count < MAX_SPECTATORS)
		  {
			  if(target != ch && target != victim)
			  {
				lst[(*spectator_count)++] = target;
			  }
			  target = target->next_in_room;
		  }

		  /* recurse to keep going */
	  airmail_char(ch, victim, door, lst, spectator_count, num - 1);
	  } 
    }

    return;
}

void do_throw(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim = NULL;
    char* tmp;
    int door;
    int force = 3;
    CHAR_DATA* lst[MAX_SPECTATORS];
    int spectator_count = 0;
    int movem = (ch->level +5) / 2; 

	/* Get a victim */
    tmp = one_argument(argument,arg);
    victim = get_char_room(ch, NULL, arg);

    if (victim == NULL)
    {
	send_to_char("Who do you want to throw?\n\r",ch);
	return;
    }

    if (victim->level >= LEVEL_IMMORTAL )
    {
        send_to_char( "Are you stupid?\n\r", ch );
        return;
    }

    if ( ch->fighting != '\0' || victim->fighting != '\0' )
    {
        send_to_char( "Try doing it when neither one of you are fighting.\n\r", ch );
        return;
    }

	/* Figure out how far we can throw this sucker */
	/* Heavily weighted for physical size */
    force = URANGE(0,
    (get_curr_stat(ch, STAT_STR) / 5) - ((victim->size - ch->size) * 2), 5);

	/* Heh, not very far at all.. */
    if(force <= 0)
    {
		/* Oops, ya done pissed 'em off */
       send_to_char("You grunt and heave but they refuse to budge!\r\n", ch);
       damage(ch,victim,0,TYPE_UNDEFINED,DAM_BASH,FALSE);
       return;
    }

    if ( ch->move < movem )
    { 
        printf_to_char( ch, "You need at least %d movement to throw anything!\n\r", movem );
        return;
    }

	/* Get a direction or pick one.. */
    one_argument(tmp, arg);

    if(arg[0] == '\0') 
    door = number_door();
    if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;	
	else door = number_door();

    airmail_char(ch, victim, door, lst, &spectator_count, force);
    ch->move -= movem;
}

void do_whirlwind( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *pChar;
   CHAR_DATA *pChar_next;
   OBJ_DATA *wield;
   bool found = FALSE;
   
   if (    !IS_NPC( ch ) 
        && !can_use_skpell( ch, skill_lookup("whirlwind") ) )
   {
      send_to_char( "You don't know how to do that...\n\r", ch );
      return;
   }
 
   if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
   {
      send_to_char( "You need to wield a weapon first...\n\r", ch );
      return;
   }
   
   act( "$n holds $p firmly, and starts spinning round...", ch, wield, NULL, TO_ROOM );
   act( "You hold $p firmly, and start spinning round...",  ch, wield, NULL, TO_CHAR );
   
   pChar_next = NULL;   
   for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
   {
      pChar_next = pChar->next_in_room;
      if ( IS_NPC( pChar ) )
      {
         found = TRUE;
         act( "$n turns towards YOU!", ch, NULL, pChar, TO_VICT    );
         multi_hit( ch, pChar, skill_lookup("whirlwind") );
      }
   }
   
   if ( !found )
   {
      act( "$n looks dizzy, and a tiny bit embarassed.", ch, NULL, NULL, TO_ROOM );
      act( "You feel dizzy, and a tiny bit embarassed.", ch, NULL, NULL, TO_CHAR );
   }
   
   WAIT_STATE( ch, skill_table[skill_lookup("whirlwind")].beats );
   
   if ( !found && number_percent() < 20 )
   {
      act( "$n loses $s balance and falls into a heap.",  ch, NULL, NULL, TO_ROOM );
      act( "You lose your balance and fall into a heap.", ch, NULL, NULL, TO_CHAR );
      ch->position = POS_STUNNED;
   }
   
   return;
}      

void do_circle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
 
    one_argument( argument, arg );
 
    if ( !IS_NPC( ch )
        && !can_use_skpell( ch, skill_lookup("circle") ) )
    {
        send_to_char( "You don't know how to do that.\n\r", ch );
        return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char("But you aren't fighting anyone!\n\r",ch);
            return;
        }
    }        
            
    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }
 
    if (victim == ch)
    {
        send_to_char("You try and circle yourself but you just fall down.\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim ) )
      return;
 
    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a weapon to circle.\n\r", ch );
        return;
    }
 
    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[skill_lookup("circle")].beats );
    if ( number_percent( ) < get_skill(ch,skill_lookup("circle"))
    || ( get_skill(ch,skill_lookup("circle")) >= 2 && !IS_AWAKE(victim) ) )
    {
        check_improve(ch,skill_lookup("circle"),TRUE,1);
        multi_hit( ch, victim, skill_lookup("circle") );
    }
    else
    {
        check_improve(ch,skill_lookup("circle"),FALSE,1);
        damage( ch, victim, 0, skill_lookup("circle"),DAM_NONE,TRUE);
    }
 
    return;
}
 
void do_surrender( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    if ( (mob = ch->fighting) == NULL )
    {
	send_to_char( "But you're not fighting!\n\r", ch );
	return;
    }
    act( "You surrender to $N!", ch, NULL, mob, TO_CHAR );
    act( "$n surrenders to you!", ch, NULL, mob, TO_VICT );
    act( "$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT );
    stop_fighting( ch, TRUE );
 
    if ( !IS_NPC( ch ) && IS_NPC( mob ) 
    &&   ( !HAS_TRIGGER_MOB( mob, TRIG_SURR ) 
        || !p_percent_trigger( mob, NULL, NULL, ch, NULL, NULL, TRIG_SURR ) ) )
    {
	act( "$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR );
	multi_hit( mob, ch, TYPE_UNDEFINED );
    }
}
 

bool check_critical(CHAR_DATA *ch, CHAR_DATA *victim)
{
        OBJ_DATA *obj;

        obj = get_eq_char(ch,WEAR_WIELD);

        if (
             ( get_eq_char(ch,WEAR_WIELD) == NULL ) || 
             ( get_skill(ch,skill_lookup("critical strike"))  <  1 ) ||
             ( get_weapon_skill(ch,get_weapon_sn(ch))  !=  100 ) ||
             ( number_range(0,100) > get_skill(ch,skill_lookup("critical strike")) )
           )
                return FALSE;

        if ( number_range(0,100) > 5 )
                return FALSE;

        act("$p CRITICALLY STRIKES $n!",victim,obj,NULL,TO_NOTVICT);
        act("CRITICAL STRIKE!",ch,NULL,victim,TO_VICT);
        check_improve(ch,skill_lookup("critical strike"),TRUE,6);
        return TRUE;
}


bool check_counter( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)
{
        int chance;
        int dam_type;
        OBJ_DATA *wield;

        if (    ( get_eq_char(victim, WEAR_WIELD) == NULL ) ||
                ( !IS_AWAKE(victim) ) ||
                ( !can_see(victim,ch) ) ||
                ( get_skill(victim,skill_lookup("counter")) < 1 )
           )
           return FALSE;

        wield = get_eq_char(victim,WEAR_WIELD);

        chance = get_skill(victim,skill_lookup("counter")) / 8; /* changed to 8 and 4 respectively */
        chance += ( victim->level - ch->level ) / 4;
        chance += 2 * (get_curr_stat(victim,STAT_DEX) - get_curr_stat(ch,STAT_DEX));
        chance += get_weapon_skill(victim,get_weapon_sn(victim)) -
                        get_weapon_skill(ch,get_weapon_sn(ch));
        chance += (get_curr_stat(victim,STAT_STR) - get_curr_stat(ch,STAT_STR) );

    if ( number_percent( ) >= chance )
        return FALSE;

        dt = skill_lookup("counter");

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else 
            dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    act( "You reverse $n's attack and counter with your own!", ch, NULL, victim, TO_VICT    );
    act( "$N reverses your attack!", ch, NULL, victim, TO_CHAR    );

    damage(victim,ch,dam/2, skill_lookup("counter") , dam_type ,TRUE ); /* DAM MSG NUMBER!! */

    check_improve(victim,skill_lookup("counter"),TRUE,6);

    return TRUE;
}

void do_warcry(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;

  if (IS_NPC(ch) ||
      !can_use_skpell( ch, skill_lookup("warcry") ) )
  {
      send_to_char( "You don't know how to warcry.\n\r", ch );
      return;
  }
           
  if (is_affected(ch, skill_lookup("warcry")))
  {
      send_to_char("You are already affected by warcry.\n\r",ch);
      return;
  }   

  if (ch->mana < 30)
  {
      send_to_char("You can't concentrate enough right now.\n\r",ch);
      return;
  }

  WAIT_STATE( ch, skill_table[skill_lookup("warcry")].beats );

  if (number_percent() > ch->pcdata->learned[skill_lookup("warcry")])
  {
      send_to_char("You grunt softly.\n\r", ch);
      act("$n makes some soft grunting noises.", ch, NULL, NULL, TO_ROOM);
      return;
  }     

  ch->mana -= 30;

  af.where      = TO_AFFECTS;
  af.type      = skill_lookup("warcry");
  af.level       = ch->level;
  af.duration  = 6+ch->level;
  af.location  = APPLY_HITROLL;
  af.modifier  = ch->level / 8;
  af.bitvector = 0;    
  affect_to_char( ch, &af );
  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = 0 - ch->level / 8;
  affect_to_char( ch, &af );
  send_to_char( "You feel righteous as you yell out your warcry.\n\r", ch );
  return;
}     

void do_assassinate( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( !IS_NPC( ch )
        && !can_use_skpell( ch, skill_lookup("assassinate") ) )
    {
        send_to_char( "You don't know to assassinate.\n\r", ch );
        return;
    }              

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char("But you aren't fighting anyone!\n\r",ch);
            return;
        }
    }           
 
    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }    
   
    if ( victim == ch )
    {
        send_to_char( "You can't assassinate yourself!\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim ) )
      return;     

 
    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a weapon to assassinate.\n\r", ch );
        return;
    }     

    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[skill_lookup("assassinate")].beats );  
 
    if ( number_percent( ) < get_skill(ch,skill_lookup("assassinate"))
    || ( get_skill(ch,skill_lookup("assassinate")) >= 2 && !IS_AWAKE(victim) ) )
    {
        check_improve(ch,skill_lookup("assassinate"),TRUE,1);
        multi_hit( ch, victim, skill_lookup("assassinate") );
    }                      
    else
    {
        check_improve(ch,skill_lookup("assassinate"),FALSE,1);
        damage( ch, victim, 0, skill_lookup("assassinate"),DAM_NONE,TRUE);
    }

    return;
}          


void crowd_brawl(CHAR_DATA *ch)
{
    CHAR_DATA *rch, *rch_next, *vch, *vch_next;
    CHAR_DATA *is_fighting[45];
    SHOP_DATA *pshop;
    int chance;
    int counter;
    int to_fight;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;

	chance = number_range(1,250);

	if ((rch->fighting == NULL)
	 &&((!IS_IMMORTAL(rch) && !IS_NPC(rch))
	 ||(IS_NPC(rch)
	 &&!(IS_SET(rch->act,ACT_TRAIN)
	 ||  IS_SET(rch->act,ACT_PRACTICE)
	 ||  IS_SET(rch->act,ACT_IS_HEALER)
	 ||  IS_SET(rch->act,ACT_IS_CHANGER)
	 || ((pshop = rch->pIndexData->pShop) != NULL))))
	 &&IS_AWAKE(rch)
	 &&(chance <= 5))
	{
	  counter = 0;
	  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	  {
	    vch_next = vch->next_in_room;

	    if ((vch->fighting != NULL) && (counter <= 44))
	    {
	      is_fighting[counter] = vch;
	      ++counter;

	      if ((IS_NPC(vch)) && (counter <= 44))
	      {
	        is_fighting[counter] = vch;
	        ++counter;
	      }
	      if ((IS_NPC(vch)) && (counter <= 44))
	      {
	        is_fighting[counter] = vch;
	        ++counter;
	      }
  	    }
	  }

	  to_fight = number_range(1,counter);
	  to_fight -= 1;

	  if (!IS_NPC(rch) && !IS_NPC(is_fighting[to_fight]))
	  {
	    if (is_same_clan(rch,is_fighting[to_fight]))
	      continue;

	    if (((UMAX(rch->level,is_fighting[to_fight]->level) -
		  UMIN(rch->level,is_fighting[to_fight]->level)) > 7)
	       ||(is_fighting[to_fight]->level < 10))
	      continue;
	  }
	  else if (IS_NPC(rch))
	  {
	    if (rch->pIndexData->vnum == 3708)
	      continue;
	  }

	  if (IS_IMMORTAL(is_fighting[to_fight]))
	    continue;

	  if ((rch->position <= POS_RESTING) && (chance > 1))
	    continue;

	  /* gotta see them to attack them */
	  if (!can_see(rch,is_fighting[to_fight]))
	    continue;

	  /* not against group members */
	  if (is_same_group(rch,is_fighting[to_fight]))
	    continue;

	  /* charmed mobs and pets, whether grouped or not */
	  if (IS_AFFECTED(rch,AFF_CHARM)
	   &&((rch->master == is_fighting[to_fight])
	   ||(is_fighting[to_fight]->master == rch)
	   ||(is_fighting[to_fight]->master == rch->master)))
	    continue;

	  if (IS_SET(rch->act,ACT_PET)
	   &&((rch->master == is_fighting[to_fight])
	   ||(is_fighting[to_fight]->master == rch)
	   ||(is_fighting[to_fight]->master == rch->master)))
	    continue;

	  if (is_fighting[to_fight] != NULL)
	  {
	    rch->fighting = is_fighting[to_fight];
	    rch->position = POS_FIGHTING;
	    act("You find yourself caught up in the brawl!",rch,NULL,NULL,TO_CHAR);
	    act("$n finds $mself involved in the brawl.",rch,NULL,NULL,TO_ROOM);
	  }
	  else
	    bug("Crowd_brawl - person to fight is NULL.", 0);
	}
    }

    return;
}  


bool check_steel_skin(CHAR_DATA *ch,CHAR_DATA *victim ) 
{ 

    if ( !IS_AFFECTED2(victim, AFF_STEEL_SKIN) )
        return FALSE;

  if (victim->subtype > 0)
  {
  xact_new( "{i$n's attack bounces off your steel skin.{x",ch,NULL,victim,TO_VICT, POS_RESTING, SUPPRESS_DODGE ); 
  xact_new( "{hYour attack bounces off $N's steel skin.{x",ch,NULL,victim,TO_CHAR, POS_RESTING, SUPPRESS_DODGE ); 
  victim->subtype--; 
  if(victim->subtype<=0) 
  { 
          send_to_char("{xYour skin feels soft once again.\n\r",victim); 
          affect_strip(victim,skill_lookup("steel skin")); 
  } 
      return TRUE; 
  } 
  
  return FALSE; 
} 

void do_bloodlust( CHAR_DATA *ch, char *argument)
{
  int chance, hp_percent;

  if ((chance = get_skill(ch,skill_lookup("bloodlust"))) == 0
  || (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BLOODLUST))
  || (!IS_NPC(ch)
  && !can_use_skpell( ch, skill_lookup("bloodlust") ) ) )
  {
     send_to_char("{hYou try to become consumed but nothing happens.{x\n\r",ch);
     return;
  }

  if (IS_AFFECTED(ch,AFF_CALM))
  {
      send_to_char("{hYou're feeling too mellow.{x\n\r",ch);
      return;
  }

  if (IS_AFFECTED2(ch,AFF_BLOODLUST))
  {
      send_to_char("{hYou are already consumed!{x\n\r",ch);
      return;
  }
 
  if (ch->position == POS_FIGHTING)
      chance += 10;

  hp_percent = 100 * ch->hit/ch->max_hit;
  chance += 25 - hp_percent/2;

  if (number_percent() < chance)
  {
      AFFECT_DATA af;

      WAIT_STATE(ch,PULSE_VIOLENCE);
      ch->hit *= .90;
      ch->move *= .90;

      send_to_char("You shed your blood and feel battle lust take over!{x\n\r",ch);
      act("{k$n sheds some blood and is consumed by battle lust!{x",ch,NULL,NULL,TO_ROOM);
      check_improve(ch,skill_lookup("bloodlust"),TRUE,2);

      af.where = TO_AFFECTS2;
      af.type = skill_lookup("bloodlust");
      af.level = ch->level;
      af.duration = number_fuzzy(ch->level / 7);
      af.modifier = ch->level / 2 ;
      af.bitvector = AFF_BLOODLUST;

      af.location = APPLY_HITROLL;
      affect_to_char(ch,&af);

      af.location = APPLY_DAMROLL;
      affect_to_char(ch,&af);

  }

  else
  {
      WAIT_STATE(ch,3 * PULSE_VIOLENCE);
      ch->hit *= .95;

      send_to_char("{hYou shed some blood but nothing happens.{x\n\r",ch);
      check_improve(ch,skill_lookup("bloodlust"),FALSE,2);
  }
}


void do_nature(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;

  if ( get_skill(ch,skill_lookup("nature")) == 0
  || (!IS_NPC(ch)
  && !can_use_skpell( ch, skill_lookup("nature") )))
  {  
     send_to_char("You scream at the top of your lungs!\n\r",ch);
     return;
  }

  if (IS_AFFECTED(ch,AFF_CALM))
  {
     send_to_char("{hYou're feeling to mellow.{x\n\r",ch);
     return;
  }

  if ( is_affected(ch, skill_lookup("nature")))
  {
      send_to_char("You are already as wild as possible.\n\r",ch);
      return;
  } 

  WAIT_STATE( ch, skill_table[skill_lookup("nature")].beats );

  if (number_percent() > ch->pcdata->learned[skill_lookup("nature")])
  {
     act("$n throws back $s head and howls loudly.", ch, NULL, NULL, TO_ROOM);
     return;
  }

  af.where = TO_AFFECTS;
  af.type = skill_lookup("nature");
  af.level = ch->level;
  af.duration = ch->level / 5;
  af.location = APPLY_AC;
  af.modifier = 0 - ch->level;
  af.bitvector = 0;
  affect_to_char( ch, &af ); 

  af.where = TO_AFFECTS;
  af.type = skill_lookup("nature");
  af.level = ch->level;
  af.duration = ch->level / 5;
  af.location = APPLY_HITROLL;
  af.modifier = ch->level / 3;
  af.bitvector = 0;
  affect_to_char( ch, &af ); 

  af.where = TO_AFFECTS;
  af.type = skill_lookup("nature");
  af.level = ch->level;
  af.duration = ch->level / 5;
  af.location = APPLY_DAMROLL;
  af.modifier = ch->level / 3;
  af.bitvector = 0;
  affect_to_char( ch, &af ); 
  send_to_char("You focus and feel the beast within come forth!\n\r",ch);
  return;
}

void do_knee( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  int dam;

  if ( !IS_NPC(ch)
      && !can_use_skpell( ch, skill_lookup("knee") ) )
  {
      send_to_char("You better leave the martial arts to fighters.\n\r", ch );
      return;
  }

  if ( ( victim = ch->fighting ) == NULL )
  {
      send_to_char( "You aren't fighting anyone.\n\r", ch );
      return;
  } 

  WAIT_STATE( ch, skill_table[skill_lookup("knee")].beats );
 
  if ( get_skill(ch,skill_lookup("knee")) > number_percent())
  {
      check_improve(ch,skill_lookup("knee"),TRUE,1);
      act( "{i$n grabs your head and slams their knee into your face.{x", ch, NULL, victim, TO_VICT);
      act( "{hYou grab $N and slam your knee into their face.{x", ch, NULL, victim, TO_CHAR);
      act( "{k$n grabs $N and slams $s knee into their face.{x", ch, NULL, victim, TO_NOTVICT);
      dam = number_range ( (ch->level * 7 ), ( 250 ));
      damage( ch, victim, dam, skill_lookup("knee"),DAM_BASH,TRUE);
      damage( ch, victim, dam, skill_lookup("knee"),DAM_BASH,TRUE);
      damage( ch, victim, dam, skill_lookup("knee"),DAM_BASH,TRUE);
  }

  else
  {
      check_improve(ch,skill_lookup("knee"),FALSE,1);
      act( "{i$n tries to knee you and misses.{x", ch, NULL, victim, TO_VICT);
      act( "{hYou try to knee $N and miss.{x", ch, NULL, victim, TO_CHAR);
      act( "{k$n tries to knee $N and misses.{x", ch, NULL, victim, TO_NOTVICT);
      damage( ch, victim, 0, skill_lookup("knee"),DAM_BASH,TRUE);
  }

  return;
}


void do_srush( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    OBJ_DATA *obj;
    int dam;
    int chance;
 
    one_argument( argument, arg );

    if ( get_skill(ch,skill_lookup("srush")) == 0
    ||	 (!IS_NPC(ch)
    &&	  !can_use_skpell( ch, skill_lookup("srush") ) ) )
    {	
	send_to_char("Shield Rush? What's that?\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char("But you aren't fighting anyone!\n\r",ch);
            return;
        }
    }          

    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }        

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }
    
    if (is_safe(ch,victim))
        return;     

    if ( (obj = get_eq_char( ch, WEAR_SHIELD ) )== NULL )
    {
        send_to_char("You need a shield for this manuever.\n\r",ch);
        return;
    }

    /* modifiers */

    /* size  and weight */
    chance = ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 15;
    else
	chance += (ch->size - victim->size) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /25;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 30;

    /* level */
    chance += (ch->level - victim->level);

    WAIT_STATE( ch, skill_table[skill_lookup("srush")].beats );
    if ( number_percent( ) < get_skill(ch,skill_lookup("srush"))
    || ( get_skill(ch,skill_lookup("srush")) >= 2 && !IS_AWAKE(victim) ) )
    {
        check_improve(ch,skill_lookup("srush"),TRUE,3);
	act( "{i$n knocks you off your feet with $s shield.{x", ch, NULL, victim, TO_VICT);
	act( "{hYou knock $N off there feet with your shield.{x", ch, NULL, victim, TO_CHAR);
	act( "{k$n knocks $N of $S feet with there shield.{x", ch, NULL, victim, TO_NOTVICT);
	dam = dice( 425 + 25, 25 );
        damage( ch, victim, dam, skill_lookup("srush"),DAM_BASH,TRUE);
        DAZE_STATE(victim,2 * PULSE_VIOLENCE);  
        victim->position = POS_RESTING;
    }
    else
    {
        check_improve(ch,skill_lookup("srush"),FALSE,3);
	act( "{i$n tries to knock you off your feet.{x", ch, NULL, victim, TO_VICT);
	act( "{hYou try to knock $N off $S feet.{x", ch, NULL, victim, TO_CHAR);
	act( "{k$n tries to knock $N off $S feet.{x", ch, NULL, victim, TO_NOTVICT);
        damage( ch, victim, 0, skill_lookup("srush"),DAM_BASH,TRUE);
    }
 
    return;
}


bool check_phase( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,skill_lookup("phase")) / 2;

    if (!can_see(victim,ch))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    xact_new( "Your body phases to avoid $n's attack.", ch, NULL, victim, TO_VICT, POS_RESTING, SUPPRESS_DODGE    );
    xact_new( "$N's body phases to avoid your attack.", ch, NULL, victim, TO_CHAR, POS_RESTING, SUPPRESS_DODGE    );
    check_improve(victim,skill_lookup("phase"),TRUE,6);
    return TRUE;
}


void do_spin( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance;
  
    one_argument( argument, arg );

    if ( get_skill(ch,skill_lookup("spin")) == 0
    ||	 (!IS_NPC(ch)
    &&	  !can_use_skpell( ch, skill_lookup("spin") ) ) )
    {	
	send_to_char("You can't do a spinning slash attack.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char("But you aren't fighting anyone!\n\r",ch);
            return;
        }
    }

    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }         
 
    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a primary weapon to do a spinning slash attack.\n\r", ch );
        return;
    }
  
    WAIT_STATE( ch, skill_table[skill_lookup("spin")].beats );
    if ( number_percent( ) < get_skill(ch,skill_lookup("spin"))
    || ( get_skill(ch,skill_lookup("spin")) >= 2 && !IS_AWAKE(victim) ) )
    {
        check_improve(ch,skill_lookup("spin"),TRUE,1);
	act( "{i$n hits you with a spinning slash attack!{x", ch, NULL, victim, TO_VICT);
	act( "{hYour spinning slash attack strikes $N resulting in lots of blood!{x", ch, NULL, victim, TO_CHAR);
	act( "{k$n strikes $N with a spinning slash attack.{x", ch, NULL, victim, TO_NOTVICT);
        multi_hit( ch, victim, skill_lookup("spin") );
        if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL)
        {
            return;
        }
        if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
        {
	    return;
        }
	chance = (get_skill(ch,skill_lookup("disarm"))/2);
	if (number_percent() <= chance )
	{
	    chance = (get_skill(ch,skill_lookup("disarm"))/2);
	    if (number_percent() <= chance )

	    act("{7Unable to block $n's spinning slash attack, your weapon tears free!{x",ch,NULL,victim,TO_VICT);
	    act("{7$N's weapon breaks free as you hit them with a spinning slash attack!{x",ch,NULL,victim,TO_CHAR);
	    check_improve(ch,skill_lookup("disarm"),TRUE,1);
            obj_from_char( obj );
            if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY ) )
        	obj_to_char( obj, victim );
            else
            {
	        obj_to_room( obj, victim->in_room );
	        if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	        get_obj(victim,obj,NULL);
            }
        }
    
    }
    else
    {
        check_improve(ch,skill_lookup("spin"),FALSE,1);
	act( "{i$n tries to hit you with a spinning slash attack.{x", ch, NULL, victim, TO_VICT);
	act( "{h$N dodges your spinning slash attack.{x", ch, NULL, victim, TO_CHAR);
	act( "{k$n does a spinnning slash attack on $N .{x", ch, NULL, victim, TO_NOTVICT);
        damage( ch, victim, 0, skill_lookup("spin"),DAM_NONE,TRUE);
    }
 
    return;
}

void do_rage( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *pChar;
   CHAR_DATA *pChar_next;
   OBJ_DATA *wield;
   int chance;

    chance = number_range( 1, 100);
   
    if ( get_skill(ch,skill_lookup("rage")) == 0
    ||	 (!IS_NPC(ch)
    &&	  !can_use_skpell( ch, skill_lookup("rage") ) ) )
   {
      send_to_char( "You don't know how to do that...\n\r", ch );
      return;
   }

   if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
   {
      send_to_char( "You need to wield a weapon first...\n\r", ch );
      return;
   }
   
   act( "In a rage $n attacks everyone in the room!", ch, wield, NULL, TO_ROOM );
   act( "In a rage you attack everything in the room!",  ch, wield, NULL, TO_CHAR );
   
    for ( pChar = char_list; pChar != NULL; pChar = pChar_next )
    {
        pChar_next        = pChar->next;

        if ( pChar->in_room == NULL )
            continue;

        if ( pChar->in_room == ch->in_room )
        {
            if (is_safe(ch,pChar))
                return;    

            if ( pChar != ch && !is_safe_spell(ch,pChar,TRUE))
            {
                if (is_same_group(pChar,ch) )
                {
                    continue;
                }
         
        WAIT_STATE( ch, skill_table[skill_lookup("rage")].beats );
        act( "$n turns towards YOU!", ch, NULL, pChar, TO_VICT    );
	check_improve(ch,skill_lookup("rage"),TRUE,4);
        multi_hit( ch, pChar, skill_lookup("rage") );

        if (chance < 15)
        {
        act( "$n spins back toward YOU!", ch, NULL, pChar, TO_VICT    );
        act( "$n spins back to attack again!",  ch, wield, NULL, TO_ROOM );
        act( "You spin back to attack again!",  ch, wield, NULL, TO_CHAR );
        multi_hit( ch, pChar, skill_lookup("rage") );
            continue;
        }

      }
    }   
 
   }

   return;
} 

//Stances by Majik

void do_stance (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_STRING_LENGTH];
    char *stances = "neutral aggressive defensive";      

    argument = one_argument (argument, arg);

    if (IS_NPC (ch))
    {
        send_to_char ("Only players can change stance.\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        send_to_char ("Switch to which stance?\n\r", ch);
        printf_to_char( ch, "Valid stances are: %s\n\r", stances );
        return;
    }

    else
    {
        if (!str_prefix (arg, "neutral"))
        {   
            ch->stance = 0;
            ch->stance_hit_bonus = 0;
            ch->stance_ac_bonus = 0;
            send_to_char ("You are in a neutral stance.\n\r", ch);   
            return;
        }

        else if (!str_prefix (arg, "aggressive"))
        {
            ch->stance = 1;
            ch->stance_hit_bonus = ch->hitroll/2;
            ch->stance_ac_bonus = ch->armor[1]/4;
            send_to_char ("You are in an aggressive stance.\n\r", ch);   
            return;
        }

        else if (!str_prefix (arg, "defensive"))
        {
            ch->stance = 2;
            ch->stance_hit_bonus = (ch->hitroll/4) * -1;
            ch->stance_ac_bonus = (ch->armor[1]/2) * -1;
            send_to_char ("You are in an defensive stance.\n\r", ch);   
            return;
        }        

        else
        {
            send_to_char ("Valid stances are aggressive neutral and defensive.\n\r", ch);
            return;
        }
    }
}


/* Bowfire code -- actual firing function */
void do_fire( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim = NULL;
    OBJ_DATA *arrow;
    OBJ_DATA *bow;
    ROOM_INDEX_DATA *was_in_room;
    EXIT_DATA *pexit;
    int dam ,door ,chance;

    bow = get_eq_char(ch, WEAR_WIELD);
    if (bow == NULL)
    {
	send_to_char("{WWhat are you going to do, throw the arrow at them?{x\n\r", ch);
	return;
    }

    if (bow->value[0] != WEAPON_BOW)
    {
	send_to_char("{WYou might want to use a bow to fire that arrow with{x\n\r", ch);
	return;
    }    

    one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
        send_to_char( "{WFire an arrow at who?{x\n\r", ch );
        return;
    }

    if (!str_cmp(arg, "none") || !str_cmp(arg, "self") || victim == ch)
    {
        send_to_char("{WHow exactly did you plan on firing an arrow at yourself?{x\n\r", ch );
        return;
    }

    if ( ( arrow = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
        send_to_char( "{WYou hold nothing in your hand.{x\n\r", ch );
        return;
    }

    if ( arrow->item_type != ITEM_ARROW )
    {
        send_to_char( "{WYou can only a fire arrows or quarrels.{x\n\r", ch );
        return;
    }
	
    if ( arg[0] == '\0' )
    {
        if ( ch->fighting != NULL )
        {
            victim = ch->fighting;
        }
        else
        {
            send_to_char( "{WFire at whom or what?{x\n\r", ch );
            return;
        }
    }
    else
    {
        
	/* See if who you are trying to shoot at is nearby... */

        if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL)
        {
            was_in_room = ch->in_room;


            for( door=0 ; door<=5 && victim==NULL ; door++ )
             { 
                if ( (  pexit = was_in_room->exit[door] ) != NULL
                   &&   pexit->u1.to_room != NULL
                   &&   pexit->u1.to_room != was_in_room 
                   &&   !strcmp( pexit->u1.to_room->area->name ,
                                was_in_room->area->name ) )
                   { 
                     ch->in_room = pexit->u1.to_room;
                     victim = get_char_room ( ch, NULL, arg ); 
                    }
               
              }

            ch->in_room = was_in_room;

            if(victim==NULL)
              {
               send_to_char( "{WYou can't find it.{x\n\r", ch );
               return;
              }
            else
              {  if(IS_SET(pexit->exit_info,EX_CLOSED))
                    { send_to_char("{WYou can't fire through a door.{x",ch);
                      return;
                     } 
             }
        }
    }

    if((ch->in_room) == (victim->in_room))
    {
       send_to_char("{WDon't you think that standing a bit further away would be wise?{x\n\r", ch);
       return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE ); 

    if(arrow->item_type== ITEM_ARROW )
    {
        if ( victim != NULL )
        {
            act( "{W$n {Wfires $p {Wat $N{W.{x", ch,  arrow, victim, TO_NOTVICT );
            act( "{WYou fire $p {Wat $N{W.{x", ch,   arrow, victim, TO_CHAR );
            act( "{W$N {Wfires $p {Wat you.{x",ch,   arrow, victim, TO_VICT );
        }
      
        if (ch->level <   arrow->level
        ||  number_percent() >= 20 + get_skill(ch,skill_lookup("bow")) * 4/5 )
        {       
              act( "{WYou fire $p {Wmissing, and it lands harmlessly on the ground.{x",
                 ch,  arrow,NULL,TO_CHAR);
              act( "{W$r fires $p {Wmissing, and it lands harmlessly on the ground.{x",
                 ch,  arrow,NULL,TO_ROOM);
              obj_from_char(arrow);
			  obj_to_room(arrow, victim->in_room);
              check_improve(ch,skill_lookup("bow"),FALSE,2);
        }

        else
        {      

              chance=dice(1,10);

    switch (chance)
    {
        case 1 :
	case 2 :
	case 3 :
	case 4 :
	case 5 :
	case 6 :
                        obj_from_char(arrow);
			obj_to_char(arrow, victim);
			wear_obj(victim, arrow,TRUE);
                        dam      =  dice(arrow->value[1],arrow->value[2]);
			damage( ch, victim, dam, skill_lookup("bow"), DAM_PIERCE, TRUE );						
			check_improve(ch,skill_lookup("bow"),TRUE,2);
			break;
	case 7 :
	case 8 :
	case 9 :
            obj_from_char(arrow);
			obj_to_char(arrow, victim);
			wear_obj(victim, arrow,TRUE);
                        dam      = 3 * ( dice(arrow->value[1],arrow->value[2]))/2;
			damage( ch, victim, dam, skill_lookup("bow"), DAM_PIERCE, TRUE );						
			check_improve(ch,skill_lookup("bow"),TRUE,2);
			break;
	case 10 :
            obj_from_char(arrow);
			obj_to_char(arrow, victim);
			wear_obj(victim, arrow,TRUE);
                        dam      =  2 * ( dice(arrow->value[1],arrow->value[2]));
			damage( ch, victim, dam, skill_lookup("bow"), DAM_PIERCE, TRUE );						
			check_improve(ch,skill_lookup("bow"),TRUE,2);
			break;
	}		
   }

  } 

    return;
}


void do_gouge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,skill_lookup("gouge"))) == 0
    ||   (!IS_NPC(ch)
    &&    !can_use_skpell( ch, skill_lookup("gouge") ) ) )
    {
	send_to_char("You don't know how to gouge.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;


    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 25;

    chance += (ch->level - victim->level) * 2;

    if (chance % 5 == 0)
	chance += 1;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("$n is blinded by a poke in the eyes!{x",victim,NULL,NULL,TO_ROOM);
	act("$n gouges at your eyes!",ch,NULL,victim,TO_VICT);
        damage(ch,victim,number_range(50,75),skill_lookup("gouge"),DAM_NONE,FALSE);
	send_to_char("You see nothing but stars!\n\r",victim);
	check_improve(ch,skill_lookup("gouge"),TRUE,2);
	WAIT_STATE(ch,skill_table[skill_lookup("gouge")].beats);

	af.where	= TO_AFFECTS;
	af.type 	= skill_lookup("gouge");
	af.level 	= (ch->level + 10) /2;
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;
	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,skill_lookup("gouge"),DAM_NONE,TRUE);
	check_improve(ch,skill_lookup("gouge"),FALSE,2);
	WAIT_STATE(ch,skill_table[skill_lookup("gouge")].beats);
    }
}


void do_tithe( CHAR_DATA *ch, char *argument )
{
	int	cost;/* amount of gold needed */
	int	fillup;/* amount of mana needed */
	int     paid;/*Full Amount Player can pay*/
        int     pumped;/*Amount of mana to be given back*/
        int     manacost = 3;

	if ( !IS_NPC( ch )
             && !can_use_skpell( ch, skill_lookup("tithe") ) )
	{
            send_to_char("You need a deity first!\n\r", ch);
	    return;
        }

	if (ch->mana >= ch->max_mana)
	{
            ch->mana = ch->max_mana;
	    send_to_char("You're already at full mana!\n\r", ch);
	    return;
        }

	fillup = ch->max_mana - ch->mana;

	cost = fillup * manacost;

	if (ch->gold >= cost) 
	{
            printf_to_char( ch, "Thank you for your tithing of %d gold!\n\r", cost );
	    ch->mana = ch->max_mana;
	    ch->gold -= cost;
	    return;
	}

	cost = ch->gold;
	paid = (cost-(cost % manacost));	
	pumped = paid/manacost;
	
	if(pumped > 0)
	{
           printf_to_char( ch, "Thank you for the tithing of %d gold!\n\r", paid );
	   ch->mana += pumped;
	   ch->gold -= paid;
	   return;
	}

	return;
}


void do_sharpen(CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  AFFECT_DATA af;
  int percent, skill;

  argument = one_argument(argument, arg1);
  
  if (arg1[0] == '\0')
  {
    send_to_char("Sharpen what weapon?\n\r",ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg1, ch)) == NULL)
  {
    send_to_char("You do not have that weapon in your inventory.\n\r", ch );
    return;
  }
    
  if ((skill = get_skill(ch, skill_lookup("sharpen"))) < 1)
  {
    send_to_char("You don't have enough skill to enhance the edge of your weapon.\n\r",ch);
    return;
  }

  if (obj->item_type == ITEM_WEAPON)
  {
    if(IS_WEAPON_STAT(obj,WEAPON_SHARP))
    {
      act("$p has already been sharpened.",ch,obj,NULL,TO_CHAR);
      return;
    }

    percent = number_percent();

    if(percent < skill)
    {
	WAIT_STATE(ch,PULSE_VIOLENCE);

        af.where      = TO_WEAPON;
        af.type       = skill_lookup("sharpen");
        af.level      = ch->level/2;
        af.duration   = 8;
        af.location   = 0;
        af.modifier   = 0;
        af.bitvector  = WEAPON_SHARP;
        affect_to_obj(obj,&af);
      
        act("$n silently sharpens the edge of $p.",ch,obj,NULL,TO_ROOM);
        act("you sharpen the edge of $p.",ch,obj,NULL,TO_CHAR);
        check_improve(ch,skill_lookup("sharpen"),TRUE,3);
    }
    else
    {
	WAIT_STATE(ch,2 * PULSE_VIOLENCE);
	send_to_char("Your attempt to sharpen your weapon fails.\n\r",ch);
	check_improve(ch,skill_lookup("sharpen"),FALSE,2);
    }
  }
}


void do_cleave( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( MOUNTED(ch) ) 
    {
        send_to_char("You can't cleave while riding!\n\r", ch);
        return;
    }

    one_argument( argument, arg );

    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, skill_lookup("cleave") ) )
      {
	send_to_char("You don't know how to cleave.\n\r",ch);
	return;
      }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Cleave whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
        return;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "You need to wield a weapon to cleave.\n\r", ch );
	return;
    }

    if ( (victim->hit < (0.9 * victim->max_hit)) &&
	 (IS_AWAKE(victim)) )
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    WAIT_STATE( ch, skill_table[skill_lookup("cleave")].beats );
    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   number_percent( ) < get_skill(ch,skill_lookup("cleave")) )
    {
	check_improve(ch,skill_lookup("cleave"),TRUE,1);
	multi_hit(ch,victim,skill_lookup("cleave"));
    }
    
    else
    {
	check_improve(ch,skill_lookup("cleave"),FALSE,1);
	damage( ch, victim, 0, skill_lookup("cleave"),DAM_NONE, TRUE );
    }
  
    return;
}

void do_blackjack(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int chance;

    if ( MOUNTED(ch) ) 
    {
        send_to_char("You can't blackjack while riding!\n\r", ch);
        return;
    }

    if ( IS_NPC(ch) ||
	!can_use_skpell( ch, skill_lookup("blackjack") ) )

    {
	send_to_char("You can't do that.\n\r",ch);
        sound( "ycant.wav", ch );
	return;
    }

    if ( (victim = get_char_room(ch,NULL,argument)) == NULL)
      {
	send_to_char("You do not see that person here.\n\r",ch);
	return;
      }

    if (ch == victim)
      {
	send_to_char("Why do you want to blackjack yourself?\n\r",ch);
	return;
      }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )  
    {
	send_to_char( "You don't want to hit your master over the head!\n\r",ch);
	return;
    } 

    if (IS_AFFECTED(victim,AFF_SLEEP))  
    {
      act("$E is already asleep.",ch,NULL,victim,TO_CHAR);
      return;
    }

    if (is_safe(ch,victim))
        return;

    WAIT_STATE(ch,skill_table[skill_lookup("blackjack")].beats);

    chance = 0.5 * get_skill(ch,skill_lookup("blackjack"));
    chance += URANGE( 0, (get_curr_stat(ch,STAT_DEX)-20)*2, 10);
    chance += can_see(victim, ch) ? 0 : 5;

    if ( IS_NPC(victim) )
	if ( victim->pIndexData->pShop != NULL )
	   chance -= 40;
 
      if (IS_NPC(ch) || 
 	number_percent() < chance)
      {
	act("You hit $N on the head as hard as you can and watch them fall to the ground with a thud.",
	    ch,NULL,victim,TO_CHAR);
	act("You feel a sudden pain erupting through your skull as your hit on the head!",
	    ch,NULL,victim,TO_VICT);
	act("$n smacks $N as hard as $S can on the back of the head causing them to fall on the ground in a heap!",
	    ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,skill_lookup("blackjack"),TRUE,1);
	
	af.type = skill_lookup("blackjack");
        af.where = TO_AFFECTS;
	af.level = ch->level;
	af.duration = number_range( 1, 2 );
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_SLEEP;
	affect_join ( victim,&af );

	if (IS_AWAKE(victim))
	  victim->position = POS_SLEEPING;
      }

      else 
      {
        printf_to_char( ch, "You try desperately to hit %s over the back of the head but fail miserably!\n\r", victim->name );
	damage(ch,victim,ch->level / 2,skill_lookup("blackjack"),DAM_NONE, TRUE);
	check_improve(ch,skill_lookup("blackjack"),FALSE,1);
      }
}

void do_stake( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument(argument,arg);
 
  if ( (chance = get_skill(ch,skill_lookup("stake"))) &&
        !can_use_skpell( ch, skill_lookup("stake")))
    {	
	send_to_char("Stake? What's that?\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("Stake What undead??\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (!IS_NPC(victim) && !IS_VAMPIRE( victim ) )
    {
	send_to_char("You cannot stake someone who isn't a vampire.\n\r",ch);
	return;
    }

    if (IS_NPC(victim) && (!is_name("vampire",victim->name) &&
			   !is_name("undead", victim->name) &&
			   !is_name("zombie", victim->name) &&
			   !is_name("corpse", victim->name))) 
    {

	send_to_char("You cannot stake this mob.\n\r",ch);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You aren't undead.. you cannot stake yourself.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;


    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 15;
    else
	chance += (ch->size - victim->size) * 10; 


    /* stats */
    chance -= GET_AC(victim,AC_PIERCE) /25;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 10;

    /* level */
    chance += (ch->level - victim->level);

    /* now the attack */
    if (number_percent() < chance )
    {
    
	act("$n has stuck a stake in your heart!",
		ch,NULL,victim,TO_VICT);
	act("You slam a stake into $N!",ch,NULL,victim,TO_CHAR);
	act("$n shoves a stake into $N .",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,skill_lookup("stake"),TRUE,1);

	DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[skill_lookup("stake")].beats);
	victim->position = POS_RESTING;
	
	damage(ch,victim,((ch->level*(dice((int)ch->level/4,6))) + ch->level),skill_lookup("stake"),
	    DAM_PIERCE,TRUE);
	
    }

    else
    {
	damage(ch,victim,0,skill_lookup("stake"),DAM_PIERCE,TRUE);
	act("You fall flat on your face!",
	    ch,NULL,victim,TO_CHAR);
	act("$n falls flat on $s face.",
	    ch,NULL,victim,TO_NOTVICT);
	act("You evade $n's stake, causing $m to fall flat on $s face.",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,skill_lookup("stake"),FALSE,1);
	ch->position = POS_RESTING;
	WAIT_STATE(ch,skill_table[skill_lookup("stake")].beats * 3/2); 
    }

    check_killer(ch,victim);

}

void do_tail(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance, dam;
    int knock;

    one_argument(argument,arg);
    
    if (IS_NPC(ch) && IS_AFFECTED(ch,AFF_CHARM))
	return;

    if ( (chance = get_skill(ch,skill_lookup("tail"))) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TAIL))
    ||   (!IS_NPC(ch)
    &&    !can_use_skpell( ch, skill_lookup("tail") )))
    {   
	send_to_char("You don't have a tail to strike with like that.\n\r",ch);
        return;
    }
 
    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char("But you aren't fighting anyone!\n\r",ch);
            return;
        }
    }

    else if ((victim = get_char_room(ch,NULL,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    chance = URANGE(5,chance,90);

    chance += (ch->level - victim->level);
    chance -= get_curr_stat(victim,STAT_DEX);
    chance += get_curr_stat(ch,STAT_DEX)/3;
    chance += get_curr_stat(ch,STAT_STR)/3;

	act("$n violently lashes out with $s tail.",ch,0,0,TO_ROOM);
	send_to_char("You violently lash out with your tail!\n\r",ch);    

    if (number_percent() > chance)
    {
	damage(ch,victim,0,skill_lookup("tail"),DAM_BASH,TRUE);
	return;
    }

    dam = dice(ch->level, 3);
    knock = (ch->level + 15);
        
    damage(ch,victim,dam,skill_lookup("tail"),DAM_BASH,TRUE);
    if (number_percent() < knock)
    {
	act("$n is sent crashing to the ground by the force of the blow!",victim,0,0,TO_ROOM);
	send_to_char("The tail strike sends you crashing to the ground!\n\r",victim);
	WAIT_STATE(victim,( 3 * PULSE_VIOLENCE/2));
    }
    WAIT_STATE(ch,2*PULSE_VIOLENCE);
    return;
}

bool check_force_shield( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

	if(!IS_AFFECTED2(victim, AFF_FORCE_SHIELD))
		return FALSE;

    chance = 100 / 15;

    if (victim->level >= ch->level)
	chance += 2;

    if ( number_percent( ) >= chance )
        return FALSE;

    xact_new("Your force-shield blocks $n's attack!", ch, NULL, victim, TO_VICT, POS_RESTING, SUPPRESS_SHIELD);
    xact_new("$N's force-shield blocks your attack.", ch, NULL, victim, TO_CHAR, POS_RESTING, SUPPRESS_SHIELD);

    return TRUE;
}

/*
 *  Shield Spell Group by Tandon
 *  Static Shield Check
 */

bool check_static_shield( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance, sn;
    AFFECT_DATA *shock;

	if(!IS_AFFECTED2(victim, AFF_STATIC_SHIELD))
		return FALSE;

    chance = 10;

    if (victim->level >= ch->level)
	chance += 2;

    if ( number_percent( ) >= chance )
        return FALSE;

    
	sn = skill_lookup( "static shield" );
	shock = affect_find (victim->affected, sn);
	
	if(shock != NULL)
	{
		damage (victim, ch, number_fuzzy(shock->level / 5), sn, DAM_ENERGY, TRUE);
	}

    if (get_eq_char (ch, WEAR_WIELD) == NULL)
		return TRUE;

    xact_new("Your static shield catches $n!", victim, NULL, ch, TO_VICT, POS_RESTING, SUPPRESS_SHIELD);
    xact_new("$N's static shield catches you!", victim, NULL, ch, TO_CHAR, POS_RESTING, SUPPRESS_SHIELD);
    
	spell_heat_metal (skill_lookup( "heat metal" ),
                     victim->level/2, victim, (void *) ch, TARGET_CHAR);		

    return TRUE;
}

/*
 *  Shield Spell Group by Tandon
 *  Flame Shield Check
 */

bool check_flame_shield( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance, sn;
	AFFECT_DATA *burn;

	if(!IS_AFFECTED2(victim, AFF_FLAME_SHIELD))
		return FALSE;

    if (get_eq_char (victim, WEAR_WIELD) != NULL)
		return FALSE;

    chance = 100 / 3;

    if (victim->level >= ch->level)
	chance += 2;

    if ( number_percent( ) >= chance )
        return FALSE;

	sn = skill_lookup( "flame shield" );
	burn = affect_find (victim->affected, sn);
	
	if(burn != NULL)
	{   
		fire_effect (ch, burn->level, number_fuzzy(10), TARGET_CHAR);
		damage (victim, ch, number_fuzzy(burn->level), sn, DAM_FIRE, TRUE);
	}

    return TRUE;
}



void do_mobslay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Mobslay who?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "{1You slay $M in cold blood!{x",  ch, NULL, victim, TO_CHAR    );
    act( "{1$n slays you in cold blood!{x", ch, NULL, victim, TO_VICT    );
    act( "{1$n slays $N in cold blood!{x",  ch, NULL, victim, TO_NOTVICT );
    raw_kill( victim, ch );
    return;
}



void do_feed( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int dam;
    int number = number_range( 1, 100 );
  
    if ( get_skill(ch,skill_lookup("feed")) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_FEED))	
    ||	 (!IS_NPC(ch)
    &&	  !can_use_skpell( ch, skill_lookup("feed") ) ) )
    {	
	send_to_char("Feed? What's that?\n\r",ch);
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }
 
    if ( !IS_VAMPIRE( ch ) )
    {
        send_to_char( "I'm sorry your not a vampire you can't bite someone!\n\r", ch );
        return;
    }

    if(!can_see(ch,victim))
    {
        send_to_char("You can't see your opponent's neck!\n\r",ch);
        return;
    }

    WAIT_STATE( ch, skill_table[skill_lookup("feed")].beats );
    if ( number_percent( ) <  (get_skill(ch,skill_lookup("feed")) + (ch->level - victim->level))
    || ( get_skill(ch,skill_lookup("feed")) >= 2 && !IS_AWAKE(victim)))
    {
        check_improve(ch,skill_lookup("feed"),TRUE,1);
	act( "{r$n moves in close and bites you!{x", ch, NULL, victim, TO_VICT);
	act( "{bYou move in close and bite $N.{x", ch, NULL, victim, TO_CHAR);
	act( "{m$n moves in close and bites $N.{x", ch, NULL, victim, TO_NOTVICT);

	dam = number_range( ((ch->level/2)+(victim->level/2)),
	((ch->level)+(victim->level/2))*2 );

        damage( ch, victim, dam, skill_lookup("feed"),DAM_NEGATIVE,TRUE);

        if ( number > 50 )
        {
          dam = 2 * number_range(((ch->level/2) + (victim->level/2)),
          ((ch->level) + (victim->level/2))*2);
          damage(ch,victim,dam,skill_lookup("feed"),DAM_NEGATIVE,TRUE);
        }
    }

    else
    {
        check_improve(ch,skill_lookup("feed"),FALSE,1);
	act( "{r$n tries to move in close and bite you, but hits only air.{x", ch, NULL, victim, TO_VICT);
	act( "{bYou chomp a mouthfull of air.{x", ch, NULL, victim, TO_CHAR);
	act( "{m$n tries to move in close and bite $N.{x", ch, NULL, victim, TO_NOTVICT);
        damage( ch, victim, 0, skill_lookup("feed"),DAM_NEGATIVE,TRUE);
    }
 
    return;
}


void do_rub( CHAR_DATA *ch, char *argument)
{
 int chance;

 if ( IS_NPC( ch ) )
 {
     send_to_char( "Sorry dude you can't rub your a mob!\n\r", ch );
     return;
 }

 if ((chance = get_skill(ch,skill_lookup("rub"))) == 0
    || !can_use_skpell( ch, skill_lookup("rub") ) )
 {
      send_to_char("{xYou don't know the first thing about rubbing your eyes.{x\n\r",ch);
      return;
 }

 if(!IS_AFFECTED(ch,AFF_BLIND))
 {
  send_to_char("{BBut you aren't blind!{x\n\r",ch);
  return;
 }

 if(!is_affected(ch,skill_lookup("fire breath")) && !is_affected(ch,skill_lookup("dirt")))
 {
  send_to_char("{xRubbing won't help that!{x\n\r",ch);
  return;
 }

 WAIT_STATE(ch,skill_table[skill_lookup("rub")].beats);
 chance = (chance*3)/4;
 if(number_percent()>chance)
 {
  send_to_char("{RYou failed to rub your eyes clear!{x\n\r",ch);
  check_improve(ch,skill_lookup("rub"),FALSE,2);
  return;
 }

 if(is_affected(ch,skill_lookup("fire breath")))
  affect_strip(ch,skill_lookup("fire breath"));
 if(is_affected(ch,skill_lookup("dirt")))
  affect_strip(ch,skill_lookup("dirt"));
 send_to_char("{xYou rubbed your eyes clear!{x\n\r",ch);
 check_improve(ch,skill_lookup("rub"),TRUE,2);
 return;

}


void do_throatchop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if (arg[0] == '\0')
    {
        send_to_char("Chop who's throat?\n\r",ch);
        return;
    }

    if (ch->fighting != NULL)
    {
	send_to_char("You can't get at thier throat!\n\r",ch);
	return;
    }
 
    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;


    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "You need to wield a weapon to chop thier throat.\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit / 2)
    {
	act( "$N is far to suspicious to be chopped.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[skill_lookup("throatchop")].beats );
    if ( number_percent( ) < get_skill(ch,skill_lookup("throatchop"))
    || ( get_skill(ch,skill_lookup("throatchop")) >= 2 && !IS_AWAKE(victim) ) )
    {
	check_improve(ch,skill_lookup("throatchop"),TRUE,1);
	multi_hit( ch, victim, skill_lookup("throatchop") );
    }
    else
    {
	check_improve(ch,skill_lookup("throatchop"),FALSE,1);
	damage( ch, victim, 0, skill_lookup("throatchop"),DAM_NONE,TRUE);
    }

    return;
}



void do_rally(CHAR_DATA *ch, char *argument)
{
     CHAR_DATA *gch;
     int heal;
     int chance;

     if ((chance = get_skill(ch, skill_lookup("rally")) == 0 ) )
     {
          send_to_char("You scream and holler, but nobody listens.\n\r",ch);
          return;
     }

     if (IS_AFFECTED(ch,AFF_CALM))
     {
          send_to_char("You're feeling to mellow to lead your companions on.\n\r",ch);
          return;
     }

     if (ch->position == POS_FIGHTING)
          chance += 25;

     if (number_percent() > chance)
     {
          send_to_char("You try to rally your companions, but your efforts fail.\n\r", ch);
          act("{R$n tries to rally $s companions, but fails.{w",ch,NULL,NULL,TO_ROOM);
          WAIT_STATE(ch,PULSE_VIOLENCE * 2);
          return;
     }

     send_to_char("You call your companions to arms, promising them glory!\n\r", ch);
     act("{R$n calls $s companions to arms, promising great glory!{w", ch, NULL, NULL, TO_ROOM);

     for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
     {
          if ( gch != ch && is_same_group(ch, gch) && !IS_AFFECTED(gch, AFF_CALM) )
          {
               if ( !is_affected(gch,skill_lookup("rally") ) )
               {
                    AFFECT_DATA af;
                    send_to_char ("You answer the call to arms!\n\r", gch);

                    check_improve(ch,skill_lookup("rally"),TRUE,2);

                    af.type         = skill_lookup("rally");
                    af.level        = ch->level;
                    af.duration     = number_fuzzy(UMAX(2,ch->level / 8) );
                    af.modifier     = UMAX(1,ch->level/5);

                    af.bitvector = 0;

                    af.location     = APPLY_HITROLL;
                    af.where = TO_AFFECTS;
                    affect_to_char(gch,&af);

                    af.location     = APPLY_DAMROLL;
                    affect_to_char(gch,&af);

                    af.modifier = UMAX(1, ch->level/10);
                    af.location   = APPLY_STR;
                    affect_to_char(gch,&af);

               }
		/* Always do this part */

               heal = dice(1, ch->level) + ch->level / 3;
               gch->hit = UMIN( gch->hit + heal, gch->max_hit );
               send_to_char("You feel better!\n\r", gch);

               update_pos( gch );
          }
     }

     WAIT_STATE(ch,PULSE_VIOLENCE * 2);

}

void do_punch( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int number = 0;

    number = number_range( 0, 5 );

    if ( !IS_NPC(ch)
    &&   !can_use_skpell(ch, skill_lookup("punch")) )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't punch while riding!\n\r", ch);
        return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[skill_lookup("punch")].beats );
    if ( get_skill(ch,skill_lookup("punch")) > number_percent())
    {
	damage(ch,victim,number_range( 50, ch->level + 100 / 2 ), skill_lookup("punch"),DAM_BASH,TRUE);
        DAZE_STATE( victim, 3 * PULSE_VIOLENCE );
	check_improve(ch,skill_lookup("punch"),TRUE,1);
        if ( number <= 1 )
        {
            printf_to_char( ch, "{YYou make a series of lightning fast punches right to %ss head!{x\n\r", victim->short_descr );
            damage(ch,victim,number_range( 80, ch->level + 100 / 2 ), skill_lookup("punch"), DAM_BASH, TRUE );
            damage(ch,victim,number_range( 80, ch->level + 100 / 2 ), skill_lookup("punch"), DAM_BASH, TRUE );
            check_improve(ch,skill_lookup("punch"),TRUE,1);
            rnd_sound( 3, ch, "punch.wav", "punch3.wav", "punch6.wav" );
            rnd_sound( 3, victim, "punch.wav", "punch3.wav", "punch6.wav" );
        }
    }
    else
    {
	damage( ch, victim, 0, skill_lookup("punch"),DAM_BASH,TRUE);
	check_improve(ch,skill_lookup("punch"),FALSE,1);
    }
	
    check_killer(ch,victim);
    return;
}


