/*******************************************************************************
 *                 ROM Version 2.4 beta                                        *
 *      Original DikuMUD by Hans Staerfeldt, Katja Nyboe,                      *
 *      Tom Madsen, Michael Seifert, and Sebastian Hammer                      *
 *      Based on MERC 2.1 code by Hatchet, Furey, and Kahn                     *
 *      ROM 2.4 copyright (c) 1993-1996 Russ Taylor                            *
 *                                                                             *
 * ___  _  _ _  _ ____ ____ ____ _  _    ____ _  _ _  _ _  _ ____ ____ ____    *
 * |  \ |  | |\ | | __ |___ |  | |\ |    |__/ |  | |\ | |\ | |___ |__/ [__     *
 * |__/ |__| | \| |__] |___ |__| | \|    |  \ |__| | \| | \| |___ |  \ ___]    *
 *              kyncode version: 3b.99  (not yet released)                     *
 *******************************************************************************/
/* Minor modifications made by Dalsor to make more WoT-like */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>

#include "merc.h"
#include "recycle.h"
#include "interp.h"
#include "const.h"

char *nsprintf(char *fr, char *fmt, ...)
{
        char buf[2*MSL];
        va_list args;

        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);

        free_string(fr);
        return str_dup(buf);
}

EVENT_DATA *create_event(int delay, char *act)
{
        EVENT_DATA *ev;

        if(!str_cmp(act, "print"))
        {
                ev=new_event();
                ev->action=ACTION_PRINT;
        } else if(!str_cmp(act, "wait"))
        {
                ev=new_event();
                ev->action=ACTION_WAIT;
        } else if(!str_cmp(act, "act"))
        {
                ev=new_event();
                ev->action=ACTION_ACT;
        } else if(!str_cmp(act, "function"))
        {       ev=new_event();
                ev->action=ACTION_FUNCTION;
        }
        else { /* Bad Action */
                return NULL;
        }
        ev->delay= delay;
        ev->next = events;
        events   = ev;
        return ev;
}

/* send an 'act' sequence to target after delay has been met */
void wait_act(int delay, char *msg, void *a1, void *a2, void *a3, int type)
{
        EVENT_DATA *ev=create_event(delay, "act");
        ev->args[0]=str_dup(msg);
        ev->argv[0]=a1;
        ev->argv[1]=a2;
        ev->argv[2]=a3;
        ev->argi[0]=type;
        return;
}

/* send a delayed FORMATED string to target ( Uses printf_ function ) */
void wait_printf(CHAR_DATA *ch, int delay, char * fmt, ...)
{
        char buf[2*MSL];
        va_list args;
        EVENT_DATA *ev = create_event(delay, "print");

        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);

        ev->args[0] = str_dup(buf);
        ev->argv[0] = ch;
        return;
}

/* after the delay has been met..it will freeze
 * the targeted victim for the set amount of 'dur'ation
 * ex:  wait_wait(ch,2,5); would:
 * cause player who triggered this call to be frozen
 * for 5 seconds after 2 seconds of function call
 * good for mprogs
 */
void wait_wait(CHAR_DATA *ch, int delay, int dur)
{
        EVENT_DATA *ev = create_event( delay, "wait");

        ev->argv[0] = ch;
        ev->argi[0] = dur;
}

/* call the specified function after delay has been met */
void wait_function (CHAR_DATA *ch, int delay, DO_FUN *do_fun, char *argument)
{
   EVENT_DATA *ev = create_event( delay, "function" );

   ev->argv[0] = ch;
   ev->do_fun  = do_fun;
   ev->args[0] = str_dup( argument );
   return;
}
