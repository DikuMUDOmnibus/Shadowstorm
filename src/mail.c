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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "include.h"       

void      mail_home     args( ( CHAR_DATA *ch, CHAR_DATA *victim, MAIL_DATA *mAil, 
                           OBJ_DATA *obj ) );
void      mail_office   args( ( CHAR_DATA *ch, CHAR_DATA *victim, MAIL_DATA *mail,
                           OBJ_DATA *obj ) );
#define   MAX_NEST      100
#define   MAX_RECIEVED  5
void      save_mail     args( ( void ) );
void      fread_char    args( ( CHAR_DATA *ch,  FILE *fp ) );   
int       recieved_mail args( ( CHAR_DATA * victim) );
int       get_mail_id   args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
MAIL_DATA *mail_list;  
static    OBJ_DATA *    rgObjNest [MAX_NEST];               

#if defined(KEY)
#undef KEY
#endif

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )  if ( !str_cmp( word, literal ) ) { field  = value; fMatch = TRUE; break; }

struct postal_type *postal_table;   

int last_mail_id;
long last_mail_time;
int maxpostal;

void do_post( CHAR_DATA *ch, char *argument )
{
  char arg[MSL];
  char arg1[MSL];
  char buf[MSL];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  MAIL_DATA *mAil;
  ROOM_INDEX_DATA *location;

  if ( arg[0] == '\0' || arg1[0] == '\0' )
  {
      send_to_char( "Only objects may be posted at this time.\n\r", ch );
      send_to_char( "Syntax: post <item> <player>\n\r", ch );
      return;
  }

  location = get_room_index( hometown_table[ch->hometown].postal );

  if ( ch->in_room != location )
  {
      send_to_char( "You must be in the post office to send mail.\n\r", ch );
      return;
  }

  if ( IS_NPC( ch ) )
  {
      send_to_char( "Mobs don't need to send anyone anything.\n\r", ch );
      return;
  }

  if ( ch->fighting != NULL )
  {
      send_to_char( "Not while your fighting!\n\r", ch );
      return;
  }

  if ( ( obj =  get_obj_list(ch, arg, ch->carrying) ) == NULL )
  {
        send_to_char( "You don't have that item.\n\r", ch );
        return;
  }
 
  /*
   *  If person is on then see if they have a house. If they do send the item
   *  there. If not then we'll have to send it to the office.  
   */
 
  /*
   * If they aren't on then we'll have to access their file, and see if they
   * have a house. 
   */

  if ( ( victim = get_char_world( ch, arg1 ) ) != NULL )
  {
      if ( recieved_mail( victim ) > MAX_RECIEVED )
      {
          send_to_char( "They already have five items in their mailbox.\n\r", ch );
          return;
      }

      for ( mAil = mail_list; mAil != NULL; mAil = mAil->next );    
      {
           if ( HAS_HOME( victim ) )
           {
               mail_home( ch, victim, mAil, obj );
           }
  
           else
           {
               mail_office( ch, victim, mAil, obj );
           }
      }
      return;
  }  

  else
  {
    FILE *fp;
    bool fOld;

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

    if ( IS_NPC(victim) || victim->pcdata == NULL )
    {
        send_to_char( "Error in loading pcdata.\n\r", ch );
        free_pcdata(victim->pcdata);
        free_char(victim);
        return;
    }   

    if ( recieved_mail( victim ) > 5 )
    {
        send_to_char( "They already have five items in their mailbox.\n\r", ch );
        return;
    }    

    for ( mAil = mail_list; mAil != NULL; mAil = mAil->next );
    {
         if ( HAS_HOME( victim ) )
         {
             mail_home( ch, victim, mAil, obj );
         }

         else
         {
             mail_office( ch, victim, mAil, obj );
         }
    }              

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
            bug( "Character not found.", 0 );
            return;
        }
    }

    free_pcdata(victim->pcdata);
    free_char(victim);
    return;
  }
}

void mail_home( CHAR_DATA *ch, CHAR_DATA *victim, MAIL_DATA *mAil, OBJ_DATA *obj )
{
  char *strtime;
  int i;
  int days = 5;

  mAil = new_mail();
  mAil->sender = str_dup( ch->name );
  mAil->to_who = str_dup( victim->name );
  mAil->item = obj;

  for (i = 0; house_table[i].name != NULL; i++)
  {
       if ( is_name( "box", house_table[i].name ) && house_table[i].type == OBJ_VNUM )
       {
           mAil->box = house_table[i].vnum;
       }
  }

  mAil->id  = get_mail_id(ch, victim);

  if (last_mail_time >= current_time)
      mAil->date_stamp = ++last_mail_time;

  else
  {
      mAil->date_stamp = current_time;
      last_mail_time  = current_time;
  }            

  mAil->expire = days * 60 * 60 * 24;
  strtime = ctime(&current_time);
  strtime[strlen(strtime) - 1] = '\0';
  mAil->date = str_dup(strtime);                 
  printf_to_char( ch, "The messenger rides up on their horse, you hand them %s, and they ride off again.\n\r", obj->name );
  mAil->next = mail_list;
  mail_list = mAil;
  save_mail();      
}

void mail_office( CHAR_DATA *ch, CHAR_DATA *victim, MAIL_DATA *mAil, OBJ_DATA *obj )
{
  char *strtime;
  int days = 5;

  mAil = new_mail();
  mAil->sender = str_dup( ch->name );
  mAil->to_who = str_dup( victim->name );
  mAil->box = hometown_table[victim->hometown].postal;
  mAil->item = obj;
  mAil->id = get_mail_id(ch, victim);

  if (last_mail_time >= current_time)
      mAil->date_stamp = ++last_mail_time;

  else
  {
      mAil->date_stamp = current_time;
      last_mail_time  = current_time;
  }

  mAil->expire = days * 60 * 60 * 24;
  strtime = ctime(&current_time);
  strtime[strlen(strtime) - 1] = '\0';
  mAil->date = str_dup(strtime);          
  send_to_char( "You hand the messenger your item, and watch as he rides off to deliver it.\n\r", ch );
  mAil->next = mail_list;
  mail_list = mAil;
  save_mail();              
}

int get_mail_id(CHAR_DATA *ch, CHAR_DATA *victim)
{
   MAIL_DATA *mail;

   for (mail = mail_list; mail; mail = mail->next)
   {
        if (mail->id == last_mail_id || ( mail->id != ch->pcdata->mailid && mail->id != victim->pcdata->mailid ) ) 
            last_mail_id++;
   }

   victim->pcdata->mailid = last_mail_id;
   ch->pcdata->mailid = last_mail_id;

   return last_mail_id;
}                

int recieved_mail(CHAR_DATA * victim)
{
   MAIL_DATA *mail;
   int count;

   for (count = 0, mail=mail_list ; mail; mail = mail->next)
   {
      if (!str_cmp( mail->to_who, victim->name ) )
          count++;
   }

   return count;
}               

MAIL_DATA *mail_free;

MAIL_DATA *new_mail(void)
{
    static MAIL_DATA mail_zero;
    MAIL_DATA *mAil;

    if (mail_free == NULL)
        mAil = (MAIL_DATA *)alloc_perm(sizeof(*mAil));
    else
    {
        mAil = mail_free;
        mail_free = mail_free->next;
    }

    *mAil = mail_zero;
    VALIDATE(mAil);
    return mAil;
}                              

void free_mail(MAIL_DATA *mAil)
{
    if (!IS_VALID(mAil))
        return;

    free_string( mAil->sender );
    free_string( mAil->to_who );
    INVALIDATE(mAil);
    mAil->next = mail_free;
    mail_free = mAil;               
}

void save_mail()
{
    FILE *fp;
    MAIL_DATA *mAil;

    if ( ( fp = file_open( MAIL_FILE, "w" ) ) == NULL )
    {
        logf2("The mail file is gone!\n\r");
        return;
    }      
    
    for(mAil = mail_list ; mAil ; mAil = mAil->next )
    {	
	fprintf(fp, "Sender %s~\n", mAil->sender );
	fprintf(fp, "Reciever %s~\n", mAil->to_who );
	/* I'd say here, you should fwrite_obj(), then on the load use fread_obj(). 
	   And I'm pretty sure I already made it so it can handle a NULL CHAR_DATA struct. 
	                                                    ~Davion */
    }	
        
    fprintf(fp, "$\n");
    logf2("Mail saved.");
    file_close(fp);
}

void do_chmail( CHAR_DATA *ch, char *argument )
{
   ROOM_INDEX_DATA *location;
   MAIL_DATA *mail;
   bool found = FALSE;

   location = get_room_index( hometown_table[ch->hometown].postal );

   if ( IS_NPC( ch ) )
   {
       send_to_char( "Mobs can't recieve objects in the mail.\n\r", ch );
       return;
   }

   if ( ch->in_room != location )
   {
       send_to_char( "You must be in the post office to check your mail.\n\r", ch );
       return;
   }
   
   if ( argument[0] == '\0' )
   {
       send_to_char( "Chmail with no argument shows what is in the mail.\n\r", ch );
       send_to_char( "Name		Sender", ch );
       send_to_char( "==================================================\n\r", ch );     
       for ( mail = mail_list; mail != NULL; mail = mail->next )
       {
            if( !str_cmp(ch->name,mail->to_who) )       
            {
               printf_to_char( ch, "%-10s	%-10s\n\r", mail->item->short_descr, mail->sender );
               found = TRUE;
            }
       }

       if ( !found )
           send_to_char( "You have no object in your mail.\n\r", ch );
		
       return;
   }
}

void load_postal(FILE *fp, struct postal_type *postal)
{
    const char *word;
    bool fMatch;

    for (;;)
    {
        word = feof (fp) ? "End" : fread_word (fp);
        fMatch = FALSE;

        switch (UPPER (word[0]))
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol (fp);
                break;

                case 'P':
                    KEY("Pcount", postal->pcount, fread_number(fp) );
                    break;

		case 'V':
		    KEY("Vnum", postal->vnum, fread_number(fp) );
		    break;

		case 'E':
                
                    if (!str_cmp (word, "End"))
                    return;

		}
	}
	return;
}

void load_postal_table()
{
    FILE *fp;
    int i;
	
    fp = file_open (POSTAL_FILE, "r");
	
    if (!fp)
    {
	bug ("Could not open " POSTAL_FILE " for reading.",0);
	exit(1);
    }
	
    fscanf (fp, "%d\n", &maxpostal);

    postal_table = (postal_type *)malloc (sizeof(struct postal_type) * (maxpostal+1));
	
    for (i = 0; i < maxpostal; i++)
    {
         postal_table[i].pcount     = 0;
         postal_table[i].vnum       = 0;
         load_postal(fp,&postal_table[i]);
    }

    file_close (fp);
}

void save_postal(const struct postal_type *postal, FILE *fp)
{
    fprintf(fp, "Vnum %d\n", postal->vnum);
    fprintf(fp, "END\n\n" );
    return;
}

void save_postal_table()
{
   FILE *fp;
   int i;
	
   fp = file_open (POSTAL_FILE, "w");
	
   fprintf (fp, "%d\n", maxpostal);
	
   for ( i = 0 ; i < maxpostal ; i++)
        save_postal(&postal_table[i], fp);
	
   file_close (fp);
}

int postal_lookup (int vnum)
{
    int i;

    for (i = 0; i < maxpostal; i++)
    {
         if ( vnum == postal_table[i].vnum )
             return i;
    }

    return -1;
}

void new_postal( CHAR_DATA *ch )
{
    struct postal_type *new_table;
    int ipostal;

    if ( !IS_SET( ch->in_room->room_flags, ROOM_POSTAL ) )
    {
        ;

        if ( (ipostal = postal_lookup( ch->in_room->vnum ) ) != -1 )
        {
            send_to_char( "That post office already exist.\n\r", ch );
            return;
        }
		
        ipostal = ch->in_room->vnum;
        maxpostal++;
     
        new_table = (postal_type *)realloc (postal_table, sizeof(struct postal_type) * maxpostal + 1);
		
        if (!new_table)
        {
            send_to_char( "Memory allocation failed.\n\r", ch );
            maxpostal--;
            return;
        }

        SET_BIT( ch->in_room->room_flags, ROOM_POSTAL );
        postal_table = new_table;
        postal_table[maxpostal-1].pcount++;
        postal_table[maxpostal-1].vnum = ipostal;             
        printf_to_char(ch, "A new post office has been added in room %d maxpostal is %d.\n\r", ipostal, maxpostal );
        save_postal_table();
        return;
    }

    else
    {
        int i,j;

        struct postal_type *new_table = (postal_type *)malloc (sizeof(struct postal_type) * maxpostal);
		
        if (!new_table)
        {
            send_to_char( "Memory allocation failed.\n\r", ch );
            return;
        }

        ipostal = postal_lookup( ch->in_room->vnum );

        if ( ipostal == -1 )
        {
            send_to_char( "That post office doesn't exist.\n\r", ch );
            return;
        }

        REMOVE_BIT( ch->in_room->room_flags, ROOM_POSTAL );

        for (i = 0, j = 0; i < maxpostal+1; i++)
	{	 
             if (i != ipostal) 
             {
                 new_table[j] = postal_table[i];
                 j++;
             }
        }

        printf_to_char( ch, "You are deleting post office %d.\n\r", ipostal );
        free(postal_table);
        postal_table = new_table;
        maxpostal--; 
        save_postal_table();
        return;
    }

    return;
}

void do_pshow( CHAR_DATA *ch, char *argument )
{
    int i;
    bool found = FALSE;
    
    send_to_char( "Vnum:\n\r", ch );
    send_to_char( "{G====={x\n\r", ch );
    for ( i = 0; i < maxpostal; i++ )
    {
         printf_to_char( ch, "[ %-5d ]\n\r", postal_table[i].vnum); 
         found = TRUE;
    }    

    if (!found)
    send_to_char("Ack their are no post offices have an imm make one.\n\r", ch );
    return;
}

void load_mail(FILE *fp, MAIL_DATA *mail)
{
    const char *word;
    bool fMatch;
    MAIL_DATA *mlist;

    if (file_exists(MAIL_FILE) )
    {
        fp = file_open( MAIL_FILE, "r" );
 
        mlist = NULL;   

        for (;;)
        {
             word = feof (fp) ? "End" : fread_word (fp);
             fMatch = FALSE;

             switch (UPPER (word[0]))
             {
                 case '*':
                    fMatch = TRUE;
                    fread_to_eol (fp);
                    break;

		 case 'S':
                    mail = new_mail();
                    if (mail_list == NULL)
                    mail_list = mail;
                    else
                    mlist->next = mail;
                    mlist = mail;
		    KEY("Sender", mail->sender, fread_string(fp) );

 	 	 case 'E':
                    if (!str_cmp (word, "End"))
                    return;
		    break;

                 case 'R':
                    KEY("Reciever", mail->to_who, fread_string(fp) );
                    break;
		}
	}

    }
	
    return;
}

