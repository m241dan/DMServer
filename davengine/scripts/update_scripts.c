#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/types.h>
#include <ctype.h>
#include <pthread.h>
#include <regex.h>

#define MAX_BUFFER 16384
#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif
typedef  unsigned char     bool;

typedef enum
{
   TEMP_FUNC, SCRIPT_FUNC, PREFIX_TEST
} WRITE;

typedef struct lua_function
{
   char *noise;
   char *header;
   char *body;
   bool wrote;
} LUA_FUNCTION;

typedef struct lua_function ** LUA_FUNCTION_ARRAY;

char *fread_file( FILE *fp );
char *one_arg_delim( char *fStr, char *bStr, char delim );
bool until_function( char *str );
bool until_end( char *str );
void *update_frameworks( void *arg );
void *update_instances( void *arg );
void *update_stats( void *arg );
bool is_prefix(const char *aStr, const char *bStr);
bool string_contains( const char *string, const char *regex_string );
int count_lua_functions( char *str );
LUA_FUNCTION_ARRAY get_functions( char *str );
LUA_FUNCTION *get_lua_func( char **str );
void free_func( LUA_FUNCTION *func );
void free_func_array( LUA_FUNCTION_ARRAY );
void print_two_strings_no_copy_by_line( FILE *script_fp, char *script_body, char *template_body );

pthread_t tid[3];

int main( void )
{
/*
   pthread_create( &(tid[0] ), NULL, &update_frameworks, NULL );
   pthread_create( &(tid[1] ), NULL, &update_instances, NULL );
   pthread_create( &(tid[2] ), NULL, &update_stats, NULL );

   pthread_join( tid[0], NULL );
   pthread_join( tid[1], NULL );
   pthread_join( tid[2], NULL );
*/
   update_frameworks( NULL );
   update_instances( NULL );
   update_stats( NULL );
   return 0;
}

void *update_frameworks( void *arg )
{
   DIR *directory;
   struct dirent *entry;
   FILE *template_fp;
   FILE *script_fp;
   char template_buf[MAX_BUFFER];
   char script_buf[MAX_BUFFER];
   WRITE write;
   LUA_FUNCTION_ARRAY script_funcs, template_funcs;
   int x, y;

   if( ( template_fp = fopen( "templates/frame.lua", "r" ) ) == NULL )
      return NULL;
   memset( &template_buf[0], 0, sizeof( template_buf ) );
   strcat( template_buf, fread_file( template_fp ) );
   fclose( template_fp );

   if( ( template_funcs = get_functions( template_buf ) ) == NULL )
   {
      printf( "%s: could not get the functions from the template.", __FUNCTION__ );
      return NULL;
   }

   directory = opendir( "frames/" );
   for( entry = readdir( directory ); entry; entry = readdir( directory ) )
   {
      char buf[255], line_one[510], line_two[510];
      if( !strcmp( entry->d_name, "." ) || !strcmp( entry->d_name, ".." ) || !strcmp( entry->d_name, ".placeholder" ) )
         continue;
      snprintf( buf, 255, "frames/%s", entry->d_name );
      if( ( script_fp = fopen( buf, "r" ) ) == NULL )
         continue;

      memset( &script_buf[0], 0, sizeof( script_buf ) );
      strcat( script_buf, fread_file( script_fp ) );
      fclose( script_fp );

      if( ( script_funcs = get_functions( script_buf ) ) == NULL )
      {
         printf( "%s: could not get the functions from script %s.", __FUNCTION__, entry->d_name );
         continue;
      }

      if( ( script_fp = fopen( buf, "w" ) ) == NULL )
         continue;

      for( x = 0; template_funcs[x] != NULL; x++ )
      {
         for( y = 0; script_funcs[y] != NULL; y++ )
            if( is_prefix( template_funcs[x]->header, script_funcs[y]->header ) )
            {
               script_funcs[y]->wrote = TRUE;
               break;
            }

         if( script_funcs[y] )
         {
             print_two_strings_no_copy_by_line( script_fp, script_funcs[y]->noise, template_funcs[x]->noise );
             fprintf( script_fp, "%s", template_funcs[x]->header );
             print_two_strings_no_copy_by_line( script_fp, script_funcs[y]->body, template_funcs[x]->body );
         }
         else
         {
            if( template_funcs[x]->noise[0] != '\0' )
               fprintf( script_fp, "%s", template_funcs[x]->noise );
            fprintf( script_fp, "%s", template_funcs[x]->header );
            if( template_funcs[x]->body[0] != '\0' )
               fprintf( script_fp, "%s", template_funcs[x]->body );
         }
         fprintf( script_fp, "%s\n\n", "end" );
      }

      for( y = 0; script_funcs[y] != NULL; y++ )
      {
         if( script_funcs[y]->wrote )
            continue;
         if( script_funcs[y]->noise[0] != '\0' )
            fprintf( script_fp, "%s", script_funcs[y]->noise );
         fprintf( script_fp, "%s", script_funcs[y]->header );
         if( script_funcs[y]->body[0] != '\0' )
            fprintf( script_fp, "%s", script_funcs[y]->body );
         fprintf( script_fp, "%s\n\n", "end" );
      }
      free_func_array( script_funcs );
      fclose( script_fp );
   }
   free_func_array( template_funcs );
   closedir( directory );
   return NULL;
}

void *update_instances( void *arg )
{
   DIR *directory;
   struct dirent *entry;
   FILE *template_fp;
   FILE *script_fp;
   char template_buf[MAX_BUFFER];
   char script_buf[MAX_BUFFER];
   WRITE write;
   LUA_FUNCTION_ARRAY script_funcs, template_funcs;
   int x, y;

   if( ( template_fp = fopen( "templates/instance.lua", "r" ) ) == NULL )
      return NULL;
   memset( &template_buf[0], 0, sizeof( template_buf ) );
   strcat( template_buf, fread_file( template_fp ) );
   fclose( template_fp );

   if( ( template_funcs = get_functions( template_buf ) ) == NULL )
   {
      printf( "%s: could not get the functions from the template.", __FUNCTION__ );
      return NULL;
   }

   directory = opendir( "instances/" );
   for( entry = readdir( directory ); entry; entry = readdir( directory ) )
   {
      char buf[255], line_one[510], line_two[510];
      if( !strcmp( entry->d_name, "." ) || !strcmp( entry->d_name, ".." ) || !strcmp( entry->d_name, ".placeholder" ) )
         continue;
      snprintf( buf, 255, "instances/%s", entry->d_name );
      if( ( script_fp = fopen( buf, "r" ) ) == NULL )
         continue;

      memset( &script_buf[0], 0, sizeof( script_buf ) );
      strcat( script_buf, fread_file( script_fp ) );
      fclose( script_fp );

      if( ( script_funcs = get_functions( script_buf ) ) == NULL )
      {
         printf( "%s: could not get the functions from script %s.", __FUNCTION__, entry->d_name );
         continue;
      }

      if( ( script_fp = fopen( buf, "w" ) ) == NULL )
         continue;

      for( x = 0; template_funcs[x] != NULL; x++ )
      {
         for( y = 0; script_funcs[y] != NULL; y++ )
            if( is_prefix( template_funcs[x]->header, script_funcs[y]->header ) )
            {
               script_funcs[y]->wrote = TRUE;
               break;
            }

         if( script_funcs[y] )
         {
             print_two_strings_no_copy_by_line( script_fp, script_funcs[y]->noise, template_funcs[x]->noise );
             fprintf( script_fp, "%s", template_funcs[x]->header );
             print_two_strings_no_copy_by_line( script_fp, script_funcs[y]->body, template_funcs[x]->body );
         }
         else
         {
            if( template_funcs[x]->noise[0] != '\0' )
               fprintf( script_fp, "%s", template_funcs[x]->noise );
            fprintf( script_fp, "%s", template_funcs[x]->header );
            if( template_funcs[x]->body[0] != '\0' )
               fprintf( script_fp, "%s", template_funcs[x]->body );
         }
         fprintf( script_fp, "%s\n\n", "end" );
      }
      for( y = 0; script_funcs[y] != NULL; y++ )
      {
         if( script_funcs[y]->wrote )
            continue;
         if( script_funcs[y]->noise[0] != '\0' )
            fprintf( script_fp, "%s", script_funcs[y]->noise );
         fprintf( script_fp, "%s", script_funcs[y]->header );
         if( script_funcs[y]->body[0] != '\0' )
            fprintf( script_fp, "%s", script_funcs[y]->body );
         fprintf( script_fp, "%s\n\n", "end" );
      }
      free_func_array( script_funcs );
      fclose( script_fp );
   }
   free_func_array( template_funcs );
   closedir( directory );

   return NULL;
}

void *update_stats( void *arg )
{
   DIR *directory;
   struct dirent *entry;
   FILE *template_fp;
   FILE *script_fp;
   char template_buf[MAX_BUFFER];
   char script_buf[MAX_BUFFER];
   WRITE write;
   LUA_FUNCTION_ARRAY script_funcs, template_funcs;
   int x, y;

   if( ( template_fp = fopen( "templates/stat.lua", "r" ) ) == NULL )
      return NULL;
   memset( &template_buf[0], 0, sizeof( template_buf ) );
   strcat( template_buf, fread_file( template_fp ) );
   fclose( template_fp );

   if( ( template_funcs = get_functions( template_buf ) ) == NULL )
   {
      printf( "%s: could not get the functions from the template.", __FUNCTION__ );
      return NULL;
   }

   directory = opendir( "stats/" );
   for( entry = readdir( directory ); entry; entry = readdir( directory ) )
   {
      char buf[255], line_one[510], line_two[510];
      if( !strcmp( entry->d_name, "." ) || !strcmp( entry->d_name, ".." ) || !strcmp( entry->d_name, ".placeholder" ) )
         continue;
      snprintf( buf, 255, "stats/%s", entry->d_name );
      if( ( script_fp = fopen( buf, "r" ) ) == NULL )
         continue;

      memset( &script_buf[0], 0, sizeof( script_buf ) );
      strcat( script_buf, fread_file( script_fp ) );
      fclose( script_fp );

      if( ( script_funcs = get_functions( script_buf ) ) == NULL )
      {
         printf( "%s: could not get the functions from script %s.", __FUNCTION__, entry->d_name );
         continue;
      }

      if( ( script_fp = fopen( buf, "w" ) ) == NULL )
         continue;

      for( x = 0; template_funcs[x] != NULL; x++ )
      {
         for( y = 0; script_funcs[y] != NULL; y++ )
            if( is_prefix( template_funcs[x]->header, script_funcs[y]->header ) )
            {
               script_funcs[y]->wrote = TRUE;
               break;
            }

         if( script_funcs[y] )
         {
             print_two_strings_no_copy_by_line( script_fp, script_funcs[y]->noise, template_funcs[x]->noise );
             fprintf( script_fp, "%s", template_funcs[x]->header );
             print_two_strings_no_copy_by_line( script_fp, script_funcs[y]->body, template_funcs[x]->body );
         }
         else
         {
            if( template_funcs[x]->noise[0] != '\0' )
               fprintf( script_fp, "%s", template_funcs[x]->noise );
            fprintf( script_fp, "%s", template_funcs[x]->header );
            if( template_funcs[x]->body[0] != '\0' )
               fprintf( script_fp, "%s", template_funcs[x]->body );
         }
         fprintf( script_fp, "%s\n\n", "end" );
      }
      for( y = 0; script_funcs[y] != NULL; y++ )
      {
         if( script_funcs[y]->wrote )
            continue;
         if( script_funcs[y]->noise[0] != '\0' )
            fprintf( script_fp, "%s", script_funcs[y]->noise );
         fprintf( script_fp, "%s", script_funcs[y]->header );
         if( script_funcs[y]->body[0] != '\0' )
            fprintf( script_fp, "%s", script_funcs[y]->body );
         fprintf( script_fp, "%s\n\n", "end" );
      }
      free_func_array( script_funcs );
      fclose( script_fp );
   }
   free_func_array( template_funcs );
   closedir( directory );

   return NULL;
}

bool until_function( char *str )
{
   if( is_prefix( str, "function" ) || str[0] == '\0' )
      return TRUE;
   return FALSE;
}

bool until_end( char *str )
{
   if( is_prefix( str, "end" ) || str[0] == '\0' )
      return TRUE;
   return FALSE;
}

char *fread_file( FILE *fp )
{
   static char buf[MAX_BUFFER];
   int c, count = 0;

   memset( &buf[0], 0, sizeof( buf ) );

   while( ( c = getc( fp ) ) != EOF )
   {
      buf[count++] = c;
      if( count >= ( MAX_BUFFER - 1 ) )
         break;
   }

   buf[strlen( buf )] = '\0';
   return buf;

}
char *one_arg_delim( char *fStr, char *bStr, char delim )
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

bool string_contains( const char *string, const char *regex_string )
{
   /* Using Regular Expression */
   regex_t regex;
   int reti;

   reti = regcomp(&regex, regex_string, 0 );
   if( reti )
   {
      printf( "%s: bad regex '%s'", __FUNCTION__, regex_string );
      return FALSE;
   }

   if( !(reti = regexec( &regex, string, 0, NULL, 0 ) ) )
      return TRUE;
   else if( reti == REG_NOMATCH )
      return FALSE;
   else
   {
      printf( "didn't know what to do with: %s", regex_string);
      return FALSE;
   }
   return FALSE;
}

int count_lua_functions( char *str )
{
   char buf[510];
   int count = 0;

   while( str && str[0] != '\0' )
   {
      str = one_arg_delim( str, buf, '\n' );
      if( is_prefix( buf, "function" ) )
         count++;
   }
   return count;
}

LUA_FUNCTION_ARRAY get_functions( char *str )
{
   LUA_FUNCTION_ARRAY func_array;
   char buf[510];
   int size, x;

   size = count_lua_functions( str );
   if( size == 0 )
   {
      printf( "%s: no lua functions in str:\n%s\n\n", __FUNCTION__, str );
      return NULL;
   }

   func_array = (LUA_FUNCTION_ARRAY)calloc( size + 1, sizeof( LUA_FUNCTION * ) );
   func_array[size] = NULL;

   for( x = 0; x < size; x++ )
      func_array[x] = get_lua_func( &str );

   return func_array;
}

LUA_FUNCTION *get_lua_func( char **str )
{
   typedef enum
   {
      NOISE, HEADER, BODY
   } MODE;

   LUA_FUNCTION *func;
   char *box = *str;
   char line[510], noise[MAX_BUFFER], header[MAX_BUFFER], body[MAX_BUFFER];
   MODE mode = NOISE;

   func = (LUA_FUNCTION *)malloc( sizeof( LUA_FUNCTION ) );
   func->noise = NULL;
   func->header = NULL;
   func->body = NULL;
   func->wrote = FALSE;

   memset( &noise[0], 0, sizeof( noise ) );
   memset( &header[0], 0, sizeof( header ) );
   memset( &body[0], 0, sizeof( body ) );

   while( box && box[0] != '\0' )
   {
      box = one_arg_delim( box, line, '\n' );
      if( line[0] == '\0' )
         continue;
      switch( mode )
      {
         case NOISE:
            if( !until_function( line ) )
            {
               strcat( noise, line );
               strcat( noise, "\n" );
               break;
            }
            mode = HEADER;
         case HEADER:
            strcat( header, line );
            strcat( header, "\n" );
            mode = BODY;
            break;
         case BODY:
            if( until_end( line ) )
            {
               box = one_arg_delim( box, line, '\n' );
               goto exit;
            }
            strcat( body, line );
            strcat( body, "\n" );
            break;
      }
   }
   exit:
   noise[strlen( noise )] = '\0';
   header[strlen( header )] = '\0';
   body[strlen( body )] = '\0';
   func->noise = strdup( noise );
   func->header = strdup( header );
   func->body = strdup( body );
   *str = box;
   return func;

}

void free_func( LUA_FUNCTION *func )
{
   free( func->noise );
   free( func->header );
   free( func->body );
   free( func );
}

void free_func_array( LUA_FUNCTION_ARRAY func_array )
{
   int x;

   for( x = 0; func_array[x] != NULL; x++ )
      free_func( func_array[x] );
   free( func_array );
}

void print_two_strings_no_copy_by_line( FILE *script_fp, char *script_body, char *template_body )
{
   char template_line[MAX_BUFFER];

   while( template_body && template_body[0] != '\0' )
   {
      template_body = one_arg_delim( template_body, template_line, '\n' );
      if( string_contains( script_body, template_line ) )
         continue;
      fprintf( script_fp, "%s\n", template_line );
   }
   if( script_body && script_body[0] != '\0' )
      fprintf( script_fp, "%s", script_body );
   return;
}
