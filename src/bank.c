/***************************************************************
  January 25th, 1998
  Gothar's Bank Code Version 1.2
  1997 Copyright <* FREEWARE *>
  If you use this code:
     1.Keep all the credits in the code.
     2.Use the help entry for bank
     3.Send a bug report,any comments or ideas
       Ian McCormick (aka Gothar)
       mcco0055@algonquinc.on.ca

Here is a listing of what the code does:
     2.Deposit gold or silver into your account
     3.Withdraw gold or silver into your account
     4.Check your account
       -shows shares bought, duration effects
        gold & silver in account and on person
     5.Midgaard Shares to enhance spell duration
       -You can buy shares and sell them at reduced rate.
       -Shares are bought buy bank gold only! 
     6.Change silver into gold 
     7.Let's you still us the Changer bit for something
        if you want.  
NOTE: This code leaves the the IS_CHANGER stuff in do_give
      so you still can use Otho's services for money exchange. 
***************************************************************/

/*
 * Improvements by Synon, Synon23@hotmail.com
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "include.h"

/* command procedures needed */
DECLARE_DO_FUN( do_help		);

void do_account (CHAR_DATA *ch, char *argument)
{
   long gold = 0, silver = 0, 
   shares = 0;
   int diff, bonus = 0;
   char buf[MAX_STRING_LENGTH];
   gold = ch->pcdata->gold_bank;
   silver = ch->pcdata->silver_bank;
   shares = ch->pcdata->shares;
   diff = (SHARE_MAX - ch->pcdata->shares_bought);
   if( ch->pcdata->shares / 4 != 0)
      bonus = shares / 4;
   ch->pcdata->duration = bonus;

   if(IS_NPC(ch) ) 
         
   {
      send_to_char("Only players need money!\n\r", ch);
      return;
   }

   sprintf( buf, "You have: %ld gold, and: %ld silver in your bank account.\n\r", gold, silver ); 
   send_to_char(buf, ch);
   sprintf( buf, "You have: %ld shares with %d hours of spell duration.\n\r", shares, bonus);
   send_to_char(buf, ch);
   sprintf( buf,"You have: %d shares left at level %d.\n\r", diff, ch->level );
   send_to_char(buf, ch);
   
   return;
}


void do_deposit (CHAR_DATA *ch, char *argument)
{
   long amount = 0; 
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   if(IS_NPC(ch) )
   {
      send_to_char("Only players need money!\n\r", ch);
      return;
   }

   if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) )
   {
      send_to_char("You must be in the Bank to deposit.\n\r",ch);
      return;
   }

   else  /* In the Bank */
   {
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, arg2 );

      if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg1 ) )
      {
          send_to_char("Deposit <value> gold\n\r",ch);
          send_to_char("Deposit <value> silver\n\r",ch);
      }
	 
      amount = atoi(arg1);

      if ( amount <= 0 )
      {
          send_to_char( "To deposit you must give money!\n\r", ch ); 
          return;
      }
               
      if(!str_cmp( arg2, "gold")) 
      {
         if (ch->gold < amount)
         {
             send_to_char("You don't have that much gold.\n\r",ch);
             return;
         }
         else 
         {
             ch->pcdata->gold_bank += amount;
             ch->gold -= amount;
             sound( "coins.wav", ch );
             act("$n deposits gold into $s account.", ch,NULL,NULL,TO_ROOM);
             sprintf( buf, "You have deposited: %ld Gold.\n\rYou have: %ld gold in your account.\n\rYou now have: %ld gold.\n\r",
                     amount, ch->pcdata->gold_bank, ch->gold);
             send_to_char( buf, ch);
             return;
         }
      }  
               
      else if(!str_cmp( arg2, "silver")) 
      {
         if (ch->silver < amount)
         {
	     send_to_char("You don't have that much silver.\n\r",ch);
             return;
         }
         
         else 
         {
             ch->pcdata->silver_bank += amount;
             ch->silver -= amount;
             sound( "coins.wav", ch );
             act("$n deposits silver into $s account.", ch,NULL,NULL, TO_ROOM);
             sprintf( buf, "You have deposited %ld Silver.\n\rYou have %ld silver in the bank.\n\rYou now have %ld silver.\n\r", amount, ch->pcdata->silver_bank, ch->silver);
             send_to_char( buf, ch);
             return;
         }
      }
   }

   send_to_char( "Deposit <amount> <gold|silver>\n\r", ch );
   return;
}



void do_withdraw (CHAR_DATA *ch, char *argument)
{
   long amount = 0; 
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   if(IS_NPC(ch))
   {
      send_to_char("Only players need money!\n\r", ch);
      return;
   }

   if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) )
   {
      send_to_char("You must be in the Bank to deposit.\n\r",ch);
      return;
   }                 

   else  /* In the Bank */
   {
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, arg2 );

      if (arg1[0] == '\0'|| arg2[0] == '\0' || !is_number( arg1 ) )
      {
          send_to_char("Withdraw <value> gold\n\r",ch);
          send_to_char("Withdraw <value> silver\n\r",ch);
          return;
      }

      amount = atoi(arg1);

      if ( amount <= 0 )
      {
	  send_to_char( "To withdraw you must give an amount!\n\r", ch );
          return;
      }
           
      if(!str_cmp( arg2, "gold")) 
      {
         if (ch->pcdata->gold_bank < amount)
         {
             send_to_char("You don't have that much gold in the bank.\n\r",ch);
             return;
         }
            
         else 
         {
             ch->pcdata->gold_bank -= amount;
             ch->gold += amount;
             sound( "coins.wav", ch );
             act("$n withdraws gold from $s account.", ch,NULL,NULL, TO_ROOM);
             sprintf( buf, "You have withdrawn %ld Gold.\n\rYou have: %ld gold in your account.\n\rYou have %ld gold..\n\r", amount, ch->pcdata->gold_bank, ch->gold);
             send_to_char( buf, ch);
             return;
         }
      }
           
      else if(!str_cmp( arg2, "silver")) 
      {
             if (ch->pcdata->silver_bank < amount)
             {
	         send_to_char("You don't have that much silver in the bank.\n\r",ch);
	         return;
             }
             else 
             {
                 ch->pcdata->silver_bank -= amount;
                 ch->silver += amount;
                 sound( "coins.wav", ch );
                 act("$n withdraws silver from $s account.", ch,NULL,NULL, TO_ROOM);
                 sprintf( buf,"You have withdrawn %ld Silver.\n\rYou have: %ld silver in your account\n\rYou now have %ld silver.", amount, ch->pcdata->silver_bank, ch->silver);
                 send_to_char( buf, ch);
                 return;
             }
      }

   }
 
   send_to_char("Syntax: withdraw <amount> <gold|silver>\n\r", ch );
   return;
}


void do_btransfer(CHAR_DATA *ch, char *argument)    
{
  long amount = 0;
  char arg1[MIL];
  char arg2[MIL];
  char arg3[MIL];
  char buf[MSL];
  CHAR_DATA *victim;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );      
  argument = one_argument( argument, arg3 );
 
  if (arg1[0] == '\0'|| arg2[0] == '\0' || arg3[0] == '\0' || !is_number( arg2 ) )
  {
      send_to_char("btransfer <name> <value> gold\n\r",ch);
      send_to_char("btransfer <name> <value> silver\n\r",ch);
      return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  {
        send_to_char( "They aren't here.\n\r", ch );
        return;
  }   
 
  if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) )
  {
      send_to_char("You must be in the Bank to transfer gold or silver.\n\r",ch);
      return;
  }              

  else
  {
      amount = atoi(arg2);

      if ( amount <= 0 )
      {
          send_to_char( "Please choose an amount greater than 0.\n\r", ch );
          return;
      }                          

      if(!str_cmp( arg3, "gold"))
      {
         if (ch->pcdata->gold_bank < amount)
         {
             send_to_char("You don't have that much gold in the bank.\n\r",ch);
             return;
         }
           
         else
         {
             ch->pcdata->gold_bank -= amount;
             victim->pcdata->gold_bank += amount;
             act("$n transfers some gold from $s account.", ch,NULL,NULL, TO_ROOM);
             printf_to_char( victim, "%s has transferred %ld gold into your account!\n\rYou have: %ld gold in your account.", ch->name, amount, victim->pcdata->gold_bank ); 
             sprintf( buf, "You transfer %ld Gold into %s's account.\n\rYou have: %ld gold in your account.\n\rYou have %ld gold..\n\r", amount, victim->name, ch->pcdata->gold_bank, ch->gold);
             send_to_char( buf, ch);
             return;
         }
      }        

      if(!str_cmp( arg3, "silver"))
      {
         if (ch->pcdata->silver_bank < amount)
         {
             send_to_char("You don't have that much silver in the bank.\n\r",ch);
             return;
         }
        
         else
         {
             ch->pcdata->silver_bank -= amount;
             victim->pcdata->silver_bank += amount;
             act("$n transfers some silver from $s account.", ch,NULL,NULL, TO_ROOM);
             printf_to_char( victim, "%s has transferred %ld silver into your account!\n\rYou have: %ld silver in your account.", ch->name, amount, victim->pcdata->silver_bank );
             sprintf( buf,"You transfer %ld Silver into %s's account.\n\rYou have: %ld silver in your account\n\rYou now have %ld silver.", amount, victim->name, ch->pcdata->silver_bank, ch->silver);
             send_to_char( buf, ch);
             return;
         }
      }          
  }
      
      send_to_char("btransfer <name> <value> gold|silver\n\r",ch);
      return;
}

void do_change (CHAR_DATA *ch, char *argument)
{
   int amount = 0,change = 0;
   char buf [MAX_STRING_LENGTH];
   char arg [MAX_INPUT_LENGTH];

   if(IS_NPC(ch))
   {
        send_to_char("Only players need to change currency!\n\r", ch);
        return;
   }

  
   if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) )
   {
      send_to_char("You must be in the Bank to change currency.\n\r",ch);
      return;
   }

   else
   {
      argument = one_argument( argument, arg );

      if ( is_number( arg ) )
      {
 	  amount = atoi(arg);
	   
          if ( amount < 100 )
          {
              send_to_char("You must change at least 100 silver at the current exchange rate.\n\r",ch);
              return;
          }

          if ( ch->silver < amount )
          {
              sprintf(buf, "You can only change %ld silver.\n\r",ch->silver);
              send_to_char( buf, ch);
              return;
          }

          else
          {
             change = amount/100;
             ch->gold += change;
             ch->silver -= amount;
             sound( "coins.wav", ch );
             sprintf( buf, "You have changed %d Silver.\n\rYou have recieved %d Gold.\n\r", amount, change);
             send_to_char( buf, ch);
             return;
          }
       }
   }

   send_to_char("Change syntax:\n\r",ch);
   send_to_char("Change <value> i.e. change 120\n\r",ch);
   send_to_char("So 1200 silver changes into 12 gold.\n\r",ch); 
   return;
}


void do_share (CHAR_DATA *ch, char *argument)
{
   long shares = 0, cost = 0;
   BUFFER *output;
   int level, row, money = 0, diff, i;
   int list_level, tax;
   int bonus = 0;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];

   shares = ch->pcdata->shares;

  
   if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) )
   {
      send_to_char("You must be in the Bank to find out about shares.\n\r",ch);
      return;
   }

   else  
   {
      argument = one_argument( argument, arg );

      if(IS_NPC(ch) ) 
      {
         send_to_char("Only players need shares!\n\r", ch);
         return;
      }

      if (arg[0] == '\0' && ch->level >= 10)
      { 
          send_to_char("Share Costs are all in Gold.\n\r",ch);
          send_to_char("To Buy Shares: Gold is taken from your Account.\n\r",ch);
          send_to_char("Share Spell Bonus +1 hour duration per 4 shares.\n\r",ch);
          send_to_char("ONLY 5 shares can be bought per level.\n\r",ch);
          send_to_char("ONLY 5 shares can be bought at one time.\n\r",ch);
          send_to_char("Share <value>\n\r",ch);
          send_to_char("For the Share prices Type 'share price'.\n\r",ch);
          return;
      }

      else if(!str_cmp(arg, "price"))
      {
          output = new_buf();
          i = 0;
          send_to_char("Share Listing <-\n\r",ch);
          sprintf(buf2,"Level:     Cost:      Level:     Cost:      Level:     Cost:      \n\r");
          send_to_char(buf2, ch);
          cost = 0;
          row = tax = 0;

          if(IS_HERO(ch) || ch->level+2 == LEVEL_HERO)
             list_level = ch->level;  /*if hero list ch->level*/
          else
             list_level = ch->level+2; /*2 level past ch->level*/

          for(level=10; level <= list_level; level++)  
          {
              if(++tax % 9 == 0)
                 cost += 120000; /* every 9 levels */
              else
                 cost += 20000;
              sprintf(buf,"%-6d     %-9ld  ",level,cost);
              if(++row % 3 == 0)
                 strcat( buf, "\n\r");
              add_buf( output, buf);
          }
         
          page_to_char(buf_string(output),ch);
          free_buf(output);
          send_to_char( "\n\r", ch);
          return;
      }
      else if(ch->level < 10)
      {
          send_to_char("Share buying starts at level 10.\n\r",ch);
          return;
      }
      else
      {
          if( is_number( arg ) )
          {
             shares = atoi(arg);
             if ( shares < 0 )
             {
                 shares = abs(shares);
                 send_to_char( "Bad market - shares don't sell well!\n\r", ch );

                 if( ch->pcdata->shares < shares)
                 {
                    sprintf( buf, "You only have %d shares.\n\r",ch->pcdata->shares);
                    send_to_char( buf, ch);
                 }
                 else
                 {
                    ch->pcdata->shares -= shares;
                    act("$n sells $s some shares.", ch,NULL,NULL, TO_ROOM);

                    if(ch->pcdata->shares == 0)
                       send_to_char( "You sell all of your shares.\n\r",ch);
                    else
                       send_to_char( "You sell of your shares.\n\r",ch);

                    money = ( 20000 / (int)((float)shares * 0.10) ); /* cash back */
                    sprintf( buf,"You get %d gold for %ld %s deposited in your Account.\n\r", 
                            money, shares, shares > 1 ? "shares" : "share");
                    send_to_char( buf, ch);
                    ch->pcdata->gold_bank += money; 
                    ch->pcdata->shares_bought -= shares;
                    if( ch->pcdata->shares / 4 != 0)
                       bonus = ch->pcdata->shares / 4;
                    ch->pcdata->duration = bonus;
                    return;
                 }   
	     }
	     else if ( shares > SHARE_MAX )
             {
	            sprintf(buf,"Bad market - don't buy more than %d shares.\n\r",SHARE_MAX);
	            send_to_char( buf, ch );
	            return;
	     }
	       
             else
             {
                 if(ch->pcdata->shares_bought > SHARE_MAX)
                 {
                    sprintf(buf,"You have reached your %d share limit per level.\n\r",SHARE_MAX);
                    send_to_char( buf, ch);
                    return;
                 }
                 else
                 {
                    if(ch->pcdata->share_level == ch->level
                       && ch->pcdata->shares_bought <= SHARE_MAX)
                    {
                       cost = 0;
                       tax = 0;

                       for(level=10;level<LEVEL_HERO;level++)
                       { 
                         if(++tax % 9 == 0)
                           cost += 120000; /* every 9 levels */
                         else
                            cost += 20000;
                         if(level == ch->level)
                            break;
                       }
                     
                       diff = (SHARE_MAX - ch->pcdata->shares_bought - shares);
                       sprintf( buf,"You can buy %d more %s at this level.\n\r", 
                                diff, shares > 1 ? "share" : "shares");
                       send_to_char( buf, ch); 
                       cost *= shares; 
                 
                       if(ch->pcdata->gold_bank < cost)
                       {
                          sprintf( buf,"You only have %ld gold in your Account.\n\r"
                                       "For %ld %s cost %ld.\n\r",ch->pcdata->gold_bank, shares,
                                          shares > 1 ? "shares" : "share", cost);
                          send_to_char( buf, ch);
                          return;
                       }
                       else
                       {
                          if( shares == 1 )
                          {
                             act("$n buys one share in the Midgaard Bakery.", ch,NULL,NULL, TO_ROOM);
                             sprintf( buf,"You buy a share in the Midgaard Bakery.\n\r");
                          }
                          else if( shares > 1 && shares <= 3)
                          {
                             act("$n buys shares in the Midgaard Merchant's Guild.", ch,NULL,NULL, TO_ROOM);
                             sprintf( buf,"You buy %ld shares in the Midgaard Merchant's Guild.\n\r", shares);
                          }
                          else 
                          {
                             act("$n buys shares in the Midgaard Waterworks.", ch,NULL,NULL, TO_ROOM);
                             sprintf( buf,"You buy %ld shares in the Midgaard Waterworks.\n\r", shares);
	                  }
                          send_to_char( buf, ch );
                          ch->pcdata->gold_bank -= cost;
                          ch->pcdata->shares += shares;
                          ch->pcdata->shares_bought += shares;
                          do_account(ch, "");
                          if( ch->pcdata->shares / 4 != 0)
                             bonus = ch->pcdata->shares / 4;
                          ch->pcdata->duration = bonus;
 	                  return;
                       }
                     }
                     else
                     {
                        bug( "Do_shares: shares_bought && share_level error.", 0 );
                        bug( "Do_shares: bought > %d && share_level not ch->level.",SHARE_MAX );
                        send_to_char( "Stock market has crashed.\n\r", ch );
                        return;
                     }
                  }
	      }
          }
       } 
   }
   return;
}
