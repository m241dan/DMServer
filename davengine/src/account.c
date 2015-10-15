/* account.c: methods pertaining to accounts written by Davenge */

#include "mud.h"

/* creation */

ACCOUNT_DATA *init_account( void )
{
   ACCOUNT_DATA *account;

   CREATE( account, ACCOUNT_DATA, 1 );
   account->idtag = init_tag();
   account->idtag->type = ACCOUNT_IDS;
   account->characters = AllocList();
   account->command_tables = AllocList();
   account->commands = AllocList();
   if( clear_account( account ) != RET_SUCCESS )
   {
      free_account( account );
      return NULL;
   }
   return account;
}

int clear_account( ACCOUNT_DATA *account )
{
   int ret = RET_SUCCESS;

   account->socket = NULL;
   FREE( account->name );
   account->name = strdup( "new_account" );
   FREE( account->password );
   account->password = strdup( "new_password" );
   account->level = 1;
   account->pagewidth = DEFAULT_PAGEWIDTH;
   FREE( account->last_command );
   account->last_command = strdup( "none" );
   account->executing_command = NULL;
   FREE( account->chatting_as );
   account->chatting_as = strdup( " " );

   /* this needs to eventually clear any lists */

   return ret;
}

CHAR_SHEET *make_character_sheet( ENTITY_INSTANCE *instance )
{
   CHAR_SHEET *sheet;

   CREATE( sheet, CHAR_SHEET, 1 );
   sheet->name = strdup( instance_name( instance ) );
   sheet->id = instance->tag->id;
   return sheet;
}

/* deletion */
int free_account( ACCOUNT_DATA *account )
{
   int ret = RET_SUCCESS;

   if( account->idtag )
      free_tag( account->idtag );
  account->idtag = NULL;

   if( account->olc )
      free_olc( account->olc );
   account->olc = NULL;

   clearlist( account->characters );
   FreeList( account->characters );
   account->characters = NULL;

   free_command_list( account->commands );
   FreeList( account->commands );
   account->commands = NULL;

   account->socket = NULL;
   FREE( account->name );
   FREE( account->password );
   FREE( account->command_tables );
   FREE( account->commands );
   FREE( account->last_command );
   FREE( account );

   return ret;
}

/* i/o */
/* this could be factored into the load by query style */
int load_account( ACCOUNT_DATA *account, const char *name )
{
   MYSQL_RES *result;
   MYSQL_ROW row;
   int num_row;

   if( !quick_query( "SELECT * FROM accounts WHERE name='%s';", name ) )
      return RET_FAILED_OTHER;

   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
   {
      report_sql_error( sql_handle );
      return RET_NO_SQL;
   }

   num_row = mysql_num_rows( result );
   if( num_row > 1 )
   {
      bug( "%s: Query for account %s turned up more than one entry...", __FUNCTION__, name );
      mysql_free_result( result );
      return RET_FAILED_OTHER;
   }
   else if( num_row == 0 )
   {
      mysql_free_result( result );
      return RET_DB_NO_ENTRY;
   }

   row = mysql_fetch_row( result );
   db_load_account( account, &row );
   load_character_sheets( account );

   mysql_free_result( result );
   return RET_SUCCESS;
}

void db_load_account( ACCOUNT_DATA *account, MYSQL_ROW *row )
{
   int counter;

   counter = db_load_tag( account->idtag, row );
   account->name = strdup( (*row)[counter++] );
   account->password = strdup( (*row)[counter++] );
   account->level = atoi( (*row)[counter++] );
   account->pagewidth = atoi( (*row)[counter++] );
   account->chatting_as = strdup( (*row)[counter++] );
}

void load_character_sheets( ACCOUNT_DATA *account )
{
   CHAR_SHEET *sheet;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT entityInstanceID, name from `account_characters` WHERE accountID=%d LIMIT 3;", account->idtag->id ) ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      CREATE( sheet, CHAR_SHEET, 1 );
      sheet->id = atoi( row[0] );
      sheet->name = strdup( row[1] );
      AttachToList( sheet, account->characters );
   }
   DetachIterator( &Iter );
   FreeList( list );
   return;
}

int new_account( ACCOUNT_DATA *account )
{
   int ret = RET_SUCCESS;

   if( !account )
   {
      BAD_POINTER( "account" );
      return ret;
   }

   if( ( ret = new_tag( account->idtag, "system" ) ) != RET_SUCCESS )
   {
      bug( "%s: could not set parameters for a new ID tag.", __FUNCTION__ );
      return ret;
   }

   if( !quick_query( "INSERT INTO accounts VALUES( %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, '%s' );",
              account->idtag->id, account->idtag->type, account->idtag->created_by,
              account->idtag->created_on, account->idtag->modified_by, account->idtag->modified_on,
              account->name, account->password, account->level, account->pagewidth, account->chatting_as ) )
      return RET_FAILED_OTHER;

   return ret;
}

int account_prompt( D_SOCKET *dsock )
{
   BUFFER *buf = buffer_new(MAX_BUFFER);
   int width = dsock->account->pagewidth;

   bprintf( buf, "/%s\\\r\n", print_header( "Account Menu", "-", width - 2 ) );
   print_commands( dsock->account, dsock->account->commands, buf, 0, dsock->account->pagewidth );
   bprintf( buf, "\\%s/\r\n", print_header( "End Menu", "-", width - 2 ) );
   bprintf( buf, "What is your choice?: " );

   text_to_buffer( dsock, buf->data );
   buffer_free( buf );
   return RET_SUCCESS; 
}

/* utility */
ACCOUNT_DATA *get_active_account_by_id( int id )
{
   ACCOUNT_DATA *account;
   ITERATOR Iter;

   if( SizeOfList( account_list ) < 1 )
      return NULL;
   AttachIterator( &Iter, account_list );
   while( ( account = (ACCOUNT_DATA *)NextInList( &Iter ) ) != NULL )
      if( account->idtag->id == id )
         break;
   DetachIterator( &Iter );

   return account;
}

ACCOUNT_DATA *get_active_account_by_name( const char *name )
{
   ACCOUNT_DATA *account;
   ITERATOR Iter;

   if( SizeOfList( account_list ) < 1 )
      return NULL;
   AttachIterator( &Iter, account_list );
   while( ( account = (ACCOUNT_DATA *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( account->name, name ) )
         break;
   DetachIterator( &Iter );

   return account;
}

ACCOUNT_DATA *check_account_reconnect(const char *act_name)
{
  ACCOUNT_DATA *account;
  ITERATOR Iter;

  AttachIterator(&Iter, account_list);
  while ((account = (ACCOUNT_DATA *) NextInList(&Iter)) != NULL)
  {
    if (!strcasecmp(account->name, act_name))
    {
      if (account->socket)
        close_socket(account->socket, TRUE);

      break;
    }
  }
  DetachIterator(&Iter);

  return account;
}

int text_to_account( ACCOUNT_DATA *account, const char *fmt, ... )
{
   va_list va;
   int res;
   char dest[MAX_BUFFER];

   va_start( va, fmt );
   res = vsnprintf( dest, MAX_BUFFER, fmt, va );
   va_end( va );

   if( res >= MAX_BUFFER -1 )
   {
      dest[0] = '\0';
      bug( "Overflow when attempting to format string for message." );
   }

   text_to_buffer( account->socket, dest );
   return res;
}


void account_quit( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;

   if( account->socket->state != STATE_ACCOUNT )
      return;

   DetachFromList( account, account_list );

   text_to_socket( account->socket, "Quitting...\r\n" );
   close_socket( account->socket, FALSE );
   return;
}

void account_settings( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;
   COMMAND *settings_command;

   if( ( settings_command = account->executing_command ) == NULL )
      return;

   if( settings_command->sub_commands )
   {
      free_command_list( settings_command->sub_commands );
      FreeList( settings_command->sub_commands );
      settings_command->sub_commands = NULL;
      text_to_account( account, "Settings Closed.\r\n" );
   }
   else
   {
      settings_command->sub_commands = AllocList();
      load_commands( settings_command->sub_commands, settings_sub_commands, account->level );
      text_to_account( account, "Settings Opened.\r\n" );
   }
   return;
}

void set_pagewidth( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;
   int value;

   if( !account )
   {
      bug( "%s: Account passed is a bad pointer.", __FUNCTION__ );
      return;
   }

   if( !arg || arg[0] == '\0' )
   {
      text_to_account( account, "Current Pagewidth: %d\r\n", account->pagewidth );
      return;
   }

   if( !is_number( arg ) )
   {
      text_to_account( account, "Pagewidth can only take a number.\r\n" );
      return;
   }

   if( ( value = atoi( arg ) ) < 40 )
   {
      text_to_account( account, "Pagewidths have an absolute minimum of 40, for sanity reasons.\r\n" );
      return;
   }

   if( value % 2 != 0 )
   {
      text_to_account( account, "Pagewidths must be an even number, it's just easier this way.\r\n" );
      return;
   }

   set_account_pagewidth( account, value );
   update_tag( account->idtag, "%s-pagewidthCommand", account->name );

   text_to_account( account, "Pagewidth set to %d.\r\n", value );
   return;
}

void account_chat( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;

   if( !arg || arg[0] == '\0' )
   {
      text_to_account( account, "Chat what?\r\n" );
      return;
   }

   communicate( CHAT_LEVEL, account, arg );
   account->socket->bust_prompt = NO_PROMPT;
   text_to_account( account, ":> " );
   return;
}

void account_chatas( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;

   if( !arg || arg[0] == '\0' )
   {
      text_to_account( account, "You must input something.\r\n" );
      return;
   }

   while( isspace( arg[0] ) )
      arg++;

   set_account_chatas( account, arg );
   text_to_account( account, "New name set.\r\n" );
   return;
}

/* setters */
inline void set_account_pagewidth( ACCOUNT_DATA *account, int pagewidth )
{
   account->pagewidth = pagewidth;
   if( !strcmp( account->idtag->created_by, "null" ) )
      return;
   if( !quick_query( "UPDATE `accounts` SET pagewidth='%d' WHERE accountID='%d';", pagewidth, account->idtag->id ) )
      bug( "%s: unable to update database for account %s with new pagewidth.", __FUNCTION__, account->name );
}

inline void set_account_chatas( ACCOUNT_DATA *account, const char *chatas )
{
   account->chatting_as = strdup( chatas );
   if( !strcmp( account->idtag->created_by, "null" ) )
      return;
   if( !quick_query( "UPDATE `accounts` SET chatting_as='%s' WHERE accountID='%d';", account->chatting_as, account->idtag->id ) )
      bug( "%s: unable to update data for account %s with new chatting_as.", __FUNCTION__, account->name );
}

/* actions */
inline void add_character_to_account( ENTITY_INSTANCE *character, ACCOUNT_DATA *account )
{
   CHAR_SHEET *sheet;
   sheet = make_character_sheet( character );
   AttachToList( sheet, account->characters );
   if( !quick_query( "INSERT INTO `account_characters` VALUES ( '%d', '%d', '%s' );", account->idtag->id, sheet->id, sheet->name ) )
      bug( "%s: could not add character with id %d to account with id %d.", __FUNCTION__, character->tag->id, account->idtag->id );
}

void account_control_entity( ACCOUNT_DATA *account, ENTITY_INSTANCE *entity )
{
   if( account->controlling )
      account_uncontrol_entity( account->controlling );
   account->controlling = entity;
   entity->account = account;
}

void account_uncontrol_entity( ENTITY_INSTANCE *entity )
{
   ACCOUNT_DATA *account;
   if( ( account = entity->account ) == NULL )
      return;

   account->controlling = NULL;
   entity->account = NULL;
}
