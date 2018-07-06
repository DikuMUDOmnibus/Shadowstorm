 /***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.	   *		
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
#include "const.h"

#if defined(KEY)
#undef KEY
#endif

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )  if ( !str_cmp( word, literal ) ) { field  = value; fMatch = TRUE; break; }


struct skill_type *skill_table;

void fwrite_skills(FILE *fp, int sn)
{
	char buf[MSL];
	int i;

	for(i = 0 ; ; i++ )
	{
		if(spellfun_table[i].name == NULL)
			break;
		if(spellfun_table[i].spell_fun == skill_table[sn].spell_fun)
			sprintf(buf, "%s", spellfun_table[i].name );
	}


	fprintf( fp, "Name %s~\n", skill_table[sn].name);
    fprintf( fp, "Levl ");
	for (i = 0 ;i < MAX_CLASS ; i++)
		fprintf(fp, "%d ", skill_table[sn].skill_level[i] );
	fprintf( fp, "\n");
	fprintf( fp, "Rate ");
	for (i = 0 ;i < MAX_CLASS ; i++)
		fprintf(fp, "%d ", skill_table[sn].rating[i] );
	fprintf( fp, "\n");
	fprintf( fp, "Spfun %s~\n", buf);
	fprintf( fp, "Targ %d\n", skill_table[sn].target);
	fprintf( fp, "Minp %d\n", skill_table[sn].minimum_position );
	fprintf( fp, "Mmna %d\n", skill_table[sn].min_mana);
	fprintf( fp, "Beat %d\n", skill_table[sn].beats);
	fprintf( fp, "Ndam %s~\n", skill_table[sn].noun_damage);
	fprintf( fp, "Moff %s~\n", skill_table[sn].msg_off);
	fprintf( fp, "Mobj %s~\n", skill_table[sn].msg_obj);
	fprintf( fp, "END\n\n");
	
	
	return;
}

void save_skills_table()
{
	FILE *fp;
	int i;
	char buf[MSL];
	
	sprintf(buf, "%s%s", SKILL_DIR, SKILLDAT);
	
	if ( ( fp = file_open( buf, "w") ) == NULL)
	{
		bug( "Skillfile: fopen", 0 );
		perror(buf);
	}
	
	fprintf (fp, "%d\n", MAX_SKILL);
	
	for ( i = 0 ; i < MAX_SKILL ; i++)
		fwrite_skills(fp, i);
		
	file_close (fp);
}

void load_skill(FILE *fp, struct skill_type *skill)
{
	char sfun[MSL];
	int i;
	const char *word;
	bool fMatch = FALSE, sMatch = FALSE;
 
	for (;;)
    {
        word = feof (fp) ? "End" : fread_word (fp);
        fMatch = FALSE;
        switch (UPPER (word[0]))
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol (fp);
                break;
			
			case 'B':
				KEY( "Beat", skill->beats, fread_number(fp) );
				break;
			case 'E':
                if (!str_cmp (word, "End"))
					return;
			case 'L':
				if( !str_cmp( word, "Levl"))
				{
					for( i = 0; i < MAX_CLASS ; i++ )
						skill->skill_level[i] = fread_number(fp );
				}
				break;
			case 'M':
				KEY( "Moff", skill->msg_off, fread_string(fp) );
				KEY( "Mobj", skill->msg_obj, fread_string(fp) );
				KEY( "Minp", skill->minimum_position, fread_number(fp) );
				KEY( "Mmna", skill->min_mana, fread_number(fp) );
				break;
			case 'N':
				KEY( "Name", skill->name, fread_string(fp) );
				KEY( "Ndam", skill->noun_damage, fread_string(fp) );
				break;
			case 'R':
				if( !str_cmp( word, "Rate"))
				{
					for( i = 0; i < MAX_CLASS ; i++ )
						skill->rating[i] = fread_number(fp );
				}
				break;
			case 'S':
				if( !str_cmp(word, "Spfun") )
				{
					sprintf(sfun, "%s", fread_string(fp) );
					for (i = 0; ;i++ )
					{
						if(spellfun_table[i].name == NULL)
							break;
						if(!strcmp(spellfun_table[i].name, sfun) )
						{
							sMatch = TRUE;
							break;
						}
					}
					if(!sMatch)
					{
						bug("Spell_fun not found!",0);
						bug(sfun,0);
					}

					skill->spell_fun = spellfun_table[i].spell_fun;
					break;
				}
				break;
			case 'T':
				KEY("Targ", skill->target, fread_number(fp) );
				break;
		}
	}


}

void load_skill_table()
{
	FILE *fp;
	int i, sn;
        char buf[MSL];
	
	for(sn=0;sn < MAX_SKILL; sn++)
	{
		skill_table[sn].name = str_dup("");
		for( i = 0 ; i < MAX_CLASS ; i++ )
		{
			skill_table[sn].skill_level[i] = 0;
			skill_table[sn].rating[i] = 0;
		}
		skill_table[sn].target = 0;
		skill_table[sn].minimum_position = 0;
		skill_table[sn].slot = 0;
		skill_table[sn].min_mana = 0;
		skill_table[sn].beats = 0;
		skill_table[sn].noun_damage = str_dup("");
		skill_table[sn].msg_off = str_dup("");
		skill_table[sn].msg_obj = str_dup("");
		skill_table[sn].spell_fun = NULL;
	}
	sprintf(buf, "%s%s", SKILL_DIR, SKILLDAT);	
	if ( ( fp = file_open( buf, "r") ) == NULL)
	{
		bug( "Skillfile: fopen", 0 );
		perror(buf);
	}
	
	fscanf (fp, "%d\n", &MAX_SKILL);

	/* IMPORTANT to use malloc so we can realloc later on */
		
	skill_table = (skill_type *)malloc (sizeof(struct skill_type) * (MAX_SKILL+1));
	
	for (i = 0; i < MAX_SKILL; i++)
		load_skill(fp,&skill_table[i]);

	/* For backwards compatibility */
			
	skill_table[MAX_SKILL].name = str_dup(""); /* empty! */		
		
	file_close(fp);
}

