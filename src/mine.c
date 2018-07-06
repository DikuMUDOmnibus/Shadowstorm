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

/******************************************************************************
 *
 * Code all wrote, by Davion of ShadowStorm. (telnet://beyond.kyndig.com:5500 )
 * Z is depth, X is east to west, Y is north to south.
 *
 ******************************************************************************/
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

#define MIN_ALL 11

void		free_cord		args( (CHAR_DATA *ch) );
void		mine			args( (CHAR_DATA *ch, int mineral, ROOM_INDEX_DATA *pRoom ) );
bool		found_mineral	args( (CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, int mineral ) );
OBJ_DATA *	create_ore		args( (ROOM_INDEX_DATA *pRoom, int mineral ) );
void		shaft_to_mine	args( (SHAFT_DATA *pShaft, MINE_DATA *pMine, int x, int y, int z ) );
void    show_list_to_char       args( ( OBJ_DATA *list, CHAR_DATA *ch,
                                    bool fShort, bool fShowNothing ) );
void    show_char_to_char       args( ( CHAR_DATA *list, CHAR_DATA *ch ) ); 


int move_dir[6][3] = { {0, -1, 0 }, { 1, 0, 0 }, { 0, 1, 0 }, { -1, 0, 0 }, { 0, 0, -1 }, { 0, 0, 1 } };
int enter_cord[6][3] = { { 12, MAX_MINE_Y-1, 0 }, { 0, 12, 0 }, { 12, 0, 0 }, { MAX_MINE_X-1, 12, 0 }, { 0, 0, 0 }, { 12, 12, 0 } };

extern char *const dir_name[];

void do_mine( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *pRoom;
	int mineral;

	pRoom = ch->in_room;

	if(argument[0] == '\0' )
	{   mine(ch, MIN_ALL, pRoom );
		return;
	}

	if( ( mineral = mineral_lookup(argument) ) == -1 )
	{
		send_to_char("What type of mineral is that!?\n\r",ch);
		return;
	}

	mine(ch, mineral, pRoom );
	return;
}

void mine( CHAR_DATA *ch, int mineral, ROOM_INDEX_DATA *pRoom )
{
	if(!IS_NPC(ch) && !IN_MINE(ch) )
	{	send_to_char("You must be in a mine!\n\r",ch);
		return;
	}
	switch (mineral)
	{
		case MIN_ALL:
			printf_to_char(ch, "You begin mining for all minerals.\n\r");
			WAIT_STATE(ch, 16*PULSE_PER_SECOND );
			found_mineral(ch, pRoom, MIN_ALL );
			break;
		default:
			printf_to_char(ch, "You begin searching for %s.\n\r", mineral_table[mineral].name );
			WAIT_STATE(ch, 16*PULSE_PER_SECOND );
			found_mineral(ch, pRoom, mineral );
			break;
	}
}

bool found_mineral(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, int mineral )
{
	int i;
	char buf[MSL];
	OBJ_DATA *ore;

	if(mineral == MIN_ALL )
	{
		wait_act(2, "You bang your tools vigorously against the ground, trying to find some minerals.", ch, NULL, NULL, TO_CHAR );
		wait_act(6, "You wipe the sweat from your brow, as the work becomes strenuous.",ch, NULL, NULL, TO_CHAR );
		for( i = 0; mineral_table[i].name != NULL; i++ )
		{
			if(pRoom->mineral[i] <= 0 )
				continue;
			if(number_range(0, 100 ) < pRoom->mineral[i] )
			{	sprintf(buf, "You found some %s.", mineral_table[i].name );
				wait_act(8, buf, ch, NULL, NULL, TO_CHAR );
				ore = create_ore(pRoom, i );
				obj_to_char(ore, ch);
				return TRUE;
			}
		}
		wait_act(8, "Your search for minerals has turned up fruitless.", ch, NULL, NULL, TO_CHAR );
		return FALSE;
	}
	if(pRoom->mineral[mineral] <= 0 )
	{	wait_act(3, "You bang your tools vigorously against the ground, trying to find some minerals.", ch, NULL, NULL, TO_ROOM );
		wait_act(5, "You didn't find a hint of the mineral", ch, NULL, NULL, TO_CHAR );
		ch->wait = 0;
		return FALSE;
	}

	wait_act(3, "You bang your tools vigorously against the ground, trying to find the mineral.", ch, NULL, NULL, TO_CHAR );
	wait_act(6, "You wipe the sweat from your brow, as the work becomes strenuous.",ch, NULL, NULL, TO_CHAR );
	if(number_range(0, 100 ) < pRoom->mineral[mineral] )
	{	sprintf(buf, "You found some %s", mineral_table[mineral].name );
		wait_act(8, buf, ch, NULL, NULL, TO_CHAR );
		return TRUE;
	}
	wait_act(8, "Your search for minerals has turned up fruitless.", ch, NULL, NULL, TO_CHAR );
	return FALSE;
}


OBJ_DATA *create_ore(ROOM_INDEX_DATA *pRoom, int mineral )
{
	OBJ_DATA *pObj;
	char buf[MSL];
	int weight;
	char size[MSL];

	pObj = create_object(get_obj_index(OBJ_VNUM_ORE), 0 );
	free_string(pObj->name);
	free_string(pObj->short_descr);
	free_string(pObj->description);
	sprintf(buf, "ore %s", mineral_table[mineral].name );
	pObj->name = str_dup(buf );
	
	weight = number_range(0, pRoom->mineral[mineral]);
	if(weight > 15 )
		weight /= 2;
	if(weight <= 2) sprintf(size, "tiny");
	else if(weight <= 5 ) sprintf(size, "small" );
	else if(weight <= 15 ) sprintf(size, "nice" );
	else if(weight <= 30 ) sprintf(size, "descent" );
	else if(weight <= 50 ) sprintf(size, "big" );
	else if(weight <= 80  ) sprintf(size, "large" );
	else sprintf(size, "huge" );
	
	sprintf(buf, "a %s piece of %s ore", size, mineral_table[mineral].name );
	pObj->short_descr = str_dup(buf);
	pObj->description = str_dup(buf);
	pObj->weight = weight;
	pObj->value[0] = mineral;
	return pObj;

}

void move_char_mine(CHAR_DATA *ch, int door )
{
	ROOM_INDEX_DATA *pRoom;
	SHAFT_DATA *pShaft;
	char buf[MSL];
	pRoom = ch->in_room;

	if(IS_NPC(ch) )
	{	send_to_char("A mine is no place for a mob.\n\r",ch);
		return;
	}

	if( (pShaft = ch->pcdata->in_shaft ) == NULL )
	{	logf2("BUG: %s is moving around without a shaft!",ch->name );
		return;
	}
	
	if( !pShaft->exit[door] || !pShaft->exit[door]->to_shaft)
	{	send_to_char("Alas, you cannot go that way.\n\r",ch);
		return;
	}
	if(ch->pcdata->cordinate[CORD_X] == enter_cord[door][CORD_X] &&
		ch->pcdata->cordinate[CORD_Y] == enter_cord[door][CORD_Y] &&
		ch->pcdata->cordinate[CORD_Z] == 0 &&
		door == DIR_UP )
	{   char_from_mine(ch);
		return;
	}
	sprintf(buf, "$n leaves %s.", dir_name[door] );
	act(buf, ch, NULL, NULL, TO_ROOM);
	ch->pcdata->in_shaft = ch->pcdata->in_shaft->exit[door]->to_shaft;
	ch->pcdata->cordinate[CORD_X] += move_dir[door][CORD_X];
	ch->pcdata->cordinate[CORD_Y] += move_dir[door][CORD_Y];
	ch->pcdata->cordinate[CORD_Z] += move_dir[door][CORD_Z];
	sprintf(buf, "$n arrives from %s.", rev_move[door] );
	act(buf, ch, NULL, NULL, TO_ROOM );
	do_function( ch, &do_look, "mine" );
}

void put_char_mine( CHAR_DATA *ch, EXIT_DATA *pexit )
{	
	char buf[MSL];
	if(IS_NPC(ch) )
	{	send_to_char("A mine is no place for a mob.\n\r",ch);
		return;
	}

	if(!pexit->mine->shaft_first)
	{	send_to_char("Alas, you cannot go that way.\n\r",ch );
		return;
	}

	if(MOUNTED(ch)  )
	{   send_to_char("Dismount -then- enter the mine!\n\r",ch);
	    return;
	}

	if(ch->master)
	{   send_to_char("And hide from your beloved master?\n\r",ch);
		return;
	}
	sprintf(buf, "$n enters the mine %sward", dir_name[pexit->orig_door] );
	act(buf, ch, NULL, NULL, TO_ROOM );
	ch->pcdata->in_shaft = pexit->to_shaft;
	ch->pcdata->in_mine = pexit->mine;
	ch->pcdata->cordinate[CORD_X] = enter_cord[pexit->orig_door][CORD_X];
	ch->pcdata->cordinate[CORD_Y] = enter_cord[pexit->orig_door][CORD_Y];
	ch->pcdata->cordinate[CORD_Z] = 0;
	sprintf(buf, "$n enters the mine from %s", rev_move[pexit->orig_door]);
	act(buf, ch, NULL, NULL, TO_ROOM );
	do_function( ch, &do_look, "mine" );
}

void char_from_mine( CHAR_DATA *ch )
{	
	exit_mine(ch);
	do_function(ch, &do_look, "auto");
}

void exit_mine( CHAR_DATA *ch )
{
	ch->pcdata->in_shaft = NULL;
	free_cord(ch);
	ch->in_room = ch->pcdata->in_mine->entrance;
	ch->pcdata->in_mine = NULL;
}

void free_cord(CHAR_DATA *ch)
{
	int i;
	if(!ch)
		return;
	if(IS_NPC(ch) )
		return;
	for( i= 0 ; i < 3 ; i ++ )
		ch->pcdata->cordinate[i] = 0;
	return;
}

int dir_lookup (const char *name)
{
    int door;

    for (door = 0; door < 6 ; door++ )
    {
        if (LOWER(name[0]) == LOWER(dir_name[door][0])
        &&  !str_prefix(name,dir_name[door]))
            return door;
    }

    return -1;
}

void do_build( CHAR_DATA *ch, char *argument )
{	char arg[MSL], arg2[MSL];
	int door, new_x, new_y, new_z;
	ROOM_INDEX_DATA *pRoom;
	EXIT_DATA *pExit;
	MINE_DATA *pMine;
	SHAFT_DATA *pShaft, *nShaft;
	bool found = FALSE;

	arg[0] = '\0';
	arg2[0] = arg[0];

	if(IS_NPC(ch) )
	{	send_to_char("I dun think so!\n\r",ch);
		return;
	}

	if( ( pRoom = ch->in_room ) == NULL )
	{	send_to_char("You arn't in a room! Run!\n\r",ch);
		return;
	}
	
	if(!IS_SET(pRoom->room_flags, ROOM_MINE ) )
	{	send_to_char("You cannot build a mine here.\n\r",ch);
		return;
	}

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);

	if(arg[0] == '\0' || arg2[0] == '\0' )
	{	send_to_char("Syntax: build <shaft|entrance|link> <direction>\n\r",ch);
		return;
	}

	if( ( door = dir_lookup(arg2) ) == -1 )
	{	send_to_char("What type of direction is that?\n\r",ch);
		return;
	}

	if(!str_cmp(arg, "entrance" ) )
	{	if(IN_MINE(ch) )
		{	send_to_char("You can't build an entrance to a mine that your in!\n\r",ch);
			return;
		}
		if(door == DIR_UP )
		{	send_to_char("You can't mine upwards!\n\r",ch);
			return;
		}
		pMine = ch->pcdata->in_mine;
		
		if( (pExit = pRoom->exit[door] ) != NULL )
		{	send_to_char("There is already something in that direction.\n\r",ch);
			return;
		}
		new_x									= enter_cord[door][CORD_X]; 
		new_y									= enter_cord[door][CORD_Y];
		new_z									= 0;
		
		send_to_char("Mine created\n\r",ch);

		pExit									= new_exit();
		pMine									= new_mine();
		pShaft									= new_shaft();




		pExit->orig_door						= door;
		
	
		pShaft->desc							= str_dup("This is the entrance to a mine.\n\r");
		pShaft->exit[rev_dir[door]]				= new_exit();
		pShaft->exit[rev_dir[door]]->u1.to_room = pRoom;
		pShaft->exit[rev_dir[door]]->orig_door	= rev_dir[door];
		pShaft->cord[CORD_X]					= enter_cord[door][CORD_X];
		pShaft->cord[CORD_Y]					= enter_cord[door][CORD_Y];
		pShaft->cord[CORD_Z]					= 0;
		pMine->shaft_first						= pShaft;
		pMine->entrance							= pRoom;
		pExit->to_shaft							= pShaft;
		pExit->mine								= pMine;
		pRoom->exit[door]						= pExit;
		shaft_to_mine(pShaft, pMine, new_x, new_y, new_z );
		
		return;
	}
	if(!str_cmp(arg, "shaft" ) )
	{	if(!IN_MINE(ch) )
		{	send_to_char("If you want to build a new mine, try building an entrance\n\r",ch);
			return;
		}
		pMine = ch->pcdata->in_mine;
		if( ( pShaft = ch->pcdata->in_shaft ) == NULL )
		{	send_to_char("You arn't in a shaft! Tell an Imm.\n\r",ch);
			return;
		}

		new_x = ch->pcdata->cordinate[CORD_X] + move_dir[door][CORD_X];
		new_y = ch->pcdata->cordinate[CORD_Y] + move_dir[door][CORD_Y];
		new_z = ch->pcdata->cordinate[CORD_Z] + move_dir[door][CORD_Z];
		if(new_x >= MAX_MINE_X || new_y >= MAX_MINE_Y || new_z >= MAX_MINE_Z ||
		   new_x < 0 || new_y < 0 || new_z < 0 )
		{	send_to_char("You can't build anymore in that direction!\n\r",ch);
			return;
		}
		if( pMine->map[new_x][new_y][new_z] != 0 )
		{	send_to_char("There is already something in that direction, if you wish to link it, be my guest.\n\r",ch);
			return;
		}

		send_to_char("Shaft created\n\r",ch);
		pExit				= new_exit();
		pExit->orig_door	= door;
		nShaft				= new_shaft();

		nShaft->exit[rev_dir[door]]				= new_exit();
		nShaft->exit[rev_dir[door]]->to_shaft	= pShaft;
		nShaft->exit[rev_dir[door]]->orig_door	= rev_dir[door];
		nShaft->desc = str_dup("This is a shaft in the mine.\n\r" );
		
		nShaft->cord[CORD_X] = new_x;
		nShaft->cord[CORD_Y] = new_y;
		nShaft->cord[CORD_Z] = new_z;
		
		pExit->to_shaft		= nShaft;
		pShaft->exit[door]	= pExit;

		shaft_to_mine(nShaft, pMine, new_x, new_y, new_z );
		return;
	}
	if(!str_cmp(arg, "link" )  )
	{	if(!IN_MINE(ch) )
		{	send_to_char("You gotta be in a mine!\n\r",ch);
			return;
		}

		if( (pShaft = ch->pcdata->in_shaft ) == NULL )
		{	send_to_char("You arn't in a shaft! Tell an Imm.\n\r",ch);
			return;
		}
		pMine = ch->pcdata->in_mine;
		new_x = ch->pcdata->cordinate[CORD_X] + move_dir[door][CORD_X];
		new_y = ch->pcdata->cordinate[CORD_Y] + move_dir[door][CORD_Y];
		new_z = ch->pcdata->cordinate[CORD_Z] + move_dir[door][CORD_Z];
		if(new_x >= MAX_MINE_X || new_y >= MAX_MINE_Y || new_z >= MAX_MINE_Z ||
		   new_x < 0 || new_y < 0 || new_z < 0 )
		{	send_to_char("You can't build anymore in that direction!\n\r",ch);
			return;
		}

		if(pMine->map[new_x][new_y][new_z] == 0 )
		{	send_to_char("You can't link to something that doesn't exist!\n\r",ch);
			return;
		}
		if(pShaft->exit[door] && (pShaft->exit[door]->to_shaft || pShaft->exit[door]->u1.to_room ) )
		{	send_to_char("There is already a link in that direction.\n\r",ch);
			return;
		}
		for( nShaft = pMine->shaft_first ; nShaft ; nShaft = nShaft->next )
			if(new_x == nShaft->cord[CORD_X] && new_y == nShaft->cord[CORD_Y] && new_z == nShaft->cord[CORD_Z] )
			{	found = TRUE;
				break;
			}

		if(!found)
		{	send_to_char("It seems there isn't a room where you say there is, but there isn't. Sooo IMMMY!!!!\n\r",ch);
			return;
		}
		nShaft->exit[rev_dir[door]] = new_exit();
		nShaft->exit[rev_dir[door]]->to_shaft = pShaft;
		nShaft->exit[rev_dir[door]]->orig_door = rev_dir[door];		
		pExit = new_exit();
		pExit->orig_door = door;
		pExit->to_shaft = nShaft;
		pShaft->exit[door] = pExit;
		send_to_char("Link established.\n\r",ch);
		return;
	}
	return;
}


MINE_DATA *mine_free;

MINE_DATA *new_mine(void)
{
    static MINE_DATA mine_zero;
    MINE_DATA *mine;
	int i, x, y, z;

    if (mine_free == NULL)
        mine = (MINE_DATA *)alloc_perm(sizeof(*mine));
    else
    {
        mine = mine_free;
        mine_free = mine_free->next;
    }

    *mine = mine_zero;
    VALIDATE(mine);
	mine->name = str_dup("New Mine");
	mine->shaft_first = NULL;
	for(i = 0; i < MAX_MINERALS ; i++ )
		mine->mineral[i] = 0;
	for(x = 0; x < MAX_MINE_X; x++ )
		for(y = 0; y < MAX_MINE_Y; y++ )
			for(z = 0; z < MAX_MINE_Z; z++ )
				mine->map[x][y][z] = 0;

    return mine;
}

void shaft_to_mine(SHAFT_DATA *pShaft, MINE_DATA *pMine, int x, int y, int z )
{	pMine->map[x][y][z] = 1;
	if(pMine->shaft_last )
		pMine->shaft_last->next = pShaft;
	pMine->shaft_last = pShaft;
	
	if(!pMine->shaft_first )
		pMine->shaft_first = pShaft;
	return;
}

SHAFT_DATA *get_shaft_cord(MINE_DATA *pMine, int new_x, int new_y, int new_z )
{
	SHAFT_DATA *nShaft;

	for( nShaft = pMine->shaft_first ; nShaft ; nShaft = nShaft->next )
		if(new_x == nShaft->cord[CORD_X] && new_y == nShaft->cord[CORD_Y] && new_z == nShaft->cord[CORD_Z] )
			return nShaft;
			
	return NULL;
}
void free_mine(MINE_DATA *mine)
{
    if (!IS_VALID(mine))
        return;

    INVALIDATE(mine);
    mine->next = mine_free;
    mine_free = mine;
}

SHAFT_DATA *shaft_free;

SHAFT_DATA *new_shaft(void)
{
    static SHAFT_DATA shaft_zero;
    SHAFT_DATA *shaft;
	int i;

    if (shaft_free == NULL)
        shaft = (SHAFT_DATA *)alloc_perm(sizeof(*shaft));
    else
    {
        shaft = shaft_free;
        shaft_free = shaft_free->next;
    }

    *shaft = shaft_zero;
    VALIDATE(shaft);
	shaft->parts = 0;
	free_string(shaft->desc);
	for( i = 0; i < MAX_MINERALS; i++ )
		shaft->mineral[i] = 0;
	for(i = 0; i < 3; i++ )
		shaft->cord[i] = -1;
    return shaft;
}

void free_shaft(SHAFT_DATA *shaft)
{
    if (!IS_VALID(shaft))
        return;

    INVALIDATE(shaft);
    shaft->next = shaft_free;
    shaft_free = shaft;
}

void look_mine(CHAR_DATA *ch )
{
	SHAFT_DATA *pShaft = ch->pcdata->in_shaft;
	int door, i=0, x, y, x_map = 8, y_map = 6,sx,sy,linenum = 1;
	char buf[MSL*2], mine_map[y_map+1][MSL];
	bool found = FALSE;

	buf[0] = '\0';
	for(x = 0; x < y_map ;x++ )
		mine_map[x][0] = '\0';

	if(!pShaft)
	{	send_to_char("You arn't in a shaft.\n\r",ch);
		return;
	}
	sprintf(buf, "%s", pShaft->desc );
	sx = ch->pcdata->cordinate[CORD_X] - (x_map/2);
	sy = ch->pcdata->cordinate[CORD_Y] - (y_map/2);
	if(sx >= MAX_MINE_X ) sx = MAX_MINE_X;
	if(sx < 0		    ) sx = 0;
	if(sy >= MAX_MINE_Y ) sy = MAX_MINE_Y;
	if(sy < 0		    ) sy = 0;
	for(y = sy; y <= sy+y_map; y++, i++ )
	{	for(x = sx; x <= sx+x_map ; x++ )
		{   
			if(ch->pcdata->in_mine->map[x][y][ch->pcdata->cordinate[CORD_Z]] == 0 )
			{   strcat(mine_map[i], " " );
				continue;
			}
			if(x == ch->pcdata->cordinate[CORD_X] && y == ch->pcdata->cordinate[CORD_Y] )
			{	strcat(mine_map[i], "{RX{x" );
				continue;
			}
			strcat(mine_map[i], "*" );
		}
	}

	send_to_char( "{r+{D-----------{r+{c   Within a Mine{x\n\r",ch);
	printf_to_char(ch, "{D|{x %s {D%-4s{x   ", mine_map[0], "|" );

	for(i = 0; ;i++)
	{	if(buf[i] == '\0' )
			break;
		if(buf[i] == '\r' )
		{	printf_to_char(ch, "%c", buf[i]);
			if(linenum == y_map )
				send_to_char("\n\r{r+{D-----------{r+{x",ch);
			if(linenum >= y_map )
				continue;
			printf_to_char(ch, "{D|{x %s {D%-4s{x", mine_map[linenum], "|" );
			linenum++;	
			continue;
		}
		printf_to_char(ch, "%c", buf[i]);
	}

	if(linenum < y_map )
	{	for(i = linenum ; i < y_map ; i++ )
			printf_to_char(ch, "\n\r{D|{x %s {D|{x", mine_map[i]);
		send_to_char("\n\r{r+{D-----------{r+{x",ch);
	}
	sprintf(buf, "\n\r{r[{DExits:{x");

	for(door = 0; door < 6 ; door++ )
	{	if(pShaft->exit[door] && 
		(pShaft->exit[door]->u1.to_room || 
		 pShaft->exit[door]->to_shaft ) )
		{	found = TRUE;
			strcat(buf, " " );
			if(pShaft->exit[door]->u1.to_room )
				strcat(buf, "{ROutside{w-{x");
			strcat(buf, dir_name[door] );
		}
	}

	if(!found)
		strcat(buf, "none");
	strcat(buf, "{r]{x\n\r" );
	send_to_char(buf, ch);
	show_list_to_char(ch->in_room->contents, ch, FALSE, FALSE );
	show_char_to_char(ch->in_room->people, ch );
	return;
}

void fwrite_mine(EXIT_DATA *pExit, FILE *fp )
{
	SHAFT_DATA *pShaft;
	int i;
	MINE_DATA *pMine = pExit->mine;
	EXIT_DATA *exit;

	fprintf(fp, "Mine\n" );
	fprintf(fp, "Name %s~\n", pMine->name );
	fprintf(fp, "Door %d\n", pExit->orig_door );


	for(pShaft = pMine->shaft_first ; pShaft ; pShaft = pShaft->next )
	{	fprintf(fp, "Shaft\n" );
		fprintf(fp, "Cord %d %d %d\n", pShaft->cord[CORD_X], pShaft->cord[CORD_Y], pShaft->cord[CORD_Z]);
		fprintf(fp, "Desc %s~\n",pShaft->desc );
		fprintf ( fp, "X " );

		for( i = 0; i < MAX_MINERALS ; i++ )
			fprintf(fp, "%d ", pMine->entrance->mineral[i] );
		
		fprintf(fp, "\n" );
		fprintf(fp, "Flags %s~\n", print_flags(pShaft->parts ) );
		
		for( i = 0; i < 6; i++ )
		{	if( !( exit = pShaft->exit[i] ) || (!exit->u1.to_room && !exit->to_shaft) )
				continue;
			
			fprintf(fp, "Exit\n" );
			fprintf(fp, "%d\n", exit->orig_door );			
			if(exit->to_shaft)
				fprintf(fp, "ToCord %d %d %d\n", exit->to_shaft->cord[CORD_X], exit->to_shaft->cord[CORD_Y], exit->to_shaft->cord[CORD_Z] );
			else				
				fprintf(fp, "ToVnum %d\n", exit->u1.vnum );

		}
		fprintf(fp, "ENDSHAFT\n");
	}
	fprintf(fp, "End\n\n" );
}

void fread_mine( ROOM_INDEX_DATA *pRoom, FILE *fp )
{
	const char *word;
	MINE_DATA *pMine;
	SHAFT_DATA *pShaft;
	EXIT_DATA *pExit;
	int i, door, door_to;
	for( ;; )
	{
		word = feof (fp) ? "End" : fread_word (fp);
		if(word[0] == '$' )
			return;
		switch (UPPER(word[0] ))
		{
			case 'C':
				if(!str_cmp(word, "Cord" ) )
				{	if(!pShaft )
					{	logf2("BUG: fread_mine- Reading cordinate without a shaft.");
						return;
					}
					pShaft->cord[CORD_X] = fread_number(fp);
					pShaft->cord[CORD_Y] = fread_number(fp);
					pShaft->cord[CORD_Z] = fread_number(fp);
				}
				break;
			case 'D': 
				if(!str_cmp(word, "Desc" ) )
				{	pShaft->desc = fread_string(fp );
					break;
				}
				if(!str_cmp(word, "Door" ) )
				{	door_to = fread_number(fp);
					break;
				}
				break;
			case 'F': pShaft->parts = fread_flag(fp );
					break;
			case 'E':
				if(!str_cmp(word, "Exit" ) )
				{	pExit = new_exit();
					door = fread_number(fp );

					pExit->mine = pMine;
					pExit->orig_door = door;
					word = fread_word(fp);
					if(!str_cmp(word, "ToVnum" ) )
						pExit->u1.vnum = fread_number(fp);
					else if(!str_cmp( word, "ToCord" ) )
						for(i=0;i<3;i++)
							pExit->cord[i] = fread_number(fp );
					pShaft->exit[door] = pExit;
					
					break;
				}
				if(!str_cmp(word, "End" ) )
				{   pRoom->exit[door_to] = new_exit();
					pRoom->exit[door_to]->mine = pMine;
					pRoom->exit[door_to]->orig_door = door_to;
					pRoom->exit[door_to]->to_shaft = pMine->shaft_first;
					return;
				}
				
				if(!str_cmp(word, "ENDSHAFT" ) )
				{	shaft_to_mine(pShaft, pMine, pShaft->cord[CORD_X], pShaft->cord[CORD_Y], pShaft->cord[CORD_Z] );
					
					break;
				}
				break;


			case 'N': pMine = new_mine();
					  pMine->name = fread_string(fp );
					  pMine->entrance = pRoom;
					  break;
			case 'S': 
				if(!str_cmp(word, "Shaft" ) )
				{	
					pShaft = new_shaft();
					break;
				}
				break;
			case 'X':
				for(i=0;i < MAX_MINERALS ; i++ )
					pShaft->mineral[i] = fread_number(fp);
				break;
		}

	}

			

	return;
}
