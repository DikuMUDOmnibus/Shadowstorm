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
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "include.h"

CMD_DATA * cmd_first;
CMD_DATA * cmd_last;


#define CMDEDIT( fun )		bool fun (CHAR_DATA *ch, char *argument )
#define LOG_NORMAL      0
#define LOG_ALWAYS      1
#define LOG_NEVER       2

extern const struct size_type cat_table[];
extern const struct size_type log_table[];

char *dofuname args ((CMD_DATA *cmd ) );
CMD_DATA *new_cmd args( (void) );
void free_cmd args( (CMD_DATA *cmd) );
DO_FUN *dofun_lookup args( (const char *name) );


struct cmdfun_type
{
	char *name;
	DO_FUN *do_fun;
};
extern const struct cmdfun_type cmdfun_table[];

void save_cmd_table()
{
	FILE *fp;
	CMD_DATA *cmd;

	if( file_exists("../data/command/command.dat" ) &&
		( fp = file_open("../data/command/command.dat","w+") ) != NULL )
	{
		for(cmd = cmd_first; cmd ; cmd = cmd->next )
		{

			fprintf(fp, "Name %s~\n", cmd->name );
			fprintf(fp, "Dofun %s~\n", dofuname(cmd) );
			fprintf(fp, "Position %d\n", cmd->position );
			fprintf(fp, "Level %d\n", cmd->level );
			fprintf(fp, "Log %d\n",cmd->log );
			fprintf(fp, "Show %d\n", cmd->show );
			fprintf(fp, "Cat %d\n", cmd->cat );
			fprintf(fp, "Disabled %d\n\r", cmd->disabled ? TRUE : FALSE);
			fprintf(fp, "End\n\n");
		}
	}
	fprintf(fp, "$\n");
	file_close(fp);
}

void load_cmd_list()
{
	FILE *fp;
	CMD_DATA *cmd;
	const char *word;
	if(file_exists("../data/command/command.dat") )
		fp = file_open("../data/command/command.dat", "r" );
	else
	{	logf2("You got no commands now! Your screwed! Blame Davion!\n\r");
		return;
	}
	for( ;; )
	{
		word = feof (fp) ? "End" : fread_word (fp);
		if(word[0] == '$' )
			return;
		switch (UPPER(word[0] ))
		{
			case 'C':
				if(!str_cmp(word, "Cat" ) )
				   cmd->cat = fread_number(fp);
				break;
			case 'D':
				if(!str_cmp(word, "Dofun" ) )
				{	cmd->do_fun = dofun_lookup(fread_string(fp) );
					break;
				}
				if(!str_cmp(word, "Disabled" ) )
				{	cmd->disabled = fread_number(fp);
					break;
				}
				break;
			case 'E':
				if(!str_cmp(word, "End" ) )
					break;
				break;
			case 'L':
				if(!str_cmp(word, "Level" ) )
				{	cmd->level = fread_number(fp);
					break;
				}
				if(!str_cmp(word, "Log" ) )
				{	cmd->log = fread_number(fp);
					break;
				}
				break;
			case 'N':
				if(!str_cmp(word, "Name" ) )
				{	cmd = new_cmd();
					cmd->name = fread_string(fp);
					break;
				}
				break;
			case 'P':
				if(!str_cmp(word, "Position" ) )
				{	cmd->position = fread_number(fp);
					break;
				}
				break;
			case 'S':
				if(!str_cmp(word, "Show" ) )
				{	cmd->show = fread_number(fp);
					break;
				}
				break;
		}//End of switch
	}//End of for

	file_close(fp);
}


char *dofuname (CMD_DATA *cmd)
{
	int i;
	for( i = 0; cmdfun_table[i].name != NULL ; i ++ )
	{
		if(cmd->do_fun == cmdfun_table[i].do_fun)
			return cmdfun_table[i].name;
	}
	logf2("BUG: %s has no do function", cmd->name );
	return "do_null";
}

DO_FUN *dofun_lookup(const char *name)
{
	int i;
	for (i = 0;cmdfun_table[i].name != NULL ; i ++)
	{
		if(!str_cmp(name, cmdfun_table[i].name ) )
			return cmdfun_table[i].do_fun;
	}
	return NULL;
}

CMD_DATA *cmd_lookup(const char *name )
{
	CMD_DATA *cmd;

	for( cmd = cmd_first ; cmd ; cmd = cmd->next )
	{
		if(!str_cmp(name, cmd->name ) )
		    return cmd;
	}
	return NULL;
}

CMD_DATA *cmd_free;

CMD_DATA *new_cmd(void)
{
    static CMD_DATA cmd_zero;
        CMD_DATA *cmd;

    if (cmd_free == NULL)
                cmd = (CMD_DATA *)alloc_perm(sizeof(*cmd));
        else
    {
                cmd = cmd_free;
                cmd_free = cmd_free->next;
    }
    *cmd = cmd_zero;

	if(!cmd_first )
		cmd_first = cmd;

	if(cmd_last)
	{	cmd_last->next = cmd;
		cmd->prev = cmd_last;
	}
	cmd_last = cmd;


     free_string(cmd->name);
     cmd->name = str_dup("NewCommand");
	 cmd->do_fun = cmdfun_table[1].do_fun;
	 cmd->level = 0;
	 cmd->position = 0;
	 cmd->show = 1;
	 cmd->log = LOG_NORMAL;

    VALIDATE(cmd);
    return cmd;
}

void free_cmd(CMD_DATA *cmd)
{
    if (!IS_VALID(cmd))
                return;

    INVALIDATE(cmd);
    cmd->next = cmd_free;
	cmd->prev = cmd_free;
	cmd_free = cmd;
}


CMDEDIT(cmdedit_show )
{
	CMD_DATA *cmd;

	EDIT_COMMAND(ch, cmd );

	printf_to_char(ch, "{DCommand name{r:         {W%s{x\n\r", cmd->name );
	  	  send_to_char("{D======================================{x\n\r",ch);
	printf_to_char(ch, "{DCommand Catagory{r:     {W%s{x\n\r", cat_table[cmd->cat].name );
	printf_to_char(ch, "{DCommand Do_function{r:  {W%s{x\n\r", dofuname(cmd) );
	printf_to_char(ch, "{DCommand Level{r:        {W%d{x\n\r", cmd->level );
	printf_to_char(ch, "{DCommand Position{r:     {W%s{x\n\r", position_table[cmd->position].name );
	printf_to_char(ch, "{DCommand Show{r:         {W%s{x\n\r", cmd->show == TRUE ? "Yes" : "No" );
	printf_to_char(ch, "{DCommand Log{r:          {W%s{x\n\r", log_table[cmd->log].name );
	return FALSE;
}

CMDEDIT( cmdedit_name )
{
	CMD_DATA *cmd;
	
	EDIT_COMMAND(ch, cmd );

	if(argument[0] == '\0')
	{
		send_to_char("Syntax: name <name>\n\r",ch);
		return FALSE;
	}

	if( cmd_lookup(argument ) != NULL )
	{
		send_to_char("There is already a command by that name.\n\r",ch);
		return FALSE;
	}

	free_string(cmd->name );
	cmd->name = str_dup(argument );
	return TRUE;
}

int cat_lookup (const char *name )
{
	int i;

	for( i = 0 ; cat_table[i].name != NULL ; i++ )
	{
		if(!str_cmp(cat_table[i].name, name )  )
			return i;
	}
	return -1;
}

CMDEDIT( cmdedit_catagory )
{
	CMD_DATA *cmd;
	int cat;
	EDIT_COMMAND(ch, cmd );

	if(argument[0] == '\0' )
	{
		send_to_char("Syntax: catagory <catagory>\n\r",ch);
		return FALSE;
	}

	if( ( cat = cat_lookup (argument ) ) == -1 )
	{
		send_to_char("That is not a valid catagory. Valid catagory's are:\n\r",ch);
		for(cat = 0; cat_table[cat].name != NULL ; cat++ )
		    printf_to_char(ch, "%s ", cat_table[cat].name);
		return FALSE;
	}

	printf_to_char(ch, "%s is now placed in the %s catagory.\n\r",cmd->name, cat_table[cat].name );
	cmd->cat = cat;
	return TRUE;
}

CMDEDIT( cmdedit_dofun )
{
	CMD_DATA *cmd;
	DO_FUN *dofun;

	EDIT_COMMAND(ch, cmd);

	if(argument[0] == '\0' )
	{
		send_to_char("Syntax: dofun <dofun>\n\r",ch);
		return FALSE;
	}

	if( ( dofun = dofun_lookup(argument ) ) == NULL )
	{
		send_to_char("No such do_function. Did you add it to the dofun_table?\n\r",ch);
		return FALSE;
	}

	cmd->do_fun = dofun;
	send_to_char("Do function set.\n\r",ch);
	return TRUE;
}

CMDEDIT (cmdedit_level )
{
	CMD_DATA *cmd;
	int level;
	char arg[MSL];

	EDIT_COMMAND(ch, cmd);

	argument = one_argument(argument,arg);

	if(arg[0] == '\0' )
	{
		send_to_char("Syntax level <#level>\n\r",ch);
		return FALSE;
	}
	if(!is_number(arg) )
	{
		send_to_char("It must be a number.\n\r",ch);
		return FALSE;
	}

	level = atoi(arg);
	if(level  > MAX_LEVEL || level < 0 )
	{
		send_to_char("It must be less than 159, and greater than 0.\n\r",ch);
		return FALSE;
	}
	send_to_char("Level set.\n\r",ch);
	cmd->level = level;
	return TRUE;
}

CMDEDIT( cmdedit_position )
{
	CMD_DATA *cmd;
	int pos;

	EDIT_COMMAND (ch, cmd );

	if(argument[0] == '\0' )
	{
		send_to_char("Syntax: position <position>\n\r",ch);
		return FALSE;
	}

	if( ( pos = position_lookup(argument ) ) == -1 )
	{
		send_to_char("What type of position is that?\n\r", ch );
		return FALSE;
	}

	send_to_char("Position set.\n\r",ch);
	cmd->position = pos;
	return TRUE;
}

CMDEDIT( cmdedit_showcmd )
{
	CMD_DATA *cmd;
	
	EDIT_COMMAND(ch, cmd);

	if(argument[0] == '\0' )
	{
		send_to_char("Syntax: cmdshow <true/false>\n\r",ch);
		return TRUE;
	}

	if(!str_cmp(argument, "true" ) )
	{
		send_to_char("This command will now show.\n\r",ch);
		cmd->show = TRUE;
		return TRUE;
	}

	if(!str_cmp(argument, "false" ) )
	{
		send_to_char("This command will no longer be shown.\n\r",ch);
		cmd->show = FALSE;
		return TRUE;
	}

	send_to_char("Either 'true' or 'false'.\n\r",ch);
	return FALSE;
}

CMDEDIT( cmdedit_log)
{
	CMD_DATA *cmd;

	EDIT_COMMAND(ch, cmd);

	if(argument[0] == '\0' )
	{
		send_to_char("Syntax: log <never|always|normal>\n\r",ch);
		return FALSE;
	}

	if(!str_cmp(argument, "never" ) )
	{
		send_to_char("Logging set to never.\n\r",ch);
		cmd->log = LOG_NEVER;
		return TRUE;
	}
	if(!str_cmp(argument, "always" ) )
	{
		send_to_char("Logging set to always.\n\r",ch);
		cmd->log = LOG_ALWAYS;
		return TRUE;
	}
		if(!str_cmp(argument, "normal" ) )
	{
		send_to_char("Logging set to normal.\n\r",ch);
		cmd->log = LOG_NORMAL;
		return TRUE;
	}
	
	send_to_char("Either never, normal, or always.\n\r",ch);
	return FALSE;
}

CMDEDIT( cmdedit_new )
{
	CMD_DATA *cmd;

	if(argument[0] == '\0' )
	{
		send_to_char("Syntax: new <New Name>\n\r",ch);
		return FALSE;
	}

	if( cmd_lookup(argument ) != NULL )
	{
		send_to_char("That command already exists.\n\r",ch);
		return FALSE;
	}
	cmd = new_cmd();
	free_string(cmd->name );
	cmd->name = str_dup(argument);
	ch->desc->pEdit = (void *) cmd;
	send_to_char("Command created.\n\r",ch);
	return TRUE;
}

CMDEDIT( cmdedit_move )
{
	CMD_DATA *cmd, *mCmd, *tCmd, *nCmd, *pCmd;
	char arg1[MSL], arg2[MSL], arg3[MSL];
	

	EDIT_COMMAND(ch, cmd);

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if(arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
	{
		send_to_char("Syntax: move <command> <befor|after> <to-command>\n\r",ch);
		return FALSE;
	}

	if( ( tCmd = cmd_lookup(arg3 ) ) == NULL )
	{
		send_to_char("To command, not existing.\n\r",ch );
		return FALSE;
	}

	if( ( mCmd = cmd_lookup(arg1 ) ) == NULL )
	{
		send_to_char("Which command would you like to move?\n\r",ch);
		return FALSE;
	}

	if( tCmd == mCmd )
	{
		send_to_char("You can't do that!\n\r",ch);
		return FALSE;
	}



	if(!str_cmp(arg2, "befor" ) )
	{
		pCmd = mCmd->prev;
		nCmd = mCmd->next;
		mCmd->next = NULL;
		mCmd->prev = NULL;

		if(cmd_last == mCmd )
			cmd_last = pCmd;
		else
			nCmd->prev = pCmd;

		if(cmd_first == mCmd )
			cmd_first = nCmd;
		pCmd->next = nCmd;

		if(cmd_first == tCmd )
		{	
			cmd_first = mCmd;
			mCmd->next = tCmd;
			tCmd->prev = mCmd;
			printf_to_char(ch, "%s moved befor %s.\n\r", mCmd->name, tCmd->name );
			return TRUE;
		}
		
		pCmd = tCmd->prev;
		printf_to_char(ch, "%s moved befor %s.\n\r", mCmd->name, tCmd->name );
		tCmd->prev = mCmd;
		pCmd->next = mCmd;
		mCmd->prev = pCmd;
		mCmd->next = tCmd;
		return TRUE;
	}

	if(!str_cmp(arg2, "after" ) )
	{
		pCmd = mCmd->prev;
		nCmd = mCmd->next;
		mCmd->next = NULL;
		mCmd->prev = NULL;

		if(cmd_last == mCmd )
			cmd_last = pCmd;
		else
			nCmd->prev = pCmd;

		if(cmd_first == mCmd )
			cmd_first = nCmd;
		else
			pCmd->next = nCmd;

		if(cmd_last == tCmd )
		{
			cmd_last = mCmd;
			mCmd->prev = tCmd;
			tCmd->next = mCmd;
			printf_to_char(ch, "%s moved after %s.\n\r", mCmd->name, tCmd->name );
			return TRUE;
		}
		nCmd = tCmd->next;
		printf_to_char(ch, "%s moved after %s.\n\r", mCmd->name, tCmd->name );
		nCmd->prev = mCmd;
		tCmd->next = mCmd;
		mCmd->next = nCmd;
		mCmd->prev = tCmd;
		return TRUE;
	}
	send_to_char("It must be 'befor' or 'after' the command.\n\r",ch);
	return FALSE;
}

CMDEDIT( cmdedit_delete )
{	CMD_DATA *pCmd, *temp, *prev;

	if(argument[0] == '\0' )
	{	send_to_char("Syntax delete <cmdname>\n\r",ch);
		return FALSE;
	}

	if( ( pCmd = cmd_lookup(argument) ) == NULL )
	{	send_to_char("That isn't a valid command.\n\r",ch );
		return FALSE;
	}
	if( !is_admin(ch) )
	{	send_to_char("Only Admins can delete command.\n\r",ch);
		return FALSE;
	}

	for(temp = cmd_first ; temp ; temp = temp->next )
	{	if(temp->next == pCmd )
		{	prev = pCmd->next;
			prev->prev = temp;
			temp->next = prev;
			free_cmd(pCmd);
			send_to_char("Commmand Delete.\n\r",ch);
			return TRUE;
		}
	}
	cmdedit_delete(ch, NULL );
	return FALSE;
}

void do_cmdlist(CHAR_DATA *ch, char *argument )
{
	int col = 0;
	CMD_DATA *cmd;

	for(cmd = cmd_first; cmd ; cmd = cmd->next )
	{
		  printf_to_char( ch, " [{c%-11s{x]", cmd->name );
		  if ( ++col % 5 == 0 )
						send_to_char("\n\r",ch);
	}
	return;
}
const	struct	cmdfun_type	cmdfun_table	[] =
{
    { "north",		do_north		},
    { "east",		do_east			},
    { "south",		do_south		},
    { "west",		do_west			},
    { "up",			do_up			},
    { "down",		do_down			},
    { "at",         do_at			},
    { "cast",		do_cast			},
    { "buy",		do_buy			},
    { "channels",   do_channels		},
    { "exits",		do_exits		},
    { "get",		do_get			},
    { "goto",       do_goto			},
    { "group",      do_group		},
    { "guild",		do_guild		},
    { "do_kill",	do_kill			},
    { "purchase",   do_purchase		},
    { "inventory",	do_inventory	},
    { "insert",     do_insert		},
    { "kill",		do_kill			},
    { "look",		do_look			},
    { "clantalk",	do_clantalk		},
    { "music",      do_music		}, 
    { "order",		do_order		},
    { "practice",   do_practice		},
    { "rest",		do_rest			},
    { "sit",		do_sit			},
    { "sockets",    do_sockets		},
    { "stand",		do_stand		},
    { "tell",		do_tell			},
    { "unlock",     do_unlock		},
    { "unlink",     do_unlink                   },
    { "wear",		do_wear			},
    { "wizhelp",	do_wizhelp		},
    { "push",       do_push			},
    { "drag",       do_drag			},
    { "donate",     do_donate		},
    { "auction",    do_auction		},
    { "bid",        do_bid			},
    { "email",		do_email		},
    { "affects",	do_affects		},
    { "affects2",   do_affects2		},
    { "areas",		do_areas		},
    { "bug",		do_bug			},
    { "board",		do_board		},
    { "commands",	do_commands		},
    { "compare",	do_compare		},
    { "consider",	do_consider		},
    { "count",		do_count		},
    { "credits",	do_credits		},
    { "equipment",	do_equipment	},
    { "examine",	do_examine		},
    { "help",		do_help			},
    { "groups",     do_groups		},
    { "motd",		do_motd			},
    { "read",		do_read			},
    { "report",		do_report		},
    { "rules",		do_rules		},
    { "score",		do_score		},
    { "skills",		do_skills		},
    { "socials",	do_socials		},
    { "show",		do_show			},
    { "spells",		do_spells		},
    { "story",		do_story		},
    { "time",		do_time			},
    { "typo",		do_typo			},
    { "unread",		do_unread		},
    { "version",    do_version		},
    { "weather",	do_weather		},
    { "who",		do_who			},
    { "whois",		do_whois		},
    { "wizlist",	do_wizlist		},
    { "scan",       do_scan			},
    { "istat",      do_istat		},
    { "slist",      do_slist		},
    { "lore",       do_lore			},
    { "finger",     do_finger		},
    { "peek",       do_peek			},
    { "note",		do_note			}, 
    { "mobdeaths",  do_mobdeaths	},
    { "mobkills",   do_mobkills		},
    { "reveal",     do_reveal		},
    { "cwho",       do_cwho			},
    { "roster",		do_roster		},
    { "alia",		do_alia			},
    { "alias",		do_alias		},
    { "autolist",	do_autolist		},
    { "autoassist",	do_autoassist	},
    { "autodamage",	do_autodamage	},
    { "autoexit",	do_autoexit		},
    { "autogold",	do_autogold		},
    { "autoloot",	do_autoloot		},
    { "autosac",	do_autosac		},
    { "autosplit",	do_autosplit	},
    { "brief",		do_brief		},
    { "colour",		do_colour		},
    { "combine",	do_combine		},
    { "compact",	do_compact		},
    { "description",do_description	},
    { "delet",		do_delet		},
    { "delete",		do_delete		},
    { "nofollow",	do_nofollow		},
    { "noloot",		do_noloot		},
    { "nosummon",	do_nosummon		},
    { "password",	do_password		},
    { "prompt",		do_prompt		},
    { "scroll",		do_scroll		},
    { "title",		do_title		},
    { "unalias",	do_unalias		},
    { "wimpy",		do_wimpy		},
    { "stance",     do_stance		},
    { "nickname",	do_nickname		},
    { "afk",		do_afk			},
    { "answer",		do_answer		},
    { "deaf",		do_deaf			},
    { "emote",		do_emote		},
    { "pmote",		do_pmote		},
    { "gossip",		do_gossip,		},
    { "grats",		do_grats		},
    { "gtell",		do_gtell		},
    { "pose",		do_pose			},
    { "quote",		do_quote		},
    { "quiet",		do_quiet		},
    { "reply",		do_reply		},
    { "replay",		do_replay		},
    { "say",		do_say			},
    { "shout",		do_shout		},
    { "yell",		do_yell			},
    { "pray",       do_pray			},
    { "beep",       do_beep			},
    { "racetalk",   do_racetalk		},
    { "whisper",    do_whisper		},
    { "hero",       do_hero			},
    { "hint",       do_hint			},
    { "penter",     do_penter		},
    { "ooc",        do_ooc			},
    { "shortdescr",	do_shortdescr	},
    { "newbie",     do_newbie		},
    { "introduce",	do_introduce	},
    { "announce",   do_announce		},
    { "strap",		do_strap		},
    { "unsheath",	do_unsheath		},
    { "sheath",		do_sheath		},
    { "belt",		do_belt			},
    { "cloak",		do_cloak		},
    { "hood",		do_hood			},
    { "brandish",	do_brandish		},
    { "close",		do_close		},
    { "drink",		do_drink		},
    { "drop",		do_drop			},
    { "eat",		do_eat			},
    { "envenom",	do_envenom		},
    { "fill",		do_fill			},
    { "give",		do_give			},
    { "heal",		do_heal			}, 
    { "list",		do_list			},
    { "lock",		do_lock			},
    { "open",		do_open			},
    { "pick",		do_pick			},
    { "pour",		do_pour			},
    { "put",		do_put			},
    { "quaff",		do_quaff		},
    { "recite",		do_recite		},
    { "remove",		do_remove		},
    { "sell",		do_sell			},
    { "sacrifice",	do_sacrifice	},
    { "value",		do_value		},
    { "wear",		do_wear			},
    { "zap",		do_zap			},
    { "forge",      do_forge		},
    { "brew",       do_brew			},
    { "resize",     do_resize		},
    { "search",     do_search		},
    { "ohide",      do_ohide		},
    { "newforge",   do_newforge		},
    { "demand",     do_demand		},
    { "exchange",   do_exchange		},
    { "boxrent",    do_boxrent		},
    { "store",      do_store		},
    { "retrieve",   do_retrieve		},
    { "smithing",   do_smithing		},
    { "repair",     do_repair		},
    { "tally",      do_tally		},
    { "sharpen",    do_sharpen		},
    { "butcher",    do_butcher		},
    { "qsmith",     do_qsmith		},
    { "purify",     do_purify           },
    { "throatchop",	do_throatchop	},
    { "backstab",	do_backstab		},
    { "bash",		do_bash			},
    { "berserk",	do_berserk		},
    { "dirt",		do_dirt			},
    { "disarm",		do_disarm		},
    { "flee",		do_flee			},
    { "kick",		do_kick			},
    { "murde",		do_murde		},
    { "murder",		do_murder		},
    { "rescue",		do_rescue		},
    { "trip",		do_trip			},
    { "engage",     do_engage		},
    { "whirlwind",  do_whirlwind	},
    { "circle",     do_circle		},
    { "surrender",  do_surrender	},
    { "war",        do_war			},
    { "warcry",     do_warcry		},
    { "assassinate",do_assassinate	},
    { "bloodlust",  do_bloodlust	},
    { "nature",     do_nature		},
    { "knee",       do_knee			},
    { "srush",      do_srush		},
    { "spin",       do_spin			},
    { "rage",       do_rage			},
    { "draw",       do_draw			},
    { "fire",       do_fire			},
    { "dislodge",   do_dislodge		},
    { "gouge",      do_gouge,		}, 
    { "cleave",     do_cleave		},
    { "blackjack",  do_blackjack	},
    { "stake",      do_stake		},
    { "tail",       do_tail			},
    { "doorbash",   do_doorbash		},
    { "feed",       do_feed			},
    { "rub",        do_rub			},
    { "mob",        do_mob			},
    { "enter", 		do_enter		},
    { "follow",		do_follow		},
    { "gain",		do_gain			},
    { "enter",		do_enter		},
    { "groups",		do_groups		},
    { "hide",		do_hide			},
    { "play",		do_play			},
    { "qui",		do_qui			},
    { "quit",		do_quit			},
    { "recall",		do_recall		},
    { "wpoint",     do_wpoint		},
    { "rent",		do_rent			},
    { "save",		do_save			},
    { "sleep",		do_sleep		},
    { "sneak",		do_sneak		},
    { "split",		do_split		},
    { "steal",		do_steal		},
    { "train",		do_train		},
    { "visible",	do_visible		},
    { "wake",		do_wake			},
    { "where",		do_where		},
    { "throw",      do_throw		},
    { "second",     do_second		},
    { "become",     do_become		},
    { "hometown",   do_hometown		},
    { "autoall",    do_autoall		},
    { "study",		do_study		},
    { "travel",     do_travel		},
    { "withdraw",   do_withdraw		},
    { "change",	    do_change		},
    { "deposit",	do_deposit		},
    { "account",	do_account		},
    { "share",	    do_share		},
    { "btransfer",  do_btransfer	},
    { "gquest",     do_gquest		},
    { "qpgive",     do_qpgive		},
    { "quest",      do_quest		},
    { "tpgive",     do_tpgive		},
    { "tpspend",    do_tpspend		},
    { "restring",   do_restring		},
    { "token",      do_token		},
    { "question",	do_question		},
    { "use",        do_use			},
    { "petlevel",   do_petlevel		},
    { "petname",    do_petname		},
    { "rental",	    do_rental		},
    { "track",      do_track		},
    { "channel",	do_channel		},
    { "clear",      do_clear		},
    { "splay",	    do_splay		},
    { "songs",	    do_songs		},
    { "bounty",     do_bounty		},
    { "recl",       do_recl		},
    { "reclass",     do_reclass		},
    { "arshow",     do_arshow		},
    { "petcall",    do_petcall		},
    { "mount",		do_mount		},
    { "dismount",	do_dismount		},
    { "beseech",    do_beseech		},
    { "tithe",      do_tithe		},
    { "autolevel",  do_autolevel	},
    { "level",      do_level		},
    { "bite", 		do_bite			},
    { "turn",		do_turn			},
    { "familiar",   do_familiar		},
    { "mtsteal",    do_mtsteal		},
    { "petshow",    do_petshow		},
    { "info",       do_info             },
    { "revert",     do_revert		},
    { "convert",    do_convert		},
    { "map",		do_map			},
    { "seize",		do_seize		},
    { "helptally",  do_helptally	},
    { "updatechar",	do_updatechar	},
    { "advance",	do_advance		},
    { "copyover",	do_copyover		},
    { "dump",		do_dump			},
    { "trust",		do_trust		},
    { "violate",	do_violate		},
    { "allow",		do_allow		},
    { "ban",		do_ban			},
    { "deny",		do_deny			},
    { "disconnect",	do_disconnect	},
    { "flag",		do_flag			},
    { "freeze",		do_freeze		},
    { "permban",	do_permban		},
    { "protect",	do_protect		},
    { "reboo",		do_reboo		},
    { "reboot",		do_reboot		},
    { "set",		do_set			},
    { "shutdow",	do_shutdow		},
    { "shutdown",	do_shutdown		},
    { "wizlock",	do_wizlock		},
    { "force",		do_force		},
    { "load",		do_load			},
    { "newlock",	do_newlock		},
    { "nochannels",	do_nochannels	},
    { "noemote",	do_noemote		},
    { "noshout",	do_noshout		},
    { "notell",		do_notell		}, 
    { "silence",	do_silence		}, 
    { "pecho",		do_pecho		}, 
    { "pardon",		do_pardon		},
    { "purge",		do_purge		},
    { "restore",	do_restore		},
    { "sla",		do_sla			},
    { "slay",		do_slay			},
    { "teleport",	do_transfer		},
    { "poofin",		do_bamfin		},
    { "poofout",	do_bamfout		},
    { "echo",		do_echo			},
    { "holylight",	do_holylight	},
    { "incognito",	do_incognito	},
    { "invis",		do_invis		},
    { "log",		do_log			},
    { "memory",		do_memory		},
    { "mwhere",		do_mwhere		},
    { "owhere",		do_owhere		},
    { "peace",		do_peace		},
    { "recho",		do_recho		},
    { "return",     do_return		},
    { "snoop",		do_snoop		},
    { "stat",		do_stat			},
    { "statlist",   do_statlist		},
    { "string",		do_string		},
    { "switch",		do_switch		},
    { "invis",		do_invis		},
    { "vnum",		do_vnum			},
    { "zecho",		do_zecho		},
    { "clone",		do_clone		},
    { "wiznet",		do_wiznet		},
    { "immtalk",	do_immtalk		},
    { "imotd",      do_imotd		},
    { "smote",		do_smote		},
    { "prefi",		do_prefi		},
    { "prefix",		do_prefix		},
    { "disable",    do_disable		},
    { "avatar",     do_avatar		},
    { "fvlist",     do_fvlist		},
    { "sedit",      do_sedit		},
    { "ifollow",    do_ifollow		},
    { "rename",     do_rename		},
    { "pwipe",      do_pwipe		},
    { "omni",       do_omni			},
    { "imp",		do_imp			},
    { "implag",     do_implag		},
    { "unrestore",  do_unrestore	},
    { "scatter",    do_scatter		},
    { "pload",      do_pload		},
    { "punload",    do_punload		},
    { "wpeace",     do_wpeace		},
    { "skillstat",  do_skillstat	},
    { "spellstat",  do_spellstat	},
    { "olevel",     do_olevel		},
    { "mlevel",     do_mlevel		},
    { "double",     do_double		},
    { "divine",     do_divine		},
    { "astrip",     do_astrip		},
    { "arealinks",  do_arealinks	},
    { "fremove",    do_fremove		},
    { "security",   do_security		},
    { "repeat",     do_repeat		},
    { "repop",      do_repop		},
    { "reward",		do_reward		},
    { "check",      do_check		},
    { "ftick",      do_ftick		},
    { "openvlist",  do_openvlist	},
    { "olist",		do_olist		},
    { "rlist",		do_rlist		},
    { "mlist",		do_mlist		},
    { "immtitle",   do_immtitle		},
    { "otype",      do_otype		},
    { "grantbp",    do_grantbp		}, 
    { "xpreset",    do_xpreset		}, 
    { "munique",    do_munique		},
    { "prand",      do_prand		},
    { "gpoint",     do_gpoint		},
    { "timeout",    do_timeout		},
    { "mobslay",    do_mobslay		},
    { "wiznetall",  do_wiznetall	},
    { "admin",      do_admin		},
    { "ibeep",      do_ibeep		},
    { "olc",		do_olc			},
    { "aedit",		do_aedit		},
    { "redit",		do_redit		},
    { "medit",		do_medit		},
    { "oedit",		do_oedit		},
    { "asave",      do_asave		},
    { "alist",		do_alist		},
    { "resets",		do_resets		},
    { "mpdump",     do_mpdump		},
    { "mpstat",     do_mpstat		},
    { "mpedit",     do_mpedit		},
    { "hedit",      do_hedit		},
    { "opedit",     do_opedit		},
    { "rpedit",     do_rpedit		},
    { "opdump",		do_opdump		},
    { "opstat",		do_opstat		},
    { "rpdump",		do_rpdump		},
    { "rpstat",		do_rpstat		},
    { "promote",    do_promote		}, 
    { "save_guilds",do_save_guilds	},
    { "gedit",      do_gedit		},
    { "instaroom",	do_instaroom	},
    { "skedit",		do_skedit		},
	{ "cmdedit",	do_cmdedit		},
	{ "cmdlist",	do_cmdlist      },
	{ "mine",	do_mine		},
        { "godchat",    do_godchat	},
    	{ "autoweather",    do_autoweather  },
    	{ "wset",           do_wset         },
	{ "build",	    do_build	    },
        { "isignal",        do_isignal      },
        { "fquit",          do_fquit        },
	{ "saveconst",	    do_saveconst    },
        { "constval",	    do_constval	    },
        { "checkvnum",	    do_checkvnum    },
        { "levelflux",      do_levelflux    },
        { "petlist",        do_petlist      },
        { "rally",          do_rally        },
        { "punch",          do_punch        },
	{ "rlgedit",	    do_rlgedit	    },
	{ "rescind",	    do_rescind	    },
	{ "exalt",	    do_exalt	    },
	{ "initiate",	    do_initiate	    },
	{ "denounce",	    do_denounce	    },
        { "trivia",         do_trivia       },
        { "suppress",       do_suppress     },
        { "homeb",          do_homeb        },
        { "objbuy",         do_objbuy       },
        { "mobbuy",         do_mobbuy       },
        { "invite",         do_invite       },
        { "home",           do_home         },
        { "hname",          do_hname        },
        { "hdesc",          do_hdesc        },
        { "join",           do_join         },
        { "boot",           do_boot         },
        { "mrelic",         do_mrelic       },
        { "pack",           do_pack         },
        { "sstep",          do_sstep        },
        { "frecall",        do_frecall      },
        { "away",           do_away         },
        { "tfunc",          do_testfunc     },
        { "pcforge",        do_pcforge      },
        { "pshow",          do_pshow        },
        { "post",           do_post         },
        { "chmail",         do_chmail       },
        { "plist",          do_plist        },        
        { "pinfo",          do_pinfo        },
        { "timer",          do_timer        },
        { "sound",          do_sound        },
        { "stop",           do_stop         },
	{ "testsound",	    do_testsound    },
        { "click",          do_click        },
        { "compress",       do_compress     },
        { "showcompress",   do_showcompress },
        { "mudinfo",        do_mudinfo      },
        { "image",          do_image        },
        { "clist",          do_clist        },
        { "inlay",          do_inlay        },
        { "marry",          do_marry        },
        { "divorce",        do_divorce      },
        { "spousetalk",     do_spousetalk   },
        { "spgate",         do_spgate       },
        { "trade",          do_trade        },
	{ "mxp",	    do_mxp	    },
	{ NULL,			NULL	    },
};

const struct size_type cat_table[] =
{
	{ "None" 		},
	{ "Config" 		},
	{ "Common" 		},
        { "Communication" 	},
	{ "Info"   		},
	{ "Object Manipulation" },
	{ "Movement" 		},
	{ "Combat"   		},
	{ NULL 			}
};

const struct size_type log_table[] =
{
	{ "Normal" },
	{ "Always" },
	{ "Never"  },
	{ NULL	   }
};
