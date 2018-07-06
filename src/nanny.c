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

bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name, bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
void	send_creation_menu  args( ( CHAR_DATA *ch ) );
int		race_count			args( (int align) );
extern bool	wizlock;
extern const	char	echo_off_str	[];
extern const	char	echo_on_str	[];
extern bool newlock;
/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    EMAIL_DATA *pMail;
    DESCRIPTOR_DATA *d_old, *d_next;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    CHAR_DATA *vch;
    char *pwdnew;
    char *p;
    int iClass,race,i,weapon, wrap, counter;
    bool fOld, found;
	RELIGION *pRlg;

    if (d->connected != CON_NOTE_TEXT)
    {	while ( isspace(*argument) )
        argument++;
    }                             
	ch = d->character;

    switch ( d->connected )
    {    default:
			bug( "Nanny: bad d->connected %d.", d->connected );
			close_socket( d );
			return;
/***************************************************************************************************/
		case CON_GET_NAME:
			if ( argument[0] == '\0' )
			{	close_socket( d );
				return;
			}

			argument[0] = UPPER(argument[0]);
			if ( !check_parse_name( argument ) )
			{	write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
			    return;
			}
			fOld = load_char_obj( d, argument );
			ch   = d->character;
			if (IS_SET(ch->act, PLR_DENY))
			{	sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
			    log_string( log_buf );
				write_to_buffer( d, "You are denied access.\n\r", 0 );
			    close_socket( d );
				return;
			}

			if (check_ban(d->host,BAN_PERMIT) && !IS_SET(ch->act,PLR_PERMIT))
			{	write_to_buffer(d,"Your site has been banned from this mud.\n\r",0);
			    close_socket(d);
				return;
			}

			if ( check_reconnect( d, argument, FALSE ) )
				fOld = TRUE;
			else if ( wizlock && !IS_IMMORTAL(ch)) 
			{	write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
				close_socket( d );	
				return;
			}
			

			if ( fOld )
			{	/* Old player */
			    write_to_buffer( d, "Password: ", 0 );
			    write_to_buffer( d, echo_off_str, 0 );
			    d->connected = CON_GET_OLD_PASSWORD;
			    return;
			}
			else
			{	/* New player */
		 	    if (newlock)
				{	write_to_buffer( d, "The game is newlocked.\n\r", 0 );
	                close_socket( d );
	                return;
		        }

				if (check_ban(d->host,BAN_NEWBIES))
			    {	write_to_buffer(d, "New players are not allowed from your site.\n\r",0);
					close_socket(d);
					return;
			    }
				sprintf( buf, "\n\r{DWe here at ShadowStorm prefer you keep your names rpish. \n\rWhich means, don't make inappropriate names. Make names as if it where \n\rMedieval times. Make them names, not something like \"Bladedancer\" or \n\rsuch. Such names will be asked to be changed.\n\r\n\r");
			    send_to_desc(  buf, d );	
			    sprintf( buf, "{DWould you like to keep {r\"{W%s{r\" {Das your name? {r({WY{r/{WN{r){D? ", argument );
				send_to_desc( buf, d );
			    d->connected = CON_CONFIRM_NEW_NAME;
				return;
			}
			break;

/***************************************************************************************************/
		case CON_GET_OLD_PASSWORD:
			#if defined(unix)
				write_to_buffer( d, "\n\r", 2 );
			#endif

			if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
			{	write_to_buffer( d, "Wrong password.\n\r", 0 );
			    close_socket( d );
			    sprintf( log_buf, "%s. Bad Password. IP: %s> %s", ch->name,d->host,argument);
				log_string( log_buf );
		        sprintf( log_buf, "%s@%s bad password.", ch->name, d->host );
				wiznet(log_buf,NULL,NULL,WIZ_LOGINS,0,get_trust(ch));
		        if ((ch = get_char_world(ch,ch->name)) != NULL)
				{	send_to_char(   "{R>{r>{D> {wBAD {WPASSWORD {wATTEMPT {D<{r<{R<{x\n\r{x",ch);
					sprintf( buf,"{w%s tried to log in with a bad password.\n\r{x",d->host);
					send_to_char(buf,ch);
		        } 
				return;
			}
 
			write_to_buffer( d, echo_on_str, 0 );

			if (check_playing(d,ch->name))
				return;

			if ( check_reconnect( d, ch->name, TRUE ) )
				return;

			sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
			log_string( log_buf );
			wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

			if ( IS_IMMORTAL(ch) )
			{	
                            printf_to_char(ch, "{DPlease Pick An Option{x\n\r\n\r{M{DE{r:{W Enter ShadowStorm\n\r{DL{r: {WLeave ShadowStorm\n\r{DS{r:{W Shadowstorm Wizlist\n\r{DW{r:{W Activate Wizinviz\n\r{DH{r:{W Who's on ShadowStorm\n\r{DD{r:{W Delete\n\r\n\r{DEnter Selection{W ->{x"); 
			    d->connected = CON_MENU;
			}
	                else
			{	
                            printf_to_char(ch, "{DPlease Pick An Option{x\n\r\n\r{M{DE{r:{W Enter ShadowStorm\n\r{DL{r: {WLeave ShadowStorm\n\r{DS{r:{W Shadowstorm Wizlist\n\r{DH{r:{W Who's on ShadowStorm\n\r{DD{r:{W Delete\n\r\n\r{DEnter Selection{W ->{x");  
        	            d->connected = CON_MENU;
			}
	        break;                                             
/***************************************************************************************************/    
        case CON_MENU:
			switch ( *argument )
			{	case 'e': case 'E':
			        write_to_buffer(d,"\n\rLogging In\n\r",0);
			        if ( IS_IMMORTAL(ch) )
			        {	do_function(ch, &do_help, "imotd" );
			            d->connected = CON_READ_IMOTD;
				}
			        else
				{
 	                            do_function(ch, &do_help, "motd" );
			            d->connected = CON_READ_MOTD;
			        }
			        break;                                          
 
		         case 's': case 'S':
				write_to_buffer( d, "\n\rShadowstorm Wizlist.\n\r",0);
				do_function(ch, &do_wizlist, "" );
		                write_to_buffer( d, "\n\r*[HIT RETURN]*\n\r",0);
			        break;

		        case 'l': case 'L':
			       write_to_buffer( d, "\n\rLeaving so soon!\n\r ", 0 );
		               close_socket(d);
			       break;

		        case 'h': case 'H':
			       write_to_buffer( d, "Who's on Shadowstorm.\n\r", 0 );
		               do_function(ch, &do_who, "" );
			       write_to_buffer( d, "\n\r*[HIT RETURN]*\n\r", 0 );
		               break;

			case 'w': case 'W':
		            if (IS_IMMORTAL(ch) )
			    {
                          	if(is_admin(ch) )
		                {	
                                   ch->invis_level = atoi(argument + 1);
				    
                                   if (ch->invis_level <= 1  || ch->invis_level > get_trust(ch))
			               ch->invis_level = get_trust(ch);
			        }
					    
                                else
				   ch->invis_level = LEVEL_IMMORTAL;
                                   printf_to_char( ch, "Invis level set to %d\n\r", ch->invis_level ); 
		            }
				
                            else
			        send_to_char( "Not at your level\n\r", ch );
				break;          

		        case 'd' : case 'D':
				do_function(ch, &do_delete, ""); 
		                printf_to_char( ch, "Entering or leaving Shadowstorm will remove your delete status.\n\r" );
			        write_to_buffer( d, "\n\r*[HIT RETURN]*\n\r",0);                 
		                break;        

	      	         	default:
                                printf_to_char(ch, "{DPlease Pick An Option{x\n\r\n\r{M{DE{r:{W Enter ShadowStorm\n\r{DL{r: {WLeave ShadowStorm\n\r{DS{r:{W Shadowstorm Wizlist\n\r{DH{r:{W Who's on ShadowStorm\n\r{DD{r:{W Delete\n\r\n\r{DEnter Selection{W ->{x");  
			        break;
			}
	        break;

/***************************************************************************************************/

		case CON_NOTE_TO:
	        handle_con_note_to (d, argument);
		    break;

		case CON_NOTE_SUBJECT:
			handle_con_note_subject (d, argument);
			break; /* subject */

                case CON_NOTE_EXPIRE:
			handle_con_note_expire (d, argument);
			break;

                case CON_NOTE_TEXT:
			handle_con_note_text (d, argument);
	                break;

                case CON_NOTE_FINISH:
		        handle_con_note_finish (d, argument);
			break;            

/***************************************************************************************************/
 
	        case CON_BREAK_CONNECT:
			switch( *argument )
			{	case 'y' : case 'Y':
		            for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
					{	d_next = d_old->next;
						if (d_old == d || d_old->character == NULL)
						    continue;
						if (str_cmp(ch->name,d_old->original ? d_old->original->name : d_old->character->name))
						    continue;
						close_socket(d_old);
					}
				    if (check_reconnect(d,ch->name,TRUE))
	    				return;
				    write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
					if ( d->character != NULL )
		            {	nuke_pets( d->character );
		                free_char( d->character );
				        d->character = NULL;
					}
					d->connected = CON_GET_NAME;
				    break;

				case 'n' : case 'N':
				    write_to_buffer(d,"Name: ",0);
					if ( d->character != NULL )
		            {	nuke_pets( d->character );
		                free_char( d->character );
				        d->character = NULL;
					}
				    d->connected = CON_GET_NAME;
				    break;

				default:
				    write_to_buffer(d,"Please type Y or N? ",0);
				    break;
			}
			break;

/***************************************************************************************************/

		case CON_CONFIRM_NEW_NAME:
			switch ( *argument )
			{	case 'y': case 'Y':
				    sprintf( buf, "New character.\n\r\n\rWhat password would you like for %s: %s",
					ch->name, echo_off_str );
				    write_to_buffer( d, buf, 0 );
					d->connected = CON_GET_NEW_PASSWORD;
					SET_BIT( ch->act, PLR_COLOUR );
					break;

				case 'n': case 'N':
				    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
					nuke_pets( d->character );
				    free_char( d->character );
					d->character = NULL;
					d->connected = CON_GET_NAME;
					break;

				default:
					write_to_buffer( d, "Please type Yes or No? ", 0 );
				    break;
			}
			break;

/***************************************************************************************************/

		case CON_GET_NEW_PASSWORD:
			#if defined(unix)
				write_to_buffer( d, "\n\r", 2 );
			#endif

			if ( strlen(argument) < 5 )
			{	write_to_buffer( d, "Password must be at least five characters long.\n\rPassword: ",0 );
			    return;
			}

			pwdnew = crypt( argument, ch->name );
			for ( p = pwdnew; *p != '\0'; p++ )
			{	if ( *p == '~' )
			    {	write_to_buffer( d, "New password not acceptable, try again.\n\rPassword: ", 0 );
					return;
			    }
			}

			free_string( ch->pcdata->pwd );
			ch->pcdata->pwd	= str_dup( pwdnew );
			write_to_buffer( d, "Please retype password: ", 0 );
			d->connected = CON_CONFIRM_NEW_PASSWORD;
			break;

/***************************************************************************************************/

		case CON_CONFIRM_NEW_PASSWORD:
			#if defined(unix)
				write_to_buffer( d, "\n\r", 2 );
			#endif
			if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
			{	write_to_buffer( d, "Passwords don't match.\n\rRetype password: ", 0 );
			    d->connected = CON_GET_NEW_PASSWORD;
				return;
			}
			write_to_buffer( d, echo_on_str, 0 );

			for(i = 0; i < MAX_CBOOL ; i ++ )
				ch->pcdata->creation[i] = FALSE;

			send_creation_menu(ch);
			d->connected = CON_CREATION_MENU;
			break;

/***************************************************************************************************/

		case CON_CREATION_MENU:
			switch(UPPER( argument[0] ))
			{	
				case 'A':
					send_to_char("\n\r{DOne of the first step to becoming one with the land, is choosing\n\ra race. Choose wisely. Feel free to ask for help.\n\rPlease note, your alignment is selected depending on your race\n\r\n\r", ch);
					wrap = 0;
					d->connected = CON_GET_NEW_RACE;
					counter = 1;
				/*This part is pretty bad. I basicly set it up so adding new races would fuck the alignment.
				  I'm gunna hate myself for this when we descide to expand.... 
																				~Davion
				*/
					if(race_count(ALIGN_NEUTRAL) > 0 )
					{	printf_to_char(ch, "\t{DNeutral Race%s{r:\n\r\t\t", race_count(ALIGN_NEUTRAL) == 1 ? "{r" : "s{r" );
						for(race = 1; race <= race_count(ALIGN_NEUTRAL) ; race++ )
						{	if(counter==MAX_PC_RACE)
								break;
							else
							{	if (!race_table[race].pc_race)
									break;
								printf_to_char(ch, "{D%d{r:{W %s{x", counter, race_table[race].name,0);
								if(strlen(race_table[race].name) <= 4)
									send_to_char("\t\t",ch);
								else if(strlen(race_table[race].name) > 10 )
									send_to_char(" ",ch);
								else
									send_to_char("\t",ch);
								wrap++;
								if(wrap == 3)
								{	send_to_char("\n\r",ch);
									wrap = 0;
								}
								counter++;
							}
						}
						send_to_char("\n\r\n\r",ch);
					}
					wrap = 0;				
					if(race_count(ALIGN_GOOD) > 0 )
					{	printf_to_char(ch, "\t{DBenevolant Race%s{r:\n\r\t\t", race_count(ALIGN_NEUTRAL) == 1 ? "{r" : "s{r" );
						for(; race <= (race_count(ALIGN_GOOD) + race_count(ALIGN_NEUTRAL) ) ; race++ )
						{	if(counter==MAX_PC_RACE)
								break;
							else
							{	if (!race_table[race].pc_race)
									break;
								printf_to_char(ch, "{D%d{r:{W %s{x", counter, race_table[race].name,0);
								if(strlen(race_table[race].name) <= 4)
									send_to_char("\t\t",ch);
								else if(strlen(race_table[race].name) > 10 )
									send_to_char(" ",ch);
								else
									send_to_char("\t",ch);
								wrap++;
								if(wrap == 3)
								{	send_to_char("\n\r",ch);
									wrap = 0;
								}
								counter++;
							}
						}
						send_to_char("\n\r",ch);
					}
					wrap = 0;
					if(race_count(ALIGN_EVIL) > 0 )
					{	printf_to_char(ch, "\t{DMalevolant Race%s{r:\n\r\t\t", race_count(ALIGN_NEUTRAL) == 1 ? "{r" : "s{r" );
						for(; race < MAX_PC_RACE ; race++ )
						{	if(counter==MAX_PC_RACE)
								break;
							else
							{	if (!race_table[race].pc_race)
									break;
								printf_to_char(ch, "{D%d{r:{W %s{x", counter, race_table[race].name,0);
								if(strlen(race_table[race].name) <= 4)
									send_to_char("\t\t",ch);
								else if(strlen(race_table[race].name) > 10 )
									send_to_char(" ",ch);
								else
									send_to_char("\t",ch);
								wrap++;
								if(wrap == 3)
								{	send_to_char("\n\r",ch);
									wrap = 0;
								}
								counter++;
							}
						}
						send_to_char("\n\r",ch);
					}

					ch->pcdata->creation[CHOSE_RACE] = TRUE;
					send_to_char("\n\r\n\r{DPlease choose one of the following. You can use the name, or the corresponding number\n\rEnter race{W-> ",ch);
					break;

				case 'B':
					counter = 1;
					wrap = 0;
					send_to_char( "\n\r{DNow, a {Wclass{D must be chosen. These are your choices{r:{x\n\r",ch);
					for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
					{	sprintf(buf, " {D%d{r:{W ", counter);
						send_to_char(buf,ch);
						send_to_char(class_table[iClass].name, ch);
						if(strlen(class_table[iClass].name) < 4)
							send_to_char("\t\t",ch);
						else if(strlen(class_table[iClass].name) > 10 )
							send_to_char(" ",ch);
						else
							send_to_char("\t",ch);
						wrap++;
						if(wrap == 3)
						{	send_to_char("\n\r",ch);
							wrap = 0;
						}
						counter++;
	
					}
					send_to_char("\n\r\n\r{DPlease choose one listed. You can use the name or the corresponding number.\n\rEnter class{W-> {x",ch);

					d->connected = CON_GET_NEW_CLASS;
					break;
				case 'C':
					send_to_char("\n\r{DWhat would you like your gender to be? {WM{Dale or {WF{Demale?\n\rEnter gender{W-> {x", ch);
					ch->pcdata->creation[CHOSE_GENDER] = TRUE;
					d->connected = CON_GET_NEW_SEX;
					break;
				case 'D':
					if(IS_SET(ch->act, PLR_REMORT))
					{	
		                            send_to_char("{DYou have already selected a home town!\n\rEnter selection{W->{x ",ch);
					    break;
					}
					send_to_char( "\n\r{DNow to select a Hometown. Your choices are{r:{x\n\r", ch );
					counter = 1;
					for (i=0;hometown_table[i].name != NULL; ++i)
					{	sprintf(buf,"{D%d{r:{W %s\n\r{x", counter, hometown_table[i].name );
						send_to_char(buf,ch);
						counter++;
					}
					send_to_char( "\n\r{DEnter hometown{W->{x ", ch);
					ch->pcdata->creation[CHOSE_HOMETOWN] = TRUE;
					d->connected = CON_GET_HOMETOWN;
					break;
				case 'E':
					counter = 1;
				    send_to_char("\n\r{DThe following Religions are available{r:\n\r", ch);
					for ( pRlg = religion_list; pRlg ; pRlg = pRlg->next )
					{	printf_to_char(ch, "{D%s {r-{D %s\n\r{x", pRlg->name, pRlg->god );
						if(pRlg->faction[FACTION_ONE] )
							printf_to_char(ch, "\t{DFaction{r:{W {D%s {r- {D%s\n\r", pRlg->faction[FACTION_ONE]->name, pRlg->faction[FACTION_ONE]->deity ); 
						if(pRlg->faction[FACTION_TWO] )
							printf_to_char(ch, "\t{DFaction{r:{W {D%s {r- {D%s\n\r", pRlg->faction[FACTION_TWO]->name, pRlg->faction[FACTION_TWO]->deity ); 
					}
					send_to_char("{DNone {r-{D None.\n\r",ch);

					write_to_buffer(d,"\n\r",0);
					send_to_char( "{DWhere do you place your faith? (help for more information)?\n\rEnter religion{W-> ", ch);
					ch->pcdata->creation[CHOSE_GOD] = TRUE;
					d->connected = CON_GET_GOD;
					break;
				case 'F':
					send_to_char("\n\r{DNow you must set your short description.\n\r\n\r",ch);
					send_to_char("Now, some guide lines for setting your short description are as follows:\n\r",ch);
					send_to_char("{D1{r:{W Please refrain from using actions within your short description.\n\r",ch);
					send_to_char("{D2{r:{W Please only put the most significant features of your character.\n\r",ch);
					send_to_char("{D3{r:{W Do not put common charactistics of your race in the short description.\n\r",ch);
					send_to_char("{D4{r:{W Make sure the description is valid in all positions, sitting, standing, ect...\n\r{x",ch);

					send_to_char("\n\r{DPlease set your short description at this time. Rember to keep it under 60 characters.\n\rEnter short descr{W->{x ",ch);
					ch->pcdata->creation[CHOSE_SHORT] = TRUE;
					d->connected = CON_SET_SHORT;
					break;
				case 'G':
					send_to_char("\n\r{DNow, which colour hair would you like?\n\r\n\r", ch);
					counter = 1;
					for(i = 0 ; i < MAX_HAIR ; i++)
					{	printf_to_char(ch, "{D%d{r:{W %s{x\n\r",counter, hair_table[i].name);
						counter++;
					}
					ch->pcdata->creation[CHOSE_ATTRIBUTES] = TRUE;
					send_to_char("\n\r{DEnter hair colour{W-> ",ch);
					d->connected = CON_GET_HAIR;
					break;
				case 'H':
					send_to_char("\n\r{DWhats your email?\n\rEnter Email{W->{x ",ch);
					d->connected = CON_GET_EMAIL;
					break;
				case 'I':
					for(i = 0 ; i < MAX_CBOOL ; i++ )
					{	if(ch->pcdata->creation[i] == FALSE)
						{	send_to_char("You must finish all selections, except E-Mail.\n\r",ch);
							send_creation_menu(ch);
							return;
						}
					}
					if(IS_SET(ch->act, PLR_REMORT))
					{	ch->gen_data = new_gen_data();
						ch->gen_data->points_chosen = ch->pcdata->points;
						add_base_groups(ch);
					}
			        add_default_groups(ch);
			        write_to_buffer( d, "\n\r", 2 );
        			write_to_buffer(d, "Please pick a weapon from the following choices:\n\r",0);
      			    buf[0] = '\0';

					ch->pcdata->learned[skill_lookup("dagger")] = 50;
				    for ( i = 0; weapon_table[i].name != NULL; i++)
        		        if (ch->pcdata->learned[skill_lookup(weapon_table[i].name)] > 0)
       			        {	strcat(buf,weapon_table[i].name);
             			    strcat(buf," ");
            			}
            			strcat(buf,"\n\rYour choice? ");
            			write_to_buffer(d,buf,0);
						d->connected = CON_PICK_WEAPON;;
						break;
				default:
					send_creation_menu(ch);
					break;
			}
			break;

/***************************************************************************************************/

		case CON_GET_EMAIL:
			if(argument[0] == '\0' )
			{   send_to_char("{DWhat was that?\n\rEnter e-mail{W->{x ",ch);
				return;
			}

			free_string(d->character->pcdata->email);
			pMail = new_email();
			free_string(pMail->name);
			pMail->name = str_dup(d->character->name);
			send_to_char("Email set.\n\r",ch);
			free_string(pMail->email);
			free_string(d->character->pcdata->email );
			d->character->pcdata->email = str_dup(argument);
			pMail->email = str_dup(argument );
			send_creation_menu(ch);
			d->connected = CON_CREATION_MENU;
			break;				

/***************************************************************************************************/

		case CON_GET_NEW_RACE:
			one_argument(argument,arg);

			if (!strcmp(arg,"help"))
			{	argument = one_argument(argument,arg);
			    if (argument[0] == '\0')
					do_function(ch, &do_help, "race help");
			    else
					do_function(ch, &do_help, argument);
		        write_to_buffer(d, "\n\r\n\r{DWhat race would you like to be (help for more information)?\n\rEnter race{W->{x  ",0);
			    break;
		  	}

			counter = 1;
			wrap = 0;
			if(is_number(argument) )
				race = atoi(argument);
			else
				race = race_lookup(argument);

			if (race == 0 || race >= MAX_PC_RACE)
			{	write_to_buffer(d,"\n\r{DThat race does not exist in these lands.\n\r",0);
		        write_to_buffer(d,"The following races here, available are{r:{x\n\r\n\r",0);
				if(race_count(ALIGN_NEUTRAL) > 0 )
				{	printf_to_char(ch, "\t{DNeutral Race%s{r:\n\r\t\t", race_count(ALIGN_NEUTRAL) == 1 ? "{r" : "s{r" );
					for(race = 1; race <= race_count(ALIGN_NEUTRAL) ; race++ )
					{	if(counter==MAX_PC_RACE)
							break;
						else
						{	if (!race_table[race].pc_race)
								break;
							printf_to_char(ch, "{D%d{r:{W %s{x", counter, race_table[race].name,0);
							if(strlen(race_table[race].name) <= 4)
								send_to_char("\t\t",ch);
							else if(strlen(race_table[race].name) > 10 )
								send_to_char(" ",ch);
							else
								send_to_char("\t",ch);
							wrap++;
							if(wrap == 3)
							{	send_to_char("\n\r",ch);
								wrap = 0;
							}
							counter++;
						}
					}
					send_to_char("\n\r\n\r",ch);
				}
					wrap = 0;			
				if(race_count(ALIGN_GOOD) > 0 )
				{	printf_to_char(ch, "\t{DBenevolant Race%s{r:\n\r\t\t", race_count(ALIGN_NEUTRAL) == 1 ? "{r" : "s{r" );
					for(; race <= (race_count(ALIGN_GOOD) + race_count(ALIGN_NEUTRAL) ) ; race++ )
					{	if(counter==MAX_PC_RACE)
							break;
						else
						{	if (!race_table[race].pc_race)
								break;
							printf_to_char(ch, "{D%d{r:{W %s{x", counter, race_table[race].name,0);
							if(strlen(race_table[race].name) <= 4)
								send_to_char("\t\t",ch);
							else if(strlen(race_table[race].name) > 10 )
								send_to_char(" ",ch);
							else
								send_to_char("\t",ch);
							wrap++;
							if(wrap == 3)
							{	send_to_char("\n\r",ch);
								wrap = 0;
							}
							counter++;
						}
					}
					send_to_char("\n\r",ch);
				}
				wrap = 0;
				if(race_count(ALIGN_EVIL) > 0 )
				{	printf_to_char(ch, "\t{DMalevolant Race%s{r:\n\r\t\t", race_count(ALIGN_NEUTRAL) == 1 ? "{r" : "s{r" );
					for(; race < MAX_PC_RACE ; race++ )
					{	if(counter==MAX_PC_RACE)
							break;
						else
						{	if (!race_table[race].pc_race)
								break;
							printf_to_char(ch, "{D%d{r:{W %s{x", counter, race_table[race].name,0);
							if(strlen(race_table[race].name) <= 4)
								send_to_char("\t\t",ch);
							else if(strlen(race_table[race].name) > 10 )
								send_to_char(" ",ch);
							else
								send_to_char("\t",ch);
							wrap++;
							if(wrap == 3)
							{	send_to_char("\n\r",ch);
								wrap = 0;
							}
							counter++;
						}
					}
					send_to_char("\n\r",ch);
				}
        
				write_to_buffer(d,"\n\r",0);
				write_to_buffer(d,"What race would you like to be (help for more information)?\n\rEnter race-> ",0);
				break;
			}

			ch->race = race;
	
			/* initialize stats */
			for (i = 0; i < MAX_STATS; i++)
				ch->perm_stat[i] = pc_race_table[race].stats[i];
			ch->affected_by = ch->affected_by|race_table[race].aff;
			ch->imm_flags	= ch->imm_flags|race_table[race].imm;
			ch->res_flags	= ch->res_flags|race_table[race].res;
			ch->vuln_flags	= ch->vuln_flags|race_table[race].vuln;
			ch->form	= race_table[race].form;
			ch->parts	= race_table[race].parts;
  	 	        ch->pcdata->boxrented  = FALSE;
			ch->pcdata->item_box  =  0;
			ch->alignment = pc_race_table[race].align;
			/* add skills */
			for (i = 0; i < 5; i++)
			{	if (pc_race_table[race].skills[i] == NULL)
				 	break;
			    group_add(ch,pc_race_table[race].skills[i],FALSE);
			}
			/* add cost */
			ch->pcdata->points = pc_race_table[race].points;
			ch->size = pc_race_table[race].size;
			send_creation_menu(ch);
			d->connected = CON_CREATION_MENU;
			break;
        
/***************************************************************************************************/

	    case CON_GET_NEW_SEX:
			switch ( argument[0] )
			{	case 'm': case 'M': ch->sex = SEX_MALE;    
					ch->pcdata->true_sex = SEX_MALE;
					break;
				case 'f': case 'F': ch->sex = SEX_FEMALE; 
				    ch->pcdata->true_sex = SEX_FEMALE;
					break;
				default:
				    send_to_char("{DHeh, whats that? Please choose one of the following:\n\r{WM{Dale or {WF{Demale\n\r\n\rEnter gender{W->{x ", ch );
				    return;
			}
			send_creation_menu(ch);
			d->connected = CON_CREATION_MENU;
			break;

/***************************************************************************************************/

	    case CON_GET_NEW_CLASS:
			if(is_number(argument) )
				iClass = atoi(argument)-1;
			else
				iClass = class_lookup(argument);
			if ( iClass <= -1 || iClass >= MAX_CLASS)
			{	send_to_char( "\n\r{DThat is not a klass on ShadowStorm, please choose one that exists.{x\n\r\n\r", ch);
				counter = 1;
				wrap = 0;
				send_to_char( "{DThese are your choices{r:{x\n\r",ch);
				for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
				{	sprintf(buf, " {D%d{r:{W ", counter);
					send_to_char(buf,ch);
					write_to_buffer(d,class_table[iClass].name,0);
					if(strlen(class_table[iClass].name) < 4)
						send_to_char("\t\t",ch);
					else if(strlen(class_table[iClass].name) > 10 )
						send_to_char(" ",ch);
					else
						send_to_char("\t",ch);
						wrap++;
					if(wrap == 3)
					{	send_to_char("\n\r",ch);
						wrap = 0;
					}
					counter++;
				}
				send_to_char("\n\r\n\r{DPlease choose one listed. You can use the name or the corresponding number.\n\rEnter class{W->{x ",ch);
				return;
			}
		    /* check to see if person is already a klass */
			if(!IS_SET(ch->act, PLR_REMORT))
			{	if(ch->pcdata->creation[CHOSE_CLASS] )
				{	send_to_char("\n\r{r*{DYou've already chosen a class! No Multiclassing till later!{r*\n\r{x\n\r",ch);
					send_creation_menu(ch);
					d->connected = CON_CREATION_MENU;				
					break;
				}
			}
			if(is_class(ch, iClass))
		    {	sprintf(buf, "You are already part %s. Try another!\n\r", class_table[iClass].name);
		        send_to_char( buf, ch);
				return;
		    }

		    /* Set the class. */
			ch->klass[number_classes(ch)] = iClass;  
			ch->klass[number_classes(ch) + 1] = -1;
		    /* Last klass is ALWAYS -1 */
			sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
			log_string( log_buf );
			wiznet("Newbie alert!  $N sighted.",ch,NULL,WIZ_NEWBIE,0,0);
		    wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));
			ch->pcdata->creation[CHOSE_CLASS] = TRUE;
			send_creation_menu(ch);
			d->connected = CON_CREATION_MENU;

			break;

/***************************************************************************************************/

	    case CON_GET_HOMETOWN:
			found = FALSE;
			if(is_number(argument) )
	        {	iClass = atoi(argument)-1;
				for (i = 0; hometown_table[i].name != NULL ; i++ )
				{	if (i == iClass)
					{	found = TRUE;
						break;
					}
				}
			}
			else
			{	iClass = get_hometown(argument); found = TRUE; }

			if (iClass <= -1 || !found)
			{	send_to_char( "\n\r{DThat's not a valid selection.\n\r", ch);
	    		send_to_char( "Valid selections are{r:{x\n\r", ch);
				counter = 1;
				for (i=0;hometown_table[i].name != NULL; ++i)
				{	sprintf(buf,"{D%d{r:{W %s\n\r{x", counter, hometown_table[i].name );
					send_to_char(buf,ch);
					counter++;
				}
    			send_to_char( "\n\r{DEnter hometown{W-> ", ch);
    			return;
	    	}
			ch->hometown = iClass;
			send_creation_menu(ch);
			d->connected = CON_CREATION_MENU;
			break;

/***************************************************************************************************/

	    case CON_GET_GOD:
		    one_argument(argument,arg);

			if (!strcmp(arg,"help"))
	        {	argument = one_argument(argument,arg);
				if (argument[0] == '\0')
					do_help(ch,"gods");
	            else
		            do_help(ch,argument);
		        send_to_char("{DWhere do you place your faith? (help for more information)?\n\rEnter religion{W-> {x", ch);
			    break;
	        }
			if(!str_cmp(argument, "None" ) )
			{	send_to_char("{DYou have chosen the path of a Heathen. May the gods {rNOT{D be with you.{x\n\r\n\r", ch);
				group_add(ch,"rom basics",FALSE);
				ch->pcdata->religion = NULL;
				add_base_groups(ch);
				ch->pcdata->learned[skill_lookup("recall")] = 50;
				write_to_buffer( d, echo_on_str, 0 );
				send_creation_menu(ch);
				d->connected = CON_CREATION_MENU;
				break;
			}


			if( ( pRlg = religion_lookup(argument) ) == NULL )
			{	if( ( pRlg = faction_lookup(argument ) ) == NULL )
				{	send_to_char("That isn't a valid Religion or Faction.\n\r",ch);
					return;
				}
			}

			printf_to_char(ch, "{DYour faith is placed in {W%s{x.\n\r\n\r", pRlg->name );
			ch->pcdata->religion = pRlg;
			group_add(ch,"rom basics",FALSE);
	                add_base_groups(ch);
	                ch->pcdata->learned[skill_lookup("recall")] = 50;
	                write_to_buffer( d, echo_on_str, 0 );
			send_creation_menu(ch);
			d->connected = CON_CREATION_MENU;
			break;


/***************************************************************************************************/

		case CON_SET_SHORT:
			if(strlen(argument) > 60)
			{	send_to_char("{DYour short description is too long, please make it something shorter.\n\rEnter short descr{W->{x ",ch);
				return;
			}

			if(str_str(argument, ch->name) )
		    {	send_to_char("{DYour name must not be included in your short description.\n\rEnter short descr{W->{x ",ch);
				return;
			}

			if(strlen(argument) < 5)
			{	send_to_char("{DPfft. It has to be longer than that!\n\rEnter short descr{W->{x ",ch);
				return;
			}

			printf_to_char(ch, "{DYour short desc has been set too{r:{W %s{x\n\r",argument);
			free_string(ch->short_descr);
			ch->short_descr = str_dup(argument);
			send_creation_menu(ch);
			d->connected = CON_CREATION_MENU;
			break;

/***************************************************************************************************/

	    case CON_GET_HAIR:
			if(is_number(argument) )
				iClass = atoi(argument)-1;
			else
				iClass = hair_lookup(argument);

			if(iClass <= -1 || iClass >= MAX_HAIR )
			{	send_to_char("\n\r{DThat is not a valid colour. Your choices are{r:{x\n\r\n\r",ch);
				counter = 1;
				for(i = 0 ; i < MAX_HAIR ; i++)
				{	printf_to_char(ch, "{D%d{r:{W %s{x\n\r",counter, hair_table[i].name);
					counter++;
				}
				send_to_char("\n\r{DEnter hair colour{W->{x ",ch);
				break;
			}

			printf_to_char(ch, "{DYour hair colour is now {W%s{D.\n\r", hair_table[iClass].name);
			ch->pcdata->hair = iClass;
			d->connected = CON_GET_EYE;
			send_to_char("{DWhich colour eyes would you like?{x\n\r\n\r",ch);
			counter = 1;
			for(i = 0 ; i < MAX_EYE ; i++)
			{	printf_to_char(ch, "{D%d{r:{W %s{x\n\r",counter, eye_table[i].name);
				counter++;
			}
			send_to_char("\n\r{DEnter eye colour{W->{x ",ch);
			break;

/***************************************************************************************************/

		case CON_GET_EYE:
			if(is_number(argument) )
				iClass = atoi(argument)-1;
			else
				iClass = eye_lookup(argument);

			if(iClass <= -1 || iClass >= MAX_EYE )
			{	send_to_char("\n\r{DThat is not a valid colour. Your choices are{r:{x\n\r\n\r",ch);
				counter = 1;
				for(i = 0 ; i < MAX_EYE ; i++)
				{	printf_to_char(ch, "{D%d{r:{W %s{x\n\r",counter, eye_table[i].name);
					counter++;
				}
				send_to_char("\n\r{DEnter eye colour{W->{x ",ch);
				break;
			}

			printf_to_char(ch, "{DYour eye colour is now {W%s{D.{x\n\r", eye_table[iClass].name);
			ch->pcdata->eye = iClass;
			d->connected = CON_GET_HEIGHT;
			send_to_char("{DHow tall would you like to be?{x\n\r\n\r",ch);

			counter = 1;
			for(i = 0 ; i < MAX_HEIGHT ; i++)
			{	printf_to_char(ch, "{D%d{r:{W %s{x\n\r",counter, height_table[i].name);
				counter++;
			}
			send_to_char("\n\r{DEnter height{W->{x ",ch);	
			break;

/***************************************************************************************************/

		case CON_GET_HEIGHT:
			if(is_number(argument) )
				iClass = atoi(argument)-1;
			else
				iClass = height_lookup(argument);

			if(iClass <= -1 || iClass >= MAX_HEIGHT )
			{	send_to_char("\n\r{DThat is not a valid height. Your choices are{r:{x\n\r\n\r",ch);
				counter = 1;
				for(i = 0 ; i < MAX_HEIGHT ; i++)
				{	printf_to_char(ch, "{D%d{r:{W %s{x\n\r",counter, height_table[i].name);
					counter++;
				}
				send_to_char("\n\r{DEnter height{W->{x ",ch);
				break;
			}

			printf_to_char(ch, "{DYour height is now {W%s{D.{x\n\r", height_table[iClass].name);
			ch->pcdata->height = iClass;
			d->connected = CON_GET_WEIGHT;
			send_to_char("{DWhat type of physique would you like?{x\n\r\n\r",ch);

			counter = 1;
			for(i = 0 ; i < MAX_WEIGHT ; i++)
			{	printf_to_char(ch, "{D%d{r:{W %s{x\n\r",counter, weight_table[i].name);
				counter++;
			}
			send_to_char("\n\r{DEnter weight{W->{x ",ch);
			break;

/***************************************************************************************************/

	    case CON_GET_WEIGHT:
			if(is_number(argument) )
				iClass = atoi(argument)-1;
			else
				iClass = weight_lookup(argument);

			if(iClass <= -1 || iClass >= MAX_WEIGHT )
			{	send_to_char("\n\r{DThat is not a valid weight. Your choices are{r:{x\n\r\n\r",ch);
				counter = 1;
				for(i = 0 ; i < MAX_WEIGHT ; i++)
				{	printf_to_char(ch, "{D%d{r:{W %s{x\n\r",counter, weight_table[i].name);
					counter++;
				}
				send_to_char("\n\r{DEnter weight{W->{x ",ch);
				break;
			}

			printf_to_char(ch, "{DYour height is now {W%s{D.{x\n\r", height_table[iClass].name);
			ch->pcdata->height = iClass;
			send_creation_menu(ch);;
			d->connected = CON_CREATION_MENU;
			break;

/***************************************************************************************************/

		case CON_DEFAULT_CHOICE:
			write_to_buffer(d,"\n\r",2);
			switch ( argument[0] )
	        {	case 'y': case 'Y': 
				    ch->gen_data = new_gen_data();
					ch->gen_data->points_chosen = ch->pcdata->points;
					do_function(ch, &do_help, "group header");
				    list_group_costs(ch);
			        if(!IS_SET(ch->act, PLR_REMORT)) /* too spammy at tier 6 or 7 */
			        {	write_to_buffer(d,"You already have the following skills:\n\r",0);
					    do_function(ch, &do_skills, "");
			        }
				    do_function(ch, &do_help, "menu choice");
				    d->connected = CON_GEN_GROUPS;
				    break;
		        case 'n': case 'N': 
				    add_default_groups(ch);
		            write_to_buffer( d, "\n\r", 2 );
				    write_to_buffer(d, "Please pick a weapon from the following choices:\n\r",0);
				    buf[0] = '\0';
				    for ( i = 0; weapon_table[i].name != NULL; i++)
						if (ch->pcdata->learned[skill_lookup(weapon_table[i].name)] > 0)
						{	strcat(buf,weapon_table[i].name);
						    strcat(buf," ");
						}
					    strcat(buf,"\n\rYour choice? ");
					    write_to_buffer(d,buf,0);
				        d->connected = CON_PICK_WEAPON;
			            break;
		        default:
		            write_to_buffer( d, "Please answer (Y/N)? ", 0 );
				    return;
	        }
			break;

/***************************************************************************************************/

	    case CON_PICK_WEAPON:
			write_to_buffer(d,"\n\r",2);
			weapon = weapon_lookup(argument);
			if (weapon == -1 || ch->pcdata->learned[skill_lookup(weapon_table[weapon].name)] <= 0)
			{	write_to_buffer(d,"That's not a valid selection. Choices are:\n\r",0);
	            buf[0] = '\0';
		        for ( i = 0; weapon_table[i].name != NULL; i++)
			        if (ch->pcdata->learned[skill_lookup(weapon_table[i].name)] > 0)
				    {	strcat(buf,weapon_table[i].name);
					    strcat(buf," ");
	                }
			        strcat(buf,"\n\rYour choice? ");
		            write_to_buffer(d,buf,0);
				    return;
			}

			ch->pcdata->learned[skill_lookup(weapon_table[weapon].name)] = 
	        UMAX(40, ch->pcdata->learned[skill_lookup(weapon_table[weapon].name)]);
			write_to_buffer(d,"\n\r",2);
			if ( IS_SET ( ch->act, PLR_REMORT ) )
		    {	write_to_buffer ( d, "Sucessful Remort!\n\r", 0 );
		        wiznet ( "Sucessful Remort!  $N sighted.", ch, NULL, 0, 0, 0 );
				REMOVE_BIT ( ch->comm, COMM_QUIET );
		        char_from_room ( ch );
		        char_to_room ( ch, get_room_index ( ROOM_VNUM_SCHOOL ) );
		        d->connected = CON_PLAYING;
	                act ( "The Priest brings you back to reality, and you feel quite different now...", ch, NULL, NULL, TO_CHAR );
			do_function ( ch, &do_pack, "Self" );
		        save_char_obj ( ch );
		        return;
		    }
			do_function(ch, &do_help, "motd");
			d->connected = CON_READ_MOTD;
			break;

/***************************************************************************************************/

	    case CON_GEN_GROUPS:
			send_to_char("\n\r",ch);

	       	if (!str_cmp(argument,"done"))
		   	{	if (ch->pcdata->points == pc_race_table[ch->race].points)
			    {	send_to_char("You didn't pick anything.\n\r",ch);
					break;
			    }
	
			    if (ch->pcdata->points <= 40 + pc_race_table[ch->race].points)
			    {	sprintf(buf, "You must take at least %d points of skills and groups", 40 + pc_race_table[ch->race].points);
					send_to_char(buf, ch);
					break;
			    }

				sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
			    send_to_char(buf,ch);
				sprintf(buf,"Experience per level: %d\n\r", exp_per_level(ch,ch->gen_data->points_chosen));
			    if (ch->pcdata->points < 40)
					ch->train = (40 - ch->pcdata->points + 1) / 2;
			    free_gen_data(ch->gen_data);
			    ch->gen_data = NULL;
				send_to_char(buf,ch);
		        write_to_buffer( d, "\n\r", 2 );
			    write_to_buffer(d, "Please pick a weapon from the following choices:\n\r",0);
	            buf[0] = '\0';
		        for ( i = 0; weapon_table[i].name != NULL; i++)
			        if (ch->pcdata->learned[skill_lookup(weapon_table[i].name)] > 0)
				    {	strcat(buf,weapon_table[i].name);
					    strcat(buf," ");
				    }
		            strcat(buf,"\n\rYour choice? ");
				    write_to_buffer(d,buf,0);
		            d->connected = CON_PICK_WEAPON;
				    break;
			}

	        if (!parse_gen_groups(ch,argument))
			    send_to_char( "Choices are: list,learned,premise,add,drop,info,help, and done.\n\r",ch);

	        do_function(ch, &do_help, "menu choice");
			    break;

/***************************************************************************************************/

		case CON_READ_IMOTD:
			write_to_buffer(d,"\n\r",2);
	        do_function(ch, &do_help, "motd");
		    d->connected = CON_READ_MOTD;
			break;

	    case CON_READ_MOTD:
		    if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
			{	write_to_buffer( d, "Warning! Null password!\n\r",0 );
				write_to_buffer( d, "Please report old password with bug.\n\r",0);
				write_to_buffer( d, "Type 'password null <new password>' to fix.\n\r",0);
	        }

			write_to_buffer( d, "\n\rWelcome to ShadowStorm Please do not feed the mobiles.\n\r", 0 );
			ch->next	= char_list;
			char_list	= ch;
			d->connected	= CON_PLAYING;
			reset_char(ch);
   
                    if ( IS_MXP( ch ) )
                    {
                        ch->pcdata->mxp = 1;
                    }
                    else
                    {
                       ch->pcdata->mxp = 0;
                    }


			if( is_clan(ch) )
			{	MEMBER_DATA *pMem;
			    if( ( pMem = member_lookup(&clan_table[ch->clan], ch->name ) ) == NULL )
			    {	pMem = new_member();
					pMem->level = ch->level;
					pMem->flevel = ch->levelflux;
					pMem->name = str_dup(ch->name);
					pMem->rank = ch->rank;
					member_to_guild(&clan_table[ch->clan], pMem);
				}
			}
		
			if ( ch->level == 0 )
			{	
                            if(ch->klass[0] == -1)
		               ch->klass[0] = 0;
			        
                            ch->perm_stat[class_table[ch->klass[0]].attr_prime] += 3;
			    ch->level	= 1;
			    ch->exp	= exp_per_level(ch,ch->pcdata->points);
			    ch->hit	= ch->max_hit;
			    ch->mana	= ch->max_mana;
			    ch->move	= ch->max_move;
			    ch->train	 = 3;
	    	     	    ch->practice = 5;
       
 	                    SET_BIT( ch->comm2, COMM_AUTOLEVEL );          
  
	 	            sprintf( buf, "The newbie\n\r");
			    send_to_char( buf, ch );

		   	    do_function( ch, &do_autoall, "on" );
			    do_function (ch, &do_pack,"Self");
                            do_function( ch, &do_divine, "self");
			    char_to_room( ch, get_room_index( hometown_table[ch->hometown].school ) );
    			    send_to_char("\n\r",ch);
			    do_function(ch, &do_help, "newbie info");
				send_to_char("\n\r",ch);
			}
			else if ( ch->in_room != NULL )
				char_to_room( ch, ch->in_room );
			else if ( IS_IMMORTAL(ch) )
				char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
			else
			    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );

			act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );

	        if (ch->mount != NULL)
                {	
                    char_to_room(ch->mount,ch->in_room);
	            act("$n has entered the game.",ch->mount,NULL,NULL,TO_ROOM);
		    add_follower(ch->mount, ch);
		    do_mount(ch, ch->mount->name);
	        }
	
	        send_to_char("\n", ch);
		do_unread(ch, "");  
         
	        if (ch->pcdata->host == NULL
		    || sizeof(ch->pcdata->host) == 0
			|| !str_cmp(ch->pcdata->host, "none"))
	        {	
                    sprintf(buf, "\n\r{BUpdating site login info{x: {W%s{x\n\r\n\r", d->host);
	            send_to_char(buf, ch);
		}
			
                else if (!str_cmp(ch->pcdata->host, d->host))
	        {	  
                    sprintf(buf, "\n\r{BUsual login{x: {W%s{x\n\r\n\r", ch->pcdata->host);
	            send_to_char(buf, ch);
                }       
		
	        else
	        {	
                    sprintf(buf, "\n\r{BLast login{x: {W%s{x\n\r", ch->pcdata->host);
	            send_to_char(buf, ch);
		    sprintf(buf, "{BCurrent login{x: {W%s{x\n\r\n\r", d->host);
		    send_to_char(buf, ch);
	        }
       
                free_string( ch->pcdata->host );
		ch->pcdata->host = str_dup(d->host);
		do_function(ch, &do_look, "auto" );
		wiznet("{B[LOGIN]{x: {R$N has left real life behind.{x",ch,NULL, WIZ_LOGINS,WIZ_SITES,get_trust(ch));
                count_connections++;

#ifdef MCCP_ENABLED
          if (ch->desc->out_compress)
               send_to_char("\n\r{G[INFO]:{x {BMCCP Compression ACTIVE! Thank you for saving us bandwidth.{x\n\r", ch);
#endif

                for ( vch = char_list; vch != NULL; vch = vch->next )
	        {      
                     if (vch->in_room == ch->in_room)
	                 continue;
                         
                     if (IS_SET(vch->comm, COMM_PENTER) && ch->level >= vch->invis_level) 
		         act("{G[INFO]:{x {R$n has decided to leave real life behind.{x",ch,0,vch,TO_VICT);
                }

		if (ch->pet != NULL)
		{	
                    char_to_room(ch->pet,ch->in_room);
		    act("$n has entered the game.",ch->pet,NULL,NULL,TO_ROOM);
		}

		if ( ch->pcdata->confirm_delete == TRUE )
	            ch->pcdata->confirm_delete = FALSE;
	        
	        if (IS_SET( ch->comm2, COMM_AUTO_AFK ) )  
	            REMOVE_BIT( ch->comm2, COMM_AUTO_AFK );  

		unfinished_quest( ch );

		break;
	    }

    return;
}


void send_creation_menu(CHAR_DATA *ch)
{	printf_to_char(ch, "{DWelcome to the ShadowStorm creation menu, {W%s\n\r", ch->name);
	send_to_char("{D==================================={x\n\r",ch);
	printf_to_char(ch,"{DA{r:{W Race Selection      {r- {W%s\n\r", ch->pcdata->creation[CHOSE_RACE] == TRUE ? pc_race_table[ch->race].name : "Not Chosen" );
	printf_to_char(ch,"{DB{r:{W Class Selection     {r- {W%s\n\r", ch->pcdata->creation[CHOSE_CLASS] == TRUE ? class_long(ch) : "Not Chosen" );
	printf_to_char(ch,"{DC{r:{W Gender Selection    {r- {W%s\n\r", ch->pcdata->creation[CHOSE_GENDER] == TRUE ? sex_table[ch->sex].name : "Not Chosen" );
	if(!IS_SET(ch->act, PLR_REMORT))
	{	printf_to_char(ch,"{DD{r:{W Hometown Selection  {r- {W%s\n\r", ch->pcdata->creation[CHOSE_HOMETOWN] == TRUE ? hometown_table[ch->hometown].name : "Not Chosen");
		printf_to_char(ch,"{DE{r:{W Diety Selection     {r- {W%s\n\r", ch->pcdata->creation[CHOSE_GOD] == TRUE ? "Set" : "Not Chosen" );
	}
	printf_to_char(ch,"{DF{r:{W Short Description   {r- {W%s\n\r", ch->pcdata->creation[CHOSE_SHORT] == TRUE ? ch->short_descr : "Not Chosen" );
	printf_to_char(ch,"{DG{r:{W Attribute Selection {r- {W%s{x\n\r", ch->pcdata->creation[CHOSE_ATTRIBUTES] == TRUE ? "Set" : "Not Chosen" );
    printf_to_char(ch,"{DH{r:{W E-Mail Address      {r- {W%s{x\n\r", email_lookup(ch->name) == NULL ? "Not Set" : ch->pcdata->email );
         send_to_char("{DI{r:{W Done",ch);
	send_to_char("\n\r{DPlease make a selection based off the corresponding letter.\n\rEnter selection{W->{x ",ch);
	return;
}

int race_count(int align)
{	
        int race, count = 0;

	for(race = 0; race < MAX_PC_RACE ; race++ )
	{	
            if(pc_race_table[race].align == align )
	       count++;
	}

	return count;
}
