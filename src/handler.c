/**************************************************************************r
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
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "include.h"
#include "sort.h"

/*
 * Local functions.
 */
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
int	moon_status args (());

/* friend stuff -- for NPC's mostly */
bool is_friend(CHAR_DATA *ch,CHAR_DATA *victim)
{
    if (is_same_group(ch,victim))
	return TRUE;

    
    if (!IS_NPC(ch))
	return FALSE;

    if (!IS_NPC(victim))
    {
	if (IS_SET(ch->off_flags,ASSIST_PLAYERS))
	    return TRUE;
	else
	    return FALSE;
    }

    if (IS_AFFECTED(ch,AFF_CHARM))
	return FALSE;

    if (IS_SET(ch->off_flags,ASSIST_ALL))
	return TRUE;

    if (ch->group && ch->group == victim->group)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_VNUM) 
    &&  ch->pIndexData == victim->pIndexData)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_RACE) && ch->race == victim->race)
	return TRUE;
     
    if (IS_SET(ch->off_flags,ASSIST_ALIGN)
    &&  !IS_SET(ch->act,ACT_NOALIGN) && !IS_SET(victim->act,ACT_NOALIGN)
    &&  ((IS_GOOD(ch) && IS_GOOD(victim))
    ||	 (IS_EVIL(ch) && IS_EVIL(victim))
    ||   (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))))
	return TRUE;

    return FALSE;
}

/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
    CHAR_DATA *fch;
    int count = 0;

    if (obj->in_room == NULL)
	return 0;

    for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
	if (fch->on == obj)
	    count++;

    return count;
}
     
/* returns material number */
int material_lookup (const char *name)
{
    int mat;

    for ( mat = 0; material_type[mat].name != NULL && material_type[mat].name[0] != '\0'; mat++)
    {
	if (name[0] == material_type[mat].name[0]
	&& !str_prefix(name,material_type[mat].name))
	    return mat;
    }

    return -1;
}

/* returns material name -- ROM OLC temp patch */
char *material_name( sh_int num )
{
	return material_type[num].name;     
}

int weapon_lookup (const char *name)
{
    int type;

    for (type = 0; weapon_table[type].name != NULL; type++)
    {
	if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
	&&  !str_prefix(name,weapon_table[type].name))
	    return type;
    }
 
    return -1;
}

int weapon_type (const char *name)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
    {
        if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
        &&  !str_prefix(name,weapon_table[type].name))
            return weapon_table[type].type;
    }
 
    return WEAPON_EXOTIC;
}


char *item_name(int item_type)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
	if (item_type == item_table[type].type)
	    return item_table[type].name;
    return "none";
}

char *weapon_name( int weapon_type)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
        if (weapon_type == weapon_table[type].type)
            return weapon_table[type].name;
    return "exotic";
}

int attack_lookup  (const char *name)
{
    int att;

    for ( att = 0; attack_table[att].name != NULL; att++)
    {
	if (LOWER(name[0]) == LOWER(attack_table[att].name[0])
	&&  !str_prefix(name,attack_table[att].name))
	    return att;
    }

    return 0;
}

/* returns a flag for wiznet */
long wiznet_lookup (const char *name)
{
    int flag;

    for (flag = 0; wiznet_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0])
	&& !str_prefix(name,wiznet_table[flag].name))
	    return flag;
    }

    return -1;
}

/* returns klass number */
int class_lookup (const char *name)
{
   int klass;
 
   for ( klass = 0; klass < MAX_CLASS; klass++)
   {
        if (LOWER(name[0]) == LOWER(class_table[klass].name[0])
        &&  !str_prefix( name,class_table[klass].name))
            return klass;
   }
 
   return -1;
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune(CHAR_DATA *ch, int dam_type)
{
    int immune, def;
    int bit;

    immune = -1;
    def = IS_NORMAL;

    if (dam_type == DAM_NONE)
	return immune;

    if (dam_type <= 3)
    {
	if (IS_SET(ch->imm_flags,IMM_WEAPON))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_WEAPON))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_WEAPON))
	    def = IS_VULNERABLE;
    }
    else /* magical attack */
    {	
	if (IS_SET(ch->imm_flags,IMM_MAGIC))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_MAGIC))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_MAGIC))
	    def = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    switch (dam_type)
    {
	case(DAM_BASH):		bit = IMM_BASH;		break;
	case(DAM_PIERCE):	bit = IMM_PIERCE;	break;
	case(DAM_SLASH):	bit = IMM_SLASH;	break;
	case(DAM_FIRE):		bit = IMM_FIRE;		break;
	case(DAM_COLD):		bit = IMM_COLD;		break;
	case(DAM_LIGHTNING):	bit = IMM_LIGHTNING;	break;
	case(DAM_ACID):		bit = IMM_ACID;		break;
	case(DAM_POISON):	bit = IMM_POISON;	break;
	case(DAM_NEGATIVE):	bit = IMM_NEGATIVE;	break;
	case(DAM_HOLY):		bit = IMM_HOLY;		break;
	case(DAM_ENERGY):	bit = IMM_ENERGY;	break;
	case(DAM_MENTAL):	bit = IMM_MENTAL;	break;
	case(DAM_DISEASE):	bit = IMM_DISEASE;	break;
	case(DAM_DROWNING):	bit = IMM_DROWNING;	break;
	case(DAM_LIGHT):	bit = IMM_LIGHT;	break;
	case(DAM_CHARM):	bit = IMM_CHARM;	break;
	case(DAM_SOUND):	bit = IMM_SOUND;	break;
        case(DAM_SILVER):       bit = IMM_SILVER;       break;
	default:		return def;
    }

    if (IS_SET(ch->imm_flags,bit))
	immune = IS_IMMUNE;
    else if (IS_SET(ch->res_flags,bit) && immune != IS_IMMUNE)
	immune = IS_RESISTANT;
    else if (IS_SET(ch->vuln_flags,bit))
    {
	if (immune == IS_IMMUNE)
	    immune = IS_RESISTANT;
	else if (immune == IS_RESISTANT)
	    immune = IS_NORMAL;
	else
	    immune = IS_VULNERABLE;
    }

    if (immune == -1)
	return def;
    else
      	return immune;
}

/* checks mob format */
bool is_old_mob(CHAR_DATA *ch)
{
    if (ch->pIndexData == NULL)
	return FALSE;
    else if (ch->pIndexData->new_format)
	return FALSE;
    return TRUE;
}
 
/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
    int skill;

    if (sn == -1) /* shorthand for level based skills */
    {
	skill = ( ( ch->level * 5 ) / 2 ) + ch->cast_ability / 1;
    }

    else if (sn < -1 || sn > MAX_SKILL)
    {
	bug("Bad sn %d in get_skill.",sn);
	skill = 0;
    }

    else if (!IS_NPC(ch))
    {
	if (!can_use_skpell(ch, sn))
	    skill = 0;
	else
	    skill = ch->pcdata->learned[sn];
    }

    else /* mobiles */
    {


        if (skill_table[sn].spell_fun != spell_null)
	    skill = 40 + 2 * ch->level;

	else if (sn == skill_lookup("sneak") || sn == skill_lookup("hide"))
	    skill = ch->level * 2 + 20;

        else if ((sn == skill_lookup("dodge") && IS_SET(ch->off_flags,OFF_DODGE))
 	||       (sn == skill_lookup("parry") && IS_SET(ch->off_flags,OFF_PARRY)))
	    skill = ch->level * 2;

 	else if (sn == skill_lookup("shield block"))
	    skill = 10 + 2 * ch->level;

	else if (sn == skill_lookup("second attack") 
	&& (IS_SET(ch->act,ACT_WARRIOR) || IS_SET(ch->act,ACT_THIEF)))
	    skill = 10 + 3 * ch->level;

	else if (sn == skill_lookup("third attack") && IS_SET(ch->act,ACT_WARRIOR))
	    skill = 4 * ch->level - 40;

        else if (sn == skill_lookup("fourth attack") && IS_SET(ch->act,ACT_WARRIOR))
            skill = 4 * ch->level -40;

	else if (sn == skill_lookup("hand to hand"))
	    skill = 40 + 2 * ch->level;

 	else if (sn == skill_lookup("trip") && IS_SET(ch->off_flags,OFF_TRIP))
	    skill = 10 + 3 * ch->level;

 	else if (sn == skill_lookup("bash") && IS_SET(ch->off_flags,OFF_BASH))
	    skill = 10 + 3 * ch->level;

	else if (sn == skill_lookup("disarm") 
	     &&  (IS_SET(ch->off_flags,OFF_DISARM) 
	     ||   IS_SET(ch->act,ACT_WARRIOR)
	     ||	  IS_SET(ch->act,ACT_THIEF)))
	    skill = 20 + 3 * ch->level;

	else if (sn == skill_lookup("berserk") && IS_SET(ch->off_flags,OFF_BERSERK))
	    skill = 3 * ch->level;

        else if ( sn == skill_lookup("bloodlust") && IS_SET(ch->off_flags,OFF_BLOODLUST))                                       
            skill = 10 + 3 * ch->level;
                                       
	else if (sn == skill_lookup("kick"))
	    skill = 10 + 3 * ch->level;

	else if (sn == skill_lookup("backstab") && IS_SET(ch->act,ACT_THIEF))
	    skill = 20 + 2 * ch->level;

  	else if (sn == skill_lookup("rescue"))
	    skill = 40 + ch->level; 

	else if (sn == skill_lookup("recall"))
	    skill = 40 + ch->level;

	else if (sn == skill_lookup("sword")
	||  sn == skill_lookup("dagger")
	||  sn == skill_lookup("spear")
	||  sn == skill_lookup("mace")
	||  sn == skill_lookup("axe")
	||  sn == skill_lookup("flail")
	||  sn == skill_lookup("whip")
	||  sn == skill_lookup("polearm")
        ||  sn == skill_lookup("bow"))
	    skill = 40 + 5 * ch->level / 2;

	else 
	   skill = 0;
    }

    if (ch->daze > 0)
    {
	if (skill_table[sn].spell_fun != spell_null)
	    skill /= 2;
	else
	    skill = 2 * skill / 3;
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	skill = 9 * skill / 10;

    return URANGE(0,skill,100);
}

/* for returning weapon information */
int get_weapon_sn(CHAR_DATA *ch)
{
    OBJ_DATA *wield;
    int sn;

    wield = get_eq_char( ch, WEAR_WIELD );
    if (wield == NULL || wield->item_type != ITEM_WEAPON)
        sn = skill_lookup("hand to hand");
    else switch (wield->value[0])
    {
        default :               sn = -1;                break;
        case(WEAPON_SWORD):     sn = skill_lookup("sword");         break;
        case(WEAPON_DAGGER):    sn = skill_lookup("dagger");        break;
        case(WEAPON_SPEAR):     sn = skill_lookup("spear");         break;
        case(WEAPON_MACE):      sn = skill_lookup("mace");          break;
        case(WEAPON_AXE):       sn = skill_lookup("axe");           break;
        case(WEAPON_FLAIL):     sn = skill_lookup("flail");         break;
        case(WEAPON_WHIP):      sn = skill_lookup("whip");          break;
        case(WEAPON_POLEARM):   sn = skill_lookup("polearm");       break;
        case(WEAPON_BOW):       sn = skill_lookup("bow");           break;      
   }
   return sn;
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
     int skill;

     /* -1 is exotic */
    if (IS_NPC(ch))
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else if (sn == skill_lookup("hand to hand"))
	    skill = 40 + 2 * ch->level;
	else 
	    skill = 40 + 5 * ch->level / 2;
    }
    
    else
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else
	    skill = ch->pcdata->learned[sn];
    }

    return URANGE(0,skill,100);
} 


/* used to de-screw characters */
void reset_char(CHAR_DATA *ch)
{
     int loc,mod,stat;
     OBJ_DATA *obj;
     AFFECT_DATA *af;
     int i;

     if (IS_NPC(ch))
	return;

    if (ch->pcdata->perm_hit == 0 
    ||	ch->pcdata->perm_mana == 0
    ||  ch->pcdata->perm_move == 0
    ||	ch->pcdata->last_level == 0)
    {
    /* do a FULL reset */
	for (loc = 0; loc < MAX_WEAR; loc++)
	{
	    obj = get_eq_char(ch,loc);
	    if (obj == NULL)
		continue;
	    if (!obj->enchanted)
	    for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	    {
		mod = af->modifier;
		switch(af->location)
		{
		    case APPLY_SEX:	ch->sex		-= mod;
					if (ch->sex < 0 || ch->sex >2)
					    ch->sex = IS_NPC(ch) ?
						0 :
						ch->pcdata->true_sex;
									break;
		    case APPLY_MANA:	ch->max_mana	-= mod;		break;
		    case APPLY_HIT:	ch->max_hit	-= mod;		break;
		    case APPLY_MOVE:	ch->max_move	-= mod;		break;
		}
	    }

            for ( af = obj->affected; af != NULL; af = af->next )
            {
                mod = af->modifier;
                switch(af->location)
                {
                    case APPLY_SEX:     ch->sex         -= mod;         break;
                    case APPLY_MANA:    ch->max_mana    -= mod;         break;
                    case APPLY_HIT:     ch->max_hit     -= mod;         break;
                    case APPLY_MOVE:    ch->max_move    -= mod;         break;
                }
            }
	}
	/* now reset the permanent stats */
	ch->pcdata->perm_hit 	= ch->max_hit;
	ch->pcdata->perm_mana 	= ch->max_mana;
	ch->pcdata->perm_move	= ch->max_move;
	ch->pcdata->last_level	= ch->played/3600;
	if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
        {
		if (ch->sex > 0 && ch->sex < 3)
	    	    ch->pcdata->true_sex	= ch->sex;
		else
		    ch->pcdata->true_sex 	= 0;
        }

    }

    /* now restore the character to his/her true condition */
    for (stat = 0; stat < MAX_STATS; stat++)
	ch->mod_stat[stat] = 0;

    if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
	ch->pcdata->true_sex = 0; 
    ch->sex		= ch->pcdata->true_sex;
    ch->max_hit 	= ch->pcdata->perm_hit;
    ch->max_mana	= ch->pcdata->perm_mana;
    ch->max_move	= ch->pcdata->perm_move;
   
    for (i = 0; i < 4; i++)
    	ch->armor[i]	= 100;

    ch->hitroll		= 0;
    ch->damroll		= 0;
    ch->cast_ability    = 0;
    ch->saving_throw	= 0;
    ch->cast_level      = 0;

    /* now start adding back the effects */
    for (loc = 0; loc < MAX_WEAR; loc++)
    {
        obj = get_eq_char(ch,loc);
        if (obj == NULL)
            continue;
	for (i = 0; i < 4; i++)
	    ch->armor[i] -= apply_ac( obj, loc, i );

        if (!obj->enchanted)
	for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;
            switch(af->location)
            {
		case APPLY_STR:		ch->mod_stat[STAT_STR]	+= mod;	break;
		case APPLY_DEX:		ch->mod_stat[STAT_DEX]	+= mod; break;
		case APPLY_INT:		ch->mod_stat[STAT_INT]	+= mod; break;
		case APPLY_WIS:		ch->mod_stat[STAT_WIS]	+= mod; break;
		case APPLY_CON:		ch->mod_stat[STAT_CON]	+= mod; break;

		case APPLY_SEX:		ch->sex			+= mod; break;
		case APPLY_MANA:	ch->max_mana		+= mod; break;
		case APPLY_HIT:		ch->max_hit		+= mod; break;
		case APPLY_MOVE:	ch->max_move		+= mod; break;

		case APPLY_AC:		
		    for (i = 0; i < 4; i ++)
			ch->armor[i] += mod; 
		    break;
		case APPLY_HITROLL:	ch->hitroll		+= mod; break;
		case APPLY_DAMROLL:	ch->damroll		+= mod; break;
  	
		case APPLY_SAVES:		ch->saving_throw += mod; break;
		case APPLY_SAVING_ROD: 		ch->saving_throw += mod; break;
		case APPLY_SAVING_PETRI:	ch->saving_throw += mod; break;
		case APPLY_SAVING_BREATH: 	ch->saving_throw += mod; break;
		case APPLY_SAVING_SPELL:	ch->saving_throw += mod; break;
                case APPLY_CAST_LEVEL:          ch->cast_level   += mod; break;
                case APPLY_CAST_ABILITY:        ch->cast_ability += mod; break;

	    }
        }
 
        for ( af = obj->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;
            switch(af->location)
            {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
                case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
 
                case APPLY_SEX:         ch->sex                 += mod; break;
                case APPLY_MANA:        ch->max_mana            += mod; break;
                case APPLY_HIT:         ch->max_hit             += mod; break;
                case APPLY_MOVE:        ch->max_move            += mod; break;
 
                case APPLY_AC:
                    for (i = 0; i < 4; i ++)
                        ch->armor[i] += mod;
                    break;
		case APPLY_HITROLL:     ch->hitroll             += mod; break;
                case APPLY_DAMROLL:     ch->damroll             += mod; break;

                case APPLY_SAVES:         ch->saving_throw += mod; break;
                case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
                case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
                case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
                case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
                case APPLY_CAST_LEVEL:  ch->cast_level           += mod; break;
                case APPLY_CAST_ABILITY: ch->cast_ability        += mod; break; 
            }
	}
    }
  
    /* now add back spell effects */
    for (af = ch->affected; af != NULL; af = af->next)
    {
        mod = af->modifier;
        switch(af->location)
        {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
                case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
 
                case APPLY_SEX:         ch->sex                 += mod; break;
                case APPLY_MANA:        ch->max_mana            += mod; break;
                case APPLY_HIT:         ch->max_hit             += mod; break;
                case APPLY_MOVE:        ch->max_move            += mod; break;
 
                case APPLY_AC:
                    for (i = 0; i < 4; i ++)
                        ch->armor[i] += mod;
                    break;
                case APPLY_HITROLL:     ch->hitroll             += mod; break;
                case APPLY_DAMROLL:     ch->damroll             += mod; break;
                
 
                case APPLY_SAVES:         ch->saving_throw += mod; break;
                case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
                case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
                case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
                case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
                case APPLY_CAST_LEVEL:  ch->cast_level          += mod; break;
                case APPLY_CAST_ABILITY: ch->cast_ability       += mod; break; 
        } 
    }

    /* make sure sex is RIGHT!!!! */
    if (ch->sex < 0 || ch->sex > 2)
	ch->sex = ch->pcdata->true_sex;
}


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    if (ch->trust)
	return ch->trust;

    if ( IS_NPC(ch) && ch->level >= LEVEL_HERO )
	return LEVEL_HERO - 1;
    else
	return ch->level;
}


/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
    return 17 + ( ch->played + (int) (current_time - ch->logon) ) / 72000;
}

/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat )
{
    int max;

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
	max = 50;

    else
    {
	max = pc_race_table[ch->race].max_stats[stat] + 4;

	if (is_prime_stat(ch, stat))
	    max += 2;

	if ( ch->race == race_lookup("human"))
	    max += 1;

 	max = UMIN(max,50);
    }
  
    return URANGE(3,ch->perm_stat[stat] + ch->mod_stat[stat], max);
}

/* command for returning max training score */
int get_max_train( CHAR_DATA *ch, int stat )
{
    int max;

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
	return 50;

    max = pc_race_table[ch->race].max_stats[stat];
    if (is_prime_stat(ch, stat))  
    {
	if (ch->race == race_lookup("human"))
	   max += 3;
	else
	   max += 2;
    }

    return UMIN(max,50);
}
   
	
/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 1000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return MAX_WEAR +  2 * get_curr_stat(ch,STAT_DEX) + ch->level;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 10000000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return str_app[get_curr_stat(ch,STAT_STR)].carry * 10 + ch->level * 25;
}



/*
 * See if a string is one of the names of an object.
 */

bool is_name ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;

    /* fix crash on NULL namelist */
    if (namelist == NULL || namelist[0] == '\0')
    	return FALSE;

    /* fixed to prevent is_name on "" returning TRUE */
    if (str[0] == '\0')
	return FALSE;

    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument(str,part);

	if (part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list,name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

	    if (!str_prefix(string,name))
		return TRUE; /* full pattern match */

	    if (!str_prefix(part,name))
		break;
	}
    }
}

bool is_exact_name(char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    if (namelist == NULL)
	return FALSE;

    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( name[0] == '\0' )
	    return FALSE;
	if ( !str_cmp( str, name ) )
	    return TRUE;
    }
}

/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
        AFFECT_DATA *paf, *af_new;
        obj->enchanted = TRUE;

        for (paf = obj->pIndexData->affected;
             paf != NULL; paf = paf->next)
        {
	    af_new = new_affect();

            af_new->next = obj->affected;
            obj->affected = af_new;
 
	    af_new->where	= paf->where;
            af_new->type        = UMAX(0,paf->type);
            af_new->level       = paf->level;
            af_new->duration    = paf->duration;
            af_new->location    = paf->location;
            af_new->modifier    = paf->modifier;
            af_new->bitvector   = paf->bitvector;
        }
    }
}
           

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    int mod,i;

    mod = paf->modifier;

    if ( fAdd )
    {
	switch (paf->where)
	{
	case TO_AFFECTS:
	    SET_BIT(ch->affected_by, paf->bitvector);
	    break;
	case TO_IMMUNE:
	    SET_BIT(ch->imm_flags,paf->bitvector);
	    break;
	case TO_RESIST:
	    SET_BIT(ch->res_flags,paf->bitvector);
	    break;
	case TO_VULN:
	    SET_BIT(ch->vuln_flags,paf->bitvector);
	    break;
        case TO_AFFECTS2:
            SET_BIT(ch->affected2_by, paf->bitvector);
	}
    }
    else
    {
        switch (paf->where)
        {
        case TO_AFFECTS:
            REMOVE_BIT(ch->affected_by, paf->bitvector);
            break;
        case TO_IMMUNE:
            REMOVE_BIT(ch->imm_flags,paf->bitvector);
            break;
        case TO_RESIST:
            REMOVE_BIT(ch->res_flags,paf->bitvector);
            break;
        case TO_VULN:
            REMOVE_BIT(ch->vuln_flags,paf->bitvector);
            break;
        case TO_AFFECTS2:
            REMOVE_BIT(ch->affected2_by, paf->bitvector);
            break;   
        }
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
	bug( "Affect_modify: unknown location %d.", paf->location );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:           ch->mod_stat[STAT_STR]	+= mod;	break;
    case APPLY_DEX:           ch->mod_stat[STAT_DEX]	+= mod;	break;
    case APPLY_INT:           ch->mod_stat[STAT_INT]	+= mod;	break;
    case APPLY_WIS:           ch->mod_stat[STAT_WIS]	+= mod;	break;
    case APPLY_CON:           ch->mod_stat[STAT_CON]	+= mod;	break;
    case APPLY_SEX:           ch->sex			+= mod;	break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:	                                  	break;
    case APPLY_AGE:						break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana		+= mod;	break;
    case APPLY_HIT:           ch->max_hit		+= mod;	break;
    case APPLY_MOVE:          ch->max_move		+= mod;	break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:
        for (i = 0; i < 4; i ++)
            ch->armor[i] += mod;
        break;
    case APPLY_HITROLL:       ch->hitroll		+= mod;	break;
    case APPLY_DAMROLL:       ch->damroll		+= mod;	break;
    case APPLY_CAST_LEVEL:    ch->cast_level            += mod; break;
    case APPLY_CAST_ABILITY: ch->cast_ability           += mod; break; 
    case APPLY_SAVES:   ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_ROD:    ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_PETRI:  ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_BREATH: ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_SPELL:  ch->saving_throw		+= mod;	break;
    case APPLY_SPELL_AFFECT:  					break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( !IS_NPC(ch) && ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
    &&   get_obj_weight(wield) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10))
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act( "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
	    wield->carried_by = ch;
	    obj_to_room( wield, ch->in_room );
	    depth--;
	}
    }

    return;
}


/* find an effect in an affect list */
AFFECT_DATA  *affect_find(AFFECT_DATA *paf, int sn)
{
    AFFECT_DATA *paf_find;
    
    for ( paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
    {
        if ( paf_find->type == sn )
	return paf_find;
    }

    return NULL;
}

/* fix object affects when removing one */
void affect_check(CHAR_DATA *ch,int where,int vector)
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
	return;

    for (paf = ch->affected; paf != NULL; paf = paf->next)
	if (paf->where == where && paf->bitvector == vector)
	{
	    switch (where)
	    {
	        case TO_AFFECTS:
		    SET_BIT(ch->affected_by,vector);
		    break;
                case TO_AFFECTS2:
                    SET_BIT(ch->affected2_by, vector);
                    break;
	        case TO_IMMUNE:
		    SET_BIT(ch->imm_flags,vector);   
		    break;
	        case TO_RESIST:
		    SET_BIT(ch->res_flags,vector);
		    break;
	        case TO_VULN:
		    SET_BIT(ch->vuln_flags,vector);
		    break;
	    }
	    return;
	}

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
	if (obj->wear_loc == -1)
	    continue;

            for (paf = obj->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector)
            {
                switch (where)
                {
                    case TO_AFFECTS:
                        SET_BIT(ch->affected_by,vector);
                        break;
                    case TO_IMMUNE:
                        SET_BIT(ch->imm_flags,vector);
                        break;
                    case TO_RESIST:
                        SET_BIT(ch->res_flags,vector);
                        break;
                    case TO_VULN:
                        SET_BIT(ch->vuln_flags,vector);
                    case TO_AFFECTS2:
                        SET_BIT(ch->affected2_by,vector);
                        break; 
                }
                return;
            }

        if (obj->enchanted)
	    continue;

        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector)
            {
                switch (where)
                {
                    case TO_AFFECTS:
                        SET_BIT(ch->affected_by,vector);
                        break;
                    case TO_IMMUNE:
                        SET_BIT(ch->imm_flags,vector);
                        break;
                    case TO_RESIST:
                        SET_BIT(ch->res_flags,vector);
                        break;
                    case TO_VULN:
                        SET_BIT(ch->vuln_flags,vector);
                        break;
                    case TO_AFFECTS2:
                        SET_BIT(ch->affected2_by,vector);
                        break;                     
                }
                return;
            }
    }
}

/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;

    VALIDATE(paf_new);	/* in case we missed it when we set up paf */
    paf_new->next	= ch->affected;
    ch->affected	= paf_new;

    affect_modify( ch, paf_new, TRUE );
    return;
}
/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;

    VALIDATE(paf_new);	/* in case we missed it when we set up paf */
    paf_new->next	= obj->affected;
    obj->affected	= paf_new;

    /* apply any affect vectors to the object's extra_flags */
    if (paf->bitvector)
        switch (paf->where)
        {
        case TO_OBJECT:
    	    SET_BIT(obj->extra_flags,paf->bitvector);
	    break;
        case TO_OBJECT2:
            SET_BIT(obj->extra2_flags,paf->bitvector);
        case TO_WEAPON:
	    if (obj->item_type == ITEM_WEAPON)
	        SET_BIT(obj->value[4],paf->bitvector);
	    break;
        }
    

    return;
}



/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    int where;
    int vector;

    if ( ch->affected == NULL )
    {
	bug( "Affect_remove: no affect.", 0 );
	return;
    }

    affect_modify( ch, paf, FALSE );
    where = paf->where;
    vector = paf->bitvector;

    if ( paf == ch->affected )
    {
	ch->affected	= paf->next;
    }
    else
    {
	AFFECT_DATA *prev;

	for ( prev = ch->affected; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == paf )
	    {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Affect_remove: cannot find paf.", 0 );
	    return;
	}
    }

    free_affect(paf);

    affect_check(ch,where,vector);
    return;
}

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf)
{
    int where, vector;
    if ( obj->affected == NULL )
    {
        bug( "Affect_remove_object: no affect.", 0 );
        return;
    }

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_modify( obj->carried_by, paf, FALSE );

    where = paf->where;
    vector = paf->bitvector;

    /* remove flags from the object if needed */
    if (paf->bitvector)
	switch( paf->where)
        {
        case TO_OBJECT:
            REMOVE_BIT(obj->extra_flags,paf->bitvector);
            break;
        case TO_OBJECT2:
            REMOVE_BIT(obj->extra2_flags,paf->bitvector);
        case TO_WEAPON:
            if (obj->item_type == ITEM_WEAPON)
                REMOVE_BIT(obj->value[4],paf->bitvector);
            break;
        }

    if ( paf == obj->affected )
    {
        obj->affected    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = obj->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Affect_remove_object: cannot find paf.", 0 );
            return;
        }
    }

    free_affect(paf);

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_check(obj->carried_by,where,vector);
    return;
}



/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }

    return;
}



/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;
    bool found;

    found = FALSE;
    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type )
	{
	    paf->level = (paf->level += paf_old->level) / 2;
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );
    return;
}




/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ch->in_room == NULL )
    {
	bug( "Char_from_room: NULL.", 0 );
	return;
    }

    if ( !IS_NPC(ch) )
	--ch->in_room->area->nplayer;

    if( !IS_NPC(ch) && IN_MINE(ch) )
	exit_mine(ch);

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    if ( ch == ch->in_room->people )
    {
	ch->in_room->people = ch->next_in_room;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
	{
	    if ( prev->next_in_room == ch )
	    {
		prev->next_in_room = ch->next_in_room;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Char_from_room: ch not found.", 0 );
    }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->on 	     = NULL;  /* sanity check! */
    return;
}



/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;

    if ( pRoomIndex == NULL )
    {
	ROOM_INDEX_DATA *room;

	bug( "Char_to_room: NULL.", 0 );
	
	if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
	    char_to_room(ch,room);
	
	return;
    }

    ch->in_room		= pRoomIndex;
    ch->next_in_room	= pRoomIndex->people;
    pRoomIndex->people	= ch;
    check_explore(ch, pRoomIndex); //Explore the room
    if ( !IS_NPC(ch) )
    {
	if (ch->in_room->area->empty)
	{
	    ch->in_room->area->empty = FALSE;
	    ch->in_room->area->age = 0;
	}
	++ch->in_room->area->nplayer;
    }

    if ( !IS_NPC(ch) )
    {
        if ( ch->in_room->area->soundfile[0] != '\0' )
        {
            music( ch->in_room->area->soundfile, ch, TRUE );
        }
        else
        {
            free_string (ch->pcdata->mplaying);
            ch->pcdata->mplaying = str_dup ( "None" );
            stop_music ( ch->desc );
        }
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0 )
	++ch->in_room->light;
	
    if (IS_AFFECTED(ch,AFF_PLAGUE))
    {
        AFFECT_DATA *af, plague;
        CHAR_DATA *vch;
        
        for ( af = ch->affected; af != NULL; af = af->next )
        {
            if (af->type == skill_lookup("plague"))
                break;
        }
        
        if (af == NULL)
        {
            REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            return;
        }
        
        if (af->level == 1)
            return;
        
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
	    &&  !IS_IMMORTAL(vch) &&
            	!IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(6) == 0)
            {
            	send_to_char("You feel hot and feverish.\n\r",vch);
            	act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            	affect_join(vch,&plague);
            }
        }
    }

    if (!IS_NPC(ch) && ch->pcdata->track_count>0)
    {
        DESCRIPTOR_DATA *d;

        for ( d = descriptor_list; d != NULL; d= d->next )
        {
            CHAR_DATA *rch;

            rch = d->original ? d->original : d->character;

            if (d->connected == CON_PLAYING &&
                rch->pcdata->tracking == ch &&
                rch->in_room != ch->in_room )
            {
                if (room_is_private(ch->in_room) &&
                    !IS_TRUSTED(rch,IMPLEMENTOR) &&
                    !is_room_owner(rch,ch->in_room) )
                {
		  if(!d->original)
                    send_to_char("You cannot track into a private room.\n\r",
                                rch);
		  else
                    send_to_char("Your body fails to track into a private room.\n\r",
				d->character);
                  continue;
                }
                char_from_room(rch);
                char_to_room(rch,pRoomIndex);
                if(!d->original)
                  send_to_char("You continue tracking...\n\r",rch);
                else
                  send_to_char("Your body continues tracking...\n\r",
                                d->character);
           }
        }
    }

    return;
}



/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );
    obj->in_shaft 	= NULL;
    if (!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_QUEST))
	update_questobjs(ch, obj);
}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;

    if ( ( ch = obj->carried_by ) == NULL )
    {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    if ( ch->carrying == obj )
    {
	ch->carrying = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Obj_from_char: obj not in list.", 0 );
    }
    obj->in_shaft	= NULL;
    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;
    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );
    return;
}



/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear, int type )
{
    if ( obj->item_type != ITEM_ARMOR )
	return 0;

    switch ( iWear )
    {
    case WEAR_BODY:	return 3 * obj->value[type];
    case WEAR_HEAD:	return 2 * obj->value[type];
    case WEAR_LEGS:	return 2 * obj->value[type];
    case WEAR_FEET:	return     obj->value[type];
    case WEAR_HANDS:	return     obj->value[type];
    case WEAR_ARMS:	return     obj->value[type];
    case WEAR_SHOULDER: return     obj->value[type];
    case WEAR_SHIELD:	return     obj->value[type];
    case WEAR_NECK_1:	return     obj->value[type];
    case WEAR_NECK_2:	return     obj->value[type];
    case WEAR_ABOUT:	return 2 * obj->value[type];
    case WEAR_WAIST:	return     obj->value[type];
    case WEAR_WRIST_L:	return     obj->value[type];
    case WEAR_WRIST_R:	return     obj->value[type];
    case WEAR_HOLD:	return     obj->value[type];
    case WEAR_FINGER_L: return     obj->value[type];
    case WEAR_FINGER_R: return     obj->value[type];
    case WEAR_WEDDING:  return     obj->value[type];
    case WEAR_FLOAT:    return     obj->value[type];
    case WEAR_LIGHT:    return     obj->value[type];
    case WEAR_WIELD:    return     obj->value[type];
    case WEAR_SECONDARY:return     obj->value[type];
    default:            return	   obj->value[type];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}

struct fix_type
{	int wear;
	int bit;
};

const struct fix_type fix_table[] =
{
	{ WEAR_HOOD,			ITEM_WEAR_HOOD	},
	{ WEAR_HEAD,			ITEM_WEAR_HEAD	},
	{ WEAR_CLOAK,			ITEM_WEAR_CLOAK	},
	{ WEAR_NECK_1,			ITEM_WEAR_NECK	},
	{ WEAR_NECK_2,			ITEM_WEAR_NECK	},	
	{ WEAR_ABOUT,			ITEM_WEAR_ABOUT	},
	{ WEAR_BACK,			ITEM_WEAR_BACK	},
	{ WEAR_SHOULDER,		ITEM_WEAR_SHOULDER	},
	{ WEAR_LAYER_ARMS,		ITEM_LAYER_ARMS	},	
	{ WEAR_ARMS,			ITEM_WEAR_ARMS	},
	{ WEAR_WRIST_L,			ITEM_WEAR_WRIST	},
	{ WEAR_WRIST_R,			ITEM_WEAR_WRIST	},
	{ WEAR_HANDS,			ITEM_WEAR_HANDS	},
	{ WEAR_FINGER_L,		ITEM_WEAR_FINGER	},		
	{ WEAR_FINGER_R,		ITEM_WEAR_FINGER	},
        { WEAR_WEDDING,                 ITEM_WEAR_WEDDING },
	{ WEAR_LAYER_BODY,		ITEM_LAYER_BODY	},
	{ WEAR_BODY,			ITEM_WEAR_BODY	},
	{ WEAR_WAIST,			ITEM_WEAR_WAIST	},
	{ WEAR_LAYER_LEGS,		ITEM_LAYER_LEGS	},	
	{ WEAR_LEGS,			ITEM_WEAR_LEGS	},
	{ WEAR_FEET,			ITEM_WEAR_FEET	},
	{ WEAR_SHIELD,			ITEM_WEAR_SHIELD	},
	{ WEAR_WIELD,			ITEM_WIELD		},
	{ WEAR_HOLD,			ITEM_HOLD		},
	{ WEAR_FLOAT,			ITEM_WEAR_FLOAT	},
	{ -1, -1 }
};

/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;
    int i, n;

    if ( get_eq_char( ch, iWear ) != NULL )
    {
	sprintf(log_buf, "Equip_char: already equipped (%d)(%s)(%s).",  iWear,ch->name,obj->name );
	log_string(log_buf); 
        return;
    }
		for( i = 0; fix_table[i].bit != -1 ; i++ )
	{	if( fix_table[i].wear == iWear )
			if( !IS_SET(obj->wear_flags, fix_table[i].bit ) )
			{	for( n = 0; fix_table[n].bit != -1; n++ )
				{	if( IS_SET(obj->wear_flags, fix_table[n].bit ) )
					{	iWear = fix_table[n].wear;
						break;
					}
				}
			}
	}

    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
    {
	/*
	 * Thanks to Morgenes for the bug fix here!
	 */
	act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
	act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
	obj_from_char( obj );
        obj->carried_by = ch;
	obj_to_room( obj, ch->in_room );
	return;
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]      	-= apply_ac( obj, iWear,i );
    obj->wear_loc	 = iWear;

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    if ( paf->location != APPLY_SPELL_AFFECT )
	        affect_modify( ch, paf, TRUE );
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT )
    	    affect_to_char ( ch, paf );
	else
	    affect_modify( ch, paf, TRUE );

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL )
	++ch->in_room->light;

    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf = NULL;
    AFFECT_DATA *lpaf = NULL;
    AFFECT_DATA *lpaf_next = NULL;
    int i;

    if ( obj->wear_loc == WEAR_NONE )
    {
	bug( "Unequip_char: already unequipped.", 0 );
	return;
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]	+= apply_ac( obj, obj->wear_loc,i );
    obj->wear_loc	 = -1;

    if (!obj->enchanted)
    {
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    if ( paf->location == APPLY_SPELL_AFFECT )
	    {
	        for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
	        {
		    lpaf_next = lpaf->next;
		    if ((lpaf->type == paf->type) &&
		        (lpaf->level == paf->level) &&
		        (lpaf->location == APPLY_SPELL_AFFECT))
		    {
		        affect_remove( ch, lpaf );
			lpaf_next = NULL;
		    }
	        }
	    }
	    else
	    {
	        affect_modify( ch, paf, FALSE );
		affect_check(ch,paf->where,paf->bitvector);
	    }
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT )
	{
	    bug ( "Norm-Apply: %d", 0 );
	    for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
	    {
		lpaf_next = lpaf->next;
		if ((lpaf->type == paf->type) &&
		    (lpaf->level == paf->level) &&
		    (lpaf->location == APPLY_SPELL_AFFECT))
		{
		    bug ( "location = %d", lpaf->location );
		    bug ( "type = %d", lpaf->type );
		    affect_remove( ch, lpaf );
		    lpaf_next = NULL;
		}
	    }
	}
	else
	{
	    affect_modify( ch, paf, FALSE );
	    affect_check(ch,paf->where,paf->bitvector);	
	}

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;
    CHAR_DATA *ch;

    if ( ( in_room = obj->in_room ) == NULL )
    {
	bug( "obj_from_room: NULL.", 0 );
	return;
    }

    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
	if (ch->on == obj)
	    ch->on = NULL;

    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }
    obj->in_shaft     = NULL;
    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{

    if (IS_OBJ_STAT (obj, ITEM_AUCTIONED))
       return;

    obj->next_content		= pRoomIndex->contents;
    pRoomIndex->contents	= obj;
    obj->in_room		= pRoomIndex;
    if(obj->carried_by && !IS_NPC(obj->carried_by) && IN_MINE(obj->carried_by ) )
    	obj->in_shaft		= obj->carried_by->pcdata->in_shaft;
    obj->carried_by		= NULL;
    obj->in_obj			= NULL;

    if (IS_SET(pRoomIndex->room_flags, ROOM_DONATION))
        obj->cost = 0; 
      
    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{

    if (IS_OBJ_STAT (obj, ITEM_AUCTIONED))
      return;

    obj->next_content		= obj_to->contains;
    obj_to->contains		= obj;
    obj->in_obj			= obj_to;
    obj->in_room		= NULL;
    obj->carried_by		= NULL;
   
    for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
	if ( obj_to->carried_by != NULL )
	{
	    obj_to->carried_by->carry_number += get_obj_number( obj );
	    obj_to->carried_by->carry_weight += get_obj_weight( obj )
		* WEIGHT_MULT(obj_to) / 100;
	}
    }

    return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    if ( obj == obj_from->contains )
    {
	obj_from->contains = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = obj_from->contains; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_obj: obj not found.", 0 );
	    return;
	}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
    {
	if ( obj_from->carried_by != NULL )
	{
	    obj_from->carried_by->carry_number -= get_obj_number( obj );
	    obj_from->carried_by->carry_weight -= get_obj_weight( obj ) 
		* WEIGHT_MULT(obj_from) / 100;
	}
    }

    return;
}



/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;

    if ( obj->in_room != NULL )
	obj_from_room( obj );
    else if ( obj->carried_by != NULL )
	obj_from_char( obj );
    else if ( obj->in_obj != NULL )
	obj_from_obj( obj );

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
	obj_next = obj_content->next_content;
	extract_obj( obj_content );
    }

    if ( object_list == obj )
    {
	object_list = obj->next;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = object_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == obj )
	    {
		prev->next = obj->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
	    return;
	}
    }

    --obj->pIndexData->count;
    free_obj(obj);
    return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
    CHAR_DATA *wch;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    /* doesn't seem to be necessary
    if ( ch->in_room == NULL )
    {
	bug( "Extract_char: NULL.", 0 );
	return;
    }
    */
    if(!IS_NPC(ch) && IN_MINE(ch) )
       exit_mine(ch);    

    nuke_pets(ch);
    ch->pet = NULL; /* just in case */

    if ( fPull )
    {
        if (has_auction(ch))
            return;
    
        if ( IS_IN_WAR ( ch ) )
            return;             

	die_follower( ch );
    }

    stop_fighting( ch, TRUE );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
        if(!fPull && !IS_OBJ_STAT(obj, ITEM_AUCTIONED))
	extract_obj( obj );
    }
    
    if (ch->in_room != NULL)
        char_from_room( ch );

    /* Death room is set in the clan tabe now */
    if ( !fPull )
    {   
      if ( is_clan(ch) &&
           (get_room_index(clan_table[ch->clan].room[2]) != NULL) )
        char_to_room(ch,get_room_index(clan_table[ch->clan].room[2]));
      else
        char_to_room(ch, get_room_index(hometown_table[ch->hometown].death));  
      return;
    }

    if ( IS_NPC(ch) )
	--ch->pIndexData->count;

    else
    {
	nuke_pets(ch);
	ch->pet = NULL; 
    }

    if ( ch->mount && ch->mount->mount == ch )
    {
	ch->mount->mount = NULL;

	if (ch->mount->riding)
	{
	    act("You fall off of $N.", ch->mount, NULL, ch, TO_CHAR);
	    act("$n falls off of $N.", ch->mount, NULL, ch, TO_ROOM);
	    ch->mount->riding = FALSE;
	    if (!IS_IMMORTAL(ch->mount))
		ch->mount->position = POS_SITTING;
	}
    }

    if ( ch->desc != NULL && ch->desc->original != NULL )
    {
	do_function(ch, &do_return, "" );
	ch->desc = NULL;
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
        if ( wch->reply == ch )
            wch->reply = NULL;
        if ( wch->mprog_target == wch )
            wch->mprog_target = NULL;
    }   
   
    if (!IS_NPC(ch) && ch->pcdata->tracking != NULL)
    {
        ch->pcdata->tracking->pcdata->track_count--;
        ch->pcdata->tracking = NULL;
    }

    if (!IS_NPC(ch) && ch->pcdata->track_count>0)
    {
      for ( wch = char_list; wch != NULL; wch = wch->next )
      {
          if ( !IS_NPC(wch) && wch->pcdata->tracking == ch )
              wch->pcdata->tracking = NULL;
      }
    }

    if ( ch == char_list )
    {
       char_list = ch->next;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = char_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == ch )
	    {
		prev->next = ch->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_char: char not found.", 0 );
	    return;
	}
    }

    if ( ch->desc != NULL )
	ch->desc->character = NULL;
    free_char( ch );
    return;
}



/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    if ( !str_cmp( arg, "self" ) )
	return ch;


    if ( ch && room )
    {
	bug( "get_char_room received multiple types (ch/room)", 0 );
	return NULL;
    }

    if ( ch )
	rch = ch->in_room->people;
    else
	rch = room->people;

    for ( ; rch != NULL; rch = rch->next_in_room )
    {
	if ( (!IS_NPC(rch) && !IS_NPC(ch) ) && (IN_MINE(ch) || IN_MINE(rch) ) && !IS_SAME_SHAFT(ch,rch) )
	    continue;
	if ( (ch && !can_see( ch, rch ))  )
	    continue;
	if ( !ch && !IS_NPC(rch) && !is_name(arg, rch->name) )
	    continue;
	if ( (ch && !IS_NPC(rch) && !is_name(arg, PERS(rch, ch) ) ) )
	    continue;
	if (IS_NPC(rch) && !is_name(arg, rch->name) )
	    continue;
	if ( ++count == number )
	    return rch;
    }

    return NULL;
}



/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ( ch && ( wch = get_char_room( ch, NULL, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );


    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if ( wch->in_room == NULL || ( ch && !can_see( ch, wch ) ) 
	||   !is_name( arg, wch->name ) )
	    continue;
	if ( ++count == number )
	    return wch;
    }

    return NULL;
}


/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;
    bool blank = FALSE;

    number = number_argument( argument, arg );
    count  = 0;

    if ( number > 0 && arg[0] == '\0' )
        blank = TRUE;

    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
 	if(!IS_NPC(ch) && !OBJ_SAME_SHAFT(ch, obj ) )
	   continue;

	if ( can_see_obj( ch, obj ) && (blank ? TRUE : is_name( arg, obj->name ) ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument, CHAR_DATA *viewer )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( obj->wear_loc == WEAR_NONE
        &&   ( viewer ? can_see_obj( viewer, obj ) : TRUE )
        &&   is_name( arg, obj->name ) )
        {
            if ( ++count == number )
                            return obj;
        }
    }

    return NULL;
}


/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument, bool character )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( obj->wear_loc != WEAR_NONE
                &&  ( character ? can_see_obj( ch, obj ) : TRUE)
        &&   is_name( arg, obj->name ) )
        {
            if ( ++count == number )
                return obj;
        }
    }

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument )
{
    OBJ_DATA *obj;
    int number, count;
    char arg[MAX_INPUT_LENGTH];

    if ( ch && room )
    {
	bug( "get_obj_here received a ch and a room",0);
	return NULL;
    }

    number = number_argument( argument, arg );
    count = 0;

    if ( ch )
    {
	obj = get_obj_list( ch, argument, ch->in_room->contents );

	if(obj && !IS_NPC(ch) &&( IN_MINE(ch) || OBJ_IN_MINE(obj)) &&  OBJ_SAME_SHAFT(ch, obj ) )
	   return obj;

	if ( obj != NULL )
	    return obj;


	if ( ( obj = get_obj_carry( ch, argument, ch ) ) != NULL )
	    return obj;

	if ( ( obj = get_obj_wear( ch, argument, TRUE ) ) != NULL )
	    return obj;
	    
    }
    else
    {
	for ( obj = room->contents; obj; obj = obj->next_content )
	{
	    if ( !is_name( arg, obj->name ) )
		continue;
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}


/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    if ( ch && ( obj = get_obj_here( ch, NULL, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( ( ch && !can_see_obj( ch, obj ) )
	|| !is_name( arg, obj->name ) )
	    continue;
	if ( ++count == number )
	    return obj;
    }

    return NULL;
}



/* deduct cost from a character */

void deduct_cost(CHAR_DATA *ch, int cost)
{
    int silver = 0, gold = 0;

    silver = UMIN(ch->silver,cost); 

    if (silver < cost)
    {
	gold = ((cost - silver + 99) / 100);
	silver = cost - 100 * gold;
    }

    ch->gold -= gold;
    ch->silver -= silver;

    if (ch->gold < 0)
    {
	bug("deduct costs: gold %d < 0",ch->gold);
	ch->gold = 0;
    }
    if (ch->silver < 0)
    {
	bug("deduct costs: silver %d < 0",ch->silver);
	ch->silver = 0;
    }
}   
/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int gold, int silver )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( gold < 0 || silver < 0 || (gold == 0 && silver == 0) )
    {
	bug( "Create_money: zero or negative money.",UMIN(gold,silver));
	gold = UMAX(1,gold);
	silver = UMAX(1,silver);
    }

    if (gold == 0 && silver == 1)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ), 0 );
    }
    else if (gold == 1 && silver == 0)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_GOLD_ONE), 0 );
    }
    else if (silver == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_GOLD_SOME ), 0 );
        sprintf( buf, obj->short_descr, gold );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[1]           = gold;
        obj->cost               = gold;
	obj->weight		= gold/5;
    }
    else if (gold == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ), 0 );
        sprintf( buf, obj->short_descr, silver );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[0]           = silver;
        obj->cost               = silver;
	obj->weight		= silver/20;
    }
 
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_COINS ), 0 );
	sprintf( buf, obj->short_descr, silver, gold );
	free_string( obj->short_descr );
	obj->short_descr	= str_dup( buf );
	obj->value[0]		= silver;
	obj->value[1]		= gold;
	obj->cost		= 100 * gold + silver;
	obj->weight		= gold / 5 + silver / 20;
    }

    return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
    int number;
 
    if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY
    ||  obj->item_type == ITEM_GEM || obj->item_type == ITEM_JEWELRY)
        number = 0;
    else
        number = 1;
 
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number( obj );
 
    return number;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;
    OBJ_DATA *tobj;

    weight = obj->weight;
    for ( tobj = obj->contains; tobj != NULL; tobj = tobj->next_content )
	weight += get_obj_weight( tobj ) * WEIGHT_MULT(obj) / 100;

    return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
    int weight;
 
    weight = obj->weight;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        weight += get_obj_weight( obj );
 
    return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    if ( pRoomIndex->light > 0 )
	return FALSE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
    ||   pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
    ||   weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}


bool is_room_owner(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    if (room->owner == NULL || room->owner[0] == '\0')
	return FALSE;

    return is_name(ch->name,room->owner);
}

/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count;


    if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
	return TRUE;

    count = 0;
    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
	count++;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return TRUE;
    
    if ( IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) )
	return TRUE;

    return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    if (IS_SET(pRoomIndex->room_flags, ROOM_ARENA))
	return TRUE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) 
    &&  get_trust(ch) < MAX_LEVEL)
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
    &&  !IS_IMMORTAL(ch))
	return FALSE;

    if(IS_SET (pRoomIndex->room_flags, ROOM_HEROES_ONLY) && ch->level < LEVEL_HERO && !IS_IMMORTAL (ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags,ROOM_NEWBIES_ONLY)
    &&  ch->level > 5 && !IS_IMMORTAL(ch))
	return FALSE;

    if (!IS_IMMORTAL(ch) && pRoomIndex->clan && ch->clan != pRoomIndex->clan)
	return FALSE;

    return TRUE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
/* RT changed so that WIZ_INVIS has levels */
    if ( ch == victim )
	return TRUE;
    
    if ( get_trust(ch) < victim->invis_level)
	return FALSE;


    if (get_trust(ch) < victim->incog_level && ch->in_room != victim->in_room)
	return FALSE;
   
    if (IS_QUESTOR(ch) && IS_NPC(victim) && victim->pIndexData->vnum == ch->pcdata->questmob)
	return TRUE;

    if((!IS_NPC (ch) && IS_SET (ch->act, PLR_HOLYLIGHT)) || (IS_NPC (ch) && IS_IMMORTAL (ch) && ch->desc != NULL))
	return TRUE;

    if (gquest_info.running == GQUEST_RUNNING && ON_GQUEST(ch) &&
	IS_NPC(victim) && is_gqmob(ch, victim->pIndexData->vnum) != -1)
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;

    if ((!IS_NPC(ch) && ch->in_room &&
    IS_SET(ch->in_room->room_flags, ROOM_ARENA)) && (!IS_NPC(victim) &&
	victim->in_room && IS_SET(victim->in_room->room_flags, ROOM_ARENA)))
	return TRUE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
	return FALSE;

    /* sneaking */
    if ( IS_AFFECTED(victim, AFF_SNEAK)
    &&   !IS_AFFECTED(ch,AFF_DETECT_HIDDEN)
    &&   victim->fighting == NULL)
    {
	int chance;
	chance = get_skill(victim,skill_lookup("sneak"));
	chance += get_curr_stat(victim,STAT_DEX) * 3/2;
 	chance -= get_curr_stat(ch,STAT_INT) * 2;
	chance -= ch->level - victim->level * 3/2;

	if (number_percent() < chance)
	    return FALSE;
    }

    if ( IS_AFFECTED(victim, AFF_HIDE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
    &&   victim->fighting == NULL)
	return FALSE;

    return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
	return FALSE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) && obj->item_type != ITEM_POTION)
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
        return FALSE;

    if(IS_SET(obj->extra_flags,ITEM_HIDDEN))
        return FALSE;

    if ( IS_OBJ_STAT(obj,ITEM_GLOW))
	return TRUE;

    if (IS_QUESTOR(ch) && ch->pcdata->questobj == obj->pIndexData->vnum)
	return TRUE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_DARK_VISION) )
	return FALSE;

    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{

    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return TRUE;

    if (IS_OBJ_STAT (obj, ITEM_AUCTIONED))
        return FALSE;

    if ( !IS_SET(obj->extra_flags, ITEM_NODROP) )
	return TRUE;

    return FALSE;
}


/*
 * Return ascii name of an affect location.
 */
const char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:		return "none";
    case APPLY_STR:		return "strength";
    case APPLY_DEX:		return "dexterity";
    case APPLY_INT:		return "intelligence";
    case APPLY_WIS:		return "wisdom";
    case APPLY_CON:		return "constitution";
    case APPLY_SEX:		return "sex";
    case APPLY_CLASS:		return "class";
    case APPLY_LEVEL:		return "level";
    case APPLY_AGE:		return "age";
    case APPLY_MANA:		return "mana";
    case APPLY_HIT:		return "hp";
    case APPLY_MOVE:		return "moves";
    case APPLY_GOLD:		return "gold";
    case APPLY_EXP:		return "experience";
    case APPLY_AC:		return "armor class";
    case APPLY_HITROLL:		return "hit roll";
    case APPLY_DAMROLL:		return "damage roll";
    case APPLY_CAST_LEVEL:      return "cast level";
    case APPLY_CAST_ABILITY:    return "cast ability";
    case APPLY_SAVES:		return "saves";
    case APPLY_SAVING_ROD:	return "save vs rod";
    case APPLY_SAVING_PETRI:	return "save vs petrification";
    case APPLY_SAVING_BREATH:	return "save vs breath";
    case APPLY_SAVING_SPELL:	return "save vs spell";
    case APPLY_SPELL_AFFECT:	return "none";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}



/*
 * Return ascii name of an affect bit vector.
 */
const char *affect_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    if ( vector & AFF_BLIND         ) strcat( buf, " blind"         );
    if ( vector & AFF_INVISIBLE     ) strcat( buf, " invisible"     );
    if ( vector & AFF_DETECT_EVIL   ) strcat( buf, " detect_evil"   );
    if ( vector & AFF_DETECT_GOOD   ) strcat( buf, " detect_good"   );
    if ( vector & AFF_DETECT_INVIS  ) strcat( buf, " detect_invis"  );
    if ( vector & AFF_DETECT_MAGIC  ) strcat( buf, " detect_magic"  );
    if ( vector & AFF_DETECT_HIDDEN ) strcat( buf, " detect_hidden" );
    if ( vector & AFF_SANCTUARY     ) strcat( buf, " sanctuary"     );
    if ( vector & AFF_FAERIE_FIRE   ) strcat( buf, " faerie_fire"   );
    if ( vector & AFF_INFRARED      ) strcat( buf, " infrared"      );
    if ( vector & AFF_CURSE         ) strcat( buf, " curse"         );
    if ( vector & AFF_POISON        ) strcat( buf, " poison"        );
    if ( vector & AFF_PROTECT_EVIL  ) strcat( buf, " prot_evil"     );
    if ( vector & AFF_PROTECT_GOOD  ) strcat( buf, " prot_good"     );
    if ( vector & AFF_SLEEP         ) strcat( buf, " sleep"         );
    if ( vector & AFF_SNEAK         ) strcat( buf, " sneak"         );
    if ( vector & AFF_HIDE          ) strcat( buf, " hide"          );
    if ( vector & AFF_CHARM         ) strcat( buf, " charm"         );
    if ( vector & AFF_FLYING        ) strcat( buf, " flying"        );
    if ( vector & AFF_PASS_DOOR     ) strcat( buf, " pass_door"     );
    if ( vector & AFF_BERSERK	    ) strcat( buf, " berserk"	    );
    if ( vector & AFF_CALM	    ) strcat( buf, " calm"	    );
    if ( vector & AFF_HASTE	    ) strcat( buf, " haste"	    );
    if ( vector & AFF_SLOW          ) strcat( buf, " slow"          );
    if ( vector & AFF_PLAGUE	    ) strcat( buf, " plague" 	    );
    if ( vector & AFF_DARK_VISION   ) strcat( buf, " dark_vision"   );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}



/*
 * Return ascii name of extra flags vector.
 */
const char *extra_bit_name( int extra_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( extra_flags & ITEM_GLOW         ) strcat( buf, " glow"         );
    if ( extra_flags & ITEM_HUM          ) strcat( buf, " hum"          );
    if ( extra_flags & ITEM_DARK         ) strcat( buf, " dark"         );
    if ( extra_flags & ITEM_LOCK         ) strcat( buf, " lock"         );
    if ( extra_flags & ITEM_EVIL         ) strcat( buf, " evil"         );
    if ( extra_flags & ITEM_INVIS        ) strcat( buf, " invis"        );
    if ( extra_flags & ITEM_MAGIC        ) strcat( buf, " magic"        );
    if ( extra_flags & ITEM_NODROP       ) strcat( buf, " nodrop"       );
    if ( extra_flags & ITEM_BLESS        ) strcat( buf, " bless"        );
    if ( extra_flags & ITEM_ANTI_GOOD    ) strcat( buf, " anti-good"    );
    if ( extra_flags & ITEM_ANTI_EVIL    ) strcat( buf, " anti-evil"    );
    if ( extra_flags & ITEM_ANTI_NEUTRAL ) strcat( buf, " anti-neutral" );
    if ( extra_flags & ITEM_NOREMOVE     ) strcat( buf, " noremove"     );
    if ( extra_flags & ITEM_INVENTORY    ) strcat( buf, " inventory"    );
    if ( extra_flags & ITEM_NOPURGE	 ) strcat( buf, " nopurge"	);
    if ( extra_flags & ITEM_VIS_DEATH	 ) strcat( buf, " vis_death"	);
    if ( extra_flags & ITEM_ROT_DEATH	 ) strcat( buf, " rot_death"	);
    if ( extra_flags & ITEM_NOLOCATE	 ) strcat( buf, " no_locate"	);
    if ( extra_flags & ITEM_SELL_EXTRACT ) strcat( buf, " sell_extract" );
    if ( extra_flags & ITEM_BURN_PROOF	 ) strcat( buf, " burn_proof"	);
    if ( extra_flags & ITEM_NOUNCURSE	 ) strcat( buf, " no_uncurse"	);
    if ( extra_flags & ITEM_LODGED       ) strcat( buf, " lodged"	);
    if ( extra_flags & ITEM_AUCTIONED    ) strcat( buf, " auctioned"    );
    if ( extra_flags & ITEM_NO_AUCTION   ) strcat( buf, " no_auction"   ); 
    if ( extra_flags & ITEM_QUEST        ) strcat( buf, " quest"        );  
    if ( extra_flags & ITEM_NOCOND       ) strcat( buf, " no_cond"      );
    if ( extra_flags & ITEM_NOREPAIR     ) strcat( buf, " no_repair"    );

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}


const char *extra2_bit_name( int extra2_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( extra2_flags & ITEM_UNIQUE       ) strcat( buf, " unique");
    if ( extra2_flags & CORPSE_NO_ANIMATE ) strcat( buf, " no_animate");
    if ( extra2_flags & ITEM_SANC	  ) strcat( buf, " sanctuary" );
    if ( extra2_flags & ITEM_RELIC        ) strcat( buf, " relic" );
    if ( extra2_flags & ITEM_INLAY1       ) strcat( buf, " inlay1");
    if ( extra2_flags & ITEM_INLAY2       ) strcat( buf, " inlay2");
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}


/* return ascii name of an act vector */
const char *act_bit_name( int act_flags )
{
    static char buf[512];

    buf[0] = '\0';

    if (IS_SET(act_flags,ACT_IS_NPC))
    { 
 	strcat(buf," npc");
        if (act_flags & ACT_FORGER      ) strcat(buf, "forger");
    	if (act_flags & ACT_SENTINEL 	) strcat(buf, " sentinel");
    	if (act_flags & ACT_SCAVENGER	) strcat(buf, " scavenger");
	if (act_flags & ACT_AGGRESSIVE	) strcat(buf, " aggressive");
	if (act_flags & ACT_STAY_AREA	) strcat(buf, " stay_area");
	if (act_flags & ACT_WIMPY	) strcat(buf, " wimpy");
	if (act_flags & ACT_PET		) strcat(buf, " pet");
	if (act_flags & ACT_MOUNT	) strcat(buf, " mount");	
	if (act_flags & ACT_TRAIN	) strcat(buf, " train");
	if (act_flags & ACT_PRACTICE	) strcat(buf, " practice");
	if (act_flags & ACT_UNDEAD	) strcat(buf, " undead");
	if (act_flags & ACT_CLERIC	) strcat(buf, " cleric");
	if (act_flags & ACT_MAGE	) strcat(buf, " mage");
	if (act_flags & ACT_THIEF	) strcat(buf, " thief");
	if (act_flags & ACT_WARRIOR	) strcat(buf, " warrior");
	if (act_flags & ACT_NOALIGN	) strcat(buf, " no_align");
	if (act_flags & ACT_NOPURGE	) strcat(buf, " no_purge");
	if (act_flags & ACT_IS_HEALER	) strcat(buf, " healer");
	if (act_flags & ACT_IS_CHANGER  ) strcat(buf, " changer");
	if (act_flags & ACT_GAIN	) strcat(buf, " skill_train");
	if (act_flags & ACT_UPDATE_ALWAYS) strcat(buf," update_always");
        if (act_flags & ACT_BOUNTY      ) strcat(buf,"  bounty");
        if (act_flags & ACT_LEVELER     ) strcat(buf,"  leveler");
    }
    else
    {
	strcat(buf," player");
	if (act_flags & PLR_AUTOASSIST	) strcat(buf, " autoassist");
	if (act_flags & PLR_AUTOEXIT	) strcat(buf, " autoexit");
	if (act_flags & PLR_AUTOLOOT	) strcat(buf, " autoloot");
	if (act_flags & PLR_AUTOSAC	) strcat(buf, " autosac");
	if (act_flags & PLR_AUTOGOLD	) strcat(buf, " autogold");
	if (act_flags & PLR_AUTOSPLIT	) strcat(buf, " autosplit");
	if (act_flags & PLR_HOLYLIGHT	) strcat(buf, " holy_light");
	if (act_flags & PLR_CANLOOT	) strcat(buf, " loot_corpse");
	if (act_flags & PLR_NOSUMMON	) strcat(buf, " no_summon");
	if (act_flags & PLR_NOFOLLOW	) strcat(buf, " no_follow");
	if (act_flags & PLR_FREEZE	) strcat(buf, " frozen");
	if (act_flags & PLR_THIEF	) strcat(buf, " thief");
	if (act_flags & PLR_KILLER	) strcat(buf, " killer");
        if (act_flags & PLR_QUESTOR     ) strcat(buf, " questor");   
        if (act_flags & PLR_WAR         ) strcat(buf, " war");
        if (act_flags & PLR_TIMEOUT     ) strcat(buf, " timeout");
        if (act_flags & PLR_FREQUENT    ) strcat(buf, " frequent");
        if (act_flags & PLR_AUTOWEATHER ) strcat(buf, " autoweather");
        if (act_flags & PLR_CONSENT     ) strcat(buf, " consent");
   }   
    
   return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *comm_bit_name(int comm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (comm_flags & COMM_QUIET		) strcat(buf, " quiet");
    if (comm_flags & COMM_DEAF		) strcat(buf, " deaf");
    if (comm_flags & COMM_NOWIZ		) strcat(buf, " no_wiz");
    if (comm_flags & COMM_NOAUCTION	) strcat(buf, " no_auction");
    if (comm_flags & COMM_NOGOSSIP	) strcat(buf, " no_gossip");
    if (comm_flags & COMM_NOQUESTION	) strcat(buf, " no_question");
    if (comm_flags & COMM_NOMUSIC	) strcat(buf, " no_music");
    if (comm_flags & COMM_NOQUOTE	) strcat(buf, " no_quote");
    if (comm_flags & COMM_COMPACT	) strcat(buf, " compact");
    if (comm_flags & COMM_BRIEF		) strcat(buf, " brief");
    if (comm_flags & COMM_PROMPT	) strcat(buf, " prompt");
    if (comm_flags & COMM_COMBINE	) strcat(buf, " combine");
    if (comm_flags & COMM_NOEMOTE	) strcat(buf, " no_emote");
    if (comm_flags & COMM_NOSHOUT	) strcat(buf, " no_shout");
    if (comm_flags & COMM_NOTELL	) strcat(buf, " no_tell");
    if (comm_flags & COMM_NOCHANNELS	) strcat(buf, " no_channels");
    if (comm_flags & COMM_NOPRAY        ) strcat(buf, " no_pray");
    if (comm_flags & COMM_NOHERO        ) strcat(buf, " no_hero");
    if (comm_flags & COMM_NORACE        ) strcat(buf, " no_race");
    if (comm_flags & COMM_NOHINT        ) strcat(buf, " no_hint");
    if (comm_flags & COMM_NOINFO        ) strcat(buf, " no_info");
   
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *imm_bit_name(int imm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (imm_flags & IMM_SUMMON		) strcat(buf, " summon");
    if (imm_flags & IMM_CHARM		) strcat(buf, " charm");
    if (imm_flags & IMM_MAGIC		) strcat(buf, " magic");
    if (imm_flags & IMM_WEAPON		) strcat(buf, " weapon");
    if (imm_flags & IMM_BASH		) strcat(buf, " blunt");
    if (imm_flags & IMM_PIERCE		) strcat(buf, " piercing");
    if (imm_flags & IMM_SLASH		) strcat(buf, " slashing");
    if (imm_flags & IMM_FIRE		) strcat(buf, " fire");
    if (imm_flags & IMM_COLD		) strcat(buf, " cold");
    if (imm_flags & IMM_LIGHTNING	) strcat(buf, " lightning");
    if (imm_flags & IMM_ACID		) strcat(buf, " acid");
    if (imm_flags & IMM_POISON		) strcat(buf, " poison");
    if (imm_flags & IMM_NEGATIVE	) strcat(buf, " negative");
    if (imm_flags & IMM_HOLY		) strcat(buf, " holy");
    if (imm_flags & IMM_ENERGY		) strcat(buf, " energy");
    if (imm_flags & IMM_MENTAL		) strcat(buf, " mental");
    if (imm_flags & IMM_DISEASE	) strcat(buf, " disease");
    if (imm_flags & IMM_DROWNING	) strcat(buf, " drowning");
    if (imm_flags & IMM_LIGHT		) strcat(buf, " light");
    if (imm_flags & VULN_IRON		) strcat(buf, " iron");
    if (imm_flags & VULN_WOOD		) strcat(buf, " wood");
    if (imm_flags & VULN_SILVER	) strcat(buf, " silver");
    if (imm_flags & VULN_STEEL          ) strcat(buf, " steel");
    if (imm_flags & VULN_MITHRIL        ) strcat(buf, " mithril");
    if (imm_flags & VULN_ADAMANTITE     ) strcat(buf, " adamantite");
    if (imm_flags & VULN_ENERGY         ) strcat(buf, " energy");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *wear_bit_name(int wear_flags)
{
    static char buf[512];

    buf [0] = '\0';
    if (wear_flags & ITEM_TAKE		) strcat(buf, " take");
    if (wear_flags & ITEM_WEAR_FINGER	) strcat(buf, " finger");
    if (wear_flags & ITEM_WEAR_WEDDING  ) strcat(buf, "wedding");
    if (wear_flags & ITEM_WEAR_NECK	) strcat(buf, " neck");
    if (wear_flags & ITEM_WEAR_BODY	) strcat(buf, " torso");
    if (wear_flags & ITEM_WEAR_HEAD	) strcat(buf, " head");
    if (wear_flags & ITEM_WEAR_LEGS	) strcat(buf, " legs");
    if (wear_flags & ITEM_WEAR_FEET	) strcat(buf, " feet");
    if (wear_flags & ITEM_WEAR_HANDS	) strcat(buf, " hands");
    if (wear_flags & ITEM_WEAR_ARMS	) strcat(buf, " arms");
    if (wear_flags & ITEM_WEAR_SHIELD	) strcat(buf, " shield");
    if (wear_flags & ITEM_WEAR_ABOUT	) strcat(buf, " body");
    if (wear_flags & ITEM_WEAR_WAIST	) strcat(buf, " waist");
    if (wear_flags & ITEM_WEAR_WRIST	) strcat(buf, " wrist");
    if (wear_flags & ITEM_WIELD		) strcat(buf, " wield");
    if (wear_flags & ITEM_HOLD		) strcat(buf, " hold");
    if (wear_flags & ITEM_NO_SAC	) strcat(buf, " nosac");
    if (wear_flags & ITEM_WEAR_FLOAT	) strcat(buf, " float");
    if (wear_flags & ITEM_WEAR_SHOULDER ) strcat(buf, " shoulder");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *form_bit_name(int form_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (form_flags & FORM_POISON	) strcat(buf, " poison");
    else if (form_flags & FORM_EDIBLE	) strcat(buf, " edible");
    if (form_flags & FORM_MAGICAL	) strcat(buf, " magical");
    if (form_flags & FORM_INSTANT_DECAY	) strcat(buf, " instant_rot");
    if (form_flags & FORM_OTHER		) strcat(buf, " other");
    if (form_flags & FORM_ANIMAL	) strcat(buf, " animal");
    if (form_flags & FORM_SENTIENT	) strcat(buf, " sentient");
    if (form_flags & FORM_UNDEAD	) strcat(buf, " undead");
    if (form_flags & FORM_CONSTRUCT	) strcat(buf, " construct");
    if (form_flags & FORM_MIST		) strcat(buf, " mist");
    if (form_flags & FORM_INTANGIBLE	) strcat(buf, " intangible");
    if (form_flags & FORM_BIPED		) strcat(buf, " biped");
    if (form_flags & FORM_CENTAUR	) strcat(buf, " centaur");
    if (form_flags & FORM_INSECT	) strcat(buf, " insect");
    if (form_flags & FORM_SPIDER	) strcat(buf, " spider");
    if (form_flags & FORM_CRUSTACEAN	) strcat(buf, " crustacean");
    if (form_flags & FORM_WORM		) strcat(buf, " worm");
    if (form_flags & FORM_BLOB		) strcat(buf, " blob");
    if (form_flags & FORM_MAMMAL	) strcat(buf, " mammal");
    if (form_flags & FORM_BIRD		) strcat(buf, " bird");
    if (form_flags & FORM_REPTILE	) strcat(buf, " reptile");
    if (form_flags & FORM_SNAKE		) strcat(buf, " snake");
    if (form_flags & FORM_DRAGON	) strcat(buf, " dragon");
    if (form_flags & FORM_AMPHIBIAN	) strcat(buf, " amphibian");
    if (form_flags & FORM_FISH		) strcat(buf, " fish");
    if (form_flags & FORM_COLD_BLOOD 	) strcat(buf, " cold_blooded");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *part_bit_name(int part_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (part_flags & PART_HEAD		) strcat(buf, " head");
    if (part_flags & PART_ARMS		) strcat(buf, " arms");
    if (part_flags & PART_LEGS		) strcat(buf, " legs");
    if (part_flags & PART_HEART		) strcat(buf, " heart");
    if (part_flags & PART_BRAINS	) strcat(buf, " brains");
    if (part_flags & PART_GUTS		) strcat(buf, " guts");
    if (part_flags & PART_HANDS		) strcat(buf, " hands");
    if (part_flags & PART_FEET		) strcat(buf, " feet");
    if (part_flags & PART_FINGERS	) strcat(buf, " fingers");
    if (part_flags & PART_EAR		) strcat(buf, " ears");
    if (part_flags & PART_EYE		) strcat(buf, " eyes");
    if (part_flags & PART_LONG_TONGUE	) strcat(buf, " long_tongue");
    if (part_flags & PART_EYESTALKS	) strcat(buf, " eyestalks");
    if (part_flags & PART_TENTACLES	) strcat(buf, " tentacles");
    if (part_flags & PART_FINS		) strcat(buf, " fins");
    if (part_flags & PART_WINGS		) strcat(buf, " wings");
    if (part_flags & PART_TAIL		) strcat(buf, " tail");
    if (part_flags & PART_CLAWS		) strcat(buf, " claws");
    if (part_flags & PART_FANGS		) strcat(buf, " fangs");
    if (part_flags & PART_HORNS		) strcat(buf, " horns");
    if (part_flags & PART_SCALES	) strcat(buf, " scales");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *weapon_bit_name(int weapon_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (weapon_flags & WEAPON_FLAMING	) strcat(buf, " flaming");
    if (weapon_flags & WEAPON_FROST	) strcat(buf, " frost");
    if (weapon_flags & WEAPON_VAMPIRIC	) strcat(buf, " vampiric");
    if (weapon_flags & WEAPON_SHARP	) strcat(buf, " sharp");
    if (weapon_flags & WEAPON_VORPAL	) strcat(buf, " vorpal");
    if (weapon_flags & WEAPON_TWO_HANDS ) strcat(buf, " two-handed");
    if (weapon_flags & WEAPON_SHOCKING 	) strcat(buf, " shocking");
    if (weapon_flags & WEAPON_POISON	) strcat(buf, " poison");
    if (weapon_flags & WEAPON_SHARP     ) strcat(buf, " sharp");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *cont_bit_name( int cont_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (cont_flags & CONT_CLOSEABLE	) strcat(buf, " closable");
    if (cont_flags & CONT_PICKPROOF	) strcat(buf, " pickproof");
    if (cont_flags & CONT_CLOSED	) strcat(buf, " closed");
    if (cont_flags & CONT_LOCKED	) strcat(buf, " locked");

    return (buf[0] != '\0' ) ? buf+1 : "none";
}


const char *off_bit_name(int off_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (off_flags & OFF_AREA_ATTACK	) strcat(buf, " area attack");
    if (off_flags & OFF_BACKSTAB	) strcat(buf, " backstab");
    if (off_flags & OFF_BASH		) strcat(buf, " bash");
    if (off_flags & OFF_BERSERK		) strcat(buf, " berserk");
    if (off_flags & OFF_DISARM		) strcat(buf, " disarm");
    if (off_flags & OFF_DODGE		) strcat(buf, " dodge");
    if (off_flags & OFF_FADE		) strcat(buf, " fade");
    if (off_flags & OFF_FAST		) strcat(buf, " fast");
    if (off_flags & OFF_KICK		) strcat(buf, " kick");
    if (off_flags & OFF_KICK_DIRT	) strcat(buf, " kick_dirt");
    if (off_flags & OFF_PARRY		) strcat(buf, " parry");
    if (off_flags & OFF_RESCUE		) strcat(buf, " rescue");
    if (off_flags & OFF_TAIL		) strcat(buf, " tail");
    if (off_flags & OFF_TRIP		) strcat(buf, " trip");
    if (off_flags & OFF_CRUSH		) strcat(buf, " crush");
    if (off_flags & OFF_BLOODLUST       ) strcat(buf, " bloodlust");
    if (off_flags & ASSIST_ALL		) strcat(buf, " assist_all");
    if (off_flags & ASSIST_ALIGN	) strcat(buf, " assist_align");
    if (off_flags & ASSIST_RACE		) strcat(buf, " assist_race");
    if (off_flags & ASSIST_PLAYERS	) strcat(buf, " assist_players");
    if (off_flags & ASSIST_GUARD	) strcat(buf, " assist_guard");
    if (off_flags & ASSIST_VNUM		) strcat(buf, " assist_vnum");
    if (off_flags & OFF_FEED            ) strcat(buf, " feed");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/*
 * Config Colour stuff
 */
void default_colour( CHAR_DATA *ch )
{
    if( IS_NPC( ch ) )
	return;

    if( !ch->pcdata )
	return;

    ch->pcdata->text[1]		= ( WHITE );
    ch->pcdata->auction[1]	= ( YELLOW );
    ch->pcdata->auction_text[1]	= ( WHITE );
    ch->pcdata->gossip[1]	= ( MAGENTA );
    ch->pcdata->gossip_text[1]	= ( MAGENTA );
    ch->pcdata->music[1]	= ( RED );
    ch->pcdata->music_text[1]	= ( RED );
    ch->pcdata->question[1]	= ( YELLOW );
    ch->pcdata->question_text[1] = ( WHITE );
    ch->pcdata->answer[1]	= ( YELLOW );
    ch->pcdata->answer_text[1]	= ( WHITE );
    ch->pcdata->quote[1]	= ( GREEN );
    ch->pcdata->quote_text[1]	= ( GREEN );
    ch->pcdata->immtalk_text[1]	= ( CYAN );
    ch->pcdata->immtalk_type[1]	= ( YELLOW );
    ch->pcdata->info[1]		= ( YELLOW );
    ch->pcdata->tell[1]		= ( GREEN );
    ch->pcdata->tell_text[1]	= ( GREEN );
    ch->pcdata->say[1]		= ( GREEN );
    ch->pcdata->say_text[1]	= ( GREEN );
    ch->pcdata->reply[1]	= ( GREEN );
    ch->pcdata->reply_text[1]	= ( GREEN );
    ch->pcdata->gtell_text[1]	= ( GREEN );
    ch->pcdata->gtell_type[1]	= ( RED );
    ch->pcdata->wiznet[1]	= ( GREEN );
    ch->pcdata->room_title[1]	= ( CYAN );
    ch->pcdata->room_text[1]	= ( WHITE );
    ch->pcdata->room_exits[1]	= ( GREEN );
    ch->pcdata->room_things[1]	= ( CYAN );
    ch->pcdata->prompt[1]	= ( CYAN );
    ch->pcdata->fight_death[1]	= ( RED );
    ch->pcdata->fight_yhit[1]	= ( GREEN );
    ch->pcdata->fight_ohit[1]	= ( YELLOW );
    ch->pcdata->fight_thit[1]	= ( RED );
    ch->pcdata->fight_skill[1]	= ( WHITE );
    ch->pcdata->text[0]		= ( NORMAL );
    ch->pcdata->auction[0]	= ( BRIGHT );
    ch->pcdata->auction_text[0]	= ( BRIGHT );
    ch->pcdata->gossip[0]	= ( NORMAL );
    ch->pcdata->gossip_text[0]	= ( BRIGHT );
    ch->pcdata->music[0]	= ( NORMAL );
    ch->pcdata->music_text[0]	= ( BRIGHT );
    ch->pcdata->question[0]	= ( BRIGHT );
    ch->pcdata->question_text[0] = ( BRIGHT );
    ch->pcdata->answer[0]	= ( BRIGHT );
    ch->pcdata->answer_text[0]	= ( BRIGHT );
    ch->pcdata->quote[0]	= ( NORMAL );
    ch->pcdata->quote_text[0]	= ( BRIGHT );
    ch->pcdata->immtalk_text[0]	= ( NORMAL );
    ch->pcdata->immtalk_type[0]	= ( NORMAL );
    ch->pcdata->info[0]		= ( NORMAL );
    ch->pcdata->say[0]		= ( NORMAL );
    ch->pcdata->say_text[0]	= ( BRIGHT );
    ch->pcdata->tell[0]		= ( NORMAL );
    ch->pcdata->tell_text[0]	= ( BRIGHT );
    ch->pcdata->reply[0]	= ( NORMAL );
    ch->pcdata->reply_text[0]	= ( BRIGHT );
    ch->pcdata->gtell_text[0]	= ( NORMAL );
    ch->pcdata->gtell_type[0]	= ( NORMAL );
    ch->pcdata->wiznet[0]	= ( NORMAL );
    ch->pcdata->room_title[0]	= ( NORMAL );
    ch->pcdata->room_text[0]	= ( NORMAL );
    ch->pcdata->room_exits[0]	= ( NORMAL );
    ch->pcdata->room_things[0]	= ( NORMAL );
    ch->pcdata->prompt[0]	= ( NORMAL );
    ch->pcdata->fight_death[0]	= ( NORMAL );
    ch->pcdata->fight_yhit[0]	= ( NORMAL );
    ch->pcdata->fight_ohit[0]	= ( NORMAL );
    ch->pcdata->fight_thit[0]	= ( NORMAL );
    ch->pcdata->fight_skill[0]	= ( NORMAL );
    ch->pcdata->text[2]		= 0;
    ch->pcdata->auction[2]	= 0;
    ch->pcdata->auction_text[2]	= 0;
    ch->pcdata->gossip[2]	= 0;
    ch->pcdata->gossip_text[2]	= 0;
    ch->pcdata->music[2]	= 0;
    ch->pcdata->music_text[2]	= 0;
    ch->pcdata->question[2]	= 0;
    ch->pcdata->question_text[2] = 0;
    ch->pcdata->answer[2]	= 0;
    ch->pcdata->answer_text[2]	= 0;
    ch->pcdata->quote[2]	= 0;
    ch->pcdata->quote_text[2]	= 0;
    ch->pcdata->immtalk_text[2]	= 0;
    ch->pcdata->immtalk_type[2]	= 0;
    ch->pcdata->info[2]		= 1;
    ch->pcdata->say[2]		= 0;
    ch->pcdata->say_text[2]	= 0;
    ch->pcdata->tell[2]		= 0;
    ch->pcdata->tell_text[2]	= 0;
    ch->pcdata->reply[2]	= 0;
    ch->pcdata->reply_text[2]	= 0;
    ch->pcdata->gtell_text[2]	= 0;
    ch->pcdata->gtell_type[2]	= 0;
    ch->pcdata->wiznet[2]	= 0;
    ch->pcdata->room_title[2]	= 0;
    ch->pcdata->room_text[2]	= 0;
    ch->pcdata->room_exits[2]	= 0;
    ch->pcdata->room_things[2]	= 0;
    ch->pcdata->prompt[2]	= 0;
    ch->pcdata->fight_death[2]	= 0;
    ch->pcdata->fight_yhit[2]	= 0;
    ch->pcdata->fight_ohit[2]	= 0;
    ch->pcdata->fight_thit[2]	= 0;
    ch->pcdata->fight_skill[2]	= 0;

    return;
}

void all_colour( CHAR_DATA *ch, char *argument )
{
    char	buf[  100 ];
    char	buf2[ 100 ];
    int		colour;
    int		bright;

    if( IS_NPC( ch ) || !ch->pcdata )
	return;

    if( !*argument )
	return;

    if( !str_prefix( argument, "red" ) )
    {
	colour = ( RED );
	bright = NORMAL;
	sprintf( buf2, "Red" );
    }
    if( !str_prefix( argument, "hi-red" ) )
    {
	colour = ( RED );
	bright = BRIGHT;
	sprintf( buf2, "Red" );
    }
    else if( !str_prefix( argument, "green" ) )
    {
	colour = ( GREEN );
	bright = NORMAL;
	sprintf( buf2, "Green" );
    }
    else if( !str_prefix( argument, "hi-green" ) )
    {
	colour = ( GREEN );
	bright = BRIGHT;
	sprintf( buf2, "Green" );
    }
    else if( !str_prefix( argument, "yellow" ) )
    {
	colour = ( YELLOW );
	bright = NORMAL;
	sprintf( buf2, "Yellow" );
    }
    else if( !str_prefix( argument, "hi-yellow" ) )
    {
	colour = ( YELLOW );
	bright = BRIGHT;
	sprintf( buf2, "Yellow" );
    }
    else if( !str_prefix( argument, "blue" ) )
    {
	colour = ( BLUE );
	bright = NORMAL;
	sprintf( buf2, "Blue" );
    }
    else if( !str_prefix( argument, "hi-blue" ) )
    {
	colour = ( BLUE );
	bright = BRIGHT;
	sprintf( buf2, "Blue" );
    }
    else if( !str_prefix( argument, "magenta" ) )
    {
	colour = ( MAGENTA );
	bright = NORMAL;
	sprintf( buf2, "Magenta" );
    }
    else if( !str_prefix( argument, "hi-magenta" ) )
    {
	colour = ( MAGENTA );
	bright = BRIGHT;
	sprintf( buf2, "Magenta" );
    }
    else if( !str_prefix( argument, "cyan" ) )
    {
	colour = ( CYAN );
	bright = NORMAL;
	sprintf( buf2, "Cyan" );
    }
    else if( !str_prefix( argument, "hi-cyan" ) )
    {
	colour = ( CYAN );
	bright = BRIGHT;
	sprintf( buf2, "Cyan" );
    }
    else if( !str_prefix( argument, "white" ) )
    {
	colour = ( WHITE );
	bright = NORMAL;
	sprintf( buf2, "White" );
    }
    else if( !str_prefix( argument, "hi-white" ) )
    {
	colour = ( WHITE );
	bright = BRIGHT;
	sprintf( buf2, "White" );
    }
    else if( !str_prefix( argument, "grey" ) )
    {
	colour = ( BLACK );
	bright = BRIGHT;
	sprintf( buf2, "White" );
    }
    else
    {
	send_to_char_bw( "Unrecognised colour, unchanged.\n\r", ch );
	return;
    }

    ch->pcdata->text[1]		= colour;
    ch->pcdata->auction[1]	= colour;
    ch->pcdata->gossip[1]	= colour;
    ch->pcdata->music[1]	= colour;
    ch->pcdata->question[1]	= colour;
    ch->pcdata->answer[1]	= colour;
    ch->pcdata->quote[1]	= colour;
    ch->pcdata->quote_text[1]	= colour;
    ch->pcdata->immtalk_text[1]	= colour;
    ch->pcdata->immtalk_type[1]	= colour;
    ch->pcdata->info[1]		= colour;
    ch->pcdata->say[1]		= colour;
    ch->pcdata->say_text[1]	= colour;
    ch->pcdata->tell[1]		= colour;
    ch->pcdata->tell_text[1]	= colour;
    ch->pcdata->reply[1]	= colour;
    ch->pcdata->reply_text[1]	= colour;
    ch->pcdata->gtell_text[1]	= colour;
    ch->pcdata->gtell_type[1]	= colour;
    ch->pcdata->wiznet[1]	= colour;
    ch->pcdata->room_title[1]	= colour;
    ch->pcdata->room_text[1]	= colour;
    ch->pcdata->room_exits[1]	= colour;
    ch->pcdata->room_things[1]	= colour;
    ch->pcdata->prompt[1]	= colour;
    ch->pcdata->fight_death[1]	= colour;
    ch->pcdata->fight_yhit[1]	= colour;
    ch->pcdata->fight_ohit[1]	= colour;
    ch->pcdata->fight_thit[1]	= colour;
    ch->pcdata->fight_skill[1]	= colour;
    ch->pcdata->text[0]		= bright;
    ch->pcdata->auction[0]	= bright;
    ch->pcdata->gossip[0]	= bright;
    ch->pcdata->music[0]	= bright;
    ch->pcdata->question[0]	= bright;
    ch->pcdata->answer[0]	= bright;
    ch->pcdata->quote[0]	= bright;
    ch->pcdata->quote_text[0]	= bright;
    ch->pcdata->immtalk_text[0]	= bright;
    ch->pcdata->immtalk_type[0]	= bright;
    ch->pcdata->info[0]		= bright;
    ch->pcdata->say[0]		= bright;
    ch->pcdata->say_text[0]	= bright;
    ch->pcdata->tell[0]		= bright;
    ch->pcdata->tell_text[0]	= bright;
    ch->pcdata->reply[0]	= bright;
    ch->pcdata->reply_text[0]	= bright;
    ch->pcdata->gtell_text[0]	= bright;
    ch->pcdata->gtell_type[0]	= bright;
    ch->pcdata->wiznet[0]	= bright;
    ch->pcdata->room_title[0]	= bright;
    ch->pcdata->room_text[0]	= bright;
    ch->pcdata->room_exits[0]	= bright;
    ch->pcdata->room_things[0]	= bright;
    ch->pcdata->prompt[0]	= bright;
    ch->pcdata->fight_death[0]	= bright;
    ch->pcdata->fight_yhit[0]	= bright;
    ch->pcdata->fight_ohit[0]	= bright;
    ch->pcdata->fight_thit[0]	= bright;
    ch->pcdata->fight_skill[0]	= bright;

    sprintf( buf, "All Colour settings set to %s.\n\r", buf2 );
    send_to_char_bw( buf, ch );

    return;
}

/*
 * See if a string is one of the names of an object.
 */

bool is_full_name( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
        namelist = one_argument( namelist, name );
        if ( name[0] == '\0' )
            return FALSE;
        if ( !str_cmp( str, name ) )
            return TRUE;
    }
}

int get_hometown( char * argument )
{
	int i = 0;
	while ( hometown_table[i].name != NULL )
	{
		if (!str_cmp(argument,hometown_table[i].name))
			return i;
		i++;
	}
	
	return -1;
}

bool in_donation_room(OBJ_DATA *obj)
{
   if (!obj)
      return FALSE;

   if (obj->in_room)
   {
      if (IS_SET(obj->in_room->room_flags, ROOM_DONATION))
         return TRUE;
      else
         return FALSE;
   }

   if (obj->in_obj)
      if (obj->in_obj->in_room)
      {
         if (IS_SET(obj->in_obj->in_room->room_flags, ROOM_DONATION))
            return TRUE;
         else
            return FALSE;
      }

   if (obj->on)
      if (obj->on->in_room)
      {
         if (IS_SET(obj->on->in_room->room_flags, ROOM_DONATION))
            return TRUE;
         else
            return FALSE;
      }

   return FALSE;
}

 
CHAR_DATA *get_char_id(long id)
{
  CHAR_DATA *ch, *ch_next;

  for(ch = char_list; ch != NULL; ch = ch_next)
  {
    ch_next = ch->next;

    if(ch->id == id)
      break;      
  }

  return ch;
}
 
CHAR_DATA * get_char_area( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ach;
    int number, count;

    if ((ach = get_char_room( ch, NULL, argument )) != NULL)
	return ach;

    number = number_argument( argument, arg );
    count = 0;
    for ( ach = char_list; ach != NULL; ach = ach->next )
    {
	if (ach->in_room->area != ch->in_room->area
	||  !can_see( ch, ach ) || !is_name( arg, ach->name ))
	    continue;
	if (++count == number)
	    return ach;
    }
    return NULL;
}


const char *affect2_bit_name( int vector )
{
    static char buf[512];
    buf[0] = '\0';
/* if (vector & AFF_TEST_FLAG) strcat(buf, " test_flag"); */
    if ( vector & AFF_STEEL_SKIN  ) strcat( buf, " steel_skin"         );    
    if ( vector & AFF_TURNING     ) strcat( buf, " turning"            );
    if ( vector & AFF_BLOODLUST   ) strcat( buf, " bloodlust"          );
    if ( vector & AFF_DIVINE_PROTECTION ) strcat( buf, " divine_protection" );
    if ( vector & AFF_FORCE_SHIELD ) strcat( buf, " force_shield" );
    if ( vector & AFF_STATIC_SHIELD ) strcat( buf, " static_shield" );
    if ( vector & AFF_FLAME_SHIELD ) strcat( buf, " flame_shield" );
    if ( vector & AFF_LIFEFORCE ) strcat( buf, " lifeforce" );
    if ( vector & AFF_DARKSHIELD ) strcat( buf, " dark_shield" );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}


void affect_to_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;
    paf_new->next	= room->affected;
    room->affected	= paf_new;

    if ( paf->where == TO_AFFECTS )
	SET_BIT(room->affected_by,paf->bitvector);

    return;
}

void affect_remove_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf )
{
    int where, vector;
    if ( room->affected == NULL )
    {
        bug( "Affect_remove_room: no affect.", 0 );
        return;
    }

    where = paf->where;
    vector = paf->bitvector;

    if (paf->bitvector)
	switch( paf->where)
        {
        case TO_AFFECTS:
            REMOVE_BIT(room->affected_by,paf->bitvector);
            break;
        }

    if ( paf == room->affected )
    {    
        room->affected    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = room->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Affect_remove_room: cannot find paf.", 0 );
            return;
        }
    }

    free_affect(paf);

    return;

}


void check_spirit( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *spirit;

        /* only happens 1 in 25 times and only to NPCs */

    if ( number_range(0,24) != 0 || !IS_NPC(victim))
        return;

    spirit = create_mobile ( victim->pIndexData );
    SET_BIT ( spirit->form,
        FORM_INSTANT_DECAY|FORM_UNDEAD|FORM_INTANGIBLE );
    SET_BIT ( spirit->act, ACT_AGGRESSIVE );
    SET_BIT ( spirit->affected_by, AFF_PASS_DOOR );

    sprintf(buf,"the spirit of %s",victim->short_descr);
    spirit->short_descr = str_dup(buf);

    sprintf(buf,"spirit %s",victim->name);
    spirit->name = str_dup(buf);

    char_to_room( spirit, ch->in_room );

    act("You cower in fear as $N appears before you!",ch,NULL,spirit,TO_CHAR);
    act("$N suddenly appears and attacks $n!",ch,NULL,spirit,TO_ROOM);

    multi_hit( spirit, ch, TYPE_UNDEFINED );

    return;
}


/*
 * Return ascii name of an affect bit vector.
 */
const char *raffect_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}
                      
/*
 * Return true if a room is affected by a spell.
 */
bool is_affected_room( ROOM_INDEX_DATA *room, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = room->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->type == sn )
            return TRUE;
    }

    return FALSE;
}
          
/*
 * Strip all affects of a given sn.
 */
void affect_strip_room( ROOM_INDEX_DATA *room, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = room->affected; paf != NULL; paf = paf_next )
    {
        paf_next = paf->next;
        if ( paf->type == sn )
            affect_remove_room( room, paf );
    }

    return;
}
           
int strlen_color( char *argument )
{
    char        *str;
    int         length;

    if ( argument==NULL || argument[0]=='\0' )
        return 0;

    length=0;
    str=argument;        

     while ( *str != '\0' )
    {
        if ( *str != '{' )
        {
            str++;
            length++;
            continue;
        }

        if (*(++str) == '{')
            length++;

        str++;                 
    }

    return length;
}        

bool check_color_string(char *string ) 
{ 
	int i,length; 
	int count=1;
 
	/* Sanity check for empty strings */ 
	if ( *(string+0) == '\0') { 
		return FALSE; 
	}  

	/* Determine the length of the string. */
	length = strlen(string);

	/*
	 * subtacrt one from length to get to the last character
         * Then if the string doesn't end in {, we are done
	 */
	if(*(string+(--length)) != '{') {
		return FALSE;
	}

	/* Loop through the string backwards until we know how
         * many {'s are at the end.
	 * Also we already checked length so we can set i to length - 1
	 */
	for(i=length-1;i>=0;i--) {

		/* If the current character isn't a '{'
		 * We are done.
		 */
		if (*(string+i) != '{')
			break;

		/* If it is add to count and continue */
		if(*(string+(i-1)) == '{') {
			count++;
			continue;
		}
	}

	/* Now if we divide by two and have a remainder,
	 * It's an odd number of {'s and we need to kill
         * the last one, otherwise we can just leave it be.
         */
	if( count % 2 ) {
		*(string+length) = '\0';
		return TRUE;
	}

	return FALSE; 
}

const char *comm2_bit_name(int comm_flags)
{
    static char buf[512];

    buf[0] = '\0';     
  
    /*if (comm_flags & COMM_TEST         ) strcat(buf, " test_flag"); */  
    if (comm_flags & COMM_NOIMP          ) strcat(buf, " no_Imp");
    if (comm_flags & COMM_SHOW_ARMOR     ) strcat(buf, " show_armor");
    if (comm_flags & COMM_NONEWBIE       ) strcat(buf, " no_newbie");
    if (comm_flags & COMM_ANNOUNCE       ) strcat(buf, " no_announce");
    if (comm_flags & COMM_AUTOLEVEL      ) strcat(buf, " autolevel");
    if (comm_flags & COMM_PETLEVEL       ) strcat(buf, " petlevel");
    if (comm_flags & COMM_NOADMIN        ) strcat(buf, " no_admin");
    if (comm_flags & COMM_NOQUEST        ) strcat(buf, " no_quest");
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

void obj_to_box( OBJ_DATA *obj, CHAR_DATA *ch )
{
    obj->next_content           = ch->pcdata->box;
    ch->pcdata->box             = obj;
    ch->pcdata->item_box++;
    obj->carried_by             = ch;
    obj->in_room                = NULL;
    obj->in_obj                 = NULL;
}            


void obj_from_box( OBJ_DATA *obj )
{
    CHAR_DATA *ch;

    if ( ( ch = obj->carried_by ) == NULL )
    {
        bug( " Obj_from_char: null ch.", 0 );
        return;
    }          

    if ( ch->pcdata->box == obj )
        ch->pcdata->box = obj->next_content;
    else
    {
        OBJ_DATA *prev;

        for ( prev = ch->pcdata->box; prev != NULL; prev = prev->next_content )
        {
            if ( prev->next_content == obj )
            {
                prev->next_content = obj->next_content;
                break;
            }
        }

        if ( prev == NULL )
            bug( "Obj_from_char: obj not in list.", 0 );
    }       

    obj->carried_by             = NULL;
    obj->next_content           = NULL;
    ch->pcdata->item_box--;
    return;
}       

/*  returns the number of areas */
int get_area_list_size()
{
    AREA_DATA *pArea;
    int size = 0;
    for (pArea = area_first; pArea; pArea = pArea->next)
        ++size;
    return size;
}        

// compare_int -- function used by quicksort
static int compare_int(const void *int1, const void *int2)
{
    //Compare two integers (used during median-of-three partitioning).
    if (*(const int *)int1 > *(const int *)int2)
        return 1;
    else if (*(const int *)int1 < *(const int *)int2)
        return -1;
    else
        return 0;
}

// partition -- function used by quicksort
static int partition(void *data, int esize, int i, int k, int (*compare)
                     (const void *key1, const void *key2))
{
    char  *a = (char *)data;
    void  *pval, *temp;
    int   r[3];

    // Allocate storage for the partition value and swapping.
    if ((pval = malloc(esize)) == NULL)
        return -1;
    if ((temp = malloc(esize)) == NULL)
    {
        free(pval);
        return -1;
    }

    // Use the median-of-three method to find the partition value.
    r[0] = (rand() % (k - i + 1)) + i;
    r[1] = (rand() % (k - i + 1)) + i;
    r[2] = (rand() % (k - i + 1)) + i;
    issort(r, 3, sizeof(int), compare_int);
    memcpy(pval, &a[r[1] * esize], esize);

    // Create two partitions around the partition value.
    i--;
    k++;
    while (1)
    {
        // Move left until an element is found in the wrong partition.
        do
        {
            k--;
        } while (compare(&a[k * esize], pval) > 0);

        // Move right until an element is found in the wrong partition.
        do
        {
            i++;
        } while (compare(&a[i * esize], pval) < 0);

        if (i >= k)
        {
            // Stop partitioning when the left and right counters cross.
            break;
        }
        else
        {
            // Swap the elements now under the left and right counters.
            memcpy(temp, &a[i * esize], esize);
            memcpy(&a[i * esize], &a[k * esize], esize);
            memcpy(&a[k * esize], temp, esize);
        }
    }

    // Free the storage allocated for partitioning.
    free(pval);
    free(temp);

    // Return the position dividing the two partitions.
    return k;
}

int qksort(void *data, int size, int esize, int i, int k, int (*compare)
           (const void *key1, const void *key2))
{
    int j;

    // Stop the recursion when it is not possible to partition further.
    if (i < k)
    {
        // Determine where to partition the elements.
        if ((j = partition(data, esize, i, k, compare)) < 0)
            return -1;
        // Recursively sort the left partition.
        if (qksort(data, size, esize, i, j, compare) < 0)
            return -1;
        // Recursively sort the right partition.
        if (qksort(data, size, esize, j + 1, k, compare) < 0)
            return -1;
    }
    return 0;
}

int issort(void *data, int size, int esize, int (*compare)(const void *key1,
                                                           const void *key2))
{
    char               *a = (char *)data;
    void               *key;
    int                i, j;

    if ((key = (char *)malloc(esize)) == NULL)
        return -1;

    for (j = 1; j < size; j++)
    {
        memcpy(key, &a[j * esize], esize);
        i = j - 1;

        while (i >= 0 && compare(&a[i * esize], key) > 0)
        {
            memcpy(&a[(i + 1) * esize], &a[i * esize], esize);
            i--;
        }
        memcpy(&a[(i + 1) * esize], key, esize);
    }
    free(key);
    return 0;
}


char *PERS(CHAR_DATA *ch, CHAR_DATA *looker)
{
     NICK_DATA *pNick;
     int moon;
     static char buf[MSL];
     
     buf[0] = '\0';

        if ( looker == ch )
	    return ch->name;

        if(can_see( looker, ch ) )
        {
            if( IS_NPC(ch) )
	    return ch->short_descr; 

	    else
	    {
		if(ch->iswere)
		{

		    moon = moon_status();
		    switch (moon)
		    {
			case MOON_THREE:
				if ( IS_IMMORTAL ( looker ) )
		                        return ch->name;
				else
				 	return "An oddly formed, wolf-like creature";
			case MOON_FULL: 
				if ( IS_IMMORTAL ( looker ) )
		                        return ch->name;
				else
					return "A werewolf";
			default: break;
		    }
		}
	       	if(has_nick_for_ch(looker, ch) )
		{
		   for( pNick = looker->pcdata->nick_first; pNick ; pNick = pNick->next )
		       if(!str_cmp(pNick->realname, ch->name) )
                       {
                          if ( !IS_NPC( looker ) && IS_MXP( looker ) )
                          {
  			    SNP( buf, MXP_SECURE "<send href=\"click %s\">%s</SEND>" MXP_LLOCK, pNick->realname, ch->name );
                            return buf;
                          }
                          else
	 		    return pNick->nickname;
                       }
                }
		else if ( IS_IMMORTAL ( looker ) )
                {
                        if ( IS_MXP( looker ) )
                        {
 			SNP( buf, MXP_SECURE "<send href=\"click %s\">%s</SEND>" MXP_LLOCK, ch->name, ch->name );
                        return buf;
                        }
                        else
		 	return ch->name;
                } 
	    }
            if ( !IS_NPC( looker ) && IS_MXP( looker ) )
            {
            SNP( buf, MXP_SECURE "<send href=\"click %s\">%s</SEND>" MXP_LLOCK, ch->short_descr, ch->short_descr );
            return buf;
            }
            else
            return ch->short_descr;
	}

	return "someone";
}

bool has_nick_for_ch(CHAR_DATA *ch, CHAR_DATA *victim)
{
    NICK_DATA *pNick;
    
    if(IS_NPC(ch) || IS_NPC(victim) )
	return FALSE;
    for(pNick = ch->pcdata->nick_first; pNick ; pNick = pNick->next)
    {
	if(is_name( pNick->realname, victim->name) )
	    return TRUE;
    }
    return FALSE;
} 

bool CAN_WEAR(OBJ_DATA *obj, int part)
{
    CHAR_DATA *ch;
    ch = obj->carried_by;
  
    if(ch && ch->iswere && ch->wereform == MOON_FULL )
	return FALSE;
    if (IS_SET(obj->wear_flags,  part))
	return TRUE;

    return FALSE;
}


bool is_admin(CHAR_DATA *ch)
{
  int count;

  for (count = 0;count < MAX_ADMIN; count++)
  {
       if (!str_cmp (admin_table[count], ch->name))
           return TRUE;
  }
        
  return FALSE;
}            

void timeout( CHAR_DATA *ch )
{
  ROOM_INDEX_DATA *location;

  if ( ( location = get_room_index( hometown_table[ch->hometown].recall )) == NULL )
  {
        send_to_char( "You are completely lost.\n\r", ch );
        send_to_char( "Please let an imm know that you don't have a recall spot!\n\r", ch );
        return;
  }     
 
  ch->pcdata->timeout = 0;
  REMOVE_BIT( ch->act, PLR_TIMEOUT );
  char_from_room( ch );
  char_to_room( ch, location );
  do_look( ch, "auto" );
  send_to_char( "Ok your free try and follow the rules!\n\r", ch );
  info( ch, 0, "{G[INFO]:{x %s is no longer in a timeout! They can come out and play again!\n\r", ch->name );
  return;
}
  
int eye_lookup(const char *name)
{
    int eye;

    for ( eye = 0; eye_table[eye].name != NULL; eye++)
    {
         if (LOWER(name[0]) == LOWER(eye_table[eye].name[0])
         &&  !str_prefix( name,eye_table[eye].name))
             return eye;
    }

    return -1;
}


int hair_lookup(const char *name)
{
    int hair;

    for ( hair = 0; hair_table[hair].name != NULL; hair++)
    {
         if (LOWER(name[0]) == LOWER(hair_table[hair].name[0])
         &&  !str_prefix( name,hair_table[hair].name))
             return hair;
    }

    return -1;
}

int height_lookup(const char *name)
{
    int height;

    for ( height = 0; height_table[height].name != NULL; height++)
    {
         if (LOWER(name[0]) == LOWER(height_table[height].name[0])
         &&  !str_prefix( name,height_table[height].name))
             return height;
    }

    return -1;
}

int weight_lookup(const char *name)
{
    int weight;

    for ( weight = 0; weight_table[weight].name != NULL; weight++)
    {
         if (LOWER(name[0]) == LOWER(weight_table[weight].name[0])
         &&  !str_prefix( name,weight_table[weight].name))
             return weight;
    }

    return -1;
}


int levelflux_dam( CHAR_DATA *ch )
{  
   int bonus = 0;
  
   if ( ch->levelflux == 1 )
       bonus = dice( (ch->level / 15) + 4, 10 );

   if ( ch->levelflux == 2 )
       bonus = dice( ( ch->level / 14) + 4, 12 );
     
   if ( ch->levelflux == 3 )
       bonus = dice( ( ch->level / 12) + 4, 14 );

   if ( ch->levelflux == 4 )
       bonus = dice( ( ch->level / 10) + 4, 16 );
 
   if ( ch->levelflux == 5 )
       bonus = dice( ( ch->level / 8 ) + 4, 20 );

   return bonus;
}

MEMBER_DATA *member_lookup(CLAN_DATA *pClan, char *name)
{
    MEMBER_DATA *pMem;

    for(pMem = pClan->member_first ; pMem ; pMem = pMem->next)
    {
	if(!str_cmp(name, pMem->name) )
	    return pMem;
    }
   return NULL;
}

/* Multi klass functions start here */

/* Use for things like:
 *  ch->level >= skill_table[sn].skill_level[ch->klass]
 */
bool can_use_skpell ( CHAR_DATA * ch, int sn )
{
    int iClass;

    if ( IS_NPC ( ch ) )
        return TRUE;

    if ( is_race_skill ( ch, sn ) )
        return TRUE;

    for ( iClass = 0; ch->klass[iClass] != -1; iClass++ )
        if ( ch->level >= skill_table[sn].skill_level[ch->klass[iClass]] )
            return TRUE;

    return FALSE;
}         

/* Used for things like:
 *  class_table[ch->klass].fMana
 */
bool has_spells(CHAR_DATA * ch)
{
    int iClass;

    if (IS_NPC(ch))
	return FALSE;
    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
	if (class_table[iClass].fMana)
	    return TRUE;
    return FALSE;
}

/* Used for things like
 *   ch->klass == 2
 */
bool is_class(CHAR_DATA * ch, int klass)
{
    int iClass;

    if (IS_NPC(ch))
	return FALSE;

    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
    {
	if (ch->klass[iClass] == klass)
	    return TRUE;
    }

    return FALSE;
}

/* Used for things like:
 *   ch->klass == victim->klass
 */
bool is_same_class(CHAR_DATA * ch, CHAR_DATA * victim)
{
    int iClass;

    if (IS_NPC(ch) || IS_NPC(victim))
	return FALSE;

    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
    {
	if (ch->klass[iClass] == victim->klass[iClass])
	    return TRUE;
    }

    return FALSE;
}

/* Returns a users prime klass (first class) */
int prime_class(CHAR_DATA * ch)
{
    return ch->klass[0];
}

/* Returns the number of classes a user has */
int number_classes(CHAR_DATA * ch)
{
    int iClass;

    if (IS_NPC(ch))
	return 0;
    for (iClass = 0; ch->klass[iClass] != -1; iClass++);
    return iClass;
}

/* Outputs klass names in long format */
char *class_long(CHAR_DATA * ch)
{
    static char buf[512];
    int iClass;

    buf[0] = '\0';
    if (IS_NPC(ch))
	return "Mobile";
    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
    {
	strcat(buf, "/");
	strcat(buf, class_table[ch->klass[iClass]].name);
    }
    return buf + 1;
}

/* Outputs klass names in a 3 letter who format
 *  Ex. W+3  mean Warrior plus 3 remorts (4 classes in total)
 */
char *class_who(CHAR_DATA * ch)
{
    static char buf[512];

    buf[0] = '\0';
    if (IS_NPC(ch))
	return "Mob";

    if (number_classes(ch) > 1)
	sprintf(buf, "%c+%d", class_table[ch->klass[0]].who_name[0], number_classes(ch) - 1);
    else
	sprintf(buf, class_table[ch->klass[0]].who_name);
    return buf;
}

/* Outputs klass names in short format */
char *class_short(CHAR_DATA * ch)
{
    static char buf[512];
    int iClass;

    buf[0] = '\0';
    if (IS_NPC(ch))
	return "Mob";
    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
    {
	strcat(buf, "/");
	strcat(buf, class_table[ch->klass[iClass]].who_name);
    }
    return buf + 1;
}

/* Sends number of classes to a string, Used for saving. */
char *class_numbers(CHAR_DATA * ch, bool pSave)
{
    static char buf[512];
    char buf2[10];
    int iClass;

    buf[0] = '\0';

    if (IS_NPC(ch))
	return "0";

    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
    {
	strcat(buf, " ");
	sprintf(buf2, "%d", ch->klass[iClass]);
	strcat(buf, buf2);
    }
    if (pSave)
	strcat(buf, " -1");
    return buf + 1;
}

/* Used for things like:
 *   level = skill_table[sn].skill_level[ch->klass]
 * Finds the lowest skill level for all classes.
 */
int skill_level(CHAR_DATA * ch, int sn)
{
    int iClass;
    int tempskill = 999;

    if (is_race_skill(ch, sn))
	return 1;

    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
	tempskill = UMIN(tempskill, skill_table[sn].skill_level[ch->klass[iClass]]);

    return tempskill == 999 ? 0 : tempskill;
}

/* Used for things like:
 *   train = skill_table[sn].rating[ch->klass]
 * Finds the lowest skill rating for all classes.
 */
int skill_rating(CHAR_DATA * ch, int sn)
{
    int iClass;
    int temprate = 999;

    if (is_race_skill(ch, sn))
	return 2;

    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
	temprate = UMIN(temprate, skill_table[sn].rating[ch->klass[iClass]]);

    return temprate == 999 ? 0 : temprate;
}

/* Used for things like:
 *   train = group_table[sn].rating[ch->klass]
 * Finds the lowest group rating for all classes.
 */

int group_rating(CHAR_DATA * ch, int gn)
{
    int iClass;
    int temprate = 999;

    if (is_race_skill(ch, gn))
	return 2;

    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
	temprate = UMIN(temprate, group_table[gn].rating[ch->klass[iClass]]);

    return temprate == 999 ? 0 : temprate;
}

/* Used to find the max amount of hp gain for a klass in advance_level() */
int get_hp_gain(CHAR_DATA * ch)
{
    int iClass = 0;
    int gain = 0;
    int count = 0;

    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
    {
	gain = UMAX(gain, number_range(class_table[ch->klass[iClass]].hp_min, class_table[ch->klass[iClass]].hp_max));
	count++;
    }
    return number_range(gain, gain + count);
}

int hp_max(CHAR_DATA *ch)
{
    int iClass;
    int tmp = 0;

    for(iClass = 0; ch->klass[iClass] != -1; iClass++)
        tmp = UMAX(tmp, class_table[ch->klass[iClass]].hp_max);

    return tmp;
}

/* Used to find if a stat is a prime of a users classes */
bool is_prime_stat(CHAR_DATA * ch, int stat)
{
    int iClass = 0;

    if (IS_NPC(ch))
	return TRUE;

    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
    {
	if (class_table[ch->klass[iClass]].attr_prime == stat)
	    return TRUE;
    }
    return FALSE;
}

/* Adds all klass default groups while keeping the charge at 40 */
void add_default_groups(CHAR_DATA * ch)
{
    int iClass = 0;

    if (IS_NPC(ch))
	return;

    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
	if(class_table[ch->klass[iClass]].default_group != NULL)
	group_add(ch, class_table[ch->klass[iClass]].default_group, FALSE);

    ch->pcdata->points += 40;
}

/* Adds all klass base groups */
void add_base_groups(CHAR_DATA * ch)
{
    int iClass = 0;

    if (IS_NPC(ch))
	return;

    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
	if(class_table[ch->klass[iClass]].base_group != NULL)
	group_add(ch, class_table[ch->klass[iClass]].base_group, FALSE);
}

/* Returns TRUE if a group number is a klass base group
   used in creation so base groups never get dropped */
bool check_base_group(CHAR_DATA *ch, int gn)
{
    int iClass = 0;

    if(IS_NPC(ch))
	return FALSE;

	if(gn < 0 || gn > MAX_GROUP)
		return FALSE;

    for(iClass = 0; ch->klass[iClass] != -1; iClass++)
    {
	if(class_table[ch->klass[iClass]].base_group == NULL)
	continue;

	if(group_lookup(class_table[ch->klass[iClass]].base_group) == gn)
		return TRUE;
    }
    return FALSE;
}

/* returns TRUE if a skill number is a klass base skill 
    Used in creation so base skills never get dropped */
bool is_base_skill(CHAR_DATA *ch, int sn)
{
    int iClass = 0;
    int gn, x;

    if(IS_NPC(ch))
	return FALSE;

    if(sn < 0 || sn > MAX_SKILL)
	return FALSE;

    for(iClass = 0; ch->klass[iClass] != -1; iClass++)
    {
	if(class_table[ch->klass[iClass]].base_group == NULL)
	continue;

	if((gn = group_lookup(class_table[ch->klass[iClass]].base_group)) != -1)
	{
		for(x = 0; x < MAX_IN_GROUP; x++)
		{
		if(group_table[gn].spells[x] == NULL)
		break;

		if(skill_lookup(group_table[gn].spells[x]) == sn)
		return TRUE;
		}
	}
    }
    return FALSE;
}

/* Gets lowest thac00 for all classes */
int get_thac00(CHAR_DATA * ch)
{
    int temp = 999,
     iClass = 0;

    if (IS_NPC(ch))
	return 0;
    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
	temp = UMIN(temp, class_table[ch->klass[iClass]].thac0_00);

    return temp;
}

/* gets lowest thac32 for all classes */
int get_thac32(CHAR_DATA * ch)
{
    int temp = 999,
     iClass = 0;

    if (IS_NPC(ch))
	return 0;
    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
	temp = UMIN(temp, class_table[ch->klass[iClass]].thac0_32);

    return temp;
}

/* Used for race exp multiplications */
int class_mult(CHAR_DATA * ch)
{
    int temp = 999,
     iClass = 0;

    if (IS_NPC(ch))
	return 0;

    for (iClass = 0; ch->klass[iClass] != -1; iClass++)
	temp = UMIN(temp, pc_race_table[ch->race].class_mult[ch->klass[iClass]]);

    return temp;
}

/* Simple bonus function for eq levels */
int lvl_bonus(CHAR_DATA * ch)
{
    return MAX_CLASS + number_classes(ch);
}

/* Returns TRUE if skill number is a race skill 
   If you have clan skills you can do something simalar
   for those */
bool is_race_skill(CHAR_DATA * ch, int sn)
{
    int i;

    for (i = 0; i < 5; i++)
    {
	if (pc_race_table[ch->race].skills[i] == NULL)
	    continue;
	if (skill_lookup(pc_race_table[ch->race].skills[i]) == sn)
	    return TRUE;
	if (group_lookup(pc_race_table[ch->race].skills[i]) == sn)
	    return TRUE;
    }
    return FALSE;
}

/* Multiclass functions end here */


bool weather_move( CHAR_DATA *ch, ROOM_INDEX_DATA *to_room, EXIT_DATA *pexit )
{
    ROOM_INDEX_DATA *in_room = ch->in_room;

    if (weather_info.sky == SKY_BLIZZARD) 
    { 		
        if  (((in_room->sector_type == SECT_INSIDE) || (in_room->room_flags == ROOM_INDOORS)) 
            && ((to_room->sector_type != SECT_INSIDE )|| (to_room->room_flags != ROOM_INDOORS))
            &&((!IS_AFFECTED(ch, AFF_FLYING)) && (!IS_AFFECTED(ch,AFF_HASTE)) && (!IS_AFFECTED(ch,AFF_INFRARED))))
        {
            send_to_char( "Their is a blizzard going on outside you can't go out there like that!\n\r", ch );
            return FALSE;
        }

       if ( IS_AFFECTED ( ch, AFF_FLYING ) )
       {
           if ( number_range( 0, 100) >= 75  ) /* 25 % chance for these poor souls */
           {
               send_to_char( "You can't take any more flying. Have a seat and rest for a few minutes.\n\r", ch );
               ch->move *= .80;
               return FALSE;
           }
       }

       if ( IS_AFFECTED ( ch, AFF_INFRARED ) )
       {
           if ( number_range( 0, 100) >= 75  ) /* 25 % chance of sight loss */
           {
               send_to_char( "Your infravision begins to fail. Have a seat for a few minutes.\n\r", ch );
               ch->move *= .80;
               return FALSE;
           }
       }
    }
	 
    if (weather_info.sky == SKY_HAILSTORM) 
    { 		
        if  (((in_room->sector_type == SECT_INSIDE) || (in_room->room_flags == ROOM_INDOORS)) 
            && ((to_room->sector_type != SECT_INSIDE )|| (to_room->room_flags != ROOM_INDOORS))
            && ((!IS_AFFECTED(ch, AFF_FLYING)) 
            && (!IS_AFFECTED(ch,AFF_HASTE)) 
            && (!IS_AFFECTED(ch,AFF_INFRARED))))
        {
            send_to_char( "It's {YHailing{x outside rest, relax, wait the storm out.\n\r", ch );
            return FALSE;
        }

        if ( IS_AFFECTED ( ch, AFF_FLYING ) )
        {
            if ( number_range( 0, 100) >= 75  ) /* 25 % chance for these poor souls */
            {
                send_to_char( "Your can't take any more flying. Have a seat and rest for a few minutes.\n\r", ch );
	        ch->move *= .80;
                return FALSE;
            }

        }

        if ( IS_AFFECTED ( ch, AFF_INFRARED ) )
        {
            if ( number_range( 0, 100) >= 75  ) /* 25 % chance of sight loss */
            {
                send_to_char( "Your infravision begins to fail. Have a seat for a few minutes.\n\r", ch );
                ch->move *= .80;
                return FALSE;
            }

        }
    }
	 
    if (weather_info.sky == SKY_ICESTORM) 
    {  		
        if (((in_room->sector_type == SECT_INSIDE) || (in_room->room_flags == ROOM_INDOORS)) 
          && ((to_room->sector_type != SECT_INSIDE ) || (to_room->room_flags != ROOM_INDOORS))
          && ((!IS_AFFECTED(ch, AFF_FLYING)) &&
          (!IS_AFFECTED(ch,AFF_HASTE)) &&  (!IS_AFFECTED(ch,AFF_INFRARED))))
        {
           send_to_char( "Theirs an {BIceStorm{x outside rest, relax, wait the storm out.\n\r", ch );
           return FALSE;
        }

        if ( IS_AFFECTED ( ch, AFF_FLYING ) )
        {
            if ( number_range( 0, 100) >= 75  ) /* 25 % chance for these poor souls */
            {
                send_to_char( "Your can't take any more flying. Have a seat and rest for a few minutes.\n\r", ch );
                ch->move *= .80;
                return FALSE;
            }

        }

        if ( IS_AFFECTED ( ch, AFF_INFRARED ) )
        {
            if ( number_range( 0, 100) >= 75  ) /* 25 % chance of sight loss */
            {
                send_to_char( "Your infravision begins to fail. Have a seat for a few minutes.\n\r", ch );
                ch->move *= .80;
                return FALSE;
            }
        }
    }
	
    if (weather_info.sky == SKY_FOGGY) 
    { 		
        if  (((in_room->sector_type == SECT_INSIDE) || (in_room->room_flags == ROOM_INDOORS)) 
            && ((to_room->sector_type != SECT_INSIDE )|| (to_room->room_flags != ROOM_INDOORS))
            &&((!IS_AFFECTED(ch, AFF_FLYING)) &&
            (!IS_AFFECTED(ch,AFF_HASTE)) && (!IS_AFFECTED(ch,AFF_INFRARED))))
        {
             send_to_char( "You wouldn't be able to see where you were going, due to the {Cfog{x.\n\r", ch );
             return FALSE;
        }

        if ( IS_AFFECTED ( ch, AFF_FLYING ) )
        {
            if ( number_range( 0, 100) >= 75  ) /* 25 % chance for these poor souls */
            {
                send_to_char( "Your tired from flying. Have a seat and rest for a few minutes.\n\r", ch );
	        ch->move *= .80;
                return FALSE;
            }
        }
       
        if ( IS_AFFECTED ( ch, AFF_INFRARED ) )
        { 
            if ( number_range( 0, 100) >= 75  ) /* 25 % chance of sight loss */
            {
                send_to_char( "Your infravision begins to fail. Have a seat for a few minutes.\n\r", ch );
	        ch->move *= .80;
                return FALSE;
            }
        }
    }

    return TRUE;
}


CHAR_DATA *prog_char_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg );

    if(ch && pRoom )
  	return NULL;

    if(ch)
	rch = ch->in_room->people;
    else
	rch = pRoom->people;

    count  = 0;

    for ( ; rch != NULL; rch = rch->next_in_room )
    {
        if ( (ch && !can_see( ch, rch )) || !is_name( arg, rch->name ) )
            continue;
        if ( ++count == number )
            return rch;
    }

    return NULL;
}


void weather_check( CHAR_DATA *ch )
{
    if ( weather_info.sky == SKY_BLIZZARD && number_chance( 25 ) )
    {
        blizzard(ch);
    }
        
    if ( weather_info.sky == SKY_FOGGY && number_chance( 25 ) )
    {
        fog(ch);
    }

    if ( weather_info.sky == SKY_ICESTORM && number_chance( 25 ) )
    {      
        ice(ch);
        blizzard(ch);
    }	

    if ( weather_info.sky == SKY_CLOUDY && number_chance( 25 ) )
    {
        blizzard(ch);
    }

    return;
}

int temperature_change( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *in_room;
    AREA_DATA *pArea;

    in_room = ch->in_room; 
    pArea = ch->in_room->area;

    if ( pArea->temperature < pArea->min_temp && weather_info.sky == SKY_BLIZZARD )
    pArea->temperature--;

    return pArea->temperature;
}          

void dead_pet( CHAR_DATA *victim, PET_DATA *pEt )
{
    int amount = number_range( 1000, 1500 ); 
    int i;
 
    pEt = new_pet();
    pEt->name = str_dup(victim->name);
    pEt->cname = str_dup(victim->powner);
    pEt->short_desc = str_dup(victim->short_descr);
    pEt->long_desc = str_dup(victim->long_descr);
    pEt->cost = ( amount * victim->level ) / 2;
    pEt->vnum = victim->pIndexData->vnum;
    pEt->level  = victim->level;
    pEt->max_hit = victim->max_hit;
    pEt->max_mana = victim->max_mana;
    pEt->max_move = victim->max_move;
    pEt->exp = victim->exp;
    pEt->xp_tolevel = victim->xp_tolevel;
    pEt->levelflux = victim->levelflux;
    pEt->practice = victim->practice;
    pEt->train = victim->train;
    pEt->damroll = victim->damroll;
    pEt->hitroll = victim->hitroll;
    for (i = 0; i < 4; i++)
         pEt->armor[i] = victim->armor[i];    
    pEt->gold = victim->gold;
    pEt->silver = victim->silver;
    pEt->next = pet_list;
    pet_list = pEt; 
    save_pets();   
 
    return;   
}

void revived_pet( CHAR_DATA *pet, PET_DATA *pEt )
{
    int i;

    free_string( pet->name );
    pet->name = str_dup(pEt->name);
    pet->powner = str_dup(pEt->cname);
    pet->short_descr = str_dup(pEt->short_desc);
    pet->long_descr = str_dup(pEt->long_desc);
    pet->level = pEt->level;
    pet->max_hit = pEt->max_hit;
    pet->max_mana = pEt->max_mana;
    pet->max_move = pEt->max_move;
    pet->hit = pet->max_hit;
    pet->mana = pet->max_mana;
    pet->move = pet->max_move;
    pet->exp = pEt->exp;
    pet->xp_tolevel = pEt->xp_tolevel;
    pet->levelflux = pEt->levelflux;
    pet->practice = pEt->practice;
    pet->train = pEt->train;
    pet->damroll = pEt->damroll;
    pet->hitroll = pEt->hitroll;
    for (i = 0; i < 4; i++)
         pet->armor[i] = pEt->armor[i];    
    pet->gold = pEt->gold;
    pet->silver = pEt->silver;
    return;
}

const char *suppress_bit_name(int suppress_flags)
{
  static char buf[512];

  buf[0] = '\0';
  if(suppress_flags & SUPPRESS_FLAGS    ) strcat(buf, " flags");
  if(suppress_flags & SUPPRESS_FLAG_EFFECTS) strcat(buf, " flageffects");
  if(suppress_flags & SUPPRESS_DODGE    ) strcat(buf, " dodge");
  if(suppress_flags & SUPPRESS_BURN     ) strcat(buf, " burn");
  if(suppress_flags & SUPPRESS_STD      ) strcat(buf, " std");
  if(suppress_flags & SUPPRESS_SHIELD   ) strcat(buf, " shield");
  if(suppress_flags & SUPPRESS_SHIELD_COMP) strcat(buf, " shieldcomp");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}               

bool can_pack(CHAR_DATA *ch)
{
    OBJ_DATA *object;
    bool found;
 
    if ( ch->desc == NULL )
        return TRUE;
 
    if ( ch->level > HERO )
	return TRUE;

    /*
     * search the list of objects.
     */
    found = TRUE;

    for ( object = ch->carrying; object != NULL; object = object->next_content )
    {
    	if (object->pIndexData->vnum == OBJ_VNUM_SCHOOL_SATCHEL)
	    found = FALSE;
    }

    if (found)
	return TRUE;
 
    return FALSE;
}

void affect_join_obj2 (OBJ_DATA * obj, AFFECT_DATA * paf)
{
    AFFECT_DATA *paf_old;
    bool found;

    found = FALSE;
    for (paf_old = obj->affected; paf_old != NULL; paf_old = paf_old->next)
    {
        if (paf_old->type == paf->type)
        {
            paf->level = (paf->level += paf_old->level) / 2;
            paf->duration += paf_old->duration;
            paf->modifier += paf_old->modifier;
            affect_remove_obj (obj, paf_old);
            break;
        }
    }

    affect_to_obj(obj, paf);
    return;
}

