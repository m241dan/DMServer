/* methods for using paks written by Davenge */

#include "mud.h"

bool load_pak_on_framework( const char *pak_name, ENTITY_FRAMEWORK *frame )
{
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;

   if( !pak_name || pak_name[0] == '\0' )
      return FALSE;

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT * FROM `paks` WHERE name='%s';", pak_name ) ) )
   {
      FreeList( list );
      return FALSE;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      SPECIFICATION *spec;
      STAT_FRAMEWORK *fstat;
      int type = atoi( row[1] );
      const char *name = row[2];
      int value = atoi( row[3] );

      switch( type )
      {
         default: break;
         case PAK_STAT:
            fstat = get_stat_framework_by_name( name );
            add_stat_to_frame( fstat, frame );
            break;
         case PAK_SPEC:
            spec = init_specification();
            spec->type = match_string_table( name, spec_table );
            spec->value = value;
            add_spec_to_framework( spec, frame );
            break;
      }
   }
   DetachIterator( &Iter );
   FreeList( list );
   return TRUE;
}

bool load_pak_on_instance( const char *pak_name, ENTITY_INSTANCE *instance )
{
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;

   if( !pak_name || pak_name[0] == '\0' )
      return FALSE;

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT * FROM `paks` WHERE name='%s';", pak_name ) ) )
   {
      FreeList( list );
      return FALSE;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      SPECIFICATION *spec;
      STAT_FRAMEWORK *fstat;
      int type = atoi( row[1] );
      const char *name = row[2];
      int value = atoi( row[3] );

      switch( type )
      {
         default: break;
         case PAK_STAT:
            fstat = get_stat_framework_by_name( name );
            stat_instantiate( instance, fstat );
            break;
         case PAK_SPEC:
            spec = init_specification();
            spec->type = match_string_table( name, spec_table );
            spec->value = value;
            add_spec_to_instance( spec, instance );
            break;
      }

   }
   DetachIterator( &Iter );
   FreeList( list );
   return TRUE;

}

inline bool add_pak_stat( const char *name, const char *stat )
{
   if( !quick_query( "INSERT INTO `paks` VALUES ( '%s', '%d', '%s', '%d' );", name, PAK_STAT, stat, 0 ) )
      return FALSE;
   return TRUE;
}

inline bool add_pak_spec( const char *name, const char *spec_name, int value )
{
   if( !quick_query( "INSERT INTO `paks` VALUES ( '%s', '%d', '%s', '%d' );", name, PAK_SPEC, spec_name, value ) )
      if( !quick_query( "UPDATE `paks` SET value='%d' WHERE label='%s' AND name='%s';", value, spec_name, name ) )
         return FALSE;
   return TRUE;
}

inline bool rem_pak_entry( const char *name, const char *label )
{
   if( !quick_query( "DELETE FROM `paks` WHERE name='%s' AND label='%s';", name, label ) )
      return FALSE;
   return TRUE;
}

const char *return_pak_contents( const char *pak_name )
{
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;
   static char buf[MAX_BUFFER];

   if( !pak_name || pak_name[0] == '\0' )
      return NULL;


   mud_printf( buf, "Pak %s Stats:\r\n", pak_name );

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT label FROM `paks` WHERE name='%s' AND type='%d';", pak_name, PAK_STAT ) ) )
   {
      FreeList( list );
      strcat( buf, " - None\r\n" );
   }
   else
   {
      AttachIterator( &Iter, list );
      while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
      {
         const char *label = row[0];
         strcat( buf, quick_format( " - %s\r\n", label ) );
      }
      DetachIterator( &Iter );
      FreeList( list );
   }

   strcat( buf, quick_format( "Pak %s Specs:\r\n", pak_name ) );

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT label, value FROM `paks` WHERE name='%s' AND type='%d';", pak_name, PAK_SPEC ) ) )
   {
      FreeList( list );
      strcat( buf, " - None\r\n" );
   }
   else
   {
      AttachIterator( &Iter, list );
      while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
      {
         const char *label = row[0];
         int value = atoi(row[1] );
         strcat( buf, quick_format( " - %s : %d\r\n", label, value ) );
      }
      DetachIterator( &Iter );
      FreeList( list );
   }
   buf[strlen( buf )] = '\0';
   return buf;
}

