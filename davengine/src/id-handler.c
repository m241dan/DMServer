/* id-handlers.c written by Davenge */

#include "mud.h"

ID_HANDLER *handlers[MAX_ID_HANDLER];

ID_HANDLER *init_handler( void )
{
   ID_HANDLER *handler;

   CREATE( handler, ID_HANDLER, 1 );
   handler->recycled_ids = AllocList();
   if( clear_handler( handler ) != RET_SUCCESS )
   {
      free_handler( handler );
      return NULL;
   }
   return handler;
}

int clear_handler( ID_HANDLER *handler )
{
   ITERATOR Iter;
   int *rec_id;
   int ret = RET_SUCCESS;

   if( !handler )
   {
      BAD_POINTER( "handler" );
      return ret;
   }

   handler->type = -1;
   FREE( handler->name );
   handler->name = strdup( "new handler" );
   handler->top_id = 0;
   handler->can_recycle = FALSE;

   if( SizeOfList( handler->recycled_ids ) > 0 )
   {
      AttachIterator( &Iter, handler->recycled_ids );
      while( ( rec_id = (int *)NextInList( &Iter ) ) != NULL )
         FREE( rec_id );
      DetachIterator( &Iter );
   }
   return ret;
}

int free_handler( ID_HANDLER *handler )
{
   int ret = RET_SUCCESS;
   FREE( handler->name );
   if( SizeOfList( handler->recycled_ids ) > 0 )
      clear_handler( handler );
   FreeList( handler->recycled_ids );
   handler->recycled_ids = NULL;
   FREE( handler );
   return ret;
}

ID_TAG *init_tag( void )
{
   ID_TAG *tag;

   CREATE( tag, ID_TAG, 1 );
   if( clear_tag( tag ) != RET_SUCCESS )
   {
      free_tag( tag );
      return NULL;
   }
   return tag;
}

int clear_tag( ID_TAG *tag )
{
   int ret = RET_SUCCESS;

   if( !tag )
   {
      BAD_POINTER( "tag" );
      return ret;
   }

   tag->type = -1;
   tag->id = -1;
   FREE( tag->created_by );
   FREE( tag->created_on );
   FREE( tag->modified_by );
   FREE( tag->modified_on );
   tag->created_by = strdup( "null" );
   tag->created_on = strdup( strip_nl( ctime( &current_time ) ) );
   tag->modified_by = strdup( "null" );
   tag->modified_on = strdup( strip_nl( ctime( &current_time ) ) );

   return ret;
}

int free_tag( ID_TAG *tag )
{
   int ret = RET_SUCCESS;

   FREE( tag->created_on );
   FREE( tag->created_by );
   FREE( tag->modified_by );
   FREE( tag->modified_on);
   FREE( tag );
   return ret;
}

int delete_tag( ID_TAG *tag )
{
   int ret = RET_SUCCESS;

   if( handlers[tag->type]->can_recycle )
   {
      int *to_recycle;

      CREATE( to_recycle, int, 1 );
      *to_recycle = tag->id;
      AttachToList( to_recycle, handlers[tag->type]->recycled_ids );

      if( !quick_query( "INSERT INTO `id-recycled` VALUES( '%d', '%d' );", tag->type, tag->id ) )
         bug( "%s: did not update recycled ids database with tag %d of type %d.", __FUNCTION__, tag->id, tag->type );
   }
   free_tag( tag );
   return ret;
}

int new_tag( ID_TAG *tag, const char *creator )
{
   int ret = RET_SUCCESS;

   if( tag->type < 0 )
   {
      bug( "%s: tag has bad type: %d", __FUNCTION__, tag->type );
      return RET_FAILED_OTHER;
   }

   tag->id = get_new_id( tag->type );
   tag->created_by = strdup( creator );
   tag->modified_by = strdup( creator );
   tag->created_on = strdup( strip_nl( ctime( &current_time ) ) );
   tag->modified_on = strdup( strip_nl( ctime( &current_time ) ) );

   return ret;
}

int db_load_tag( ID_TAG *tag, MYSQL_ROW *row )
{
   int counter = 0;

   tag->id = atoi( (*row)[counter++] );
   tag->type = atoi( (*row)[counter++] );
   tag->created_by = strdup( (*row)[counter++] );
   tag->created_on = strdup( (*row)[counter++] );
   tag->modified_by = strdup( (*row)[counter++] );
   tag->modified_on = strdup( (*row)[counter++] );

   return counter;
}

int update_tag( ID_TAG *tag, const char *effector, ... )
{
   char buf[50];
   int ret = RET_SUCCESS;
   va_list va;
   int res;

   if( !strcmp( tag->created_by, "null" ) )
      return RET_FAILED_OTHER;

   va_start( va, effector );
   res = vsnprintf( buf, 50, effector, va );
   va_end( va );

   if( res >= 50 - 1 )
   {
      bug( "%s: effector name too long. Length produced = %d", __FUNCTION__, res );
      return RET_FAILED_OTHER;
   }

   tag->modified_by = strdup( buf );
   tag->modified_on = strdup( strip_nl( ctime( &current_time ) ) );

   if( !quick_query( "UPDATE `%s` SET modified_by='%s', modified_on='%s' WHERE %s='%d';", 
      tag_table_strings[tag->type], tag->modified_by, tag->modified_on, tag_table_whereID[tag->type], tag->id ) )
      bug( "%s: could not save the update to the database.", __FUNCTION__ );

   return ret;
}


int load_id_handlers( void )
{
   MYSQL_RES *result;
   MYSQL_ROW row;
   ID_HANDLER *handler;
   int ret = RET_SUCCESS;

   if( !quick_query( "SELECT * FROM `id-handlers`;" ) )
      return RET_FAILED_OTHER;

   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
   {
      report_sql_error( sql_handle );
      return RET_DB_NO_ENTRY;
   }

   while( ( row = mysql_fetch_row( result ) ) != NULL )
   {
      if( ( handler = init_handler() ) == NULL )
      {
         BAD_POINTER( "handler" );
         return ret;
      }

      handler->type = atoi( row[0] );
      handler->name = strdup( row[1] );
      handler->top_id = atoi( row[2] );
      handler->can_recycle = (bool)atoi( row[3] );
      if( handlers[handler->type] != NULL )
      {
         bug( "%s: two handlers have identitical IDs", __FUNCTION__ );
         free_handler( handler );
         return RET_FAILED_OTHER;
      }
      handlers[handler->type] = handler;
   }

   mysql_free_result( result );
   return ret;
}

int load_recycled_ids( void )
{
   MYSQL_RES *result;
   MYSQL_ROW row;
   int ret = RET_SUCCESS;

   if( !quick_query( "SELECT * FROM `id-recycled`;" ) )
      return RET_FAILED_OTHER;

   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
   {
      report_sql_error( sql_handle );
      return RET_DB_NO_ENTRY;
   }

   while( ( row = mysql_fetch_row( result ) ) != NULL )
   {
      int type;
      int *id;
      CREATE( id, int, 1 );
      type = atoi( row[0] );
      *id = atoi( row[1] );
      AttachToList( id, handlers[type]->recycled_ids );
   }

   mysql_free_result( result );
   return ret;
}

int get_new_id( int type )
{
   ID_HANDLER *handler;
   int *id;     /* have to use a pointer to remove from list, cannot use locally allocated memory */
   int rec_id; /* must use this integer as storage because the recycled id memory will need to be deleted before return */


   if( ( handler = handlers[type] ) == NULL )
   {
      bug( "%s: %d is a bad handler type.", __FUNCTION__, type );
      return -1;
   }

   if( handler->can_recycle && SizeOfList( handler->recycled_ids ) > 0 )
   {
      ITERATOR Iter;

      AttachIterator( &Iter, handler->recycled_ids );
      if( ( id = (int *)NextInList( &Iter ) ) == NULL )
      {
         bug( "%s: could not get id from recycled list of handler %s.", __FUNCTION__, handler->name);
         return -1;
      }
      DetachFromList( id, handler->recycled_ids );
      rec_id = *id;
      FREE( id );
      DetachIterator( &Iter );

      if( !quick_query( "DELETE FROM `id-recycled` WHERE type='%d' AND rec_id='%d';", type, rec_id ) )
         bug( "%s: ID %d of type %d was not properly deleted from database of recycled ids.", rec_id, type );

      return rec_id;
   }

   if( !quick_query( "UPDATE `id-handlers` SET top_id='%d' WHERE type='%d';", ( handler->top_id + 1 ), handler->type ) )
      bug( "%s: handler of type %d was not incremented in database properly.", __FUNCTION__, type );

   return handler->top_id++;
}

int get_potential_id( int type )
{
   ID_HANDLER *handler;
   int *id;     /* have to use a pointer to remove from list, cannot use locally allocated memory */
   int rec_id; 


   if( ( handler = handlers[type] ) == NULL )
   {
      bug( "%s: %d is a bad handler type.", __FUNCTION__, type );
      return -1;
   }

   if( handler->can_recycle && SizeOfList( handler->recycled_ids ) > 0 )
   {
      ITERATOR Iter;

      AttachIterator( &Iter, handler->recycled_ids );
      if( ( id = (int *)NextInList( &Iter ) ) == NULL )
      {
         bug( "%s: could not get id from recycled list of handler %s.", __FUNCTION__, handler->name );
         return -1;
      }
      rec_id = *id;
      id = NULL;
      DetachIterator( &Iter );

      return rec_id;
   }
   return handler->top_id;
}

ID_TAG *copy_tag( ID_TAG *tag )
{
   ID_TAG *tag_copy;

   if( !tag )
   {
      bug( "%s: passed a NULL tag.", __FUNCTION__ );
      return NULL;
   }

   tag_copy = init_tag();
   tag_copy->type = tag->type;
   tag_copy->id = tag->id;
   tag_copy->created_by = strdup( tag->created_by );
   tag_copy->created_on = strdup( tag->created_on );
   tag_copy->modified_by = strdup( tag->modified_by );
   tag_copy->modified_on = strdup( tag->modified_on );

   return tag_copy;
}

void fwrite_id_tag_export( FILE *fp, ID_TAG *tag, int *id_table )
{
   fprintf( fp, "#IDTAG\n" );
   fprintf( fp, "ID           %d\n", id_table ? get_id_table_position( id_table, tag->id ) : 0 );
   fprintf( fp, "Type         %d\n", tag->type );
   fprintf( fp, "CreatedOn    %s~\n", tag->created_on );
   fprintf( fp, "CreatedBy    %s~\n", tag->created_by );
   fprintf( fp, "ModifiedOn   %s~\n", tag->modified_on );
   fprintf( fp, "ModifiedBy   %s~\n", tag->modified_by );
   fprintf( fp, "END\n\n" );
}

ID_TAG *fread_id_tag_import( FILE *fp, int *id_table )
{
   ID_TAG *tag;
   char *word;
   int position;
   bool found, done = FALSE;

   CREATE( tag, ID_TAG, 1 );

   word = ( feof( fp ) ? "END" : fread_word( fp ) );
   while( !done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case 'C':
            SREAD( "CreatedBy", tag->created_by );
            SREAD( "CreatedOn", tag->created_on );
            break;
         case 'E':
            if( !strcmp( word, "END" ) )
               return tag;
            break;
         case 'I':
            if( !strcmp( word, "ID" ) )
            {
               found = TRUE;
               if( id_table )
               {
                  position = fread_number( fp );
                  tag->id = id_table[position];
               }
               else
                  tag->id = fread_number( fp );
               break;
            }
            break;
         case 'M':
            SREAD( "ModifiedBy", tag->modified_by );
            SREAD( "ModifiedOn", tag->modified_on );
            break;
         case 'T':
            IREAD( "Type", tag->type );
      }
      if( !found )
         bug( "%s: bad file format %s", __FUNCTION__, word );
      if( !done )
         word = ( feof( fp ) ? "END" : fread_word( fp ) );
   }
   free_tag( tag );
   return NULL;
}

int *build_workspace_id_table( LLIST *workspace_list )
{
   WORKSPACE *wSpace;
   int *table;
   ITERATOR Iter;
   int x = 0;

   if( !workspace_list )
   {
      bug( "%s: passed a NULL workspace_list.", __FUNCTION__ );
      return NULL;
   }

   CREATE( table, int, ( SizeOfList( workspace_list ) + 1 ) );
   AttachIterator( &Iter, workspace_list );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      table[x++] = wSpace->tag->id;
   DetachIterator( &Iter );

   table[x] = -1; /* terminator */
   return table;
}

int *build_instance_id_table( LLIST *instance_list )
{
   ENTITY_INSTANCE *instance;
   int *table;
   ITERATOR Iter;
   int x = 0;

   if( !instance_list )
   {
      bug( "%s: passed a NULL instance_list.", __FUNCTION__ );
      return NULL;
   }

   CREATE( table, int, ( SizeOfList( instance_list ) + 1 ) );
   AttachIterator( &Iter, instance_list );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      table[x++] = instance->tag->id;
   DetachIterator( &Iter );

   table[x] = -1; /* terminator */
   return table;
}

int *build_framework_id_table( LLIST *framework_list )
{
   ENTITY_FRAMEWORK *frame;
   int *table;
   ITERATOR Iter;
   int x = 0;

   if( !framework_list )
   {
      bug( "%s: passed a NULL framework_list.", __FUNCTION__ );
      return NULL;
   }

   CREATE( table, int, ( SizeOfList( framework_list ) + 1 ) );
   AttachIterator( &Iter, framework_list );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      table[x++] = frame->tag->id;
   DetachIterator( &Iter );

   table[x] = -1; /* terminator */
   return table;
}

int get_id_table_position( int *table, int id )
{
   int x;

   for( x = 0; table[x] != -1; x++ )
      if( table[x] == id )
         return x;

   return -1;
}

void print_table( int *table )
{
   int x = 0;
   int value;

   while( ( value = *table++ ) != -1 )
      bug( "%s: [%d] = %d.", __FUNCTION__, x++, value );

   return;
}

int *build_id_table_import( DIR *project_directory, int type )
{
   int *id_table;
   int size, x;

   if( ( size = directory_file_count_regex( project_directory, tag_table_extensions[type] ) ) == 0 )
      return NULL;

   CREATE( id_table, int, ( size + 1 ) );
   for( x = 0; x < size; x++ )
      id_table[x] = get_new_id( type );
   id_table[x] = -1;

   return id_table;
}
