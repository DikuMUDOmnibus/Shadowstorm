/* Ported from Sundermud *Credit to Lotherius* by Synon */

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "include.h"

// Strips line feed. Because zmud barfs with <SEND> </SEND> on a simple line feed.
char *strip_lf ( char *str )
{
     static char         newstr[MAX_STRING_LENGTH];
     int                 i, j;
     
     for ( i = j = 0; str[i] != '\0'; i++ )
          if ( str[i] != '\n' )
          {
               newstr[j++] = str[i];
          }
     newstr[j] = '\0';
     return newstr;
}

// Returns a string without \r and ~.
char *strip_cr ( char *str )
{
     static char         newstr[MAX_STRING_LENGTH];
     int                 i, j;

     for ( i = j = 0; str[i] != '\0'; i++ )
          if ( str[i] != '\r' )
          {
               newstr[j++] = str[i];
          }
     newstr[j] = '\0';

     return newstr;
}

/* Sound Stuff */

void do_sound ( CHAR_DATA * ch, char *argument )
{
     char arg[MSL];

     if ( IS_NPC(ch) || !ch->desc )
          return;

     argument = one_argument ( argument, arg );
          
     if ( !ch->desc->msp && !ch->desc->mxp )
     {
          send_to_char ( "Attempting to enable MSP sound.\n\r\n\r", ch );
          send_to_char ( "!!SOUND(off U=" TXT_SOUNDURL ")",ch );
          send_to_char ( "!!MUSIC(off U=" TXT_SOUNDURL ")",ch );
          send_to_char ( "\n\rIf you have an older version of Zmud you must read {Whelp MSP{w\n\r", ch);
          send_to_char ( "for information on how to install sounds. Other clients may vary.\n\r", ch);
          send_to_char ( "{WMSP sound is on. You must have an MSP client. (Help MSP)\n\r", ch );
          send_to_char ( "Now showing the sound control panel:\n\r\n\r", ch);
          ch->desc->msp = TRUE;
          ch->pcdata->svolume = 75;
          ch->pcdata->mvolume = 50;
          ch->pcdata->msp = ch->desc->msp;
          sound( "thx.wav", ch );
     }     

     if ( !str_cmp ( arg, "off" ) )
     {
          if ( ch->desc->mxp )
          {
               send_to_char ( "Cannot disable sound in MXP completely, but we will set the volume to Zero.\n\r", ch );
               ch->pcdata->mvolume = 0;
               ch->pcdata->svolume = 0;
          }

          else if ( ch->desc->msp )
          {
               send_to_char ( "Sound is now off.\n\r", ch );
               send_to_char ( "!!SOUND(off)",ch );
               send_to_char ( "!!MUSIC(off)",ch );
               ch->desc->msp = FALSE;
               ch->pcdata->msp = ch->desc->msp;
          }

          else
          {
               send_to_char ( "You don't have sounds enabled to begin with.\n\r", ch );
               return;
          }
          send_to_char ( "\n\rIf you prefer, the \"stop\" command will stop the current sound and leave sound on.\n\r", ch);
          return;
     }

     if ( !str_cmp ( arg, "m" ) || !str_cmp (arg, "music") )
     {
          if ( atoi(argument) < 0 || atoi(argument) > 100)
          {               send_to_char ("Volume values range from 0 (Off) to 100\%\n\r", ch);
               return;
          }
          printf_to_char( ch, "Music volume: %d\n\r", atoi(argument) );
          ch->pcdata->mvolume = atoi(argument);

          return;
     }

     if ( !str_cmp ( arg, "s" ) || !str_cmp (arg, "sounds") )
     {
          if ( atoi(argument) < 0 || atoi(argument) > 100)
          {
               send_to_char ("Volume values range from 0 (Off) to 100\%\n\r", ch);
               return;
          }
          printf_to_char( ch, "Sound Effects volume: %d\n\r", atoi(argument) );
          ch->pcdata->svolume = atoi(argument);
          return;
     }
     
     printf_to_char ( ch, "{G[{W---{G]{w Music              {G[{W---{G]{w Sound Effects\n\r");
     send_to_char (     "----------------------------------------------\n\r", ch);
     printf_to_char( ch, "{G[{W%3s{G]{w (m) Music Volume   {G[{W%3s{G]{w (s) Sounds Volume\n\r",
                    ( ch->pcdata->mvolume > 0 ? itos(ch->pcdata->mvolume) : "OFF" ),
                    ( ch->pcdata->svolume > 0 ? itos(ch->pcdata->svolume) : "OFF" ) );
     send_to_char (     "----------------------------------------------\n\r", ch);
     send_to_char("Syntax: sound <type> <vol>\n\rEnter 0 for OFF, type is \"m\" or \"s\"\n\r", ch);
     if ( ch->desc->msp )
          send_to_char("Use \"sound off\" to turn MSP off completely.\n\r", ch);
     return;
}

void do_stop ( CHAR_DATA *ch, char *argument)
{
     if (IS_NPC(ch) || !ch->desc)
          return;

     if ( ch->desc->mxp )
     {
          send_to_char ( MXP_SECURE "<SOUND off><MUSIC off>" MXP_LLOCK, ch );
          send_to_char ( "Currently playing sounds and music halted.\r\n", ch );
     }

     else if ( ch->desc->msp )
     {
          /* We're not going to clear "playing" because we don't want to restart the song yet. */
          send_to_char ( "!!SOUND(off)",ch );
          send_to_char ( "!!MUSIC(off)",ch );
          send_to_char ( "Currently playing sounds and music halted.\n\r", ch);
     }
     else     
          send_to_char("You don't have sound enabled...\n\r", ch);     
     return;
}

void stop_music ( DESCRIPTOR_DATA *d )
{
     if ( d != NULL && d->mxp )
     {
         send_to_desc ( MXP_SECURE "<MUSIC off>" MXP_LLOCK, d );
         return;
     }

     else if (d != NULL && d->msp )
     {
          send_to_desc ( "!!MUSIC(off)", d );
          return;
     }

     return;
}

/* Call this with: filename, ch */
/* eventually add sound classes */

void sound ( const char *fname, CHAR_DATA *ch )
{
     if ( IS_NPC(ch) || !ch->desc )
          return;

     if ( ch->desc->mxp )
     {
         printf_to_char( ch, MXP_SECURE "<SOUND %s V=%d U=" TXT_SOUNDURL ">" MXP_LLOCK, fname, ch->pcdata->svolume );
     }

     else if ( ch->desc->msp )
     {
         printf_to_char( ch, "!!SOUND(%s V=%d U=" TXT_SOUNDURL "%s)", fname, ch->pcdata->svolume, fname );
     }

     return;
}

void rnd_sound(int max, CHAR_DATA *ch, ...)
{
	va_list vlst;
	int i;
	int random_number = dice(1, max) - 1;

	va_start(vlst, ch);
	for(i = 0; i < random_number; i++)
	{
		/* Sorry about the premeture exit, but without it we could crash
		   the mud. */
		if(i > max)
			return;
		va_arg(vlst, char*);
	}

	sound(va_arg(vlst, char*), ch);
	va_end(vlst);

	return;
}

void music ( const char *fname, CHAR_DATA *ch, bool repeat )
{
     if ( IS_NPC(ch) || !ch->desc )
          return;
     
     if ( !ch->desc->msp && !ch->desc->mxp )
          return;

     if ( !str_cmp ( fname, ch->pcdata->mplaying ) )
          return;
     
     free_string ( ch->pcdata->mplaying );
     ch->pcdata->mplaying = str_dup ( fname );
     
     if (repeat)
     {
         if ( ch->desc->mxp )          
              printf_to_char( ch, MXP_SECURE "<MUSIC %s V=%d L=-1 C=1 U=" TXT_SOUNDURL ">" MXP_LLOCK, fname, ch->pcdata->mvolume );

         else if ( ch->desc->msp )
              printf_to_char( ch, "!!MUSIC(%s V=%d L=-1 C=1 U=" TXT_SOUNDURL "%s)", fname, ch->pcdata->mvolume, fname );          
     }

     else
     {
          if ( ch->desc->mxp )
              printf_to_char( ch, MXP_SECURE "<MUSIC %s V=%d C=1 U=" TXT_SOUNDURL ">" MXP_LLOCK, fname, ch->pcdata->mvolume );
          else if ( ch->desc->msp )
              printf_to_char( ch, "!!MUSIC(%s V=%d C=1 U=" TXT_SOUNDURL "%s)", fname, ch->pcdata->mvolume, fname );
     }

     return;
}

void do_testsound(CHAR_DATA * ch, char *argument)
{
	rnd_sound(3, ch, "thunder1.wav", "thunder2.wav", "thunder3.wav");
	return;
}

void mxp_init ( DESCRIPTOR_DATA *d )
{
    char buf[MSL];
    
    SNP ( buf, MXP_SECURE
           "<!ELEMENT RName '<FONT \"Comic Sans MS\" COLOR=CYAN> <B>' FLAG=\"RoomName\">"
           "<!ELEMENT RDesc FLAG='RoomDesc'>"
           "<!ELEMENT RExits FLAG='RoomExit'>"
           "<!ELEMENT Ex '<SEND>'>" 
           "<!ELEMENT List \"<send href='buy &#39;&name;&#39;' ""hint='Buy &desc;'>\" " "ATT='name desc'>");

    write_to_buffer ( d, buf, 0 );
    SNP ( buf, MXP_SECURE "<SUPPORT image send frame stat>" );
    write_to_buffer ( d, buf, 0 );
    SNP ( buf, MXP_LLOCK ); // Locked mode, no player MXP tags..... 
    write_to_buffer ( d, buf, 0 );
    
    return;
}

/*
 * Opens a "secure" channel for tags.
 * Implemented as char so it can be embedded in other text.
 */
char                         *tag_secure ( DESCRIPTOR_DATA *d )
{
    static char                   buf[128];
    
    buf[0] = '\0';
    
    if ( d->mxp )
        SLCAT ( buf, MXP_SECURE );
    else
        SLCAT ( buf, "" );
    
    return buf;
}

/*
 * Closes secure tag channel
 * Implemented as char so it can be embedded in other text.
 */
char                         *tag_close ( DESCRIPTOR_DATA *d )
{
    static char                   buf[128];
    
    buf[0] = '\0';
    
    if ( d->mxp )
        SLCAT ( buf, MXP_LOPEN ); /* We're going to use locked open mode now. */
    else
        SLCAT ( buf, "" );
    return buf;
}

void inline_image ( DESCRIPTOR_DATA *d, char *image, char *align, bool pageit )
{
     char buf[MSL];

     if ( d->mxp )
     {
         
          SNP( buf, "<IMAGE FName=\"%s\" URL=\"" TXT_IMAGEURL "\" ALIGN=\"%s\">",
                strip_cr(strip_lf(image)), align );
          
          if ( pageit )
          {
               if ( !d->character )
               {
                    bugf( "No character on inline_image paged call." );
                    return;
               }
               page_to_char ( MXP_SECURE, d->character );
               page_to_char ( buf, d->character );
               page_to_char ( MXP_LLOCK, d->character );
          }
          else
          {
               send_to_desc ( MXP_SECURE, d);
               send_to_desc ( buf, d );
               send_to_desc ( MXP_LLOCK, d );
          }
     }
     return;
}

void do_image ( CHAR_DATA *ch, char *argument )
{
     inline_image ( ch->desc, argument, "bottom", FALSE );
}

char *click_cmd ( DESCRIPTOR_DATA *d, char *text, char *command, char *mouseover )
{
     static char buf[MSL];
     
     buf[0] = '\0';

     if ( d->mxp )
     {
          SNP ( buf, MXP_SECURE "<send \"%s\" hint=\"%s\">%s</SEND>" MXP_LLOCK, command, mouseover, strip_cr(strip_lf(text)) );
     }
     else
     {
          SNP ( buf, text );
     }

     return buf;
}

/* I think we can make use of this as well Synon */
void mxp_to_char(CHAR_DATA *ch, char *txt, int mxp_style)
{
  char buf[2*MAX_STRING_LENGTH];
  int i = 0, j = 0;

  if (!ch->desc)
    return;

  if (IS_MXP( ch ) )
  {
    switch(mxp_style)
    {
      default:
	bug("Mxp_to_char: strange style '%d'.", mxp_style);
	strcpy(buf, txt);
      case MXP_SAFE:
        sprintf(buf, "%s%s%s", "\e[0z", txt, "\e[7z");
	break;
      case MXP_ALL:
//	    sprintf(buf, "%s%s%s", "\e[1z", txt, "\e[7z");
	buf[j++] = '\e'; buf[j++] = '['; buf[j++] = '1'; buf[j++] = 'z';
	while (txt[i] != '\0')
	{
          switch(txt[i])
	  {
            default:
	      buf[j++] = txt[i++];
	      break;
            case '<':
	      if (!memcmp(&txt[i], "<BR>", strlen("<BR>")))
	      {
	        i += strlen("<BR>");
		buf[j++] = '\n'; buf[j++] = '\r';
                buf[j++] = '\e'; buf[j++] = '['; buf[j++] = '1'; buf[j++] = 'z';
	      }
              else
                buf[j++] = txt[i++];
	      break;
	  }
	}
        buf[j++] = '\e'; buf[j++] = '['; buf[j++] = '7'; buf[j++] = 'z';
        buf[j] = '\0';
    	break;
      case MXP_NONE:
        sprintf(buf, "%s%s", "\e[7z", txt);
	break;
    }
  }
  else
  {
    while(txt[i] != '\0')
    {
      switch(txt[i])
      {
        default:
	  buf[j++] = txt[i++];
	  break;
        case '<':
	  if (!memcmp(&txt[i], "<B>", strlen("<B>")))
	    i += strlen("<B>");
	  else if (!memcmp(&txt[i], "</B>", strlen("</B>")))
	    i += strlen("</B>");
	  else if (!memcmp(&txt[i], "<U>", strlen("<U>")))
	    i += strlen("<U>");
	  else if (!memcmp(&txt[i], "</U>", strlen("</U>")))
	    i += strlen("</U>");
	  else if (!memcmp(&txt[i], "<I>", strlen("<I>")))
	    i += strlen("<I>");
	  else if (!memcmp(&txt[i], "</I>", strlen("</I>")))
	    i += strlen("</I>");
	  else if (!memcmp(&txt[i], "<BR>", strlen("<BR>")))
	  {
            if (mxp_style == MXP_ALL)
            {
              buf[j++] = '\n';
              buf[j++] = '\r';
            }
	    i += strlen("<BR>");
	  }
	  else if (!memcmp(&txt[i], "<SEND", strlen("<SEND")))
	  {
	    i += strlen("<SEND");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<A href", strlen("<A href")))
	  {
	    i += strlen("<A href");
	    while(txt[i++] != '>')
              ;
	  }
          else if (!memcmp(&txt[i], "<EXPIRE", strlen("<EXPIRE")))
          {
            i += strlen("<EXPIRE");
            while(txt[i++] != '>')
              ;
          }
	  else if (!memcmp(&txt[i], "</SEND>", strlen("</SEND>")))
	    i += strlen("</SEND>");
	  else if (!memcmp(&txt[i], "</A>", strlen("</A>")))
	    i += strlen("</A>");
          else
	    buf[j++] = txt[i++];
	  break;
      }
    }
    buf[j] = '\0';
  }

  send_to_char(buf, ch);
} 

void sound_to_room( const char *fname, CHAR_DATA *ch, bool to_world )
{
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;
    
    if ( !to_world )
    {
        for( vch = ch->in_room->people; vch; vch = vch->next_in_room )
        {
            if( IS_NPC( vch ) ) 
            return;

            if ( vch->desc->mxp )
            {
                printf_to_char( vch, MXP_SECURE "<SOUND %s V=%d U=" TXT_SOUNDURL ">" MXP_LLOCK, fname, vch->pcdata->svolume );
            }
 
            else if ( vch->desc->msp )
            {
                printf_to_char( vch, "!!SOUND(%s V=%d U=" TXT_SOUNDURL "%s)", fname, vch->pcdata->svolume, fname );
            }
        }
    }
  
    else
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
             vch = d->character;

             if  ( vch == NULL || IS_NPC(vch) )
                  continue;

             if ( vch->desc->mxp )
             {
                 printf_to_char( vch, MXP_SECURE "<SOUND %s V=%d U=" TXT_SOUNDURL ">" MXP_LLOCK, fname, vch->pcdata->svolume );
             }

             else if ( vch->desc->msp )
             {
                 printf_to_char( vch, "!!SOUND(%s V=%d U=" TXT_SOUNDURL "%s)", fname, vch->pcdata->svolume, fname );
             }
        }
    }

    return;
}

void do_clist( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
        printf_to_char( ch, "Mobs don't have clients.\n\r", ch );
        return;
    }

    printf_to_char( ch, "Type Command Status\n\r" );
    printf_to_char( ch, "-------------------\n\r");
    printf_to_char( ch, "{W%-6s {g%-8s %s{x\n\r", "MXP", "None", ch->desc->mxp == TRUE ? "{GOn{x" : "{ROff{x" );
    printf_to_char( ch, "{W%-6s {g%-8s %s{x\n\r", "MSP", "Sound", ch->desc->msp == TRUE ? "{GOn{x" : "{ROff{x" );
    printf_to_char( ch, "{W%-6s {g%-8s %s{x\n\r", "MCCP", "Compress", ch->desc->out_compress ? "{GOn{x" : "{ROff{x" );
    printf_to_char( ch, "{W%-6s {g%-8s %s{x", "Color", "Color", IS_SET( ch->act, PLR_COLOUR ) ? "{GOn{x" : "{ROff{x" );
    return;
}
