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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "include.h"

int mortal_online args (());
int immortal_online args (());
void *get_pcdata args( (CHAR_DATA *ch) );

int MAX_SKILL;

char *	const	where_name	[] =
{
"{r<{Dused as light{r>          ",
"{r<{Dworn as hood{r>           ",
"    {r<{Dworn on head{r>           ",
"{r<{Dworn as cloak{r>{W        ",
"    {r<{Dworn around neck{r>{W		",
"    {r<{Dworn around neck{r>{W     ",
"    {r<{Dworn on torso{r>{W	    ",
"    {r<{Dworn on back{r>{W         ",
"    {r<{Dworn on shoulder{r>{W     ",
"{r<{Dworn around arms{r>{W     ",
"    {r<{Dworn on arms{r>{W         ",
"    {r<{Dworn around wrist{r>{W    ",
"    {r<{Dworn around wrist{r>{W    ",
"    {r<{Dworn on hands{r>{W        ",
"    {r<{Dworn on finger{r>{W       ",
"    {r<{Dworn on finger{r>{W       ",
"    {r<{Dwedding ring{r>{W         ",
"{r<{Dworn around body{r>{W     ",
"    {r<{Dworn about body{r>{W      ",
"    {r<{Dworn about waist{r>{W     ",
"{r<{Dworn around legs{r>{W     ",
"    {r<{Dworn on legs{r>{W         ",
"    {r<{Dworn on feet{r>{W         ",
"{r<{Dworn as shield{r>{W       ",
"{r<{Dwielded{r>{W              ",
"{r<{Dsecondary weapon{r>{W     ",
"{r<{Dheld{r>{W                 ",
"{r<{Dfloating nearby{r>{W      ",
"{W<{Rlodged in a leg{W>        ",
"{W<{Rlodged in an arm{W>       ",
"{W<{Rlodged in a rib{W>        ",
};


/* for  keeping track of the player count */
int max_on = 0;

sh_int count_connections;

/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );
void    look_sky                args( ( CHAR_DATA *ch ) );
#define MAX_NEST        100
static  OBJ_DATA *      rgObjNest       [MAX_NEST];
void    read_version            args( ( char *version ) );
bool    write_version           args( ( char *argument ) );
int     moon_status             args( (                ) );

extern  void    fread_char      args( ( CHAR_DATA *ch,  FILE *fp ) );   


int compare_mdeaths(const void *v1, const void *v2)
{
    return (*(MOB_INDEX_DATA * *)v2)->kills - (*(MOB_INDEX_DATA * *)v1)->killed;
}


char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];
  //  char cmdbuf[MSL];
    buf[0] = '\0';

    if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
    ||  (obj->description == NULL || obj->description[0] == '\0'))
	return buf;

    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "(Invis) "     );
    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
         && IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "(Red Aura) "  );
    if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
    &&  IS_OBJ_STAT(obj,ITEM_BLESS))	      strcat(buf,"(Blue Aura) "	);
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
         && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "(Magical) "   );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "(Glowing) "   );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "(Humming) "   );
    if (IS_QUESTOR(ch) && (obj->pIndexData->vnum == ch->pcdata->questobj))
	strcat(buf, "{r[{RTARGET{x{r] {x");
    if (IS_OBJ_STAT(obj, ITEM_AUCTIONED) )    strcat(buf,  "(Auctioned) ");    
    if(IS_OBJ_STAT(obj,ITEM_HIDDEN))          strcat(buf,  "(Hidden) ");

    if ( fShort )
    {
	if ( obj->short_descr != NULL )
        {
            if (IS_MXP( ch ) )
            SNP( buf, MXP_SECURE "<send href=\"look %s|eat %s|wear %s|remove %s|drop %s\" hint=\"Click to see menu|look|eat|wear|remove|drop\">%s</Send>" MXP_LLOCK, obj->name, obj->name, obj->name, obj->name, obj->name, obj->short_descr );
//            SNP( cmdbuf, "look %s", obj->name );
//            strcat( buf, click_cmd( ch->desc, obj->short_descr, cmdbuf, "Look at" ) ); 
            else
	    strcat( buf, obj->short_descr );
        }
    }
    else
    {
	if ( obj->description != NULL)
        {
            if ( IS_MXP( ch ) )
            SNP( buf, MXP_SECURE "<send href=\"look %s|get %s|drink %s|get all\" hint=\"Click to see menu|look|get|drink|get all\">%s</Send>" MXP_LLOCK, obj->name, obj->name, obj->name, obj->description );
//            SNP( cmdbuf, "look %s", obj->name );
//            strcat( buf, click_cmd( ch->desc, obj->description, cmdbuf, "Look at" ) ); 
            else
	    strcat( buf, obj->description );
        }
    }

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    output = new_buf();

    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    if ( count > 5000 )
    {
       send_to_char( "That is WAY too much junk!  Drop some of it!\n\r", ch );
       return;
    }
    prgpstrShow	= (char **)alloc_mem( count * sizeof(char *) );
    prgnShow    = (int *)alloc_mem( count * sizeof(int)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && obj->belted_vnum <= 0 && can_see_obj( ch, obj ) && obj->strap_loc < 0) 
	{
	    if(!obj->carried_by && !IS_NPC(ch) && !OBJ_SAME_SHAFT(ch, obj) )
	       continue;

	    pstrShow = format_obj_to_char( obj, ch, fShort );

	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if (prgpstrShow[iShow][0] == '\0')
	{
	    free_string(prgpstrShow[iShow]);
	    continue;
	}

	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		add_buf(output,buf);
	    }
	    else
	    {
		add_buf(output,"     ");
	    }
	}
	add_buf(output,prgpstrShow[iShow]);
	add_buf(output,"\n\r");
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }
    page_to_char(buf_string(output),ch);

    /*
     * Clean up.
     */
    free_buf(output);
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}


void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH],message[MAX_STRING_LENGTH];
    char cmdbuf[MSL];

    buf[0] = '\0';

    if ( RIDDEN(victim) )
    {
	    if ( ch != RIDDEN(victim) )
		strcat( buf, "(Ridden) " );
	    else
	        strcat( buf, "(Your mount) " );
    }
    if ( IS_SET(victim->comm,COMM_AFK	  )   ) strcat( buf, "{Y[AFK]{x "	     );
    if ( IS_SET(victim->comm2,COMM_AFK    )   ) strcat( buf, "{Y[Auto AFK]{x "   );
    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "(Invis) "      );
    if ( victim->invis_level >= LEVEL_HERO    ) strcat( buf, "{B(Wizi){x "	     );
    if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "(Hide) "       );
    if ( IS_AFFECTED(victim, AFF_CHARM)       ) strcat( buf, "(Charmed) "    );
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "{C(Translucent){x ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "(Pink Aura) "  );
    if ( IS_EVIL(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) strcat( buf, "{R(Red Aura){x "   );
    if ( IS_GOOD(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_GOOD)     ) strcat( buf, "(Golden Aura) ");
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) strcat( buf, "{W(White Aura){x " );
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_WAR) ) 
                                                strcat( buf, "{R(WAR){x " );    
    if (!IS_NPC(victim) && !victim->desc      ) strcat( buf, "{r({RLinkdead{r){x " );        
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER ) )
						strcat( buf, "(KILLER) "     );
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF  ) )
						strcat( buf, "(THIEF) "      );
    if (IS_QUESTOR(ch) && IS_NPC(victim) && (victim->pIndexData->vnum == ch->pcdata->questmob))
	                                        strcat(buf,  "{r[{RTARGET{x{r]{x ");
    if ( !IS_NPC(victim) && victim->pcdata->spouse[0] != '\0' ) 
                                                strcat(buf, "[Wed] ");
/*    if (IS_NPC(victim) && is_gqmob(ch, victim->pIndexData->vnum) != 0)
    {
            strcat(buf, "(Gquest) ");
    }    */

    if ( victim->position == victim->start_pos && victim->long_descr[0] != '\0' )
    {    
        SNP( cmdbuf, "click %s", victim->name );
        strcat( buf, click_cmd( ch->desc, victim->long_descr, cmdbuf, "Menu" ) );
	//strcat( buf, victim->long_descr );
        strcat( buf, "\r\n" );
	send_to_char( buf, ch );
	return;
    }

    if ( !str_cmp( victim->name, "Synon" ) )
    {
       do_image( ch, "axew.bmp" );
    }
   strcat( buf, PERS( victim, ch ) );

   switch ( victim->position )
   {
    case POS_DEAD:     strcat( buf, " is DEAD!!" );              break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded." );   break;
    case POS_INCAP:    strcat( buf, " is incapacitated." );      break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
    case POS_SLEEPING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],SLEEP_AT))
  	    {
		sprintf(message," is sleeping at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],SLEEP_ON))
	    {
		sprintf(message," is sleeping on %s.",
		    victim->on->short_descr); 
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message, " is sleeping in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else 
	    strcat(buf," is sleeping here.");
	break;
    case POS_RESTING:  
        if (victim->on != NULL)
	{
            if (IS_SET(victim->on->value[2],REST_AT))
            {
                sprintf(message," is resting at %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else if (IS_SET(victim->on->value[2],REST_ON))
            {
                sprintf(message," is resting on %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else 
            {
                sprintf(message, " is resting in %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
	}
        else
	    strcat( buf, " is resting here." );       
	break;
    case POS_SITTING:  
        if (victim->on != NULL)
        {
            if (IS_SET(victim->on->value[2],SIT_AT))
            {
                sprintf(message," is sitting at %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else if (IS_SET(victim->on->value[2],SIT_ON))
            {
                sprintf(message," is sitting on %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else
            {
                sprintf(message, " is sitting in %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
        }
        else
	    strcat(buf, " is sitting here.");
	break;
    case POS_STANDING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],STAND_AT))
	    {
		sprintf(message," is standing at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],STAND_ON))
	    {
		sprintf(message," is standing on %s.",
		   victim->on->short_descr);
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message," is standing in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else if ( MOUNTED(victim) )
	{
	    strcat( buf, " is here and is riding " );
	    strcat( buf, MOUNTED(victim)->short_descr );
	}	
	else
	    strcat( buf, " is here." );
	break;
    case POS_FIGHTING:
	strcat( buf, " is here, fighting " );
	if ( victim->fighting == NULL )
	    strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "someone who left??" );
	break;
    }

    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}


void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj, *temp;
    int iWear;
    int percent;
    bool found;
    bool beltfound = FALSE;

    if ( can_see( victim, ch ) )
    {
	if (ch == victim)
	    act( "$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
	else
	{
	    act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	    act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
	}
    }

    if ( victim->description[0] != '\0' )
    {
	send_to_char( victim->description, ch );
    }

    else
    {
       if (!IS_NPC (victim))
       {
          sprintf (buf, "%s the %s %s has",
            PERS(victim,ch), race_table[victim->race].name, victim->sex == 0 ? "it" : victim->sex == 1 ? "male" : "female");
          send_to_char (buf, ch);
          sprintf (buf, " %s eyes and %s hair. %s body is of a %s build, and %s\n\r%s in stature.\n\r",
			eye_table[victim->pcdata->eye].name, 
			hair_table[victim->pcdata->hair].name,
			victim->sex == 0 ? "Its" : victim->sex == 1 ? "His" : "Her", 
			weight_table[victim->pcdata->weight].name, 
			victim->sex == 0 ? "it is" : victim->sex == 1 ? "he's" : "she's",
			height_table[victim->pcdata->height].name );
                 
          send_to_char (buf, ch);
         }

  	 act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
         display_bp(ch, victim);

    }

    if ( MOUNTED(victim) )
    {
	    sprintf( buf, "%s is riding %s.\n\r", victim->name, MOUNTED(victim)->short_descr);
	    send_to_char( buf, ch);
    }
    if ( RIDDEN(victim) )
    {
        sprintf( buf, "%s is being ridden by %s.\n\r", victim->short_descr, RIDDEN(victim)->name );
        send_to_char( buf, ch);
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS(victim, ch) );

    if (percent >= 100) 
	strcat( buf, " is in excellent condition.\n\r");
    else if (percent >= 90) 
	strcat( buf, " has a few scratches.\n\r");
    else if (percent >= 75) 
	strcat( buf," has some small wounds and bruises.\n\r");
    else if (percent >=  50) 
	strcat( buf, " has quite a few wounds.\n\r");
    else if (percent >= 30)
	strcat( buf, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
	strcat ( buf, " looks pretty hurt.\n\r");
    else if (percent >= 0 )
	strcat (buf, " is in awful condition.\n\r");
    else
	strcat(buf, " is bleeding to death.\n\r");

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    if( (obj = get_eq_char(victim,WEAR_CLOAK) ) != NULL && obj->part == FALSE)
    {
	send_to_char("You can't seem to see their belongings.\n\r",ch);
	sprintf(buf,"{r:{Dworn as cloak{r:{W     %s\n\r",obj->short_descr);
	send_to_char(buf,ch);
	return;
    }

    found = FALSE;

    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
		if ( ( obj = get_eq_char( victim, iWear ) ) == NULL )
		{
			show_strapped(ch,victim,iWear);
			continue;
		}

		if ( can_see_obj( ch, obj ) && obj->item_type != ITEM_BELT )
		{
			if ( !found )
			{
				send_to_char( "\n\r", ch );
				act( "$N is using:", ch, NULL, victim, TO_CHAR );
				found = TRUE;
			}
			    if( get_eq_char(victim, WEAR_CLOAK ) != NULL  && 
			      (iWear > WEAR_CLOAK && iWear < WEAR_LAYER_ARMS ))
					continue;
			    if( get_eq_char(victim, WEAR_LAYER_ARMS) != NULL && 
			      (iWear > WEAR_LAYER_ARMS && iWear < WEAR_LAYER_BODY) )
					continue;
			    if( get_eq_char(victim, WEAR_LAYER_BODY) != NULL && 
				  (iWear > WEAR_LAYER_BODY && iWear < WEAR_LAYER_LEGS) )
					continue;
			    if( get_eq_char(victim, WEAR_LAYER_LEGS) != NULL && 
				  (iWear > WEAR_LAYER_LEGS && iWear < WEAR_SHIELD) )
					continue;

			    send_to_char( where_name[iWear], ch );
			    send_to_char( "{x",ch);
			    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
			    send_to_char( "\n\r", ch );
		    	    show_strapped(ch,victim,iWear);
		}
    }

    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
		if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
		&&   can_see_obj( ch, obj ) && obj->item_type == ITEM_BELT )
		{
			if ( !found )
			{
				send_to_char( "\n\r", ch );
				act( "$N is using:\n\r", ch, NULL, victim, TO_CHAR );
				found = TRUE;
			}
			printf_to_char( ch, "{r:{Dstrapped to %s{r:{W\n\r", obj->short_descr );
			for ( temp = victim->carrying; temp; temp = temp->next_content )
			{
				if (temp->belted_vnum == obj->pIndexData->vnum)
				{
					printf_to_char( ch, "   %s\n\r", temp->short_descr );
					beltfound = TRUE;
				}		
			}
			if ( !beltfound )
				send_to_char( "   Nothing.\n\r", ch );
		}	
	}
  
    if ( victim != ch
    &&   !IS_NPC(ch)
    &&   number_percent( ) < get_skill(ch,skill_lookup("peek")))
    {
	send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
	check_improve(ch,skill_lookup("peek"),TRUE,4);
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch 
	    || ( RIDDEN(rch) && rch->in_room == RIDDEN(rch)->in_room && RIDDEN(rch) != ch ) )
	    continue;

	if ( get_trust(ch) < rch->invis_level)
	    continue;
	if (!IS_NPC(ch) && !IS_NPC(rch) && (IN_MINE(ch) || IN_MINE(rch) ) && !IS_SAME_SHAFT(ch,rch) )
	    continue; 
	if(!IS_NPC(ch) && IS_NPC(rch) && IN_MINE(ch) )
	    continue;
	if ( can_see( ch, rch )  )
	{
	    show_char_to_char_0( rch, ch );
            if( MOUNTED(rch) && (rch->in_room == MOUNTED(rch)->in_room) )
            show_char_to_char_0( MOUNTED(rch), ch );	
	}
	else if ( room_is_dark( ch->in_room )
	&&        IS_AFFECTED(rch, AFF_INFRARED ) )
	{
	    send_to_char( "You see glowing red eyes watching YOU!\n\r", ch );
	}
    }

    return;
} 



bool check_blind( CHAR_DATA *ch )
{

    if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
	send_to_char( "You can't see a thing!\n\r", ch ); 
	return FALSE; 
    }

    return TRUE;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (ch->lines == 0)
	    send_to_char("You do not page long messages.\n\r",ch);
	else
	{
	    sprintf(buf,"You currently display %d lines per page.\n\r",
		    ch->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("You must provide a number.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
        send_to_char("Paging disabled.\n\r",ch);
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("You must provide a reasonable number.\n\r",ch);
	return;
    }

    sprintf(buf,"Scroll set to %d lines.\n\r",lines);
    send_to_char(buf,ch);
    ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;
     
    col = 0;
   
    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
	sprintf(buf,"%-12s",social_table[iSocial].name);
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("\n\r",ch);
    }

    if ( col % 6 != 0)
	send_to_char("\n\r",ch);
    return;
}


 
/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_motd(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "motd");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{  
    do_function(ch, &do_help, "imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "rules");
}

void do_story(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "story");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    send_to_char("   {Caction     status{x\n\r",ch);
    send_to_char("{cO-----------------------------------O{x\n\r",ch);
 
    send_to_char("{c|{x autoassist     ",ch);
    if (IS_SET(ch->act,PLR_AUTOASSIST))
        send_to_char("ON                 {c|{x\n\r",ch);
    else
        send_to_char("OFF                {c|{x\n\r",ch); 
    send_to_char("{c|{x autodamage     ",ch);
    if (IS_SET(ch->act,PLR_AUTODAMAGE))
        send_to_char("ON                 {c|{x\n\r",ch);
    else
        send_to_char("OFF                {c|{x\n\r",ch);

    send_to_char("{c|{x autoexit       ",ch);
    if (IS_SET(ch->act,PLR_AUTOEXIT))
        send_to_char("ON                 {c|{x\n\r",ch);
    else
        send_to_char("OFF                {c|{x\n\r",ch);

    send_to_char("{c|{x autoweather    ",ch);
    if (IS_SET(ch->act,PLR_AUTOWEATHER))
        send_to_char("ON                 {c|{x\n\r",ch);
    else
        send_to_char("OFF                {c|{x\n\r",ch);

    send_to_char("{c|{x autogold       ",ch);
    if (IS_SET(ch->act,PLR_AUTOGOLD))
        send_to_char("ON                 {c|{x\n\r",ch);
    else
        send_to_char("OFF                {c|{x\n\r",ch);

    send_to_char("{c|{x autoloot       ",ch);
    if (IS_SET(ch->act,PLR_AUTOLOOT))
        send_to_char("ON                 {c|{x\n\r",ch);
    else
        send_to_char("OFF                {c|{x\n\r",ch);

    send_to_char("{c|{x autosac        ",ch);
    if (IS_SET(ch->act,PLR_AUTOSAC))
        send_to_char("ON                 {c|{x\n\r",ch);
    else
        send_to_char("OFF                {c|{x\n\r",ch);

    send_to_char("{c|{x autosplit      ",ch);
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
        send_to_char("ON                 {c|{x\n\r",ch);
    else
        send_to_char("OFF                {c|{x\n\r",ch);
  
    send_to_char("{c|{x autolevel      ",ch);
    if (IS_SET(ch->comm2, COMM_AUTOLEVEL))
        send_to_char("ON                 {c|{x\n\r",ch);
    else
        send_to_char("OFF                {c|{x\n\r",ch);
    
    send_to_char("{c|{x automap        ",ch);
    if(IS_SET(ch->act, PLR_AUTOMAP ) )
        send_to_char("ON                 {c|{x\r\n",ch);
    else
        send_to_char("OFF                {c|{x\r\n",ch);

    send_to_char("{c|{x pet leveling   ",ch);
    if (IS_SET(ch->comm2, COMM_PETLEVEL))
        send_to_char("ON                 {c|{x\n\r",ch);
    else
        send_to_char("OFF                {c|{x\n\r",ch);

    send_to_char("{c|{x compact mode   ",ch);
    if (IS_SET(ch->comm,COMM_COMPACT))
        send_to_char("ON                 {c|{x\n\r",ch);
    else
        send_to_char("OFF                {c|{x\n\r",ch);

    send_to_char("{c|{x prompt         ",ch);
    if (IS_SET(ch->comm,COMM_PROMPT))
	send_to_char("ON                 {c|{x\n\r",ch);
    else
	send_to_char("OFF                {c|{x\n\r",ch);

    send_to_char("{c|{x combine items  ",ch);
    if (IS_SET(ch->comm,COMM_COMBINE))
	send_to_char("ON                 {c|{x\n\r",ch);
    else
	send_to_char("OFF                {c|{x\n\r",ch);

    send_to_char("{c|{x mxp enabled    ",ch);
    if(ch->desc->mxp)
        send_to_char("Enabled            {c|{x\r\n",ch);
    else
	send_to_char("Disabled           {c|{x\r\n",ch);

    send_to_char("{cO-----------------------------------O{x\n\r",ch);

    if (!IS_SET(ch->act,PLR_CANLOOT))
	send_to_char("{c|{x Your corpse is safe from thieves. {c|{x\n\r",ch);
    else 
        send_to_char("{c|{x Your corpse may be looted.        {c|{x\n\r",ch);

    if (IS_SET(ch->act,PLR_NOSUMMON))
	send_to_char("{c|{x You cannot be summoned.           {c|{x\n\r",ch);
    else
	send_to_char("{c|{x You can be summoned.              {c|{x\n\r",ch);
   
    if (IS_SET(ch->act,PLR_NOFOLLOW))
	send_to_char("{c|{x You do not welcome followers.     {c|{x\n\r",ch);
    else
	send_to_char("{c|{x You accept followers.             {c|{x\n\r",ch);

    send_to_char("{cO-----------------------------------O{x\n\r",ch);

}

void do_autoassist(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    
    if (IS_SET(ch->act,PLR_AUTOASSIST))
    {
      send_to_char("Autoassist removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOASSIST);
    }
    else
    {
      send_to_char("You will now assist when needed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOASSIST);
    }
}

void do_autodamage(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
     return;

    if (IS_SET(ch->act,PLR_AUTODAMAGE))
    {
        send_to_char("Autodamage is currently OFF.\n\r",ch);      
        REMOVE_BIT(ch->act,PLR_AUTODAMAGE);    
    }    
    else
    {            
        send_to_char("Autodamage is currently ON.\n\r",ch);
        SET_BIT(ch->act,PLR_AUTODAMAGE); 
    }
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOEXIT))
    {
      send_to_char("Exits will no longer be displayed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOEXIT);
    }
    else
    {
      send_to_char("Exits will now be displayed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOEXIT);
    }
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOGOLD))
    {
      send_to_char("Autogold removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOGOLD);
    }
    else
    {
      send_to_char("Automatic gold looting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOGOLD);
    }
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOLOOT))
    {
      send_to_char("Autolooting removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOLOOT);
    }
    else
    {
      send_to_char("Automatic corpse looting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOLOOT);
    }
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSAC))
    {
      send_to_char("Autosacrificing removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSAC);
    }
    else
    {
      send_to_char("Automatic corpse sacrificing set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSAC);
    }
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
    {
      send_to_char("Autosplitting removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
    }
    else
    {
      send_to_char("Automatic gold splitting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSPLIT);
    }
}
void do_automap(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOMAP))
    {
      send_to_char("Automap removed from descriptions.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOMAP);
    }
    else
    {
      send_to_char("Automap in descriptions set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOMAP);
    }
}

void do_brief(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_BRIEF))
    {
      send_to_char("Full descriptions activated.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_BRIEF);
    }
    else
    {
      send_to_char("Short descriptions activated.\n\r",ch);
      SET_BIT(ch->comm,COMM_BRIEF);
    }
}

void do_compact(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMPACT))
    {
      send_to_char("Compact mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMPACT);
    }
    else
    {
      send_to_char("Compact mode set.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMPACT);
    }
}

void do_show(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
    {
      send_to_char("Affects will no longer be shown in score.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_SHOW_AFFECTS);
    }
    else
    {
      send_to_char("Affects will now be shown in score.\n\r",ch);
      SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
    }
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
 
   if ( argument[0] == '\0' )
   {
	if (IS_SET(ch->comm,COMM_PROMPT))
   	{
      	    send_to_char("You will no longer see prompts.\n\r",ch);
      	    REMOVE_BIT(ch->comm,COMM_PROMPT);
    	}
    	else
    	{
      	    send_to_char("You will now see prompts.\n\r",ch);
      	    SET_BIT(ch->comm,COMM_PROMPT);
    	}
       return;
   }
 
   if( !strcmp( argument, "all" ) )
      strcpy( buf, "<%hhp %mm %vmv> ");
   else
   {
      if ( strlen(argument) > 50 )
         argument[50] = '\0';
      strcpy( buf, argument );
      smash_tilde( buf );
      if (str_suffix("%c",buf))
	strcat(buf," ");
	
   }
 
   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   sprintf(buf,"Prompt set to %s\n\r",ch->prompt );
   send_to_char(buf,ch);
   return;
}

void do_combine(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMBINE))
    {
      send_to_char("Long inventory selected.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    }
    else
    {
      send_to_char("Combined inventory selected.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}

void do_noloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_CANLOOT))
    {
      send_to_char("Your corpse is now safe from thieves.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_CANLOOT);
    }
    else
    {
      send_to_char("Your corpse may now be looted.\n\r",ch);
      SET_BIT(ch->act,PLR_CANLOOT);
    }
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
      send_to_char("You now accept followers.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
    else
    {
      send_to_char("You no longer accept followers.\n\r",ch);
      SET_BIT(ch->act,PLR_NOFOLLOW);
      die_follower( ch );
    }
}

void do_nosummon(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
      if (IS_SET(ch->imm_flags,IMM_SUMMON))
      {
	send_to_char("You are no longer immune to summon.\n\r",ch);
	REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
      }
      else
      {
	send_to_char("You are now immune to summoning.\n\r",ch);
	SET_BIT(ch->imm_flags,IMM_SUMMON);
      }
    }
    else
    {
      if (IS_SET(ch->act,PLR_NOSUMMON))
      {
        send_to_char("You are no longer immune to summon.\n\r",ch);
        REMOVE_BIT(ch->act,PLR_NOSUMMON);
      }
      else
      {
        send_to_char("You are now immune to summoning.\n\r",ch);
        SET_BIT(ch->act,PLR_NOSUMMON);
      }
    }
}

void do_mxp(CHAR_DATA *ch, char *argument )
{	if(IS_NPC(ch) )
		return;

	if(ch->desc->mxp)
	{	ch->desc->mxp = FALSE;
		ch->pcdata->mxp = FALSE;

		send_to_char("MXP Disabled\r\n",ch);

		return;
	}
	else
	{	ch->desc->mxp = TRUE;
		ch->pcdata->mxp = TRUE;

		send_to_char("MXP Enabled!\r\n",ch);
		return;
	}
}



void GenerateDetailMap(CHAR_DATA *ch, char *);
void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    int number,count;

    if ( ch->desc == NULL )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room ) )
    {
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    count = 0;

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	if(!IS_NPC(ch) && IN_MINE(ch) )
	{   do_function(ch, &do_look, "mine" );
	    return;
	}
   
	sprintf( buf, "{s%s", ch->in_room->name );
	send_to_char( buf, ch );

        if ( (IS_IMMORTAL(ch) && (IS_NPC(ch) || IS_SET(ch->act,PLR_HOLYLIGHT)))
        ||   IS_BUILDER(ch, ch->in_room->area) )  
	{
	    sprintf(buf," {r[{RRoom %d{r]\n",ch->in_room->vnum);
	    send_to_char(buf,ch);
	}

	send_to_char( "{x\n\r", ch );

        if ( IS_SET(ch->act, PLR_AUTOWEATHER) && IS_OUTDOORS(ch))
        {
            show_weather(ch);
        }

	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
	{       if(IS_SET(ch->act, PLR_AUTOMAP) )
                        GenerateDetailMap(ch, ch->in_room->description);
                else
                        printf_to_char( ch, "{S  %s{x", ch->in_room->description );
	    
	}

        if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
	{
	    send_to_char("\n\r",ch);
            do_function(ch, &do_exits, "auto" );
	}
	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->people,   ch );
	return;
    }

    if( !str_cmp(arg1, "mine" ) )
    {
	if(IS_NPC(ch) )
	{   send_to_char("Nah, mobs don't need to look around mines.\n\r",ch);
	    return;
	}
	if(!IN_MINE(ch) )
	{   send_to_char("You can't look around a mine if you arn't in one.\n\r",ch);
	    return;
	}
	look_mine(ch);
	return;
    }

    if ( !str_cmp( arg1, "sky" ) )
    {
      if ( !IS_OUTSIDE(ch) )
      {
        send_to_char( "You can't see the sky indoors.\n\r", ch );
        return;
      }
      else
      {
        look_sky( ch );
        return;
      }
    }


    if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on"))
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, NULL, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    sprintf( buf, "It's %sfilled with  a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than half-" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about half-"     : "more than half-",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

       case ITEM_QUIVER:
		if ( obj->value[0] <= 0 )
		{
		send_to_char( "{WThe quiver is out of arrows.{x\n\r", ch );
		break;
		}
		
		if (obj->value[0] == 1 )
		{
		send_to_char( "{WThe quiver has 1 arrow remaining in it.{x\n\r", ch );
		break;
		}
		
		if (obj->value[0] > 1 )
		{
		sprintf( buf, "{WThe quiver has %d arrows in it.{x\n\r", obj->value[0]);
		}
		send_to_char( buf, ch);
		break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p holds:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	}
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
        if ( victim->level <= 10 )
        sound( "newbiecry.wav", ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{  /* player can see object */
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    {	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
                    show_obj_cond( ch, obj );
		    return;
	    	}
	    	else continue;
	    }

 	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
 	    if ( pdesc != NULL )
            {
 	    	if (++count == number)
 	    	{	
		    send_to_char( pdesc, ch );
                    show_obj_cond( ch, obj );
		    return;
	     	}
		else continue;
            } 

	    if ( is_name( arg3, obj->name ) )
	    	if (++count == number)
	    	{                    
	    	    send_to_char( obj->description, ch );
	    	    send_to_char( "\n\r",ch);
                    show_obj_cond( ch, obj );
		    return;
		  }
	  }
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
                    show_obj_cond( ch, obj );
		    return;
	    	}

	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
                    show_obj_cond( ch, obj );
		    return;
	    	}

	    if ( is_name( arg3, obj->name ) )
		if (++count == number)
		{
		    send_to_char( obj->description, ch );
		    send_to_char("\n\r",ch);
                    show_obj_cond( ch, obj );
		    return;
		}
	}
    }

    pdesc = get_extra_descr(arg3,ch->in_room->extra_descr);
    if (pdesc != NULL)
    {
	if (++count == number)
	{
	    send_to_char(pdesc,ch);
	    return;
	}
    }
    
    if (count > 0 && count != number)
    {
    	if (count == 1)
    	    sprintf(buf,"You only see one %s here.\n\r",arg3);
    	else
    	    sprintf(buf,"You only see %d of those here.\n\r",count);
    	
    	send_to_char(buf,ch);
    	return;
    }

         if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
    else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
    else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
    else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
    else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
    else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nothing special there.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' ' )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
	else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
	    act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
	}
    }

    return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, char *argument )
{
    do_function(ch, &do_look, argument);
}

void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

   if ( !str_cmp( arg, "box" ) )
        {
            if (ch->in_room->vnum != ROOM_VNUM_RENTBOX )
            {
                send_to_char( "Do you see a box here?\n\r", ch );
                return;
            }

            send_to_char( "You have the following in your box:\n\r", ch );
            show_list_to_char( ch->pcdata->box, ch, TRUE, TRUE );
            return;
        }          

    do_function(ch, &do_look, arg );

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;
	
	case ITEM_JUKEBOX:
	    do_function(ch, &do_play, "list");
	    break;

	case ITEM_MONEY:
	    if (obj->value[0] == 0)
	    {
	        if (obj->value[1] == 0)
		    sprintf(buf,"Odd...there's no coins in the pile.\n\r");
		else if (obj->value[1] == 1)
		    sprintf(buf,"Wow. One gold coin.\n\r");
		else
		    sprintf(buf,"There are %d gold coins in the pile.\n\r",
			obj->value[1]);
	    }
	    else if (obj->value[1] == 0)
	    {
		if (obj->value[0] == 1)
		    sprintf(buf,"Wow. One silver coin.\n\r");
		else
		    sprintf(buf,"There are %d silver coins in the pile.\n\r",
			obj->value[0]);
	    }
	    else
		sprintf(buf,
		    "There are %d gold and %d silver coins in the pile.\n\r",
		    obj->value[1],obj->value[0]);
	    send_to_char(buf,ch);
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    sprintf(buf,"in %s",argument);
	    do_function(ch, &do_look, buf );
	}
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;

    fAuto  = !str_cmp( argument, "auto" );

    if(!IS_NPC(ch) && IN_MINE(ch) )
    {	look_mine(ch);
	return;
    }
    if ( !check_blind( ch ) )
	return;

    if (fAuto)
	sprintf(buf,"{r[{DExits{r:{W");
   
    else if (IS_IMMORTAL(ch))
	sprintf(buf,"Obvious exits from room %d:\n\r",ch->in_room->vnum);
    else
	sprintf(buf,"Obvious exits:\n\r");

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] ) != NULL
	&&   (pexit->u1.to_room != NULL || pexit->to_shaft != NULL ) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
//                if (IS_SET (pexit->exit_info, EX_CLOSED))
//	        {
  //              strcat (buf, "{r({W");
                if (IS_MXP(ch))
                strcat (buf, MXP_SECURE "<Ex>" MXP_LLOCK );
		strcat (buf, dir_name[door]);
                if (IS_MXP(ch))
                strcat (buf, MXP_SECURE "</Ex>" MXP_LLOCK );  
//		strcat (buf, "{r){W");
//		}
//		else
//		{   
                     if(pexit->to_shaft)
                     {
		       strcat(buf, "{DMine{r-{W" );
			strcat (buf, dir_name[door]);
                     }
//		}
		    
	    }
	    else
	    {
		if (!IS_SET (pexit->exit_info, EX_CLOSED)) 
		{   
		sprintf( buf + strlen(buf), "%-5s - %s", 
		    capitalize( dir_name[door] ),
		    room_is_dark( pexit->u1.to_room )
			?  "Too dark to tell"
			: pexit->u1.to_room->name);
		}
		if (IS_IMMORTAL(ch))
		    sprintf(buf + strlen(buf), 
			" (room %d)\n\r",pexit->u1.to_room->vnum);
		else
		    sprintf(buf + strlen(buf), "\n\r");
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

    if ( fAuto )
	strcat( buf, "{r]{x\n\r" );

    send_to_char( buf, ch );
    return;
}

void do_pinfo( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
       send_to_char( "Sorry mobs don't have player info!\n\r", ch );
       return;
    }

    printf_to_char( ch, "\r\n{cO----------------------------------------------------------O{x\n\r" );

    send_to_char( "{c|                                                          {c|{x\n\r", ch );

    sprintf(buf, 
    "{c|{x Gold       : %-14ld Silver       : %-13ld {c|{x\n\r", ch->gold, ch->silver );
    send_to_char(buf,ch);

    printf_to_char( ch, "{c|{x Bank Gold  : %-14ld Bank Silver  : %-13ld {c|{x\n\r", ch->pcdata->gold_bank, ch->pcdata->silver_bank ); 

    printf_to_char( ch, "{c|{x Experience : %-14d Exp to level : %-13d {c|{x\n\r", ch->exp,
    (ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);  

    sprintf(buf, "{c|{x Questpoints: %-14d Trivia Points: %-13d {c|{x\n\r", ch->pcdata->questpoints, ch->pcdata->trivia);
    send_to_char(buf, ch);

    printf_to_char( ch, "{c|{x Levelflux  : %-14d Iquestpoints : %-13d {c|{x\n\r", ch->levelflux, ch->pcdata->qps );

    printf_to_char( ch, "{c|{x Questtime  : %-14d Nextquest    : %-13d {c|{x\n\r", ch->pcdata->nextquest );

    printf_to_char( ch, "{c|{x Pkill Wins : %-14d Pkill Losses : %-13d {c|{x\n\r", ch->pcdata->kills_pc, ch->pcdata->deaths_pc );   

    printf_to_char( ch, "{c|{x Mob Kills  : %-14d Mob Deaths   : %-13d {c|{x\n\r", ch->pcdata->kills_mob, ch->pcdata->deaths_mob ); 

    send_to_char( "{c|{x                                                          {c|{x\n\r", ch );

    if ( IS_SET( ch->act, PLR_FREQUENT ) )
        send_to_char( "{c|{x You are a member of the Frequent Questors guild.         {c|{x\n\r", ch );

    send_to_char( "{c|{x                                                          {c|{x\n\r", ch );

    printf_to_char( ch, "{cO----------------------------------------------------------O{x\n\r" );

    return;
}

void do_score( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	int i;
        int percent;

	if(IS_NPC(ch ) )
		return;

	printf_to_char(ch, "{D-------------------------------------------------------------------\n\r");
	printf_to_char(ch, "{DName{r:{W      %-15s   {DLevel{r:  {W%-12d   {DAge     {r:  {W%d\n\r", ch->name, ch->level, get_age(ch));
	printf_to_char(ch, "{DRace{r:{W      %-15s   {DSex  {r:  {W%-12s   {DSecurity{r:  {W%d\n\r", race_table[ch->race].name, ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female", ch->pcdata->security);
	printf_to_char(ch, "{DClan{r:{W      %-15s   {DRank {r:  {W%-12s   {DHours   {r:  {W%d\n\r", is_clan(ch) ? player_clan(ch) : "None", is_clan(ch) ? player_rank(ch) : "0", ( ch->played + (int) (current_time - ch->logon) ) / 3600);
	printf_to_char(ch, "{D-------------------------------------------------------------------\n\r");
    printf_to_char(ch, "{DClass{r:{W %-8s\n\r", class_long(ch));
	printf_to_char(ch, "{D-------------------------------------------------------------------\n\r");
	if (!IS_SET(ch->comm2, COMM_AUTOLEVEL))
		printf_to_char(ch, "{DYou have {W%ld{D exp saved right now. You need {W%d{D exp for one level.\n\r", ch->pcdata->current_exp, exp_per_level( ch, ch->pcdata->points));
	else
		printf_to_char(ch, "You have {W%d{D experience and need {W%d{D til next level!!!\n\r", ch->exp, ((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp) > 0 ? ((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp) : 0);
	printf_to_char(ch, "{D-------------------------------------------------------------------\n\r");
	printf_to_char(ch, "{DStr {r:{W %d {r[{W%d{r]  {DHitPt       {r:{W %-5ld {Dof {W%-5ld  {DMob Kills    {r: {W%-7d{x\n\r", ch->perm_stat[STAT_STR], get_curr_stat(ch,STAT_STR), ch->hit,  ch->max_hit, ch->pcdata->kills_mob);
	printf_to_char(ch, "{DInt {r:{W %d {r[{W%d{r]  {DMana        {r:{W %-5ld {Dof {W%-5ld  {DMob Deaths   {r: {W%-7d{x\n\r", ch->perm_stat[STAT_INT], get_curr_stat(ch,STAT_INT), ch->mana,  ch->max_mana, ch->pcdata->deaths_mob);
	printf_to_char(ch, "{DWis {r:{W %d {r[{W%d{r]  {DMoves       {r:{W %-5ld {Dof {W%-5ld  {DPlayer Kills {r: {W%-7d{x\n\r", ch->perm_stat[STAT_WIS], get_curr_stat(ch,STAT_WIS), ch->move,  ch->max_move, ch->pcdata->kills_pc);
	printf_to_char(ch, "{DDex {r:{W %d {r[{W%d{r]  {DPracs       {r:{W %-4d            {DPlayer Deaths{r: {W%-7d{x\n\r", ch->perm_stat[STAT_DEX], get_curr_stat(ch,STAT_DEX), ch->practice, ch->pcdata->deaths_pc);
	printf_to_char(ch, "{DCon {r:{W %d {r[{W%d{r]  {DTrain       {r:{W %-4d            {DQuest Points {r: {W%-7d{x\n\r", ch->perm_stat[STAT_CON], get_curr_stat(ch,STAT_CON), ch->train, ch->pcdata->questpoints);
    	printf_to_char(ch, "{DHitR{r:{W %3d      {DCast Level  {r: {W%-3d             {DTrivia Points{r: {W%-7d{x\n\r", GET_HITROLL(ch), ch->cast_level, ch->pcdata->trivia);
	printf_to_char(ch, "{DDamR{r:{W %3d      {DCast Ability{r: {W%-3d             {DTotal Trivia{r : {W%-7d{x\n\r", GET_DAMROLL(ch), ch->cast_ability, ch->pcdata->totaltpoints);
	printf_to_char(ch, "{D-------------------------------------------------------------------{x\n\r");
	if(HAS_RELIGION(ch) )
	{    printf_to_char(ch, "{DYou're a %s in %s", ch->pcdata->religion->rank[prime_class(ch)][ch->pcdata->rank], ch->pcdata->religion->name );
	     if(IN_FACTION(ch) )
	     	printf_to_char(ch, ", which is faction of %s", ch->pcdata->religion->main->name );
	     send_to_char(".\n\r{x", ch );
	     printf_to_char(ch, "{D-------------------------------------------------------------------{x\n\r");
	}
	printf_to_char(ch, "{DGold  {r: {W%-7ld {DBank Gold  {r: {W%-5ld {DItems {r: {W%-7d {Dof {W%-7d\n\r", ch->gold, ch->pcdata->gold_bank,ch->carry_number, can_carry_n(ch));
	printf_to_char(ch, "{DSilver{r: {W%-7ld {DBank Silver{r: {W%-5ld {DWeight{r: {W%-7ld {Dof {W%-7d.\n\r", ch->silver, ch->pcdata->silver_bank, get_carry_weight(ch) / 10, can_carry_w(ch) /10);
	printf_to_char(ch, "{D-------------------------------------------------------------------{x\n\r");
	printf_to_char(ch, "{DWimpy set to {W%d{D hit points.{x\n\r", ch->wimpy );
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
		send_to_char( "{DYou are {Wdrunk{x.\n\r",   ch );
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
		send_to_char( "{DYou are {Wthirsty{x.\n\r", ch );
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER]   ==  0 )
		send_to_char( "{DYou are {Whungry{x.\n\r",  ch );
        if ( ch->pcdata->condition[COND_TIRED] > 0 )
	percent = ( 100 * ch->pcdata->condition[COND_TIRED] / 48 );
    else
	percent = -1;

    if ( percent >= 90 )
	send_to_char( "{DYou are well {Wrested{x.\n\r", ch );
    else if ( percent >= 75 )
	send_to_char( "{DYou feel {Wrested{x.\n\r", ch );
    else if ( percent >= 50 )
	send_to_char( "{DYou are a bit {Wtired{x.\n\r", ch );
    else if ( percent >= 25 )
	send_to_char( "{DYou feel {Wfatigued{x.\n\r", ch );
    else if ( percent >= 7 )
	send_to_char( "{DYou are {Wtired{x.\n\r", ch );
    else
	send_to_char( "{DYou are {Wasleep {Don your feet{x!\n\r", ch );

	switch ( ch->position )
	{	case POS_DEAD:
			send_to_char( "{DYou are {rD{RE{rA{RD{D!!{x\n\r",         ch );
			break;
		case POS_MORTAL:
			send_to_char( "{DYou are {Wmortally wounded{x\n\r",      ch );
			break;
		case POS_INCAP:
			send_to_char( "{DYou are {Wincapacitated{x\n\r", ch );
			break;
		case POS_STUNNED:
			send_to_char( "{DYou are {Wstunned{x\n\r",               ch );
			break;
		case POS_SLEEPING:
			send_to_char( "{DYou are {Wsleeping{x\n\r",              ch );
			break;
		case POS_RESTING:
			send_to_char( "{DYou are {Wresting{x\n\r",               ch );
			break;
		case POS_SITTING:
			send_to_char( "{DYou are {Wsitting{x\n\r",               ch );
			break;
        case POS_STANDING:
  			if(MOUNTED(ch))
			{	sprintf( buf, "{DYou are {Wriding on %s{x\n\r", MOUNTED(ch)->short_descr );
  				send_to_char( buf, ch);
			}
			else
				send_to_char( "{DYou are {Wstanding{x.\n\r",          ch );
	        break;

		case POS_FIGHTING:
			send_to_char( "{DYou are {Wfighting{x.\n\r",          ch );
			break;
	}
          
        if ( IS_MXP( ch ) )
        {
            printf_to_char(ch,"{DYou are in a " );
            SNP( buf, MXP_SECURE "<send href=\"stance aggressive|stance neutral|stance defensive\" hint=\"click to see menu|aggressive|neutral|defensive\">{W%s{x</Send>{x " MXP_LLOCK, stance_table[ch->stance].name );
            send_to_char( buf, ch );
            printf_to_char(ch,"{Dstance{x\n\r" );
        }
        else
	printf_to_char(ch,"{DYou are in a {W%s{D stance{x\n\r", stance_table[ch->stance].name);

	if(RIDDEN(ch))
		printf_to_char(ch, "{DYou are ridden by {W%s{x\n\r", IS_NPC(RIDDEN(ch)) ? RIDDEN(ch)->short_descr : RIDDEN(ch)->name);

	if (ch->level >= 25)
		printf_to_char(ch, "{DArmor{r:\n\r");
   	for (i = 0; i < 4; i++)
	{	char * temp;
	
		switch(i)
		{	case(AC_PIERCE):    temp = "piercing";      break;
			case(AC_BASH):      temp = "bashing";       break;
			case(AC_SLASH):     temp = "slashing";      break;
			case(AC_EXOTIC):    temp = "magic";         break;
			default:            temp = "error";         break;
       }

		if   (GET_AC(ch,i) >=  101 )
			printf_to_char(ch,"\t{D%-8s{r:   [{W%d{r] -  {Dhopelessly vulnerable{x\n\r",temp, GET_AC(ch,i) );
		else if (GET_AC(ch,i) >= 70)
			printf_to_char(ch,"{D\t%-8s{r:   [{W%d{r] -  {Ddefenseless{x\n\r",temp, GET_AC(ch,i));
		else if (GET_AC(ch,i) >= 30)
			printf_to_char(ch,"\t{D%-8s{r:   [{W%d{r] -  {Dbarely protected{x\n\r",temp, GET_AC(ch,i));
		else if (GET_AC(ch,i) >= 10)
			printf_to_char(ch,"\t{D%-8s{r:   [{W%d{r] -  {Dslightly armored{x\n\r",temp, GET_AC(ch,i));
		else if (GET_AC(ch,i) >= -10)
			printf_to_char(ch,"\t{D%-8s{r:   [{W%d{r] -  {Dsomewhat armored{x\n\r",temp, GET_AC(ch,i));
		else if (GET_AC(ch,i) >= -30)
			printf_to_char(ch,"\t{D%-8s{r:   [{W%d{r] -  {Darmored{x\n\r",temp, GET_AC(ch,i));
		else if (GET_AC(ch,i) >= -50)
			printf_to_char(ch,"\t{D%-8s{r:   [{W%d{r] -  {Dwell-armored{x\n\r",temp, GET_AC(ch,i));
		else if (GET_AC(ch,i) >= -200)
			printf_to_char(ch,"\t{D%-8s{r:   [{W%d{r] -  {Dvery well-armored{x\n\r",temp, GET_AC(ch,i));
		else if (GET_AC(ch,i) >= -300)
			printf_to_char(ch,"\t{D%-8s{r:   [{W%d{r] -  {Dheavily armored{x\n\r",temp, GET_AC(ch,i));
		else if (GET_AC(ch,i) >= -400)
			printf_to_char(ch,"\t{D%-8s{r:   [{W%d{r] -  {Dsuperbly armored{x\n\r",temp, GET_AC(ch,i));
		else if (GET_AC(ch,i) >= -500)
			printf_to_char(ch,"\t{D%-8s{r:   [{W%d{r] -  {Dalmost invulnerable{x\n\r",temp, GET_AC(ch,i));
		else
			printf_to_char(ch,"\t{D%-8s{r:   [{W%d{r] -  {Ddivinely armored{x\n\r",temp, GET_AC(ch,i));

	}	

	printf_to_char(ch, "{D-------------------------------------------------------------------{x\n\r");
    if ( IS_IMMORTAL(ch))
    {	send_to_char("{DHoly Light{r:{W ",ch);
	
       	if (IS_SET(ch->act,PLR_HOLYLIGHT))
			send_to_char("on{x",ch);
		else
			send_to_char("off{x",ch);
        
		if (ch->invis_level)
  			printf_to_char(ch, "{D  Invisible{r: {W%d{x",ch->invis_level);
     	if (ch->incog_level)
  			printf_to_char(ch,"{D  Incognito{r: {W%d{x",ch->incog_level);
        
		send_to_char("\n\r",ch);
	}

		printf_to_char(ch, "{DAlignment{r: [{W%d{r] -{D  ", ch->alignment );
    
	if ( ch->alignment >  900 ) send_to_char( "angelic{x.\n\r", ch );
	else if ( ch->alignment >  700 ) send_to_char( "saintly{x.\n\r", ch );
	else if ( ch->alignment >  350 ) send_to_char( "good{x.\n\r",    ch );
	else if ( ch->alignment >  100 ) send_to_char( "kind{x.\n\r",    ch );
	else if ( ch->alignment > -100 ) send_to_char( "neutral{x.\n\r", ch );
	else if ( ch->alignment > -350 ) send_to_char( "mean{x.\n\r",    ch );
	else if ( ch->alignment > -700 ) send_to_char( "evil{x.\n\r",    ch );
	else if ( ch->alignment > -900 ) send_to_char( "demonic{x.\n\r", ch );
	else                             send_to_char( "satanic{x.\n\r", ch );

	send_to_char( "{DMXP Enabled: [", ch );
        if(IS_MXP(ch))
        {
                send_to_char( "{RX{D]{x", ch );
        }
        else
        {
                send_to_char( " {D]{x", ch );
        }

        send_to_char( "  {DMSP Enabled: [", ch );
        if (IS_MSP( ch ))
            send_to_char( "{RX{D]{x", ch );
        else 
            send_to_char( " {D]{x", ch );
        send_to_char( " {DMCCP Enabled: {D[{x", ch );
        if ( ch->desc->out_compress ) 
            send_to_char( "{RX{D]{x\n\r\n\r", ch );
        else
            send_to_char( " {D]{x\n\r\n\r", ch );

	if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
		do_function(ch, &do_affects, "");
}
   
void do_affects(CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    char buf[MAX_STRING_LENGTH];
    
    if ( ch->affected != NULL )
    {	printf_to_char(ch, "{r|{D   %-20s {r|{D   %-20s {r|{D   %-5s {r|{D %-5s {r|\n\r", "Spell", "Modifier", "Value", "Duration" );
		send_to_char("{r========================================================================\n\r",ch);

		for ( paf = ch->affected; paf != NULL; paf = paf->next )
		{	if ( paf->duration == -1 )
				sprintf( buf, "{r|{W   %-20s {r|{W   %-20s {r|{W   %-6d {r|{W    %-5s {r|\n\r", skill_table[paf->type].name, affect_loc_name( paf->location ),paf->modifier, "Perminent" );    
			else
				sprintf( buf, "{r|{W   %-20s {r|{W   %-20s {r|{W   %-5d {r|{W    %-5d {r|\n\r{x", skill_table[paf->type].name, affect_loc_name( paf->location ), paf->modifier, paf->duration );    
    	
		    send_to_char( buf, ch );
		}
    }
    else 
	send_to_char("You are not affected by any spells.\n\r",ch);

    return;
}



char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    char crs_tme[MSL], str_tme[MSL], sys_tme[MSL], sst[MSL], ssd[MSL], ssm[MSL];
    char *suf;
    int day;
    struct tm *time_str;
    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";
    
    if(last_crash_time > (int) current_time || last_crash_time < first_boot_time )
        sprintf(crs_tme, "ShadowStorm hasn't crashed since Original Boot" );
    else
    {   
        time_str = localtime( (time_t *) &last_crash_time  );
	strftime (crs_tme, 256, "%A, %B %d 20%y at %l:%M %p", time_str);
    }
	time_str = localtime( (time_t *) &first_boot_time  );
	strftime (str_tme, 256, "%A, %B %d 20%y at %l:%M %p", time_str);
	time_str = localtime( &current_time );
   	strftime (sys_tme, 256, "%A, %B %d 20%y, %l:%M %p", time_str);
	sprintf(sst, "%d o'clock %s", (time_info.hour % 12 == 0) ? 12 : time_info.hour %12, time_info.hour >= 12 ? "pm" : "am" );
	sprintf(ssd, "The Day of %s, %d%s",  day_name[day % 7], day, suf );
	sprintf(ssm, "The Month of %s",  month_name[time_info.month] );
	printf_to_char(ch, "{DShadowStorm's Time{r:            {W%s\n\r", sst );
	printf_to_char(ch, "{DShadowStorm's Day{r:             {W%s\n\r", ssd );
	printf_to_char(ch, "{DShadowStorm's Month{r:          {W %s\n\r", ssm );
	printf_to_char(ch, "{DShadowStorm's Last Crash{r:     {W %s\n\r", crs_tme );
	printf_to_char(ch, "{DShadowStorm's Original Boot{r:{W   %s\n\r", str_tme );
	printf_to_char(ch, "{DSystem time{r:{W                   %s{x\n\r", sys_tme );
    return;
}

void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    BUFFER *output;
    bool found = FALSE;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
    int level;

    output = new_buf();

    if ( argument[0] == '\0' )
	argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
	    strcat(argall," ");
	strcat(argall,argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
    	level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

	if (level > get_trust( ch ) )
	    continue;

	if ( is_name( argall, pHelp->keyword ) )
	{
	    /* add seperator if found */
	    if (found)
		add_buf(output,
    "\n\r============================================================\n\r\n\r");
	    if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
	    {
		add_buf(output,pHelp->keyword);
		add_buf(output,"\n\r");
	    }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		add_buf(output,pHelp->text+1);
	    else
		add_buf(output,pHelp->text);
	    found = TRUE;
	    /* small hack :) */
	    if (ch->desc != NULL && ch->desc->connected != CON_PLAYING 
	    &&  		    ch->desc->connected != CON_GEN_GROUPS)
		break;
	}
    }

    if(!found)
    {
       send_to_char("No help on that word.\n\r", ch);
       sound( "nohelp.wav", ch );
    }   
    else
	page_to_char(buf_string(output),ch);
    free_buf(output);
}


/* whois command */
void do_whois (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    one_argument(argument,arg);
  
    if (arg[0] == '\0')
    {
	send_to_char("You must provide a name.\n\r",ch);
	return;
    }

    output = new_buf();

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;
	char const *klass;

 	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;
	
	wch = ( d->original != NULL ) ? d->original : d->character;

 	if (!can_see(ch,wch))
	    continue;

	if (!str_prefix(arg,wch->name))
	{
	    found = TRUE;
	    
	    /* work out the printing */
	    klass = class_who(wch);
	    switch(wch->level)
	    {
		case MAX_LEVEL - 0 : klass = "IMP"; 	break;
		case MAX_LEVEL - 1 : klass = "CRE";	break;
		case MAX_LEVEL - 2 : klass = "SUP";	break;
		case MAX_LEVEL - 3 : klass = "DEI";	break;
		case MAX_LEVEL - 4 : klass = "GOD";	break;
		case MAX_LEVEL - 5 : klass = "IMM";	break;
		case MAX_LEVEL - 6 : klass = "DEM";	break;
		case MAX_LEVEL - 7 : klass = "ANG";	break;
		case MAX_LEVEL - 8 : klass = "AVA";	break;
	    }
    
	    /* a little formatting */
	    sprintf(buf, "\n\r{C[{B%3d {M%6s {R%s{C]{x %s%s%s%s%s%s%s%s%s%s%s%s\n\r",
		wch->level,
		wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name
					: "     ",
		klass,
	     wch->incog_level >= LEVEL_HERO ? "{W({cIncog{W){X ": "",
 	     wch->invis_level >= LEVEL_HERO ? "{W({MWizi{W){X " : "",
             clan_table[wch->clan].who_name, 
	     IS_SET(wch->comm, COMM_AFK) ? "{W[{YAFK{W]{X " : "",
             IS_SET(wch->comm2,COMM_AUTO_AFK) ? "{W[{YAuto AFK{W]{X " : "", 
             IS_SET(wch->act, PLR_WAR) ? "{W({RWAR{W){X " : "",
             IS_SET(wch->act,PLR_KILLER) ? "{R<KILLER>{X " : "",
             IS_SET(wch->act,PLR_THIEF) ? "{R<{DTHIEF{>R{X " : "",
             IS_SET(wch->act,PLR_TIMEOUT) ? "{C({cTIMEOUT{C){x " : "",
	     !IS_NPC(wch) ? (wch->pcdata->pretitle[0] == '\0' ? "" : wch->pcdata->pretitle ) : "",
             wch->name, IS_NPC(wch) ? "" : wch->pcdata->title);
 	    add_buf(output,buf);
            
            sprintf(buf,"\n\r{cO-----------------------------------------------------O{x\n\r" );
            add_buf( output, buf );  
	    sprintf(buf,"{c|                                                     |{x\n\r" );
            add_buf( output, buf );
            sprintf(buf,"{c| {CPkill Wins :{R %-10d {CPkill Losses :{R %-12d {c|{x\n\r",
            wch->pcdata->kills_pc, wch->pcdata->deaths_pc );
            add_buf( output, buf );
            sprintf(buf,"{c| {CMob Kills  :{B %-10d {CMob Deaths   :{B %-12d {c|{x\n\r",
            wch->pcdata->kills_mob, wch->pcdata->deaths_mob );
            add_buf( output, buf );
            sprintf(buf,"{c| {CQuestpoints:{G %-10d {CTrivia Points:{G %-12d {c|{x\n\r", 
            wch->pcdata->questpoints, wch->pcdata->trivia );
            add_buf( output, buf );
            sprintf(buf,"{c| {CCast Level :{G %-10d {CCast Ability :{G %-12d {c|{x\n\r", wch->cast_level, wch->cast_ability );
            add_buf( output, buf );
            sprintf(buf,"{c| {CClan       :{G %-10s {CRank         :{G %-12s {c|{x\n\r", player_clan( wch ), player_rank( wch ) );
            add_buf( output, buf );
            sprintf(buf,"{c| {CHdam       :{G %-10d {CMdam         :{G %-12d {c|{x\n\r", wch->pcdata->hdam, wch->pcdata->mdam );
            add_buf(output, buf );
            sprintf(buf,"{c|                                                     |{x\n\r" );
            add_buf( output, buf );
            sprintf(buf,"{cO-----------------------------------------------------O{x\n\r" );
            add_buf( output, buf );
	    
	}
    }

    if (!found)
    {
	send_to_char("No one of that name is playing.\n\r",ch);
	return;
    }

    page_to_char(buf_string(output),ch);
    free_buf(output);
}


/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MSL];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    FILE *fp = NULL;
    int iClass;
    int iRace;
    int iClan;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    int count = 0;
    int max = 0;
    int half;
    int sechalf;
    int level;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool rgfClan[MAX_CLAN];
    bool fClassRestrict = FALSE;
    bool fClanRestrict = FALSE;
    bool fClan = FALSE;
    bool fRaceRestrict = FALSE;
    bool fImmortalOnly = FALSE;
    char olc[MSL]; 

    
	if(IS_NPC(ch) )
	    return;
	/*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
        rgfRace[iRace] = FALSE;
    for (iClan = 0; iClan < MAX_CLAN; iClan++)
	rgfClan[iClan] = FALSE;
 
    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
        char arg[MAX_STRING_LENGTH];
 
        argument = one_argument( argument, arg );
        if ( arg[0] == '\0' )
            break;
 
        if ( is_number( arg ) )
        {
            switch ( ++nNumber )
            {
            case 1: iLevelLower = atoi( arg ); break;
            case 2: iLevelUpper = atoi( arg ); break;
            default:
                send_to_char( "Only two level numbers allowed.\n\r", ch );
                return;
            }
        }
        else
        {
 
            /*
             * Look for classes to turn on.
             */
            if (!str_prefix(arg,"immortals"))
            {
                fImmortalOnly = TRUE;
            }
            else
            {
                iClass = class_lookup(arg);
                if (iClass == -1)
                {
                    iRace = race_lookup(arg);
 
                    if (iRace == 0 || iRace >= MAX_PC_RACE)
		    {
			if (!str_prefix(arg,"clan"))
			    fClan = TRUE;
			else
		        {
			    iClan = clan_lookup(arg);
			    if (iClan)
			    {
				fClanRestrict = TRUE;
			   	rgfClan[iClan] = TRUE;
			    }
			    else
			    {
                        	send_to_char(
                            	"That's not a valid race, class, or clan.\n\r",
				   ch);
                            	return;
			    }
                        }
		    }
                    else
                    {
                        fRaceRestrict = TRUE;
                        rgfRace[iRace] = TRUE;
                    }
                }
                else
                {
                    fClassRestrict = TRUE;
                    rgfClass[iClass] = TRUE;
                }
            }
        }
    }
 
    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output = new_buf();
  for( level = MAX_LEVEL ; level > 0 ; level--)
  {
    if(level == LEVEL_HERO && mortal_online() > 0)
	add_buf(output,"\n\r{CMortals Online:{X\n\r");
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
        char const *klass;
 
        /*
         * Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.
         */
        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
 
        wch   =  d->character ? d->character : d->original;

	if (!can_see(ch,wch))
	    continue;
	if(wch->level != level)
	    continue;
	
	if(IS_NPC(wch) ) continue;
        count++;

        if ( wch->level < iLevelLower
        ||   wch->level > iLevelUpper
        || ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
        || ( fClassRestrict && !rgfClass[wch->klass[0]] )
        || ( fRaceRestrict && !rgfRace[wch->race])
 	|| ( fClan && !is_clan(wch))
	|| ( fClanRestrict && !rgfClan[wch->clan]))
            continue;

        nMatch++;
 
        /*
         * Figure out what to print for class.
	 */
	klass = class_who(wch);
	switch ( wch->level )
	{
	default: break;
            {
                case MAX_LEVEL - 0 : klass = "{CIMP{X";     break;
                case MAX_LEVEL - 1 : klass = "{CCRE{X";     break;
                case MAX_LEVEL - 2 : klass = "{CSUP{X";     break;
                case MAX_LEVEL - 3 : klass = "{CDEI{X";     break;
                case MAX_LEVEL - 4 : klass = "{CGOD{X";     break;
                case MAX_LEVEL - 5 : klass = "{CIMM{X";     break;
                case MAX_LEVEL - 6 : klass = "{CDEM{X";     break;
                case MAX_LEVEL - 7 : klass = "{CANG{X";     break;
                case MAX_LEVEL - 8 : klass = "{CAVA{X";     break;
            }
	}

    	switch( wch->desc->editor )
    	{
        	case ED_AREA:
        	case ED_ROOM:
        	case ED_OBJECT:
	        case ED_MOBILE:
                case ED_HELP:
                case ED_SKILL:
			sprintf(olc, "{R({WOLC{R){x ");
                	break;
        	default:
			sprintf(olc, "{x");
 			break;
    	}

       if (wch->level >= LEVEL_IMMORTAL && wch->pcdata->immtitle != NULL)
       {
          if (strlen_color(wch->pcdata->immtitle) == 14)
           sprintf(buf3, "{r[{W%s{r]{x", wch->pcdata->immtitle);
          else if (strlen_color(wch->pcdata->immtitle) == 15)
           sprintf(buf3, "{r[{W%s {r]{x", wch->pcdata->immtitle);       
          else
          {
            half = ((14 - strlen_color(wch->pcdata->immtitle)) / 2);
            sechalf = (14 - (half + strlen_color(wch->pcdata->immtitle)));
            sprintf(buf3, "{r[{W%*c%s%*c{r]{x", half, ' ', wch->pcdata->immtitle, sechalf,
              ' ');
          }
       }
       else
          sprintf( buf3, "{r[{W%3d {w%6s {D%s{r]{x",
             wch->level,
             wch->race < MAX_PC_RACE ?
             pc_race_table[wch->race].who_name : "        ", klass);    

	/*
	 * Format it up.
	 */
	sprintf( buf, "%s %s%s%s%s%s%s%s%s%s%s%s%s\n\r", buf3,
	    olc,
	    wch->incog_level >= LEVEL_HERO ? "{W({cIncog{W){X ": "",
 	     wch->invis_level >= LEVEL_HERO ? "{W({MWizi{W){X " : "",
             clan_table[wch->clan].who_name, 
	     IS_SET(wch->comm, COMM_AFK) ? "{W[{YAFK{W]{X " : "",
             IS_SET(wch->comm2,COMM_AUTO_AFK) ? "{W[{YAuto AFK{W]{X " : "", 
             IS_SET(wch->act, PLR_WAR) ? "{W({RWAR{W){X " : "",
             IS_SET(wch->act,PLR_TIMEOUT) ? "{C({cTIMEOUT{C){x " : "",
            ON_GQUEST(wch) ? "{W({GGQuest{W){X" : "", 
	     wch->pcdata->pretitle[0] == '\0' ? "" : wch->pcdata->pretitle,
	    wch->name,
	    IS_NPC(wch) ? "" : wch->pcdata->title );
	add_buf(output,buf);
    }
  }
    max_on = UMAX(count,max_on);       

    fp = file_open(MAX_WHO_FILE, "r");
    max = fread_number( fp );
    file_close(fp);

    if ( max_on > max )
    {
	fp = file_open(MAX_WHO_FILE, "w");
        fprintf( fp, "%d\n", max_on );
	file_close(fp);
    }

send_to_char("\r\n{D                ______          __           ______\n\r",ch);
send_to_char("{D   --=<{r((((((({D / __/ / ___ ____/ /__ _    __/ __/ /____  ______ _ {r))))){D>=--\n\r",ch);
send_to_char("{D--=<{r((((((((({D _\\ \\/ _ Y _ `/ _  / _ \\ |/|/ /\\ \\/ __/ _ \\/ __/  ' \\ {r))))))){D>=--\n\r",ch);
send_to_char("{D   --=<{r((((({D /___/_//_|_,_/\\_,_/\\___/__,__/___/\\__/\\___/_/ /_/_/_/ {r)))){D>=--{x\n\r\n\r{x",ch);
if(immortal_online > 0 )
send_to_char("{DImmortals Online{r: {X\n\r",ch);

    if ( double_exp )
    {
     sprintf( buf2,"\n\r{DDouble exp is on for {W%d {Dticks!{x", global_exp );
     add_buf(output,buf2);
    }  
    sprintf( buf2, "\n\r{DPlayers found{r: {W%d      {DMost on today{r    : {W%d{D\n\rMax on ever  {r: {W%d     {DTotal connections{r:{W %d{x\n\r", nMatch,
max_on, max, count_connections );
    add_buf(output,buf2);
    if (!ON_GQUEST(ch) && gquest_info.minlevel < ch->level &&
	gquest_info.maxlevel > ch->level)
	add_buf(output, "{DThere is a Global Quest running you can join.{x\n\r");
    page_to_char( buf_string(output), ch );
    free_buf(output);
    return;
}

void do_count ( CHAR_DATA *ch, char *argument )
{
    int count;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
	    count++;

    max_on = UMAX(count,max_on);

    if (max_on == count)
        sprintf(buf,"There are %d characters on, the most so far today.\n\r",
	    count);
    else
	sprintf(buf,"There are %d characters on, the most on today was %d.\n\r",
	    count,max_on);

    send_to_char(buf,ch);
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj, *temp, *sheathed;
    int iWear;
    bool found, beltfound = FALSE;

    send_to_char( "You are using:\n\r", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
		if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
		{
			show_strapped(ch,ch,iWear);
			continue;
		}
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	    continue;

	send_to_char( where_name[iWear], ch );
	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
	else
	{
	    send_to_char( "something.\n\r", ch );
	}
	found = TRUE;
	show_strapped(ch,ch,iWear);

    }
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
		if ( ( obj = get_eq_char( ch, iWear ) ) == NULL || obj->item_type != ITEM_BELT )
			continue;

		if ( found )
			send_to_char( "\n\r", ch );
		found = TRUE;
		printf_to_char( ch, "{r:{Dstrapped to %s{r:{W{x\n\r", obj->short_descr );
		for ( temp = ch->carrying; temp; temp = temp->next_content )
		{
			if (temp->belted_vnum == obj->pIndexData->vnum)
			{
				printf_to_char( ch, "   %s\n\r", temp->short_descr );
				beltfound = TRUE;
					if(temp->item_type == ITEM_SHEATH)
					{
					  for(sheathed =temp->contains; sheathed; sheathed = sheathed->next_content)
						    printf_to_char( ch, "        %s\n\r",sheathed->short_descr);
					}
			}
		}
		if ( !beltfound )
			send_to_char( "   Nothing.\n\r", ch );
    }

    if ( !found )
	send_to_char( "Nothing.\n\r", ch );

    return;
}



void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != WEAR_NONE
	    &&  can_see_obj(ch,obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == NULL)
	{
	    send_to_char("You aren't wearing anything comparable.\n\r",ch);
	    return;
	}
    } 

    else if ( (obj2 = get_obj_carry(ch,arg2,ch) ) == NULL )
    {
	send_to_char("You do not have that item.\n\r",ch);
	return;
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    if (obj1->pIndexData->new_format)
		value1 = (1 + obj1->value[2]) * obj1->value[1];
	    else
	    	value1 = obj1->value[1] + obj1->value[2];

	    if (obj2->pIndexData->new_format)
		value2 = (1 + obj2->value[2]) * obj2->value[1];
	    else
	    	value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}



void do_credits( CHAR_DATA *ch, char *argument )
{
    do_function(ch, &do_help, "diku" );
    return;
}



void do_where( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Players near you:\n\r", ch );
	found = FALSE;
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room != NULL
	    &&   !IS_SET(victim->in_room->room_flags,ROOM_NOWHERE)
 	    &&   (is_room_owner(ch,victim->in_room) 
	    ||    !room_is_private(victim->in_room))
	    &&   victim->in_room->area == ch->in_room->area
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    victim->name, victim->in_room->name );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "None\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !IS_AFFECTED(victim, AFF_HIDE)
	    &&   !IS_AFFECTED(victim, AFF_SNEAK)
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    PERS(victim, ch), victim->in_room->name );
		send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
	    act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}




void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
    {
	send_to_char("Don't even think about it.\n\r",ch);
	return;
    }

    diff = victim->level - ch->level;

         if ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
    else if ( diff <=  -5 ) msg = "$N is no match for you.";
    else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
    else if ( diff <=   1 ) msg = "The perfect match!";
    else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
    else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
    else                    msg = "Death will thank you for your gift.";

    act( msg, ch, NULL, victim, TO_CHAR );
     
    if(ch->alignment > 800) /* Very good characters can tell a lot */
    {
       if (victim->alignment >= 900)
		 msg = "$N represents all that is Holy and Good.";
       else if (victim->alignment >= 700)
		 msg = "$N is an ally, $E's on your side.";
       else if (victim->alignment >= 250)
		 msg = "$N has Good intentions, at least.";
       else if (victim->alignment > -250)
		 msg = "$N is neutral.";
       else if (victim->alignment > -900)
		 msg = "$N is an enemy, $E's rotten.";
       else  msg = "$N is black-hearted! $E is Absolutely Evil!";
   }

   else if(ch->alignment < -800) /* so can very evil characters */
   {
       if (victim->alignment >= 900)
		 msg = "$N absolutely stinks of Righteousness.";
       else if (victim->alignment >= 700)
		 msg = "$N is a goody goody.";
       else if (victim->alignment >= 250)
		 msg = "$N likes to think $E's Good.";
       else if (victim->alignment > -250)
		 msg = "$N is just a neutral coward.";
       else if (victim->alignment > -900)
		 msg = "$N is merely mean and selfish.";
       else  msg = "You and $N are two of a kind, $E is truly Evil.";
   }

/* Very Neutral characters can get a lot of information too. */

   else if( (ch->alignment < 100) && (ch->alignment > -101) ) 
   {
       if (victim->alignment >= 700)
		 msg = "$N is committed to the side of Good.";
       else if (victim->alignment >= 250)
		 msg = "$N is trying to be Good.";
       else if (victim->alignment >= -250)
		 msg = "$N is neither Good nor Evil.";
       else if (victim->alignment > -700)
		 msg = "$N is trying to be Evil.";
       else  msg = "$N is committed to the side of Evil.";
   }

/* Neither very good nor very evil nor very neutral */
   else
   {
        if (victim->alignment > 900)
		  msg = "$N seems Good.";
        else if (victim->alignment > -900)
		  msg = "You cannot perceive $N's alignment.";
        else  msg = "$N seems Evil.";
    }

    act( msg, ch, NULL, victim, TO_CHAR );
   
    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }

    if ( strlen(argument) > 45 )
	argument[45] = '\0';

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}



void do_description( CHAR_DATA *ch, char *argument )
{
   string_append(ch, &ch->description);
    return;
}



void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
	"You say 'I have %ld/%ld hp %ld/%ld mana %ld/%ld mv %d xp.'\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    send_to_char( buf, ch );

    sprintf( buf, "$n says 'I have %ld/%ld hp %ld/%ld mana %ld/%ld mv %d xp.'",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}

void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH], buf2[MSL], buf3[MSL];
	BUFFER *buffer;
	int sn;

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	int col;
	buffer = new_buf();
	col    = 0;

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    if ( !can_use_skpell(ch, sn)
	      || ch->pcdata->learned[sn] < 1 /* skill is not known */)
		continue;
            sprintf(buf2,"{D%s{r:{W",capitalize(skill_table[sn].name ));

		     if(ch->pcdata->learned[sn] == 1   ) sprintf(buf3, "{w%3d{D%%", ch->pcdata->learned[sn]);
		else if(ch->pcdata->learned[sn] <= 75  ) sprintf(buf3, "{W%3d{D%%", ch->pcdata->learned[sn]);
		   else                                  sprintf(buf3, "{c%3d{D%%", ch->pcdata->learned[sn]);
	    sprintf( buf, "%-25s %s   ",
		buf2,buf3);
		add_buf(buffer, buf);
	    if ( ++col % 3 == 0 )
		add_buf(buffer, "\n\r");
	}
   
	if ( col % 3 != 0 )
	    add_buf( buffer,"\n\r" );

	sprintf( buf, "You have %d practice sessions left.\n\r",
	    ch->practice );
	add_buf( buffer, buf );
	page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
    }
    else
    {
	CHAR_DATA *mob;
	int adept;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
	    send_to_char( "You have no practice sessions left.\n\r", ch );
	    return;
	}

	if ( ( sn = find_spell( ch,argument ) ) < 0
	|| ( !IS_NPC(ch)
	&&   (!can_use_skpell(ch, sn) 
 	||    ch->pcdata->learned[sn] < 1 /* skill is not known */
	||    skill_rating(ch, sn) == 0)))
	{
	    send_to_char( "You can't practice that.\n\r", ch );
	    return;
	}

	adept = IS_NPC(ch) ? 100 : class_table[ch->klass[0]].skill_adept;

	if ( ch->pcdata->learned[sn] >= adept )
	{
	    sprintf( buf, "You are already learned at %s.\n\r",
		skill_table[sn].name );
	    send_to_char( buf, ch );
	}
	else
	{
	    ch->practice--;
	    ch->pcdata->learned[sn] += 
		int_app[get_curr_stat(ch,STAT_INT)].learn /  skill_rating(ch, sn);
	    if ( ch->pcdata->learned[sn] < adept )
	    {
		act( "You practice $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n practices $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	    else
	    {
		ch->pcdata->learned[sn] = adept;
		act( "You are now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n is now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	}
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
	return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
	send_to_char( "Such cowardice ill becomes you.\n\r", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "Wimpy set to %d hit points.\n\r", wimpy );
    send_to_char( buf, ch );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_istat(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    sprintf(buf, "Retrieving race information for %s%ss.\n\r",
        IS_IMMORTAL(ch) ? "immortal " : "", race_table[ch->race].name);
    send_to_char(buf, ch);
    sprintf(buf, "Str[%2d/%2d] Max[%2d] %s\n\r",
        ch->perm_stat[STAT_STR],
        get_curr_stat(ch, STAT_STR),
        get_max_train(ch, STAT_STR),
        ch->perm_stat[STAT_STR] == get_max_train(ch, STAT_STR) ? "Maxed" : "");
    send_to_char(buf, ch);
    sprintf(buf, "Int[%2d/%2d] Max[%2d] %s\n\r",
            ch->perm_stat[STAT_INT],
            get_curr_stat(ch, STAT_INT),
            get_max_train(ch, STAT_INT),
        ch->perm_stat[STAT_INT] == get_max_train(ch, STAT_INT) ? "Maxed" : "");
    send_to_char(buf, ch);
    sprintf(buf, "Wis[%2d/%2d] Max[%2d] %s\n\r",
            ch->perm_stat[STAT_WIS],
            get_curr_stat(ch, STAT_WIS),
        get_max_train(ch, STAT_WIS),
        ch->perm_stat[STAT_WIS] == get_max_train(ch, STAT_WIS) ? "Maxed" : "");
    send_to_char(buf, ch);
    sprintf(buf, "Dex[%2d/%2d] Max[%2d] %s\n\r",
            ch->perm_stat[STAT_DEX],
            get_curr_stat(ch, STAT_DEX),
        get_max_train(ch, STAT_DEX),
        ch->perm_stat[STAT_DEX] == get_max_train(ch, STAT_DEX) ? "Maxed" : "");
    send_to_char(buf, ch);
    sprintf(buf, "Con[%2d/%2d] Max[%2d] %s\n\r",
            ch->perm_stat[STAT_CON],
            get_curr_stat(ch, STAT_CON),
        get_max_train(ch, STAT_CON),
        ch->perm_stat[STAT_CON] == get_max_train(ch, STAT_CON) ? "Maxed" : "");
    send_to_char(buf, ch);
    return;
}


void do_become( CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *oldch;
  char * name;
  char * person = strdup(argument);
  char * password = strdup(argument);
  bool fOld;
  extern bool wizlock;
  extern bool newlock;

  password = one_argument(password,person);

  if ( password[0] == '\0' )
  {
      send_to_char( "Syntax: <char> <password>\n\r", ch );
      return;
  }

    if (IS_NPC(ch))
        return;
	
    d = ch->desc;
    name = strdup(ch->name);
    person[0] = UPPER(person[0]);
    oldch = d->character;
    fOld = load_char_obj( d, person );
    ch = d->character;
    
    if ( IS_SET(ch->act, PLR_DENY) )
    {
        sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
        log_string( log_buf );
        write_to_buffer( d, "You are denied access.\n\r", 0 );
  	d->character = oldch;
        return;
    }

    if ( wizlock && !IS_HERO(ch))
    {
	write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
	d->character = oldch;
	return;
    }

    if ( newlock )
    {
        write_to_buffer( d, "The game is newlocked.\n\r", 0 );
        d->character = oldch;
        return;
    } 

    if ( fOld )
    {
	
	if ( !strcmp( password,ch->pcdata->pwd ))
  	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    d->character = oldch;
	    return;
	}
	extract_char( oldch, TRUE ); 
	d->character = ch;			 
	sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
	log_string( log_buf );
	do_help( ch, "motd" );
	d->connected = CON_READ_MOTD;
	sprintf( log_buf, "%s has become %s", name, person );
	log_string( log_buf );
	free(name);
	return;
    }
    else
    {
        write_to_buffer( d,"Nope. Sorry... Character doesn't Exist.\n\r",0);
	d->character = oldch;
	return;
    }
	
}

void do_lore( CHAR_DATA *ch, char *argument )
{ 
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    if ( IS_NPC(ch)
    &&   can_use_skpell( ch, skill_lookup("lore") ))
    {
        send_to_char(
            "You would like to what?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "What Would you like to know more about?\n\r", ch );
        return;
    }

        if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
        {
            send_to_char( "You cannot lore that because you do not have that.\n\r", ch );
            return;
        }


      if ( number_percent( ) < get_skill(ch,skill_lookup("lore")))
       {
        send_to_char( "You learn more about this object:\n\r", ch );
        check_improve(ch,skill_lookup("lore"),TRUE,4);

    sprintf( buf, "Name(s): %s\n\r",
        obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
        obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\rExtra2 bits %s\n\r",
        wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ), extra2_bit_name( obj->extra2_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Weight: %d (10th pounds)\n\r",
        obj->weight / 10);
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d\n\r",
        obj->level, obj->cost);
    send_to_char( buf, ch );

    show_obj_cond( ch, obj );

    /* now give out vital statistics as per identify */

    switch ( obj->item_type )
    {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            sprintf( buf, "Level %d spells of:", obj->value[0] );
            send_to_char( buf, ch );

            if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[1]].name, ch );
                send_to_char( "'", ch );
            }

            if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[2]].name, ch );
                send_to_char( "'", ch );
            }

            if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[3]].name, ch );
                send_to_char( "'", ch );
            }

            if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
            {
                send_to_char(" '",ch);
                send_to_char(skill_table[obj->value[4]].name,ch);
                send_to_char("'",ch);
            }

            send_to_char( ".\n\r", ch );
        break;

        case ITEM_WAND:
        case ITEM_STAFF:
            sprintf( buf, "Has %d(%d) charges of level %d",
                obj->value[1], obj->value[2], obj->value[0] );
            send_to_char( buf, ch );

            if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[3]].name, ch );
                send_to_char( "'", ch );
            }

            send_to_char( ".\n\r", ch );
        break;

        case ITEM_DRINK_CON:
            sprintf(buf,"It holds %s-colored %s.\n\r",
                liq_table[obj->value[2]].liq_color,
                liq_table[obj->value[2]].liq_name);
            send_to_char(buf,ch);
            break;


        case ITEM_WEAPON:
            send_to_char("Weapon type is ",ch);
            switch (obj->value[0])
            {
                case(WEAPON_EXOTIC):
                    send_to_char("exotic\n\r",ch);
                    break;
                case(WEAPON_SWORD):
                    send_to_char("sword\n\r",ch);
                    break;
                case(WEAPON_DAGGER):
                    send_to_char("dagger\n\r",ch);
                    break;
                case(WEAPON_SPEAR):
                    send_to_char("spear/staff\n\r",ch);
                    break;
                case(WEAPON_MACE):
                    send_to_char("mace/club\n\r",ch);
                    break;
                case(WEAPON_AXE):
                    send_to_char("axe\n\r",ch);
                    break;
                case(WEAPON_FLAIL):
                    send_to_char("flail\n\r",ch);
                    break;
                case(WEAPON_WHIP):
                    send_to_char("whip\n\r",ch);
                    break;
                case(WEAPON_POLEARM):
                    send_to_char("polearm\n\r",ch);
                    break;
                case(WEAPON_BOW):
                    send_to_char("bow\n\r",ch);
                    break;
                default:
                    send_to_char("unknown\n\r",ch);
                    break;
            }
            if (obj->pIndexData->new_format)
            {
                sprintf(buf,"Damage is %dd%d (average %d)\n\r",
                    obj->value[1],obj->value[2],
                    (1 + obj->value[2]) * obj->value[1] / 2);
                send_to_char( buf, ch );

                sprintf( buf, "Min damage: %d Max damage: %d\n\r", 
                obj->value[1], obj->value[1] * obj->value[2] );
            }

            else
                sprintf( buf, "Damage is %d to %d (average %d)\n\r",
                    obj->value[1], obj->value[2],
                    ( obj->value[1] + obj->value[2] ) / 2 );
            send_to_char( buf, ch );

            sprintf(buf,"Damage noun is %s.\n\r",
                (obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ?
                    attack_table[obj->value[3]].noun : "undefined");
            send_to_char(buf,ch);

            if (obj->value[4])  /* weapon flags */
            {
                sprintf(buf,"Weapons flags: %s\n\r",
                    weapon_bit_name(obj->value[4]));
                send_to_char(buf,ch);
            }
        break;

        case ITEM_ARMOR:
            sprintf( buf,
            "Armor klass is %d pierce, %d bash, %d slash, and %d vs.magic\n\r",
                obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
            send_to_char( buf, ch );
        break;

        case ITEM_CONTAINER:
            sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
            send_to_char(buf,ch);
            if (obj->value[4] != 100)
            {
                sprintf(buf,"Weight multiplier: %d%%\n\r",
                    obj->value[4]);
                send_to_char(buf,ch);
            }
        break;

       case ITEM_SOCKETS: 
     send_to_char("Gem type is: ",ch); 
     switch (obj->value[0]) 
     { 
        case 0 : send_to_char("none.  Tell an immortal.\n\r",ch);    
        break; 
        case 1 : send_to_char("sapphire.\n\r",ch);   break; 
        case 2 : send_to_char("ruby.\n\r",ch);   break; 
        case 3 : send_to_char("emerald.\n\r",ch);    break; 
        case 4 : send_to_char("diamond.\n\r",ch);    break; 
        case 5 : send_to_char("topaz.\n\r",ch);  break; 
        case 6 : send_to_char("skull.\n\r",ch);  break; 
        default : send_to_char("Unknown.  Tell an immortal.\n\r",ch);    
        break; 
     } 
     send_to_char("Gem value is: ",ch); 
     switch (obj->value[1]) 
         { 
         case 0 : send_to_char("chip.\n\r",ch);    break; 
         case 1 : send_to_char("flawed.\n\r",ch);   break; 
         case 2 : send_to_char("flawless.\n\r",ch);   break; 
         case 3 : send_to_char("perfect.\n\r",ch);    break; 
         default : send_to_char("Unknown.  Tell an immortal.\n\r",ch);   
         break; 
         } 
     break; 
    
    }

    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
        EXTRA_DESCR_DATA *ed;

        send_to_char( "Extra description keywords: '", ch );

        for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );
            if ( ed->next != NULL )
                send_to_char( " ", ch );
        }

        for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next 
)
        {
            send_to_char( ed->keyword, ch );
            if ( ed->next != NULL )
                send_to_char( " ", ch );
        }

        send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        sprintf( buf, "Affects %s by %d, level %d",
            affect_loc_name( paf->location ), paf->modifier,paf->level );
        send_to_char(buf,ch);
        if ( paf->duration > -1)
            sprintf(buf,", %d hours.\n\r",paf->duration);
        else
            sprintf(buf,".\n\r");
        send_to_char( buf, ch );
        if (paf->bitvector)
        {
            switch(paf->where)
            {
                case TO_AFFECTS:
                    sprintf(buf,"Adds %s affect.\n",
                        affect_bit_name(paf->bitvector));
                    break;
                case TO_WEAPON:
                    sprintf(buf,"Adds %s weapon flags.\n",
                        weapon_bit_name(paf->bitvector));
                    break;
                case TO_OBJECT:
                    sprintf(buf,"Adds %s object flag.\n",
                        extra_bit_name(paf->bitvector));
                    break;
                case TO_IMMUNE:
                    sprintf(buf,"Adds immunity to %s.\n",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_RESIST:
                    sprintf(buf,"Adds resistance to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_VULN:
                    sprintf(buf,"Adds vulnerability to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_AFFECTS2:
                    sprintf(buf,"Adds %s affect2.\n",
                        affect2_bit_name(paf->bitvector));
                    break;    
                case TO_OBJECT2:
                   sprintf(buf,"Adds %s object flag.\n",
                        extra2_bit_name(paf->bitvector));    
                default:
                    sprintf(buf,"Unknown bit %d: %d\n\r",
                        paf->where,paf->bitvector);
                    break;
            }
            send_to_char(buf,ch);
        }
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
        sprintf( buf, "Affects %s by %d, level %d.\n\r",
            affect_loc_name( paf->location ), paf->modifier,paf->level );
        send_to_char( buf, ch );
        if (paf->bitvector)
        {
            switch(paf->where)
            {
                case TO_AFFECTS:
                    sprintf(buf,"Adds %s affect.\n",
                        affect_bit_name(paf->bitvector));
                    break;
                case TO_OBJECT:
                    sprintf(buf,"Adds %s object flag.\n",
                        extra_bit_name(paf->bitvector));
                    break;
                case TO_IMMUNE:
                    sprintf(buf,"Adds immunity to %s.\n",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_RESIST:
                    sprintf(buf,"Adds resistance to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_VULN:
                    sprintf(buf,"Adds vulnerability to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_AFFECTS2:
                    sprintf(buf,"Adds %s affect2.\n",
                        affect2_bit_name(paf->bitvector));
                    break;                                 
                case TO_OBJECT2:
                    sprintf(buf,"Adds %s object flag.\n",
                        extra2_bit_name(paf->bitvector));
                    break;                                  
                default:
                    sprintf(buf,"Unknown bit %d: %d\n\r",
                        paf->where,paf->bitvector);
                    break;
            }
            send_to_char(buf,ch);
        }
    }
  }
    return;
}


void do_hometown( CHAR_DATA *ch, char *argument )
{
   char arg[MSL];
   char buf[MSL];
   int cost = 1000;
   int i;

   argument = one_argument( argument, arg );         

   if ( IS_NPC(ch))
   {
       send_to_char( "Mobs don't have hometowns!\n\r", ch );
       return;
   }

   if ( arg[0] == '\0' )
   {
       send_to_char( "Valid hometowns are:\n\r", ch );
       send_to_char( "{C*******************{x\n\r", ch );
       for (i=0;hometown_table[i].name != NULL; ++i)
       {
             sprintf(buf,"[%-15s]\n\r\n\r", hometown_table[i].name );
             send_to_char( buf, ch );
       }
     
       printf_to_char( ch, "[Cost: %-5d gold]\n\r", cost );
       return;
   }
           
   if (get_hometown(arg) == -1)
   {
       send_to_char( "That's not a valid hometown.\n\r", ch );
       return;
   }
  
   if ( ch->gold < cost )
   {
       send_to_char( "You don't have enough gold to change your hometown!\n\r", ch );
       return;
   }

   ch->hometown = get_hometown( arg );
   ch->gold -= cost;
   sprintf( buf, "Your hometown is now set to %s. \n\r",hometown_table[ch->hometown].name );
   send_to_char( buf, ch );
   return;
}         


void do_autoall(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];

   argument = one_argument( argument, arg );

   if ( arg[0] == '\0' )
   {
       send_to_char( "Syntax: autoall <on>\n\r", ch );
       send_to_char( "Syntax: autoall <off>\n\r", ch );
       return;
   }

   if (IS_NPC(ch))
   return;

   if ( !str_cmp( arg, "on" ) )
   {
       SET_BIT(ch->act,PLR_AUTOASSIST);
       SET_BIT(ch->act,PLR_AUTOEXIT);
       SET_BIT(ch->act,PLR_AUTOGOLD);
       SET_BIT(ch->act,PLR_AUTOLOOT);
       SET_BIT(ch->act,PLR_AUTOSAC);
       SET_BIT(ch->act,PLR_AUTOSPLIT);
       SET_BIT(ch->act,PLR_AUTODAMAGE);
       SET_BIT(ch->act,PLR_AUTOMAP);
       send_to_char("All autos turned on.\n\r",ch);
   }

   if ( !str_cmp( arg, "off" ) )
   {
       REMOVE_BIT(ch->act,PLR_AUTOASSIST);
       REMOVE_BIT(ch->act,PLR_AUTOEXIT);
       REMOVE_BIT(ch->act,PLR_AUTOGOLD);
       REMOVE_BIT(ch->act,PLR_AUTOLOOT);
       REMOVE_BIT(ch->act,PLR_AUTOSAC);
       REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
       REMOVE_BIT(ch->act,PLR_AUTODAMAGE);
       REMOVE_BIT(ch->act,PLR_AUTOMAP);
       send_to_char("All autos turned off.\n\r",ch);
   }

   return;
}

void do_petname(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char command[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char name[MSL];

  smash_tilde(argument);   

  if ( ch->pet == NULL )
  {
    send_to_char("You dont have a pet!\n\r", ch);
    return;
  }

  if ( ch->in_room != ch->pet->in_room )
  {
    send_to_char("Kinda hard for your pet to learn his new name\n\r",ch);
    send_to_char("if he's not even with you! *boggle*\n\r", ch);
    return;
  }        
 
  argument = one_argument(argument, command);

  if ( command[0] == '\0' ||  argument[0] == '\0' )
  {
    send_to_char(
    "\n\rsyntax: petname [name|short|long|desc] <argument>\n\r",ch);
    send_to_char( "\n\r  example: \"petname name fido\n\r", ch);
    send_to_char( "            \"petname short is hungry!\"\n\r",ch);
    send_to_char("\n\rTry using color in the descriptions!\n\r\n\r", ch);
    send_to_char(
    "See \"help petname\" and \"help color\" for more information.\n\r",ch);
    return;
  } 
 
  if ( !str_prefix(command, "name") )
  {
     if ( argument[0] == '{' ) 
       {
          argument++; argument++;
       }

     argument = one_argument(argument, arg2);

     sprintf(name, "%s%s", PLAYER_DIR, capitalize(arg2));
     if (file_exists(name))
     {
       send_to_char("That name is already taken!\n\r", ch);
       return;
     }

     free_string(ch->pet->name);
     ch->pet->name = capitalize( str_dup(arg2) );
     sprintf(buf, "Your pet has been renamed to: %s\n\r", ch->pet->name);
     send_to_char(buf, ch);

  } /* name */  

  else

  if ( !str_prefix(command, "short") )
  {
    if ( argument[0] == '\0' ) return;
    free_string(ch->pet->short_descr);

    /* short description should not have an "\n\r" appended to it. */
    ch->pet->short_descr = str_dup(argument);

    sprintf(buf, "%s's short description set to: \n\r%s\n\r",
            ch->pet->name, ch->pet->short_descr);

    send_to_char(buf, ch);
  } /* short */
  else

  if ( !str_prefix(command, "long") )
  {
    if ( argument[0] == '\0' ) return;
    free_string(ch->pet->long_descr);

    /* long descr needs "\n\r" appended to it. */
    sprintf(buf, "%s\n\r", argument);
    ch->pet->long_descr = str_dup(buf);

    sprintf(buf, "%s's long description set to: \n\r%s\n\r",
            ch->pet->name, ch->pet->long_descr);
    send_to_char(buf, ch);
  }  
  else

  if ( !str_prefix(command, "description") )
  {
#if defined (OldDesc)
    do_description(ch, argument);
#else
    if ( argument[0] == '\0' )
    {
        string_append( ch, &ch->pet->description );
        return;
    }
#endif
  } 

  else
    do_help(ch, "petname");
  
  return;

}            
 

void do_finger( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *b;

    FILE *fp;
    bool fOld;
    int s, d, h, m;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Finger whom?\n\r", ch );
        return;
    }          

    if ( ( victim = get_char_world( ch, arg ) ) != NULL)
    {
        if (!IS_NPC(victim) && can_see( ch, victim ) )
        {
    	    do_function(ch, &do_whois, arg);
            act( "\n\r$N is on right now!", ch, NULL, victim, TO_CHAR );
       
    	    s = current_time - victim->pcdata->last_typed;
            d = s/86400;
    	    s-=d*86400;
    	    h = s/3600;
    	    s -= h*3600;
    	    m = s/60;
	    s -= m*60;   
	    printf_to_char(ch, "They haven't typed anything in: %d days %d hours %d minutes %d seconds\n\r", d, h,m,s);    
    
            for ( b = descriptor_list; b != NULL; b = b->next )
            {
                 if ( b->connected != CON_PLAYING 
                 &&   b->character != victim )
                 {
                     continue;
                 }
     
                 if ( ch->level >= LEVEL_IMMORTAL )
                 {
                 if ( b->character == victim && b->host == NULL )
                 {
                      printf_to_char( ch, "\n\rCurrent login: %s\n\r", victim->pcdata->host );
                 }
                      
                 else if ( b->character == victim )
                 {
                      printf_to_char( ch, "\n\r{BCurrent login{x: {W%s{x\n\r", victim->pcdata->host );
               	      printf_to_char( ch, "{BLast login{x   : {W%s{x\n\r", b->host );
                 }
            }
            }

            if (victim->desc == NULL)
	        act( "However, $N is linkdead.", ch, NULL, victim, TO_CHAR );
            return;
        }
    }

    victim = new_char();
    victim->pcdata = new_pcdata();

    fOld = FALSE;  
    sprintf( buf, "%s%s", PLAYER_DIR, capitalize( arg ) );
    if (file_exists(buf))
    {
        int iNest;

	fp = file_open(buf, "r");

        for ( iNest = 0; iNest < MAX_NEST; iNest++ )
            rgObjNest[iNest] = NULL;   

        fOld = TRUE;
        for ( ; ; )
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
                bug( "Load_char_obj: # not found.", 0 );
                break;
            }

            word = fread_word( fp );
            if      ( !str_cmp( word, "PLAYER" ) ) fread_char( victim, fp );
            else if ( !str_cmp( word, "OBJECT" ) ) break;
            else if ( !str_cmp( word, "O"      ) ) break;
            else if ( !str_cmp( word, "PET"    ) ) break;   
            else if ( !str_cmp( word, "END"    ) ) break;
            else
            {
                bug( "Load_char_obj: bad section.", 0 );
                break;
            }
        }
        file_close( fp );
    }          
    if ( !fOld )
    {
        send_to_char("No player by that name exists.\n\r",ch);
        free_pcdata(victim->pcdata);
        free_char(victim);
        return;
    }    

    if ( (victim->level > LEVEL_HERO) && (victim->level > ch->level) && (ch->level < LEVEL_HERO))
    {
        send_to_char("The gods wouldn't like that.\n\r",ch);
        free_pcdata(victim->pcdata);
        free_char(victim);
        return;
    }

    if ( IS_NPC(victim) || victim->pcdata == NULL )
    {
        send_to_char( "Error loading pcdata.  Stop.\n\r", ch );
        free_pcdata(victim->pcdata);
        free_char(victim);
        return;
    }   
    s = current_time - victim->llogoff;
    d = s/86400;
    s-=d*86400;
    h = s/3600;
    s -= h*3600;
    m = s/60;
    s -= m*60;   

    if ( ch->level > LEVEL_IMMORTAL )
    {

        sprintf(buf,"{cName  : {C%s{x\n\r"
                "{cLevel : {C%-11d {cTrust  : {C%d{x\n\r"
                "{cRace  : {C%-5s       {cClass  : {C%-15s{x\n\r"
                "{mQuest : {M%-7d     {gPlayed : {G%d {ghours{x\n\r"
                "{mPkills: {R%-10d  {mPDeaths: {B%d{x\n\r"
                "{mMkills: {B%-10d  {mMDeaths: {R%d{x\n\r"
                "{mHdam  : {R%-10d  {gClan   : {G%s{x\n\r\n\r"  
                "Last {Dlogged off{x on %s\n\r",   
                victim->name,
                victim->level,
                victim->trust,
                pc_race_table[victim->race].name,
                class_long( victim ),
                victim->pcdata->questpoints,
                (int) (victim->played / 3600 ),
                victim->pcdata->kills_pc,   
                         victim->pcdata->deaths_pc,
                victim->pcdata->kills_mob, victim->pcdata->deaths_mob, 
                victim->pcdata->hdam, clan_table[victim->clan].name,
                (char *) ctime(&victim->llogoff));
                
    }

    else
        sprintf(buf,"{cName: {C%s{x\n\r"
                "{cLevel: {C%-11d {gClan: {G%s{x\n\r"
                "{cClass: {C%-11s {cRace: {C%s{x\n\r"
                "{mPkills: {R%-10d {mPDeaths: {B%d{x\n\r"
                "{mMkills: {B%-10d {mMDeaths: {R%d{x\n\r"
                "Last {Dlogged off{x on %s\n\r",
                victim->name,
                victim->level,
                clan_table[victim->clan].name,
                class_long( victim ),
                pc_race_table[victim->race].name,
                victim->pcdata->kills_pc,        
                victim->pcdata->deaths_pc,
                victim->pcdata->kills_mob,
                victim->pcdata->deaths_mob,
                (char *) ctime(&victim->llogoff));

    send_to_char(buf,ch);
    printf_to_char(ch,
        "{wThat was {B%d {wdays, {C%d {whours, {M%d {wminutes and {Y%d {wseconds ago.{x\n\r",
        d, h, m, s);

    if ( IS_IMMORTAL(ch))
    printf_to_char(ch,"\n\r{BLast login{x: {W%s\n\r", victim->pcdata->host );

    if ( victim == char_list )
    {
       char_list = victim->next;
    }
    else
    {
        CHAR_DATA *prev;

        for ( prev = char_list; prev != NULL; prev = prev->next )
        {
            if ( prev->next == victim )
            {
                prev->next = victim->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Finger: char not found.", 0 );
            return;
        }
    }


        free_pcdata(victim->pcdata);
        free_char(victim);
    return;
}

                  
void do_peek (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;

  if ((victim = get_char_room (ch, NULL, argument)) == NULL)
  {
      send_to_char ("They aren't here.\n\r", ch);
      return;
  }   

  if (number_percent () < get_skill (ch, skill_lookup("peek")))
  {
      act ("You peek at $N's inventory:", ch, NULL, victim, TO_CHAR);
      check_improve (ch, skill_lookup("peek"), TRUE, 4);
      show_list_to_char (victim->carrying, ch, TRUE, TRUE);
      if (get_curr_stat (victim, STAT_INT) > get_curr_stat (ch, STAT_INT))
        act ("$n peeks inside your inventory!", ch, NULL, victim, TO_VICT);
  }
  else
  {
      act ("You failed to peek inside $N's inventory!", ch, NULL, victim, TO_CHAR);
      act ("$n tried to peek inside your inventory!", ch, NULL, victim, TO_VICT);
      act ("$n tried to peek inside $N's inventory!", ch, NULL, victim, TO_NOTVICT);
      check_improve (ch, skill_lookup("peek"), FALSE, 4);
  }
  
  return;  
}

/* cloak wear and remove */
void do_cloak(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	one_argument(argument,arg);

	if(IS_NPC(ch))
	{
		send_to_char("You can't do this on Mobs.\n\r",ch);
		return;
	}

	if(arg[0] == '\0')
	{
		send_to_char("Syntax: cloak <part or close>\n\r",ch);
		return;
	}
	if( ( obj = get_eq_char(ch,WEAR_CLOAK) ) == NULL )
	{
	    send_to_char("You arn't wearing a cloak.\n\r",ch);
	    return;
	}

	if(!str_cmp(arg,"close"))
	{
	    if(obj->part == FALSE)
	    {
		send_to_char("Your cloak is already closed.\n\r",ch);
		return;
	    }

	    obj->part = FALSE;
	    send_to_char("You close and tie your cloak.\n\r",ch);
	    act("$n closes and ties $s cloak",ch,NULL,NULL,TO_ROOM);
	    return;
	}

	if(!str_cmp(arg,"part"))
	{
	    if(obj->part == TRUE)
	    {
		send_to_char("Your cloak is already parted.\n\r",ch);
		return;
	    }

	    obj->part = TRUE;
	    send_to_char("You part you cloak, exposing the equipment beneath.\n\r",ch);
	    act("$n unties $s cloak.",ch,NULL,NULL,TO_ROOM);
	    return;
	}

	return;
}

/* hood remove and lift */
void do_hood(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_STRING_LENGTH];
	OBJ_DATA *obj;

	buf[0] = '\0';
	one_argument(argument,arg);

	if(IS_NPC(ch))
	{
		send_to_char("Mobs are not allowed to do this.\n\r",ch);
		return;
	}

	if(arg[0] == '\0')
	{
		send_to_char("Syntax: hood <raise> or <remove or lower>\n\r",ch);
		return;
	}

	if( ( obj = get_eq_char(ch, WEAR_HOOD) ) == NULL)
	{
	    send_to_char("You are not wearing a hood which you can raise!\n\r",ch);
	    return;
    	}

	if(!str_cmp(arg,"raise"))
	{
		if(!IS_SET(ch->mask, HOOD_REMOVED) )
		{
		    send_to_char("Your hood is already raised!\n\r",ch);
		    return;
 		}
		send_to_char("You raise the hood of your cloak masking yourself.\n\r",ch);
		act("Someone raises the hood of their cloak.",ch,NULL,NULL,TO_ROOM);
		REMOVE_BIT(ch->mask,HOOD_REMOVED);
		SET_BIT(ch->mask,HOOD_RAISED);
		sprintf(buf, "Someone wearing %s", obj->short_descr);
		free_string(ch->short_descr);
		ch->short_descr = str_dup(buf);
		return;
	}
	else if(!str_cmp(arg,"remove") || !str_cmp(arg,"lower") )
	{
		if(IS_SET(ch->mask, HOOD_REMOVED) )
		{
		    send_to_char("Your hood is already down!\n\r",ch);
		    return;
 		}
		send_to_char("You remove your hood to reveal yourself.\n\r",ch);
		act("Someone removes the hood of their cloak.",ch,NULL,NULL,TO_ROOM);
		REMOVE_BIT(ch->mask,HOOD_RAISED);
		SET_BIT(ch->mask,HOOD_REMOVED);
		free_string(ch->short_descr);
		ch->short_descr = str_dup(ch->name);
		return;
	}
	else
	{
		send_to_char("Syntax: hood <raise> or <remove or lower>\n\r",ch);
		return;
	}

	return;
}

void do_arshow(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm2,COMM_SHOW_ARMOR))
    {
      send_to_char("Armor messages will no longer be shown in score.\n\r",ch);
      REMOVE_BIT(ch->comm2,COMM_SHOW_ARMOR);
    }      

    else
    {
      send_to_char("Armor messages will now be shown in score.\n\r",ch);
      SET_BIT(ch->comm2,COMM_SHOW_ARMOR);
    }
}        
    
#define NUM_DAYS 35
/* Match this to the number of days per month; this is the moon cycle */
#define NUM_MONTHS 17
/* Match this to the number of months defined in month_name[].  */
#define MAP_WIDTH 72
#define SHOW_WIDTH MAP_WIDTH/2
#define MAP_HEIGHT 9
/* Should be the string length and number of the constants below.*/
const char * star_map[] =
{
"   W.N     ' .     :. M,N     :  y:.,N    `  ,       B,N      .      .  ",
" W. :.N .      G,N  :M.: .N  :` y.N    .      :     B:   .N       :     ",
"    W:N    G.N:       M:.,N:.:   y`N      ,    c.N           .:    `    ",
"   W.`:N       '. G.N  `  : ::.      y.N      c'N      B.N R., ,N       ",
" W:'  `:N .  G. N    `  :    .y.N:.          ,     B.N      :  R:   . .N",
":' '.   .    G:.N      .'   '   :::.  ,  c.N   :c.N    `        R`.N    ",
"      :       `        `        :. ::. :     '  :        ,   , R.`:N    ",
"  ,       G:.N              `y.N :. ::.c`N      c`.N   '        `      .",
"     ..        G.:N :           .:   c.N:.    .              .          "
};

/***************************CONSTELLATIONS*******************************
  Lupus     Gigas      Pyx      Enigma   Centaurus    Terken    Raptus
   The       The       The       The       The         The       The  
White Wolf  Giant     Pixie     Sphinx    Centaur      Drow     Raptor
*************************************************************************/	
const char * sun_map[] =
{
"\\'|'/",
"- O -",
"/.|.\\"
};
const char * moon_map[] =
{
" @@@ ",
"@@@@@",
" @@@ "
};

void look_sky ( CHAR_DATA * ch )
{
    static char buf[MAX_STRING_LENGTH];
    static char buf2[4];
    int starpos, sunpos, moonpos, moonphase, i, linenum;

   send_to_char("You gaze up towards the heavens and see:\n\r",ch);

   sunpos  = (MAP_WIDTH * (24 - time_info.hour) / 24);
   moonpos = (sunpos + time_info.day * MAP_WIDTH / NUM_DAYS) % MAP_WIDTH;
   if ((moonphase = ((((MAP_WIDTH + moonpos - sunpos ) % MAP_WIDTH ) +
                      (MAP_WIDTH/16)) * 8 ) / MAP_WIDTH)
		         > 4) moonphase -= 8;
   starpos = (sunpos + MAP_WIDTH * time_info.month / NUM_MONTHS) % MAP_WIDTH;
   /* The left end of the star_map will be straight overhead at midnight during 
      month 0 */

   for ( linenum = 0; linenum < MAP_HEIGHT; linenum++ )
   {
     if ((time_info.hour >= 6 && time_info.hour <= 18) &&
         (linenum < 3 || linenum >= 6))
       continue;
     sprintf(buf,"{W|{x");
     for ( i = MAP_WIDTH/4; i <= 3*MAP_WIDTH/4; i++)
     {
       /* plot moon on top of anything else...unless new moon & no eclipse */
       if ((time_info.hour >= 6 && time_info.hour <= 18)  /* daytime? */
        && (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2) /* in sky? */
        && ( i >= moonpos - 2 ) && (i <= moonpos + 2) /* is this pixel near moon? */
        && ((sunpos == moonpos && time_info.hour == 12) || moonphase != 0  ) /*no eclipse*/
        && (moon_map[linenum-3][i+2-moonpos] == '@'))
       {
         if ((moonphase < 0 && i - 2 - moonpos >= moonphase) ||
             (moonphase > 0 && i + 2 - moonpos <= moonphase))
           strcat(buf,"{W@");
         else
           strcat(buf," ");
       }
       else
       if ((linenum >= 3) && (linenum < 6) && /* nighttime */
           (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2) /* in sky? */
        && ( i >= moonpos - 2 ) && (i <= moonpos + 2) /* is this pixel near moon? */
        && (moon_map[linenum-3][i+2-moonpos] == '@'))
       {
         if ((moonphase < 0 && i - 2 - moonpos >= moonphase) ||
             (moonphase > 0 && i + 2 - moonpos <= moonphase))
           strcat(buf,"{W@");
         else
           strcat(buf," ");
       }
       else /* plot sun or stars */
       {
         if (time_info.hour>=6 && time_info.hour<=18) /* daytime */
         {
           if ( i >= sunpos - 2 && i <= sunpos + 2 )
           {
             sprintf(buf2,"{Y%c",sun_map[linenum-3][i+2-sunpos]);
             strcat(buf,buf2);
           }
           else
             strcat(buf," ");
         }
         else
         {
           switch (star_map[linenum][(MAP_WIDTH + i - starpos)%MAP_WIDTH])
           {
             default     : strcat(buf," ");    break;
             case '.'    : strcat(buf,".");    break;
             case ','    : strcat(buf,",");    break;
             case ':'    : strcat(buf,":");    break;
             case '`'    : strcat(buf,"`");    break;
             case 'R'    : strcat(buf,"{R ");  break;
             case 'G'    : strcat(buf,"{G ");  break;
             case 'B'    : strcat(buf,"{B ");  break;
             case 'W'    : strcat(buf,"{W ");  break;
             case 'M'    : strcat(buf,"{M ");  break;
             case 'N'    : strcat(buf,"{x ");  break;
             case 'y'    : strcat(buf,"{Y ");  break;
             case 'c'    : strcat(buf,"{c ");  break;
           }
         }
       }
     }
     strcat(buf,"{W|{x\n\r");
     send_to_char(buf,ch);
   }
     if(!str_cmp(ch->name, "Davion") )
     {
	printf_to_char(ch, "Moonphase %d", moon_status());
     }
}


void do_mobdeaths(CHAR_DATA * ch, char *argument)
{
    extern int top_mob_index;
    BUFFER *buffer;
    char buf[MSL];
    MOB_INDEX_DATA *pMobIndex;
    MOB_INDEX_DATA *mobs[top_mob_index];
    long vnum;
    int nMatch,
    count, i;
    bool found;

    found = FALSE;
    nMatch = 0;
    count = 0;
    i = 0;
    buffer = new_buf();

    add_buf(buffer, "{R                 ShadowStorms Most Popular Mobs                       {x\n\r");
    add_buf(buffer, "{GNum  Mob Name                  Level  Area Name            Deaths{x\n\r");
    add_buf(buffer, "{m---  ------------------------- -----  -------------------- ------{x\n\r");

    for (vnum = 0; nMatch < top_mob_index; vnum++)
    {
	if ((pMobIndex = get_mob_index(vnum)) != NULL)
	{
	    nMatch++;

	    if (pMobIndex->killed > 2)
	    {
		mobs[count] = pMobIndex;
		count++;
		found = TRUE;
	    }
	}
    }

    qsort(mobs, count, sizeof(pMobIndex), compare_mdeaths);

    for (i = 0; i < count; i++)
    {
	if (i == 49)
	    break;
        sprintf(buf,"%3d) %-25s %5d  %-20s {R%6d{x\n\r", i + 1, mobs[i]->short_descr, mobs[i]->level, mobs[i]->area->name, mobs[i]->killed);
        add_buf( buffer, buf );
    }

    if (!found)
	add_buf(buffer, "\n\rNo Mobs listed yet.\n\r");

    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);

    return;
}

int compare_mkills(const void *v1, const void *v2)
{
    return (*(MOB_INDEX_DATA * *)v2)->kills - (*(MOB_INDEX_DATA * *)v1)->kills;
}

void do_mobkills(CHAR_DATA * ch, char *argument)
{
    extern int top_mob_index;
    BUFFER *buffer;
    char buf[MSL];
    MOB_INDEX_DATA *pMobIndex;
    MOB_INDEX_DATA *mobs[top_mob_index];
    long vnum;
    int nMatch,
     count,
     i;
    bool found;

    found = FALSE;
    nMatch = 0;
    count = 0;
    i = 0;
    buffer = new_buf();

    add_buf(buffer, "{R                 Shadowstorms Most Dangerous Monsters                 {x\n\r");
    add_buf(buffer, "{GNum  Mob Name                  Level  Area Name            Deaths{x\n\r");
    add_buf(buffer, "---  ------------------------- -----  -------------------- ------{x\n\r");

    for (vnum = 0; nMatch < top_mob_index; vnum++)
    {
	if ((pMobIndex = get_mob_index(vnum)) != NULL)
	{
	    nMatch++;

	    if (pMobIndex->kills > 2)
	    {
		mobs[count] = pMobIndex;
		count++;
		found = TRUE;
	    }
	}
    }

    qsort(mobs, count, sizeof(pMobIndex), compare_mkills);

    for (i = 0; i < count; i++)
    {
	if (i == 49)
	    break;
	sprintf(buf, "%3d) %-25s %5d  %-20s {R%6d{x\n\r", i + 1, mobs[i]->short_descr, mobs[i]->level, mobs[i]->area->name, mobs[i]->kills);
        add_buf( buffer, buf );
    }

    if (!found)
	add_buf(buffer, "\n\rNo Mobs listed yet.\n\r");

    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);

    return;
}

void do_nickname(CHAR_DATA *ch, char *argument )
{
    NICK_DATA *nick;
    char arg[MSL], arg2[MSL], buf[MSL];
    CHAR_DATA *victim; 
    bool found = FALSE;

    arg[0] = '\0';
    arg2[0] = arg[0];

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if(arg[0] == '\0' && arg2[0] == '\0' )
    {
    	if(ch->pcdata->nick_first == NULL )
        {
	   send_to_char("You currently have no people nicknamed.\n\r",ch);
	   return;
        }
		else
		{
			send_to_char("You have the following nicknames.\n\r",ch);
			for(nick = ch->pcdata->nick_first; nick; nick = nick->next )
			{
				sprintf(buf, "{W%s{w is nicknamed to: {W%s{x\n\r",nick->shortd, nick->nickname);
				send_to_char(buf,ch);
			}
			return;
		}
    }
  
    if(arg[0] == '\0' || arg2[0] == '\0' )
    {
		send_to_char("Syntax:\n\r\tnickname <Character to be named> <String to namethem>\n\r\tnickname erase [all/playername]\n\r",ch);
		return;
    }

    if(!str_cmp(arg, "erase") )
    {
	if(!str_cmp(arg2, "all") )
	{
	    for(nick = ch->pcdata->nick_first; nick; nick = nick->next)
		free_nick(nick);
	    ch->pcdata->nick_first = NULL;
	    send_to_char("Nicknames erased.\n\r",ch);
	    return;
	}
	else {
	for(nick = ch->pcdata->nick_first; nick; nick = nick->next)
	{
	    if(!str_cmp(nick->shortd, arg2) )
	    {
		if( nick == ch->pcdata->nick_first )
		        ch->pcdata->nick_first = nick->next;
			
	       	free_nick(nick);
		printf_to_char(ch, "%s erased from your nickname list", arg2 );
		break;
	    }
  	}
	return;
	}
    }

    if( ( victim = get_char_room(ch, NULL, arg) ) == NULL )
    {
		send_to_char("They arn't here to nickname!\n\r",ch);
		return;
    }

	if(IS_NPC(victim) )
	{
		send_to_char("Not on mobs.\n\r",ch);
		return;
    }

  
    for(nick = ch->pcdata->nick_first; nick; nick = nick->next)
    {
		if(!str_cmp(victim->name, nick->realname) )
		{
			found = TRUE;
		    break;
 		}
    }

    if(!found)
    {
		nick = new_nick();
		free_string(nick->realname);
		free_string(nick->nickname);
		free_string(nick->shortd  );
		nick->realname = str_dup(victim->name);
		nick->nickname = str_dup(capitalize(arg2));
		nick->shortd   = str_dup(victim->short_descr);

		nick_to_char(ch, nick);	
		printf_to_char(ch, "%s has been nicknamed too: %s\n\r",nick->shortd, nick->nickname);
		return;
    }
    else
    {
		free_string(nick->nickname);
		free_string(nick->shortd  );
		nick->nickname = str_dup(capitalize(arg2));
		nick->shortd   = str_dup(victim->short_descr);
		printf_to_char(ch, "%s's nickname has been changed too %s\n\r",nick->shortd, nick->nickname);
		return;
    }	


}

void do_shortdescr(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];

    arg[0] = '\0';
  
    sprintf(arg, "%s", argument);

    if(argument[0] == '\0' )
    {
	send_to_char("Syntax: shortdesc <shortdescripton>\n\r",ch);
	return;
    }

    if(strlen_color(arg) > 60 )
    {
	send_to_char("Short description over 60 characters. Description rejected.\n\r",ch);
	return;
    }
    if(strstr(argument, ch->name) )
    {
	send_to_char("Your name must not be included in your short description.\n\r",ch);
	return;
    }
    if(strlen_color(arg) < 5)
    {
	send_to_char("Pfft. It has to be longer than that!\n\r",ch);
	return;
    }

    printf_to_char(ch, "Your short desc has been changed too: %s\n\r",arg);
    free_string(ch->short_descr);
    ch->short_descr = str_dup(arg);
    return;
}

void do_introduce( CHAR_DATA *ch, char *argument)
{
    char arg[MSL], buf[MSL];
    CHAR_DATA *victim;
    NICK_DATA *nick;

    arg[0] = '\0';

    argument = one_argument(argument, arg);

    if(arg[0] == '\0')
    {
	send_to_char("Introduce yourself to whom?\n\r",ch);
	return;
    }
    
    if( (victim = get_char_room(ch, NULL, arg) ) == NULL )
    {
	send_to_char("You kindly introduce yourself to a wall.\n\r",ch);
	return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "You can't introduce yourself to a mob.\n\r", ch );
        return;
    }   

    if( has_nick_for_ch(victim, ch) )
    {
	send_to_char("You can't introduce yourself to someone who knows you!\n\r",ch);
	return;
    }

    sprintf(buf, "Hello, my name is %s.", ch->name);
    do_function(ch, &do_say, buf);

    nick = new_nick();
    free_string(nick->realname);
    free_string(nick->nickname);
    free_string(nick->shortd  );
    nick->realname = str_dup(ch->name);
    nick->nickname = str_dup(ch->name);
    nick->shortd   = str_dup(ch->short_descr);
    nick_to_char(victim, nick);	
}

void do_immtitle(CHAR_DATA *ch, char *argument)
{
  if (argument[0] == '\0')
  {
    ch->pcdata->immtitle = NULL;
    send_to_char("Immtitle cleared.\n\r", ch);
    return;
  }

  if (strlen_color(argument) > 12)  
  {
    send_to_char("Immtitle must be 12 or under characters long.\n\r", ch); return;
  }

  free_string( ch->pcdata->immtitle );
  ch->pcdata->immtitle = str_dup(argument);
  send_to_char("Immtitle set.\n\r", ch);
}

void do_version (CHAR_DATA * ch, char *argument)
{
  char version[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];

  version[0] = '\0';

  if (IS_NPC (ch))
    return;

  if (ch->level < MAX_LEVEL)	/* Stuff trusted characters! */
    {
      read_version (version);
      sprintf (buf, "\n\r{CShadowStorms{x current version is : %s\n\r", version);
      send_to_char (buf, ch);
      return;
    }

  else
    {
      if (argument[0] == '\0')
	{
	  read_version (version);
	  sprintf (buf, "\n\r{CShadowStorms{x current version is : %s\n\r", version);
	  send_to_char (buf, ch);
	  return;
	}
      else
	{
	  if (write_version (argument))
	    {
	      sprintf (buf, "{CShadowStorms{x current version changed to %s\n\r", argument);
	      send_to_char (buf, ch);
	      sprintf (buf, "%s has changed {CShadowStorms{x current version number to %s",
		       ch->name, argument);
	      log_string (buf);
	    }
	  else
	    send_to_char ("Write version failed - please report", ch);

	  return;
	}
    }

}


bool write_version (char *argument)	/*Returns true if sucsessful, else false */
{
  FILE *versionfp;
  char buf[MAX_STRING_LENGTH];

  versionfp = file_open(VERSION_FILE,"w");
  sprintf (buf, "%s\n", argument);
  fprintf (versionfp, "#\n");
  fprintf (versionfp, buf);
  file_close (versionfp);
  return TRUE;
}

void read_version (char *version)	/*dumps the version No. in version */
{
    FILE *versionfp;

    if (!file_exists(VERSION_FILE))
    {
	bug ("read_version : can't open version file", 0);
	sprintf (version, "V0.0 -- Please report!\n\r");
	return;
    }

    versionfp = file_open(VERSION_FILE, "r") ;

    if (fread_letter (versionfp) != '#')
    {
	bug ("read_version : # not found", 0);
	sprintf (version, "V0.0 -- Please report!\n\r");
    }
    else
	sprintf (version, fread_string_eol (versionfp));

    file_close(versionfp);
    return;
}

void display_bp(CHAR_DATA *ch, CHAR_DATA *victim)
{
   int	bp;
   char  buf[MSL];

   if ( (IS_NPC(ch)) || (IS_NPC(victim)) )
      return;
   
   if (victim->bp_points == 0)
       return;
	  
   if (victim->bp_points < 0)
   {
       bp = -1 * victim->bp_points;  
       sprintf(buf, "$N has %d black marks above $S shoulder.", bp);
       act( buf, ch, NULL, victim, TO_CHAR );
       return;
   }

   sprintf(buf, "$N has %d glowing rings floating above $S shoulder.\n\r", victim->bp_points);
   act( buf, ch, NULL, victim, TO_CHAR );
   return;
}
   
void do_autolevel( CHAR_DATA *ch, char *argument )
{
    if (IS_SET(ch->comm2,COMM_AUTOLEVEL))
    {
      send_to_char("You must now find the leveler and level on your own!\n\r",ch);
      REMOVE_BIT(ch->comm2,COMM_AUTOLEVEL);
    }
    else
    {
	  if (!IS_NPC(ch) && ch->pcdata->current_exp > 0)
	  {
              gain_exp(ch, ch->pcdata->current_exp);
	      ch->pcdata->current_exp = 0;
	  }
  
          send_to_char("You will now be automatically leveled!\n\r",ch);
          SET_BIT(ch->comm2,COMM_AUTOLEVEL);
    }
}    

void do_petlevel( CHAR_DATA *ch, char *argument )
{
   
  if ( ch->pet == NULL )
  {
      send_to_char("You need a pet before you can level one up!\n\r", ch );
      return;
  }
 
  if (IS_SET( ch->comm2, COMM_PETLEVEL ) )
  {  
      send_to_char("Your pets will no longer gain experience and level!\n\r", ch );
      REMOVE_BIT( ch->comm2, COMM_PETLEVEL );
      return;
  }

  else 
  {
      send_to_char("Your pets will now gain experience and level with you!\n\r", ch );
      SET_BIT( ch->comm2, COMM_PETLEVEL );
      return;
  }
}

void do_affects2 (CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA *paf, *paf_last = NULL;
    char *buf4;
    char buf3[MSL];
    char buf2[MSL];
    char buf[MSL];
    bool found = FALSE;
    long filter;
    long printme;
    BUFFER *buffer;
    OBJ_DATA *obj;
    int iWear;
    CHAR_DATA *victim;

    if ( IS_IMMORTAL ( ch ) && argument[0] )
    {
        victim = get_char_world ( ch, argument );
        if ( victim == NULL )
        {
            send_to_char ( "They're not in the game.\n\r", ch );
            return;
        }
    }
    else
        victim = ch;

    buffer = new_buf (  );
    if ( victim->affected != NULL )
    {
        if ( victim == ch )
            add_buf ( buffer,
                      "You are affected by the following spells:\n\r" );
        else
        {
            sprintf ( buf, "%s is affected by the following spells:\n\r",
                      PERS(victim, ch) );
            add_buf ( buffer, buf);
        }
        for ( paf = victim->affected; paf != NULL; paf = paf->next )
        {
            if ( paf_last != NULL && paf->type == paf_last->type )
            {
                if ( get_trust ( victim ) >= 20 )
                    add_buf ( buffer, "                          " );
                else
                    continue;
            }
            else
            {
                sprintf ( buf, "Spell: %-19s", skill_table[paf->type].name != NULL
                    ? skill_table[paf->type].name : "unknown" );
                add_buf ( buffer, buf);
            }

            if ( get_trust ( victim ) >= 20 )
            {
                sprintf ( buf, ": modifies %s by %d ",
                          flag_string ( apply_flags, paf->location ),
                          paf->modifier );
                add_buf ( buffer, buf);
                if ( paf->duration == -1 )
                    add_buf ( buffer, "permanently" );
                else
                {
                    sprintf ( buf, "for %d hours", paf->duration );
                    add_buf ( buffer, buf);
                }
            }

            add_buf ( buffer, "\n\r" );
            paf_last = paf;
        }
        found = TRUE;
        add_buf ( buffer, "\n\r" );
    }
    if ( race_table[victim->race].aff != 0 &&
         IS_AFFECTED ( victim, race_table[victim->race].aff ) )
    {
        if ( victim == ch )
            add_buf ( buffer,
                      "You are affected by the following racial abilities:\n\r" );
        else
        {
            sprintf ( buf,
                      "%s is affected by the following racial abilities:\n\r",
                      PERS(victim, ch) );
            add_buf ( buffer, buf);
        }

        strcpy ( buf3,
                 flag_string ( affect_flags, race_table[victim->race].aff ) );
        buf4 = buf3;
        buf4 = one_argument ( buf4, buf2 );
        while ( buf2[0] )
        {
            sprintf ( buf, "Spell: %-19s", buf2 );
            add_buf ( buffer, buf );
            add_buf ( buffer, "\n\r" );
            buf4 = one_argument ( buf4, buf2 );
        }
        found = TRUE;
        add_buf ( buffer, "\n\r" );
    }
    if ( victim->affected_by != 0 &&
         ( victim->affected_by != race_table[victim->race].aff ) )
    {
        bool print = FALSE;

        for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
        {
            if ( ( obj = get_eq_char ( victim, iWear ) ) != NULL )
            {
                for ( paf = obj->affected; paf != NULL; paf = paf->next )
                {
                    if ( !IS_SET ( victim->affected_by, paf->bitvector ) )
                        continue;

                    if ( paf->where != TO_AFFECTS )
                        continue;

                    filter = paf->bitvector;
                    filter &= victim->affected_by;
                    printme = filter;
                    if ( !print )
                    {
                        if ( victim == ch )
                            add_buf ( buffer,
                                      "You are affected by the following equipment spells:\n\r" );
                        else
                        {
                            sprintf ( buf,
                                      "%s is affected by the following equipment spells:\n\r",
                                      PERS(victim, ch) );
                            add_buf ( buffer, buf);
                        }

                        print = TRUE;
                    }

                    strcpy ( buf3, flag_string ( affect_flags, printme ) );
                    buf4 = buf3;
                    buf4 = one_argument ( buf4, buf2 );
                    while ( buf2[0] )
                    {
                        sprintf ( buf, "Spell: %-19s: %s", buf2,
                                  obj->short_descr );
                        add_buf ( buffer, buf);
                        add_buf ( buffer, "\n\r" );
                        buf4 = one_argument ( buf4, buf2 );
                    }
                }
                if ( !obj->enchanted )
                {
                    for ( paf = obj->pIndexData->affected; paf != NULL;
                          paf = paf->next )
                    {
                        if ( !IS_SET ( victim->affected_by, paf->bitvector ) )
                            continue;
                        if ( paf->where != TO_AFFECTS )
                            continue;
                        filter = paf->bitvector;
                        filter &= victim->affected_by;
                        printme = filter;
                        if ( !print )
                        {
                            if ( victim == ch )
                                add_buf ( buffer,
                                          "You are affected by the following equipment spells:\n\r" );
                            else
                            {
                                sprintf ( buf,
                                          "%s is affected by the following equipment spells:\n\r",
                                          PERS(victim, ch) );
                                add_buf ( buffer, buf);
                            }
                            print = TRUE;
                        }

                        strcpy ( buf3,
                                 flag_string ( affect_flags, printme ) );

                        buf4 = buf3;
                        buf4 = one_argument ( buf4, buf2 );
                        while ( buf2[0] )
                        {
                            sprintf ( buf, "Spell: %-19s: %s", buf2,
                                      obj->short_descr );
                            add_buf ( buffer, buf);
                            add_buf ( buffer, "\n\r" );
                            buf4 = one_argument ( buf4, buf2 );
                        }
                    }
                }
            }
        }
        found = TRUE;
        if ( print )
            add_buf ( buffer, "\n\r" );
    }
    if ( !found )
    {
        if ( victim == ch )
            add_buf ( buffer, "You are not affected by any spells.\n\r" );
        else
        {
            sprintf ( buf, "%s is not affected by any spells.\n\r",
                      PERS(victim, ch) );
            add_buf ( buffer, buf);
        }
    }

    page_to_char ( buf_string ( buffer ), ch );
    free_buf ( buffer );
    return;
}

void do_helptally(CHAR_DATA *ch, char *argument)
{
        HELP_DATA *pHelp;
        int tally, count = 0;
        char buf[MSL*3], buf2[MSL];
        bool found = FALSE;
            int col;
	CMD_DATA *cmd;

        buf[0] = '\0';
        tally = 0;
            col = 0;
	for ( cmd = cmd_first; cmd; cmd = cmd->next)
        {
		count++;
		for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
                {
                        if ( is_name( cmd->name, pHelp->keyword ) )
                        {
                                tally++;
                                found = TRUE;
                                break;
                        }


                }
                if(!found)
                {

					  sprintf( buf2, " [{c%-11s{x]", cmd->name );
					  strcat( buf, buf2 );

					  if ( ++col % 5 == 0 )
							strcat(buf,  "\n\r");

					


                }
                found = FALSE;
	 }
					  if (col % 5 != 0 )
					  {
							strcat(buf, "\n\r");
							col = 0;
					  }
					  if ( col % 5 != 0 )
							strcat(buf, "\n\r");
	 
	 printf_to_char(ch, "The following commands lack a helpfile.\n\r%s",buf);
         printf_to_char(ch, "You have %d helpfiles that have keywords as a command name. You have %d commands.\n\r", tally, count);
         return;
}

  
int mortal_online()
{
    DESCRIPTOR_DATA *d;
    int i = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *vch;
	vch = d->original ? d->original : d->character;
	if(d->connected != CON_PLAYING)
	    continue;
	if(!IS_IMMORTAL(vch) )
	    i++;
    }
    return i;
}

int immortal_online()
{
    DESCRIPTOR_DATA *d;
    int i = 0;
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *vch;
	vch = d->original ? d->original : d->character;
	if(d->connected != CON_PLAYING)
	    continue;
	if(IS_IMMORTAL(vch) )
	    i++;
    }
    return i;
}

void do_email (CHAR_DATA *ch, char *argument)
{
    EMAIL_DATA * pMail;

    printf_to_char(ch, "%-10s %-20s\n\r", "Name", "E-mail" );
    printf_to_char(ch, "{R======================================{x\n\r");

    if(argument[0] == '\0' )
    {
	for(pMail = email_first ; pMail ; pMail = pMail->next )
	    printf_to_char(ch, "%-10s %-20s\n\r", pMail->name, pMail->email );
        return;
    }
    if( (pMail = email_lookup(argument) ) == NULL )
    {
	send_to_char("No such person has listed there email by that name.\n\r",ch);
	return;
    }
    printf_to_char(ch, "%-10s %-20s\n\r", pMail->name, pMail->email );

    return;
}

void save_email()
{
    FILE *fp;
    EMAIL_DATA *pMail;

    if ( (fp = file_open("../data/email.dat", "w" ) ) == NULL )
    {
	logf2("Email file gone\n\r");
 	return;
    }
     for( pMail = email_first ; pMail ; pMail = pMail->next )
     {
    	fprintf(fp, "Name %s~\n", pMail->name );
	fprintf(fp, "Email %s~\n", pMail->email );
     }
    fprintf(fp, "$\n");
    file_close(fp);
   return;
}

void load_email()
{
     FILE *fp;
     EMAIL_DATA *pMail;
     const char *word;

     if( file_exists("../data/email.dat" ) )
     {
	fp = file_open("../data/email.dat", "r" );
	for(;;)
 	{
	    word = feof( fp ) ? "End" : fread_word( fp );
	    if(word[0] == '$' )
		return;

	    if(!str_cmp(word, "Name" ) )
	    {
		pMail = new_email();
		pMail->name = fread_string(fp);
	    }
	    if(!str_cmp(word, "Email" ) )
		pMail->email = fread_string(fp);
	}
      }
      return;
}

/* show suppression settings - Gregor Stipicic aka Fade, 2001 */
void do_suppress(CHAR_DATA *ch, char *argument)
{
  char arg[MIL];

  argument = one_argument(argument, arg);

  if(IS_NPC(ch))
     return;

  if(arg[0]=='\0')
  {
     send_to_char("{CS{cuppression settings{D:\n\r",ch);
     printf_to_char(ch,"{CW{ceapon flags{D:   %s  {CF{clag effects{D: %s  {CD{codge/etc.{D: %s{x\n\r",
     SUP_OO(ch,SUPPRESS_FLAGS), SUP_OO(ch,SUPPRESS_FLAG_EFFECTS), SUP_OO(ch,SUPPRESS_DODGE));
  
    printf_to_char(ch,"{CO{cbject effects{D: %s{x\n\r", SUP_OO(ch,SUPPRESS_BURN));
    printf_to_char(ch,"{cShields are %s compressed into one line.{x\n\r",
    IS_SET(ch->pcdata->suppress,SUPPRESS_SHIELD)?"":"not ");
    return;
  }
  
  if(!str_prefix(arg,"weapon"))
  {
     if(IS_SET(ch->pcdata->suppress,SUPPRESS_FLAGS))
     {
        REMOVE_BIT(ch->pcdata->suppress,SUPPRESS_FLAGS);
        send_to_char("{cWeapon flags are no longer suppressed.{x\n\r",ch);
     }
   
     else
     {
        SET_BIT(ch->pcdata->suppress,SUPPRESS_FLAGS);
        send_to_char("{cWeapon flags are suppressed now.{x\n\r",ch);
     }
     
     return;
  }

  if(!str_prefix(arg,"flags"))
  {
     if(IS_SET(ch->pcdata->suppress,SUPPRESS_FLAG_EFFECTS))
     {
        REMOVE_BIT(ch->pcdata->suppress,SUPPRESS_FLAG_EFFECTS);
        send_to_char("{cFlag effects are no longer suppressed.{x\n\r",ch);
     }

     else
     {
        SET_BIT(ch->pcdata->suppress,SUPPRESS_FLAG_EFFECTS);
        send_to_char("{cFlag effects are suppressed now.{x\n\r",ch);
     }
     
     return;
  }

  if(!str_prefix(arg,"dodge"))
  {
     if(IS_SET(ch->pcdata->suppress,SUPPRESS_DODGE))
     {
        REMOVE_BIT(ch->pcdata->suppress,SUPPRESS_DODGE);
        send_to_char("{cDodges/etc. are no longer suppressed.{x\n\r",ch);
     }
 
     else
     {
        SET_BIT(ch->pcdata->suppress,SUPPRESS_DODGE);
        send_to_char("{cDodges/etc. are suppressed now.{x\n\r",ch);
     }

     return;
  }

  if(!str_prefix(arg,"object"))
  {
     if(IS_SET(ch->pcdata->suppress,SUPPRESS_BURN))
     {
        REMOVE_BIT(ch->pcdata->suppress,SUPPRESS_BURN);
        send_to_char("{cObject effects (burning potions, etc.) are no longer suppressed.{x\n\r",ch);
     }
 
     else
     {
        SET_BIT(ch->pcdata->suppress,SUPPRESS_BURN);
        send_to_char("{cObject effects (burning potions, etc.) are suppressed now.{x\n\r",ch);
     }

     return;
  }

  if(!str_prefix(arg,"shields"))
  {
     if(IS_SET(ch->pcdata->suppress,SUPPRESS_SHIELD))
     {
        REMOVE_BIT(ch->pcdata->suppress,SUPPRESS_SHIELD);
        SET_BIT(ch->pcdata->suppress,SUPPRESS_SHIELD_COMP);
        send_to_char("{cShields are no longer compressed into one line.{x\n\r",ch);
     }

     else
     {
        SET_BIT(ch->pcdata->suppress,SUPPRESS_SHIELD);
        REMOVE_BIT(ch->pcdata->suppress,SUPPRESS_SHIELD_COMP);
        send_to_char("{cShields are compressed into one line now.{x\n\r",ch);
     }

     return;
  }

  send_to_char("{cSyntax{D: {xsuppress {cweapon{D/{cflags{D/{cdodge{D/{cobject{D/{cshields{x\n\r",ch);
  return;

}

void do_away( CHAR_DATA *ch, char *argument )
{

  if ( IS_NPC( ch ) )
  {
      send_to_char( "I'm sorry mobs don't have away messages.\n\r", ch );
      return;
  }

  printf_to_char( ch, "Your current away message is: %s", ch->pcdata->afk );
  return;

}

void do_click( CHAR_DATA *ch, char *argument )
{
     CHAR_DATA *victim;
     char cmdbuf[MSL];

     if ( ( victim = get_char_room ( ch, NULL, argument ) ) != NULL )
     {
          printf_to_char( ch, "\n\r[%s] Options: ", victim->short_descr );
          SNP ( cmdbuf, "look %s", victim->name );
          printf_to_char( ch, "%s ", click_cmd ( ch->desc, "(Look At) ", cmdbuf, "Look more closely" ) );

          if ( IS_NPC ( victim ) ) // NPC Specific
          {     
               if ( victim->spec_fun == spec_lookup ( "spec_questmaster" ) )
               {
                   printf_to_char( ch, "%s ", click_cmd ( ch->desc, "(Quest Request)", "quest request", "Ask for a quest" ) );
                   printf_to_char( ch, "%s ", click_cmd ( ch->desc, "(Quest Complete)", "quest complete", "Complete a quest" ) );
                   printf_to_char( ch, "%s ", click_cmd ( ch->desc, "(Quest List)", "quest list", "List Quest Items" ) );
               }
               if ( victim->pIndexData->pShop != NULL )
               {
                   printf_to_char( ch, "%s ", click_cmd ( ch->desc, "(Shop) ", "list", "List items for sale" ) );
               }               

               if ( IS_SET( victim->act, ACT_IS_HEALER ) )
               {
                   printf_to_char( ch, "%s ", click_cmd( ch->desc, "(Healer)", "heal", "List the heals he can perform" ) );
               }
          }
          else 						// Player Specific
          {
               SNP ( cmdbuf, "whois %s", victim->name );
               printf_to_char( ch, "%s ", click_cmd ( ch->desc, "(Whois)", cmdbuf, "View whois info" ) );
               SNP ( cmdbuf, "follow %s", victim->name );
               printf_to_char( ch, "%s ", click_cmd ( ch->desc, "(Follow)", cmdbuf, "Follow this person" ) );
          }
          if ( !is_safe ( ch, victim ) ) // FALSE here disables 
          {
               SNP ( cmdbuf, "consider %s", victim->name );
               printf_to_char( ch, "%s ", click_cmd ( ch->desc, "(Consider)", cmdbuf, "Consider potential enemy" ) );
               SNP ( cmdbuf, "kill %s", victim->name );
               printf_to_char( ch, "%s ", click_cmd ( ch->desc, "(Kill)", cmdbuf, "Kill them!" ) );
          }
          send_to_char ( "\n\r", ch );
     }
     else
     {
          send_to_char ( "\n\rTarget no longer valid.\n\r", ch );
     }
}

