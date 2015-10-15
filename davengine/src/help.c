/*
 * This file contains the dynamic help system.
 * If you wish to update a help file, simply edit
 * the entry in ../help/ and the mud will load the
 * new version next time someone tries to access
 * that help file.
 */
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <dirent.h> 

/* include main header file */
#include "mud.h"

LLIST     *  help_list = NULL;   /* the linked list of help files     */
char     *  greeting;           /* the welcome greeting              */
char     *  motd;               /* the MOTD help file                */

/*
 * Loads all the helpfiles found in ../help/
 */
void load_helps()
{
  HELP_DATA *new_help;
  char buf[MAX_BUFFER];
  char *s;
  DIR *directory;
  struct dirent *entry;

  log_string("Load_helps: getting all help files.");

  help_list = AllocList();

  directory = opendir("../help/");
  for (entry = readdir(directory); entry; entry = readdir(directory))
  {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;

    snprintf(buf, MAX_BUFFER, "../help/%s", entry->d_name);
    s = read_help_entry(buf);

    if (s == NULL)
    {
      bug("load_helps: Helpfile %s does not exist.", buf);
      continue;
    }

    if ((new_help = malloc(sizeof(*new_help))) == NULL)
    {
      bug("Load_helps: Cannot allocate memory.");
      abort();
    }

    new_help->keyword    =  strdup(entry->d_name);
    new_help->text       =  strdup(s);
    new_help->load_time  =  time(NULL);
    AttachToList(new_help, help_list);

    if (!strcasecmp("GREETING", new_help->keyword))
      greeting = new_help->text;
    else if (!strcasecmp("MOTD", new_help->keyword))
      motd = new_help->text;
  }
  closedir(directory);
}

void get_help( D_SOCKET *dsock, char *arg )
{
   const char *helpbody = NULL;
   char buf[MAX_BUFFER];

   arg = one_arg_delim( arg, buf, ' ' );

   if( !arg || arg[0] == '\0' )
   {
      text_to_buffer( dsock, "Help what?\r\n" );
      return;
   }

   if( ( helpbody = grab_help_from_wiki( arg ) ) != NULL )
   {
      text_to_buffer( dsock, print_bar( "-", dsock->account ? dsock->account->pagewidth : 80 ) );
      text_to_buffer( dsock, "\r\n" );
      text_to_buffer( dsock, helpbody );
      text_to_buffer( dsock, "\r\n" );
      text_to_buffer( dsock, print_bar( "-", dsock->account ? dsock->account->pagewidth : 80 ) );
      text_to_buffer( dsock, "\r\n" );

      return;
   }
   else
   {
      text_to_buffer( dsock, "No such help file.\r\n" );
      return;
   }

}

const char *grab_help_from_wiki( const char *name )
{
   MYSQL_RES *result;
   MYSQL_ROW row;
   static char buf[MAX_BUFFER];
   char query[MAX_BUFFER];
   int old_id;

   if( help_handle == NULL )
   {
      bug( "%s: has found that the HELP handle is NULL.", __FUNCTION__ );
      return NULL;
   }

   mud_printf( query, "SELECT page_latest FROM `page` WHERE UPPER( page_title ) = UPPER( '%s' );", format_string_for_sql( name ) );

   if( mysql_query( help_handle, query ) )
   {
      report_sql_error( help_handle );
      return NULL;
   }
   if( ( result = mysql_store_result( help_handle ) ) == NULL )
      return NULL;
   if( mysql_num_rows( result ) == 0 )
      return NULL;

   row = mysql_fetch_row( result );
   old_id = atoi( row[0] );
   mysql_free_result( result );

   mud_printf( query, "SELECT old_text FROM `text` WHERE old_id=%d;", old_id );

   if( mysql_query( help_handle, query ) )
   {
      report_sql_error( help_handle );
      return NULL;
   }
   if( ( result = mysql_store_result( help_handle ) ) == NULL )
      return NULL;
   if( mysql_num_rows( result ) == 0 )
      return NULL;

   row = mysql_fetch_row( result );
   mud_printf( buf, "%s", row[0] );
   mysql_free_result( result );

   buf[strlen( buf )] = '\0';

   return buf;
}
