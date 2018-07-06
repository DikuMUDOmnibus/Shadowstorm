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
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@hypercube.org)				   *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "string.h"
#include "magic.h"
#include "const.h"

int flag_lookup (const char *name, const struct flag_type *flag_table)
{
    int flag;

    for (flag = 0; flag_table[flag].name != NULL && flag_table[flag].name[0] != '\0'; flag++)
    {
	if (LOWER(name[0]) == LOWER(flag_table[flag].name[0])
	&&  !str_prefix(name,flag_table[flag].name))
	    return flag_table[flag].bit;
    }

    return NO_FLAG;
}

int position_lookup (const char *name)
{
   int pos;

   for (pos = 0; position_table[pos].name != NULL; pos++)
   {
	if (LOWER(name[0]) == LOWER(position_table[pos].name[0])
	&&  !str_prefix(name,position_table[pos].name))
	    return pos;
   }
   
   return -1;
}

int sex_lookup (const char *name)
{
   int sex;
   
   for (sex = 0; sex_table[sex].name != NULL; sex++)
   {
	if (LOWER(name[0]) == LOWER(sex_table[sex].name[0])
	&&  !str_prefix(name,sex_table[sex].name))
	    return sex;
   }

   return -1;
}

int size_lookup (const char *name)
{
   int size;
 
   for ( size = 0; size_table[size].name != NULL; size++)
   {
        if (LOWER(name[0]) == LOWER(size_table[size].name[0])
        &&  !str_prefix( name,size_table[size].name))
            return size;
   }
 
   return -1;
}


/* returns race number */
int race_lookup (const char *name)
{
   int race;

   for ( race = 0; race_table[race].name != NULL; race++)
   {
	if (LOWER(name[0]) == LOWER(race_table[race].name[0])
	&&  !str_prefix( name,race_table[race].name))
	    return race;
   }

   return 0;
} 

int item_lookup(const char *name)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
    {
        if (LOWER(name[0]) == LOWER(item_table[type].name[0])
        &&  !str_prefix(name,item_table[type].name))
            return item_table[type].type;
    }
 
    return -1;
}

int liq_lookup (const char *name)
{
    int liq;

    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
	if (LOWER(name[0]) == LOWER(liq_table[liq].liq_name[0])
	&& !str_prefix(name,liq_table[liq].liq_name))
	    return liq;
    }

    return -1;
}

HELP_DATA * help_lookup( char *keyword )
{
	HELP_DATA *pHelp;
	char temp[MIL], argall[MIL];

	argall[0] = '\0';

	while (keyword[0] != '\0' )
	{
		keyword = one_argument(keyword, temp);
		if (argall[0] != '\0')
			strcat(argall," ");
		strcat(argall, temp);
	}

	for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
		if ( is_name( argall, pHelp->keyword ) )
			return pHelp;

	return NULL;
}

int strap_lookup(const char *name)
{
    int type;

    for (type = 0; strap_table[type].name != NULL; type++)
    {
        if (LOWER(name[0]) == LOWER(strap_table[type].name[0])
        &&  !str_prefix(name,strap_table[type].name))
            return strap_table[type].bit;
    }
 
    return -1;
}



int cont_lookup(const char *name )
{
    int cont;

    for (cont = 0; cont < MAX_CONT; cont++)
    {
        if (LOWER(name[0]) == LOWER(cont_table[cont].name[0])
        &&  !str_prefix(name,cont_table[cont].name))
            return cont;
    }

    return -1;
}

int spellfun_lookup(const char *name )
{
    int cont;

    for (cont = 0; spellfun_table[cont].name != NULL; cont++)
    {
        if (LOWER(name[0]) == LOWER(spellfun_table[cont].name[0])
        &&  !str_prefix(name,spellfun_table[cont].name))
            return cont;
    }

    return 0;
}

int find_spellfun(SPELL_FUN *spell)
{
   int i;
   for(i = 0 ; spellfun_table[i].name != NULL ; i++ )
   {
	if(spellfun_table[i].spell_fun == spell )
	    return i;
   }

   return 0;
}

int target_lookup(const char *name)
{
    int i;
    for( i = 0; target_table[i].name != NULL; i++)
    {
	if(!str_cmp(target_table[i].name, name ) )
	    return i;
    }
    return -1;
}

EMAIL_DATA *email_lookup(const char *name )
{
    EMAIL_DATA *pMail;

    for ( pMail = email_first ; pMail ; pMail = pMail->next )
    {
	if(!str_cmp(pMail->name, name ) )
	    return pMail;
    }
    return NULL;
}

int mineral_lookup ( const char *name )
{
    int min;

    for ( min = 0 ; mineral_table[min].name != NULL ; min ++ )
    {
        if (LOWER(name[0]) == LOWER(mineral_table[min].name[0])
        &&  !str_prefix(name,mineral_table[min].name))
            return min;
    }
    return -1;
}

AREA_DATA *area_lookup ( const char *name )
{
    AREA_DATA * pArea;

    for ( pArea = area_first ; pArea ; pArea = pArea->next )
    {
        if (LOWER(name[0]) == LOWER(pArea->name[0])
        &&  !str_prefix(name,pArea->name))
            return pArea;
    }
    return NULL;
}

