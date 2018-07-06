/***************************************************************************
 *      This snippet was written by Donut & Buba for the Khrooon Mud.      *
 *            Original Coded by Yago Diaz <yago@cerberus.uab.es>           *
 *	  (C) November 1996             		   	   	   *
 *	  (C) Last Modification September 1997          		   *
 ***************************************************************************/

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
*       ROM 2.4 is copyright 1993-1996 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@efn.org)                                  *
*           Gabrielle Taylor                                               *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

/*
 * This is the file where all mob related services will be.
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "include.h"


CHAR_DATA *  find_forger	args( ( CHAR_DATA * ch ) );
CHAR_DATA *  find_restringer    args( ( CHAR_DATA * ch ) );      
CHAR_DATA *  find_innkeeper     args( ( CHAR_DATA * ch ) );
CHAR_DATA *  find_blesser       args( ( CHAR_DATA * ch ) );
CHAR_DATA *  find_sage          args( ( CHAR_DATA * ch ) );
CHAR_DATA *  find_smith         args( ( CHAR_DATA * ch ) );
CHAR_DATA *  find_summoner	args( ( CHAR_DATA * ch ) );

CHAR_DATA * find_forger ( CHAR_DATA *ch )
{
    CHAR_DATA * forger;

    for ( forger = ch->in_room->people; forger != NULL; forger = forger->next_in_room ) {
	if (!IS_NPC(forger))
	    continue;

        if ( IS_NPC(forger) && IS_SET(forger->act, ACT_FORGER) )
	    return forger;
    }

    if ( forger == NULL ) 
    {
	send_to_char("You can't do that here.\n\r", ch);
	return NULL;
    }

    return NULL;
}

CHAR_DATA * find_restringer ( CHAR_DATA *ch )
{
    CHAR_DATA * restringer;

    for ( restringer = ch->in_room->people; restringer != NULL; restringer = restringer->next_in_room )
    {
        if (!IS_NPC(restringer))
            continue;

        if (restringer->spec_fun == spec_lookup( "spec_restringer" ) )
            return restringer;
    }  

    if ( restringer == NULL || restringer->spec_fun != spec_lookup( "spec_restringer" ))
    {
        send_to_char("What? You can't do that here.\n\r", ch);
        return NULL;
    }

    if ( restringer->fighting != NULL )
    {
        send_to_char("Wait until the fighting stops.\n\r",ch );
        return NULL;
    }

    return NULL;
}

CHAR_DATA * find_innkeeper ( CHAR_DATA *ch )
{
    CHAR_DATA * innkeeper;

    for ( innkeeper = ch->in_room->people; innkeeper != NULL; innkeeper = innkeeper->next_in_room )
    {
        if (!IS_NPC(innkeeper))
            continue;

        if (innkeeper->spec_fun == spec_lookup( "spec_innkeeper" ) )
            return innkeeper;
    }  

    if ( innkeeper == NULL || innkeeper->spec_fun != spec_lookup( "spec_innkeeper" ))
    {
        send_to_char("What? You can't do that here.\n\r", ch);
        return NULL;
    }

    if ( innkeeper->fighting != NULL )
    {
        send_to_char("Wait until the fighting stops.\n\r",ch );
        return NULL;
    }

    return NULL;
}                        

CHAR_DATA * find_blacksmith( CHAR_DATA *ch )
{
    CHAR_DATA * blacksmith;

    for ( blacksmith = ch->in_room->people; blacksmith != NULL; blacksmith = blacksmith->next_in_room )
    {
        if (!IS_NPC(blacksmith))
            continue;

        if (blacksmith->spec_fun == spec_lookup( "spec_blacksmith" ) )
            return blacksmith;
    }  

    if ( blacksmith == NULL || blacksmith->spec_fun != spec_lookup( "spec_blacksmith" ))
    {
        send_to_char("What? You can't do that here.\n\r", ch);
        return NULL;
    }

    if ( blacksmith->fighting != NULL )
    {
        send_to_char("Wait until the fighting stops.\n\r",ch );
        return NULL;
    }

    return NULL;
}                        

CHAR_DATA * find_blesser ( CHAR_DATA *ch )
{
    CHAR_DATA * blesser;

    for ( blesser = ch->in_room->people; blesser != NULL; blesser = blesser->next_in_room )
    {
        if (!IS_NPC(blesser))
            continue;

        if (blesser->spec_fun == spec_lookup( "spec_blesser" ) )
            return blesser;
    }  

    if ( blesser == NULL || blesser->spec_fun != spec_lookup( "spec_blesser" ))
    {
        send_to_char("What? You can't do that here.\n\r", ch);
        return NULL;
    }

    if ( blesser->fighting != NULL )
    {
        send_to_char("Wait until the fighting stops.\n\r",ch );
        return NULL;
    }

    return NULL;
}        

CHAR_DATA * find_sage ( CHAR_DATA *ch )
{
    CHAR_DATA * sage;

    for ( sage = ch->in_room->people; sage != NULL; sage = sage->next_in_room )
    {
        if (!IS_NPC(sage))
            continue;

        if (sage->spec_fun == spec_lookup( "spec_sage" ) )
            return sage;
    }  

    if ( sage == NULL || sage->spec_fun != spec_lookup( "spec_sage" ))
    {
        send_to_char("What? You can't do that here.\n\r", ch);
        return NULL;
    }

    if ( sage->fighting != NULL )
    {
        send_to_char("Wait until the fighting stops.\n\r",ch );
        return NULL;
    }

    return NULL;
}                   

CHAR_DATA * find_smith( CHAR_DATA *ch )
{
    CHAR_DATA * smith;

    for ( smith = ch->in_room->people; smith != NULL; smith = smith->next_in_room )
    {
        if (!IS_NPC(smith))
            continue;

        if (smith->spec_fun == spec_lookup( "spec_smith" ) )
            return smith;
    }  

    if ( smith == NULL || smith->spec_fun != spec_lookup( "spec_smith" ))
    {
        send_to_char("What? You can't do that here.\n\r", ch);
        return NULL;
    }

    if ( smith->fighting != NULL )
    {
        send_to_char("Wait until the fighting stops.\n\r",ch );
        return NULL;
    }

    return NULL;
}        

CHAR_DATA *find_summoner ( CHAR_DATA *ch )
{
   CHAR_DATA * summoner;

   for ( summoner = ch->in_room->people; summoner != NULL; summoner = summoner->next_in_room )
   {
	if (!IS_NPC(summoner))
	    continue;

	if (summoner->spec_fun == spec_lookup( "spec_summoner" ))
	    return summoner;
   }

   if (summoner == NULL || summoner->spec_fun != spec_lookup( "spec_summoner" ))
   {
	send_to_char("You can't do that here.\n\r",ch);
	return NULL;
   }

   if ( summoner->fighting != NULL)
   {
	send_to_char("Wait until the fighting stops.\n\r",ch);
	return NULL;
   }

   return NULL;
}

void do_forge( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *	obj;
    CHAR_DATA * forger;
    char	weapon[MAX_INPUT_LENGTH];
    char *	words;
    int		cost;

    forger = find_forger(ch);
				    
    if (!forger)
	return;

    if (argument[0] == '\0') {
	act("$N says 'You have to tell me the type of forge do you want on your weapon.'", ch, NULL, forger, TO_CHAR);
	send_to_char("You have to tell me the weapon you want to forge.\n\r", ch);
	send_to_char("My possible forges are:\n\r",ch);
	send_to_char("  flame     : flaming weapon           1250 quest points\n\r",ch);
	send_to_char("  drain     : vampiric weapon          1750 quest points\n\r",ch);
	send_to_char("  shocking  : electric weapon          1750 quest points\n\r",ch);
	send_to_char("  frost     : frost weapon             2250 quest points\n\r",ch);
	send_to_char("  sharp     : sharp weapon             2500 quest points\n\r",ch);
	send_to_char("  vorpal    : vorpal weapon            1750 quest points\n\r",ch);
	send_to_char("\n\rType 'forge <weapon> <type>' to forge the weapon.\n\r",ch);
	return;
    }
    
    argument = one_argument ( argument, weapon);

    if ( ( obj = get_obj_carry( ch, weapon, ch ) ) == NULL ) {
        act ("$N says 'You're not carrying that.'", ch, NULL, forger, TO_CHAR);
        return;
    }
    
    if(obj->item_type != ITEM_WEAPON) {
        act ("$N says 'This is not a weapon!.'", ch, NULL, forger, TO_CHAR);
        return;
    }
  
    if ( argument[0] == '\0' ) {
        act ("$N says 'Pardon? type 'forge' to see the list of modifications.'", ch, NULL, forger, TO_CHAR);
	return;
    }    	  
   

    if (!str_prefix(argument,"flame")) 
    {
	if (IS_WEAPON_STAT( obj, WEAPON_FLAMING) ) {
	    act("$N says '$p is already flaming.'", ch, obj, forger, TO_CHAR);
	    return;
	}

	words = "yrawz braoculo";
	cost  = 1250;

	if ( cost > ch->pcdata->questpoints ) 
        {
	    act ("$N says 'You do not have enough quest points for my services'", ch, NULL, forger, TO_CHAR);
	    return;
	}

    	act("$n utters the words '$T'.", forger, NULL, words, TO_ROOM);

	spell_flame_blade( skill_lookup("flame blade"), 50, forger, obj, 0 );

	if ( IS_WEAPON_STAT ( obj, WEAPON_FLAMING) ) {
	    act ("$N gives $p to $n.", ch, obj, forger, TO_ROOM);
	    act ("$N says 'Take care with $p, now is a lot powerful.'", ch, obj, forger, TO_CHAR);
	    ch->pcdata->questpoints -= cost;
	    forger->gold  += cost;
	}
	else
	    act ("$N says 'I'm sorry I can't help you.'", ch, NULL, forger, TO_CHAR); 
	return;
    }
    
    if (!str_prefix(argument,"drain")) 
    {
	if (IS_WEAPON_STAT( obj, WEAPON_VAMPIRIC) ) {
	    act("$N says '$p is already vampiric.'", ch, obj, forger, TO_CHAR);
	    return;
	}

	words = "egruui braoculo";
	cost  = 1750;

	if ( cost > ch->pcdata->questpoints ) 
        {
	    act ("$N says 'You do not have enough gold for my services'", ch, NULL, forger, TO_CHAR);
	    return;
	}
    
    	act("$n utters the words '$T'.", forger, NULL, words, TO_ROOM);

	spell_drain_blade( skill_lookup("drain blade"), 50, forger, obj, 0 );

	if ( IS_WEAPON_STAT ( obj, WEAPON_VAMPIRIC) ) 
        {
	    act ("$N gives $p to $n.", ch, obj, forger, TO_ROOM);
	    act ("$N says 'Take care with $p, now is a lot powerful.'", ch, obj, forger, TO_CHAR);
	    ch->pcdata->questpoints -= cost;
	    forger->gold  += cost;
	}
	else
	    act ("$N says 'I'm sorry I can't help you.'", ch, NULL, forger, TO_CHAR); 
	return;
    }

    if (!str_prefix(argument,"shocking")) 
    {
	if (IS_WEAPON_STAT( obj, WEAPON_SHOCKING) ) 
        {
	    act("$N says '$p is already electrical.'", ch, obj, forger, TO_CHAR);
	    return;
	}

	words = "gpaqtuio braoculo";
	cost  = 1750;

	if ( cost > ch->pcdata->questpoints ) 
        {
	    act ("$N says 'You do not have enough quest points for my services.'", ch, NULL, forger, TO_CHAR);
	    return;
	}

    	act("$n utters the words '$T'.", forger, NULL, words, TO_ROOM);

	spell_shocking_blade( skill_lookup("shocking blade"), 50, forger, obj, 0 );

	if ( IS_WEAPON_STAT ( obj, WEAPON_SHOCKING) ) 
        {
	    act ("$N gives $p to $n.", ch, obj, forger, TO_ROOM);
	    act ("$N says 'Take care with $p, now is a lot powerful.'", ch, obj, forger, TO_CHAR);
	    ch->pcdata->questpoints -= cost;
	    forger->gold  += cost;
	}
	else
	    act ("$N says 'I'm sorry I can't help you.'", ch, NULL, forger, TO_CHAR); 
	return;
    }

    if (!str_prefix(argument,"frost")) 
    {
	if (IS_WEAPON_STAT( obj, WEAPON_FROST) ) 
        {
	    act("$N says '$p is already frost.'", ch, obj, forger, TO_CHAR);
	    return;
	}

	words = "yfagh braoculo";
	cost  = 2250;

	if ( cost > ch->pcdata->questpoints ) 
        {
	    act ("$N says 'You do not have enough quest points for my services'", ch, NULL, forger, TO_CHAR);
	    return;
	}

    	act("$n utters the words '$T'.", forger, NULL, words, TO_ROOM);

	spell_frost_blade( skill_lookup("frost blade"), 50, forger, obj, 0 );
	
	if ( IS_WEAPON_STAT ( obj, WEAPON_FROST) ) 
        {
	    act ("$N gives $p to $n.", ch, obj, forger, TO_ROOM);
	    act ("$N says 'Take care with $p, now is a lot powerful.'", ch, obj, forger, TO_CHAR);
	    ch->pcdata->questpoints -= cost;
	    forger->gold  += cost;
	}
	else
	    act ("$N says 'I'm sorry I can't help you.'", ch, NULL, forger, TO_CHAR); 
	return;
    }

    if (!str_prefix(argument,"sharp")) 
    {
	if (IS_WEAPON_STAT( obj, WEAPON_SHARP) ) 
        {
	    act("$N says '$p is already sharp.'", ch, obj, forger, TO_CHAR);
	    return;
	}

	words = "gpabras braoculo";
	cost  = 2500;

	if ( cost > ch->pcdata->questpoints ) 
        {
	    act ("$N says 'You do not have enough quest points for my services'", ch, NULL, forger, TO_CHAR);
	    return;
	}

    	act("$n utters the words '$T'.", forger, NULL, words, TO_ROOM);

	spell_sharp_blade( skill_lookup("sharp blade"), 50, forger, obj, 0 );
	
	if ( IS_WEAPON_STAT ( obj, WEAPON_SHARP) ) 
        {
	    act ("$N gives $p to $n.", ch, obj, forger, TO_ROOM);
	    act ("$N says 'Take care with $p, now is a lot powerful.'", ch, obj, forger, TO_CHAR);
	    ch->pcdata->questpoints -= cost;
	    forger->gold  += cost;
	}
	else
	    act ("$N says 'I'm sorry I can't help you.'", ch, NULL, forger, TO_CHAR); 
	return;
    }

    if (!str_prefix(argument,"vorpal")) 
    {
	if (IS_WEAPON_STAT( obj, WEAPON_VORPAL) ) 
        {
	    act("$N says '$p is already vorpal.'", ch, obj, forger, TO_CHAR);
	    return;
	}

	words = "zafsar braoculo";
	cost  = 1750;

	if ( cost > ch->pcdata->questpoints ) 
        {
	    act ("$N says 'You do not have enough quest points for my services'", ch, NULL, forger, TO_CHAR);
	    return;
	}

    	act("$n utters the words '$T'.", forger, NULL, words, TO_ROOM);

	spell_vorpal_blade( skill_lookup("vorpal blade"), 50, forger, obj, 0 );
	
	if ( IS_WEAPON_STAT ( obj, WEAPON_VORPAL) ) 
        {
	    act ("$N gives $p to $n.", ch, obj, forger, TO_ROOM);
	    act ("$N says 'Take care with $p, now is a lot powerful.'", ch, obj, forger, TO_CHAR);
	    ch->pcdata->questpoints -= cost;
	    forger->gold  += cost;
	}
	else
	    act ("$N says 'I'm sorry I can't help you.'", ch, NULL, forger, TO_CHAR); 
	return;
    }

    act ("$N says 'Pardon? Type 'forge' to see the list of modifications.'", ch, NULL, forger, TO_CHAR);
    return;								 
}


void spell_drain_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA * obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
   
    if(obj->item_type != ITEM_WEAPON) 
    {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)) 
    {
	send_to_char("This weapon is already vampiric.\n\r", ch);
	return;
    }

    if(IS_OBJ_STAT(obj,ITEM_BLESS)) 
    {
	send_to_char("This weapon is too blessed.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_VAMPIRIC;
    affect_to_obj(obj, &af);

    act("$p carried by $n turns dark and vampiric.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you turns dark and vampiric.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_shocking_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA * obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) 
    {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_SHOCKING)) 
    {
	send_to_char("This weapon is already electrical.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_SHOCKING;
    affect_to_obj(obj, &af);

    act("$p carried by $n sparks with electricity.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you sparks with electricity.", ch, obj, NULL, TO_CHAR);
    return;
}


void spell_flame_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) 
    {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_FLAMING)) 
    {
	send_to_char("This weapon is already flaming.\n\r", ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_FROST)) 
    {
	send_to_char("This weapon is too frost to handle this magic.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_FLAMING;
    affect_to_obj(obj, &af);

    act("$p carried by $n gets a fiery aura.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you gets a fiery aura.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_frost_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) 
    {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_FROST)) 
    {
	send_to_char("This weapon is already frost.\n\r", ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_FLAMING)) 
    {
	send_to_char("This weapon is too hot to handle this magic.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_FROST;
    affect_to_obj(obj, &af);

    act("$p carried by $n grows wickedly cold.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you grows wickedly cold.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_sharp_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) 
    {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_SHARP)) 
    {
	send_to_char("This weapon is already sharp.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_SHARP;
    affect_to_obj(obj, &af);

    act("$p carried by $n looks newly honed.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you looks newly honed.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_vorpal_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) 
    {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return ;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_VORPAL)) 
    {
	send_to_char("This weapon is already vorpal.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = level/2;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_VORPAL;
    affect_to_obj(obj, &af);

    act("$p carried by $n gleams witch magical strengh.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you gleams witch magical strengh.", ch, obj, NULL, TO_CHAR);
    return;
}


void do_restring( CHAR_DATA *ch, char *argument )
{
    char arg  [MIL];
    char arg1 [MIL];
    char arg2 [MIL];
    CHAR_DATA *restringer;
    OBJ_DATA *obj;
    int cost = 1500;

    restringer = find_restringer(ch);        

    smash_tilde( argument );

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );

    strcpy( arg2, argument );
                              
    if ( arg[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' )
    {
         printf_to_char( ch, "The cost for a restring is %d gold.\n\r", cost );
         send_to_char( "Syntax: Restring <obj> <field>\n\r", ch );
         send_to_char( "Valid fields are: short name and long\n\r", ch );
         return;
    }   

    if ( ( obj = (get_obj_carry(ch, arg, restringer) ) ) == NULL )
    {
         send_to_char( "You don't have that item.\n\r", ch );
         return;
    }

    if ( cost > ch->gold )
    {
        send_to_char( "You don't have enough gold.\n\r", ch );
        return;
    }       

    if ( !str_prefix( arg1, "name" ) )
    {
        free_string( obj->name );
        obj->name = str_dup( arg2 );
    }   

    else if ( !str_prefix( arg1, "short" ) )
    {
        free_string( obj->short_descr );
        obj->short_descr = str_dup(arg2);
    }

    else if ( !str_prefix( arg1, "long" ) )
    {
        free_string( obj->description );
        obj->description = str_dup( arg2 );
    }
        
    else
    {
        send_to_char("Syntax: valid fields are: name short and long\n\r",ch);
        return;
    }

    ch->gold -= cost;
    restringer->gold += cost;
    act( "$n takes your $p works on it and hands it back to you.", restringer, obj,NULL,TO_VICT );
    act( "$N takes $n's item works on it and then hands it back to them.", restringer, NULL, ch, TO_ROOM);
}        

/* do_bounty        */
/* Global variables */
BOUNTY_DATA	*bounty_list;

/* Command functions needed */
DECLARE_DO_FUN(do_say	);

/* Function definitions */
int     clan_lookup     args( (const char *name) );

/* And some local defines */
#define BOUNTY_PLACE_CHARGE	3
#define BOUNTY_REMOVE_CHARGE	100
#define BOUNTY_ADD_CHARGE	5
#define MIN_BOUNTY		1000

void save_bounties()
{
  FILE *fp;
  BOUNTY_DATA *bounty;
  bool found = FALSE;

  fp = file_open( BOUNTY_FILE, "w" );

  for( bounty = bounty_list; bounty != NULL; bounty = bounty->next)
  {
    found = TRUE;
    fprintf(fp,"%-12s %d\n",bounty->name,bounty->amount);
  }

  file_close(fp);

  if(!found)
    unlink(BOUNTY_FILE);
}

void load_bounties()
{
  FILE *fp;
  BOUNTY_DATA *blist;

  if (!file_exists(BOUNTY_FILE))
	return;

  fp = file_open( BOUNTY_FILE, "r" );

  blist = NULL;
  for( ; ; )
  {
    BOUNTY_DATA *bounty;
    if ( feof(fp) )
    {
      file_close(fp);
      return;
    }

    bounty = new_bounty();

    bounty->name = str_dup(fread_word(fp));
    bounty->amount = fread_number(fp);
    fread_to_eol(fp);

    if(bounty_list == NULL)
	bounty_list = bounty;
    else
	blist->next = bounty;
    blist = bounty;
  }
}

bool is_bountied(CHAR_DATA *ch)
{
  BOUNTY_DATA *bounty;
  bool found = FALSE;

  if(IS_NPC(ch))
    return FALSE;

  for(bounty = bounty_list; bounty != NULL; bounty = bounty->next)
  {
    if( !str_cmp(ch->name,bounty->name) )
      found = TRUE;
  }

  return found;
}

void do_bounty(CHAR_DATA *ch, char *argument)
{
  FILE *fp;
  CHAR_DATA *vch;
  BOUNTY_DATA *bounty;
  BOUNTY_DATA *prev=NULL;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char strsave[MAX_STRING_LENGTH];
  const char *word;
  bool found = FALSE;
  bool clanned = FALSE;
  int cash, amount, surcharge;

  if( IS_NPC(ch) )
    return;

  /* Check for the bounty collector */
  for(vch = ch->in_room->people; vch ; vch = vch->next_in_room)
  {
    if(IS_NPC(vch) && IS_SET(vch->act, ACT_BOUNTY))
      break;
  }

  if(vch == NULL)
  {
    send_to_char("You cannot do that here.\n\r",ch);
    return;
  }

  /* Only clanned chars/pkillers can place bounties. */
 /* if( !is_clan(ch) )
  {
    do_say(vch,"You must be in a clan to place or collect a bounty.");
    return;
  }*/

  argument = one_argument(argument, arg);
  one_argument(argument, arg2);

  if(arg[0] == '\0')
  {
    do_say(vch,"These are the bounties I will honor:");
    send_to_char("  Name              Bounty\n\r",ch);
    send_to_char("  ============      ===============\n\r",ch);
    for(bounty = bounty_list;bounty != NULL;bounty=bounty->next)
    {
      sprintf(buf,"  %-12s      %d\n\r",bounty->name,bounty->amount);
      send_to_char(buf,ch);
    }

    return;
  }

  if( arg2[0] == '\0' 
  ||  (str_cmp(arg,"remove") && str_cmp(arg,"collect") && !is_number(arg2)) )
  {
    send_to_char("Syntax: bounty\n\r",ch);
    send_to_char("        bounty remove \n\r",ch);
    send_to_char("        bounty collect \n\r",ch);
    send_to_char("        bounty <name> <amount> \n\r",ch);
    return;
  }

  /* For collecting bounties. */
  if( !str_cmp(arg,"collect") )
  {
    OBJ_DATA *obj;

    for(bounty = bounty_list; bounty != NULL; prev=bounty, bounty=bounty->next)
    {
      if( !str_cmp(arg2,bounty->name) )
	break;
    }

    if( bounty == NULL )
    {
      do_say(vch,"That individual is not in the books.");
      return;
    }

    for(obj = ch->carrying; obj != NULL; obj = obj->next)
    {
      if( is_name(arg2,obj->name)
      &&  (obj->pIndexData->vnum == OBJ_VNUM_SEVERED_HEAD) )
	break;
    }

    if( obj == NULL )
    {
      do_say(vch,"I'm afraid I'll need proof of the death.");
      return;
    }

    extract_obj(obj);
    do_say(vch,"Very well, here is your payment.");
    ch->pcdata->gold_bank += bounty->amount;


    /*
     * OK, we've got a valid bounty and they've
     * been paid.  Time to remove the bounty.
     */
    if(prev == NULL)
      bounty_list = bounty_list->next;
    else
      prev->next = bounty->next;
    free_bounty(bounty);
    save_bounties();
    return;
  }

  /* For buying off bounties. */
  if( !str_cmp(arg,"remove") )
  {
    for(bounty = bounty_list; bounty != NULL; prev=bounty, bounty=bounty->next)
    {
      if( !str_cmp(arg2,bounty->name) )
	break;
    }

    if( bounty == NULL )
    {
      do_say(vch,"That individual is not in the books.");
      return;
    }

    surcharge = BOUNTY_REMOVE_CHARGE;
    amount = bounty->amount+(bounty->amount*surcharge/100);
    cash = ch->gold + (ch->silver/100) + ch->pcdata->gold_bank + (ch->pcdata->silver_bank/100);
    sprintf( buf, "The amount for removal is %d Gold.\n\r", amount);
    send_to_char( buf, ch);                                             

    if(cash < amount)
    {
      do_say(vch,"You cannot afford to remove that bounty.");
      return;
    }

    if(ch->pcdata->silver_bank < (amount*100))
    {
      amount -= (ch->pcdata->silver_bank/100);
      ch->pcdata->silver_bank %= 100;
    }
    else
    {
      ch->pcdata->silver_bank -= amount*100;
      amount = 0;
    }

    if(ch->pcdata->gold_bank < amount)
    {
      amount -= ch->pcdata->gold_bank;
      ch->pcdata->gold_bank = 0;
    }
    else
    {
      ch->pcdata->gold_bank -= amount;
      amount = 0;
    }

    if(ch->silver < (amount*100))
    {
      amount -= (ch->silver/100);
      ch->silver %= 100;
    }
    else
    {
      ch->silver -= amount*100;
      amount = 0;
    }

    if(ch->gold < amount)
    {
      amount -= ch->gold;
      ch->gold = 0;
    }
    else
    {
      ch->gold -= amount;
      amount = 0;
    }

    /*
     * OK, we've got a valid bounty and they can
     * pay.  Time to remove the bounty.
     */
    if(prev == NULL)
      bounty_list = bounty_list->next;
    else
      prev->next = bounty->next;

    printf_to_char( ch, "The Bounty Processor says '%'s has been removed from the books.\n\r", bounty->name );
    info( ch, 0, "{G[INFO]:{x Someone has had their bounty removed.\n\r" );
    free_bounty(bounty);
    save_bounties();
    return;
  }

  /*
   * This code allows you to bounty those
   * offline as well as those not visible.
   */

  sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(arg));
  if (!file_exists(strsave))
  {
    do_say(vch,"I am afraid there is no such person.");
    return;
  }

  fp = file_open( strsave, "r" );

  strcpy(arg,capitalize(arg));

  while( !found )
  {
    char *clanName;

    word = feof(fp) ? "End" : fread_word( fp );

    if( !str_cmp(word, "Clan") )
    {
      found = TRUE;
      clanName = fread_string(fp);
      clanned = clan_lookup(clanName) ? TRUE : FALSE;
      free_string(clanName);
    }
    else if( !str_cmp(word, "End") )
    {
      found = TRUE;
    }
    else
    {
      fread_to_eol( fp );
    }
  }

  file_close( fp );

  /* Only clanned chars can be bountied */
/*  if( !clanned )
  {
    do_say(vch,"You cannot place bounties on non-clanned characters.");
    return;
  }*/

  /* Minimum bounty, change as desired */
  if( (amount = atoi(arg2)) < MIN_BOUNTY)
  {
    sprintf(buf,"I do not accept bounties under %d gold.",MIN_BOUNTY);
    do_say(vch,buf);
    return;
  }

  /*
   * Char's cash total in gold, bounties
   * are always set in gold. Notice the
   */
  cash = ch->gold + (ch->silver/100) + ch->pcdata->gold_bank + (ch->pcdata->silver_bank/100);

  /*
   * Does the bounty already exist?
   * Let's find out.
   */
  for(bounty = bounty_list;bounty != NULL;bounty = bounty->next)
  {
    if( !str_cmp(bounty->name,arg) )
      break;
  }

  if(bounty == NULL)
  {
    surcharge = BOUNTY_PLACE_CHARGE;
    if(cash < (amount+(amount*surcharge/100)))
    {
      do_say(vch,"You cannot afford to place that bounty.");
      return;
    }
    bounty = new_bounty();
    bounty->name = str_dup(arg);
    bounty->amount = amount;
    bounty->next = bounty_list;
    bounty_list = bounty;
    amount += amount*surcharge/100;
  }
  else
  {
    sprintf(buf,"There is a %d%% surcharge to add to an existing bounty.",BOUNTY_ADD_CHARGE);
    do_say(vch,buf);
    surcharge = BOUNTY_PLACE_CHARGE + BOUNTY_ADD_CHARGE;
    amount += amount*surcharge/100;
    if(cash < amount)
    {
      do_say(vch,"You cannot afford to place that bounty.");
      return;
    }
    bounty->amount += amount;
  }

  save_bounties();

  sprintf( buf, "The cost of the bounty is %d Gold.\n\r", amount);
  send_to_char( buf, ch);

  /*
   * Now make them pay, start with bank
   * then go to cash on hand.  Also use
   * up that annoying silver first.
   */
  if(ch->pcdata->silver_bank < (amount*100))
  {
    amount -= (ch->pcdata->silver_bank/100);
    ch->pcdata->silver_bank %= 100;
  }
  else
  {
    ch->pcdata->silver_bank -= amount*100;
    amount = 0;
  }

  if(ch->pcdata->gold_bank < amount)
  {
    amount -= ch->pcdata->gold_bank;
    ch->pcdata->silver_bank = 0;
  }
  else
  {
    ch->pcdata->gold_bank -= amount;
    amount = 0;
  }

  if(ch->silver < (amount*100))
  {
    amount -= (ch->silver/100);
    ch->silver %= 100;
  }
  else
  {
    ch->silver -= amount*100;
    amount = 0;
  }

  if(ch->gold < amount)
  {
    amount -= ch->gold;
    ch->gold = 0;
  }
  else
  {
    ch->gold -= amount;
    amount = 0;
  }

  printf_to_char( ch, "The Bounty Processor says '%'s has been recorded in the books.\n\r", bounty->name );
  info( ch, 0, "{G[INFO]:{x {BSomeone has just had a bounty placed on their head!{x\n\r" );
 
}

/* Synon */
void do_exchange( CHAR_DATA *ch, char * argument )
{
  char arg[MSL];
  CHAR_DATA *innkeeper;
  OBJ_DATA *obj;
  int qreward  = 50;
  int preward  = 1;
  int treward  = 1;
  int ereward  = 50;
  int etreward = 2;

  innkeeper = find_innkeeper( ch );
  
  if (!innkeeper)
      return;   

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
      printf_to_char(ch, "{RTypes{x :  Quest:            Practice:       Train:       Exp:      Exp: \n\r" );
      printf_to_char(ch, "===========================================================================\n\r" );
      printf_to_char(ch, "{BValues{x:  Questpoints: %-2d   Practices: %-2d   Trains: %-2d   Exp: %-2d   Trains: %-2d\n\r\n\r", qreward, preward, treward, ereward, etreward ); 
      send_to_char( "Syntax: exchange <token>\n\r", ch );
      return;
  }     

  if ( IS_NPC( ch ) )
  {
      send_to_char( "You can't do that.\n\r", ch );
      return;
  }
     
  else
  {
      if ( !str_cmp( arg, "quest" )  )
      {

          if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
          {
            send_to_char( "You don't have anything like that in your inventory.\n\r", ch );
            return;
          }
           
          printf_to_char( ch, "You exchange a quest token for %d questpoints.\n\r", qreward );  
          obj_from_char( obj );
          extract_obj( obj );
          ch->pcdata->questpoints += qreward;
          printf_to_char( ch, "You now have %d questpoints.\n\r", ch->pcdata->questpoints );
          return;
      }

      else if ( !str_cmp( arg, "practice" ) )
      {

          if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
          {
            send_to_char( "You don't have anything like that in your inventory.\n\r", ch );
            return;
          }

          printf_to_char( ch, "You exchange a practice token for %d practice.\n\r", preward );
          obj_from_char( obj );
          extract_obj( obj);
          ch->practice += preward;
          printf_to_char( ch, "You now have %d practices.\n\r", ch->practice );
          return;
      }

      else if ( !str_cmp( arg, "train" ) )
      {

          if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
          {
            send_to_char( "You don't have anything like that in your inventory.\n\r", ch );
            return;
          }

          printf_to_char( ch, "You exchange a train token for %d train.\n\r", treward );
          obj_from_char( obj );
          extract_obj( obj);
          ch->train += treward;
          printf_to_char( ch, "You now have %d trains.\n\r", ch->train );
          return;
      }

      else if ( !str_cmp( arg, "exp" ) )
      {

          if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
          {
            send_to_char( "You don't have anything like that in your inventory.\n\r", ch );
            return;
          }

          if ( ch->level < LEVEL_HERO )
          {

              printf_to_char( ch, "You exchange an exp token for %d exp.\n\r", ereward );
              obj_from_char( obj );
              extract_obj( obj);
              printf_to_char( ch, "You need %d exp to level.\n\r", (ch->level + 1) * 
              exp_per_level(ch,ch->pcdata->points) - ch->exp); 
              gain_exp( ch, ereward );
              advance_level( ch, TRUE );
              return;          
          }

          else
          {
              printf_to_char( ch, "You exchange an exp token for %d trains.\n\r", etreward );
              obj_from_char( obj );
              extract_obj( obj );
              ch->train += etreward;
              printf_to_char( ch, "You now have %d trains.\n\r", ch->train );    
              return;
          }
      }
   
      else          
      {
          send_to_char( "Syntax: exchange <token>. Valid tokens are.\n\rexp\n\rtrain\n\rquest\n\rpractice\n\r", ch );
          return;
      }
  }
} 

void do_petcall(CHAR_DATA *ch, char *argument)
{
    int cost = 200;

    if( IS_NPC(ch) )
    {	
       send_to_char("Mobs dont need pets!\n\r",ch);
       return;
    }

    if ( IS_IMMORTAL( ch ) )
    {
        send_to_char( "Your an immortal just transfer your pet!\n\r", ch );
        return;
    }

    if( IN_MINE(ch) )
    {	
       send_to_char("Your pet isn't able to hear you in the mine.\n\r",ch);
       return;
    }

    if ( ch->pet == NULL )
    {
        send_to_char ( "You don't have a pet!\n\r", ch );
        return;
    }

    char_from_room( ch->pet );
    char_to_room( ch->pet, ch->in_room );
    act( "$n whistles loudly, and $N appears in the room.", ch, NULL, ch->pet, TO_ROOM );
    act( "You whistle loudly, and $N appears in the room.", ch, NULL, ch->pet, TO_CHAR );
    if ( ch->move > 200 )
    ch->move -= cost;
    else
      ch->move *= .25;
    printf_to_char( ch, "Calling your pet to your side exhaust you and drains %d movement.\n\r", cost );
}
        
void do_repair(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *blacksmith;
    char arg[MIL];
    char arg1[MIL];
    char buf[MIL];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found = FALSE;
    int cost = 0;
    int count;

    blacksmith = find_blacksmith( ch );

    if (!blacksmith)
        return;

    argument = one_argument(argument,arg);
    argument = one_argument( argument, arg1 );

    if (arg[0] == '\0')
    {
	do_say(blacksmith,"I will repair your weapon for a price.\n\r");
	send_to_char("Type repair <weapon> estimate to be assessed for damage.\n\r",ch);
        printf_to_char( ch, "You can repair all <estimate> to be assessed for repairs on all eq. \n\r" );
        printf_to_char( ch, "To repair your eq type: repair all <repair>\n\r" );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        if ( arg1[0] == '\0' )
        {
            printf_to_char( ch, "You can repair all <estimate> to be assessed for repairs on all eq. \n\r" );
            printf_to_char( ch, "To repair your eq type: repair all <repair>\n\r" );
            return;
        }    

        if ( !str_cmp( arg1, "repair" ) )
        {
            for (obj = ch->carrying; obj; obj = obj_next)
            {
                 obj_next = obj->next_content;

                 if (IS_SET(obj->extra_flags, ITEM_NOREPAIR))
                 continue;    

                 if ( obj->condition <= 50 && obj->condition > 0 && count++ > 1 )
                 {
                     found = TRUE;
                     cost += ( (obj->level * 10) +
                     ((obj->cost * (100 - obj->condition)) /100)    );
                     cost /= 100;
                 }                                    
            }
                 
            if ( cost <= ch->gold )
            {
                for (obj = ch->carrying; obj; obj = obj_next)
                {
                     obj_next = obj->next_content;

                     if (IS_SET(obj->extra_flags, ITEM_NOREPAIR))
                     continue;    

                     if ( obj->condition <= 50 && obj->condition > 0 && count++ > 1 )
                     {
                         found = TRUE;
                         set_obj_condition( obj, 100 );
                         
                     }                                    
                }
            }
             
            if ( cost > ch->gold )
            {
                printf_to_char( ch, "You don't have enough gold for my services.\n\r" );        
                return;
            }

            if ( !found )
            {
                do_say( blacksmith, "You don't have anything that needs repairing.\n\r" );    
                found = FALSE;    
                return;
            }

            ch->gold -= cost;
            blacksmith->gold += cost;                                        
            sprintf(buf, "$N takes $n's damaged objects repairs them and returns them to $n");
            wait_act( 0, buf,ch,NULL,blacksmith,TO_ROOM);
            sprintf(buf, "$N charges you %d gold takes your items, repairs them, and returns them to you.\n\r", cost );
            wait_act( 1, buf, ch, NULL, blacksmith, TO_CHAR );
            return;
        }

        else if ( !str_cmp( arg1, "estimate" ) )
        {
            for (obj = ch->carrying; obj; obj = obj_next)
            {
                 obj_next = obj->next_content;      

                 if (IS_SET(obj->extra_flags, ITEM_NOREPAIR))
                 continue;

                 if ( obj->condition <= 50 && obj->condition > 0 && count++ > 1 )
                 {
                     found = TRUE;
                     cost += ( (obj->level * 10) +
                     ((obj->cost * (100 - obj->condition)) /100)    );
                     cost /= 100;            
                 }
            }
 
            if ( !found )
            {
                do_say( blacksmith, "You don't have anything that needs repairing.\n\r" ); 
                found = FALSE;
                return;
            }

            printf_to_char( ch, "It will cost %d gold to fix those items.\n\r", cost );
            return;
        }
    }

    if (( obj = get_obj_carry(ch, arg, ch)) == NULL)
    {
	do_say(blacksmith,"You don't have that item");
	return;
    }

    if (obj->condition >= 100)
    {
	do_say(blacksmith,"But that item is not broken.");
        return;
    }

    if (obj->condition == 0)
    {
	sprintf(buf ,"%s is beyond repair.\n\r", obj->short_descr);
	do_say(blacksmith,buf);
   	return;
    }

    if (IS_SET(obj->extra_flags, ITEM_NOREPAIR)) 
    {
        send_to_char( "This object can not be repaired.\n\r", ch );
        return;
    }

    cost = ( (obj->level * 10) +
		((obj->cost * (100 - obj->condition)) /100)    );
    cost /= 100;

    if (!str_cmp( arg1, "estimate" ) )
    {
        sprintf(buf, "It will cost %d gold to fix that item", cost);
        do_say(blacksmith,buf);
    }

    if (cost > ch->gold)
    {
	do_say(blacksmith,"You do not have enough gold for my services.");
	return;
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);

    ch->gold -= cost;
    blacksmith->gold += cost;
    sprintf(buf, "$N takes %s from $n, repairs it, and returns it to $n", obj->short_descr); 
    wait_act( 0, buf,ch,NULL,blacksmith,TO_ROOM);
    sprintf(buf, "$N takes %s, repairs it, and returns it to you.\n\r", obj->short_descr);
    wait_act( 1, buf, ch, NULL, blacksmith, TO_CHAR ); 
    set_obj_condition( obj, 100 );
}


void do_beseech(CHAR_DATA *ch, char *argument)
{
   char buf[MSL];
   char arg[MSL];
   char arg1[MSL];
   CHAR_DATA *blesser;
   int value; 
    
   blesser = find_blesser( ch );
  
   if (!blesser)
       return;   

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg1 );  

   if (IS_NPC(ch))
      return;

   if ( arg[0] == '\0' || arg1[0] == '\0' || !is_number( arg ) )
   {	
       send_to_char( "Syntax: beseech <amount> <type>.\n\r", ch );
       send_to_char( "Valid types are:\n\r", ch );
       send_to_char( "     level, train, practice, gold, silver, questpoints\n\r", ch );
       send_to_char( "     hp, mana, move\n\r", ch );
       return;
   }

   value = atoi( arg );

   if ( value < 1 || value > 10 )
   {
       send_to_char( "You must choose a value between 1 and 10.\n\r", ch );
       return;
   }

   if ( !str_cmp( arg1, "level" ) )
   {
       if ( ch->level >= LEVEL_HERO )
           send_to_char( "Sorry you can't level anymore!\n\r", ch );

       if (ch->bp_points < value )
       {
           printf_to_char( ch, "You don't have %d blessing points!\n\r", value );
           return;
       }
  
       if ( value < 2 )
           send_to_char( "Sorry you need at least 2 blessing points for one level!\n\r", ch );
   
       ch->bp_points -= value;
       ch->level += value / 2;
       advance_level( ch, TRUE );
       sprintf(buf,"You are now level %d.\n\r",ch->level);
       send_to_char(buf,ch);
       ch->exp   = exp_per_level(ch,ch->pcdata->points) 
   		       * UMAX( 1, ch->level );
       do_function( ch, &do_save, "" );
       return;
   }

   if ( !str_cmp( arg1, "train" ) )
   {
       if ( ch->bp_points < value )
       {
           printf_to_char( ch, "You don't have %d blessing points!\n\r", value );
           return;
       }
      
       ch->bp_points -= value;
       ch->train += value * 2;        
       printf_to_char( ch, "You gained %d trains!\n\r", value * 2 );
       printf_to_char( ch, "You now have %d trains!\n\r", ch->train );
       do_function( ch, &do_save, "" );
       return;
    
   }

   if ( !str_cmp( arg1, "practice" ) )
   {
       if ( ch->bp_points < value )
       {
           printf_to_char( ch, "You don't have %d blessing points!\n\r", value );
           return;
       }

       ch->bp_points -= value;
       ch->practice += value * 20;
       printf_to_char( ch, "You gained %d practices!\n\r", value * 20 );
       printf_to_char( ch, "You now have %d practies!\n\r", ch->practice );
       do_function( ch, &do_save, "" );
       return;
   }

   if ( !str_cmp( arg1, "gold" ) )
   {
       if ( ch->bp_points < value )
       {
           printf_to_char( ch, "You don't have %d blessing points!\n\r", value );
           return;
       }
     
       ch->bp_points -= value;
       ch->gold += value * 500;
       printf_to_char( ch, "You gained %d gold!\n\r", value * 500 );
       printf_to_char( ch, "You now have %d gold!\n\r", ch->gold );
       do_function( ch, &do_save, "" );
       return;
   }
		 
   if ( !str_cmp( arg1, "silver" ) )
   {
       if ( ch->bp_points < value )
       {
           printf_to_char( ch, "You don't have %d blessing points!\n\r", value );
           return;
       }
      
       ch->bp_points -= value;
       ch->silver += value * 5000;
       printf_to_char( ch, "You gained %d silver!\n\r", value * 5000 );
       printf_to_char( ch, "You now have %d silver!\n\r", ch->silver );
       do_function( ch, &do_save, "" );
       return;
   }
  
   if ( !str_cmp( arg1, "questpoints" ) )
   {
       if ( ch->bp_points < value )
       {
           printf_to_char( ch, "You don't have %d blessing points!\n\r", value );
           return;
       }
 
       ch->bp_points -= value;
       ch->pcdata->questpoints += value * 50;
       printf_to_char( ch, "You gained %d questpoints!\n\r", value * 50 );
       printf_to_char( ch, "You now have %d questpoints!\n\r", ch->pcdata->questpoints );
       do_function( ch, &do_save, "" );
       return;
   }

   if ( !str_cmp( arg1, "hp" ) )
   {
       if ( ch->bp_points < value )
       {
           printf_to_char( ch, "You don't have %d blessing points!\n\r", value );
           return;
       }

       ch->bp_points -= value;
       ch->max_hit += value * 25;
       printf_to_char( ch, "You gained %d hp!\n\r", value * 25 );
       printf_to_char( ch, "You now have %d hit points!\n\r", ch->max_hit );
       do_function( ch, &do_save, "" );
       return;
   }

   if ( !str_cmp( arg1, "mana" ) )
   {
       if ( ch->bp_points < value )
       {
           printf_to_char( ch, "You don't have %d blessing points!\n\r", value );
           return;
       }
        
       ch->bp_points -= value;
       ch->max_mana += value *25;
       printf_to_char( ch, "You gained %d mana!\n\r", value * 25 );
       printf_to_char( ch, "You now have %d mana!\n\r", ch->max_mana );
       do_function( ch, &do_save, "" );
       return;
   }

   if ( !str_cmp( arg1, "move" ) )
   {  
       if ( ch->bp_points < value )
       {
           printf_to_char( ch, "You don't have %d blessing points!\n\r", value );
           return;
       }
       
       ch->bp_points -= value;
       ch->max_move += value * 25;
       printf_to_char( ch, "You gained %d move!\n\r", value * 25 );
       printf_to_char( ch, "You now have %d movement!\n\r", ch->max_move );
       do_function( ch, &do_save, "" );
       return;
   }

   send_to_char( "Syntax: beseech <amount> <type>\n\r", ch);
   return;      
}
   

void do_reveal( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    CHAR_DATA *sage;
    int cost;

    sage = find_sage( ch );

    if (!sage)
        return;

    if ( argument[0] == '\0' )
    { 
        send_to_char("Syntax: reveal objectname\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, argument, ch ) ) == NULL )
    {
       send_to_char( "You are not carrying that.\n\r", ch );
       return;
    }

    if (IS_IMMORTAL(ch))
    {
       send_to_char( "Your an immortal you don't need my services!\n\r", ch );
       return;
    }

    cost = obj->level * 50;

    if ( !str_cmp( argument, "cost" ) )
    {
        printf_to_char( ch, "The %s tells you, it will cost you %d gold to identify this object!\n\r", sage->name, cost );
        return;
    }

    if (ch->gold < cost)
    {
       printf_to_char( ch, "%s refuses to look at %s you need more gold.\n\r", sage->name, obj );
       return;
    }

    else
    {
       ch->gold -= cost;
       wait_act( 0, "$n takes your gold, and $p agrees to identify it.\n\r", sage->name, obj, NULL, TO_CHAR );
       wait_act( 1, "$n studies $p and glances towards the ground, when he looks up he reveals $p's stats for you.\n\r", sage->name, obj->name, obj->name, TO_CHAR );
       do_lore( ch, "obj->name" );
    }
 
    return;

}

void do_mtsteal( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *mount;
    int amount = 200;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Use: mtsteal <person who is mounted>.\n\r", ch );
        return;
    }

    if ( ch->move < amount )
    {
        printf_to_char( ch, "You need %d movement to do that.\n\r", amount );
        return;
    }

    if ( !( victim = get_char_room( ch, NULL, argument ) ) )
    {
        send_to_char( "They're not here.\n\r", ch );
        return;
    }

    if ( !MOUNTED(victim ))
    {
        send_to_char( "They're not mounted...\n\r", ch );
        return;
    }

    if (MOUNTED(ch))
    {
        send_to_char("You are already riding.\n\r", ch);
        return;
    }

    if ( victim->position == POS_FIGHTING
        || victim->fighting )
    {
        send_to_char( "Wait until they're done fighting.\n\r", ch );
        return;
    }

    if ( number_percent( ) <= 50 )
    {
        mount = MOUNTED( victim );
        victim->riding = FALSE;
        mount->riding = FALSE;
        ch->mount = mount;
        ch->riding = TRUE;
        mount->mount = ch;
        mount->riding = TRUE;
        ch->move /=2;
        act( "You steal $N's mount!", ch, NULL, victim, TO_CHAR );
        act( "$n has stolen your mount!", ch, NULL, victim, TO_VICT );
        return;
    }

    act( "You tried to steal $N's mount but failed.", 
	 ch, NULL, victim, TO_CHAR );
    printf_to_char( victim, "%s has tried to steal your mount!\n\r", ch->name );
    act( "$n tried to steal $N's mount but failed.", ch, NULL, victim, TO_ROOM );
    return;
}

void do_petshow( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *in_room;
 
  in_room = ch->in_room;

  if ( ch->pet == NULL )
  {
      send_to_char( "I'm sorry you don't have a pet!\n\r", ch );
      return;
  }

  if ( ch->pet->in_room->area != ch->in_room->area )
  {
      send_to_char( "Your pet has to be in the same area as you!\n\r", ch );
      return;
  }

  send_to_char("{bHere are the stats for your pet:{x\n\r", ch );
  send_to_char("{cName         Hit      Mana     Move     Location    Exp to level    Current exp    Levelflux{x\n\r",ch);     
  send_to_char("{C============================================================================================={x\n\r", ch );
  printf_to_char(ch, "%-12s %-8d %-8d %-8d %-15s %-16d %-13d %-7d\n\r", capitalize(ch->pet->short_descr), ch->pet->hit, ch->pet->mana, ch->pet->move, ch->pet->in_room->area->name, ch->pet->xp_tolevel, ch->pet->exp, ch->pet->levelflux );
  return; 

}


void do_convert( CHAR_DATA *ch, char *argument )
{
  char arg[MSL];
  char arg2[MSL];
  char arg3[MSL];
  CHAR_DATA *mob;
  int amount;
  int cost = 40;

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

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if ( arg[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' ) 
  {
      send_to_char( "Syntax: convert <amount> <type> to <type>\n\r", ch );
      send_to_char( "Valid types are: hp mana move\n\r", ch );
      printf_to_char( ch, "The current cost of conversion is: %d gold.\n\r", cost );
      return;
  }

  amount = is_number(arg) ? atoi(arg) : 1;        

  if ( amount < 1 || amount > 32000 )
  { 
      send_to_char( "Please choose a value between 1 and 32000.\n\r", ch );
      return;
  }

  if ( !str_cmp( arg2, "hp" ) )
  {
      if ( !str_cmp ( arg3, "move" ) )
      {
          if ( ch->pcdata->perm_hit <= amount )
          {
              send_to_char( "I'm sorry you don't have enough hitpoints to make that conversion!\n\r", ch );
              return;
          }

          if ( ch->gold < cost )
          {
              send_to_char( "I'm sorry you don't have enough gold!\n\r", ch );
              return;
          }

          ch->max_hit -= amount;
          ch->hit = ch->max_hit;
          ch->pcdata->perm_hit -= amount;
          ch->max_move += amount;
          ch->move = ch->max_move;
          ch->pcdata->perm_move += amount;
          ch->gold -= amount;
          printf_to_char( ch, "You convert %d hp into movement.\n\rYou have %d movement, and %d hp!\n\r", amount, ch->max_move, ch->max_hit );
          return;
      }
    
      else if ( !str_cmp ( arg3, "mana" ) )
      {
          if ( ch->pcdata->perm_hit <= amount )
          {
              send_to_char( "I'm sorry you don't have enough hitpoints to make that conversion!\n\r", ch );
              return;
          }
     
          if ( ch->gold < cost )
          {
              send_to_char( "I'm sorry you don't have enough gold!\n\r", ch );
              return;
          }        

          ch->max_hit -= amount;
          ch->hit = ch->max_hit;
          ch->pcdata->perm_hit -= amount;
          ch->max_mana += amount;
          ch->mana = ch->max_mana;
          ch->pcdata->perm_mana += amount;
          ch->gold -= amount;
          printf_to_char( ch, "You convert %d hp into mana.\n\rYou have %d mana and %d hp!\n\r", amount, ch->max_mana, ch->max_hit );
          return;
      }
  }
  
  else if ( !str_cmp( arg2, "mana" ) )
  {
      if ( !str_cmp( arg3, "hp" ) )
      {
          if ( ch->pcdata->perm_mana <= amount )
          {
              send_to_char( "I'm sorry you don't have enough mana to make that conversion!\n\r", ch );
              return;
          }             
         
          if ( ch->gold < cost )
          {
              send_to_char( "I'm sorry you don't have enough gold!\n\r", ch );
              return;
          }        

          ch->max_mana -= amount;
          ch->mana = ch->max_mana;
          ch->pcdata->perm_mana -= amount;
          ch->max_hit += amount;
          ch->hit = ch->max_hit;
          ch->pcdata->perm_hit += amount;
          ch->gold -= amount;
          printf_to_char( ch, "You convert %d mana into hp.\n\rYou have %d hp and %d mana!\n\r", amount, ch->max_hit, ch->max_mana );
          return;
      }

      else if ( !str_cmp( arg3, "move" ) )
      {
          if ( ch->pcdata->perm_mana <= amount )
          {
              send_to_char( "I'm sorry you don't have enough mana to make that conversion!\n\r", ch );
              return;
          }

          if ( ch->gold < cost )
          {
              send_to_char( "I'm sorry you don't have enough gold!\n\r", ch );
              return;
          }        

          ch->max_mana -= amount;
          ch->mana = ch->max_mana;
          ch->pcdata->perm_mana -= amount;
          ch->max_move += amount;
          ch->move = ch->max_move;
          ch->pcdata->perm_move += amount;
          ch->gold -= amount;
          printf_to_char( ch, "You convert %d mana into move.\n\rYou have %d move and %d mana!\n\r", amount, ch->max_move, ch->max_mana );
          return;
      }
  }  

  else if ( !str_cmp( arg2, "move" ) )
  {
      if ( !str_cmp( arg3, "hp" ) )
      {
          if ( ch->pcdata->perm_move <= amount )
          {
              send_to_char( "I'm sorry you don't have enough movement to make that conversion!\n\r", ch );
              return;
          }

          if ( ch->gold < cost )
          {
              send_to_char( "I'm sorry you don't have enough gold!\n\r", ch );
              return;
          }        

          ch->max_move -= amount;
          ch->move = ch->max_move;
          ch->pcdata->perm_move -= amount;
          ch->max_hit += amount;
          ch->hit = ch->max_hit;
          ch->pcdata->perm_hit += amount;
          ch->gold -= amount;
          printf_to_char( ch, "You convert %d movement into hp.\n\rYou have %d hp and %d movement!\n\r", amount, ch->max_hit, ch->max_move );
          return;
      }

      else if ( !str_cmp( arg3, "mana" ) )
      {
          if ( ch->pcdata->perm_move <= amount )
          {
              send_to_char( "I'm sorry you don't have enough movement to make that conversion!\n\r", ch );
              return;
          }

          if ( ch->gold < cost )
          {
              send_to_char( "I'm sorry you don't have enough gold!\n\r", ch );
              return;
          }        

          ch->max_move -= amount;
          ch->move = amount;
          ch->pcdata->perm_move -= amount;
          ch->max_mana += amount;
          ch->hit = ch->max_hit;
          ch->pcdata->perm_hit += amount;
          ch->gold -= amount;
          printf_to_char( ch, "You convert %d movement into mana.\n\rYou have %d mana and %d movement!\n\r", amount, ch->max_mana, ch->max_move );
          return;
      }
  }

  send_to_char( "Syntax: convert <amount> <type> to <type>\n\r", ch );
  send_to_char( "Valid types are: hp, mana, move\n\r", ch );
  return;
}


void do_qsmith(CHAR_DATA *ch, char *argument)  
{
    OBJ_DATA *obj;
    AFFECT_DATA paf;
    char arg1[MIL];
    char arg2[MIL];
    char arg3[MIL];
    CHAR_DATA *smith;
    char buf[MSL];
    int affect_modify = 0, bit = 0, enchant_type, pos, i, cost;
	
    smith = find_smith( ch );
  
    if (!smith)
      return;   

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
	
    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "This is a valid list of types and affects:\n\r", ch );
        for (i = 0; smith_table[i].type != NULL; i++) 
        {
             sprintf(buf, "{c%-10s{x {C%-25s{x {c%-2d{x {Ciqp{x\n\r", i < 14 ? "Type" : "Affect", smith_table[i].type, smith_table[i].cost);
             send_to_char( buf, ch );
        }
	send_to_char("\n\rSyntax: qsmith <object> <type> <value>\n\r",ch);
	send_to_char("Syntax: qsmith <object> affect <name>\n\r",ch);
	return;
    }
	
    if ((obj = get_obj_here(ch, NULL, arg1)) == NULL)
    {
 	send_to_char("No such object exists!\n\r",ch);
	return;
    }

    else if (!str_prefix(arg2,"hp"))
    {
	enchant_type = APPLY_HIT;
    }

    else if (!str_prefix(arg2,"str"))
    {
    	enchant_type = APPLY_STR;
    }

    else if (!str_prefix(arg2,"dex"))
    {
	enchant_type = APPLY_DEX;
        cost = 1;
    }

    else if (!str_prefix(arg2,"int"))
    {
	enchant_type = APPLY_INT;
    }

    else if (!str_prefix(arg2,"wis"))
    {
	enchant_type = APPLY_WIS;
    }

    else if (!str_prefix(arg2,"con"))
    {
	enchant_type = APPLY_CON;
    }

    else if (!str_prefix(arg2,"mana"))
    {
	enchant_type = APPLY_MANA;
    }

    else if (!str_prefix(arg2,"move"))
    {
	enchant_type = APPLY_MOVE;
    }

    else if (!str_prefix(arg2,"ac"))
    {
	enchant_type = APPLY_AC;
    }

    else if (!str_prefix(arg2,"hitroll"))
    {
	enchant_type = APPLY_HITROLL;
    }

    else if (!str_prefix(arg2,"damroll"))
    {
	enchant_type = APPLY_DAMROLL;
        cost = 2;
    }

    else if (!str_prefix(arg2,"saves"))
    {
	enchant_type = APPLY_SAVING_SPELL;
        cost = 2;
    }

    else if (!str_prefix(arg2,"clevel"))
    {
        enchant_type = APPLY_CAST_LEVEL;
    }

    else if (!str_prefix(arg2,"cability"))
    {
        enchant_type = APPLY_CAST_ABILITY;
    }

    else if (!str_prefix(arg2,"affect"))
    {
	enchant_type = APPLY_SPELL_AFFECT;
    }

    else
    {
 	send_to_char("That apply is not possible!\n\r",ch);
	return;
    }
	
    if (enchant_type == APPLY_SPELL_AFFECT)
    {
	for (pos = 0; affect_flags[pos].name != NULL; pos++)
	     if (!str_cmp(affect_flags[pos].name,arg3))
		 bit = affect_flags[pos].bit;
    }
	
    else
    {
	if ( is_number(arg3) )
        {
	    affect_modify = atoi(arg3);  
            cost = affect_modify;
	}
	
        else
	{
	    send_to_char("Applies require a value.\n\r", ch);
	    return;
	}
    }

    affect_enchant(obj);

    /* create the affect */
    paf.where	             = TO_AFFECTS;
    paf.type	             = 0;
    paf.level	             = ch->level;
    paf.duration             = -1;
    paf.location             = enchant_type;
    paf.modifier             = affect_modify;
    paf.bitvector            = bit;
    	
    if ( enchant_type == APPLY_SPELL_AFFECT )
    {
	/* make table work with skill_lookup */		
        for ( i=0 ; arg3[i] != '\0'; i++ )
        {
             if ( arg3[i] == '_' )
              	 arg3[i] = ' ';
        }
				
        paf.type      = skill_lookup(arg3);

        for ( i = 0; smith_table[i].type != NULL; i++ )
        {
             if (is_name(arg3, smith_table[i].type))
             {
                 if (ch->pcdata->qps >= smith_table[i].cost)
                 {      
                     ch->pcdata->qps -= smith_table[i].cost;       
                 }
                
                 affect_to_obj(obj,&paf); 
                 sprintf(buf, "$N takes %s, adds your affect and studies his work carefully.\n\r", obj->short_descr ); 
                 wait_act( 2, buf, ch, NULL, smith, TO_CHAR );   
                 sprintf(buf, "$N says, You owe me %d immortal questpoints for adding those affects.\n\r", smith_table[i].cost );
                 wait_act( 4, buf, ch, NULL, smith, TO_CHAR );
                 sprintf(buf, "You pay $N %d immortal questpoints and thank him for his services.\n\r", smith_table[i].cost );
                 wait_act( 6, buf, ch, NULL, smith, TO_CHAR );
                 sprintf(buf, "$N smiles, taking your questpoints, then gives you %s.\n\rI guarantee all my work so if you have any problems come back and see me.\n\r", obj->short_descr );
                 wait_act( 8, buf, ch, NULL, smith, TO_CHAR );
                 sprintf(buf, "You have %d immortal questpoints left.\n\r", ch->pcdata->qps );
                 wait_act( 10, buf, ch, NULL, NULL, TO_CHAR );
             }
        }
    } 

    else
    {
        for ( i = 0; smith_table[i].type != NULL; i++ )
        {
             if (is_name(arg3, smith_table[i].type))
             {
                 if (ch->pcdata->qps >= smith_table[i].cost)
                 {      
                     ch->pcdata->qps -= ( smith_table[i].cost * cost );       
                 }
                  
                 affect_to_obj(obj,&paf); 
                 sprintf(buf, "$N takes %s, adds your affect and studies his work carefully.\n\r", obj->short_descr ); 
                 wait_act( 2, buf, ch, NULL, smith, TO_CHAR );   
                 sprintf(buf, "$N says, You owe me %d immortal questpoints for adding those affects.\n\r", smith_table[i].cost * cost );
                 wait_act( 4, buf, ch, NULL, smith, TO_CHAR );
                 sprintf(buf, "You pay $N %d immortal questpoints and thank him for his services.\n\r", smith_table[i].cost * cost );
                 wait_act( 6, buf, ch, NULL, smith, TO_CHAR );
                 sprintf(buf, "$N smiles, taking your questpoints, then gives you %s.\n\rI guarantee all my work so if you have any problems come back and see me.\n\r", obj->short_descr );
                 wait_act( 8, buf, ch, NULL, smith, TO_CHAR );
                 sprintf(buf, "You have %d immortal questpoints left.\n\r", ch->pcdata->qps );
                 wait_act( 10, buf, ch, NULL, NULL, TO_CHAR );
                 return;
             }
        }
    }
} 


PET_DATA *pet_list;

void save_pets()
{
  FILE *fp;
  PET_DATA *pEt;

  if ( ( fp = file_open( PET_FILE, "w" ) ) == NULL )
  { 
      logf2("The pet file is gone!\n\r");
      return;
  }

  for( pEt = pet_list; pEt != NULL; pEt = pEt->next)
  {
      fprintf(fp,"Cname %s~\n", pEt->cname );
      fprintf(fp,"Name %s~\n", pEt->name );
      fprintf(fp,"Shd %s~\n", pEt->short_desc );
      fprintf(fp,"Lnd %s~\n", pEt->long_desc );
      fprintf(fp,"Cost %d\n", pEt->cost );
      fprintf(fp,"Vnum %d\n", pEt->vnum );
      fprintf(fp,"Level %d\n", pEt->level );
      fprintf(fp,"Mhit %d\n", pEt->max_hit );
      fprintf(fp,"Mmana %d\n", pEt->max_mana );
      fprintf(fp,"Mmove %d\n", pEt->max_move );
      fprintf(fp,"Exp %d\n", pEt->exp );
      fprintf(fp,"Xptolevel %d\n", pEt->xp_tolevel );
      fprintf(fp,"Levelflux %d\n", pEt->levelflux );
      fprintf(fp,"Practice %d\n", pEt->practice );
      fprintf(fp,"Train %d\n", pEt->train );
      fprintf(fp,"Damroll %d\n", pEt->damroll );
      fprintf(fp,"Hitroll %d\n", pEt->hitroll );
      fprintf(fp,"Armor %d %d %d %d\n", pEt->armor[0], pEt->armor[1], pEt->armor[2], pEt->armor[3] );
      fprintf(fp,"Gold %d\n", pEt->gold );
      fprintf(fp,"Silver %d\n\n", pEt->silver );
  }

  fprintf(fp, "$\n");
  logf2("Pets saved.");
  file_close(fp);   
  return;
}

void load_pets()
{
  FILE *fp;
  const char *word;
  PET_DATA *plist;
  PET_DATA *pEt;

  if (file_exists(PET_FILE))
  {
      fp = file_open( PET_FILE, "r" );

      plist = NULL;

      for( ; ; )
      {
          word = feof( fp ) ? "End" : fread_word( fp );

          if(word[0] == '$' )
             return;                                    

          if (!str_cmp(word, "Cname" ) )
          {
              pEt = new_pet();
              if(pet_list == NULL)
              pet_list = pEt;
              else
              plist->next = pEt;
              plist = pEt;       
              pEt->cname = str_dup(fread_string(fp));
          }   

          if (!str_cmp(word, "Name" ) )
              pEt->name = str_dup(fread_string(fp));

          if (!str_cmp(word, "Shd" ) )
              pEt->short_desc = str_dup(fread_string(fp));
 
          if (!str_cmp(word, "Lnd" ) )
              pEt->long_desc = str_dup(fread_string(fp));

          if (!str_cmp(word, "Cost" ) )
              pEt->cost = fread_number(fp);
       
          if (!str_cmp(word, "Vnum" ) )
              pEt->vnum = fread_number(fp); 
         
          if (!str_cmp(word, "Level" ) )
              pEt->level = fread_number(fp);
        
          if (!str_cmp(word, "Mhit" ) )
              pEt->max_hit = fread_number(fp);
          
          if (!str_cmp(word, "Mmana" ) )
              pEt->max_mana = fread_number(fp);
         
          if (!str_cmp(word, "Mmove" ) )
              pEt->max_move = fread_number(fp);
        
          if (!str_cmp(word, "Exp" ) )
              pEt->exp = fread_number(fp);

          if (!str_cmp(word, "Xptolevel" ) )
              pEt->xp_tolevel = fread_number(fp);
        
          if (!str_cmp(word, "Levelflux" ) )
              pEt->levelflux = fread_number(fp);
       
          if (!str_cmp(word, "Practice" ) )
              pEt->practice = fread_number(fp);
          
          if (!str_cmp(word, "Train" ) )
              pEt->train = fread_number(fp);
          
          if (!str_cmp(word, "Damroll" ) )
              pEt->damroll = fread_number(fp);
            
          if (!str_cmp(word, "Hitroll" ) )
              pEt->hitroll = fread_number(fp);

          if (!str_cmp(word, "Armor" ) )
          {
              int i;

              for (i = 0; i < 4; i++)
                   pEt->armor[i] = fread_number(fp);
          }                

          if (!str_cmp(word, "Gold" ) )
              pEt->gold = fread_number(fp);
       
          if (!str_cmp(word, "Silver" ) )
              pEt->silver = fread_number(fp);
      }  
  }
 
  return;
}


bool on_petlist(CHAR_DATA *ch)
{
  PET_DATA *pEt;
  bool found = FALSE;

  for(pEt = pet_list; pEt != NULL; pEt = pEt->next)
  {
    if( !str_cmp(ch->name,pEt->cname) )
      found = TRUE;
  }

  return found;
}


void do_petlist( CHAR_DATA *ch, char *argument )
{
   char arg[MSL];
   char arg1[MSL];
   PET_DATA *pEt;
   PET_DATA *prev = NULL;
   CHAR_DATA *pet;
   bool found = FALSE;
   int vnum;

   argument = one_argument( argument, arg );
   one_argument( argument, arg1 );

   if ( IS_NPC( ch ) )
   {
       send_to_char( "Sorry mobs can not own pets!\n\r", ch );
       return;
   }

   if(arg[0] == '\0')
   {
      send_to_char("Name              Cost\n\r",ch);
      send_to_char("{B======================{x\n\r",ch);

      for(pEt = pet_list;pEt != NULL;pEt = pEt->next)
      {
          if( !str_cmp(ch->name,pEt->cname) )       
          {
              printf_to_char( ch, "%-12s      %-4d\n\r", pEt->name, pEt->cost);
              found = TRUE;
          }
      } 

      if ( !found )
      {
          send_to_char( "You do not have any pets that you can ressurect!\n\r", ch );
          return;
      }

      return;
   }

   if ( arg1[0] == '\0' )
   {
       send_to_char( "Syntax: petlist revive <name>\n\r", ch );
       return;
   }

   if ( !str_cmp( arg, "revive" ) )
   {
       for ( pEt = pet_list; pEt != NULL; prev = pEt, pEt = pEt->next )
       {
            if ( !str_cmp( arg1, pEt->name ) )
                break;
       }
  
       if ( pEt == NULL )
       {
           send_to_char( "That pet is not on the list!\n\r", ch );
           return;
       }

       if ( ch->pet != NULL )
       {
           send_to_char("You already own a pet.\n\r",ch);
           return;
       }          

       vnum = pEt->vnum;

       if (get_mob_index(vnum) == '\0')
       {
           send_to_char( "This pet has been deleted from the mud, contact an immortal.\n\r", ch );
           return;
       }    
         
       pet = create_mobile( get_mob_index( vnum ) );
       SET_BIT(pet->act, ACT_PET);
       SET_BIT(pet->affected_by, AFF_CHARM);           
       pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;                  
       ch->gold -= pEt->cost;
       revived_pet( pet, pEt );
       char_to_room( pet, ch->in_room );
       send_to_char( "Ok here is your pet enjoy.\n\r", ch );
       add_follower( pet, ch );
       pet->leader = ch;
       ch->pet = pet;
             
       if ( prev == NULL )
           pet_list = pet_list->next;
       else
           pEt->next = pEt->next;
       free_pet(pEt);
       save_pets();
       return;       
   }
}    
    
void do_level( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *leveler;

    for ( leveler = ch->in_room->people;
         leveler != NULL;
         leveler = leveler->next_in_room )
    {
         if ( IS_NPC(leveler) && IS_SET(leveler->act, ACT_LEVELER) )
                break;
    }

    if (leveler == NULL || !can_see(ch,leveler))
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }                     

    if (ch->pcdata->current_exp < exp_per_level(ch, ch->pcdata->points))
    {
        sprintf(buf, "You have insufficient experience to gain another level, %s.",ch->name);
        do_say(leveler,buf);
        return;
    }

        ch->level += 1;
        sprintf(buf,"%s gained level %d",ch->name,ch->level);
        log_string(buf);                   
        sprintf(buf,"$N has attained level %d!",ch->level);
        wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);
        advance_level(ch,FALSE);
        info( ch, 0, "{G[INFO]:{x{G%s{x {Bhas just attained level{x {R%d{x!{x\n\r", ch->name, ch->level );
        affect_strip(ch,skill_lookup("plague"));
        affect_strip(ch,skill_lookup("poison"));
        affect_strip(ch,skill_lookup("blindness"));
        affect_strip(ch,skill_lookup("sleep"));
        affect_strip(ch,skill_lookup("curse"));

        ch->hit        = ch->max_hit;
        ch->mana       = ch->max_mana;
        ch->move       = ch->max_move;
        update_pos( ch);
        ch->pcdata->current_exp -= exp_per_level(ch, ch->pcdata->points);
        send_to_char( "{G[INFO]:{x {BSynon has given you the power to continue!{x\n\r", ch );
        save_char_obj(ch);                              
        return;
}

void do_travel(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *summoner,*pet;
    ROOM_INDEX_DATA *room;
    int i = 0;
    char  buf[MSL], arg[MSL];
    char  arg1[MSL];

    argument = one_argument(argument, arg );
    argument = one_argument(argument, arg1);

    summoner = find_summoner (ch);

    if (!summoner)
        return;

    if (arg[0] == '\0')
    {
	printf_to_char(ch, "You must tell me what travel you want to do:\n\r"
		     "TRAVEL list - shows possible locations to travel to.\n\r"
		     "TRAVEL buy <name> - Travel to selected location.\n\r");
	return;
    }

    if (!strcmp( arg, "list"))
    {
	printf_to_char(ch, "%s says you may travel to the following locations:\n\r", PERS(summoner,ch));

	for(i=0;summoner_table[i].name != NULL;i++)
	{
	   sprintf(buf,"\t%-15s - %-3d %s\n\r",
		summoner_table[i].name,summoner_table[i].cost, MONEY); 
	   send_to_char(buf,ch);
        }

	return;
    }

    if (!strcmp( arg, "buy"))
    {
	if (arg1[0] == '\0')
	{
	    sprintf(buf, "You must tell me where you wish to travel"); 
	    act("$N says '$t'.", ch, buf, summoner, TO_CHAR);
	    return;
	}

	for(i=0;summoner_table[i].name != NULL;i++)
 	    if(is_name(arg1,summoner_table[i].name))
 		break;

        if(summoner_table[i].name == NULL)
        {
            sprintf(buf, "That location of travel is not on the list");
            act("$N says '$t'.", ch, buf, summoner, TO_CHAR);
            return;
        }

        if ( (ch->silver + 100 * ch->gold) < summoner_table[i].cost )
        {
                sprintf(buf, "You don't have enough %s for my services",MONEY);
                act("$N says '$t'.", ch, buf, summoner, TO_CHAR);
		return;
        }
	
        if( (room = get_room_index(summoner_table[i].vnum) ) == NULL )
        {
   	     send_to_char("That location's room doesn't seem to exist, please inform your Imp.\n\r",ch);
	     return;
 	}
	
        if ( (pet = ch->pet) != NULL && pet->in_room == ch->in_room)
        {
     	     char_from_room( pet );
	     char_to_room( pet, get_room_index(summoner_table[i].vnum) );
	}

        char_from_room( ch );
        char_to_room( ch, get_room_index(summoner_table[i].vnum) );
        deduct_cost(ch,summoner_table[i].cost);
        printf_to_char(ch, "%s utters the words 'hasidsindsad'\n\rYou are surrounded by a violet fog.\n\r", summoner->short_descr);
        do_function( ch, &do_look, "" );
        return;
    }

    else
        do_function( ch, &do_travel, "" );
}

