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
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@hypercube.org)				   *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

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
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
#include "const.h"
 
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif


/*int rename(const char *oldfname, const char *newfname);*/

char *print_flags(int flag)
{
    int count, pos = 0;
    static char buf[52];


    for (count = 0; count < 32;  count++)
    {
        if (IS_SET(flag,1<<count))
        {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
    }

    if (pos == 0)
    {
        buf[pos] = '0';
        pos++;
    }

    buf[pos] = '\0';

    return buf;
}


/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];



/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, int iNest ) );
void	fwrite_pet	args( ( CHAR_DATA *pet, FILE *fp) );
void	fwrite_mount	args( ( CHAR_DATA *pet, FILE *fp) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_pet	args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_mount	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_bobj      args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fwrite_bobj     args( ( CHAR_DATA *ch, OBJ_DATA *obj, 
                            FILE *fp, int iNest ) ); 


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;

    if ( IS_NPC(ch) )
	return;

    if ( !IS_VALID(ch)) 

    {
        bug("save_char_obj: Trying to save an invalidated character.\n", 0);
        return;
    }

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

#if defined(unix)
    /* create god log */
    if (IS_IMMORTAL(ch) || ch->level >= LEVEL_IMMORTAL)
    {
	sprintf(strsave, "%s%s",GOD_DIR, capitalize(ch->name));
	fp = file_open(strsave,"w");
	fprintf(fp,"Lev %2d Trust %2d  %s%s\n",
	    ch->level, get_trust(ch), ch->name, ch->pcdata->title);
	file_close( fp );
    }
#endif

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
    fp = file_open( TEMP_FILE, "w" );
    fwrite_char( ch, fp );
    if ( ch->carrying != NULL )
	fwrite_obj( ch, ch->carrying, fp, 0 );
    if ( ch->pcdata->box != NULL )
	fwrite_bobj( ch, ch->pcdata->box, fp, 0 );       
    /* save the pets */
    if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
	fwrite_pet(ch->pet,fp);
    if (ch->mount && ch->mount->in_room == ch->in_room)
	fwrite_mount(ch->mount, fp);
    fprintf( fp, "#END\n" );
    file_close( fp );
    rename(TEMP_FILE,strsave);
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int sn, gn, pos, i;

    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"	);

    fprintf( fp, "Name %s~\n",	ch->name		);
    fprintf( fp, "Id   %ld\n", ch->id			);
    fprintf( fp, "LogO %ld\n",	current_time		);
    fprintf( fp, "Vers %d\n",   6			);
    if (ch->short_descr[0] != '\0')
      	fprintf( fp, "ShD  %s~\n",	ch->short_descr	);
    if( ch->long_descr[0] != '\0')
	fprintf( fp, "LnD  %s~\n",	ch->long_descr	);
    if (ch->description[0] != '\0')
    	fprintf( fp, "Desc %s~\n",	ch->description	);
    if (ch->pcdata->host) 
    {
 	fprintf(fp, "Host %s~\n", ch->pcdata->host);
    } 
    else 
    {
	fprintf(fp, "Host none~\n");
    }
    fprintf( fp, "Spouse %s~\n", ch->pcdata->spouse );
    if( ch->prompt != NULL
     || !str_cmp( ch->prompt,"<%hhp %mm %vmv> " )
     || !str_cmp( ch->prompt,"{c<%hhp %mm %vmv>{x " ) )
        fprintf( fp, "Prom %s~\n",      ch->prompt  	);
    fprintf( fp, "Race %s~\n", pc_race_table[ch->race].name );
    if (ch->pcdata->rentroom != 0)
       fprintf( fp, "Rroom %d\n",  ch->pcdata->rentroom  );
    if (ch->clan)
    	fprintf( fp, "Clan %s~\n",clan_table[ch->clan].name);
    if (ch->rank)
    fprintf( fp, "Rank %d\n", ch->rank                  );
    fprintf( fp, "Sex  %d\n",	ch->sex			);
    fprintf( fp, "Box   %d\n",   ch->pcdata->boxrented  );
    if( ch->pcdata->h_vnum )
       fprintf(fp,"HVnum %d\n", ch->pcdata->h_vnum);
    if( ch->pcdata->horesets )
       fprintf(fp,"HOResets %d\n", ch->pcdata->horesets);
    if( ch->pcdata->hmresets )
       fprintf(fp,"HMResets %d\n", ch->pcdata->hmresets);
    if (ch->pcdata->height)
        fprintf( fp, "Height %d\n", ch->pcdata->height);
    if (ch->pcdata->weight)
	fprintf( fp, "Weight %d\n", ch->pcdata->weight);
    if (ch->pcdata->eye)
	fprintf( fp, "Eye %d\n", ch->pcdata->eye);
        fprintf( fp, "Svolume %d\n", ch->pcdata->svolume );
        fprintf( fp, "Mvolume %d\n", ch->pcdata->mvolume );
        fprintf( fp, "Msp %d\n", ch->pcdata->msp );
        fprintf( fp, "Mxp %d\n", ch->pcdata->mxp );
    if (ch->pcdata->hair)
	fprintf( fp, "Hair %d\n", ch->pcdata->hair);
    fprintf( fp, "Cla  %s\n", class_numbers(ch, TRUE)   );
    if (ch->pcdata->stay_race)
        fprintf(fp, "StayRace\n");   
    if (ch->pcdata->title2 != NULL)
    fprintf( fp, "Title2 %s~\n", ch->pcdata->title2);  
    if (ch->pcdata->immtitle != NULL)
    fprintf( fp, "Immtitle %s~\n",  ch->pcdata->immtitle);
    fprintf( fp, "Hmtown %d\n", ch->hometown            );
    fprintf( fp, "Levl %d\n",	ch->level		);
    if (ch->trust != 0)
	fprintf( fp, "Tru  %d\n",	ch->trust	);
    fprintf( fp, "Sec  %d\n",    ch->pcdata->security	);	/* OLC */
    fprintf( fp, "Lflux %d\n",   ch->levelflux   );
    fprintf( fp, "Stance %d\n", ch->stance);  
    fprintf( fp, "Dblq %d\n", ch->pcdata->dblq );
    fprintf( fp, "Bcount %d\n", ch->pcdata->bcount );
    fprintf( fp, "Mailid %d\n", ch->pcdata->mailid );
    fprintf( fp, "Plyd %d\n",
	ch->played + (int) (current_time - ch->logon)	);
    fprintf( fp, "Scro %d\n", 	ch->lines		);
    fprintf( fp, "Kill %d %d\n", ch->pcdata->kills_mob, ch->pcdata->kills_pc);
    fprintf( fp, "Deat %d %d\n", ch->pcdata->deaths_mob, ch->pcdata->deaths_pc);
    fprintf( fp, "Room %d\n",
        (  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
        && ch->was_in_room != NULL )
            ? ch->was_in_room->vnum
            : ch->in_room == NULL ? 3001 : ch->in_room->vnum );
    fprintf( fp, "Bites %d\n", ch->bites);
    fprintf( fp, "Bldthrst %d\n", ch->bloodthirst);
    fprintf( fp, "Isvmp %s\n", ch->isvamp ? "TRUE" : "FALSE");
    fprintf( fp, "Iswere %s\n", ch->iswere ? "TRUE" : "FALSE");
    fprintf( fp, "Wereform %d\n", ch->wereform);
    if (ch->pcdata->questpoints != 0)
        fprintf(fp, "QuestPnts %d\n", ch->pcdata->questpoints);
    if (ch->pcdata->nextquest != 0)
        fprintf(fp, "QuestNext %d\n", ch->pcdata->nextquest);
    else if (ch->pcdata->countdown != 0)
        fprintf(fp, "QuestCount %d\n", ch->pcdata->countdown);
    if(ch->pcdata->questgiver != 0)
        fprintf(fp, "QuestGiver %d\n", ch->pcdata->questgiver);
    if(ch->pcdata->questloc != 0)
        fprintf(fp, "QuestLoc   %d\n", ch->pcdata->questloc);
    if (ch->pcdata->questobj != 0)
        fprintf(fp, "QuestObj %d\n", ch->pcdata->questobj);
    else if (ch->pcdata->questmob != 0)
        fprintf(fp, "QuestMob %d\n", ch->pcdata->questmob);
    if (ch->pcdata->qps > 0)
        fprintf(fp, "Qps     %d\n", ch->pcdata->qps );
    if(ch->pcdata->trivia != 0)
        fprintf(fp, "Trivia  %d\n", ch->pcdata->trivia);
    if (ch->pcdata->totaltpoints != 0)
        fprintf(fp, "TotalTPnts %d\n", ch->pcdata->totaltpoints);
    fprintf( fp, "HMV  %ld %ld %ld %ld %ld %ld\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
    if (ch->gold > 0)
      fprintf( fp, "Gold %ld\n",	ch->gold		);
    else
      fprintf( fp, "Gold %d\n", 0			); 
    if (ch->silver > 0)
	fprintf( fp, "Silv %ld\n",ch->silver		);
    else
	fprintf( fp, "Silv %d\n",0			);
    if (ch->pcdata->gold_bank > 0)
      fprintf( fp, "Gold_bank %ld\n",ch->pcdata->gold_bank);
    else
      fprintf( fp, "Gold_bank %d\n", 0);
    if (ch->pcdata->silver_bank > 0)
	fprintf( fp, "Silv_bank %ld\n",ch->pcdata->silver_bank);
    else
	fprintf( fp, "Silv_bank %d\n",0);
    if (ch->pcdata->shares > 0)
	fprintf( fp, "Shares %d\n",ch->pcdata->shares);
    else
	fprintf( fp, "Shares %d\n",0);
    if (ch->pcdata->share_level > 0)
	fprintf( fp, "Share_level %d\n",ch->pcdata->share_level);
    else
	fprintf( fp, "Share_level %d\n",0);
    if (ch->pcdata->shares_bought > 0)
	fprintf( fp, "Shares_bought %d\n",ch->pcdata->shares_bought);
    else
	fprintf( fp, "Shares_bought %d\n",0);
    if (ch->pcdata->shares / 4 != 0)
    {
       ch->pcdata->duration = ch->pcdata->shares / 4;
	fprintf( fp, "Duration %d\n",ch->pcdata->duration);
    }
    else
	fprintf( fp, "Duration %d\n",0);
    fprintf( fp, "Exp  %d\n",	ch->exp			);
   
    if(ch->act != 0)
    {

       if (!IS_NPC(ch) && gquest_info.running == GQUEST_OFF && IS_SET(ch->act, PLR_GQUEST))
       {
           REMOVE_BIT(ch->act, PLR_GQUEST);
           reset_gqmob(ch, 0);

           if (war_info.iswar == WAR_OFF && IS_SET(ch->act, PLR_WAR))
               REMOVE_BIT(ch->act, PLR_WAR);        
       }
	
       fprintf(fp, "Act  %s\n", print_flags(ch->act));
    }

    if (ON_GQUEST(ch) || (gquest_info.running != GQUEST_OFF
        && count_gqmobs(ch) == gquest_info.mob_count))
    {
        fprintf(fp, "GQmobs %d ", gquest_info.mob_count);
	
        for (i = 0; i < gquest_info.mob_count; i++)
	     fprintf(fp, "%ld ", ch->pcdata->gq_mobs[i]);
	     fprintf(fp, "\n");
    }

    if (ch->affected_by != 0)
	fprintf( fp, "AfBy %s\n",   print_flags(ch->affected_by));
    fprintf( fp, "Comm %s\n",       print_flags(ch->comm));
    fprintf( fp, "Comm2 %s\n",      print_flags(ch->comm2));
    fprintf( fp, "Supp %s\n",       print_flags(ch->pcdata->suppress));    
    if (ch->wiznet)
    	fprintf( fp, "Wizn %s\n",   print_flags(ch->wiznet));
    if (ch->invis_level)
	fprintf( fp, "Invi %d\n", 	ch->invis_level	);
    if (ch->incog_level)
	fprintf(fp,"Inco %d\n",ch->incog_level);
    fprintf( fp, "Pos  %d\n",	
	ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
    if (ch->practice != 0)
    	fprintf( fp, "Prac %d\n",	ch->practice	);
    if (ch->train != 0)
	fprintf( fp, "Trai %d\n",	ch->train	);
    if (ch->saving_throw != 0)
	fprintf( fp, "Save  %d\n",	ch->saving_throw);
    fprintf( fp, "Alig  %d\n",	ch->alignment		);
    if (ch->hitroll != 0)
	fprintf( fp, "Hit   %d\n",	ch->hitroll	);
    if (ch->damroll != 0)
	fprintf( fp, "Dam   %d\n",	ch->damroll	);
    if (ch->pcdata->hdam > 0 )
        fprintf( fp, "Hdam %d\n",       ch->pcdata->hdam );
     if (ch->pcdata->mdam > 0 )
        fprintf( fp, "Mdam %d\n",       ch->pcdata->mdam );   
    if (ch->cast_level != 0)
        fprintf( fp, "Clevel %d\n",     ch->cast_level  );
    if (ch->cast_ability != 0)
        fprintf( fp, "Cability %d\n",   ch->cast_ability );
        fprintf( fp, "Wpoint %d %d %d %d %d %d %d %d %d %d\n", 
        ch->wpoint[0], ch->wpoint[1], ch->wpoint[2], ch->wpoint[3], ch->wpoint[4],
        ch->wpoint[5], ch->wpoint[6], ch->wpoint[7], ch->wpoint[8], ch->wpoint[9] );
    if ( ch->gpoint > 0 )
        fprintf( fp, "Gpoint %d\n", ch->gpoint );
        fprintf( fp, "Fquit %d\n", ch->fquit );
    if ( ch->pcdata->timeout > 0 )
        fprintf( fp, "Tmout %d\n", ch->pcdata->timeout );
    fprintf( fp, "ACs %d %d %d %d\n",	
	ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3]);
    if (ch->wimpy !=0 )
	fprintf( fp, "Wimp  %d\n",	ch->wimpy	);
    fprintf( fp, "Attr %d %d %d %d %d\n",
	ch->perm_stat[STAT_STR],
	ch->perm_stat[STAT_INT],
	ch->perm_stat[STAT_WIS],
	ch->perm_stat[STAT_DEX],
	ch->perm_stat[STAT_CON] );

    fprintf (fp, "AMod %d %d %d %d %d\n",
	ch->mod_stat[STAT_STR],
	ch->mod_stat[STAT_INT],
	ch->mod_stat[STAT_WIS],
	ch->mod_stat[STAT_DEX],
	ch->mod_stat[STAT_CON] );

    if ( IS_NPC(ch) )
    {
	fprintf( fp, "Vnum %d\n",	ch->pIndexData->vnum	);
    }
    else
    {
	if(ch->pcdata->religion )
	{	fprintf(fp, "Rlg %s~\n", ch->pcdata->religion->name );
		fprintf(fp, "Rrnk %d\n", ch->pcdata->rank );
		fprintf(fp, "Blss %d\n", ch->pcdata->bless );
	}
	fprintf( fp, "Ptit %s~\n", ch->pcdata->pretitle );
	fprintf( fp, "Cord %d %d %d\n",  ch->pcdata->cordinate[CORD_X],  ch->pcdata->cordinate[CORD_Y], ch->pcdata->cordinate[CORD_Z] );
 	fprintf( fp, "Eml %s~\n",	ch->pcdata->email	);
	fprintf( fp, "Pass %s~\n",	ch->pcdata->pwd		);
	if (ch->pcdata->bamfin[0] != '\0')
	    fprintf( fp, "Bin  %s~\n",	ch->pcdata->bamfin);
	if (ch->pcdata->bamfout[0] != '\0')
		fprintf( fp, "Bout %s~\n",	ch->pcdata->bamfout);
	fprintf( fp, "Titl %s~\n",	ch->pcdata->title	);
    	fprintf( fp, "Pnts %d\n",   	ch->pcdata->points      );
	fprintf( fp, "TSex %d\n",	ch->pcdata->true_sex	);
	fprintf( fp, "LLev %d\n",	ch->pcdata->last_level	);
	fprintf( fp, "CExp %ld\n",	ch->pcdata->current_exp );
	fprintf( fp, "HMVP %ld %ld %ld\n", ch->pcdata->perm_hit, 
						   ch->pcdata->perm_mana,
						   ch->pcdata->perm_move);
        if (ch->pcdata->afk[0] != '\0')
            fprintf( fp, "AFK %s~\n",     ch->pcdata->afk);

	fprintf( fp, "Cnd  %d %d %d %d\n",
	    ch->pcdata->condition[0],
	    ch->pcdata->condition[1],
	    ch->pcdata->condition[2],
	    ch->pcdata->condition[3] );
        
        if(ch->pcdata->explored->set > 0 )
        {       EXPLORE_HOLDER *pExp;

                fprintf(fp, "Explored %d\n", ch->pcdata->explored->set);
                for(pExp = ch->pcdata->explored->bits ; pExp ; pExp = pExp->next )
                        fprintf(fp, "%d %d\n", pExp->mask, pExp->bits );
                fprintf(fp, "-1 -1\n" );
        }
	/*
	 * Write Colour Config Information.
	 */
	fprintf( fp, "Coloura     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->text[2],
		ch->pcdata->text[0],
		ch->pcdata->text[1],
		ch->pcdata->auction[2],
		ch->pcdata->auction[0],
		ch->pcdata->auction[1],
		ch->pcdata->gossip[2],
		ch->pcdata->gossip[0],
		ch->pcdata->gossip[1],
		ch->pcdata->music[2],
		ch->pcdata->music[0],
		ch->pcdata->music[1],
		ch->pcdata->question[2],
		ch->pcdata->question[0],
		ch->pcdata->question[1] );
	fprintf( fp, "Colourb     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->answer[2],
		ch->pcdata->answer[0],
		ch->pcdata->answer[1],
		ch->pcdata->quote[2],
		ch->pcdata->quote[0],
		ch->pcdata->quote[1],
		ch->pcdata->quote_text[2],
		ch->pcdata->quote_text[0],
		ch->pcdata->quote_text[1],
		ch->pcdata->immtalk_text[2],
		ch->pcdata->immtalk_text[0],
		ch->pcdata->immtalk_text[1],
		ch->pcdata->immtalk_type[2],
		ch->pcdata->immtalk_type[0],
		ch->pcdata->immtalk_type[1] );
	fprintf( fp, "Colourc     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->info[2],
		ch->pcdata->info[0],
		ch->pcdata->info[1],
		ch->pcdata->tell[2],
		ch->pcdata->tell[0],
		ch->pcdata->tell[1],
		ch->pcdata->reply[2],
		ch->pcdata->reply[0],
		ch->pcdata->reply[1],
		ch->pcdata->gtell_text[2],
		ch->pcdata->gtell_text[0],
		ch->pcdata->gtell_text[1],
		ch->pcdata->gtell_type[2],
		ch->pcdata->gtell_type[0],
		ch->pcdata->gtell_type[1] );
	fprintf( fp, "Colourd     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->room_title[2],
		ch->pcdata->room_title[0],
		ch->pcdata->room_title[1],
		ch->pcdata->room_text[2],
		ch->pcdata->room_text[0],
		ch->pcdata->room_text[1],
		ch->pcdata->room_exits[2],
		ch->pcdata->room_exits[0],
		ch->pcdata->room_exits[1],
		ch->pcdata->room_things[2],
		ch->pcdata->room_things[0],
		ch->pcdata->room_things[1],
		ch->pcdata->prompt[2],
		ch->pcdata->prompt[0],
		ch->pcdata->prompt[1] );
	fprintf( fp, "Coloure     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->fight_death[2],
		ch->pcdata->fight_death[0],
		ch->pcdata->fight_death[1],
		ch->pcdata->fight_yhit[2],
		ch->pcdata->fight_yhit[0],
		ch->pcdata->fight_yhit[1],
		ch->pcdata->fight_ohit[2],
		ch->pcdata->fight_ohit[0],
		ch->pcdata->fight_ohit[1],
		ch->pcdata->fight_thit[2],
		ch->pcdata->fight_thit[0],
		ch->pcdata->fight_thit[1],
		ch->pcdata->fight_skill[2],
		ch->pcdata->fight_skill[0],
		ch->pcdata->fight_skill[1] );
	fprintf( fp, "Colourf     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->wiznet[2],
		ch->pcdata->wiznet[0],
		ch->pcdata->wiznet[1],
		ch->pcdata->say[2],
		ch->pcdata->say[0],
		ch->pcdata->say[1],
		ch->pcdata->say_text[2],
		ch->pcdata->say_text[0],
		ch->pcdata->say_text[1],
		ch->pcdata->tell_text[2],
		ch->pcdata->tell_text[0],
		ch->pcdata->tell_text[1],
		ch->pcdata->reply_text[2],
		ch->pcdata->reply_text[0],
		ch->pcdata->reply_text[1] );
	fprintf( fp, "Colourg     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->auction_text[2],
		ch->pcdata->auction_text[0],
		ch->pcdata->auction_text[1],
		ch->pcdata->gossip_text[2],
		ch->pcdata->gossip_text[0],
		ch->pcdata->gossip_text[1],
		ch->pcdata->music_text[2],
		ch->pcdata->music_text[0],
		ch->pcdata->music_text[1],
		ch->pcdata->question_text[2],
		ch->pcdata->question_text[0],
		ch->pcdata->question_text[1],
		ch->pcdata->answer_text[2],
		ch->pcdata->answer_text[0],
		ch->pcdata->answer_text[1] );

	fprintf(fp, "GStats  %d ", MAX_GAMESTAT);
	for (i = 0; i < MAX_GAMESTAT; i++)
	    fprintf(fp, "%ld ", ch->pcdata->gamestat[i]);
	fprintf(fp, "\n");

	/* write alias */
        for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (ch->pcdata->alias[pos] == NULL
	    ||  ch->pcdata->alias_sub[pos] == NULL)
		break;

	    fprintf(fp,"Alias %s %s~\n",ch->pcdata->alias[pos],
		    ch->pcdata->alias_sub[pos]);
	}

	/* Save note board status */
	/* Save number of boards in case that number changes */
	fprintf (fp, "Boards       %d ", MAX_BOARD);
	for (i = 0; i < MAX_BOARD; i++)
	    fprintf (fp, "%s %ld ", boards[i].short_name, ch->pcdata->last_note[i]);
	fprintf (fp, "\n");
	if(ch->pcdata->nick_first)
	{
	    NICK_DATA *pNick;
	    for(pNick = ch->pcdata->nick_first ; pNick ; pNick = pNick->next )
	    	fprintf( fp, "Nick %s~ %s~ %s~\n", pNick->realname, pNick->shortd, pNick->nickname);
	}


	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )
	    {
		fprintf( fp, "Sk %d '%s'\n",
		    ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	}

	for ( gn = 0; gn < MAX_GROUP; gn++ )
        {
            if ( group_table[gn].name != NULL && ch->pcdata->group_known[gn])
            {
                fprintf( fp, "Gr '%s'\n",group_table[gn].name);
            }
        }
    }

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type>= MAX_SKILL)
	    continue;
	
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
	    skill_table[paf->type].name,
	    paf->where,
	    paf->level,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector
	    );
    }

    fprintf( fp, "End\n\n" );
    return;
}

/* write a pet */
void fwrite_pet( CHAR_DATA *pet, FILE *fp)
{
    AFFECT_DATA *paf;
    
    fprintf(fp,"#PET\n");
    
    fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);
    
    fprintf(fp,"Name %s~\n", pet->name);
    fprintf(fp,"LogO %ld\n", current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)
    	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
    	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
    	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
    	fprintf(fp,"Race %s~\n", race_table[pet->race].name);
    if (pet->clan)
        fprintf( fp, "Clan %s~\n",clan_table[pet->clan].name);
    fprintf( fp, "Powner %s~\n", pet->powner );
    fprintf(fp,"Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
    	fprintf(fp,"Levl %d\n", pet->level);
    fprintf(fp, "HMV  %ld %ld %ld %ld %ld %ld\n",
    	pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
    fprintf(fp, "Levlflux %d\n", pet->levelflux );
   if (pet->gold > 0)
    	fprintf(fp,"Gold %ld\n",pet->gold);
    if (pet->silver > 0)
	fprintf(fp,"Silv %ld\n",pet->silver);
    if (pet->exp > 0)
    	fprintf(fp, "Exp  %d\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
    	fprintf(fp, "Act  %s\n", print_flags(pet->act));
    if (pet->affected_by != pet->pIndexData->affected_by)
    	fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));
    if (pet->comm != 0)
    	fprintf(fp, "Comm %s\n", print_flags(pet->comm));
    if (pet->comm != 0)
        fprintf(fp, "Comm2 %s\n", print_flags(pet->comm2));
    fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
    	fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->pIndexData->alignment)
    	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
    	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
    	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
    	pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
    fprintf(fp, "Attr %d %d %d %d %d\n",
    	pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
    	pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
    	pet->perm_stat[STAT_CON]);
    fprintf(fp, "AMod %d %d %d %d %d\n",
    	pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
    	pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
    	pet->mod_stat[STAT_CON]);
    
    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
    	if (paf->type < 0 || paf->type >= MAX_SKILL)
    	    continue;
    	    
    	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
    	    skill_table[paf->type].name,
    	    paf->where, paf->level, paf->duration, paf->modifier,paf->location,
    	    paf->bitvector);
    }
    
    fprintf(fp,"End\n");
    return;
}
  

    
/* write a mount */
void fwrite_mount( CHAR_DATA *pet, FILE *fp)
{
    AFFECT_DATA *paf;
    
    fprintf(fp,"#MOUNT\n");

    fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);
    
    fprintf(fp,"Name %s~\n", pet->name);
    fprintf(fp,"LogO %ld\n", current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)
    	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
    	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
    	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
    	fprintf(fp,"Race %s~\n", race_table[pet->race].name);
    if (pet->clan)
        fprintf( fp, "Clan %s~\n",clan_table[pet->clan].name);
    fprintf(fp,"Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
    	fprintf(fp,"Levl %d\n", pet->level);
    fprintf(fp, "HMV  %ld %ld %ld %ld %ld %ld\n",
    	pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
    if (pet->gold > 0)
    	fprintf(fp,"Gold %ld\n",pet->gold);
    if (pet->silver > 0)
        fprintf(fp,"Silv %ld\n",pet->silver);
    if (pet->exp > 0)
    	fprintf(fp, "Exp  %d\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
        fprintf(fp, "Act  %s\n", print_flags(pet->act));
    if (pet->affected_by != pet->pIndexData->affected_by)
        fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));
    if (pet->comm != 0)
        fprintf(fp, "Comm %s\n", print_flags(pet->comm));
    if (pet->comm2 != 0)
        fprintf(fp, "Comm2 %s\n", print_flags(pet->comm2));
    fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
    	fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->pIndexData->alignment)
    	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
    	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
    	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
    	pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
    fprintf(fp, "Attr %d %d %d %d %d\n",
    	pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
    	pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
    	pet->perm_stat[STAT_CON]);
    fprintf(fp, "AMod %d %d %d %d %d\n",
    	pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
    	pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
    	pet->mod_stat[STAT_CON]);
    
    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
    	if (paf->type < 0 || paf->type >= MAX_SKILL)
    	    continue;
    	    
    	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
    	    skill_table[paf->type].name,
    	    paf->where, paf->level, paf->duration, paf->modifier,paf->location,
    	    paf->bitvector);
    }
    
    fprintf(fp,"End\n");
    return;
}
    


  
  
    
/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
	fwrite_obj( ch, obj->next_content, fp, iNest );

    /*
     * Castrate storage characters.
     */
    if(ch)
    if ( (ch->level < obj->level - 15 && obj->item_type != ITEM_CONTAINER)
    ||   obj->item_type == ITEM_KEY
    ||   (obj->item_type == ITEM_MAP && !obj->value[0]))
	return;

    fprintf( fp, "#O\n" );
    fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum        );
    if (!obj->pIndexData->new_format)
	fprintf( fp, "Oldstyle\n");
    if (obj->enchanted)
	fprintf( fp,"Enchanted\n");
    if(!ch && obj->in_room)
        fprintf(fp, "Inrm %d\n", obj->in_room->vnum );
    fprintf( fp, "Nest %d\n",	iNest	  	     );
    fprintf( fp, "Size   %d\n", obj->size                           ); 
    /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name)
    	fprintf( fp, "Name %s~\n",	obj->name		     );
    if ( obj->short_descr != obj->pIndexData->short_descr)
        fprintf( fp, "ShD  %s~\n",	obj->short_descr	     );
    if ( obj->description != obj->pIndexData->description)
        fprintf( fp, "Desc %s~\n",	obj->description	     );
    if ( obj->extra_flags != obj->pIndexData->extra_flags)
        fprintf( fp, "ExtF %d\n",	obj->extra_flags	     );
    if ( obj->extra2_flags != obj->pIndexData->extra2_flags           );
        fprintf( fp, "ExtF2 %d\n",       obj->extra2_flags            );
    if ( obj->wear_flags != obj->pIndexData->wear_flags)
        fprintf( fp, "WeaF %d\n",	obj->wear_flags		     );
    if ( obj->item_type != obj->pIndexData->item_type)
        fprintf( fp, "Ityp %d\n",	obj->item_type		     );
    if ( obj->weight != obj->pIndexData->weight)
        fprintf( fp, "Wt   %d\n",	obj->weight		     );
    if ( obj->condition != obj->pIndexData->condition)
	fprintf( fp, "Cond %d\n",	obj->condition		     );
	fprintf( fp, "strap %d\n",	obj->strap_loc		    );
	if ( obj->belted_vnum > 0 )
		fprintf( fp, "Belt %d\n", obj->belted_vnum );
    /* variable data */

    fprintf( fp, "Wear %d\n",   obj->wear_loc                );
    if (obj->level != obj->pIndexData->level)
        fprintf( fp, "Lev  %d\n",	obj->level		     );
    if ( obj->plevel > 0 )
        fprintf( fp, "Plev %d\n", obj->plevel );
    if ( obj->exp > 0 )
        fprintf( fp, "Exp %d\n", obj->exp );
    if ( obj->xp_tolevel > 0 )
        fprintf( fp, "Xptolevel %d\n", obj->xp_tolevel );
    if (obj->timer != 0)
        fprintf( fp, "Time %d\n",	obj->timer	     );
    fprintf( fp, "Cost %d\n",	obj->cost		     );
    if (obj->value[0] != obj->pIndexData->value[0]
    ||  obj->value[1] != obj->pIndexData->value[1]
    ||  obj->value[2] != obj->pIndexData->value[2]
    ||  obj->value[3] != obj->pIndexData->value[3]
    ||  obj->value[4] != obj->pIndexData->value[4]) 
    	fprintf( fp, "Val  %d %d %d %d %d\n",
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4]	     );
   
    if (obj->valueorig[0] != obj->pIndexData->value[0]
    ||  obj->valueorig[1] != obj->pIndexData->value[1]
    ||  obj->valueorig[2] != obj->pIndexData->value[2]
    ||  obj->valueorig[3] != obj->pIndexData->value[3]
    ||  obj->valueorig[4] != obj->pIndexData->value[4])       
     fprintf( fp, "ValOrig  %d %d %d %d %d\n",
            obj->valueorig[0], obj->valueorig[1], obj->valueorig[2],
                obj->valueorig[3], obj->valueorig[4]         );
                                                                        
    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
    case ITEM_PILL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1 '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2 '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;
        fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
            skill_table[paf->type].name,
            paf->where,
            paf->level,
            paf->duration,
            paf->modifier,
            paf->location,
            paf->bitvector
            );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExDe %s~ %s~\n",
	    ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_obj( ch, obj->contains, fp, iNest + 1 );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    char strsave[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat, iClass, i;

    ch = new_char();
    ch->pcdata = new_pcdata();
    if(d)   
    {   d->character			= ch;
        ch->desc				= d;
    }

    ch->pcdata->afk                     = str_dup ("");
    ch->pcdata->pretitle		= str_dup("");
    ch->pcdata->last_typed		= current_time;
    ch->pcdata->email			= str_dup( "none" );
    ch->name				= str_dup( name );
    ch->id				    = get_pc_id();
    ch->race				= race_lookup("human");
    ch->act				    = PLR_NOSUMMON;
    ch->comm				= COMM_COMBINE | COMM_PROMPT;
    for(iClass = 0; iClass < MAX_MCLASS; iClass++)
        ch->klass[iClass]   = -1;
    ch->prompt 				= str_dup("<%hhp %mm %vmv> ");
    ch->pcdata->confirm_delete		= FALSE;
    ch->pcdata->board                   = &boards[DEFAULT_BOARD];
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->hometown			= 0;
    ch->levelflux                       = 0;
    ch->pcdata->title			= str_dup( "" );
    ch->pcdata->title2                  = str_dup( "" );
    ch->pcdata->immtitle                = str_dup( "" );
    for (stat =0; stat < MAX_STATS; stat++)
	ch->perm_stat[stat]		= 13;
    ch->pcdata->condition[COND_THIRST]	= 48; 
    ch->pcdata->condition[COND_FULL]	= 48;
    ch->pcdata->condition[COND_HUNGER]	= 48; 
    ch->pcdata->condition[COND_TIRED]   = 48;
    ch->pcdata->mvolume                 = 50;
    ch->pcdata->svolume                 = 75;
    for (i = 0; i < MAX_GAMESTAT; i++)
         ch->pcdata->gamestat[i]        = 0;    
    ch->pcdata->boxrented               = 0;
    for ( i = 0; i < 3 ; i++ )
         ch->pcdata->cordinate[i]       = 0;
    ch->pcdata->item_box                = 0;  
    ch->pcdata->kills_mob	        = 0;
    ch->pcdata->kills_pc	        = 0;
    ch->pcdata->deaths_mob	        = 0;
    ch->pcdata->deaths_pc	        = 0;
    ch->pcdata->tracking                = NULL;
    ch->pcdata->track_count             = 0;
    ch->pcdata->trivia                  = 0;
    ch->pcdata->totaltpoints            = 0;
    end_quest(ch, 0);
    ch->pcdata->spouse                  = str_dup( "" );
    ch->pcdata->nick_first		= NULL;
    ch->pcdata->security		= 0;	/* OLC */
    ch->bloodthirst 		= 0;
    ch->bites			= 0;
    ch->isvamp			= FALSE;
    ch->iswere			= FALSE;
    ch->wereform		= 0;
    ch->pcdata->detail_map_x = 4;
    ch->pcdata->detail_map_y = 3;    
    ch->pcdata->suppress        = SUPPRESS_SHIELD_COMP;   
    ch->pcdata->text[0]		= ( NORMAL );
    ch->pcdata->text[1]		= ( WHITE );
    ch->pcdata->text[2]		= 0;
    ch->pcdata->auction[0]	= ( BRIGHT );
    ch->pcdata->auction[1]	= ( YELLOW );
    ch->pcdata->auction[2]	= 0;
    ch->pcdata->auction_text[0]	= ( BRIGHT );
    ch->pcdata->auction_text[1]	= ( WHITE );
    ch->pcdata->auction_text[2]	= 0;
    ch->pcdata->gossip[0]	= ( NORMAL );
    ch->pcdata->gossip[1]	= ( MAGENTA );
    ch->pcdata->gossip[2]	= 0;
    ch->pcdata->gossip_text[0]	= ( BRIGHT );
    ch->pcdata->gossip_text[1]	= ( MAGENTA );
    ch->pcdata->gossip_text[2]	= 0;
    ch->pcdata->music[0]	= ( NORMAL );
    ch->pcdata->music[1]	= ( RED );
    ch->pcdata->music[2]	= 0;
    ch->pcdata->music_text[0]	= ( BRIGHT );
    ch->pcdata->music_text[1]	= ( RED );
    ch->pcdata->music_text[2]	= 0;
    ch->pcdata->question[0]	= ( BRIGHT );
    ch->pcdata->question[1]	= ( YELLOW );
    ch->pcdata->question[2]	= 0;
    ch->pcdata->question_text[0] = ( BRIGHT );
    ch->pcdata->question_text[1] = ( WHITE );
    ch->pcdata->question_text[2] = 0;
    ch->pcdata->answer[0]	= ( BRIGHT );
    ch->pcdata->answer[1]	= ( YELLOW );
    ch->pcdata->answer[2]	= 0;
    ch->pcdata->answer_text[0]	= ( BRIGHT );
    ch->pcdata->answer_text[1]	= ( WHITE );
    ch->pcdata->answer_text[2]	= 0;
    ch->pcdata->quote[0]	= ( NORMAL );
    ch->pcdata->quote[1]	= ( YELLOW );
    ch->pcdata->quote[2]	= 0;
    ch->pcdata->quote_text[0]	= ( NORMAL );
    ch->pcdata->quote_text[1]	= ( GREEN );
    ch->pcdata->quote_text[2]	= 0;
    ch->pcdata->immtalk_text[0]	= ( NORMAL );
    ch->pcdata->immtalk_text[1]	= ( CYAN );
    ch->pcdata->immtalk_text[2]	= 0;
    ch->pcdata->immtalk_type[0]	= ( NORMAL );
    ch->pcdata->immtalk_type[1]	= ( YELLOW );
    ch->pcdata->immtalk_type[2]	= 0;
    ch->pcdata->info[0]		= ( BRIGHT );
    ch->pcdata->info[1]		= ( YELLOW );
    ch->pcdata->info[2]		= 1;
    ch->pcdata->say[0]		= ( NORMAL );
    ch->pcdata->say[1]		= ( GREEN );
    ch->pcdata->say[2]		= 0;
    ch->pcdata->say_text[0]	= ( BRIGHT );
    ch->pcdata->say_text[1]	= ( GREEN );
    ch->pcdata->say_text[2]	= 0;
    ch->pcdata->tell[0]		= ( NORMAL );
    ch->pcdata->tell[1]		= ( GREEN );
    ch->pcdata->tell[2]		= 0;
    ch->pcdata->tell_text[0]	= ( BRIGHT );
    ch->pcdata->tell_text[1]	= ( GREEN );
    ch->pcdata->tell_text[2]	= 0;
    ch->pcdata->reply[0]	= ( NORMAL );
    ch->pcdata->reply[1]	= ( GREEN );
    ch->pcdata->reply[2]	= 0;
    ch->pcdata->reply_text[0]	= ( BRIGHT );
    ch->pcdata->reply_text[1]	= ( GREEN );
    ch->pcdata->reply_text[2]	= 0;
    ch->pcdata->gtell_text[0]	= ( NORMAL );
    ch->pcdata->gtell_text[1]	= ( GREEN );
    ch->pcdata->gtell_text[2]	= 0;
    ch->pcdata->gtell_type[0]	= ( NORMAL );
    ch->pcdata->gtell_type[1]	= ( RED );
    ch->pcdata->gtell_type[2]	= 0;
    ch->pcdata->wiznet[0]		= ( NORMAL );
    ch->pcdata->wiznet[1]		= ( GREEN );
    ch->pcdata->wiznet[2]		= 0;
    ch->pcdata->room_title[0]	= ( NORMAL );
    ch->pcdata->room_title[1]	= ( CYAN );
    ch->pcdata->room_title[2]	= 0;
    ch->pcdata->room_text[0]	= ( NORMAL );
    ch->pcdata->room_text[1]	= ( WHITE );
    ch->pcdata->room_text[2]	= 0;
    ch->pcdata->room_exits[0]	= ( NORMAL );
    ch->pcdata->room_exits[1]	= ( GREEN );
    ch->pcdata->room_exits[2]	= 0;
    ch->pcdata->room_things[0]	= ( NORMAL );
    ch->pcdata->room_things[1]	= ( CYAN );
    ch->pcdata->room_things[2]	= 0;
    ch->pcdata->prompt[0]	= ( NORMAL );
    ch->pcdata->prompt[1]	= ( CYAN );
    ch->pcdata->prompt[2]	= 0;
    ch->pcdata->fight_death[0]	= ( BRIGHT );
    ch->pcdata->fight_death[1]	= ( RED );
    ch->pcdata->fight_death[2]	= 0;
    ch->pcdata->fight_yhit[0]	= ( NORMAL );
    ch->pcdata->fight_yhit[1]	= ( GREEN );
    ch->pcdata->fight_yhit[2]	= 0;
    ch->pcdata->fight_ohit[0]	= ( NORMAL );
    ch->pcdata->fight_ohit[1]	= ( YELLOW );
    ch->pcdata->fight_ohit[2]	= 0;
    ch->pcdata->fight_thit[0]	= ( NORMAL );
    ch->pcdata->fight_thit[1]	= ( RED );
    ch->pcdata->fight_thit[2]	= 0;
    ch->pcdata->fight_skill[0]	= ( BRIGHT );
    ch->pcdata->fight_skill[1]	= ( WHITE );
    ch->pcdata->fight_skill[2]	= 0;
	ch->pcdata->religion = NULL;
	ch->pcdata->bless = 0;
	ch->pcdata->rank = 0;
    ch->turned			= FALSE;
    ch->pcdata->in_mine		= NULL;
    ch->pcdata->in_shaft	= NULL;
    for(i = 0; i < MAX_CBOOL ; i++ )
	ch->pcdata->creation[i] = FALSE;

    if (gquest_info.running == GQUEST_OFF && IS_SET(ch->act, PLR_GQUEST))
    {
        REMOVE_BIT(ch->act, PLR_GQUEST);
        reset_gqmob(ch, 0);
    }

    found = FALSE;

    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
    if (file_exists(strsave))
    {
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
    #endif

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ( !file_exists(strsave) )
	return FALSE;
    if ( file_exists(strsave) )
    {
	int iNest;

	fp = file_open( strsave, "r" );

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
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
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp );
            else if ( !str_cmp( word, "BOX"    ) ) fread_bobj ( ch, fp ); 
	    else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
	    else if ( !str_cmp( word, "MOUNT"  ) ) fread_mount( ch, fp );	    
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	file_close( fp );
    }

    /* initialize race */
    if (found)
    {
	int i;

	if (ch->race == 0)
	    ch->race = race_lookup("human");

	ch->size = pc_race_table[ch->race].size;
	ch->dam_type = 17; /*punch */

	for (i = 0; i < 5; i++)
	{
	    if (pc_race_table[ch->race].skills[i] == NULL)
		break;
            group_add(ch,pc_race_table[ch->race].skills[i],FALSE);   
	}
	ch->affected_by = ch->affected_by|race_table[ch->race].aff;
	ch->imm_flags	= ch->imm_flags | race_table[ch->race].imm;
	ch->res_flags	= ch->res_flags | race_table[ch->race].res;
	ch->vuln_flags	= ch->vuln_flags | race_table[ch->race].vuln;
	ch->form	= race_table[ch->race].form;
	ch->parts	= race_table[ch->race].parts;
    }

	
    /* RT initialize skills */

    if (found && ch->version < 2)  /* need to add the new skills */
    {
	group_add(ch,"rom basics",FALSE);
	add_base_groups(ch);
	add_default_groups(ch);
	ch->pcdata->learned[skill_lookup("recall")] = 50;
    }
 
    /* fix levels */
    if (found && ch->version < 3 && (ch->level > 35 || ch->trust > 35))
    {
	switch (ch->level)
	{
	    case(40) : ch->level = 159;	 break;  /* imp -> imp */
	    case(39) : ch->level = 157;  break;	/* god -> supreme */
	    case(38) : ch->level = 155;  break;	/* deity -> god */
	    case(37) : ch->level = 152;  break;	/* angel -> demigod */
	}

        switch (ch->trust)
        {
            case(40) : ch->trust = 159;  break;	/* imp -> imp */
            case(39) : ch->trust = 157;  break;	/* god -> supreme */
            case(38) : ch->trust = 155;  break;	/* deity -> god */
            case(37) : ch->trust = 152;  break;	/* angel -> demigod */
            case(36) : ch->trust = 150;  break;	/* hero -> hero */
        }
    }

    /* ream gold */
    if (found && ch->version < 4)
    {
	ch->gold   /= 100;
    }
  
    if (found && ch->version < 4 )
    {
        ch->pcdata->learned[skill_lookup("spell craft")] = 1;
    }       

    if (found && !IS_SET(ch->pcdata->suppress,SUPPRESS_SHIELD)
              && !IS_SET(ch->pcdata->suppress,SUPPRESS_SHIELD_COMP))
              SET_BIT(ch->pcdata->suppress,SUPPRESS_SHIELD_COMP);        

    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

/* provided to free strings */
#if defined(KEYS)
#undef KEYS
#endif

#define KEYS( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    free_string(field);			\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    const char *word;
    bool fMatch;
    int count = 0;
    int lastlogoff = current_time;
    int percent, i;

    sprintf(buf,"Loading %s.",ch->name);
    log_string(buf);

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Act",		ch->act,		fread_flag( fp ) );
	    KEY( "AffectedBy",	ch->affected_by,	fread_flag( fp ) );
	    KEY( "AfBy",	ch->affected_by,	fread_flag( fp ) );
            KEY( "AFK",         ch->pcdata->afk,        fread_string(fp));
	    KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
	    KEY( "Alig",	ch->alignment,		fread_number( fp ) );

	    if (!str_cmp( word, "Alia"))
	    {
		if (count >= MAX_ALIAS)
		{
		    fread_to_eol(fp);
		    fMatch = TRUE;
		    break;
		}

		ch->pcdata->alias[count] 	= str_dup(fread_word(fp));
		ch->pcdata->alias_sub[count]	= str_dup(fread_word(fp));
		count++;
		fMatch = TRUE;
		break;
	    }

            if (!str_cmp( word, "Alias"))
            {
                if (count >= MAX_ALIAS)
                {
                    fread_to_eol(fp);
                    fMatch = TRUE;
                    break;
                }
 
                ch->pcdata->alias[count]        = str_dup(fread_word(fp));
                ch->pcdata->alias_sub[count]    = fread_string(fp);
                count++;
                fMatch = TRUE;
                break;
            }

	    if (!str_cmp( word, "AC") || !str_cmp(word,"Armor"))
	    {
		fread_to_eol(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word,"ACs"))
	    {
		int i;

		for (i = 0; i < 4; i++)
		    ch->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= ch->affected;
		ch->affected	= paf;
		fMatch = TRUE;
		break;
	    }

            if (!str_cmp(word, "Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                    paf->type = sn;
 
                paf->where  = fread_number(fp);
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = ch->affected;
                ch->affected    = paf;
                fMatch = TRUE;
                break;
            }

	    if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))
	    {
		int stat;
		for (stat = 0; stat < MAX_STATS; stat ++)
		   ch->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))
	    {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    ch->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'B':
	    KEY(  "Blss",       ch->pcdata->bless,	fread_number( fp ) );
	    KEYS( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEYS( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
            KEY(  "Bcount",     ch->pcdata->bcount,     fread_number( fp ) );
	    KEYS( "Bin",		ch->pcdata->bamfin,	fread_string( fp ) );
	    KEYS( "Bout",	ch->pcdata->bamfout,	fread_string( fp ) );
            KEY(  "Box",        ch->pcdata->boxrented,  fread_number( fp ) );   
	    KEY( "Bldthrst",    ch->bloodthirst,	fread_number( fp ) );
  	    KEY( "Bites",	ch->bites,		fread_number( fp ) );
	    /* Read in board status */
	    if (!str_cmp(word, "Boards" ))
	    {
		int i,num = fread_number (fp); /* number of boards saved */
                char *boardname;

                for (; num ; num-- ) /* for each of the board saved */
                {
		    boardname = fread_word (fp);
		    i = board_lookup (boardname); /* find board number */

		    if (i == BOARD_NOTFOUND) /* Does board still exist ? */
                    {
			sprintf (buf, "fread_char: %s had unknown board name: %s. Skipped.",
			    ch->name, boardname);
			log_string (buf);
			fread_number (fp); /* read last_note and skip info */
		    }
		    else /* Save it */
			ch->pcdata->last_note[i] = fread_number (fp);
		} /* for */

                fMatch = TRUE;
	      } /* Boards */
	    break;

	case 'C':
	    KEY( "CExp",	ch->pcdata->current_exp, fread_number( fp ) );
            KEY( "Cability",    ch->cast_ability,       fread_number( fp ) );
            KEY( "Clevel",      ch->cast_level,         fread_number( fp ) );
  
            if ( !str_cmp( word, "Cord"  ) )
	    {
                fMatch = TRUE;
	   	for( i = 0 ; i < 3 ; i++ )
		   ch->pcdata->cordinate[i] = fread_number(fp);
		break;
	    }
            if ( !str_cmp( word, "Cla" ) ||  !str_cmp( word, "Class" ))
            {
                int iClass=0;
                fMatch = TRUE;
                for ( iClass = 0; iClass < MAX_MCLASS; iClass++ )
                     ch->klass[ iClass ] = -1;
 
                for ( iClass = 0; iClass < MAX_MCLASS; iClass++ )
                {
                     ch->klass[ iClass ] = fread_number( fp );
                     if ( ch->klass[ iClass ] == -1 || ch->version <= 5)
                         break;
                }
            }       
            
            if ( !str_cmp( word, "Clan" ) ) 
            {
                char *tmp = fread_string(fp);
                ch->clan = clan_lookup(tmp);
                free_string(tmp);
                fMatch = TRUE;
                break;
            }

	    KEY( "Comm",	ch->comm,		fread_flag( fp ) ); 
            KEY( "Comm2",       ch->comm2,              fread_flag( fp ) );

	    if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))
	    {
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
            if (!str_cmp(word,"Cnd"))
            {
                ch->pcdata->condition[0] = fread_number( fp );
                ch->pcdata->condition[1] = fread_number( fp );
                ch->pcdata->condition[2] = fread_number( fp );
		ch->pcdata->condition[3] = fread_number( fp );
                fMatch = TRUE;
                break;
            }

	    if( !str_cmp( word, "Coloura" ) )
	    {
		LOAD_COLOUR( text )
		LOAD_COLOUR( auction )
		LOAD_COLOUR( gossip )
		LOAD_COLOUR( music )
		LOAD_COLOUR( question )
		fMatch = TRUE;
		break;
	    }
	    if( !str_cmp( word, "Colourb" ) )
	    {
		LOAD_COLOUR( answer )
		LOAD_COLOUR( quote )
		LOAD_COLOUR( quote_text )
		LOAD_COLOUR( immtalk_text )
		LOAD_COLOUR( immtalk_type )
		fMatch = TRUE;
		break;
	    }
	    if( !str_cmp( word, "Colourc" ) )
	    {
		LOAD_COLOUR( info )
		LOAD_COLOUR( tell )
		LOAD_COLOUR( reply )
		LOAD_COLOUR( gtell_text )
		LOAD_COLOUR( gtell_type )
		fMatch = TRUE;
		break;
	    }
	    if( !str_cmp( word, "Colourd" ) )
	    {
		LOAD_COLOUR( room_title )
		LOAD_COLOUR( room_text )
		LOAD_COLOUR( room_exits )
		LOAD_COLOUR( room_things )
		LOAD_COLOUR( prompt )
		fMatch = TRUE;
		break;
	    }
	    if( !str_cmp( word, "Coloure" ) )
	    {
		LOAD_COLOUR( fight_death )
		LOAD_COLOUR( fight_yhit )
		LOAD_COLOUR( fight_ohit )
		LOAD_COLOUR( fight_thit )
		LOAD_COLOUR( fight_skill )
		fMatch = TRUE;
		break;
	    }
	    if( !str_cmp( word, "Colourf" ) )
	    {
		LOAD_COLOUR( wiznet )
		LOAD_COLOUR( say )
		LOAD_COLOUR( say_text )
		LOAD_COLOUR( tell_text )
		LOAD_COLOUR( reply_text )
		fMatch = TRUE;
		break;
	    }
	    if( !str_cmp( word, "Colourg" ) )
	    {
		LOAD_COLOUR( auction_text )
		LOAD_COLOUR( gossip_text )
		LOAD_COLOUR( music_text )
		LOAD_COLOUR( question_text )
		LOAD_COLOUR( answer_text )
		fMatch = TRUE;
		break;
	    }
          
	    break;

	case 'D':
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "Dam",		ch->damroll,		fread_number( fp ) );
            KEY( "Dblq",        ch->pcdata->dblq,       fread_number( fp ) );
	    KEYS( "Description",	ch->description,	fread_string( fp ) );
	    KEYS( "Desc",	ch->description,	fread_string( fp ) );

            if ( !str_cmp( word, "Deat" ))
	    {
			ch->pcdata->deaths_mob = fread_number( fp );
			ch->pcdata->deaths_pc = fread_number( fp );
			fMatch = TRUE;
			break;
	    }
            KEY( "Duration",    ch->pcdata->duration,   fread_number( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

		percent = UMIN(percent,100);
 
    		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
        	    ch->hit	+= (ch->max_hit - ch->hit) * percent / 100;
        	    ch->mana    += (ch->max_mana - ch->mana) * percent / 100;
        	    ch->move    += (ch->max_move - ch->move)* percent / 100;
    		}
		return;
	    }
	    KEY ("Eml", ch->pcdata->email, fread_string(fp) );
            KEY ("Eye", ch->pcdata->eye, fread_flag( fp ) );
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
            if(!str_cmp(word, "Explored") )
            {       int mask, bit;
                    EXPLORE_HOLDER *pExp;
                    ch->pcdata->explored->set = fread_number(fp);
                    while(1)
                    {       mask = fread_number(fp);
                            bit = fread_number(fp);
                            if(mask == -1)
                                    break;
                            for(pExp = ch->pcdata->explored->bits ; pExp ; pExp = pExp->next )
                                    if(pExp->mask == mask)
                                            break;
                            if(!pExp)
                            {       pExp = (EXPLORE_HOLDER *)calloc(1, sizeof(*pExp) );
                                    pExp->next = ch->pcdata->explored->bits;
                                    ch->pcdata->explored->bits = pExp;
                                    pExp->mask = mask;
                            }
                            pExp->bits = bit;
                    }
                    fMatch = TRUE;
            }
            
	    break;

        case 'F':
            KEY( "Fquit",       ch->fquit,              fread_number( fp ) );

            break;

	case 'G':
	    KEY( "Gold",	ch->gold,		fread_number( fp ) );
            KEY( "Gold_bank",   ch->pcdata->gold_bank,  fread_number(fp  ) );
            KEY( "Gpoint",      ch->gpoint,             fread_number( fp ) );            

            if (!str_cmp(word, "GStats"))
            {
                int i, maxStat = 0;
                maxStat = fread_number(fp);
                fMatch = TRUE;
                for (i = 0; i < maxStat; i++)
                     ch->pcdata->gamestat[i] = fread_number(fp);
                break;
            }       

            if ( !str_cmp( word, "Group" )  || !str_cmp(word,"Gr"))
            {
                int gn;
                char *temp;
 
                temp = fread_word( fp ) ;
                gn = group_lookup(temp);
                /* gn    = group_lookup( fread_word( fp ) ); */
                if ( gn < 0 )
                {
                    fprintf(stderr,"%s",temp);
                    bug( "Fread_char: unknown group. ", 0 );
                }
                else
		    gn_add(ch,gn);
                fMatch = TRUE;
            }

            if (!str_cmp(word, "GQmobs"))
	    {
		int maxMob = fread_number(fp);
		int i;

		for (i = 0; i < maxMob; i++)
		    ch->pcdata->gq_mobs[i] = fread_number(fp);
		fMatch = TRUE;
	    }

	    break;

	case 'H':
            KEY( "Hair", ch->pcdata->hair, fread_flag( fp ) );
            KEY( "Hdam",        ch->pcdata->hdam,       fread_number( fp ) );
            KEY( "Height", ch->pcdata->height, fread_flag( fp ) );
	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
	    KEY( "Hit",		ch->hitroll,		fread_number( fp ) );
            KEY( "Hmtown",	ch->hometown,		fread_number( fp ) );
            KEYS("Host",        ch->pcdata->host,       fread_string( fp ) );
            KEY( "HVnum",       ch->pcdata->h_vnum,     fread_number( fp ) );
            KEY( "HResets",     ch->pcdata->horesets,   fread_number( fp ) );
            KEY( "HOResets",    ch->pcdata->horesets,   fread_number( fp ) );
            KEY( "HMResets",    ch->pcdata->hmresets,   fread_number( fp ) ); 

	    if ( !str_cmp( word, "HpManaMove" ) || !str_cmp(word,"HMV"))
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

            if ( !str_cmp( word, "HpManaMovePerm" ) || !str_cmp(word,"HMVP"))
            {
                ch->pcdata->perm_hit	= fread_number( fp );
                ch->pcdata->perm_mana   = fread_number( fp );
                ch->pcdata->perm_move   = fread_number( fp );
                fMatch = TRUE;
                break;
            }
      
	    break;

	case 'I':
	    KEY( "Id",		ch->id,			fread_number( fp ) );
	    KEY( "InvisLevel",	ch->invis_level,	fread_number( fp ) );
	    KEY( "Inco",	ch->incog_level,	fread_number( fp ) );
	    KEY( "Invi",	ch->invis_level,	fread_number( fp ) );
            KEY( "Qps",         ch->pcdata->qps,        fread_number( fp ) );
            if ( !str_cmp( word, "immtitle" ) )
            {
                ch->pcdata->immtitle = fread_string( fp );
                fMatch = TRUE;
                break;
            }      
            if(!str_cmp(word, "Isvmp" ) )
	    {
		if(!str_cmp(fread_word(fp), "TRUE") )
		   ch->isvamp = TRUE;
		else
		   ch->isvamp = FALSE;
		fMatch = TRUE;
		break;
	    }
            if(!str_cmp(word, "Iswere" ) )
	    {
		if(!str_cmp(fread_word(fp), "TRUE") )
		   ch->iswere = TRUE;
		else
		   ch->iswere = FALSE;
		fMatch = TRUE;
		break;
	    }
	    break;

        case 'K':
	    if ( !str_cmp( word, "Kill" ))
	    {
			ch->pcdata->kills_mob = fread_number( fp );
			ch->pcdata->kills_pc = fread_number( fp );
			fMatch = TRUE;
			break;
	    }
	    break;

	case 'L':
	    KEY( "LastLevel",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "LLev",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "Level",	ch->level,		fread_number( fp ) );
	    KEY( "Lev",		ch->level,		fread_number( fp ) );
	    KEY( "Levl",	ch->level,		fread_number( fp ) );
            KEY( "Lflux",       ch->levelflux,          fread_number( fp ) );
            if (!str_cmp(word,"LogO"))
            {
                lastlogoff              = fread_number( fp );
                ch->llogoff             = (time_t) lastlogoff;
                fMatch = TRUE;
                break;
            }    
	    KEYS( "LongDescr",	ch->long_descr,		fread_string( fp ) );
	    KEYS( "LnD",		ch->long_descr,		fread_string( fp ) );
	    break;

        case 'M':
            KEY( "Mailid",      ch->pcdata->mailid,     fread_number( fp ) );
            KEY( "Mdam",        ch->pcdata->mdam,       fread_number( fp ) );
            KEY( "Mvolume",     ch->pcdata->mvolume,    fread_number( fp ) );
            KEY( "Msp",         ch->pcdata->msp,        fread_number( fp ) );
            KEY( "Mxp",         ch->pcdata->mxp,        fread_number( fp ) );
            break;

	case 'N':
	    KEYS( "Name",	ch->name,		fread_string( fp ) );
            if(!str_cmp(word, "Nick") )
	    {
		NICK_DATA *pNick;
		pNick = new_nick();
		pNick->realname  = fread_string(fp);    
		pNick->shortd = fread_string(fp);
		pNick->nickname = fread_string(fp);
		nick_to_char(ch, pNick);
		fMatch = TRUE;
		break;
	    }
	   

          break;

	case 'P':
	    KEYS( "Password",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEYS( "Pass",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Played",	ch->played,		fread_number( fp ) );
	    KEY( "Plyd",	ch->played,		fread_number( fp ) );
	    KEY( "Points",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Pnts",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Position",	ch->position,		fread_number( fp ) );
	    KEY( "Pos",		ch->position,		fread_number( fp ) );
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Prac",	ch->practice,		fread_number( fp ) );
        KEYS( "Prompt",     ch->prompt,             fread_string( fp ) );
 	    KEYS( "Prom",	ch->prompt,		fread_string( fp ) );
	    KEYS( "Ptit",	ch->pcdata->pretitle,   fread_string( fp ) );
	    break;

        case 'Q':
            KEY( "QuestPnts",   ch->pcdata->questpoints,fread_number( fp ));
	    KEY( "QuestNext",   ch->pcdata->nextquest,  fread_number( fp ));
	    KEY( "QuestCount",  ch->pcdata->countdown,  fread_number( fp ));
	    KEY( "QuestLoc",    ch->pcdata->questloc,   fread_number( fp ));
	    KEY( "QuestObj",    ch->pcdata->questobj,   fread_number( fp ));
	    KEY( "QuestGiver",  ch->pcdata->questgiver, fread_number( fp ));
	    KEY( "QuestMob",    ch->pcdata->questmob,   fread_number( fp ));
            break;

	case 'R':

            if ( !str_cmp( word, "Race" ) ) 
            {
                char *tmp = fread_string(fp);
                ch->race = race_lookup(tmp);
				if(ch->race == 0 )
					ch->race = 1;
                free_string(tmp);
                fMatch = TRUE;
                break;
            }
			if( !str_cmp( word, "Rlg" ) )
			{	char *string;
				string = fread_string(fp);
				if( (ch->pcdata->religion = religion_lookup(string ) ) == NULL )
					if( ( ch->pcdata->religion = faction_lookup(string) ) == NULL )
						logf2("BUG: Player found with invalid religion! %s", string );
				fMatch = TRUE;
				free_string(string);
				break;
			}
			KEY( "Rrnk",	 ch->pcdata->rank,		fread_number( fp ) );
            KEY( "Rank",     ch->rank,              fread_number( fp ) );
            KEY( "Rroom",    ch->pcdata->rentroom,  fread_number( fp ) );

	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( ch->in_room == NULL )
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		else if (IS_SET(ch->in_room->room_flags, ROOM_ARENA))
		    ch->in_room = get_room_index(ROOM_VNUM_TEMPLE);
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'S':
	    KEY( "SavingThrow",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Save",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Scro",	ch->lines,		fread_number( fp ) );
	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    KEYS( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
            KEY( "Shares",	ch->pcdata->shares,	fread_number( fp ) );
            KEY( "Shares_bought",ch->pcdata->shares_bought,fread_number(fp ) );
            KEY( "Share_level",	ch->pcdata->share_level,fread_number( fp ) );
	    KEYS( "ShD",	ch->short_descr,	fread_string( fp ) );
            KEYS( "Spouse",     ch->pcdata->spouse,     fread_string( fp ) );
	    KEY( "Sec",         ch->pcdata->security,	fread_number( fp ) );	/* OLC */
            KEY( "Silv",        ch->silver,             fread_number( fp ) );
            KEY( "Silv_bank",   ch->pcdata->silver_bank,fread_number( fp ) );
            KEY("Stance",       ch->stance,             fread_number( fp ) );
            KEY("Supp",         ch->pcdata->suppress,   fread_flag( fp ) );  
            KEY("Svolume",      ch->pcdata->svolume,    fread_number( fp ) );

	    if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk"))
	    {
		int sn;
		int value;
		char *temp;

		value = fread_number( fp );
		temp = fread_word( fp ) ;
		sn = skill_lookup(temp);
		/* sn    = skill_lookup( fread_word( fp ) ); */
		if ( sn < 0 )
		{
		    fprintf(stderr,"%s",temp);
		    bug( "Fread_char: unknown skill. ", 0 );
		}
		else
		    ch->pcdata->learned[sn] = value;
		fMatch = TRUE;
	    }

            if (!str_cmp(word, "StayRace"))
            {
                ch->pcdata->stay_race = TRUE;
                fMatch = TRUE;
                break;
            }            

	    break;

	case 'T':
            KEY( "Tmout",       ch->pcdata->timeout,    fread_number( fp ) );
            KEY ("TotalTPnts",  ch->pcdata->totaltpoints, fread_number( fp ) );
            KEY( "TrueSex",     ch->pcdata->true_sex,  	fread_number( fp ) );
	    KEY( "TSex",	ch->pcdata->true_sex,   fread_number( fp ) );
	    KEY( "Trai",	ch->train,		fread_number( fp ) );
            KEY( "Trivia",      ch->pcdata->trivia,     fread_number( fp ) );
	    KEY( "Trust",	ch->trust,		fread_number( fp ) );
	    KEY( "Tru",		ch->trust,		fread_number( fp ) );

	    if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
	    {
		ch->pcdata->title = fread_string( fp );
    		if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ',' 
		&&  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }

            if ( !str_cmp( word, "title2" ) )
            {
                ch->pcdata->title2 = fread_string( fp );
                fMatch = TRUE;
                break;
            }                       

	    break;

	case 'V':
	    KEY( "Version",     ch->version,		fread_number ( fp ) );
	    KEY( "Vers",	ch->version,		fread_number ( fp ) );
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
            if (!str_cmp(word,"Wpoint"))
            {
                int i;

                for (i = 0; i <= 9; i++)
                {
                     ch->wpoint[i] = fread_number(fp);
                }
                fMatch = TRUE;
                break;
            }  
 
            KEY( "Weight",      ch->pcdata->weight,     fread_flag( fp ) );
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wimp",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wizn",	ch->wiznet,		fread_flag( fp ) );
	    KEY( "Wereform",	ch->wereform,		fread_number(fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    bug( "Fread_char: No match", 0 );
	    bug( word, 0 );
	    fread_to_eol( fp );
	}
    }
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp )
{
    const char *word;
    CHAR_DATA *pet;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
    	int vnum;
    	
    	vnum = fread_number(fp);
    	if (get_mob_index(vnum) == NULL)
	{
    	    bug("Fread_pet: bad vnum %d.",vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
	}
    	else
    	    pet = create_mobile(get_mob_index(vnum));
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }
    
    for ( ; ; )
    {
    	word 	= feof(fp) ? "END" : fread_word(fp);
    	fMatch = FALSE;
    	
    	switch (UPPER(word[0]))
    	{
    	case '*':
    	    fMatch = TRUE;
    	    fread_to_eol(fp);
    	    break;
    		
    	case 'A':
    	    KEY( "Act",		pet->act,		fread_flag(fp));
    	    KEY( "AfBy",	pet->affected_by,	fread_flag(fp));
    	    KEY( "Alig",	pet->alignment,		fread_number(fp));
    	    
    	    if (!str_cmp(word,"ACs"))
    	    {
    	    	int i;
    	    	
    	    	for (i = 0; i < 4; i++)
    	    	    pet->armor[i] = fread_number(fp);
    	    	fMatch = TRUE;
    	    	break;
    	    }
    	    
    	    if (!str_cmp(word,"AffD"))
    	    {
    	    	AFFECT_DATA *paf;
    	    	int sn;
    	    	
    	    	paf = new_affect();
    	    	
    	    	sn = skill_lookup(fread_word(fp));
    	     	if (sn < 0)
    	     	    bug("Fread_char: unknown skill.",0);
    	     	else
    	     	   paf->type = sn;
    	     	   
    	     	paf->level	= fread_number(fp);
    	     	paf->duration	= fread_number(fp);
    	     	paf->modifier	= fread_number(fp);
    	     	paf->location	= fread_number(fp);
    	     	paf->bitvector	= fread_number(fp);
    	     	paf->next	= pet->affected;
    	     	pet->affected	= paf;
    	     	fMatch		= TRUE;
    	     	break;
    	    }

            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                   paf->type = sn;
 
		paf->where	= fread_number(fp);
                paf->level      = fread_number(fp);
                paf->duration   = fread_number(fp);
                paf->modifier   = fread_number(fp);
                paf->location   = fread_number(fp);
                paf->bitvector  = fread_number(fp);
                paf->next       = pet->affected;
                pet->affected   = paf;
                fMatch          = TRUE;
                break;
            }
    	     
    	    if (!str_cmp(word,"AMod"))
    	    {
    	     	int stat;
    	     	
    	     	for (stat = 0; stat < MAX_STATS; stat++)
    	     	    pet->mod_stat[stat] = fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"Attr"))
    	    {
    	         int stat;
    	         
    	         for (stat = 0; stat < MAX_STATS; stat++)
    	             pet->perm_stat[stat] = fread_number(fp);
    	         fMatch = TRUE;
    	         break;
    	    }
    	    break;
    	     
    	 case 'C':
             KEY( "Clan",       pet->clan,       clan_lookup(fread_string(fp)));
    	     KEY( "Comm",	pet->comm,		fread_flag(fp));
             KEY( "Comm2",      pet->comm2,             fread_flag(fp));
    	     break;
    	     
    	 case 'D':
    	     KEY( "Dam",	pet->damroll,		fread_number(fp));
    	     KEY( "Desc",	pet->description,	fread_string(fp));
    	     break;
    	     
    	 case 'E':
    	     if (!str_cmp(word,"End"))
	     {
		pet->leader = ch;
		pet->master = ch;
		ch->pet = pet;
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
 
    		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
		    percent = UMIN(percent,100);
    		    pet->hit	+= (pet->max_hit - pet->hit) * percent / 100;
        	    pet->mana   += (pet->max_mana - pet->mana) * percent / 100;
        	    pet->move   += (pet->max_move - pet->move)* percent / 100;
    		}
    	     	return;
	     }
    	     KEY( "Exp",	pet->exp,		fread_number(fp));
    	     break;
    	     
    	 case 'G':
    	     KEY( "Gold",	pet->gold,		fread_number(fp));
    	     break;
    	     
    	 case 'H':
    	     KEY( "Hit",	pet->hitroll,		fread_number(fp));
    	     
    	     if (!str_cmp(word,"HMV"))
    	     {
    	     	pet->hit	= fread_number(fp);
    	     	pet->max_hit	= fread_number(fp);
    	     	pet->mana	= fread_number(fp);
    	     	pet->max_mana	= fread_number(fp);
    	     	pet->move	= fread_number(fp);
    	     	pet->max_move	= fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	     }
    	     break;
    	     
     	case 'L':
    	     KEY( "Levl",	pet->level,		fread_number(fp));
             KEY( "Levlflux",   pet->levelflux,         fread_number(fp));
    	     KEY( "LnD",	pet->long_descr,	fread_string(fp));
	     KEY( "LogO",	lastlogoff,		fread_number(fp));
    	     break;
    	     
    	case 'N':
    	     KEY( "Name",	pet->name,		fread_string(fp));
    	     break;
    	     
    	case 'P':
    	     KEY( "Pos",	pet->position,		fread_number(fp));
             KEY( "Powner",     pet->powner,            fread_string(fp));
    	     break;
    	     
	case 'R':
    	    KEY( "Race",	pet->race, race_lookup(fread_string(fp)));
    	    break;
 	    
    	case 'S' :
    	    KEY( "Save",	pet->saving_throw,	fread_number(fp));
    	    KEY( "Sex",		pet->sex,		fread_number(fp));
    	    KEY( "ShD",		pet->short_descr,	fread_string(fp));
            KEY( "Silv",        pet->silver,            fread_number( fp ) );
    	    break;
    	    
    	if ( !fMatch )
    	{
    	    bug("Fread_pet: no match.",0);
    	    fread_to_eol(fp);
    	}
    	
    	}
    }
}



void fread_obj( CHAR_DATA *ch, FILE *fp )
{
    OBJ_DATA *obj;
    const char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;  /* to prevent errors */
    bool make_new;    /* update object */
    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word   = feof( fp ) ? "End" : fread_word( fp );
    if (!str_cmp(word,"Vnum" ))
    {
        int vnum;
	first = FALSE;  /* fp will be in right place */
 
        vnum = fread_number( fp );
        if (  get_obj_index( vnum )  == NULL )
	{
            bug( "Fread_obj: bad vnum %d.", vnum );
	}
        else
	{
	    obj = create_object(get_obj_index(vnum),-1);
	    new_format = TRUE;
	}
	    
    }

    if (obj == NULL)  /* either not found or old style */
    {
    	obj = new_obj();
    	obj->name		= str_dup( "" );
    	obj->short_descr	= str_dup( "" );
    	obj->description	= str_dup( "" );
    }

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )
    {
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if (!str_cmp(word,"AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_obj: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_obj: unknown skill.",0);
                else
                    paf->type = sn;
 
		paf->where	= fread_number( fp );
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = obj->affected;
                obj->affected   = paf;
                fMatch          = TRUE;
                break;
            }

	    break;

	case 'B':
		KEY( "Belt",	obj->belted_vnum,	fread_number( fp ) );
		break;

	case 'C':
	    KEY( "Cond",	obj->condition,		fread_number( fp ) );
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    KEY( "Desc",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted"))
	    {
		obj->enchanted = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }

            KEY( "Exp",         obj->exp,               fread_number( fp ) );
	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtF",	obj->extra_flags,	fread_number( fp ) );
            KEY( "ExtraFlags2", obj->extra2_flags,      fread_number( fp ) );
            KEY( "ExtF2",       obj->extra2_flags,      fread_number( fp ) );  

	    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
	    {
		EXTRA_DESCR_DATA *ed;

		ed = new_extra_descr();

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

            if ( !str_cmp( word, "End" ) )
            {
                if ( !fNest || ( fVnum && obj->pIndexData == NULL ) )
                {
                    bug( "Fread_obj: incomplete object.", 0 );
                    free_obj(obj);
                    return;
                }
                else
                {
                    if ( !fVnum )
                    {
                        free_obj( obj ); 
                        obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0 );
                    }    

		    if (!new_format)
		    {
		    	obj->next	= object_list;
		    	object_list	= obj;
		    	obj->pIndexData->count++;
		    }

                    if (IS_OBJ_STAT (obj, ITEM_AUCTIONED))
                    {
                        bug("char loaded with auctioned item.", 0);
	                REMOVE_BIT (obj->extra_flags, ITEM_AUCTIONED);
                    }

		    if (!obj->pIndexData->new_format 
		    && obj->item_type == ITEM_ARMOR
		    &&  obj->value[1] == 0)
		    {
			obj->value[1] = obj->value[0];
			obj->value[2] = obj->value[0];
		    }
		    if (make_new)
		    {
			int wear;
			
			wear = obj->wear_loc;
			extract_obj(obj);

			obj = create_object(obj->pIndexData,0);
			obj->wear_loc = wear;
		    }
		    if ( iNest == 0 || rgObjNest[iNest] == NULL )
		    {   if(ch)
			    obj_to_char( obj, ch );
			else
			    obj_to_room(obj, obj->in_room );
		    }			
		    else
			obj_to_obj( obj, rgObjNest[iNest-1] );
		    return;
		}

	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Ityp",	obj->item_type,		fread_number( fp ) );
	    if(!str_cmp(word, "Inrm" ) )
	    {   obj->in_room = get_room_index(fread_number(fp) );
	        fMatch = TRUE;
		break;
 	    }
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    KEY( "Lev",		obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

   	case 'O':
	    if ( !str_cmp( word,"Oldstyle" ) )
	    {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format)
		    make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;
		    
        case 'P':
            KEY("Plev",       obj->plevel,            fread_number( fp ) );
            break; 

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		obj->short_descr,	fread_string( fp ) );
            KEY( "Size",        obj->size,              fread_number( fp ) ); 
	    KEY( "strap",	obj->strap_loc,		fread_number( fp ) );
	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    KEY( "Time",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		   obj->value[0] = obj->pIndexData->value[0];
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val" ) )
	    {
		obj->value[0] 	= fread_number( fp );
	 	obj->value[1]	= fread_number( fp );
	 	obj->value[2] 	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		obj->value[4]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

            if ( !str_cmp( word, "ValOrig" ) )
            {
                obj->valueorig[0]       = fread_number( fp );
                obj->valueorig[1]       = fread_number( fp );
                obj->valueorig[2]       = fread_number( fp );
                obj->valueorig[3]       = fread_number( fp );
                obj->valueorig[4]       = fread_number( fp );
                fMatch = TRUE;
                break;
            }                 

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		    bug( "Fread_obj: bad vnum %d.", vnum );
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
	    break;

        case 'X':
            KEY( "Xptolevel",   obj->xp_tolevel,        fread_number( fp ) );
            break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}


/* load a mount from the forgotten reaches */
void fread_mount( CHAR_DATA *ch, FILE *fp )
{
    const char *word;
    CHAR_DATA *pet;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
    	int vnum;
    	
    	vnum = fread_number(fp);
    	if (get_mob_index(vnum) == NULL)
	{
    	    bug("Fread_mount: bad vnum %d.",vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
	}
    	else
    	    pet = create_mobile(get_mob_index(vnum));
    }
    else
    {
        bug("Fread_mount: no vnum in file.",0);
        pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }
    
    for ( ; ; )
    {
    	word 	= feof(fp) ? "END" : fread_word(fp);
    	fMatch = FALSE;
    	
    	switch (UPPER(word[0]))
    	{
    	case '*':
    	    fMatch = TRUE;
    	    fread_to_eol(fp);
    	    break;
    		
    	case 'A':
            KEY( "Act",         pet->act,               fread_flag(fp));
            KEY( "AfBy",        pet->affected_by,       fread_flag(fp));
    	    KEY( "Alig",	pet->alignment,		fread_number(fp));
    	    
    	    if (!str_cmp(word,"ACs"))
    	    {
    	    	int i;
    	    	
    	    	for (i = 0; i < 4; i++)
    	    	    pet->armor[i] = fread_number(fp);
    	    	fMatch = TRUE;
    	    	break;
    	    }
    	    
    	    if (!str_cmp(word,"AffD"))
    	    {
    	    	AFFECT_DATA *paf;
    	    	int sn;
    	    	
                paf = new_affect();

                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                   paf->type = sn;

                paf->level      = fread_number(fp);
                paf->duration   = fread_number(fp);
                paf->modifier   = fread_number(fp);
                paf->location   = fread_number(fp);
                paf->bitvector  = fread_number(fp);
                paf->next       = pet->affected;
                pet->affected   = paf;
                fMatch          = TRUE;
                break;
	   }

            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;

                paf = new_affect();

                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                   paf->type = sn;

                paf->where      = fread_number(fp);
                paf->level      = fread_number(fp);
                paf->duration   = fread_number(fp);
                paf->modifier   = fread_number(fp);
                paf->location   = fread_number(fp);
                paf->bitvector  = fread_number(fp);
                paf->next       = pet->affected;
                pet->affected   = paf;
                fMatch          = TRUE;
                break;
	    }

    	    if (!str_cmp(word,"AMod"))
    	    {
    	     	int stat;
    	     	
    	     	for (stat = 0; stat < MAX_STATS; stat++)
    	     	    pet->mod_stat[stat] = fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"Attr"))
    	    {
    	         int stat;
    	         
    	         for (stat = 0; stat < MAX_STATS; stat++)
    	             pet->perm_stat[stat] = fread_number(fp);
    	         fMatch = TRUE;
    	         break;
    	    }
    	    break;
    	     
    	 case 'C':
    	     KEY( "Clan",       pet->clan,       clan_lookup(fread_string(fp)));
	     KEY( "Comm",	pet->comm,		fread_flag(fp));
             KEY( "Comm2",      pet->comm2,             fread_flag(fp));
    	     break;
    	     
    	 case 'D':
    	     KEY( "Dam",	pet->damroll,		fread_number(fp));
    	     KEY( "Desc",	pet->description,	fread_string(fp));
    	     break;
    	     
    	 case 'E':
    	     if (!str_cmp(word,"End"))
	     {
                ch->mount = pet;
                pet->mount = ch;
                /* adjust hp mana move up  -- here for speed's sake */
                percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

                if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
                &&  !IS_AFFECTED(ch,AFF_PLAGUE))
                {
                    percent = UMIN(percent,100);
                    pet->hit    += (pet->max_hit - pet->hit) * percent / 100;
                    pet->mana   += (pet->max_mana - pet->mana) * percent / 100;
                    pet->move   += (pet->max_move - pet->move)* percent / 100;
                }

    	     	return;
	     }
    	     KEY( "Exp",	pet->exp,		fread_number(fp));
    	     break;
    	     
    	 case 'G':
    	     KEY( "Gold",	pet->gold,		fread_number(fp));
    	     break;
    	     
    	 case 'H':
    	     KEY( "Hit",	pet->hitroll,		fread_number(fp));
    	     
    	     if (!str_cmp(word,"HMV"))
    	     {
    	     	pet->hit	= fread_number(fp);
    	     	pet->max_hit	= fread_number(fp);
    	     	pet->mana	= fread_number(fp);
    	     	pet->max_mana	= fread_number(fp);
    	     	pet->move	= fread_number(fp);
    	     	pet->max_move	= fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	     }
    	     break;
    	     
     	case 'L':
    	     KEY( "Levl",	pet->level,		fread_number(fp));
    	     KEY( "LnD",	pet->long_descr,	fread_string(fp));
             KEY( "LogO",       lastlogoff,             fread_number(fp));
    	     break;
    	     
    	case 'N':
    	     KEY( "Name",	pet->name,		fread_string(fp));
    	     break;
    	     
    	case 'P':
    	     KEY( "Pos",	pet->position,		fread_number(fp));
    	     break;
    	     
	case 'R':
    	    KEY( "Race",	pet->race, race_lookup(fread_string(fp)));
    	    break;
 	    
    	case 'S' :
    	    KEY( "Save",	pet->saving_throw,	fread_number(fp));
    	    KEY( "Sex",		pet->sex,		fread_number(fp));
    	    KEY( "ShD",		pet->short_descr,	fread_string(fp));
            KEY( "Silv",        pet->silver,            fread_number( fp ) );
    	    break;
    	    
    	if ( !fMatch )
    	{
    	    bug("Fread_mount: no match.",0);
    	    fread_to_eol(fp);
    	}
    	
    	}
    }
    
}

/*
 * Write an object and its contents.
 */
void fwrite_bobj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
	fwrite_bobj( ch, obj->next_content, fp, iNest );

    /*
     * Castrate storage characters.
     */
    if ( (ch->level < obj->level - 15 && obj->item_type != ITEM_CONTAINER)
    ||   obj->item_type == ITEM_KEY
    ||   (obj->item_type == ITEM_MAP && !obj->value[0]))
	return;

    fprintf( fp, "#BOX\n" );
    fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum        );
    if (!obj->pIndexData->new_format)
	fprintf( fp, "Oldstyle\n");
    if (obj->enchanted)
	fprintf( fp,"Enchanted\n");
    fprintf( fp, "Nest %d\n",	iNest	  	     );
    fprintf( fp, "Size   %d\n", obj->size                           ); 
    /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name)
    	fprintf( fp, "Name %s~\n",	obj->name		     );
    if ( obj->short_descr != obj->pIndexData->short_descr)
        fprintf( fp, "ShD  %s~\n",	obj->short_descr	     );
    if ( obj->description != obj->pIndexData->description)
        fprintf( fp, "Desc %s~\n",	obj->description	     );
    if ( obj->extra_flags != obj->pIndexData->extra_flags)
        fprintf( fp, "ExtF %d\n",	obj->extra_flags	     );
    if ( obj->extra2_flags != obj->pIndexData->extra2_flags           );
        fprintf( fp, "ExtF2 %d\n",       obj->extra2_flags            );
    if ( obj->wear_flags != obj->pIndexData->wear_flags)
        fprintf( fp, "WeaF %d\n",	obj->wear_flags		     );
    if ( obj->item_type != obj->pIndexData->item_type)
        fprintf( fp, "Ityp %d\n",	obj->item_type		     );
    if ( obj->weight != obj->pIndexData->weight)
        fprintf( fp, "Wt   %d\n",	obj->weight		     );
    if ( obj->condition != obj->pIndexData->condition)
	fprintf( fp, "Cond %d\n",	obj->condition		     );
	fprintf( fp, "strap %d\n",	obj->strap_loc		    );
	if ( obj->belted_vnum > 0 )
		fprintf( fp, "Belt %d\n", obj->belted_vnum );
    /* variable data */


    if ( obj->plevel > 0 )
        fprintf( fp, "Plevel %d\n", obj->plevel );
    if ( obj->exp > 0 )
        fprintf( fp, "Exp %d\n", obj->exp );
    if ( obj->xp_tolevel > 0 )
        fprintf( fp, "Xptolevel %d\n", obj->xp_tolevel );         
    fprintf( fp, "Wear %d\n",   obj->wear_loc                );
    if (obj->level != obj->pIndexData->level)
        fprintf( fp, "Lev  %d\n",	obj->level		     );
    if (obj->timer != 0)
        fprintf( fp, "Time %d\n",	obj->timer	     );
    fprintf( fp, "Cost %d\n",	obj->cost		     );
    if (obj->value[0] != obj->pIndexData->value[0]
    ||  obj->value[1] != obj->pIndexData->value[1]
    ||  obj->value[2] != obj->pIndexData->value[2]
    ||  obj->value[3] != obj->pIndexData->value[3]
    ||  obj->value[4] != obj->pIndexData->value[4]) 
    	fprintf( fp, "Val  %d %d %d %d %d\n",
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4]	     );
   
    if (obj->valueorig[0] != obj->pIndexData->value[0]
    ||  obj->valueorig[1] != obj->pIndexData->value[1]
    ||  obj->valueorig[2] != obj->pIndexData->value[2]
    ||  obj->valueorig[3] != obj->pIndexData->value[3]
    ||  obj->valueorig[4] != obj->pIndexData->value[4])       
     fprintf( fp, "ValOrig  %d %d %d %d %d\n",
            obj->valueorig[0], obj->valueorig[1], obj->valueorig[2],
                obj->valueorig[3], obj->valueorig[4]         );
                                                                        
    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
    case ITEM_PILL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1 '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2 '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;
        fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
            skill_table[paf->type].name,
            paf->where,
            paf->level,
            paf->duration,
            paf->modifier,
            paf->location,
            paf->bitvector
            );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExDe %s~ %s~\n",
	    ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_bobj( ch, obj->contains, fp, iNest + 1 );

    return;
}

void fread_bobj( CHAR_DATA *ch, FILE *fp )
{
    OBJ_DATA *obj;
    const char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;  /* to prevent errors */
    bool make_new;    /* update object */
    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word   = feof( fp ) ? "End" : fread_word( fp );
    if (!str_cmp(word,"Vnum" ))
    {
        int vnum;
	first = FALSE;  /* fp will be in right place */
 
        vnum = fread_number( fp );
        if (  get_obj_index( vnum )  == NULL )
	{
            bug( "Fread_bobj: bad vnum %d.", vnum );
	}
        else
	{
	    obj = create_object(get_obj_index(vnum),-1);
	    new_format = TRUE;
	}
	    
    }

    if (obj == NULL)  /* either not found or old style */
    {
    	obj = new_obj();
    	obj->name		= str_dup( "" );
    	obj->short_descr	= str_dup( "" );
    	obj->description	= str_dup( "" );
    }

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )
    {
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if (!str_cmp(word,"AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_bobj: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_bobj: unknown skill.",0);
                else
                    paf->type = sn;
 
		paf->where	= fread_number( fp );
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = obj->affected;
                obj->affected   = paf;
                fMatch          = TRUE;
                break;
            }
	    break;

	case 'B':
		KEY( "Belt",	obj->belted_vnum,	fread_number( fp ) );
		break;

	case 'C':
	    KEY( "Cond",	obj->condition,		fread_number( fp ) );
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    KEY( "Desc",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted"))
	    {
		obj->enchanted = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }

            KEY( "Exp",         obj->exp,               fread_number( fp ) );
	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtF",	obj->extra_flags,	fread_number( fp ) );
            KEY( "ExtraFlags2", obj->extra2_flags,      fread_number( fp ) );
            KEY( "ExtF2",       obj->extra2_flags,      fread_number( fp ) );  

	    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
	    {
		EXTRA_DESCR_DATA *ed;

		ed = new_extra_descr();

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

            if ( !str_cmp( word, "End" ) )
            {
                if ( !fNest || ( fVnum && obj->pIndexData == NULL ) )
                {
                    bug( "Fread_bobj: incomplete object.", 0 );
                    free_obj(obj);
                    return;
                }
                else
                {
                    if ( !fVnum )
                    {
                        free_obj( obj ); 
                        obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0 );
                    }    

		    if (!new_format)
		    {
		    	obj->next	= object_list;
		    	object_list	= obj;
		    	obj->pIndexData->count++;
		    }

                    if (IS_OBJ_STAT (obj, ITEM_AUCTIONED))
                    {
                        bug("char loaded with auctioned item.", 0);
	                REMOVE_BIT (obj->extra_flags, ITEM_AUCTIONED);
                    }

		    if (!obj->pIndexData->new_format 
		    && obj->item_type == ITEM_ARMOR
		    &&  obj->value[1] == 0)
		    {
			obj->value[1] = obj->value[0];
			obj->value[2] = obj->value[0];
		    }
		    if (make_new)
		    {
			int wear;
			
			wear = obj->wear_loc;
			extract_obj(obj);

			obj = create_object(obj->pIndexData,0);
			obj->wear_loc = wear;
		    }
		    if ( iNest == 0 || rgObjNest[iNest] == NULL )
			obj_to_char( obj, ch );
		    else
			obj_to_obj( obj, rgObjNest[iNest-1] );
		    return;
		}

	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Ityp",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    KEY( "Lev",		obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_bobj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

   	case 'O':
	    if ( !str_cmp( word,"Oldstyle" ) )
	    {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format)
		    make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;

        case 'P':
            KEY("Plevel",       obj->plevel,            fread_number( fp ) );
            break;

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		obj->short_descr,	fread_string( fp ) );
            KEY( "Size",        obj->size,              fread_number( fp ) ); 
	    KEY( "strap",	obj->strap_loc,		fread_number( fp ) );
	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_bobj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_bobj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    KEY( "Time",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		   obj->value[0] = obj->pIndexData->value[0];
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val" ) )
	    {
		obj->value[0] 	= fread_number( fp );
	 	obj->value[1]	= fread_number( fp );
	 	obj->value[2] 	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		obj->value[4]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

            if ( !str_cmp( word, "ValOrig" ) )
            {
                obj->valueorig[0]       = fread_number( fp );
                obj->valueorig[1]       = fread_number( fp );
                obj->valueorig[2]       = fread_number( fp );
                obj->valueorig[3]       = fread_number( fp );
                obj->valueorig[4]       = fread_number( fp );
                fMatch = TRUE;
                break;
            }                 

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		    bug( "Fread_bobj: bad vnum %d.", vnum );
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
	    break;

        case 'X':
            KEY( "Xptolevel",   obj->xp_tolevel,        fread_number( fp ) );
            break;                     

	}

	if ( !fMatch )
	{
	    bug( "Fread_bobj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}

