/****************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,			*
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.	*
 *																			*
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael			*
 *  Chastain, Michael Quan, and Mitchell Tse.								*
 *																			*
 *  In order to use any part of this Merc Diku Mud, you must comply with	*
 *  both the original Diku license in 'license.doc' as well the Merc		*
 *  license in 'license.txt'.  In particular, you may not remove either of	*
 *  these copyright notices.												*
 *																			*
 *  Much time and thought has gone into this software and you are			*
 *  benefitting.  We hope that you share your changes too.  What goes		*
 *  around, comes around.													*
 ***************************************************************************/
 
/****************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor								*
*	ROM has been brought to you by the ROM consortium						*
*	    Russ Taylor (rtaylor@hypercube.org)									*
*	    Gabrielle Taylor (gtaylor@hypercube.org)							*
*	    Brian Moore (zump@rom.org)											*
*	By using this code, you have agreed to follow the terms of the			*
*	ROM license, in the file Rom24/doc/rom.license							*
***************************************************************************/


#if defined(macintosh)
#include <types.h>
#elif defined(WIN32)
#include <sys/types.h>
#include <time.h>
#define NOCRYPT
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "db.h"
#include "const.h"

#define DIF(a,b) (~((~a)|(b)))
/*stuff to set up key*/
#if defined(KEY)
#undef KEY
#endif

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value ) if ( !str_cmp( string, literal ) )	{ field  = value; fMatch = TRUE; break;	}

void fread_rooms( FILE *fp )
{
	char buf[MSL];
	for ( ;; )
	{
		char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
			fread_to_eol( fp );
			continue;
	    }

	    if ( letter != '#' )
	    {
			bug( "Load_skill_table: # not found.", 0 );
			break;
	    }

	    word = fread_word( fp );
		if ( !str_cmp( word, "END"	) )
		{
			sprintf(buf,"#END found\n\n" );
			append_new_save(NEW_SAVE_TEST,buf);
			return;
		}
		else if ( !str_cmp( word, "ROOM" ) )
	    {
			sprintf(buf,"#ROOM found" );
			append_new_save(NEW_SAVE_TEST,buf);
			fread_one_room( fp );
			continue;
	    }
	}
}

/*
 * Snarf a room section. Enzo Style.
 */
void fread_one_room( FILE *fp )
{
    ROOM_INDEX_DATA *pRI;

/*    if ( area_last == NULL )
    {
		bug( "Load_resets: no #AREA seen yet.", 0 );
		exit( 1 );
    }*/
	const char *string;
	char letter;
    char buf[MSL];
    bool fMatch;
	long iHash;
	int door;

    CREATE( pRI, ROOM_INDEX_DATA, 1 );
	pRI->light			= 0;
	pRI->heal_rate		= 100;
	pRI->mana_rate		= 100;
	pRI->owner			= str_dup("");
	pRI->people			= NULL;
	pRI->contents		= NULL;
	pRI->extra_descr	= NULL;
	pRI->clan			= 0;
	pRI->area			= area_last;
	pRI->map_next		= NULL;

	for ( door = 0; door <= 5; door++ )
	    pRI->exit[door] = NULL;

    for ( ; ; )
    {
		string   = feof( fp ) ? "End" : fread_word( fp );
		fMatch = FALSE;
		switch ( UPPER(string[0]) )
		{
			case '*':
				fMatch = TRUE;
				fread_to_eol( fp );
				break;
			case 'D':
				KEY( "Desc",	pRI->description,	fread_string( fp ));
				break;
			case 'E':
				if (!str_cmp(string, "EndRoom"))
				{	pRI->area                 = area_last;            /* OLC */
					sprintf(buf,"EndRoom %d\n",pRI->vnum);
					iHash                   = pRI->vnum % MAX_KEY_HASH;
					pRI->next         = room_index_hash[iHash];
					room_index_hash[iHash]   = pRI;
					top_room++;
					top_vnum_room = top_vnum_room < pRI->vnum ? pRI->vnum : top_vnum_room;	/* OLC */
					assign_area_vnum( pRI->vnum );	/* OLC */
					append_new_save(NEW_SAVE_TEST,buf);
					get_room_index(pRI->vnum);
					return;
				}
				if (!str_cmp(string, "Exit"))
				{
					EXIT_DATA *pexit;

					door = fread_number( fp );
					if ( door < 0 || door > 5 )
					{
						bug( "Fread_rooms: vnum %d has bad door number.", pRI->vnum );
						exit( 1 );
					}
						pexit				= (EXIT_DATA *)alloc_perm( sizeof(*pexit) );
						pexit->description	= fread_string( fp );
						sprintf(buf,"Door %d Desc %s",door,pexit->description);
						append_new_save(NEW_SAVE_TEST,buf);
						pexit->keyword		= fread_string( fp );
						pexit->rs_flags		= 0;                    /* OLC */
						pexit->key			= fread_number( fp );
						pexit->u1.vnum		= fread_number( fp );
						pexit->exit_info		= fread_flag(fp);
						pexit->orig_door	= door;			/* OLC */
						pexit->rs_flags		= pexit->exit_info;
						pRI->exit[door]	= pexit;
						top_exit++;
				}
				if (!str_cmp(string, "ExtraDesc"))
				{
					EXTRA_DESCR_DATA *ed;

					ed					= (EXTRA_DESCR_DATA *)alloc_perm( sizeof(*ed) );
					ed->keyword			= fread_string( fp );
					ed->description		= fread_string( fp );
					ed->next			= pRI->extra_descr;
					pRI->extra_descr	= ed;
					top_ed++;
				}
				break;
			case 'H': /* healing room */
				KEY( "HealRate",	pRI->heal_rate,	fread_number( fp ));
				break;
			case 'M':
				KEY( "Mana Rate",	pRI->mana_rate,	fread_number( fp ));
				if(!str_cmp(string, "Mine" ) )
				    fread_mine(pRI, fp );
				break;
			case 'N':
				KEY( "Name",	pRI->name,		fread_string( fp ));
				break;

			case 'R':
				KEY( "Room_flags",	pRI->room_flags,	fread_flag( fp ));
				break;
			case 'S':
				KEY( "Sector_type",	pRI->sector_type,	fread_number( fp ));
				break;
			case 'V':
				if (!str_cmp( string, "Vnum"))
				{
					pRI->vnum	= fread_number( fp );
					sprintf(buf,"room->vnum %d",pRI->vnum);
					append_new_save(NEW_SAVE_TEST,buf);
					break;
				}
				break;
			case 'X':
				if (!str_cmp( string, "X"))
				{
					pRI->mineral[MIN_STEEL] = fread_number( fp );
					pRI->mineral[MIN_IRON]       = fread_number( fp );
					pRI->mineral[MIN_COPPER]     = fread_number( fp );
					pRI->mineral[MIN_ADAMANTIUM] = fread_number( fp );
					pRI->mineral[MIN_PLATINUM]   = fread_number( fp );
					pRI->mineral[MIN_BRONZE]     = fread_number( fp );
					pRI->mineral[MIN_SILVER]     = fread_number( fp );
					pRI->mineral[MIN_MARBLE]     = fread_number( fp );
					pRI->mineral[MIN_BRASS]      = fread_number( fp );
					pRI->mineral[MIN_GRANITE]    = fread_number( fp );
					pRI->mineral[MIN_ELECTRUM]   = fread_number( fp );
				}
				break;
		}
	}
	for ( ; ; )
	{
	    letter = fread_letter( fp );
		if ( letter == 'R' )
	    {
			PROG_LIST *pRprog;
			char *word;
			int trigger = 0;

			pRprog		= (PROG_LIST *)alloc_perm(sizeof(*pRprog));
			word		= fread_word( fp );
			if ( !(trigger = flag_lookup( word, rprog_flags )) )
			{
				bug( "ROOMprogs: invalid trigger.",0);
				exit(1);
			}
			SET_BIT( pRI->rprog_flags, trigger );
			pRprog->trig_type	= trigger;
			pRprog->vnum		= fread_number( fp );
			pRprog->trig_phrase	= fread_string( fp );
			pRprog->next		= pRI->rprogs;
			pRI->rprogs	= pRprog;
	    }

	    else
	    {
		bug( "Load_rooms: vnum %d has flag not 'DES'.", pRI->vnum );
		exit( 1 );
	    }
	}
    return;
}
