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
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"


/* The new MAXMAP_X and MAXMAP_Y is the actual size of the display map.
   To get the space left and right of each room, you need a space before
   each room, and then you have one dangling at the end. Same for above
   and below, except you hang one dangling at the bottom. So I made new
   defines for the (MAX_Y*2)+1, instead of having that expression all
   over the code. Make expanding the map later easier.

   There's better ways to do this, but it works for now. Basically you use
   the MAXMAP_* defines when you want to traverse the entire map, for when
   you initialize it and so forth. For the actual room coordinates that you
   pass to areamapper, you use the normal MAX_* defines, so you can skip over
   the exit places more accurately. You could of course do all the calculations
   manually in the code, but eh.. it works.

   To expand it in the future, you could make the ROOM_ and EXIT_ flags be a
   bitvector, then set each room to the right roomflag, then for the exits
   you can add whatever combo of flags you'd like based on what kind of exit
   it is. Then you could have different exit symbols for one-way and two-ways
   exits, or if it's a door, and if it's a door, change color if it's open
   or closed or locked, etc.

   --Chilalin
 */


#define MAX_X 80
#define MAX_Y 30
#define MAXMAP_X (MAX_X*2+1)
#define MAXMAP_Y (MAX_Y*2+1)


#define DIR_X 0
#define DIR_Y 1

#define MAX_DIR_CON 4

#define ROOM_NONE  0
#define ROOM_NORM  1
#define ROOM_UP    2
#define ROOM_DOWN  3
#define ROOM_IN    4
#define EXIT_NORTH 5
#define EXIT_SOUTH 6
#define EXIT_EAST  7
#define EXIT_WEST  8

void areamapper args( (ROOM_INDEX_DATA *pRoom, int x, int y, int rType) );
void mapexit	args( (int dir, int x, int y) );

int get_room_type args( (int dir) );
int dirconnect[MAXMAP_X*MAXMAP_Y][4];

int map[MAXMAP_X][MAXMAP_Y];


int setexit[6][2] ={ {0, -1},{ 1, 0},{ 0, 1},{ -1, 0}, {0, 0}, { 0, 0 }};

void do_map(CHAR_DATA *ch, char *argument)
{
    int i, x, y, strtX = 0;
    bool sendRow = FALSE, sColF= FALSE;
    ROOM_INDEX_DATA *pRoom;

    for(y = 0 ; y < MAXMAP_Y ; y++)
        for(x = 0 ; x < MAXMAP_X ; x++ )
            map[x][y] = 0;

    for(i = 0 ; i < MAXMAP_X*MAXMAP_Y ; i++ )
        for(x = 0 ; i < MAX_DIR_CON ; i++ )
            dirconnect[x][i] = 0;

    for(i = ch->in_room->area->min_vnum ; i < ch->in_room->area->max_vnum ; i++)
    {
        if( (pRoom = get_room_index(i)  )== NULL )
            continue;
        REMOVE_BIT(pRoom->room_flags, ROOM_MAPPED );
    }
    if(!ch->in_room)
    {
        send_to_char("Ooo your in twouble.\n\r",ch);
        return;
    }


    areamapper(ch->in_room, MAX_X/2, MAX_Y/2, ROOM_IN);
    for(x = 0; x<MAXMAP_X ; x++ )
    {
        for(i = 0; i < MAXMAP_Y ; i++ )
        {
            if(map[x][i] > 0 )
            {
                strtX = x;
                sColF = TRUE;
                break;
            }
        }
        if(sColF)
        {
            sColF=FALSE;
            break;
        }
    }
    for( y = 0; y < MAXMAP_Y ; y++ ) // Collum
    {
        for(x = 0; x<MAXMAP_X ; x++ )
        {
            if(map[x][y] > 0 )
            {
                sendRow = TRUE;
                break;
            }

        }

        if(sendRow)
        {
            for(x = strtX ; x < MAXMAP_X ; x++ ) // Row
            {
                switch( map[x][y] )
                {
                case ROOM_NONE:
                    send_to_char(" ",ch);
                    break;
                case ROOM_NORM:
                    send_to_char("{W+{x",ch);
                    break;
                case ROOM_UP:
                    send_to_char("{R+{x",ch);
                    break;
                case ROOM_DOWN:
                    send_to_char("{B+{x",ch);
                    break;
                case ROOM_IN:
                    send_to_char("{C*{x",ch);
                    break;
                case EXIT_NORTH:
                case EXIT_SOUTH:
                    send_to_char("{W|{x",ch);
                    break;
                case EXIT_EAST:
                case EXIT_WEST:
                    send_to_char("{W-{x",ch);
                    break;
                default:
                    send_to_char("X",ch);
                }
            }
            send_to_char("\n\r",ch);
        }
        sendRow = FALSE;
    }
}

void areamapper(ROOM_INDEX_DATA *pRoom, int x, int y, int rType)
{
    int dir;
    int newx, newy;
    ROOM_INDEX_DATA *nRoom;
    EXIT_DATA *pExit;

    newx = ((x*2)+1);
    newy = ((y*2)+1);

    if(newx > MAXMAP_X || newy > MAXMAP_Y )
        return;

    if(map[newx][newy] != ROOM_IN )
        map[newx][newy] = rType;

    SET_BIT(pRoom->room_flags, ROOM_MAPPED );

    for(dir = 0; dir < MAX_DIR ; dir ++ )
    {
        if( (pExit = pRoom->exit[dir] ) == NULL)
            continue;
        if( ((nRoom = pExit->u1.to_room ) == NULL) ||
           (nRoom->area != pRoom->area) )
            continue;
        mapexit( dir, newx+setexit[dir][DIR_X], newy+setexit[dir][DIR_Y] );

        if ( IS_SET(nRoom->room_flags, ROOM_MAPPED) )
            continue;


        areamapper(	nRoom, x + setexit[dir][DIR_X], y + setexit[dir][DIR_Y], get_room_type(dir) );
    }
}

int get_room_type(int dir)
{
	if (dir == DIR_UP)
		return ROOM_UP;
	if(dir == DIR_DOWN)
		return ROOM_DOWN;
	return ROOM_NORM;
}

void mapexit( int dir, int x, int y )
{
    switch( dir )
    {
    case DIR_NORTH:
    case DIR_SOUTH:
        map[x][y] = EXIT_NORTH;
        break;
    case DIR_EAST:
    case DIR_WEST:
        map[x][y] = EXIT_EAST;
        break;
    }
}


/**********************
 ** This Map system was originally developed for A Dark Portal (mudbytes.net 3083)
 ** Ported to ShadowStorm by Davion
 **********************/
char *getaline( char *, char * );
#define MAX_MAP_DIR 4
int offsets[4][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0,-1} };


typedef struct detail_map_holder
{       int x,y;
        ROOM_INDEX_DATA ***map;
        char **details;
} DetailMapHolder;

typedef struct room_holder
{       ROOM_INDEX_DATA *pRoom;
        int x,y,dir;
        struct room_holder *next, *from;
} RoomHolder;

typedef struct path_holder
{       ROOM_INDEX_DATA *pRoom;
        int to, from;
        struct path_holder *next;
} PathHolder;

bool has_shop(ROOM_INDEX_DATA *pRoom )
{       CHAR_DATA *keeper;
        for ( keeper = pRoom->people; keeper; keeper = keeper->next_in_room )
        {       if ( IS_NPC(keeper) && keeper->pIndexData->pShop != NULL )
                        return TRUE;
        }
        return FALSE;
}
PathHolder * on_path(PathHolder *path, ROOM_INDEX_DATA *pRoom)
{       PathHolder *pPath;

        for(pPath = path ; pPath ; pPath = pPath->next )
        {       if(pRoom == pPath->pRoom)
                        return pPath;
        }
        return NULL;
}

char *get_str(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, int level, PathHolder *path )
{       static char buf[4];
        PathHolder *pPath;
        sprintf(buf, "...");
        pPath = on_path(path, pRoom);
        
        switch( level )
        {       case 0:
                        if(!pRoom->exit[DIR_WEST]  || !pRoom->exit[DIR_NORTH] || IS_SET(pRoom->exit[DIR_NORTH]->exit_info, EX_CLOSED)  )
                                buf[0] = '#';
                        if(!pRoom->exit[DIR_NORTH] )
                                buf[1] = '#';
                        else
                        {       if(IS_SET(pRoom->exit[DIR_NORTH]->exit_info, EX_CLOSED) )
                                        buf[1] = '_';
                                if( pPath && (pPath->to == DIR_NORTH || pPath->from == DIR_NORTH) )
                                        buf[1] = 'N';
                        }
                        if(!pRoom->exit[DIR_EAST] || !pRoom->exit[DIR_NORTH] || IS_SET(pRoom->exit[DIR_NORTH]->exit_info, EX_CLOSED)  )
                                buf[2] = '#';
                        if(pRoom->exit[DIR_UP])
                        {       buf[2] = '^';
                                if( pPath && pPath->to == DIR_UP)
                                        buf[2] = 'U';
                        }
                        break;
                case 1:
                        if(!pRoom->exit[DIR_WEST])
                                buf[0] = '#';
                        else
                        {       if(IS_SET(pRoom->exit[DIR_WEST]->exit_info, EX_CLOSED) )
                                        buf[0] = '|';
                                if( pPath && (pPath->to == DIR_WEST || pPath->from == DIR_WEST ) )
                                        buf[0] = 'W';
                        }
                        if(has_shop(pRoom) )
                                buf[1] = '$';
                        if(pPath)
                        {       if(!pPath->next) //End of the path
                                        buf[1] = 'X';
                                else
                                {       if(pPath->to == DIR_UP || pPath->to == DIR_DOWN )
                                                buf[1] = '/';
                                        else if( pPath->to == DIR_NORTH || pPath->to == DIR_SOUTH )
                                                buf[1] = '|';
                                        else if( pPath->to == DIR_EAST || pPath->to == DIR_WEST )
                                                buf[1] = '-';
                                }
                        }
                        if(ch->in_room == pRoom )
                                buf[1] = '@';

                        if(!pRoom->exit[DIR_EAST] )
                                buf[2] = '#';
                        else
                        {       if(IS_SET(pRoom->exit[DIR_EAST]->exit_info, EX_CLOSED) )
                                        buf[2] = '|';
                                if( pPath && (pPath->to == DIR_EAST || pPath->from == DIR_EAST ) )
                                        buf[2] = 'E';

                        }
                        break;
                case 2:
                        if(!pRoom->exit[DIR_WEST] || !pRoom->exit[DIR_SOUTH] || IS_SET(pRoom->exit[DIR_SOUTH]->exit_info, EX_CLOSED) )
                                buf[0] = '#';
                        if(pRoom->exit[DIR_DOWN] )
                        {       buf[0] = 'v';
                                if( pPath && pPath->to == DIR_DOWN )
                                        buf[0] = 'D';
                        }
                        if(!pRoom->exit[DIR_SOUTH] )
                                buf[1] = '#';
                        else
                        {       if(IS_SET(pRoom->exit[DIR_SOUTH]->exit_info, EX_CLOSED) )
                                        buf[1] = '_';
                                if(pPath && (pPath->to == DIR_SOUTH || pPath->from == DIR_SOUTH) )
                                        buf[1] = 'S';
                        }
                        if(!pRoom->exit[DIR_EAST] || !pRoom->exit[DIR_SOUTH] || IS_SET(pRoom->exit[DIR_SOUTH]->exit_info, EX_CLOSED) )
                                buf[2] = '#';
                        break;
                default: return "???";
        };

        return buf;
}
char * get_description_line(int length, char *descr, char *ptr )
{       bool reached = FALSE;

        while ( 1 )
        {       if(*descr == '\0' || reached )
                        break;
                if(*descr == '\n' || *descr == '\r' )
                {       if(*descr == '\n') *ptr++ = ' ';
                        descr++;
                        length++;
                        if(length >= 80 )
                                break;
                        continue;
                }
                *ptr++ = *descr++;
                length++;
                if(length >= 70)
                        if(*descr == ' ')
                        {       descr++;
                                reached = TRUE;
                        }
        }
        *ptr = '\0';
        return descr;
}


PathHolder * GetPath(CHAR_DATA *ch, ROOM_INDEX_DATA *start, ROOM_INDEX_DATA *end, int max_steps);
void DetailMapBuild(CHAR_DATA *ch, int x, int y, DetailMapHolder *Map, ROOM_INDEX_DATA *pRoom );
void GenerateDetailMap(CHAR_DATA *ch, char *descr)
{       int x,y, i_y,i;
        DetailMapHolder *DetailMap;
        ROOM_INDEX_DATA *pRoom, *point;
        char buf[MSL], tmp[MSL], line[MSL], *ptr;
        int line_map = 0, line_on = 0;
        PathHolder *path = NULL, *p_next;

        buf[0] = '\0';

        if(IS_NPC(ch) )
                return;


        x = ch->pcdata->detail_map_x;
        y = ch->pcdata->detail_map_y;

        pRoom = ch->in_room;

        DetailMap = (DetailMapHolder *)calloc(sizeof(*DetailMap), 1);
        DetailMap->map = (ROOM_INDEX_DATA ***)calloc(sizeof(*DetailMap->map), y);
        for(i_y = 0; i_y < y ; ++i_y)
                DetailMap->map[i_y] = (ROOM_INDEX_DATA **)calloc(sizeof(*DetailMap->map[i_y] ), x);
        DetailMap->x = x;
        DetailMap->y = y;
        
        DetailMapBuild(ch, x/2, y/2, DetailMap, pRoom);
        if( (point = ch->pcdata->map_point ) )
                path = GetPath(ch, pRoom, point, 20);

        DetailMap->details = (char **) calloc(sizeof(*DetailMap->details), y * 3);
        for(i_y = 0; i_y < y * 3; ++i_y )
                DetailMap->details[i_y] = (char *)calloc(sizeof(*DetailMap->details[i_y]), x * 3 );
        for(y = 0; y < DetailMap->y * 3; ++y)
        {       for(x = 0; x < DetailMap->x ; ++x )
                {       if( ( pRoom = DetailMap->map[y/3][x])  == NULL || !explored_vnum(ch, pRoom->vnum) )
                        {       strcat(buf, "   ");
                                continue;
                        }
                        else
                        {       char buf2[4];
                                sprintf(buf2, "%s", get_str(ch, pRoom, y % 3, path) );
                                switch(buf2[0])
                                {       case '#': strcat(buf, "{D#{x"); break;
                                        case '.': strcat(buf, "{r.{x"); break;
                                        case '|': strcat(buf, "{D|{x"); break;
                                        case 'v': strcat(buf, "{Wv{x"); break;
                                        case 'W': strcat(buf, "{G-{x"); break;
                                        case 'D': strcat(buf, "{Gv{x"); break;
                                        default: sprintf(tmp, "{R%c{x", buf[0]); strcat(buf,tmp); break;
                                }
                                switch(buf2[1])
                                {       case '#': strcat(buf, "{D#{x"); break;
                                        case '.': strcat(buf, "{r.{x"); break;
                                        case '@': strcat(buf, "{R@{x"); break;
                                        case '$': strcat(buf, "{Y${x"); break;
                                        case '_': strcat(buf, "{D_{x"); break;
                                        case '/': strcat(buf, "{G/{x"); break;
                                        case 'X': strcat(buf, "{GX{x"); break;
                                        case '-': strcat(buf, "{G-{x"); break;
                                        case '|': strcat(buf, "{G|{x"); break;
                                        case 'S': case 'N': strcat(buf, "{G|{x"); break;
                                        default: sprintf(tmp, "{R%c{x", buf[1]); strcat(buf,tmp); break;
                                }
                                switch(buf2[2])
                                {       case '#': strcat(buf, "{D#{x"); break;
                                        case '.': strcat(buf, "{r.{x"); break;
                                        case '|': strcat(buf, "{D|{x"); break;
                                        case '^': strcat(buf, "{M^{x"); break;
                                        case 'E': strcat(buf, "{G-{x"); break;
                                        case 'U': strcat(buf, "{G^{x"); break;
                                        default: sprintf(tmp, "{R%c{x", buf[2]); strcat(buf,tmp); break;
                                }
                        }
                }
                strcat(buf, "\n\r");
        }
        ptr = descr;
        line_map += (DetailMap->y * 3)+2; //plus 2 for % border;
        ptr = buf;
        while( 1 )
        {       line_on++;
                
                if(line_on <= line_map)
                {       if(line_on == 1 || line_on == line_map)
                        {       for(i = 0; i < (DetailMap->x * 3) +2; ++i)
                                        send_to_char("{W%{x",ch);
                        }
                        else
                        {       ptr = getaline(ptr, line);
                                printf_to_char(ch, "{W%%{x%s{W%%{x", line);
                        }
                }

                if( *descr != '\0' )
                {       int len = DetailMap->y*3+3;
                        if(line_on > line_map )
                                len = 0;
                        descr = get_description_line(len, descr, line);
                        printf_to_char(ch, "  %s", line);
                }
                if( *descr == '\0' && line_on > line_map )
                        break;
                else
                        send_to_char("\n\r",ch);
        }
        send_to_char("\n\r",ch);
 
        for(i_y = 0; i_y < DetailMap->y ; ++i_y )
                free(DetailMap->map[i_y]);
        free(DetailMap->map);
        for(i_y = 0; i_y < DetailMap->y * 3; ++i_y )
                free(DetailMap->details[i_y]);
        free(DetailMap->details);

        free(DetailMap);
        if(path)
                for(;path ; path = p_next)
                {       p_next = path->next;
                        free(path);
                }
}
                         

bool if_mapped(RoomHolder *first, int vnum)
{       RoomHolder *i;
        for(i = first ; i != NULL ; i = i->next )
        {       if(i->pRoom->vnum == vnum)
                        return TRUE;
        }
        return FALSE;
}






//This uses BFS like DetailMapBuild but it searches up and down, so the path exists if it goes up or down.
//The walkto pathfinding algorithm in act_move.c on the NW uses recurssion and simply shoots off and maps one direction
//at a time. Using BFS means I'll get the closest location first. No need to store multiple paths and leave the
//system open to infinite loops.
//Going to recycle RoomHolder so we don't have bunch of struct defs. x will be used for steps
PathHolder * GetPath(CHAR_DATA *ch, ROOM_INDEX_DATA *start, ROOM_INDEX_DATA *end, int max_steps)
{       RoomHolder *black_list, *white_list, *i_r, *i_next = NULL, *hold, *i_last;
        EXIT_DATA *pExit;
        ROOM_INDEX_DATA *to_room;
        int dir;
        PathHolder *path_list = NULL, *pPath;

        bool found = FALSE; //If we found the room, no need to map more, just throw everything allocated to the white_list

        white_list = NULL;
        black_list = (RoomHolder *)calloc(sizeof(*black_list), 1);
        black_list->pRoom = start;
        i_last = black_list;

        for(i_r = black_list ; i_r ; i_r = i_next)
        {       if(i_r->pRoom == end )
                        break;
                if(i_r->x + 1 <= max_steps && !found )
                for(dir = 0; dir < MAX_DIR ; ++dir)
                {       if( !(pExit = i_r->pRoom->exit[dir] ) )
                                continue;
                        if( !(to_room = pExit->u1.to_room )
                        || if_mapped(white_list, to_room->vnum )
                        || to_room == i_r->pRoom )
                                continue;
                        hold = (RoomHolder *)calloc(sizeof(*hold), 1);
                        hold->x = i_r->x+1;
                        hold->pRoom = to_room;
                        hold->from = i_r;
                        hold->dir = rev_dir[dir];
                        i_last->next = hold;
                        i_last = hold;
                }
                i_next = i_r->next;
                i_r->next = white_list;
                white_list = i_r;
        }

        if(i_r)//Path was found. Lets build it!
        {       int temp_to = -1;
                for( hold = i_r ; hold ; hold = hold->from )
                {       pPath = (PathHolder *)calloc(1, sizeof(*pPath) );
                        pPath->pRoom = hold->pRoom;
                        pPath->from = hold->dir;
                        pPath->to = temp_to;
                        temp_to = rev_dir[pPath->from];
                        pPath->next = path_list;
                        path_list = pPath;
                }
                path_list->from = -1; //Starting point.
                i_next = NULL;
                for( ; i_r ; i_r = i_next)
                {       i_next = i_r->next;
                        i_r->next = white_list;
                        white_list = i_r;
                }
        }
        //Free what we've allocated
        for( i_r = white_list ; i_r ; i_r = i_last )
        {       i_last = i_r->next;
                free(i_r);
        }
        return path_list;
}

void DetailMapBuild(CHAR_DATA *ch, int x, int y, DetailMapHolder *Map, ROOM_INDEX_DATA *pRoom )
{       EXIT_DATA *pExit;
        RoomHolder *black_list, *white_list, *i_r,  *i_last, *hold, *i_next;
        ROOM_INDEX_DATA *to_room;
        int dir;

        white_list = NULL;
        black_list = (RoomHolder *)calloc(sizeof(*black_list), 1);
        black_list->x = x;
        black_list->y = y;
        black_list->pRoom = pRoom;
        i_last = black_list;
        
        for(i_r = black_list ; i_r ; i_r = i_next)
        {       Map->map[i_r->y][i_r->x] = i_r->pRoom;  
                

                for( dir = 0 ; dir < MAX_MAP_DIR ; ++dir )
                {       int new_x, new_y;

                        if( !(pExit = i_r->pRoom->exit[dir] ) )
                                continue;
                        if( !(to_room = pExit->u1.to_room )
                        || if_mapped(white_list, to_room->vnum )
                        || to_room == i_r->pRoom )
                                continue;
                        new_x = i_r->x + offsets[dir][1];
                        new_y = i_r->y + offsets[dir][0];
                        if(new_x >= Map->x || new_y >= Map->y || new_x < 0 || new_y < 0 ||  Map->map[new_y][new_x] != 0 )
                                continue;
                        
                        hold = (RoomHolder *)calloc(sizeof(*hold), 1);
                        hold->x = new_x;
                        hold->y = new_y;
                        hold->pRoom = to_room;
                        i_last->next = hold;
                        i_last = hold;
                }
                i_next = i_r->next;
                i_r->next = white_list;
                white_list = i_r;
        }

        for( i_r = white_list ; i_r ; i_r = i_last )
        {       i_last = i_r->next;
                free(i_r);
        }

}













