/* functions for dynamic entity variables written by Davenge */

#include "mud.h"

EVAR *new_int_var( const char *name, int value )
{
   EVAR *var;

   CREATE( var, EVAR, 1 );
   var->name = strdup( name );
   var->value = strdup( itos( value ) );
   var->type = VAR_INT;
   return var;

}

EVAR *new_str_var( const char *name, const char *string )
{
   EVAR *var;

   CREATE( var, EVAR, 1 );
   var->name = strdup( name );
   var->value = strdup( string );
   var->type = VAR_STR;
   return var;
}

void free_var( EVAR *var )
{
   FREE( var->name );
   FREE( var->value );
   FREE( var );
   return;
}
void clear_evar_list( LLIST *list )
{
   EVAR *var;
   ITERATOR Iter;

   AttachIterator( &Iter, list );
   while( ( var = (EVAR *)NextInList( &Iter ) ) != NULL )
      free_var( var );
   DetachIterator( &Iter );
}

void db_load_var( EVAR *var, MYSQL_ROW *row )
{
   int counter = 0;

   var->name = strdup( (*row)[counter++] );
   var->value = strdup( (*row)[counter++] );
   var->owner = atoi( (*row)[counter++] );
   var->type = (bool)atoi( (*row)[counter++] );
   return;
}

inline void delete_variable_from_instance( EVAR *var, ENTITY_INSTANCE *instance )
{
   DetachFromList( var, instance->evars );
   if( !quick_query( "DELETE FROM `entity_variables` WHERE owner=%d;", instance->tag->id ) )
      bug( "%s: could not delete variable %s from instance %d from the database.", __FUNCTION__, var->name, instance->tag->id );
   free_var( var );
}

void new_global_var( EVAR *var )
{
   AttachToList( var, global_variables );
   var->owner = -1;
   if( !quick_query( "INSERT INTO `entity_variables` VALUES( '%s', '%s', %d, %d );", var->name, var->value, var->owner, (int)var->type ) )
      bug( "%s: could not add new global variable %s to database.", __FUNCTION__, var->name );
}

void new_entity_var( ENTITY_INSTANCE *entity, EVAR *var )
{
   AttachToList( var, entity->evars );
   var->owner = entity->tag->id;
   if( !quick_query( "INSERT INTO `entity_variables` VALUES( '%s', '%s', %d, %d );", var->name, var->value, var->owner, (int)var->type ) )
      bug( "%s: could not add variable %ds belonging to entity %s to the database.", __FUNCTION__, var->name, instance_name( entity ) );
}

inline EVAR *remove_global_var( const char *name )
{
   EVAR *var;
   if( ( var = get_global_var( name ) ) == NULL )
      return NULL;
   DetachFromList( var, global_variables );
   quick_query( quick_format( "DELETE FROM `entity_variables` WHERE name='%s' AND owner='-1';", var->name ) );
   return var;
}

inline void remove_and_free_global_var( const char *name )
{
   EVAR *var = remove_global_var( name );
   free_var( var );
}

inline EVAR *remove_entity_var( ENTITY_INSTANCE *entity, const char *name )
{
   EVAR *var;
   if( ( var = get_entity_var( entity, name ) ) == NULL )
      return NULL;
   DetachFromList( var, entity->evars );
   quick_query( quick_format( "DELETE FROM `entity_variables` WHERE name='%s' AND owner=%d;", var->name, var->owner ) );
   return var;
}

inline void remove_and_free_entity_var( ENTITY_INSTANCE *entity, const char *name )
{
   EVAR *var = remove_entity_var( entity, name );
   free_var( var );
}

void update_var_name( EVAR *var, const char *name )
{
   if( !quick_query( "UPDATE `entity_variables` SET name='%s' WHERE name='%s' AND owner=%d;", name, var->name, var->owner ) )
      bug( "%s: could not update variable name %s to %s in the database.", __FUNCTION__, var->name, name );
   FREE( var->name );
   var->name = strdup( name );
}

void update_var_value( EVAR *var, const char *value )
{
   if( !quick_query( "UPDATE `entity_variables` SET value='%s' WHERE name='%s' AND owner=%d;", value, var->name, var->owner ) )
      bug( "%s: could not update variable name %s value to %s in the database.", __FUNCTION__, var->name, value );
   FREE( var->value );
   var->value = strdup( value );
}

void update_var_owner( EVAR *var, ENTITY_INSTANCE *entity )
{
   if( !quick_query( "UPDATE `entity_variables` SET owner=%d WHERE name='%s' AND owner=%d;", entity->tag->id, var->name, var->owner ) )
      bug( "%s: could not update variable owner %d to %d in the database.", __FUNCTION__, var->owner, entity->tag->id );
   var->owner = entity->tag->id;
}

void update_var_type( EVAR *var, bool type )
{
   if( !quick_query( "UPDATE `entity_variables` SET type=%d WHERE name='%s' AND owner=%d;", type, var->name, var->owner ) )
      bug( "%s: could not update variable %s type to %d.", __FUNCTION__, var->name, type );
   var->type = type;
}

inline EVAR *get_global_var( const char *name )
{
   return get_var_list( name, global_variables );
}

inline EVAR *get_entity_var( ENTITY_INSTANCE *entity, const char *name )
{
   return get_var_list( name, entity->evars );
}

EVAR *get_var_list( const char *name, LLIST *list )
{
   EVAR *var;
   ITERATOR Iter;

   AttachIterator( &Iter, list );
   while( ( var = (EVAR *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( var->name, name ) )
         break;
   DetachIterator( &Iter );

   return var;
}

void load_global_vars( void )
{
   EVAR *var;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;

   list = AllocList();
   if( !db_query_list_row( list, "SELECT * FROM `entity_variables` WHERE owner='-1';" ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      CREATE( var, EVAR, 1 );
      db_load_var( var, &row );
      AttachToList( var, global_variables );
   }
   DetachIterator( &Iter );
   FreeList( list );

   return;
}

void load_entity_vars( ENTITY_INSTANCE *entity )
{
   EVAR *var;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT * FROM `entity_variables` WHERE owner=%d;", entity->tag->id ) ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      CREATE( var, EVAR, 1 );
      db_load_var( var, &row );
      AttachToList( var, entity->evars );
   }
   DetachIterator( &Iter );
   FreeList( list );

   return;
}


