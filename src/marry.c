#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "const.h"

void make_ring args ( ( CHAR_DATA *victim, CHAR_DATA *victim2 ) );
DECLARE_DO_FUN( do_look );

void do_marry( CHAR_DATA *ch, char *argument)
{
    char arg1[MIL];
    char arg2[MIL];
    CHAR_DATA *victim;
    CHAR_DATA *victim2;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
       send_to_char("{RSyntax: {Wmarry {M<char1> <char2>{x\n\r",ch);
       return;
    }

    if (!strcmp(arg1, arg2))
    {
        send_to_char("{RYou can not marry anyone to themselves!{x", ch);
	return;
    }

    if ( ((victim = get_char_world(ch,arg1)) == NULL) ||
       ((victim2 = get_char_world(ch,arg2)) == NULL))
    {
       send_to_char("{RBoth characters must be playing!{x\n\r", ch );
       return;
    }

    if ( IS_NPC(victim) || IS_NPC(victim2))
    {
       send_to_char("{RSorry! Mobs can't get married!{x\n\r", ch);
       return;
    }

    if (victim->level < MIN_LEVEL_MARRY || victim2->level < MIN_LEVEL_MARRY)
    {
       send_to_char("{RThey are not of the proper level to marry.{x\n\r", ch);
       return;
    }

    if (victim->pcdata->spouse[0] == '\0' && victim2->pcdata->spouse[0] == '\0')
    {
       printf_to_char( ch, "{GYou now pronounce %s and %s husband and wife.\n\r", victim->name, victim2->name );
       send_to_char("{GYou say 'I do.'{x\n\r", victim);
       send_to_char("{GYou say 'I do.'{x\n\r", victim2);
       act("{W$n {Gand {W$N are now married to each other!{x\n\r", victim, NULL, victim2, TO_NOTVICT);
       info( ch, 0, "{G[INFO]: {R%s and %s are now married to each other.{x\n\r", victim->name, victim2->name );
       victim->pcdata->spouse = str_dup(victim2->name);
       victim2->pcdata->spouse = str_dup(victim->name);
       make_ring( victim, victim2 );
       return;
     }

     else
     {
        send_to_char("{ROne or the other are already wedded!{x\n\r", ch);
        return;
     }

     return;
}

void do_divorce( CHAR_DATA *ch, char *argument)
{
    char arg1[MIL];
    char arg2[MIL];
    CHAR_DATA *victim;
    CHAR_DATA *victim2;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
       send_to_char("{RSyntax: {Wdivorce {M<char1> <char2>{x\n\r",ch);
       return;
    }

    if ( ((victim = get_char_world(ch,arg1)) == NULL) ||
         ((victim2 = get_char_world(ch,arg2)) == NULL))
    {
       send_to_char("{RBoth characters must be playing!{x\n\r", ch );
       return;
    }

    if ( IS_NPC(victim) || IS_NPC(victim2))
    {
       send_to_char("{RI don't think they're Married to the Mob!{x\n\r", ch);
       return;
    }

   if ( !str_cmp( victim->pcdata->spouse, victim2->name ) && !str_cmp( victim2->pcdata->spouse, victim->name ) )
   {
       printf_to_char( ch, "{RYou give %s and %s their divorce decree.{x\n\r", victim->name, victim2->name );
       printf_to_char( victim, "{RYour divorce from %s is final.{x\n\r", victim2->name );
       printf_to_char( victim2, "{RYour divorce from %s is final.{x\n\r", victim->name );
       info( ch, 0, "{G[INFO]: {R%s and %s have decided to get a divorce.\n\r", victim->name, victim2->name );
       act("{W$n {Gand {W$N {Gswap divorce decrees, they are no-longer married.{x",victim, NULL, victim2, TO_NOTVICT);
       free_string( victim->pcdata->spouse  );
       free_string( victim2->pcdata->spouse );
       victim->pcdata->spouse = str_dup( "" );
       victim2->pcdata->spouse = str_dup( "" );
       return;
   }

   else
   {
       send_to_char("{RThey aren't married, so how can they divorce!{x", ch);
       return;
   }

}

void make_ring( CHAR_DATA *victim, CHAR_DATA *victim2 )
{
    OBJ_DATA * ring;
    OBJ_DATA * ring2;

    ring = create_object( get_obj_index( OBJ_VNUM_DIAMOND_RING ), 0 );
    ring2 = create_object( get_obj_index( OBJ_VNUM_WEDDING_BAND ), 0 );

    switch( victim->sex )
    {
	   case SEX_FEMALE:
	   {
              obj_to_char( ring, victim ); 
              break;
	   }
	
           case SEX_MALE:
	   {
              obj_to_char( ring2, victim ); 
              break;
	   }
	
           case SEX_NEUTRAL:
	   default:
	   {
              obj_to_char( ring2, victim );   
	      break;
	   }
    }

    switch( victim2->sex )
    {
           case SEX_FEMALE:
	   {
               obj_to_char( ring, victim2 );
	       break;
	   }
		
           case SEX_MALE:
	   {
               obj_to_char( ring2, victim2 );
	       break;
	   }
            
	   case SEX_NEUTRAL:
	   default:
	   {
               obj_to_char( ring2, victim2 );
	       break;
	   }
    }
 
    return;
}

void do_spousetalk( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *spouse;
   char buf[MAX_STRING_LENGTH];

   if (argument[0] == '\0' )
   {
      send_to_char("What do you wish to tell your other half?\n\r", ch);
      return;
   }

   else  
   {
      if ( ( spouse = get_char_world(ch, ch->pcdata->spouse ) ) )
      {
          sprintf(buf, "You say to %s, '%s'\n\r", ch->pcdata->spouse, argument );
          send_to_char(buf, ch);
          sprintf(buf, "%s says to you '%s'\n\r", ch->name, argument );
          send_to_char(buf, spouse);
      }

      else
         send_to_char("Your spouse doesn't seem to be here!", ch);
   }
}

void do_consent( CHAR_DATA *ch )
{
   if (IS_NPC(ch))
       return;
    
   if ( IS_SET(ch->act, PLR_CONSENT) )
   {
       send_to_char( "You no longer give consent.\n\r", ch);
       REMOVE_BIT(ch->act, PLR_CONSENT);
       return;
   }
                           
   send_to_char( "You now give consent to Married!\n\r", ch);
   SET_BIT(ch->act, PLR_CONSENT);
   return;
}

void do_spgate( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int cost = 40;

    if (!IS_AWAKE( ch ) )
    {
	send_to_char( "In your dreams or what?\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
    {
	send_to_char( "Not while you are fighting.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->pcdata->spouse[0] == '\0' )
    {
	send_to_char( "But you are not married!\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, ch->pcdata->spouse ) ) == NULL )
    {
	sprintf( buf, "%s is not connected.\n\r", victim->pcdata->spouse );
	send_to_char( buf, ch );
	return;
    }

    if ( IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) ||
	 IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) ||
	 IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) ||
	 IS_SET( victim->in_room->room_flags, ROOM_GODS_ONLY ) ||
	 IS_SET( victim->in_room->room_flags, ROOM_IMP_ONLY ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }
   
    if ( ch->mana > cost )
        ch->mana -= cost;    
    
    else
    {
        printf_to_char( ch, "You need at least %d mana to spouse gate.\n\r", cost );
        return;
    }

    act( "$n steps into a rose-colored gate and disappears.", ch, NULL, NULL, TO_ROOM );

    char_from_room( ch );
    char_to_room( ch, victim->in_room );
    act( "$n steps into the room from a rose-colored gate.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
}

