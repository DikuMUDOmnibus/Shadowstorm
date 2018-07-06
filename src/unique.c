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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "const.h"

#define MAX_ARMOR_SUFFIX 25
#define MAX_WEAPON_SUFFIX 15
#define MAX_PREFIX 18

CHAR_DATA *random_mob args( (void) );
void format_obj args( (OBJ_DATA *obj) );
void format_obj_weapon args( (OBJ_DATA *obj) );
void format_obj_armor args( (OBJ_DATA *obj) );
int wear_bit args( (int loc) );
void name_obj args( (CHAR_DATA *mob,OBJ_DATA *obj) );
char *weapon_type_name args( (OBJ_DATA *obj) );
char *armor_type_name args( (OBJ_DATA *obj) );
int which_location args( (void) );
void apply_good_affect args( (OBJ_DATA *obj,bool positive) );

DECLARE_DO_FUN(do_wear );

struct unique_attrib_table unique_table_armor_suffix[MAX_ARMOR_SUFFIX]    =
{
  { "" },
  { "taste"           },
  { "protection"      },
  { "value"           },
  { "power"           },
  { "dark power"      },
  { "holiness"        },
  { "desecration"     },
  { "hope"            },
  { "hopelessness"    },
  { "care"            },
  { "invunlerability" },
  { "Ferric"          },
  { "Dagda"           },
  { "Chele"           },
  { "Khain"           },
  { "Keogh"           },
  { "Svartalfar"      },
  { "Makaruda"        },
  { "Moirai"          },
  { "Diku"            },
  { "the fallen"      },
  { "the gods"        },
  { "the Shrike"      },
  { "mystery"         },
};

struct unique_attrib_table unique_table_weapon_suffix[MAX_WEAPON_SUFFIX]    =
{
  { "" },
  { "destruction"       },
  { "sharpness"         },
  { "power"             },
  { "maiming"           },
  { "killing"           },
  { "slaying"           },
  { "havok"             },
  { "crushing"          },
  { "cutting"           },
  { "fear"              },
  { "pillage"           },
  { "slashing"},
  { "annoyance"         },
  { "striking"          },
};

struct unique_attrib_table unique_table_prefix[MAX_PREFIX]      =
{
  { "" },
  { "flashing"       },
  { "dull"           },
  { "well crafted"   },
  { "finely crafted" },
  { "shiny"          },
  { "fine"           },
  { "fantastic"      },
  { "scuffed"        },
  { "brilliant"      },
  { "hewn"           },
  { "scratched"      },
  { "dark"           },
  { "intense"        },
  { "battered"       },
  { "polished"       },
  { "faded"          },
  { "enigmatic"      },
};


void create_unique(void)
{
  int x;
  int i;
  OBJ_DATA *obj;
  char buf[MSL];
  CHAR_DATA *mob;

  if ( (obj = create_object( get_obj_index( OBJ_UNIQUE_DUMMY), 0) ) == NULL)
  {
      log_string( "Bug with OBJ_UNIQUE_DUMMY");
      return;
  }

  free_string(obj->short_descr);
  free_string(obj->description);

  mob = random_mob();

  if ( IS_SET( mob->in_room->area->area_flags , NO_UNIQUE ) )
      return;

  SET_BIT(obj->wear_flags,ITEM_TAKE);
  
  obj->level = mob->level;

  format_obj(obj);

  name_obj(mob,obj);

  SET_BIT( obj->extra2_flags, ITEM_UNIQUE );
  SET_BIT( obj->extra2_flags, ITEM_RELIC );

  obj->xp_tolevel = MIN_XP;

  i = ( (mob->level-10)/10);
  
  if(i ==0)
     i++;

  for(x = 0; x < i; x++)
     apply_good_affect(obj,TRUE);

  if(mob->level > 20)
  {
      i = (mob->level - 20) / 10;
  
      if(i == 0)
 	 i++;
  
      for(x = 0; x < i; x++)
 	 apply_good_affect(obj,FALSE);
  }

  obj_to_char(obj,mob);
  do_wear(mob,obj->name);
  
  sprintf(buf,"The unique object, %s, loaded to %s in room %d\n\r",
	  obj->short_descr,mob->name,mob->in_room->vnum);
  log_string( buf );
  
}

void do_tally(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  int tally = 0;
  bool show = FALSE;
 
  if (!str_cmp(argument, "shadow" ) )
      show = TRUE;

  for(obj = object_list; obj != NULL; obj = obj->next )
  {
      CHAR_DATA *vch;

      if( (vch = obj->carried_by) == NULL)
	continue;

      if(!IS_NPC(vch))
	continue;

      if(IS_SET(obj->extra2_flags,ITEM_UNIQUE) )
      {	 
         tally++; 
                 
         if(show)
         printf_to_char(ch, "{D%d{r){W %s {r({DOn{r:{W %d{r)[{DIn{r:{W %d{r]{x\n\r", tally, obj->name, obj->carried_by && obj->carried_by->pIndexData ? obj->carried_by->pIndexData->vnum : 0, obj->carried_by ? obj->carried_by->in_room->vnum : 0 );
                   
      }
  }
     
  printf_to_char( ch, "{CTheir are currently %d unique items in the wilderness.{x\n\r", tally );
  return;
}


int which_location()
{
   int i;
   int location = 0;

   i = number_range(1,11);
   
   switch(i)
   {
     case 1:
       location = APPLY_STR;
       break;
     case 2:
       location = APPLY_DEX;
       break;
     case 3:
       location = APPLY_INT;
       break;
     case 4:
       location = APPLY_WIS;
       break;
     case 5:
       location = APPLY_CON;
       break;
     case 6:
       location = APPLY_MANA;
       break;
     case 7:
       location = APPLY_HIT;
       break;
     case 8:
       location = APPLY_MOVE;
       break;
     case 9:
       location = APPLY_HITROLL;
       break;
     case 10:
       location = APPLY_DAMROLL;
       break;
     case 11:
       location = APPLY_SAVES;
       break;
   }

   return location;

}

/*Lets apply some nice affects.*/
void apply_good_affect(OBJ_DATA *obj,bool positive)
{
  int location;
  AFFECT_DATA *af;
  int mult;
  int value;
  int max,min;

  location = which_location();
  
  switch(location)
  {
    default:
      mult = 10;
      break;
    case(APPLY_HIT):
      mult = 20;
      break;
    case(APPLY_MOVE):
      mult = 20;
      break;
    case(APPLY_MANA):
      mult = 20;
      break;
  }

  value = (1+(obj->level/10)) * mult;
  max = value*2;
  min = value/2;

  if(min == 0)
    min++;

  /*apply a negative affect*/
  if(!positive)
    {
      value = value*-1;
      max = value/2;
      min = value*2;
    }

  af = new_affect();               
  af->location = location;
  af->modifier = number_range(min,max);
  af->where = 0;
  af->type = -1;
  af->duration = -1;
  af->bitvector = 0;
  af->level = obj->level;
  af->next = obj->affected;
  obj->affected = af;


}

/*the fido's armor of taste*/
void name_obj(CHAR_DATA *mob,OBJ_DATA *obj)
{
  char buf[MSL];
  char buf2[MSL];
  char buf3[MSL];
  int i = number_range(1,MAX_PREFIX-1);
  int x = number_range(1,MAX_WEAPON_SUFFIX-1);
  int y = number_range(1,MAX_ARMOR_SUFFIX-1);


  sprintf(buf,"the %s %s of %s",
	  unique_table_prefix[i].descriptive,
	  obj->item_type == ITEM_WEAPON ? weapon_type_name(obj) :
	  armor_type_name(obj),
	  obj->item_type == ITEM_WEAPON ?
	  unique_table_weapon_suffix[x].descriptive :
	  unique_table_armor_suffix[y].descriptive);
	  
  sprintf(buf2,"%s is laying here on the ground.",buf);

  sprintf(buf3,"%s",buf);

  free_string(obj->name);
  free_string(obj->short_descr);
  free_string(obj->description);
  obj->name = str_dup(buf3);
  obj->short_descr = str_dup(buf);
  obj->description = str_dup(buf2);

}

void format_obj_weapon(OBJ_DATA *obj)
{
  int i;
  int size,dice;
  double avg;

  /*What type of weapon?*/
  i = number_range(0,8);
  obj->value[0] = i;
  
  /*Set it to wield*/
  SET_BIT(obj->wear_flags,ITEM_WIELD);
  
  avg = obj->level;
  dice = (obj->level/10+1);
  size = dice/2;

  for (size=dice/2 ; dice * (size +2)/2 < avg ; size++ )
    { }  

  dice = UMAX(1, dice);
  size = UMAX(2, size);
  
  obj->value[1] = dice;
  obj->value[2] = size;
  /*End autodamage routine.*/

}

char *weapon_type_name(OBJ_DATA *obj)
{
    char *buf;
    buf = '\0';
    
    switch(obj->value[0])
    {
    case(WEAPON_EXOTIC) : buf = str_dup("exotic weapon"); break;
    case(WEAPON_SWORD): buf = str_dup("sword"); break;
    case(WEAPON_DAGGER): buf = str_dup("dagger"); break;
    case(WEAPON_SPEAR): buf = str_dup("spear"); break;
    case(WEAPON_MACE): buf = str_dup("mace"); break;
    case(WEAPON_AXE): buf = str_dup("axe"); break;
    case(WEAPON_FLAIL): buf = str_dup("flail"); break;
    case(WEAPON_WHIP): buf = str_dup("whip"); break;
    case(WEAPON_POLEARM): buf = str_dup("polearm"); break;
    }
 
    return buf; 
}

char *armor_type_name(OBJ_DATA *obj)
{
  int i;

  char *buf;
  buf = '\0';

  switch(obj->wear_loc)
    {
    case 0:
      buf = str_dup("light");
      break;
    case 1:
    case 2:
      buf = str_dup("ring");
      break;
    case 3:
    case 4:
      i = number_range(1,3);
      if(i == 1)
	buf = str_dup("necklace");
      else if(i == 2)
	buf = str_dup("pendant");
      else
	buf = str_dup("neck guard");
      break;
    case 5:
      i = number_range(1,3);
      if(i == 1)
	buf = str_dup("armor");
      else if(i == 2)
	buf = str_dup("breastplate");
      else
	buf = str_dup("chain mail");
      break;
    case 6:
      i = number_range(1,3);
      if(i == 1)
	buf = str_dup("skullcap");
      else if(i == 2)
	buf = str_dup("helmet");
      else
	buf = str_dup("helm");
      break;
    case 7:
      i = number_range(1,3);
      if(i == 1)
	buf = str_dup("leggings");
      else if(i == 2)
	buf = str_dup("leg plates");
      else
	buf = str_dup("pants");
      break;
    case 8:
      i = number_range(1,3);
      if(i == 1)
	buf = str_dup("sandals");
      else if(i == 2)
	buf = str_dup("boots");
      else
	buf = str_dup("clogs");
      break;
    case 9: 
      i = number_range(1,3);
      if(i == 1)
	buf = str_dup("gloves");
      else if(i == 2)
	buf = str_dup("gauntlets");
      else
	buf = str_dup("sap gloves");
      break;
    case 10:
      buf = str_dup("arm plates");
      break;
    case 11:
      buf = str_dup("shield");
      break;
    case 12:
      i = number_range(1,3);
      if(i == 1)
	buf = str_dup("cloak");
      else if(i == 2)
	buf = str_dup("cape");
      else
	buf = str_dup("coat");
      break;
    case 13:
      i = number_range(1,3);
      if( i == 1)
	buf = str_dup("belt");
      else if(i == 2)
	buf = str_dup("girdle");
      else
	buf = str_dup("chain");
      break;
    case 14:
    case 15:
      i = number_range(1,3);
      if( i == 1)
	buf = str_dup("bracelet");
      else if(i == 2)
	buf = str_dup("wrist band");
      else
	buf = str_dup("band");
      break;
    }
  
  return buf;    
}

void format_obj_armor(OBJ_DATA *obj)
{
  /*Objects of ARMOR type can be ring, boots, shield, etc. Lets assign
    a random wear location to decide what this will be.*/

  obj->wear_loc = number_range(0,15);
  
  /*Ok, we have a type now. Lets go through and set a wear bit.*/
  switch(obj->wear_loc)
    {
    case 0:
      obj->item_type = ITEM_LIGHT;
      break;
    case 1:
    case 2:
      SET_BIT(obj->wear_flags,ITEM_WEAR_FINGER);
      break;
    case 3:
    case 4:
      SET_BIT(obj->wear_flags,ITEM_WEAR_NECK);
      break;
    case 5:
      SET_BIT(obj->wear_flags,ITEM_WEAR_BODY);
      break;
    case 6:
      SET_BIT(obj->wear_flags,ITEM_WEAR_HEAD);
      break;
    case 7:
      SET_BIT(obj->wear_flags,ITEM_WEAR_LEGS);
      break;
    case 8:
      SET_BIT(obj->wear_flags,ITEM_WEAR_FEET);
      break;
    case 9:
      SET_BIT(obj->wear_flags,ITEM_WEAR_HANDS);
      break;
    case 10:
      SET_BIT(obj->wear_flags,ITEM_WEAR_ARMS);
      break;
    case 11:
      SET_BIT(obj->wear_flags,ITEM_WEAR_SHIELD);
      break;
    case 12:
      SET_BIT(obj->wear_flags,ITEM_WEAR_ABOUT);
      break;
    case 13:
      SET_BIT(obj->wear_flags,ITEM_WEAR_WAIST);
      break;
    case 14:
    case 15:
      SET_BIT(obj->wear_flags,ITEM_WEAR_WRIST);
      break;
    }				       

  /*Ok, we've determined what type of armor it is, set appropriate wear.*/

  /*Now, lets set it's armor values*/
  obj->value[0] = number_range( (obj->level/2)-1 , (obj->level/2)+2);
  obj->value[1] = number_range( (obj->level/2)-1 , (obj->level/2)+2);
  obj->value[2] = number_range( (obj->level/2)-1 , (obj->level/2)+2);
  obj->value[3] = number_range( (obj->level/2)-1 , (obj->level/2)+2);
}

void format_obj(OBJ_DATA *obj)
{
   int i;
  
   /* Weapon, Armor? */

   i = number_range(0,1);

   switch(i)
   {
      case 0:
       obj->item_type = ITEM_WEAPON;
       format_obj_weapon(obj);
       break;
      case 1:
       obj->item_type = ITEM_ARMOR;
       format_obj_armor(obj);
       break;
   } 
} 

CHAR_DATA *random_mob()
{
  CHAR_DATA *mob;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  char buf[MSL];
  int i,x;

  mob = NULL;

  i = (number_range(1,top_mob_index));
  x = 0;

  for(vch = char_list; vch != NULL; vch = vch_next)
  {
      vch_next = vch->next;

      if(!IS_NPC(vch)
         || (vch->pIndexData == NULL
         || vch->name == NULL
         || vch->short_descr == NULL
         || vch->in_room == NULL )
         || vch->pIndexData->vnum < 100
         || vch->in_room->clan > 0
         || IS_SET( vch->act, ACT_TRAIN | ACT_PRACTICE | ACT_IS_HEALER
            | ACT_PET | ACT_GAIN | ACT_BOUNTY | ACT_MOUNT | ACT_FORGER
            | ACT_IS_CHANGER )
         || IS_SET( vch->imm_flags, IMM_WEAPON | IMM_MAGIC )
         || IS_SET( vch->affected_by, AFF_CHARM )
         || IS_SET( vch->in_room->room_flags, ROOM_PET_SHOP) )
        continue;

      x++;

      if ( i == 0 || x == 0 || vch == NULL )
      {
          sprintf(buf,"RANDOM_MOB: Null mob, i is 0, or x is 0. I: %d, X: %d, Vch: %s", i, x, vch->name );
          log_string(buf);
      }

      if(x == i)
        mob = vch;
      if(x >= top_mob_index)
        break;
  }

  return mob;
}
