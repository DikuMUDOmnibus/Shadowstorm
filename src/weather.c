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

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "interp.h"
#include "db.h"
#include "magic.h"
#include "const.h"

void    lightning        args( ( void ) );
void    ice	         args( ( CHAR_DATA *ch ) );
void    hail	         args( ( void ) );
void    blizzard         args( ( CHAR_DATA *ch ) );
void    fog              args( ( CHAR_DATA *ch ) );
void    tornado          args( ( void ) );
void    weather_update   args( ( void ) );
extern char *target_name;

/*
 * Update the weather.
 */
void weather_update( void )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    buf[0] = '\0';

    switch ( ++time_info.hour )
    {
    case  5:
	weather_info.sunlight = SUN_LIGHT;
	strcat( buf, "The {yday{x has begun.\n\r" );
	break;

    case  6:
	weather_info.sunlight = SUN_RISE;
	strcat( buf, "The {Ysun{x rises in the east.\n\r" );
	break;

    case 19:
	weather_info.sunlight = SUN_SET;
	strcat( buf, "The {Ysun{x slowly disappears in the west.\n\r" );
	break;

    case 20:
	weather_info.sunlight = SUN_DARK;
	strcat( buf, "The {Dnight{x has begun.\n\r" );
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	break;

    }

    if ( time_info.day   >= 35 )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 12 )
    {
	time_info.month = 0;
	time_info.year++;
    }


    switch ( weather_info.sky )
    {
        default: 
	    weather_info.sky = SKY_CLOUDLESS;
	    break;

        case SKY_CLOUDLESS:
            if ((number_chance(10)) && (time_info.hour <= 6) )
            {
	        weather_info.sky = SKY_CLOUDLESS;
            }

            else if (number_chance(15))
            {
                weather_info.sky = SKY_FOGGY;
            }   
          
            else if (number_chance(15))
            {
	        weather_info.sky = SKY_CLOUDY;
            }

            else if (number_chance(20))
            {
	        weather_info.sky = SKY_RAINING;
            }
      
            else if (number_chance(25))
            {
	        weather_info.sky = SKY_RAINING;
            }

            else if (number_chance(45))
            {
                weather_info.sky = SKY_CLOUDLESS;
            }

	    break;
	
    case SKY_CLOUDY:
            if (number_chance(15))
            {
                weather_info.sky = SKY_CLOUDY;
            }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_SNOWING;
	    }
	
            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_HAILSTORM;
	    }
	
            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_THUNDERSTORM;
	    }

	    else if (number_chance(15))
	    {
	        weather_info.sky = SKY_ICESTORM;
	    }      
            
            else if (number_chance(20))
	    {
	        weather_info.sky = SKY_CLOUDLESS;
	    }

            else if (number_chance(25))
	    {
	        weather_info.sky = SKY_CLOUDY;
	    }

            break;

    case SKY_BREEZE:
            if (number_chance(10))
            {
                weather_info.sky = SKY_BREEZE;
            }

            else if (number_chance(15))
            {
                weather_info.sky = SKY_CLOUDY;
            }
            
            else if (number_chance(15))
            {
                weather_info.sky = SKY_THUNDERSTORM;
            }

            else if (number_chance(15))
            {
                weather_info.sky = SKY_CLOUDLESS;
            }

            else if (number_chance(55))
            {
                weather_info.sky = SKY_RAINING;
            }
        
            break;

    case SKY_WIND:
            if (number_chance(10))
            {
                weather_info.sky = SKY_WIND;
            }
      
            else if (number_chance(15))
            {
                weather_info.sky = SKY_CLOUDY;
            }
            
            else if (number_chance(15))
            {
                weather_info.sky = SKY_THUNDERSTORM;
                tornado();
            }

            else if (number_chance(15))
            {
                weather_info.sky = SKY_RAINING;
            }

            else if (number_chance(20))
            {
                weather_info.sky = SKY_WIND;
                tornado();
            }

            else if (number_chance(25))
            {
                weather_info.sky = SKY_HAILSTORM;
                tornado();
	        hail();
            }
            
            else if (number_chance(55))
            {
                weather_info.sky = SKY_BLIZZARD;
            }

            break;

    case SKY_RAINING:
            if ( number_chance( 10 ) )
            {
                weather_info.sky = SKY_RAINING;
            }

            else if (number_chance(10))
            {
                weather_info.sky = SKY_CLOUDY;
            }
      
            else if (number_chance(15))
            {
                weather_info.sky = SKY_LIGHTNING;
                lightning( );
            }

            else if (number_chance(10))
            {
                weather_info.sky = SKY_THUNDERSTORM;
            }
            
            else if (number_chance(20))
            {
                weather_info.sky = SKY_HAILSTORM;
            }

            else if (number_chance(55))
            {
                weather_info.sky = SKY_RAINING;
            }
        
            break;

    case SKY_SNOWING:
       	    if (number_chance(10))
	    {
	        weather_info.sky = SKY_SNOWING;
	    }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_BLIZZARD;
	    }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_CLOUDY; 
	    }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_RAINING;
	    }

       	    else if (number_chance(50))
	    {
	        weather_info.sky = SKY_SNOWING;
	    }

	    break;

    case SKY_LIGHTNING:
            if (number_chance(10))
	    {
	        weather_info.sky = SKY_LIGHTNING;
            }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_CLOUDY;
	    }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_THUNDERSTORM;
            }

            else if (number_chance(15))
	    {  
	        weather_info.sky = SKY_RAINING;
	    }

            else if (number_chance(40))
	    {
	        weather_info.sky = SKY_LIGHTNING;
            }

	    break;

    case SKY_FOGGY:
            if (number_chance(10))
	    {
                weather_info.sky = SKY_FOGGY;
 	        break;
	    }

            else if (number_chance(20))
	    {
                weather_info.sky = SKY_CLOUDY;
            }

            else if (number_chance(30))
            {
                weather_info.sky = SKY_RAINING;
            }

            else if (number_chance(40))
	    {
                weather_info.sky = SKY_FOGGY;
 	        break;
	    }

    case SKY_THUNDERSTORM:
            if (number_chance(10))
	    {
	        weather_info.sky = SKY_THUNDERSTORM;
            }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_CLOUDY;
	    }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_RAINING;
            }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_LIGHTNING;
	    }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_HAILSTORM;
                hail ();    
            }

            else if (number_chance(40))
	    {
	        weather_info.sky = SKY_THUNDERSTORM;
            }

  	    break;

    case SKY_HAILSTORM:
            if (number_chance(15))
	    {
	        weather_info.sky = SKY_HAILSTORM;
	        hail();
	    }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_CLOUDY;
	    }

            else if (number_chance(30))
	    {
	        weather_info.sky = SKY_RAINING;
            }

            else if (number_chance(50))
	    {
	        weather_info.sky = SKY_HAILSTORM;
	        hail();
	    }

  	    break;

    case SKY_ICESTORM:
            if (number_chance(10))
	    {
                weather_info.sky = SKY_ICESTORM;
            }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_CLOUDY;
            }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_SNOWING;
	    }
     
            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_BLIZZARD;
            }

            else if (number_chance(55))
	    {
                weather_info.sky = SKY_ICESTORM;
            }
	
            break;

    case SKY_BLIZZARD:
            if (number_chance(10))
	    {
	        weather_info.sky = SKY_BLIZZARD;
	    }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_CLOUDY;
	    }

            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_SNOWING;
	    }
        
            else if (number_chance(15))
	    {
	        weather_info.sky = SKY_ICESTORM;
	    }

            else if (number_chance(55))
	    {
	        weather_info.sky = SKY_BLIZZARD;
	    }

            break; 
    }

    if ( buf[0] != '\0' )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   IS_OUTDOORS(d->character)
	    &&   IS_AWAKE(d->character) )
		send_to_char( buf, d->character );
	}
    }

    return;
}

void do_weather( CHAR_DATA *ch, char *argument )
{
    char buf2[MSL];
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *in_room;
    char *suf;
    int day;
    int message;

    in_room = ch->in_room;
    pArea = ch->in_room->area;

    if ( in_room == NULL || pArea == NULL )
        bug( "PArea or in_room was null for do_weather", 0 );

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";


    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the weather indoors.\n\r", ch );
	return;
    }
        
    send_to_char("{x\n\r",ch); 

    if (weather_info.sky == SKY_RAINING)
    {
        message = number_range( 0, MAX_RAIN -1 );
        printf_to_char( ch, "\n\r{B[{WWeather{B] {c%s{x\n\r",
        rain_table[message].description );
        rnd_sound( 2, ch, "RAIN3.wav", "RAIN4.wav" );
    }
    
    else if (weather_info.sky == SKY_CLOUDY)
    {
        message = number_range( 0, MAX_CLOUD -1 );
        printf_to_char( ch, "\n\r{B[{WWeather{B] {c%s{x\n\r", 
        cloud_table[message].description );
    }
                 
    else if (weather_info.sky == SKY_CLOUDLESS)
    {
        message = number_range( 0, MAX_CLOUDY -1 );
        printf_to_char( ch, "\n\r{B[{WWeather{B] {c%s{x\n\r",
        cloudy_table[message].description );
    }
                
    else if (weather_info.sky == SKY_THUNDERSTORM)
    {
        message = number_range( 0, MAX_THUNDERSTORM -1 );
        printf_to_char( ch, "\n\r{B[{WWeather{B] {c%s{x\n\r",
        thunderstorm_table[message].description );
    }
                 
    else if (weather_info.sky == SKY_ICESTORM)
    {
        message = number_range( 0, MAX_ICESTORM -1 );
        printf_to_char( ch, "\n\r{B[{WWeather{B] {c%s{x\n\r",
        icestorm_table[message].description );
    }
                
    else if (weather_info.sky == SKY_HAILSTORM)
    {
        message = number_range( 0, MAX_HAILSTORM -1 );
        printf_to_char( ch, "\n\r{B[{WWeather{B] {c%s{x\n\r",
        hailstorm_table[message].description );
    }
                
    else if (weather_info.sky == SKY_SNOWING)
    {
        message = number_range( 0, MAX_SNOW -1 );
        printf_to_char( ch, "\n\r{B[{WWeather{B] {c%s{x\n\r",
        snow_table[message].description );
    }
      
    else if (weather_info.sky == SKY_BLIZZARD)
    {
        message = number_range( 0, MAX_BLIZZARD -1 );
        printf_to_char( ch, "\n\r{B[{WWeather{B] {c%s{x\n\r",
        blizzard_table[message].description );
    }
         
    else if (weather_info.sky == SKY_FOGGY)
    {
        message = number_range( 0, MAX_FOGGY -1 );
        printf_to_char( ch, "\n\r{B[{WWeather{B] {c%s{x\n\r",
        foggy_table[message].description );
    }

    else if (weather_info.sky == SKY_LIGHTNING)
    {
        message = number_range( 0, MAX_LIGHTNING -1 );
        printf_to_char( ch, "\n\r{B[{WWeather{B] {c%s{x\n\r",
        lightning_table[message].description );
    }

    else if (weather_info.sky == SKY_BREEZE)
    {
        message = number_range( 0, MAX_BREEZE -1 );
        printf_to_char( ch, "\n\r{B[{WWeather{B] {c%s{x\n\r",
        breeze_table[message].description );
    }

    else if (weather_info.sky == SKY_WIND)
    {
        message = number_range( 0, MAX_WIND -1 );
        printf_to_char( ch, "\n\r{B[{WWeather{B] {c%s{x\n\r",
        wind_table[message].description );
    }

    sprintf( buf2,
    "{B[{W Time  {B] {cIt is %d o'clock %s{x\n\r",
    (time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
    time_info.hour >= 12 ? "pm" : "am");
    send_to_char( buf2, ch );
    printf_to_char( ch, "{B[ {WTemp {B ] {cIt is currently %d degrees farenheit!{x\n\r", pArea->temperature );
    return;

} 

bool IS_OUTDOORS( CHAR_DATA *ch )
{
    if(!IS_NPC(ch) && ch->desc->connected != CON_PLAYING )
       return FALSE;

    if(!ch->in_room)
    {	
       logf2("[*BUG*] In_room null! IS_OUTDOORS");
       return FALSE;
    }

    if(!IS_NPC(ch) && IN_MINE(ch) )
       return FALSE;

    if( !IS_SET(ch->in_room->room_flags, ROOM_INDOORS) &&
       ch->in_room->sector_type != SECT_INSIDE )
       return TRUE;

    else
       return FALSE;
}

void do_autoweather(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->act,PLR_AUTOWEATHER))
    {
        send_to_char("You will no longer see weather descriptions in rooms\n\r",ch);
        REMOVE_BIT(ch->act,PLR_AUTOWEATHER);
    }

    else
    {
        send_to_char("You will now see weather descriptions in rooms\n\r",ch);
        SET_BIT(ch->act,PLR_AUTOWEATHER);
    }
}

void show_weather(CHAR_DATA *ch)
{
    int message;

    if (weather_info.sky == SKY_RAINING)
    {
        message = number_range( 0, MAX_RAIN -1 );
        printf_to_char( ch, "\n\r{g[{YWeather{g] {C%s{x\n\r\n\r",
        rain_table[message].description );
    }
        
    else if (weather_info.sky == SKY_CLOUDY)
    {
        message = number_range( 0, MAX_CLOUD -1 );
        printf_to_char( ch, "\n\r{g[{YWeather{g] {C%s{x\n\r\n\r",
                 cloud_table[message].description );
    }
            
    else if (weather_info.sky == SKY_CLOUDLESS)
    {
        message = number_range( 0, MAX_CLOUDY -1 );
        printf_to_char( ch, "\n\r{g[{YWeather{g] {C%s{x\n\r\n\r",
                 cloudy_table[message].description );
    }
                 
    else if (weather_info.sky == SKY_THUNDERSTORM)
    {
        message = number_range( 0, MAX_THUNDERSTORM -1 );
        printf_to_char( ch, "\n\r{g[{YWeather{g] {C%s{x\n\r\n\r",
                 thunderstorm_table[message].description );
    }
                 
    else if (weather_info.sky == SKY_ICESTORM)
    {
        message = number_range( 0, MAX_ICESTORM -1 );
        printf_to_char( ch, "\n\r{g[{YWeather{g] {C%s{x\n\r\n\r", 
        icestorm_table[message].description );
    }
                
    else if (weather_info.sky == SKY_HAILSTORM)
    {
        message = number_range( 0, MAX_HAILSTORM -1 );
        printf_to_char( ch, "\n\r{g[{YWeather{g] {C%s{x\n\r\n\r", 
        hailstorm_table[message].description );
    }
               
    else if (weather_info.sky == SKY_SNOWING)
    {
        message = number_range( 0, MAX_SNOW -1 );
        printf_to_char( ch, "\n\r{g[{YWeather{g] {C%s{x\n\r\n\r", 
        snow_table[message].description );
    }
           
    else if (weather_info.sky == SKY_BLIZZARD)
    {
        message = number_range( 0, MAX_BLIZZARD -1 );
        printf_to_char( ch, "\n\r{g[{YWeather{g] {C%s{x\n\r\n\r", 
        blizzard_table[message].description );
    }
               
    else if (weather_info.sky == SKY_FOGGY)
    {
        message = number_range( 0, MAX_FOGGY );
        printf_to_char( ch, "\n\r{g[{YWeather{g] {C%s{x\n\r\n\r", 
        foggy_table[message].description );
    }

    else if (weather_info.sky == SKY_LIGHTNING)
    {
        message = number_range( 0, MAX_LIGHTNING -1 );
        printf_to_char( ch, "\n\r{g[{YWeather{g] {C%s{x\n\r\n\r", 
        lightning_table[message].description );
    }

    else if (weather_info.sky == SKY_BREEZE)
    {
        message = number_range( 0, MAX_BREEZE -1 );
        printf_to_char( ch, "\n\r{g[{YWeather{g] {C%s{x\n\r\n\r",
        breeze_table[message].description );
    }

    else if (weather_info.sky == SKY_WIND)
    {
        message = number_range( 0, MAX_WIND -1 );
        printf_to_char( ch, "\n\r{g[{YWeather{g] {C%s{x\n\r",
        wind_table[message].description );
    }
      
    else 
    {
        send_to_char("{g[{YWeather{g] {CBUG!!!!!!!!!!!!!!! A description does not exist for this condition!{x\n\r",ch);
    }
}

void lightning( void )
{
  DESCRIPTOR_DATA *d;

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
       if ( d->connected == CON_PLAYING
           && IS_OUTDOORS( d->character )
           && IS_AWAKE  ( d->character )
           && number_chance(10)
           && !IS_IMMORTAL(d->character)
           && d->character->level > 17
           && weather_info.sky == SKY_LIGHTNING && d->character->fighting == NULL ) 
       {

           send_to_char("{x{RYou see a brilliant flash come down from the sky, and then black out!{x\n\r",d->character);
           act( "$n has been struck by lightning!", d->character, NULL, NULL,TO_ROOM);
        
           if(check_immune(d->character,DAM_LIGHTNING) != IS_IMMUNE) 
           {
              if(d->character->fighting) 
              {
                 stop_fighting(d->character,TRUE); 
              }
                 
              if(check_immune(d->character,DAM_LIGHTNING) != IS_RESISTANT)
              if (d->character->level < LI1);
              damage( d->character, d->character, number_range( 40, 80 ), 0, DAM_LIGHTNING, FALSE );
              if (d->character->level < LI2);
              damage( d->character, d->character, number_range( 75, 140 ), 0, DAM_LIGHTNING, FALSE );
              if (d->character->level < LI3);
              damage( d->character, d->character, number_range( 90, 180 ), 0, DAM_LIGHTNING, FALSE );
              WAIT_STATE(d->character,40); 
           } 
             
           else 
           {
              if(check_immune(d->character,DAM_LIGHTNING) == IS_VULNERABLE) 
              {
                 damage( d->character, d->character, number_range( 150, 250 ), 0, DAM_LIGHTNING, FALSE );
                 WAIT_STATE(d->character,40); 
              } 
           }
       } 
  }
}

void blizzard( CHAR_DATA *ch )
{
       if  ( IS_OUTDOORS( ch )
           && weather_info.sky == SKY_BLIZZARD ) 
       {

           if ( number_range( 0, 2000 ) <= 1000  )
           {
               send_to_char("{RThe sky before you is a mist of white blur. Perhaps you should find a safe place indoors.{x \n\r",ch);
           }

           else if ( number_range( 0, 2000 ) <= 1250  )
           {
               send_to_char("{RNot being able to see where you are going, you slip and fall into a hole.{x \n\r",ch );
               act( "$n falls face first into the oncoming drifts!", ch, NULL, NULL,TO_ROOM);
               damage( ch, ch, number_range( 15, 30 ), 0, DAM_BASH, FALSE );
           }

           else if ( number_range( 0, 2000 ) <= 1500  )
           {
               send_to_char("Not being able to see where your going you manage to run face first into a large rock! \n\r",ch);
               act( "$n has planted themselves face first into a huge rock!", ch, NULL, NULL,TO_ROOM);
               damage( ch, ch, number_range( 20, 40 ), 0, DAM_BASH, FALSE );
           } 

           else if ( number_range( 0, 2000 ) <= 1950  )
           {
               send_to_char("{RThe sky before you is a mist of white blur. Perhaps you should find a safe place indoors.{x \n\r",ch);
           }

           else if ( number_range( 0, 2000 ) <= 2000  )
           {   
               send_to_char("Your frozen limbs slowly begin to stop responding!{x \n\r", ch);
               act( "You watch as $n's limbs start to freeze !", ch, NULL, NULL,TO_ROOM);
               damage( ch, ch, number_range( 25, 50 ), 0, DAM_COLD, FALSE );
           }
       } 
}

void ice( CHAR_DATA *ch )
{

       if  ( IS_OUTDOORS( ch )
           && weather_info.sky == SKY_ICESTORM )
       {
           if ( number_range( 0, 2000 ) <= 1000  )
           {
               send_to_char("{RIt's starting to rain sheets of ice. Perhaps you should find a way inside{x \n\r", ch);
           }
        
           else if ( number_range( 0, 2000 ) <= 1250  )
           {
               send_to_char("{RThe ice around your feet firms up and causes you to fall flat on your face!{x\n\r",ch );
               act( "$n falls flat on their face as the ice around their feet begins to harden!", ch, NULL, NULL,TO_ROOM);
               damage( ch, ch, number_range( 10, 20 ), 0, DAM_COLD, FALSE );
           }

           else if ( number_range( 0, 2000 ) <= 1500  )
           {
               send_to_char("{RYou loose traction on the ice and begin to slide, you slam right into a tree! Good Job. Better hope you didn't break anything!{x\n\r",ch);
               act( "$n slips on the ice and slides right into a tree!", ch, NULL, NULL,TO_ROOM);
               damage( ch, ch, number_range( 15, 30 ), 0, DAM_BASH, FALSE );
           }

           else if ( number_range( 0, 2000 ) <= 1950  )
           {
               send_to_char("{RThe sky before you is a mist of white blur. Perhaps you should find a safe place indoors.{x \n\r",ch);
           }

           else if ( number_range( 0, 2000 ) <= 2000  )
           {
               send_to_char("{RYour body twitches and your limbs start to freeze one by one.{x \n\r", ch);
               act( "You watch as $n's limbs start to freeze !", ch, NULL, NULL,TO_ROOM);
               damage( ch, ch, number_range( 20, 40 ), 0, DAM_COLD, FALSE );
           }
       }  
}

void hail( void )
{
  DESCRIPTOR_DATA *d;

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
       if ( d->connected == CON_PLAYING
          && IS_OUTDOORS( d->character )
          && weather_info.sky == SKY_HAILSTORM && d->character->fighting == NULL )
       {

          if ( number_range( 0, 2000 ) <= 1000  )
          {
              send_to_char("Was that a golfball or hail? It might be a good idea to find yourself a way indoors quickly.\n\r",d->character);
          }
          
          else if ( number_range( 0, 2000 ) <= 1250  )
          {
              send_to_char("Oh MAN! You were just hit in the face by hail!\n\r",d->character );
              act( "You watch in amusement as $n is hit in the face by a piece of hail.  ", d->character, NULL, NULL,TO_ROOM);
              damage( d->character, d->character, number_range( 15, 30 ), 0, DAM_COLD, FALSE );
              
          }
           
          else if ( number_range( 0, 2000 ) <= 1950  )
          {
              send_to_char("The sky before you is a mist of white blur. Perhaps you should find a safe place indoors.\n\r",d->character);
          }

          else if ( number_range( 0, 2000 ) <= 2000  )
          {
              send_to_char("{RYour body twitches and your limbs start to freeze one by one.{x \n\r", d->character);
              act( "You watch as $n's limbs start to freeze!", d->character, NULL, NULL,TO_ROOM);
              damage( d->character, d->character, number_range( 20, 40 ), 0, DAM_COLD, FALSE );
          }
       }      
  }
}
 
void fog( CHAR_DATA *ch )
{
       if ( IS_OUTDOORS( ch )
          && weather_info.sky == SKY_FOGGY )
       {
 
           if ( number_range( 0, 2000 ) <= 1000  )
           {
               send_to_char("{RThe morning fog is as thick as pea soup. Perhaps you should find your way indoors.{x \n\r",ch);
           }
          
           else if ( number_range( 0, 2000 ) <= 1250  )
           {
               send_to_char("{RNot being able to see where you are going, you slip and fall into a hole.{x \n\r",ch );
               act( "$n falls face first into a hole!", ch, NULL, NULL,TO_ROOM);
               damage( ch, ch, number_range( 5, 25 ), 0, DAM_BASH, FALSE ); 
           }
         
           else if ( number_range( 0, 2000 ) <= 1500  )
           {
               send_to_char("{RYou've walked straight into a tree! Way to go!!{x \n\r",ch);
               act( "$n has planted themselves headfirst into a tree. Encore!", ch, NULL, NULL,TO_ROOM);
               damage( ch, ch, number_range( 10, 30 ), 0, DAM_BASH, FALSE );
           }

           else if ( number_range( 0, 2000 ) <= 1950  )
           {
              send_to_char("{RThe morning fog is as thick as pea soup. Perhaps you should find your way indoors.{x \n\r",ch);
           }

           else if ( number_range( 0, 2000 ) <= 2000  )
           {

              send_to_char("{RNot being able to see where you're going, you fall straight into a river.{x \n\r", ch);
              act( "$n pulls $m clothes out of the river after falling in!", ch, NULL, NULL,TO_ROOM);
              damage( ch, ch, number_range( 20, 40 ), 0, DAM_BASH, FALSE );
           }
       }  
}

 
void tornado( void )
{
  DESCRIPTOR_DATA *d;
  ROOM_INDEX_DATA *pRoomIndex;

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
       if ( d->connected == CON_PLAYING
          && IS_OUTDOORS( d->character )
          && number_chance( 10 )
          && d->character->level >= 25 
          && weather_info.sky == SKY_WIND && d->character->fighting == NULL )
       {

          pRoomIndex = get_random_room( d->character );
          sound( "tornado.wav", d->character );

          if ( number_range( 0, 2000 ) <= 1000  )
          {
              send_to_char("A large tornado forms in the distance, the funnel slowly stretches towards the ground, and starts moving in your direction.\n\r",d->character);
          }
          
          else if ( number_range( 0, 2000 ) <= 1250  )
          {
              send_to_char("Oh No! A large tornado has formed, and some of the debree from it just hit you in the face!\n\r",d->character );
              act( "You watch in horror as $n is hit in the face by debree from a tornado.", d->character, NULL, NULL,TO_ROOM);
              damage( d->character, d->character, number_range( 40, 80 ), 0, DAM_WIND, FALSE );
          }
           
          else if ( number_range( 0, 2000 ) <= 1950  )
          {
              send_to_char("A large funnel cloud is forming on the horizon, the possibility of a tornado is highly likely. You should find your way indoors.\n\r",d->character);
          }

          else if ( number_range( 0, 2000 ) <= 2000  )
          {
              send_to_char("{cThe clouds above you twist, turn and thunder into life as a large tornado is formed. The tornado touches down and begins sucking up everything in it's path, their is nothing you can do but watch in horror as you are sucked up into the eye of the tornado.{x\n\r", d->character);
              act( "You watch in horror as $n is sucked up into the eye of a large tornado that has formed, perhaps you should RUN!", d->character, NULL, NULL,TO_ROOM);
              damage( d->character, d->character, number_range( 250, 500 ), 0, DAM_WIND, FALSE );
              char_from_room( d->character );
              char_to_room( d->character, pRoomIndex );  
              do_function( d->character, &do_look, "auto" );
          }
       }   
  }
}

bool number_chance(int num)
{
    if (number_range(1,100) <= num) return TRUE;
    else return FALSE;
}

void spell_control_weather(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    if ( !str_cmp( target_name, "better" ) )
    {

        if (weather_info.sky == SKY_CLOUDLESS)
        {
            send_to_char("But it's so beautiful outside already\n\r",ch);
            return;
        }

        else if (weather_info.sky == SKY_CLOUDY)
        {
            send_to_char("You recite the ancient spell and the clouds part in obedience.\n\r",ch);
            act( "$n makes a strange movement with their hands and the clouds part.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_CLOUDLESS;
            return;
        }

        else if (weather_info.sky == SKY_RAINING)
        {
            send_to_char("You recite the ancient spell and the  rain stops in obedience.\n\r",ch);
            act( "$n makes a strange movement with their hands and the rain stops.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_CLOUDY;
            return;
        }
        
        else if (weather_info.sky == SKY_LIGHTNING)
        {
            send_to_char("You recite the ancient spell and the lightning ceases in obedience.\n\r",ch);
            act( "$n makes a strange movement with their hands and the lightning stops.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_THUNDERSTORM;
            return;
        }
         
        else if (weather_info.sky == SKY_THUNDERSTORM)
        {
            send_to_char("You recite the ancient spell and the storm ceases.\n\r",ch);
            act( "$n makes a strange movement with their hands and the  storm ceases.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_RAINING;
            return;
        }
       
        else if (weather_info.sky == SKY_SNOWING)
        {
            send_to_char("You recite the ancient spell and the snow ceases in obedience.\n\r",ch);
            act( "$n makes a strange movement with their hands and the snow ceases.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_CLOUDY;
            return;
        }
 
        else if (weather_info.sky == SKY_BLIZZARD)
        {
            send_to_char("You recite the ancient spell and the  horizon clears.\n\r",ch);
            act( "$n makes a strange movement with their hands and the horizon clears.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_SNOWING;
            return;
        }

        else if (weather_info.sky == SKY_ICESTORM)
        {
            send_to_char("You recite the ancient spell and the  horizon clears.\n\r",ch);
            act( "$n makes a strange movement with their hands and the horizon clears.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_RAINING;
            return;
        }

        else if (weather_info.sky == SKY_HAILSTORM)
        {
            send_to_char("You recite the ancient spell and the  horizon clears.\n\r",ch);
            act( "$n makes a strange movement with their hands and the horizon clears.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_RAINING;
            return;
        }

        else if (weather_info.sky == SKY_FOGGY)
        {
            send_to_char("You recite the ancient spell and the  horizon clears.\n\r",ch);
            act( "$n makes a strange movement with their hands and the horizon clears.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_CLOUDY;
            return;
        }

        else 
        {
            send_to_char("Bad Weather Call. Please notify the imms.\n\r",ch);
        }

    }

    else if ( !str_cmp( target_name, "worse" ) )
    {
        if (weather_info.sky == SKY_CLOUDLESS)
        {
            send_to_char("You recite the ancient spell and the clouds come at your command.\n\r",ch);
            act( "$n makes a strange movement with their hands and the clouds darken the sky.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_CLOUDY;
            return;
        }
          
        else if (weather_info.sky == SKY_CLOUDY)
        {
            send_to_char("You recite the ancient spell and the clouds trickle down rain.\n\r",ch);
            act( "$n makes a strange movement with their hands and the clouds open up to rain.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_RAINING;
            return; 
        }

        else if (weather_info.sky == SKY_RAINING)
        {
            send_to_char("You recite the ancient spell and the  rain  turns to hail.\n\r",ch);
            act( "$n makes a strange movement with their hands and the rain turns to hail.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_HAILSTORM;
            return;
        }
        
        else if (weather_info.sky == SKY_LIGHTNING)
        {
            send_to_char("You recite the ancient spell and the  clouds send down sheets of ice\n\r",ch);
            act( "$n makes a strange movement with their hands and the lightning turns to ice.", ch, NULL, NULL, TO_ROOM);
            weather_info.sky = SKY_ICESTORM;
            return;
        }

        else if (weather_info.sky == SKY_THUNDERSTORM)
        {
           send_to_char("You recite the ancient spell and the clouds clap in thunderous approval.\n\r",ch);
           act( "$n makes a strange movement with their hands and the  clouds clap in thunder.", ch, NULL, NULL, TO_ROOM);
	   rnd_sound(3, ch, "thunder1.wav", "thunder2.wav", "thunder3.wav");
           weather_info.sky = SKY_LIGHTNING;
           return;
        }

        else if (weather_info.sky == SKY_SNOWING)
        {
           send_to_char("You recite the ancient spell and the snow increases in obedience.\n\r",ch);
           act( "$n makes a strange movement with their hands and the snow turns to a blizzard.", ch, NULL, NULL, TO_ROOM);
           weather_info.sky = SKY_BLIZZARD; 
           return;
        }

        else if (weather_info.sky == SKY_BLIZZARD)
        {
           send_to_char("It's already as bad as it can get\n\r",ch);
           return;
        }

        else if (weather_info.sky == SKY_ICESTORM)
        {
           send_to_char("It's already as bad as it can get\n\r",ch);
           return;
        }

        else if (weather_info.sky == SKY_HAILSTORM)
        {
           send_to_char("It's already as bad as it can get\n\r",ch);
           return;
        }
      
        else if (weather_info.sky == SKY_FOGGY)
        {
           send_to_char("You recite the ancient spell and the  horizon clears.\n\r",ch);
           act( "$n makes a strange movement with their hands and the horizon clears.", ch, NULL, NULL, TO_ROOM);
           weather_info.sky = SKY_CLOUDY;
           return;
        }

        else 
        {
           send_to_char("Bad Weather Call. Please notify the imms.\n\r",ch);
        }

    }

    else
        send_to_char ("Do you want it to get better or worse?\n\r", ch );

    send_to_char( "Ok.\n\r", ch );
    return;
}

