/* the file containg functions pertainign to methods minus lua stuff written by Davenge */

#include "mud.h"

STAT_FRAMEWORK *init_stat_framework( void )
{
   STAT_FRAMEWORK *fstat;

   CREATE( fstat, STAT_FRAMEWORK, 1 );
   fstat->tag = init_tag();
   fstat->tag->type = ENTITY_STAT_FRAMEWORK_IDS;
   fstat->name = strdup( "null" );
   fstat->softfloor = -1;
   fstat->hardfloor = -2;
   fstat->softcap = 1;
   fstat->hardcap = 2;
   return fstat;
}

void free_stat_framework( STAT_FRAMEWORK *fstat )
{
   FREE( fstat->name );
   free_tag( fstat->tag );
   FREE( fstat );
   return;
}
void new_stat_framework( STAT_FRAMEWORK *fstat )
{
   if( !fstat )
   {
      bug( "%s: passed a NULL fstat.", __FUNCTION__ );
      return;
   }
   if( !strcmp( fstat->tag->created_by, "null" ) )
   {
      if( new_tag( fstat->tag, "system" ) != RET_SUCCESS )
      {
         bug( "%s: failed to pull new tag from handler.", __FUNCTION__ );
         return;
      }
   }

   if( !quick_query( "INSERT INTO `stat_frameworks` VALUES ( '%d', '%d', '%s', '%s', '%s', '%s', '%s', '%d', '%d', '%d', '%d', '%d' );",
      fstat->tag->id, fstat->tag->type, fstat->tag->created_by, fstat->tag->created_on, fstat->tag->modified_by, fstat->tag->modified_on,
      fstat->name, fstat->softcap, fstat->hardcap, fstat->softfloor, fstat->hardfloor, (int)fstat->pool ) )
      bug( "%s: could not add to database %s.", __FUNCTION__, fstat->name );

   init_s_script( fstat, TRUE );

   return;
}

inline void new_stat_on_frame( STAT_FRAMEWORK *fstat, ENTITY_FRAMEWORK *frame )
{
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   quick_query( "INSERT INTO `entity_framework_stats` VALUES ( '%d', '%d' );", frame->tag->id, fstat->tag->id );
}

inline void add_stat_to_frame( STAT_FRAMEWORK *fstat, ENTITY_FRAMEWORK *frame )
{
   int nober;
   if( get_stat_from_framework_by_id( frame, fstat->tag->id, &nober ) ) return;
   AttachToList( fstat, frame->stats );
   new_stat_on_frame( fstat, frame );
}

void db_load_stat_framework( STAT_FRAMEWORK *fstat, MYSQL_ROW *row )
{
   int counter;

   counter = db_load_tag( fstat->tag, row );
   fstat->name = strdup( (*row)[counter++] );
   fstat->softcap = atoi( (*row)[counter++] );
   fstat->hardcap = atoi( (*row)[counter++] );
   fstat->softfloor = atoi( (*row)[counter++] );
   fstat->hardfloor = atoi( (*row)[counter++] );
   fstat->pool = atoi( (*row)[counter++] );
   return;
}

void load_framework_stats( ENTITY_FRAMEWORK *frame )
{
   STAT_FRAMEWORK *fstat;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;

   if( !frame )
      return;

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT statFrameworkID FROM `entity_framework_stats` WHERE entityFrameworkID=%d;", frame->tag->id ) ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      fstat = get_stat_framework_by_id( atoi( row[0] ) );
      AttachToList( fstat, frame->stats );
   }
   DetachIterator( &Iter );
   FreeList( list );

   return;
}

STAT_INSTANCE *init_stat( void )
{
   STAT_INSTANCE *stat;

   CREATE( stat, STAT_INSTANCE, 1 );
   stat->owner = NULL;
   stat->framework = NULL;
   return stat;
}

void free_stat( STAT_INSTANCE *stat )
{
   stat->owner = NULL;
   stat->framework = NULL;
   FREE( stat );
   return;
}

void new_stat_instance( STAT_INSTANCE *stat )
{
   if( !stat )
   {
      bug( "%s: passed a NULL stat.", __FUNCTION__ );
      return;
   }
   if( !stat->owner )
   {
      bug( "%s: this stat has no owner.", __FUNCTION__ );
      return;
   }
   if( !stat->framework )
   {
      bug( "%s: this stat has no framework.", __FUNCTION__ );
      return;
   }
   if( !quick_query( "INSERT INTO `entity_stats` VALUES ( '%d', '%d', '%d', '%d' );",
      stat->framework->tag->id, stat->owner->tag->id, stat->perm_stat, stat->mod_stat ) )
      bug( "%s: could not save stat into database.", __FUNCTION__ );

   return;
}

void db_load_stat_instance( STAT_INSTANCE *stat, MYSQL_ROW *row )
{
   int counter = 0;

   stat->framework = get_stat_framework_by_id( atoi( (*row)[counter++] ) );
   stat->owner = get_instance_by_id( atoi( (*row)[counter++] ) );
   stat->perm_stat = atoi( (*row)[counter++] );
   stat->mod_stat = atoi( (*row)[counter++] );
   return;
}

void free_stat_list( LLIST *list )
{
   STAT_INSTANCE *stat;
   ITERATOR Iter;

   AttachIterator( &Iter, list );
   while( ( stat = (STAT_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( stat, list );
      free_stat( stat );
   }
   DetachIterator( &Iter );
   return;
}

void stat_instantiate( ENTITY_INSTANCE *owner, STAT_FRAMEWORK *fstat )
{
   STAT_INSTANCE *stat;

   if( get_stat_from_instance_by_id( owner, fstat->tag->id ) )
      return;
   stat = init_stat();
   stat->framework = fstat;
   stat->owner = owner;
   AttachToList( stat, owner->stats );
   if( !strcmp( owner->tag->created_by, "null" ) )
      return;
   new_stat_instance( stat );
}

void load_entity_stats( ENTITY_INSTANCE *entity )
{
   STAT_INSTANCE *stat;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;

   list = AllocList();

   if( !db_query_list_row( list, quick_format( "SELECT * FROM `entity_stats` WHERE owner=%d;", entity->tag->id ) ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      CREATE( stat, STAT_INSTANCE, 1 );
      db_load_stat_instance( stat, &row );
      AttachToList( stat, entity->stats );
   }
   DetachIterator( &Iter );
   FreeList( list );

   return;
}

void instantiate_entity_stats_from_framework( ENTITY_INSTANCE *entity )
{
   STAT_FRAMEWORK *fstat;
   int MAX_STAT = 0;
   int from, x;

   MAX_STAT = get_potential_id( ENTITY_STAT_FRAMEWORK_IDS );

   for( x = 0; x < MAX_STAT; x++ )
      if( ( fstat = get_stat_from_framework_by_id( entity->framework, x, &from ) ) != NULL )
         stat_instantiate( entity, fstat );

   return;
}

void clear_stat_list( LLIST *list )
{
   STAT_INSTANCE *stat;
   ITERATOR Iter;

   AttachIterator( &Iter, list );
   while( ( stat = (STAT_INSTANCE *)NextInList( &Iter ) ) != NULL )
      free_stat( stat );
   DetachIterator( &Iter );
}

inline void delete_stat_from_instance( STAT_INSTANCE *stat, ENTITY_INSTANCE *instance )
{
   DetachFromList( stat, instance->stats );
   if( !quick_query( "DELETE FROM `entity_stats` WHERE owner=%d;", instance->tag->id ) )
      bug( "%s: could not delete a stat with framework %d from instance %d from database.", __FUNCTION__, stat->framework->tag->id, instance->tag->id );
   free_stat( stat );
}

void load_new_stats( ENTITY_INSTANCE *instance )
{
   ENTITY_FRAMEWORK *framework;
   STAT_FRAMEWORK *fstat;
   ITERATOR Iter;

   framework = instance->framework;
   while( framework )
   {
      AttachIterator( &Iter, framework->stats );
      while( ( fstat = (STAT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
         if( !get_stat_from_instance_by_id( instance, fstat->tag->id ) )
            stat_instantiate( instance, fstat );
      DetachIterator( &Iter );
      framework = framework->inherits;
   }
}


bool inherited_frame_has_any_stats( ENTITY_FRAMEWORK *frame )
{
   if( !frame->inherits )
      return FALSE;
   while( ( frame = frame->inherits ) != NULL )
      if( SizeOfList( frame->stats ) > 0 )
         return TRUE;

   return FALSE;
}

STAT_FRAMEWORK *get_stat_framework_by_query( const char *query )
{
   STAT_FRAMEWORK *fstat;
   MYSQL_ROW row;

   if( ( row = db_query_single_row( query ) ) == NULL )
      return NULL;

   CREATE( fstat, STAT_FRAMEWORK, 1 );
   CREATE( fstat->tag, ID_TAG, 1 );
   db_load_stat_framework( fstat, &row );
   AttachToList( fstat, stat_frameworks );
   FREE( row );
   return fstat;
}
inline STAT_FRAMEWORK *get_stat_framework_by_id( int id )
{
   STAT_FRAMEWORK *fstat;
   if( ( fstat = get_active_stat_framework_by_id( id ) ) == NULL )
      fstat = load_stat_framework_by_id( id );
   return fstat;
}

STAT_FRAMEWORK *get_active_stat_framework_by_id( int id )
{
   STAT_FRAMEWORK *fstat;
   ITERATOR Iter;

   AttachIterator( &Iter, stat_frameworks );
   while( ( fstat = (STAT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( fstat->tag->id == id )
         break;
   DetachIterator( &Iter );
   return fstat;
}

inline STAT_FRAMEWORK *load_stat_framework_by_id( int id )
{
   return get_stat_framework_by_query( quick_format( "SELECT * FROM `stat_frameworks` WHERE statFrameworkID=%d;", id ) );
}
inline STAT_FRAMEWORK *get_stat_framework_by_name( const char *name )
{
   STAT_FRAMEWORK *fstat;
   if( ( fstat = get_active_stat_framework_by_name( name ) ) == NULL )
      fstat = load_stat_framework_by_name( name );
   return fstat;
}

STAT_FRAMEWORK *get_active_stat_framework_by_name( const char *name )
{
   STAT_FRAMEWORK *fstat;
   ITERATOR Iter;

   AttachIterator( &Iter, stat_frameworks );
   while( ( fstat = (STAT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( fstat->name, name ) )
         break;
   DetachIterator( &Iter );
   return fstat;
}

inline STAT_FRAMEWORK *load_stat_framework_by_name( const char *name )
{
   return get_stat_framework_by_query( quick_format( "SELECT * FROM `stat_frameworks` WHERE name='%s';", name ) );
}

STAT_FRAMEWORK *get_stat_from_framework_by_id( ENTITY_FRAMEWORK *frame, int id, int *spec_from )
{
   STAT_FRAMEWORK *fstat;
   ITERATOR Iter;

   AttachIterator( &Iter, frame->stats );
   while( ( fstat = (STAT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( fstat->tag->id == id )
         break;
   DetachIterator( &Iter );

   if( fstat )
      return fstat;

   if( frame->inherits )
   {
      *spec_from = 1;
      return get_stat_from_framework_by_id( frame->inherits, id, spec_from );
   }
   return NULL;
}

STAT_FRAMEWORK *get_stat_from_framework_by_name( ENTITY_FRAMEWORK *frame, const char *name, int *spec_from )
{
   STAT_FRAMEWORK *fstat;
   ITERATOR Iter;

   AttachIterator( &Iter, frame->stats );
   while( ( fstat = (STAT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( fstat->name, name ) )
         break;
   DetachIterator( &Iter );

   if( fstat )
      return fstat;

   if( frame->inherits )
   {
      *spec_from = 1;
      return get_stat_from_framework_by_name( frame->inherits, name, spec_from );
   }
   return NULL;
}

STAT_INSTANCE  *get_stat_from_instance_by_id( ENTITY_INSTANCE *entity, int id )
{
   STAT_INSTANCE *stat;
   ITERATOR Iter;

   if( id < 0 )
      return NULL;

   AttachIterator( &Iter, entity->stats );
   while( ( stat = (STAT_INSTANCE *)NextInList( &Iter ) ) != NULL )
      if( stat->framework->tag->id == id )
         break;
   DetachIterator( &Iter );

   return stat;
}
STAT_INSTANCE  *get_stat_from_instance_by_name( ENTITY_INSTANCE *entity, const char *name )
{
   STAT_INSTANCE *stat;
   ITERATOR Iter;

   AttachIterator( &Iter, entity->stats );
   while( ( stat = (STAT_INSTANCE *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( stat->framework->name, name ) )
         break;
   DetachIterator( &Iter );

   return stat;
}

STAT_FRAMEWORK *get_primary_dmg_stat_from_framework( ENTITY_FRAMEWORK *frame, int *source )
{
   if( frame->f_primary_dmg_received_stat )
      return frame->f_primary_dmg_received_stat;

   if( !frame->inherits )
      return NULL;

   *source = 1;
   return get_primary_dmg_stat_from_framework( frame->inherits, source );
}

int get_effective_change( int cap, int floor, int raw, int effective_value, int change )
{
   int ef_change;
   int value;

   if( ( value = cap - raw ) < 0 )
   {
      ef_change = value - change;
      if( ef_change > 0 )
         return( ef_change * -1 );
      return 0;
   }
   else if( ( value = floor - raw ) > 0 )
   {
      ef_change = value - change;
      if( ef_change < 0 )
         return abs( ef_change );
      return 0;
   }
   else
      return urange( ( floor - effective_value ), change, ( cap - effective_value ) );
}

inline void set_softcap( STAT_FRAMEWORK *fstat, int value )
{
   fstat->softcap = value;
   if( !strcmp( fstat->tag->created_by, "null" ) ) return;
   if( !quick_query( "UPDATE `stat_frameworks` SET softcap=%d WHERE statFrameworkID=%d;", value, fstat->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}

inline void set_hardcap( STAT_FRAMEWORK *fstat, int value )
{
   fstat->hardcap = value;
   if( !strcmp( fstat->tag->created_by, "null" ) ) return;
   if( !quick_query( "UPDATE `stat_frameworks` SET hardcap=%d WHERE statFrameworkID=%d;", value, fstat->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}

inline void set_softfloor( STAT_FRAMEWORK *fstat, int value )
{
   fstat->softfloor = value;
   if( !strcmp( fstat->tag->created_by, "null" ) ) return;
   if( !quick_query( "UPDATE `stat_frameworks` SET softfloor=%d WHERE statFrameworkID=%d;", value, fstat->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}

inline void set_hardfloor( STAT_FRAMEWORK *fstat, int value )
{
   fstat->hardfloor = value;
   if( !strcmp( fstat->tag->created_by, "null" ) ) return;
   if( !quick_query( "UPDATE `stat_frameworks` SET hardfloor=%d WHERE statFrameworkID=%d;", value, fstat->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}
inline void set_name( STAT_FRAMEWORK *fstat, const char *name )
{
   FREE( fstat->name );
   fstat->name = strdup( name );
   if( !strcmp( fstat->tag->created_by, "null" ) ) return;
   if( !quick_query( "UPDATE `stat_frameworks` SET name='%s' WHERE statFrameworkID=%d;", name, fstat->tag->id ) )
      bug( "%s: could not update database with new name.", __FUNCTION__ );
}

inline void set_stat_style( STAT_FRAMEWORK *fstat, bool value )
{
   fstat->pool = value;
   if( !strcmp( fstat->tag->created_by, "null" ) ) return;
   if( !quick_query( "UPDATE `stat_frameworks` SET type=%d WHERE statFrameworkID=%d;", (int)value, fstat->tag->id ) )
      bug( "%s: could not update database with new name.", __FUNCTION__ );
}

inline int  get_stat_total( STAT_INSTANCE *stat )
{
   return ( stat->perm_stat + stat->mod_stat );
}

inline int  get_stat_effective_perm( STAT_INSTANCE *stat )
{
   return urange( stat->framework->softfloor, stat->perm_stat, stat->framework->softcap );
}

inline int  get_stat_effective_mod( STAT_INSTANCE *stat )
{
   return urange( ( stat->framework->hardfloor - stat->framework->softfloor ), stat->mod_stat, ( stat->framework->hardcap - stat->framework->softcap ) );
}

inline int  get_stat_value( STAT_INSTANCE *stat )
{
   int perm = urange( stat->framework->softfloor, stat->perm_stat, stat->framework->softcap );
   return urange( stat->framework->hardfloor, ( perm + stat->mod_stat ), stat->framework->hardcap );
}

inline int  get_stat_current( STAT_INSTANCE *stat )
{
   return stat->mod_stat;
}

inline int  get_stat_max( STAT_INSTANCE *stat )
{
   return stat->perm_stat;
}

inline void set_stat_current( STAT_INSTANCE *stat, int value )
{
   if( value > stat->perm_stat ) value = stat->perm_stat;
   else if( value < stat->framework->softfloor ) value = stat->framework->softfloor;
   lua_set_stat( stat, ( value - stat->mod_stat ) );
   stat->mod_stat = value;
   if( !quick_query( "UPDATE `entity_stats` SET mod_stat=%d WHERE statFrameworkID=%d AND owner=%d;", value, stat->framework->tag->id, stat->owner->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}

inline void set_stat_max( STAT_INSTANCE *stat, int value )
{
   if( value > stat->framework->softcap ) value = stat->framework->softcap;
   else if( value < stat->framework->softfloor ) value = stat->framework->softfloor;
   stat->perm_stat = value;
   if( !quick_query( "UPDATE `entity_stats` SET perm_stat=%d WHERE statFrameworkID=%d AND owner=%d;", value, stat->framework->tag->id, stat->owner->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}

inline void inc_pool_stat( STAT_INSTANCE *stat, int value )
{
   if( ( value + stat->mod_stat ) > stat->perm_stat ) value = stat->perm_stat - stat->mod_stat;
   stat->mod_stat += value;
   lua_set_stat( stat, value );
   if( !quick_query( "UPDATE `entity_stats` SET mod_stat=%d WHERE statFrameworkID=%d AND owner=%d;", stat->mod_stat, stat->framework->tag->id, stat->owner->tag->id ) )
      bug( "%s: coudl not update database with new value.", __FUNCTION__ );
}

inline void dec_pool_stat( STAT_INSTANCE *stat, int value )
{
   if( ( stat->mod_stat - value ) < stat->framework->softfloor ) value = stat->mod_stat - stat->framework->softfloor;
   stat->mod_stat -= value;
   lua_set_stat( stat, ( value * -1 ) );
   if( !quick_query( "UPDATE `entity_stats` SET mod_stat=%d WHERE statFrameworkID=%d AND owner=%d;", stat->mod_stat, stat->framework->tag->id, stat->owner->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}

inline void set_perm_stat( STAT_INSTANCE *stat, int value )
{
   lua_set_stat( stat, get_effective_change( stat->framework->softcap, stat->framework->softfloor, get_stat_total( stat ), get_stat_value( stat ), ( value - stat->perm_stat ) ) );
   stat->perm_stat = value;
   if( !quick_query( "UPDATE `entity_stats` SET perm_stat=%d WHERE statFrameworkID=%d AND owner=%d;", value, stat->framework->tag->id, stat->owner->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}

inline void add_perm_stat( STAT_INSTANCE *stat, int value )
{
   /* no lua for the moment */
   lua_set_stat( stat, get_effective_change( stat->framework->softcap, stat->framework->softfloor, get_stat_total( stat ), get_stat_value( stat ), value ) );
   stat->perm_stat += value;
   if( !quick_query( "UPDATE `entity_stats` SET perm_stat=%d WHERE statFrameworkID=%d AND owner=%d;", value, stat->framework->tag->id, stat->owner->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}

inline void set_mod_stat( STAT_INSTANCE *stat, int value )
{
   lua_set_stat( stat, get_effective_change( stat->framework->hardcap, stat->framework->hardfloor, get_stat_total( stat ), get_stat_value( stat ), ( value - stat->mod_stat ) ) );
   stat->mod_stat = value;
   if( !quick_query( "UPDATE `entity_stats` SET mod_stat=%d WHERE statFrameworkID=%d AND owner=%d;", value, stat->framework->tag->id, stat->owner->tag->id ) )
      bug( "%s: could not update databaes with new value.", __FUNCTION__ );
}

inline void add_mod_stat( STAT_INSTANCE *stat, int value )
{
   lua_set_stat( stat, get_effective_change( stat->framework->hardcap, stat->framework->hardfloor, get_stat_total( stat ), get_stat_value( stat ), value ) );
   stat->mod_stat += value;
   if( !quick_query( "UPDATE `entity_stats` SET mod_stat=%d WHERE statFrameworkID=%d AND owner=%d;", value, stat->framework->tag->id, stat->owner->tag->id ) )
      bug( "%s: could not update databaes with new value.", __FUNCTION__ );

}
inline void set_stat_owner( STAT_INSTANCE *stat, ENTITY_INSTANCE *owner )
{
   if( owner != stat->owner ) DetachFromList( stat, stat->owner->stats );
   if( !quick_query( "UPDATE `entity_stats` SET owner=%d WHERE statFrameworkID=%d and owner=%d;", owner->tag->id, stat->framework->tag->id, stat->owner->tag->id ) )
      bug( "%s: could not update databse with new owner.", __FUNCTION__ );
   stat->owner = owner;
}

inline void restore_pool_stats( ENTITY_INSTANCE *instance )
{
   STAT_INSTANCE *stat;
   ITERATOR Iter;
   AttachIterator( &Iter, instance->stats );
   while( ( stat = (STAT_INSTANCE *)NextInList( &Iter ) ) != NULL )
      if( stat->framework->pool )
         set_mod_stat( stat, stat->perm_stat);
   DetachIterator( &Iter );
}

void lua_set_stat( STAT_INSTANCE *stat, int change  )
{
   int ret, top = lua_gettop( lua_handle );

   if( change == 0 )
      return;

   if( !s_script_exists( stat->framework ) )
      return;

   if( change > 0 )
   {
      prep_stack( get_stat_instance_script_path( stat ), "onStatGain" );
      push_instance( stat->owner, lua_handle );
      lua_pushnumber( lua_handle, change );
      if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
         bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, get_stat_instance_script_path( stat ), lua_tostring( lua_handle, -1 ) );
   }
   else
   {
      prep_stack( get_stat_instance_script_path( stat ), "onStatLose" );
      push_instance( stat->owner, lua_handle );
      lua_pushnumber( lua_handle, change );
      if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
         bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, get_stat_instance_script_path( stat ), lua_tostring( lua_handle, -1 ) );

   }

   lua_settop( lua_handle, top );
   return;
}

FILE *open_s_script( STAT_FRAMEWORK *fstat, const char *permissions )
{
   FILE *script;
   script = fopen( get_stat_framework_script_path( fstat ), permissions );
   return script;
}

bool s_script_exists( STAT_FRAMEWORK *fstat )
{
   FILE *script;

   if( !strcmp( fstat->tag->created_by, "null" ) )
      return FALSE;

   if( ( script = fopen( quick_format( "../scripts/stats/%d.lua", fstat->tag->id ), "r" ) ) == NULL )
      return FALSE;

   fclose( script );
   return TRUE;

}

void init_s_script( STAT_FRAMEWORK *fstat, bool force )
{
   FILE *temp, *dest;

   if( s_script_exists( fstat ) && !force )
      return;

   if( ( temp = fopen( "../scripts/templates/stat.lua", "r" ) ) == NULL )
   {
      bug( "%s: could not open the template.", __FUNCTION__ );
      return;
   }

   if( ( dest = fopen( quick_format( "../scripts/stats/%d.lua", fstat->tag->id ), "w" ) ) == NULL )
   {
      bug( "5s: could not open the script.", __FUNCTION__ );
      return;
   }
   copy_flat_file( dest, temp );
   fclose( dest );
   fclose( temp );
   return;
}

const char *print_s_script( STAT_FRAMEWORK *fstat )
{
   const char *buf;
   FILE *fp;

   if( !s_script_exists( fstat ) )
      return "This framework has no script.";

   if( ( fp = open_s_script( fstat, "r" ) ) == NULL )
      return "There was a pretty bug error.";

   buf = fread_file( fp );
   fclose( fp );
   return buf;
}





