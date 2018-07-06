
#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>              
#include <stdarg.h>
#include "include.h"

/*

 Implementation of a dynamically expanding buffer.
 
 Inspired by Russ Taylor's <rtaylor@efn.org> buffers in ROM 2.4b2.
 
 The buffer is primarily used for null-terminated character strings.
 
 A buffer is allocated with buffer_new, written to using buffer_strcat,
 cleared (if needed) using buffer_clear and free'ed using buffer_free.
 
 If BUFFER_DEBUG is defined, the buffer_strcat call is defined as having
 2 extra parameters, __LINE__ and __FILE__. These are then saved
 to the bug file if an overflow occurs.
 
 Erwin S. Andreasen <erwin@pip.dknet.dk>
 
*/ 

#define EMEM_SIZE -1 /* find_mem_size returns this when block is too large */
#define NUL '\0'

/* Find in rgSizeList a memory size at least this long */
int find_mem_size (int min_size)
{
	int i;
	
	for (i = 0; i < MAX_MEM_LIST; i++)
		if (rgSizeList[i] >= min_size)
			return rgSizeList[i];
	
	/* min_size is bigger than biggest allowable size! */
	
	return EMEM_SIZE;
}

/* Create a new buffer, of at least size bytes */

#ifndef BUFFER_DEBUG /* no debugging */
BUFFER1 * __buffer_new (int min_size)

#else				 /* debugging - expect filename and line */
BUFFER1 * __buffer_new (int min_size, const char * file, unsigned line) 
#endif

{
	int size;
	BUFFER1 *buffer;
	char buf[200]; /* for the bug line */
	
	size = find_mem_size (min_size);
	
	if (size == EMEM_SIZE)
	{
#ifdef BUFFER_DEBUG
		sprintf (buf, "Buffer size too big: %d bytes (%s:%u).", min_size, file, line);
#else
		sprintf (buf, "Buffer size too big: %d bytes.", min_size);
#endif

		bug (buf,0);
		abort();
	}
	
	buffer = (BUFFER1 *)alloc_mem (sizeof(BUFFER1));
	
	buffer->size = size;
	buffer->data = (char *)alloc_mem (size);
	buffer->overflowed = FALSE;
	
	buffer->len = 0;
	
	return buffer;
} /* __buf_new */

/* Add a string to a buffer. Expand if necessary */

#ifndef BUFFER_DEBUG /* no debugging */
void __buffer_strcat (BUFFER1 *buffer, const char *text)

#else				 /* debugging - expect filename and line */
void __buffer_strcat (BUFFER1 *buffer, const char *text, const char * file, unsigned line)
#endif

{
	int new_size;
	int text_len;
	char *new_data;
	char buf[200];
	
	if (buffer->overflowed) /* Do not attempt to add anymore if buffer is already overflowed */
		return;

	if (!text) /* Adding NULL string ? */
		return;
	
	text_len = strlen(text);
	
	if (text_len == 0) /* Adding empty string ? */
		return;
		
	/* Will the combined len of the added text and the current text exceed our buffer? */

	if ((text_len+buffer->len+1) > buffer->size) /* expand? */
	{
		new_size = find_mem_size (buffer->size + text_len + 1);
		if (new_size == EMEM_SIZE) /* New size too big ? */
		{
#ifdef BUFFER_DEBUG
			sprintf (buf, "Buffer overflow, wanted %d bytes (%s:%u).", text_len+buffer->len, file, line);
#else
			sprintf (buf, "Buffer overflow, wanted %d bytes.",text_len+buffer->len);
#endif				
			bug (buf, 0);
			buffer->overflowed = TRUE;
			return;
		}

		/* Allocate the new buffer */
		
		new_data = (char *)alloc_mem (new_size);		
		
		/* Copy the current buffer to the new buffer */
		
		memcpy (new_data, buffer->data, buffer->len);
		free_mem (buffer->data, buffer->size);
		buffer->data = new_data;
		buffer->size = new_size;

	} /* if */

	memcpy (buffer->data + buffer->len, text, text_len);	/* Start copying */
	buffer->len += text_len;	/* Adjust length */
	buffer->data[buffer->len] = NUL; /* Null-terminate at new end */
	
} /* __buf_strcat */


/* Free a buffer */
void buffer_free (BUFFER1 *buffer)
{
	/* Free data */
	free_mem (buffer->data, buffer->size);
	
	/* Free buffer */
	
	free_mem (buffer, sizeof(BUFFER1));
}

/* Clear a buffer's contents, but do not deallocate anything */

void buffer_clear (BUFFER1 *buffer)
{
	buffer->overflowed = FALSE;
	buffer->len = 0;
}

/* print stuff, append to buffer. safe. */
int bprintf (BUFFER1 *buffer, char *fmt, ...)
{
	char buf[MSL];
	va_list va;
	int res;
	
	va_start (va, fmt);
	res = vsnprintf (buf, MSL, fmt, va);
	va_end (va);

	if (res >= MSL-1)	
	{
		buf[0] = NUL;
		bugf ("Overflow when printing string %s", fmt);
	}
	else
		buffer_strcat (buffer, buf);

	return res;	
}

