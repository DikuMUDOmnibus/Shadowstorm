/* Portions of this code were taken from Sundermud */

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


void show_obj_cond (CHAR_DATA *ch, OBJ_DATA *obj)
{
    int condition = 0;
	
    if (IS_SET(obj->extra_flags, ITEM_NOCOND)) 
	return;

    if (obj->condition == 100)
	condition = 0;
    else if (obj->condition > 90)
	condition = 1;
    else if (obj->condition > 75)
	condition = 2;
    else if (obj->condition > 50)
	condition = 3;
    else if (obj->condition > 25)
	condition = 4;
    else if (obj->condition > 10)
	condition = 5;
    else if (obj->condition >  0)
	condition = 6;
    else if (obj->condition == 0)
	condition = 7;
	
    printf_to_char( ch, "Condition: %s\n\r", cond_table[condition]);
    return;
}	

void check_damage_obj (CHAR_DATA *ch, OBJ_DATA *obj, int chance)
{ 
    bool damageall = FALSE;
    bool done = FALSE;
    int damage_pos;
    OBJ_DATA *damobj = NULL;
    int stop = 0;
    int newbie = 20;
	
    if (obj == NULL)
        damageall = TRUE;
  
    if (damageall) 
    {
	if (number_percent () <= chance) 
	{
	    while ( ( !done ) && ( stop <= 30 ) )
	    {
		   damage_pos = number_range (1, MAX_WEAR);
		   if ((damobj = get_eq_char (ch, damage_pos)) != NULL) 
			done = TRUE;
			stop++;
            }
			
            if (done)
		damage_obj(ch, damobj, 1);
	    return;
	}
		
        else return;
    }

    if (IS_SET(obj->extra_flags, ITEM_QUEST ) )
	return;

    if (ch->level <= newbie || IS_NPC( ch ) )
        return;

    if (number_percent () <= chance) 
    {
	damage_obj(ch, obj, 1);
	return;
    }

    return;
}		

void damage_obj (CHAR_DATA *ch, OBJ_DATA *obj, int damage)
{
    int i;
	
    if (obj == NULL)
    {
	bug ("NULL obj passed to damage_obj",0);
	return;
    }

    if(IS_SET(obj->extra2_flags, ITEM_SANC ) ) damage /= 2;
    obj->condition -= damage;
    obj->condition = URANGE (0, obj->condition, 100);

    if (obj->condition == 0)
    {
        printf_to_char( ch, "%s has become to badly damaged to use!\n\r", ( (obj->short_descr && obj->short_descr[0] != '\0') ? obj->short_descr : "Unknown") );
	unequip_char (ch, obj);
	return;
    }
  		
    if (obj->item_type == ITEM_ARMOR)
    {
	for ( i = 0 ; i < 4 ; i++)		
	{
	     ch->armor[i] += apply_ac (obj, obj->wear_loc, i);
	     obj->value[i] = obj->valueorig[i] * (obj->condition)/100;
				
             if ((obj->value[i] == 0) && (obj->valueorig[i] != 0))  
		  obj->value[i] = 1; 
		  ch->armor[i] -= apply_ac(obj, obj->wear_loc, i);
	}
    }
	
    return;
}			

void set_obj_condition(OBJ_DATA *obj, int condition)
{
    int i;

    obj->condition = condition;
       
    if (obj->item_type == ITEM_ARMOR)
    {
       for (i = 0; i <= 4; i++)
       {
           obj->value[i] = obj->valueorig[i];
       }
    }   		

    return;
}

