/*** buffer.h ****/

#define BUFFER_DEBUG

typedef struct buffer_type BUFFER1;

struct buffer_type
{
	char *data; 	/* The data */
        int len;	/* The current len of the buffer */
        int size;	/* The allocated size of data */
        bool overflowed; /* Is the buffer overflowed? */
};

#ifdef BUFFER_DEBUG /* Debugged version */

#define buffer_new(size)           __buffer_new (size, __FILE__, __LINE__)
#define buffer_strcat(buffer,text) __buffer_strcat (buffer, text, __FILE__, __LINE__)

BUFFER1 * __buffer_new (int size, const char *file, unsigned line);
void __buffer_strcat (BUFFER1 *buffer, const char *text, const char *file, unsigned line);

#else  /* not debugged version */

#define buffer_new(size)           __buffer_new (size)
#define buffer_strcat(buffer,text) __buffer_strcat (buffer,text)

BUFFER1 * __buffer_new (int size);
void __buffer_strcat (BUFFER1 *buffer, const char *text);

#endif


void buffer_free (BUFFER1 *buffer);
void buffer_clear (BUFFER1 *buffer);
int find_mem_size (int min_size);
int bprintf (BUFFER1 *buffer, char *fmt, ...);




