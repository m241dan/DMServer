/* the elements library written by Davenge */

#include "mud.h"

/* creation | deletion */
ELEMENT_FRAMEWORK *init_element_frame( void )
{
   ELEMENT_FRAMEWORK *frame;
   CREATE( frame, ELEMENT_FRAMEWORK, 1 );
   frame->name = strdup( "null" );
   frame->strong_against = AllocList();
   frame->weak_against = AllocList();
   frame->composition = AllocList();
   return frame;
}

void free_element_frame( ELEMENT_FRAMEWORK *frame )
{
   FREE( frame->name );
   clearlist( frame->strong_against );
   FreeList( frame->strong_against );
   clearlist( frame->weak_against );
   FreeList( frame->weak_against );
   clearlist( frame->composition ); /* this one probably needs a free_composition_list */
   FreeList( frame->composition );
   FREE( frame );
}

ELEMENT_INFO *init_element_info( void )
{
   ELEMENT_INFO *element;
   CREATE( element, ELEMENT_INFO, 1 );
   element->owner = NULL;
   element->frame = NULL;
   return element;
}

void free_element_info( ELEMENT_INFO *element )
{
   element->owner = NULL;
   element->frame = NULL;
   FREE( element );
}

void clear_eleinfo_list( LLIST *list )
{
   ELEMENT_INFO *info;
   ITERATOR Iter;

   AttachIterator( &Iter, list );
   while( ( info = (ELEMENT_INFO *)NextInList( &Iter ) ) != NULL )
      free_element_info( info );
   DetachIterator( &Iter );
}

COMPOSITION *init_composition( void )
{
   COMPOSITION *comp;
   CREATE( comp, COMPOSITION, 1 );
   comp->frame = NULL;
   return comp;
}

void free_composition( COMPOSITION *comp )
{
   comp->owner = NULL;
   comp->frame = NULL;
   FREE( comp );
}

void clear_comp_list( LLIST *list )
{
   COMPOSITION *comp;
   ITERATOR Iter;

   AttachIterator( &Iter, list );
   while( ( comp = (COMPOSITION *)NextInList( &Iter ) ) != NULL )
      free_composition( comp );
   DetachIterator( &Iter );
}

/* utility */
void load_elements_table( void )
{
   ELEMENT_FRAMEWORK *eleframe;
   int table_len, x, top = lua_gettop( lua_handle );

   lua_getglobal( lua_handle, "elements_table" );
   if( lua_isnil( lua_handle, -1 ) || lua_type( lua_handle, -1 ) != LUA_TTABLE )
   {
      bug( "%s: missing element table.", __FUNCTION__ );
      lua_pop( lua_handle, 1 );
      return;
   }
   lua_len( lua_handle, -1 );
   table_len = lua_tonumber( lua_handle, -1 );
   lua_pop( lua_handle, 1 );

   for( x = 1; x < (table_len+1); x++ )
   {
      lua_pushnumber( lua_handle, x );
      lua_gettable( lua_handle, -2 );
      if( lua_isnil( lua_handle, -1 ) || lua_type( lua_handle, -1 ) != LUA_TTABLE )
      {
         bug( "%s: bad element table entry %d", __FUNCTION__, x );
         continue;
      }
      eleframe = init_element_frame();
      if( !base_load_lua_element( eleframe, lua_handle ) )
      {
         free_element_frame( eleframe );
         continue;
      }
      AttachToList( eleframe, element_frameworks );
   }
   lua_settop( lua_handle, top );
   load_elements_list();
   return;
}

/* load a lua element on top of the stack */
bool base_load_lua_element( ELEMENT_FRAMEWORK *eleframe, lua_State *L )
{
   if( lua_isnil( L, -1 ) || lua_type( L, -1 ) != LUA_TTABLE )
   {
      bug( "%s: bad table onto of the stack.", __FUNCTION__ );
      return FALSE;
   }
   lua_pushstring( L, "name" );
   lua_gettable( L, -2 );

   eleframe->name = strdup( lua_tostring( L, -1 ) );
   lua_pop( L, 2 );
   return TRUE;
}

void load_elements_list( void )
{
   ELEMENT_FRAMEWORK *eleframe;
   ITERATOR Iter;

   AttachIterator( &Iter, element_frameworks );
   while( ( eleframe = (ELEMENT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      if( !get_element_from_lua_table( eleframe->name, lua_handle ) )
         continue;
      load_element( eleframe, lua_handle );
      lua_pop( lua_handle, 1 );
   }
   DetachIterator( &Iter );
}

void load_element( ELEMENT_FRAMEWORK *eleframe, lua_State *L )
{
   int top = lua_gettop( L );

   if( lua_isnil( L, -1 ) || lua_type( L, -1 ) != LUA_TTABLE )
   {
      bug( "%s: no table on top of the stack.", __FUNCTION__ );
      return;
   }
   load_element_strengths( eleframe, L );
   load_element_weaknesses( eleframe, L );
   load_element_composition( eleframe, L );

   lua_settop( L, top );
   return;
}

/* the following two functions could be factored into one */

void load_element_strengths( ELEMENT_FRAMEWORK *eleframe, lua_State *L )
{
   ELEMENT_FRAMEWORK *strength;
   int x, table_len, top = lua_gettop( L );
   if( lua_isnil( L, -1 ) || lua_type( L, -1 ) != LUA_TTABLE )
   {
      bug( "%s: no table on top of the stack..", __FUNCTION__ );
      return;
   }
   clearlist( eleframe->strong_against );
   lua_pushstring( L, "strong_against" );
   lua_gettable( L, -2 );
   if( lua_isnil( L, -1 ) || lua_type( L, -1 ) != LUA_TTABLE )
   {
      bug( "%s: no table on top of the stack...", __FUNCTION__ );
      lua_settop( L, top );
      return;
   }

   lua_len( L, -1 );
   table_len = lua_tonumber( L, -1 );
   lua_pop( L, 1 );

   for( x = 1; x < (table_len+1); x++ )
   {
      lua_pushnumber( L, x );
      lua_gettable( L, -2 );
      if( lua_isnil( L, -1 ) || lua_type( L, -1 ) != LUA_TSTRING )
      {
         bug( "%s: bad value at index %d.", __FUNCTION__, x );
         lua_pop( L, 1 );
         continue;
      }
      if( ( strength = get_element_framework( lua_tostring( L, -1 ) ) ) == NULL )
      {
         bug( "%s: no element with name %s loaded.", __FUNCTION__, lua_tostring( L, -1 ) );
         lua_pop( L, 1 );
         continue;
      }
      AttachToList( strength, eleframe->strong_against );
   }
   lua_settop( L, top );
}

void load_element_weaknesses( ELEMENT_FRAMEWORK *eleframe, lua_State *L )
{
   ELEMENT_FRAMEWORK *weakness;
   int x, table_len, top = lua_gettop( L );
   if( lua_isnil( L, -1 ) || lua_type( L, -1 ) != LUA_TTABLE )
   {
      bug( "%s: no table on top of the stack..", __FUNCTION__ );
      return;
   }
   clearlist( eleframe->weak_against );
   lua_pushstring( L, "weak_against" );
   lua_gettable( L, -2 );
   if( lua_isnil( L, -1 ) || lua_type( L, -1 ) != LUA_TTABLE )
   {
      bug( "%s: no table on top of the stack...", __FUNCTION__ );
      lua_settop( L, top );
      return;
   }

   lua_len( L, -1 );
   table_len = lua_tonumber( L, -1 );
   lua_pop( L, 1 );

   for( x = 1; x < (table_len+1); x++ )
   {
      lua_pushnumber( L, x );
      lua_gettable( L, -2 );
      if( lua_isnil( L, -1 ) || lua_type( L, -1 ) != LUA_TSTRING )
      {
         bug( "%s: bad value at index %d.", __FUNCTION__, x );
         lua_pop( L, 1 );
         continue;
      }
      if( ( weakness = get_element_framework( lua_tostring( L, -1 ) ) ) == NULL )
      {
         bug( "%s: no element with name %s loaded.", __FUNCTION__, lua_tostring( L, -1 ) );
         lua_pop( L, 1 );
         continue;
      }
      AttachToList( weakness, eleframe->weak_against );
   }
   lua_settop( L, top );
}

void load_element_composition( ELEMENT_FRAMEWORK *eleframe, lua_State *L )
{
   COMPOSITION *comp;
   int x, table_len, top = lua_gettop( L );
   if( lua_isnil( L, -1 ) || lua_type( L, -1 ) != LUA_TTABLE )
   {
      bug( "%s: no table on top of the stack..", __FUNCTION__ );
      return;
   }
   clearlist( eleframe->composition ); /* may need clear_comp_list */
   lua_pushstring( L, "composition" );
   lua_gettable( L, -2 );
   if( lua_isnil( L, -1 ) || lua_type( L, -1 ) != LUA_TTABLE )
   {
      bug( "%s: no table on top of the stack...", __FUNCTION__ );
      lua_settop( L, top );
      return;
   }

   lua_len( L, -1 );
   table_len = lua_tonumber( L, -1 );
   lua_pop( L, 1 );

   for( x = 1; x < (table_len+1); x++ )
   {
      lua_pushnumber( L, x );
      lua_gettable( L, -2 );
      if( lua_isnil( L, -1 ) || lua_type( L, -1 ) != LUA_TTABLE )
      {
         bug( "%s: bad value at index %d.", __FUNCTION__, x );
         lua_pop( L, -1 );
         continue;
      }
      lua_pushnumber( L, 1 );
      lua_gettable( L, -2 );
      lua_pushnumber( L, 2 );
      lua_gettable( L, -3 );
      comp = init_composition();
      comp->frame = get_element_framework( lua_tostring( L, -2 ) );
      comp->amount = lua_tonumber( L, -1 );
      lua_pop( L, 3 );
      add_composition_to_element( comp, eleframe );
   }
   lua_settop( L, top );
}

/* getter */
ELEMENT_FRAMEWORK *get_element_framework( const char *name )
{
   ELEMENT_FRAMEWORK *eleframe;
   ITERATOR Iter;

   AttachIterator( &Iter, element_frameworks );
   while( ( eleframe = (ELEMENT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( eleframe->name, name ) )
         break;
   DetachIterator( &Iter );
   return eleframe;
}

bool get_element_from_lua_table( const char *name, lua_State *L )
{
   int ret, top = lua_gettop( L );

   prep_stack( "../scripts/settings/element_table.lua", "getElement" );
   lua_pushstring( L, name );
   if( ( ret = lua_pcall( L, 1, LUA_MULTRET, 0 ) ) )
   {
      bug( "%s: ret: %d path: %s\r\n - error message %s.", __FUNCTION__, ret, "../scripts/settings/element_table.lua", lua_tostring( lua_handle, -1 ) );
      lua_settop( L, top );
      return FALSE;
   }
   return TRUE;
}

int get_instance_element_data( ENTITY_INSTANCE *instance, const char *elename, int which )
{
   ELEMENT_INFO *info;
   ITERATOR Iter;
   int value = 0;

   AttachIterator( &Iter, instance->elements );
   while( ( info = (ELEMENT_INFO *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( info->frame->name, elename ) )
         break;
   DetachIterator( &Iter );

   if( info )
      value += get_info_data( info, which );
   value += get_framework_element_data( instance->framework, elename, which );

   return value;
}

int get_framework_element_data( ENTITY_FRAMEWORK *frame, const char *elename, int which )
{
   ELEMENT_INFO *info = NULL;
   ITERATOR Iter;
   int value = 0;

   while( frame )
   {
      AttachIterator( &Iter, frame->elements );
      while( ( info = (ELEMENT_INFO *)NextInList( &Iter ) ) != NULL )
         if( !strcasecmp( info->frame->name, elename ) )
            break;
      DetachIterator( &Iter );
      if( info )
         break;
      else if( frame->inherits )
         frame = frame->inherits;
      else
         return value;
   }
   if( info )
      value += get_info_data( info, which );
   return value;
}

int get_info_data( ELEMENT_INFO *info, int which )
{
   switch( which )
   {
      default: return 0;
      case INFO_PEN:
         return info->pen;
      case INFO_RES:
         return info->res;
      case INFO_POTENCY:
         return info->potency;
   }
}

void get_instance_composition( ENTITY_INSTANCE *instance, LLIST *comprised )
{
   ELEMENT_FRAMEWORK *eleframe;
   COMPOSITION *comp;
   ITERATOR Iter;

   AttachIterator( &Iter, element_frameworks );
   while( ( eleframe = (ELEMENT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      if( ( comp = get_composition_from_instance( instance, eleframe->name ) ) == NULL )
         continue;
      AttachToList( comp, comprised );
   }
   DetachIterator( &Iter );
   return;
}

void get_framework_composition( ENTITY_FRAMEWORK *frame, LLIST *comprised )
{
   ELEMENT_FRAMEWORK *eleframe;
   COMPOSITION *comp;
   ITERATOR Iter;

   AttachIterator( &Iter, element_frameworks );
   while( ( eleframe = (ELEMENT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      if( ( comp = get_composition_from_framework( frame, eleframe->name ) ) == NULL )
         continue;
      AttachToList( comp, comprised );
   }
   DetachIterator( &Iter );
   return;
}

COMPOSITION *get_composition_from_instance( ENTITY_INSTANCE *instance, const char *elename )
{
   COMPOSITION *comp;
   ITERATOR Iter;

   AttachIterator( &Iter, instance->elements );
   while( ( comp = (COMPOSITION *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( comp->frame->name, elename ) )
         break;
   DetachIterator( &Iter );

   if( !comp )
      comp = get_composition_from_framework( instance->framework, elename );

   return comp;
}

COMPOSITION *get_composition_from_framework( ENTITY_FRAMEWORK *frame, const char *elename )
{
   COMPOSITION *comp;
   ITERATOR Iter;

   while( frame )
   {
      AttachIterator( &Iter, frame->composition );
      while( ( comp = (COMPOSITION *)NextInList( &Iter ) ) != NULL )
         if( !strcasecmp( comp->frame->name, elename ) )
            break;
      DetachIterator( &Iter );
      if( comp )
         return comp;
      else if( frame->inherits )
         frame = frame->inherits;
      else
         break;
   }
   return NULL;
}

/* action */
inline void add_composition_to_element( COMPOSITION *comp, ELEMENT_FRAMEWORK *eleframe )
{
   comp->owner = eleframe;
   comp->ownertype = COMP_OWNER_ELEMENT;
   AttachToList( comp, eleframe->composition );
}

inline void add_composition_to_instance( COMPOSITION *comp, ENTITY_INSTANCE *instance )
{
   comp->owner = instance;
   comp->ownertype = COMP_OWNER_INSTANCE;
   AttachToList( comp, instance->composition );
   if( !quick_query( "INSERT INTO `compositions` VALUES( 'i%d', '%s', '%d' );", instance->tag->id, comp->frame->name, comp->amount ) )
      bug( "%s: could not insert into database new instance composition", __FUNCTION__ );
}

inline void add_composition_to_framework( COMPOSITION *comp, ENTITY_FRAMEWORK *frame )
{
   comp->owner = frame;
   comp->ownertype = COMP_OWNER_FRAME;
   AttachToList( comp, frame->composition );
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   if( !quick_query( "INSERT INTO `compositions` VALUES ( 'f%d', '%s', '%d' );", frame->tag->id, comp->frame->name, comp->amount ) )
      bug( "%s: could not insert into database new framework composition", __FUNCTION__ );
}

void set_composition_amount( COMPOSITION *comp, int amount )
{
   char identifier = 'i';
   int id;
   comp->amount = amount;

   if( !comp->owner )
      return;
   else if( comp->ownertype == COMP_OWNER_FRAME )
   {
      if( !strcmp( ((ENTITY_FRAMEWORK *)comp->owner)->tag->created_by, "null" ) )
         return;
       id = ((ENTITY_FRAMEWORK *)comp->owner)->tag->id;
       identifier = 'f';
   }
   else if( comp->ownertype == COMP_OWNER_INSTANCE )
      id = ((ENTITY_INSTANCE *)comp->owner)->tag->id;
   else
      return;
   if( !quick_query( "UPDATE `compositions` SET amount=%d WHERE owner='%c%d' AND element='%s';", comp->amount, identifier, id, comp->frame->name ) )
      bug( "%s: could not update database with instaces new composition amount.", __FUNCTION__ );
}


void print_element_list( LLIST *list )
{
   ELEMENT_FRAMEWORK *eleframe, *component;
   COMPOSITION *comp;
   ITERATOR Iter, IterTwo;
   char tempstring[MAX_BUFFER];
   int x = 0;

   AttachIterator( &Iter, list );
   while( ( eleframe = (ELEMENT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      bug( "%d: %s", x++, eleframe->name );

      mud_printf( tempstring, "Strengths: " );
      AttachIterator( &IterTwo, eleframe->strong_against );
      while( ( component = (ELEMENT_FRAMEWORK *)NextInList( &IterTwo ) ) != NULL )
      {
         mudcat( tempstring, component->name );
         mudcat( tempstring, ", " );
      }
      DetachIterator( &IterTwo );
      bug( "%s", tempstring );

      mud_printf( tempstring, "Weaknesses: " );
      AttachIterator( &IterTwo, eleframe->weak_against );
      while( ( component = (ELEMENT_FRAMEWORK *)NextInList( &IterTwo ) ) != NULL )
      {
         mudcat( tempstring, component->name );
         mudcat( tempstring, ", " );
      }
      DetachIterator( &IterTwo );
      bug( "%s", tempstring );

      memset( &tempstring[0], 0, sizeof( tempstring ) );
      AttachIterator( &IterTwo, eleframe->composition );
      while( ( comp = (COMPOSITION *)NextInList( &IterTwo ) ) != NULL )
         mudcat( tempstring, quick_format( "Composed of %d%% of %s\r\n", comp->amount, comp->frame->name ) );
      DetachIterator( &IterTwo );
      bug( "%s", tempstring );
   }
   DetachIterator( &Iter );
}
