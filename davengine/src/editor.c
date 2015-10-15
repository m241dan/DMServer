/* editor.c: methods pertaining to the Editors written by Davenge */

#include "mud.h"

void editor_global_return( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   E_CHAIN *link;
   ITERATOR Iter;

   if( SizeOfList( olc->chain ) <= 0 )
   {
      text_to_olc( olc, "You have nothing to return to.\r\n" );
      return;
   }

   AttachIterator( &Iter, olc->chain );
   link = (E_CHAIN *)NextInList( &Iter );
   DetachIterator( &Iter );

   DetachFromList( link, olc->chain );
   free_editor( olc );
   switch( link->state )
   {
      default: break;
      case STATE_EFRAME_EDITOR:
         boot_eFramework_editor( olc, (ENTITY_FRAMEWORK *)link->to_edit );
         break;
      case STATE_EINSTANCE_EDITOR:
         boot_instance_editor( olc, (ENTITY_INSTANCE *)link->to_edit );
         break;
      case STATE_WORKSPACE_EDITOR:
         boot_workspace_editor( olc, (WORKSPACE *)link->to_edit );
         break;
      case STATE_PROJECT_EDITOR:
         boot_project_editor( olc, (PROJECT *)link->to_edit );
         break;
      case STATE_SFRAME_EDITOR:
         boot_sFramework_editor( olc, (STAT_FRAMEWORK *)link->to_edit );
         break;
   }
   free_link( link );
   return;
}

void editor_switch( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   E_CHAIN *link;
   void *to_edit;
   int type;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Switch to what?\r\n" );
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

   if( type == SEL_STRING )
   {
      text_to_olc( olc, (char *)to_edit );
      return;
   }

   link = make_editor_chain_link( olc->editing, olc->editing_state );
   add_link_to_chain( link, olc->chain );
   free_editor( olc );

   switch( type )
   {
      default: return;
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
   }
   return;
}

int free_editor( INCEPTION *olc )
{
   int ret = RET_SUCCESS;

   olc->editing = NULL;
   free_command_list( olc->editor_commands );
   FreeList( olc->editor_commands );
   olc->editor_commands = NULL;
   olc->editing_state = olc->account->socket->prev_state;

   return ret;
}

int init_eFramework_editor( INCEPTION *olc, ENTITY_FRAMEWORK *frame )
{
   int ret = RET_SUCCESS;

   if( !frame )
      olc->editing = init_eFramework();
   else
      olc->editing = frame;

   olc->editing_state = STATE_EFRAME_EDITOR;
   text_to_olc( olc, "Opening the Framework Editor...\r\n" );
   olc->editor_commands = AllocList();

   return ret;
}

void boot_eFramework_editor( INCEPTION *olc, ENTITY_FRAMEWORK *frame )
{
   int state = olc->account->socket->state;

   if( state < STATE_EFRAME_EDITOR || state > STATE_PROJECT_EDITOR )
      olc->editor_launch_state = state;
   init_eFramework_editor( olc, frame );
   olc->editing_state = STATE_EFRAME_EDITOR;
   change_socket_state( olc->account->socket, olc->editing_state );
   editor_eFramework_info( olc->account->socket );
   get_commands( olc->account->socket );
   text_to_olc( olc, "Type -> /info || /commands for help.\r\n" );
   return;
}

void editor_eFramework_info( D_SOCKET *socket )
{
   ENTITY_FRAMEWORK *frame;
   INCEPTION *olc;
   char tempstring[MAX_BUFFER];
   const char *border = "|";
   int space_after_pipes;

   if( !socket->account || !socket->account->olc )
   {
      text_to_socket( socket, "You have a NULL account or olc...\r\n" );
      return;
   }
   olc = socket->account->olc;
   if( ( frame = (ENTITY_FRAMEWORK *)socket->account->olc->editing ) == NULL )
   {
      bug( "%s: not editing anything...", __FUNCTION__ );
      return;
   }

   space_after_pipes = socket->account->pagewidth - ( strlen( border ) * 2 );
   if( !strcmp( frame->tag->created_by, "null" ) )
      mud_printf( tempstring, "Potential Framework ID: %d", get_potential_id( frame->tag->type ) );
   else
      mud_printf( tempstring, "Framework ID: %d", frame->tag->id );

   if( frame->inherits )
      mudcat( tempstring, quick_format( "| Inherits from %s ID: %d", chase_name( frame->inherits ), frame->inherits->tag->id ) );

   text_to_olc( olc, "/%s\\\r\n", print_header( tempstring, "-", space_after_pipes ) );
   text_to_olc( olc, "%s", return_framework_strings( frame, border, socket->account->pagewidth ) );
   text_to_olc( olc, "\\%s/\r\n\r\n", print_bar( "-", space_after_pipes ) );

   if( SizeOfList( frame->fixed_contents ) > 0 || inherited_frame_has_any_fixed_possession( frame ) )
   {
      text_to_olc( olc, "/%s\\\r\n", print_header( "Fixed Contents", "-", space_after_pipes ) );
      text_to_olc( olc, "%s", return_framework_fixed_content( frame, border, socket->account->pagewidth ) );
      text_to_olc( olc, "\\%s/\r\n", print_bar( "-", space_after_pipes ) );
   }

   if( SizeOfList( frame->specifications ) > 0 || inherited_frame_has_any_spec( frame ) )
      text_to_olc( olc, "%s", return_framework_specs( frame, socket->account->pagewidth ) );

   if( SizeOfList( frame->stats ) > 0 || inherited_frame_has_any_stats( frame ) )
      text_to_olc( olc, "%s", return_framework_stats( frame, socket->account->pagewidth ) );

   text_to_olc( olc, "\r\n" );

   return;
}

int editor_eFramework_prompt( D_SOCKET *dsock, bool commands )
{
   ENTITY_FRAMEWORK *frame;
   INCEPTION *olc;
   char tempstring[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !dsock->account || !dsock->account->olc )
   {
      text_to_buffer( dsock, "Bad Prompt:>> " );
      return RET_SUCCESS;
   }

   olc = dsock->account->olc;
   frame = (ENTITY_FRAMEWORK *)olc->editing;

   if( !strcmp( frame->tag->created_by, "null" ) )
      mud_printf( tempstring, "Potential Framework ID: %d", get_potential_id( frame->tag->type ) );
   else
      mud_printf( tempstring, "Framework ID: %d", frame->tag->id );

   if( frame->inherits )
      mudcat( tempstring, quick_format( "\r\nInherits  ID: %d", frame->inherits->tag->id ) );

   text_to_olc( olc, "%s ||> ", tempstring );
   return ret;
}

const char *return_framework_strings( ENTITY_FRAMEWORK *frame, const char *border, int width )
{
   STAT_FRAMEWORK *fstat;
   static char buf[MAX_BUFFER];
   char tempstring[MAX_BUFFER];
   int space_after_border;
   int value, source;

   memset( &buf[0], 0, sizeof( buf ) );
   space_after_border = width - ( strlen( border ) * 2 );

   mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " Name : %s%s", chase_name( frame ), !strcmp( frame->name, "_inherited_" ) ? " ( inherited )" : "" ),
      space_after_border ),
      border );

   mudcat( buf, tempstring );

   mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " Short : %s%s", chase_short_descr( frame ), !strcmp( frame->short_descr, "_inherited_" ) ? " ( inherited )" : "" ),
      space_after_border ),
      border );

   mudcat( buf, tempstring );


   mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " Long : %s%s", chase_long_descr( frame ), !strcmp( frame->long_descr, "_inherited_" ) ? " ( inherited )" : "" ),
      space_after_border ),
      border );

   mudcat( buf, tempstring );

   mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " Desc : %s%s", chase_description( frame ), !strcmp( frame->description, "_inherited_" ) ? " ( inherited )" : "" ),
      space_after_border ),
      border );

   mudcat( buf, tempstring );

   source = 0;
   if( ( fstat = get_primary_dmg_stat_from_framework( frame, &source ) ) != NULL )
   {
      mud_printf( tempstring, "%s%s%s\r\n", border,
         fit_string_to_space(
         quick_format( " Primary Dmg Stat : %s%s", fstat->name, source == 1 ? "( inherited )" : "" ),
         space_after_border ),
         border );

      mudcat( buf, tempstring );
   }

   source = 0;
   if( ( value = get_frame_tspeed( frame, &source ) ) != 0 )
   {
      mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " Thought Speed : %d%s", value, source == 1 ? "( inherited )" : "" ),
      space_after_border ),
      border );

      mudcat( buf, tempstring );
   }

   source = 0;
   if( ( value = get_frame_height( frame, &source ) ) != 0 )
   {
      mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " Height : %d%s", value, source == 1 ? "( inherited )" : "" ),
      space_after_border ),
      border );
   }

   source = 0;
   if( ( value = get_frame_weight( frame, &source ) ) != 0 )
   {
      mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " Weight : %d%s", value, source == 1 ? "( inherited )" : "" ),
      space_after_border ),
      border );
   }

   source = 0;
   if( ( value = get_frame_width( frame, &source ) ) != 0 )
   {
      mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " Width : %d%s", value, source == 1 ? "( inherited )" : "" ),
      space_after_border ),
      border );
   }

   source = 0;
   if( ( value = get_frame_spawn_time( frame, &source ) ) != 0 )
   {
      mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " If killed, will respawn in %d%s seconds", value, source == 1 ? "( inherited )": "" ),
      space_after_border ),
      border );

      mudcat( buf, tempstring );
   }

   buf[strlen( buf )] = '\0';
   return buf;
}

const char *return_framework_fixed_content( ENTITY_FRAMEWORK *frame, const char *border, int width )
{
   static char buf[MAX_BUFFER];

   memset( &buf[0], 0, sizeof( buf ) );

   mudcat( buf, return_fixed_content_list( frame->fixed_contents, border, width, FALSE ) );
   while( ( frame = frame->inherits ) != NULL )
      mudcat( buf, return_fixed_content_list( frame->fixed_contents, border, width, TRUE ) );

   buf[strlen( buf )] = '\0';
   return buf;
}

const char *return_fixed_content_list( LLIST *fixed_list, const char *border, int width, bool inherited )
{
   ENTITY_FRAMEWORK *fixed_content;
   ITERATOR Iter;
   static char buf[MAX_BUFFER];
   char tempstring[MAX_BUFFER];
   int space_after_border;

   memset( &buf[0], 0, sizeof( buf ) );
   space_after_border = width - ( strlen( border ) * 2 );

   AttachIterator( &Iter, fixed_list );
   while( ( fixed_content = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space( quick_format( "(%-7d) %s, %s%s", fixed_content->tag->id, chase_name( fixed_content ), chase_short_descr( fixed_content ), inherited ? " (inherited)" : "" ), space_after_border ), border );
      mudcat( buf, tempstring );
   }
   DetachIterator( &Iter );

   buf[strlen( buf )] = '\0';
   return buf;
}

const char *return_framework_specs( ENTITY_FRAMEWORK *frame, int width )
{
   const char *const spec_from_table[] = {
      "", "", "( i )"
   };
   SPECIFICATION *spec;
   LLIST *spec_strings;
   ITERATOR Iter;
   char *spec_string;
   static char buf[MAX_BUFFER];
   int x, longest, spec_string_len, spec_from;
   int num_column, column_count = 0;
   /* going to get three spec to space */

   spec_strings = AllocList();
   mud_printf( buf, "%s\r\n", print_header( "Specifications", "-", width ) );

   for( longest = 0, x = MAX_SPEC; x >= 0; x--)
   {
      spec_from = 0;
      if( ( spec = frame_has_spec_detailed_by_type( frame, x, &spec_from ) ) == NULL )
         continue;
      CREATE( spec_string, char, MAX_BUFFER );
      mud_printf( spec_string, "%s : %d%s ", spec_table[spec->type], spec->value, spec_from_table[spec_from] );
      if( ( spec_string_len = strlen( spec_string ) ) > longest )
         longest = spec_string_len;
      AttachToList( spec_string, spec_strings );
   }

   num_column = floor( (double)width / (double)longest );

   AttachIterator( &Iter, spec_strings );
   while( ( spec_string = (char *)NextInList( &Iter ) ) != NULL )
   {
      mudcat( buf, fit_string_to_space( spec_string, longest ) );
      if( ++column_count >= num_column )
      {
         mudcat( buf, "\r\n" );
         column_count = 0;
      }
   }
   DetachIterator( &Iter );
   if( column_count != 0 )
      mudcat( buf, "\r\n" );

   FreeList( spec_strings );
   return buf;
}

const char *return_framework_stats( ENTITY_FRAMEWORK *frame, int width )
{
   const char *const stat_from_table[] = {
      "", "( i )"
   };
   STAT_FRAMEWORK *sframe;
   LLIST *stat_strings;
   ITERATOR Iter;
   char *stat_string;
   static char buf[MAX_BUFFER];
   int x, longest, stat_string_len, stat_from;
   int num_column, column_count = 0;

   stat_strings = AllocList();
   mud_printf( buf, "%s\r\n", print_header( "Stats", "-", width ) );

   for( longest = 0, ( x = get_potential_id( ENTITY_STAT_FRAMEWORK_IDS ) - 1 ); x >= 0; x-- )
   {
      stat_from = 0;
      if( ( sframe = get_stat_from_framework_by_id( frame, x, &stat_from ) ) == NULL )
         continue;
      if( sframe == frame->f_primary_dmg_received_stat )
         continue;
      CREATE( stat_string, char, MAX_BUFFER );
      mud_printf( stat_string, "%s%s", sframe->name, stat_from_table[stat_from] );
      if( SizeOfList( stat_strings ) != 0 )
         mudcat( stat_string, ", " );
      else
         mudcat( stat_string, " " );
      if( ( stat_string_len = strlen( stat_string ) ) > longest )
         longest = stat_string_len;
      AttachToList( stat_string, stat_strings );
   }

   num_column = floor( (double)width / (double)longest );

   AttachIterator( &Iter, stat_strings );
   while( ( stat_string = (char *)NextInList( &Iter ) ) != NULL )
   {
      mudcat( buf, fit_string_to_space( stat_string, longest ) );
      if( ++column_count >= num_column )
      {
         mudcat( buf, "\r\n" );
         column_count = 0;
      }
   }
   DetachIterator( &Iter );
   if( column_count != 0 )
      mudcat( buf, "\r\n" );

   FreeList( stat_strings );
   return buf;
}

const char *return_framework_specs_and_stats( ENTITY_FRAMEWORK *frame, const char *border, int width )
{
   const char *const spec_from_table[] = {
      "", "", "( inherited )"
   };
   const char *const stat_from_table[] = {
      "", "( inherited )"
   };
   SPECIFICATION *spec;
   STAT_FRAMEWORK *fstat;
   static char buf[MAX_BUFFER];
   char tempstring[MAX_BUFFER];
   int space_after_border;
   int x, y, spec_from, MAX_STAT, stat_from;

   space_after_border = width - ( strlen( border ) * 2 );

   mud_printf( buf, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );
   space_after_border = width - ( strlen( border ) * 3 );
   mud_printf( tempstring, "%s%s", border, print_header( "Specifications", " ", space_after_border / 2 ) );
   mudcat( buf, tempstring );
   mudcat( buf, border );
   mud_printf( tempstring, " %s%s\r\n", print_header( "Stats", " ", space_after_border / 2 ), border );
   mudcat( buf, tempstring );
   space_after_border = width - ( strlen( border ) * 2 );
   mud_printf( tempstring, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );
   mudcat( buf, tempstring );

   /* later when stats are in it will look like ;x < MAX_SPEC || y < MAX_STAT; */
   MAX_STAT = get_potential_id( ENTITY_STAT_FRAMEWORK_IDS );

   for( x = 0, y = 0, spec_from = 0, stat_from = 0; x < MAX_SPEC || y < MAX_STAT; )
   {
      spec = NULL;
      fstat = NULL;
      if( x < MAX_SPEC )
        while( ( spec = frame_has_spec_detailed_by_type( frame, x++, &spec_from ) ) == NULL && x < MAX_SPEC );
      if( y < MAX_STAT )
         while( ( ( fstat = get_stat_from_framework_by_id( frame, y++, &stat_from ) ) == NULL && y < MAX_STAT ) || ( fstat != NULL && fstat == frame->f_primary_dmg_received_stat ) );

      if( !spec && !fstat )
         break;
      if( spec )
         mud_printf( tempstring, "%s%s", border, fit_string_to_space( quick_format( " %s : %d%s", spec_table[spec->type], spec->value, spec_from_table[spec_from] ), ( space_after_border / 2 ) - 1 ) );
      else
         mud_printf( tempstring, "%s%s", border, print_header( " ", " ", ( space_after_border / 2 ) - 1 ) );
      mudcat( buf, tempstring );

      mudcat( buf, border );

      if( fstat )
         mud_printf( tempstring, " %s%s\r\n", print_header( quick_format( " %s %s", fstat->name, stat_from_table[stat_from] ), " ", ( space_after_border / 2 ) - 1 ), border );
      else
         mud_printf( tempstring, " %s%s\r\n", print_header( " ", " ", ( space_after_border / 2 ) - 1 ), border );

      mudcat( buf, tempstring );

   }
   buf[strlen( buf )] = '\0';
   return buf;

}

void eFramework_name( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the name to what?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( strlen( arg ) > MAX_FRAMEWORK_NSL )
   {
      text_to_olc( olc, "%s is too long.\r\n", arg );
      return;
   }
   if( ( frame = (ENTITY_FRAMEWORK *)olc->editing ) == NULL )
   {
      text_to_olc( olc, "You aren't actually editing anything...\r\n" );
      change_socket_state( olc->account->socket, STATE_OLC );
      return;
   }

   set_frame_name( frame, arg );
   update_tag( frame->tag, olc->account->name );
   text_to_olc( olc, "Name changed.\r\n" );
   return;
}

void eFramework_short( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the short description to what?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( strlen( arg ) > MAX_FRAMEWORK_NSL )
   {
      text_to_olc( olc, "%s is too long.\r\n", arg );
      return;
   }
   if( ( frame = (ENTITY_FRAMEWORK*)olc->editing ) == NULL )
   {
      text_to_olc( olc, "You aren't actually editing anything...\r\n" );
      change_socket_state( olc->account->socket, STATE_OLC );
      return;
   }
   set_frame_short_descr( frame, arg );
   update_tag( frame->tag, olc->account->name );
   text_to_olc( olc, "Short description changed.\r\n" );
   return;
}

void eFramework_long( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the long description to what?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( strlen( arg ) > MAX_FRAMEWORK_NSL )
   {
      text_to_olc( olc, "%s is too long.\r\n", arg );
      olc_short_prompt( olc );
      return;
   }
   if( ( frame = (ENTITY_FRAMEWORK*)olc->editing ) == NULL )
   {
      text_to_olc( olc, "You aren't actually editing anything...\r\n" );
      change_socket_state( olc->account->socket, STATE_OLC );
      return;
   }

   set_frame_long_descr( frame, arg );
   update_tag( frame->tag, olc->account->name );
   text_to_olc( olc, "Long description changed.\r\n" );
   return;
}

void eFramework_description( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the description to what?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( strlen( arg ) > MAX_BUFFER )
   {
      text_to_olc( olc, "%s is too long.\r\n", arg );
      olc_short_prompt( olc );
      return;
   }
   if( ( frame = (ENTITY_FRAMEWORK*)olc->editing ) == NULL )
   {
      text_to_olc( olc, "You aren't actually editing anything...\r\n" );
      change_socket_state( olc->account->socket, STATE_OLC );
      return;
   }

   set_frame_description( frame, arg );
   update_tag( frame->tag, olc->account->name );
   text_to_olc( olc, "Long description changed.\r\n" );
   return;
}

void eFramework_set_tspeed( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;
   int value;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the thought speed in quarter seconds. So, 4 = 1 second.\r\n" );
      return;
   }

   if( !is_number( arg ) )
   {
      text_to_olc( olc, "Thought speeds must be numbers.\r\n" );
      return;
   }
   value = atoi( arg );
   if( value < -1 || value == 0 )
   {
      text_to_olc( olc, "Thought speeds must be greater than zero or negative one for inheritance.\r\n" );
      return;
   }

   set_frame_tspeed( frame, value );
   update_tag( frame->tag, olc->account->name );
   text_to_olc( olc, "Thought speed set.\r\n" );
   return;
}

void eFramework_set_spawn_time( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;
   int value;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the respawn time in seconds. Zero means the instance does not respawn.\r\n" );
      return;
   }

   if( !is_number( arg ) )
   {
      text_to_olc( olc, "Spawn timers must be numbers.\r\n" );
      return;
   }
   value = atoi( arg );
   if( value < -1 )
   {
      text_to_olc( olc, "Spawn timers must be 0 or greater.\r\n - Note: Zero means it won't respawn.\r\n - Note: negative one for inheritance" );
      return;
   }
   set_frame_spawn_time( frame, value );
   update_tag( frame->tag, olc->account->name );
   text_to_olc( olc, "Spawn time set.\r\n" );
   return;
}

void eFramework_set_height( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;
   int value;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the height to what?\r\n" );
      return;
   }
   if( !is_number( arg ) )
   {
      text_to_olc( olc, "Height must be a number.\r\n" );
      return;
   }
   value = atoi( arg );
   if( value < -1 )
   {
      text_to_olc( olc, "Height must be zero or greater. Negative one denotes inheritance.\r\n" );
      return;
   }
   set_frame_height( frame, value );
   update_tag( frame->tag, olc->account->name );
   text_to_olc( olc, "Height set.\r\n" );
   return;
}

void eFramework_set_weight( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;
   int value;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the weight to what?\r\n" );
      return;
   }
   if( !is_number( arg ) )
   {
      text_to_olc( olc, "Weight must be a number.\r\n" );
      return;
   }
   value = atoi( arg );
   if( value < -1 )
   {
      text_to_olc( olc, "Height must be zero or greater. Negative one denotes inheritance.\r\n" );
      return;
   }
   set_frame_weight( frame, value );
   update_tag( frame->tag, olc->account->name );
   text_to_olc( olc, "Weight set.\r\n" );
   return;
}

void eFramework_set_width( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;
   int value;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the width to what?\r\n" );
      return;
   }
   if( !is_number( arg ) )
   {
      text_to_olc( olc, "Width must be a number.\r\n" );
      return;
   }
   value = atoi( arg );
   if( value < 0 )
   {
      text_to_olc( olc, "Width must be zero or greater, Negative one denotes inheritance.\r\n" );
      return;
   }
   set_frame_width( frame, value );
   update_tag( frame->tag, olc->account->name );
   text_to_olc( olc, "Width set.\r\n" );
   return;
}

void eFramework_addStat( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;
   STAT_FRAMEWORK *fstat;
   int spec_from;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Add what stat?\r\n" );
      return;
   }

   if( is_number( arg ) )
      fstat = get_stat_framework_by_id( atoi( arg ) );
   else
      fstat = get_stat_framework_by_name( arg );

   if( !fstat )
   {
      text_to_olc( olc, "No such stat.\r\n" );
      return;
   }

   if( fstat == get_stat_from_framework_by_id( frame, fstat->tag->id, &spec_from ) )
   {
      text_to_olc( olc, "Frame already has stat %s on %s.\r\n", fstat->name, spec_from == 0 ? "it" : "its inheritance" );
      return;
   }

   add_stat_to_frame( fstat, frame );
   update_tag( frame->tag, olc->account->name );
   text_to_olc( olc, "Stat added.\r\n" );
   return;
}

void eFramework_addSpec( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;
   SPECIFICATION *spec;
   char spec_arg[MAX_BUFFER];
   int spec_type, spec_value;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Valid Specs: %s.\r\n", print_string_table( spec_table ) );
      return;
   }

   arg = one_arg( arg, spec_arg );

   if( ( spec_type = match_string_table_no_case( spec_arg, spec_table ) ) == -1 )
   {
      text_to_olc( olc, "Invalid Spec Type.\r\n" );
      return;
   }

   if( !arg || arg[0] == '\0' )
   {
     text_to_olc( olc, "Spec Value Defaulting to 1.\r\n" );
     spec_value = 1;
   }
   else if( !is_number( arg ) )
   {
      text_to_olc( olc, "Spec Value must be a number.\r\n" );
      return;
   }
   else
      spec_value = atoi( arg );

   if( ( spec = spec_list_has_by_type( frame->specifications, spec_type ) ) != NULL )
      text_to_olc( olc, "Override current %s specification who's value was %d.\r\n", spec_table[spec->type], spec->value );
   else
      spec = init_specification();

   spec->type = spec_type;
   spec->value = spec_value;
   add_spec_to_framework( spec, frame );
   update_tag( frame->tag, olc->account->name );
   text_to_olc( olc, "%s added to %s with the value of %s.\r\n", spec_table[spec_type], frame->name, itos( spec->value ) );
   return;

}

void eFramework_remSpec( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;
   SPECIFICATION *spec;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "What spec do you want to remove?\r\n" );
      return;
   }

   if( ( spec = spec_list_has_by_name( frame->specifications, arg ) ) == NULL )
   {
      text_to_olc( olc, "This framework does not that Spec.\r\n" );
      return;
   }
   rem_spec_from_framework( spec, frame );
   text_to_olc( olc, "Spec removed.\r\n" );
   return;
}

void eFramework_done( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;

   if( !strcmp( frame->tag->created_by, "null" ) )
   {
      new_tag( frame->tag, olc->account->name );
      new_eFramework( frame );
      if( olc->using_workspace )
         add_frame_to_workspace( frame, olc->using_workspace );
   }
   if( SizeOfList( olc->chain ) > 0 )
      clearlist( olc->chain );
   free_editor( olc );
   change_socket_state( olc->account->socket, olc->editor_launch_state );
   text_to_olc( olc, "Exiting Entity Framework Editor.\r\n" );
   olc_show_prompt( olc );
   return;
}

void eFramework_save( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;

   if( !strcmp( frame->tag->created_by, "null" ) )
   {
      new_tag( frame->tag, olc->account->name );
      new_eFramework( frame );
      if( olc->using_workspace )
         add_frame_to_workspace( frame, olc->using_workspace );
   }
   text_to_olc( olc, "Saved.\r\n" );
   return;
}

void eFramework_addContent( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;
   ENTITY_FRAMEWORK *frame_to_add;
   int value;

   if( check_selection_type( arg ) != SEL_FRAME )
   {
      if( is_number( arg ) )
      {
         value = atoi( arg );
         if( ( frame_to_add = get_framework_by_id( value ) ) == NULL )
         {
            text_to_olc( olc, "There's no framework with the ID of %d.\r\n", value );
            return;
         }
      }
      else if( ( frame_to_add =  get_framework_by_name( arg ) ) == NULL )
      {
         text_to_olc( olc, "There's no framework with the name %s.\r\n", arg );
         return;
      }
   }
   else
   {
      if( !interpret_entity_selection( arg ) )
      {
         text_to_olc( olc, STD_SELECTION_ERRMSG_PTR_USED );
         return;
      }
      switch( input_selection_typing )
      {
         default: clear_entity_selection(); return;
         case SEL_FRAME:
            frame_to_add = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
            break;
         case SEL_STRING:
            text_to_olc( olc, (char *)retrieve_entity_selection(), arg );
            return;
      }
   }
   add_frame_to_fixed_contents( frame_to_add, frame );
   update_tag( frame->tag, olc->account->name );
   text_to_olc( olc, "%s added to %s's fixed contents.\r\n", chase_name( frame_to_add ), chase_name( frame ) );
   return;
}

void eFramework_script( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;

   if( !strcmp( frame->tag->created_by, "null" ) )
   {
      text_to_olc( olc, "This framework must completely exist before you can start scripting it, save or done and reopen.\r\n" );
      olc_short_prompt( olc );
      return;
   }
   text_to_olc( olc, "%s", print_f_script( frame ) );
   return;
}

void eFramework_addPak( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Add what Pak?\r\n" );
      olc_short_prompt( olc );
      return;
   }
   load_pak_on_framework( arg, frame );
   text_to_olc( olc, "You load the Pak.\r\n" );
   return;
}

void eFramework_setPrimaryDmg( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;
   STAT_FRAMEWORK *fstat;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the Primary Damage Stat to what?\r\n" );
      olc_short_prompt( olc );
      return;
   }
   if( ( fstat = get_stat_framework_by_name( arg ) ) == NULL )
   {
      text_to_olc( olc, "%s not a Stat.\r\n", arg );
      olc_short_prompt( olc );
      return;
   }
   if( !fstat->pool )
   {
      text_to_olc( olc, "Registered Stats cannot be primary damage stats. Only Pool type stats allowed.\r\n" );
      olc_short_prompt( olc );
      return;
   }
   if( fstat == frame->f_primary_dmg_received_stat )
   {
      text_to_olc( olc, "Primary Damage Stat is already set to %s.\r\n", fstat->name );
      return;
   }

   set_primary_dmg_stat_framework( frame, fstat );
   text_to_olc( olc, "You set the Primary Damage Stat to %s.\r\n", frame->f_primary_dmg_received_stat->name );
   return;
}

int init_project_editor( INCEPTION *olc, PROJECT *project )
{
   int ret = RET_SUCCESS;

   if( !project )
      olc->editing = init_eFramework();
   else
      olc->editing = project;

   olc->editing_state = STATE_PROJECT_EDITOR;
   text_to_olc( olc, "Opening the Project Editor...\r\n" );
   olc->editor_commands = AllocList();

   return ret;
}

void boot_project_editor( INCEPTION *olc, PROJECT *project )
{
   int state = olc->account->socket->state;

   if( state < STATE_EFRAME_EDITOR || state > STATE_PROJECT_EDITOR )
      olc->editor_launch_state = state;
   init_project_editor( olc, project );
   olc->editing_state = STATE_PROJECT_EDITOR;
   change_socket_state( olc->account->socket, olc->editing_state );
   return;
}


int editor_project_prompt( D_SOCKET *dsock, bool commands )
{
   INCEPTION *olc;
   BUFFER *buf = buffer_new( MAX_BUFFER );
   PROJECT *project;
   const char *border = "|";
   char tempstring[MAX_BUFFER];
   int space_after_border;
   int ret = RET_SUCCESS;

   project = (PROJECT *)dsock->account->olc->editing;
   olc = dsock->account->olc;
   space_after_border = dsock->account->pagewidth - ( strlen( border ) * 2 );

   if( !strcmp( project->tag->created_by, "null" ) )
      mud_printf( tempstring, "Potential Project ID: %d", get_potential_id( project->tag->type ) );
   else
      mud_printf( tempstring, "Project ID: %d", project->tag->id );

   text_to_olc( olc, "/%s\\\r\n", print_header( tempstring, "-", dsock->account->pagewidth - 2 ) );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Name   : %s", project->name ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Public : %s", project->Public ? "Yes" : "No" ), space_after_border ), border );

   if( SizeOfList( project->workspaces ) > 0 )
      text_to_olc( olc, "%s", return_project_workspaces_string( project, border, dsock->account->pagewidth ) );

   text_to_olc( olc, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );
   if( commands )
   {
      print_commands( dsock->account->olc, dsock->account->olc->editor_commands, buf, 0, dsock->account->pagewidth );
      bprintf( buf, "\\%s/\r\n", print_bar( "-", space_after_border ) );
   }
   text_to_olc( olc, buf->data );
   buffer_free( buf );
   return ret;
}

const char *return_project_workspaces_string( PROJECT *project, const char *border, int width )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;
   static char buf[MAX_BUFFER];
   char tempstring[MAX_BUFFER];
   int space_after_border;

   space_after_border = width - ( strlen( border ) * 2 );

   mud_printf( buf, "%s%s%s\r\n", border, print_header( "Workspaces", "-", space_after_border ), border );

   AttachIterator( &Iter, project->workspaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
   {
      mud_printf( tempstring, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " %s %s", wSpace->Public ? "Public :" : "Private:", wSpace->name ), space_after_border ), border );
      mudcat( buf, tempstring );
   }
   DetachIterator( &Iter );

   buf[strlen( buf )] = '\0';
   return buf;
}

void project_name( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   PROJECT *project = (PROJECT *)olc->editing;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Name it what?\r\n" );
      return;
   }

   if( strlen( arg ) > 20 )
   {
      text_to_olc( olc, "%s is too long.\r\n", arg );
      return;
   }

   FREE( project->name );
   project->name = strdup( arg );
   text_to_olc( olc, "Name changed.\r\n" );

   if( strcmp( project->tag->created_by, "null" ) )
   {
      quick_query( "UPDATE projects SET name='%s' WHERE projectID=%d;", format_string_for_sql( project->name ), project->tag->id );
      update_tag( project->tag, olc->account->name );
   }
   return;
}

void project_public( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   PROJECT *project = (PROJECT *)olc->editing;

   if( project->Public )
      project->Public = FALSE;
   else
      project->Public = TRUE;

   text_to_olc( olc, "Project changed to %s.\r\n", project->Public ? "public" : "private" );

   if( strcmp( project->tag->created_by, "null" ) )
   {
      quick_query( "UPDATE projects SET public=%d WHERE projectID=%d;", (int)project->Public, project->tag->id );
      update_tag( project->tag, olc->account->name );
   }
   return;

}

void project_done( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   PROJECT *project = (PROJECT *)olc->editing;

   if( !strcmp( project->tag->created_by, "null" ) )
   {
      new_tag( project->tag, olc->account->name );
      new_project( project );
   }
   if( SizeOfList( olc->chain ) > 0 )
      clearlist( olc->chain );
   free_editor( olc );
   change_socket_state( olc->account->socket, olc->editor_launch_state );
   text_to_olc( olc, "Exiting the Project editor.\r\n" );
   return;
}


int init_workspace_editor( INCEPTION *olc, WORKSPACE *wSpace )
{
   int ret = RET_SUCCESS;

   if( !wSpace )
      olc->editing = init_workspace();
   else
      olc->editing = wSpace;

   olc->editing_state = STATE_WORKSPACE_EDITOR;
   text_to_olc( olc, "Opening the Workspace Editor...\r\n" );
   olc->editor_commands = AllocList();

   return ret;
}

void boot_workspace_editor( INCEPTION *olc, WORKSPACE *wSpace )
{
   int state = olc->account->socket->state;

   if( state < STATE_EFRAME_EDITOR || state > STATE_PROJECT_EDITOR )
      olc->editor_launch_state = state;
   init_workspace_editor( olc, wSpace );
   olc->editing_state = STATE_WORKSPACE_EDITOR;
   change_socket_state( olc->account->socket, olc->editing_state );
   return;
}

int editor_workspace_prompt( D_SOCKET *dsock, bool commands )
{
   INCEPTION *olc;
   BUFFER *buf = buffer_new( MAX_OUTPUT );
   WORKSPACE *wSpace;
   const char *border = "|";
   char tempstring[MAX_BUFFER];
   int space_after_border;
   int ret = RET_SUCCESS;

   wSpace = (WORKSPACE *)dsock->account->olc->editing;
   olc = dsock->account->olc;
   space_after_border = dsock->account->pagewidth - ( strlen( border ) * 2 );

   if( !strcmp( wSpace->tag->created_by, "null" ) )
      mud_printf( tempstring, "Potential Workspace ID: %d", get_potential_id( wSpace->tag->type ) );
   else
      mud_printf( tempstring, "Workspace ID: %d", wSpace->tag->id );

   text_to_olc( olc, "/%s\\\r\n", print_header( tempstring, "-", dsock->account->pagewidth - 2 ) );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Name      : %s", wSpace->name ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Desc      : %s", wSpace->description ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Public    : %s", wSpace->Public ? "Yes" : "No" ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Frameworks: %d", SizeOfList( wSpace->frameworks ) ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Instances : %d", SizeOfList( wSpace->instances ) ), space_after_border ), border );

   text_to_olc( olc, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );
   if( commands )
   {
      print_commands( dsock->account->olc, dsock->account->olc->editor_commands, buf, 0, dsock->account->pagewidth );
      bprintf( buf, "\\%s/\r\n", print_bar( "-", dsock->account->pagewidth - 2 ) );
   }
   text_to_olc( olc, buf->data );

   buffer_free( buf );
   return ret;
}

void workspace_name( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE *wSpace = (WORKSPACE *)olc->editing;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Name it what?\r\n" );
      return;
   }

   if( strlen( arg ) > 20 )
   {
      text_to_olc( olc, "%s is too long.\r\n", arg );
      return;
   }

   FREE( wSpace->name );
   wSpace->name = strdup( arg );
   text_to_olc( olc, "Name changed.\r\n" );

   if( strcmp( wSpace->tag->created_by, "null" ) )
   {
      quick_query( "UPDATE workspaces SET name='%s' WHERE workspaceID=%d;", format_string_for_sql( wSpace->name ), wSpace->tag->id );
      update_tag( wSpace->tag, olc->account->name );
   }
   return;
}

void workspace_description( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE *wSpace = (WORKSPACE *)olc->editing;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Describe it as...?\r\n" );
      return;
   }

   if( strlen( arg ) > MAX_BUFFER )
   {
      text_to_olc( olc, "%s is too long.\r\n", arg );
      return;
   }

   FREE( wSpace->description );
   wSpace->description = strdup( arg );
   text_to_olc( olc, "Description changed.\r\n" );

   if( strcmp( wSpace->tag->created_by, "null" ) )
   {
      quick_query( "UPDATE workspaces SET description='%s' WHERE workspaceID=%d;", format_string_for_sql( wSpace->description ), wSpace->tag->id );
      update_tag( wSpace->tag, olc->account->name );
   }
   return;

}

void workspace_public( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE *wSpace = (WORKSPACE *)olc->editing;

   if( wSpace->Public )
      wSpace->Public = FALSE;
   else
      wSpace->Public = TRUE;

   text_to_olc( olc, "Workspace changed to %s.\r\n", wSpace->Public ? "public" : "private" );

   if( strcmp( wSpace->tag->created_by, "null" ) )
   {
      quick_query( "UPDATE workspaces SET public=%d WHERE workspaceID=%d;", (int)wSpace->Public, wSpace->tag->id );
      update_tag( wSpace->tag, olc->account->name );
   }
   return;
}

void workspace_done( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE *wSpace = (WORKSPACE *)olc->editing;

   if( !strcmp( wSpace->tag->created_by, "null" ) )
   {
      new_tag( wSpace->tag, olc->account->name );
      new_workspace( wSpace );
   }
   if( SizeOfList( olc->chain ) > 0 )
      clearlist( olc->chain );
   free_editor( olc );
   change_socket_state( olc->account->socket, olc->editor_launch_state );
   text_to_olc( olc, "Exiting the Workspace editor.\r\n" );
   return;
}

int init_instance_editor( INCEPTION *olc, ENTITY_INSTANCE *instance )
{
   int ret = RET_SUCCESS;

   if( !instance )
      olc->editing = init_eInstance();
   else
      olc->editing = instance;

   olc->editing_state = STATE_EINSTANCE_EDITOR;
   text_to_olc( olc, "Opening the Instance Editor...\r\n" );
   olc->editor_commands = AllocList();

   return ret;

}

void boot_instance_editor( INCEPTION *olc, ENTITY_INSTANCE *instance )
{
   int state = olc->account->socket->state;

   if( state < STATE_EFRAME_EDITOR || state > STATE_PROJECT_EDITOR )
      olc->editor_launch_state = state;
   init_instance_editor( olc, instance );
   olc->editing_state = STATE_EINSTANCE_EDITOR;
   change_socket_state( olc->account->socket, olc->editing_state );
   editor_instance_info( olc->account->socket );
   get_commands( olc->account->socket );
   text_to_olc( olc, "Type -> /info || /commands for help.\r\n" );
   return;
}

void editor_instance_info( D_SOCKET *socket )
{
   ENTITY_INSTANCE *instance;
   INCEPTION *olc;
   char tempstring[MAX_BUFFER];
   const char *border = "|";
   int space_after_border;

   if( !socket->account || !socket->account->olc )
   {
      text_to_socket( socket, "You have NULL account or olc...\r\n" );
      return;
   }
   olc = socket->account->olc;
   if( ( instance = (ENTITY_INSTANCE *)olc->editing ) == NULL )
   {
      bug( "%s: not editing anything...", __FUNCTION__ );
      return;
   }

   space_after_border = socket->account->pagewidth - ( strlen( border ) * 2 );

   mud_printf( tempstring, "Instance ID: %d | Framework ID: %d", instance->tag->id, instance->framework->tag->id );

   text_to_olc( olc, "/%s\\\r\n", print_header( tempstring, "-", socket->account->pagewidth - 2 ) );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Name(framework)  : %s", instance_name( instance ) ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Short(framework) : %s", instance_short_descr( instance ) ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Long(framework)  : %s", instance_long_descr( instance ) ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Desc(framework)  : %s", instance_description( instance ) ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " This instance is level %d", instance->level ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " This instance is %slive.", instance->live ? "" : "not" ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " This instance's framework's ID %d", instance->framework->tag->id ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " This instance is contained by %s.", instance->contained_by ? instance_short_descr( instance->contained_by ) : "The Ether" ), space_after_border ), border );
   if( !instance->home )
      text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( " This instace has no home.", space_after_border ), border );
   else
      text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " This instance's home is (%d)%s.", instance->home->tag->id, instance_short_descr( instance->home ) ), space_after_border), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Height : %d.", get_height( instance ) ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Weight : %d.", get_weight( instance ) ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Width  : %d.", get_width( instance ) ), space_after_border ), border );

   if( SizeOfList( instance->contents ) > 0 )
      text_to_olc( olc, "%s", return_instance_contents_string( instance, border, socket->account->pagewidth ) );
   if( instance->primary_dmg_received_stat )
   {
      text_to_olc( olc, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );
      text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Primary Damage: %s %d/%d",
         instance->primary_dmg_received_stat->framework->name, instance->primary_dmg_received_stat->mod_stat,
         instance->primary_dmg_received_stat->perm_stat ), space_after_border ), border );
      text_to_olc( olc, "\\%s/\r\n",print_bar( "-", socket->account->pagewidth - 2 ) );
   }
   if( SizeOfList( instance->specifications ) > 0 || SizeOfList( instance->framework->specifications ) > 0 || inherited_frame_has_any_spec( instance->framework ) )
      text_to_olc( olc, "\r\n%s", return_instance_specs( instance, socket->account->pagewidth ) );
   if( SizeOfList( instance->stats ) > 0 )
      text_to_olc( olc, "\r\n%s", return_instance_stats( instance, socket->account->pagewidth ) );
   text_to_olc( olc, "\r\n" );
   return;
}

int editor_instance_prompt( D_SOCKET *dsock, bool commands )
{
   INCEPTION *olc;
   ENTITY_INSTANCE *instance;
   int ret = RET_SUCCESS;

   if( !dsock->account || !dsock->account->olc )
   {
      text_to_buffer( dsock, "Bad Prompt:>> " );
      return ret;
   }

   olc = dsock->account->olc;
   instance = (ENTITY_INSTANCE *)olc->editing;

   text_to_olc( olc, "Instance ID: %d | Framework ID %d ||> ", instance->tag->id, instance->framework->tag->id );
   return ret;
}

const char *return_instance_contents_string( ENTITY_INSTANCE *instance, const char *border, int width )
{
   ENTITY_INSTANCE *content;
   ITERATOR Iter;
   static char buf[MAX_BUFFER];
   char tempstring[MAX_BUFFER];
   int space_after_border;

   memset( &buf[0], 0, sizeof( buf ) );
   space_after_border = width - ( strlen( border ) * 2 );

   mud_printf( tempstring, "%s%s%s\r\n", border, print_header( "Instance Contents", "-", space_after_border ), border );
   mudcat( buf, tempstring );

   AttachIterator( &Iter, instance->contents );
   while( ( content = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      mud_printf( tempstring, "%s%s%s\r\n", border, fit_string_to_space( quick_format( "(%-7d) %s, %s", content->tag->id, instance_name( content ), instance_short_descr( content ) ), space_after_border ), border );
      mudcat( buf, tempstring );
   }
   DetachIterator( &Iter );

   buf[strlen( buf )] = '\0';
   return buf;
}

const char *return_instance_specs( ENTITY_INSTANCE *instance, int width )
{
   const char *const spec_from_table[] = {
      "", "( f )", "( i )"
   };
   SPECIFICATION *spec;
   LLIST *spec_strings;
   ITERATOR Iter;
   char *spec_string;
   static char buf[MAX_BUFFER];
   int x, longest, spec_string_len, spec_from;
   int num_column, column_count = 0;

   spec_strings = AllocList();

   mud_printf( buf, "%s\r\n", print_header( "Specifications", "-", width ) );

   for( longest = 0, x = MAX_SPEC; x >= 0; x-- )
   {
      spec_from = 0;
      if( ( spec = has_spec_detailed_by_type( instance, x, &spec_from ) ) == NULL )
         continue;
      CREATE( spec_string, char, MAX_BUFFER );
      mud_printf( spec_string, "%s : %d%s ", spec_table[spec->type], spec->value, spec_from_table[spec_from] );
      if( ( spec_string_len = strlen( spec_string ) ) > longest )
         longest = spec_string_len;
      AttachToList( spec_string, spec_strings );
   }

   num_column = floor( (double)width / (double)longest );

   AttachIterator( &Iter, spec_strings );
   while( ( spec_string = (char *)NextInList( &Iter ) ) != NULL )
   {
      mudcat( buf, fit_string_to_space( spec_string, longest ) );
      if( ++column_count >= num_column )
      {
         mudcat( buf, "\r\n" );
         column_count = 0;
      }
   }
   DetachIterator( &Iter );
   if( column_count != 0 )
      mudcat( buf, "\r\n" );

   FreeList( spec_strings );
   return buf;
}

const char *return_instance_stats( ENTITY_INSTANCE *instance, int width )
{
   STAT_INSTANCE *stat;
   LLIST *stat_strings;
   ITERATOR Iter;
   char *stat_string;
   static char buf[MAX_BUFFER];
   int x, longest, stat_string_len;
   int num_column, column_count = 0;

   stat_strings = AllocList();

   mud_printf( buf, "%s\r\n", print_header( "Stats", "-", width ) );
   for( longest = 0, ( x = get_potential_id( ENTITY_STAT_FRAMEWORK_IDS ) - 1 ); x >= 0; x-- )
   {
      if( ( stat = get_stat_from_instance_by_id( instance, x ) ) == NULL )
         continue;
      if( stat == instance->primary_dmg_received_stat )
         continue;
      CREATE( stat_string, char, MAX_BUFFER );
      mud_printf( stat_string, "\"%s\" is %d (P: %d | M: %d) ", stat->framework->name, get_stat_total( stat ), stat->perm_stat, stat->mod_stat );
      if( ( stat_string_len = strlen( stat_string ) ) > longest )
         longest = stat_string_len;
      AttachToList( stat_string, stat_strings );
   }

   num_column = floor( (double)width / (double)longest );

   AttachIterator( &Iter, stat_strings );
   while( ( stat_string = (char *)NextInList( &Iter ) ) != NULL )
   {
      mudcat( buf, fit_string_to_space( stat_string, longest ) );
      if( ++column_count >= num_column )
      {
         mudcat( buf, "\r\n" );
         column_count = 0;
      }
   }
   DetachIterator( &Iter );
   if( column_count != 0 )
      mudcat( buf, "\r\n" );

   FreeList( stat_strings );
   return buf;
}

const char *return_instance_spec_and_stats( ENTITY_INSTANCE *instance, const char *border, int width )
{
   const char *const spec_from_table[] = {
      "", "( framework )", "( inherited )"
   };
   SPECIFICATION *spec;
   STAT_INSTANCE *stat;
   static char buf[MAX_BUFFER];
   char tempstring[MAX_BUFFER];
   int space_after_border;
   int x, y, spec_from, MAX_STAT;

   space_after_border = width - ( strlen( border ) * 2 );

   mud_printf( buf, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );

   space_after_border = width - ( strlen( border ) * 3 );

   mud_printf( tempstring, "%s%s", border, print_header( "Specifications", " ", space_after_border / 2 ) );
   mudcat( buf, tempstring );

   mudcat( buf, border );

   mud_printf( tempstring, " %s%s\r\n", print_header( "Stats", " ", space_after_border / 2 ), border );
   mudcat( buf, tempstring );

   space_after_border = width - ( strlen( border ) * 2 );

   mud_printf( tempstring, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );
   mudcat( buf, tempstring );

   MAX_STAT = get_potential_id( ENTITY_STAT_FRAMEWORK_IDS );

   /* later when stats are in it will look like ;x < MAX_SPEC || y < MAX_STAT; */
   for( x = 0, y = 0, spec_from = 0; x < MAX_SPEC || y < MAX_STAT; )
   {
      spec = NULL;
      stat = NULL;
      if( x < MAX_SPEC )
         while( ( spec = has_spec_detailed_by_type( instance, x++, &spec_from ) ) == NULL && x < MAX_SPEC );
      if( y < MAX_STAT )
         while( ( ( stat = get_stat_from_instance_by_id( instance, y++ ) ) == NULL && y < MAX_STAT ) || ( stat != NULL && stat == instance->primary_dmg_received_stat ));

      if( !spec && !stat )
         break;

      if( spec )
         mud_printf( tempstring, "%s%s", border, fit_string_to_space( quick_format( " %s : %d%s", spec_table[spec->type], spec->value, spec_from_table[spec_from] ), ( space_after_border / 2 ) - 1 ) );
      else
         mud_printf( tempstring, "%s%s", border, print_header( " ", " ", ( space_after_border / 2 ) - 1 ) );
      mudcat( buf, tempstring );

      mudcat( buf, border );

      if( stat )
         mud_printf( tempstring, " %s%s\r\n", fit_string_to_space( quick_format( "%s:  P: %d M: %d T: %d", stat->framework->name, stat->perm_stat, stat->mod_stat, ( stat->perm_stat + stat->mod_stat ) ), ( space_after_border / 2 ) - 1) , border );
      else
         mud_printf( tempstring, " %s%s\r\n", print_header( " ", " ", ( space_after_border / 2 ) - 1 ), border );
      mudcat( buf, tempstring );

   }
   buf[strlen( buf )] = '\0';
   return buf;

}

void instance_load( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;

   text_to_olc( olc, "You load up %s.\r\n", instance_name( instance ) );
   update_tag( instance->tag, olc->account->name );
   full_load_instance( instance );
   return;
}

void instance_live( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;

   instance_toggle_live( instance );
   update_tag( instance->tag, olc->account->name );
   text_to_olc( olc, "You set the instance to %s.\r\n", instance->live ? "live" : "not live" );
   return;
}

void instance_level( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;

   if( !is_number( arg ) )
   {
      text_to_olc( olc, "You need to put in a level to set it to.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   set_instance_level( instance, atoi( arg ) );
   update_tag( instance->tag, olc->account->name );
   text_to_olc( olc, "You set the instance to level %d.\r\n", instance->level );
   return;
}

void instance_setStat( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;
   STAT_INSTANCE *stat;
   char buf[MAX_BUFFER];
   int value;

   arg = one_arg( arg, buf );

   if( is_number( buf ) )
      stat = get_stat_from_instance_by_id( instance, atoi( buf ) );
   else
      stat = get_stat_from_instance_by_name( instance, buf );

   if( !stat )
   {
      text_to_olc( olc, "No such Stat.\r\n" );
      return;
   }

   if( !is_number( arg ) && ( arg[0] == '-' && !is_number( arg + 1 ) ) )
   {
      text_to_olc( olc, "You must input a number to set the stat to.\r\n" );
      return;
   }
   value = atoi( arg );

   set_perm_stat( stat, value );
   text_to_olc( olc, "Stat set.\r\n" );
   return;
}

void instance_autowrite( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;

   if( !f_script_exists( instance->framework ) )
   {
      text_to_olc( olc, "This instance's framework has no script generated.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( autowrite_init( instance ) )
   {
      text_to_olc( olc, "You write the init script for this instance's framework.\r\n" ) ;
      olc_short_prompt( olc );
      return;
   }
   else
   {
      text_to_olc( olc, "Failed to write the init script for this instance's framework.\r\n" );
      olc_short_prompt( olc );
      return;
   }
}

void instance_addcontent( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;
   ENTITY_INSTANCE *instance_to_add;
   int value;

   if( check_selection_type( arg ) != SEL_INSTANCE )
   {
      if( is_number( arg ) )
      {
         value = atoi( arg );
         if( ( instance_to_add = get_instance_by_id( value ) ) == NULL )
         {
            text_to_olc( olc, "There's no instance with the ID of %d.\r\n", value );
            olc_short_prompt( olc );
            return;
         }
      }
      else if( ( instance_to_add = get_instance_by_name( arg ) ) == NULL )
      {
         text_to_olc( olc, "There's no instance with the name %s.\r\n", arg );
         return;
      }
   }
   else
   {
      if( !interpret_entity_selection( arg ) )
      {
         text_to_olc( olc, STD_SELECTION_ERRMSG_PTR_USED );
         return;
      }
      switch( input_selection_typing )
      {
         default: clear_entity_selection(); return;
         case SEL_INSTANCE:
            instance_to_add = (ENTITY_INSTANCE *)retrieve_entity_selection();
            break;
         case SEL_STRING:
            text_to_olc( olc, (char *)retrieve_entity_selection(), arg );
            return;
      }
   }
   entity_to_world( instance_to_add, instance );
   update_tag( instance->tag, olc->account->name );
   text_to_olc( olc, "%s added to %s's contents.\r\n", instance_name( instance_to_add ), instance_name( instance ) );
   return;
}


void instance_addspec( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;
   SPECIFICATION *spec;
   char spec_arg[MAX_BUFFER];
   int spec_type, spec_value;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Valid Specs: %s.\r\n", print_string_table( spec_table ) );
      olc_short_prompt( olc );
      return;
   }

   arg = one_arg( arg, spec_arg );

   if( ( spec_type = match_string_table_no_case( spec_arg, spec_table ) ) == -1 )
   {
      text_to_olc( olc, "Invalid Spec Type.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Spec Value Defaulting to 1.\r\n" );
      spec_value = 1;
   }
   else if( !is_number( arg ) )
   {
      text_to_olc( olc, "Spec Value must be a number.\r\n" );
      return;
   }
   else
      spec_value = atoi( arg );

   if( ( spec = spec_list_has_by_type( instance->specifications, spec_type ) ) != NULL )
      text_to_olc( olc, "Overriding current %s specification who's value was %d.\r\n", spec_table[spec->type], spec->value );
   else
      spec = init_specification();

   spec->type = spec_type;
   spec->value = spec_value;
   add_spec_to_instance( spec, instance );
   update_tag( instance->tag, olc->account->name );
   text_to_olc( olc, "%s added to %s with the value of %s.\r\n", spec_table[spec_type], instance_name( instance ), itos( spec->value ) );
   return;
}

void instance_remspec( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;
   SPECIFICATION *spec;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "What Spec do you want to remove?\r\n" );
      return;
   }

   if( ( spec = spec_list_has_by_name( instance->specifications, arg ) ) == NULL )
   {
      text_to_olc( olc, "This instance does not have that Spec.\r\n" );
      return;
   }
   rem_spec_from_instance( spec, instance );
   text_to_olc( olc, "Spec removed.\r\n" );
   return;
}

void instance_script( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;

   text_to_olc( olc, "%s\r\n", print_i_script( instance ) );
   olc_short_prompt( olc );
   return;
}

void instance_restore( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;

   restore_pool_stats( instance );
   text_to_olc( olc, "Pool stats restored.\r\n" );
   return;
}

void instance_done( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;

   if( !strcmp( instance->tag->created_by, "null" ) )
   {
      new_tag( instance->tag, olc->account->name );
      new_eInstance( instance );
      if( olc->using_workspace )
         add_instance_to_workspace( instance, olc->using_workspace );
   }
   if( SizeOfList( olc->chain ) > 0 )
      clearlist( olc->chain );
   free_editor( olc );
   change_socket_state( olc->account->socket, olc->editor_launch_state );
   text_to_olc( olc, "Exiting Entity Instance Editor.\r\n" );
   return;
}

void instance_addPak( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Add what Pak?\r\n" );
      olc_short_prompt( olc );
      return;
   }
   load_pak_on_instance( arg, instance );
   text_to_olc( olc, "You load the Pak.\r\n" );
   return;

}

void instance_reinit( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)olc->editing;

   onInstanceInit_trigger( instance );
   text_to_olc( olc, "You reinitialize %d : %s from the script.\r\n", instance->tag->id, instance_name( instance ) );
   return;
}

int init_sFramework_editor( INCEPTION *olc, STAT_FRAMEWORK *fstat )
{
   int ret = RET_SUCCESS;

   if( !fstat )
      olc->editing = init_stat_framework();
   else
      olc->editing = fstat;

   olc->editing_state = STATE_SFRAME_EDITOR;
   text_to_olc( olc, "Opening the Stat Framework Editor...\r\n" );
   olc->editor_commands = AllocList();

   return ret;
}

void boot_sFramework_editor( INCEPTION *olc, STAT_FRAMEWORK *fstat )
{
   int state = olc->account->socket->state;

   if( state < STATE_EFRAME_EDITOR || state > STATE_PROJECT_EDITOR )
      olc->editor_launch_state = state;
   init_sFramework_editor( olc, fstat );
   olc->editing_state = STATE_SFRAME_EDITOR;
   change_socket_state( olc->account->socket, olc->editing_state );
   return;
}

void editor_sFramework_prompt( D_SOCKET *dsock, bool commands )
{
   INCEPTION *olc;
   STAT_FRAMEWORK *fstat;
   BUFFER *buf = buffer_new( MAX_BUFFER );
   const char *border = "|";
   char tempstring[MAX_BUFFER];
   int space_after_border;

   fstat = (STAT_FRAMEWORK *)dsock->account->olc->editing;
   olc = dsock->account->olc;

   space_after_border = dsock->account->pagewidth - ( strlen( border ) * 2 );

   if( !strcmp( fstat->tag->created_by, "null" ) )
      mud_printf( tempstring, "Potential %s Stat ID: %d", fstat->pool ? "Pool" : "Registered",  get_potential_id( fstat->tag->type ) );
   else
      mud_printf( tempstring, "%s Stat ID: %d", fstat->pool ? "Pool" : "Registered", fstat->tag->id );

   text_to_olc( olc, "/%s\\\r\n", print_header( tempstring, "-", dsock->account->pagewidth - 2 ) );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " Name      : %s", fstat->name ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " SoftCap   : %s", fstat->softcap == MAX_INT ? "Max" : itos( fstat->softcap ) ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " HardCap   : %s", fstat->hardcap == MAX_INT ? "Max" : itos( fstat->hardcap ) ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " SoftFloor : %s", fstat->softfloor == MIN_INT ? "Min" : itos( fstat->softfloor ) ), space_after_border ), border );
   text_to_olc( olc, "%s%s%s\r\n", border, fit_string_to_space( quick_format( " HardFloor : %s", fstat->hardfloor == MIN_INT ? "Min" : itos( fstat->hardfloor ) ), space_after_border ), border );

   text_to_olc( olc, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );
   if( commands )
   {
      print_commands( dsock->account->olc, dsock->account->olc->editor_commands, buf, 0, dsock->account->pagewidth );
      bprintf( buf, "\\%s/\r\n", print_bar( "-", dsock->account->pagewidth - 2 ) );
   }
   text_to_olc( olc, buf->data );
   buffer_free( buf );
   return;
}

void sFramework_name( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   STAT_FRAMEWORK *fstat = (STAT_FRAMEWORK *)olc->editing;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Name this stat what?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   set_name( fstat, arg );
   update_tag( fstat->tag, olc->account->name );
   text_to_olc( olc, "You set the name to %s.\r\n", arg );
   return;
}

void sFramework_softcap( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   STAT_FRAMEWORK *fstat = (STAT_FRAMEWORK *)olc->editing;
   int value;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the softcap to what?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( !strcasecmp( arg, "max" ) )
      value = MAX_INT;
   else
   {
      if( !is_number( arg ) && ( arg[0] == '-' && !is_number( arg + 1 ) ) )
      {
         text_to_olc( olc, "You must input a number.\r\n" );
         olc_short_prompt( olc );
         return;
      }
      value = atoi( arg );
   }

   if( value < fstat->softfloor )
   {
      text_to_olc( olc, "The value of softcap cannot be less than the softfloor.\r\n" );
      return;
   }

   if( value > fstat->hardcap )
   {
      text_to_olc( olc, "The value of the softcap cannot be greater than the hardcap.\r\n" );
      return;
   }

   set_softcap( fstat, value );
   update_tag( fstat->tag, olc->account->name );
   text_to_olc( olc, "You set the softcap to %d.\r\n", fstat->softcap );
   return;
}

void sFramework_hardcap( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   STAT_FRAMEWORK *fstat = (STAT_FRAMEWORK *)olc->editing;
   int value;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the hardcap to what?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( !strcasecmp( arg, "max" ) )
      value = MAX_INT;
   else
   {
      if( !is_number( arg ) && ( arg[0] == '-' && !is_number( arg + 1 ) ) )
      {
         text_to_olc( olc, "You must input a number.\r\n" );
         olc_short_prompt( olc );
         return;
      }
      value = atoi( arg );
   }

   if( value < fstat->softcap )
   {
      text_to_olc( olc, "The value of hardcap cannot be less than the softcap.\r\n" );
      return;
   }

   set_hardcap( fstat, value );
   update_tag( fstat->tag, olc->account->name );
   text_to_olc( olc, "You set the hardcap to %d.\r\n", fstat->hardcap );
   return;

}

void sFramework_softfloor( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   STAT_FRAMEWORK *fstat = (STAT_FRAMEWORK *)olc->editing;
   int value;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the softfloor to what?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( !strcasecmp( arg, "min" ) )
      value = MIN_INT;
   else
   {
      if( !is_number( arg ) && ( arg[0] == '-' && !is_number( arg + 1 ) ) )
      {
         text_to_olc( olc, "You must input a number.\r\n" );
         olc_short_prompt( olc );
         return;
      }
      value = atoi( arg );
   }

   if( value > fstat->softcap )
   {
      text_to_olc( olc, "The value of the softfloor cannot be greater than the softcap.\r\n" );
      return;
   }

   if( value < fstat->hardfloor )
   {
      text_to_olc( olc, "The value of the softfloor cannot be lower than the hardfloor.\r\n" );
      return;
   }

   set_softfloor( fstat, value );
   update_tag( fstat->tag, olc->account->name );
   text_to_olc( olc, "You set the softfloor to %d.\r\n", fstat->softfloor );
   return;
}

void sFramework_hardfloor( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   STAT_FRAMEWORK *fstat = (STAT_FRAMEWORK *)olc->editing;
   int value;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the hardfloor to what?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( !strcasecmp( arg, "min" ) )
      value = MIN_INT;
   else
   {
      if( !is_number( arg ) && ( arg[0] == '-' && !is_number( arg + 1 ) ) )
      {
         text_to_olc( olc, "You must input a number.\r\n" );
         olc_short_prompt( olc );
         return;
      }
      value = atoi( arg );
   }

   if( value > fstat->softfloor )
   {
      text_to_olc( olc, "The value of the hardfloor cannot be greater than the softfloor.\r\n" );
      return;
   }

   set_hardfloor( fstat, value );
   update_tag( fstat->tag, olc->account->name );
   text_to_olc( olc, "You set the hardfloor to %d.\r\n", fstat->hardfloor );
   return;
}

void sFramework_type( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   STAT_FRAMEWORK *fstat = (STAT_FRAMEWORK *)olc->editing;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Set the type to what?\r\n" );
      olc_short_prompt( olc );
      return;
   }
   if( !strcasecmp( arg, "pool" ) )
   {
      if( fstat->pool )
      {
         text_to_olc( olc, "It's already a pool style stat.\r\n" );
         olc_short_prompt( olc );
         return;
      }
      set_stat_style( fstat, TRUE );
   }
   else if( !strcasecmp( arg, "registered" ) )
   {
      if( !fstat->pool )
      {
         text_to_olc( olc, "It's already a registered style stat.\r\n" );
         olc_short_prompt( olc );
         return;
      }
      set_stat_style( fstat, FALSE );
   }
   update_tag( fstat->tag, olc->account->name );
   text_to_olc( olc, "Stat style changed.\r\n" );
   return;
}

void sFramework_script( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   STAT_FRAMEWORK *fstat = (STAT_FRAMEWORK *)olc->editing;

   if( !strcmp( fstat->tag->created_by, "null" ) )
   {
      text_to_olc( olc, "This stat framework must completely exist before you can start script it, save or done and reopen.\r\n" );
      olc_short_prompt( olc );
      return;
   }
   text_to_olc( olc, "%s", print_s_script( fstat ) );
   return;
}

void sFramework_save( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   STAT_FRAMEWORK *fstat = (STAT_FRAMEWORK *)olc->editing;

   if( !strcmp( fstat->tag->created_by, "null" ) )
   {
      new_tag( fstat->tag, olc->account->name );
      new_stat_framework( fstat );
   }
   text_to_olc( olc, "Saved.\r\n" );
   return;
}

void sFramework_done( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   STAT_FRAMEWORK *fstat = (STAT_FRAMEWORK *)olc->editing;

   if( !strcmp( fstat->tag->created_by, "null" ) )
   {
      new_tag( fstat->tag, olc->account->name );
      new_stat_framework( fstat );
   }
   if( SizeOfList( olc->chain ) > 0 )
      clearlist( olc->chain );
   free_editor( olc );
   change_socket_state( olc->account->socket, olc->editor_launch_state );
   text_to_olc( olc, "Exiting Stat Framework Editor.\r\n" );
   olc_show_prompt( olc );
   return;
}
