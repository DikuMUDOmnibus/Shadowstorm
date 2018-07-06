#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
 
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif

int html_colour args (( char type, char *string ));
void html_colourconv args (( char *buffer, const char *txt, CHAR_DATA *ch ));

void who_html_update (void)
{

  FILE *fp; 
  DESCRIPTOR_DATA *d;
  char buf[2*MAX_INPUT_LENGTH]; 
  char buf2[2*MAX_INPUT_LENGTH];
  
  buf[0] = '\0';
  buf2[0] = '\0';
  
  if ( (fp = file_open("../../public_html/online.html", "w") ) == NULL)
  {
      logf2( "%s",  "Online html doesn't exist.\n\r");
  }

  else
  {
  fprintf(fp, "%s", "<html>\n");
  fprintf(fp, "<head>\n");
  fprintf(fp, "<title>\n");
  fprintf(fp, "Players currently on ShadowStorm\n");

  fprintf(fp, "</title>\n");
 
fprintf(fp, "</head>\n");
fprintf(fp, "<BODY BGCOLOR=#000000 text=#e6e6e6 font=arial link=#e6e6e6 vlink=#e6e6e6 alink=#e6e6e6 LEFTMARGIN=0 TOPMARGIN=0 MARGINWIDTH=0 MARGINHEIGHT=0>\n");
fprintf(fp, "<BR>\n");
fprintf(fp, "<BR>\n");
  fprintf(fp, "<font face=""Arial""><center>\n");
  fprintf(fp, "<font size=+2>Players currently on ShadowStorm</font>\n");
 fprintf(fp, "<BR>\n");
fprintf(fp, "<BR>\n");
fprintf(fp, "<BR>\n");

  fprintf(fp, "<CENTER><TABLE BORDER=1 bordercolor=DDDDDD font face=""Arial"">\n");
  fprintf(fp, "<TR ALIGN=LEFT VALIGN=CENTER font face=""Arial"">\n");
  fprintf(fp, "<TD font face=""Arial""><B>Level</B></TD>\n");
  fprintf(fp, "<TD font face=""Arial""><B>Race</B></TD>\n");
  fprintf(fp, "<TD font face=""Arial""><B>Class</B></TD>\n");
  fprintf(fp, "<TD font face=""Arial""><B>Guild</B></TD>\n");
  fprintf(fp, "<TD font face=""Arial""><B>Name</B></TD></TR>\n");
    
  for ( d = descriptor_list; d != NULL ; d = d->next )
  {
    CHAR_DATA *wch;
    char klass[5];
    
    if ( d->connected != CON_PLAYING)
        continue;
    wch   = ( d->original != NULL ) ? d->original : d->character;
    klass[0] = '\0';

    if (!(wch->invis_level > LEVEL_HERO))
    {
	fprintf(fp, "<TR ALIGN=LEFT VALIGN=CENTER font face=""Arial"">\n");
	fprintf(fp, "<TD font face=""Arial"">%d</TD>\n", wch->level);
	fprintf(fp, "<TD font face=""Arial"">%s</TD>\n", race_table[wch->race].name);
	fprintf(fp, "<TD font face=""Arial"">%s</TD>\n", class_long(wch));
	fprintf(fp, "<TD font face=""Arial"">%s</TD>\n", clan_table[wch->clan].name);
	fprintf(fp, "<TD font face=""Arial"">%s", wch->name);

      buf2[0] = '\0';
      sprintf(buf2, "%s", (IS_NPC(wch)? "" : wch->pcdata->title));
	html_colourconv( buf, buf2, wch );
      fprintf(fp, buf);
      fprintf(fp, "</TD></TR>\n");
 
    }  /*end if */
  }    /*end for */
  
  fprintf(fp, "</TABLE></CENTER>\n");
  fprintf(fp, "<BR>\n");
    fprintf(fp, "<HR Width=80>\n");
	  fprintf(fp, "<BR>\n");
  fprintf(fp, "<font face=""Arial""><center>\n");
  sprintf(buf, "Last updated at %s Eastern Time.\n", ((char *) ctime( &current_time )));
  fprintf(fp, buf);
  fprintf(fp, "</center></font>\n");

  fprintf(fp, "<br><br>\n");

  fprintf(fp, "<font face=""Arial""><center>\n");
  fprintf(fp, "<P><A HREF=""./"" TARGET=""_top"">\n");
  fprintf(fp, "Return to main page</A> </P></CENTER></Font>\n");
  fprintf(fp, "   </div></td>\n");
 
 fprintf(fp, "   </tr>\n");
  fprintf(fp, "</table>\n");

  fprintf(fp, "</body>\n");
  fprintf(fp, "</html>\n");
  file_close( fp ); 
 // fpReserve = fopen( NULL_FILE, "r" );
  } /*end if */ 
  
  return;
}
/* end function */

int html_colour( char type, char *string )
{
    char	code[ 25 ];
    char	*p = '\0';

        
    switch( type )
    {
	default:
	case '\0':
	    code[0] = '\0';
	    break;
	case ' ':
	    sprintf( code, " " );
	    break;
	case 'x':
	    sprintf( code, "<font color=""#006400"">" );
	    break;
	case 'b':
	    sprintf( code, "<font color=""#00008B"">" );
	    break;
	case 'c':
	    sprintf( code, "<font color=""#008B8B"">" );
	    break;
	case 'g':
	    sprintf( code, "<font color=""#006400"">" );
	    break;
	case 'm':
	    sprintf( code, "<font color=""#8B008B"">" );
	    break;
	case 'r':
	    sprintf( code, "<font color=""#8B0000"">" );
	    break;
	case 'w':
	    sprintf( code, "<font color=""#808080"">" );
	    break;
	case 'y':
	    sprintf( code, "<font color=""#808000"">" );
	    break;
	case 'B':
	    sprintf( code, "<font color=""#0000FF"">" );
	    break;
	case 'C':
	    sprintf( code, "<font color=""#OOFFFF"">" );
	    break;
	case 'G':
	    sprintf( code, "<font color=""#00FF00"">" );
	    break;
	case 'M':
	    sprintf( code, "<font color=""#FF00FF"">" );
	    break;
	case 'R':
	    sprintf( code, "<font color=""#FF0000"">" );
	    break;
	case 'W':
	    sprintf( code, "<font color=""#FFFFFF"">" );
	    break;
	case 'Y':
	    sprintf( code, "<font color=""#FFFF00"">" );
	    break;
	case 'D':
	    sprintf( code, "<font color=""#636363"">" );
	    break;
	case '{':
	    sprintf( code, "{" );
	    break;
    }

    p = code;
    while( *p != '\0' )
    {
	*string = *p++;
	*++string = '\0';
    }

    return( strlen( code ) );
}

void html_colourconv( char *buffer, const char *txt, CHAR_DATA *ch )
{
    const	char	*point;
		int	skip = 0;

    for( point = txt ; *point ; point++ )
    {
	if( *point == '{' )
	{
	    point++;
	    if( *point == '\0' )
		point--;
	    else
	      skip = html_colour( *point, buffer );
	    while( skip-- > 0 )
		++buffer;
	    continue;
	}
	/* Following is put in to prevent adding HTML links to titles,
	   except for IMMS who know what they're doing and can be
	   punished if they screw it up! */
	if( (*point == '<') && (!IS_IMMORTAL(ch)) )
	{
	    *buffer = '[';
	    *++buffer = '\0';
	    continue;
	}
	if( (*point == '>') && (!IS_IMMORTAL(ch)) )
	{
	    *buffer = ']';
	    *++buffer = '\0';
	    continue;
	}
	*buffer = *point;
	*++buffer = '\0';
    }			
    *buffer = '\0';
    return;
}


                                       
