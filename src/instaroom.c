/* Instaroom code by Kyndig
 * Allows resets to be placed on a room, according to current
 * mob/obj/container/exit placement of that room.
 * Syntax: instaroom
 *
 *
 * Additional needed install instructions:
 * Place the following 2 lines in => struct room_index_data{ in merc.h:
 *   RESET_DATA *        last_mob_reset;
 *   RESET_DATA *        last_obj_reset;
 * 
 * Place the following line in => struct  reset_data{ in merc.h
 * RESET_DATA *        prev;
 *
 * You can build an 'instant area reset maker' based off this code
 * Developed by Kyndig from http://www.kyndig.com/  kyndig@kyndig.com
 */
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
#include <unistd.h>
#include "merc.h"
#include "olc.h"
#include "recycle.h"
#include "const.h"


/* Locals */
void wipe_resets( ROOM_INDEX_DATA *pRoom );
void instaroom( ROOM_INDEX_DATA *pRoom );


/* NOTE: This does not create RANDOM room exits, my exit system was
 * abit to much to port it to stock ROM so I simply removed. It will be up
 * to you to edit these functions to return what you desire
 */


/* Original idea taken from the SMAUG server, the LINK macro is a direct
 * reflection of the outstanding work of Thoric from realms.org
 */

/* double-linked list handling macros -Thoric */
#define LINK(link, first, last, next, prev)                     \
do                                                              \
{                                                               \
    if ( !(first) )                                             \
      (first)                   = (link);                       \
    else                                                        \
      (last)->next              = (link);                       \
    (link)->next                = NULL;                         \
    (link)->prev                = (last);                       \
    (last)                      = (link);                       \
} while(0)



/* called from do_instaroom further below */
void reset_instaroom( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA     *pRoom;
  CHAR_DATA           *mob;
  OBJ_DATA            *obj, *inobj;

  pRoom = ch->in_room;

  /* Containers can NOT be closed when doing an 'instaroom'. Thus a builder will have to close
   * the container after the reset is installed
   */

  /* lets go through a mob first */
  for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
  {
     if ( IS_NPC( mob ) )
     {
         /* only mobs with this areas vnums are allowed */
        if ( mob->pIndexData->area != pRoom->area )
        {
           send_to_char( "There is a mob in this room that is not part of your area, resets not set.\n\r", ch );
           return;
        }
    
        for ( obj = mob->carrying; obj; obj = obj->next_content )
        {
           if( obj->pIndexData->area != pRoom->area )
           {
               send_to_char( "There is an object in a MOB that is not part of your area, resets not set.\n\r", ch );
              return;
           }
      
           if ( IS_SET( obj->value[1], CONT_CLOSED ) )
           {
              send_to_char( "There is a container in a MOB I can't see inside of. Get the container from the mob, open\n\r"
                            "it up, give it _back_ to your mob, then do an instaroom.\n\r" 
                            "AFTER you have set the container to load into the mob,\n\r"
                            "you can then get the container from him again, close/lock it, and return it....In other words\n\r"
                            "A container must be open first, the reset installed, after that, you can close/lock it.\n\r",ch);
              return;
           }

           if ( obj->contains )
           {
               for (inobj = obj->contains; inobj; inobj = obj->next_content )
               {
                   if ( inobj->pIndexData->area != pRoom->area )
                   {
                      send_to_char("There is an object in a container which a MOB in this room has, which is not\n\r"
                                   "a vnum for this area. RESETS NOT SET.\n\r",ch);
                      return;
                   }
               }
           }
        }
     }
  }/* done looking at mobs and their eq/inv */


  /* lets take a look at objects in the room and their contents */ 
  for ( obj = pRoom->contents; obj != NULL; obj = obj->next_content )
  {
      if( obj->pIndexData->area != pRoom->area )
      {
          send_to_char( "There is an object in this room that is not a vnum of your area, resets not set.\n\r", ch );
          return;
      }
  
      if ( IS_SET( obj->value[1], CONT_CLOSED ) )
      {
          send_to_char( "There is a container in this room I can't see inside of. Open it up first, do the\n\r"
                        "instaroom command, THEN you can close/lock the container.\n\r",ch);
          return;
      }

      if ( obj->contains )
      {
         for (inobj = obj->contains; inobj; inobj = obj->next_content )
         {
            if ( inobj->pIndexData->area != pRoom->area )
            {
               send_to_char("There is an object in a container that does not a vnum in this area. No resets set.\n\r",ch);
               return;
            }
         }
      }
  }

  if ( pRoom->reset_first )
    wipe_resets(pRoom);  
  instaroom( pRoom );

  send_to_char( "Room resets installed.\n\r", ch );
  return;
}

/* Separate function for recursive purposes */
void delete_reset( ROOM_INDEX_DATA *pRoom, RESET_DATA *pReset, int insert_loc, bool wipe_all)
{

  if( !pRoom->reset_first )
  {
     return;
  }

  if ( insert_loc-1 <= 0 )
  {
       pReset = pRoom->reset_first;
       pRoom->reset_first = pRoom->reset_first->next;
       if ( !pRoom->reset_first )
           pRoom->reset_last = NULL;
   }
   else
   {
       int iReset = 0;
       RESET_DATA *prev = NULL;

       for ( pReset = pRoom->reset_first;
             pReset;
             pReset = pReset->next )
       {
             if ( ++iReset == insert_loc )
                    break;
             prev = pReset;
       }

       if ( !pReset )
       {
           return;
       }

        if ( prev )
            prev->next = prev->next->next;
        else
            pRoom->reset_first = pRoom->reset_first->next;

        for ( pRoom->reset_last = pRoom->reset_first;
              pRoom->reset_last->next;
              pRoom->reset_last = pRoom->reset_last->next );
   }/*else*/

   if ( pReset == pRoom->last_mob_reset )
    pRoom->last_mob_reset = NULL;
  if ( pReset == pRoom->last_obj_reset )
    pRoom->last_obj_reset = NULL;

   free_reset_data( pReset );

   /* TRUE/FALSE call on delete_reset here, so we can delete all resets or just one */
   if( wipe_all )
   {
     if( ( pReset = pRoom->reset_first)  != '\0')
       delete_reset( pRoom, pReset, 1, TRUE );
   }
}

void wipe_resets( ROOM_INDEX_DATA *pRoom )
{
  RESET_DATA *pReset;
  int i = 1;
  
  for ( pReset = pRoom->reset_first; pReset != NULL; )
  {
    if (pReset)
    {
      delete_reset(pRoom, pReset, i, TRUE);
    }
    i++;
    pReset = pReset->next;
  }
  return;
}

/* make_reset called by add_new_reset in order to create a blank
 * reset to load data into the reset_list 
 */
RESET_DATA *make_reset( char letter, int arg1, int arg2, int arg3, int arg4 )
{
        RESET_DATA *pReset;

        pReset          = new_reset_data();
        pReset->command = letter;
        pReset->arg1    = arg1;
        pReset->arg2    = arg2;
        pReset->arg3    = arg3;
        pReset->arg4    = arg4;
        return pReset;
}


/* add_new_reset called from several places below, it adds in the actual pReset
 * arguments to the reset_list..tricky footwork here 
 */
RESET_DATA *add_new_reset( ROOM_INDEX_DATA *pRoom, char letter, int arg1, int arg2, int arg3, int arg4 )
{
    RESET_DATA *pReset;

    if ( !pRoom )
    {
        bug( "add_reset: NULL area!", 0 );
        return NULL;
    }

    letter = UPPER(letter);
    pReset = make_reset( letter, arg1, arg2, arg3, arg4 );
    switch( letter )
    {
        case 'M':  pRoom->last_mob_reset = pReset;      break;
        case 'E':  case 'G':  case 'P':
        case 'O':  pRoom->last_obj_reset = pReset;      break;
            break;
    }

    /* LINK code from the SMAUG server */
    LINK( pReset, pRoom->reset_first, pRoom->reset_last, next, prev );
    return pReset;
}


/* Called from instaroom */
/* The below function debugged by Chalc on the Romlist, thanx again Chalc */
void add_obj_reset( ROOM_INDEX_DATA *pRoom, char cm, OBJ_DATA *obj, int v2, int v3, int v4 )
{
  add_new_reset( pRoom, cm, obj->pIndexData->vnum, v2, v3, v4 );

  if(obj->contains)
  {
    OBJ_INDEX_DATA **objList;
    OBJ_DATA *inObj;
    int *objCount;
    int count;
    int itemCount;

    for(inObj = obj->contains, itemCount = 0; inObj; inObj = inObj->next_content) itemCount++;

    /* Now have count objects in obj, allocate space for lists */
    objList = (OBJ_INDEX_DATA **)alloc_mem(itemCount * sizeof(OBJ_INDEX_DATA *));
    objCount = (int *)alloc_mem(itemCount * sizeof(int));
    
    /* Initialize Memory */
    memset(objList, 0, itemCount * sizeof(OBJ_INDEX_DATA *));
    memset(objCount, 0, itemCount * sizeof(int));

    /* Figure out how many of each obj is in the container */
    for(inObj = obj->contains; inObj; inObj = inObj->next_content)
    {
      for(count = 0; objList[count] && objList[count] != inObj->pIndexData; count++);
      if(!objList[count]) objList[count] = inObj->pIndexData;
      objCount[count]++;
    }    

    /* Create the resets */
    for(count = 0; objList[count]; count++)
      add_new_reset(pRoom, 'P', objList[count]->vnum, objCount[count], obj->pIndexData->vnum, objCount[count]);

    /* Free the memory */
    free_mem(objList, itemCount * sizeof(OBJ_INDEX_DATA *));
    free_mem(objCount, itemCount * sizeof(int));
  }
  /* And Done */
  return;
}

void instaroom( ROOM_INDEX_DATA *pRoom )
{
  CHAR_DATA *rch;
  OBJ_DATA *obj;
  
  for ( rch = pRoom->people; rch; rch = rch->next_in_room )
  {
    if ( !IS_NPC(rch) )
      continue;
    add_new_reset( pRoom, 'M',rch->pIndexData->vnum, rch->pIndexData->count, pRoom->vnum, 3 );
    for ( obj = rch->carrying; obj; obj = obj->next_content )
    {
      if ( obj->wear_loc == WEAR_NONE )
        add_obj_reset( pRoom, 'G', obj, 1, 0, 3 );
      else
        add_obj_reset( pRoom, 'E', obj, 1, obj->wear_loc, 3 );
    }
  }
  for ( obj = pRoom->contents; obj; obj = obj->next_content )
  {
    add_obj_reset( pRoom, 'O', obj, 1, pRoom->vnum, 1 );
  }
  return;
}

void do_instaroom( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *pRoom = ch->in_room;
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument, arg);

  if(!strstr( ch->in_room->area->builders, ch->name ) )
  {  send_to_char("You aren't the builder of this area!\n\r",ch);
     return;
  }

  if(ch->pcdata->security < ch->in_room->area->security )
  {     
        send_to_char("You don't have the security to edit this area.\n\r",ch);
	return;
  }

  if ( pRoom->reset_first )
    wipe_resets(pRoom);  
  instaroom( pRoom );

  send_to_char( "Room resets installed.\n\r", ch );
}

