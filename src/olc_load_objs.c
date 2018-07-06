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

/*
 * Snarf an obj section. Enzo style
 */
void fread_one_object( FILE *fp )
{
    OBJ_INDEX_DATA *pOI;
	const char *string;
	char letter;
    char buf[MSL];
    bool fMatch;
	long iHash;

    CREATE( pOI, OBJ_INDEX_DATA, 1 );
	pOI->value[0] = 0;
	pOI->value[1] = 0;
	pOI->value[2] = 0;
	pOI->value[3] = 0;
	pOI->value[4] = 0;

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
			case 'A':
				if (!str_cmp( string, "AffectObj"))
				{
					AFFECT_DATA *paf;
					paf                     = (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
					paf->where				= TO_OBJECT;
					paf->type               = -1;
					paf->level              = pOI->level;
					paf->duration           = -1;
					paf->location           = fread_number( fp );
					paf->modifier           = fread_number( fp );
					paf->bitvector          = 0;
					paf->next               = pOI->affected;
					pOI->affected     = paf;
					top_affect++;
				}
				break;
			case 'C':
				KEY( "Cost",	pOI->cost, fread_number( fp ) );
				if (!str_cmp( string, "Condition"))
				{
					letter 				= fread_letter( fp );
					switch (letter)
 					{
						case ('P') :		pOI->condition = 100; break;
						case ('G') :		pOI->condition =  90; break;
						case ('A') :		pOI->condition =  75; break;
						case ('W') :		pOI->condition =  50; break;
						case ('D') :		pOI->condition =  25; break;
						case ('B') :		pOI->condition =  10; break;
 						case ('R') :		pOI->condition =   0; break;
						default:			pOI->condition = 100; break;
					}
				}
				break;
			case 'D':
				KEY( "Desc",	pOI->description,		fread_string( fp ));
				break;
			case 'E':
				if ( !str_cmp( string, "EndObj" ) )
				{
					pOI->area                 = area_last;            /* OLC */
					sprintf(buf,"End obj %d\n",pOI->vnum);
					iHash                   = pOI->vnum % MAX_KEY_HASH;
					pOI->next         = obj_index_hash[iHash];
					obj_index_hash[iHash]   = pOI;
					top_obj_index++;
					top_vnum_obj = top_vnum_obj < pOI->vnum ? pOI->vnum : top_vnum_obj;   /* OLC */
					assign_area_vnum( pOI->vnum );                                   /* OLC */
					append_new_save(NEW_SAVE_TEST,buf);
					get_obj_index(pOI->vnum);
					return;
				}
                                KEY( "Exp",     pOI->exp,               fread_number( fp ));
				KEY( "Extra",	pOI->extra_flags,	fread_flag( fp ));
                                KEY( "Extra2",  pOI->extra2_flags,      fread_flag( fp ));
				if ( !str_cmp( string, "ExtraDesc" ) )      
				{
					EXTRA_DESCR_DATA *ed;
					
					ed                      = (EXTRA_DESCR_DATA *)alloc_perm( sizeof(*ed) );
					ed->keyword             = fread_string( fp );
					ed->description         = fread_string( fp );
					sprintf(buf,"ED->name %s, ED->desc %s",ed->keyword,ed->description);
					append_new_save(NEW_SAVE_TEST,buf);
					ed->next                = pOI->extra_descr;
					pOI->extra_descr  = ed;
					top_ed++;
				}
				break;
			case 'F':				
				if (!str_cmp( string, "Faff"))
				{
					AFFECT_DATA *paf;
					paf                     = (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
					paf->where				= TO_AFFECTS;
					paf->type               = -1;
					paf->level              = pOI->level;
					paf->duration           = -1;
					paf->location           = fread_number(fp);
					paf->modifier           = fread_number(fp);
					paf->bitvector          = fread_flag(fp);
					paf->next               = pOI->affected;
					pOI->affected			= paf;
					top_affect++;
				}
				if (!str_cmp( string,"Fimm"))
				{
					AFFECT_DATA *paf;
					paf                     = (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
					paf->where				= TO_IMMUNE;
					paf->type               = -1;
					paf->level              = pOI->level;
					paf->duration           = -1;
					paf->location           = fread_number(fp);
					paf->modifier           = fread_number(fp);
					paf->bitvector          = fread_flag(fp);
					paf->next               = pOI->affected;
					pOI->affected			= paf;
					top_affect++;
				}
				if (!str_cmp( string,"Fres"))
				{
					AFFECT_DATA *paf;
					paf                     = (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
					paf->where				= TO_RESIST;
					paf->type               = -1;
					paf->level              = pOI->level;
					paf->duration           = -1;
					paf->location           = fread_number(fp);
					paf->modifier           = fread_number(fp);
					paf->bitvector          = fread_flag(fp);
					paf->next               = pOI->affected;
					pOI->affected			= paf;
					top_affect++;
				}
				if (!str_cmp( string, "Fvul"))
				{
					AFFECT_DATA *paf;
					paf                     = (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
					paf->where				= TO_VULN;
					paf->type               = -1;
					paf->level              = pOI->level;
					paf->duration           = -1;
					paf->location           = fread_number(fp);
					paf->modifier           = fread_number(fp);
					paf->bitvector          = fread_flag(fp);
					paf->next               = pOI->affected;
					pOI->affected			= paf;
					top_affect++;
				}
				break;
			case 'I':
				if (!str_cmp( string, "Item_type"))
				{
					pOI->item_type = item_lookup(fread_word( fp ));
				}
				break;
			case 'L':
				KEY( "Level",	pOI->level,			fread_number( fp ));
				break;
			case 'M':
				KEY( "Material",	pOI->material,		fread_string( fp ));
				break;
			case 'N':
				if (!str_cmp( string, "Name"))
				{
					pOI->name	=	fread_string( fp );
					sprintf(buf,"obj->name %s",pOI->name);
					append_new_save(NEW_SAVE_TEST,buf);
				}
				break;

			case 'O':
				if ( !str_cmp(string, "Oprg" ) )
				{
				    PROG_LIST *pOprog;
				    char *word;
				    int trigger = 0;

				    pOprog			= (PROG_LIST *)alloc_perm(sizeof(*pOprog));
				    word			= fread_word( fp );
				    if ( !(trigger = flag_lookup( word, oprog_flags )) )
				    {
					bug( "OBJprogs: invalid trigger.",0);
					exit(1);
				    }
				    SET_BIT( pOI->oprog_flags, trigger );
				    pOprog->trig_type	= trigger;
				    pOprog->vnum	 	= fread_number( fp );
				    pOprog->trig_phrase	= fread_string( fp );
				    pOprog->next		= pOI->oprogs;
				    pOI->oprogs	= pOprog;
				    break;
				}
				break;

                        case 'P':
                                KEY( "Plevel",          pOI->plevel,           fread_number( fp ) );
                                break;
			case 'S':
				KEY( "Short",		pOI->short_descr,		fread_string( fp ));
				break;
			case 'V':
				if (!str_cmp( string, "Vnum"))
				{
					pOI->vnum	= fread_number( fp );
					pOI->new_format = TRUE;
					sprintf(buf,"obj->vnum %d",pOI->vnum);
					append_new_save(NEW_SAVE_TEST,buf);
				}
				if (!str_cmp( string, "Values"))
				{
					switch(pOI->item_type)
					{
					case ITEM_WEAPON:
						pOI->value[0]		= weapon_type(fread_word(fp));
						pOI->value[1]		= fread_number(fp);
						pOI->value[2]		= fread_number(fp);
						pOI->value[3]		= attack_lookup(fread_word(fp));
						pOI->value[4]		= fread_flag(fp);
						break;
					case ITEM_CONTAINER:
						pOI->value[0]         = fread_number(fp);
						pOI->value[1]         = fread_flag(fp);
						pOI->value[2]		  = fread_number(fp);
						pOI->value[3]         = fread_number(fp);
						pOI->value[4]         = fread_number(fp);
						break;
					case ITEM_DRINK_CON:
					case ITEM_FOUNTAIN:
						pOI->value[0]         = fread_number(fp);
						pOI->value[1]         = fread_number(fp);
						CHECK_POS(pOI->value[2], liq_lookup(fread_word(fp)), "liq_lookup" );
						pOI->value[3]         = fread_number(fp);
						pOI->value[4]         = fread_number(fp);
						break;
					case ITEM_WAND:
					case ITEM_STAFF:
						pOI->value[0]		= fread_number(fp);
						pOI->value[1]		= fread_number(fp);
						pOI->value[2]		= fread_number(fp);
						pOI->value[3]		= skill_lookup(fread_word(fp));
						pOI->value[4]		= fread_number(fp);
						break;
					case ITEM_POTION:
					case ITEM_PILL:
					case ITEM_SCROLL:
 						pOI->value[0]		= fread_number(fp);
						pOI->value[1]		= skill_lookup(fread_word(fp));
						pOI->value[2]		= skill_lookup(fread_word(fp));
						pOI->value[3]		= skill_lookup(fread_word(fp));
						pOI->value[4]		= skill_lookup(fread_word(fp));
						break;
			        case ITEM_QUIVER:
				    case ITEM_ARROW:
						pOI->value[0]             = fread_number(fp);
						pOI->value[1]         = fread_number(fp);
	                    pOI->value[2]         = fread_number(fp);
		                pOI->value[3]         = fread_number(fp);
			            pOI->value[4]         = fread_number(fp);
				        break;
			        case ITEM_SHEATH:
		                pOI->value[0]     = fread_number(fp);
				        pOI->value[1]     = fread_number(fp);
		                pOI->value[2]     = fread_number(fp);
				        pOI->value[3]     = fread_number(fp);
					    break;
			        case ITEM_BELT:
			            pOI->value[0]         = fread_number(fp);
			            pOI->value[1]         = fread_number(fp);
			            break;

					default:
						pOI->value[0]		= fread_flag( fp );
						pOI->value[1]		= fread_flag( fp );
						pOI->value[2]		= fread_flag( fp );
						pOI->value[3]		= fread_flag( fp );
						pOI->value[4]		= fread_flag( fp );
						break;
					}
				}
				break;
			case 'W':
				KEY( "Wear",	pOI->wear_flags,		fread_flag( fp ));
				KEY( "Weight",	pOI->weight,			fread_number( fp ));
				break;

                        case 'X':
                                KEY( "Xptolevel", pOI->xp_tolevel,             fread_number( fp ) );
                                break;

			break;
		}
	}
}

/*
 * Append a string to a file.
 */
void append_new_save( char *file, char *str )
{
    FILE *fp;

    if (!file_exists(file))
	perror(file);
    else
    {
	fp = file_open(file, "a");
	/*fprintf(fp, "%s\n", str);*/
	file_close(fp);
    }

    return;
}

void fread_objects( FILE *fp )
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
		else if ( !str_cmp( word, "OBJ" ) )
	    {
			sprintf(buf,"#OBJ found" );
			append_new_save(NEW_SAVE_TEST,buf);
			fread_one_object( fp );
			continue;
	    }
	}
}

