/* olc.c: methods pertainingto the OLC written by Davenge */

#include "mud.h"

INCEPTION *init_olc( void )
{
   INCEPTION *olc;

   CREATE( olc, INCEPTION, 1 );
   olc->commands = AllocList();
   olc->wSpaces = AllocList();
   olc->using_filter = init_wfilter();
   olc->chain = AllocList();
   if( clear_olc( olc ) != RET_SUCCESS )
   {
      free_olc( olc );
      return NULL;
   }
   return olc;
}

int clear_olc( INCEPTION *olc )
{
   int ret = RET_SUCCESS;
   olc->project = NULL;
   olc->using_workspace = NULL;
   olc->editing = NULL;
   if( olc->editor_commands )
   {
      free_command_list( olc->editor_commands );
      olc->editor_commands = NULL;
   }
   return ret;
}

int free_olc( INCEPTION *olc )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;
   int ret = RET_SUCCESS;

   AttachIterator( &Iter, olc->wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      unuse_workspace( wSpace, olc->account );
   DetachIterator( &Iter );

   free_editor_chain( olc->chain );
   olc->chain = NULL;

   clearlist( olc->wSpaces );
   FreeList( olc->wSpaces );
   olc->wSpaces = NULL;

   free_command_list( olc->commands );
   FreeList( olc->commands );
   olc->commands = NULL;

   free_wfilter( olc->using_filter );
   olc->using_filter = NULL;

   olc->account = NULL;
   olc->using_workspace = NULL;

   if( olc->editor_commands )
   {
      free_command_list( olc->editor_commands );
      olc->editor_commands = NULL;
   }
   FREE( olc );

   return ret;
}

WORKSPACE *init_workspace( void )
{
   WORKSPACE *wSpace;

   CREATE( wSpace, WORKSPACE, 1 );
   wSpace->tag = init_tag();
   wSpace->tag->type = WORKSPACE_IDS;
   wSpace->frameworks = AllocList();
   wSpace->instances = AllocList();
   wSpace->who_using = AllocList();
   if( clear_workspace( wSpace ) != RET_SUCCESS )
   {
      free_workspace( wSpace );
      return NULL;
   }
   return wSpace;

}

int clear_workspace( WORKSPACE *wSpace )
{
   int ret = RET_SUCCESS;

   FREE( wSpace->name );
   wSpace->name = strdup( "new workspace" );
   FREE( wSpace->description );
   wSpace->description = strdup( "blank description" );
   wSpace->Public = FALSE;

   /* this needs to eventually clear lists */

   return ret;
}

int free_workspace( WORKSPACE *wSpace )
{
   int ret = RET_SUCCESS;

   if( wSpace->tag )
      free_tag( wSpace->tag );

   clearlist( wSpace->frameworks );
   FreeList( wSpace->frameworks );
   wSpace->frameworks = NULL;

   clearlist( wSpace->instances );
   FreeList( wSpace->instances );
   wSpace->instances = NULL;

   clearlist( wSpace->who_using );
   FreeList( wSpace->who_using );
   wSpace->who_using = NULL;


   FREE( wSpace->name );
   FREE( wSpace->description );
   FREE( wSpace );

   return ret;
}

WORKSPACE_FILTER *init_wfilter( void )
{
   WORKSPACE_FILTER *filter;

   CREATE( filter, WORKSPACE_FILTER, 1 );
   CREATE( filter->spec_filters, GRAB_PARAMS, 1 );
   filter->limit = 10;
   return filter;
}

int free_wfilter( WORKSPACE_FILTER *filter )
{
   int x, ret = RET_SUCCESS;

   if( filter->name_count > 0 )
   {
      for( x = 0; x < filter->name_count; x++ )
        FREE( filter->filter_name[x] );
      FREE( filter->filter_name );
   }

   if( filter->short_count > 0 )
   {
      for( x = 0; x < filter->short_count; x++ )
         FREE( filter->filter_short[x] );
      FREE( filter->filter_short );
   }

   if( filter->long_count > 0 )
   {
      for( x = 0; x < filter->long_count; x++ )
         FREE( filter->filter_long[x] );
      FREE( filter->filter_long );
   }

   if( filter->desc_count > 0 )
   {
      for( x = 0; x < filter->desc_count; x++ )
         FREE( filter->filter_desc[x] );
      FREE( filter->filter_desc );
   }

   FREE( filter->spec_filters );

   return ret;
}

WORKSPACE_FILTER *reset_wfilter( WORKSPACE_FILTER *filter )
{
   free_wfilter( filter );
   filter = init_wfilter();
   return filter;
}

E_CHAIN *make_editor_chain_link( void *editing, int state )
{
   E_CHAIN *link;

   CREATE( link, E_CHAIN, 1 );
   link->to_edit = editing;
   link->state = state;
   return link;
}

void free_editor_chain( LLIST *list )
{
   E_CHAIN *link;
   ITERATOR Iter;

   AttachIterator( &Iter, list );
   while( ( link = (E_CHAIN *)NextInList( &Iter ) ) != NULL )
   {
      link->to_edit = NULL;
      free( link );
   }
   FreeList( list );
   return;
}

void free_link( E_CHAIN *link )
{
   link->to_edit = NULL;
   FREE( link );
}

void add_link_to_chain( E_CHAIN *link, LLIST *chain )
{
   AttachToList( link, chain );
   return;
}

WORKSPACE *load_workspace_by_query( const char *query )
{
   WORKSPACE *wSpace = NULL;
   MYSQL_ROW row;


   if( ( row = db_query_single_row( query ) ) == NULL )
      return NULL;

   if( ( wSpace = init_workspace() ) == NULL )
      return NULL;

   db_load_workspace( wSpace, &row );
   AttachToList( wSpace, active_wSpaces );
   load_workspace_entries( wSpace );
   free( row );
   return wSpace;
}

WORKSPACE *get_workspace_by_id( int id )
{
   WORKSPACE *wSpace;

   if( ( wSpace = get_active_workspace_by_id( id ) ) == NULL )
      wSpace = load_workspace_by_id( id );

   return wSpace;
}

WORKSPACE *get_active_workspace_by_id( int id )
{
   return workspace_list_has_by_id( active_wSpaces, id );
}

WORKSPACE *load_workspace_by_id( int id )
{
   return load_workspace_by_query( quick_format( "SELECT * FROM `%s` WHERE %s=%d;", tag_table_strings[WORKSPACE_IDS], tag_table_whereID[WORKSPACE_IDS], id ) );
}

WORKSPACE *get_workspace_by_name( const char *name )
{
   WORKSPACE *wSpace;

   if( ( wSpace = get_active_workspace_by_name( name ) ) == NULL )
      wSpace = load_workspace_by_name( name );

   return wSpace;
}

WORKSPACE *get_active_workspace_by_name( const char *name )
{
   return workspace_list_has_by_name( active_wSpaces, name );
}

WORKSPACE *load_workspace_by_name( const char *name )
{
   return load_workspace_by_query( quick_format( "SELECT * FROM `%s` WHERE name='%s' LIMIT 1;", tag_table_strings[WORKSPACE_IDS], name ) );
}

void db_load_workspace( WORKSPACE *wSpace, MYSQL_ROW *row )
{
   int counter;

   counter = db_load_tag( wSpace->tag, row );

   wSpace->name = strdup( (*row)[counter++] );
   wSpace->description = strdup( (*row)[counter++] );
   wSpace->Public = (bool)(atoi( (*row)[counter++] ));
}

void unuse_workspace( WORKSPACE *wSpace, ACCOUNT_DATA *account )
{
   if( account->olc->using_workspace == wSpace )
      account->olc->using_workspace = NULL;
   DetachFromList( account, wSpace->who_using );
   if( SizeOfList( wSpace->who_using ) < 1 )
   {
      DetachFromList( wSpace, active_wSpaces );
      free_workspace( wSpace );
   }
   else
   {
      ACCOUNT_DATA *other_accounts_using;
      ITERATOR Iter;

      AttachIterator( &Iter, wSpace->who_using );
      while( ( other_accounts_using = (ACCOUNT_DATA *)NextInList( &Iter ) ) != NULL )
         text_to_account( other_accounts_using, "%s is no longer using %s workspace.\r\n", account->name, wSpace->name );
      DetachIterator( &Iter );
   }
   return;
}

WORKSPACE *workspace_list_has_by_name( LLIST *workspace_list, const char *name )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( !name || name[0] == '\0' )
      return NULL;
   if( !workspace_list || SizeOfList( workspace_list ) < 1 )
      return NULL;

   AttachIterator( &Iter, workspace_list );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( wSpace->name, name ) )
          break;
   DetachIterator( &Iter );

   return wSpace;
}

WORKSPACE *workspace_list_has_by_id( LLIST *workspace_list, int id )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( id < 0 )
      return NULL;
   if( !workspace_list || SizeOfList( workspace_list ) < 1 )
      return NULL;

   AttachIterator( &Iter, workspace_list );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( wSpace->tag->id == id )
         break;
   DetachIterator( &Iter );

   return wSpace;
}

void inception_open( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;

   text_to_account( account, "Opening Inception OLC...\r\n\r\n" );
   if( !account->olc )
   {
      account->olc = init_olc();
      account->olc->account = account;
   }
   change_socket_state( account->socket, STATE_OLC );
   return;
}

int olc_prompt( D_SOCKET *dsock, bool commands )
{
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;
   ACCOUNT_DATA *account = dsock->account;
   BUFFER *buf = buffer_new( MAX_OUTPUT );
   INCEPTION *olc;
   WORKSPACE *wSpace;
   ITERATOR Iter, IterF, IterI;
   char tempstring[MAX_BUFFER];
   int ret = RET_SUCCESS;
   int space_after_pipes, max_list, x;
   /* int style, max_frameworks; */
   if( !account )
   {
      BAD_POINTER( "account" );
      return ret;
   }

   if( ( olc = account->olc ) == NULL )
   {
      BAD_POINTER( "olc" );
      return ret;
   }

   if( dsock->bust_prompt == SHORT_PROMPT )
   {
      if( olc->using_workspace )
         text_to_olc( olc, "\r\nUsing: %s> ", olc->using_workspace->name );
      else
         text_to_olc( olc, "\r\nInception OLC> " );
      return ret;
   }

   space_after_pipes = account->pagewidth - 2;

   if( !olc->project )
      text_to_olc( olc, "/%s\\\r\n", print_header( "Inception OLC", "-", space_after_pipes ) );
   else
      text_to_olc( olc, "/%s\\\r\n", print_header( quick_format( "Inception OLC - Project: %s", olc->project->name ), "-", space_after_pipes ) );

   mud_printf( tempstring, " You have %d workspaces loaded.", SizeOfList( olc->wSpaces ) );
   text_to_olc( olc, "|%s|\r\n", fit_string_to_space( tempstring, space_after_pipes ) );
   if( SizeOfList( olc->wSpaces ) > 0 )
   {
      AttachIterator( &Iter, olc->wSpaces );
      while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      {
         mud_printf( tempstring, "- %s", wSpace->name );
         text_to_olc( olc, "|%s|\r\n", fit_string_to_space( tempstring, space_after_pipes ) );
      }
      DetachIterator( &Iter );
   }

   if( olc->using_workspace )
   {
      if( olc->using_filter->limit == 0 )
         max_list = UMAX( SizeOfList( olc->using_workspace->frameworks ), SizeOfList( olc->using_workspace->instances ) );
      else
         max_list = olc->using_filter->limit;

      text_to_olc( olc, "|%s|\r\n", print_header( quick_format( "Workspace: \"%s\"", olc->using_workspace->name ), "-", space_after_pipes ) );
      text_to_olc( olc, "|%s|", print_header( "Frameworks", " ", ( space_after_pipes -1 ) / 2 ) );
      text_to_olc( olc, "%s |\r\n", print_header( "Instances", " ", ( space_after_pipes - 1 ) / 2 ) );
      text_to_olc( olc, "|%s|\r\n", print_bar( "-", space_after_pipes ) );

      AttachIterator( &IterF, olc->using_workspace->frameworks );
      AttachIterator( &IterI, olc->using_workspace->instances );
      for( x = 0; x < max_list; x++ )
      {
         while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &IterF ) ) != NULL )
         {
            if( !frame_filter_pass( frame, olc->using_filter ) )
               continue;
            break;
         }
         while( ( instance = (ENTITY_INSTANCE *)NextInList( &IterI ) ) != NULL )
         {
            if( !instance_filter_pass( instance, olc->using_filter ) )
               continue;
            break;
         }

         if( frame )
            text_to_olc( olc, "|%s|", fit_string_to_space( quick_format( " %s%-4d: %s",
               frame->inherits ? quick_format( "(f%d) ", frame->inherits->tag->id ) : "",
               frame->tag->id, chase_name( frame ) ), ( space_after_pipes - 1 ) / 2 ) );
         else
            text_to_olc( olc, "|%s|", print_header( " ", " ", ( space_after_pipes - 1 ) / 2 ) );

         if( instance )
            text_to_olc( olc, " %s|\r\n", fit_string_to_space( quick_format( " (f%d) %-4d: %s", instance->framework->tag->id, instance->tag->id, instance_name( instance ) ), ( space_after_pipes - 1 ) / 2 ) );
         else
            text_to_olc( olc, " %s|\r\n", print_header( " ", " ", ( space_after_pipes - 1 ) / 2 ) );
      }
      DetachIterator( &IterF );
      DetachIterator( &IterI );
   }
/*
   if( olc->using_workspace )
   {
      max_frameworks = SizeOfList( olc->using_workspace->frameworks );
      max_list = UMAX( SizeOfList( olc->using_workspace->frameworks ), SizeOfList( olc->using_workspace->instances ) );

      if( max_list == max_frameworks )
         style = 1;
      else
         style = 2;

      mud_printf( tempstring, "%s Workspace", olc->using_workspace->name );
      text_to_olc( olc, "|%s|\r\n", print_header( tempstring, "-", space_after_pipes ) );


      switch( style )
      {
         case 1:
            text_to_olc( olc, "|%s|", print_header( "Frameworks", " ", ( space_after_pipes - 1 ) / 2 ) );
            text_to_olc( olc, " %s|\r\n", print_header( "Instances", " ", ( space_after_pipes - 1 ) / 2 ) );
            break;
         case 2:
            text_to_olc( olc, "|%s|", print_header( "Instances", " ", ( space_after_pipes - 1 ) / 2 ) );
            text_to_olc( olc, " %s|\r\n", print_header( "Frameworks", " ", ( space_after_pipes - 1 ) / 2 ) );
            break;
      }
      text_to_olc( olc, "|%s|\r\n", print_bar( "-", space_after_pipes ) );

      AttachIterator( &IterF, olc->using_workspace->frameworks );
      AttachIterator( &IterI, olc->using_workspace->instances );
      for( x = 0; x < max_list; x++ )
      {
         frame = (ENTITY_FRAMEWORK *)NextInList( &IterF );
         instance = (ENTITY_INSTANCE *)NextInList( &IterI );
         switch( style )
         {
            case 1:
               text_to_olc( olc, "|%s|", fit_string_to_space( quick_format( " %-7d: %s", frame->tag->id, frame->name ), ( space_after_pipes - 1 ) / 2 ) );
               if( !instance )
                  text_to_olc( olc, " %s|\r\n", print_header( " ", " ", ( space_after_pipes - 1 ) / 2 ) );
               else
                  text_to_olc( olc, " %s|\r\n", fit_string_to_space( quick_format( " %-7d: %s", instance->tag->id, instance_name( instance ) ), ( space_after_pipes - 1 ) / 2 ) );
               break;
            case 2:
               text_to_olc( olc, "|%s|", fit_string_to_space( quick_format( " %-7d: %s", instance->tag->id, instance_name( instance ) ) , ( space_after_pipes - 1 ) / 2 ) );
               if( !frame )
                  text_to_olc( olc, " %s|\r\n", print_header( " ", " ", ( space_after_pipes - 1 ) / 2 ) );
               else
                  text_to_olc( olc, " %s|\r\n", fit_string_to_space( quick_format( " %-7d: %s", frame->tag->id, frame->name ), ( space_after_pipes - 1 ) / 2 ) );
               break;
         }
      }
      DetachIterator( &IterF );
      DetachIterator( &IterI );
   }
*/
   text_to_olc( olc, "|%s|\r\n", print_bar( "-", space_after_pipes ) );
   if( !commands )
      return ret;
   print_commands( dsock->account->olc, dsock->account->olc->commands, buf, 0, account->pagewidth );
   text_to_olc( olc, buf->data );
   buffer_free( buf );
   text_to_olc( olc, "\\%s/\r\n", print_header( "Version 0.1", "-", space_after_pipes ) );
   return ret;
}

int new_workspace( WORKSPACE *wSpace )
{
   int ret = RET_SUCCESS;

   if( !wSpace )
   {
      BAD_POINTER( "wSpace" );
      return ret;
   }

   if( !strcmp( wSpace->tag->created_on, "null" ) )
   {
      if( ( ret = new_tag( wSpace->tag, "system" ) ) != RET_SUCCESS )
      {
         bug( "%s: called to new_tag failed giving back the returned code.", __FUNCTION__ );
         return ret;
      }
   }

   if( !quick_query( "INSERT INTO workspaces VALUES( %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', %d );",
              wSpace->tag->id, wSpace->tag->type, wSpace->tag->created_by,
              wSpace->tag->created_on, wSpace->tag->modified_by, wSpace->tag->modified_on,
              wSpace->name, wSpace->description, (int)wSpace->Public ) )
      return RET_FAILED_OTHER;

   return ret;
}

int add_frame_to_workspace( ENTITY_FRAMEWORK *frame, WORKSPACE *wSpace )
{
   int ret = RET_SUCCESS;

   if( !wSpace )
   {
      BAD_POINTER( "wSpace" );
      return ret;
   }
   if( !frame )
   {
      BAD_POINTER( "frame" );
      return ret;
   }
   if( framework_list_has_by_id( wSpace->frameworks, frame->tag->id ) )
      return RET_LIST_HAS;

   AttachToList( frame, wSpace->frameworks );
   new_workspace_entry( wSpace, frame->tag );
   return ret;
}

int add_instance_to_workspace( ENTITY_INSTANCE *instance, WORKSPACE *wSpace )
{
   int ret = RET_SUCCESS;

   if( !wSpace )
   {
      BAD_POINTER( "wSpace" );
      return ret;
   }
   if( !instance )
   {
      BAD_POINTER( "instance" );
      return ret;
   }
   if( instance_list_has_by_id( wSpace->instances, instance->tag->id ) )
      return RET_LIST_HAS;

   AttachToList( instance, wSpace->instances );
   new_workspace_entry( wSpace, instance->tag );
   return ret;
}

void rem_frame_from_workspace( ENTITY_FRAMEWORK *frame, WORKSPACE *wSpace )
{
   DetachFromList( frame, wSpace->frameworks );
   quick_query( "DELETE FROM `workspace_entries` WHERE entry='f%d' AND workspaceID=%d;", frame->tag->id, wSpace->tag->id );
   return;
}

void rem_instance_from_workspace( ENTITY_INSTANCE *instance, WORKSPACE *wSpace )
{
   DetachFromList( instance, wSpace->instances );
   quick_query( "DELETE FROM `workspace_entries` WHERE entry='i%d' AND workspaceID=%d;", instance->tag->id, wSpace->tag->id );
}

int add_workspace_to_olc( WORKSPACE *wSpace, INCEPTION *olc )
{
   ACCOUNT_DATA *account;
   ITERATOR Iter;
   char who_using[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( SizeOfList( wSpace->who_using ) > 0 )
   {
      memset( &who_using[0], 0, sizeof( who_using ) );
      AttachIterator( &Iter, wSpace->who_using );
      while( ( account = (ACCOUNT_DATA *)NextInList( &Iter ) ) != NULL )
      {
         if( who_using[0] != '\0' )
            strcat( who_using, ", " );
         strcat( who_using, account->name );
      }
      DetachIterator( &Iter );
      text_to_olc( olc, " These users(%s) are already using this workspace.", who_using );
   }
   text_to_olc( olc, "\r\n" );
   AttachToList( wSpace,  olc->wSpaces );
   AttachToList( olc->account, wSpace->who_using );

   if( olc->project && !workspace_list_has_by_id( olc->project->workspaces, wSpace->tag->id ) )
      add_workspace_to_project( wSpace, olc->project );

   return ret;
}

int new_workspace_entry( WORKSPACE *wSpace, ID_TAG *tag )
{
   int ret = RET_SUCCESS;

   if( !wSpace )
   {
      BAD_POINTER( "wSpace" );
      return ret;
   }

   if( !tag )
   {
      BAD_POINTER( "tag" );
      return ret;
   }
   if( !quick_query( "INSERT INTO workspace_entries VALUES ( %d, '%c%d' );", wSpace->tag->id, tag_table_characters[tag->type], tag->id ) )
      return RET_FAILED_OTHER;

   return ret;
}

int load_workspace_entries( WORKSPACE *wSpace )
{
   ENTITY_INSTANCE *instance;
   ENTITY_FRAMEWORK *frame;
   MYSQL_RES *result;
   MYSQL_ROW row;
   int ret = RET_SUCCESS;
   int id;

   if( !wSpace )
   {
      BAD_POINTER( "wSpace" );
      return ret;
   }

   if( !quick_query( "SELECT entry FROM workspace_entries WHERE workspaceID=%d;", wSpace->tag->id ) )
      return RET_FAILED_OTHER;
   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
     return RET_FAILED_OTHER;

   if( mysql_num_rows( result ) < 1 )
   {
      mysql_free_result( result );
      return RET_DB_NO_ENTRY;
   }

   while( ( row = mysql_fetch_row( result ) ) != NULL )
   {
      switch( row[0][0] )
      {
         default: continue;
         case 'f':
            id = atoi( row[0]+1 );
            if( ( frame = get_framework_by_id( id ) ) == NULL )
            {
               bug( "%s: bad entry in workspace_entries %d,", __FUNCTION__, id );
               continue;
            }
            AttachToList( frame, wSpace->frameworks );
            break;
         case 'i':
            id = atoi( row[0]+1 );
            if( ( instance = get_instance_by_id( id ) ) == NULL )
            {
               bug( "%s: bad entry in workspace_entries %d,", __FUNCTION__, id );
               continue;
            }
            AttachToList( instance, wSpace->instances );
            break;
      }
   }
   mysql_free_result( result );
   return ret;
}

void switch_using( INCEPTION *olc, char *arg )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( !strcasecmp( arg, "none" ) )
   {
      text_to_olc( olc, "You are no longer using any workspace.\r\n" );
      olc->using_workspace = NULL;
      return;
   }

   AttachIterator( &Iter, olc->wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( arg, wSpace->name ) )
         break;
   DetachIterator( &Iter );

   if( !wSpace )
   {
      text_to_olc( olc, "You have no such workspace loaded. Remember, be specific!\r\n" );
      olc_short_prompt( olc );
   }
   else
   {
      olc->using_workspace = wSpace;
      text_to_olc( olc, "You are now using %s.\r\n", wSpace->name );
   }
}

void grab_entity( INCEPTION *olc, char *arg, GRAB_PARAMS *params )
{
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;
   char buf[MAX_BUFFER];
   int ret;

   while( arg && arg[0] != '\0' )
   {
      arg = one_arg( arg, buf );

      if( !interpret_entity_selection( buf ) )
         continue;

      switch( input_selection_typing )
      {
         default: clear_entity_selection(); continue;
         case SEL_FRAME:
            frame = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
            if( !should_grab_framework( frame, params ) )
               break;
            if( ( ret = add_frame_to_workspace( frame, olc->using_workspace ) ) == RET_SUCCESS )
               text_to_olc( olc, "Framework %d: %s loaded into %s workspace.\r\n", frame->tag->id, frame->name, olc->using_workspace->name );
            else if( ret == RET_LIST_HAS )
            {
               text_to_olc( olc, "This workspace already has the framework: %s\r\n", buf );
               olc_short_prompt( olc );
            }
            break;
         case SEL_INSTANCE:
            instance = (ENTITY_INSTANCE *)retrieve_entity_selection();
            if( !should_grab_instance( instance, params ) )
               break;
            if( ( ret = add_instance_to_workspace( instance, olc->using_workspace ) ) == RET_SUCCESS )
               text_to_olc( olc, "Instance %d: %s loaded into %s workspace.\r\n", instance->tag->id, instance_name( instance ), olc->using_workspace->name );
            else if( ret == RET_LIST_HAS )
            {
               text_to_olc( olc, "this workspace already has that instance: %s\r\n", buf );
               olc_short_prompt( olc );
            }
            break;
         case SEL_STRING:
            text_to_olc( olc, (char *)retrieve_entity_selection(), buf );
            olc_short_prompt( olc );
            break;
      }
   }
   return;
}

void grab_entity_range( INCEPTION *olc, char *arg )
{
   GRAB_PARAMS params;
   char *rng_ptr;
   char ranges[MAX_BUFFER];
   char buf[MAX_BUFFER];
   char grab_str[MAX_BUFFER];
   char type;
   int start, end;
   int x;

   memset( &ranges[0], 0, sizeof( ranges ) );
   params = grab_params( ranges, arg, ' ' );

   rng_ptr = ranges;
   while( rng_ptr && rng_ptr[0] != '\0' )
   {
      rng_ptr = one_arg( rng_ptr, buf );
      if( string_contains( buf, "-" ) )
      {
         if( !grab_range_and_type( buf, &type, &start, &end ) )
         {
            text_to_olc( olc, "Improper range: %s\r\n", buf );
            continue;
         }

         if( type != 'f' && type != 'i' )
         {
            text_to_olc( olc, "'%c' is invalid, (f)rameworks and (i)nstances only.", type );
            continue;
         }

         for( x = start; x <= end; x++ )
         {
            mud_printf( grab_str, "%c%d", type, x );
            grab_entity( olc, grab_str, &params );
         }
      }
      else
         grab_entity( olc, buf, &params );
   }
   return;
}

GRAB_PARAMS grab_params( char *ranges, char *arg, char delim )
{
   GRAB_PARAMS params;
   char buf[MAX_BUFFER];

   reset_params( &params );

   while( arg && arg[0] != '\0' )
   {
      arg = one_arg_delim( arg, buf, delim );
      if( !strcmp( buf, "noexits" ) )
      {
         params.no_exits = TRUE;
         continue;
      }
      if( !strcmp( buf, "noobjects" ) )
      {
         params.no_objects = TRUE;
         continue;
      }
      if( !strcmp( buf, "norooms" ) )
      {
         params.no_rooms = TRUE;
         continue;
      }
      if( !strcmp( buf, "nomobiles" ) || !strcmp( buf, "nomobs" ) )
      {
         params.no_mobiles = TRUE;
         continue;
      }
      strcat( ranges, buf );
      strcat( ranges, quick_format( "%c", delim ) );
   }
   return params;
}

bool should_grab_instance( ENTITY_INSTANCE *instance, GRAB_PARAMS *params )
{
   if( !params )
      return TRUE;

   if( should_grab_from_specs( instance->specifications, params ) )
      if( should_grab_framework( instance->framework, params ) )
         return TRUE;
   return FALSE;

}

bool should_grab_framework( ENTITY_FRAMEWORK *frame, GRAB_PARAMS *params )
{
   if( !params )
      return TRUE;

   if( !should_grab_from_specs( frame->specifications, params ) )
      return FALSE;
   if( frame->inherits )
      if( !should_grab_framework( frame->inherits, params ) )
         return FALSE;
   return TRUE;
}

bool should_grab_from_specs( LLIST *specs, GRAB_PARAMS *params )
{
   if( params->no_exits && spec_list_has_by_type( specs, SPEC_ISEXIT ) )
      return FALSE;
   if( params->no_objects && spec_list_has_by_type( specs, SPEC_ISOBJECT ) )
      return FALSE;
   if( params->no_rooms && spec_list_has_by_type( specs, SPEC_ISROOM ) )
      return FALSE;
   if( params->no_mobiles && spec_list_has_by_type( specs, SPEC_ISMOB ) )
      return FALSE;
   return TRUE;
}

void reset_params( GRAB_PARAMS *params )
{
  params->no_exits = FALSE;
  params->no_objects = FALSE;
  params->no_rooms = FALSE;
  params->no_mobiles = FALSE;
}

int text_to_olc( INCEPTION *olc, const char *fmt, ... )
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

   text_to_buffer( olc->account->socket, dest );
   return res;
}

void olc_no_prompt( INCEPTION *olc )
{
   if( !olc->account || !olc->account->socket )
      return;
   olc->account->socket->bust_prompt = NO_PROMPT;
   return;
}

void olc_short_prompt( INCEPTION *olc )
{
   if( !olc->account || !olc->account->socket )
      return;
   olc->account->socket->bust_prompt = SHORT_PROMPT;
}

void olc_show_prompt( INCEPTION *olc )
{
   if( !olc->account || !olc->account->socket )
      return;
   olc->account->socket->bust_prompt = NORMAL_PROMPT;
   return;
}

bool workspace_list_has_name( LLIST *wSpaces, const char *name )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( !name || name[0] == '\0' )
      return FALSE;

   if( !wSpaces || SizeOfList( wSpaces ) < 1 )
      return FALSE;

   AttachIterator( &Iter, wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( wSpace->name, name ) )
         break;
   DetachIterator( &Iter );

   if( wSpace )
      return TRUE;
   return FALSE;
}



WORKSPACE *copy_workspace( WORKSPACE *wSpace, bool copy_frameworks, bool copy_instances )
{
   WORKSPACE *wSpace_copy;

   if( !wSpace )
   {
      bug( "%s: passed a NULL wSpace.", __FUNCTION__ );
      return NULL;
   }

   CREATE( wSpace_copy, WORKSPACE, 1 );
   wSpace_copy->tag = copy_tag( wSpace->tag );

   wSpace_copy->name = strdup( wSpace->name );
   wSpace_copy->description = strdup( wSpace->description );

   wSpace_copy->Public = wSpace->Public;

   /* new filter copying needs to be here...

   wSpace_copy->hide_frameworks = wSpace->hide_frameworks;
   wSpace_copy->hide_instances = wSpace->hide_instances;
   */
   wSpace_copy->frameworks = copy_framework_list( wSpace->frameworks, copy_frameworks, copy_frameworks, copy_frameworks, copy_frameworks );
   wSpace_copy->instances = copy_instance_list( wSpace->instances, copy_instances, copy_instances, copy_instances, copy_instances );

   return wSpace_copy;
}

LLIST *copy_workspace_list( LLIST *wSpaces, bool copy_instances, bool copy_frameworks )
{
   LLIST *list;

   if( !wSpaces || SizeOfList( wSpaces ) < 1 )
   {
       bug( "%s: was passed an empty or NULL wSpaces.", __FUNCTION__ );
       return NULL;
   }

   list = AllocList();
   copy_workspaces_into_list( wSpaces, list, copy_instances, copy_frameworks);

   return list;
}

void copy_workspaces_into_list( LLIST *wSpaces, LLIST *copy_into_list, bool copy_instances, bool copy_frameworks )
{
   WORKSPACE *wSpace;
   WORKSPACE *wSpace_copy;
   ITERATOR Iter;

   if( !wSpaces )
   {
      bug( "%s: was passed a NULL wSpaces.", __FUNCTION__ );
      return;
   }
   if( !copy_into_list )
   {
      bug( "%s: was passed a NULL copy_into_list.", __FUNCTION__ );
      return;
   }

   AttachIterator( &Iter, wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
   {
      if( copy_instances || copy_frameworks )
      {
         wSpace_copy = copy_workspace( wSpace, copy_instances, copy_frameworks );
         AttachToList( wSpace_copy, copy_into_list );
         continue;
      }
      AttachToList( wSpace, copy_into_list );
   }
   DetachIterator( &Iter );

   return;
}

void toggle_no_exit( INCEPTION *olc )
{
   GRAB_PARAMS *spec_filters = olc->using_filter->spec_filters;

   if( spec_filters->no_exits )
      spec_filters->no_exits = FALSE;
   else
      spec_filters->no_exits = TRUE;

   text_to_olc( olc, "You toggle No Exits %s.\r\n", spec_filters->no_exits ? "on" : "off" );
   return;
}

void toggle_no_objects( INCEPTION *olc )
{
   GRAB_PARAMS *spec_filters = olc->using_filter->spec_filters;

   if( spec_filters->no_objects )
      spec_filters->no_objects = FALSE;
   else
      spec_filters->no_objects = TRUE;

   text_to_olc( olc, "You toggle No Objects %s.\r\n", spec_filters->no_objects ? "on" : "off" );
   return;
}

void toggle_no_rooms( INCEPTION *olc )
{
   GRAB_PARAMS *spec_filters = olc->using_filter->spec_filters;

   if( spec_filters->no_rooms )
      spec_filters->no_rooms = FALSE;
   else
      spec_filters->no_rooms = TRUE;

   text_to_olc( olc, "You toggle No Rooms %s.\r\n", spec_filters->no_rooms ? "on" : "off" );
   return;
}

void toggle_no_mobiles( INCEPTION *olc )
{
   GRAB_PARAMS *spec_filters = olc->using_filter->spec_filters;

   if( spec_filters->no_mobiles )
      spec_filters->no_mobiles = FALSE;
   else
      spec_filters->no_mobiles = TRUE;

   text_to_olc( olc, "You toggle No Mobs %s.\r\n", spec_filters->no_mobiles ? "on" : "off" );
   return;
}

void set_limit_filter( INCEPTION *olc, char *arg )
{
   int limit;

   if( !is_number( arg ) )
   {
      text_to_olc( olc, "Limit only takes a whole number.\r\n" );
      return;
   }
   limit = atoi( arg );

   text_to_olc( olc, "Display limit on Using Workspace set to %s.\r\n", limit == 0 ? "unlimited" : quick_format( "%d", limit ) );
   olc->using_filter->limit = limit;
   return;

}

void toggle_name_filter( INCEPTION *olc, char *arg )
{
   WORKSPACE_FILTER *filter = olc->using_filter;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Filter out what names?\r\n" );
      return;
   }

   if( filter->name_count < 1 )
   {
      CREATE( filter->filter_name, char *, 1 );
      filter->filter_name[0] = strdup( arg );
      filter->name_count++;
      text_to_olc( olc, "You are now filtering any instance or framework with the name %s.\r\n", arg );
      return;
   }

   text_to_olc( olc, "You are now %sfiltering any instance or framework with the name %s.\r\n",  handle_string_filter( &filter->filter_name, arg, &filter->name_count ) ? "" : "no longer ", arg );
   return;
}

void toggle_short_filter( INCEPTION *olc, char *arg )
{
   WORKSPACE_FILTER *filter = olc->using_filter;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Filter out what shorts?\r\n" );
      return;
   }

   if( filter->short_count < 1 )
   {
      CREATE( filter->filter_short, char *, 1 );
      filter->filter_short[0] = strdup( arg );
      filter->short_count++;
      text_to_olc( olc, "You are now filtering any instance or framework with a short description containing %s.\r\n", arg  );
      return;
   }

   text_to_olc( olc, "You are now %sfiltering any instance or framework with the short description containing %s.\r\n", handle_string_filter( &filter->filter_short, arg, &filter->short_count ) ? "" : "no longer ", arg );
   return;
}

void toggle_long_filter( INCEPTION *olc, char *arg )
{
   WORKSPACE_FILTER *filter = olc->using_filter;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Filter out what longs?\r\n" );
      return;
   }

   if( filter->long_count < 1 )
   {
      CREATE( filter->filter_long, char *, 1 );
      filter->filter_long[0] = strdup( arg );
      filter->long_count++;
      text_to_olc( olc, "You are now filtering any instance or framework with a long description containing %s.\r\n", arg );
      return;
   }

   text_to_olc( olc, "You are now %sfiltering any instance or framework with the long description containg %s.\r\n", handle_string_filter( &filter->filter_long, arg, &filter->long_count ) ? "" : "no longer ", arg );
   return;
}

void toggle_desc_filter( INCEPTION *olc, char *arg )
{
   WORKSPACE_FILTER *filter = olc->using_filter;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Filter out what descriptions?\r\n" );
      return;
   }

   if( filter->desc_count < 1 )
   {
      CREATE( filter->filter_desc, char *, 1 );
      filter->filter_desc[0] = strdup( arg );
      filter->desc_count++;
      text_to_olc( olc, "You are now filtering any instance or framework with a description containing %s.\r\n", arg );
      return;
   }

   text_to_olc( olc, "You are now %sfiltering any instance or framework with a description containg %s.\r\n", arg );
   return;
}

bool handle_string_filter( char ***filter_string, char *arg, int *count )
{
   char **realloc_filter;
   char **char_swap;
   int x, y;

   for( x = 0; x < *count; x++ )
   {
      if( !strcmp( (*filter_string)[x], arg ) )
      {
         CREATE( char_swap, char *, ( *count - 1 ) );
         for( y = 0, x = 0; y < *count; y++)
         {
            if( strcmp( (*filter_string)[y], arg ) )
               char_swap[x++] = strdup( (*filter_string)[y] );
            FREE( (*filter_string)[y] );
         }
         *count -= 1;
         realloc_filter = (char **)realloc( *filter_string, *count * sizeof( char ** ) );
         *filter_string = realloc_filter;
         for( y = 0; y < *count; y++ )
            (*filter_string)[y] = strdup( char_swap[y] );
         return FALSE;
      }
   }
   *count += 1;
   realloc_filter = (char **)realloc( *filter_string, *count * sizeof( char ** ) );
   *filter_string = realloc_filter;
   (*filter_string)[*count - 1] = strdup( arg );
   return TRUE;
}

bool frame_filter_pass( ENTITY_FRAMEWORK *frame, WORKSPACE_FILTER *filter )
{
   if( !should_grab_framework( frame, filter->spec_filters ) )
      return FALSE;
   if( !filter_string_check( chase_name( frame ), filter->filter_name, filter->name_count, TRUE ) )
      return FALSE;
   if( !filter_string_check( chase_short_descr( frame ), filter->filter_short, filter->short_count, FALSE ) )
      return FALSE;
   if( !filter_string_check( chase_long_descr( frame ), filter->filter_long, filter->long_count, FALSE ) )
      return FALSE;
   if( !filter_string_check( chase_description( frame ), filter->filter_desc, filter->desc_count, FALSE ) )
      return FALSE;
   return TRUE;

}

bool instance_filter_pass( ENTITY_INSTANCE *instance, WORKSPACE_FILTER *filter )
{
   if( !should_grab_instance( instance, filter->spec_filters ) )
      return FALSE;
   if( !filter_string_check( instance_name( instance ), filter->filter_name, filter->name_count, TRUE ) )
      return FALSE;
   if( !filter_string_check( instance_short_descr( instance ), filter->filter_short, filter->short_count, FALSE ) )
      return FALSE;
   if( !filter_string_check( instance_long_descr( instance ), filter->filter_long, filter->long_count, FALSE ) )
      return FALSE;
   if( !filter_string_check( instance_description( instance ), filter->filter_desc, filter->desc_count, FALSE ) )
      return FALSE;
   return TRUE;
}

bool filter_string_check( const char *arg, char **arg_list, int max, bool precise )
{
   int x;

   for( x = 0; x < max; x++ )
   {
      if( precise )
      {
         if( !strcmp( arg, arg_list[x] ) )
            return FALSE;
      }
      else
      {
         if( string_contains( arg, arg_list[x] ) )
            return FALSE;
      }
   }
   return TRUE;
}

void show_all_frameworks_to_olc( INCEPTION *olc )
{
   LLIST *list = AllocList();
   MYSQL_ROW row;
   ITERATOR Iter;

   if( !db_query_list_row( list, "SELECT entityFrameworkID, name FROM `entity_frameworks`;" ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
      text_to_olc( olc, "Framework %d: %s\r\n", atoi( row[0] ), row[1] );
   DetachIterator( &Iter );

   FreeList( list );
   return;
}

void show_all_instances_to_olc( INCEPTION *olc )
{
   LLIST *list = AllocList();
   MYSQL_ROW row;
   ITERATOR Iter;

   if( !db_query_list_row( list, "SELECT `entity_instances`.entityInstanceID, `entity_frameworks`.name FROM `entity_instances` LEFT JOIN `entity_frameworks` ON `entity_instances`.frameworkID=`entity_frameworks`.entityFrameworkID ORDER BY `entity_instances`.entityInstanceID;" ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
      text_to_olc( olc, "Instance %d: %s\r\n", atoi( row[0] ), row[1] );
   DetachIterator( &Iter );

   FreeList( list );
   return;
}

void show_all_workspaces_to_olc( INCEPTION *olc )
{
   LLIST *list = AllocList();
   MYSQL_ROW row;
   ITERATOR Iter;

   if( !db_query_list_row( list, "SELECT workspaceID, name FROM `workspaces`;" ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
      text_to_olc( olc, "Workspace %d: %s\r\n", atoi( row[0] ), row[1] );
   DetachIterator( &Iter );

   FreeList( list );
   return;
}

void show_all_projects_to_olc( INCEPTION *olc )
{
   LLIST *list = AllocList();
   MYSQL_ROW row;
   ITERATOR Iter;

   if( !db_query_list_row( list, "SELECT projectID, name FROM `projects`;" ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
      text_to_olc( olc, "Project %d: %s\r\n", atoi( row[0] ), row[1] );
   DetachIterator( &Iter );

   FreeList( list );
   return;
}

void show_all_stats_to_olc( INCEPTION *olc )
{
   LLIST *list = AllocList();
   MYSQL_ROW row;
   ITERATOR Iter;

   if( !db_query_list_row( list, "SELECT statFrameworkID, name FROM `stat_frameworks`;" ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
      text_to_olc( olc, "Stat %d: %s\r\n", atoi( row[0] ), row[1] );
   DetachIterator( &Iter );

   FreeList( list );
   return;
}

void show_range_frameworks_to_olc( INCEPTION *olc, int start, int end )
{
   LLIST *list = AllocList();
   MYSQL_ROW row;
   ITERATOR Iter;

   if( !db_query_list_row( list, quick_format( "SELECT entityFrameworkID, name FROM `entity_frameworks` WHERE entityFrameworkID BETWEEN %d AND %d ORDER BY entityFrameworkID;", start, end ) ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
      text_to_olc( olc, "Framework %d: %s\r\n", atoi( row[0] ), row[1] );
   DetachIterator( &Iter );

   FreeList( list );
   return;
}

void show_range_instances_to_olc( INCEPTION *olc, int start, int end )
{
   LLIST *list = AllocList();
   MYSQL_ROW row;
   ITERATOR Iter;
   if( !db_query_list_row( list, quick_format( "SELECT `entity_instances`.entityInstanceID, `entity_frameworks`.name FROM `entity_instances` LEFT JOIN `entity_frameworks` ON `entity_instances`.frameworkID=`entity_frameworks`.entityFrameworkID WHERE `entity_instances`.entityInstanceID >= %d AND `entity_instances`.entityInstanceID <= %d ORDER BY `entity_instances`.entityInstanceID;", start, end ) ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
      text_to_olc( olc, "Instance %d: %s\r\n", atoi( row[0] ), row[1] );
   DetachIterator( &Iter );

   FreeList( list );
   return;
}

void show_range_workspaces_to_olc( INCEPTION *olc, int start, int end )
{
   LLIST *list = AllocList();
   MYSQL_ROW row;
   ITERATOR Iter;

   if( !db_query_list_row( list, quick_format( "SELECT workspaceID, name FROM `workspaces` WHERE workspaceID BETWEEN %d AND %d;", start, end ) ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
      text_to_olc( olc, "Workspace %d: %s\r\n", atoi( row[0] ), row[1] );
   DetachIterator( &Iter );

   FreeList( list );
   return;
}

void olc_file( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   COMMAND *file_command;

   if( ( file_command = olc->account->executing_command ) == NULL )
      return;

   if( file_command->sub_commands )
   {
      free_command_list( file_command->sub_commands );
      FreeList( file_command->sub_commands );
      file_command->sub_commands = NULL;
      text_to_olc( olc, "File Menu Closed.\r\n" );
   }
   else
   {
      file_command->sub_commands = AllocList();
      load_commands( file_command->sub_commands, file_sub_commands, olc->account->level );
      text_to_olc( olc, "File Menu Opened.\r\n" );
   }
   return;
}

void olc_workspace( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   COMMAND *workspace_command;

   if( ( workspace_command = olc->account->executing_command ) == NULL )
      return;

   if( workspace_command->sub_commands )
   {
      free_command_list( workspace_command->sub_commands );
      FreeList( workspace_command->sub_commands );
      workspace_command->sub_commands = NULL;
      text_to_olc( olc, "Workspace Commands Menu Closed.\r\n" );
   }
   else
   {
      workspace_command->sub_commands = AllocList();
      load_commands( workspace_command->sub_commands, workspace_sub_commands, olc->account->level );
      text_to_olc( olc, "Workspace Commands Menu Opened.\r\n" );
   }
   return;
}

void workspace_new( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE *wSpace;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "You need to enter a name for your new workspace.\r\n" );
      return;
   }

   if( ( wSpace = get_workspace_by_name( arg ) ) )
   {
      text_to_olc( olc, "A workspace with that name already exists.\r\n" );
      return;
   }

   wSpace = init_workspace();
   if( new_tag( wSpace->tag, olc->account->name ) != RET_SUCCESS )
   {
      text_to_olc( olc, "You could not get a new tag for your workspace, therefore, it was not created.\r\n" );
      free_workspace( wSpace );
      olc_short_prompt( olc );
      return;
   }
   FREE( wSpace->name );
   wSpace->name = strdup( arg );
   if( new_workspace( wSpace ) != RET_SUCCESS )
   {
      text_to_olc( olc, "Your new workspace could not be saved to the database it will not be created.\r\n" );
      free_workspace( wSpace );
      return;
   }
   AttachToList( wSpace, olc->wSpaces );
   add_workspace_to_olc( wSpace, olc );
   text_to_olc( olc, "A new workspace %s has been created and loaded.\r\n", wSpace->name );
   return;
}

void workspace_load( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE *wSpace;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;
   char buf[MAX_BUFFER];
   bool found = FALSE;
   int x;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "What's the name of the workspace you want to load?\r\n" );
      return;
   }

   arg = one_arg( arg, buf );

   /* workspace messaging, needs factoring */

   AttachIterator( &Iter, active_wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
   {
      if(  ( x = strcasecmp( buf, wSpace->name ) ) == 0 || x == -110 )
      {
         if( workspace_list_has_name( olc->wSpaces, wSpace->name ) )
         {
            text_to_olc( olc, "You already have workspace %s loaded.\r\n", wSpace->name );
            found = TRUE;
            continue;
         }
         found = TRUE;
         if( !wSpace->Public && strcmp( wSpace->tag->created_by, olc->account->name ) )
            text_to_olc( olc, "The workspace %s is private and you did not create it.\r\n", wSpace->name );
         else
         {
            text_to_olc( olc, "Workspace %s loaded into your OLC.", wSpace->name );
            add_workspace_to_olc( wSpace, olc );
         }
      }
   }
   DetachIterator( &Iter );

   /* needs factoring */

   list = AllocList();
   if( db_query_list_row( list, quick_format( "SELECT * FROM workspaces WHERE name LIKE '%s%%';", format_string_for_sql( buf ) ) ) )
   {
      AttachIterator( &Iter, list );
      while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
      {
         wSpace = init_workspace();
         db_load_workspace( wSpace, &row );
         if( workspace_list_has_name( active_wSpaces, wSpace->name ) )
         {
            free_workspace( wSpace );
            continue;
         }
         load_workspace_entries( wSpace );
         found = TRUE;
         AttachToList( wSpace, active_wSpaces );
         text_to_olc( olc, "Workspace %s loaded from database.", wSpace->name );
         add_workspace_to_olc( wSpace, olc );
      }
      DetachIterator( &Iter );
   }
   FreeList( list );

   if( !found )
      text_to_olc( olc, "No workspaces with that name exist.\r\n" );
   return;
}

void workspace_unload( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( SizeOfList( olc->wSpaces ) < 1 )
   {
      text_to_olc( olc, "You have no workspaces to unload.\r\n" );
      return;
   }

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Unload which workspace?\r\n" );
      return;
   }

   AttachIterator( &Iter, olc->wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( wSpace->name, arg ) )
         break;
   DetachIterator( &Iter );

   if( !wSpace )
   {
      text_to_olc( olc, "You have no such workspace loaded.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( olc->project )
      rem_workspace_from_project( wSpace, olc->project );
   DetachFromList( wSpace, olc->wSpaces );
   unuse_workspace( wSpace, olc->account );
   text_to_olc( olc, "Workspace unloaded.\r\n" );
   return;
}

void workspace_grab( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Grab what?\r\n" );
      return;
   }

   if( !olc->using_workspace )
   {
      text_to_olc( olc, "You have to be using a workspace to grab.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( string_contains( arg, "-" ) )
      grab_entity_range( olc, arg );
   else
      grab_entity( olc, arg, NULL );

   return;

}
void workspace_ungrab( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;
   char buf[MAX_BUFFER];

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Ungrab what?\r\n" );
      return;
   }
   if( !olc->using_workspace )
   {
      text_to_olc( olc, "You can't ungrab anything from a workspace you aren't using.\r\n" );
      return;
   }

   while( arg && arg[0] != '\0' )
   {
      arg = one_arg_delim( arg, buf, ',' );

      if( !interpret_entity_selection( buf ) )
         continue;

      switch( input_selection_typing )
      {
         default: clear_entity_selection(); continue;
         case SEL_FRAME:
            frame = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
            if( framework_list_has_by_id( olc->using_workspace->frameworks, frame->tag->id ) )
            {
               text_to_olc( olc, "You remove %d frame from %s.\r\n", frame->tag->id, olc->using_workspace->name );
               rem_frame_from_workspace( frame, olc->using_workspace );
               break;
            }
            text_to_olc( olc, "%s workspace does not have frame %d.\r\n", olc->using_workspace->name, frame->tag->id );
            break;
         case SEL_INSTANCE:
            instance = (ENTITY_INSTANCE *)retrieve_entity_selection();
            if( instance_list_has_by_id( olc->using_workspace->instances, instance->tag->id ) )
            {
               text_to_olc( olc, "You remove %d instance from %s.\r\n", instance->tag->id, olc->using_workspace->name );
               rem_instance_from_workspace( instance, olc->using_workspace );
               break;
            }
            text_to_olc( olc, "%s workspace does not have instance %d.\r\n", olc->using_workspace->name, instance->tag->id );
            break;
         case SEL_STRING:
            text_to_olc( olc, (char *)retrieve_entity_selection(), buf );
            olc_short_prompt( olc );
            break;
      }
   }
   return;
}

void olc_frameworks( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   COMMAND *frameworks_command;

   if( ( frameworks_command = olc->account->executing_command ) == NULL )
      return;

   if( frameworks_command->sub_commands )
   {
      free_command_list( frameworks_command->sub_commands );
      FreeList( frameworks_command->sub_commands );
      frameworks_command->sub_commands = NULL;
      text_to_olc( olc, "Frameworks Commands Menu Closed.\r\n" );
   }
   else
   {
      frameworks_command->sub_commands = AllocList();
      load_commands( frameworks_command->sub_commands, frameworks_sub_commands, olc->account->level );
      text_to_olc( olc, "Frameworks Commands Menu Opened.\r\n" );
   }
}

void framework_create( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;

   if( olc->editing )
   {
      text_to_olc( olc, "There's already something loaded in your editor, finish that first.\r\n" );
      change_socket_state( olc->account->socket, olc->editing_state );
      return;
   }
   if( !arg || arg[0] == '\0' )
      boot_eFramework_editor( olc, NULL );
   else
   {
      frame = init_eFramework();
      load_pak_on_framework( arg, frame );
      boot_eFramework_editor( olc, frame );
   }
   return;
}

void olc_screate( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   if( olc->editing )
   {
      text_to_olc( olc, "There's already something loaded in your editor, finish that first.\r\n" );
      change_socket_state( olc->account->socket, olc->editing_state );
      return;
   }
   boot_sFramework_editor( olc, NULL );
   return;
}

void olc_edit( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   void *to_edit;
   int type;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Edit what?\r\n" );
      return;
   }

   if( !interpret_entity_selection( arg ) )
   {
      text_to_olc( olc, STD_SELECTION_ERRMSG_PTR_USED );
      olc_short_prompt( olc );
      return;
   }
   type = input_selection_typing;

   to_edit = retrieve_entity_selection();
   switch( type )
   {
      default: clear_entity_selection(); return;
      case SEL_FRAME:
         boot_eFramework_editor( olc, (ENTITY_FRAMEWORK *)to_edit );
         return;
      case SEL_INSTANCE:
         boot_instance_editor( olc, (ENTITY_INSTANCE *)to_edit );
         return;
      case SEL_WORKSPACE:
         boot_workspace_editor( olc, (WORKSPACE *)to_edit );
         return;
      case SEL_PROJECT:
         boot_project_editor( olc, (PROJECT *)to_edit );
         return;
      case SEL_STAT_FRAMEWORK:
         boot_sFramework_editor( olc, (STAT_FRAMEWORK *)to_edit );
         return;
      case SEL_STRING:
         text_to_olc( olc, (char *)to_edit, arg );
         return;

   }
   return;
}

void framework_iedit( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *to_edit;
   ENTITY_FRAMEWORK *inherited_to_edit;

   if( olc->editing && ( arg && arg[0] != '\0' ) )
   {
      text_to_olc( olc, "You alrady have something loaded in your editor, type editor with no arguments to load it and complete it.\r\n" );
      return;
   }

   if( ( to_edit = olc_edit_selection( olc, arg ) ) == NULL ) /* handles its own messaging */
      return;

   if( ( inherited_to_edit = create_inherited_framework( to_edit ) ) == NULL ) /* does its own setting and databasing */
   {
      text_to_olc( olc, "Something has gone wrong tryin gto create an inherited frame.\r\n" );
      return;
   }

   boot_eFramework_editor( olc, inherited_to_edit );
   if( olc->using_workspace )
      add_frame_to_workspace( inherited_to_edit, olc->using_workspace );
   text_to_olc( olc, "You begin to edit %s.\r\n", chase_name( inherited_to_edit ) );
   return;
}

void olc_instantiate( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;

   if( check_selection_type( arg ) != SEL_FRAME )
   {
      text_to_olc( olc, "Use proper selection typing for a Framework.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( !interpret_entity_selection( arg ) )
   {
      text_to_olc( olc, STD_SELECTION_ERRMSG_PTR_USED );
      olc_short_prompt( olc );
      return;
   }

   switch( input_selection_typing )
   {
      default:
         clear_entity_selection();
         text_to_olc( olc, "There's been a major problem. Contact your nearest admin.\r\n" );
         olc_short_prompt( olc );
         return;
      case SEL_FRAME:
         frame = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
      case SEL_STRING:
         text_to_olc( olc, (char *)retrieve_entity_selection(), arg );
   }

   if( ( instance = eInstantiate( frame ) ) == NULL )
   {
      text_to_olc( olc, "There's been a major problem, framework you are trying to instantiate from may not be live.\r\n" );
      return;
   }

   AttachToList( instance, eInstances_list );
   if( olc->using_workspace )
      add_instance_to_workspace( instance, olc->using_workspace );

   text_to_olc( olc, "You create a new instance using the %s framework, its ID is %d.\r\n", frame->name, instance->tag->id );
   return;
}

void olc_using( void *passed, char *arg )
{
   INCEPTION * olc = (INCEPTION *)passed;

   if( SizeOfList( olc->wSpaces ) < 1 )
   {
      text_to_olc( olc, "You have no workspaces loaded.\r\n" );
      return;
   }
   switch_using( olc, arg );
   return;
}

void olc_builder( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *builder;

   if( ( builder = init_builder() ) == NULL )
   {
      text_to_olc( olc, "Could not allocate memory for a Builder... that's really bad.\r\n" );
      olc_short_prompt( olc );
      return;
   }
   AttachToList( builder, eInstances_list );
   builder->account = olc->account;
   text_to_olc( olc, "You enter builder mode.\r\n" );
   socket_control_entity( olc->account->socket, builder );
   account_control_entity( olc->account, builder );
   change_socket_state( olc->account->socket, STATE_BUILDER );
   load_instance_timers( builder );
   if( olc->builder_location != 0 )
   {
      entity_to_world( builder, get_instance_by_id( olc->builder_location ) );
      entity_look( builder, "" );
   }
   return;

}

void olc_show( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;

   text_to_olc( olc, "Your current Inception looks like this.\r\n" );
   olc_show_prompt( olc );

}
void olc_quit( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;

   text_to_olc( olc, "You close the Inception OLC.\r\n" );
   change_socket_state( olc->account->socket, STATE_ACCOUNT );
   return;
}

void olc_load( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;
   PROJECT *project;
   WORKSPACE *wSpace;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Load what?\r\n" );
      return;
   }

   if( !interpret_entity_selection( arg ) )
   {
      text_to_olc( olc, STD_SELECTION_ERRMSG_PTR_USED );
      olc_short_prompt( olc );
      return;
   }

   switch( input_selection_typing )
   {
      default:
         clear_entity_selection();
         text_to_olc( olc, "Invalid selection type, frames, instances and workspaces only.\r\n" );
         return;
      case SEL_FRAME:
         frame = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
         instance = full_load_eFramework( frame );
         break;
      case SEL_INSTANCE:
         instance = (ENTITY_INSTANCE *)retrieve_entity_selection();
         full_load_instance( instance );
         break;
      case SEL_WORKSPACE:
         wSpace = (WORKSPACE *)retrieve_entity_selection();
         full_load_workspace( wSpace );
         text_to_olc( olc, "You load all the instances in %s.\r\n", wSpace->name );
         return;
      case SEL_PROJECT:
         project = (PROJECT *)retrieve_entity_selection();
         full_load_project( project );
         text_to_olc( olc, "You load all the instances in %s.\r\n", project->name );
         free_project( project );
         return;
      case SEL_STRING:
         text_to_olc( olc, (char *)retrieve_entity_selection(), arg );
         return;
   }

   if( olc->using_workspace )
      add_instance_to_workspace( instance, olc->using_workspace );

   text_to_olc( olc, "You completely load %s.\r\n", instance_name( instance ) );
   return;
}

void olc_chat( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Chat what?\r\n" );
      return;
   }

   communicate( CHAT_LEVEL, olc->account, arg );
   olc->account->socket->bust_prompt = NO_PROMPT;
   text_to_olc( olc, ":> " );
   return;
}

void olc_ufilter( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE_FILTER *filter = olc->using_filter;
   GRAB_PARAMS *spec_filters = filter->spec_filters;
   char input[MAX_BUFFER], buf[MAX_BUFFER];
   char *input_ptr = input;
   int x;

   /* factor this */
   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "/%s\\\r\n", print_header( "Using Workspace Spec Filters", "-", olc->account->pagewidth - 2 ) );
      text_to_olc( olc, "|%s|\r\n", print_header( quick_format( "[%s] No Exits [%s] No Objects [%s] No Rooms [%s] No Mobs", 
         spec_filters->no_exits ? "X" : " ", spec_filters->no_objects ? "X": " ", spec_filters->no_rooms ? "X" : " ",
         spec_filters->no_mobiles ? "X" : " " ), "-", olc->account->pagewidth - 2 ) );
      text_to_olc( olc, "\\%s/\r\n", print_bar( "-", olc->account->pagewidth - 2 ) );
      if( filter->name_count > 0 )
         for( x = 0; x < filter->name_count; x++ )
            text_to_olc( olc, "Name Filter : \"%s\"\r\n", filter->filter_name[x] );
      if( filter->short_count > 0 )
         for( x = 0; x < filter->short_count; x++ )
            text_to_olc( olc, "Short Filter : \"%s\"\r\n", filter->filter_short[x] );
      if( filter->long_count > 0 )
         for( x = 0; x < filter->long_count; x++ )
            text_to_olc( olc, "Long Filter : \"%s\"\r\n", filter->filter_long[x] );
      if( filter->desc_count > 0 )
         for( x = 0; x < filter->desc_count; x++ )
            text_to_olc( olc, "Desc Filter : \"%s\"\r\n", filter->filter_desc[x] );
      text_to_olc( olc, "\r\n%s\r\n", print_header( "Proper Use", "-", olc->account->pagewidth ) );
      text_to_olc( olc, " Type ufilter <flag> - takes comma lists\r\n" );
      text_to_olc( olc, " Repeat To Remove.\r\n" );
      text_to_olc( olc, " Flags - no_exit, no_objects, no_rooms, no_mobs\r\n" );
      text_to_olc( olc, "       - limit <number> max number to show. (0 = unlimited)\r\n" );
      text_to_olc( olc, "       - name <keyword(s) to filter>\r\n" );
      text_to_olc( olc, "       - short <keyword(s) to filter>\r\n" );
      text_to_olc( olc, "       - long <keyword(s) to filter>\r\n" );
      text_to_olc( olc, "       - desc <keyword(s) to filter>\r\n" );
      text_to_olc( olc, "\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( !strcasecmp( arg, "reset" ) )
   {
      olc->using_filter = reset_wfilter( olc->using_filter );
      text_to_olc( olc, "Using Filter reset.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   while( arg && arg[0] != '\0' )
   {
      arg = one_arg_delim( arg, input, ',' );
      input_ptr = one_arg( input, buf );

      if( !strcmp( buf, "no_exits" ) )
      {
         toggle_no_exit( olc );
         continue;
      }
      if( !strcmp( buf, "no_objects" ) )
      {
         toggle_no_objects( olc );
         continue;
      }
      if( !strcmp( buf, "no_rooms" ) )
      {
         toggle_no_rooms( olc );
         continue;
      }
      if( !strcmp( buf, "no_mobs" ) )
      {
         toggle_no_mobiles( olc );
         continue;
      }
      if( !strcmp( buf, "limit" ) )
      {
         set_limit_filter( olc, input_ptr );
         continue;
      }

      if( !strcmp( buf, "name" ) )
      {
         toggle_name_filter( olc, input_ptr );
         continue;
      }
      if( !strcmp( buf, "short" ) )
      {
         toggle_short_filter( olc, input_ptr );
         continue;
      }
      if( !strcmp( buf, "long" ) )
      {
         toggle_long_filter( olc, input_ptr );
         continue;
      }
      if( !strcmp( buf, "desc" ) )
      {
         toggle_desc_filter( olc, input_ptr );
         continue;
      }
      text_to_olc( olc, "Improper usage: %s %s\r\n", buf, input_ptr );
      olc_short_prompt( olc );
   }
   return;
}

void olc_list( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   char buf[MAX_BUFFER];
   char type;
   int start, end;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "%s\r\n", print_header( "Proper Use", "-", olc->account->pagewidth ) );
      text_to_olc( olc, " To List All Frameworks: list f\r\n" );
      text_to_olc( olc, " To List All Instances:  list i\r\n" );
      text_to_olc( olc, " To List All Workspaces: list w\r\n" );
      text_to_olc( olc, " To List All Projects:   list p\r\n" );
      text_to_olc( olc, " To List All Stat:       list s\r\n" );
      text_to_olc( olc, " List also takes ranges: (example)list i5-10\r\n" );
      text_to_olc( olc, " List also takes comma lists and no_spec flags.\r\n" );
      text_to_olc( olc, " (Note: Only Frameworks, Instances and Workspaces take range.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   while( arg && arg[0] != '\0' )
   {
      arg = one_arg_delim( arg, buf, ',' );

      type = tolower( buf[0] );
      if( type != 'f' && type != 'i' && type != 'w' && type != 'p' && type != 's' )
      {
         text_to_olc( olc, "%c not valid, (f)rameworks, (i)nstances, (w)orkspaces, (p)rojects and (s)tats only.\r\n", type );
         continue;
      }

      if( strlen( buf ) == 1 )
      {
         switch( tolower( buf[0] ) )
         {
            case 'f':
               show_all_frameworks_to_olc( olc );
               return;
            case 'i':
               show_all_instances_to_olc( olc );
               return;
            case 'w':
               show_all_workspaces_to_olc( olc );
               return;
            case 'p':
               show_all_projects_to_olc( olc );
               return;
            case 's':
               show_all_stats_to_olc( olc );
               return;
         }
      }

      if( !grab_range_and_type( buf, &type, &start, &end ) )
      {
         text_to_olc( olc, "Invalid range given: %s\r\n", buf );
         continue;
      }
      switch( tolower( buf[0] ) )
      {
         default: text_to_olc( olc, "Bad format: %s", buf ); continue;
         case 'f':
            show_range_frameworks_to_olc( olc, start, end );
            continue;
         case 'i':
            show_range_instances_to_olc( olc, start, end );
            continue;
         case 'w':
            show_range_workspaces_to_olc( olc, start, end );
            continue;
      }
   }
   return;
}

void olc_pak( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   char buf[MAX_BUFFER], name[MAX_BUFFER];

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Usage pak <operator> <pak_name> ...\r\n" );
      text_to_olc( olc, " - operators: add, rem, show\r\n" );
      text_to_olc( olc, " - for stats add <stat_name>\r\n" );
      text_to_olc( olc, " - for specs add <space_name> <value>\r\n" );
      text_to_olc( olc, " Notes: for specs, you must pass a value even if its zero or it will assume its a stat and likely say no such stat exists.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   arg = one_arg( arg, buf );
   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "On what pak?\r\n" );
      olc_short_prompt( olc );
      return;
   }
   arg = one_arg( arg, name );

   if( !strcasecmp( buf, "show"  ) )
   {
      text_to_olc( olc, return_pak_contents( name ) );
      olc_short_prompt( olc );
      return;
   }

   if( !strcasecmp( buf, "add" ) )
   {
      arg = one_arg( arg, buf );
      if( !arg || arg[0] == '\0' )
      {
         if( !get_stat_framework_by_name( buf ) )
         {
            text_to_olc( olc, "No such stat exists: %s\r\n", buf );
            olc_short_prompt( olc );
            return;
         }
         if( add_pak_stat( name, buf ) )
            text_to_olc( olc, "Stat %s added to Pak %s.\r\n", name, buf );
         else
            text_to_olc( olc, "That Pak likely already has that Stat.\r\n" );
         olc_short_prompt( olc );
         return;
      }
      else
      {
         int value;
         if( !is_number( arg ) )
         {
            text_to_olc( olc, "Specs take number values only.\r\n" );
            olc_short_prompt( olc );
            return;
         }
         if( match_string_table( buf, spec_table ) == -1 )
         {
            text_to_olc( olc, "No such spec exists: %s\r\n", buf );
            olc_short_prompt( olc );
            return;
         }
         value = atoi( arg );
         if( add_pak_spec( name, buf, value ) )
            text_to_olc( olc, "Spec %s added/updated to Pak %s with the value of %d.\r\n", buf, name, value );
         else
            text_to_olc( olc, "There is something wrong with the database.\r\n" );
         olc_short_prompt( olc );
         return;
      }
   }

   if( !strcasecmp( buf, "rem" ) )
   {
      if( !arg || arg[0] == '\0' )
      {
         text_to_olc( olc, "Remove which stat or spec?\r\n" );
         olc_short_prompt( olc );
         return;
      }
      if( rem_pak_entry( name, arg ) )
         text_to_olc( olc, "Pak Entry removed.\r\n" );
      else
         text_to_olc( olc, "There was an issue with the database.\r\n" );
      olc_short_prompt( olc );
      return;
   }
   return;
}

void olc_delete( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance;
/*   ENTITY_FRAMEWORK *frame;
   PROJECT *project;
   WORKSPACE *wSpace; comment to avoid warnings for now */

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Delete what?\r\n" );
      return;
   }

   if( !interpret_entity_selection( arg ) )
   {
      text_to_olc( olc, STD_SELECTION_ERRMSG_PTR_USED );
      olc_short_prompt( olc );
      return;
   }

   switch( input_selection_typing )
   {
      default:
         clear_entity_selection();
         text_to_olc( olc, "Invalid selection type, frame, instance, workspace and project only.\r\n" );
         return;
      case SEL_FRAME:
      case SEL_WORKSPACE:
      case SEL_PROJECT:
         text_to_olc( olc, "Instances only for the moment.\r\n" );
         return;
      case SEL_INSTANCE:
         instance = (ENTITY_INSTANCE *)retrieve_entity_selection();
         text_to_olc( olc, "You delete %s from existence.\r\n", instance_name( instance ) );
         delete_eInstance( instance );
         return;
   }
}
