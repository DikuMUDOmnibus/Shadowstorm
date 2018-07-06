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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "const.h"

extern char *target_name;

void spell_farsight( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    if (IS_AFFECTED(ch,AFF_BLIND))
    {
        send_to_char("Maybe it would help if you could see?\n\r",ch);
        return;
    }
    if(!IS_NPC(ch) && IN_MINE(ch) )
    {	send_to_char("You are surrounded by a mine.\n\r",ch);
	return;
    }
    do_function(ch, &do_scan, target_name);
}


void spell_portal( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;

    if(!IS_NPC(ch) && IN_MINE(ch) )
    {	send_to_char("Not in a mine!\n\r",ch);
	return;
    }

        if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room)
	||   IS_SET(victim->in_room->room_flags, ROOM_ARENA)
	||   IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    || (IS_NPC(victim) && is_gqmob(ch, victim->pIndexData->vnum) != -1)   
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_NONE) ) 
    ||	(is_clan(victim) && !is_same_clan(ch,victim)))
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }   

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch) 
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
	send_to_char("You lack the proper component for this spell.\n\r",ch);
	return;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
     	act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
     	act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
     	extract_obj(stone);
    }

    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 2 + level / 25; 
    portal->value[3] = victim->in_room->vnum;
    obj_to_room(portal,ch->in_room);

    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
}

void spell_nexus( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;
    ROOM_INDEX_DATA *to_room, *from_room;

    from_room = ch->in_room;
 
    if(!IS_NPC(ch) && IN_MINE(ch) )
    {	send_to_char("Not within a mine.\n\r",ch);
	return;
    }
        if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   (to_room = victim->in_room) == NULL
    ||   !can_see_room(ch,to_room) || !can_see_room(ch,from_room)
	||   IS_SET(victim->in_room->room_flags, ROOM_ARENA)
	||   IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(to_room->room_flags, ROOM_SAFE)
    ||	 IS_SET(from_room->room_flags,ROOM_SAFE)
    ||   IS_SET(to_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(to_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(to_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(from_room->room_flags,ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    || (IS_NPC(victim) && is_gqmob(ch, victim->pIndexData->vnum) != -1)   
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_NONE) ) 
    ||	 (is_clan(victim) && !is_same_clan(ch,victim)))
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }   
 
    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch)
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return;
    }
 
    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
        extract_obj(stone);
    }

    /* portal one */ 
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + level / 10;
    portal->value[3] = to_room->vnum;
 
    obj_to_room(portal,from_room);
 
    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
	return;

    /* portal two */
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + level/10;
    portal->value[3] = from_room->vnum;

    obj_to_room(portal,to_room);

    if (to_room->people != NULL)
    {
	act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM);
	act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR);
    }
}

void spell_group_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *gch;
    AFFECT_DATA af;
    int sanc_sn;

    sanc_sn = skill_lookup("sanctuary");

    for( gch=ch->in_room->people; gch != NULL; gch=gch->next_in_room)
    {
        if( !is_same_group( gch, ch ) )
           continue;
        
        if( IS_AFFECTED(gch, AFF_SANCTUARY ) )
        {
           if(gch == ch)
              send_to_char("You are already in sanctuary.\n\r", ch);
           else
              act( "$N is already in sanctuary.", ch, NULL, gch, TO_CHAR);
              continue;
        }

        af.type      = sanc_sn;
        af.level     = level;
        af.duration  = number_fuzzy( level/6 );
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_SANCTUARY;
        affect_to_char( gch, &af );

        send_to_char( "You are surrounded by a white aura.\n\r",gch );
       
        if( ch != gch )
           act( "$N is surrounded by a white aura.", ch, NULL, gch, TO_CHAR );
    }
    return;
}                   

void spell_quench( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

        if ( IS_NPC(ch) )
                return;

        ch->pcdata->condition[COND_THIRST] = 30;
        send_to_char( "{RYou are no longer thirsty.{x\n\r", victim );
        return;
}    

void spell_sate( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

        if ( IS_NPC(ch) )
                return;

        ch->pcdata->condition[COND_HUNGER] = 24;
        send_to_char( "{RYou are no longer hungry.{x\n\r", victim );
        return;
}   

void spell_moss_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("Your skin is already covered in moss.\n\r",ch);
        else
          act("$N already has the texture of moss.",ch,NULL,victim,TO_CHAR);
        return;                    
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -15;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin turns to moss.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin turns to moss.\n\r", victim );
    return;
}             

void spell_mud_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your skin is already as mud.\n\r",ch);
	else
	  act("$N already has the texture of mud.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -10;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin turns to mud.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin turns to mud.\n\r", victim );
    return;
}

void spell_diamond_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("Your skin already has a brilliant glow.\n\r",ch);
        else
          act("$N already has a brilliant glow.",ch,NULL,victim,TO_CHAR);
        return;
    }   

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -80;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin is protected by a brilliant arua.", victim, NULL,NULL, TO_ROOM );
    send_to_char( "Your skin is protected by a brilliant aura.\n\r", victim ); 
    return;
}

void spell_ice_bolt( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = 150 + dice( level/4, 12 );

    if ( saves_spell( level, victim, DAM_COLD ) )
      dam = (dam * 100) / 120;

    act( "$N struck in the chest by a bolt of ice from $n!", ch, NULL, victim, TO_NOTVICT );
    act( "You strike $N with a large bolt of ice!\n\r", victim, NULL, ch, TO_CHAR );

    damage( ch, victim, dam, sn, DAM_COLD, TRUE );         
    return;
}     

void spell_fire_bolt( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = 150 + dice( level/4, 12 );

    if ( saves_spell( level, victim, DAM_FIRE ) )
      dam = (dam * 100) / 120;

    act( "$N is struck in the chest by a bolt of fire from $n!", ch, NULL, ch, TO_NOTVICT );
    act( "You strike $n with a large bolt of fire!\n\r", ch, NULL, ch, TO_CHAR );

    damage( ch, victim, dam, sn, DAM_FIRE, TRUE );  
    return;
}

void spell_gas_bolt( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = 150 + dice( level/4, 12 );

    if ( saves_spell( level, victim, DAM_OTHER ) )
      dam = (dam * 100) / 120;
 
    act( "$N struck in the chest by a bolt of gas from $n!", ch, NULL, ch, TO_NOTVICT );
    act( "You strike $n with a large bolt of gas!\n\r", ch, NULL, ch, TO_CHAR );

    damage( ch, victim, dam, sn, DAM_OTHER, TRUE );
    return;
}
      
void spell_acid_bolt( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = 150 + dice( level/4, 12 );

    if ( saves_spell( level, victim, DAM_ACID ) )
      dam = (dam * 100) / 120;

    act( "$N struck in the chest by a bolt of acid from $n!", ch, NULL, ch, TO_NOTVICT );
    act( "You strike $n with a large bolt of acid!\n\r", ch, NULL, ch, TO_CHAR );

    damage( ch, victim, dam, sn, DAM_ACID, TRUE );

    return;
}          

void spell_holy_bolt( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = 150 + dice( level/4, 12 );

    if ( saves_spell( level, victim, DAM_HOLY ) && saves_spell( level, victim, DAM_LIGHTNING))
        dam = (dam * 100) / 120;

    act( "$N struck in the chest by a bolt of holy light from $n!", ch, NULL, ch, TO_NOTVICT );
    act( "You strike $n with a large bolt of holy light!\n\r", ch, NULL, ch, TO_CHAR );

    damage( ch, victim, dam, sn, DAM_HOLY, TRUE );
    return;
}            

void spell_iceblast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam         = ( level /2 )
                   +dice( ( level/6 ), 10 )
                   +dice( ( level/6 ), 10 );
    act( "A blast of ice flies from $n's hands!", ch, NULL, NULL, TO_ROOM );
    send_to_char( "A blast of ice flies from your hands!\n\r", ch );

    if ( saves_spell( level, victim, DAM_COLD ) )
                dam /= 2;

    act( "$n is struck by the blast of ice!!", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are struck by the ice blast!!\n\r", victim );

    damage( ch, victim, dam, sn, DAM_COLD, TRUE );           

    return;
}

void spell_gasblast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam         = ( level /2 )
                   +dice( ( level/6 ), 10 )
                   +dice( ( level/6 ), 10 );
    act( "A blast of gas flies from $n's hands!", ch, NULL, NULL, TO_ROOM );
    send_to_char( "A blast of gas flies from your hands!\n\r", ch );

    if ( saves_spell( level, victim, DAM_OTHER ) )
  		dam /= 2;

    act( "$n is struck by the blast of gas!!", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are struck by the gas blast!!\n\r", victim );

    damage( ch, victim, dam, sn, DAM_OTHER, TRUE );
    return;
}

void spell_electric_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam         = ( level /2 )
                   +dice( ( level/6 ), 10 )
                   +dice( ( level/6 ), 10 );

    act( "A blast of electricity flies from $n's hands!", ch, NULL, NULL, TO_ROOM );
    send_to_char( "A blast of electricity flies from your hands!\n\r", ch );

    if ( saves_spell( level, victim, DAM_LIGHTNING ) )
  		dam /= 2;

    act( "$n is struck by the blast of electricity!!", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are struck by the electric blast!!\n\r", victim );

    damage( ch, victim, dam, sn, DAM_LIGHTNING, TRUE );
    return;
}

void spell_lightningblast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam         = ( level /2 )
                   +dice( ( level/6 ), 10 )
                   +dice( ( level/6 ), 10 );

    act( "A blast of lightning flies from $n's hands!", ch, NULL, NULL, TO_ROOM );
    send_to_char( "A blast of lightning flies from your hands!\n\r", ch );

    if ( saves_spell( level, victim, DAM_LIGHTNING ) )
  		dam /= 2;

    act( "$n is struck by the blast of lightning!!", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are struck by the lightning blast!!\n\r", victim );

    damage( ch, victim, dam, sn, DAM_LIGHTNING, TRUE );
    return;
}

void spell_holyblast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam         = ( level /2 )
                   +dice( ( level/6 ), 10 )
                   +dice( ( level/6 ), 10 );
   
    act( "A blast of holy light flies from $n's hands!", ch, NULL, NULL, TO_ROOM );
    send_to_char( "A blast of holy light flies from your hands!\n\r", ch );

    if ( saves_spell( level, victim, DAM_HOLY ) )
  		dam /= 2;

    act( "$n is struck by the blast of holy light!!", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are struck by the blast of holy light!!\n\r", victim );

    damage( ch, victim, dam, sn, DAM_HOLY, TRUE );
    return;
}

void spell_cone_of_cold( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch;

    act("$n shoots a {bcone{x of {bcold{x at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n shoots a {bcone{x of {bcold{x at you.",ch,NULL,victim,TO_VICT);
    act("You shoot a {bcone{x of {bcold{x at $N.",ch,NULL,victim,TO_CHAR);

    hpch = UMAX(12,ch->hit);
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,12);                                       

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);      

    if (saves_spell(level,victim,DAM_COLD))
        dam /= 2;

    damage(ch, victim, dam, sn, DAM_COLD, TRUE);
}        


void spell_desert_fist( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int chance;
  
    chance = number_range( 1, 100 );

    if ( (ch->in_room->sector_type != SECT_HILLS)
        && (ch->in_room->sector_type != SECT_MOUNTAIN)
        && (ch->in_room->sector_type != SECT_DESERT) )
    {
         send_to_char("You don't find any sand here to create a fist.\n\r",ch);
         ch->wait = 0;
         return;
    }
                  
    act("An existing parcel of sand rises up and forms a fist and pummels $n.",
                victim,NULL,NULL,TO_ROOM);
    act("An existing parcel of sand rises up and forms a fist and pummels you.",
                victim,NULL,NULL,TO_CHAR);
    dam = dice( level + 25 , 20 );
    damage(ch,victim,dam,sn,DAM_OTHER,TRUE);
    sand_effect(victim,level,dam,TARGET_CHAR);

    if ( chance > 50 ) 
        return;     

    dam = dice( level + 40, 25 );
    damage(ch,victim,dam,sn,DAM_BASH,TRUE);
    sand_effect(victim,level,dam,TARGET_CHAR);
    act("A large mound of sand rises up and forms a huge fist pummeling you!\n\r", victim, NULL, NULL,TO_CHAR);
}

             
void spell_random( int sn, int level, CHAR_DATA * ch, void *vo, int target )
{
    SPELL_FUN *spell_list[MAX_SKILL];
    int sn_list[MAX_SKILL];
    int i;
    int count = 0;
    int prandom = 0, gsn = 0;
 
    for(i = 0; i < MAX_SKILL; i++)
    {
        spell_list[i] = NULL;
        sn_list[i] = i;
    }

    for( i = 0; i < MAX_SKILL; i++ )
    {
        if(skill_table[i].spell_fun == spell_null
            || skill_table[i].target == TAR_CHAR_OFFENSIVE
            || skill_table[i].target == TAR_OBJ_INV
            || skill_table[i].target == TAR_IGNORE
            || skill_table[i].target == TAR_OBJ_CHAR_OFF )
            continue;

        spell_list[count] = skill_table[i].spell_fun;
        sn_list[count] = i;
        count++;
    }

    prandom = number_range(0, count);
    gsn = sn_list[prandom];
    spell_list[prandom]( gsn, ch->level, ch, ch, skill_table[gsn].target );
}


void spell_essence_of_mist(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
 AFFECT_DATA af;
 CHAR_DATA *victim = (CHAR_DATA*)vo;

 if(ch!=victim)
 {
  send_to_char("Bug in spell_essence_of_mist: ch!=victim\n\r",ch); 
  return;
 }
 
 if(is_affected(ch,skill_lookup("essence of mist")) || is_affected(ch,skill_lookup("essence of magic"))
               ||is_affected(ch,skill_lookup("essence of life")))
 {
  send_to_char("The form of your body has already been changed.\n\r",ch);
  return;
 }
 /* res weapon */
 af.where     = TO_RESIST;
 af.type      = skill_lookup("essence of mist");
 af.level     = level;
 af.duration  = -1;
 af.location  = APPLY_NONE;
 af.modifier  = 0;
 af.bitvector = RES_WEAPON;
 if(!IS_SET(ch->res_flags,RES_WEAPON))
  affect_to_char( victim, &af );

 /* vuln magic */
 af.where     = TO_VULN;
 af.bitvector = VULN_MAGIC;
 if(!IS_SET(ch->vuln_flags,VULN_MAGIC))
  affect_to_char(victim, &af);

 /* racial dex max + 4 */
 af.where     = TO_AFFECTS;
 af.location  = APPLY_DEX;
 af.modifier  = 4;
 af.bitvector = 0;
 affect_to_char( victim, &af);

 /* dex + 4 */
 af.location  = APPLY_DEX;
 affect_to_char(victim, &af);

 /* racial str max -4 */
 af.location  = APPLY_STR;
 af.modifier  = -4;
 affect_to_char( victim, &af);

 send_to_char("{WM{ci{Cs{xt{Ws{x start to swirl as your body is transformed.\n\r",ch);
 act("{WM{ci{Cs{xt{Ws{x start to swirl as $n's body is transformed.",ch,NULL,NULL,TO_ROOM);

 return;
}

void spell_essence_of_magic(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
 AFFECT_DATA af;
 CHAR_DATA *victim = (CHAR_DATA*)vo;

 if(ch!=victim)
 {
  send_to_char("Bug in spell_essence_of_magic: ch!=victim\n\r",ch);
  return;
 }

 if(is_affected(ch,skill_lookup("essence of mist"))||is_affected(ch,skill_lookup("essence of magic"))
             ||is_affected(ch,skill_lookup("essence of life")))
 {
  send_to_char("The form of your body has already been changed.\n\r",ch);
  return;
 }
 /* res magic */
 af.where     = TO_RESIST;
 af.type      = skill_lookup("essence of magic");
 af.level     = level;
 af.duration  = -1;
 af.location  = APPLY_NONE;
 af.modifier  = 0;
 af.bitvector = RES_MAGIC;
 if(!IS_SET(ch->res_flags,RES_MAGIC))
  affect_to_char( victim, &af );

 /* sanctuary */
 af.where = TO_AFFECTS;
 af.bitvector = AFF_SANCTUARY;
 if(!IS_AFFECTED(ch,AFF_SANCTUARY))
  affect_to_char(victim, &af);

 /* vuln holy */
 af.where     = TO_VULN;
 af.bitvector = VULN_HOLY;
 if(!IS_SET(ch->vuln_flags,VULN_HOLY))
  affect_to_char(victim, &af);

 /* vuln fire */
 af.where     = TO_VULN;
 af.bitvector = VULN_FIRE;
 if(!IS_SET(ch->vuln_flags,VULN_FIRE))
  affect_to_char(victim, &af);

 /* race int max + 4 */
 af.where     = TO_AFFECTS;
 af.location  = APPLY_INT;
 af.modifier  = 4;
 af.bitvector = 0;
 affect_to_char( victim, &af);

 /* int + 4 */
 af.location  = APPLY_INT;
 affect_to_char(victim, &af);
 af.where     = TO_AFFECTS;

 /* race wis max + 4 */
 af.location  = APPLY_WIS;
 af.modifier  = 4;
 af.bitvector = 0;
 affect_to_char( victim, &af);

 /* wis + 4 */
 af.location  = APPLY_WIS;
 affect_to_char(victim, &af);

 send_to_char("Your voice turns to a piercing screech as your body is transformed.\n\r",ch);
 act("$n's voice turns to a piercing screech as $s body is transformed.",ch,NULL,NULL,TO_ROOM);

 return;
}

void spell_essence_of_life(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
 AFFECT_DATA af;
 CHAR_DATA *victim = (CHAR_DATA*)vo;

 if(ch!=victim)
 {
  send_to_char("Bug in spell_essence_of_life: ch!=victim\n\r",ch);
  return;
 }

 if(is_affected(ch,skill_lookup("essence of mist"))||is_affected(ch,skill_lookup("essence of magic"))
      ||is_affected(ch,skill_lookup("essence of life")))
 {
  send_to_char("The form of your body has already been changed.\n\r",ch);
  return;
 }
 /* res bash */
 af.where     = TO_RESIST;
 af.type      = skill_lookup("essence of life");
 af.level     = level;
 af.duration  = -1;
 af.location  = APPLY_NONE;
 af.modifier  = 0;
 af.bitvector = RES_BASH;
 if(!IS_SET(ch->res_flags,RES_BASH))
  affect_to_char( victim, &af );

 /* res pierce */
 af.bitvector = RES_PIERCE;
 if(!IS_SET(ch->res_flags,RES_PIERCE))
  affect_to_char(victim, &af);

 /* vuln silver */
 af.where     = TO_VULN;
 af.bitvector = VULN_SILVER;
 if(!IS_SET(ch->vuln_flags,VULN_SILVER))
  affect_to_char(victim, &af);

 af.where 	= TO_IMMUNE;
 af.bitvector   = IMM_FIRE;
 if (!IS_SET(ch->imm_flags,IMM_FIRE))
  affect_to_char(victim, &af);

 /* race str max + 4 */
 af.where     = TO_AFFECTS;
 af.location  = APPLY_STR;
 af.modifier  = 4;
 af.bitvector = 0;
 affect_to_char( victim, &af);

 /* str + 4 */
 af.location  = APPLY_STR;
 affect_to_char(victim, &af);
 af.where     = TO_AFFECTS;

 /* race con max + 4 */
 af.location  = APPLY_CON;
 af.modifier  = 4;
 af.bitvector = 0;
 affect_to_char( victim, &af);

 /* con + 4 */
 af.location  = APPLY_CON;
 affect_to_char(victim, &af);

 /* hp + level*5 */
 af.location  = APPLY_HIT;
 af.modifier  = level*50;
 affect_to_char(victim, &af);

 send_to_char("You growl menacingly as your body is transformed.\n\r",ch);
 act("$n growls menacingly as $s body is transformed.",ch,NULL,NULL,TO_ROOM);

 return;
}

void do_revert(CHAR_DATA *ch,char *argument)
{

 if(!is_affected(ch,skill_lookup("essence of mist"))
     && !is_affected(ch,skill_lookup("essence of magic"))
     && !is_affected(ch,skill_lookup("essence of life")))
 {
  send_to_char("You are already in your true form.\n\r",ch);
  return;
 }

 affect_strip(ch,skill_lookup("essence of mist"));
 affect_strip(ch,skill_lookup("essence of magic"));
 affect_strip(ch,skill_lookup("essence of life")); 
 send_to_char( "The world around you seems to shift and bend as you return to normal.\n\r", ch );
 act("$n returns to his true form.",ch,NULL,NULL,TO_ROOM);
 return;

}

/* Reflect magic back on caster */
void spell_mirror_shield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    AFFECT_DATA af;
    
    if (IS_AFFECTED2(ch,AFF_MIRROR_SHIELD))
    {
        send_to_char("You are already protected by a mirror shield.\n\r", ch);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    /* High duration is because it loses effectiveness as it's hit */
    af.duration  = level * 2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_MIRROR_SHIELD;
    
    affect_to_char(ch,&af);
    act("$n is surrounded by a shimmering mirror.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You are surrounded by a shimmering mirror.\n\r",ch);
    return;
}
