/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    
*
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   
*
*  code is allowed provided you add a credit line to the effect of:         
*
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     
*
*  of the standard diku/rom credits. If you use this or a modified version  
*
*  of this code, let me know via email: moongate@moongate.ams.com. Further  
*
*  updates will be posted to the rom mailing list. If you'd like to get     
*
*  the latest version of quest.c, please send a request to the above add-   
*
*  ress. Quest Code v2.03. Please do not remove this notice from this file. 
****************************************************************************/

/****************************************************************************
 * Updated Quest Code copyright 1999-2001                                   
 * Markanth : markanth@spaceservices.net                                    
 * Devil's Lament : spaceservices.net port 3778                             
 * Web Page : http://spaceservices.net/~markanth/                           
 * Quest objects are now updated by owner's level, code is more dynamic.    
 * All I ask in return is that you give me credit on your mud somewhere     
 * or email me if you use it.                                               
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "const.h"
DECLARE_DO_FUN( do_restore );
DECLARE_DO_FUN( do_transfer );
DECLARE_SPELL_FUN( spell_identify );


extern int mobile_count;

void do_function args((CHAR_DATA *ch, DO_FUN *do_fun, char *argument));


struct quest_type 
{
    char *name;
    char *who_name;
    int vnum;
    int cost;
};

#define QUEST_ARMS 46
#define QUEST_AURA 234
#define QUEST_LEGGINGS 233
#define QUEST_SWORD 213
#define QUEST_BPLATE 212
#define QUEST_BOOTS 44
#define QUEST_MASK 211
#define QUEST_GLOVES 224
#define QUEST_FLAME 232
#define QUEST_HELM 55
#define QUEST_WINGS 52
#define QUEST_BAG 230
#define QUEST_SHIELD 63
#define QUEST_REGEN 227
#define QUEST_INVIS 43
#define QUEST_DEED 216
#define QUEST_DAGGER 99
#define QUEST_DECANT 45
#define QUEST_TRIVIA    OBJ_VNUM_TRIVIA_PILL
#define OBJ_VNUM_TRIVIA_PILL 215

/*
 * Object vnums for Quest Rewards 
 */
const struct quest_type quest_table[] = {
    {"nohunger", "{YNo Hunger/Thirst (quest buy nohunger){x", 0, 3000},
    {"aura", "{YAura{W of {RSanctuary{x", QUEST_AURA, 2600},
    {"leggings", "{YThe{W leggings of the {RGods{x", QUEST_LEGGINGS, 2600},
    {"sword", "{YSword{W of the {RAncients{x", QUEST_SWORD, 2500},
    {"breastplate", "{YBreastPlate{W of the {RAncients{x", QUEST_BPLATE, 2500},
    {"boots", "{YBoots{W of {RHell{x", QUEST_BOOTS, 2500},
    {"mask", "{YMask{W of the {RAncients{x", QUEST_MASK, 2500},
    {"gloves", "{YGloves{W of {RProtection{x", QUEST_GLOVES, 2500},
    {"flame", "{YFlame{W of the {RAncients{x", QUEST_FLAME, 2500},
    {"helm", "{YHelm{W of {RCaptured Inferno{x", QUEST_HELM, 2300},
    {"wings", "{YWings{W of the {RPegasus{x", QUEST_WINGS, 2000},
    {"bag", "{YBag{W of the {RAncients{x", QUEST_BAG, 1000},
    {"shield", "{YShield{W of {RThought{x", QUEST_SHIELD, 1500},
    {"regeneration", "{YRing{W of {RRegeneration{x", QUEST_REGEN, 700},
    {"invisibility", "{YCloak{W of {RInvisibility{x", QUEST_INVIS, 500},
    {"dagger", "{GDagger of Acidx", QUEST_DAGGER, 1500},
    {"deed", "{WAn {YOfficial {RDeed{x", QUEST_DEED, 500},
    {"decanter", "{CDecanter of Endless Water{x", QUEST_DECANT, 500},
    {"shoulder", "{CPads with diamond spiked tips{x", QUEST_ARMS, 2000},
    {"trivia", "{YTrivia {RPill{x", QUEST_TRIVIA, 100},
    {"frequent", "{YFrequent {RQuestor{x", 1, 1000 },
    {"levelflux", "{YLevelflux{x", 2, 10000 },
    {"train",     "{Y100 {RTrains{x", 3, 2000 },
    {"practice",  "{Y1000 {RPractices{x", 4, 2000 }, 
    {"dblquest", "{YDouble {RQuest{x", 5, 5000 },
    {NULL, 0, 0}
};

/* Quests to find objects */
#define QUEST_OBJQUEST1 71
#define QUEST_OBJQUEST2 78
#define QUEST_OBJQUEST3 82
#define QUEST_OBJQUEST4 92

/*
 * CHANCE function. I use this everywhere in my code, very handy :> 
 */
bool chance(int num)
{
    if (number_range(1, 100) <= num)
	return TRUE;
    else
	return FALSE;
}

/* is object in quest table? */
int is_qobj(OBJ_DATA *obj)
{
    int i;

    if (!obj || !obj->pIndexData)
	return -1;

    for (i = 0; quest_table[i].name != NULL; i++)
    {
	if (obj->pIndexData->vnum == quest_table[i].vnum)
	    return i;
    }
    return -1;
}

int qobj_cost(OBJ_DATA * obj)
{
    int i;

    if (!obj || !obj->pIndexData)
	return 0;

    for (i = 0; quest_table[i].name != NULL; i++)
    {
	if (obj->pIndexData->vnum == quest_table[i].vnum)
	    return quest_table[i].cost;
    }
    return 0;
}

/*
 * * Add or enhance an obj affect. 
 */
void affect_join_obj(OBJ_DATA * obj, AFFECT_DATA * paf)
{
    AFFECT_DATA *paf_old;
    bool found;

    found = FALSE;
    for (paf_old = obj->affected; paf_old != NULL; paf_old = paf_old->next)
    {
	if (paf_old->location == paf->location)
	{
	    paf_old->level = paf->level;
	    paf_old->modifier = paf->modifier;
	    found = TRUE;
	}
    }
    if (!found)
	affect_to_obj(obj, paf);
    return;
}

void add_apply(OBJ_DATA * obj, int loc, int mod, int where, int type, int dur,int vector, int level)
{
    AFFECT_DATA pAf;

    if (obj == NULL)
	return;

    if (!obj->enchanted)
	affect_enchant(obj);

    pAf.location = loc;
    pAf.modifier = mod;
    pAf.where = where;
    pAf.type = type;
    pAf.duration = dur;
    pAf.bitvector = vector;
    pAf.level = level;
    affect_join_obj(obj, &pAf);

    return;
}

/* Updates a quest object.  AFFECTS like sanctuary ect..
   are done in game or on area file. (Anything not level based) */
void update_questobjs(CHAR_DATA * ch, OBJ_DATA * obj)
{
    int bonus,
     pbonus,
     cost;

    if (obj == NULL || obj->pIndexData == NULL)
    {
	bug("update_questobjs: NULL obj", 0);
	return;
    }
    if (ch == NULL)
    {
	bug("update_questobjs: NULL ch", 0);
	return;
    }

    if (!IS_OBJ_STAT(obj, ITEM_QUEST) && is_qobj(obj) == -1)
	return;

    bonus = UMAX(5, ch->level / 10);
    pbonus = UMAX(5, ch->level / 5);
    cost = qobj_cost(obj);

    if (obj->level != ch->level)
	obj->level = ch->level;
    if (obj->condition != -1)
	obj->condition = -1;
    if (obj->cost != cost)
	obj->cost = cost;
    if (!CAN_WEAR(obj, ITEM_NO_SAC))
	SET_BIT(obj->wear_flags, ITEM_NO_SAC);
    if (!IS_OBJ_STAT(obj, ITEM_BURN_PROOF))
	SET_BIT(obj->extra_flags, ITEM_BURN_PROOF);
    if (!IS_OBJ_STAT(obj, ITEM_NODROP))
	SET_BIT(obj->extra_flags, ITEM_NODROP);

	/* Bonuses could get fun here */
    if (obj->pIndexData->vnum == QUEST_BPLATE)
    {
	add_apply(obj, APPLY_DAMROLL, pbonus, TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_HITROLL, pbonus, TO_OBJECT, 0, -1, 0, ch->level);
    } else if (obj->pIndexData->vnum == QUEST_SHIELD)
    {
	add_apply(obj, APPLY_DAMROLL, pbonus, TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_HITROLL, pbonus, TO_OBJECT, 0, -1, 0, ch->level);
    } else if (obj->pIndexData->vnum == QUEST_LEGGINGS)
    {
	add_apply(obj, APPLY_DAMROLL, pbonus, TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_HITROLL, pbonus, TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_HIT, UMAX(50, ch->level), TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_MANA, UMAX(50, ch->level), TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_MOVE, UMAX(50, ch->level), TO_OBJECT, 0, -1, 0, ch->level);
    } else if (obj->pIndexData->vnum == QUEST_MASK)
    {
	add_apply(obj, APPLY_DAMROLL, pbonus, TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_HITROLL, pbonus, TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_HIT, UMAX(50, ch->level), TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_MANA, UMAX(50, ch->level), TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_MOVE, UMAX(50, ch->level), TO_OBJECT, 0, -1, 0, ch->level);
    } else if (obj->pIndexData->vnum == QUEST_WINGS)
    {
	add_apply(obj, APPLY_HIT, UMAX(50, ch->level), TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_MANA, UMAX(50, ch->level), TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_MOVE, UMAX(50, ch->level), TO_OBJECT, 0, -1, 0, ch->level);
    } else if (obj->pIndexData->vnum == QUEST_AURA)
    {
	add_apply(obj, APPLY_HIT, UMAX(50, ch->level), TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_MANA, UMAX(50, ch->level), TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_MOVE, UMAX(50, ch->level), TO_OBJECT, 0, -1, 0, ch->level);
    } else if (obj->item_type == ITEM_CONTAINER)
    {
    /* weight modifiers */
	obj->weight = -1 * (50 + (ch->level * 1.5));
	obj->value[0] = 1000 + (20 * ch->level);
	obj->value[3] = 1000 + (20 * ch->level);
    }
    if (obj->item_type == ITEM_WEAPON)
    {
    /* weapon values of levelD5 */
	obj->value[1] = UMAX(15, ( ch->level + 20 ) /5);
	obj->value[2] = ch->level < 80 ? 4 : 5;
	add_apply(obj, APPLY_DAMROLL, bonus, TO_OBJECT, 0, -1, 0, ch->level);
	add_apply(obj, APPLY_HITROLL, bonus, TO_OBJECT, 0, -1, 0, ch->level);
    } else if (obj->item_type == ITEM_ARMOR)
    {
    /* AC value of player level */
	obj->value[0] = UMAX(20, ch->level);
	obj->value[1] = UMAX(20, ch->level);
	obj->value[2] = UMAX(20, ch->level);
	obj->value[3] = (5 * UMAX(20, ch->level)) / 6;
    } else if (obj->item_type == ITEM_STAFF)
	obj->value[0] = UMAX(40, ch->level / 3);
    return;
}

/* Usage info on the QUEST commands*/
/* Keep this in line with the do_quest function's keywords */
void quest_usage(CHAR_DATA * ch)
{
    send_to_char("{GQUEST commands: POINTS, INFO, TIME, REQUEST, COMPLETE, LIST,\n\r" 
		   "                BUY, QUIT, SELL, REPLACE, IDENTIFY, CONVERT.\n\r"
                   "                CONVERTALL, REVERT, REVERTALL DBLQUEST{x\n\r",ch );
	if(IS_IMMORTAL(ch))
	send_to_char("{GQUEST RESET (player): resets players quest.{x\n\r", ch);
    send_to_char("{GFor more information, type 'HELP QUEST'.{x\n\r", ch);
    return;
}

/* Obtain additional location information about sought item/mob */
void quest_where(CHAR_DATA * ch, char *what)
{
    char buf[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *room;

    if(!ch || IS_NPC(ch))
    return;

    if (ch->pcdata->questloc <= 0)
    {
	bug("QUEST INFO: ch->questloc = %ld", ch->pcdata->questloc);
	return;
    }
    if (ch->in_room == NULL)
	return;

    room = get_room_index(ch->pcdata->questloc);
    if (room->area == NULL)
    {
	bug("QUEST INFO: room(%ld)->area == NULL", ch->pcdata->questloc);
	return;
    }
    if (room->area->name == NULL)
    {
	bug("QUEST INFO: area->name == NULL", 0);
	return;
    }
    sprintf(buf, "{RRumor has it this %s was last seen in the area known as %s,{x\n\r", what, room->area->name);
    send_to_char(buf, ch);
    if (room->name == NULL)
    {
	bug("QUEST INFO: room(%ld)->name == NULL", ch->pcdata->questloc);
	return;
    }
    sprintf(buf, "{Rnear %s.{x\n\r", room->name);
    send_to_char(buf, ch);

}				/* end quest_where() */

OBJ_DATA *has_questobj(CHAR_DATA * ch)
{
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObj;

    if(!ch || IS_NPC(ch) || ch->pcdata->questobj <= 0)
    return NULL;

    if ((pObj = get_obj_index(ch->pcdata->questobj)) == NULL)
	return NULL;

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	if (obj != NULL && obj->pIndexData == pObj)
	    return obj;

    return NULL;
}

/*
 * The main quest function 
 */
void do_quest(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *questman;
    OBJ_DATA *obj = NULL;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf[MSL];
    char arg1[MIL];
    char arg2[MIL];
    int i = 0;

    if (IS_NPC(ch))
    {
	send_to_char("I'm sorry, you can't quest.\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
	quest_usage(ch);
	return;
    }

    if (!strcmp(arg1, "info"))
    {
	MOB_INDEX_DATA *qm_mobindex;

	if (!IS_QUESTOR(ch))
	{
	    send_to_char("You aren't currently on a quest.\n\r", ch);
	    return;
	}
	send_to_char("\n\r", ch);
	if (ch->pcdata->questgiver <= 0)
	{
	    bug("QUEST INFO: quest giver = %d", ch->pcdata->questgiver);
	    send_to_char("{RIt seems your quest master died of old age waiting for you.{x\n\r", ch);
	    end_quest(ch, 10);
	    return;
	}
	qm_mobindex = get_mob_index(ch->pcdata->questgiver);
	if (qm_mobindex == NULL)
	{
	    bug("QUEST INFO: quest giver %d has no MOB_INDEX_DATA!", ch->pcdata->questgiver);
	    send_to_char("{RYour quest master has fallen very ill. Please contact an imm!{x\n\r", ch);
	    end_quest(ch, 10);
	    return;
	}
	if (ch->pcdata->questmob == -1 || has_questobj(ch))	/* killed target mob */
	{
	    send_to_char("{RYour quest is {fALMOST{x{R complete!{x\n\r", ch);
	    sprintf(buf, "{RGet back to %s{R before your time runs out!{x\n\r", (qm_mobindex->short_descr == NULL ? "your quest master" : qm_mobindex->short_descr));
	    send_to_char(buf, ch);
	    return;
	} else if (ch->pcdata->questobj > 0)	/* questing for an object 
*/
	{
	    questinfoobj = get_obj_index(ch->pcdata->questobj);
	    if (questinfoobj != NULL)
	    {
		sprintf(buf, "{RYou recall the quest which the %s{R gave you.{x\n\r", (qm_mobindex->short_descr == NULL ? "your quest master" : qm_mobindex->short_descr));
		send_to_char(buf, ch);
		sprintf(buf, "{RYou are on a quest to recover the fabled %s{R!{x\n\r", questinfoobj->name);
		send_to_char(buf, ch);
		quest_where(ch, "treasure");
		return;
	    }
	    /* quest object not found! */
	    bug("No info for quest object %d", ch->pcdata->questobj);
	    ch->pcdata->questobj = 0;
	    REMOVE_BIT(ch->act, PLR_QUESTOR);
	    /* no RETURN -- fall thru to 'no quest', below */
	} else if (ch->pcdata->questmob > 0)	/* questing for a mob */
	{
	    questinfo = get_mob_index(ch->pcdata->questmob);
	    if (questinfo != NULL)
	    {
		sprintf(buf, "{RYou are on a quest to slay the dreaded %s{R!{x\n\r", questinfo->short_descr);
		send_to_char(buf, ch);
		quest_where(ch, "fiend");
		return;
	    }
	    /* quest mob not found! */
	    bug("No info for quest mob %d", ch->pcdata->questmob);
	    ch->pcdata->questmob = 0;	/* Changed to mob instead of obj - Lotus */
	    REMOVE_BIT(ch->act, PLR_QUESTOR);
	    /* no RETURN -- fall thru to 'no quest', below */
	}
	/* we shouldn't be here */
	bug("QUEST INFO: Questor with no kill, mob or obj", 0);
	return;
    }
    if (!strcmp(arg1, "points"))
    {
	sprintf(buf, "You have %d quest points.\n\r", ch->pcdata->questpoints);
	send_to_char(buf, ch);
	return;
    } else if (!strcmp(arg1, "time"))
    {
	if (!IS_SET(ch->act, PLR_QUESTOR))
	{
	    send_to_char("You aren't currently on a quest.\n\r", ch);
	    if (ch->pcdata->nextquest > 1)
	    {
		sprintf(buf, "There are %d minutes remaining until you can go on another quest.\n\r", ch->pcdata->nextquest);
		send_to_char(buf, ch);
	    } else if (ch->pcdata->nextquest == 1)
	    {
		sprintf(buf, "There is less than a minute remaining until you can go on another quest.\n\r");
		send_to_char(buf, ch);
	    }
	} else if (ch->pcdata->countdown > 0)
	{
	    sprintf(buf, "Time left for current quest: %d\n\r", ch->pcdata->countdown);
	    send_to_char(buf, ch);
	}
	return;
    }

    for (questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room)
    {
	if (!IS_NPC(questman))
	    continue;
	if (questman->spec_fun == spec_lookup("spec_questmaster"))
	    break;
    }

    if (questman == NULL || questman->spec_fun != spec_lookup("spec_questmaster"))
    {
	send_to_char("You can't do that here.\n\r", ch);
	return;
    }

    if (questman->fighting != NULL)
    {
	send_to_char("Wait until the fighting stops.\n\r", ch);
	return;
    }

    if (!strcmp(arg1, "list"))
    {
	act("$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM);
	send_to_char("\t{YCurrent Quest Items available for Purchase:{x\n\r", ch);
        for (i = 0; quest_table[i].who_name != NULL; i++)
        {
	    sprintf(buf, "\t%-5dqp ........ %s{x\n\r", quest_table[i].cost, quest_table[i].who_name);
   		send_to_char(buf, ch);
    }
	send_to_char("\tTo buy an item, type 'QUEST BUY <item>'.\n\r", ch);
	send_to_char("\tFor more info on quest items type 'help questitems'\n\r", ch);
	return;
        }

    else if (!strcmp(arg1, "buy"))
    {

	if (arg2[0] == '\0')
	{
	    send_to_char("To buy an item, type 'QUEST BUY <item>'.\n\r", ch);
	    return;
	}

	for (i = 0; quest_table[i].name != NULL; i++)
	{
	    if (is_name(arg2, quest_table[i].name))
	    {
		if (ch->pcdata->questpoints >= quest_table[i].cost)
		{
		    if (quest_table[i].vnum == 0)
		    {
			ch->pcdata->questpoints -= quest_table[i].cost;
			ch->pcdata->condition[COND_FULL] = -1;
			ch->pcdata->condition[COND_THIRST] = -1;
			act("$N calls upon the power of the gods to relieve your mortal burdens.", ch, NULL, questman, TO_CHAR);
			act("$N calls upon the power of the gods to relieve $n's mortal burdens.", ch, NULL, questman, TO_ROOM);
			return;
		    }

                    if (quest_table[i].vnum == 1)
                    {
                        ch->pcdata->questpoints -= quest_table[i].cost;
                        SET_BIT( ch->act, PLR_FREQUENT );
                        send_to_char( "You are now a considered a frequent questor, you will have to wait less time between quests.\n\r", ch ); 
                        act("$N now recognizes $n as a frequent questor!", ch, NULL, questman, TO_ROOM );
                        return;
                    }

                    if ( ch->levelflux < 6 )
                    {
                        if (quest_table[i].vnum == 2 )
                        {
                            ch->pcdata->questpoints -= quest_table[i].cost;
                            ch->levelflux++;
                            printf_to_char( ch, "Your level flux has been raised to %d!\n\r", ch->levelflux );
                            return;
                        }

                   }
              
                   if (quest_table[i].vnum == 3 )
                   {
                       ch->pcdata->questpoints -= quest_table[i].cost;
                       ch->train += 100;
                       send_to_char( "The questmaster gives you 100 trains.\n\r", ch );
                       return;
                   }

                   if (quest_table[i].vnum == 4)
                   {
                       ch->pcdata->questpoints -= quest_table[i].cost;
                       ch->practice += 1000;
                       send_to_char( "The questmaster gives you 1000 practices.\n\r", ch );
                       return;
                   }

                   if (quest_table[i].vnum == 5)
                   {
                       ch->pcdata->questpoints -= quest_table[i].cost;
                       ch->pcdata->dblq = 1;
                       send_to_char( "The questmaster promises to give you double questpoints for every quest you complete from now on.\n\r", ch );
                       return;
                   }

                   else if ((obj = create_object(get_obj_index(quest_table[i].vnum), ch->level)) == NULL)
	           {
		       send_to_char("That object could not be found, contact an immortal.\n\r", ch);
		       return;
		   }

                   else
 	           {
			ch->pcdata->questpoints -= quest_table[i].cost;
			if (!IS_IMMORTAL(ch))
			{
			    sprintf(buf, "Bought a %s (%d) for %d questpoints.", quest_table[i].name, quest_table[i].vnum, quest_table[i].cost);
			    append_file(ch, QUEST_FILE, buf);
			}
		   }

		    if (!IS_SET(obj->pIndexData->extra_flags, ITEM_QUEST))
		    {
			SET_BIT(obj->pIndexData->extra_flags, ITEM_QUEST);
			SET_BIT(obj->extra_flags, ITEM_QUEST);
			SET_BIT(obj->pIndexData->area->area_flags, AREA_CHANGED);
		    }
		    act("$N gives $p to $n.", ch, obj, questman, TO_ROOM);
		    act("$N gives you $p.", ch, obj, questman, TO_CHAR);
		    obj_to_char(obj, ch);
		    save_char_obj(ch);
		    return;
		}
 
                else
		{
		    sprintf(buf, "Sorry, %s, but you need %d quest points for that.", ch->name, quest_table[i].cost);
		    do_mob_tell(ch, questman, buf);
		    return;
		}
	    }
	}

	sprintf(buf, "I don't have that item, %s.", ch->name);
	do_mob_tell(ch, questman, buf);
	return;
    }

    else if (!strcmp(arg1, "sell"))
    {
	if (arg2[0] == '\0')
	{
	    send_to_char("To sell an item, type 'QUEST SELL <item>'.\n\r", ch);
	    return;
	}
	if ((obj = get_obj_carry(ch, arg2, ch)) == NULL)
	{
	    send_to_char("Which item is that?\n\r", ch);
	    return;
	}

	if (!IS_OBJ_STAT(obj, ITEM_QUEST))
	{
	    sprintf(buf, "That is not a quest item, %s.", ch->name);
	    do_mob_tell(ch, questman, buf);
	    return;
	}

	for (i = 0; quest_table[i].name != NULL; i++)
	{
	    if (quest_table[i].vnum <= 0)
		continue;
	    if (quest_table[i].vnum == obj->pIndexData->vnum)
	    {
		ch->pcdata->questpoints += quest_table[i].cost / 3;
		act("$N takes $p from $n.", ch, obj, questman, TO_ROOM);
		sprintf(buf, "$N takes $p from you for %d quest points.", quest_table[i].cost / 3);
		act(buf, ch, obj, questman, TO_CHAR);
		extract_obj(obj);
		save_char_obj(ch);
		return;
	    }
	}
	sprintf(buf, "I only take items I sell, %s.", ch->name);
	do_mob_tell(ch, questman, buf);
	return;
	}
	else if (!strcmp(arg1, "reset"))
	{
		CHAR_DATA *victim;

		if(!IS_IMMORTAL(ch))
		{
			quest_usage(ch);
			return;
		}

		if (IS_NULLSTR(arg2))
		{
		send_to_char("Syntax: quest reset <player>\n\r", ch);
		return;
		}

		if ((victim = get_char_world(ch, arg2)) == NULL)
		{
		send_to_char("They aren't here.\n\r", ch);
		return;
		}

		if (IS_NPC(victim))
		{
		send_to_char("Mobs dont quest.\n\r", ch);
		return;
		}

		end_quest(victim, 0);

		if(victim != ch)
			send_to_char("You clear thier quest.\n\r", ch);
		sprintf(buf, "%s has cleared your quest.\n\r", PERS(ch, victim));
		send_to_char(buf, ch);
		return;
    } else if (!strcmp(arg1, "replace"))
    {
	if (arg2[0] == '\0')
	{
	    send_to_char("To replace an item, type 'QUEST REPLACE <item>'.\n\r", ch);
	    return;
	}
	if ((obj = get_obj_carry(ch, arg2, ch)) == NULL)
	{
	    send_to_char("Which item is that?\n\r", ch);
	    return;
	}

	if (!IS_OBJ_STAT(obj, ITEM_QUEST))
	{
	    sprintf(buf, "That is not a quest item, %s.", ch->name);
	    do_mob_tell(ch, questman, buf);
	    return;
	}
	if (ch->pcdata->questpoints < 75)
	{
	    do_mob_tell(ch, questman, "It costs 75 questpoints to replace an item.");
	    return;
	}
	for (i = 0; quest_table[i].name != NULL; i++)
	{
	    if (quest_table[i].vnum <= 0)
		continue;
	    if (quest_table[i].vnum == obj->pIndexData->vnum)
	    {
		OBJ_DATA *newobj;

		if ((newobj = create_object(get_obj_index(quest_table[i].vnum), ch->level)) == NULL)
		{
		    sprintf(buf, "I could not find a new quest item for you %s.", ch->name);
		    do_mob_tell(ch, questman, buf);
		} else
		{
		    obj_to_char(newobj, ch);
            extract_obj(obj);
            ch->pcdata->questpoints -= 75;
            act("$N takes $p from $n and gives $m a new one.", ch, obj, questman, TO_ROOM);
            act("$N replaces $p with a new one for 75 quest points.", ch, obj, questman, TO_CHAR);
            save_char_obj(ch);
		}
		return;
	    }
	}
	sprintf(buf, "I only replace items I sell, %s.", ch->name);
	do_mob_tell(ch, questman, buf);
	return;
    }

    else if (!strcmp(arg1, "identify"))
    {

	if (IS_NULLSTR(arg2))
	{
	    printf_to_char(ch,"To identify an item, type 'QUEST IDENTIFY <item>'.\n\r");
	    return;
	}

	for (i = 0; quest_table[i].name != NULL; i++)
	{
	    if (is_name(arg2, quest_table[i].name))
	    {
		if (quest_table[i].vnum == 0)
		{
		    send_to_char("That isn't a quest item.\n\r", ch);
		    return;
		} else if ((obj = create_object(get_obj_index(quest_table[i].vnum), ch->level)) == NULL)
		{
		    send_to_char("That object could not be found, contact an immortal.\n\r", ch);
		    return;
		} else
		{
		    if (!IS_SET(obj->pIndexData->extra_flags, ITEM_QUEST))
		    {
			SET_BIT(obj->pIndexData->extra_flags, ITEM_QUEST);
			SET_BIT(obj->extra_flags, ITEM_QUEST);
			SET_BIT(obj->pIndexData->area->area_flags, AREA_CHANGED);
		    }
		    obj_to_char(obj, ch);
		    sprintf(buf, "%s costs %d questpoints.", obj->short_descr, quest_table[i].cost);
			send_to_char(buf, ch);
		    spell_identify(0, ch->level, ch, obj, TAR_OBJ_INV);
		    extract_obj(obj);
		    return;
		}
	    }
	}
	sprintf(buf, "I don't have that item, %s.", ch->name);
	do_mob_tell(ch, questman, buf);
	return;
    }

    else if (!strcmp(arg1, "request"))
    {
	act("$n asks $N for a quest.", ch, NULL, questman, TO_ROOM);
	act("You ask $N for a quest.", ch, NULL, questman, TO_CHAR);
	if (IS_SET(ch->act, PLR_QUESTOR) || ON_GQUEST(ch)) 
	{
	    do_mob_tell(ch, questman, "But you're already on a quest!");
	    return;
	}
	if (ch->pcdata->nextquest > 0)
	{
	    sprintf(buf, "You're very brave, %s, but let someone else have a chance.", ch->name);
	    do_mob_tell(ch, questman, buf);
	    do_mob_tell(ch, questman, "Come back later.");
	    return;
	}

	sprintf(buf, "Thank you, brave %s!", ch->name);
	do_mob_tell(ch, questman, buf);
	ch->pcdata->questmob = 0;
	ch->pcdata->questobj = 0;

	generate_quest(ch, questman);
	return;
    } 
    else if (!strcmp(arg1, "complete"))
    {
	if (ch->pcdata->questgiver != questman->pIndexData->vnum)
	{
	    do_mob_tell(ch, questman, "I never sent you on a quest! Perhaps you're thinking of someone else.");
	    return;
	}

	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    int reward, points;
            int gold, quest, train, practice, silver;

	    if (ch->pcdata->questmob == -1 && ch->pcdata->countdown > 0)
	    {
		reward = number_range(125, 375);
		points = number_range(25, 70);
		act("$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM);
		act("You inform $N you have completed $s quest.", ch, NULL, questman, TO_CHAR);

		do_mob_tell(ch, questman, "Congratulations on completing your quest!");

		sprintf(buf, "As a reward, I am giving you %d quest points, and %d gold.", points, reward);
		do_mob_tell(ch, questman, buf);

		if (chance(points/5))
		{
		    send_to_char("You gain an extra {YTrivia {RPoint{x!\n\r", ch);
		    ch->pcdata->trivia += 1;
		}

                if ( ch->pcdata->countdown >= 8 ) 
                {
                    
                    switch ( number_range( 0, 4 ) )
                    {
                        default:
                        quest = number_range( 10, 25 );
                        printf_to_char( ch, "{cAs a reward for completing your quest so fast i'm giving you {C%d {cquestpoints!\n\r", quest ); 
                        ch->pcdata->questpoints += quest;
                        break;
                        
                        case 0:
                        gold = number_range( 80, 200  );
                        printf_to_char( ch, "{yAs a reward for completing your quest so fast i'm giving {Y%d {ygold!{x\n\r", gold );
                        ch->gold += gold;
                        break;
                    
                        case 1:
                        quest = number_range( 10, 25 );
                        printf_to_char( ch, "{cAs a reward for completing your quest so fast i'm giving you {C%d {cquestpoints!\n\r", quest ); 
                        ch->pcdata->questpoints += quest;
                        break;
              
                        case 2:
                        train = number_range( 2, 8 );
                        printf_to_char( ch, "{bAs a reward for completing your quest so fast i'm giving you {B%d {btrains!{x\n\r", train );
                        ch->train += train;
                        break;
                      
                        case 3:
                        practice = number_range( 5, 10 );
                        printf_to_char( ch, "{rAs a reward for completing your quest so fast i'm giving you {R%d {rpractices!{x\n\r", practice );
                        ch->practice += practice;
                        break;
           
                        case 4:
                        silver = number_range( 250, 500 );
                        printf_to_char( ch, "{gAs a reward for completing your quest so fast i'm giving you {G%d {gsilver{x\n\r", silver );
                        ch->silver += silver;
                        break;                         
                    }
                }

                if ( IS_SET( ch->act, PLR_FREQUENT ) )
                end_quest( ch, 5 );
                else
		end_quest(ch, 10);
                if ( ch->pcdata->dblq == 1 )
                reward *= 2;
		ch->gold += reward;
                if ( ch->pcdata->dblq == 1 || double_qp )
                points *= 2;
		ch->pcdata->questpoints += points;
		save_char_obj(ch);
		return;
	    } 
            else if (ch->pcdata->questobj > 0 && ch->pcdata->countdown > 0)
	    {
		if ((obj = has_questobj(ch)) != NULL)
		{
		    reward = number_range(125, 375);
		    points = number_range(25, 75);
		    act("$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM);
		    act("You inform $N you have completed $s quest.", ch, NULL, questman, TO_CHAR);

		    act("You hand $p to $N.", ch, obj, questman, TO_CHAR);
		    act("$n hands $p to $N.", ch, obj, questman, TO_ROOM);

		    do_mob_tell(ch, questman, "Congratulations on completing your quest!");
		    sprintf(buf, "As a reward, I am giving you %d quest points, and %d gold.", points, reward);
		    do_mob_tell(ch, questman, buf);

                    if ( ch->pcdata->countdown >= 8 ) 
                    {
                        switch ( number_range( 0, 4 ) )
                        {
                            default:
                            quest = number_range( 10, 25 );   
                            printf_to_char( ch, "{cAs a reward for completing your quest so fast i'm giving you {C%d {cquestpoints!\n\r", quest ); 
                            ch->pcdata->questpoints += quest;
                            break;

                            case 0:
                            gold = number_range( 80, 200  );
                            printf_to_char( ch, "{yAs a reward for completing your quest so fast i'm giving {Y%d {ygold!{x\n\r", gold );
                            ch->gold += gold;
                            break;
                    
                            case 1:
                            quest = number_range( 10, 25 );
                            printf_to_char( ch, "{cAs a reward for completing your quest so fast i'm giving you {C%d {cquestpoints!\n\r", quest ); 
                            ch->pcdata->questpoints += quest;
                            break;
              
                            case 2:
                            train = number_range( 2, 8 );
                            printf_to_char( ch, "{bAs a reward for completing your quest so fast i'm giving you {B%d {btrains!{x\n\r", train );
                            ch->train += train;
                            break;
                      
                            case 3:
                            practice = number_range( 5, 10 );
                            printf_to_char( ch, "{rAs a reward for completing your quest so fast i'm giving you {R%d {rpractices!{x\n\r", practice );
                            ch->practice += practice;
                            break;
             
                            case 4:
                            silver = number_range( 250, 500 );
                            printf_to_char( ch, "{gAs a reward for completing your quest so fast i'm giving you {G%d {gsilver{x\n\r", silver );
                            ch->silver += silver;
                            break;                         
                        }
                    }

		    if (chance(points/5))
		    {
			send_to_char("You gain an extra {YTrivia {RPoint{x!\n\r", ch);
			ch->pcdata->trivia += 1;
		    }

                    if ( IS_SET( ch->act, PLR_FREQUENT ) )
                    end_quest(ch, 5 );
                    else
		    end_quest(ch, 10);
                 
                    if ( ch->pcdata->dblq == 1 )
                    reward *= 2;
		    ch->gold += reward;
                    if ( ch->pcdata->dblq == 1 || double_qp )
                    points *= 2;
		    ch->pcdata->questpoints += points;
		    extract_obj(obj);
		    save_char_obj(ch);
		    return;
		} 
                else
		{
		    do_mob_tell(ch, questman, "You haven't completed the quest yet, but there is still time!");
		    return;
		}
		return;
	    } 
            else if ((ch->pcdata->questmob > 0 || ch->pcdata->questobj > 0) && ch->pcdata->countdown > 0)
	    {
		do_mob_tell(ch, questman, "You haven't completed the quest yet, but there is still time!");
		return;
	    }
	}
	if (ch->pcdata->nextquest > 0)
	    sprintf(buf, "But you didn't complete your quest in time!");
	else
	    sprintf(buf, "You have to REQUEST a quest first, %s.", ch->name);
	do_mob_tell(ch, questman, buf);
	return;
    }

    else if (!strcmp(arg1, "quit") || !strcmp(arg1, "fail"))
    {
	act("$n informs $N $e wishes to quit $s quest.", ch, NULL, questman, TO_ROOM);
	act("You inform $N you wish to quit $s quest.", ch, NULL, questman, TO_CHAR);
	if (ch->pcdata->questgiver != questman->pIndexData->vnum)
	{
	    do_mob_tell(ch, questman, "I never sent you on a quest! Perhaps you're thinking of someone else.");
	    return;
	}

	if (IS_SET(ch->act, PLR_QUESTOR))
	{
            if ( IS_SET( ch->act, PLR_FREQUENT ) )
	    end_quest(ch, 10);
            else
            end_quest(ch, 15);
	    do_mob_tell(ch, questman, "Your quest is over, but for your cowardly behavior, you may not quest again for 15 minutes.");
	    return;
	} else
	{
	    send_to_char("You aren't on a quest!", ch);
	    return;
	}
    }
   
    else if (!str_cmp(arg1, "convert"))
    {
        if(ch->pcdata->qps <= 0)
        {
           send_to_char("You don't have any iquest points to convert.\n\r",ch);
           return;
        }

     ch->pcdata->qps--;
     ch->pcdata->questpoints += 100;
     send_to_char("One immortal questpoint converted into 100 atomautic questpoints.\n\r",ch);
     return;
    }
  
    else if (!str_cmp(arg1, "convertall"))
    {
        if ( ch->pcdata->qps <= 0 )
        {
            send_to_char( "I'm sorry you don't have enough immortal questpoints!\n\r", ch );
            return;
        }

        if ( ch->pcdata->qps >= 500 )
        {
            send_to_char( "I'm sorry you have to many questpoints to convert at once! 320 is the max!\n\r", ch );
            return;
        }

        ch->pcdata->questpoints += ( 100 * ch->pcdata->qps );
        ch->pcdata->qps = 0;
        send_to_char( "Ok all of your immortal questpoints have been converted to automatic questpoints!\n\r", ch );
        return;
    }

    else if (!str_cmp(arg1,"revert"))
    {

        if(ch->pcdata->questpoints<100)
        {
  	   send_to_char("You don't have enough aquest points to revert.\n\r",ch);
           return;
        }

        ch->pcdata->qps++;
        ch->pcdata->questpoints -= 100;
        send_to_char("100 aquest points reverted into one iquest point.\n\r",ch);
        return;
     }

     else if (!str_cmp(arg1, "revertall"))
     {
         if ( ch->pcdata->questpoints < 100 )
         {
             send_to_char("You don't have enough automatic questpoints to rever.\n\r", ch );
             return;
         }

         ch->pcdata->qps += ( ch->pcdata->questpoints / 100 );
         ch->pcdata->questpoints = 0;
         send_to_char("Reverting all your questpoints into immortal questpoints.\n\r", ch );
         return;
     }

	quest_usage(ch);
	return;
}

#define MAX_QMOB_COUNT mobile_count

void generate_quest(CHAR_DATA * ch, CHAR_DATA * questman)
{
    CHAR_DATA *victim = NULL;
    ROOM_INDEX_DATA *room = NULL;
    CHAR_DATA *mobs[MAX_QMOB_COUNT];
    size_t mob_count;
    OBJ_DATA *questitem = NULL;
    char buf[MSL];
    int mrange;

    /*
     * * find MAX_QMOB_COUNT quest mobs and store their vnums in mob_buf 
     */

    mob_count = 0;
    for (victim = char_list; victim; victim = victim->next)
    {

	if (!IS_NPC(victim)
	    || !quest_level_diff(ch, victim)
	    || (victim->pIndexData == NULL
		|| victim->in_room == NULL
		|| victim->pIndexData->pShop != NULL)
	    || (IS_EVIL(victim) && IS_EVIL(ch) && chance(50))
	    || (IS_GOOD(victim) && IS_GOOD(ch) && chance(50))
	    || victim->pIndexData->vnum < 100
        || victim->in_room->clan > 0
	    || IS_SET(victim->imm_flags, IMM_WEAPON | IMM_MAGIC)
	    || IS_SET(victim->act, ACT_TRAIN | ACT_PRACTICE | 
        ACT_IS_HEALER | ACT_PET | ACT_PET | ACT_GAIN)
        || IS_SET(victim->affected_by, AFF_CHARM)
		|| IS_SET(victim->in_room->room_flags, ROOM_PET_SHOP)
        || questman->pIndexData == victim->pIndexData
        || (IS_SET(victim->act, ACT_SENTINEL) && 
            IS_SET(victim->in_room->room_flags, ROOM_PRIVATE | ROOM_SOLITARY | ROOM_SAFE)))
	    continue;
	mobs[mob_count++] = victim;
	if (mob_count >= (size_t)MAX_QMOB_COUNT)
	    break;
    }

    if (mob_count == 0) /* not likely but just in case */
    {
	do_mob_tell(ch, questman, "I'm sorry, but I don't have any quests for you at this time.");
	do_mob_tell(ch, questman, "Try again later.");
        if ( IS_SET( ch->act, PLR_FREQUENT ) )
	end_quest(ch, 2);
        else
        end_quest(ch, 4);
	return;
    }

    /*
     * at this point the player is sure to get a quest 
     */
    ch->pcdata->questgiver = questman->pIndexData->vnum;
    mrange = number_range(0, mob_count - 1);
    while ((victim = mobs[mrange]) == NULL)
	mrange = number_range(0, mob_count - 1);
    room = victim->in_room;
    ch->pcdata->questloc = room->vnum;

    /* countdown set here so we can use it for object timers */
    ch->pcdata->countdown = number_range(12, 30);

    /*
     * 20% chance it will send the player on a 'recover item' quest.
     */

    if (chance(20))
    {
	long objvnum = 0;

	switch (number_range(0, 3))
	{
	    case 0:
		objvnum = QUEST_OBJQUEST1;
		break;

	    case 1:
		objvnum = QUEST_OBJQUEST2;
		break;

	    case 2:
		objvnum = QUEST_OBJQUEST3;
		break;

	    case 3:
		objvnum = QUEST_OBJQUEST4;
		break;
	}

	questitem = create_object(get_obj_index(objvnum), ch->level);
	obj_to_room(questitem, room);
	REMOVE_BIT(ch->act, PLR_CANLOOT);
	free_string(questitem->owner);
	questitem->owner = str_dup(ch->name);
	questitem->cost = 0;
	questitem->timer = (4 * ch->pcdata->countdown + 10) / 3;
	ch->pcdata->questobj = questitem->pIndexData->vnum;

	switch (number_range(0, 1))
	{
	    default:
	    case 0:
		sprintf(buf, "Vile pilferers have stolen %s from the royal treasury!", questitem->short_descr);
		do_mob_tell(ch, questman, buf);
		do_mob_tell(ch, questman, "My court wizardess, with her magic mirror, has pinpointed its location.");
		break;
	    case 1:
		sprintf(buf, "A powerful wizard has stolen %s for his personal power!", questitem->short_descr);
		do_mob_tell(ch, questman, buf);
		break;
	}

	if (room->name != NULL)
	{
	    sprintf(buf, "Look for %s somewhere in the vicinity of %s!", questitem->short_descr, room->name);
	    do_mob_tell(ch, questman, buf);

	    sprintf(buf, "That location is in the general area of %s.", room->area->name);
	    do_mob_tell(ch, questman, buf);
	}
    }

    /*
     * Quest to kill a mob 
     */

    else
    {
	switch (number_range(0, 3))
	{
	    default:
	    case 0:
		sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.", victim->short_descr);
		do_mob_tell(ch, questman, buf);
		do_mob_tell(ch, questman, "This threat must be eliminated!");
		break;

	    case 1:
		sprintf(buf, "Thera's most heinous criminal, %s, has escaped from the dungeon!", victim->short_descr);
		do_mob_tell(ch, questman, buf);
		sprintf(buf, "Since the escape, %s has murdered %d civillians!", victim->short_descr, number_range(2, 20));
		do_mob_tell(ch, questman, buf);
		do_mob_tell(ch, questman, "The penalty for this crime is death, and you are to deliver the sentence!");
		break;

	    case 2:
		sprintf(buf, "The Mayor of Midgaard has recently been attacked by %s.  This is an act of war!", victim->short_descr);
		do_mob_tell(ch, questman, buf);
		sprintf(buf, "%s must be severly dealt with for this injustice.", victim->short_descr);
		do_mob_tell(ch, questman, buf);
		break;

	    case 3:
		sprintf(buf, "%s has been stealing valuables from the citizens of Arkham.", victim->short_descr);
		do_mob_tell(ch, questman, buf);
		sprintf(buf, "Make sure that %s never has the chance to steal again.", victim->short_descr);
		do_mob_tell(ch, questman, buf);
		break;

	}

	if (room->name != NULL)
	{
	    sprintf(buf, "Seek %s out somewhere in the vicinity of %s!", victim->short_descr, room->name);
	    do_mob_tell(ch, questman, buf);

	    sprintf(buf, "That location is in the general area of %s.", room->area->name);
	    do_mob_tell(ch, questman, buf);
	}
	ch->pcdata->questmob = victim->pIndexData->vnum;
    }
    if (ch->pcdata->questmob > 0 || ch->pcdata->questobj > 0)
    {
	SET_BIT(ch->act, PLR_QUESTOR);
	sprintf(buf, "You have %d minutes to complete this quest.", ch->pcdata->countdown);
	do_mob_tell(ch, questman, buf);
	do_mob_tell(ch, questman, "May the gods go with you!");
    } else
	end_quest(ch, 2);
    return;
}

bool quest_level_diff(CHAR_DATA * ch, CHAR_DATA * mob)
{
    int bonus = 20;

    if (IS_IMMORTAL(ch))
	return TRUE;
    else if (ch->level > (mob->level + bonus) || ch->level < (mob->level - bonus))
	return FALSE;
    else
	return TRUE;
}

void quest_update(void)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;
    char buf [MSL];

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	if (d->connected == CON_PLAYING && (ch = d->original ? d->original : d->character) != NULL)
	{

	    if (ch->pcdata->nextquest > 0)
	    {
		ch->pcdata->nextquest--;
		if (ch->pcdata->nextquest == 0)
		{
		    send_to_char("You may now quest again.\n\r", ch);
		    return;
		}
	    } else if (IS_SET(ch->act, PLR_QUESTOR))
	    {
		if (--ch->pcdata->countdown <= 0)
		{
                    if ( IS_SET( ch->act, PLR_FREQUENT ) )
		    end_quest(ch, 5);
                    else
                    end_quest(ch, 10 );
		    sprintf(buf, "You have run out of time for your quest!\n\rYou may quest again in %d minutes.\n\r", ch->pcdata->nextquest);
	    	    send_to_char(buf, ch);
		}
		if (ch->pcdata->countdown > 0 && ch->pcdata->countdown < 6)
		{
		    send_to_char("Better hurry, you're almost out of time for your quest!\n\r", ch);
		    return;
		}
	    }
	}
    }
    return;
}

void end_quest(CHAR_DATA * ch, int time)
{
    REMOVE_BIT(ch->act, PLR_QUESTOR);
    ch->pcdata->questgiver = 0;
    ch->pcdata->countdown = 0;
    ch->pcdata->questmob = 0;
    ch->pcdata->questobj = 0;
    ch->pcdata->questloc = 0;
    ch->pcdata->nextquest = time;
}

/* handy dandy, victim must be an NPC */
void do_mob_tell(CHAR_DATA * ch, CHAR_DATA * victim, char *argument)
{
    char buf[MSL];

    if (!victim)
	return;

    sprintf(buf, "{R%s tells you '{W%s{R'{x\n\r", victim->short_descr, argument);
    send_to_char(buf, ch);
    return;
}

void do_tpspend(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *triviamob;
    char buf[MSL];
    char arg1[MIL];
    char arg2[MIL];
    int sav_trust;

    argument = one_argument(argument, arg1);
    strcpy(arg2, argument);

    if(!ch || IS_NPC(ch))
    return;

    if (arg1[0] == '\0')
    {
	send_to_char("Syntax: Tpspend <item>\n\r", ch);
	send_to_char("        Tpspend list\n\r", ch);
	return;
    }

    if (is_name(arg1, "list"))
    {
	send_to_char("{WTrivia Point Options{x\n\r", ch);
	send_to_char("{Ycorpse retrival{W......{R1tp\n\r", ch);
	send_to_char("{Ytransfer{W.............{R1tp\n\r", ch);
	send_to_char("{Yrestore{W..............{R1tp\n\r", ch);
	send_to_char("{Y5 trains{W.............{R1tp\n\r", ch);
	send_to_char("{Y40 practices{W.........{R1tp\n\r", ch);
	send_to_char("{Y75 questpoints{W.......{R1tp\n\r", ch);
	send_to_char("{Y1 Trivia Pill{W........{R1tp\n\r", ch);
	send_to_char("{WSee HELP TRIVIA for important info before buying.{X\n\r", ch);
	send_to_char("{WFor transfers and corpses, you do not have to be at a trivia shop.\n\r", ch);
	return;
    } else if (is_name(arg1, "corpse"))
    {
	if (ch->pcdata->trivia >= 1)
	{
	    OBJ_DATA *obj;
	    int count = 0;

           for (obj = ch->carrying ; obj != NULL ; obj = obj->next_content) 
	    {
		if (is_name(ch->name, obj->owner) && obj->item_type == ITEM_CORPSE_PC)
		{
                    if(obj->in_room) 
	 	    obj_from_room(obj);
                   
                    else if(obj->carried_by)
                    obj_from_char(obj);
          
                    else if(obj->in_obj)
                    obj_from_obj(obj->in_obj);
	            obj->carried_by = ch;
        	    obj_to_room(obj, ch->in_room);
		    count++;
		}
	    }
	    if (count == 0)
		send_to_char("You have no corpses in the game.\n\r", ch);
	    else
	    {
		ch->pcdata->trivia -= 1;
		if (count == 1)
		{
		    send_to_char("Your corpse appears in the room.\n\r", ch);
		    act("$n's corpse appears in the room.", ch, NULL, NULL, TO_ROOM);
		} else
		{
		    send_to_char("All your corpses appear in the room.\n\r", ch);
		    act("All of $n's corpses appear in the room.", ch, NULL, NULL, TO_ROOM);
		}
	    }
	    return;
	} else
	{
	    send_to_char("You don't have enough trivia points for that.", ch);
	    return;
	}
    } else if (is_name(arg1, "transfer"))
    {
	if (ch->pcdata->trivia >= 1)
	{
	    ROOM_INDEX_DATA *oldroom;

	    if (arg2[0] == '\0')
	    {
		send_to_char("Transfer you where? [recall/room name/character name]\n\r", ch);
		return;
	    } else if (!str_cmp(arg2, "recall"))
		sprintf(arg2, "%d", ROOM_VNUM_TEMPLE);

	    oldroom = ch->in_room;
	    sprintf(buf, "self '%s'", arg2);
	    sav_trust = ch->trust;
	    ch->trust = MAX_LEVEL;
	    do_function(ch, &do_transfer, buf);
	    ch->trust = sav_trust;
	    if (oldroom != ch->in_room)
		ch->pcdata->trivia -= 1;
	    else
		send_to_char("Whoops! You were not charged for that transfer.\n\r", ch);
	    return;
	} else
	{
	    send_to_char("You don't have enough trivia points for that.", ch);
	    return;
	}
    }
    for (triviamob = ch->in_room->people; triviamob != NULL; triviamob = triviamob->next_in_room)
    {
	if (!IS_NPC(triviamob))
	    continue;
	if (triviamob->spec_fun == spec_lookup("spec_triviamob"))
	    break;
    }

    if (triviamob == NULL || triviamob->spec_fun != spec_lookup("spec_triviamob"))
    {
	send_to_char("You can't do that here.\n\r", ch);
	return;
    }

    if (triviamob->fighting != NULL)
    {
	send_to_char("Wait until the fighting stops.\n\r", ch);
	return;
    }

    if (is_name(arg1, "practices pracs practice"))
    {
	if (ch->pcdata->trivia >= 1)
	{
	    ch->pcdata->trivia -= 1;
	    ch->practice += 40;
	    act("$N gives 40 practices to $n.", ch, NULL, triviamob, TO_ROOM);
	    act("$N gives you 40 practices.", ch, NULL, triviamob, TO_CHAR);
	    return;
	} else
	{
	    sprintf(buf, "Sorry, %s, but you don't have enough trivia points for that.", ch->name);
	    do_mob_tell(ch, triviamob, buf);
	    return;
	}
    } else if (is_name(arg1, "trains train"))
    {
	if (ch->pcdata->trivia >= 1)
	{
	    ch->pcdata->trivia -= 1;
	    ch->train += 5;
	    act("$N gives 5 training sessions to $n.", ch, NULL, triviamob, TO_ROOM);
	    act("$N gives you 5 training sessions.", ch, NULL, triviamob, TO_CHAR);
	    return;
	} else
	{
	    sprintf(buf, "Sorry, %s, but you don't have enough trivia points for that.", ch->name);
	    do_mob_tell(ch, triviamob, buf);
	    return;
	}
    } else if (is_name(arg1, "questpoints points"))
    {
	if (ch->pcdata->trivia >= 1)
	{
	    ch->pcdata->trivia -= 1;
	    ch->pcdata->questpoints += 75; /* a trivia point costs 100 quest points 25% conversion */
	    act("$N gives 75 questpoints to $n.", ch, NULL, triviamob, TO_ROOM);
	    act("$N gives you 75 questpoints.", ch, NULL, triviamob, TO_CHAR);
	    return;
	} else
	{
	    sprintf(buf, "Sorry, %s, but you don't have enough trivia points for that.", ch->name);
	    do_mob_tell(ch, triviamob, buf);
	    return;
	}
    } else if (is_name(arg1, "pill"))
    {
	OBJ_DATA *obj = NULL;

	if (ch->pcdata->trivia >= 1)
	{
	    obj = create_object(get_obj_index(OBJ_VNUM_TRIVIA_PILL), 1);
	    if (obj != NULL)
	    {
		act("$N gives $p to $n.", ch, obj, triviamob, TO_ROOM);
		act("$N gives you $p.", ch, obj, triviamob, TO_CHAR);
		obj_to_char(obj, ch);
		ch->pcdata->trivia -= 1;
		return;
	    } else
	    {
		sprintf(buf, "I don't any more trivia pills to give, %s.", ch->name);
		do_mob_tell(ch, triviamob, buf);
	    }
	    return;
	}

	else
	{
	    sprintf(buf, "Sorry, %s, but you don't have enough trivia points for that.", ch->name);
	    do_mob_tell(ch, triviamob, buf);
	    return;
	}
    } else if (is_name(arg1, "restore"))
    {
	if (ch->pcdata->trivia >= 1)
	{
        sav_trust = ch->trust;
        ch->trust = MAX_LEVEL;
        do_function(ch, &do_restore, "all");
        ch->trust = sav_trust;
        ch->pcdata->trivia -= 1;
        return;
	} else
	{
	    sprintf(buf, "Sorry, %s, but you don't have enough trivia points for that.", ch->name);
	    do_mob_tell(ch, triviamob, buf);
	    return;
	}
    } else
	do_tpspend(ch, "list");
}

void do_qpgive(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MIL],
     buf[MIL];
    int amount;

    if(!ch || IS_NPC(ch))
    return;

    argument = one_argument(argument, arg);

    if (argument[0] == '\0' || !is_number(arg))
    {
	send_to_char("Syntax: qpgive [amount] [person]\n\r", ch);
	return;
    }

    if ((amount = atoi(arg)) <= 0)
    {
	send_to_char("Give how many questpoints?\n\r", ch);
	return;
    }

    if (amount > ch->pcdata->questpoints)
    {
	send_to_char("You don't have that many questpoints to give.\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, NULL, argument)) == NULL)
    {
	send_to_char("That person is not here.\n\r", ch);
	return;
    }

    if(IS_NPC(victim))
    {
       send_to_char("NPC's don't need quest points.\n\r", ch);
       return;
    }

    if(victim == ch)
    {
       printf_to_char( ch, "You give yourself %d questpoints..... don't you feel better?\n\r", amount );
       return;
    }

    ch->pcdata->questpoints -= amount;
    victim->pcdata->questpoints += amount;

    sprintf(buf, "%d", amount);
    act("$n gives you $t questpoints.", ch, buf, victim, TO_VICT);
    act("You give $N $t questpoints.", ch, buf, victim, TO_CHAR);
    act("$n gives $N $t questpoints.", ch, buf, victim, TO_ROOM);

    return;
}

void do_tpgive(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MIL],
     buf[MIL];
    int amount;

    if(!ch || IS_NPC(ch))
    return;

    argument = one_argument(argument, arg);

    if (argument[0] == '\0' || !is_number(arg))
    {
	send_to_char("Syntax: tpgive [amount] [person]\n\r", ch);
	return;
    }

    if ((amount = atoi(arg)) <= 0)
    {
	send_to_char("Give how many trivia points?\n\r", ch);
	return;
    }

    if (amount > ch->pcdata->trivia)
    {
	send_to_char("You don't have that many trivia points to give!!\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, NULL, argument)) == NULL)
    {
	send_to_char("That person is not here.\n\r", ch);
	return;
    }

    if(IS_NPC(victim))
    {
       send_to_char("NPC's don't need trivia points.\n\r", ch);
       return;
    }

    if(victim == ch)
    {
        printf_to_char( ch, "You give yourself %d trivia points...... don't you feel better?\n\r", amount );
        return;
    }

    ch->pcdata->trivia -= amount;
    victim->pcdata->trivia += amount;

    sprintf(buf, "%d", amount);
    act("$n gives you $t trivia points.", ch, buf, victim, TO_VICT);
    act("You give $N $t trivia points.", ch, buf, victim, TO_CHAR);
    act("$n gives $N $t trivia points.", ch, buf, victim, TO_ROOM);

    return;
}


void do_token ( CHAR_DATA *ch, char *argument )
{

   OBJ_DATA *obj=NULL;   
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_STRING_LENGTH];
   int value;

   one_argument( argument, arg );

   if (IS_NPC(ch))
        return;

   if (arg[0] == '\0')
   {
        send_to_char("How many qp should the token be worth?\n\r", ch);
        return;
   }

   if (ch->pcdata->questpoints <= 0 )
   {
        send_to_char("You have no qp left to spend!\n\r", ch);
        return;
   }

   else  
   { 
        value = atoi(arg);

        if (value <= 0 )
        {
           send_to_char("Tokens may only by made for positive amounts.\n\r", ch);
           return;
        }

        else if (value > ch->pcdata->questpoints )
        {
           send_to_char("You don't have that many quest points.\n\r",ch);
           return;
        }

        else
        {
           obj = create_object(get_obj_index(OBJ_VNUM_TOKEN), 0);
           sprintf( buf, obj->short_descr, value );
           free_string( obj->short_descr );
           obj->short_descr = str_dup( buf );
           sprintf( buf, obj->description, value );
           free_string( obj->description );
           obj->description = str_dup( buf );
           obj->value[0]   =    value;
           ch->pcdata->questpoints -=   value;
           obj_to_char(obj,ch);
           act( "You snap your fingers and produce a quest token!", ch, NULL, NULL,TO_CHAR);
           act( "$n snaps $s fingers and produces a quest token!", ch, NULL, NULL, TO_ROOM);
           return;
        }
   }

   return;
}


void unfinished_quest(CHAR_DATA *ch)
{
    if (IS_QUESTOR(ch))
    {
        bool found = FALSE;

        sprintf(log_buf, "Creating unfinished quest for %s.", ch->name);
        log_string(log_buf);
        wiznet(log_buf, NULL, NULL, 0, 0, 0);
        if (ch->pcdata->questobj > 0)
        {
            OBJ_DATA *obj = NULL;
            ROOM_INDEX_DATA *pRoom = NULL;      
 
                 for(obj = object_list; obj != NULL; obj = obj->next)
                 if(obj->pIndexData->vnum == ch->pcdata->questobj
                    && is_name(ch->name, obj->owner))
                    found = TRUE;

            if(!found)
            {
                if ((obj = create_object(get_obj_index(ch->pcdata->questobj), ch->level)) == NULL)
                end_quest(ch, 0);
                else
                {
                if ((pRoom = get_room_index(ch->pcdata->questloc)) == NULL)
                {
                    pRoom = get_random_room(ch);
                    ch->pcdata->questobj = pRoom->vnum;
                }
                obj_to_room(obj, pRoom);
            }       

            do_quest(ch, "info");
            send_to_char("\n\r", ch);
            }
        } else if (ch->pcdata->questmob > 0)
        {
            CHAR_DATA *mob;

            for(mob = char_list; mob != NULL; mob = mob->next)
                if(IS_NPC(mob) && mob->pIndexData->vnum == ch->pcdata->questmob)
                   break;

            if (!mob)
            end_quest(ch, 5);
            else
            {
            ch->pcdata->questloc = mob->in_room->vnum;
            do_quest(ch, "info");
            send_to_char("\n\r", ch);
            }            

        }
    }
}
       

void update_all_qobjs(CHAR_DATA * ch)
{
    OBJ_DATA *obj;
    AFFECT_DATA *o_paf;

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
        if (IS_OBJ_STAT(obj, ITEM_QUEST))
        {
            update_questobjs(ch, obj);

            for (o_paf = obj->pIndexData->affected; o_paf != NULL; o_paf = o_paf->next)
            affect_join_obj(obj, o_paf);
            for (o_paf = obj->affected; o_paf != NULL; o_paf = o_paf->next)
            affect_check(ch, o_paf->where, o_paf->bitvector);
        }
        else
        {         
            for (o_paf = obj->affected; o_paf != NULL; o_paf = o_paf->next)
                affect_check(ch, o_paf->where, o_paf->bitvector);
            if (!obj->enchanted)
            {
                for (o_paf = obj->pIndexData->affected; o_paf != NULL;
                     o_paf = o_paf->next)
                    affect_check(ch, o_paf->where, o_paf->bitvector);
            }
        }
    }
}                    


void do_levelflux( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *questman;
    ROOM_INDEX_DATA *in_room;
    int cost = 5000;

    in_room = ch->in_room;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Sorry mobs don't have pets!\n\r", ch );
        return;
    }

    if ( ch->pet == NULL )
    {
        send_to_char( "You don't have a pet get one first!\n\r", ch );
        return;
    }

    if ( ch->pet->in_room->vnum != ch->in_room->vnum )
    {
        send_to_char( "Your pet has to be here with you!\n\r", ch );
        return;
    }

    if ( ch->pet->level < 40 )    
    {
        send_to_char( "Your pet has to be at least level 40 to raise his levelflux!\n\r", ch );
        return;
    }

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room )
    {
        if (!IS_NPC(questman)) continue;
        if (questman->spec_fun == spec_lookup( "spec_questmaster" )) break;
    }

    if (questman == NULL || questman->spec_fun != spec_lookup( "spec_questmaster" ))
    {
        send_to_char("You can't do that here.\n\r",ch);
        return;
    }            
  
    if ( questman->fighting != NULL)
    {
        send_to_char("Wait until the fighting stops.\n\r",ch);
        return;
    }
       
    if ( ch->pet->levelflux >= 5 )
    {
        send_to_char( "I'm sorry you can't train your levelflux past 5!\n\r", ch );
        return;
    }

    if ( ch->pcdata->questpoints < cost )
    {
        printf_to_char( ch, "I'm sorry it takes %d questpoints to buy a levelflux.\n\r", cost );
        return;
    }    

    ch->pet->levelflux++;
    ch->pcdata->questpoints -= cost;
    printf_to_char( ch, "Your pets levelflux is now %d.\n\r", ch->pet->levelflux );
    printf_to_char( ch, "The questmaster relieves you of %d questpoints.\n\r", cost );
    return;
} 

