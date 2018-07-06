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
#include "recycle.h"
#include "const.h"

/*
 * Local functions.
 */
void	say_spell	args( ( CHAR_DATA *ch, int sn ) );

/* imported functions */
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void 	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
int     find_door       args( ( CHAR_DATA *ch, char *arg ) );       



/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( skill_table[sn].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}

int find_spell( CHAR_DATA *ch, const char *name )
{
    /* finds a spell the character can cast if possible */
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&  !str_prefix(name,skill_table[sn].name))
	{
	    if ( found == -1)
		found = sn;
	    if (can_use_skpell(ch, sn)
	    &&  ch->pcdata->learned[sn] > 0)
		    return sn;
	}
    }
    return found;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
	return -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( slot == skill_table[sn].slot )
	    return sn;
    }

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	abort( );
    }

    return -1;
}



/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *	old;
	char *	knew;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"		},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "ra",		"gru"		},
	{ "fresh",	"ima"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].knew );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf2, "$n utters the words, '%s'.", buf );
    sprintf( buf,  "$n utters the words, '%s'.", skill_table[sn].name );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
        {
	    act((!IS_NPC(rch) && ch->klass==rch->klass) ? buf : buf2,
	        ch, NULL, rch, TO_VICT );
        }
    }    

    return;
}


/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *victim, int dam_type )
{
    int save;

    save = 50 + ( victim->level - level) * 5 - victim->saving_throw * 2;
    if (IS_AFFECTED(victim,AFF_BERSERK))
	save += victim->level/2;

    switch(check_immune(victim,dam_type))
    {
	case IS_IMMUNE:		return TRUE;
	case IS_RESISTANT:	save += 2;	break;
	case IS_VULNERABLE:	save -= 2;	break;
    }

    if (!IS_NPC(victim) && has_spells(victim))
	save = 9 * save / 10;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

/* RT save for dispels */

bool saves_dispel( int dis_level, int spell_level, int duration)
{
    int save;
    
    if (duration == -1)
      spell_level += 5;  
      /* very hard to dispel permanent effects */

    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

/* co-routine for dispel magic and cancellation */

bool check_dispel( int dis_level, CHAR_DATA *victim, int sn)
{
    AFFECT_DATA *af;

    if (is_affected(victim, sn))
    {
        for ( af = victim->affected; af != NULL; af = af->next )
        {
            if ( af->type == sn )
            {
                if (!saves_dispel(dis_level,af->level,af->duration))
                {
                    affect_strip(victim,sn);
        	    if ( skill_table[sn].msg_off )
        	    {
            		send_to_char( skill_table[sn].msg_off, victim );
            		send_to_char( "\n\r", victim );
        	    }
		    return TRUE;
		}
		else
		    af->level--;
            }
        }
    }
    return FALSE;
}

/* for finding mana costs -- temporary version */
int mana_cost (CHAR_DATA *ch, int min_mana, int level)
{
    if (ch->level + 2 == level)
	return 1000;
    return UMAX(min_mana,(100/(2 + ch->level - level)));
}



/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn;
    int target;

    /*
     * Switched NPC's can cast spells, but others can't.
     */
    if ( IS_NPC(ch) && ch->desc == NULL)
	return;

    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Cast which what where?\n\r", ch );
	return;
    }

    if ((sn = find_spell(ch,arg1)) < 1
    ||  skill_table[sn].spell_fun == spell_null
    || (!IS_NPC(ch) && (!can_use_skpell(ch, sn)
    ||   		 ch->pcdata->learned[sn] == 0)))
    {
	send_to_char( "You don't know any spells of that name.\n\r", ch );
	return;
    }
  
    if ( ch->position < skill_table[sn].minimum_position )
    {
	send_to_char( "You can't concentrate enough.\n\r", ch );
	return;
    }

    if (IS_SET (ch->in_room->room_flags, ROOM_NOMAGIC))
    {
        send_to_char( "You utter the words ... But nothing happens.\n\r",ch);
        return;
    }

    if (ch->level + 2 == skill_level(ch, sn))
	mana = 50;
    else
    	mana = UMAX(
	    skill_table[sn].min_mana,
	    100 / ( 2 + ch->level - skill_level(ch, sn) ) );

    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
    target	= TARGET_NONE;
      
    switch ( skill_table[sn].target )
    {
    default:
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "Cast the spell on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, NULL,  target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}
/*
        if ( ch == victim )
        {
            send_to_char( "You can't do that to yourself.\n\r", ch );
            return;
        }
*/


	if ( !IS_NPC(ch) )
	{

            if (is_safe(ch,victim) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return; 
	    }
	check_killer(ch,victim);
	}

        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
	    send_to_char( "You can't do that on your own follower.\n\r",
		ch );
	    return;
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, NULL, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
	{
	    send_to_char( "You cannot cast this spell on another.\n\r", ch );
	    return;
	}

	vo = (void *) ch;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the spell be cast upon?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, target_name, ch ) ) == NULL )
	{
	    send_to_char( "You are not carrying that.\n\r", ch );
	    return;
	}

	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
	if (arg2[0] == '\0')
	{
	    if ((victim = ch->fighting) == NULL)
	    {
		send_to_char("Cast the spell on whom or what?\n\r",ch);
		return;
	    }
	
	    target = TARGET_CHAR;
	}
	else if ((victim = get_char_room(ch, NULL, target_name)) != NULL)
	{
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) /* check the sanity of the attack */
	{
	    if(is_safe_spell(ch,victim,FALSE) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return;
	    }

            if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
            {
                send_to_char( "You can't do that on your own follower.\n\r",
                    ch );
                return;
            }

	    if (!IS_NPC(ch))
		check_killer(ch,victim);

	    vo = (void *) victim;
 	}
	else if ((obj = get_obj_here(ch, NULL, target_name)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break; 

    case TAR_OBJ_CHAR_DEF:
        if (arg2[0] == '\0')
        {
            vo = (void *) ch;
            target = TARGET_CHAR;                                                 
        }
        else if ((victim = get_char_room(ch, NULL, target_name)) != NULL)
        {
            vo = (void *) victim;
            target = TARGET_CHAR;
	}
	else if ((obj = get_obj_carry(ch,target_name,ch)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break;
    }
	    
    if ( !IS_NPC(ch) && ch->mana < mana )
    {
	send_to_char( "You don't have enough mana.\n\r", ch );
        sound( "nomana.wav", ch );
	return;
    }
      
    if ( str_cmp( skill_table[sn].name, "ventriloquate" ) )
	say_spell( ch, sn );
      
    WAIT_STATE( ch, skill_table[sn].beats );
      
    if ( number_percent( ) > get_skill(ch,sn) )
    {
	send_to_char( "You lost your concentration.\n\r", ch );
	check_improve(ch,sn,FALSE,1);
	ch->mana -= mana / 2;
    }
    else
    {
        ch->mana -= mana;
        if (IS_NPC(ch) || has_spells(ch)) 
	/* klass has spells */
            (*skill_table[sn].spell_fun) ( sn, ch->level + ch->cast_level, ch, vo,target);
        else
            (*skill_table[sn].spell_fun) (sn, (3 * ch->level/4) + ch->cast_level, ch, vo,target);
        check_improve(ch,sn,TRUE,1);
    }

    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch)
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {	check_killer(victim,ch);
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}



/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo;
    int target = TARGET_NONE;

    if ( sn <= 0 )
	return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return;
    }

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	if (is_safe(ch,victim) && ch != victim)
	{
	    send_to_char("Something isn't right...\n\r",ch);
	    return;
	}
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
	if ( victim == NULL )
	    victim = ch;
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( obj == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
        if ( victim == NULL && obj == NULL)
        {
	    if (ch->fighting != NULL)
		victim = ch->fighting;
	    else
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }
        }
	    if (victim != NULL)
	    {
		if (is_safe_spell(ch,victim,FALSE) && ch != victim)
		{
		    send_to_char("Somehting isn't right...\n\r",ch);
		    return;
		}

		vo = (void *) victim;
		target = TARGET_CHAR;
	    }
	    else
	    {
	    	vo = (void *) obj;
	    	target = TARGET_OBJ;
	    }
        break;


    case TAR_OBJ_CHAR_DEF:
	if (victim == NULL && obj == NULL)
	{
	    vo = (void *) ch;
	    target = TARGET_CHAR;
	}
	else if (victim != NULL)
	{
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	}
	else
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	
	break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo,target);

    

    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		check_killer(victim,ch);
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}



/*
 * Spell functions.
 */
void spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 12 );
    if ( saves_spell( level, victim, DAM_ACID ) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_ACID, TRUE);
    return;
}



void spell_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already armored.\n\r",ch);
	else
	  act("$N is already armored.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    if(!IS_NPC(ch))
    af.duration = 24 + ch->pcdata->duration;
    else
    af.duration  = 24;
    af.modifier  = -20;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel someone protecting you.\n\r", victim );
    sound( "Armoron.wav", ch );
    sound( "Armoron.wav", victim );
    if ( ch != victim )
	act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;
	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	{
	    act("$p is already blessed.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (IS_OBJ_STAT(obj,ITEM_EVIL))
	{
	    AFFECT_DATA *paf;

	    paf = affect_find(obj->affected,skill_lookup("curse"));
	    if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
	    {
		if (paf != NULL)
		    affect_remove_obj(obj,paf);
		act("$p glows a pale blue.",ch,obj,NULL,TO_ALL);
		REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
		return;
	    }
	    else
	    {
		act("The evil of $p is too powerful for you to overcome.",
		    ch,obj,NULL,TO_CHAR);
		return;
	    }
	}
	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= 6 + level;
	af.location	= APPLY_SAVES;
	af.modifier	= -1;
	af.bitvector	= ITEM_BLESS;
	affect_to_obj(obj,&af);

	act("$p glows with a holy aura.",ch,obj,NULL,TO_ALL);

	if (obj->wear_loc != WEAR_NONE)
	    ch->saving_throw -= 1;
	return;
    }

    /* character target */
    victim = (CHAR_DATA *) vo;


    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already blessed.\n\r",ch);
	else
	  act("$N already has divine favor.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 6+level;
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 8;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    affect_to_char( victim, &af );
    send_to_char( "You feel righteous.\n\r", victim );
    if ( ch != victim )
	act("You grant $N the favor of your god.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_BLIND) || saves_spell(level,victim,DAM_OTHER))
	return;


    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = 1+level;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
    send_to_char( "You are blinded!\n\r", victim );
    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_burning_hands(int sn,int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,	14,	17, 20, 23, 26, 29,
	29, 29, 30, 30,	31,	31, 32, 32, 33, 33,
	34, 34, 35, 35,	36,	36, 37, 37, 38, 38,
	39, 39, 40, 40,	41,	41, 42, 42, 43, 43,
	44, 44, 45, 45,	46,	46, 47, 47, 48, 48
    };
    int dam;

    level	= UMIN(level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_FIRE) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_FIRE,TRUE);
    sound( "burninghand.wav", ch );
    if ( ch != victim )
    sound( "burninghand.wav", victim );
    return;
}



void spell_call_lightning( int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You must be out of doors.\n\r", ch );
	return;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char( "You need bad weather.\n\r", ch );
	return;
    }

    dam = dice(level/2, 8);

    printf_to_char( ch, "ShadowStorm's lightning strikes your foes!\n\r" );
    act( "$n calls ShadowStorm's lightning to strike $s foes!",
	ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
		damage( ch, vch, saves_spell( level,vch,DAM_LIGHTNING) 
		? dam / 2 : dam, sn,DAM_LIGHTNING,TRUE);
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) )
	    send_to_char( "Lightning flashes in the sky.\n\r", vch );
    }

    return;
}

/* RT calm spell stops all fighting in the room */

void spell_calm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;    
    int chance;
    AFFECT_DATA af;

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
	    if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) ||
				IS_SET(vch->act,ACT_UNDEAD)))
	      return;

	    if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
	    ||  is_affected(vch,skill_lookup("frenzy")))
	      return;
	    
	    send_to_char("A wave of calm passes over you.\n\r",vch);

	    if (vch->fighting || vch->position == POS_FIGHTING)
	      stop_fighting(vch,FALSE);


	    af.where = TO_AFFECTS;
	    af.type = sn;
  	    af.level = level;
	    af.duration = level/4;
	    af.location = APPLY_HITROLL;
	    if (!IS_NPC(vch))
	      af.modifier = -5;
	    else
	      af.modifier = -2;
	    af.bitvector = AFF_CALM;
	    affect_to_char(vch,&af);

	    af.location = APPLY_DAMROLL;
	    affect_to_char(vch,&af);
	}
    }
}

void spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
 
    level += 2;

    if ((!IS_NPC(ch) && IS_NPC(victim) && 
	 !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) ) ||
        (IS_NPC(ch) && !IS_NPC(victim)) )
    {
	send_to_char("You failed, try dispel magic.\n\r",ch);
	return;
    }

    /* unlike dispel magic, the victim gets NO save */
 
    /* begin running through the spells */
 
    if (check_dispel(level,victim,skill_lookup("armor")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("calm")))
    {
	found = TRUE;
	act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel (level, victim, skill_lookup ("force shield")))
    {
        act ("The force-shield encircling $n fades.", victim, NULL, NULL,
             TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("static shield")))
    {
        act ("The static energy surrounding $n dissipates.", victim, NULL, NULL,
             TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("flame shield")))
    {
        act ("The flames protecting $n sputter and die.", victim, NULL, NULL,
             TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
	act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("haste")))
    {
	act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection good")))
        found = TRUE; 
 
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    damage( ch, (CHAR_DATA *) vo, dice(1, 8) + level / 3, sn,DAM_HARM,TRUE);
    return;
}



void spell_cause_critical(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    damage( ch, (CHAR_DATA *) vo, dice(3, 8) + level - 6, sn,DAM_HARM,TRUE);
    return;
}



void spell_cause_serious(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    damage( ch, (CHAR_DATA *) vo, dice(2, 8) + level / 2, sn,DAM_HARM,TRUE);
    return;
}

void spell_chain_lightning(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam;

    /* first strike */

    act("A lightning bolt leaps from $n's hand and arcs to $N.",
        ch,NULL,victim,TO_ROOM);
    act("A lightning bolt leaps from your hand and arcs to $N.",
	ch,NULL,victim,TO_CHAR);
    act("A lightning bolt leaps from $n's hand and hits you!",
	ch,NULL,victim,TO_VICT);  

    dam = dice(level,6);
    if (saves_spell(level,victim,DAM_LIGHTNING))
 	dam /= 3;
    damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE);
    last_vict = victim;
    level -= 4;   /* decrement damage */

    /* new targets */
    while (level > 0)
    {
	found = FALSE;
	for (tmp_vict = ch->in_room->people; 
	     tmp_vict != NULL; 
	     tmp_vict = next_vict) 
	{
	  next_vict = tmp_vict->next_in_room;
	  if (!is_safe_spell(ch,tmp_vict,TRUE) && tmp_vict != last_vict)
	  {
	    found = TRUE;
	    last_vict = tmp_vict;
	    act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);
	    act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);
	    dam = dice(level,6);
	    if (saves_spell(level,tmp_vict,DAM_LIGHTNING))
		dam /= 3;
	    damage(ch,tmp_vict,dam,sn,DAM_LIGHTNING,TRUE);
	    level -= 4;  /* decrement damage */
	  }
	}   /* end target searching loop */
	
	if (!found) /* no target found, hit the caster */
	{
	  if (ch == NULL)
     	    return;

	  if (last_vict == ch) /* no double hits */
	  {
	    act("The bolt seems to have fizzled out.",ch,NULL,NULL,TO_ROOM);
	    act("The bolt grounds out through your body.",
		ch,NULL,NULL,TO_CHAR);
	    return;
	  }
	
	  last_vict = ch;
	  act("The bolt arcs to $n...whoops!",ch,NULL,NULL,TO_ROOM);
	  send_to_char("You are struck by your own lightning!\n\r",ch);
	  dam = dice(level,6);
	  if (saves_spell(level,ch,DAM_LIGHTNING))
	    dam /= 3;
	  damage(ch,ch,dam,sn,DAM_LIGHTNING,TRUE);
	  level -= 4;  /* decrement damage */
	  if (ch == NULL) 
	    return;
	}
    /* now go back and find more targets */
    }
}
	  

void spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ))
    {
	if (victim == ch)
	  send_to_char("You've already been changed.\n\r",ch);
	else
	  act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if (saves_spell(level , victim,DAM_OTHER))
	return;	
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2 * level;
    af.location  = APPLY_SEX;
    do
    {
	af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel different.\n\r", victim );
    act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_safe(ch,victim)) return;

    if ( victim == ch )
    {
	send_to_char( "You like yourself even better!\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level, victim,DAM_CHARM) )
	return;


    if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
    {
	send_to_char(
	    "The mayor does not allow charming in the city limits.\n\r",ch);
	return;
    }
  
    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
    sound( "CHARMYES.wav", ch );
    sound( "CHARMYES.wav", victim );
    if ( ch != victim )
	act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  6,  7,  8,	 9, 12, 13, 13, 13,
	14, 14, 14, 15, 15,	15, 16, 16, 16, 17,
	17, 17, 18, 18, 18,	19, 19, 19, 20, 20,
	20, 21, 21, 21, 22,	22, 22, 23, 23, 23,
	24, 24, 24, 25, 25,	25, 26, 26, 26, 27
    };
    AFFECT_DATA af;
    int dam;

    level	= UMIN(level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( !saves_spell( level, victim,DAM_COLD ) )
    {
	act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
	af.where     = TO_AFFECTS;
	af.type      = sn;
        af.level     = level;
	af.duration  = 6;
	af.location  = APPLY_STR;
	af.modifier  = -1;
	af.bitvector = 0;
	affect_join( victim, &af );
    }
    else
    {
	dam /= 2;
    }

    damage( ch, victim, dam, sn, DAM_COLD,TRUE );
    return;
}



void spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0,  0,  0,
	30, 35, 40, 45, 50,	55, 55, 55, 56, 57,
	58, 58, 59, 60, 61,	61, 62, 63, 64, 64,
	65, 66, 67, 67, 68,	69, 70, 70, 71, 72,
	73, 73, 74, 75, 76,	76, 77, 78, 79, 79
    };
    int dam;

    level	= UMIN(level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2,  dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_LIGHT) )
	dam /= 2;
    else 
	spell_blindness(skill_lookup("blindness"),
	    level/2,ch,(void *) victim,TARGET_CHAR);

    damage( ch, victim, dam, sn, DAM_LIGHT,TRUE );
    return;
}



void spell_continual_light(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *light;

    if (target_name[0] != '\0')  /* do a glow on some object */
    {
	light = get_obj_carry(ch,target_name,ch);
	
	if (light == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}

	if (IS_OBJ_STAT(light,ITEM_GLOW))
	{
	    act("$p is already glowing.",ch,light,NULL,TO_CHAR);
	    return;
	}

	SET_BIT(light->extra_flags,ITEM_GLOW);
	act("$p glows with a white light.",ch,light,NULL,TO_ALL);
	return;
    }

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    light->carried_by = ch;
    obj_to_room( light, ch->in_room );
    act( "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM );
    act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
    return;
}

void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj;
    int food;

    food = number_range(1, 13);

    obj = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );

    free_string(obj->name);
    free_string(obj->short_descr);
    free_string(obj->description);
    
    switch( food )
    {
	default:
	case 1: 
	    obj->name = str_dup("curled fern tip'");
	    obj->short_descr = str_dup("{ca curled {Gfe{grn t{Gip{x");
	    obj->description = str_dup("{cA fresh, curled {Gfe{grn t{Gip{c has been gathered here.{x");
	    break;
	case 2: 
	    obj->name = str_dup("dandelion leaf");
	    obj->short_descr = str_dup("{ca {Yd{ya{wn{Ydel{wi{yo{Yn {gleaf{x");
	    obj->description = str_dup("{cA gathered {Yd{ya{wn{Ydel{wi{yo{Yn {gleaf {clies here to be eaten.{x");
	    break;
	case 3: 
	    obj->name = str_dup("young pokeweed shoots");
	    obj->short_descr = str_dup("{cyoung pokeweed {gs{Gh{goo{Gt{gs{x");
	    obj->description = str_dup("{cSome young pokeweed {gs{Gh{goo{Gt{gs{c lie about, deliciously edible.{x");
	    break;
	case 4: 
	    obj->name = str_dup("pine nut");
	    obj->short_descr = str_dup("{ca {yp{Yi{yne {cn{yu{ct{x");
	    obj->description = str_dup("{cA harvested {yp{Yi{yne {cn{yu{ct is gathered here.{x");
	    break;
	case 5: 
	    obj->name = str_dup("cattail root");
	    obj->short_descr = str_dup("{ca {yc{Ya{ctta{Yi{yl r{Do{wo{yt{x");
	    obj->description = str_dup("{cA dug {yc{Ya{ctta{Yi{yl r{Do{wo{yt{c lies here invitingly.{x");
	    break;
	case 6: 
	    obj->name = str_dup("clover leaves");
	    obj->short_descr = str_dup("{gcl{Go{yv{ger l{Ge{gav{Ge{gs{x");
	    obj->description = str_dup("{cSome gathered {gcl{Go{yv{ger l{Ge{gav{Ge{gs{c lie in a small pile.{x");
	    break;
	case 7: 
	    obj->name = str_dup("thistle root");
	    obj->short_descr = str_dup("{ca {mt{Mh{mist{Ml{me {yr{coo{yt{x");
	    obj->description = str_dup("{cA fresh {mt{Mh{mist{Ml{me {yr{coo{yt {clies here, recently dug.{x");
	    break;
	case 8: 
	    obj->name = str_dup("mint leaf");
	    obj->short_descr = str_dup("{ca {Gm{Wi{gn{Gt {gl{cea{gf{x");
	    obj->description = str_dup("{cA crisp {Gm{Wi{gn{Gt {gl{cea{gf{c lies here, fragrant and fresh.{x");
	    break;
	case 9: 
	    obj->name = str_dup("green onion");
	    obj->short_descr = str_dup("{ca {Gg{yr{ge{ye{Gn{c o{gnio{cn{x");
	    obj->description = str_dup("{cA young {Gg{yr{ge{ye{Gn{c o{gnio{cn has been harvested here.{x");
	    break;
	case 10: 
	    obj->name = str_dup("redbud flower");
	    obj->short_descr = str_dup("{ca {Mre{mdb{Mud{c flower{x");
	    obj->description = str_dup("{cAn edible {Mre{mdb{Mud{c flower rests here.{x");
	    break;
	case 11: 
	    obj->name = str_dup("fennel seeds");
	    obj->short_descr = str_dup("{Yf{yenne{Yl{c seeds{x");
	    obj->description = str_dup("{cA handful of {Yf{yenne{Yl{c seeds lies gathered here.{x");
	    break;
	case 12: 
	    obj->name = str_dup("rose hips");
	    obj->short_descr = str_dup("{Rr{Wo{ws{re h{wi{Wp{Rs{x");
	    obj->description = str_dup("{cSeveral wild {Rr{Wo{ws{re h{wi{Wp{Rs{c have been gathered here.{x");
	    break;
	case 13: 
	    obj->name = str_dup("milkweed blossom");
	    obj->short_descr = str_dup("{ca {Wm{wilkwee{Wd{c blossom{x");
	    obj->description = str_dup("{cA gathered {Wm{wilkwee{Wd{c blossom lies here.{x");
	    break;

    }

    obj->value[0] = level / 2;
    obj->value[1] = level;
    obj->carried_by = ch;
    obj_to_room( obj, ch->in_room );
    act( "$p suddenly appears.", ch, obj, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, obj, NULL, TO_CHAR );
    return;
}

void spell_create_rose( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *rose;
    rose = create_object(get_obj_index(OBJ_VNUM_ROSE), 0);
    act("$n has created a beautiful red rose.",ch,rose,NULL,TO_ROOM);
    send_to_char("You create a beautiful red rose.\n\r",ch);
    obj_to_char(rose,ch);
    return;
}

void spell_create_spring(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *spring;

    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level;
    spring->carried_by = ch;
    obj_to_room( spring, ch->in_room );
    act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
    act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
    return;
}



void spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "It is unable to hold water.\n\r", ch );
	return;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
	send_to_char( "It contains some other liquid.\n\r", ch );
	return;
    }

    water = UMIN(
		level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1]
		);
  
    if ( water > 0 )
    {
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "%s water", obj->name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	}
	act( "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return;
}



void spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, skill_lookup("blindness") ) )
    {
        if (victim == ch)
          send_to_char("You aren't blind.\n\r",ch);
        else
          act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        send_to_char( "Your vision returns!\n\r", victim );
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}



void spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(3, 8) + level - 6;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

/* RT added to cure plague */
void spell_cure_disease( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, skill_lookup("plague") ) )
    {
        if (victim == ch)
          send_to_char("You aren't ill.\n\r",ch);
        else
          act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if (check_dispel(level,victim,skill_lookup("plague")))
    {
	send_to_char("Your sores vanish.\n\r",victim);
	act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
    }
    else
	send_to_char("Spell failed.\n\r",ch);
}



void spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(1, 8) + level / 3;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
 
    if ( !is_affected( victim, skill_lookup("poison") ) )
    {
        if (victim == ch)
          send_to_char("You aren't poisoned.\n\r",ch);
        else
          act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level,victim,skill_lookup("poison")))
    {
        send_to_char("A warm feeling runs through your body.\n\r",victim);
        act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(2, 8) + level /2 ;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;
        if (IS_OBJ_STAT(obj,ITEM_EVIL))
        {
            act("$p is already filled with evil.",ch,obj,NULL,TO_CHAR);
            return;
        }

        if (IS_OBJ_STAT(obj,ITEM_BLESS))
        {
            AFFECT_DATA *paf;

            paf = affect_find(obj->affected,skill_lookup("bless"));
            if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
            {
                if (paf != NULL)
                    affect_remove_obj(obj,paf);
                act("$p glows with a red aura.",ch,obj,NULL,TO_ALL);
                REMOVE_BIT(obj->extra_flags,ITEM_BLESS);
                return;
            }
            else
            {
                act("The holy aura of $p is too powerful for you to overcome.",
                    ch,obj,NULL,TO_CHAR);
                return;
            }
        }

        af.where        = TO_OBJECT;
        af.type         = sn;
        af.level        = level;
        af.duration     = 2 * level;
        af.location     = APPLY_SAVES;
        af.modifier     = +1;
        af.bitvector    = ITEM_EVIL;
        affect_to_obj(obj,&af);

        act("$p glows with a malevolent aura.",ch,obj,NULL,TO_ALL);

	if (obj->wear_loc != WEAR_NONE)
	    ch->saving_throw += 1;
        return;
    }

    /* character curses */
    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(victim,AFF_CURSE) || saves_spell(level,victim,DAM_NEGATIVE))
	return;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2*level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (level / 8);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
	act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
    return;
}

/* RT replacement demonfire spell */

void spell_demonfire(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( !IS_NPC(ch) && !IS_EVIL(ch) )
    {
        victim = ch;
	send_to_char("The demons turn upon you!\n\r",ch);
    }


    if (victim != ch)
    {
	act("$n calls forth the demons of Hell upon $N!",
	    ch,NULL,victim,TO_ROOM);
        act("$n has assailed you with the demons of Hell!",
	    ch,NULL,victim,TO_VICT);
	send_to_char("You conjure forth the demons of hell!\n\r",ch);
    }
    dam = dice( level, 10 );
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
    spell_curse(skill_lookup("curse"), 3 * level / 4, ch, (void *) victim,TARGET_CHAR);
}

void spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) )
    {
	if (victim == ch)
	  send_to_char("You can already sense evil.\n\r",ch);
	else
	  act("$N can already detect evil.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}


void spell_detect_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_DETECT_GOOD) )
    {
        if (victim == ch)
          send_to_char("You can already sense good.\n\r",ch);
        else
          act("$N can already detect good.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_hidden(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
    {
        if (victim == ch)
          send_to_char("You are already as alert as you can be. \n\r",ch);
        else
          act("$N can already sense hidden lifeforms.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
        if (victim == ch)
          send_to_char("You can already see invisible.\n\r",ch);
        else
          act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
    {
        if (victim == ch)
          send_to_char("You can already sense magical auras.\n\r",ch);
        else
          act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_char( "You smell poisonous fumes.\n\r", ch );
	else
	    send_to_char( "It looks delicious.\n\r", ch );
    }
    else
    {
	send_to_char( "It doesn't look poisoned.\n\r", ch );
    }

    return;
}



void spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
  
    if ( !IS_NPC(ch) && IS_EVIL(ch) )
	victim = ch;
  
    if ( IS_GOOD(victim) )
    {
	act( "Synon protects $N.", ch, NULL, victim, TO_ROOM );
	return;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (victim->hit > (ch->level * 4))
      dam = dice( level, 4 );
    else
      dam = UMAX(victim->hit, dice(level,4));
    if ( saves_spell( level, victim,DAM_HOLY) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    return;
}


void spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    if ( !IS_NPC(ch) && IS_GOOD(ch) )
        victim = ch;
 
    if ( IS_EVIL(victim) )
    {
        act( "$N is protected by $S evil.", ch, NULL, victim, TO_ROOM );
        return;
    }
 
    if ( IS_NEUTRAL(victim) )
    {
        act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
        return;
    }
 
    if (victim->hit > (ch->level * 4))
      dam = dice( level, 4 );
    else
      dam = UMAX(victim->hit, dice(level,4));
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
    return;
}


/* modified for enhanced use */

void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    if (saves_spell(level, victim,DAM_OTHER))
    {
	send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	send_to_char( "You failed.\n\r", ch);
	return;
    }

    /* begin running through the spells */ 

    if (check_dispel(level,victim,skill_lookup("armor")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("calm")))
    {
        found = TRUE;
        act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
        act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("haste")))
    {
        act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;
 

    if (check_dispel(level,victim,skill_lookup("protection evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (IS_AFFECTED(victim,AFF_SANCTUARY) 
	&& !saves_dispel(level, victim->level,-1)
	&& !is_affected(victim,skill_lookup("sanctuary")))
    {
	REMOVE_BIT(victim->affected_by,AFF_SANCTUARY);
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
	return;
}

void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char( "The earth trembles beneath your feet!\n\r", ch );
    act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
            {
		if (IS_AFFECTED(vch,AFF_FLYING))
		    damage(ch,vch,0,sn,DAM_BASH,TRUE);
		else
		    damage( ch,vch,level + dice(2, 8), sn, DAM_BASH,TRUE);
            }
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "The earth trembles and shivers.\n\r", vch );
    }

    return;
}

void spell_enchant_armor( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf; 
    int result, fail;
    int ac_bonus, added;
    bool ac_found = FALSE;

    if (obj->item_type != ITEM_ARMOR)
    {
	send_to_char("That isn't an armor.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	send_to_char("The item must be carried to be enchanted.\n\r",ch);
	return;
    }

    if (IS_OBJ_STAT(obj, ITEM_QUEST))
    {
	send_to_char("You can't enchant quest items.\n\r", ch);
	return;
    }

    /* this means they have no bonus */
    ac_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_AC )
            {
	    	ac_bonus = paf->modifier;
		ac_found = TRUE;
	    	fail += 5 * (ac_bonus * ac_bonus);
 	    }

	    else  /* things get a little harder */
	    	fail += 20;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_AC )
  	{
	    ac_bonus = paf->modifier;
	    ac_found = TRUE;
	    fail += 5 * (ac_bonus * ac_bonus);
	}

	else /* things get a little harder */
	    fail += 20;
    }

    /* apply other modifiers */
    fail -= level;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,85);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 3)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen.\n\r",ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
	{
	    af_new = new_affect();
	
	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where	= paf->where;
	    af_new->type 	= UMAX(0,paf->type);
	    af_new->level	= paf->level;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (90 - level/5))  /* success! */
    {
	act("$p shimmers with a gold aura.",ch,obj,NULL,TO_CHAR);
	act("$p shimmers with a gold aura.",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = -1;
    }
    
    else  /* exceptional enchant */
    {
	act("$p glows a brillant gold!",ch,obj,NULL,TO_CHAR);
	act("$p glows a brillant gold!",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = -2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO)
	obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

    if (ac_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_AC)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
	    }
	}
    }
    else /* add a new affect */
    {
 	paf = new_affect();

	paf->where	= TO_OBJECT;
	paf->type	= sn;
	paf->level	= level;
	paf->duration	= -1;
	paf->location	= APPLY_AC;
	paf->modifier	=  added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
    }

}




void spell_enchant_weapon(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf; 
    int result, fail;
    int hit_bonus, dam_bonus, added;
    bool hit_found = FALSE, dam_found = FALSE;

    if (obj->item_type != ITEM_WEAPON)
    {
	send_to_char("That isn't a weapon.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	send_to_char("The item must be carried to be enchanted.\n\r",ch);
	return;
    }

    if (IS_OBJ_STAT(obj, ITEM_QUEST))
    {
	send_to_char("You can't enchant quest items.\n\r", ch);
	return;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_HITROLL )
            {
	    	hit_bonus = paf->modifier;
		hit_found = TRUE;
	    	fail += 2 * (hit_bonus * hit_bonus);
 	    }

	    else if (paf->location == APPLY_DAMROLL )
	    {
	    	dam_bonus = paf->modifier;
		dam_found = TRUE;
	    	fail += 2 * (dam_bonus * dam_bonus);
	    }

	    else  /* things get a little harder */
	    	fail += 25;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
  	{
	    hit_bonus = paf->modifier;
	    hit_found = TRUE;
	    fail += 2 * (hit_bonus * hit_bonus);
	}

	else if (paf->location == APPLY_DAMROLL )
  	{
	    dam_bonus = paf->modifier;
	    dam_found = TRUE;
	    fail += 2 * (dam_bonus * dam_bonus);
	}

	else /* things get a little harder */
	    fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,95);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p shivers violently and explodes!",ch,obj,NULL,TO_CHAR);
	act("$p shivers violently and explodeds!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen.\n\r",ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
	{
	    af_new = new_affect();
	
	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where	= paf->where;
	    af_new->type 	= UMAX(0,paf->type);
	    af_new->level	= paf->level;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (100 - level/5))  /* success! */
    {
	act("$p glows blue.",ch,obj,NULL,TO_CHAR);
	act("$p glows blue.",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = 1;
    }
    
    else  /* exceptional enchant */
    {
	act("$p glows a brillant blue!",ch,obj,NULL,TO_CHAR);
	act("$p glows a brillant blue!",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = 2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO - 1)
	obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

    if (dam_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_DAMROLL)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
		if (paf->modifier > 4)
		    SET_BIT(obj->extra_flags,ITEM_HUM);
	    }
	}
    }
    else /* add a new affect */
    {
	paf = new_affect();

	paf->where	= TO_OBJECT;
	paf->type	= sn;
	paf->level	= level;
	paf->duration	= -1;
	paf->location	= APPLY_DAMROLL;
	paf->modifier	=  added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
        affect_join_obj2 ( obj, paf );
    }

    if (hit_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
            if ( paf->location == APPLY_HITROLL)
            {
		paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX(paf->level,level);
                if (paf->modifier > 4)
                    SET_BIT(obj->extra_flags,ITEM_HUM);
            }
	}
    }
    else /* add a new affect */
    {
        paf = new_affect();
 
        paf->type       = sn;
        paf->level      = level;
        paf->duration   = -1;
        paf->location   = APPLY_HITROLL;
        paf->modifier   =  added;
        paf->bitvector  = 0;
        paf->next       = obj->affected;
        obj->affected   = paf;
        affect_join_obj2( obj, paf );
    }

}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;


    if ( saves_spell( level, victim,DAM_NEGATIVE) )
    {
	send_to_char("You feel a momentary chill.\n\r",victim);  	
	return;
    }


    if ( victim->level <= 2 )
    {
	dam		 = ch->hit + 1;
    }
    else
    {
	gain_exp( victim, 0 - number_range( level/2, 3 * level / 2 ) );
	victim->mana	/= 2;
	victim->move	/= 2;
	dam		 = dice(1, level);
	ch->hit		+= dam;
    }

    send_to_char("You feel your life slipping away!\n\r",victim);
    send_to_char("Wow....what a rush!\n\r",ch);
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);

    return;
}



void spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,  30,	 35,  40,  45,  50,  55,
	 60,  65,  70,  75,  80,	 82,  84,  86,  88,  90,
	 92,  94,  96,  98, 100,	102, 104, 106, 108, 110,
	112, 114, 116, 118, 120,	122, 124, 126, 128, 130
    };
    int dam;

    level	= UMIN(level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim, DAM_FIRE) )
	dam /= 2;
    rnd_sound( 2, ch, "fireball.wav", "fireball2.wav" );
    rnd_sound( 2, victim, "fireball.wav", "fireball2.wav" );
    damage( ch, victim, dam, sn, DAM_FIRE ,TRUE);
    return;
}


void spell_fireproof(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
 
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
    {
        act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR);
        return;
    }
 
    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy(level / 4);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_BURN_PROOF;
 
    affect_to_obj(obj,&af);
 
    act("You protect $p from fire.",ch,obj,NULL,TO_CHAR);
    act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM);
}



void spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(6 + level / 2, 8);
    if ( saves_spell( level, victim,DAM_FIRE) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_FIRE ,TRUE);
    sound( "flame.wav", ch );
    if ( ch != victim )
    sound( "flame.wav", victim );
    return;
}



void spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
	return;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by a pink outline.\n\r", victim );
    act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return;
}



void spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *ich;

    act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
	if (ich->invis_level > 0)
	    continue;

	if ( ich == ch || saves_spell( level, ich,DAM_OTHER) )
	    continue;

	affect_strip ( ich, skill_lookup("invis")			);
	affect_strip ( ich, skill_lookup("mass invis")		);
	affect_strip ( ich, skill_lookup("sneak")			);
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
	REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE	);
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
	act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
	send_to_char( "You are revealed!\n\r", ich );
    }

    return;
}

void spell_floating_disc( int sn, int level,CHAR_DATA *ch,void *vo,int target )
{
    OBJ_DATA *disc, *floating;

    floating = get_eq_char(ch,WEAR_FLOAT);
    if (floating != NULL && IS_OBJ_STAT(floating,ITEM_NOREMOVE))
    {
	act("You can't remove $p.",ch,floating,NULL,TO_CHAR);
	return;
    }

    disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);
    disc->value[0]	= ch->level * 10; /* 10 pounds per level capacity */
    disc->value[3]	= ch->level * 5; /* 5 pounds per level max per item */
    disc->timer		= ch->level * 2 - number_range(0,level / 2); 

    act("$n has created a floating black disc.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You create a floating disc.\n\r",ch);
    obj_to_char(disc,ch);
    wear_obj(ch,disc,TRUE);
    return;
}


void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
	if (victim == ch)
	  send_to_char("You are already airborne.\n\r",ch);
	else
	  act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    sound( "fly.wav", victim );
    return;
}

/* RT clerical berserking spell */

void spell_frenzy(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK))
    {
	if (victim == ch)
	  send_to_char("You are already in a frenzy.\n\r",ch);
	else
	  act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (is_affected(victim,skill_lookup("calm")))
    {
	if (victim == ch)
	  send_to_char("Why don't you just relax for a while?\n\r",ch);
	else
	  act("$N doesn't look like $e wants to fight anymore.",
	      ch,NULL,victim,TO_CHAR);
	return;
    }

    if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
	(IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
	(IS_EVIL(ch) && !IS_EVIL(victim))
       )
    {
	act("Your god doesn't seem to like $N",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level;
    af.duration	 = level / 3;
    af.modifier  = level / 6;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10 * (level / 12);
    af.location  = APPLY_AC;
    affect_to_char(victim,&af);

    send_to_char("You are filled with holy wrath!\n\r",victim);
    act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

/* RT ROM-style gate */
    
void spell_gate( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    bool gate_pet;

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
    ||   (IS_NPC(victim) && is_gqmob(ch, victim->pIndexData->vnum) != -1)   
    ||   victim->level >= level + 3
    ||   (is_clan(victim) && !is_same_clan(ch,victim))
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */ 
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER) ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }	
    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;
    
    act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You step through a gate and vanish.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);

    act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
    do_function(ch, &do_look, "auto");

    if (gate_pet)
    {
	act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
	send_to_char("You step through a gate and vanish.\n\r",ch->pet);
	char_from_room(ch->pet);
	char_to_room(ch->pet,victim->in_room);
	act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
	do_function(ch->pet, &do_look, "auto");
    }


    if( MOUNTED(ch) )
    {
        act("$n steps through a gate and vanishes.",MOUNTED(ch),NULL,NULL,TO_ROOM);
        send_to_char("You step through a gate and vanish.\n\r",MOUNTED(ch));
        char_from_room(MOUNTED(ch));
        char_to_room(MOUNTED(ch),victim->in_room);
        act("$n has arrived through a gate.",MOUNTED(ch),NULL,NULL,TO_ROOM);
        do_look(MOUNTED(ch),"auto");
    }
    
}



void spell_giant_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already as strong as you can get!\n\r",ch);
	else
	  act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your muscles surge with heightened power!\n\r", victim );
    act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_harm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = UMAX(  20, victim->hit - dice(1,4) );
    if ( saves_spell( level, victim,DAM_HARM) )
	dam = UMIN( 50, dam / 2 );
    dam = UMIN( 100, dam );
    damage( ch, victim, dam, sn, DAM_HARM ,TRUE);
    return;
}

/* RT haste spell */

void spell_haste( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE)
    ||   IS_SET(victim->off_flags,OFF_FAST))
    {
	if (victim == ch)
	  send_to_char("You can't move any faster!\n\r",ch);
 	else
	  act("$N is already moving as fast as $E can.",
	      ch,NULL,victim,TO_CHAR);
        return;
    }

    if (IS_AFFECTED(victim,AFF_SLOW))
    {
	if (!check_dispel(level,victim,skill_lookup("slow")))
	{
	    if (victim != ch)
	        send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel momentarily faster.\n\r",victim);
	    return;
	}
        act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/2;
    else
      af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself moving more quickly.\n\r", victim );
    act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->hit = UMIN( victim->hit + 100, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling fills your body.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_heat_metal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool fail = TRUE;
 
   if (!saves_spell(level + 2,victim,DAM_FIRE) 
   &&  !IS_SET(victim->imm_flags,IMM_FIRE))
   {
        for ( obj_lose = victim->carrying;
	      obj_lose != NULL; 
	      obj_lose = obj_next)
        {
	    obj_next = obj_lose->next_content;
            if ( number_range(1,2 * level) > obj_lose->level 
	    &&   !saves_spell(level,victim,DAM_FIRE)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_NONMETAL)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF))
            {
                switch ( obj_lose->item_type )
                {
               	case ITEM_ARMOR:
		if (obj_lose->wear_loc != -1) /* remove the item */
		{
		    if (can_drop_obj(victim,obj_lose)
		    &&  (obj_lose->weight / 10) < 
			number_range(1,2 * get_curr_stat(victim,STAT_DEX))
		    &&  remove_obj( victim, obj_lose->wear_loc, TRUE ))
		    {
		        act("$n yelps and throws $p to the ground!",
			    victim,obj_lose,NULL,TO_ROOM);
		        act("You remove and drop $p before it burns you.",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level) / 3);
                        obj_from_char(obj_lose);
			obj_lose->carried_by = victim;;
                        obj_to_room(obj_lose, victim->in_room);
                        fail = FALSE;
                    }
		    else /* stuck on the body! ouch! */
		    {
			act("Your skin is seared by $p!",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level));
			fail = FALSE;
		    }

		}
		else /* drop it if we can */
		{
		    if (can_drop_obj(victim,obj_lose))
		    {
                        act("$n yelps and throws $p to the ground!",
                            victim,obj_lose,NULL,TO_ROOM);
                        act("You and drop $p before it burns you.",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 6);
                        obj_from_char(obj_lose);
			obj_lose->carried_by = victim;
                        obj_to_room(obj_lose, victim->in_room);
			fail = FALSE;
                    }
		    else /* cannot drop */
		    {
                        act("Your skin is seared by $p!",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 2);
			fail = FALSE;
                    }
		}
                break;
                case ITEM_WEAPON:
		if (obj_lose->wear_loc != -1) /* try to drop it */
		{
		    if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
			continue;

		    if (can_drop_obj(victim,obj_lose) 
		    &&  remove_obj(victim,obj_lose->wear_loc,TRUE))
		    {
			act("$n is burned by $p, and throws it to the ground.",
			    victim,obj_lose,NULL,TO_ROOM);
			send_to_char(
			    "You throw your red-hot weapon to the ground!\n\r",
			    victim);
			dam += 1;
			obj_from_char(obj_lose);
			obj_lose->carried_by = victim;
			obj_to_room(obj_lose,victim->in_room);
			fail = FALSE;
		    }
		    else /* YOWCH! */
		    {
			send_to_char("Your weapon sears your flesh!\n\r",
			    victim);
			dam += number_range(1,obj_lose->level);
			fail = FALSE;
		    }
		}
                else /* drop it if we can */
                {
                    if (can_drop_obj(victim,obj_lose))
                    {
                        act("$n throws a burning hot $p to the ground!",
                            victim,obj_lose,NULL,TO_ROOM);
                        act("You and drop $p before it burns you.",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 6);
                        obj_from_char(obj_lose);
			obj_lose->carried_by = victim;
                        obj_to_room(obj_lose, victim->in_room);
                        fail = FALSE;
                    }
                    else /* cannot drop */
                    {
                        act("Your skin is seared by $p!",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 2);
                        fail = FALSE;
                    }
                }
                break;
		}
	    }
	}
    } 
    if (fail)
    {
        send_to_char("Your spell had no effect.\n\r", ch);
	send_to_char("You feel momentarily warmer.\n\r",victim);
    }
    else /* damage! */
    {
	if (saves_spell(level,victim,DAM_FIRE))
	    dam = 2 * dam / 3;
	damage(ch,victim,dam,sn,DAM_FIRE,TRUE);
    }
}

/* RT really nasty high-level attack spell */
void spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int bless_num, curse_num, frenzy_num;
   
    bless_num = skill_lookup("bless");
    curse_num = skill_lookup("curse"); 
    frenzy_num = skill_lookup("frenzy");

    act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM);
    send_to_char("You utter a word of divine power.\n\r",ch);
 
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;

	if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
	    (IS_EVIL(ch) && IS_EVIL(vch)) ||
	    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) )
	{
 	  send_to_char("You feel full more powerful.\n\r",vch);
	  spell_frenzy(frenzy_num,level,ch,(void *) vch,TARGET_CHAR); 
	  spell_bless(bless_num,level,ch,(void *) vch,TARGET_CHAR);
	}

	else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
		 (IS_EVIL(ch) && IS_GOOD(vch)) )
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
            spell_curse(curse_num,level,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);
	    dam = dice(level,6);
	    damage(ch,vch,dam,sn,DAM_ENERGY,TRUE);
	  }
	}

        else if (IS_NEUTRAL(ch))
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
            spell_curse(curse_num,level/2,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);
	    dam = dice(level,4);
	    damage(ch,vch,dam,sn,DAM_ENERGY,TRUE);
   	  }
	}
    }  
    
    send_to_char("You feel drained.\n\r",ch);
    ch->move = 0;
    ch->hit /= 2;
}
 
void spell_identify( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

    sprintf( buf,
	"Object '%s' is type %s, extra flags %s, extra2 flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",

	obj->name,
	item_name(obj->item_type),
	extra_bit_name( obj->extra_flags ),
        extra2_bit_name( obj->extra2_flags ),
	obj->weight / 10,
	obj->cost,
	obj->level
	);
    send_to_char( buf, ch );

    switch ( obj->item_type )
    {
    case ITEM_SCROLL: 
    case ITEM_POTION:
    case ITEM_PILL:
	sprintf( buf, "Level %d spells of:", obj->value[0] );
	send_to_char( buf, ch );

	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[1]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[2]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	{
	    send_to_char(" '",ch);
	    send_to_char(skill_table[obj->value[4]].name,ch);
	    send_to_char("'",ch);
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_WAND: 
    case ITEM_STAFF: 
	sprintf( buf, "Has %d charges of level %d",
	    obj->value[2], obj->value[0] );
	send_to_char( buf, ch );
      
	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_DRINK_CON:
        sprintf(buf,"It holds %s-colored %s.\n\r",
            liq_table[obj->value[2]].liq_color,
            liq_table[obj->value[2]].liq_name);
        send_to_char(buf,ch);
        break;

    case ITEM_CONTAINER:
	sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
	    obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
	send_to_char(buf,ch);
	if (obj->value[4] != 100)
	{
	    sprintf(buf,"Weight multiplier: %d%%\n\r",
		obj->value[4]);
	    send_to_char(buf,ch);
	}
	break;

   case ITEM_SOCKETS: 
     send_to_char("Gem type is: ",ch); 
     switch (obj->value[0]) 
     { 
        case 0 : send_to_char("none.  Tell an immortal.\n\r",ch);    
        break; 
        case 1 : send_to_char("sapphire.\n\r",ch);   break; 
        case 2 : send_to_char("ruby.\n\r",ch);   break; 
        case 3 : send_to_char("emerald.\n\r",ch);    break; 
        case 4 : send_to_char("diamond.\n\r",ch);    break; 
        case 5 : send_to_char("topaz.\n\r",ch);  break; 
        case 6 : send_to_char("skull.\n\r",ch);  break; 
        default : send_to_char("Unknown.  Tell an immortal.\n\r",ch);    
        break; 
     } 
     send_to_char("Gem value is: ",ch); 
     switch (obj->value[1]) 
         { 
         case 0 : send_to_char("chip.\n\r",ch);    break; 
         case 1 : send_to_char("flawed.\n\r",ch);   break; 
         case 2 : send_to_char("flawless.\n\r",ch);   break; 
         case 3 : send_to_char("perfect.\n\r",ch);    break; 
         default : send_to_char("Unknown.  Tell an immortal.\n\r",ch);   
         break; 
         } 
     break; 
		
    case ITEM_WEAPON:
 	send_to_char("Weapon type is ",ch);
	switch (obj->value[0])
	{
	    case(WEAPON_EXOTIC) : send_to_char("exotic.\n\r",ch);	break;
	    case(WEAPON_SWORD)  : send_to_char("sword.\n\r",ch);	break;	
	    case(WEAPON_DAGGER) : send_to_char("dagger.\n\r",ch);	break;
	    case(WEAPON_SPEAR)	: send_to_char("spear/staff.\n\r",ch);	break;
	    case(WEAPON_MACE) 	: send_to_char("mace/club.\n\r",ch);	break;
	    case(WEAPON_AXE)	: send_to_char("axe.\n\r",ch);		break;
	    case(WEAPON_FLAIL)	: send_to_char("flail.\n\r",ch);	break;
	    case(WEAPON_WHIP)	: send_to_char("whip.\n\r",ch);		break;
	    case(WEAPON_POLEARM): send_to_char("polearm.\n\r",ch);	break;
	    default		: send_to_char("unknown.\n\r",ch);	break;
 	}
	if (obj->pIndexData->new_format)
        {
	    sprintf(buf,"Damage is %dd%d (average %d).\n\r",
		obj->value[1],obj->value[2],
		(1 + obj->value[2]) * obj->value[1] / 2);
            send_to_char( buf, ch );

            sprintf(buf, "Min damage: %d Max damage: %d.\n\r",
            obj->value[1], obj->value[1] * obj->value[2] );
        }
	else
	    sprintf( buf, "Damage is %d to %d (average %d).\n\r",
	    	obj->value[1], obj->value[2],
	    	( obj->value[1] + obj->value[2] ) / 2 );
	send_to_char( buf, ch );
        if (obj->value[4])  /* weapon flags */
        {
            sprintf(buf,"Weapons flags: %s\n\r",weapon_bit_name(obj->value[4]));
            send_to_char(buf,ch);
        }
	break;

    case ITEM_ARMOR:
	sprintf( buf, 
	"Armor klass is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r", 
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	send_to_char( buf, ch );
	break;
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char(buf,ch);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        sprintf(buf,"Adds %s affect.\n",
                            affect_bit_name(paf->bitvector));
                        break;
                    case TO_OBJECT:
                        sprintf(buf,"Adds %s object flag.\n",
                            extra_bit_name(paf->bitvector));
                        break;
                    case TO_IMMUNE:
                        sprintf(buf,"Adds immunity to %s.\n",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_RESIST:
                        sprintf(buf,"Adds resistance to %s.\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_VULN:
                        sprintf(buf,"Adds vulnerability to %s.\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_OBJECT2:
                        sprintf(buf,"Adds %s object flag.\n",
                            extra2_bit_name(paf->bitvector));
                        break;  
                    default:
                        sprintf(buf,"Unknown bit %d: %d\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
	        send_to_char( buf, ch );
	    }
	}
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d",
	    	affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );
            if ( paf->duration > -1)
                sprintf(buf,", %d hours.\n\r",paf->duration);
            else
                sprintf(buf,".\n\r");
	    send_to_char(buf,ch);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        sprintf(buf,"Adds %s affect.\n",
                            affect_bit_name(paf->bitvector));
                        break;
                    case TO_OBJECT:
                        sprintf(buf,"Adds %s object flag.\n",
                            extra_bit_name(paf->bitvector));
                        break;
		    case TO_WEAPON:
			sprintf(buf,"Adds %s weapon flags.\n",
			    weapon_bit_name(paf->bitvector));
			break;
                    case TO_IMMUNE:
                        sprintf(buf,"Adds immunity to %s.\n",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_RESIST:
                        sprintf(buf,"Adds resistance to %s.\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_VULN:
                        sprintf(buf,"Adds vulnerability to %s.\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_OBJECT2:
                        sprintf(buf,"Adds %s object flag.\n",
                            extra2_bit_name(paf->bitvector));
                        break;       
                    default:
                        sprintf(buf,"Unknown bit %d: %d\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
                send_to_char(buf,ch);
            }
	}
    }

    return;
}



void spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
    {
	if (victim == ch)
	  send_to_char("You can already see in the dark.\n\r",ch);
	else
	  act("$N already has infravision.\n\r",ch,NULL,victim,TO_CHAR);
	return;
    }
    act( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM );

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes glow red.\n\r", victim );
    return;
}



void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* object invisibility */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;	

	if (IS_OBJ_STAT(obj,ITEM_INVIS))
	{
	    act("$p is already invisible.",ch,obj,NULL,TO_CHAR);
	    return;
	}
	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= level + 12;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ITEM_INVIS;
	affect_to_obj(obj,&af);

	act("$p fades out of sight.",ch,obj,NULL,TO_ALL);
	return;
    }

    /* character invisibility */
    victim = (CHAR_DATA *) vo;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
	return;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level + 12;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\n\r", victim );
    return;
}



void spell_know_alignment(int sn,int level,CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    ap = victim->alignment;

         if ( ap >  700 ) msg = "$N has a pure and good aura.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N is a black-hearted murderer.";
    else msg = "$N is the embodiment of pure evil!.";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}



void spell_lightning_bolt(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0, 25, 28,
	31, 34, 37, 40, 40,	41, 42, 42, 43, 44,
	44, 45, 46, 46, 47,	48, 48, 49, 50, 50,
	51, 52, 52, 53, 54,	54, 55, 56, 56, 57,
	58, 58, 59, 60, 60,	61, 62, 62, 63, 64
    };
    int dam;

    level	= UMIN(level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_LIGHTNING) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE);
    sound( "lightning.wav", ch );
    if ( ch != victim )
    sound( "lightning.wav", victim );
    return;
}



void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

    buffer = new_buf();
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name ) 
    	||   IS_OBJ_STAT(obj,ITEM_NOLOCATE | ITEM_AUCTIONED) || number_percent() > 2 * level
	||   ch->level < obj->level)
	    continue;

	found = TRUE;
        number++;

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
	{
	    sprintf( buf, "one is carried by %s\n\r",
		PERS(in_obj->carried_by, ch) );
	}
	else
	{
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf( buf, "one is in %s [Room %d]\n\r",
		    in_obj->in_room->name, in_obj->in_room->vnum);
	    else 
	    	sprintf( buf, "one is in %s\n\r",
		    in_obj->in_room == NULL
		    	? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = UPPER(buf[0]);
	add_buf(buffer,buf);

    	if (number >= max_found)
	    break;
    }

    if ( !found )
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}

void spell_energy_spike(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{	CHAR_DATA *victim = (CHAR_DATA *) vo;
 	AFFECT_DATA af;
	int dam;
    int hpch;
 
    hpch = UMAX( 100, ch->hit );
    dam  = number_range( hpch+10, hpch+30 );
	dam = dam /3;
	if (ch->level <= 20)
		damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
	else if (ch->level <= 30)
	{	damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
		damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
	}
	else if (ch->level <= 40)
	{	damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
		damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
		damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
	}
	else if (ch->level <= 50)
	{	damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
		damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
		damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
		damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
	}
	else if (ch->level >= 60)
	{	damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
		damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
		damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
		damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
		af.type      = sn;
		af.duration  = level;
		af.location  = APPLY_STR;
		af.modifier  = 0 - number_range(1,5);
		af.bitvector = AFF_POISON;
		affect_join( victim, &af );
		send_to_char( "The overload of magic gets you sick.\n\r", victim );
	}
	else
	    damage(ch, victim, dam, sn, DAM_ENERGY, TRUE );
 
	return;
}

void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;
    
    heal_num = skill_lookup("heal");
    refresh_num = skill_lookup("refresh"); 

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ((IS_NPC(ch) && IS_NPC(gch)) ||
	    (!IS_NPC(ch) && !IS_NPC(gch)))
	{
	    spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
	    spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);  
	}
    }
}
	    

void spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) )
	    continue;
	act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade out of existence.\n\r", gch );

	af.where     = TO_AFFECTS;
	af.type      = sn;
    	af.level     = level/2;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char( gch, &af );
    }
    send_to_char( "Ok.\n\r", ch );

    return;
}



void spell_null( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return;
}



void spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	if (victim == ch)
	  send_to_char("You are already out of phase.\n\r",ch);
	else
	  act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You turn translucent.\n\r", victim );
    return;
}

/* RT plague spell, very nasty */

void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell(level,victim,DAM_DISEASE) || 
        (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
	if (ch == victim)
	  send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
	else
	  act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	  = sn;
    af.level	  = level * 3/4;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = -5; 
    af.bitvector = AFF_PLAGUE;
    affect_join(victim,&af);
   
    send_to_char
      ("You scream in agony as plague sores erupt from your skin.\n\r",victim);
    act("$n screams in agony as plague sores erupt from $s skin.",
	victim,NULL,NULL,TO_ROOM);
}

void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;


    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
	    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	    {
		act("Your spell fails to corrupt $p.",ch,obj,NULL,TO_CHAR);
		return;
	    }
	    obj->value[3] = 1;
	    act("$p is infused with poisonous vapors.",ch,obj,NULL,TO_ALL);
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

	    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	    {
		act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
		return;
	    }

	    af.where	 = TO_WEAPON;
	    af.type	 = sn;
	    af.level	 = level / 2;
	    af.duration	 = level/8;
 	    af.location	 = 0;
	    af.modifier	 = 0;
	    af.bitvector = WEAPON_POISON;
	    affect_to_obj(obj,&af);

	    act("$p is coated with deadly venom.",ch,obj,NULL,TO_ALL);
	    return;
	}

	act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
	return;
    }

    victim = (CHAR_DATA *) vo;

    if ( saves_spell( level, victim,DAM_POISON) )
    {
	act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = -2;
    af.bitvector = AFF_POISON;
    affect_join( victim, &af );
    send_to_char( "You feel very sick.\n\r", victim );
    act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_protection_evil(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) 
    ||   IS_AFFECTED(victim, AFF_PROTECT_GOOD))
    {
        if (victim == ch)
          send_to_char("You are already protected.\n\r",ch);
        else
          act("$N is already protected.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "You feel holy and pure.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from evil.",ch,NULL,victim,TO_CHAR);
    return;
}
 
void spell_protection_good(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) 
    ||   IS_AFFECTED(victim, AFF_PROTECT_EVIL))
    {
        if (victim == ch)
          send_to_char("You are already protected.\n\r",ch);
        else
          act("$N is already protected.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "You feel aligned with darkness.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from good.",ch,NULL,victim,TO_CHAR);
    return;
}


void spell_ray_of_truth (int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, align;
 
    if (IS_EVIL(ch) )
    {
        victim = ch;
        send_to_char("The energy explodes inside you!\n\r",ch);
    }
 
    if (victim != ch)
    {
        act("$n raises $s hand, and a blinding ray of light shoots forth!",
            ch,NULL,NULL,TO_ROOM);
        send_to_char(
	   "You raise your hand and a blinding ray of light shoots forth!\n\r",
	   ch);
    }

    if (IS_GOOD(victim))
    {
	act("$n seems unharmed by the light.",victim,NULL,victim,TO_ROOM);
	send_to_char("The light seems powerless to affect you.\n\r",victim);
	return;
    }

    dam = dice( level, 10 );
    if ( saves_spell( level, victim,DAM_HOLY) )
        dam /= 2;

    align = victim->alignment;
    align -= 350;

    if (align < -1000)
	align = -1000 + (align + 1000) / 3;

    dam = (dam * align * align) / 1000000;

    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    spell_blindness(skill_lookup("blindness"), 
	3 * level / 4, ch, (void *) victim,TARGET_CHAR);
}


void spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance, percent;

    if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
    {
	send_to_char("That item does not carry charges.\n\r",ch);
	return;
    }

    if (obj->value[3] >= 3 * level / 2)
    {
	send_to_char("Your skills are not great enough for that.\n\r",ch);
	return;
    }

    if (obj->value[1] == 0)
    {
	send_to_char("That item has already been recharged once.\n\r",ch);
	return;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[3]; /* harder to do high-level spells */
    chance -= (obj->value[1] - obj->value[2]) *
	      (obj->value[1] - obj->value[2]);

    chance = UMAX(level/2,chance);

    percent = number_percent();

    if (percent < chance / 2)
    {
	act("$p glows softly.",ch,obj,NULL,TO_CHAR);
	act("$p glows softly.",ch,obj,NULL,TO_ROOM);
	obj->value[2] = UMAX(obj->value[1],obj->value[2]);
	obj->value[1] = 0;
	return;
    }

    else if (percent <= chance)
    {
	int chargeback,chargemax;

	act("$p glows softly.",ch,obj,NULL,TO_CHAR);
	act("$p glows softly.",ch,obj,NULL,TO_CHAR);

	chargemax = obj->value[1] - obj->value[2];
	
	if (chargemax > 0)
	    chargeback = UMAX(1,chargemax * percent / 100);
	else
	    chargeback = 0;

	obj->value[2] += chargeback;
	obj->value[1] = 0;
	return;
    }	

    else if (percent <= UMIN(95, 3 * chance / 2))
    {
	send_to_char("Nothing seems to happen.\n\r",ch);
	if (obj->value[1] > 1)
	    obj->value[1]--;
	return;
    }

    else /* whoops! */
    {
	act("$p glows brightly and explodes!",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly and explodes!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
    }
}

void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN( victim->move + level, victim->max_move );
    if (victim->max_move == victim->move)
        send_to_char("You feel fully refreshed!\n\r",victim);
    else
        send_to_char( "You feel less tired.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool found = FALSE;

    /* do object cases first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	{
	    if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
	    &&  !saves_dispel(level + 2,obj->level,0))
	    {
		REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
		act("$p glows blue.",ch,obj,NULL,TO_ALL);
		return;
	    }

	    act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR);
	    return;
	}
	act("There doesn't seem to be a curse on $p.",ch,obj,NULL,TO_CHAR);
	return;
    }

    /* characters */
    victim = (CHAR_DATA *) vo;

    if (check_dispel(level,victim,skill_lookup("curse")))
    {
	send_to_char("You feel better.\n\r",victim);
	act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
    }

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
        if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	&&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
        {   /* attempt to remove curse */
            if (!saves_dispel(level,obj->level,0))
            {
                found = TRUE;
                REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                act("Your $p glows blue.",victim,obj,NULL,TO_CHAR);
                act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM);
            }
         }
    }
}

void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
	if (victim == ch)
	  send_to_char("You are already in sanctuary.\n\r",ch);
	else
	  act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a white aura.\n\r", victim );
    sound( "sanc.wav", victim );
    return;
}



void spell_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already shielded from harm.\n\r",ch);
	else
	  act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 8 + level;
    af.location  = APPLY_AC;
    af.modifier  = -20;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a force shield.\n\r", victim );
    return;
}



void spell_shocking_grasp(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0, 20, 25, 29, 33,
	36, 39, 39, 39, 40,	40, 41, 41, 42, 42,
	43, 43, 44, 44, 45,	45, 46, 46, 47, 47,
	48, 48, 49, 49, 50,	50, 51, 51, 52, 52,
	53, 53, 54, 54, 55,	55, 56, 56, 57, 57
    };
    int dam;

    level	= UMIN(level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_LIGHTNING) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE);
    return;
}



void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
  
    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
    ||   (level + 2) < victim->level
    ||   saves_spell( level-4, victim,DAM_CHARM) )
    {
        send_to_char( "Sorry you can't do that!\n\r", ch );
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_range( 1, 2 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );
   
    if ( IS_AWAKE(victim) )
    {
        printf_to_char( ch, "You cast a spell on %s putting them to sleep.\n\r", victim->name ); 
	send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
	act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
	victim->position = POS_SLEEPING;
    }
    return;
}

void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
        if (victim == ch)
          send_to_char("You can't move any slower!\n\r",ch);
        else
          act("$N can't get any slower than that.",
              ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (saves_spell(level,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC))
    {
	if (victim != ch)
            send_to_char("Nothing seemed to happen.\n\r",ch);
        send_to_char("You feel momentarily lethargic.\n\r",victim);
        return;
    }
 
    if (IS_AFFECTED(victim,AFF_HASTE))
    {
        if (!check_dispel(level,victim,skill_lookup("haste")))
        {
	    if (victim != ch)
            	send_to_char("Spell failed.\n\r",ch);
            send_to_char("You feel momentarily slower.\n\r",victim);
            return;
        }

        act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
        return;
    }
 

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
    return;
}




void spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your skin is already as hard as a rock.\n\r",ch); 
	else
	  act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -40;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin turns to stone.\n\r", victim );
    return;
}



void spell_summon( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
    ||   victim->level >= level + 3
	||   IS_SET(victim->in_room->room_flags, ROOM_ARENA)
	||   IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
    ||   victim->fighting != NULL
    ||   (IS_NPC(victim) && is_gqmob(ch, victim->pIndexData->vnum) != -1)   
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||	 (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER)) )

    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    act( "$n has summoned you!", ch, NULL, victim,   TO_VICT );
    sound( "summon.wav", victim );
    sound( "summon.wav", ch );
    do_function(victim, &do_look, "auto" );
    return;
}



void spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;

    if ( victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
	||   IS_SET(victim->in_room->room_flags, ROOM_ARENA)
	||   IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    || ( victim != ch && IS_SET(victim->imm_flags,IMM_SUMMON))
    || ( !IS_NPC(ch) && victim->fighting != NULL )
    || ( victim != ch
    && ( saves_spell( level - 5, victim,DAM_OTHER))))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    pRoomIndex = get_random_room(victim);

    if (victim != ch)
	send_to_char("You have been teleported!\n\r",victim);

    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    do_function(victim, &do_look, "auto" );
    return;
}



void spell_ventriloquate( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s says '%s'.\n\r",              speaker, target_name );
    sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name );
    buf1[0] = UPPER(buf1[0]);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if (!is_exact_name( speaker, vch->name) && IS_AWAKE(vch))
	    send_to_char( saves_spell(level,vch,DAM_OTHER) ? buf2 : buf1, vch );
    }

    return;
}



void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell( level, victim,DAM_OTHER) )
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 2;
    af.location  = APPLY_STR;
    af.modifier  = -1 * (level / 5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char( "You feel your strength slip away.\n\r", victim );
    act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
    return;
}



/* RT recall spell is back */

void spell_word_of_recall( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *location;
    
    if (IS_NPC(victim))
      return;
   
    if ((location = get_room_index(clan_table[ch->clan].room[0] )) == NULL )
        return;

    if ((location = get_room_index( ROOM_VNUM_TEMPLE)) == NULL)
    {
	send_to_char("You are completely lost.\n\r",victim);
	return;
    } 

    if (IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL) ||
	IS_AFFECTED(victim,AFF_CURSE))
    {
	send_to_char("Spell failed.\n\r",victim);
	return;
    }

    if (victim->fighting != NULL)
	stop_fighting(victim,TRUE);
    
    ch->move /= 2;
    act("$n disappears.",victim,NULL,NULL,TO_ROOM);
    char_from_room(victim);
    char_to_room(victim,location);
    act("$n appears in the room.",victim,NULL,NULL,TO_ROOM);
    do_function(victim, &do_look, "auto");
}

/*
 * NPC spells.
 */
void spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch;

    act("$n spits acid at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n spits a stream of corrosive acid at you.",ch,NULL,victim,TO_VICT);
    act("You spit acid at $N.",ch,NULL,victim,TO_CHAR);

    hpch = UMAX(12,ch->hit);
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,16);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    
    if (saves_spell(level,victim,DAM_ACID))
    {
	acid_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage(ch,victim,dam/2,sn,DAM_ACID,TRUE);
    }
    else
    {
	acid_effect(victim,level,dam,TARGET_CHAR);
	damage(ch,victim,dam,sn,DAM_ACID,TRUE);
    }
}



void spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam;
    int hpch;

    act("$n breathes forth a cone of fire.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a cone of hot fire over you!",ch,NULL,victim,TO_VICT);
    act("You breath forth a cone of fire.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX( 10, ch->hit );
    hp_dam  = number_range( hpch/9+1, hpch/5 );
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
    fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE) 
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	if (vch == victim) /* full damage */
	{
	    if (saves_spell(level,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level,dam,TARGET_CHAR);
		damage(ch,vch,dam,sn,DAM_FIRE,TRUE);
	    }
	}
	else /* partial damage */
	{
	    if (saves_spell(level - 2,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage(ch,vch,dam/4,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	}
    }
}

void spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam, hpch;

    act("$n breathes out a freezing cone of frost!",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a freezing cone of frost over you!",
	ch,NULL,victim,TO_VICT);
    act("You breath out a cone of frost.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(12,ch->hit);
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,16);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    cold_effect(victim->in_room,level,dam/2,TARGET_ROOM); 

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	if (vch == victim) /* full damage */
	{
	    if (saves_spell(level,vch,DAM_COLD))
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level,dam,TARGET_CHAR);
		damage(ch,vch,dam,sn,DAM_COLD,TRUE);
	    }
	}
	else
	{
	    if (saves_spell(level - 2,vch,DAM_COLD))
	    {
		cold_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage(ch,vch,dam/4,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	}
    }
}

    
void spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam,hp_dam,dice_dam,hpch;

    act("$n breathes out a cloud of poisonous gas!",ch,NULL,NULL,TO_ROOM);
    act("You breath out a cloud of poisonous gas.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(16,ch->hit);
    hp_dam = number_range(hpch/15+1,8);
    dice_dam = dice(level,12);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    poison_effect(ch->in_room,level,dam,TARGET_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(ch) && IS_NPC(vch) 
	&&   (ch->fighting == vch || vch->fighting == ch)))
	    continue;

	if (saves_spell(level,vch,DAM_POISON))
	{
	    poison_effect(vch,level/2,dam/4,TARGET_CHAR);
	    damage(ch,vch,dam/2,sn,DAM_POISON,TRUE);
	}
	else
	{
	    poison_effect(vch,level,dam,TARGET_CHAR);
	    damage(ch,vch,dam,sn,DAM_POISON,TRUE);
	}
    }
}

void spell_lightning_breath(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch;

    act("$n breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
    act("You breathe a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

    hpch = UMAX(10,ch->hit);
    hp_dam = number_range(hpch/9+1,hpch/5);
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

    if (saves_spell(level,victim,DAM_LIGHTNING))
    {
	shock_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage(ch,victim,dam/2,sn,DAM_LIGHTNING,TRUE);
    }
    else
    {
	shock_effect(victim,level,dam,TARGET_CHAR);
	damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE); 
    }
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE);
    return;
}

void spell_high_explosive(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 30, 120 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE);
    return;
}

void spell_trivia_pill(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (victim == NULL)
	victim = ch;

    if (IS_NPC(victim))
	return;

    victim->pcdata->trivia++;
    send_to_char("{YYou've gained a {RTrivia Point{Y!{x\n\r", victim);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void spell_vicegrip(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (victim != ch)
  {
      send_to_char("You may only cast this on yourself.\n\r", ch);
      return;
  }

  if (IS_AFFECTED(victim, sn))
  { 
      if (victim == ch)
          send_to_char("You already have a vicegrip on your weapon.\n\r", ch);
      else
          act("$N already has a vicegrip on their weapon.",ch, NULL, victim, TO_CHAR);
      return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 4 * (ch->level + 7) / 4;
  af.location  = APPLY_AC; /* Change this to whatever. */
  af.modifier  = -4 * (ch->level +5) /10;
  af.bitvector = AFF_VICEGRIP;

  affect_to_char(victim, &af);
  act("$n's hands grip their weapon with an iron hold.",victim, NULL,NULL,TO_ROOM);
  send_to_char("You hold on to your weapon with all your might.\n\r",victim);
  return;
}


void spell_hurricane(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam,hp_dam,dice_dam,hpch;     

    act("$n calls upon the winds of a hurricane for help.",ch,NULL,NULL,TO_NOTVICT);
    act("You call down the winds of a hurricane to help you.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(16,ch->hit);
    hp_dam = number_range(hpch/15+1,8);
    dice_dam = dice(level,12);              

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell(ch,vch,TRUE)
        ||  (IS_NPC(ch) && IS_NPC(vch)
        &&   (ch->fighting == vch || vch->fighting == ch)))
            continue;             

        if ( is_safe(ch, vch) )
          continue;

        if (!IS_AFFECTED(vch,AFF_FLYING)) dam /= 2;

        if (vch->size == SIZE_TINY)  dam *= 1.5;
        else if (vch->size == SIZE_SMALL)  dam *= 1.3;
        else if (vch->size == SIZE_MEDIUM)  dam *= 1;
        else if (vch->size == SIZE_LARGE)  dam *= 0.9;
        else if (vch->size == SIZE_HUGE)  dam *= 0.7;
        else dam *= 0.5;

        if (saves_spell(level,vch,DAM_OTHER))
            damage(ch,vch,dam/2,sn,DAM_OTHER,TRUE);
        else
            damage(ch,vch,dam,sn,DAM_OTHER,TRUE);
    }
}


void spell_greater_heal( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    victim->hit = UMIN( victim->hit +250, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling flows through your body.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}


void spell_super_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    victim->hit = UMIN( victim->hit + 500, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling fills your body.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );     
}


void spell_ultimate_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    victim->hit = UMIN( victim->hit + 1000, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling fills your body.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}           


void spell_group_heal(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;

    heal_num = skill_lookup("ultimate heal");
    refresh_num = skill_lookup("refresh");

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ((IS_NPC(ch) && IS_NPC(gch)) ||
            (!IS_NPC(ch) && !IS_NPC(gch)))
        {
            spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);     
            spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);
        }
    }
}
                  
void spell_bark_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;   

    if ( is_affected( ch, sn ) )
    {
        if (victim == ch)
          send_to_char("Your skin is already turned to bark.\n\r",ch);
        else    
          act("$N skin is already turned to bark.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -30;
    af.bitvector = 0;        
    affect_to_char( victim, &af );
    act( "$n's skin turns to bark.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin turns to bark.\n\r", victim );
    sound( "barkskin.wav", victim );
    return;
}             


void spell_knock ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    char arg[MAX_INPUT_LENGTH];
    int chance=0;
    int door;
    const       sh_int  rev_dir         []              =
        {
            2, 3, 0, 1, 5, 4, 9, 8, 7, 6
        };

    target_name = one_argument(target_name,arg);

    if (arg[0] == '\0')
    {
    send_to_char("Knock which door or direction.\n\r",ch);
    return;
    }

    if (ch->fighting)
    {
        send_to_char("Wait until the fight finishes.\n\r",ch);
        return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev;

        pexit = ch->in_room->exit[door];
        if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
            { send_to_char( "It's already open.\n\r",      ch ); return; }
        if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
            { send_to_char( "Just try to open it.\n\r",     ch ); return; 
}
        if ( IS_SET(pexit->exit_info, EX_NOPASS) )
        {
              send_to_char( "A mystical shield protects the exit.\n\r",ch );
              return;   
        }

    chance = ch->level / 5 + get_curr_stat(ch,STAT_INT) + get_skill(ch,sn) / 5;

    act("You cast knock on the $d, and try to open the $d!",
             ch,NULL,pexit->keyword,TO_CHAR);

    if (room_is_dark(ch->in_room))
                chance /= 2;

    /* now the attack */
    if (number_percent() < chance )
     {
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
        act( "$n knocks on the $d and opens the lock.", ch, NULL,
                pexit->keyword, TO_ROOM );
        send_to_char( "You successfully open the door.\n\r", ch );

        /* open the other side */
        if ( ( to_room   = pexit->u1.to_room            ) != NULL
        &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
        &&   pexit_rev->u1.to_room == ch->in_room )
        {
            CHAR_DATA *rch;
               REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
            REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
            for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
                act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
        }
     }
    else
     {
        act("You couldn't magically open the $d!",
            ch,NULL,pexit->keyword,TO_CHAR);
        act("$n failed to magically open the $d.",
            ch,NULL,pexit->keyword,TO_ROOM);
     }
    return;
    }

  send_to_char("You can't see that here.\n\r",ch);
  return;
}

void spell_group_defense( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
        CHAR_DATA *gch;
        AFFECT_DATA af;
        int shield_sn, armor_sn, detect_invis_sn;

        shield_sn = skill_lookup("shield");
        armor_sn = skill_lookup("armor");
        detect_invis_sn = skill_lookup("detect invis");

        for( gch=ch->in_room->people; gch != NULL; gch=gch->next_in_room)
        {
                if( !is_same_group( gch, ch))
                        continue;
                if( is_affected( gch, armor_sn ) )
                {
                    if( gch == ch)
                        send_to_char("You are already armored.\n\r",ch);
                    else                                          
                        continue;
                }

                af.type      = armor_sn;
                af.level     = level;
                af.duration  = level;
                af.location  = APPLY_AC;
                af.modifier  = -20;
                af.bitvector = 0; 
                affect_to_char( gch, &af );

                send_to_char( "You feel someone protecting you.\n\r",gch);
                if( ch != gch )
                        act("$N is protected by your magic.",
                                ch, NULL, gch, TO_CHAR );

                if( !is_same_group( gch, ch ) )
                        continue;
                if( is_affected( gch, shield_sn ) )
                {
                  if( gch == ch)
                        send_to_char("You are already shielded.\n\r",ch);
                  else
                        act( "$N is already shielded.", ch, NULL, gch, TO_CHAR);
                  continue;
                }

                af.type      = shield_sn;            
                      af.level     = level;
                af.duration  = level;
                af.location  = APPLY_AC;
                af.modifier   = -20;
                af.bitvector = 0;
                affect_to_char( gch, &af );
                send_to_char( "You are surrounded by a force shield.\n\r",gch);
                if( ch != gch )
                   act("$N is surrounded by a force shield.", ch, NULL, gch, TO_CHAR );
               
          if( !is_same_group( gch, ch ) )
             continue;
              
          if( is_affected( gch, shield_sn ) )
          {     
           if (gch == ch)
             send_to_char("You can already see invisible.\n\r",ch);
           else
             act("$N can already see invisible things.",ch,NULL,gch,TO_CHAR);
             return;
          }

          af.where     = TO_AFFECTS;
          af.type      = sn;
          af.level     = level;  
          af.duration  = (level +4) / 4;
          af.modifier  = 0;
          af.location  = APPLY_NONE;
          af.bitvector = AFF_DETECT_INVIS;
          affect_to_char( gch, &af );
          send_to_char( "Your eyes tingle.\n\r", ch );      
          if ( ch != gch )
           act("$n can now see invisible.\n\r", ch, NULL, gch, TO_CHAR);

        }
        return;
}                        
               
void spell_meteor_swarm( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if ( !is_safe_spell(ch,vch,TRUE))
        {
            dam  = dice(20, 40);
            if ( saves_spell( level, vch, DAM_FIRE ) )
                dam /= 2;
            act( "$n is hit full force with a mass of fireballs.",vch,NULL,NULL,TO_ROOM);
            damage( ch, vch, dam, sn, DAM_FIRE, TRUE );
        }
    }        
    return;
}

void spell_wizards_fire( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam         = ( level /2 )
                   +dice( ( level/6 ), 50 )
                   +dice( ( level/6 ), 50 );
 
    act( "A blast of wizards fire flies from $n's hands!", ch, NULL, NULL, TO_ROOM );
    send_to_char( "A blast of wizards fire flies from your hands!\n\r", ch );
     
    if ( saves_spell( level, victim, DAM_FIRE ) )
                dam /= 2;

    act( "$n is struck by a blast of wizards fire!!", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are struck by a blast of wizards fire!\n\r", victim );

    damage( ch, victim, dam, sn, DAM_FIRE, TRUE );
    return;
}          

void spell_synon_aura(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;                                                       

    if (is_affected(victim,sn) )
    {
        if (victim == ch)
          send_to_char("You are already in a frenzy.\n\r",ch);
        else
          act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
        return;
    }  

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = MAX_LEVEL;
    af.duration  = -1;
    af.modifier  = MAX_LEVEL / 3;;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10;
    af.location  = APPLY_AC;
    affect_to_char(victim,&af);
                                  
    send_to_char("You are protected by Synon's aura!\n\r",victim);
    act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}    

void do_channel( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int transfer, mana;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Channel how much to whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if(( victim == ch))
     {
       send_to_char("Don't be cheap.\n\r",ch);
       return;
     }

    if (!is_number( arg1 ) && !strcmp(arg1, "all"))
    {
        send_to_char( "Argument must be numerical or all.\n\r", ch);
        return;
    }

   if (!strcmp(arg1, "all"))
    mana = ch->mana;
   else mana = atoi(arg1);

   if (mana == 0)
     {
        send_to_char("You have to channel something!\n\r",ch);
        return;
     }
   if (mana > ch->mana)
     {
        send_to_char("Hah? You don't have that much!\n\r",ch);
        return;
     }

   /* percent of mana to transfer */
   transfer =  (number_range(0, get_skill(ch,skill_lookup("channel" ))) * mana / 100);
   /* skill failure */
   if( transfer == 0 )
    {
       act("{gA beam of {MENERGY{g shoots from your forehead and {RFIZZLES{g!!{x", ch, 0, victim, TO_CHAR);
       act("{g$n places $s fingers on $s temples and a beam of {MENERGY{R FIZZLES{g!!{x", ch, 0, victim, TO_ROOM);
       check_improve(ch,skill_lookup("channel"),FALSE,5);
       return;
    }


   /* boni etc. (based on practice gain.) */

  transfer += wis_app[get_curr_stat(ch,STAT_WIS)].practice * 10;
  transfer += wis_app[get_curr_stat(victim,STAT_WIS)].practice * 10;
  transfer += number_range(0,get_skill(victim,skill_lookup("channel")))/2;
  /* can't let them get to much in battle (concentration loss */
  if (ch->position == POS_FIGHTING)
    transfer *= 2/3;


  /* They can't gain more than they sent! */
  if( (mana < transfer) )
    transfer = mana;
  /* finally, the transfer! */
  victim->mana += transfer;
  ch->mana -= mana;
  check_improve(ch,skill_lookup("channel"),TRUE,5);
  act("{g$n places $s fingers on $s temples and a beam of {MENERGY{g shoots to $N!!{x", ch, 0, victim, TO_NOTVICT);
  act("{gA beam of {MENERGY{g shoots from your forehead to $N!!{x", ch, 0, victim, TO_CHAR);
  act("{gA beam of {MENERGY{g shoots from $n's forehead to yours!!{x", ch, 0, victim, TO_VICT);
  send_to_char("{YYou feel energized!!!!{x\n\r",victim);
  send_to_char("{bYou feel your energy drop.{x\n\r",ch);
  return;
}

void spell_summon_ggolem( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{
  CHAR_DATA *gch;
  CHAR_DATA *golem;
  AFFECT_DATA af;
  int i = 0;

  if (is_affected(ch,sn))
    {
      send_to_char("You lack the power to summon another golem right now.\n\r",
		   ch);
      return;
    }

  send_to_char("You attempt to summon a greater golem.\n\r",ch);
  act("$n attempts to summon a greater golem.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
          ( gch->pIndexData->vnum == MOB_VNUM_GGOLEM ) )
	{
	    send_to_char("More golems are more than you can control!\n\r",ch);
	    return;
	}
    }

  golem = create_mobile( get_mob_index(MOB_VNUM_GGOLEM) );


  for (i = 0; i < MAX_STATS; i ++)
       golem->perm_stat[i] = UMIN(25,15 + ch->level/10);
            
  golem->perm_stat[STAT_STR] += 3;
  golem->perm_stat[STAT_INT] -= 1;
  golem->perm_stat[STAT_CON] += 2;

  golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: UMIN( (10 * ch->pcdata->perm_hit) + 4000, 30000);
  golem->hit = golem->max_hit;
  golem->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  golem->mana = golem->max_mana;
  golem->level = ch->level;
  for (i=0; i < 3; i++)
  golem->armor[i] = interpolate(golem->level,100,-100);
  golem->armor[3] = interpolate(golem->level,100,0);
  golem->gold = 0;
  golem->timer = 0;
  golem->damage[DICE_NUMBER] = 13;   
  golem->damage[DICE_TYPE] = 9;
  golem->damage[DICE_BONUS] = ch->level / 2 + 10;
  golem->hitroll = (ch->level / 2) + 20;
  golem->damroll = (ch->level / 2) + 20;
  
  char_to_room(golem,ch->in_room);
  send_to_char("You summoned a greater golem!\n\r",ch);
  act("$n summons a greater golem!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level; 
  af.duration           = 30;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);  

  SET_BIT(golem->affected_by, AFF_CHARM);
  golem->master = golem->leader = ch;

}

void spell_lesser_golem(int sn,int level, CHAR_DATA *ch,void *vo,int target)
{
        CHAR_DATA *golem;
        AFFECT_DATA af;
        CHAR_DATA *check;

        if (is_affected(ch,sn))
        {
        send_to_char("You aren't up to fashioning another golem yet.\n\r",ch);
        return;
        }

        for (check = char_list; check != NULL; check = check->next)
        {
	if (IS_NPC(check))     
   		if ( (check->master == ch) && (check->pIndexData->vnum == MOB_VNUM_L_GOLEM) )
        	{
		send_to_char("You already have a golem under your command.\n\r",ch);
        	return;
        	}
	}
        af.where = TO_AFFECTS;
        af.level = level;
        af.location = 0;
        af.modifier = 0;
        af.duration = 50;
        af.bitvector = 0;
        af.type = sn;
        affect_to_char(ch,&af);
        act("$n fashions a flesh golem!",ch,0,0,TO_ROOM);
        send_to_char("You fashion a flesh golem to serve you!\n\r",ch);

        golem = create_mobile(get_mob_index(MOB_VNUM_L_GOLEM) );
        golem->level = ch->level;
        golem->damroll += level/2;
        golem->hitroll += level/2;
        golem->max_hit = ch->max_hit;
        golem->hit = golem->max_hit;
        golem->max_move = ch->max_move;
        golem->move = golem->max_move;
        golem->damage[DICE_NUMBER] = 8;
        golem->damage[DICE_BONUS] = ch->level / 2 + 10; 
        char_to_room(golem,ch->in_room);
        add_follower(golem, ch);
        golem->leader = ch;
        SET_BIT(golem->affected_by, AFF_CHARM);
        return;
}

/* Fire and Ice spell for elementalists */
void spell_fire_and_ice(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, sn_frost, sn_fire, d_type, sn_type;

    sn_frost = skill_lookup("frost breath");
    sn_fire = skill_lookup("fire breath");
    if (sn_fire == -1 || sn_frost == -1)
    {
        send_to_char("The elements fail to combine.\n\r",ch);
        return;
    }

    if (number_percent() > 50)
    {
        sn_type = sn_frost;
        d_type = DAM_COLD;
    }          

    else
    {
        sn_type = sn_fire;
        d_type = DAM_FIRE;
    }

    dam = dice(level + 4,15);

    act("$n unleashes a blast of fire and ice!",ch,0,0,TO_ROOM);
    send_to_char("You unleash a blast of fire and ice!\n\r",ch);

    if (saves_spell(level,victim,d_type))
        dam /= 2;

    damage(ch,victim,dam,sn_type,d_type,TRUE);

    dam = dice(level + 2, 15);

    if (d_type == DAM_COLD)
    {
        d_type = DAM_FIRE;
        sn_type = sn_fire;
    }
    else
    {
        d_type = DAM_COLD;
        sn_type = sn_frost;
    }
    damage(ch,victim,dam,sn_type,d_type,TRUE);
    return;
}         

void spell_sunbolt(int sn,int level, CHAR_DATA *ch,void *vo,int target)
{
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        int dam;

        act("$n prays to the Gods and a beam of sunlight erupts from $s hands!",ch,0,0,TO_ROOM);
        send_to_char("You pray to the Gods and a beam of sunlight erupts from your hands!\n\r",ch);

        if (IS_GOOD(victim))
        {
        act("$n is unaffected by the blast of light.",victim,0,0,TO_ROOM);
        act("You are unaffected by the blast of light.",victim,0,0,TO_CHAR);
        return;
        }       

        dam = dice(level,7);

        if (IS_EVIL(victim))
                dam += dice(level,3);

        if (IS_SET(victim->act,ACT_UNDEAD) && number_percent() > 60)
        {
        act("The positive energy begins to vaporise $n!",victim,0,0,TO_ROOM);
        act("The positive energy begins to vaporise you!",victim,0,0,TO_VICT);
        dam += dice(level,7);
        }    

        if ( !(saves_spell(level,victim,DAM_LIGHT)) )
        {
          spell_blindness(skill_lookup("blindness"), level, ch, (void *) victim, TARGET_CHAR);
        }

        if (saves_spell(level,victim,DAM_LIGHT))
                dam /= 2;

        damage(ch,victim,dam,sn,DAM_LIGHT,TRUE);

        return;
}             

void spell_heroes_feast(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *gch;
    int sate_num, quench_num;
    
    sate_num = skill_lookup("sate");
    quench_num = skill_lookup("quench"); 

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ((IS_NPC(ch) && IS_NPC(gch)) ||
            (!IS_NPC(ch) && !IS_NPC(gch)))
        {
            spell_sate(sate_num,level,ch,(void *) gch,TARGET_CHAR);
            spell_quench(quench_num,level,ch,(void *) gch,TARGET_CHAR);  
        }
    }
}


void spell_mind_shatter( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("Your mind is already broken.\n\r",ch);
        else
          act("$N is already reeling.",ch,NULL,victim,TO_CHAR);
        return;

    }
    af.where     = TO_VULN;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;     
    af.bitvector = VULN_MAGIC;
    affect_to_char( victim, &af );

    af.where     = TO_VULN;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = VULN_BASH;
    affect_to_char( victim, &af );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 25;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    af.location  = APPLY_AC;
    af.modifier  = 100;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( "Your mind begins to tear apart!.\n\r", victim );
    if ( ch != victim )
        act("$N's mind begins to tear apart!",ch,NULL,victim,TO_CHAR);
    return;
}


void spell_wizards_firestorm(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  send_to_char( "You call down a storm of fire!\n\r", ch );
  act( "$n calls down a storm of fire!", ch, NULL, NULL, TO_ROOM );

  dam = dice(200 + 20, 20);

  spell_plague(skill_lookup("plague"), level,ch,(void *) victim,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_FIRE,TRUE );

  dam = dice(210 + 20, 20);

  spell_plague(skill_lookup("plague"), level,ch,(void *) victim,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_FIRE,TRUE );

  dam = dice(200 + 20, 20);

  damage( ch, victim, dam, sn, DAM_FIRE,TRUE );

  dam = dice(200 + 20, 20);

  damage( ch, victim, dam, sn, DAM_FIRE,TRUE );
  return;
}

void spell_prismatic_spray(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int chance;

    chance = number_range( 1, 100);

    if (IS_AFFECTED2(victim, AFF_TURNING) && chance >= 50)
    {
	 send_to_char("Your spell bounces harmlessly off thier magical shield.\r\n", ch);
	return;
    }

    send_to_char( "You raise your hand and out leaps a colored spray!\n\r", ch );
    act( "$n sprays color from their fingers!", ch, NULL, NULL, TO_ROOM );

    dam = dice(70 + 20, 15);

  spell_blindness(skill_lookup("blindness"), level,ch,(void *) victim,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_LIGHT,TRUE );

    dam = dice(70 + 20, 15);

  spell_weaken(skill_lookup("weaken"), level,ch,(void *) 
victim,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_NEGATIVE,TRUE );

    dam = dice(70 + 20, 15);

  cold_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_COLD,TRUE );

    dam = dice(65 + 20, 15);

  fire_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_FIRE,TRUE );

    dam = dice(70 + 20, 15);

  spell_poison(skill_lookup("poison"), level,ch,(void *) victim,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_POISON,TRUE );

    dam = dice(70 + 20, 15);

  acid_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_ACID,TRUE );

    dam = dice(70 + 20, 15);

  shock_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_LIGHTNING,TRUE );
    return;
}


void spell_revitalize( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int chance;

    chance = number_range( 1, 100);

    if (IS_AFFECTED2(victim, AFF_TURNING) && chance >= 50)
    {
         send_to_char("Your spell bounces harmlessly off thier magical shield.\r\n", ch);
        return;
    }       

    if (ch->fighting != NULL)
    {
	send_to_char("You are too occupied to revitalize anyone.{x\n\r",ch);
	return;
    }

    if ( victim->fighting != NULL)
    {
        send_to_char("You cannot revitalize someone in combat!{x\n\r",ch);
        return;
    }

    if ( victim->hit >= victim->max_hit )
    {
        send_to_char("That would be pointless..\n\r",ch);
        return;
    }

    send_to_char( "Godly essence flows through your body knitting your wounds!\n\r", victim );
    send_to_char( "You call divine power to heal!\n\r",ch);

               affect_strip(victim,skill_lookup("plague"));
               affect_strip(victim,skill_lookup("poison"));
               affect_strip(victim,skill_lookup("weaken") );
               affect_strip(victim,skill_lookup("blindness"));
               affect_strip(victim,skill_lookup("sleep"));
               affect_strip(victim,skill_lookup("curse"));

               victim->hit        = victim->max_hit;
               victim->move       = victim->max_move;
               update_pos(victim);

     return;
}


void spell_sturning( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int chance;
    ROOM_INDEX_DATA *pRoomIndex;

    pRoomIndex = get_random_room(victim);

    chance = number_range( 1, 100);


    if ( is_affected( victim, sn ) )
    {
        send_to_char( "You are already shielded.\n\r", ch );
	return;
    }

    af.where     = TO_AFFECTS2;
    af.type      = sn;
    af.level     = level;
    af.duration  = 3;
    af.location  = APPLY_DEX;
    af.modifier  = -4;
    af.bitvector = AFF_TURNING;
    affect_to_char(victim, &af);
    
    send_to_char( "You are protected by a shimmering shield\n\r", victim );
    act("$n becomes surrounded by a shimmering shield.",victim,NULL,NULL,TO_ROOM);

    if (chance > 35)
    {
        return;
    }
    send_to_char("A deep rumbling can be heard as your shimmering shield explodes in your face!\n\r", ch);
    act("$n grows pale as $s shield explodes sending them reeling through time and space.",victim,NULL,NULL,TO_ROOM);
    ch->hit   -= 750;
    ch->mana  -= 750;
    ch->move   = 1;
    affect_strip(victim,skill_lookup("spell turning") );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    do_look( victim, "auto" );
    return;
}


void spell_steel_skin(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your skin is already as hard as possible.\n\r",ch);
	else
	  act("$N's skin is already hard as steel.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS2;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_STEEL_SKIN;
    affect_to_char( victim, &af );
    victim->subtype = level / 6;
    send_to_char( "Your skin becomes hard as steel.\n\r", victim );
    if ( ch != victim )
        act("$N's skin becomes hard as steel.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_tsunami(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int chance;
    int dam;

    chance = number_range( 1, 100);

    if (IS_AFFECTED2(victim, AFF_TURNING) && chance >= 50)
    {
	send_to_char("Your spell bounces harmlessly off thier magical shield.\r\n", ch);
	return;
    }

    send_to_char( "You call a massive wave of water!\n\r", ch );
    act( "A massive wave of water flows over you!", ch, NULL, NULL, TO_ROOM );

    dam = dice(140 + 22, 22);

    damage( ch, victim, dam, sn, DAM_DROWNING,TRUE );

    dam = dice(135 + 21, 21);

    damage( ch, victim, dam, sn, DAM_DROWNING,TRUE );

    if (chance > 40)
    {
        return;
    }

    dam = dice(125 + 20, 20);

    damage( ch, victim, dam, sn, DAM_DROWNING,TRUE );

    if (chance > 20)
    {
        return;
    }
    dam = dice(115 + 20, 20);

    damage( ch, victim, dam, sn, DAM_DROWNING,TRUE );
    return;
}


void spell_shadow_shield( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{ 
   CHAR_DATA *victim = (CHAR_DATA *) vo; 
   AFFECT_DATA af; 

   if ( is_affected( victim, sn ) ) 
   { 
    if (victim == ch) 
      send_to_char("You are already protected.\n\r",ch); 
    else 
      act("$N is already protected.",ch,NULL,victim,TO_CHAR); 
    return; 
   } 

   af.where     = TO_RESIST; 
   af.type      = sn; 
   af.level     = level; 
   af.duration  = 2; 
   af.location  = APPLY_NONE; 
   af.modifier  = 0; 
   af.bitvector = RES_NEGATIVE; 
   affect_to_char( victim, &af ); 
   act( "$n is surrounded by shifting shadows.", victim, NULL, NULL, TO_ROOM ); 
   send_to_char( "You become surrounded by shifting shadows.\n\r", victim ); 
   return; 
} 


void spell_weapon_bless( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
   OBJ_DATA *obj; 
   AFFECT_DATA af; 

   if (target == TARGET_OBJ) 
   { 
    obj = (OBJ_DATA *) vo; 
    if (IS_OBJ_STAT(obj,ITEM_BLESS)) 
    { 
        act("$p is already blessed.",ch,obj,NULL,TO_CHAR); 
        return; 
    } 

    if (IS_OBJ_STAT(obj,ITEM_EVIL)) 
    { 
        AFFECT_DATA *paf; 

        paf = affect_find(obj->affected,skill_lookup("curse")); 
        if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0)) 
        { 
       if (paf != NULL) 
           affect_remove_obj(obj,paf); 
       act("$p glows soft gold.",ch,obj,NULL,TO_ALL); 
       REMOVE_BIT(obj->extra_flags,ITEM_EVIL); 
       return; 
        } 
        else 
        { 
       act("The evil of $p resists your blessings.", 
           ch,obj,NULL,TO_CHAR); 
       return; 
        } 
    } 

    af.where    = TO_OBJECT; 
    af.type       = sn; 
    af.level    = level; 
    af.duration    = 25; 
    af.bitvector    = ITEM_BLESS; 

    af.location    = APPLY_HITROLL; 
    af.modifier    = 15; 
    affect_to_obj(obj,&af); 

    af.location    = APPLY_DAMROLL; 
    af.modifier    = 15; 
    affect_to_obj(obj,&af); 

    act("$p glows a brilliant gold!",ch,obj,NULL,TO_ALL); 
    return; 
   } 
} 


void spell_dark_blessing( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{ 
   CHAR_DATA *victim = (CHAR_DATA *) vo; 
   AFFECT_DATA af; 

   if ( !IS_EVIL(ch )  || !IS_EVIL(victim ) )
   {
       send_to_char("You must be evil to call upon the dark gods.\n\r", ch );
       return;
   }

   if ( is_affected( victim, sn ) ) 
   { 
    if (victim == ch) 
      send_to_char("You are already affected by the dark gift.\n\r",ch); 
    else 
      act("$N is already gifted.",ch,NULL,victim,TO_CHAR); 
    return; 

   } 
   af.where     = TO_AFFECTS; 
   af.type      = sn; 
   af.level     = level; 
   af.duration  = 12; 
   af.bitvector = 0; 

   af.location  = APPLY_DAMROLL; 
   af.modifier  = level / 4; 
   affect_to_char( victim, &af ); 

   af.location  = APPLY_HITROLL; 
   af.modifier  = level / 4; 
   affect_to_char( victim, &af ); 


   af.location  = APPLY_AC; 
   af.modifier  = 0 - level; 
   affect_to_char( victim, &af ); 

   send_to_char( "You beg the dark powers to protect you.\n\r", victim ); 
   if ( ch != victim ) 
    act("$N is protected by the dark powers.",ch,NULL,victim,TO_CHAR); 

   send_to_char("The dark powers take there toll on you.\n\r",ch); 
   ch->hit -= 300; 

   return; 
} 

void spell_divine_protection( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already protected.\n\r",ch);
        else
          act("$N is already protected.",ch,NULL,victim,TO_CHAR);
        return;

    }
    af.where     = TO_AFFECTS2;
    af.type      = sn;
    af.level     = level;
    af.duration  = 7;     
    af.bitvector = AFF_DIVINE_PROTECTION;

    af.location  = APPLY_DAMROLL;
    af.modifier  = level / 7;
    affect_to_char( victim, &af );

    af.location  = APPLY_HITROLL;
    af.modifier  = level / 7;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVES;
    af.modifier  = 0 - level / 7;
    affect_to_char( victim, &af );

    af.location  = APPLY_AC;
    af.modifier  = 0 - level;
    affect_to_char( victim, &af );

    send_to_char( "The very essense of the gods protects you!\n\r", victim );
    if ( ch != victim )
        act("$N is protected by your gods.",ch,NULL,victim,TO_CHAR);
    return;
}


void spell_moonblade(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    send_to_char( "You call upon the power of the moon!\n\r", ch );

    dam = dice(120 + 20, 20);
     
  spell_slow(skill_lookup("slow"), level,ch,(void *) victim,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_HOLY,TRUE );

    dam = dice(120 + 20, 20);

  spell_blindness(skill_lookup("blindness"), level,ch,(void *) victim,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_ENERGY,TRUE );
    return;
}

void spell_tornado(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int chance;

    chance = number_range( 1, 100);

    send_to_char( "You call forth the power of the wind!\n\r", ch );
    act( "$n calls forth the power of the wind!", ch, NULL, NULL, TO_ROOM );

    dam = dice(90 + 20, 20);

  spell_blindness(skill_lookup("blindness"), level,ch,(void *) victim,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_SOUND,TRUE );

    if (chance > 25)
    {
        return;
    }
    dam = dice(110 + 20, 20);

  damage( ch, victim, dam, sn, DAM_SOUND,TRUE );
  send_to_char( "Caught in the storm you feel yourself black out.\n\r", victim );
  victim->position = POS_RESTING;
  return;
}

/* i want this to be the most powerful spell in the game if you edit it 
 * you must make sure all the other ones are weaker 
 */
void spell_wizards_fury(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  send_to_char( "You find the cool calm center within and release all the fury of your magic!\n\r", ch );
  act( "$n calls upon the power of all their magic!", ch, NULL, NULL, TO_ROOM );

  dam = dice(200 + 20, 20);
  fire_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_FIRE,TRUE );

  dam = dice(200 + 20, 20);
  shock_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_LIGHTNING,TRUE );

  dam = dice(200 + 20, 20);
  cold_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_COLD,TRUE );

  dam = dice(200 + 20, 20);
  damage( ch, victim, dam, sn, DAM_BASH,TRUE );

  return;
}

void spell_glacier(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  
  if (IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC) )
  {
    send_to_char( "Your magic fizzles and dies.\n\r", ch);
    return;
  }

  send_to_char( "You wave your hand and form a large glacier!\n\r", ch);
  act( "$n waves his hand and a large glacier forms!", ch, NULL, NULL, TO_ROOM);

  dam = dice(50 + 20, 20);
  damage( ch, victim, dam, sn, DAM_COLD,TRUE);

  dam = dice(50 + 20, 20);
  damage( ch, victim, dam, sn, DAM_COLD,TRUE);

  return;
}
  
void spell_thunderstorm( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;

  if ( IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC) )
  {
      send_to_char( "Your magic fizzles and dies.\n\r", ch);
      return;
  }  

  send_to_char( "You call forth lightning from the heavens!\n\r", ch );
  act( "$n calls forth lightning from the heavens!", ch, NULL, NULL, TO_ROOM );

  dam = dice(180 + 20, 20);
  shock_effect( ch->in_room, level, dam, TARGET_ROOM );

  for ( vch = char_list; vch != NULL; vch = vch_next )
  {
       vch_next = vch->next;
        
       if ( vch->in_room == NULL )
           continue;

       if ( vch->in_room == ch->in_room )
       {

           if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
           {

               if (is_same_group(vch,ch) )
               {
                   continue;
               }

               if ( saves_spell( level, vch, DAM_LIGHTNING ) )
               {

                   shock_effect( vch, level/2, dam/4, TARGET_CHAR );
                   damage( ch, vch, dam, sn, DAM_LIGHTNING, TRUE );
               }

               else
               {
                   shock_effect( vch, level, dam, TARGET_CHAR );
                   damage( ch, vch, dam, sn, DAM_LIGHTNING, TRUE );
               }

            } 

       }
  }

  return;
}


void spell_mystic_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   AFFECT_DATA af;

   if ( is_affected( victim, sn ) )
   {
       if (victim == ch)
           send_to_char("You are already shielded from harm.\n\r",ch);
       else
           act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
           return;
   }    

   af.type      = sn;
   af.duration  = 4 + (level/3);
   af.location  = APPLY_AC;
   af.modifier  = -10 - (level / 7);
   af.bitvector = 0;
   affect_to_char( victim, &af );

   if ( ch != victim )
   {
       act( "$n is surrounded by $N's mystic armour.", victim, NULL, ch, TO_ROOM );
       act( "$N is surrounded by your mystic armour.", ch, NULL, victim, TO_CHAR );
       act( "You are surrounded by $N's mystic armour.", victim, NULL, ch, TO_CHAR );
   }
   else
   {
       act( "$n is surrounded by a mystic armour.", victim, NULL, ch, TO_ROOM );
       send_to_char("You are protected by a mystic armor!", ch);
   }
  
   return;
}


void spell_resilience( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
  AFFECT_DATA af;

    if (!is_affected(ch, sn))
    {
      send_to_char("You are now resistive to draining attacks.\n\r", ch);

      af.where = TO_RESIST;
      af.type = sn;
      af.duration = level / 10;
      af.level = ch->level;
      af.bitvector = RES_ENERGY;
      af.location = 0;
      af.modifier = 0;    
      affect_to_char(ch, &af);
    }
    else
      send_to_char("You are already resistive to draining attacks.\n\r",ch);
      return;
}     
    

void spell_magic_resistance ( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  AFFECT_DATA af;

    if (!is_affected(ch, sn))
    {
      send_to_char("You are now resistive to magic.\n\r", ch);

      af.where = TO_RESIST;
      af.type = sn;
      af.duration = level / 10;          
       af.level = ch->level;
      af.bitvector = RES_MAGIC;
      af.location = 0;
      af.modifier = 0;
      affect_to_char(ch, &af);
    }
    else
      send_to_char("You are already resistive to magic.\n\r", ch);
 
    return;
}              

void spell_sandstorm( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam;
    int hpch;          

    if ( (ch->in_room->sector_type == SECT_AIR)
        || (ch->in_room->sector_type == SECT_WATER_SWIM)
        || (ch->in_room->sector_type == SECT_WATER_NOSWIM) )
    {
        send_to_char("You don't find any sand here to make a storm.\n\r",ch);
        ch->wait = 0;
        return;
    }

    act("$n creates a sand storm with sands lying on the floor.",ch,NULL,NULL,TO_ROOM);
    act("You create a huge sand storm.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX( 10, ch->hit );
    hp_dam  = number_range( hpch/9+1, hpch/5 );    

    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
    sand_effect(ch->in_room,level,dam/2,TARGET_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell(ch,vch,TRUE)
        ||  (IS_NPC(vch) && IS_NPC(ch)
        &&   (ch->fighting != vch /*|| vch->fighting != ch*/)))
            continue;
   
        if ( is_safe(ch, vch) )
            continue;
                               
        if (saves_spell(level,vch,DAM_COLD))
        {
                sand_effect(vch,level/2,dam/4,TARGET_CHAR);
                damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
        }

        else
        {
                sand_effect(vch,level,dam,TARGET_CHAR);
                damage(ch,vch,dam,sn,DAM_COLD,TRUE);
        }
    }
}               

void spell_fire_blade(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *sword;
    AFFECT_DATA *pAf;

    sword = create_object( get_obj_index( OBJ_VNUM_SBLADE ), 0 );

    pAf             =   new_affect();
    pAf->location   =   APPLY_HITROLL;
    pAf->modifier   =   (ch->level / 3);
    pAf->where	    =   TO_OBJECT;
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	sword->level;
    pAf->next       =   sword->affected;
    sword->affected  =   pAf;
    sword->level     =   level;

    pAf             =   new_affect();
    pAf->location   =   APPLY_DAMROLL;
    pAf->modifier   =   (ch->level / 3);
    pAf->where	    =   TO_OBJECT;
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	sword->level;
    pAf->next       =   sword->affected;
    sword->affected  =   pAf;
    sword->level     =   level;

    pAf             =   new_affect();
    pAf->location   =   APPLY_HIT;
    pAf->modifier   =   (ch->level * 2);
    pAf->where	    =   TO_OBJECT;
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	sword->level;
    pAf->next       =   sword->affected;
    sword->affected  =   pAf;
    sword->level     =   level;

    pAf             =   new_affect();
    pAf->location   =   APPLY_MANA;
    pAf->modifier   =   (ch->level );
    pAf->where	    =   TO_OBJECT;
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	sword->level;
    pAf->next       =   sword->affected;
    sword->affected  =   pAf;
    sword->level     =   level;

    sword->value[1] = level / 5 - 3;
    sword->value[2] = level / 5 - 4;
    obj_to_char(sword, ch);
    send_to_char("You summon a Blade made of Flames.\n\r",ch);
    send_to_char("You feel weak, perhaps you should rest.\n\r",ch);
    act( "$n summons a Blade made of Flames.", ch, sword, NULL, TO_ROOM );
    ch->hit   *= .50;
    if ( ch->move >= 100 )
    ch->move  -= 100;
    return;
}


void spell_call_wolf(int sn,int level, CHAR_DATA *ch, void *vo, int target)
{
     CHAR_DATA *mob;

     mob = create_mobile(get_mob_index(MOB_VNUM_WOLF));

     if (mob == NULL)
     {
         send_to_char("Nothing happens.\n\r",ch);
         return;
     }

     char_to_room(mob,ch->in_room);
     mob->leader     = ch;
     mob->master     = ch;
     mob->level      = level;
     mob->max_hit    = level * level / 2;
     mob->hit        = mob->max_hit;
     mob->max_mana   = level + 1000 / 2;
     mob->mana       = mob->max_mana;
     mob->max_move   = level * level / 2;
     mob->move       = mob->max_move;
     mob->damroll    = level/2;
     mob->hitroll    = level/2;
     mob->armor[0]   = -level*4;
     mob->armor[1]   = -level*4;
     mob->armor[2]   = -level*4;
     mob->armor[3]   = -level*3;

     SET_BIT(mob->affected_by, AFF_CHARM);
     act("A gate opens up and $n steps forth.",mob,0,0,TO_ROOM);
     return;
}


void spell_banshee_scream( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *pChar, *pChar_next;
  int dam;
  pChar_next = NULL;

  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
  {
       pChar_next = pChar->next_in_room;

       if ( !is_safe( ch, pChar ) && (pChar != ch))
       {
           act( "$n screams a horrible sound! Your ears pop{x!", ch, NULL, pChar, TO_VICT    );
           dam = number_range( 300, 500 );

           if ( saves_spell( ch->level, pChar, DAM_SOUND ) )
               dam /= 2;
           damage( ch, pChar, dam, sn, DAM_SOUND,TRUE);
       }
  }

  return;
}


void spell_resurrect( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj;
    CHAR_DATA *mob;
    int i;

    obj = get_obj_here( ch, NULL, target_name );

    if ( obj == NULL )
    {
        send_to_char( "Resurrect what?\n\r", ch );
        return;
    }

    /* Nothing but NPC corpses. */

    if( obj->item_type != ITEM_CORPSE_NPC )
    {
        if( obj->item_type == ITEM_CORPSE_PC )
            send_to_char( "You can't resurrect players.\n\r", ch );
        else
            send_to_char( "It would serve no purpose...\n\r", ch );
        return;
    }

    if( obj->level > (ch->level + 2) )
    {
        send_to_char( "You couldn't call forth such a great spirit.\n\r", ch );
        return;
    }

    if( ch->pet != NULL )
    {
        send_to_char( "You already have a pet.\n\r", ch );
        return;
    }

    mob = create_mobile( get_mob_index( MOB_VNUM_ZOMBIE ) );
    mob->level                  = obj->level;
    mob->max_hit                = mob->level * 8 + number_range(
                                        mob->level * mob->level/4,
                                        mob->level * mob->level);

    mob->max_hit *= .9;
    mob->hit                    = mob->max_hit;
    mob->max_mana               = 100 + dice(mob->level,10);
    mob->mana                   = mob->max_mana;
    for (i = 0; i < 3; i++)
        mob->armor[i]           = interpolate(mob->level,100,-100);
    mob->armor[3]               = interpolate(mob->level,100,0);

    for (i = 0; i < MAX_STATS; i++)
        mob->perm_stat[i] = 11 + mob->level/4;

    /* You rang? */
    char_to_room( mob, ch->in_room );
    act( "$p moves slowly and suddenly springs to life as a gruesome zombie!", ch, obj, NULL, TO_ROOM );
    act( "$p moves slowly and suddently sprints to life as a gruesome zombie!", ch, obj, NULL, TO_CHAR );

    extract_obj(obj);

    /* Yessssss, massssssster... */
    SET_BIT(mob->affected_by, AFF_CHARM);
    SET_BIT(mob->act, ACT_PET);
    mob->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    add_follower( mob, ch );
    mob->leader = ch;
    ch->pet = mob;
    /* For a little flavor... */
    do_say( mob, "I wish to serve you master!" );
    return;
}

void spell_call_slayer(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *mob;

    mob = create_mobile(get_mob_index(MOB_VNUM_SLAYER));

    if (mob == NULL)
    {
        send_to_char("Nothing happens.\n\r",ch);
        return;
    }

    char_to_room(mob,ch->in_room);
    mob->leader = ch;
    mob->master = ch;
    mob->level = level;
    mob->max_hit =  ( level * 2 ) * level + 5; 
    mob->hit = mob->max_hit;
    mob->max_mana = 100 + dice(mob->level,10);
    mob->mana     = mob->max_mana;   
    mob->damroll = level/2;
    mob->armor[0] = -level*5;
    mob->armor[1] = -level*5;
    mob->armor[2] = -level*5;
    mob->armor[3] = -level*3;
    mob->alignment = ch->alignment;

    SET_BIT(mob->affected_by, AFF_CHARM);
    act("A gate opens up and $n steps forth.",mob,0,0,TO_ROOM);
    return;
}   


/* hacked off the animate dead spell, mummies are more powerful though,
and you can't control as many. (Ceran)
*/
void spell_create_mummy( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *mummy;
    OBJ_DATA *corpse;
    OBJ_DATA *obj_next;
    OBJ_DATA *obj;
    CHAR_DATA *search;
    AFFECT_DATA af;
    char *name;
    char *last_name;
    char *obj_name;
    char buf1[MSL];
    char buf2[MSL];
    int chance;
    int z_level;
    int control;
    int i;
   

    if ( (is_affected(ch,skill_lookup("animate dead"))
    || is_affected(ch,sn))
	&& level < 70)
    {
        send_to_char("You have not yet regained your powers to control the dead.\n\r",ch);
	return;
    }

    control = 0;

    for (search = char_list; search != NULL; search = search->next)
    {
	if (IS_NPC(search) && (search->master == ch) 
	&& search->pIndexData->vnum == MOB_VNUM_ZOMBIE)
        control += 6;
    else if (IS_NPC(search) && (search->master == ch)
    && search->pIndexData->vnum == MOB_VNUM_SKELETON)
        control += 4;
    else if (IS_NPC(search) && (search->master == ch)
    && search->pIndexData->vnum == MOB_VNUM_MUMMY)
        control += 12;
    }

    control += 4;

    if ((ch->level < 30 && control > 12) || (ch->level < 35 && control > 18)
    || (ch->level < 40 && control > 24) || (ch->level < 52 && control > 30)
    || control >= 36)
    {
        send_to_char("You already control as many undead as you can.\n\r",ch);
	return;
    }
    if(!IS_NPC(ch) && IN_MINE(ch) )
    {	send_to_char("You can't do that in a mine!\n\r",ch);
	return;
    }

    if (level >= 70)
	target_name = "corpse";

    if (target_name[0] == '\0')
    {
        send_to_char("Attempt to mummify which corpse?\n\r",ch);
	return;
    }

    obj_name = str_dup(target_name);

    corpse = get_obj_here(ch,NULL,obj_name);

    if (corpse == NULL)
    {
        send_to_char("You can't mummify that.\n\r",ch);
  	return;
    }

    if ((corpse->item_type != ITEM_CORPSE_NPC) && (corpse->item_type != ITEM_CORPSE_PC) )
    {
        send_to_char("You can't mummify and animate that.\n\r",ch);
	return;
    }

    if (IS_SET(corpse->extra2_flags,CORPSE_NO_ANIMATE))
    {
        send_to_char("That corpse does not have the stability to be animated anymore.\n\r",ch);
	return;
    }

    name = corpse->short_descr;

    for (obj = corpse->contains; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	obj_from_obj(obj);
	obj->carried_by = ch;
	obj_to_room(obj,ch->in_room);
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 20;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    if (level < 70)
	affect_to_char(ch,&af);

    chance = get_skill(ch,sn);

    if (ch->level < corpse->level)
    {
        chance += (2*ch->level);
        chance -= (2*corpse->level);
    }

    chance = URANGE(10,chance,90);

    if (number_percent() > chance)
    {
	act("You fail and destroy $p",ch,corpse,NULL,TO_CHAR);
        act("$n tries to mummify $p but destroys it.",ch,corpse,NULL,TO_ROOM);
	extract_obj(corpse);
	return;
    }

    act("$n utters an incantation and $p slowly stumbles to it's feet.",ch,corpse,NULL,TO_ROOM);
    act("$p shudders and slowly stumbles to it's feet!",ch,corpse,NULL,TO_ROOM);
    act("You invoke the powers of death and $p slowly rises to it's feet.",ch,corpse,NULL,TO_CHAR);

    mummy = create_mobile(get_mob_index(MOB_VNUM_MUMMY));
    char_to_room(mummy,ch->in_room);

    z_level = UMAX(1,corpse->level - number_range(0,2));
    mummy->level = z_level;
    mummy->max_hit = (dice(z_level, 25));
    mummy->max_hit += (z_level * 25);
    mummy->hit = mummy->max_hit;
    mummy->damroll += (z_level*4/5);
    mummy->alignment = -1000;
    for (i = 0; i < 3; i++)
         mummy->armor[i] = interpolate(mummy->level,100,-100);

    mummy->armor[3]      = interpolate(mummy->level,100,0);

    for (i = 0; i < MAX_STATS; i++)
         mummy->perm_stat[i] = 11 + mummy->level/4;          

    last_name = name;
    
    last_name = one_argument(corpse->short_descr, name);
    last_name = one_argument(last_name,name);
    last_name = one_argument(last_name,name);
    name = last_name;

    extract_obj(corpse);
    
    sprintf( buf1, "the mummy of %s", name);
    sprintf( buf2, "A torn and shredded mummy of %s is standing here.\n\r", name);
    free_string(mummy->short_descr);
    free_string(mummy->long_descr);
    mummy->short_descr = str_dup(buf1);
    mummy->long_descr = str_dup(buf2);

    add_follower(mummy,ch);
    mummy->leader = ch;

    af.type = skill_lookup("animate dead");
    af.duration = -1;
    af.bitvector = AFF_CHARM;
    affect_to_char(mummy,&af);

    return;
}

/* You need to strip the flesh of corpses using this spell if your necros
want to animate skeleton on the remains. (Ceran)
*/
void spell_decay_corpse(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *skeleton;
    char *obj_name;
    char *name;
    char *last_name;
    int chance;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if (target_name[0] == '\0')
    {
        send_to_char("Decay which corpse?\n\r",ch);
	return;
    }

    obj_name = str_dup(target_name);

    corpse = get_obj_here(ch,NULL,obj_name);

    if (corpse == NULL)
    {
        send_to_char("You can't find that object.\n\r",ch);
  	return;
    }

    if ((corpse->item_type != ITEM_CORPSE_NPC) && (corpse->item_type != ITEM_CORPSE_PC) )
    {
        send_to_char("You can't decay that.\n\r",ch);
	return;
    }

    name = corpse->short_descr;

    for (obj = corpse->contains; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	obj_from_obj(obj);
	obj->carried_by = ch;
	obj_to_room(obj,ch->in_room);
    }

    chance = get_skill(ch,sn);

    if (number_percent() > chance)
    {
        act("Your decaying becomes uncontrolled and you destroy $p.",ch,corpse,NULL,TO_CHAR);
        act("$n tries to decay $p but reduces it to a puddle of slime.",ch,corpse,NULL,TO_ROOM);
	extract_obj(corpse);
	return;
    }

    act("$n decays the flesh off $p.",ch,corpse,NULL,TO_ROOM);
    act("You decay the flesh off $p and reduce it to a skeleton.",ch,corpse,NULL,TO_CHAR);

    skeleton = create_object(get_obj_index(OBJ_VNUM_SKELETON), 1);
    obj->carried_by = ch;
    obj_to_room(skeleton,ch->in_room);

    skeleton->level = corpse->level;

    last_name = name;
    last_name = one_argument(corpse->short_descr, name);
    last_name = one_argument(last_name,name);
    last_name = one_argument(last_name,name);
    name = last_name;
    extract_obj(corpse);
    
    sprintf( buf1, "the skeleton of %s", name);
    sprintf( buf2, "A skeleton of %s is lying here in a puddle of decayed flesh.", name);
    free_string(skeleton->short_descr);
    free_string(skeleton->description);
    skeleton->short_descr = str_dup(buf1);
    skeleton->description = str_dup(buf2);
    skeleton->item_type = ITEM_SKELETON;

    return;
}

void spell_magic_plate(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *plate;
    AFFECT_DATA af;
    int mod;
    plate = create_object(get_obj_index(OBJ_VNUM_MAGIC_PLATE),0);
    plate->level = ch->level;
    plate->timer = 48;

    mod = (2 + ch->level/10);
    if (mod > 6)    mod = 6;

    plate->size = ch->size;
    af.where = TO_OBJECT;
    af.type = sn;
    af.duration = -1;
    af.bitvector = 0;
    af.level = level;
    af.location = APPLY_HITROLL;
    af.modifier = mod;
    affect_to_obj(plate,&af);
    af.location = APPLY_DAMROLL;
    affect_to_obj(plate,&af);
    af.location = APPLY_HIT;
    af.modifier = URANGE(20,2*ch->level,75);
    affect_to_obj(plate,&af);
    af.location = APPLY_STR;
    af.modifier = 1;
    affect_to_obj(plate,&af);
    af.location = APPLY_WIS;
    affect_to_obj(plate,&af);
    if (ch->level > 40)
        SET_BIT(plate->extra_flags, ITEM_GLOW);
    obj_to_char(plate,ch);
    act("$n creates $p!",ch,plate,0,TO_ROOM);
    act("You create $p!",ch,plate,0,TO_CHAR);
    return;
}


/* This is a great spell for having fun with, especially if you come
across a player's corpse :)
-Ceran
*/
void spell_raise_dead( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *zombie;
    OBJ_DATA *corpse;
    OBJ_DATA *obj_next;
    OBJ_DATA *obj;
    CHAR_DATA *search;
    AFFECT_DATA af;
    char *name;
    char *last_name;
    char *obj_name;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int chance;
    int z_level;
    int control;
   
    if (level == 72)	/* necromancy scroll */
	target_name = "corpse";

    if ((is_affected(ch,sn)
    || is_affected(ch,skill_lookup("mummify")))
	&& level < 70)
    {
	send_to_char("You have not yet regained your powers over the dead.\n\r",ch);
	return;
    }

    control = 0;

    for (search = char_list; search != NULL; search = search->next)
    {
	if (IS_NPC(search) && (search->master == ch) 
	&& search->pIndexData->vnum == MOB_VNUM_ZOMBIE)
        control += 6;
    else if (IS_NPC(search) && (search->master == ch)
    && search->pIndexData->vnum == MOB_VNUM_SKELETON)
        control += 4;
    else if (IS_NPC(search) && (search->master == ch)
    && search->pIndexData->vnum == MOB_VNUM_MUMMY)
        control += 12;
    }
    control += 4;

    if ((ch->level < 30 && control > 12) || (ch->level < 35 && control > 18)
    || (ch->level < 40 && control > 24) || (ch->level < 52 && control > 30)
    || control >= 36)
    {
    send_to_char("You already control as many undead as you can.\n\r",ch);
	return;
    }

    if (target_name[0] == '\0')
    {
	send_to_char("Animate which corpse?\n\r",ch);
	return;
    }
    obj_name = str_dup(target_name);

    corpse = get_obj_here(ch,NULL,obj_name);

    if (corpse == NULL)
    {
	send_to_char("You can't animate that.\n\r",ch);
  	return;
    }

    if ((corpse->item_type != ITEM_CORPSE_NPC) && (corpse->item_type != ITEM_CORPSE_PC) )
    {
	send_to_char("You can't animate that.\n\r",ch);
	return;
    }

   if (IS_SET(corpse->extra_flags,CORPSE_NO_ANIMATE))
    {
	send_to_char("That corpse can not sustain further life beyond the grave.\n\r",ch);
	return;
    }

    name = corpse->short_descr;


    for (obj = corpse->contains; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	obj_from_obj(obj);
	obj->carried_by = ch;
	obj_to_room(obj,ch->in_room);
    }


    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 12;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    if (level < 70)
	    affect_to_char(ch,&af);

    chance = get_skill(ch,sn);

    if (ch->level < corpse->level)
    {
	chance += (3*ch->level);
	chance -= (3*corpse->level);
    }

    chance = URANGE(20,chance,90);

    if (number_percent() > chance)
    {
	act("You fail and destroy $p",ch,corpse,NULL,TO_CHAR);
	act("$n tries to animate a corpse but destroys it.",ch,NULL,NULL,TO_ROOM);
	extract_obj(corpse);
	return;
    }

    act("$n utters an incantation and a burning red glow flares into the eyes of $p.",ch,corpse,NULL,TO_ROOM);
    act("$p shudders and comes to life!",ch,corpse,NULL,TO_ROOM);
    act("You call upon the powers of the dark to give life to $p.",ch,corpse,NULL,TO_CHAR);

 
    zombie = create_mobile(get_mob_index(MOB_VNUM_ZOMBIE));
    char_to_room(zombie,ch->in_room);

    z_level = UMAX(1,corpse->level - number_range(3,6));
    zombie->level = z_level;
    zombie->max_hit = (dice(z_level, 15));
    zombie->max_hit += (z_level * 20);
    zombie->hit = zombie->max_hit;
    zombie->damroll += (z_level*7/10);
    zombie->alignment = -1000;
    last_name = name;
    
    last_name = one_argument(corpse->short_descr, name);
    last_name = one_argument(last_name,name);
    last_name = one_argument(last_name,name);
    name = last_name;

    extract_obj(corpse);
    
    sprintf( buf1, "the zombie of %s", name);
    sprintf( buf2, "A zombie of %s is standing here.\n\r", name);
    free_string(zombie->short_descr);
    free_string(zombie->long_descr);
    zombie->short_descr = str_dup(buf1);
    zombie->long_descr = str_dup(buf2);

    add_follower(zombie,ch);
    zombie->leader = ch;
    
    af.duration = -1;
    af.bitvector = AFF_CHARM;
    affect_to_char(zombie,&af);


    return;
}


void spell_adamantite_golem( int sn, int level, CHAR_DATA *ch, void *vo, int target )	
{
  CHAR_DATA *gch;
  CHAR_DATA *golem;
  AFFECT_DATA af;
  int i = 0;

  if (is_affected(ch,sn))
    {
      send_to_char("You lack the power to create another golem right now.\n\r",
		   ch);
      return;
    }

  send_to_char("You attempt to create an Adamantite golem.\n\r",ch);
  act("$n attempts to create an Adamantite golem.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  ( gch->pIndexData->vnum == MOB_VNUM_ADAMANTITE_GOLEM ) )
	{
	    send_to_char("More golems are more than you can control!\n\r",ch);
	    return;
	}
    }

  golem = create_mobile( get_mob_index(MOB_VNUM_ADAMANTITE_GOLEM) );


  for (i = 0; i < MAX_STATS; i ++)
       golem->perm_stat[i] = UMIN(25,15 + ch->level/10);
            
  golem->perm_stat[STAT_STR] += 3;
  golem->perm_stat[STAT_INT] -= 1;
  golem->perm_stat[STAT_CON] += 2;

  golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: UMIN( (10 * ch->pcdata->perm_hit) + 4000, 30000);
  golem->hit = golem->max_hit;
  golem->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  golem->mana = golem->max_mana;
  golem->level = ch->level;
  golem->hitroll = ch->level / 2 + 5;
  for (i=0; i < 3; i++)
    golem->armor[i] = interpolate(golem->level,100,-100);
  golem->armor[3] = interpolate(golem->level,100,0);
  golem->gold = 0;
  golem->timer = 0;
  golem->damage[DICE_NUMBER] = 13;   
  golem->damage[DICE_TYPE] = 9;
  golem->damage[DICE_BONUS] = ch->level / 2 + 10;

  char_to_room(golem,ch->in_room);
  send_to_char("You created an Adamantite golem!\n\r",ch);
  act("$n creates an Adamantite golem!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level; 
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);  

  SET_BIT(golem->affected_by, AFF_CHARM);
  golem->master = golem->leader = ch;

}


void spell_stone_golem( int sn, int level, CHAR_DATA *ch, void *vo, int target )	
{
  CHAR_DATA *gch;
  CHAR_DATA *golem;
  AFFECT_DATA af;
  int i=0;

  if (is_affected(ch,sn)) 
    {
      send_to_char("You lack the power to create another golem right now.\n\r",
		   ch);
      return;
    }

  send_to_char("You attempt to create a stone golem.\n\r",ch);
  act("$n attempts to create a stone golem.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  ( gch->pIndexData->vnum == MOB_VNUM_STONE_GOLEM ) )
	{
	  i++;
	  if (i > 2)
           {
	    send_to_char("More golems are more than you can control!\n\r",ch);
	    return;
	   }
	}
    }

  golem = create_mobile( get_mob_index(MOB_VNUM_STONE_GOLEM) );


  for (i = 0; i < MAX_STATS; i ++)
       golem->perm_stat[i] = UMIN(25,15 + ch->level/10);
            
  golem->perm_stat[STAT_STR] += 3;
  golem->perm_stat[STAT_INT] -= 1;
  golem->perm_stat[STAT_CON] += 2;

  golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: UMIN( (5 * ch->pcdata->perm_hit) + 2000, 30000);
  golem->hit = golem->max_hit;
  golem->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  golem->mana = golem->max_mana;
  golem->level = ch->level;
  for (i=0; i < 3; i++)
    golem->armor[i] = interpolate(golem->level,100,-100);
  golem->armor[3] = interpolate(golem->level,100,0);
  golem->gold = 0;
  golem->timer = 0;
  golem->damage[DICE_NUMBER] = 8;   
  golem->damage[DICE_TYPE] = 4;
  golem->damage[DICE_BONUS] = ch->level / 2;

  char_to_room(golem,ch->in_room);
  send_to_char("You created a stone golem!\n\r",ch);
  act("$n creates a stone golem!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level; 
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);  

  SET_BIT(golem->affected_by, AFF_CHARM);
  golem->master = golem->leader = ch;

}


void spell_iron_golem( int sn, int level, CHAR_DATA *ch, void *vo, int target )	
{
  CHAR_DATA *gch;
  CHAR_DATA *golem;
  AFFECT_DATA af;
  int i = 0;

  if (is_affected(ch,sn)) 
    {
      send_to_char("You lack the power to create another golem right now.\n\r",
		   ch);
      return;
    }

  send_to_char("You attempt to create an iron golem.\n\r",ch);
  act("$n attempts to create an iron golem.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  ( gch->pIndexData->vnum == MOB_VNUM_IRON_GOLEM ) )
	{
	    send_to_char("More golems are more than you can control!\n\r",ch);
	    return;
	}
    }

  golem = create_mobile( get_mob_index(MOB_VNUM_IRON_GOLEM) );


  for (i = 0; i < MAX_STATS; i ++)
       golem->perm_stat[i] = UMIN(25,15 + ch->level/10);
            
  golem->perm_stat[STAT_STR] += 3;
  golem->perm_stat[STAT_INT] -= 1;
  golem->perm_stat[STAT_CON] += 2;

  golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: UMIN( (10 * ch->pcdata->perm_hit) + 1000, 30000);
  golem->hit = golem->max_hit;
  golem->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  golem->mana = golem->max_mana;
  golem->level = ch->level;
  for (i=0; i < 3; i++)
    golem->armor[i] = interpolate(golem->level,100,-100);
  golem->armor[3] = interpolate(golem->level,100,0);
  golem->gold = 0;
  golem->timer = 0;
  golem->damage[DICE_NUMBER] = 11;   
  golem->damage[DICE_TYPE] = 5;
  golem->damage[DICE_BONUS] = ch->level / 2 + 10;

  char_to_room(golem,ch->in_room);
  send_to_char("You created an iron golem!\n\r",ch);
  act("$n creates an iron golem!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level; 
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);  

  SET_BIT(golem->affected_by, AFF_CHARM);
  golem->master = golem->leader = ch;

}

void spell_preserve(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int vnum, chance;

    vnum = obj->pIndexData->vnum;
	
    if (vnum != OBJ_VNUM_SEVERED_HEAD
	&& vnum != OBJ_VNUM_TORN_HEART
	&& vnum != OBJ_VNUM_SLICED_ARM
	&& vnum != OBJ_VNUM_SLICED_LEG
	&& vnum != OBJ_VNUM_GUTS
	&& vnum != OBJ_VNUM_BRAINS)
    {
        send_to_char("You can't preserve that.\n\r",ch);
        return;
    }

    if (obj->timer > 10)
    {
        send_to_char("It's already well preserved.\n\r",ch);
        return;
    }

    chance = get_skill(ch,sn);
    chance *= 90;

    if (number_percent() > chance)
    {
        act("$n destroys $p.",ch,obj,NULL,TO_ROOM);
        act("You fail and destroy $p.",ch,obj,NULL,TO_CHAR);
        extract_obj(obj);
        return;
    }

    printf_to_char( ch, "You surround %s with a magical energy preserving it.\n\r", obj->name );
    obj->timer += number_range(level/2,level);
    return;
}

void spell_raise_skeleton( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *skeleton;
    OBJ_DATA *corpse;
    OBJ_DATA *obj_next;
    OBJ_DATA *obj;
    CHAR_DATA *search;
    AFFECT_DATA af;
    char *name;
    char *last_name;
    char *obj_name;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int chance;
    int z_level;
    int control;
   

    if (is_affected(ch,sn))
    {
    send_to_char("You have not yet regained your powers to animate bones.\n\r",ch);
	return;
    }

    control = 0;

    for (search = char_list; search != NULL; search = search->next)
    {
	if (IS_NPC(search) && (search->master == ch) 
	&& search->pIndexData->vnum == MOB_VNUM_ZOMBIE)
        control += 6;
        else if (IS_NPC(search) && (search->master == ch)
        && search->pIndexData->vnum == MOB_VNUM_SKELETON)
        control += 4;
        else if (IS_NPC(search) && (search->master == ch)
        && search->pIndexData->vnum == MOB_VNUM_MUMMY)
        control += 12;
    }

    control += 4;

    if ((ch->level < 30 && control > 12) || (ch->level < 35 && control > 18)
    || (ch->level < 40 && control > 24) || (ch->level < 52 && control > 30)
    || control >= 36)
    {
        send_to_char("You already control as many undead as you can.\n\r",ch);
	return;
    }

    if (target_name[0] == '\0')
    {
        send_to_char("Animate which skeleton?\n\r",ch);
	return;
    }

    obj_name = str_dup(target_name);

    corpse = get_obj_here(ch,NULL,obj_name);

    if (corpse == NULL)
    {
	send_to_char("You can't animate that.\n\r",ch);
  	return;
    }

    if (corpse->item_type != ITEM_SKELETON)
    {
	send_to_char("You can't animate that.\n\r",ch);
	return;
    }

    if (IS_SET(corpse->extra_flags,CORPSE_NO_ANIMATE))
    {
        send_to_char("That skeleton does not have the stability to be animated anymore.\n\r",ch);
	return;
    }

    name = corpse->short_descr;

    for (obj = corpse->contains; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	obj_from_obj(obj);
	obj->carried_by = ch;
	obj_to_room(obj,ch->in_room);
    }


    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 8;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    affect_to_char(ch,&af);

    chance = get_skill(ch,sn);

    if (ch->level < corpse->level)
    {
        chance += (4*ch->level);
	chance -= (3*corpse->level);
    }

    chance = URANGE(10,chance,95);

    if (number_percent() > chance)
    {
	act("You fail and destroy $p",ch,corpse,NULL,TO_CHAR);
        act("$n tries to animate a skeleton but destroys it.",ch,NULL,NULL,TO_ROOM);
	extract_obj(corpse);
	return;
    }

    act("$n utters an incantation and $p slowly stumbles to it's feet.",ch,corpse,NULL,TO_ROOM);
    act("$p shudders and slowly stumbles to it's feet!",ch,corpse,NULL,TO_ROOM);
    act("You invoke the powers of death and $p slowly rises to it's feet.",ch,corpse,NULL,TO_CHAR);

    skeleton = create_mobile(get_mob_index(MOB_VNUM_SKELETON));
    char_to_room(skeleton,ch->in_room);

    z_level = UMAX(1,corpse->level - number_range(6,11));
    skeleton->level = z_level;
    skeleton->max_hit = (dice(z_level, 12));
    skeleton->max_hit += (z_level * 15);
    skeleton->hit = skeleton->max_hit;
    skeleton->max_move = (dice(z_level, 12));
    skeleton->move = skeleton->max_move;
    skeleton->damroll += (z_level*2/3);
    skeleton->alignment = -1000;
    skeleton->damage[DICE_NUMBER] = 11;
    skeleton->damage[DICE_BONUS] = ch->level / 2 + 10;
                                                     
    last_name = name;
    
    last_name = one_argument(corpse->short_descr, name);
    last_name = one_argument(last_name,name);
    last_name = one_argument(last_name,name);
    name = last_name;

    extract_obj(corpse);
    
    sprintf( buf1, "the skeleton of %s", name);
    sprintf( buf2, "A skeleton of %s is standing here.\n\r", name);
    free_string(skeleton->short_descr);
    free_string(skeleton->long_descr);
    skeleton->short_descr = str_dup(buf1);
    skeleton->long_descr = str_dup(buf2);

    add_follower(skeleton,ch);
    skeleton->leader = ch;

    af.type = skill_lookup("animate dead");
    af.duration = -1;
    af.bitvector = AFF_CHARM;
    affect_to_char(skeleton,&af);

    return;
}

void spell_forceshield (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            send_to_char ("You are already force-shielded.\n\r", ch);
        else
            act ("$N is already force-shielded.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS2;
    af.type = sn;
    af.level = level;
    af.duration = level / 4;
    af.location = APPLY_AC;
    af.modifier = (level / 5) * -1;
    af.bitvector = AFF_FORCE_SHIELD;
    affect_to_char (victim, &af);
    act ("A sparkling force-shield encircles $n.", victim, NULL, NULL, TO_ROOM);
    send_to_char ("You are encircled by a sparkling force-shield.\n\r", victim);
    return;
}

void spell_staticshield (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            send_to_char ("You are surrounded by static charge.\n\r", ch);
        else
            act ("$N is already surrounded by static charge.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS2;
    af.type = sn;
    af.level = level;
    af.duration = level / 3;
    af.location = APPLY_AC;
    af.modifier = (level / 4) * -1;
    af.bitvector = AFF_STATIC_SHIELD;
    affect_to_char (victim, &af);
    act ("$n is surrounded by a pulse of static charge.", victim, NULL, NULL, TO_ROOM);
    send_to_char ("You are surrounded by a pulse of static charge.\n\r", victim);
    return;
}

void spell_flameshield (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            send_to_char ("You are already protected by fire.\n\r", ch);
        else
            act ("$N is already protected by fire.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS2;
    af.type = sn;
    af.level = level;
    af.duration = (level / 10);
    af.location = APPLY_AC;
    af.modifier = (level / 2) * -1;
    af.bitvector = AFF_FLAME_SHIELD;
    affect_to_char (victim, &af);
    act ("$n is shielded by red walls of flame.", victim, NULL, NULL, TO_ROOM);
    send_to_char ("You are shielded by red walls of flame.\n\r", victim);
    return;
}

void spell_animal_instinct( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already animalistic.\n\r",ch);

	else
	  act("$N is already animalistic.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level/2;
    af.modifier  = level/25;
    af.location  = APPLY_STR;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level/2;
    af.modifier  = level/20;
    af.location  = APPLY_DAMROLL;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You suddenly look like a wild beast!\n\r", victim );

    if ( ch != victim )
	act("$N suddenly grows fangs and claws!",ch,NULL,victim,TO_CHAR);

    return;
}

void spell_powerstorm( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    act( "$n makes a firey blaze of magic engulf the room!", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;

	if ( vch->in_room == NULL )
	    continue;

	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
	        damage( ch,vch,level / 3 * 2 + dice(25, 25), sn, DAM_FIRE,TRUE);
	        damage( ch,vch,level / 3 * 2 + dice(25, 25), sn, DAM_FIRE,TRUE);
   	        continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "A blazing storm of energy rumbles through the area.\n\r", vch );
    }

    return;
}

void spell_earthdrain( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = ch->move * .90;
    victim->mana = UMIN( victim->mana + heal, victim->max_mana );
    victim->move = 0;
    update_pos( victim );
    send_to_char( "{cThe forces of the earth fill you with energy!{x\n\r", victim );
    act( "$n draws magic from the very earth!", ch, NULL, NULL, TO_ROOM );
}


/* Fun spell! Does random stat changes, can easily be fixed to add maladicts/benedicts */
void spell_chaos_flare( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int rnum;
    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already touched by chaos.\n\r",ch);

	else
	  act("$N's skin is already touched by chaos.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level/3;
    rnum = number_percent();

        if (rnum <= 5)
	{	
	    af.modifier  = -30 - level / 5;
	    af.location  = APPLY_AC;	
	    af.bitvector = 0;
	    affect_to_char( victim, &af );
	    send_to_char( "Glinting scales form over your skin!\n\r", victim );

	    if ( ch != victim )
		act("$N's skin is suddenly covered with metallic scales.",ch,NULL,victim,TO_CHAR);
	    return;
	}

        if (rnum <= 15)
 	{	
	    af.modifier  = level / 20;
	    af.location  = APPLY_DAMROLL;	
	    af.bitvector = 0;
	    affect_to_char( victim, &af );
	    send_to_char( "Sharp spikes jut out of your skin!\n\r", victim );

	    if ( ch != victim )
		act("$N's skin is suddenly covered with jagged spikes.",ch,NULL,victim,TO_CHAR);
	    return;
	}

        if (rnum <= 25)
	{	
	    af.modifier  = level / 20;
	    af.location  = APPLY_HITROLL;	
	    af.bitvector = 0;
	    affect_to_char( victim, &af );
	    send_to_char( "Your eyes gleam.\n\r", victim );
	    if ( ch != victim )
		act("$N's eyes gleam.",ch,NULL,victim,TO_CHAR);
	    return;
	}

        if (rnum <= 35)
	{	
	    af.modifier  = level*2;
	    af.location  = APPLY_MOVE;	
	    af.bitvector = 0;
	    affect_to_char( victim, &af );
	    send_to_char( "You suddenly grow an extra set of legs!\n\r", victim );
	    if ( ch != victim )
		act("$N suddenly grows an extra set of legs! Yipes!",ch,NULL,victim,TO_CHAR);

	    return;
	}

        if (rnum <= 45)
	{	
	    af.modifier  = level / 20;
	    af.location  = APPLY_CON;	
	    af.bitvector = 0;
	    affect_to_char( victim, &af );
	    send_to_char( "You grow much tougher!\n\r", victim );

	    if ( ch != victim )
		act("$N seems much tougher all of a sudden.",ch,NULL,victim,TO_CHAR);

	    return;
	}

        if (rnum <= 50)
	{	
	    af.modifier  = level / 4;
	    af.location  = APPLY_DAMROLL;	
	    af.bitvector = 0;
	    affect_to_char( victim, &af );
	    send_to_char( "{YA blaze of light surrounds you!{x\n\r", victim );

	    if ( ch != victim )
		act("{YA blazing halo surrounds $N!{x",ch,NULL,victim,TO_CHAR);
	    return;
	}

        if (rnum <= 65)
	{	
	    af.modifier  = 1 - level / 20;
	    af.location  = APPLY_DEX;	
	    af.bitvector = 0;
	    affect_to_char( victim, &af );
	    send_to_char( "One of your arms suddenly turns into a flipper.\n\r", victim );

	    if ( ch != victim )
		act("One of $N's arms turns into a.. dolphin flipper.",ch,NULL,victim,TO_CHAR);
	    return;
	}

        if (rnum <= 75)
	{	
	    af.modifier  = 1 - level / 20;
	    af.location  = APPLY_INT;	
	    af.bitvector = 0;
	    affect_to_char( victim, &af );
	    send_to_char( "You suddenly feel very stupid.\n\r", victim );

	    if ( ch != victim )
		act("$N is suddenly looking very stupid.",ch,NULL,victim,TO_CHAR);
	    return;
	}

        if (rnum <= 85)
	{	
	    af.modifier  = level * 3;
	    af.location  = APPLY_HIT;	
	    af.bitvector = 0;
	    affect_to_char( victim, &af );
	    send_to_char( "You grow two sizes bigger!\n\r", victim );

	    if ( ch != victim )
		act("$N suddenly gets bigger.. and bigger.. and bigger.",ch,NULL,victim,TO_CHAR);
	    return;
	}

        if (rnum <= 95)
	{	
	    af.modifier  = 1 + level * 2;
	    af.location  = APPLY_AC;	
	    af.bitvector = 0;
	    affect_to_char( victim, &af );
	    send_to_char( "You suddenly feel quite vulnerable. They're all out to get you!\n\r", victim );

	    if ( ch != victim )
		act("$N looks might paranoid all of a sudden.",ch,NULL,victim,TO_CHAR);
	    return;
	}

        if (rnum <= 100)
	{	
	    af.modifier  = 1 - level;
	    af.location  = APPLY_DAMROLL;	
	    af.bitvector = 0;
	    affect_to_char( victim, &af );
	    send_to_char( "{cAck! You turn into an oozing gelatinous blob!\n\r", victim );

	    if ( ch != victim )
		act("{c$N's been turned into a green oozing blob!{c",ch,NULL,victim,TO_ROOM);
 	    return;
	}

    return;
}

void spell_mantle( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already protected against magic.\n\r",ch);

	else
	  act("$N is already protected.",ch,NULL,victim,TO_CHAR);

	return;

    }

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level / 3;
    af.modifier  = 1 - level / 6;
    af.location  = APPLY_SAVES;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by a glowing spell mantle.\n\r", victim );

    if ( ch != victim )
	act("$N is surrounded by a glowing spell mantle.",ch,NULL,victim,TO_CHAR);

    return;
}


void spell_arcane_magic( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int numba;    

    dam = dice( level * 3 / 2, 14 );
	
    numba = number_percent();

    if (numba <= 10)
    {
		if ( saves_spell( level, victim, DAM_ACID ) )
			dam /= 2;
		damage( ch, victim, dam, sn,DAM_ACID,TRUE);
		acid_effect(victim,level,dam,TARGET_CHAR);
		return;
    }

    if (numba <= 20)
    {
		if ( saves_spell( level, victim, DAM_FIRE ) )
			dam /= 2;
		damage( ch, victim, dam, sn,DAM_FIRE,TRUE);
		fire_effect(victim,level,dam,TARGET_CHAR);
		return;
    }

    if (numba <= 30)
    {
		if ( saves_spell( level, victim, DAM_LIGHTNING ) )
			dam /= 2;
		damage( ch, victim, dam, sn,DAM_LIGHTNING,TRUE);
		shock_effect(victim,level,dam,TARGET_CHAR);
		return;
    }

    if (numba <= 40)
    {
	if ( saves_spell( level, victim, DAM_COLD ) )
 	    dam /= 2;
		
	damage( ch, victim, dam, sn,DAM_COLD,TRUE);
	cold_effect(victim,level,dam,TARGET_CHAR);
	return;
    }

    if (numba <= 50)
    {
	if ( saves_spell( level, victim, DAM_HOLY ) )
        dam /= 2;
        damage( ch, victim, dam, sn,DAM_HOLY,TRUE);
	return;
    }

    if (numba <= 60)
    {
	if ( saves_spell( level, victim, DAM_LIGHT ) )
	    dam /= 2;
	damage( ch, victim, dam, sn,DAM_LIGHT,TRUE);
	return;
    }

    if (numba <= 70)
    {
	if ( saves_spell( level, victim, DAM_DROWNING ) )
	    dam /= 2;
	damage( ch, victim, dam, sn,DAM_DROWNING,TRUE);
	return;
    }

    if (numba <= 80)
    {
	if ( saves_spell( level, victim, DAM_DISEASE ) )
   	    dam /= 2;
	damage( ch, victim, dam, sn,DAM_DISEASE,TRUE);
	return;
    }

    if (numba <= 90)
    {
	if ( saves_spell( level, victim, DAM_SLASH ) )
	    dam /= 2;
	damage( ch, victim, dam, sn,DAM_SLASH,TRUE);
	return;
    }

    if (numba <= 100)
    {
	if ( saves_spell( level, victim, DAM_NEGATIVE ) )
 	     dam /= 2;
        dam /=5;
	damage( ch, victim, dam, sn,DAM_NEGATIVE,TRUE);
	acid_effect(victim,level,dam,TARGET_CHAR);
	fire_effect(victim,level,dam,TARGET_CHAR);
	cold_effect(victim,level,dam,TARGET_CHAR);
	shock_effect(victim,level,dam,TARGET_CHAR);
	return;
    }

    return;
}


void spell_animate( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    CHAR_DATA *pet;
    int	length;

    if  ((obj->pIndexData->vnum > 17)
	|| (obj->pIndexData->vnum < 12))
    {
	send_to_char( "That's not a body part!\n\r", ch );
	return;
    }

    pMobIndex = get_mob_index( MOB_VNUM_ANIMATE );
    pet = create_mobile( pMobIndex );
    SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{Gis branded with the mark of %s.{x.\n\r",
	obj->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    free_string( pet->short_descr );
    pet->short_descr = str_dup( obj->short_descr );
    free_string( pet->name );
    pet->name = str_dup( obj->name );
    sprintf( buf, "%s", obj->description);
    length = strlen(buf)-12;
    strncpy( arg, buf, length);
    arg[length] = '\0';
    sprintf( buf, "%s floating here.\n\r", arg);
    free_string( pet->long_descr );
    pet->long_descr = str_dup( buf );
    pet->level = ch->level;
    char_to_room( pet, ch->in_room );
    add_follower( pet, ch );
    pet->leader = ch;
    obj_from_char( obj );
    sprintf( buf, "%s floats up and starts following you.\n\r", pet->short_descr);
    send_to_char( buf, ch);
    sprintf( buf, "%s floats up and starts following $n.", pet->short_descr);
    act( buf, ch, NULL, NULL, TO_ROOM);
    return;
}

void spell_lifeforce(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    AFFECT_DATA af;

    if(victim != ch)
    {
	send_to_char("This spell cannot be cast on others.\n\r",ch);
	return;
    }

    if( IS_AFFECTED2(victim,AFF_LIFEFORCE))
    {
	send_to_char("You are already surrounded by a {2green{x shield.\n\r",ch);
	return;
    }

    af.where = TO_AFFECTS2;
    af.type = sn;
    af.level = level;
    af.duration = level/10;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_LIFEFORCE;
    affect_to_char( victim, &af );
    act( "$n is surrouned by a {2green{x shield.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You are surrounded by a {2green{x shield.\n\r",victim);
    sound( "spellb.wav", victim );
    return;
}


void spell_silver_fire(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  send_to_char( "You find the cool calm center within and release all the fury of your magic!\n\r", ch );
  act( "$n calls upon the power of all their magic!", ch, NULL, NULL, TO_ROOM );

  dam = dice(225 + 20, 20);
  damage( ch, victim, dam, sn, DAM_FIRE,TRUE );

  dam = dice(230 + 20, 20);
  shock_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_FIRE,TRUE );

  dam = dice(230 + 20, 20);
  cold_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_SILVER,TRUE );

  dam = dice(225 + 20, 20);
  damage( ch, victim, dam, sn, DAM_SILVER,TRUE );

  dam = dice(230 + 20, 20);
  fire_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_SILVER, TRUE );
  return;
}



void spell_soulburn(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
    
  act("$n sends {GA{gcrid {GM{gist{x flowing into $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n sends {GA{gcrid {GM{gist{x flowing  into you!",ch,NULL,victim,TO_VICT);
    act("You send {GA{gcrid {GM{gist{x flowing  into  $N.",ch,NULL,victim,TO_CHAR);
    
  dam = dice(195 + 20, 20);
  damage( ch, victim, dam, sn, DAM_COLD,TRUE );
  
  dam = dice(175 + 20, 20);
  cold_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_ACID,TRUE );
  
  dam = dice(195 + 20, 20);
  damage( ch, victim, dam, sn, DAM_COLD,TRUE );
  
  dam = dice(175 + 20, 20);
  cold_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_ACID,TRUE );
  
  dam = dice(195 + 20, 20);
  damage( ch, victim, dam, sn, DAM_COLD, TRUE );
  return;
}


void spell_icelance(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
    
  act("$n creates three large ice and magically impales $N.",ch,NULL,victim,TO_NOTVICT);
  act("$n creates three large spikes of ice and impales you with them!",ch,NULL,victim,TO_VICT);
  act("You create three large spikes of ice and magically impale $N.",ch,NULL,victim,TO_CHAR);
    
  dam = dice(130 + 15, 18);
  damage( ch, victim, dam, sn, DAM_COLD,TRUE );
  
  dam = dice(150 + 17, 18);
  cold_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_ACID,TRUE );
  
  dam = dice(145 + 18, 17);
  damage( ch, victim, dam, sn, DAM_COLD,TRUE );
  
  dam = dice(175 + 20, 20);
  cold_effect(victim,level,dam,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_ACID,TRUE );
  sound( "spell5.wav", ch );
  if ( ch != victim )
  sound( "spell5.wav", victim );
  return;
}



void spell_shield_darkness(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	
    AFFECT_DATA af;

    if( IS_AFFECTED2(ch,AFF_DARKSHIELD))
    {
		send_to_char("You are already surrounded by a {Ddark{x shield.\n\r",ch);
		return;
    }

    af.where = TO_AFFECTS2;
    af.type = sn;
    af.level = level;
    af.duration = level/10;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_DARKSHIELD;
    affect_to_char( ch, &af );
	act( "$n is surrouned by a {Ddark{x shield.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You are surrounded by a {Ddark{x shield.\n\r",ch);
	return;
}

/* Allow a mage to have a weapon dance in their floating spot */
SPELL (spell_dancing_sword)
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;
    AFFECT_DATA  af;
    AFFECT_DATA *paf;
    
    /* Do all the checks here that so the item will float immediatly */
    if (obj->item_type != ITEM_WEAPON)
    {
        send_to_char("Weapons are the only things that can dance.\n\r",ch);
        return;
    }
    
    if ((obj->level > ch->level      && number_classes(ch) == 1) ||
        (obj->level > ch->level + 5  && number_classes(ch) == 2) ||
        (obj->level > ch->level + 10 && number_classes(ch) == 3) ||
        (obj->level > ch->level + 15 && number_classes(ch) == 4))
    {
        send_to_char("You are too weak to float that weapon.\n\r", ch);
        return;
    }
    
    if (!remove_obj(ch,WEAR_FLOAT,TRUE))
    {
        send_to_char("Remove your floating item first.\n\r", ch);
        return;
    }
    
    for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
        if (paf->type == sn)
        {
            send_to_char("The weapon is already dancing.\n\r", ch);
            return;
        }
    }
    
    if (obj->wear_loc != -1)
    {
        send_to_char("The item can't be worn.\n\r", ch);
        return;
    }
    
    
    /* Okay, should be good to go */
    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_obj(obj, &af);
    
    /* It now will be destroyed */
    obj->timer = ch->level;
    obj->wear_flags -= ITEM_WIELD;
    obj->wear_flags += ITEM_WEAR_FLOAT;
    act("$t floats up and dances around $n.",ch,obj->name,NULL,TO_ROOM);
    act("$t floats around you dancing wildly.",ch,obj->name,NULL,TO_CHAR);
    /* Use equip_char and not wear_obj to avoid two messages for wearing it */
    equip_char(ch,obj,WEAR_FLOAT);
    return;
}
