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
*       ROM 2.4 is copyright 1993-1998 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@hypercube.org)                            *
*           Gabrielle Taylor (gtaylor@hypercube.org)                       *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

/********************************************************************************
 * Stat List code copyright 1999-2001                                           *
 * Markanth : dlmud@dlmud.com                                                   *
 * Devil's Lament : dlmud.com port 3778                                         *
 * Web Page : http://www.dlmud.com                                              *
 *                                                                              *
 * All I ask in return is that you give me credit on your mud somewhere         *
 * or email me if you use it.                                                   *
 ********************************************************************************/

/* Improvements by Synon for ShadowStorm */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "recycle.h"
#include "interp.h"
#include "const.h"

STAT_DATA *stat_list;
void show_stats args((CHAR_DATA * ch, int type));

void save_statlist(void)
{
    STAT_DATA *pstat;
    FILE *fp;
    bool found = FALSE;
    int i;

    if ((fp = file_open(STAT_FILE, "w")) == NULL)
    {
	perror(STAT_FILE);
    }

    fprintf(fp, "#%d\n", MAX_GAMESTAT);

    for (pstat = stat_list; pstat != NULL; pstat = pstat->next)
    {
	found = TRUE;
	fprintf(fp, "%s ", pstat->name);
	for (i = 0; i < MAX_GAMESTAT; i++)
	    fprintf(fp, "%ld ", pstat->gamestat[i]);
	fprintf(fp, "\n");
    }

    file_close(fp);
    if (!found)
	unlink(STAT_FILE);
}

void load_statlist(void)
{
   FILE *fp;
   STAT_DATA *stat_last;
   int i,
   maxStat = 0;

   if ( file_exists( STAT_FILE ) )
   {
       fp = file_open(STAT_FILE, "r");

       fscanf(fp, "#%d\n", &maxStat);

       stat_last = NULL;

       for (;;)
       {
	    STAT_DATA *pstat;

	    if (feof(fp))
	    {
	        fclose(fp);
	        return;
	    }

	    pstat = new_stat_data();
	    pstat->name = str_dup(fread_word(fp));
	    for (i = 0; i < maxStat; i++)
	    pstat->gamestat[i] = fread_number(fp);
	    fread_to_eol(fp);

	    if (stat_list == NULL)
	    stat_list = pstat;
	    else
	    stat_last->next = pstat;
	    stat_last = pstat;
       }
   }
}

void update_statlist(CHAR_DATA * ch, bool del)
{
    STAT_DATA *prev;
    STAT_DATA *curr;
    int i;

    if (IS_NPC(ch) || IS_IMMORTAL(ch))
	return;

    prev = NULL;

    for (curr = stat_list; curr != NULL; prev = curr, curr = curr->next)
    {
	if (!str_cmp(ch->name, curr->name))
	{
	    if (prev == NULL)
		stat_list = stat_list->next;
	    else
		prev->next = curr->next;

	    free_stat_data(curr);
	    save_statlist();
	}
    }

    if (del || IS_IMMORTAL(ch) || IS_NPC(ch))
    {
	return;
    }

    curr = new_stat_data();
    curr->name = str_dup(ch->name);
    for (i = 0; i < MAX_GAMESTAT; i++)
        curr->gamestat[i] = ch->pcdata->gamestat[i];

    curr->next = stat_list;
    stat_list = curr;
    save_statlist();
    return;
}

void do_statlist(CHAR_DATA * ch, char *argument)
{
    int choice;
    char buf[MAX_STRING_LENGTH],
    arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || !is_number( arg ) )
    {
	send_to_char("             {GOPTIONS AVAILABLE{x\n\r", ch);
        send_to_char("{cO-------------------------------------------O{x\n\r",ch);
	send_to_char("{c|{x     1) Ranking of Player Kills            {c|{x\n\r", ch);
	send_to_char("{c|{x     2) Ranking of Player Deaths           {c|{x\n\r", ch);
	send_to_char("{c|{x     3) Ranking of Mob Kills               {c|{x\n\r", ch);
	send_to_char("{c|{x     4) Ranking of Mob Deaths              {c|{x\n\r", ch);
        if (!IS_IMMORTAL(ch))
        send_to_char("{cO-------------------------------------------O{x\n\r",ch);

        if(IS_IMMORTAL(ch))
        {
	send_to_char("{c|{x     5) <name> - Deletes from statlist     {c|{x\n\r", ch);
        send_to_char("{cO-------------------------------------------O{x\n\r",ch);
        }
	return;
    }

    choice = atoi(arg);

    if ( ( choice == 5 || !str_cmp( arg, "delete" ) ) && IS_IMMORTAL(ch) ) 
    {
	STAT_DATA *prev = NULL;
	STAT_DATA *curr = NULL;
	bool found = FALSE;

	for (curr = stat_list; curr != NULL; prev = curr, curr = curr->next)
	{
	    if (!str_cmp(argument, curr->name))
	    {
		if (prev == NULL)
		    stat_list = stat_list->next;
		else
		    prev->next = curr->next;

		free_stat_data(curr);
		save_statlist();
		found = TRUE;
	    }
	}

	if (!found)
        {

	    sprintf(buf, "Error deleting %s.\n\r", argument);
            send_to_char(buf, ch);
        }
    } 

    else if (choice == 1 || !str_prefix(arg, "pkills"))
         show_stats(ch, PK_KILLS);
    else if (choice == 3 || !str_prefix(arg, "mkills"))
	show_stats(ch, MOB_KILLS);
    else if (choice == 2 || !str_prefix(arg, "pdeaths"))
	show_stats(ch, PK_DEATHS);
    else if (choice == 4 || !str_prefix(arg, "mdeaths"))
	show_stats(ch, MOB_DEATHS);
    else
	do_function(ch, &do_statlist, "");

    return;
}

int compare_type;

int compare_stats (const void *v1, const void *v2)
{
	STAT_DATA *stat1 = *(STAT_DATA **) v1;
	STAT_DATA *stat2 = *(STAT_DATA **) v2;

	if (!stat2)
		return 1;
	if (!stat1)
		return 2;

	return stat2->gamestat[compare_type] - stat1->gamestat[compare_type];
}

int count_statlist ()
{
	STAT_DATA *stat;
	int i = 0;

	for (stat = stat_list; stat != NULL; stat = stat->next)
		i++;

	return i;
}

void show_stats(CHAR_DATA * ch, int type)
{
    STAT_DATA *curr;
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    STAT_DATA * *top;
    int count, pos, loop;
    int size = count_statlist();
    bool found = FALSE;
    const char *stat_name[MAX_GAMESTAT] = 
    {
	"PLAYER KILLERS",
	"MOB KILLERS",
	"PK DEATHS",
	"MOB DEATHS"
    };

    output = new_buf (  );
    count = 0;
    pos = 0;
    compare_type = type;

    top = (STAT_DATA **)alloc_mem(size);

    sprintf(buf, "RANKING OF %s", stat_name[type]);
    add_buf(output, buf);
    add_buf(output, "\n\r");
    sprintf(buf, "{R----------------------------{x");
    add_buf(output, buf);
    add_buf(output, "\n\r");
    
    loop = 0;

	for (curr = stat_list; curr != NULL; curr = curr->next)
	{
		top[count] = curr;
		count++;
		found = TRUE;
	}

	if (found)
	{
		qsort (top, count, sizeof (*top), compare_stats);

		for (loop = 0; loop < count; loop++)
		{
			if (loop >= 50)
				break;

			sprintf (buf, "%2d) %-20s [%8ld]    ", loop + 1,
							top[loop]->name, top[loop]->gamestat[type]);
                        add_buf(output, buf);

			if (++pos % 2 == 0)
			{
				add_buf (output, "\n\r");
				pos = 0;
			}
		}
	}

	if (!found)
		add_buf(output, "\n\rNo one found yet.\n\r");
	else if (pos % 2 != 0)
		add_buf (output, "\n\r");
	else
		add_buf(output, "\n\r");
	page_to_char (buf_string (output), ch);
	free_buf (output);
	free_mem (top, size);
	return;
}
