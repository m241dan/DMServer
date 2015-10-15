/*
 * This file handles string copy/search/comparison/etc.
 */
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

/* include main header file */
#include "mud.h"

/*
 * Checks if aStr is a prefix of bStr.
 */
size_t mudcat( char *dst, const char *src  )
{
   register char *d = dst;
   register const char *s = src;
   register size_t n = MAX_BUFFER;
   size_t dlen;

   if( !src )
   {
      bug( "%s: NULL src string passed!", __FUNCTION__ );
      return 0;
   }

   /*
    * Find the end of dst and adjust bytes left but don't go past end
    */
   while( n-- != 0 && *d != '\0' )
      d++;
   dlen = d - dst;
   n = MAX_BUFFER - dlen;

   if( n == 0 )
      return ( dlen + strlen( s ) );
   while( *s != '\0' )
   {
      if( n != 1 )
      {
         *d++ = *s;
         n--;
      }
      s++;
   }
   *d = '\0';
   return ( dlen + ( s - src ) );   /* count does not include NUL */
}

bool is_prefix(const char *aStr, const char *bStr)
{
  /* NULL strings never compares */
  if (aStr == NULL || bStr == NULL) return FALSE;

  /* empty strings never compares */
  if (aStr[0] == '\0' || bStr[0] == '\0') return FALSE;

  /* check if aStr is a prefix of bStr */
  while (*aStr)
  {
    if( *bStr == '\0' )
       break;
    if (tolower(*aStr++) != tolower(*bStr++))
      return FALSE;
  }

  /* success */
  return TRUE;
}

int number_arg( char *fStr, char *bStr )
{
   char buf[MAX_BUFFER];

   if( !string_contains( fStr, "\\." ) )
      return -1;

   fStr = one_arg_delim( fStr, buf, '.' );

   memcpy( bStr, fStr, strlen( fStr ) + 1 );

   if( !strcasecmp( buf, "all" ) && bStr[0] != '\0' )
      return -2;
   else if( !is_number( buf ) )
      return -1;
   else
      return atoi( buf );
}

int number_arg_single( char *string )
{
   char buf[MAX_BUFFER];
   char *orig_string = string;
   char *non_inc_orig_string = orig_string;

   if( !string_contains( string, "\\." ) )
      return -1;

   string = one_arg_delim( string, buf, '.' );

   while( string && *string != '\0' )
      *orig_string++ = *string++;
   *orig_string = '\0';

   if( !strcasecmp( buf, "all" ) && non_inc_orig_string[0] != '\0' )
      return -2;
   else if( !is_number( buf ) )
      return -1;
   else
      return atoi( buf );
}

char *one_arg(char *fStr, char *bStr)
{
   char delim;

  /* skip leading spaces */
  while (isspace(*fStr))
    fStr++;

   delim = ' ';
   if( *fStr == '"' )
     delim = *fStr++;

  /* copy the beginning of the string */
  while (*fStr != '\0')
  {
    /* have we reached the end of the first word ? */
    if (*fStr == delim)
    {
      fStr++;
      break;
    }

    /* copy one char */
    *bStr++ = *fStr++;
  }

  /* terminate string */
  *bStr = '\0';

  /* skip past any leftover spaces */
  while (isspace(*fStr))
    fStr++;

  /* return the leftovers */
  return fStr;
}

char *one_arg_delim( char *fStr, char *bStr, char delim )
{
  /* skip leading spaces */
  while (isspace(*fStr))
    fStr++;

  /* copy the beginning of the string */
  while (*fStr != '\0')
  {
    /* have we reached the end of the first word ? */
    if (*fStr == delim)
    {
      fStr++;
      break;
    }

    /* copy one char */
    *bStr++ = *fStr++;
  }

  /* terminate string */
  *bStr = '\0';

  /* skip past any leftover spaces */
  while (isspace(*fStr))
    fStr++;

  /* return the leftovers */
  return fStr;
}


/* doens't skip leading spaces */
char *one_arg_delim_literal( char *fStr, char *bStr, char delim )
{
  /* copy the beginning of the string */
  while (*fStr != '\0')
  {
    /* have we reached the end of the first word ? */
    if (*fStr == delim)
    {
      fStr++;
      break;
    }

    /* copy one char */
    *bStr++ = *fStr++;
  }

  /* terminate string */
  *bStr = '\0';

  /* return the leftovers */
  return fStr;

}

char *capitalize(char *txt)
{
   static char buf[MAX_BUFFER];
   mud_printf( buf, "%s", downcase( txt ) );
   buf[0] = toupper( buf[0] );
   return buf;
}

char *downcase( const char *txt )
{
   static char buf[MAX_BUFFER];
   int size, x;
   memset( &buf[0], 0, sizeof( buf ) );

   if( !txt || txt[0] == '\0' )
      return buf;

   size = strlen( txt );

   for( x = 0; x < size; x++ )
      buf[x] = tolower( txt[x] );
   buf[size] = '\0';

   return buf; 
}

/*  
 * Create a new buffer.
 */
BUFFER *__buffer_new(int size)
{
  BUFFER *buffer;
    
  buffer = malloc(sizeof(BUFFER));
  buffer->size = size;
  buffer->data = malloc(size);
  buffer->len = 0;
  return buffer;
}

/*
 * Add a string to a buffer. Expand if necessary
 */
void __buffer_strcat(BUFFER *buffer, const char *text)  
{
  int new_size;
  int text_len;
  char *new_data;
 
  /* Adding NULL string ? */
  if (!text)
    return;

  text_len = strlen(text);
    
  /* Adding empty string ? */ 
  if (text_len == 0)
    return;

  /* Will the combined len of the added text and the current text exceed our buffer? */
  if ((text_len + buffer->len + 1) > buffer->size)
  { 
    new_size = buffer->size + text_len + 1;
   
    /* Allocate the new buffer */
    new_data = malloc(new_size);
  
    /* Copy the current buffer to the new buffer */
    memcpy(new_data, buffer->data, buffer->len);
    FREE(buffer->data);
    buffer->data = new_data;  
    buffer->size = new_size;
  }
  memcpy(buffer->data + buffer->len, text, text_len);
  buffer->len += text_len;
  buffer->data[buffer->len] = '\0';
}

/* free a buffer */
void buffer_free(BUFFER *buffer)
{
  /* Free data */
  FREE(buffer->data);
 
  /* Free buffer */
  FREE(buffer);
}

/* Clear a buffer's contents, but do not deallocate anything */
void buffer_clear(BUFFER *buffer)
{
  buffer->len = 0;
  buffer->data[0] = '\0';
}

/* print stuff, append to buffer. safe. */
int bprintf(BUFFER *buffer, char *fmt, ...)
{  
  char buf[MAX_BUFFER];
  va_list va;
  int res;
    
  va_start(va, fmt);
  res = vsnprintf(buf, MAX_BUFFER, fmt, va);
  va_end(va);
    
  if (res >= MAX_BUFFER - 1)  
  {
    buf[0] = '\0';
    bug("Overflow when printing string %s", fmt);
  }
  else
    buffer_strcat(buffer, buf);
   
  return res;
}

char *strdup(const char *s)
{
  char *pstr = NULL;
  int len;

  len = strlen(s) + 1;
  pstr = (char *)malloc( len );
  mud_printf( pstr, "%s", s );
  return pstr;
}

int strcasecmp(const char *s1, const char *s2)
{
  int i = 0;

  while (s1[i] != '\0' && s2[i] != '\0' && toupper(s1[i]) == toupper(s2[i]))
    i++;

  /* if they matched, return 0 */
  if (s1[i] == '\0' && s2[i] == '\0')
    return 0;

  /* is s1 a prefix of s2? */
  if (s1[i] == '\0')
    return -110;

  /* is s2 a prefix of s1? */
  if (s2[i] == '\0')
    return 110;

  /* is s1 less than s2? */
  if (toupper(s1[i]) < toupper(s2[i]))
    return -1;

  /* s2 is less than s1 */
  return 1;
}

int mud_printf( char *dest, const char *format, ... )
{
   va_list va;
   int res;

   va_start( va, format );
   res = vsnprintf( dest, MAX_BUFFER, format, va );
   va_end( va );

   if( res >= MAX_BUFFER -1 )
   {
      dest[0] = '\0';
      bug( "Overflow when printing string %s", format );
   }

   return res;
}
/*
char *colorize( const char *to_color )
{
   static char buf[MAX_BUFFER];
   char *ptr;
   memset( &buf[0], 0 sizeof( buf ) );

   ptr = buf;

   while( *to_color )
   {
      
   }


}
*/

const char *print_header( const char *title, const char *pattern, int width )
{
   static char buf[MAX_BUFFER];
   const char *pat_ptr;
   char *buf_ptr;
   int title_len = strlen( smash_color( title ) );
   int raw_title_len = strlen( title );
   int pattern_len = strlen( smash_color( pattern ) );
   int each_sides_pattern_len, side_pattern_remainder, loop_limit, extra, x;

   memset( &buf, 0, sizeof( buf ) );

   each_sides_pattern_len = ( width - title_len - 2 ) / 2; /* minus two for preceeding and appending spaces to the title */
   side_pattern_remainder = each_sides_pattern_len % pattern_len;
   loop_limit = each_sides_pattern_len - side_pattern_remainder;
   extra = title_len % 2;

   pat_ptr = print_bar( pattern, loop_limit );
   strcat( buf, pat_ptr );

   buf_ptr = &buf[strlen(buf)];

   for( x = -1 ; x < side_pattern_remainder; x++ )
      *buf_ptr++ = ' ';

   for( x = 0; x < raw_title_len; x++ )
      *buf_ptr++ = *title++;

   for( x = -1; x < ( side_pattern_remainder + extra ); x++ )
      *buf_ptr++ = ' ';

   strcat( buf, pat_ptr );
   return buf;
}

void bprint_commandline( void *extra, BUFFER *buf, COMMAND *com, int sublevel, int pagewidth )
{
   char command_desc[MAX_BUFFER];
   char symbol[4];
   int subindent, commandspace, commanddescspace = 0;

   if( com->can_sub )
   {
      if( !com->sub_commands )
         snprintf( symbol, 4, "(+)" );
      else
         snprintf( symbol, 4, "(-)" );
   }
   else if( sublevel > 0 )
      snprintf( symbol, 4, "  -" );
   else
      snprintf( symbol, 4, "   " );

   subindent = sublevel * 3;
   commandspace = strlen( com->cmd_name );
   if( commandspace > ( pagewidth - 8 - subindent ) )
      commandspace = pagewidth - 8 - subindent;

   if( commandspace < 1 )
   {
      bprintf( buf, "| COMMAND SPACING PROBLEM\r\n" );
      return;
   }

   if( com->desc_func )
      mud_printf( command_desc, "%s", (*com->desc_func)( extra ) );
   else
      mud_printf( command_desc, "" );

   commanddescspace = pagewidth - 8 - subindent - commandspace;
   add_spaces( command_desc, ( commanddescspace - strlen( smash_color( command_desc ) ) ) );

   bprintf( buf, "| %-*.*s%-3.3s %-*.*s%s |\r\n",
                  subindent, subindent, "   ",
                  symbol,
                  commandspace, commandspace, com->cmd_name,
                  command_desc );

   return;
}

void print_commands( void *extra, LLIST *commands, BUFFER *buf, int sublevel, int pagewidth )
{
   ITERATOR Iter;
   COMMAND *com;

   AttachIterator( &Iter, commands );
   while( ( com = (COMMAND *)NextInList( &Iter ) ) != NULL )
   {
      bprint_commandline( extra, buf, com, sublevel, pagewidth );
      if( com->can_sub && com->sub_commands )
         print_commands( extra, com->sub_commands, buf, ( sublevel + 1 ), pagewidth );
   }
   DetachIterator( &Iter );

   return;
}

char *strip_cr( const char *str )
{
  static char newstr[MAX_BUFFER];
  int i, j;

  if( !str || str[0] == '\0' )
    {
      newstr[0] = '\0';
      return newstr;
    }

  for( i = j = 0; str[i] != '\0'; i++ )
    if( str[i] != '\r' )
      {
        newstr[j++] = str[i];
      }

  newstr[j] = '\0';
  return newstr;
}

char *strip_nl( const char *str )
{
   static char newstr[MAX_BUFFER];
   int i, j;

   if( !str || str[0] == '\0' )
   {
      newstr[0] = '\0';
      return newstr;
   }

   for( i = j = 0; str[i] != '\0'; i++ )
      if( str[i] != '\n' )
         newstr[j++] = str[i];

   newstr[j] = '\0';
   return newstr;
}

const char *handle_pagewidth( int width, const char *txt )
{
   static char buf[MAX_OUTPUT];
   bool color = FALSE;
   char *ptr;
   int x;

   memset( &buf[0], 0, sizeof(buf) );
   ptr = buf;
   x = 0;

   while( *txt != '\0' )
   {
      if( *txt == '#' && !color )
         color = TRUE;
      else if( *txt == '#' && color )
      {
         x++;
         color = FALSE;
      }

      if( *txt != '#' )
      {
         if( color )
            color = FALSE;
         else
            x++;
      }

      if( *txt == '\n' || *txt == '\r' )
         x = 0;

      if( x > width )
      {
         x = 0;
         if( !isspace( *txt ) )
            while( !isspace( *txt ) )
            {
               txt--;
               ptr--;
            }
            txt++;
         *ptr++ = '\n';
         *ptr++ = '\r';
         continue;
      }
      *ptr++ = *txt++;
   }

   buf[strlen(buf)] = '\0';

   return buf;
}

bool is_number( const char *arg )
{
   if( *arg == '\0' )
      return FALSE;

   for( ; *arg != '\0'; arg++ )
   {
      if( !isdigit( *arg ) )
         return FALSE;
   }

   return TRUE;
}

char *smash_color( const char *str )
{
   static char ret[MAX_BUFFER];
   char *retptr;

   memset( &ret[0], 0, sizeof(ret) );
   retptr = ret;

   if(str == NULL)
      return NULL;

   for ( ; *str != '\0'; str++ )
   {
      if (*str == '#' && *(str + 1) != '\0' )
         str++;
      else
      {
         *retptr = *str;

         retptr++;
      }
   }
   *retptr = '\0';
   return ret;
}

char *smash_newline( const char *str )
{
   static char ret[MAX_BUFFER];
   char *retptr;

   memset( &ret[0], 0, sizeof( ret ) );
   retptr = ret;

   if( str == NULL )
      return NULL;

   for( ; *str != '\0'; str++ )
   {
      if( *str == '\n' )
         continue;
      else
      {
         *retptr = *str;
         retptr++;
      }
   }
   *retptr = '\0';
   return ret;
}

int color_count( const char *str )
{
   int count = 0;

   if( str == NULL )
      return 0;

   for( ; *str != '\0'; str++ )
      if( *str == '#' )
         count++;

   return count;
}

void add_spaces( char *str, int amount )
{
   int x;

   for( x = 0; x < amount; x++ )
      strcat( str, " " );

   return;
}

void add_lead_space( char *str, int amount )
{
   char padding[MAX_BUFFER];
   char *pad_ptr;

   memset( &padding, 0, sizeof( padding ) );

   add_spaces( padding, amount );

   if( ( strlen( padding ) + strlen( str ) ) > MAX_BUFFER  - 1 )
   {
      bug( "%s: buffer overflow, not adding lead spaces", __FUNCTION__ );
      return;
   }

   strcat( padding, str );
   pad_ptr = padding;

   while( *pad_ptr != '\0' )
      *str++ = *pad_ptr++;

   str[strlen(str)] = '\0';
   return;
}

char *center_string( const char *to_center, int length )
{
   static char buf[MAX_BUFFER];
   char *buf_ptr;
   int string_size;
   int to_add;
   int extra_space;

   memset( &buf, 0, sizeof( buf ) );

   if( !to_center || to_center[0] == '\0' )
   {
      bug( "%s: attempting to center a blank string.", __FUNCTION__ );
      return buf;
   }

   string_size = strlen( smash_color( to_center ) );

   to_add = ( length - string_size ) / 2;
   extra_space = ( length - string_size ) % 2;

   add_lead_space( buf, to_add );

   buf_ptr = &buf[strlen(buf)];

   while( *to_center != '\0' )
      *buf_ptr++ = *to_center++;

   add_spaces( buf, to_add + extra_space );
   buf[strlen(buf)] = '\0';
   return buf;
}

char *fit_string_to_space( const char *orig, int space )
{
   static char buf[MAX_BUFFER];
   char *buf_ptr;
   int length, to_add, x;

   memset( &buf[0], 0, sizeof( buf ) );

   if( !orig || orig[0] == '\0' )
   {
      bug( "%s: passed a null string.", __FUNCTION__ );
      return buf;
   }

   if( ( length = strlen( smash_color( orig ) ) ) < space )
   {
      to_add = space - length;
      strcat( buf, orig );
      add_spaces( buf, to_add );
      return buf;
   }

   buf_ptr = buf;

   for( x = 0; x < space; )
   {
      if( *orig == '\0' )
         break;

      if( *orig == '#' )
      {
         *buf_ptr++ = *orig++;
         if( *orig == '\0' )
            break;
         *buf_ptr++ = *orig++;
         continue;
      }
      *buf_ptr++ = *orig++;
      x++;
   }
   buf[strlen( buf )] = '\0';
   return buf;
}

char *print_bar( const char *pattern, int width )
{
   static char buf[MAX_BUFFER];
   const char *pat_ptr;
   char *buf_ptr;
   int x;

   memset( &buf[0], 0, sizeof( buf ) );

   if( !pattern || pattern[0] == '\0' )
   {
      bug( "%s: passed a NULL pattern string.", __FUNCTION__ );
      return buf;
   }

   if( ( strlen( pattern ) * width ) > MAX_BUFFER )
   {
      bug( "%s: buffer overflow.", __FUNCTION__ );
      return buf;
   }

   buf_ptr = buf;
   pat_ptr = pattern;

   for( x = 0; x < width; )
   {
      if( *pat_ptr == '\0' )
         pat_ptr = pattern;

      if( *pat_ptr == '#' )
      {
         *buf_ptr++ = *pat_ptr++;
         if( *pat_ptr == '\0' )
         {
            x++;
            continue;
         }
         *buf_ptr++ = *pat_ptr++;
         continue;
      }
      *buf_ptr++ = *pat_ptr++;
      x++;
   }
   buf[strlen(buf)] = '\0';
   return buf;
}

const char *itos( int value )
{
   static char buf[MAX_BUFFER];
   mud_printf( buf, "%d", value );
   return buf;
}

const char *quick_format( const char *format, ... )
{
   static char buf[MAX_BUFFER];
   memset( &buf, 0, sizeof( buf ) );
   va_list va;
   int res;

   va_start( va, format );
   res = vsnprintf( buf, MAX_BUFFER, format, va );
   va_end( va );

   if( res >= MAX_BUFFER -1 )
   {
      buf[0] = '\0';
      bug( "Overflow when printing string %s", format );
   }
   return buf;
}

const char *format_string_for_sql( const char *string )
{
   static char buf[MAX_BUFFER];
   char *buf_ptr;
   memset( &buf[0], 0, sizeof( buf ) );
   buf_ptr = buf;

   while( *string != '\0' )
   {
      if( *string == ';' )
      {
         string++;
         continue;
      }
      if( *string == 39 )
         *buf_ptr++ = 39;
      *buf_ptr++ = *string++;
   }
   buf[strlen(buf)] = '\0';
   return buf;
}

bool string_contains( const char *string, const char *regex_string )
{
   /* Using Regular Expression */
   regex_t regex;
   int reti;

   reti = regcomp(&regex, regex_string, 0 );
   if( reti )
   {
      bug( "%s: bad regex '%s'", __FUNCTION__, regex_string );
      return FALSE;
   }

   if( !(reti = regexec( &regex, string, 0, NULL, 0 ) ) )
      return TRUE;
   else if( reti == REG_NOMATCH )
      return FALSE;
   else
   {
      log_string( "didn't know what to do with: %s", regex_string);
      return FALSE;
   }
   return FALSE;
}

bool grab_range_and_type( char *arg, char *type, int *start, int *end )
{
   char range_start[MAX_BUFFER], range_end[MAX_BUFFER];
   char *rng_start, *rng_end;

   if( !string_contains( arg, "-" ) )
      return FALSE;

   memset( &range_start[0], 0, sizeof( range_start ) );
   memset( &range_end[0], 0, sizeof( range_end ) );
   rng_start = range_start;
   rng_end = range_end;

   rng_end = one_arg_delim( arg, rng_start, '-' );

   if( !is_number( rng_start + 1 ) || !is_number( rng_end ) )
      return FALSE;

   *type = rng_start[0];
   *start = atoi( rng_start + 1 );
   *end = atoi( rng_end );

   return TRUE;

}
