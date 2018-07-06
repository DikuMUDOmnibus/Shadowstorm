/****************************************************************************
 * Automated Warfare Code                                                   *
 * Markanth : dlmud@dlmud.com                                               *
 * Devil's Lament : dlmud.com port 3778                                     *
 * Web Page : http://www.dlmud.com                                          *
 *                                                                          *
 * Provides 4 types of automated wars.                                      *
 *                                                                          *
 * All I ask in return is that you give me credit on your mud somewhere     *
 * or email me if you use it.                                               *
 ****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "interp.h"
#include "tables.h"
#include "const.h"
/* local variables*/

CHAR_DATA * find_warmaster ( CHAR_DATA *ch )
{
    CHAR_DATA * warmaster;

    for ( warmaster = ch->in_room->people; warmaster != NULL; warmaster = warmaster->next_in_room )
    {
        if (!IS_NPC(warmaster))
            continue;

        if (warmaster->spec_fun == spec_lookup( "spec_warmaster" ) )
            return warmaster;
    }  

    if  ( warmaster == NULL || warmaster->spec_fun != spec_lookup( "warmaster" ))
    {
        send_to_char("What? You can't do that here.\n\r", ch);
        return NULL;
    }

    if ( warmaster->fighting != NULL )
    {
        send_to_char("Wait until the fighting stops.\n\r",ch );
        return NULL;
    }

    return NULL;
}                        

char *wartype_name(int type)
{
    switch (type)
    {
    case 1:
	return "Race";
    case 2:
	return "Class";
    case 3:
	return "Genocide";
    case 4:
	return "Clan";
    default:
	return "Unknown";
    }
}

bool start_war(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *wch, *warmaster;
    int blevel, elevel, type;

    warmaster = find_warmaster( ch );
  
    if (!warmaster)
        return FALSE;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
	send_to_char("Syntax: war start <min_level> <max_level> <type>\n\r"
	       "where <type> is either:\n\r"
	       "1 - race war,\n\r2 - klass war,\n\r3 - genocide war,\n\r4 - clan war\n\r",
	       ch);
	return FALSE;
    }

    blevel = atoi(arg1);
    elevel = atoi(arg2);
    type = atoi(arg3);

    if (blevel <= 0 || blevel > MAX_LEVEL)
    {
	sprintf(buf, "Level must be between 1 and %d.\n\r", MAX_LEVEL);
	send_to_char(buf, ch);
	return FALSE;
    }

    if (blevel <= 0 || elevel > MAX_LEVEL)
    {
	sprintf(buf, "Level must be between 1 and %d.\n\r", MAX_LEVEL);
	send_to_char(buf, ch);
	return FALSE;
    }

    if (elevel < blevel)
    {
	send_to_char("Max level must be greater than the min level.\n\r", ch);
	return FALSE;
    }

    if (elevel - blevel < 5)
    {
	send_to_char("Levels must have a difference of at least 5.\n\r", ch);
	return FALSE;
    }

    if (type < 1 || type > 4)
    {
	send_to_char
	  ("The type either has to be 1 (race), 2 (class), 3 (genocide) or 4 (clan).\n\r",
	   ch);
	return FALSE;
    }

    if (war_info.iswar != WAR_OFF)
    {
	send_to_char("There is already a war going!\n\r", ch);
	return FALSE;
    }

    if (!IS_IMMORTAL(ch))
    {
	int cost = 500;

	if (ch->gold < cost)
	{
	    sprintf(buf,
		     "$N tells you 'It costs %d gold to start a %s war.'", cost,
		     wartype_name(type));
	    act(buf, ch, NULL, warmaster, TO_CHAR);
	    return FALSE;
	}
	else
	{
	    sprintf(buf,
		     "$N tells you 'Thank you $n, %s war started, you are %d gold lighter.'",
		     wartype_name(type), cost);
		act(buf, ch, NULL, warmaster, TO_CHAR);
	    ch->gold -= cost;
	}
    }

    war_info.iswar = WAR_WAITING;
    free_string(war_info.who);
    war_info.who = str_dup(ch->name);
    war_info.min_level = blevel;
    war_info.max_level = elevel;
    war_info.wartype = type;
    sprintf(buf, "$n announces a %s war for levels %d to %d.  Type 'WAR' to kill or be killed.",
	     wartype_name(war_info.wartype), war_info.min_level,
	     war_info.max_level);
    war_channel(ch, buf);
    sprintf(buf, "You announce a %s war for levels %d to %d.\n\r",
	     wartype_name(war_info.wartype), war_info.min_level,
	     war_info.max_level);
    send_to_char(buf, ch);
    war_info.timer = 3;
    war_info.next = 0;
    for (wch = char_list; wch != NULL; wch = wch->next)
    {
	if (!IS_NPC(wch) && IS_SET(wch->act, PLR_WAR))
	    REMOVE_BIT(wch->act, PLR_WAR);
    }
    return TRUE;
}

void auto_war(void)
{
    CHAR_DATA *wch, *wch_last, *warmaster;
    int maxlvl = 0, minlvl = MAX_LEVEL, middle = LEVEL_HERO / 2;
    int clan = 0, count = 0, lbonus = 0, half = 0;
    char buf[MAX_INPUT_LENGTH];

    if (war_info.iswar != WAR_OFF)
	return;

    for (wch = char_list; wch != NULL; wch = wch->next)
    {
	if (!IS_NPC(wch) && !IS_IMMORTAL(wch))
	{
	    count++;
	    maxlvl = UMAX(maxlvl, wch->level);
	    minlvl = UMIN(minlvl, wch->level);
	    if (is_clan(wch))
	    {
		for (wch_last = char_list; wch_last != NULL;
		     wch_last = wch_last->next)
		{
		    if (!IS_NPC(wch_last) && !IS_IMMORTAL(wch_last) && is_clan(wch_last) &&
			!is_same_clan(wch, wch_last))
			clan++;
		}
	    }
	}
    }

    if (count < 2)
    {
	end_war();
	return;
    }

    lbonus = number_range(5, 10);
    minlvl = UMAX(1, minlvl - lbonus);
    maxlvl = UMIN(LEVEL_HERO, maxlvl + lbonus);
    half = ((maxlvl - minlvl) / 2);
    middle = URANGE(minlvl, maxlvl - half, maxlvl);
    minlvl = number_range(minlvl, middle - lbonus);
    maxlvl = number_range(middle + lbonus, maxlvl);

    for(warmaster = char_list; warmaster != NULL; warmaster = warmaster->next)
    {
        if(!IS_NPC(warmaster))
            continue;
        if(warmaster->pIndexData->vnum == MOB_VNUM_WARMASTER)
            break;
    }

    war_info.iswar = WAR_WAITING;
    free_string(war_info.who);
    war_info.who =
      !warmaster ? str_dup("AutoWar (tm)") : str_dup(warmaster->short_descr);
    war_info.min_level = minlvl;
    war_info.max_level = maxlvl;

    if (clan >= 2)
	war_info.wartype = number_range(1, 4);
    else
	war_info.wartype = number_range(1, 3);

    sprintf(buf, "{G%s {B%s{x war for levels {G%d to {B%d%s{x.  Type 'WAR' to kill or be killed.",
	     !warmaster ? "A" : "$n announces a",
	     wartype_name(war_info.wartype), war_info.min_level,
	     war_info.max_level, !warmaster ? " has started" : "");
    war_channel(warmaster, buf);
    war_info.timer = 3;
    war_info.next = 0;
    for (wch = char_list; wch != NULL; wch = wch->next)
    {
	if (!IS_NPC(wch) && IS_SET(wch->act, PLR_WAR))
	    REMOVE_BIT(wch->act, PLR_WAR);
    }
}

void end_war(void)
{
    CHAR_DATA *wch;

    free_string(war_info.who);
    war_info.who = str_dup("");
    war_info.wartype = 0;
    war_info.min_level = 0;
    war_info.max_level = 0;
    war_info.iswar = WAR_OFF;
    war_info.inwar = 0;
    war_info.timer = 0;
    war_info.next = number_range(30, 60);
    for (wch = char_list; wch != NULL; wch = wch->next)
    {
	if (!IS_NPC(wch) && IS_SET(wch->act, PLR_WAR))
	{
	    stop_fighting(wch, TRUE);
	    REMOVE_BIT(wch->act, PLR_WAR);
	    if (IS_SET(wch->in_room->room_flags, ROOM_ARENA) ||
		wch->in_room->vnum == ROOM_VNUM_WAITROOM)
	    {
		char_from_room(wch);
		char_to_room(wch, get_room_index(ROOM_VNUM_TEMPLE));
	    }
	    wch->hit = wch->max_hit;
	    wch->mana = wch->max_mana;
	    wch->move = wch->max_move;
	    update_pos(wch);
            do_function( wch, &do_look, "auto" );
	}
    }
}

void do_war(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    int i = 0;

    if (IS_NPC(ch))
    {
	send_to_char("Mobiles not supported yet.\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:  {Rwar {Bstart <minlev> <maxlev> <#type>{x\n\r", ch);
	send_to_char("         {Rwar {Btalk <message>{x\n\r", ch);
	send_to_char("         {Rwar {Bstatus{x\n\r", ch);
	send_to_char("         {Rwar {Binfo{x\n\r", ch);
	send_to_char("         {Rwar {Bjoin{x\n\r", ch);
	if (IS_IMMORTAL(ch))
    {
	    send_to_char("         war end\n\r"
	                 "         war next\n\r", ch);
    }
	return;
    }

    else if (!str_cmp(arg, "start"))
    {
	start_war(ch, argument);
    return;
    }

    else if (!str_cmp(arg, "talk"))
    {
	war_talk(ch, argument);
	return;
    }

    else if (!str_cmp(arg, "next") && IS_IMMORTAL(ch))
    {
	if (war_info.iswar == TRUE)
	{
	    send_to_char("Not while a war is running.\n\r", ch);
	    return;
	}

	i = is_number(argument) ? atoi(argument) : number_range(30, 100);
	war_info.next = i;
	sprintf(buf, "The next war will start in %d minutes.\n\r",
	war_info.next);
        send_to_char(buf, ch);
	return;
    }

    if (war_info.iswar != TRUE)
    {
	sprintf(buf,
		 "Their is no war going! The next war will start in %d minutes.\n\r",
		 war_info.next);
        send_to_char(buf, ch);
	return;
    }

    if (!str_cmp(arg, "end") && IS_IMMORTAL(ch))
    {
	end_war();
	sprintf(buf, "You end the war. Next war in %d minutes.\n\r",
		 war_info.next);
    send_to_char(buf, ch);
	sprintf(buf, "$n has ended the war. The next autowar will start in %d minutes.",
		 war_info.next);
    war_channel(ch, buf);
	sprintf(buf, "You have ended the war. The next autowar will start in %d minutes.\n\r",
		 war_info.next);
    send_to_char(buf, ch);
	return;
    }
    else if (!str_cmp(arg, "info"))
    {
	sprintf(buf, "Started by  : %s\n\r",
		 war_info.who[0] == '\0' ? "Unknown" : war_info.who);
    send_to_char(buf, ch);
	sprintf(buf, "Fighting    : %d player%s.\n\r", war_info.inwar,
		 war_info.inwar == 1 ? "" : "s");
    send_to_char(buf, ch);
	sprintf(buf, "Levels      : %d - %d\n\r", war_info.min_level,
		 war_info.max_level);
    send_to_char(buf, ch);
	sprintf(buf, "Status      : %s for %d minutes.\n\r",
		 war_info.iswar == WAR_WAITING ? "Waiting" : "Running",
		 war_info.timer);
    send_to_char(buf, ch);
	sprintf(buf, "Type        : %s war.\n\r",
		 wartype_name(war_info.wartype));
    send_to_char(buf, ch);
	return;
    }
    else if (!str_cmp(arg, "check"))
    {
	CHAR_DATA *wch;
	bool found = FALSE;

	for (wch = char_list; wch != NULL; wch = wch->next)
	{
	    if (!IS_NPC(wch) && IS_SET(wch->act, PLR_WAR))
	    {
		sprintf(buf,
			 "%-12s : [%ld%% hit] [%ld%% mana] [Pos: %s]\n\r",
			 wch == ch ? "You" : wch->name,
			 wch->hit * 100 / wch->max_hit,
			 wch->mana * 100 / wch->max_mana,
			 position_table[wch->position].short_name);
	    send_to_char(buf, ch);	 
		found = TRUE;
	    }
	}
	if (!found)
	    send_to_char("No one in the war yet.\n\r", ch);
	return;
    }
    else if (!str_cmp(arg, "join"))
    {
	if (war_info.iswar == WAR_RUNNING)
	{
	    send_to_char("The war has allready started, your too late.\n\r", ch);
	    return;
	}

	if (ch->level < war_info.min_level || ch->level > war_info.max_level)
	{
	    send_to_char("Sorry, you can't join this war.\n\r", ch);
	    return;
	}

	if (IS_SET(ch->act, PLR_WAR))
	{
	    send_to_char("You are already in the war.\n\r", ch);
	    return;
	}

        if (war_info.wartype == 4 && !is_clan(ch) )
	{
	    send_to_char("You aren't in a clan, you can't jon this war.\n\r", ch);
	    return;
	}

	if ((location = get_room_index(ROOM_VNUM_WAITROOM)) == NULL)
	{
	    send_to_char("The arena isn't completed yet, sorry.\n\r", ch);
	    return;
	}
	else
	{
	    act("$n has gone to join the war!", ch, NULL, NULL,
		TO_ROOM);
	    char_from_room(ch);
	    char_to_room(ch, location);
	    SET_BIT(ch->act, PLR_WAR);
	    sprintf(buf, "%s (Level %d) joins the war!", ch->name, ch->level);
            war_channel(NULL, buf);
	    act("$n has arrived to join the war!", ch, NULL, NULL, TO_ROOM);
	    war_info.inwar++;
            do_function( ch, &do_look, "auto" );
	}
	return;
    }

    auto_war( );    
    return;
}

bool abort_race_war(void)
{
    CHAR_DATA *ch;
    CHAR_DATA *vict;

    for (ch = char_list; ch != NULL; ch = ch->next)
    {
	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_WAR))
	{
	    for (vict = char_list; vict != NULL; vict = vict->next)
	    {
		if (!IS_NPC(vict) && IS_SET(vict->act, PLR_WAR))
		{
		    if (ch->race == vict->race)
			continue;
		    else
			return FALSE;
		}
	    }
	}
    }
    return TRUE;
}

bool abort_class_war(void)
{
    CHAR_DATA *ch;
    CHAR_DATA *vict;

    for (ch = char_list; ch != NULL; ch = ch->next)
    {
	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_WAR))
	{
	    for (vict = char_list; vict != NULL; vict = vict->next)
	    {
		if (!IS_NPC(vict) && IS_SET(vict->act, PLR_WAR))
		{
		    if (ch->klass == vict->klass)
			continue;
		    else
			return FALSE;
		}
	    }
	}
    }
    return TRUE;
}

bool abort_clan_war(void)
{
    CHAR_DATA *ch;
    CHAR_DATA *vict;

    for (ch = char_list; ch != NULL; ch = ch->next)
    {
	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_WAR) && is_clan(ch))
	{
	    for (vict = char_list; vict != NULL; vict = vict->next)
	    {
		if (!IS_NPC(vict) && IS_SET(vict->act, PLR_WAR) && is_clan(vict))
		{
		    if (is_same_clan(ch, vict))
			continue;
		    else
			return FALSE;
		}
	    }
	}
    }
    return TRUE;
}

void war_update ( void )
{
    char buf[MAX_STRING_LENGTH];

    if ( war_info.iswar == WAR_OFF && war_info.next > 0 )
    {
        if ( --war_info.next <= 0 )
            auto_war (  );
    }

    else if ( war_info.iswar == WAR_WAITING )
    {
        int randm = 0;

        war_info.timer--;

        if ( war_info.timer > 0 )
        {
            sprintf ( buf, "%d minute%s left to join the war. (Levels %d - %d, %s War)",
                       war_info.timer, war_info.timer == 1 ? "" : "s",
                       war_info.min_level, war_info.max_level,
                       wartype_name ( war_info.wartype ) );
            war_channel(NULL, buf);
        }
        else
        {
            if ( war_info.inwar < 2 )
            {
                end_war (  );
                sprintf ( buf, "Not enough people for a war.  Next autowar in %d minutes.",
                           war_info.next );
                war_channel(NULL, buf);
            }
            else if ( war_info.wartype == 1 && abort_race_war (  ) )
            {
                end_war (  );
                sprintf ( buf, "Not enough races for a war.  Next autowar in %d minutes.",
                           war_info.next );
                war_channel(NULL, buf);
            }
            else if ( war_info.wartype == 2 && abort_class_war (  ) )
            {
                end_war (  );
                sprintf ( buf, "Not enough classes for a war.  Next autowar in %d minutes.",
                           war_info.next );
                war_channel(NULL, buf);
            }
            else if ( war_info.wartype == 4 && abort_clan_war (  ) )
            {
                end_war (  );
                sprintf ( buf, "Not enough clans for a war.  Next autowar in %d minutes.",
                           war_info.next );
                war_channel(NULL, buf);
            }
            else
            {
                CHAR_DATA *wch;

                sprintf(buf,  "The battle begins! %d players are fighting!",
                           war_info.inwar );
                war_channel ( NULL, buf);
                          
                war_info.timer =
                    number_range ( 3 * war_info.inwar, 5 * war_info.inwar );
                war_info.iswar = WAR_RUNNING;
                for ( wch = char_list; wch != NULL; wch = wch->next )
                {
                    if ( !IS_NPC(wch) && IS_SET ( wch->act, PLR_WAR ) )
                    {
                        randm = number_range ( 12004, 12080  );
                        char_from_room ( wch );
                        char_to_room ( wch, get_room_index ( randm ) );
                        do_function ( wch, &do_look, "auto" );
                    }
                }
            }
        }
    }

    else if ( war_info.iswar == WAR_RUNNING )
    {
        if ( war_info.inwar == 0 )
        {
            end_war (  );
            sprintf ( buf, "No one left in the War, next war will start in %d minutes.",
                       war_info.next );
                war_channel(NULL, buf);
            return;
        }

        switch ( war_info.timer )
        {
            case 0:
                end_war (  );
                sprintf ( buf, "Time has run out on the War, next war will start in %d minutes.",
                           war_info.next );
                war_channel(NULL, buf);
                return;
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 10:
            case 15:
                sprintf ( buf, "%d minute%s remaining in the war.",
                           war_info.timer, war_info.timer > 1 ? "s" : "" );
                war_channel(NULL, buf);
            default:
                war_info.timer--;
                break;
        }
        return;
    }
}

void check_war(CHAR_DATA * ch, CHAR_DATA * victim)
{
    CHAR_DATA *wch;
    char buf[MAX_STRING_LENGTH];
    int reward = number_range(500, 1500);
    int qreward = number_range( 50, 150 );

    if (IS_NPC(ch) || IS_NPC(victim))
	return;

    REMOVE_BIT(victim->act, PLR_WAR);
    war_info.inwar--;
    stop_fighting(victim, TRUE);
    char_from_room(victim);
    char_to_room(victim, get_room_index(ROOM_VNUM_ALTAR));
	victim->hit = UMAX(1, victim->hit);
	victim->mana = UMAX(1, victim->hit);
	victim->move = UMAX(1, victim->hit);
    update_pos(victim);
    do_look(victim, "auto");
    send_to_char("\n\r", ch);
    send_to_char("\n\r", victim);
    sprintf(buf, "%s was killed in combat by %s!",
	     victim->name, ch->name);
    war_channel(NULL, buf);
    switch (war_info.wartype)
    {
    case 1:
	if (abort_race_war())
	{
	    sprintf(buf, "The %s's have won the War!",
		     pc_race_table[ch->race].name);
            war_channel(NULL, buf);

	    for (wch = char_list; wch != NULL; wch = wch->next)
	    {
		if (!IS_NPC(wch) && IS_SET(wch->act, PLR_WAR))
		    continue;

		if (wch->race == ch->race)
		{
		    wch->gold += reward;
                    wch->pcdata->questpoints += qreward;
		    sprintf(buf,
			     "You recieve %d gold and %d questpoints for winning the war!\n\r", reward, qreward );
		    send_to_char(buf, wch);
		}

	    }
	    end_war();
	    return;
	}			// end abort
	break;
    case 2:
	if (abort_class_war())
	{
	    sprintf(buf, "The %s's have won the War!", class_long( ch ) );
            war_channel(NULL, buf);

	    for (wch = char_list; wch != NULL; wch = wch->next)
	    {
		if (!IS_NPC(wch) && IS_SET(wch->act, PLR_WAR))
		    continue;
		if (wch->klass == ch->klass)
		{
		    wch->gold += reward;
                    wch->pcdata->questpoints += reward;
		    sprintf(buf,
			     "You recieve %d gold and %d questpoints for winning the war!\n\r", reward, qreward );
			send_to_char(buf, wch);
		}
	    }
	    end_war();
	    return;
	}
	break;
    case 4:
	if (abort_clan_war())
	{
	    sprintf(buf, "%s has won the War!",
		     clan_table[ch->clan].who_name);
        war_channel(NULL, buf);
	    for (wch = char_list; wch != NULL; wch = wch->next)
	    {
		if (!IS_NPC(wch) && IS_SET(wch->act, PLR_WAR))
		    continue;
		if (is_same_clan(ch, wch))
		{
		    wch->gold += reward;
                    wch->pcdata->questpoints += qreward;
		    sprintf(buf,
			     "You recieve %d gold and %d questpoints for winning the war!\n\r", reward, qreward );
			send_to_char(buf, wch);
		}
	    }
	    end_war();
	    return;
	}
	break;
    case 3:
	if (war_info.inwar == 1)
	{
	    sprintf(buf, "%s has won the War!", ch->name); 
            war_channel(NULL, buf);
	    ch->gold += reward;
            ch->pcdata->questpoints += reward;
            sprintf(buf,
	             "You recieve %d gold and %d questpoints for winning the war!\n\r", reward, qreward );
			send_to_char(buf, ch);
	    end_war();
	    return;
	}
	break;
    }
    return;
}

bool is_safe_war(CHAR_DATA * ch, CHAR_DATA * wch)
{
    if (war_info.iswar == WAR_OFF)
	return FALSE;

    if (!IS_IN_WAR(ch) || !IS_IN_WAR(wch))
	return FALSE;

    if (war_info.wartype == 3)
	return FALSE;

    if (war_info.wartype == 1 && ch->race == wch->race)
	return TRUE;

    if (war_info.wartype == 2 && ch->klass == wch->klass)
	return TRUE;

    if (war_info.wartype == 4 && is_same_clan(ch, wch))
	return TRUE;

    return FALSE;
}

void war_talk(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    
    if (argument[0] == '\0')
    {
	send_to_char
	  ("Wartalk what?\n\r",
	   ch);
	return;
    }
    sprintf(buf, "(WarTalk) You drum: %s\n\r", argument);
	send_to_char(buf, ch);

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *victim;

	if (d->connected == CON_PLAYING && (victim = d->character) != ch &&
	    !IS_SET(victim->comm, COMM_QUIET) && IS_IN_WAR(victim))
	{
	    sprintf(buf, "(WarTalk) %s drums: %s\n\r",
		     PERS(ch, victim), argument);
		send_to_char(buf, victim);
    }
    }
    return;
}

void extract_war ( CHAR_DATA * ch )
{
    char buf[MAX_STRING_LENGTH];

    if ( war_info.iswar != WAR_OFF && IS_SET ( ch->act, PLR_WAR ) )
    {
        REMOVE_BIT ( ch->act, PLR_WAR );
        war_info.inwar--;
        if ( war_info.iswar == WAR_RUNNING )
        {
            if ( war_info.inwar == 0 || war_info.inwar == 1 )
            {
                war_channel ( ch, "$n has left. War over." );
                end_war (  );
            }
            if ( abort_race_war (  ) )
            {
                war_channel ( ch, "$n has left. War over." );
                end_war (  );
            }
            else if ( abort_class_war (  ) )
            {
                war_channel ( ch, "$n has left. War over." );
                end_war (  );
            }
            else if ( abort_clan_war (  ) )
            {
                war_channel ( ch, "$n has left. War over." );
                end_war (  );
            }
            else
            {
                sprintf ( buf, "$n has left. %d players in the war.",
                           war_info.inwar );
                war_channel(ch, buf);
            }
        }
       
        do_function( ch, &do_recall, "" );
    }
}

void war_channel(CHAR_DATA *ch, char *message)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *dch;
    char buf[MAX_INPUT_LENGTH];

    for(d = descriptor_list; d != NULL; d = d->next)
    {
        if(d->connected != CON_PLAYING)
            continue;

        if((dch = d->character) == NULL)
            continue;

        sprintf(buf, "WAR: %s", message);

        if(ch) /* don't use $N only $n in message */
            act(buf, ch, NULL, dch, TO_VICT);
        else
        {
            strcat(buf, "\n\r");
            send_to_char(buf, dch);
        }
    }
}
