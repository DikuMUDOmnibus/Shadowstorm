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

/* 
 * Farm functions for Shadowstorm Mud
 * By Davion
 */


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

#if defined(KEY)
#undef KEY
#endif

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )  if ( !str_cmp( word, literal ) ) { field  = value; fMatch = TRUE; break; }

void do_plow(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *pRoom;
    PLANT_DATA *plant;        
    
    pRoom = ch->in_room;
  
    if(pRoom->sector_type == SECT_FARM )
    {
	send_to_char("This land is already plowed.\n\r",ch);
	return;
    }

    if(pRoom->sector_type != SECT_FIELD )
    {
	send_to_char("You cannot plow anything but fields!\n\r",ch);
	return;
    }
   
    for(plant = pRoom->plant_first; plant; plant = plant->next)
	free_plant(plant);

    send_to_char("You plow the field.\n\r",ch);
    WAIT_STATE(ch, PULSE_PER_SECOND*10);
    pRoom->sector_type = SECT_FARM;
    return;
}

void do_plant(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *pRoom;
    char arg1[MSL];
    OBJ_DATA *seed;
    PLANT_DATA *plant;

    arg1[0] = '\0';
  
    plant = new_plant();

    argument = one_argument(argument, arg1);

    if(arg1[0] == '\0' )
    {
	send_to_char("Syntax:\n\r\tplant <seed>\n\r",ch);
	return;
    }
    pRoom = ch->in_room;
 
   
    if(pRoom->sector_type != SECT_FARM )
    {
	send_to_char("You can only plant in plowed lands.\n\r",ch);
	return;
    }
    if ( ( seed = get_obj_carry(ch, arg1, ch) ) == NULL )
    {
	send_to_char("You don't have that seed.\n\r",ch);
	return;
    }

    if(seed->item_type != ITEM_SEED )
    {
	send_to_char("You can only plant seeds.\n\r",ch);
	return;
    }
    
    plant = make_plant_from_seed(seed, plant);
    extract_obj(seed); 
    plant_in_room(pRoom, plant);    
    send_to_char("You carefully dig up the soil, and plant the seed.\n\r",ch);
    act_new("$n squats down, digs up some soil, and plants a seed.", ch, NULL, NULL, TO_ROOM, POS_RESTING);
    return;
}

PLANT_DATA *make_plant_from_seed(OBJ_DATA *seed, PLANT_DATA *plant)
{
	SEED_DATA *pSeed;
        plant->seed = seed->value[0];
	if( (pSeed = get_seed(plant) ) == NULL )
		return NULL;
    plant->seedtype = pSeed;
    free_string( plant->name );
    plant->name = str_dup(pSeed->name);
    plant->stage = 0;
    return plant;
}

SEED_DATA *get_seed(PLANT_DATA *pPlant)
{
	SEED_DATA *pSeed;
	int count = 0;
	for( pSeed = seed_first ; pSeed ; pSeed = pSeed->next )
	{
		if(count == pPlant->seed )
			return pSeed;
		count++;
	}

	return seed_first;
}

void show_plant_to_char(ROOM_INDEX_DATA *pRoom, CHAR_DATA *ch)
{
	PLANT_DATA *pPlant;
	SEED_DATA *pSeed;


	for( pPlant = pRoom->plant_first ; pPlant ; pPlant =pPlant->next )
	{
	if( (pSeed = get_seed(pPlant) ) == NULL)
		continue;
		switch (pPlant->stage)
		{
		    case 0: break;
		    case 1: printf_to_char(ch, "A sprouting %s\n\r", plant_type_table[pSeed->planttype].name ); break;
		    case 2: printf_to_char(ch, "A plant, sprouting %s %s leaves\n\r", osize_table[pSeed->leafsize], pSeed->leafcol ); break;
		    case 3: printf_to_char(ch, "A plant with a %s flower, and %s leaves.\n\r",pSeed->flowercol, pSeed->leafcol); break;	
		    case 4: printf_to_char(ch, "A %s\n\r", pSeed->name ); break;
		    default : send_to_char("This plant is broken. Report to an immortal.\n\r",ch);
		}
	}
	return;
}

void plant_update(ROOM_INDEX_DATA *pRoom)
{
    PLANT_DATA *plant;
    char message[MSL];    
    CHAR_DATA *vch;
    bool SEND = FALSE;

   for ( plant = pRoom->plant_first ; plant ; plant = plant->next)
   {	
    if(plant->stage >= MAX_STAGE)
    	return;
    if(plant->age < 10 )
    {
	plant->age++;
	return;
    }
    else
       plant->age = 0;

    switch ( plant->stage )
    {
	case SEED_STAGE:
		sprintf(message, "A small sprout shoots up from the ground.\n\r");
	   	plant->stage++;
		SEND = TRUE;
		break;
        default:
		sprintf(message, "A sprout slowly grows toward the sky!\n\r");
	   	plant->stage++;
		SEND = TRUE;
		break;
	case STAGE_THREE:
		sprintf(message, "%s finishes growing!\n\r",plant->name);
		SEND = TRUE;
	   	plant->stage++;
		break;
	case STAGE_FOUR:
		break;
    }

    if( (vch = pRoom->people ) == NULL )
        return;
   if(SEND)
    for ( ; vch ; vch = vch->next_in_room)
	send_to_char(message, vch );
   }
    return;
}   

void do_uproot(CHAR_DATA *ch, char *argument)
{
	SEED_DATA *pSeed;
	PLANT_DATA *pPlant;
	char arg[MSL], buf[MSL];
    OBJ_DATA *pObj;

	arg[0] = '\0';
	buf[0] = arg[0];
	argument = one_argument(argument, arg);

	if(arg[0] == '\0' )
	{
		send_to_char("What would you like to uproot?\n\r",ch);
		return;
	}

	if(ch->in_room->plant_first == NULL)
	{
		send_to_char("There are no plants in this room.\n\r",ch);
		return;
	}

	if( (pPlant = get_plant_room(ch, arg) ) == NULL )
	{
		send_to_char("Which plant would you like to uproot?",ch);
		return;
	}

	if( (pSeed = get_seed(pPlant) ) == NULL )
	{
		send_to_char("It seems the seed of the plant is broken. Report to an Immortal.\n\r",ch);
		return;
	}

    if( (pObj = create_object(get_obj_index(OBJ_HERB_DUMMY), 0) ) == NULL )
	{
		send_to_char("The Herb dummy aint right. Report to an Immortal.\n\r",ch);
		return;
	}
	
	if(pPlant->stage == STAGE_FOUR)
	{
		pObj->value[0] = pSeed->affeat;
		pObj->value[1] = pSeed->affsmoke;
		pObj->value[2] = pSeed->affdrink;
	}
	else
	{
		pObj->value[0] = AFF_UNUSED_FLAG;
		pObj->value[1] = AFF_UNUSED_FLAG;
		pObj->value[2] = AFF_UNUSED_FLAG;
	}

	free_string(pObj->name);
	free_string(pObj->short_descr);
	free_string(pObj->description);
	if(pPlant == ch->in_room->plant_first)
		ch->in_room->plant_first = pPlant->next;
	free_plant(pPlant);
	pObj->name = str_dup(pPlant->name);
	sprintf(buf, "A %s", pPlant->name);
	pObj->short_descr = str_dup(buf);
	pObj->description = pObj->short_descr;
	pObj->level = ch->level;
	obj_to_char(pObj, ch);
	printf_to_char(ch, "You reach down and uproot %s.\n\r",pObj->short_descr );
	sprintf(buf, "$n reaches down and uproots %s.", pObj->short_descr);

	act_new(buf, ch, NULL, NULL, TO_ROOM, POS_RESTING);
	return;
}

void fwrite_seed()
{
	FILE *fp;
	SEED_DATA *pSeed;


	fp = file_open( PLANT_DIR, "w" );

	for( pSeed = seed_first ; pSeed ; pSeed = pSeed->next)
	{
		fprintf(fp, "#PLANTDAT\n");
		fprintf(fp, "Name %s~\n", pSeed->name );
		fprintf(fp, "Leafcol %s~\n", pSeed->leafcol );
		fprintf(fp, "Flowercol %s~\n", pSeed->flowercol );
		fprintf(fp, "Leafsize %d\n", pSeed->leafsize );
		fprintf(fp, "Affeat %s~\n", skill_table[pSeed->affeat].name );
		fprintf(fp, "Affsmoke %s~\n", skill_table[pSeed->affsmoke].name );
		fprintf(fp, "Affdrink %s~\n", skill_table[pSeed->affdrink].name );
		fprintf(fp, "Ptype %d\n", pSeed->planttype );
		fprintf(fp, "Colour %d\n", pSeed->colour );
		fprintf(fp, "Parts %s\n", print_flags(pSeed->parts) );
		fprintf(fp, "Flowersize %d\n",pSeed->flowersize );
		fprintf(fp, "Budcol %s~\n",pSeed->budcol);
		fprintf(fp, "Budsize %d\n", pSeed->budsize );
		fprintf(fp, "Needlecol %s~\n", pSeed->needlecol);
		fprintf(fp, "Needlesize %d\n",pSeed->needlesize);
		fprintf(fp, "Fruitname %s~\n", pSeed->fruitname);
		fprintf(fp, "Fruitsize %d\n",pSeed->fruitsize);
		fprintf(fp, "Fruitcol %s~\n", pSeed->fruitcol);
		fprintf(fp, "Fruitnumb %d\n", pSeed->fruitnumb);
		fprintf(fp, "Vegname %s~\n", pSeed->vegname );
		fprintf(fp, "Vegsize %d\n", pSeed->vegsize );
		fprintf(fp, "Vegcol %s~\n", pSeed->vegcol );
		fprintf(fp, "END\n\n\n");
	}
	fprintf(fp, "$\n");
	file_close(fp);
	return;
}

void save_seeds()
{
	fwrite_seed();
	logf2("Plants saved");
}

SEED_DATA *load_seed(SEED_DATA *pSeed, FILE *fp)
{
	char *word;
	bool fMatch = FALSE;
 
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

			case 'A':
				if (!str_cmp(word, "Affeat" ) )
				{
					pSeed->affeat = skill_lookup(fread_string(fp) );
					fMatch = TRUE;
					break;
				}

				if (!str_cmp(word, "Affsmoke" ) )
				{
					pSeed->affsmoke = skill_lookup(fread_string(fp) );
					fMatch = TRUE;
					break;
				}

				if (!str_cmp(word, "Affdrink") )
				{
					pSeed->affdrink = skill_lookup(fread_string(fp) );
					fMatch = TRUE;
					break;
				}
				break;
			case 'B':
				KEY("Budsize", pSeed->budsize, fread_number(fp) );
				KEY("Budcol", pSeed->budcol, fread_string(fp) );
			case 'C':
				KEY("Colour", pSeed->colour, fread_number(fp));
			    break;
			case 'E':
				if (!str_cmp(word, "End" ) )
					return pSeed;
			case 'F':
				KEY("Flowercol", pSeed->flowercol, fread_string(fp) );
				KEY("Flowersize", pSeed->flowersize, fread_number(fp) );
				KEY("Fruitsize", pSeed->fruitsize, fread_number(fp) );
				KEY("Fruitcol", pSeed->fruitcol, fread_string(fp ) );
				KEY("Fruitname", pSeed->fruitname, fread_string(fp) );
				KEY("Fruitnumb", pSeed->fruitnumb, fread_number(fp ) );
				break;

			case 'L':
				KEY("Leafsize", pSeed->leafsize, fread_number(fp) );
				KEY("Leafcol", pSeed->leafcol, fread_string(fp) );
				break;

			case 'N':
				KEY("Name", pSeed->name, fread_string(fp) );
				KEY("Needlecol", pSeed->needlecol, fread_string(fp) );
				KEY("Needlesize", pSeed->needlesize, fread_number(fp) );
				break;
			case 'P':
				KEY("Ptype", pSeed->planttype, fread_number(fp) );
				KEY("Parts", pSeed->parts,		fread_flag(fp) );
				break;

			case 'V':
				KEY("Vegname", pSeed->vegname, fread_string(fp) );
				KEY("Vegsize", pSeed->vegsize, fread_number(fp) );
				KEY("Vegcol", pSeed->vegcol, fread_string(fp) );
		}
		if(!fMatch)
			logf2("Bug: load_sead. fMatch false: %s", word);
	}
	return NULL;
}

void load_seed_data()
{
	SEED_DATA *pSeed;
	char *read;
	FILE *fp;

	if (!file_exists(PLANT_DIR))
		return;

	fp = file_open( PLANT_DIR, "r" );

	for (; ; )
	{
		read = fread_word(fp);
		if(*read == '$')
		    return;
		pSeed = new_seed();
		free_string(pSeed->name);
		free_string(pSeed->leafcol);
		free_string(pSeed->flowercol);
		free_string(pSeed->budcol);
		free_string(pSeed->needlecol);
		free_string(pSeed->fruitcol);
		free_string(pSeed->vegcol);

		pSeed->name = str_dup("");
		pSeed->leafcol = str_dup("");
		pSeed->flowercol = str_dup("");

		pSeed = load_seed(pSeed, fp);

		if(!seed_first)
			seed_first = pSeed;
		if(seed_last)
			seed_last->next = pSeed;
		seed_last = pSeed;
		pSeed->next = NULL;
	}
	file_close(fp);
	return;
}
