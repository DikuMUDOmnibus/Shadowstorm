/****************************************************************************
 * Multi-Automated Auction Code                                             *
 * Markanth : dlmud@dlmud.com                                               *
 * Devil's Lament : dlmud.com port 3778                                     *
 * Web Page : http://www.dlmud.com                                          *
 *                                                                          *
 * Provides automated support for multiple auctioned items.                 *
 * Includes advanced number parser by                                       *
 * Erwin S. Andreasen, erwin@andreasen.org                                  *
 *                                                                          *
 * All I ask in return is that you give me credit on your mud somewhere     *
 * or email me if you use it.                                               *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "include.h"


AUCTION_DATA *auction_list;

AUCTION_DATA *auction_lookup(sh_int num)
{
    AUCTION_DATA *pAuc;

    for(pAuc = auction_list; pAuc != NULL; pAuc = pAuc->next)
    {
        if(pAuc->number == num)
            return pAuc;
    }
    return NULL;
}

int last_auc_id;

int get_auc_id (void)
{
    last_auc_id++;

   if( last_auc_id > 999)
       last_auc_id = 1;

   return last_auc_id;
}

void do_auction(CHAR_DATA *ch, char *argument)
{
   AUCTION_DATA *auc;
   AUCTION_DATA *p;
   OBJ_DATA    *obj = NULL;
   long         minbid = 1;
   char         arg1[MAX_INPUT_LENGTH];
   char         arg2[MAX_INPUT_LENGTH];
   char         buf[MAX_STRING_LENGTH];

   argument = one_argument (argument, arg1);
   argument = one_argument (argument, arg2);

   if (ch == NULL)
      return;

   if(arg1[0] == '\0')
    {
       if(IS_SET(ch->comm, COMM_NOAUCTION))
        {
           REMOVE_BIT(ch->comm, COMM_NOAUCTION);
           send_to_char("AUCTION channel is now ON.\n\r", ch);
        }
        else
        {
            SET_BIT(ch->comm, COMM_NOAUCTION);
            send_to_char("AUCTION channel is now OFF.\n\r", ch);
        }
        return;
    }

    else if (!str_cmp (arg1, "talk"))
    {
       /* nice hack here */
       if(argument[0] != '\0')
       {
          strcat(arg2, " ");
          strcat(arg2, argument);
       }

       do_auction_talk(ch, argument);
       return;
   }

   else if (!str_cmp (arg1, "stop") && IS_IMMORTAL (ch))
   {

       if(arg2[0] == '\0' || !is_number(arg2))
       {
            send_to_char("Stop which auction?\n\r", ch);
            return;
       }

       if((auc = auction_lookup(atoi(arg2))) == NULL)
       {
            send_to_char("No such auction.\n\r", ch);
            return;
       }
 
      sprintf(buf, "$n has stopped the auction and confiscated %s!", auc->item->short_descr);
      auction_channel (ch, buf);
      sprintf(buf, "AUCTION: You stop the auction and confiscate %s.\n\r", auc->item->short_descr);
      send_to_char(buf, ch);
      obj_from_char(auc->item);
      obj_to_char(auc->item, ch);
      reset_auc(auc, TRUE);
      return;
   }

   if (!str_cmp( arg1, "sell" ) )
   {    
       if ( arg2[0] == '\0' )
       {
           send_to_char( "Please enter a minimum bid.\n\r", ch );
           return;
       }

       if (!HAS_HOME(ch ) )
       {
           send_to_char( "If you only had a home.\n\r", ch );
           return;
       }

      if (count_auc (ch) >= 3)
      {
          send_to_char ("You are only allowed to auction 3 items at a time!\n\r", ch);
          return;
      }
 
      if (arg2[0] != '\0')
          minbid = atol (arg2);

      if (minbid > 500000)
      {
          send_to_char ("Minimum bids can't be higher than 500000.\n\r", ch);
          return;
      }

      if (auction_list == NULL)
      {
          auc = new_auction ();
          auction_list = auc;
          auction_list->next = NULL;
      } 
      else
      {
          auc = new_auction ();

          for (p = auction_list; p; p = p->next)
          {
	       if (p->next == NULL)
	       p->next = auc;
               auc->next = NULL;
          }
     }

     auc->owner = ch;
     auc->bid = minbid;
     auc->number = get_auc_id();
     auc->status = 0;
     auc->house = ch;
     auc->ishouse = TRUE;
   
     sprintf( buf, "%s is auctioning their house (Num %d). Current bid is %ld.\n\r", ch->name, auc->number, auc->bid );
     auction_channel(ch, buf);
     printf_to_char(ch, "AUCTION: You are auctioning your house (Num %d). Current bid is %ld.", auc->number, auc->bid);
     return;
      
   }

   if ((obj = get_obj_carry (ch, arg1, ch)) == NULL)
   {
      send_to_char ("You aren't carrying that item.\n\r", ch);
      return;
   }

   if (IS_OBJ_STAT (obj, ITEM_AUCTIONED))
   {
      send_to_char ("That items is already being auctioned.\n\r", ch);
      return;
   }

   if (IS_OBJ_STAT (obj, ITEM_NODROP))
   {
      send_to_char ("You can't let go of that item.\n\r", ch);
      return;
   }

   if (IS_OBJ_STAT (obj, ITEM_NO_AUCTION))
   {
      send_to_char ("That item cannot be auctioned.\n\r", ch);
      return;
   }

   if (obj->item_type == ITEM_CORPSE_PC
       || obj->item_type == ITEM_CORPSE_NPC)
   {
      send_to_char ("Not a good idea....\n\r", ch);
      return;
   }

   if (count_auc (ch) >= 3)
   {
      send_to_char ("You are only allowed to auction 3 items a time!\n\r", ch);
      return;
   }

   if (arg2[0] != '\0')
      minbid = atol (arg2);

   if (minbid > 500000)
   {
      send_to_char ("Minumum bids can't be higher than 500000.\n\r", ch);
      return;
   }

   if (auction_list == NULL)
   {
      auc = new_auction ();
      auction_list = auc;
      auction_list->next = NULL;
   }
   else
   {
      auc = new_auction ();

      for (p = auction_list; p; p = p->next)
      {
	 if (p->next == NULL)
	    p->next = auc;
	 auc->next = NULL;
      }
   }

   SET_BIT (obj->extra_flags, ITEM_AUCTIONED);
   auc->owner = ch;
   auc->item = obj;
   auc->bid = minbid;
   auc->number = get_auc_id();
   auc->status = 0;

   sprintf(buf, "$n is auctioning %s (Level %d, Num %d). Current bid is %ld.",
   auc->item->short_descr, auc->item->level, auc->number, auc->bid);
   auction_channel(ch, buf);
   sprintf(buf, "AUCTION: You are auctioning %s (Level %d, Num %d). Current bid is %ld.",
   auc->item->short_descr, auc->item->level, auc->number, auc->bid);
   send_to_char(buf, ch);
   return;
}

void auction_update (void)
{
   AUCTION_DATA *auc;
   char buf[MAX_STRING_LENGTH];

   for (auc = auction_list; auc != NULL; auc = auc->next)
   {
      auc->status++;

     if(!auc->item && !auc->ishouse)
     {
         reset_auc(auc, TRUE);
     }
     
     if ( auc->ishouse )
     {
         sprintf( buf, "{wAUCTION: Item  #%d: %s's house going %s", auc->number, auc->house->name, auctn_table[auc->status-1].name);
         do_function( NULL, &do_echo, buf );
     }
 
     else
     {
        sprintf(buf, "{wAUCTION: Item #%d: %s going %s",auc->number,auc->item->short_descr, auctn_table[auc->status-1].name);
        do_function(NULL, &do_echo, buf);
     }

     if (auc->status == AUCTION_LENGTH)
     {
         if (auc->high_bidder == NULL)
         {
             reset_auc (auc, TRUE);
         }
        
         else if (auc->high_bidder->gold < auc->bid)
         {
             send_to_char("You can't cover your bid in the auction anymore!\n\r", auc->high_bidder);
             sprintf(buf, "%s can't cover the bid anymore, sale stopped.\n\r", GET_NAME( auc->high_bidder));
             send_to_char(buf, auc->owner);
             reset_auc(auc, TRUE);
         }
         else
         {
             if ( auc->ishouse )
             {
                 sprintf(buf, "%s SOLD their house to %s for %ld gold.", auc->house->name, GET_NAME(auc->high_bidder), auc->bid);
                 auction_channel(NULL, buf);
             }
               
             else
             { 
                sprintf(buf, "%s SOLD to %s for %ld gold.", auc->item->short_descr,
                GET_NAME (auc->high_bidder), auc->bid);
                auction_channel(NULL, buf);
             }

             reset_auc(auc, FALSE);
         }
    }
   }
   return;
}

void reset_auc (AUCTION_DATA * auc, bool forced)
{
   char buf[MSL];

   if (!IS_VALID (auc))
      return;

   if ( auc->ishouse )      
   {
       if (!forced && auc->high_bidder != NULL && auc->bid > 0)
       {
           auc->owner->gold += (auc->bid * 9) / 10;
           auc->high_bidder->gold -= auc->bid;
       
           sprintf(buf, "You recieve %ld gold for the sale of your house.\n\r", (auc->bid * 9) / 10 );
           send_to_char( buf, auc->owner);
           sprintf( buf, "You are sold %s's house for %ld gold.\n\r", auc->owner->name, auc->bid );
           send_to_char(buf, auc->high_bidder);
           auc->high_bidder->pcdata->h_vnum = auc->owner->pcdata->h_vnum;
           auc->owner->pcdata->h_vnum = 0;
           auc->ishouse = FALSE;
           auc->house = NULL;
       }

       else if (auc->owner != NULL)
       {
           sprintf(buf, "Sale of %s's house stopped.\n\r", auc->house->name );
           send_to_char(buf, auc->owner);
           auc->house = NULL;
           auc->ishouse = FALSE;
       }
   }

   else if (auc->item != NULL)
   {
       if (IS_OBJ_STAT (auc->item, ITEM_AUCTIONED) && auc->house == NULL )
           REMOVE_BIT (auc->item->extra_flags, ITEM_AUCTIONED);
       else
         bug ("Reset_auction: item not flagged auction item", 0);
 
       if (!forced && auc->high_bidder != NULL && auc->bid > 0)
       {
           auc->owner->gold += (auc->bid * 9) / 10;
           auc->high_bidder->gold -= auc->bid;

           sprintf (buf, "You recieve %ld gold for the sale of %s.\n\r", (auc->bid * 9) / 10, auc->item->short_descr);
           send_to_char(buf, auc->owner);
           obj_from_char (auc->item);
           obj_to_char (auc->item, auc->high_bidder);
           sprintf (buf, "You are sold %s for %ld gold.\n\r", auc->item->short_descr, auc->bid);
           send_to_char(buf, auc->high_bidder);
           auc->item = NULL;
       }
       
       else if (auc->owner != NULL)
       {
           sprintf (buf, "Sale of %s stopped.\n\r", auc->item->short_descr);
           send_to_char(buf, auc->owner);
           auc->item = NULL;
       }
   }
       
   auc->bid = 0;
   auc->high_bidder = NULL;
   auc->owner = NULL;
   auc->status = 0;
   auc->number = 0;

   if (auc == auction_list)
   {
      if (auc->next != NULL)
         auction_list = auc->next;
      else
         auction_list = NULL;

      free_auction (auc);
      return;
   }

   free_auction (auc);
   return;
}

int count_auc (CHAR_DATA * ch)
{
   AUCTION_DATA *q;
   int          count;

   q = auction_list;

   if (!q)
      return 0;

   for (count = 0; q; q = q->next)
   {
      if (q->owner == ch)
	 count++;
   }

   return count;
}

long advatoi (const char *s)
{
   char         string[MAX_INPUT_LENGTH];
   char        *stringptr = string;
   char         tempstring[2];
   long         number = 0;
   long         multiplier = 0;

   strcpy (string, s);

   while (isdigit (*stringptr))
   {
      strncpy (tempstring, stringptr, 1);
      number = (number * 10) + atol (tempstring);
      stringptr++;
   }

   switch (UPPER (*stringptr))
   {
   case 'K':
      multiplier = 1000;
      number *= multiplier;
      stringptr++;
      break;
   case 'M':
      multiplier = 1000000;
      number *= multiplier;
      stringptr++;
      break;
   case '\0':
      break;
   default:
      return 0;

   }

   while (isdigit (*stringptr) && (multiplier > 1))
   {
      strncpy (tempstring, stringptr, 1);

      multiplier = multiplier / 10;
      number = number + (atol (tempstring) * multiplier);
      stringptr++;
   }

   if (*stringptr != '\0' && !isdigit (*stringptr))

      return 0;

   return (number);
}

long parsebet (const long currentbet, const char *argument)
{
   long         newbet = 0;
   char         string[MAX_INPUT_LENGTH];
   char        *stringptr = string;

   strcpy (string, argument);

   if (*stringptr)
   {
      if (isdigit (*stringptr))
	 newbet = advatoi (stringptr);
      else if (*stringptr == '+')
      {
	 if (strlen (stringptr) == 1)
	    newbet = (currentbet * 125) / 100;
	 else
	    newbet = (currentbet * (100 + atoi (++stringptr))) / 100;
      }
      else
      {
	 printf ("considering: * x \n\r");

	 if ((*stringptr == '*') || (*stringptr == 'x'))
	 {

	    if (strlen (stringptr) == 1)
	       newbet = currentbet * 2;
	    else
	       newbet = currentbet * atoi (++stringptr);
	 }
      }
   }

   return newbet;

}


void do_bid(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    AUCTION_DATA *auc;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (auction_list == NULL)
    {
	send_to_char ("There's nothing up for auction right now.\n\r", ch);
	return;
    }

    if(arg1[0] == '\0')
    {
      send_to_char("Num   Seller       Item Description                    Lvl    Last Bid   Time\n\r", ch);
      send_to_char("---   ------------ ----------------------------------- --- ------------- ----\n\r", ch);
      for (auc = auction_list; auc; auc = auc->next)
      {
	    if (!auc->ishouse && !IS_OBJ_STAT (auc->item, ITEM_AUCTIONED))
	        bug ("Auctioned item is not flagged Auctioned.", 0);

        if ( !auc->ishouse )
        {         
            sprintf(buf, "%3d - %-12s %35s %3d %13ld %4d\n\r", auc->number, GET_NAME(auc->owner), auc->item->short_descr, auc->item->level, auc->bid, auc->status);
            send_to_char(buf, ch);
        }
           
        else
        {
            sprintf(buf, "%3d - %-12s %35s's house %13ld %4d\n\r", auc->number, GET_NAME( auc->owner ), auc->house->name, auc->bid, auc->status);
            send_to_char(buf, ch);
        }
      }
      send_to_char("------------------------------------------------------------------------------\n\r", ch);
        send_to_char("Type: 'Bid <num>' to see stats and 'Bid <num> <amount>' to bid on an item.\n\r", ch);
      return;
    }
   else if ((auc = auction_lookup(atoi(arg1))) != NULL)
   {
        if(arg2[0] == '\0')
        {
          if (ch == auc->owner && !IS_IMMORTAL (ch))
          {
             if ( !auc->ishouse )
             {
                 sprintf(buf, "You're auctioning %s.\n\r", auc->item->short_descr);
                 sprintf(buf, "Current bid is %ld gold.\n\r", auc->bid);
                 return;
             }
             else
             { 
                sprintf(buf, "You're auctioning your house.\n\r" );
                sprintf(buf, "Current bid is %ld gold.\n\r", auc->bid );
                return;
             }                  
          }

          if ( !auc->ishouse )
          {
              spell_identify (0, ch->level, ch, auc->item, TAR_OBJ_INV);
              sprintf(buf, "Current bid is %ld gold.\n\r", auc->bid);
              send_to_char(buf, ch);
              return;
          }
          else 
          {
              do_function( ch, &do_homeb, "value" );
              sprintf(buf, "Current bid is %ld gold.\n\r", auc->bid );
              return;
          }
       }
       else
       {
          long         bid = 0;

          if (ch == auc->high_bidder)
          {
              send_to_char ("You already have the highest bid!\n\r", ch);
              return;
          }

          if (ch == auc->owner)
          {
             send_to_char ("You cannot bid on your own items!\n\r", ch);
             return;
          }

          bid = parsebet (auction_list->bid, arg2);

          if (ch->gold < bid)
          {
              send_to_char ("You can't cover that bid.\n\r", ch);
              return;
          }

          if (bid < auc->bid)
          {
              sprintf(buf, "The minimum bid is %ld gold.\n\r", auc->bid);
              send_to_char(buf, ch);
              return;
          }

          if (bid < (auc->bid + 10))
          {
              sprintf(buf, "You must outbid %ld gold by at least 10.", auc->bid);
              send_to_char(buf, ch);
              return;
          }

          if ( !auc->ishouse )
          {
              sprintf(buf, "%ld gold has been offered for %s.",
              bid, auc->item->short_descr);
              auction_channel (NULL, buf);
          }
          else
          {
              sprintf(buf, "%ld gold has been offered for %s's house.", bid, auc->house->name );
              auction_channel(NULL, buf);
          }
          auc->high_bidder = ch;
          auc->bid = bid;
          auc->status = 0;
          return;
       }
   }
}

bool has_auction(CHAR_DATA *ch)
{
   AUCTION_DATA *auc;

   for(auc = auction_list; auc != NULL; auc = auc->next)
    {
       if(auc->owner == ch || auc->high_bidder == ch)
           return TRUE;
    }

    return FALSE;
}

void auction_channel(CHAR_DATA *ch, char *message)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *dch;
    char buf[MAX_INPUT_LENGTH];

    for(d = descriptor_list; d != NULL; d = d->next)
    {
        if(d->connected != CON_PLAYING)
            continue;

        if((dch = d->character) == NULL || dch == ch)
            continue;

        sprintf(buf, "AUCTION: %s", message);

        if(ch) /* don't use $N only $n in message */
            act_new(buf, ch, NULL, dch, TO_VICT, POS_DEAD);
        else
            send_to_char(buf, dch);
    }
}

void do_auction_talk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOAUCTION))
      {
	send_to_char("Auction channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOAUCTION);
      }
      else
      {
	send_to_char("Auction channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOAUCTION);
      }
    }
    else  /* auction message sent, turn auction on if it is off */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
	}

	REMOVE_BIT(ch->comm,COMM_NOAUCTION);
    }

    sprintf( buf, "You auction '%s'\n\r", argument );
    send_to_char( buf, ch );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm,COMM_NOAUCTION) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	{
	    act_new("$n auctions '$t'",
		    ch,argument,d->character,TO_VICT,POS_DEAD);
 	}
    }
}
	

