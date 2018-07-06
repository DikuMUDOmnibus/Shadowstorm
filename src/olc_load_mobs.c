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

void fread_mobiles( FILE *fp )
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
			bug( "Load_mobiles: # not found.", 0 );
			break;
	    }

	    word = fread_word( fp );
		if ( !str_cmp( word, "END"	) )
		{
			sprintf(buf,"#END found\n\n" );
			append_new_save(NEW_SAVE_TEST,buf);
			return;
		}
		else if ( !str_cmp( word, "MOB" ) )
	    {
			sprintf(buf,"#MOB found" );
			append_new_save(NEW_SAVE_TEST,buf);
			fread_one_mobile( fp );
			continue;
	    }
	}
}


/*
 * Snarf a mob section.  new style
 */
void fread_one_mobile( FILE *fp )
{
    MOB_INDEX_DATA *pMI;
	const char *string;
    char buf[MSL];
    bool fMatch;
	long iHash;
	long vector;

    CREATE( pMI, MOB_INDEX_DATA, 1 );

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
                                KEY( "Aggression", pMI->aggression,     fread_number( fp ) );
				KEY( "Align",	pMI->alignment,			fread_number( fp ));
				if (!str_cmp(string,"Armor"))
				{
					/* read armor klass */
					pMI->ac[AC_PIERCE]	= fread_number( fp ) * 10;
					pMI->ac[AC_BASH]		= fread_number( fp ) * 10;
					pMI->ac[AC_SLASH]		= fread_number( fp ) * 10;
					pMI->ac[AC_EXOTIC]	= fread_number( fp ) * 10;
				}
				if (!str_cmp(string,"Act_flags"))
				{
					pMI->act                  = fread_flag( fp ) 
						| ACT_IS_NPC
						| race_table[pMI->race].act;
				}
				if (!str_cmp(string, "Aff"))
				{
					pMI->affected_by          = fread_flag( fp )
						| race_table[pMI->race].aff;
				}
				break;
                        case 'C':
                                KEY( "Clevel",          pMI->cast_level,   fread_number( fp ) );
                                KEY( "Cability",           pMI->cast_ability, fread_number( fp ) );
                                break;
			case 'D':
				KEY( "Dam_type",	pMI->dam_type,		attack_lookup(fread_word(fp)));
				KEY( "Default_pos",	pMI->default_pos,	position_lookup(fread_word(fp)));
				KEY( "Description",	pMI->description,	fread_string( fp ));
				if (!str_cmp(string,"Dam_dice"))
				{	
					/* read damage dice */
					pMI->damage[DICE_NUMBER]	= fread_number( fp );
					fread_letter( fp );
					pMI->damage[DICE_TYPE]	= fread_number( fp );
					fread_letter( fp );
					pMI->damage[DICE_BONUS]	= fread_number( fp );
				}
				break;
			case 'E':
				if ( !str_cmp( string, "EndMOB" ) )
				{
					pMI->area                 = area_last;            /* OLC */
					sprintf(buf,"EndMob %d\n",pMI->vnum);
					iHash                   = pMI->vnum % MAX_KEY_HASH;
					pMI->next         = mob_index_hash[iHash];
					mob_index_hash[iHash]   = pMI;
					top_mob_index++;
					top_vnum_mob = top_vnum_mob < pMI->vnum ? pMI->vnum : top_vnum_mob;   /* OLC */
					assign_area_vnum( pMI->vnum );                                   /* OLC */
					append_new_save(NEW_SAVE_TEST,buf);
					get_mob_index(pMI->vnum);
					return;
				}
			case 'F':
				if (!str_prefix(string,"Fact"))
				{
					vector		= fread_flag(fp);
					REMOVE_BIT(pMI->act,vector);
				}
				if (!str_prefix(string,"Faff1"))
				{
					vector		= fread_flag(fp);
					REMOVE_BIT(pMI->affected_by,vector);
				}
				if (!str_prefix(string,"Foff"))
				{
					vector		= fread_flag(fp);
					REMOVE_BIT(pMI->off_flags,vector);
				}
				if (!str_prefix(string,"Fimm"))
				{
					vector		= fread_flag(fp);
					REMOVE_BIT(pMI->imm_flags,vector);
				}
				if (!str_prefix(string,"Fres"))
				{
					vector		= fread_flag(fp);
					REMOVE_BIT(pMI->res_flags,vector);
				}
				if (!str_prefix(string,"Fvul"))
				{
					vector		= fread_flag(fp);
					REMOVE_BIT(pMI->vuln_flags,vector);
				}
				if (!str_prefix(string,"Ffor"))
				{
					vector		= fread_flag(fp);
					REMOVE_BIT(pMI->form,vector);
				}
				if (!str_prefix(string,"Fpar"))
				{
					vector		= fread_flag(fp);
					REMOVE_BIT(pMI->parts,vector);
				}
				if (!str_cmp(string,"Form"))
				{
					pMI->form = fread_flag( fp )
						| race_table[pMI->race].form;
				}
				break;
			case 'G':
				KEY( "Group",	pMI->group,			fread_number( fp ));
				break;
			case 'H':
				KEY( "Hitroll",	pMI->hitroll,		fread_number( fp ));
				if (!str_cmp(string,"Hit_dice"))
				{		
					/* read hit dice */
					pMI->hit[DICE_NUMBER]     = fread_number( fp );  
					/* 'd'          */                fread_letter( fp ); 
					pMI->hit[DICE_TYPE]   	= fread_number( fp );
					/* '+'          */                fread_letter( fp );   
					pMI->hit[DICE_BONUS]      = fread_number( fp ); 
				}
				break;
			case 'I':
				if (!str_cmp(string,"Imm"))
				{
					pMI->imm_flags		= fread_flag( fp )
						| race_table[pMI->race].imm;
				}
				break;
			case 'L':
				KEY( "Level",	pMI->level,			fread_number( fp ));
				KEY( "Long",	pMI->long_descr,	fread_string( fp ));
				break;
			case 'M':
				if (!str_cmp(string,"Mana_dice"))
				{
					/* read mana dice */
					pMI->mana[DICE_NUMBER]	= fread_number( fp );
					fread_letter( fp );
					pMI->mana[DICE_TYPE]	= fread_number( fp );
					fread_letter( fp );
					pMI->mana[DICE_BONUS]	= fread_number( fp );
				}
				KEY( "Material",	pMI->material,		str_dup(fread_word( fp )));
				if (!str_cmp(string,"MobTrig"))
				{
					PROG_LIST *pMprog;
					char *word;
					int trigger = 0;
					
					pMprog              = (PROG_LIST *)alloc_perm(sizeof(*pMprog));
					word   		    = fread_word( fp );
					if ( !(trigger = flag_lookup( word, mprog_flags )) )
					{
						bug("MOBprogs: invalid trigger.",0);
						exit(1);
					}
					SET_BIT( pMI->mprog_flags, trigger );
					pMprog->trig_type   = trigger;
					pMprog->vnum        = fread_number( fp );
					pMprog->trig_phrase = fread_string( fp );
					pMprog->next        = pMI->mprogs;
					pMI->mprogs   = pMprog;
				}
				break;
			case 'N':
				KEY( "Name",	pMI->player_name,	fread_string( fp ));
				break;
			case 'O':
				if (!str_cmp(string,"Off"))
				{
					pMI->off_flags		= fread_flag( fp )
						| race_table[pMI->race].off;
				}
				break;
			case 'P':
				if (!str_cmp(string,"Parts"))
				{
					pMI->parts	=	fread_flag( fp )
						| race_table[pMI->race].parts;
				}
				break;
			case 'R':
				KEY( "Race",	pMI->race,			race_lookup(fread_string( fp )));
				if (!str_cmp(string,"Res"))
				{	
					pMI->res_flags		= fread_flag( fp )	
						| race_table[pMI->race].res;
				}
				break;
			case 'S':
				KEY( "Short",		pMI->short_descr,	fread_string( fp ));
				KEY( "Start_pos",	pMI->start_pos,		position_lookup(fread_word(fp)));
				KEY( "Sex",			pMI->sex,			sex_lookup(fread_word(fp)));
				if (!str_cmp(string,"Size"))
				{
					CHECK_POS( pMI->size, size_lookup(fread_word(fp)), "size" );
				}
				break;
			case 'V':
				if (!str_cmp(string,"Vuln"))
				{
					pMI->vuln_flags		= fread_flag( fp )
						| race_table[pMI->race].vuln;
				}
				if (!str_cmp( string, "Vnum"))
				{
					pMI->vnum	= fread_number( fp );
					sprintf(buf,"mob->vnum %d",pMI->vnum);
					append_new_save(NEW_SAVE_TEST,buf);
					pMI->new_format = TRUE;
				}
				break;
			case 'W':
				KEY( "Wealth",	pMI->wealth,	fread_number( fp ));
				break;
                        case 'X':
                                KEY( "Xp_tolevel", pMI->xp_tolevel, fread_number( fp ) );
                                break;  
			break;
		}
    }
	return;
}
