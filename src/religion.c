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

/* For the OLC. Makes it so we can create an OLC Function!*/
#define RLGEDIT( fun )		bool fun (CHAR_DATA *ch, char *argument )

/*Local Prototypes */

RELIGION *new_religion	args( (void) );
int god_lookup			args( ( const char *name ) );
int relgrank_lookup		args( ( const char *name ) );
void free_religion		args( (RELIGION *pRlg) );
void join_religion		args( (CHAR_DATA *ch, RELIGION *pRlg ) );
void denounce_faith		args( (CHAR_DATA *ch, bool force ) );
bool same_main_relg		args( (CHAR_DATA *ch, CHAR_DATA *victim ) );

/* Local structures */
extern struct sex_type relgrank_table[];
extern struct sex_type god_table[];


/*Begining of OLC Functions */

RLGEDIT(rlgedit_show)
{	RELIGION *pRlg, *pFct;
	BUFFER *buffer;
	int faction, rank, iClass;
	char buf[MSL], arg[MSL], arg2[MSL], buf2[MSL];

	argument = one_argument(argument, arg);
	EDIT_RELIGION(ch, pRlg );

	if(arg[0] == '\0' )
	{	printf_to_char(ch, "{DReligion Editor for{r:{W %s\n\r\n\r", pRlg->name );
		printf_to_char(ch, "{DGod{r:           {W%10s{x\n\r", pRlg->god);
		printf_to_char(ch, "{DTemple{r:        {W%10s{x\n\r", pRlg->temple ? pRlg->temple->name : "Not Set"  );
		printf_to_char(ch, "{DDonation Vnum{r:   {W%10d{x\n\r",pRlg->donation_vnum );
		printf_to_char(ch, "\n\r{DExalt Message{r:{W %s{x\n\r", pRlg->pmt_msg );
		printf_to_char(ch, "\n\r{DRescind Message{r: {W%s{x\n\r", pRlg->dmt_msg );
		printf_to_char(ch, "\n\r{DSacrifice Message{r: {W%s{x\n\r", pRlg->sac_msg );
		printf_to_char(ch, "\n\r{DChannel Name{r: {W%s\n\r", pRlg->chan_name );

		send_to_char("{D---------------------------------------------------------\n\r",ch);
		send_to_char("{DFactions{r:{x\n\r",ch);
		faction = 0;
		if(!pRlg->faction[faction])
		{	send_to_char("{DThis religion has no Faction.\n\r",ch);
			send_to_char("{DType 'show ranks' to view the rank info.\n\r",ch);
			return FALSE;
		}
		printf_to_char(ch, "                      {W%-15s   %-15s\n\r", 
				"Faction 1", 
				pRlg->faction[FACTION_TWO] ?  "Faction 2" : "");

		printf_to_char(ch, "{DFaction Name{r:      [{W%-15s{r] [{W%-15s{r]\n\r", 
				pRlg->faction[FACTION_ONE]->name,
				pRlg->faction[FACTION_TWO] ? pRlg->faction[FACTION_TWO]->name : "" );

		printf_to_char(ch, "{DDeity{r:             [{W%-15s{r] [{W%-15s{r]\n\r", 
				pRlg->faction[FACTION_ONE]->deity,
				pRlg->faction[FACTION_TWO] ? pRlg->faction[FACTION_TWO]->deity : "");

		printf_to_char(ch, "{DTemple{r:            [{W%-15s{r] [{W%-15s{r]\n\r", 
				pRlg->faction[FACTION_ONE]->temple ? pRlg->faction[FACTION_ONE]->temple->name : "Not Set",
				pRlg->faction[FACTION_TWO] ? ( pRlg->faction[FACTION_TWO]->temple ? pRlg->faction[FACTION_TWO]->temple->name : "Not Set" ) : "");

		printf_to_char(ch, "{DDonation Vnum{r:     [{W%-15d{r] [{W%-15d{r]\n\r",
				pRlg->faction[FACTION_ONE]->donation_vnum,
				pRlg->faction[FACTION_TWO] ? pRlg->faction[FACTION_TWO]->donation_vnum : 0);
		

		send_to_char("{DType 'show <one|two> <rank|message>'\n\r",ch);
		send_to_char("{DType 'show ranks' to view the rank info.\n\r",ch);
		return TRUE;
	}
	if(!str_cmp(arg, "ranks" ) )
	{	
		printf_to_char(ch, "{DThese are the ranks and abilities for {W%s{D.{x\n\r\n\r", pRlg->name );
		printf_to_char(ch, "{D%-10s{r|{D%-10s{r|{D%20s{r|{D%20s{r|{x\n\r", "Rank Name", "Class Name", "Rank Title", "Rank Ability" );
		buffer = new_buf();
		for(rank = 0; rank < MAX_RELG_RANK ; rank++ )
			for(iClass = 0; iClass < MAX_CLASS ; iClass++ )
			{	sprintf(buf2, "\n\r%10s", relgrank_table[rank].name );
				sprintf(buf, "{D%-10s{r|{D%-10s{r|{W%20s{r|{W%20s{r|{x\n\r", iClass == 0 ? buf2 : "", class_table[iClass].name, pRlg->rank[iClass][rank], pRlg->skpell[iClass][rank] );
				add_buf(buffer, buf);
			}
		add_buf(buffer,"{D*************************************************************{x\n\r" );
		add_buf(buffer, "{DTo see Faction ranks, type {r'{Wshow <one|two> rank{r'{x.\n\r");
		page_to_char(buf_string(buffer), ch);
		free_buf(buffer);

		return TRUE;
	}
	if(!str_cmp(arg, "one" ) )
	{	if( ( pFct = pRlg->faction[FACTION_ONE] ) == NULL)
		{	send_to_char("This Religion does not have Factions to show!\n\r",ch);
			return FALSE;
		}
	}
	else if (!str_cmp(arg, "two" ) )
	{	if( ( pFct = pRlg->faction[FACTION_TWO] ) == NULL)
		{	send_to_char("This Religion does not have a second Factions to show!\n\r",ch);
			return FALSE;
		}
	}
	else
	{	send_to_char("Syntax:  show\n\r",ch);
		send_to_char("Syntax:  show ranks\n\r",ch);
		send_to_char("Syntax:  show <faction number (\"one\" or \"two\" )> <rank|message>.\n\r",ch);
		return FALSE;
	}

	argument = one_argument(argument, arg2 );
	if( arg2[0] == '\0' )
	{	send_to_char("Would you like to see ranks, or messages for the faction?\n\r",ch);
		return FALSE;
	}
	if(!str_cmp(arg2, "ranks" ) )
	{	printf_to_char(ch, "{DThese are the ranks and abilities for the faction {W%s{D.{x\n\r\n\r", pFct->name );
		printf_to_char(ch, "{D%-10s{r|{D%-10s{r|{D%20s{r|{D%20s{r|{x\n\r", "Rank Name", "Class Name", "Rank Title", "Rank Ability" );
		buffer = new_buf();
		for(rank = 0; rank < MAX_RELG_RANK ; rank++ )
			for(iClass = 0; iClass < MAX_CLASS ; iClass++ )
			{	sprintf(buf2, "\n\r%10s", relgrank_table[rank].name );
				sprintf(buf, "{D%-10s{r|{D%-10s{r|{W%20s{r|{W%20s{r|{x\n\r", iClass == 0 ? buf2 : "", class_table[iClass].name, pFct->rank[iClass][rank], pFct->skpell[iClass][rank] );
				add_buf(buffer, buf);
			}
		page_to_char(buf_string(buffer), ch);
		free_buf(buffer);
		return TRUE;
	}
	if(!str_cmp(arg2, "message" ) )
	{	printf_to_char(ch, "{DThese are messages for the faction {W%s{D.{x\n\r\n\r", pFct->name );
		printf_to_char(ch, "{DExalt Message{r:{W %s{x\n\r", pFct->pmt_msg );
		printf_to_char(ch, "\n\r{DRescind Message{r: {W%s{x\n\r", pFct->dmt_msg );
		printf_to_char(ch, "\n\r{DSacrifice Message{r: {W%s{x\n\r", pFct->sac_msg );
		printf_to_char(ch, "\n\r{DChannel Name{r: {W%s\n\r", pFct->chan_name );
		return TRUE;
	}

	send_to_char("Syntax:  show\n\r",ch);
	send_to_char("Syntax:  show ranks\n\r",ch);
	send_to_char("Syntax:  show <faction number (\"one\" or \"two\" )> <rank|message>.\n\r",ch);
	return FALSE;
}
		

RLGEDIT( rlgedit_new )
{	RELIGION *pRlg;
	
	if( argument[0] == '\0' )
	{	send_to_char("Syntax: Rlgedit new <name>\n\r",ch);
		return FALSE;
	}
	
	if( (pRlg = religion_lookup(argument) ) != NULL )
	{	send_to_char("That religion already exists. Try another name.\n\r",ch);
		return FALSE;
	}

	pRlg = new_religion();
	free_string(pRlg->name);
	pRlg->name = str_dup(capitalize(argument));
	pRlg->next = religion_list;
	religion_list = pRlg;
	printf_to_char(ch, "The order of %s, has now been created.\n\r",pRlg->name );
	ch->desc->editor = ED_RELIGION;
	ch->desc->pEdit = (void *) pRlg;
	return TRUE;
}


RLGEDIT( rlgedit_name )
{	RELIGION * pRlg;
	
	EDIT_RELIGION( ch, pRlg );

	if( argument[0] == '\0' )
	{	send_to_char("Syntax: name <name>\n\r",ch);
		return FALSE;
	}

	if( religion_lookup(argument) != NULL )
	{	send_to_char("That religion already exists. Please use another name.\n\r",ch);
		return FALSE;
	}
	printf_to_char(ch, "The order of %s has been renamed to the order of %s\n\r",pRlg->name, capitalize(argument) );
	free_string(pRlg->name);
	pRlg->name = str_dup(capitalize(argument ) );
	return TRUE;
}

RLGEDIT( rlgedit_god )
{	RELIGION * pRlg;
	int god;

	EDIT_RELIGION(ch, pRlg);

	if(argument[0] == '\0' )
	{	send_to_char("Syntax: god <god name>\n\r", ch );
		return FALSE;
	}

	if( ( god = god_lookup(argument )  ) == -1 )
	{	send_to_char("That isn't a valid god.\n\r",ch);
		return FALSE;
	}

	free_string(pRlg->god);
	pRlg->god = str_dup(capitalize(argument ) );
	printf_to_char(ch, "The god for the order of %s has been set too %s.\n\r",pRlg->name, pRlg->god );
	return TRUE;
}

RLGEDIT( rlgedit_temple )
{	RELIGION *pRlg;
	AREA_DATA *pArea;
	int value;

	EDIT_RELIGION(ch, pRlg);

	if(argument[0] == '\0' )
	{	send_to_char("Syntax: temple <area vnum>\n\r",ch);
		return FALSE;
	}
	if(is_number(argument ) )
	{	value = atoi( argument );
		if ( !( pArea = get_area_data( value ) ) )
		{	send_to_char( "That area vnum does not exist.\n\r", ch );
		    return FALSE;
		}

		printf_to_char(ch, "Temple for the order of %s is set to %s.\n\r",pRlg->name, pArea->name );
		pRlg->temple = pArea;
		return TRUE;
	}
	else
	{	send_to_char("Syntax: temple <area vnum>\n\r",ch);
		return FALSE;
	}
	return TRUE;
}

RLGEDIT( rlgedit_donation)
{	RELIGION * pRlg;
	int vnum;

	EDIT_RELIGION(ch, pRlg );
	
	if(argument[0] == '\0' )
	{	send_to_char("Syntax: Donation <vnum>\n\r",ch);
		return FALSE;
	}

	if(!is_number(argument ) )
	{	send_to_char("A vnum is a form of number.\n\r",ch);
		return FALSE;
	}

	vnum = atoi(argument );
	if( get_room_index(vnum) == NULL )
	{	send_to_char("That isn't a valid Vnum.\n\r",ch);
		return FALSE;
	}

	pRlg->donation_vnum = vnum;
	printf_to_char(ch, "Donation Room Vnum set to %d.\n\r", pRlg->donation_vnum );
	return TRUE;
}

RLGEDIT( rlgedit_rank )
{	RELIGION *pRlg;
	char arg[MSL], arg2[MSL];
	int rank, iClass;

	EDIT_RELIGION( ch, pRlg );

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);

	if(arg[0] == '\0' || argument[0] == '\0')
	{	send_to_char("Syntax: rank <rank name> <class name> <title>\n\r",ch);
		return FALSE;
	}

	if( ( rank = relgrank_lookup(arg) ) == -1 )
	{	send_to_char("That isn't a valid rank.\n\r",ch);
		return FALSE;
	}
	if( ( iClass = class_lookup(arg2 ) ) == -1 )
	{	send_to_char("That isn't a valid class.\n\r",ch);
		return FALSE;
	}
	
	free_string(pRlg->rank[iClass][rank]);
	pRlg->rank[iClass][rank] = str_dup(capitalize(argument) );
	printf_to_char(ch, "%s, %s's %s rank was changed too %s.\n\r", pRlg->name, class_table[iClass].name, relgrank_table[rank].name, argument);
	return TRUE;
}

RLGEDIT( rlgedit_skill )
{	RELIGION *pRlg;
	char arg[MSL], arg2[MSL], arg3[MSL];
	int rank, sn, iClass;

	EDIT_RELIGION(ch, pRlg );

	argument = one_argument(argument, arg );
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if(arg[0] == '\0' || arg2[0] == '\0' )
	{	send_to_char("Syntax: skill <rank name> <class name> <skill/spell name>\n\r",ch);
		return FALSE;
	}

	if( ( rank = relgrank_lookup(arg) ) == -1 )
	{	send_to_char("That isn't a valid Religious Rank.\n\r",ch);
		return FALSE;
	}

	if( ( sn = skill_lookup(arg2) ) == -1 )
	{	send_to_char("Thats not a valid spell, or skill!\n\r",ch);
		return FALSE;
	}
	if( ( iClass = class_lookup(arg3 ) ) == -1 )
	{	send_to_char("That isn't a valid class.\n\r",ch);
		return FALSE;
	}

	printf_to_char(ch, "%s's %s rank for %s class's ability was changed too %s.\n\r", pRlg->name, relgrank_table[rank].name, class_table[iClass].name, skill_table[sn].name );
	free_string(pRlg->skpell[iClass][rank]);
	pRlg->skpell[iClass][rank] = str_dup(skill_table[sn].name );
	return TRUE;
}

RLGEDIT( rlgedit_faction )
{	RELIGION * pRlg;
	char arg[MSL], arg2[MSL], arg3[MSL], arg4[MSL], arg5[MSL];
	int faction;
	RELIGION *pFct;
	argument = one_argument(argument, arg);

	EDIT_RELIGION(ch, pRlg);
	if(arg[0] == '\0' )
	{	send_to_char("Syntax: faction create <name>\n\r",ch);
		send_to_char("        faction <one|two> <command> <value>\n\r",ch);
		send_to_char("Commands: Name, Donation, Skill, Rank, Temple, Deity, message.\n\r",ch);
		return FALSE;
	}

	if(!str_cmp(arg, "create" ) )
	{	if(argument[0] == '\0' )
		{	rlgedit_faction(ch, argument );
			return FALSE;
		}
		if(pRlg->faction[FACTION_ONE] && pRlg->faction[FACTION_TWO] )
		{	send_to_char("This religions already got enough factions!\n\r",ch);
			return FALSE;
		}
		if(pRlg->faction[FACTION_ONE])
			faction = FACTION_TWO;
		else 
			faction = FACTION_ONE;
		
		pRlg->faction[faction] = new_religion();
		pRlg->faction[faction]->isfaction = TRUE;
		pRlg->faction[faction]->name = str_dup(capitalize(argument));
		pRlg->faction[faction]->main = pRlg;
		printf_to_char(ch, "The Faction of %s has been added to the order of %s.\n\r", pRlg->faction[faction]->name, pRlg->name );
		return TRUE;
	}

	argument = one_argument(argument, arg2);
	if(!str_cmp(arg, "one" ) )
	{	if( (pFct = pRlg->faction[FACTION_ONE] ) == NULL )
		{	send_to_char("That faction has yet to be created.\n\r",ch);
			return FALSE;
		}
	}
	else if( !str_cmp(arg, "two" ) )
	{	if( (pFct = pRlg->faction[FACTION_TWO] ) == NULL )
		{	send_to_char("That faction has yet to be created.\n\r",ch);
			return FALSE;
		}
	}
	else
	{	send_to_char("Syntax: faction create <name>\n\r",ch);
		send_to_char("        faction <one|two> <command> <value>\n\r",ch);
		send_to_char("Commands: Name, Donation, Skill, Rank, Temple, deity, message.\n\r",ch);
		return FALSE;
	}		
	if(arg2[0] == '\0' )
	{	send_to_char("Syntax: faction create <name>\n\r",ch);
		send_to_char("        faction <one|two> <command> <value>\n\r",ch);
		send_to_char("Commands: Name, Donation, Skill, Rank, Temple, deity, message.\n\r",ch);
		return FALSE;
	}
	if(!str_cmp(arg2, "name" ) )
	{	if(argument[0] == '\0' ) 
		{	send_to_char("What would you like to change the name too?\n\r",ch);
			return FALSE;
		}
		printf_to_char(ch, "The Faction, %s, has been changed to %s.\n\r",pFct->name, capitalize(argument) );
		free_string(pFct->name);
		pFct->name = str_dup(capitalize(argument) );
		return TRUE;
	}

	if(!str_cmp(arg2, "Donation" ) )
	{	int vnum;
		argument = one_argument(argument, arg3 );
		if(arg3[0] == '\0' )
		{	send_to_char("Where would you like to set the donation room?\n\r",ch);
			return FALSE;
		}

		if(!is_number(arg3) )
		{	send_to_char("It must be a valid vnum.\n\r", ch );
			return FALSE;
		}
		vnum = atoi(arg3);
		if( get_room_index(vnum) == NULL )
		{	send_to_char("That isn't a valid room.\n\r",ch);
			return FALSE;
		}

		printf_to_char(ch, "The Faction, %s's donation room changed to %d.\n\r", pFct->name, vnum );
		pFct->donation_vnum = vnum;
		return TRUE;
	}

	if(!str_cmp(arg2, "temple" ) )
	{	int value;
		AREA_DATA *pArea;
		argument = one_argument(argument, arg3);
		if(arg3[0] == '\0' )
		{	send_to_char("Syntax: temple <area vnum>\n\r",ch);
			return FALSE;
		}
		if(is_number(arg3 ) )
		{	value = atoi( arg3 );
			if ( !( pArea = get_area_data( value ) ) )
			{	send_to_char( "That area vnum does not exist.\n\r", ch );
			    return FALSE;
			}	

			printf_to_char(ch, "Temple for the faction, %s is set to %s.\n\r",pRlg->name, pArea->name );
			pFct->temple = pArea;
			return TRUE;
		}
		else
		{	send_to_char("Syntax: temple <area vnum>\n\r",ch);
			return FALSE;
		}
		return TRUE;
	}

	if(!str_cmp(arg2, "deity" ) )
	{	int god;
		argument = one_argument(argument, arg3);
		if(arg3[0] == '\0' )
		{	send_to_char("What would you like to set the factions Deity too?\n\r",ch);
			return FALSE;
		}
		if( ( god = god_lookup(arg3) ) == -1 )
		{	send_to_char("That isn't a valid god.\n\r",ch);
			return FALSE;
		}
		printf_to_char(ch, "The Faction, %s's deity set too %s.\n\r",pFct->name, god_table[god].name );
		free_string(pFct->deity);
		pFct->deity = str_dup(god_table[god].name );
		return TRUE;
	}

	if(!str_cmp(arg2, "Rank" ) )
	{	int rank, iClass;
		argument = one_argument(argument, arg3);
		argument = one_argument(argument, arg4);
		argument = one_argument(argument, arg5);
		if(arg3[0] == '\0' || arg4[0] == '\0' )
		{	send_to_char("Syntax: faction <number> <rank> <class> <title>\n\r",ch);
			return FALSE;
		}

		if( ( rank = relgrank_lookup(arg4) ) == -1 )
		{	send_to_char("That isn't a valid Religious Rank.\n\r",ch);
			return FALSE;
		}

		if( ( iClass = class_lookup(arg5 ) ) == -1 )
		{	send_to_char("That isn't a valid class.\n\r",ch);
			return FALSE;
		}
		
		free_string(pFct->rank[iClass][rank]);
		pFct->rank[iClass][rank] = str_dup(argument);
		printf_to_char(ch, "The faction, %s,  %s's %s rank was changed too %s.\n\r", pFct->name, class_table[iClass].name, relgrank_table[rank].name, argument);
		return TRUE;
	}

	if(!str_cmp(arg2, "Skill" ) )
	{	int sn, rank, iClass;
		argument = one_argument(argument, arg3 );
		argument = one_argument(argument, arg4 );
		argument = one_argument(argument, arg5 );
		if(arg3[0] == '\0' || arg4[0] == '\0' )
		{	send_to_char("Syntax: Faction skill <rank name> <class name> <skill/spell name>\n\r",ch );
			return FALSE;
		}
		if( ( rank = relgrank_lookup(arg3) ) == -1 )
		{	send_to_char("That isn't a valid Religious Rank.\n\r",ch);
			return FALSE;
		}
		if( ( sn = skill_lookup(arg4) ) == -1 )
		{	send_to_char("That isn't a valid spell or skill!\n\r",ch);
			return FALSE;
		}
		if( ( iClass = class_lookup(arg5 ) ) == -1 )
		{	send_to_char("That isn't a valid class.\n\r",ch);
			return FALSE;
		}

		printf_to_char(ch, "The faction, %s's %s rank for %s class's ability was changed too %s.\n\r", pFct->name, relgrank_table[rank].name, class_table[iClass].name, skill_table[sn].name );
		free_string(pFct->skpell[iClass][rank]);
		pFct->skpell[iClass][rank] = str_dup(skill_table[sn].name );
		return TRUE;
	}
	/* To lazy to write this properly, lets hope it works :) */
	if(!str_cmp(arg2, "message" ) )
	{	ch->desc->pEdit = (void *) pFct;
		rlgedit_message(ch, argument );
		ch->desc->pEdit = (void *) pRlg;
		return TRUE;
	}
	rlgedit_faction(ch, NULL);
	return FALSE;
}

RLGEDIT( rlgedit_list )
{	RELIGION *pRlg;
	
	for(pRlg = religion_list ; pRlg ; pRlg = pRlg->next )
		printf_to_char(ch, "{D%-15s {r-{D %s\n\r",pRlg->name, pRlg->god );
	return TRUE;
}

RLGEDIT( rlgedit_delete )
{	RELIGION *pRlg, *temp;

	if(argument[0] == '\0' )
	{	send_to_char("Syntax: Delete <religion>\n\r",ch);
		return FALSE;
	}

	if( ( pRlg = religion_lookup(argument) ) == NULL )
	{	send_to_char("There is no such religion.\n\r",ch);
		return FALSE;
	}

	for(temp = religion_list ; temp ; temp = temp->next )
		if(temp->next == pRlg )
			temp->next = pRlg->next;
	if(religion_list == pRlg )
		religion_list = pRlg->next;
	free_religion(pRlg);
	send_to_char("Religion Deleted!\n\r",ch);
	return TRUE;
}

RLGEDIT( rlgedit_message )
{	RELIGION *pRlg;
	char arg[MSL];

	argument = one_argument(argument, arg );
	EDIT_RELIGION(ch, pRlg);

	if( arg[0] == '\0' )
	{	send_to_char("Syntax:  message <sac|exalt|rescind|channame> <string>\n\r",ch);
		return FALSE;
	}

	if(!str_cmp(arg, "sac" ) )
	{	if(argument[0] == '\0' )
		{	send_to_char("What would you like your sacrifice message to be?\n\r",ch );
			return FALSE;
		}
		free_string(pRlg->sac_msg );
		pRlg->sac_msg = str_dup(argument);
		send_to_char("Sacrifice message set!\n\r", ch);
	}
	if(!str_cmp(arg, "exalt" ) )
	{	if(argument[0] == '\0' )
		{	send_to_char("What would you like your exalt message to be?\n\r",ch );
			return FALSE;
		}
		free_string(pRlg->pmt_msg );
		pRlg->pmt_msg = str_dup(argument);
		send_to_char("Exalt message set!\n\r", ch);
	}
	if(!str_cmp(arg, "rescind" ) )
	{	if(argument[0] == '\0' )
		{	send_to_char("What would you like your rescind message to be?\n\r",ch );
			return FALSE;
		}
		free_string(pRlg->dmt_msg );
		pRlg->dmt_msg = str_dup(argument);
		send_to_char("Rescind message set!\n\r", ch);
	}
	if(!str_cmp(arg, "channame" ) )
	{	if(argument[0] == '\0' )
		{	send_to_char("What would you like your channel name to be?\n\r",ch );
			return FALSE;
		}
		free_string(pRlg->chan_name );
		pRlg->chan_name = str_dup(argument);
		send_to_char("Channel name set!\n\r", ch);
	}
	return FALSE;
}

/*End of OLC Functions */

/*Begining of File I/O*/
void save_religion()
{	FILE *rList, *fp;
	RELIGION *pRlg;
	char buf[MSL];
	int rank, iClass;

	rList = file_open(RELG_LIST, "w");
	for(pRlg = religion_list ; pRlg ; pRlg = pRlg->next )
	{	fprintf(rList, "%s\n", pRlg->name );
		sprintf(buf, "%s%s",RELG_DIR, pRlg->name);
		fp = file_open(buf, "w");
		fprintf(fp, "Name %s~\n", pRlg->name );
		fprintf(fp, "God %s~\n", pRlg->god );
		fprintf(fp, "Temple %s~\n", pRlg->temple ? pRlg->temple->name : "None" );
		fprintf(fp, "Donation %d\n", pRlg->donation_vnum );
		fprintf(fp, "Ranks\n" );
		for( rank = 0; rank < MAX_RELG_RANK ; rank++ )
		{	for(iClass = 0; iClass < MAX_CLASS ; iClass++ )
			{	fprintf(fp, "%s~\n", pRlg->rank[iClass][rank] );
				fprintf(fp, "%s~\n", pRlg->skpell[iClass][rank] );
			}
		}
		fprintf(fp, "Faction %s~\n", pRlg->faction[FACTION_ONE] ? ( pRlg->faction[FACTION_TWO] ? "two" : "one" ) : "none" );
		if(pRlg->faction[FACTION_ONE] )
		{	RELIGION *faction = pRlg->faction[FACTION_ONE];
			fprintf(fp, "%s~\n", faction->name );
			fprintf(fp, "%s~\n", faction->deity );
			fprintf(fp, "%s~\n", faction->temple ? faction->temple->name : "None" );
			fprintf(fp, "%d\n", faction->donation_vnum );
			
			for(rank = 0; rank < MAX_RELG_RANK ; rank++ )
			{	for(iClass = 0; iClass < MAX_CLASS ; iClass++ )
				{	fprintf(fp, "%s~\n", faction->rank[iClass][rank] );
					fprintf(fp, "%s~\n",faction->skpell[iClass][rank]);
				}
			}
			if(pRlg->faction[FACTION_TWO] )
			{	faction = pRlg->faction[FACTION_TWO];
				fprintf(fp, "%s~\n", faction->name );
				fprintf(fp, "%s~\n", faction->deity );
				fprintf(fp, "%s~\n", faction->temple ? faction->temple->name : "None" );
				fprintf(fp, "%d\n", faction->donation_vnum );
				for(rank = 0; rank < MAX_RELG_RANK ; rank++ )
				{	for(iClass = 0; iClass < MAX_CLASS ; iClass++ )
					{	fprintf(fp, "%s~\n", faction->rank[iClass][rank] );
						fprintf(fp, "%s~\n",faction->skpell[iClass][rank]);
					}
				}
			}
		}
		fprintf(fp, "$\n");
		file_close(fp);
	}
	fprintf(rList, "$\n");
	file_close(rList);
}
#define KEYS( field, value )					\
				    free_string(field);			\
				    field  = value;			\

				
void load_religion()
{	RELIGION *pRlg;
	FILE *fp, *rList;
	char *word, *string;
	char buf[MSL];
	AREA_DATA *pArea;
	int rank, iClass;

	if( !file_exists(RELG_LIST ) )
	{	logf2("Relg_list not there. No religions!");
		return;
	}

	rList = file_open(RELG_LIST, "r" );
	
	for( word = fread_word(rList); str_cmp(word, "$" ); word = fread_word(rList) )
	{	sprintf(buf, "%s%s", RELG_DIR, word);
		if(!file_exists(buf ) )
		{	logf2("%s religious file missing.",word);
			continue;
		}
		rank = 0; iClass = rank;
		fp = file_open(buf, "r" );
		for(string = fread_word(fp); str_cmp(word, "$" ); word = fread_word(fp) )
		{
			if(!str_cmp(string, "Name" ) )
			{	pRlg = new_religion();
				KEYS(pRlg->name, fread_string(fp) );
				pRlg->next = religion_list;
				religion_list = pRlg;
				continue;
			}
			if(!str_cmp(string, "God" ) )
			{	KEYS(pRlg->god, fread_string(fp) );
				continue;
			}
			if(!str_cmp(string, "Temple" ) )
			{	if( ( pArea = area_lookup(fread_string(fp) ) ) == NULL )
				{	logf2("%s temple not defined.",pRlg->name );
					continue;
				}
				pRlg->temple = pArea;
				continue;
			}
			if(!str_cmp(string, "Donation" ) )
			{	pRlg->donation_vnum = fread_number(fp);
				continue;
			}
			if(!str_cmp(string, "Ranks" ) )
			{	for(rank = 0; rank < MAX_RELG_RANK ; rank++ )
				{	for(iClass = 0; iClass < MAX_CLASS ; iClass++ )
					{	KEYS(pRlg->rank[iClass][rank], fread_string(fp) );
						KEYS(pRlg->skpell[iClass][rank], fread_string(fp) );
					}
				}
				continue;
			}
			if(!str_cmp(string, "Faction" ) )
			{	KEYS(string, fread_string(fp) );
				if(!str_cmp(string,"none" ) )
					continue;
				else if(!str_cmp(string, "one") || !str_cmp(string, "two" ) )
				{	RELIGION *faction;
					faction = new_religion();
					KEYS(faction->name, fread_string(fp));
					KEYS(faction->deity, fread_string(fp));
					if( ( pArea = area_lookup(fread_string(fp) ) ) == NULL )
						logf2("Faction %s lacks temple.", faction->name );
					else
						faction->temple = pArea;
					faction->donation_vnum = fread_number(fp);
					for (rank = 0; rank < MAX_RELG_RANK ; rank++)
					{	for(iClass = 0; iClass < MAX_CLASS ; iClass++ )
						{	KEYS(faction->rank[iClass][rank], fread_string(fp) );
							KEYS(faction->skpell[iClass][rank], fread_string(fp) );
						}
					}
					faction->isfaction = TRUE;
					faction->main = pRlg;
					pRlg->faction[FACTION_ONE] = faction;
					
				}
				if(!str_cmp(string, "two" ) )
				{	RELIGION *faction;
					faction = new_religion();
					KEYS(faction->name, fread_string(fp));
					KEYS(faction->deity, fread_string(fp));
					if( ( pArea = area_lookup(fread_string(fp) ) ) == NULL )
						logf2("Faction %s lacks temple.", faction->name );
					else
						faction->temple = pArea;
					faction->donation_vnum = fread_number(fp);
					for (rank = 0; rank < MAX_RELG_RANK ; rank++)
					{	for(iClass = 0; iClass < MAX_CLASS ; iClass++ )
						{	KEYS(faction->rank[iClass][rank], fread_string(fp) );
							KEYS(faction->skpell[iClass][rank], fread_string(fp) );
						}
					}
					faction->isfaction = TRUE;
					faction->main = pRlg;
					pRlg->faction[FACTION_TWO] = faction;
				}
				continue;
			}
			logf2("Invalid string in Religion. %s", word );
		}
	}
}
		
/*End of File I/O */

/*Begining of do_functions*/

void do_exalt(CHAR_DATA *ch, char *argument )
{	CHAR_DATA *victim;
	char arg[MSL];

	if(IS_NPC(ch) )
		return;

	if(!HAS_RELIGION(ch) )
	{	send_to_char("You don't have a religion.\n\r",ch);
		return;
	}
	if( ch->pcdata->rank < RANK_PRIEST )
	{	send_to_char("You cannot exalt people!\n\r",ch );
		return;
	}
	one_argument(argument, arg );

	if(arg[0] == '\0' )
	{	send_to_char("Syntax: Exalt <person>\n\r",ch);
		return;
	}
	if(!IS_IMMORTAL(ch) )
	{	if( ( victim = get_char_room(ch, NULL, arg) )  == NULL )
		{	send_to_char("They arn't here.\n\r",ch);
			return;
		}
	}
	else
	{	if( ( victim = get_char_world(ch, arg) ) == NULL )
		{	send_to_char("They arn't here.\n\r",ch);
			return;
		}
	}
	if(IS_NPC(victim) )
	{	send_to_char("Not on NPC's.\n\r",ch);
		return;
	}
	if(!HAS_RELIGION(victim) )
	{	send_to_char("They don't have a religion.\n\r",ch);
		return;
	}

	if( ch->pcdata->rank == RANK_GOD && (IN_FACTION(ch) || IN_FACTION(victim) ) )
	{ if(!same_main_relg(ch,victim) )
	  {   send_to_char("They arn't in your religion!\n\r",ch);
	      return;
	  }
    }
	else if(!IS_SAME_RELIGION(ch, victim) && ch->pcdata->rank != RANK_GOD  )
	{	send_to_char("They arn't in your religion!\n\r",ch); 
		return; 
	}

	if( ch->pcdata->rank <= victim->pcdata->rank )
	{	send_to_char("You cannot exalt them.\n\r", ch );
		return;
	}

	if(!IS_IMMORTAL(ch) && ch->pcdata->rank <= RANK_PRIEST && victim->pcdata->rank >= RANK_BISHOP )
	{	send_to_char("You cannot exalt them.\n\r",ch);
		return;
	}

	if(victim->pcdata->rank == RANK_DEITY && ch->pcdata->rank != RANK_GOD )
	{	send_to_char("You cannot exalt to God Rank.\n\r",ch);
		return;
	}
	if(victim->pcdata->rank == RANK_GOD )
	{	send_to_char("They cannot get any higher!\n\r",ch);
		return;
	}
	if(IN_FACTION(victim) && victim->pcdata->rank == RANK_DEITY )
	{	send_to_char("You can't promote them any higher in a Faction.\n\r",ch);
		return;
	}
	if(!IN_FACTION(victim) && victim->pcdata->rank == RANK_PRIEST)
	{	send_to_char("You cannot exalt them anymore, if you wish to make them a god, use the makegod command.\n\r",ch);
		return;
	}

	victim->pcdata->rank++;
	victim->pcdata->learned[skill_lookup(victim->pcdata->religion->skpell[prime_class(victim)][victim->pcdata->rank] ) ] = 1;
	printf_to_char(ch, "%s has been exalted to %s.\n\r", victim->name, relgrank_table[victim->pcdata->rank].name );
	act_new(victim->pcdata->religion->pmt_msg, victim, NULL, NULL, TO_CHAR, POS_SLEEPING);
	return;
}

void do_rescind(CHAR_DATA *ch, char *argument )
{	CHAR_DATA *victim;

	char arg[MSL];

	if(IS_NPC(ch) )
		return;
	if(!HAS_RELIGION(ch) )
	{	send_to_char("You don't have a religion.\n\r",ch);
		return;
	}
	if( ch->pcdata->rank < RANK_PRIEST )
	{	send_to_char("You cannot rescind peoples ranks!\n\r",ch );
		return;
	}
	one_argument(argument, arg );

	if(arg[0] == '\0' )
	{	send_to_char("Syntax: rescind <person>\n\r",ch);
		return;
	}
	if(!IS_IMMORTAL(ch) )
	{	if( ( victim = get_char_room(ch, NULL, arg) )  == NULL )
		{	send_to_char("They arn't here.\n\r",ch);
			return;
		}
	}
	else
	{	if( ( victim = get_char_world(ch, arg) ) == NULL )
		{	send_to_char("They arn't here.\n\r",ch);
			return;
		}
		
	}
	if(IS_NPC(victim) )
	{	send_to_char("Not on NPC's.\n\r",ch);
		return;
	}
	if(!HAS_RELIGION(victim) )
	{	send_to_char("They don't have a religion.\n\r",ch);
		return;
	}
	if( ch->pcdata->rank == RANK_GOD && (IN_FACTION(ch) || IN_FACTION(victim) ) )
	{ if(!same_main_relg(ch,victim) )
	  {   send_to_char("They arn't in your religion!\n\r",ch);
	      return;
	  }
    }
	else if(!IS_SAME_RELIGION(ch, victim) && ch->pcdata->rank != RANK_GOD  )
	{	send_to_char("They arn't in your religion!\n\r",ch); 
		return; 
	}

	if( ch->pcdata->rank <= victim->pcdata->rank )
	{	send_to_char("You cannot rescind their rank!\n\r", ch );
		return;
	}

	if(!IS_IMMORTAL(ch) && ch->pcdata->rank <= RANK_PRIEST && victim->pcdata->rank >= RANK_BISHOP )
	{	send_to_char("You cannot rescind their rank.\n\r",ch);
		return;
	}

	if(victim->pcdata->rank == RANK_INITIATE )
	{	send_to_char("They cannot get any lower!\n\r",ch);
		return;
	}
	victim->pcdata->learned[skill_lookup(victim->pcdata->religion->skpell[prime_class(victim)][victim->pcdata->rank] ) ] = 0;
	--victim->pcdata->rank;
	act_new(victim->pcdata->religion->dmt_msg,ch, NULL, victim, TO_VICT, POS_SLEEPING );
	printf_to_char(ch, "You have lowered thier rank to %s.\n\r", relgrank_table[victim->pcdata->rank].name );
	return;
}

void do_denounce(CHAR_DATA *ch, char * argument )
{	char arg[MSL], arg2[MSL];
	CHAR_DATA *victim;
	if(IS_NPC(ch ) )
		return;

	if(!str_cmp(argument, "faith" ) )
	{	denounce_faith(ch, FALSE);
		return;
	}
	argument = one_argument(argument, arg );
	argument = one_argument(argument, arg2 );
	
	if(!str_cmp(arg, "follower" ) )
	{	if(ch->pcdata->rank <= RANK_PRIEST )
		{	do_function(ch, &do_denounce, NULL );
			return;
		}

		if(!IS_IMMORTAL(ch) )
		{	if( ( victim = get_char_room(ch, NULL, arg2 ) ) == NULL )
			{	send_to_char("They arn't here.\n\r",ch);
				return;
			}
		}
		else
		{	if( ( victim = get_char_world(ch, arg2 ) ) == NULL )
			{	send_to_char("They arn't here.\n\r", ch);
				return;
			}
		}
		if(IS_NPC(victim) )
		{	send_to_char("Not on NPC's.\n\r",ch);
			return;
		}
		if( ch->pcdata->rank == RANK_GOD && (IN_FACTION(ch) || IN_FACTION(victim) ) )
		{ if(!same_main_relg(ch,victim) )
		  {   send_to_char("They arn't in your religion!\n\r",ch);
			  return;
		  }
		}
		else if(!IS_SAME_RELIGION(ch, victim) && ch->pcdata->rank != RANK_GOD  )
		{	send_to_char("They arn't in your religion!\n\r",ch); 
			return; 
		}


		if(ch->pcdata->rank <= victim->pcdata->rank )
		{	send_to_char("You cannot denounce their faith.\n\r",ch);
			return;
		}

		send_to_char("You denounce thier faith!\n\r",ch);
		denounce_faith(victim, TRUE);
		return;
	}

	if(ch->pcdata->rank <= RANK_PRIEST )
		send_to_char("To denounce a followers faith, use the syntax: 'denounce follower <name>'\n\r",ch);
	send_to_char("To denounce your faith, please type use the syntax: 'denounce faith'\n\r", ch);
	return;
}

void do_initiate(CHAR_DATA *ch, char *argument )
{	char arg[MSL], arg2[MSL], arg3[MSL];
	CHAR_DATA *victim;

	if(IS_NPC(ch ) )
		return;

	argument = one_argument(argument, arg  );
	argument = one_argument(argument, arg2 );
	argument = one_argument(argument, arg3 );
	if(!HAS_RELIGION(ch) )
	{	send_to_char("How do you expect to initiate someone into a religion, when you, yourself do not have one?\n\r",ch);
		return;
	}
	if(ch->pcdata->rank < RANK_PRIEST )
	{	send_to_char("You can't initiate people!\n\r",ch);
		return;
	}

	if(arg[0] == '\0' )
	{	send_to_char("Syntax: Initiate <person>\n\r",ch);
		if(ch->pcdata->rank > RANK_PRIEST )
			send_to_char("Syntax:  Initiate <person> faction\n\r",ch);
		return;
	}

	if(ch->pcdata->rank < RANK_PRIEST )
	{	send_to_char("You can't initiate people!\n\r",ch);
		return;
	}

	if(!IS_IMMORTAL(ch) )
	{	if( ( victim = get_char_room(ch, NULL, arg ) ) == NULL )
		{	send_to_char("They arn't here.\n\r",ch);
			return;
		}
	}
	else
	{	if( ( victim = get_char_world(ch, arg ) ) == NULL )
		{	send_to_char("They arn't here.\n\r", ch);
			return;
		}
	}
	if(IS_NPC(victim) )
	{	send_to_char("Not on NPC's.\n\r",ch);
		return;
	}
	if(arg2[0] == '\0' )
	{	if( HAS_RELIGION(victim) )
		{	send_to_char("They are already in a religion!\n\r",ch);
			return;
		}
		printf_to_char(ch, "They have been initiated.\n\r" );
		join_religion(victim, ch->pcdata->religion );
	}
	if(!str_cmp(arg2, "faction" ) )
	{	if( ch->pcdata->rank < RANK_DEITY 
		|| (!IN_FACTION(ch) && ch->pcdata->rank < RANK_GOD ) )
		{	send_to_char("You cannot initiate someone into a faction!\n\r",ch);
			return;
		}
		if(!ch->pcdata->religion->faction[FACTION_ONE] )
		{	send_to_char("You don't have any factions.\n\r",ch);
			return;
		}

		if(!str_cmp(arg3, "one" ) )
		{	if(ch->pcdata->rank != RANK_GOD )
			{	send_to_char("Only Gods can toy with peoples faith.\n\r",ch);
				return;
			}
			send_to_char("They have been swiched to the first Faction of your Religion.\n\r",ch);
			denounce_faith(victim, TRUE );
			join_religion(victim, ch->pcdata->religion->faction[FACTION_ONE] );
			return;
		}
		if(!str_cmp(arg3, "two" ) )
		{	if(ch->pcdata->rank != RANK_GOD )
			{	send_to_char("Only Gods can toy with peoples faith.\n\r",ch);
				return;
			}
			if(!ch->pcdata->religion->faction[FACTION_TWO] )
			{	send_to_char("You don't have a second faction.\n\r",ch);
				return;
			}
			send_to_char("They have been swiched to the second Faction of your Religion.\n\r",ch);
			denounce_faith(victim, TRUE );
			join_religion(victim, ch->pcdata->religion->faction[FACTION_TWO] );
			return;
		}
		if(!same_main_relg(ch, victim) )
		{	send_to_char("You atleast have to have the same main religion!\n\r",ch);
			return;
		}
		send_to_char("They have joined your Faction.\n\r",ch);
		denounce_faith(victim, TRUE );
		join_religion(victim, ch->pcdata->religion );
	}
	return;
}

/*This is made so the first time you create a 
  religion you can make yourself a god! Make sure 
  only responsable people have this. */

void do_makegod(CHAR_DATA *ch, char *argument ) 
{	char arg[MSL];
	RELIGION *pRlg;

	argument = one_argument(argument, arg);

	if(arg[0] == '\0' )
	{	send_to_char("Syntax: makegod <religion>\n\r",ch);
		return;
	}

	if( ( pRlg = religion_lookup(arg) ) == NULL )
	{	if( (pRlg = faction_lookup(arg) ) == NULL )
		{	send_to_char("That isn't a valid religion, or faction.\n\r",ch);
			return;
		}
	}

	ch->pcdata->religion = pRlg;
	ch->pcdata->rank = RANK_GOD;
	return;
}
/*End of do_functions*/

/*Stuff for joining/leaving a religion or Faction */

void join_religion(CHAR_DATA *ch, RELIGION *pRlg )
{	if(IS_NPC(ch) )
		return;
	if(ch->pcdata->religion )
	{	printf_to_char(ch, "You must denounce your faith to %s first.\n\r",ch->pcdata->religion->name );
		return;
	}
	
	ch->pcdata->religion = pRlg;
	printf_to_char(ch, "You now find solace in %s.\n\r", pRlg->name );
	ch->pcdata->bless = 0;
	ch->pcdata->rank = 0;
}

void denounce_faith(CHAR_DATA *ch, bool force )
{	int rank, sn;
	if(IS_NPC(ch) )
		return;

	if(!ch->pcdata->religion )
	{	send_to_char("You cannot denounce your faith when you do not have a faith!\n\r",ch);
		return;
	}

	if(!force)
		printf_to_char(ch, "You denounce %s as your faith, and choose the path of a heathen.\n\r", ch->pcdata->religion->name );
	else
		printf_to_char(ch, "You have been excommunicated from your faith!  You are now of the heathen.\n\r" );
	ch->pcdata->bless = 0;
	for( rank = ch->pcdata->rank; rank >= 0; rank-- )
	{	sn = skill_lookup(ch->pcdata->religion->skpell[prime_class(ch)][rank]);
		if( ch->pcdata->learned[sn] >= 1 )
		{	printf_to_char(ch, "You loose your %s ability.\n\r", skill_table[sn].name );
			ch->pcdata->learned[sn] = 0;
		}
	}
	ch->pcdata->religion = NULL;
	ch->pcdata->rank = 0;
	return;
}

/*Sacrifice and bless points stuff */
void religious_sacrifice(CHAR_DATA *ch, OBJ_DATA *obj )
{	act_new(ch->pcdata->religion->sac_msg, ch, obj, NULL, TO_CHAR, POS_RESTING );
	ch->pcdata->bless += ch->pcdata->rank;
	extract_obj(obj);
}
/*Begining of Recycling functions*/
RELIGION *relg_free;
#define STRING_NEW(string)           \
				free_string(string);  \
				string = str_dup("Not Set"); \

RELIGION *new_religion(void)
{
    static RELIGION relg_zero;
    RELIGION *pRlg;
	int rank, faction, iClass;

    if (relg_free == NULL)
        pRlg = (RELIGION *)alloc_perm(sizeof(*pRlg));
    else
    {
        pRlg = relg_free;
        relg_free = relg_free->next;
    }
	*pRlg = relg_zero;
	

	
	pRlg->isfaction = FALSE;

	STRING_NEW(pRlg->pmt_msg);
	STRING_NEW(pRlg->dmt_msg);
	STRING_NEW(pRlg->chan_name);
	STRING_NEW(pRlg->sac_msg );
	STRING_NEW(pRlg->god);
	STRING_NEW(pRlg->deity);
	STRING_NEW(pRlg->name );

	for(rank = 0; rank < MAX_RELG_RANK ; rank++ )
	{	for(iClass = 0; iClass < MAX_CLASS ; iClass++ )
		{	free_string(pRlg->skpell[iClass][rank]);
			pRlg->skpell[iClass][rank] = str_dup("reserved");
			free_string(pRlg->rank[iClass][rank]);
			pRlg->rank[iClass][rank] = str_dup(relgrank_table[rank].name );
		}
	}
	for(faction = 0; faction < MAX_FACTION ; faction++ )
		pRlg->faction[faction] = NULL;


    VALIDATE(pRlg);
    return pRlg;
}


void free_religion(RELIGION *pRlg)
{	int faction, rank, iClass;

	if (!IS_VALID(pRlg))
        return;

	free_string(pRlg->name);
	
	pRlg->isfaction = FALSE;
	free_string(pRlg->deity);
	free_string(pRlg->god);
	
	for(rank = 0; rank < MAX_RELG_RANK ; rank++ )
		for(iClass = 0; iClass < MAX_CLASS ; iClass++ )
		{	free_string(pRlg->skpell[iClass][rank]);
			free_string(pRlg->rank[iClass][rank]);
		}

	for(faction = 0; faction < MAX_FACTION ; faction++ )
		pRlg->faction[faction] = NULL;

	pRlg->temple = NULL;
	pRlg->donation_vnum = 1;
    INVALIDATE(pRlg);
    pRlg->next = relg_free;
    relg_free = pRlg;
}
/*End of Recycling functions*/
/*Begining of lookup functions*/
int god_lookup ( const char *name )
{
    int god;

    for ( god = 0 ; god_table[god].name != NULL ; god++ )
    {
        if (LOWER(name[0]) == LOWER(god_table[god].name[0])
        &&  !str_prefix(name,god_table[god].name))
            return god;
    }
    return -1;
}

int relgrank_lookup ( const char *name )
{
    int relgrank;

    for ( relgrank = 0 ; relgrank_table[relgrank].name != NULL ; relgrank++ )
    {
        if (LOWER(name[0]) == LOWER(relgrank_table[relgrank].name[0])
        &&  !str_prefix(name,relgrank_table[relgrank].name))
            return relgrank;
    }
    return -1;
}

RELIGION *religion_lookup ( const char *name )
{
    RELIGION * pRlg;

    for ( pRlg = religion_list ; pRlg ; pRlg = pRlg->next )
    {
        if (LOWER(name[0]) == LOWER(pRlg->name[0])
        &&  !str_prefix(name,pRlg->name))
            return pRlg;
    }
    return NULL;
}

RELIGION *faction_lookup ( const char *name )
{
    RELIGION * pRlg;
	int i;

    for ( pRlg = religion_list ; pRlg ; pRlg = pRlg->next )
    {	for(i = 0; i < MAX_FACTION; i++ )
		{	if(!pRlg->faction[i] )
				continue;

			if (LOWER(name[0]) == LOWER(pRlg->faction[i]->name[0])
			&&  !str_prefix(name,pRlg->faction[i]->name))
				return pRlg->faction[i];
		}
    }
    return NULL;
}
/*End of Lookup Functions */
/*Begining of Structures */
struct sex_type relgrank_table[] =
{
	{  "Initiate" },
	{  "Acolyte"  },
	{  "Disciple" },
	{  "Bishop"	  },
	{  "Priest"   },
	{  "Deity"    },
	{  "God"	  }
};

struct sex_type god_table[] = 
{
	{ "Davion" },
	{ "Synon"  },
	{ "Sola"   },
	{ NULL	   },
};

/*End of Structures */


char *god_name(CHAR_DATA *ch )
{	if( religion_lookup(ch->pcdata->religion->name ) != NULL )
		return ch->pcdata->religion->god;
	if( faction_lookup(ch->pcdata->religion->name ) != NULL )
		return ch->pcdata->religion->deity;
	return "ShadowStorm";
}
/*This function was written to be able to tell if someone is in the same Religion
  But a different Faction of it */
bool same_main_relg(CHAR_DATA *ch, CHAR_DATA *victim )
{	if(IN_FACTION(ch) && IN_FACTION(victim))
		if(ch->pcdata->religion->main == victim->pcdata->religion->main )
			return TRUE;
	if(IN_FACTION(ch) && !IN_FACTION(victim) )
		if(ch->pcdata->religion->main == victim->pcdata->religion )
			return TRUE;
	if(!IN_FACTION(ch) && IN_FACTION(victim) )
		if(ch->pcdata->religion == victim->pcdata->religion->main )
			return TRUE;

	return FALSE;
}
