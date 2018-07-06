/*

In order to use any part of this Merc Diku Mud, you must comply with
both the original Diku license in 'license.doc' as well the Merc
license in 'license.txt'.  In particular, you may not remove either of
these copyright notices.

Much time and thought has gone into this software and you are
benefitting.  We hope that you share your changes too.  What goes
around, comes around.

ROM 2.4 is copyright 1993-1998 Russ Taylor
ROM has been brought to you by the ROM consortium
Russ Taylor (rtaylor@hypercube.org)
Gabrielle Taylor (gtaylor@hypercube.org)
Brian Moore (zump@rom.org)
By using this code, you have agreed to follow the terms of the
ROM license, in the file Rom24/doc/rom.license

*/

/* Automated Trivia. Trivia code will spit out random trivia questions
 * over a trivia channel. Players can answer the questions for quest points.
 * This code was written by Eric Lowe, aka Dalsor of AWW-MUD.org
 * It has been released under the same restrictions as the Diku/Merc/ROM
 * restrictions. In other words, you can use it, you can modify it, but
 * you cannot claim ownership of it, or claim it as your intellectual
 * property.
 *
 * Kudos: Erwin (always) for obvious reasons, Kyndig for being the most excellent
 * boon to the entire MUD community he is, Markanth for releasing his battle
 * code which inspired Tourney which provided insight on how to make Trivia
 * work the way it does, Muerte for goading me into writing more code, even
 * if it wasn't always the code he wanted, Caxandra for leaning over my shoulder
 * and asking 'whatcha doin?', and the playtesters at AWW for nit-picking.
 */


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
#include <unistd.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "trivia.h"

int tQuestion;
int tHighRange;
int tLowRange;
int TRIV_QUESTIONS = 20;

int qAsked[20];
int currTQuestion;

TRIVIA_SCORE *          trivia_score_hash [MAX_KEY_HASH];

/*local functions*/
bool chance args( (int num) );


void talk_trivia( char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MSL];
    CHAR_DATA *original;

    sprintf( buf, "{W{w[{gT{Gr{Wiv{Gi{ga{w]:{W {w%s{x\n\r", argument );

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        original = d->original ? d->original : d->character; /* if switched */
        if ( d->connected == CON_PLAYING && !IS_SET( d->character->comm,COMM_NOQUEST ) )
        send_to_char( buf , original );

    }
}

void close_trivia( void )
{
	char buf[MSL];

	if ( !trivia->running )
	{
		return;
	}

	sprintf( buf,"{wThe {WShadowStorm {wOfficial {gT{Gr{Wiv{Gi{ga{w Contest is now Closed!{x\n\r");
	talk_trivia( buf );
        if ( trivia->qnumber +1 > 0 )
        sprintf( buf, "After %d questions the winner is!\n\r", trivia->qnumber +1 );
        talk_trivia( buf );
	trivia->running = FALSE;
	trivia->question = NULL;
	trivia->answer = NULL;
	trivia->reward = 0;
	trivia->qnumber = 0;
	trivia->timer = 0;
	return;
}

void trivia_question( void )
{
	char buf[MSL];
	int tQuestion = number_range( tLowRange, tHighRange );
	int aQ;

	/* this needs tweaking */
	tQuestion = number_range( tLowRange, tHighRange );
	for ( aQ = 0; aQ < 20; aQ++ )
	{
		if ( tQuestion == qAsked[aQ] )
		{
			trivia_question();
			return;
		}
	}

	sprintf( buf, "{wQuestion number {W%d {wworth {W%d {wtrivia points.{x", trivia->qnumber +1, TriviaTable[tQuestion].reward );
	talk_trivia( buf );

	sprintf( buf, "{W%s{x", TriviaTable[tQuestion].question );
	talk_trivia( buf );
	trivia->answer = TriviaTable[tQuestion].answer;
	trivia->timer = 20;
	currTQuestion++;
	qAsked[currTQuestion] = tQuestion;
	return;
}

void trivia_update( void )
{
	char buf[MSL];

	if ( !trivia->running )
		return;

	if ( trivia->qnumber > TRIV_QUESTIONS )
	{
	/* end the trivia */
	bugf("Trivia questions exceeded, aborting normally\n\r",0);
	close_trivia();
	}

	if ( trivia->timer <= 0 )
	{
		sprintf( buf, "{wTime ran out for Trivia Question {W%d {wwith no winner!{x\n\r", trivia->qnumber +1 );
		talk_trivia( buf );

		trivia->qnumber++;

		if ( trivia->qnumber > TRIV_QUESTIONS )
		{
			close_trivia();
			return;
		}

		trivia_question();
	}

	if ( trivia->timer >= 1 )
		trivia->timer--;

	return;
}

void do_trivia( CHAR_DATA * ch, char *argument )
{
	char  arg1[MIL];
	char  arg2[MIL];
        char  buf[MSL];

	argument = one_argument( argument, arg1 );

	if ( arg1[0] == '\0' )
	{

           if(!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
           {
		send_to_char( "Trivia Syntax:\n\r", ch );
		send_to_char( "Trivia Talk <string>\n\r", ch );
                send_to_char( "Trivia Answer <string>\n\r", ch );
		send_to_char( "Trivia Open\n\r", ch );
		send_to_char( "Trivia Close\n\r", ch );
           }

           else if(!IS_NPC(ch) && ch->level <= LEVEL_IMMORTAL)
           {
                send_to_char( "Trivia Syntax:\n\r", ch );
                send_to_char( "Trivia Talk <string>\n\r", ch );
                send_to_char( "Trivia Answer <string>\n\r", ch );
           }
                return;
	}
	if ( !str_prefix( arg1, "talk" ) )
	{
		if ( !trivia->running )
		{
			send_to_char( "Trivia is not running.\n\r", ch );
                        return;
		}
		sprintf( buf, "%s says, '%s'", ch->name, argument );
		talk_trivia( buf );
		return;
	}
	if ( !str_prefix( arg1, "open" ) )
	{
		int count = 0, total = 0, aQ = 0;
		if (!IS_NPC(ch) && ch->level <= LEVEL_IMMORTAL)
		{
			send_to_char( "You are not authorized to do this.\n\r", ch );
                        return;
		}

		if ( trivia->running )
		{
			send_to_char( "Trivia is already running.\n\r", ch );
                        return;
		}

		for ( count = 0; TriviaTable[count].question != NULL; count++ )
			total++;

		printf_to_char( ch, "Found %d questions.\n\r", total );
		tLowRange = 0;
		tHighRange = total -1;
		sprintf( buf,"{wThe {WShadowStorm {wOfficial {gT{Gr{Wiv{Gi{ga{w Contest is now Open!{x\n\r");
		talk_trivia( buf );
                trivia->running = TRUE;
		trivia->question = NULL;
		trivia->answer = NULL;
		trivia->timer = 20;
		for ( aQ = 0; aQ < 20; aQ++ )
			qAsked[aQ] = 0;
		currTQuestion = -1;
		trivia_question();

		return;
	}
	if ( !str_prefix( arg1, "close" ) )
	{
		if (!IS_NPC(ch) && ch->level <= !IS_IMMORTAL( ch ) )
		{
			send_to_char( "You are not authorized to do this.\n\r", ch );
			return;
		}
		if ( !trivia->running )
		{
			send_to_char( "Trivia is not running.\n\r", ch );
                        return;
		}
		sprintf( buf,"{wThe {WShadowStorm {wOfficial {gT{Gr{Wiv{Gi{ga{w Contest is now Closed!{x\n\r");
		talk_trivia( buf );
		trivia->running = FALSE;
		trivia->question = NULL;
		trivia->answer = NULL;
		trivia->reward = 0;
		trivia->qnumber = 0;
		trivia->timer = 0;
		return;
	}

	if ( !str_prefix( arg1, "answer" ) )
	{
		argument = one_argument( argument, arg2 );
		if ( !trivia->running )
		{
			send_to_char( "Trivia is not running.\n\r", ch );
                        return;
		}
		if ( arg2[0] == '\0' )
		{
		        send_to_char( "Trivia Answer what?\n\r", ch );
                        return;
		}
		strcpy( argument, arg2 );

		sprintf( buf,"{w%s guesses, %s{x\n\r", ch->name, argument );
		talk_trivia( buf );

		if ( !str_cmp( argument, trivia->answer ) )
		{
                        int pracreward;
                        int trainreward;
                        int questreward;

			sprintf( buf,"{W%s {wcorrectly answered question {W#%d{w!{x\n\r", ch->name, trivia->qnumber +1 );
			talk_trivia( buf );
			ch->pcdata->trivia += TriviaTable[tQuestion].reward;
                        ch->pcdata->totaltpoints += TriviaTable[tQuestion].reward;
			printf_to_char( ch, "{wYou receive {W%d {wTrivia Points!{x\n\r\n\r", TriviaTable[tQuestion].reward );
			trivia->qnumber++;

     if (chance(2))
     {
               pracreward = number_range(1, 5);
               printf_to_char(ch, "You gain %d practices!\n\r", pracreward );
               ch->practice += pracreward;
     }

     if (chance(1))
     {
               trainreward = number_range(1, 2);
               printf_to_char(ch, "You gain %d training sessions!!\n\r", trainreward );
               ch->train += trainreward;
     }

     if (chance(2))
     {
               questreward = number_range( 5, 10 );
               send_to_char("You gain a questpoint!\n\r", ch);
               ch->pcdata->questpoints += questreward;
     }

     if (chance(3))
     {
               send_to_char("You gain a blessing point!\n\r", ch);
               ch->bp_points++;
     }

			if ( trivia->qnumber > TRIV_QUESTIONS )
			{
				close_trivia();
				return;
			}

			trivia_question();
		}
		else
		{
			printf_to_char( ch, "{R%s {wwas not the correct answer to Question {W%d{w.{x\n\r", argument, trivia->qnumber +1 );
			return;
		}
		return;
	}
}

void do_ttalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUEST))
      {
        send_to_char("Trivia channel is now ON\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOQUEST);
      }
      else
      {
        send_to_char("Trivia channel is now OFF\n\r",ch);
        SET_BIT(ch->comm,COMM_NOQUEST);
      }
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOQUEST);

    sprintf( buf, "%s says, '%s'", ch->name, argument );
    talk_trivia( buf );

    return;
}

void do_guess( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument,arg);

    sprintf( buf, "answer '%s'", arg );
    do_function(ch, &do_trivia, buf );

    return;
}

void load_trivia_scores (void)
{
   FILE *fp;
   TRIVIA_SCORE *score;
   int iHash;
   char stop;

   fclose (fpReserve);

   if ((fp = fopen (TRIVIA_SCORE_FILE, "r")) == NULL)
   {
      bug ("Trivia score file missing.", 0);
      perror (TRIVIA_SCORE_FILE);
   }

   for (;;)
   {
      iHash = 1 % MAX_KEY_HASH;
      score = (TRIVIA_SCORE *)alloc_perm (sizeof (*score));
      score->name = fread_string (fp);
      score->points = fread_number (fp);
      score->totalpoints = fread_number (fp);
      score->next = trivia_score_hash[iHash];
      trivia_score_hash[iHash] = score;
      stop = fread_letter(fp);
     
      if (stop == '$')
         break;
   }

   fclose (fp);
   fpReserve = fopen (NULL_FILE, "r");
}

void save_trivia_scores (void)
{
   FILE *fp;
   TRIVIA_SCORE *score;

   fclose (fpReserve);

   if ((fp = fopen (TRIVIA_SCORE_FILE, "r")) == NULL)
   {
      bug ("Trivia score file missing.", 0);
      perror (TRIVIA_SCORE_FILE);
   }

   for (score = trivia_score_hash[1 % MAX_KEY_HASH]; score != NULL; score = score->next)
   {
      fprintf (fp, "%s\n", score->name);
      fprintf (fp, "%d\n", score->points);
      fprintf (fp, "%d\n", score->totalpoints);
  
      if (score->next == NULL)
         fprintf (fp, "$\n");
      else
         fprintf (fp, "#\n");
   }
   
   return;
}

TRIVIA_SCORE *new_score (void)
{
   TRIVIA_SCORE *score;
   int iHash;

   iHash = 1 % MAX_KEY_HASH;
   score = (TRIVIA_SCORE *)alloc_perm (sizeof (*score));
   score->name = str_dup ("Nobody");
   score->points = 0;
   score->totalpoints = 0;
   score->next = trivia_score_hash[iHash];
   trivia_score_hash[iHash] = score;

   return score;
}

TRIVIA_SCORE *trivia_score_lookup (char *argument)
{
   TRIVIA_SCORE *score;

   for (score = trivia_score_hash[1 % MAX_KEY_HASH]; score != NULL; score = score->next)
   {
      if (!str_cmp (argument, score->name))
         return score;
   }

   return NULL;
}

void do_trivia_scores (CHAR_DATA *ch, char *argument)
{
   TRIVIA_SCORE *curr_score;
   char buf[MSL];

   if (argument[0] != '\0')
   {
      if ((curr_score = trivia_score_lookup (argument)) == NULL)
      {
         send_to_char ("No such player.\n\r", ch);
         return;
      }
     
      sprintf (buf, "{DPlayer: {R%s\n\r", capitalize (curr_score->name));
      send_to_char (buf, ch);
      sprintf (buf, "{DPoints: {R%d\n\r", curr_score->points);
      send_to_char (buf, ch);
      sprintf (buf, "{DTotal : {R%d\n\r", curr_score->totalpoints);
      send_to_char (buf, ch);

      return;
   }

   return;
}

