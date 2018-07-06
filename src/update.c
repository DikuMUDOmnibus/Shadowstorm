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
#include <stdlib.h>
#include <ctype.h>
#include "merc.h"
#include "interp.h"
#include "music.h"
#include "const.h"

/*
 * Local functions.
 */
int	hit_gain	args( ( CHAR_DATA *ch ) );
int	mana_gain	args( ( CHAR_DATA *ch ) );
int	move_gain	args( ( CHAR_DATA *ch ) );
void	mobile_update	args( ( void ) );
void	weather_update	args( ( void ) );
void	char_update	args( ( void ) );
void	obj_update	args( ( void ) );
void	aggr_update	args( ( void ) );
void    quest_update    args( ( void ) );
bool    in_donation_room args( (OBJ_DATA *obj) );    
void    room_aff_update args( ( ROOM_INDEX_DATA *room ) );
void	newbie_update	args( ( void ) );
void    quest_update    args( ( void ) ); 
void    gquest_update   args( ( void ) );
void    olcautosave     args( ( void ) );
void    who_html_update args( ( void ) );
void    save_area       args( ( AREA_DATA *pArea ) );      
void    save_area_list  args( ( void ) );
void    gain_self_exp   args( ( CHAR_DATA *ch, int gain ) );
void    gain_bloodthirst args( ( CHAR_DATA *ch, int value ) );
void    check_werewolf  args( ( ) );
void    free_event      args( (EVENT_DATA *ev) );
void    event_update    args( ( void ) );
void    gain_object_exp args( ( CHAR_DATA *ch, OBJ_DATA *obj, int gain ) );
void    advance_level_object args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void    trivia_update   args( ( void ) );   
void    reboot          args( ( void ) );
void    shutdown        args( ( void ) );
void    update_timers   args( ( void ) );
int     mortal_online   args( ( ) );
extern CHAR_DATA *copyover_person;

EVENT_DATA	*events;

/* used for saving */

int	save_number = 0;


int     global_exp;
int     global_qp;      
int     global_quad;
int     copyovert;
int     reboott;
int     shutdownt;
sh_int  display;
sh_int  qpdisplay;
sh_int  quaddisplay;
sh_int  displayt;
sh_int  rdisplay;
sh_int  sdisplay;

#define kill_event \
				if(!ev_next)  {   free_event(ev); return; }  \
				if(ev_last!=NULL) ev_last->next=ev_next;  \
				             else events=ev_next;  \
				free_event(ev);  \
				continue;
/*
 * Advancement stuff.
 */
void advance_level( CHAR_DATA *ch, bool hide )
{
    char buf[MAX_STRING_LENGTH];
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;
    int hpbonus, movebonus, manabonus;
    MEMBER_DATA *pMem;
	
    ch->pcdata->share_level = ch->level;
    ch->pcdata->shares_bought = 0;

    hpbonus = ( ch->levelflux * 7 );
    manabonus = ( ch->levelflux * 7 );
    movebonus = ( ch->levelflux * 2 );

    ch->pcdata->last_level = 
	( ch->played + (int) (current_time - ch->logon) ) / 3600;

    add_hp    = con_app[get_curr_stat(ch,STAT_CON)].hitp + get_hp_gain(ch);
    add_mana 	= number_range(2,(2*get_curr_stat(ch,STAT_INT)
				  + get_curr_stat(ch,STAT_WIS))/5);
    if (!has_spells(ch)) 
	add_mana /= 2;
    add_move	= number_range( 1, (get_curr_stat(ch,STAT_CON)
				  + get_curr_stat(ch,STAT_DEX))/6 );
    add_prac	= wis_app[get_curr_stat(ch,STAT_WIS)].practice;

    add_hp = (add_hp * 9/10) + hpbonus;
    add_mana = (add_mana * 9/10) + manabonus;
    add_move = (add_move * 9/10) + movebonus;

    add_hp	= UMAX(  2, add_hp   );
    add_mana	= UMAX(  2, add_mana );
    add_move	= UMAX(  6, add_move );

    ch->max_hit 	+= add_hp;
    ch->max_mana	+= add_mana;
    ch->max_move	+= add_move;
    ch->practice	+= add_prac;
    ch->train		+= 1;
    ch->pcdata->perm_hit	+= add_hp;
    ch->pcdata->perm_mana	+= add_mana;
    ch->pcdata->perm_move	+= add_move;

    update_all_qobjs(ch);    

    if (!hide)
    {
    	sprintf(buf,
	    "You gain %d hit point%s, %d mana, %d move, and %d practice%s.\n\r",
	    add_hp, add_hp == 1 ? "" : "s", add_mana, add_move,
	    add_prac, add_prac == 1 ? "" : "s");
	send_to_char( buf, ch );
    }

    if( is_clan(ch) )
    {
	if( (pMem = member_lookup(&clan_table[ch->clan], ch->name) ) != NULL )
	    pMem->level = ch->level;
    }
 
    return;
}   



void gain_exp( CHAR_DATA *ch, int gain )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) || ch->level >= LEVEL_HERO || IS_WEREWOLF(ch))
	return;

    if (!IS_SET( ch->comm2, COMM_AUTOLEVEL ) )
    {
        gain_self_exp( ch, gain );    
        return;
    }

    if (IS_SET( ch->comm2, COMM_PETLEVEL ) && ch->pet != NULL )  
    {
        pet_gain_exp( ch->pet, gain );
	gain /= 2; // Pet got half of it. ;)
    }    

    ch->exp = UMAX( exp_per_level(ch,ch->pcdata->points), ch->exp + gain );
    while ( ch->level < LEVEL_HERO && ch->exp >= 
	exp_per_level(ch,ch->pcdata->points) * (ch->level+1) )
    {
	send_to_char( "{GYou raise a level!!{x  ", ch );
	ch->level += 1;
        rnd_sound( 2, ch, "level.wav", "level1.wav" );
        sound( "level.wav", ch );
        if ( !IS_NPC(ch) && ch->pcdata->bcount++ == 10  ) 
        {
            ch->pcdata->bcount = 0;
            ch->pcdata->bless++;       
            printf_to_char( ch, "You have gained a blessing point!\n\r"  );
        }
	sprintf(buf,"%s gained level %d",ch->name,ch->level);
	log_string(buf);
	sprintf(buf,"$N has attained level %d!",ch->level);
	wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);
	advance_level(ch,FALSE);
        info( ch, 0, "{G[INFO]:{x {G%s{x {Bhas just attained level{x {R%d{x!{x\n\r", ch->name, ch->level );
        ch->pcdata->mdam = (ch->level * 50);
        affect_strip(ch,skill_lookup("plague"));
        affect_strip(ch,skill_lookup("poison"));
        affect_strip(ch,skill_lookup("blindness"));
        affect_strip(ch,skill_lookup("sleep"));
        affect_strip(ch,skill_lookup("curse"));

        ch->hit        = ch->max_hit;
        ch->mana       = ch->max_mana;
        ch->move       = ch->max_move;
        update_pos( ch);
        send_to_char( "{G[INFO]:{x {BSynon has given you the power to continue!{x\n\r", ch );
        save_char_obj(ch);
    }

    return;
}

void gain_self_exp( CHAR_DATA *ch, int gain )
{

    if (IS_SET( ch->comm2, COMM_PETLEVEL ) && ch->pet != NULL ) 
    {
        pet_gain_exp( ch->pet, gain );
	gain /= 2; // Pet got half of it. ;)
    }      

    if ( IS_NPC(ch) || ch->level >= LEVEL_HERO )
	return;

    if (ch->pcdata->current_exp > exp_per_level(ch,ch->pcdata->points) * 6)
    send_to_char( "{YYou have already gained enough exp for five levels - go level up!{x\n\r", ch );

    else if (ch->pcdata->current_exp < exp_per_level(ch, ch->pcdata->points)) 
    {
        ch->pcdata->current_exp += gain;
 
	if ( ch->level < LEVEL_HERO && ch->pcdata->current_exp >= exp_per_level(ch, ch->pcdata->points))
	    send_to_char( "{YYou may raise a level!! Go visit the healer and type level!!{x\n\r", ch );
    }
	
    else if (ch->pcdata->current_exp > exp_per_level(ch, ch->pcdata->points) * 5 && ch->pcdata->current_exp < exp_per_level(ch, ch->pcdata->points) * 6)
    {
        ch->pcdata->current_exp += gain; 
	send_to_char( "{YYou may raise five levels!! You may not gain any more exp until you level at the healer!!{x\n\r", ch );
    }
        
    else
        ch->pcdata->current_exp += gain;

    return;
}   


void pet_gain_exp( CHAR_DATA *pet, int gain )
{
    char buf[MSL];

    if ( !IS_NPC(pet) || pet->level >= LEVEL_HERO || !IS_SET( pet->act, ACT_PET ) )
	return;

    pet->exp += gain;

    if ( pet->level < LEVEL_HERO && pet->exp >= pet->xp_tolevel )
    {
	pet->level++;
        pet->exp -= pet->xp_tolevel;
	sprintf(buf,"%s gained level %d",pet->name,pet->level);
	log_string(buf);
	sprintf(buf,"$N has attained level %d!",pet->level);
	wiznet(buf,pet,NULL,WIZ_LEVELS,0,0);
	advance_level_pet(pet,FALSE);
        affect_strip(pet,skill_lookup("plague"));
        affect_strip(pet,skill_lookup("poison"));
        affect_strip(pet,skill_lookup("blindness"));
        affect_strip(pet,skill_lookup("sleep"));
        affect_strip(pet,skill_lookup("curse"));
        sprintf( buf, "{BI just gained level %d!{x\n\r", pet->level );
        do_say( pet, buf );
        do_say( pet, "{BYou can use the petshow command to see what my stats are now!{x\n\r" );
        pet->hit       = pet->max_hit;
        pet->mana      = pet->max_mana;
        pet->move      = pet->max_move;
        update_pos( pet);
        save_char_obj(pet);
    }

    return;  
}

void gain_object_exp( CHAR_DATA *ch, OBJ_DATA *obj, int gain )
{
	int leftover = 0;

    if ( IS_NPC(ch) || obj->plevel >= MOBJLEVEL || IS_WEREWOLF(ch) )
		return;

    printf_to_char( ch, "%s has gained %d exp.\n\r", capitalize( obj->short_descr ), gain );     
    obj->exp += gain;
    obj->xp_tolevel -= gain;

	if(obj->xp_tolevel <= 0 )
	{	obj->exp += obj->xp_tolevel;
		advance_level_object(ch,obj);
		leftover = ( obj->xp_tolevel * 1 );
		obj->plevel++;
		printf_to_char( ch, "%s has raised to level %d. To see your objects stats lore or identify it.\n\r", capitalize( obj->short_descr ), obj->plevel );
		obj->xp_tolevel = 1500 + ( obj->plevel * 150 );
		obj->xp_tolevel -= leftover;
		return;
	}
	return;
}


void advance_level_pet( CHAR_DATA *pet, bool hide )
{
    char buf[MSL];
    int add_hp = number_range( 20, 40 );
    int add_mana = number_range( 15, 30 );
    int add_move = number_range( 25, 40 );
    int add_ac = number_range( 10, 25 );
    int add_prac = number_range( 5, 10 );
    int hpbonus, manabonus, movebonus;
    int i;

    hpbonus   = ( pet->levelflux * 7 );
    manabonus = ( pet->levelflux * 7 );
    movebonus = ( pet->levelflux * 2 );    

    add_hp    = ( add_hp * 9/10 ) + hpbonus;
    add_mana  = ( add_mana * 9/10 ) + manabonus;
    add_move  = ( add_move * 9/10 ) + movebonus;
    add_prac  = add_prac * 4/5;
    add_ac    = add_ac * 9/10;

    add_hp    = UMAX(  2, add_hp   );
    add_mana  = UMAX(  2, add_mana );
    add_move  = UMAX(  6, add_move );

    pet->max_hit   += add_hp;
    pet->max_mana  += add_mana;
    pet->max_move  += add_move; 
    pet->practice  += add_prac;  
    pet->train	   ++;
    pet->damroll   += number_range( 1, 2 );
    pet->hitroll   += number_range( 1, 2 );
    pet->damage[DICE_NUMBER]++;
    pet->damage[DICE_TYPE]++;
    pet->damage[DICE_BONUS] = number_range( 4, 8 );
    for ( i = 0; i < 3; i++ )
         pet->armor[i] -= add_ac;    

    if (!hide)
    {
    	sprintf(buf,
	    "I gained %d hit point%s, %d mana, and %d moves.\n\r",
	    add_hp, add_hp == 1 ? "" : "s", add_mana, add_move);
	do_say( pet, buf );
    }

    return;
}   

void advance_level_object( CHAR_DATA *ch, OBJ_DATA *obj )
{
    int pbonus = number_range( 5, 10 );
    int bonus = number_range( 4, 8 );

    pbonus  = pbonus * 9/10;
    bonus =   bonus * 8/10;

    pbonus  = UMAX(  6, pbonus );
    bonus = UMAX( 1, bonus );

    add_apply(obj, APPLY_DAMROLL, pbonus, TO_OBJECT, 0, -1, 0, obj->plevel * 5);
    add_apply(obj, APPLY_HITROLL, pbonus, TO_OBJECT, 0, -1, 0, obj->plevel * 5); 
    add_apply(obj, APPLY_HIT, pbonus, TO_OBJECT, 0, -1, 0, obj->plevel * 5);
    add_apply(obj, APPLY_MANA, pbonus, TO_OBJECT, 0, -1, 0, obj->plevel * 5);
    add_apply(obj, APPLY_MOVE, pbonus, TO_OBJECT, 0, -1, 0, obj->plevel * 5);

    if (obj->item_type == ITEM_WEAPON)
    {
        obj->value[1] += bonus/4;
        obj->value[2] += bonus/5;
    } 

    else if (obj->item_type == ITEM_ARMOR)
    {
        obj->value[0] -= UMAX(1, obj->plevel);
        obj->value[1] -= UMAX(1, obj->plevel);
        obj->value[2] -= UMAX(1, obj->plevel);
        obj->value[3] -= (5 * UMAX(1, obj->plevel)) / 10;
    }
   
    return;
}   

/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain =  10 + ch->level;
 	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;

	switch(ch->position)
	{
	    default : 		gain /= 2;			break;
	    case POS_SLEEPING: 	gain = 3 * gain/2;		break;
	    case POS_RESTING:  					break;
	    case POS_FIGHTING:	gain /= 3;		 	break;
 	}

	
    }
    else
    {
	gain = UMAX(3,get_curr_stat(ch,STAT_CON) - 3 + ch->level/2); 
	gain += hp_max(ch) - 10;
 	number = number_percent();
	if (number < get_skill(ch,skill_lookup("fast healing")))
	{
	    gain += number * gain / 100;
	    if (ch->hit < ch->max_hit)
		check_improve(ch,skill_lookup("fast healing"),TRUE,8);
	}

	switch ( ch->position )
	{
	    default:	   	gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:  	gain /= 2;			break;
	    case POS_FIGHTING: 	gain /= 6;			break;
	}

	if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;
       
        if ( ch->in_room->vnum == ch->pcdata->rentroom  && ch->silver > 0)
            gain *= 2;

        if ( IS_SET(ch->act, PLR_TENNANT) && ch->silver > 0)
            ch->silver -= 10;

        if ( ch->bloodthirst <= 0 && IS_VAMPIRE( ch ) )
		 gain = 0;

	 if ( ch->bloodthirst >= 0 && IS_VAMPIRE( ch ) && ( time_info.hour > 19 && time_info.hour < 5 ) )
		gain *= 2;
    }

    gain = gain * ch->in_room->heal_rate / 100;
    
    gain *= 3;
    
    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
	gain /=2 ;

    return UMIN(gain, ch->max_hit - ch->hit);
}


int mana_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = 5 + ch->level;
	switch (ch->position)
	{
	    default:		gain /= 2;		break;
	    case POS_SLEEPING:	gain = 3 * gain/2;	break;
   	    case POS_RESTING:				break;
	    case POS_FIGHTING:	gain /= 3;		break;
    	}
    }
    else
    {
	gain = (get_curr_stat(ch,STAT_WIS) 
	      + get_curr_stat(ch,STAT_INT) + ch->level) / 2;
	number = number_percent();
	if (number < get_skill(ch,skill_lookup("meditation")))
	{
	    gain += number * gain / 100;
	    if (ch->mana < ch->max_mana)
	        check_improve(ch,skill_lookup("meditation"),TRUE,8);
	}

	if (!has_spells(ch))
	    gain /= 2;

	switch ( ch->position )
	{
	    default:		gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:	gain /= 2;			break;
	    case POS_FIGHTING:	gain /= 6;			break;
	}

	if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;
        if ( ch->bloodthirst <= 0 && IS_VAMPIRE( ch ) )
                 gain = 0;

         if ( ch->bloodthirst >= 0 && IS_VAMPIRE( ch ) && ( time_info.hour > 19 && time_info.hour < 5 ) )
                gain *= 2;

    }

    gain = gain * ch->in_room->mana_rate / 100;

    gain *= 3;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[4] / 100;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;

    return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain( CHAR_DATA *ch )
{
    int gain;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = ch->level;
    }

    else
    {
	gain = UMAX( 15, ch->level );

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_stat(ch,STAT_DEX);		break;
	case POS_RESTING:  gain += get_curr_stat(ch,STAT_DEX) / 2;	break;
	}

	if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

        if ( ch->bloodthirst <= 0 && IS_VAMPIRE( ch ) )
                 gain = 0;

         if ( ch->bloodthirst >= 0 && IS_VAMPIRE( ch ) && ( time_info.hour > 19 && time_info.hour < 5 ) )
                gain *= 2;

    }

    gain = gain * ch->in_room->heal_rate/100;

    gain *= 3;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;

    return UMIN(gain, ch->max_move - ch->move);
}



void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;

    if ( value == 0 || IS_NPC(ch) || ( ch->level >= LEVEL_HERO && iCond != COND_DRUNK ) )
	return;

    condition				= ch->pcdata->condition[iCond];

    if (condition == -1)
        return;

    ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 48 );

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_HUNGER:
	    send_to_char( "You are hungry.\n\r",  ch );
	    break;

	case COND_THIRST:
	    send_to_char( "You are thirsty.\n\r", ch );
	    break;

	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char( "You are sober.\n\r", ch );
	    break;
       
        case COND_TIRED:
            if ( condition != 0 )
                send_to_char( "You are getting {mtired{x. You should get some sleep!\n\r", ch);
            break;
	}
    }

    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all mobs. */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if ( !IS_NPC(ch) || ch->in_room == NULL || IS_AFFECTED(ch,AFF_CHARM))
	    continue;

	if (ch->in_room->area->empty && !IS_SET(ch->act,ACT_UPDATE_ALWAYS))
	    continue;

	/* Examine call for special procedure */
	if ( ch->spec_fun != 0 )
	{
	    if ( (*ch->spec_fun) ( ch ) )
		continue;
	}

	if (ch->pIndexData->pShop != NULL) /* give him some gold */
	    if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth)
	    {
		ch->gold += ch->pIndexData->wealth * number_range(1,20)/5000000;
		ch->silver += ch->pIndexData->wealth * number_range(1,20)/50000;
	    }
	 
       	/*
	 * Check triggers only if mobile still in default position
	 */
	if ( ch->position == ch->pIndexData->default_pos )
	{
	    /* Delay */
	    if ( HAS_TRIGGER_MOB( ch, TRIG_DELAY) 
	    &&   ch->mprog_delay > 0 )
	    {
		if ( --ch->mprog_delay <= 0 )
		{
                    p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_ENTRY );  
		    continue;
		}
	    } 
	    if ( HAS_TRIGGER_MOB( ch, TRIG_RANDOM) )
	    {
		if( p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_RANDOM ) )
		continue;
	    }
	}

	/* That's all for sleeping / busy monster, and empty zones */
	if ( ch->position != POS_STANDING )
	    continue;

	/* Scavenge */
	if ( IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->contents != NULL
        &&   !IS_SET(ch->in_room->room_flags, ROOM_DONATION)
	&&   number_bits( 6 ) == 0 )
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = 0;
	    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	    {
		if ( CAN_WEAR(obj, ITEM_TAKE) && can_loot(ch, obj)
		     && obj->cost > max  && obj->cost > 0)
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best )
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
	    }
	}

	/* Wander */
	if ( !IS_SET(ch->act, ACT_SENTINEL) 
	&& number_bits(3) == 0
	&& ( door = number_bits( 5 ) ) <= 5
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	&& ( !IS_SET(ch->act, ACT_STAY_AREA)
	||   pexit->u1.to_room->area == ch->in_room->area ) 
	&& ( !IS_SET(ch->act, ACT_OUTDOORS)
	||   !IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)) 
	&& ( !IS_SET(ch->act, ACT_INDOORS)
	||   IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)))
	{
	    move_char( ch, door, FALSE );
	}
    }

    return;
}

/*
 * Update all chars, including mobs.
*/
void char_update( void )
{   
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;
    OBJ_DATA *wield;
    OBJ_DATA *hold;
    OBJ_DATA *secondary;
    int  number = number_range( 1, 100 );

    ch_quit	= NULL;

    /* update save counter */
    save_number++;

    if (save_number > 29)
	save_number = 0;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
  
        if (!IS_VALID(ch)) 
        {
            bug("update_char: Trying to work with an invalidated character.\n", 0); 
            break;
        }

	ch_next = ch->next;

        if ( ch->timer > 30 )
            ch_quit = ch;

	if ( ch->position >= POS_STUNNED )
	{
            /* check to see if we need to go home */
            if (IS_NPC(ch) && ch->zone != NULL && ch->zone != ch->in_room->area
            && ch->desc == NULL &&  ch->fighting == NULL 
	    && !IS_AFFECTED(ch,AFF_CHARM) && number_percent() < 5)
            {
            	act("$n wanders on home.",ch,NULL,NULL,TO_ROOM);
            	extract_char(ch,TRUE);
            	continue;
            }

	    if ( ch->hit  < ch->max_hit )
		ch->hit  += hit_gain(ch);
	    else
		ch->hit = ch->max_hit;

	    if ( ch->mana < ch->max_mana )
		ch->mana += mana_gain(ch);
	    else
		ch->mana = ch->max_mana;

	    if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);
	    else
		ch->move = ch->max_move;
	}

	if ( ch->position == POS_STUNNED )
	    update_pos( ch );

        if ( !IS_NPC(ch) && ch->pcdata->mdam < ch->level * 50 )
            ch->pcdata->mdam = (ch->level * 50);     

        if ( !IS_NPC(ch) && mortal_online() >= 5 && number == 1 )
        {
            do_function( ch, &do_divine, "all" );
            do_function( ch, &do_restore, "all" );
            info( ch, 0, "Divine grace has been granted to the mud.\n\r" );
        }

        if ( !IS_NPC( ch ) && ch->desc != NULL && ch->pcdata->msp == 1 && ch->desc->msp != TRUE )
            ch->desc->msp = ch->pcdata->msp;

        if ( !IS_NPC( ch ) && ch->desc != NULL && ch->pcdata->mxp == 1 )
            ch->desc->mxp = TRUE;

	if ( !IS_NPC(ch) && ch->level < LEVEL_IMMORTAL )
	{
	    OBJ_DATA *obj;

	    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room != NULL )
		{
		    --ch->in_room->light;
		    act( "$p goes out.", ch, obj, NULL, TO_ROOM );
		    act( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR );
		    extract_obj( obj );
		}
	 	else if ( obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p flickers.",ch,obj,NULL,TO_CHAR);
	    }

            if ( ch->pcdata->timeout-- > 0 )
            {
                printf_to_char( ch, "You have %d ticks of timeout left!\n\r", ch->pcdata->timeout );

                if ( ch->pcdata->timeout == 0 )
                {
                    timeout( ch );
                }
            }     

	    if ( ++ch->timer >= 12 )
	    {
		    if ( ch->fighting != NULL )
			stop_fighting( ch, TRUE );
                    if ( ch->timer == 12 )
                    set_auto_afk( ch );
		    if (ch->level > 1)
		        save_char_obj( ch );
	    }

            if ( !IS_NPC(ch) && ch->position > POS_SLEEPING
                && ch->pcdata->condition[COND_TIRED] == 0
		&& ch->timer < 12 && ch->desc )
            {
            wield = get_eq_char( ch, WEAR_WIELD );
	    secondary = get_eq_char( ch, WEAR_SECONDARY );
	    hold = get_eq_char( ch, WEAR_HOLD );
            act( "{w$n is sleeping on $s feet!{x",ch,NULL,NULL,TO_ROOM);
            if ( wield )
            {
                act( "{w$p {wclatters to the ground.",ch,wield,NULL,TO_ROOM );
		    send_to_char( "{wSomething clatters to the ground waking you.{x\n\r", ch );
                obj_from_char( wield );
                obj_to_room( wield, ch->in_room );
            }
	    if( secondary && !wield )
            {
                act( "{w$p {wclatters to the ground.",ch,secondary,NULL,TO_ROOM );
		send_to_char( "{wSomething clatters to the ground waking you.{x\n\r", ch );
                obj_from_char( secondary );
                obj_to_room( secondary, ch->in_room );
            }
	    if( hold && !wield && !secondary )
            {
                act( "{w$p {wclatters to the ground.",ch,hold,NULL,TO_ROOM );
		send_to_char( "{wSomething clatters to the ground waking you.{x\n\r", ch );
                obj_from_char( hold );
                obj_to_room( hold, ch->in_room );
            }
        } 
	    gain_condition( ch, COND_DRUNK,  -1 );
	    gain_condition( ch, COND_FULL, ch->size > SIZE_MEDIUM ? -4 : -2 );
	    gain_condition( ch, COND_THIRST, -1 );
	    gain_condition( ch, COND_HUNGER, ch->size > SIZE_MEDIUM ? -2 : -1);
            gain_bloodthirst( ch, -1 );
 
            if ( !IS_NPC(ch) && !IS_IMMORTAL(ch) )
            {
                if ( ch->position == POS_RESTING )
                    gain_condition( ch, COND_TIRED, 0 );
                else if (ch->position == POS_SLEEPING )
                    gain_condition( ch, COND_TIRED,
                            get_curr_stat(ch,STAT_CON) / 2 );
                else
                    gain_condition( ch, COND_TIRED,
			get_curr_stat(ch,STAT_CON) > 19 ? -1 : -2 );
            } 

	}

	for ( paf = ch->affected; paf != NULL; paf = paf_next )
	{
	    paf_next	= paf->next;
	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
            }
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char( skill_table[paf->type].msg_off, ch );
			send_to_char( "\n\r", ch );
		    }
		}
	  
		affect_remove( ch, paf );
	    }
	}

	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */

        if (is_affected(ch, skill_lookup("plague")) && ch != NULL)
        {
            AFFECT_DATA *af, plague;
            CHAR_DATA *vch;
            int dam;

	    if (ch->in_room == NULL)
		continue;
            
	    act("$n writhes in agony as plague sores erupt from $s skin.",
		ch,NULL,NULL,TO_ROOM);
	    send_to_char("You writhe in agony from the plague.\n\r",ch);
            for ( af = ch->affected; af != NULL; af = af->next )
            {
            	if (af->type == skill_lookup("plague"))
                    break;
            }
        
            if (af == NULL)
            {
            	REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            	continue;
            }
        
            if (af->level == 1)
            	continue;
        
	    plague.where		= TO_AFFECTS;
            plague.type 		= skill_lookup("plague");
            plague.level 		= af->level - 1; 
            plague.duration 	= number_range(1,2 * plague.level);
            plague.location		= APPLY_STR;
            plague.modifier 	= -5;
            plague.bitvector 	= AFF_PLAGUE;
        
            for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
            {
                if (!saves_spell(plague.level - 2,vch,DAM_DISEASE) 
		&&  !IS_IMMORTAL(vch)
            	&&  !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(4) == 0)
            	{
            	    send_to_char("You feel hot and feverish.\n\r",vch);
            	    act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            	    affect_join(vch,&plague);
            	}
            }

	    dam = UMIN(ch->level,af->level/5+1);
	    ch->mana -= dam;
	    ch->move -= dam;
	    damage( ch, ch, dam, skill_lookup("plague"),DAM_DISEASE,FALSE);
        }
	else if ( IS_AFFECTED(ch, AFF_POISON) && ch != NULL
	     &&   !IS_AFFECTED(ch,AFF_SLOW))

	{
	    AFFECT_DATA *poison;

	    poison = affect_find(ch->affected,skill_lookup("poison"));

	    if (poison != NULL)
	    {
	        act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
	        send_to_char( "You shiver and suffer.\n\r", ch );
	        damage(ch,ch,poison->level/10 + 1,skill_lookup("poison"),
		    DAM_POISON,FALSE);
	    }
	}

	else if ( ch->position == POS_INCAP && number_range(0,1) == 0)
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE);
	}
	else if ( ch->position == POS_MORTAL )
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE);
	}
    }

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

	if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number)
	{
	    save_char_obj(ch);
	}

        if (ch == ch_quit)
	{
            do_function(ch, &do_quit, "" );
	}
    }

    return;
}

void update_timers()
{
    if ( copyovert-- > 0 )
    {
        displayt++;

        if ( displayt >= 3 && copyovert > 0 )
        {   
            info( NULL, 0, "{G[INFO]:{x {BThere are %d ticks left until a timed copyover!\n\r", copyovert );
            displayt = 0;
        }

        if ( copyovert == 0 )
        {
            do_function( copyover_person, do_copyover, "timer" );
            timed_copyover = FALSE;
        }
    }
       
    if ( reboott-- >= 0 )
    {
        rdisplay++;
        
        if ( rdisplay >= 3 && reboott > 0 )
        {
            info( NULL, 0, "{G[INFO]:{x {BThere are %d ticks left until a timed reboot.{x\n\r", reboott );        
            rdisplay = 0;
        }

        if ( reboott == 0 )
        {
            reboot();
            treboot = FALSE;
        }
    }

    if ( shutdownt-- >= 0 )
    {
        sdisplay++;
   
        if ( sdisplay >= 3 && shutdownt > 0 )
        {
            info( NULL, 0, "{G[INFO]:{x {BThere are %d ticks left until a timed shutdown.{x\n\r", shutdownt );
            sdisplay = 0;
        }

        if ( shutdownt == 0 )
        {
            shutdown();
            tshutdown = FALSE;
        }
    }
   
    return;
}

void update_bonuses()
{
    if ( global_exp-- >= -1 )
    {   
        display++;

        if ( display >= 3 && global_exp > 0 )
        {
            info( NULL, 0, "{G[INFO]:{x {BThere are %d ticks of double exp left.{x\n\r", global_exp );
            display = 0;
            return;
        }
        
        if (global_exp == 0)
        {
            info( NULL, 0, "{G[INFO]:{x {BDouble exp has run out!{x\n\r" ); 
            double_exp = FALSE;
            return;
        }
    }

    if ( global_qp-- >= 0 )
    {
        qpdisplay++;
        
        if ( qpdisplay >= 3 && global_qp > 0 )
        {
            info( NULL, 0, "{G[INFO]:{x {BThere are %d ticks of double questpoints left.{x\n\r", global_qp );        
            qpdisplay = 0;
            return;
        }

        if ( global_qp == 0 )
        {
            info( NULL, 0, "{G[INFO]:{x {BDouble questpoints has run out!{x\n\r" ); 
            double_qp = FALSE;
            return;
        }
    }

    if ( global_quad-- >= 0 )
    {
        quaddisplay++;
   
        if ( quaddisplay >= 3 && global_quad > 0 )
        {
            info( NULL, 0, "{G[INFO]:{x {BThere are %d ticks of quad damage left.{x\n\r", global_quad );
            quaddisplay = 0;
            return;
        }

        if ( global_quad == 0 )
        {
            info( NULL, 0, "{G[INFO]:{x {BQuad damage has run out!{x\n\r" ); 
            quad_damage = FALSE;
            return;
        }
    }
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;
    int rand = number_range( 1, 5000 );
    
    if ( max_unique-- > 0 )    
    {                   
        if ( rand <= 1 )
            create_unique();
    }

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;
	char *message;

	obj_next = obj->next;

	/* go through affects and decrement */
        for ( paf = obj->affected; paf != NULL; paf = paf_next )
        {
            paf_next    = paf->next;
            if ( paf->duration > 0 )
            {
                paf->duration--;
                if (number_range(0,4) == 0 && paf->level > 0)
                  paf->level--;  /* spell strength fades with time */
            }
            else if ( paf->duration < 0 )
                ;
            else
            {
                if ( paf_next == NULL
                ||   paf_next->type != paf->type
                ||   paf_next->duration > 0 )
                {
                    if ( paf->type > 0 && skill_table[paf->type].msg_obj )
                    {
			if (obj->carried_by != NULL)
			{
			    rch = obj->carried_by;
			    act(skill_table[paf->type].msg_obj,
				rch,obj,NULL,TO_CHAR);
			}
			if (obj->in_room != NULL 
			&& obj->in_room->people != NULL)
			{
			    rch = obj->in_room->people;
			    act(skill_table[paf->type].msg_obj,
				rch,obj,NULL,TO_ALL);
			}
                    }
                }

                affect_remove_obj( obj, paf );
            }
        }

       	/*
	 * Oprog triggers!
	 */
	if ( obj->in_room || (obj->carried_by && obj->carried_by->in_room))
	{
	    if ( HAS_TRIGGER_OBJ( obj, TRIG_DELAY )
	      && obj->oprog_delay > 0 )
	    {
	        if ( --obj->oprog_delay <= 0 )
		    p_percent_trigger( NULL, obj, NULL, NULL, NULL, NULL, TRIG_DELAY );
	    }
	    else if ( ((obj->in_room && !obj->in_room->area->empty)
	    	|| obj->carried_by ) && HAS_TRIGGER_OBJ( obj, TRIG_RANDOM ) )
		    p_percent_trigger( NULL, obj, NULL, NULL, NULL, NULL, TRIG_RANDOM );
	}

        if ( !obj )
            continue;

	if ( obj->timer <= 0 || --obj->timer > 0 )
	    continue;

        if (in_donation_room(obj))
            message = "$p shimmers and fades away.";
        else

	switch ( obj->item_type )
	{
	default:              message = "$p crumbles into dust.";  break;
	case ITEM_FOUNTAIN:   message = "$p dries up.";         break;
	case ITEM_CORPSE_NPC: message = "$p decays into dust."; break;
	case ITEM_CORPSE_PC:  message = "$p decays into dust."; break;
	case ITEM_FOOD:       message = "$p decomposes.";	break;
	case ITEM_POTION:     message = "$p has evaporated from disuse.";	
								break;
	case ITEM_PORTAL:     message = "$p fades out of existence."; break;
	case ITEM_CONTAINER: 
	    if (CAN_WEAR(obj,ITEM_WEAR_FLOAT))
		if (obj->contains)
		    message = 
		"$p flickers and vanishes, spilling its contents on the floor.";
		else
		    message = "$p flickers and vanishes.";
	    else
		message = "$p crumbles into dust.";
	    break;
	}

	if ( obj->carried_by != NULL )
	{
	    if (IS_NPC(obj->carried_by) 
	    &&  obj->carried_by->pIndexData->pShop != NULL)
		obj->carried_by->silver += obj->cost/5;
	    else
	    {
	    	act( message, obj->carried_by, obj, NULL, TO_CHAR );
		if ( obj->wear_loc == WEAR_FLOAT)
		    act(message,obj->carried_by,obj,NULL,TO_ROOM);
	    }
	}
	else if ( obj->in_room != NULL
	&&      ( rch = obj->in_room->people ) != NULL )
	{
	    	act( message, rch, obj, NULL, TO_ROOM );
	    	act( message, rch, obj, NULL, TO_CHAR );
	}

        if ((obj->item_type == ITEM_CORPSE_PC || obj->wear_loc == WEAR_FLOAT)
	&&  obj->contains)
	{   /* save the contents */
     	    OBJ_DATA *t_obj, *next_obj;

	    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
	    {
		next_obj = t_obj->next_content;
		obj_from_obj(t_obj);

		if (obj->in_obj) /* in another object */
		    obj_to_obj(t_obj,obj->in_obj);

		else if (obj->carried_by)  /* carried */
		    if (obj->wear_loc == WEAR_FLOAT)
			if (obj->carried_by->in_room == NULL)
			    extract_obj(t_obj);
			else
			{  t_obj->carried_by = obj->carried_by;
			   obj_to_room(t_obj,obj->carried_by->in_room);
			}
		    else
		    	obj_to_char(t_obj,obj->carried_by);

		else if (obj->in_room == NULL)  /* destroy it */
		    extract_obj(t_obj);

		else /* to a room */
                {
                     if (in_donation_room(obj))
                     {
                         if (t_obj->timer)
                            SET_BIT(t_obj->extra_flags,ITEM_HAD_TIMER);
                         else
                            t_obj->timer = number_range(100,200);
                     }
		     
		     obj_to_room(t_obj,obj->in_room);
                }  
	    }
	}

	extract_obj( obj );
    }

    return;
}


void room_update( AREA_DATA *pArea )
{
	ROOM_INDEX_DATA *room;
	int vnum;

  for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum ++ )
	{
	  if ( (room = get_room_index(vnum)) )
		room_aff_update(room);
	}

   return;
}


void room_aff_update( ROOM_INDEX_DATA *room )
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for ( paf = room->affected; paf != NULL; paf = paf_next )
	{
	    paf_next	= paf->next;
	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
            }
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
		/*	send_to_char( skill_table[paf->type].msg_off, ch);
			send_to_char( "\n\r", ch );  */
		    }
		}
	        
		affect_remove_room( room, paf );
	    }
	}

}

void hint_update( void )
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;

  for (ch=char_list;ch!=NULL;ch=ch_next)
  {
      ch_next = ch->next;

    if(!IS_NPC(ch) && !IS_SET(ch->comm, COMM_NOHINT) )
    {
      send_to_char("{B[{GHINT{x{B]:{x ",ch);
      switch(number_range(0,20))
      {
        default: send_to_char("You can turn the hint channel off by typing hint.", ch); break;
        case 0: send_to_char("When you die you can find your corpse in the morgue one north and down of midgaard recall.",ch); break;
        case 1: send_to_char("Type commands for a list of working commands. If you are unsure what they do type help <commandname>",ch); break;
        case 2: send_to_char("The donation room is located north of the healer.",ch); break;
        case 3: send_to_char("The bank is located south one east and up from recall.",ch); break;
        case 4: send_to_char("The restringer is located south two and up one from recall.",ch); break;
        case 5: send_to_char("The summoner is three south and west of recall.",ch); break;
        case 6: send_to_char("The questmaster is three south of recall.",ch); break;
        case 7: send_to_char("The bounty processor is four south and east of recall.",ch); break;
        case 8: send_to_char("The gambling hall is south two east one and south two of recall.",ch); break;
        case 9: send_to_char("The forger is four south and west of midgaard recall.",ch); break;
        case 10: send_to_char("Additional spells can be bought at the healer. Type heal to see a list.",ch); break;
        case 11: send_to_char("The blacksmith is located three south and up of midgaard recall. He can repair your objects.",ch); break;
        case 12: send_to_char("You can choose three difference fighting stances. Aggressive, Neutral, and Defensive.", ch ); break;
        case 13: send_to_char("You can multiclass once you reach Hero status level 150.", ch ); break;
        case 14: send_to_char("You can rent rooms at inns to gain an additional healing bonus. One is located east of Midgaard recall.", ch ); break;
        case 15: send_to_char("Objects have to be the same size as you to wear. They can be resized at any shopkeeper. Type resize and the name of the object.", ch ); break;      
        case 16: send_to_char("You can be awarded blessing points if you please your diety.\n\r", ch ); break;
        case 17: send_to_char("You can trade in your blessing points at the blesser for levels, pracs, trains, hp, mana, move etc.\n\r", ch ); break;
        case 18: send_to_char("You have the choice of leveling manually or automatically.\n\rUpon creation you will level automatically, if you wish to change this type autolevel.\n\rYou can then go to the healer and type level whenever you have gained enough experience.\n\r", ch ); break;
        case 19: send_to_char("Be on the lookout for unique objects, They load randomly so no telling where one could turn up.\n\r", ch ); break;
        case 20: send_to_char("New things are being added all the time. So read the boards to see what is being added.\n\r", ch ); break;
      }
      send_to_char("\n\r",ch);
    }
  }
    return;
}


void olcautosave( void )
{

   AREA_DATA *pArea;
   DESCRIPTOR_DATA *d;
   char buf[MAX_INPUT_LENGTH];
   
        save_cmd_table();
	save_area_list();
	save_help_new();
	save_email();
	save_skills_table();
	save_religion();

        for ( d = descriptor_list; d != NULL; d = d->next )
        {

           if ( d->editor)
                   send_to_char( "OLC Autosaving:\n\r", d->character );
   	}

	do_function(NULL, &do_save_guilds, "");
        sprintf( buf, "None.\n\r" );

        for( pArea = area_first; pArea; pArea = pArea->next ) 
        {

            /* Save changed areas. */

            if ( IS_SET(pArea->area_flags, AREA_CHANGED) )
            {

                save_area( pArea );

                sprintf( buf, "%24s - '%s'", pArea->name, pArea->file_name );

                for ( d = descriptor_list; d != NULL; d = d->next )
                {

                   if ( d->editor )
                   {
                        send_to_char( buf, d->character );
                        send_to_char( "\n\r", d->character );

                   }

                }   

                REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
            }

        }       

        if ( !str_cmp( buf, "None.\n\r" ) )
        {

           for ( d = descriptor_list; d != NULL; d = d->next )
           {
                if ( d->editor )
                        send_to_char( buf, d->character );
	   }     

        }

        return;
}


/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update( void )
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;

    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;
	if ( IS_NPC(wch)
	||   wch->level >= LEVEL_IMMORTAL
	||   wch->in_room == NULL 
 	||   wch->in_room->area->empty
        ||   IS_SET(wch->in_room->room_flags,ROOM_SAFE) )
	    continue;

	for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
	{
	    int count;

	    ch_next	= ch->next_in_room;

	    if ( !IS_NPC(ch)
	    ||   !IS_SET(ch->act, ACT_AGGRESSIVE)
	    ||   IS_AFFECTED(ch,AFF_CALM)
	    ||   ch->fighting != NULL
	    ||   IS_AFFECTED(ch, AFF_CHARM)
	    ||   !IS_AWAKE(ch)
	    ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	    ||   !can_see( ch, wch ) 
	    ||   number_bits(1) == 0)
		continue;

	    /*
	     * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */
	    count	= 0;
	    victim	= NULL;
	    for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	    {
		vch_next = vch->next_in_room;

		if ( !IS_NPC(vch)
		&&   vch->level < LEVEL_IMMORTAL
		&&   ch->level >= vch->level - 5 
		&&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
		&&   can_see( ch, vch ) )
		{
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if ( victim == NULL )
		continue;

            if ( ch->in_room->vnum == ch->aggression )
            {  
                multi_hit( ch, victim, TYPE_UNDEFINED ); 
            }

            if (RIDDEN(ch))
	    {
		if (!mount_success(RIDDEN(wch), ch, FALSE))
		{
		    send_to_char("Your mount escapes your control!\n\r", RIDDEN(ch));
		    multi_hit( ch, victim, TYPE_UNDEFINED );
		}
		else
		{
		    send_to_char("You manage to keep your mount under control.\n\r", RIDDEN(ch));
		}
	    }
	    else 
   	        multi_hit( ch, victim, TYPE_UNDEFINED );
	}
    }

    return;
}



/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( void )
{
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_violence;
    static  int     pulse_point;
    static  int	    pulse_music;
    static  int     pulse_auction;
    static  int     pulse_hint;
    static  int	    pulse_event;
    static  int     pulse_trivia;
    static  int     pulse_mudinfo;

    if ( --pulse_event        <= 0 )
    {
        pulse_event             = PULSE_EVENT;
        event_update();
    }

    if ( --pulse_area     <= 0 )
    {
	pulse_area	= PULSE_AREA;
	/* number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
	area_update	( );
        olcautosave     ( );
    }

    if ( --pulse_music	  <= 0 )
    {
	pulse_music	= PULSE_MUSIC;
	song_update();
        who_html_update();
    }

    if (--pulse_trivia <= 0)
    {
        pulse_trivia    = PULSE_TRIVIA;
        if ( trivia->running )
        {
            bugf("Trivia is running, calling update",0);
            trivia_update();
        }
    }     

    if ( --pulse_mudinfo  <= 0 )
    { 
      pulse_mudinfo = 30 * PULSE_PER_SECOND;
      update_mudinfo();
    } 

    if (--pulse_auction <= 0)
    {
        pulse_auction = PULSE_AUCTION;
        auction_update ();
    }

    if ( --pulse_hint <= 0)
    {
      pulse_hint = PULSE_HINT;
      hint_update ( );
    }

    if ( --pulse_mobile   <= 0 )
    {
	pulse_mobile	= PULSE_MOBILE;
	mobile_update	( );
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence	= PULSE_VIOLENCE;
	violence_update	( );
    }

    if ( --pulse_point    <= 0 )
    {
	wiznet("TICK!",NULL,NULL,WIZ_TICKS,0,0);
	pulse_point     = PULSE_TICK;
/* number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 ); */
	weather_update	( );
	char_update	( );
	obj_update	( );
        quest_update();
        gquest_update(); 
        update_bonuses();
        update_timers();
        war_update ( );
    }

    aggr_update( );
    tail_chain( );
    return;
}

void gain_bloodthirst( CHAR_DATA *ch, int value )
{
    int cond, dam;
    
    if ( !IS_VAMPIRE(ch ) )
	return;

    cond = ch->bloodthirst;
  
    if ( ch->bloodthirst >= 0 )
      ch->bloodthirst = URANGE( 0, cond + value, 48 );	
    
    switch ( ch->bloodthirst )
    {
	case 2: 	  
		send_to_char( "{rThe first pangs denoting your endless thirst for blood returning claw\n\rat your throat.{x\n\r", ch );
		break;
	case 1: 
		send_to_char( "{rBloodthirst begins to tear relentlessly at your very soul.{x\n\r", ch );
		break;
	default:
		break;
   }

   if ( ch->bloodthirst == 0 )
   {
	send_to_char( "{rThirst for blood rages through you, screaming to find a victim.{x\n\r", ch );
   	return;
   }

   if ( ch->bloodthirst < 0 )
   {
	send_to_char( "{rThirst for blood rages through you, screaming to find a victim.\n\rYou begin to ache with weakness!{x\n\r", ch );
	dam = number_fuzzy( number_range( 1, 10 ) );
      damage(ch,ch,dam,0,DAM_NONE,FALSE);
   	return;
   }
  
   return;
}

void event_update(void)
{
   EVENT_DATA *ev = NULL, *ev_next, *ev_last = ev;


   if(!events) return;

   for( ev = events; ev; ev = ev_next)
   {
        ev_next = ev->next;
        if(ev->delay-- <= 0)
        {
            switch(ev->action)
            {
                case ACTION_PRINT:
                  if( ( !ev->args[0]) || (! ev->argv[0] ) )
                  {
                      kill_event
                  }
                  send_to_char( ev->args[0], (CHAR_DATA *)ev->argv[0] );
                  break;

                case ACTION_FUNCTION:
                  if( !ev->args[0] )
                  {
                      kill_event
                  }
                  do_function((CHAR_DATA *)ev->argv[0], ev->do_fun, ev->args[0]);
                  break;

                case ACTION_WAIT:
                  if(!ev->argv[0])
                  {
                      kill_event
                  }
                  WAIT_STATE(ev->to,PULSE_PER_SECOND*ev->argi[0]);
                  break;

                case ACTION_ACT:
                  if(!ev->args[0] || !ev->argv[0])
                  {
                       kill_event
                  }
                  act(ev->args[0], (CHAR_DATA *)ev->argv[0], ev->argv[1],ev->argv[2],ev->argi[0]);
                  break;
            }

            if(!ev_next)
            {
                 if( (ev != events) && (ev_last != NULL) )
                 {
                      if( ev_last->next == ev )
                         ev_last->next=NULL;
                 }
                 else
                 {
                 if( ev == events)
                    events = NULL;
                 }
                 free_event( ev );
                 return;
            }

            if( ev_last != NULL)
                ev_last->next = ev_next;
            else
                events = ev_next;

            free_event( ev );
            continue;
        } /* end delay */

        ev_last = ev;

   } /* end loop */
return;
}

