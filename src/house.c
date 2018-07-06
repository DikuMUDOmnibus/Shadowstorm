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
 **************************************************************************/   

/***************************************************************************
*       ROM 2.4 is copyright 1993-1996 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@efn.org)                                  *
*           Gabrielle Taylor                                               *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/   

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "recycle.h"
#include "olc.h"
#include "interp.h"
#include "const.h"

bool change_exit  args( ( CHAR_DATA *ch, char *argument, int door ) );

void save_area (AREA_DATA *pArea );
void save_house args( ( ) );
void home_sell ( CHAR_DATA *ch, int cost );
CHAR_DATA *  find_architect     args( ( CHAR_DATA * ch ) );
HOUSE_DATA *house_list;

#define MAX_HORESET	5
#define MAX_HMRESETS	5

/*#define VNUM_START 10600
#define VNUM_STOP  12600 */

CHAR_DATA * find_architect ( CHAR_DATA *ch )
{
    CHAR_DATA * architect;

    for ( architect = ch->in_room->people; architect != NULL; architect = architect->next_in_room )
    {
        if (!IS_NPC(architect))
            continue;

        if (architect->spec_fun == spec_lookup( "spec_architect" ) )
            return architect;
    }  

    if ( architect == NULL || architect->spec_fun != spec_lookup( "spec_architect" ))
    {
        send_to_char("You can't do that here, find an Architect.\n\r", ch);
        return NULL;
    }

    if ( architect->fighting != NULL )
    {
        send_to_char("Wait until the fighting stops.\n\r",ch );
        return NULL;
    }

    return NULL;
}        

void do_objbuy(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    char arg1[MSL];
    char buf[MSL];
    OBJ_DATA *obj;
    HOUSE_DATA *hOuse;
    int i;
 
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );

    if ( !HAS_HOME( ch ) )
    {
        send_to_char("If you only had a home...\n\r",ch);
        return;
    }

    if ( !IS_HOME( ch ) )
    {
	send_to_char("You gotta be in your house.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	send_to_char("Syntax: objbuy <list>\n\rSyntax: objbuy buy <name>",ch);
	return;
    }

    if (!str_cmp(arg,"list"))
    {
	send_to_char("The following objects are available for purchase:\n\r",ch);
        send_to_char("{cName:             Cost:{x\n\r", ch );
        send_to_char("{Y========================{x\n\r", ch );
        for ( i = 0; house_table[i].name != NULL; i++ )
        {
             if ( house_table[i].type != OBJ_VNUM )
                 continue;
             printf_to_char( ch, "%-10s        %-5d\n\r", house_table[i].name, house_table[i].cost );
        }

        return;
    }

    if (!str_cmp(arg, "buy"))
    {
        if (arg1[0] == '\0')
        {
            send_to_char( "To buy an item type objbuy buy <item>.\n\r",ch );
            return;
        }
         
        for (i = 0; house_table[i].name != NULL; i++)           
        {
             if (is_name(arg1, house_table[i].name) && house_table[i].type == OBJ_VNUM)         
             {
                 if (ch->gold >= house_table[i].cost) 
                 {
                     if ( (obj = create_object(get_obj_index(house_table[i].vnum), ch->level ) )  == NULL ) 
                     {
                         send_to_char( "That object could not be found contact an imm.\n\r", ch );
                         return;
                     }

                     if( ch->pcdata->horesets > MAX_HORESET )
                     {
  	                send_to_char("Sorry you can only have 5 resetting objects.\n\r",ch);
	                return;
                     }

                     ch->pcdata->horesets++;
                     ch->gold -= house_table[i].cost;  
                     sprintf(buf,"%d", house_table[i].vnum);
                     redit_oreset(ch,buf);
                     sprintf(buf," ");
                     save_area(ch->in_room->area);
          
                     obj = create_object( get_obj_index( house_table[i].vnum ), ch->level );
          
                     for ( hOuse = house_list; hOuse != NULL; hOuse = hOuse->next )
                     {                                                     
                          if ( !str_cmp( obj->name, hOuse->objname ) ) 
                              break;

                          hOuse = new_house();
                          free_string( hOuse->oname );
                          hOuse->oname = str_dup( ch->name );
                          free_string( hOuse->objname );
                          hOuse->objname = str_dup( obj->name );
                          hOuse->ovalue = house_table[i].cost;
                          hOuse->next = house_list;
                          house_list = hOuse;
                          save_house();
                     }

          	     return;
                 }

                 else
                 {
                    printf_to_char( ch, "Sorry %s but you need %d gold for that.\n\r", ch->name, house_table[i].cost );
                    return;
                 }
             }
        }
    }

    send_to_char( "To buy an item type objbuy buy <item>.\n\r",ch );
    return;
}

void do_mobbuy(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    char arg1[MSL];
    CHAR_DATA *mob;
    char buf[MSL];  
    HOUSE_DATA *hOuse; 
    int i;
 
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );

    if ( !HAS_HOME( ch ) )
    {
        send_to_char("If you only had a home...\n\r",ch);
        return;
    }

    if ( !IS_HOME( ch ) )
    {
	send_to_char("You gotta be in your house.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
        send_to_char("Syntax: mobbuy <list>\n\rSyntax: mobbuy buy <name>",ch); 
	return;
    }

    if (!str_cmp(arg,"list"))
    {
	send_to_char("The following mobiles are available for purchase:\n\r",ch);
        send_to_char("{cName:             Cost:{x\n\r", ch );
        send_to_char("{Y========================{x\n\r", ch );
        for ( i = 0; house_table[i].name != NULL; i++ )
        {
             if ( house_table[i].type != MOB_VNUM )
                 continue;
             printf_to_char( ch, "%-10s        %-5d\n\r", house_table[i].name, house_table[i].cost );
        }

        return;

    }

    if (!str_cmp(arg, "buy"))
    {
        if (arg1[0] == '\0')
        {
            send_to_char( "To buy an item type mobbuy buy <item>.\n\r",ch );
            return;
        }           

        for (i = 0; house_table[i].name != NULL; i++)
        {        
             if (is_name(arg1, house_table[i].name) && house_table[i].type == MOB_VNUM)
             {
                 if (ch->gold >= house_table[i].cost)
                 {                  
                     mob = create_mobile(get_mob_index(house_table[i].vnum) );
                 
                     if ( mob == NULL ) 
                     {
                         send_to_char( "That mob could not be found contact an imm.\n\r", ch );
                         return;
                     }            

                     if( ch->pcdata->hmresets > MAX_HMRESETS )
                     {      
                       	send_to_char("Sorry you can only have 5 resetting mobiles.\n\r",ch);
	                return;
                     }

                     ch->pcdata->hmresets++;
                     ch->gold -= house_table[i].cost;
                     sprintf(buf,"%d", house_table[i].vnum);  
                     redit_mreset(ch,buf);
                     sprintf(buf," ");
                     save_area(ch->in_room->area);

                     mob = create_mobile( get_mob_index( house_table[i].vnum ) );
          
                     for ( hOuse = house_list; hOuse != NULL; hOuse = hOuse->next )
                     {                                                     
                          if ( !str_cmp( mob->name, hOuse->mobname ) ) 
                              break;

                          hOuse = new_house();
                          free_string( hOuse->oname );
                          hOuse->oname = str_dup( ch->name );
                          free_string( hOuse->mobname );
                          hOuse->mobname = str_dup( mob->name );
                          hOuse->mvalue = house_table[i].cost;
                          hOuse->next = house_list;
                          house_list = hOuse;
                          save_house();
                     }

                     return;                            
                 }

                 else
                 {

                   printf_to_char( ch, "Sorry %s but you need %d gold for a %s.\n\r", ch->name, house_table[i].cost, house_table[i].name );                       
                   return;
                 }
             }
        }
    }
       
    send_to_char("Syntax: mobbuy <list>\n\rSyntax: mobbuy buy <name>",ch); 
    return; 
}

void do_homeb( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *architect;
    char arg[MIL];
    int osec;
    char buf[MSL];
    int i,i2;
    int cost = 100000;
    HOUSE_DATA *hOuse;

    architect = find_architect( ch );

    if (!architect)
        return;
 
    argument = one_argument( argument, arg );

    if (IS_NPC(ch))
    {
	send_to_char("Sorry, you'll have to just wander, mobile.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' && HAS_HOME( ch ) )
    {
        send_to_char( "You already bought a house. To sell it type house <sell>.\n\r", ch );
        send_to_char( "You can also value your house. Type house <value>\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "sell" ) && HAS_HOME( ch ) )
    {
        home_sell( ch, cost );
        return;
    }

    if ( !str_cmp( arg, "value" ) && HAS_HOME( ch ) )
    {
        int value,t;
       
        for ( hOuse = house_list; hOuse != NULL; hOuse = hOuse->next )
        {                                                     
             if ( !str_cmp( ch->name, hOuse->oname ) ) 
                 break;

                 value += ( hOuse->ovalue + hOuse->mvalue );
                 t++;
                 save_house();
    
        }

        printf_to_char( ch, "You have a total of %d mobs and objects in your house.\n\r", t );
        printf_to_char( ch, "Your house is valued at a total of %d gold.\n\r", ( cost / 2 ) + value );
        return;        
    }

    if ( HAS_HOME( ch ) )
    {
        send_to_char( "You already own a house. To sell it type house <sell>.\n\r", ch );
        return;
    }

    if (ch->gold < cost)
    {
        printf_to_char( ch, "I'm sorry but it cost %d gold to buy a house.\n\r", cost );
	return;
    }

    i = VNUM_START;

    while (get_room_index(i) != NULL)
    {
	i++;

	if (i > VNUM_STOP - 5)
	{
            send_to_char("Sorry all of the house vnums are used up! Tell an immortal.\n\r", ch );
	    return;
	}
    }

    send_to_char("Okay... Attempting creation of your home.\n\r\n\r",ch);
    ch->gold -= cost;
    osec = ch->pcdata->security;  
    ch->pcdata->security = 5;

    for (i2 = 4;i2 >= 0;i2--)
    {
    	sprintf(buf,"%d",i+i2);
 	
  	if( !redit_create( ch, buf ) )
	{
      	   send_to_char("Unable to create your house. Contact an Immortal.\n\r",ch);
	   return;
	}
		
        ch->desc->editor = ED_ROOM;
        char_from_room( ch );
        char_to_room( ch, (ROOM_INDEX_DATA *)ch->desc->pEdit );
        SET_BIT( ((ROOM_INDEX_DATA *)ch->desc->pEdit)->area->area_flags, AREA_CHANGED );
	sprintf(buf,"%s's Home",ch->name);
	redit_name(ch,buf);
    }

/*
 *  Link the rooms together
 *      3
 *      |
 *   2--1--4
 *      |
 *	5
 */

    sprintf(buf,"link %d",i+1);
    change_exit(ch,buf,DIR_WEST);
    sprintf(buf,"link %d",i+2);
    change_exit(ch,buf,DIR_NORTH);
    sprintf(buf,"link %d",i+3);
    change_exit(ch,buf,DIR_EAST);
    sprintf(buf,"link %d",i+4);
    change_exit(ch,buf,DIR_SOUTH);
    if ( osec > 1 )
    ch->pcdata->security = osec;
    send_to_char("\n\rHURRAY! Your house was made successfully.\n\r",ch);
    ch->pcdata->h_vnum = i;
    save_area(ch->in_room->area);
    edit_done(ch);
    return;
}

void do_hname( CHAR_DATA *ch, char *argument )
{
    int cost = 500;

    if ( !HAS_HOME( ch ) )
    {
        send_to_char("If you only had a home...\n\r",ch);
        return;
    }

    if ( !IS_HOME( ch ) )
    {
        send_to_char( "You've got to be in your house to do that.\n\r", ch );
	return;

    }

    if (ch->gold < cost)
    {
        printf_to_char( ch, "It cost %d to change the name of a room in your house.\n\r", cost );
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Change the name to what?\n\r",ch);
	return;
    }

    ch->gold -= cost;
    send_to_char("Ok.\n\r",ch);
    redit_name(ch,argument);
    save_area(ch->in_room->area);
    edit_done(ch);
    return;
}

void do_hdesc( CHAR_DATA *ch, char *argument )
{
    int cost = 500;

    if ( !HAS_HOME( ch ) )
    {
        send_to_char("If you only had a home...\n\r",ch);
        return;
    }

    if ( !IS_HOME( ch ) )
    {
	send_to_char("You gotta be in your house.\n\r",ch);
	return;
    }

    if (ch->gold < cost)
    {
        printf_to_char( ch, "It cost % gold to change your description.\n\r", cost );
	return;
    }

    ch->gold -= cost;
    send_to_char("Ok.\n\r",ch);
    ch->desc->pEdit     = ch->in_room;
    ch->desc->editor    = 2;
    ch->pcdata->hchange	= TRUE;
    redit_desc(ch,"");
    edit_done(ch);
    save_area(ch->in_room->area);
    return;
}

void do_home( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

    if (IS_NPC(ch) && IS_SET(ch->act,ACT_PET))
    {
        send_to_char("Only players can go home.\n\r",ch);
        return;
    }

    if ( !HAS_HOME( ch ) )
    {
	send_to_char("If you only had a home...\n\r",ch);
	return;
    }

    if(IS_NPC(ch))
	location = get_room_index(ch->master->pcdata->h_vnum);
    else
	location = get_room_index(ch->pcdata->h_vnum);

    if (location == NULL)
    {
        send_to_char( "Yikes write a note to immortal and let them know your house is Null.\n\r", ch );
	return;
    }

    act( "$n prays for transportation!", ch, 0, 0, TO_ROOM );

    if ( ch->in_room == location )
        return;
   
    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_AFFECTED(ch, AFF_CURSE))
    {
        send_to_char( "ShadowStorm has forsaken you.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) != NULL )
    {
	int lose,skill;

	skill = get_skill(ch,skill_lookup("recall"));

	if ( number_percent() < 80 * skill / 100 )
	{
	    check_improve(ch,skill_lookup("recall"),FALSE,6);
	    WAIT_STATE( ch, 4 );
	    sprintf( buf, "You failed!.\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	lose = (ch->desc != NULL) ? 25 : 50;
	gain_exp( ch, 0 - lose );
	check_improve(ch,skill_lookup("recall"),TRUE,4);
	sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	send_to_char( buf, ch );
	stop_fighting( ch, TRUE );
    }

    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );

    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );

    if (ch->pet != NULL)
        do_home(ch->pet,"");
 
    if (ch->mount != NULL)
    {
        char_from_room( ch->mount );
        char_to_room( ch->mount, ch->in_room );
    }   

    return;
}

void do_invite(CHAR_DATA *ch,char *argument)
{
    CHAR_DATA *victim;
    char buf[MSL];

    if (argument[0] == '\0')
    {
	send_to_char("Invite whom to your home?\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {
	send_to_char("Who is that?\n\r",ch);
	return;
    }

    if ( !IS_HOME( ch ) )
    {
        send_to_char("Can't invite someone to your house when you aren't in it.\n\r",ch);
	return;
    }

    if (ch == victim)
    {
	send_to_char("Lonely person, huh?\n\r",ch);
	return;
    }
 
    sprintf(buf,"%s has invited you to come to their house!\n\r",ch->name);
    send_to_char(buf,victim);

    send_to_char("You have invited them to come in.\n\r",ch);
    victim->pcdata->hinvite = ch->pcdata->h_vnum;
}

void do_join(CHAR_DATA *ch,char *argument)
{
    int ohvnum = 0;

    if (ch->pcdata->hinvite == 0)
    {
	send_to_char("You haven't been invited anywhere.\n\r",ch);
	return;
    }

    ohvnum = ch->pcdata->h_vnum;
    ch->pcdata->h_vnum = ch->pcdata->hinvite;
    do_home(ch,"");
    ch->pcdata->h_vnum = ohvnum;
    ch->pcdata->hinvite = 0;
}

void home_sell( CHAR_DATA *ch, int cost )
{
    char buf[MSL];
    int i,t;
    HOUSE_DATA *hOuse;
    int value;

    send_to_char( "Ok. Attempting to sell your home.\n\r\n\r", ch );
    
    for ( i = ch->pcdata->h_vnum; i < ch->pcdata->h_vnum + 5; i++ )
    {
         sprintf( buf, "%d", i );
         redit_delete( ch, buf );
    }            

    if ( ch->pcdata->security <= 5 )
    ch->pcdata->security = 0;
    ch->gold += ( cost / 2 );
    ch->pcdata->h_vnum = 0;
    send_to_char( "\n\rYou have successfully sold your house.\n\r", ch );

    for ( hOuse = house_list; hOuse != NULL; hOuse = hOuse->next )
    {                                                     
         if ( !str_cmp( ch->name, hOuse->oname ) ) 
             break;

         value += ( hOuse->ovalue + hOuse->mvalue );
         t++;
         save_house();
    }

    printf_to_char( ch, "You recieve %d gold for selling it.\n\r", ( cost / 2 ) + value );
    printf_to_char( ch, "You had a total of %d mobs and objects in your house.\n\r", t );
    return;
}

void do_boot( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( argument[0] == '\0' )
    {     
        send_to_char( "Who would you like to boot out of your house?\n\r", ch );
        return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {
        send_to_char("Who is that?\n\r",ch);
        return;
    }                  
    
    if ( !IS_HOME( ch ) )
    {
        send_to_char("You can't boot someone if your not in your house!\n\r", ch );
        return;
    }            

    if ((victim->in_room->vnum < ch->pcdata->h_vnum) || (victim->in_room->vnum > ch->pcdata->h_vnum + 4))
    {
        send_to_char("You can't boot someone if their not in your house!\n\r",ch);
        return;
    }       

    printf_to_char( ch, "You get in %s's face and yell at %s to get out of your house!\n\r", victim->name, victim->sex == 0 ? "It" : victim->sex == 1 ? "Him" : "Her" );
    printf_to_char( victim, "%s begins yelling at you, and tells you to get out of %s house!\n\r", ch->name, ch->sex == 0 ? "It's" : ch->sex == 1 ? "His" : "Her" );  
    printf_to_char( victim, "You have been kicked out of %s's house!\n\r", ch->name );  
    do_recall( victim, "" );
    sound( "gout.wav", victim );
    sound( "gout.wav", ch );
    return;
}

void save_house()
{
  FILE *fp;
  HOUSE_DATA *hOuse;

  if ( ( fp = file_open( HOUSE_FILE, "w" ) ) == NULL )
  {
      logf2("The house file is gone!\n\r");
      return;
  }              

  for ( hOuse = house_list; hOuse != NULL; hOuse = hOuse->next )
  {
       fprintf(fp,"Oname %s~\n", hOuse->oname );
       fprintf(fp,"Objname %s~\n", hOuse->objname );
       fprintf(fp,"Mobname %s~\n", hOuse->mobname );
       fprintf(fp,"Ovalue %d\n", hOuse->ovalue );
       fprintf(fp,"Mvalue %d\n\n", hOuse->mvalue );
 }

 fprintf(fp, "$\n");
 logf2("House saved.");
 file_close(fp);
 return;
}


void load_house()
{
  FILE *fp;
  const char *word;
  HOUSE_DATA *hlist;
  HOUSE_DATA *hOuse;

  if (file_exists(HOUSE_FILE))
  {
      fp = file_open( HOUSE_FILE, "r" );

      hlist = NULL;

      for( ; ; )
      {                      
          word = feof( fp ) ? "End" : fread_word( fp );

          if(word[0] == '$' )
             return;

          if (!str_cmp(word, "Oname" ) )
          {
              hOuse = new_house();
              if (house_list == NULL)
              house_list = hOuse;
              else
              hlist->next = hOuse;
              hlist = hOuse;
              hOuse->oname = str_dup(fread_string(fp));
          }                                 
         
          if (!str_cmp(word, "Objname" ) )
              hOuse->objname = str_dup(fread_string(fp));
            
          if (!str_cmp(word, "Mobname" ) )
              hOuse->mobname = str_dup(fread_string(fp));

          if (!str_cmp(word, "Ovalue" ) )
              hOuse->ovalue = fread_number(fp);
          
          if (!str_cmp(word, "Mvalue" ) )
              hOuse->mvalue = fread_number(fp);
      }
  }

  return;

}

