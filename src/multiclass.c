/****************************************************************************
 * Multiclass Code                                                          *
 * Markanth : dlmud@dlmud.com                                               *
 * Devil's Lament : dlmud.com port 3778                                     *
 * Web Page : http://www.dlmud.com                                          *
 *                                                                          *
 * A nice and simple remort code if you like multiclassing.                 *
 * This code require you have a working knowledge of how rom skills work.   *
 *                                                                          *
 * All I ask in return is that you give me credit on your mud somewhere     *
 * or email me if you use it.                                               *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "tables.h"
#include "const.h"

void do_recl(CHAR_DATA * ch, char *argument)
{
    send_to_char("If you want to RECLASS, you must spell it out.\n\r", ch);
    return;
}

void do_reclass(CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *mob;
    AFFECT_DATA *af,
    *af_next;
	char buf[MAX_INPUT_LENGTH];
    int x, i,
     sn;

    if (IS_NPC(ch) || (d = ch->desc) == NULL)
	return;

    /*
     * check for priest or special mob
     */
    for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
    {
	if (IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER))  /* setup to do at healer's for now */
	    break;
    }

    if (mob == NULL)
    {
	send_to_char("You can't do that here.\n\r", ch);
	return;
    }

    if (ch->level < LEVEL_HERO )
    {
	send_to_char("You must be a HERO to reclass.\n\r", ch);
	return;
    }

    if(number_classes(ch) == MAX_REMORT)
    {
        send_to_char("You can't reclass any more!\n\r", ch);
        return;
    }

    for (x = 0; x < MAX_WEAR; x++)
    {
	if (get_eq_char(ch, x) != NULL)
	{
	    send_to_char("Remove all of your eq first. (heal uncurse for cursed items)\n\r", ch);
	    return;
	}
    }

    if (ch->pcdata->confirm_remort)
    {
	if (argument[0] != '\0')
	{
	    send_to_char("Reclass status removed.\n\r", ch);
	    ch->pcdata->confirm_remort = FALSE;
	    return;
	} else
	{
	    sprintf(buf, "You have chosen to reclass.  You will be allowed to pick a new %s\n\r", !ch->pcdata->stay_race ? "RACE" : "CLASS");
	    send_to_char(buf, ch);
	    send_to_char("selection section of character creation, and will be allowed recreate\n\r", ch);
	    send_to_char("your character with an additional klass and bonuses.\n\r\n\r", ch);
	    send_to_char("In the unlikely event that you are disconnected or the MUD\n\r", ch);
	    send_to_char("crashes while you are creating your character, log back on and write a\n\r", ch);
	    send_to_char("note to 'immortal' who will retrieve your backup.\n\r", ch);
	    wiznet("$N has reclassed.", ch, NULL, 0, 0, 0);
            info( ch, 0, "{G[INFO]: {R%s has just reclassed!{x\n\r", ch->name );
	    for (af = ch->affected; af != NULL; af = af_next)
	    {
		af_next = af->next;
		affect_remove(ch, af);
	    }
        SET_BIT ( ch->act, PLR_REMORT );
        SET_BIT ( ch->comm, COMM_QUIET );
        char_from_room ( ch );
        char_to_room ( ch, get_room_index ( ROOM_VNUM_LIMBO ) );
        ch->level = 1;
        ch->exp = 0;
        ch->pcdata->points = 0;
        ch->max_hit = 100 * ( number_classes ( ch ) + 1 );
        ch->max_mana = 100 * ( number_classes ( ch ) + 1 );
        ch->max_move = 100 * ( number_classes ( ch ) + 1 );
        ch->hit = ch->max_hit;
        ch->mana = ch->max_move;
        ch->move = ch->max_mana;
        ch->pcdata->perm_hit = ch->max_hit;
        ch->pcdata->perm_mana = ch->max_mana;
        ch->pcdata->perm_move = ch->max_move;
        ch->wimpy = ch->max_hit / 5;
        ch->train = 5 * ( number_classes ( ch ) + 1 );
        ch->practice = 7 * ( number_classes ( ch ) + 1 );
        ch->exp = exp_per_level ( ch, ch->pcdata->points );
        ch->levelflux = ch->levelflux;
        reset_char ( ch );
	    /* nuke any high level pets */
	    if (ch->pet != NULL)
	    {
		nuke_pets(ch);
		ch->pet = NULL;
	    }
            /* Race skills are lost.
               100% skills are kept at 50%.
               All other skills are reset back to 1%. */
	    for (sn = 0; sn < MAX_SKILL; sn++)
	    {
            if (ch->pcdata->learned[sn] > 0)
            {
                if (is_race_skill(ch, sn) && !ch->pcdata->stay_race)
                    ch->pcdata->learned[sn] = 0;
                else if (ch->pcdata->learned[sn] == 100)
                    ch->pcdata->learned[sn] = 50;
                else
                    ch->pcdata->learned[sn] = 1;
            }
	    }
            /* send char to race selection, customize this as you see fit */
	    send_to_char("\n\rNow beginning the reclassing process.\n\r\n\r", ch);
		for(i = 0; i < MAX_CBOOL ; i++)
		    ch->pcdata->creation[i] = TRUE;
		ch->pcdata->creation[CHOSE_CLASS] = FALSE;
	send_to_char("This is the Creation Menu:\n\r",ch );
	printf_to_char(ch,"{WA{D){W Race Selection      - {w%s\n\r", ch->pcdata->creation[CHOSE_RACE] == TRUE ? pc_race_table[ch->race].name : "Not Chosen" );
	printf_to_char(ch,"{WB{D){W Class Selection     - {w%s\n\r", ch->pcdata->creation[CHOSE_CLASS] == TRUE ? class_long(ch) : "Not Chosen" );
	printf_to_char(ch,"{WC{D){W Gender Selection    - {w%s\n\r", ch->pcdata->creation[CHOSE_GENDER] == TRUE ? sex_table[ch->sex].name : "Not Chosen" );
	printf_to_char(ch,"{WF{D){W Short Description   - {w%s\n\r", ch->pcdata->creation[CHOSE_SHORT] == TRUE ? ch->short_descr : "Not Chosen" );
	printf_to_char(ch,"{WG{D){W Attribute Selection - {w%s{x\n\r", ch->pcdata->creation[CHOSE_ATTRIBUTES] == TRUE ? "Set" : "Not Chosen" );
	send_to_char("{WH{D){W Done",ch);
	send_to_char("\n\rPlease make a selection based off the corresponding letter.\n\rEnter selection-> ",ch);
	d->connected = CON_CREATION_MENU;

	    return;
	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type reclass.  No argument.\n\r", ch);
	return;
    }
    send_to_char("Typing reclass with an argument will undo reclass status.\n\r", ch);
    send_to_char("Reclassing is not reversable, make sure you read help RECLASS\n\r", ch);
    send_to_char("and have an idea of what klass you want to reclass into.\n\r", ch);
    send_to_char("Type reclass again to confirm this command.\n\r", ch);
    ch->pcdata->confirm_remort = TRUE;
	if (!ch->pcdata->stay_race)
    wiznet("$N is contemplating reclassing.", ch, NULL, 0, 0, get_trust(ch));
}
