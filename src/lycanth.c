/*This is lycanth.c, which covers not only lycanthropy but also vampirism,
   both being portrayed as transmittable afflictions on A Dark Portal. This was
   undertaken on Nov. 14, 1999 by Noximist, and has not yet been completed to 
   her satisfaction.
  */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "include.h"


void do_bite( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *target;
  char buf[MSL];
  char arg[MSL];

  argument = one_argument( argument, arg );

  if ( !IS_VAMPIRE( ch ) )
  {
	send_to_char( "You had best leave the vampiric work to those of the kindred!\n\r", ch );
	return;
  }

  if ( arg[0] == '\0' )
  {
	send_to_char( "Whose flesh would your fangs like to meet?\n\r", ch );
	return;
  }

  if ( ( target = get_char_room( ch,NULL, arg ) ) == NULL )
  {
	sprintf( buf, "%s is nowhere around.\n\r", arg );
	send_to_char( buf, ch );
	return;
  }

  if( target->iswere )
  {
	send_to_char("You cannot do that to them.\n\r",ch);
	return;
  }

  if ( IS_VAMPIRE( target ) )
  {
	send_to_char( "They are already your vampiric brethren!\n\r", ch );
	return;
  }	
 
  if ( ch->bloodthirst > 30 && !IS_IMMORTAL(ch) )
  {
	send_to_char( "Your bloodthirst is already sated.\n\r", ch );
	return;
  }

  if ( target->level < ch->level - 10 )
  {
	sprintf( buf, "{rThe blood of {W%s {rwould not sate you.{x\n\r", PERS(target,ch) );
      send_to_char( buf, ch );
	return;
  }

  if ( !IS_NPC( target ) && target->bites >= 3 )
  {
	send_to_char( "They have already received the three bites - now you need only turn them!\n\r", ch );
	return;
  }
 

  if ( IS_SET( target->in_room->room_flags, ROOM_SAFE ) )
  {
	send_to_char( "The Gods protect from your advances in this place of sanctuary.\n\r", ch );
	sprintf( buf, "%s flashes two gleaming fangs, but the Gods stop any actions against you.\n\r", PERS(ch, target) );
	send_to_char( buf, target );
	return;
  }	

  if ( !IS_NPC( target ) && target->level < 10 )
  {
	sprintf( buf, "%s is as yet too young to be turned.\n\r", PERS(target,ch)  );
	send_to_char( buf, ch );
	sprintf( buf, "%s bares two gleaming fangs menacingly, but your youth protects you.\n\r",PERS( target,ch ) );
	send_to_char( buf, target );
	return;
  }

  sprintf( buf, "{rDrawing {W%s {rnear, you sink your fangs deep into %s throat, tasting the iron-tinged\n\rdroplets of blood as they flow into your mouth!{x\n\r", PERS(target,ch), target->sex == 1 ? "his" : target->sex == 2 ? "her" : "its" );	
  send_to_char( buf, ch );
  sprintf( buf, "{W%s {rdraws you near, and sinks %s fangs deep into your throat! As your blood\n\rflows into %s mouth, the power of the kindred asserts itself.{x\n\r", PERS(ch, target), ch->sex == 1 ? "his" : ch->sex == 2 ? "her" : "its", ch->sex == 1 ? "his" : ch->sex == 2 ? "her" : "its" );
  send_to_char( buf, target );

  target->bites++;
  ch->bloodthirst += 10;

  if ( !IS_NPC( target ) )
  {
    switch ( target->bites )
    {
	case 1:   send_to_char( "\n\r{rThe first small cravings for blood begin to touch you, but vampirism has not set in.\n\r", target );
		    sprintf( buf, "\n\r{W%s {rshudders and some colour drains from %s face, but vampirism has not set in.{x\n\r", PERS( target,ch), target->sex == 1 ? "his" : target->sex == 2 ? "her" : "its" );
		    send_to_char( buf, ch );
		    break;
      case 2:   send_to_char( "\n\r{rThe cravings for blood within you strengthen and the light becomes blinding,\n\rbut vampirism has not set in.\n\r", target );
		    sprintf( buf, "\n\r{W%s {rbegins to look paler, and cringes at the light, but vampirism has not set in.{x\n\r", PERS(target,ch) );
		    send_to_char( buf, ch );
		    break;
	case 3:   send_to_char( "\n\r{rThe cravings for blood within you begin to scream and the light blinds you totally,\n\rvampirism beginning to flow within you.\n\r", target );
		    sprintf( buf, "\n\r{W%s screams at the light as vampirism begins to take hold.\n\rNow you need only turn %s, and %s will be changed entirely!{x\n\r", PERS(target,ch), target->sex == 1 ? "him" : target->sex == 2 ? "her" : "it", target->sex == 1 ? "he" : target->sex == 2 ? "she" : "it" );
		    send_to_char( buf, ch );
		    break;
	default:  send_to_char( "\n\r{rAn error has occurred - tell an imm!{x\n\r", ch );
		    send_to_char( "\n\r{rAn error has occurred - tell an imm!{x\n\r", target );
		    break;
    }	
  }
	
  WAIT_STATE( ch, PULSE_PER_SECOND * 8 );

  return;
}		 	

void do_turn( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *target;
  char buf[MSL], arg[MSL];

  argument = one_argument( argument, arg );

  if ( !IS_VAMPIRE( ch ) )
  {
	send_to_char( "You had best leave the vampiric work to those of the kindred!\n\r", ch );
	return;
  }

  if ( arg[0] == '\0' )
  {
	send_to_char( "Whom would you like to turn?\n\r", ch );
	return;
  }

  if ( ( target = get_char_room( ch,NULL, arg ) ) == NULL )
  {
	sprintf( buf, "%s is nowhere around.\n\r", arg );
	send_to_char( buf, ch );
	return;
  }

  if ( IS_VAMPIRE( target ) )
  {
	send_to_char( "They are already your vampiric brethren!\n\r", ch );
	return;
  }	
 
  if ( target->bites < 3 )
  {
	send_to_char( "They have not yet received the three bites - you cannot yet turn them!\n\r", ch );
	return;
  }

  if ( IS_SET( target->in_room->room_flags, ROOM_SAFE ) )
  {
	send_to_char( "The Gods protect from your advances in this place of sanctuary.\n\r", ch );
	sprintf( buf, "%s looks dark for a moment, but the Gods stop any actions against you.\n\r", ch->name );
	send_to_char( buf, target );
	return;
  }	

  if ( target->level < 10 )
  {
	sprintf( buf, "%s is as yet too young to be turned.\n\r", target->name );
	send_to_char( buf, ch );
	sprintf( buf, "%s gazes at you dangerously, but your youth protects you.\n\r", ch->name );
	send_to_char( buf, target );
	return;
  }

  sprintf( buf, "{rYou draw {W%s {rlovingly to your side and gaze into %s eyes, then whisper\n\rinto %s ear. %s starts, then grins, as the blood of the kindred fills %s veins!{x\n\r", target->name, 
		target->sex == 1 ? "his" : target->sex == 2 ? "her" : "its", target->sex == 1 ? "his" : target->sex == 2 ? "her" : "its", target->sex == 1 ? "He" : target->sex == 2 ? "She" : "It", target->sex == 1 ? "his" : target->sex == 2 ? "her" : "its" );   
  send_to_char( buf, ch );
  sprintf( buf, "{W%s {rdraws you lovingly to %s side and gazes into your eyes, then whisper\n\rinto your ear. A burning wells up inside you as the blood of the kindred\n\r fills your veins!{x\n\r", ch->name, ch->sex == 1 ? "his" : ch->sex == 2 ? "her" : "its" );
  send_to_char( buf, target );
  send_to_char( "\n\r{rYou are now one of the kindred, a vampire! Watch out for that sunshine, and\n\rhunt down a priest if this affliction begins to destroy you, as it usually does...{x\n\r", target );  

  target->bites = 0;
  target->isvamp = TRUE;
  target->bloodthirst = 10;
 
  return;
}

void do_cure( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *priest;
    char buf[MAX_STRING_LENGTH];	
    AFFECT_DATA af;   

    if ( IS_NPC(ch) )
	return;

    for ( priest = ch->in_room->people; priest; priest = priest->next_in_room )
    {
	if ( IS_NPC(priest) && IS_SET(priest->act, ACT_IS_HEALER) )
	    break;
    }

    if ( !IS_VAMPIRE( ch ) )
    {
	send_to_char( "{DYou are not in need of such services - the kindred blood\n\rdoes not taint your veins.{x\n\r", ch );
	return;
    }

    if ( priest == NULL )
    {
	send_to_char( "{DYou concentrate on the expulsion of the sinister blood from your veins,\n\rbut without pious aid, you fail.{x\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{D%s gazes at you intently. '{W%s, this procedure will cause you\n\rmuch agony, but will cure you. If you are sure you are ready, type \"cure now\".{D'{x\n\r", capitalize( priest->short_descr ), ch->name );
	send_to_char( buf, ch );
      return;
    }

    if ( !str_cmp( "now", argument ) )    
    {
	sprintf( buf, "{D%s nods. '{WVery well, %s - I hope you find this worthwhile.{D' %s begins to chant softly.{x\n\r", capitalize( priest->short_descr ), ch->name, priest->sex == 1 ? "He" : priest->sex == 2 ? "She" : "It" );
	send_to_char( buf, ch );
	WAIT_STATE( ch, PULSE_PER_SECOND * 5 );
	send_to_char( "\n\r{DA strange pain, like a far-off burning, begins to well up within you, as\n\rif flowing through your bloodstream. As time passes, it grows more intense.{x\n\r", ch );
	WAIT_STATE( ch, PULSE_PER_SECOND * 5 );
	send_to_char( "\n\r{DSuddenly, the pain bursts into an unbearable magnitude, burning your insides as\n\rit screams through your body! The priest watches anxiously.{x\n\r", ch );
	WAIT_STATE( ch, PULSE_PER_SECOND * 5 );
	sprintf( buf, "\n\r{DAfter a few moments, the pain recedes, leaving your body purged of the\n\rkindred blood, but weak and unseeing from the force.\n\r%s nods, and says, '{WIt is done. Your sight will return after a few hours.{D'{x\n\r", capitalize( priest->short_descr ) );
      send_to_char( buf, ch );
	act("{D$n screams in agony as the blood of the kindred is purged from $s veins!{x",ch,NULL,NULL,TO_ROOM);

      ch->hit= 1;
      
      af.where     = TO_AFFECTS;
      af.type      = skill_lookup("blindness");
      af.level     = 100;
      af.location  = APPLY_HITROLL;
      af.modifier  = -4;
      af.duration  = number_range(10, 20);
      af.bitvector = AFF_BLIND;
      affect_to_char( ch, &af );

	ch->isvamp = FALSE;
	ch->bloodthirst = 0;

	return;
    }

    sprintf( buf, "{D%s looks confused. '{WWhat's that you say, %s?{D'{x\n\r", capitalize( priest->short_descr ), ch->name );
    send_to_char( buf, ch );
    return;
}
 

int moon_status( )
{
   int sunpos, moonpos, moonphase;

   sunpos  = (72 * (24 - time_info.hour) / 24);
   moonpos = (sunpos + time_info.day * 72 / 35) % 72;
   moonphase =  ((((72 + moonpos - sunpos ) % 72 ) + (72/16)) * 8 ) / 72;
   if(moonphase > 4)
      moonphase -= 8;
   if ( (time_info.hour >= 6 && time_info.hour <= 18)  /* daytime? */
      || (moonpos <= 72/4 - 2) 
	  || (moonpos >= 3*72/4 + 2)  ) /* in sky? */
      return MOON_GONE;

   switch (moonphase)
   {
		case -3: return MOON_THREE;
		case 3: return MOON_THREE;
		case -2: return MOON_TWO;
		case 2: return MOON_TWO;
		case -1: return MOON_ONE;
		case 1: return MOON_ONE;
		case 0: return MOON_GONE;
		case 4: return MOON_FULL;
		default: return moonphase;
   }
   return MOON_GONE;
}

void check_werewolf ()
{
    DESCRIPTOR_DATA *d;
    int moon, level, iWear, dam;
    AFFECT_DATA af;
    OBJ_DATA *pObj;
    level = 0;  
	for( d = descriptor_list ; d ; d = d->next )
	{
		CHAR_DATA *vch;
		vch = d->original ? d->original : d->character;
		if (d->connected != CON_PLAYING )
		    continue;
		if(!vch->iswere && !vch->turned)
		    continue;

		if( ( moon = moon_status() ) == MOON_GONE )
		{
			if(vch->wereform > moon)
				vch->wereform = moon;
			continue;
		}

		if(vch->turned && !vch->iswere )
		{
		     send_to_char("{DA darkness you've never known before pulsates through your body.\n\r",vch);
		     vch->iswere = TRUE;
		     vch->turned = FALSE;
		}
		if(vch->wereform < moon && !IS_IMMORTAL(vch))
  	        WAIT_STATE(vch, PULSE_EVENT * (3*moon) );
		if(moon >= MOON_ONE && vch->wereform < MOON_ONE)
		{
		    wait_act(0, "{DAs the clouds in your mind thicken, all one can think of is the anger, the hate.\n\rThe darkness begins to pierce every fragment of your soul. A change happens, though \n\rnot physically.Your psyche starts to descend from \"dark\" to \"darker.\" The \n\rstark rage in your eyes becomes apparent to all but the most blind. A fury swirls and \n\rswells inside your mind, preparing to consume you\n\r",vch, NULL, NULL, TO_CHAR); 
		    wait_act(0, "$n gets a wild look in $s eyes.",vch, NULL, NULL, TO_ROOM);
		    level = vch->level;
		}

		if(moon >= MOON_TWO && vch->wereform < MOON_TWO)
 	        {

			wait_act(3, "{DThe fury transforms to pure rage as you tense, throbbing, with a wild gleam to your eyes.\n\rAching gums tear as fangs force their presence. Your nails rip and pull, forming lethal\n\rclaws. Shaking from the metamorphosis, a new sensation threads through the pain and \n\ranger: strength.\n\r",vch, NULL, NULL, TO_CHAR);

		    level *= 2;
		}
	        if(moon >= MOON_THREE && vch->wereform < MOON_THREE)
		{
		    wait_act(6, "{DThick and dark patches of hair begin to grow upon your muscled flesh. Lips curling\n\rwith the agony, your jagged fangs glint with an unholy light. \"Unholy,\" however,\n\runderstates the dark storm of thoughts raging through your tormented mind. Murder, \n\rrape, pillaging, all is now possible with this overwhelming strength.\n\r",vch, NULL, NULL, TO_CHAR);
		    wait_act(6, "$n's hands slowly begin to lose form, and $s body is now completly covered with hair!",vch, NULL, NULL, TO_ROOM);
		    level *= 2;
		}
		if(moon == MOON_FULL && vch->wereform < MOON_FULL)
		{
			wait_act(9, "{DA final burst of searing pain consumes you, as bone and muscle shift and fold. Your body \n\rhinges forward as your legs twist and contort while your jaw thrusts out and force your teeth to extent,\n\rbecoming sharper and more deadly. With the gruesome snapping of bone and ligament, the\n\rtransformation is completed.\n\r",vch, NULL, NULL, TO_CHAR);
		    wait_act(9, "$n slowly begins to loose $s form. And is now turned into a Lycanthrope!.",vch, NULL, NULL, TO_ROOM);
		    level *= 2;
		    for(iWear = 0 ; iWear < MAX_WEAR ; iWear++)
		    {
				if ( ( pObj = get_eq_char( vch, iWear ) ) == NULL )
					continue;
				dam = vch->level/2;
				switch( pObj->wear_loc )
				{
					case WEAR_HOOD: 
							 unequip_char(vch, pObj);
							 damage_obj(vch, pObj, dam);
							 REMOVE_BIT(vch->mask,HOOD_RAISED);
							 SET_BIT(vch->mask,HOOD_REMOVED);
							 obj_from_char(pObj);
							 pObj->carried_by = vch;
							 obj_to_room(pObj, vch->in_room);
				 			 wait_printf(vch, 9,"%s is torn off, and falls to the ground.\n\r", pObj->short_descr );
							 break;					
					case WEAR_SHIELD:
							 unequip_char(vch, pObj);
							 obj_from_char(pObj);
							pObj->carried_by = vch; 
							obj_to_room(pObj, vch->in_room);
				 			 wait_printf(vch, 9, "As your hands loose form, you drop %s.\n\r", pObj->short_descr );
							 break;
					case WEAR_WIELD:
							 unequip_char(vch, pObj);
							 obj_from_char(pObj);
							pObj->carried_by = vch; 
							obj_to_room(pObj, vch->in_room);
				 			 wait_printf(vch, 9,"As your hands loose form, you drop %s.\n\r", pObj->short_descr );
							 break;
					case WEAR_SECONDARY:
							 unequip_char(vch, pObj);
							 obj_from_char(pObj);
							pObj->carried_by = vch; 
							obj_to_room(pObj, vch->in_room);
				 			 wait_printf(vch,9, "As your hands loose form, you drop %s.\n\r", pObj->short_descr );
							 break;
					case WEAR_HOLD:
							 unequip_char(vch, pObj);
							 obj_from_char(pObj);
							pObj->carried_by = vch;
							 obj_to_room(pObj, vch->in_room);
				 			 wait_printf(vch, 9,"As your hands loose form, you drop %s.\n\r", pObj->short_descr );
							 break;
					default: unequip_char(vch, pObj);
							 damage_obj(vch, pObj, dam);
							 obj_from_char(pObj);
							pObj->carried_by = vch;
							 obj_to_room(pObj, vch->in_room);
				 			 wait_printf(vch, 9,"%s is torn off, and falls to the ground.\n\r", pObj->short_descr );
							 break;
				}
			
			}
		    	af.where     = TO_AFFECTS;
			af.type      = skill_lookup("haste");
   			af.level     = vch->level;
		        af.duration  = 11;
		    	af.location  = APPLY_DEX;
    			af.modifier  = 5;
    			af.bitvector = AFF_HASTE;
    			affect_to_char( vch, &af );

		}

		if(level > 0 )
		{
			af.where     = TO_AFFECTS;
			af.type      = skill_lookup("frenzy");
			af.level     = vch->level;
			af.duration  = 11;
			af.modifier  = level / 5;
			af.bitvector = 0;
			af.location  = APPLY_HITROLL;
			affect_to_char(vch, &af);
			af.location  = APPLY_DAMROLL;
			affect_to_char(vch, &af);
		}
        vch->wereform = moon;
	}
}
