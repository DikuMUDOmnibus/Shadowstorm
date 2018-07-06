/**************************************************************************
*  Automated GLobal Quest Code                                            *
*  Telnet : <akbearsden.com:3778>                                      *
*  E-Mail : <dlmud@dlmud.com     >                                  *
*  Website: <http://dlmud.akbearsden.com>                         *
*                                                                         *
*  Provides automated, saveable global quests for a level range.          *
*                                                                         *
*  This version has been redone and attempted to fit stock rom.           *
*  Some things you will need to change to fit your mud                    *
***************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "const.h"

typedef struct gquest_hist GQUEST_HIST;

struct gquest_hist
{
    GQUEST_HIST *next;
    char *short_descr;
    char *text;
};


GQUEST_HIST *gqhist_first = NULL;


void gquest_channel(CHAR_DATA *ch, char *message)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *dch;
    char buf[MAX_INPUT_LENGTH];

    for(d = descriptor_list; d != NULL; d = d->next)
    {
        if(d->connected != CON_PLAYING)
            continue;

        if((dch = d->character) == NULL || dch == ch)
            continue;

        sprintf(buf, "GQUEST: %s",message);

        if(ch) /* don't use $N only $n in message */
            act(buf, ch, NULL, dch, TO_VICT);
        else
        {
            send_to_char(buf, dch);
            send_to_char("\n\r", dch);
        }
    }
}

bool save_gquest_data(void)
{
    FILE *fp = NULL;
    char buf[MIL]; 
    int i;

    sprintf(buf, "../area/%s", GQUEST_FILE );

    fp = file_open( buf, "w" );

    if(!fp)
    {	
       logf2("Bug with saving gquest file.");
       return FALSE;
    }

    fprintf(fp, "#GQUESTDATA\n\n");

    fprintf(fp, "Mobs    %d ", gquest_info.mob_count);

    for (i = 0; i < gquest_info.mob_count; i++)

    fprintf(fp, "%d ", gquest_info.mobs[i]);
    fprintf(fp, "\n");
    fprintf(fp, "Who     %s~\n", gquest_info.who);
    fprintf(fp, "Timer   %d\n",
	    (gquest_info.timer >
	     0) ? (gquest_info.timer + 1) : gquest_info.timer);
    fprintf(fp, "Involv  %d\n", gquest_info.involved);
    fprintf(fp, "Qpoints %d\n", gquest_info.qpoints);
    fprintf(fp, "Gold    %d\n", gquest_info.gold);
    fprintf(fp, "MinLev  %d\n", gquest_info.minlevel);
    fprintf(fp, "MaxLev  %d\n", gquest_info.maxlevel);
    fprintf(fp, "Running %d\n", gquest_info.running);
    fprintf(fp, "Next    %d\n",
	    (gquest_info.next >
	     0) ? (gquest_info.next + 1) : gquest_info.next);

    fprintf(fp, "#0\n");
    fprintf(fp, "\nEnd\n");
    file_close(fp);
    return TRUE;
}

#if	defined(KEY)
#undef KEY
#endif

#define	KEY( literal, field, value )                                    \
if ( !str_cmp( word, literal ) )        \
{										\
	field  = value;                     \
	fMatch = TRUE;                      \
	break;                              \
}

#if	defined(KEYS)
#undef KEYS
#endif

#define	KEYS( literal, field, value )                                   \
if ( !str_cmp( word, literal ) )        \
{										\
	free_string(field);                 \
	field  = value;                     \
	fMatch = TRUE;                      \
	break;                              \
}

bool load_gquest_data(void)
{
    FILE *fp;
    const char *word;
    bool fMatch = FALSE;

    end_gquest();

    fp = file_open( GQUEST_FILE, "r" );

    if (str_cmp(fread_word(fp), "#GQUESTDATA"))
    {
	sprintf(log_buf, "load_gquest_data: Invalid gquest data file (%s).\n\r",
	     GQUEST_FILE);
	bug(log_buf, 0);
	file_close(fp);
	return FALSE;
    }

    for (;;)
    {
	word = feof(fp) ? "End" : fread_word(fp);

	if (!str_cmp(word, "End"))
	{
	    fMatch = TRUE;
	    break;
	}

	fMatch = FALSE;

	switch (UPPER(word[0]))
	{
	case '#':		// comment identifier 
	    fMatch = TRUE;
	    fread_to_eol(fp);
	    break;

	case 'G':
	    KEY("Gold", gquest_info.gold, fread_number(fp));
	    break;

	case 'I':
	    KEY("Involv", gquest_info.involved, fread_number(fp));
	    break;

	case 'M':
	    KEY("MinLev", gquest_info.minlevel, fread_number(fp));
	    KEY("MaxLev", gquest_info.maxlevel, fread_number(fp));
	    if (!str_cmp(word, "Mobs"))
	    {
		int i;

		gquest_info.mob_count = fread_number(fp);

		for (i = 0; i < gquest_info.mob_count; i++)
		    gquest_info.mobs[i] = fread_number(fp);

		fMatch = TRUE;
		break;
	    }
	    break;

	case 'N':
	    KEY("Next", gquest_info.next, fread_number(fp));
	    break;

	case 'Q':
	    KEY("Qpoints", gquest_info.qpoints, fread_number(fp));
	    break;

	case 'R':
	    KEY("Running", gquest_info.running, fread_number(fp));
	    break;

	case 'T':
	    KEY("Timer", gquest_info.timer, fread_number(fp));
	    break;

	case 'W':
	    KEYS("Who", gquest_info.who, fread_string(fp));
	    break;
	}

	if (!fMatch)
	{
	    sprintf(log_buf, "load_gquest_data: Invalid Key: %s", word);
        bug(log_buf, 0);
        fread_to_eol(fp);
	}
    }

    file_close(fp);
    return TRUE;
}

bool start_gquest(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *registar = NULL;
    int mobs, blevel, elevel, cost;

    for (registar = ch->in_room->people; registar != NULL;
	 registar = registar->next_in_room)
    {
	if (!IS_NPC(registar))
	    continue;
	if (registar->spec_fun == spec_lookup("spec_registar"))
	    break;
    }

    if (!IS_IMMORTAL(ch) &&
	(registar == NULL ||
	 registar->spec_fun != spec_lookup("spec_registar")))
    {
	send_to_char("You can't do that here.\n\r", ch);
	return FALSE;
    }

    if (!IS_IMMORTAL(ch) && registar->fighting != NULL)
    {
	send_to_char("Wait until the fighting stops.\n\r", ch);
	return FALSE;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
	send_to_char("Syntax: gquest start <min level> <max level> <#mobs>\n\r", ch);
	return FALSE;
    }

    blevel = atoi(arg1);
    elevel = atoi(arg2);
    mobs = atoi(arg3);

    if (blevel <= 0 || blevel > MAX_LEVEL)
    {
	sprintf(buf, "Level must be between 1 and %d.\n\r", MAX_LEVEL);
	send_to_char( buf,ch);
	return FALSE;
    }

    if (blevel <= 0 || elevel > MAX_LEVEL)
    {
	sprintf(buf, "Level must be between 1 and %d.\n\r", MAX_LEVEL);
	send_to_char(buf,ch);
	return FALSE;
    }

    if (elevel <= blevel)
    {
	send_to_char("Max level must be greater than the min level.\n\r", ch);
	return FALSE;
    }

    if (elevel - blevel < 10)
    {
	send_to_char("Level difference must 10 levels or higher.\n\r", ch);
	return FALSE;
    }

    if (mobs < 5 || mobs >= MAX_GQUEST_MOB)
    {
	sprintf(buf, "Number of mobs must be between 5 and %d.\n\r", MAX_GQUEST_MOB - 1);
	send_to_char(buf,ch);
	return FALSE;
    }

    if (gquest_info.running != GQUEST_OFF)
    {
	send_to_char("There is already a global quest running!\n\r", ch);
	return FALSE;
    }

    cost = 5 + (mobs / 5);

    if (!IS_IMMORTAL(ch))
    {
	if (ch->pcdata->trivia < cost)
	{
		sprintf( buf, "$N tells you 'It costs %d Trivia Points to start a global quest with %d mobs.'",
		     cost, mobs);
		act(buf, ch, NULL, registar, TO_CHAR);
	    return FALSE;
	}

	else
	{
	    sprintf(buf, "$N tells you '%d mobs have cost you %d trivia points.'",
		     mobs, cost);
		act(buf, ch, NULL, registar, TO_CHAR);
	    ch->pcdata->trivia -= cost;
	}

    send_to_char("Oops you can't start a global quest!\n\r", ch);
    return FALSE;

    }

    gquest_info.running = GQUEST_WAITING;
    gquest_info.minlevel = blevel;
    gquest_info.maxlevel = elevel;
    gquest_info.mob_count = mobs;
    free_string(gquest_info.who);
    gquest_info.who = str_dup(ch->name);

    if (!generate_gquest(ch))
    {
	if (!IS_IMMORTAL(ch))
	{
	    sprintf(buf, "Failed to start Gquest, you are being reimbursed %d TP.\n\r", cost);
            send_to_char( buf, ch );
	    ch->pcdata->trivia += cost;
	}

	else
        send_to_char("Failed to start a gquest, not enogh mobs found.\n\r", ch);
	return FALSE;
    }

    return TRUE;
}

void auto_gquest(void)
{
    CHAR_DATA *wch = NULL, *registar = NULL;
    int middle = LEVEL_HERO / 2, maxlvl = 0;
    int minlvl = MAX_LEVEL, count = 0, lbonus = 0, half = 0;

    if (gquest_info.running != GQUEST_OFF)
	return;

    for (wch = char_list; wch != NULL; wch = wch->next)
    {
	if (!IS_NPC(wch) && !IS_IMMORTAL(wch))
	{
	    count++;
	    maxlvl = UMAX(maxlvl, wch->level);
	    minlvl = UMIN(minlvl, wch->level);
	}
    }

    if (count < 1)
    {
	end_gquest();
	return;
    }

    lbonus = number_range(5, 10);
    minlvl = UMAX(1, minlvl - lbonus);
    maxlvl = UMIN(LEVEL_HERO, maxlvl + lbonus);
    half = ((maxlvl - minlvl) / 2);
    middle = URANGE(minlvl, maxlvl - half, maxlvl);
    minlvl = number_range(minlvl, middle - lbonus);
    maxlvl = number_range(middle + lbonus, maxlvl);

    for(registar = char_list; registar != NULL; registar = registar->next)
    {
        if(!IS_NPC(registar))
            continue;

        if(registar->pIndexData->vnum == MOB_VNUM_REGISTAR)
            break;
    }

    gquest_info.running = GQUEST_WAITING;
    gquest_info.mob_count = number_range(5, MAX_GQUEST_MOB - lbonus);
    gquest_info.minlevel = UMAX(1, minlvl);
    gquest_info.maxlevel = UMIN(LEVEL_HERO, maxlvl);
    free_string(gquest_info.who);
    gquest_info.who =
      !registar ? str_dup("AutoQuest (tm)") : str_dup(registar->short_descr);
    generate_gquest(registar);
    return;
}

void post_gquest(CHAR_DATA * ch)
{
    BUFFER *output;
    CHAR_DATA *wch;
    MOB_INDEX_DATA *mob;
    int i;
    GQUEST_HIST *hist;
    char *strtime;
    char shortd[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    
    if (gquest_info.running == GQUEST_OFF || gquest_info.involved == 0)
	return;

    hist = (GQUEST_HIST *)alloc_mem(sizeof(hist));

    strtime = ctime(&current_time);
    strtime[strlen(strtime) - 1] = '\0';

    sprintf(shortd, "%24s %3d %3d %4d %12s\n\r", strtime,
	    gquest_info.minlevel, gquest_info.maxlevel, gquest_info.mob_count,
	    ch->name);

    hist->short_descr = str_dup(shortd);

    output = new_buf();

    sprintf(buf, "GLOBAL QUEST INFO\n\r-----------------\n\r");
	add_buf(output, buf);

    sprintf(buf, "Started by  : %s\n\r", gquest_info.who[0] == '\0' ? "Unknown" : gquest_info.who);
	add_buf(output, buf);

    sprintf(buf, "Levels      : %d - %d\n\r", gquest_info.minlevel, gquest_info.maxlevel);
	add_buf(output, buf);

    sprintf(buf, "Those Playing\n\r-------------\n\r");
	add_buf(output, buf);

    for (wch = char_list; wch != NULL; wch = wch->next)
	if (!IS_NPC(ch) && ON_GQUEST(wch) && count_gqmobs(wch) != gquest_info.mob_count)
	    sprintf(buf, "%s [%d mobs left]\n\r", wch->name, gquest_info.mob_count - count_gqmobs(wch));
	add_buf(output, buf);

    sprintf(buf, "%s won the GQuest.\n\r", ch->name);
	add_buf(output, buf);

    sprintf(buf, "Quest Rewards\n\r-------------\n\r");
	add_buf(output, buf);

    sprintf(buf, "Qp Reward   : %d + 3 QPs for each target.\n\r", gquest_info.qpoints);
	add_buf(output, buf);

    sprintf(buf, "Gold Reward : %d\n\r", gquest_info.gold);
	add_buf(output, buf);
    sprintf(buf, "Quest Targets\n\r-------------\n\r");
	add_buf(output, buf);

    for (i = 0; i < gquest_info.mob_count; i++)
    {
	if ((mob = get_mob_index(gquest_info.mobs[i])) != NULL)
	{
	    sprintf(buf, "%2d) [%-20s] %-30s (level %3d)\n\r", i + 1, mob->area->name, mob->short_descr, mob->level);
            add_buf(output, buf);
	}
    }

    hist->text = str_dup(buf_string(output));
    hist->next = gqhist_first;
    gqhist_first = hist;
    free_buf(output);
    return;
}

void do_gquest(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], buf[MSL];
    CHAR_DATA *wch;
    MOB_INDEX_DATA *mob;
    int i = 0;

    if (IS_NPC(ch))
    {
	send_to_char("Your the victim not the player.\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0')
    {
	send_to_char("Syntax: gquest join     - join a global quest\n\r"
	       "        gquest quit     - quit the global quest\n\r"
	       "        gquest info     - show global quest info\n\r"
	       "        gquest time     - show global quest time\n\r"
	       "        gquest check    - show what targets you have left\n\r"
	       "        gquest progress - show progress of other players\n\r"
	       "        gquest complete - completes the current quest\n\r"
	       "        gquest hist     - shows gquest history since last reboot\n\r"
	       "        gquest start    - starts a gquest\n\r", ch);

	if (IS_IMMORTAL(ch))
	{
	    send_to_char("        gquest end      - ends the gquest (IMM)\n\r", ch);
	    send_to_char("        gquest next     - sets time to next gquest.\n\r", ch);
	}
	
	return;
    }

    else if (!str_prefix(arg1, "start"))
    {
	start_gquest(ch, argument);
	return;
    }
    else if (!str_prefix(arg1, "next") && IS_IMMORTAL(ch))
    {
	if (gquest_info.running != GQUEST_OFF)
	{
	    send_to_char("Not while a gquest is running.\n\r", ch);
	    return;
	}

	i = is_number(argument) ? atoi(argument) : number_range(30, 100);
	gquest_info.next = i;
	printf_to_char(ch, "The next gquest will start in %dminutes.\n\r", gquest_info.next);
	return;
    }

    else if (!str_prefix(arg1, "hist"))
    {
	GQUEST_HIST *hist;
	int count = 0;

	if (!gqhist_first)
	{
	    send_to_char("No global quests completed yet.\n\r", ch);
	    return;
	}

	if (argument[0] == '\0')
	{
	    BUFFER *output = new_buf();

	    add_buf(output,
		    "Num Finished Time            Levels  Mobs Completed by\n\r"
		    "--- ------------------------ ------- ---- ------------\n\r");

	    for (hist = gqhist_first; hist != NULL; hist = hist->next)
	    {
		sprintf(buf, "%2d) ", ++count);
		add_buf(output, buf);
		add_buf(output, hist->short_descr);
	    }

	    add_buf(output, "Type 'gquest hist #' to view details.\n\r");
	    page_to_char(buf_string(output), ch);
	    free_buf(output);
	}

	else
	{
	    bool found = FALSE;

	    if (!is_number(argument))
	    {
		send_to_char("Syntax: gquest hist #\n\r", ch);
		return;
	    }

	    for (hist = gqhist_first; hist != NULL; hist = hist->next)
            {
		
                if (++count == atoi(argument))
		{
		    send_to_char(hist->text, ch);
		    found = TRUE;
		}
            }

	    if (!found)
		send_to_char("History data not found.\n\r", ch);
	}

	return;
    }

    else if (gquest_info.running == GQUEST_OFF)
    {
	printf_to_char(ch, "There is no global quest running.  The next Gquest will start in %d minutes.\n\r", gquest_info.next);
	return;
    }

    else if (!str_prefix(arg1, "end") && IS_IMMORTAL(ch))
    {
	end_gquest();
	sprintf(buf, "You end the global quest. Next autoquest in %d minutes.\n\r", gquest_info.next);
	send_to_char(buf,ch);
	sprintf(buf, "$n has ended the global quest. Next gquest in %d minutes.", gquest_info.next);
	return;
    }

    else if (!str_prefix(arg1, "join"))
    {

	if (IS_QUESTOR(ch))
	{
	    send_to_char("Why don't you finish your other quest first.\n\r", ch);
	    return;
	}

	if (ON_GQUEST(ch))
	{
	    send_to_char("Your allready in the global quest.\n\r", ch);
	    return;
	}

	if (gquest_info.minlevel > ch->level ||
	    gquest_info.maxlevel < ch->level)
	{
	    send_to_char("This gquest is not in your level range.\n\r", ch);
	    return;
	}

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_WAR))
	{
	    sprintf(buf, "Your %s combat right now.\n\r", war_info.iswar == WAR_WAITING ? "waiting for" : "in");
   	    send_to_char(buf, ch);
	    return;
	}

	if (count_gqmobs(ch) == gquest_info.mob_count)
	{
	    send_to_char("You have already quit this gquest.\n\r", ch);
	    return;
	}

	for (i = 0; i < gquest_info.mob_count; i++)
        {
	    ch->pcdata->gq_mobs[i] = gquest_info.mobs[i];
   	    SET_BIT(ch->act, PLR_GQUEST);
	    gquest_info.involved++;
        }
	   
        send_to_char("Your global quest flag is now on. Use 'gquest info' to see the quest(s).\n\r", ch);
   	gquest_channel(ch, "$n has joined the global quest.");
        return;
    }

    else if (!str_prefix(arg1, "quit"))
    {
	if (!ON_GQUEST(ch))
	{
	    send_to_char("Your not in a global quest.\n\r", ch);
	    return;
	}

	reset_gqmob(ch, -1);
	REMOVE_BIT(ch->act, PLR_GQUEST);
	gquest_info.involved--;
	send_to_char("Your global quest flag is now off. Sorry you couldn't complete it.\n\r",ch);
	gquest_channel(ch, "$n has quit the global quest, what a sore loser.");
	return;
    }

    else if (!str_prefix(arg1, "info"))
    {
	send_to_char("[ GLOBAL QUEST INFO ]",ch);
	sprintf(buf, "Started by  : %s\n\r", gquest_info.who[0] == '\0' ? "Unknown" : gquest_info.who);
	send_to_char(buf,ch);
	sprintf(buf, "Playing     : %d player%s.\n\r", gquest_info.involved, gquest_info.involved == 1 ? "" : "s");
	send_to_char(buf,ch);
	sprintf(buf, "Levels      : %d - %d\n\r", gquest_info.minlevel, gquest_info.maxlevel);
	send_to_char(buf,ch);
	sprintf(buf, "Status      : %s for %d minute%s.\n\r", gquest_info.running == GQUEST_WAITING ? "Waiting" : "Running", gquest_info.timer, gquest_info.timer == 1 ? "" : "s");
	send_to_char(buf,ch);
	send_to_char("[ Quest Rewards ]",ch);
	send_to_char(buf,ch); 
	sprintf(buf, "Qp Reward   : %d\n\r", gquest_info.qpoints);
	send_to_char(buf,ch);
	sprintf(buf, "Gold Reward : %d\n\r", gquest_info.gold);
	send_to_char(buf,ch);
	send_to_char("[ Quest Targets ]",ch);
	send_to_char(buf,ch);

	for (i = 0; i < gquest_info.mob_count; i++)
	{
	    if ((mob = get_mob_index(gquest_info.mobs[i])) != NULL)
	    {
		sprintf(buf, "%2d) [%-20s] %-30s (level %3d)\n\r", i + 1, mob->area->name, mob->short_descr, mob->level);
       	        send_to_char(buf,ch);
	    }
	}

	return;
    }

    else if (!str_prefix(arg1, "time"))
    {
	if (gquest_info.next > 0)
	    sprintf(buf, "THe next Global Quest will start in %d minute%s.\n\r", gquest_info.next, gquest_info.next == 1 ? "" : "s");

	else
	    sprintf(buf, "The Global Quest is %s for %d minute%s.\n\r", gquest_info.running == GQUEST_WAITING ? "Waiting" : "Running", gquest_info.timer, gquest_info.timer == 1 ? "" : "s");
            send_to_char(buf, ch);
	 
        return;
    }

    else if (!str_prefix(arg1, "progress"))
    {

	if (gquest_info.running == GQUEST_WAITING)
	{
	    send_to_char("The global quest hasn't started yet.\n\r", ch);
	    return;
	}

	for (wch = char_list; wch != NULL; wch = wch->next)
	{
	    if (!IS_NPC(wch) && ON_GQUEST(wch) && wch != ch)
	    {
		sprintf(buf, "%-12s has %d of %d mobs left.\n\r", wch->name, gquest_info.mob_count - count_gqmobs(wch), gquest_info.mob_count);
                send_to_char(buf,ch);
	    }
	}

	return;
    }

    else if (!str_prefix(arg1, "check"))
    {
        
	if (IS_IMMORTAL(ch) && argument[0] != '\0')
	{
	    if ((wch = get_char_world(ch, argument)) == NULL || IS_NPC(wch))
	    {
		send_to_char("That player is not here.\n\r", ch);
		return;
	    }
	}

	else
	    wch = ch;

	if (!ON_GQUEST(wch))
	{
	    sprintf(buf, "%s aren't on a global quest.\n\r", wch == ch ? "You" : wch->name);
	    send_to_char(buf,ch);
	    return;
	}

	sprintf(buf, "[ %s have %d of %d mobs left ]\n\r",
		wch == ch ? "You" : wch->name,
		gquest_info.mob_count - count_gqmobs(wch),
		gquest_info.mob_count);
	send_to_char(buf,ch);

	for (i = 0; i < gquest_info.mob_count; i++)
	{
	    if ((mob = get_mob_index(wch->pcdata->gq_mobs[i])) != NULL)
	    {
		sprintf(buf, "%2d) [%-20s] %-30s (level %3d)\n\r", i + 1, mob->area->name, mob->short_descr, mob->level);
	        send_to_char(buf,ch);
	    }
	}

	return;
    }

    else if (!str_prefix(arg1, "complete"))
    {
	if (!ON_GQUEST(ch))
	{
	    send_to_char("Your not in a global quest.\n\r", ch);
	    return;
	}

	if (count_gqmobs(ch) != gquest_info.mob_count)
	{
	    sprintf(buf, "You haven't finished just yet, theres still %d mobs to kill.\n\r", gquest_info.mob_count - count_gqmobs(ch));
            send_to_char(buf,ch);
	    return;
	}

	send_to_char("YES! You have completed the global quest.\n\r", ch);
	ch->pcdata->questpoints += gquest_info.qpoints;
	ch->gold += gquest_info.gold;
	post_gquest(ch);
	sprintf(buf, "You receive %d gold and %d quest points.\n\r", gquest_info.gold, gquest_info.qpoints);
	send_to_char(buf,ch);
	end_gquest();
        sprintf(buf, "$n has completed the global quest, next gquest in %d minutes.", gquest_info.next);
	gquest_channel(ch,buf);
	return;
    }

    else
	do_gquest(ch, "");
    return;
}

void end_gquest(void)
{
    CHAR_DATA *wch;

    gquest_info.running = GQUEST_OFF;
    free_string(gquest_info.who);
    gquest_info.who = str_dup("");
    gquest_info.mob_count = 0;
    gquest_info.timer = 0;
    gquest_info.involved = 0;
    gquest_info.qpoints = 0;
    gquest_info.gold = 0;
    gquest_info.minlevel = 0;
    gquest_info.maxlevel = 0;
    gquest_info.next = number_range(70, 100);
    reset_gqmob(NULL, 0);

    for (wch = char_list; wch != NULL; wch = wch->next)
    {
	if (!IS_NPC(wch) && IS_SET(wch->act, PLR_GQUEST))
	{
	    REMOVE_BIT(wch->act, PLR_GQUEST);
	    reset_gqmob(wch, 0);
	}
    }
}

void gquest_update ( void )
{
    char buf[MSL];

    if ( gquest_info.running == GQUEST_OFF )
    {
        if ( --gquest_info.next <= 0 )
            auto_gquest (  );
    }

    else if ( gquest_info.running == GQUEST_WAITING )
    {

        gquest_info.timer--;

        if ( gquest_info.timer > 0 )
        {
            sprintf ( buf, "%d minute%s left to join the global quest. (Levels %d - %d)",
                       gquest_info.timer, gquest_info.timer == 1 ? "" : "s",
                       gquest_info.minlevel, gquest_info.maxlevel );
                gquest_channel(NULL, buf);
        }

        else
        {
            if ( gquest_info.involved == 0 )
            {
                end_gquest ( );
                sprintf ( buf, "Not enough people for the global quest. The next quest will start in %d minutes.",
                           gquest_info.next );
                gquest_channel(NULL, buf);
            }

            else
            {
                gquest_info.timer =
                    number_range ( 4 * gquest_info.mob_count,
                                   6 * gquest_info.mob_count );
                gquest_info.running = GQUEST_RUNNING;
                sprintf ( buf, "The Global Quest begins! You have %d minutes to complete the task!",
                           gquest_info.timer );
                gquest_channel(NULL, buf);
            }
        }
    }

    else if ( gquest_info.running == GQUEST_RUNNING )
    {
        if ( gquest_info.involved == 0 )
        {
            end_gquest ( );
            sprintf ( buf, "No one left in the Global Quest, next quest will start in %d minutes.",
                       gquest_info.next );
                 gquest_channel(NULL, buf);
           return;
        }

        switch ( gquest_info.timer )
        {
            case 0:
                end_gquest (  );
                sprintf ( buf, "Time has run out on the Global Quest, next quest will start in %d minutes.",
                           gquest_info.next );
                 gquest_channel(NULL, buf);
               return;
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 10:
            case 15:
                sprintf( buf, "%d minute%s remaining in the global quest.",
                           gquest_info.timer,
                           gquest_info.timer > 1 ? "s" : "" );
                gquest_channel(NULL, buf);
            default:
                gquest_info.timer--;
                break;
        }
        return;
    }
}

bool generate_gquest(CHAR_DATA * who)
{
    CHAR_DATA *victim = NULL;
    long vnums[top_mob_index];
    size_t mob_count, randm;
    char buf[MAX_STRING_LENGTH];
    int i;

    reset_gqmob(NULL, 0);

    mob_count = 0;
    for (victim = char_list; victim; victim = victim->next)
    {
	if (!IS_NPC(victim))
	{
	    REMOVE_BIT(victim->act, PLR_GQUEST);
	    reset_gqmob(victim, 0);
	    continue;
	}
	else if (!IS_NPC(victim)
		 || victim->level > (gquest_info.maxlevel + 10)
		 || victim->level < (gquest_info.minlevel - 10)
		 || (victim->pIndexData == NULL
		     || victim->in_room == NULL
		     || victim->pIndexData->pShop != NULL)
		 || victim->pIndexData->vnum < 100
		 || IS_SET(victim->in_room->room_flags, ROOM_PET_SHOP)
		 || victim->in_room->clan
		 || IS_SET(victim->imm_flags, IMM_WEAPON | IMM_MAGIC)
		 || IS_SET(victim->act,
			   ACT_TRAIN | ACT_PRACTICE | ACT_IS_HEALER | ACT_PET | ACT_GAIN)
		 || IS_SET(victim->affected_by, AFF_CHARM) ||
		 (IS_SET(victim->act, ACT_SENTINEL) &&
		  IS_SET(victim->in_room->room_flags,
			 ROOM_PRIVATE | ROOM_SOLITARY | ROOM_SAFE)))
	    continue;
	vnums[mob_count] = victim->pIndexData->vnum;
	mob_count++;

	if (mob_count >= (size_t)top_mob_index)
	    break;
    }

    if (mob_count < 5)
    {
	end_gquest();
	return FALSE;
    }

    else if (mob_count < (size_t)gquest_info.mob_count)
    {
	gquest_info.mob_count = mob_count;
    }

    for (i = 0; i < gquest_info.mob_count; i++)
    {
	randm = number_range(0, mob_count - 1);
	while (!is_random_gqmob(vnums[randm]))
	    randm = number_range(0, mob_count - 1);

	gquest_info.mobs[i] = vnums[randm];
    }

    gquest_info.qpoints = number_range(15, 30) * gquest_info.mob_count;
    gquest_info.gold = number_range(100, 150) * gquest_info.mob_count;
    gquest_info.timer = 3;
    gquest_info.next = 0;
    sprintf(buf, "%s Global Quest for levels %d to %d%s.  Type 'GQUEST INFO' to see the quest.",
	     !who ? "A" : "$n announces a", gquest_info.minlevel,
	     gquest_info.maxlevel, !who ? " has started" : "");
    gquest_channel(who, buf);
    sprintf(buf, "You announce a Global Quest for levels %d to %d with %d targets.\n\r", gquest_info.minlevel, gquest_info.maxlevel, gquest_info.mob_count);
    send_to_char(buf, who);
    return TRUE;
}

int is_gqmob(CHAR_DATA * ch, int vnum)
{
    int i;

    if (gquest_info.running == GQUEST_OFF)
	return -1;

    for (i = 0; i < gquest_info.mob_count; i++)
    {
	if (ch && !IS_NPC(ch))
	{
	    if (ch->pcdata->gq_mobs[i] == vnum)
		return i;
	    else
		continue;
	}
	else
	{
	    if (gquest_info.mobs[i] == vnum)
		return i;
	    else
		continue;
	}
    }

    return -1;
}

int count_gqmobs(CHAR_DATA * ch)
{
    int i, count = 0;

    if (IS_NPC(ch))
	return 0;

    for (i = 0; i < gquest_info.mob_count; i++)
	if (ch->pcdata->gq_mobs[i] == -1)
	    count++;

    return count;
}

void reset_gqmob(CHAR_DATA * ch, int value)
{
    int i;

    for (i = 0; i < MAX_GQUEST_MOB; i++)
    {
	if (ch && !IS_NPC(ch))
	    ch->pcdata->gq_mobs[i] = value;
	else
	    gquest_info.mobs[i] = value;
    }
}

bool is_random_gqmob(int vnum)
{
    int i;

    if (get_mob_index(vnum) == NULL)
	return FALSE;

    for (i = 0; i < gquest_info.mob_count; i++)
	if (gquest_info.mobs[i] == vnum)
	    return FALSE;

    return TRUE;
}



