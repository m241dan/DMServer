/* the actual functions for lua utility written by Davenge */

#include "mud.h"

const struct luaL_Reg EntityVariablesLib_f[] = {
  { "bug",       lua_bug },
  { "getGlobal", lua_getGlobalVar },
  { "setGlobal", lua_setGlobalVar },
  { "callBack",  global_luaCallBack },
  { NULL, NULL }
};

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

void free_lua_func( LUA_FUNCTION *func )
{
   free( func->noise );
   free( func->header );
   free( func->body );
   free( func );
}

void free_lua_func_array( LUA_FUNCTION_ARRAY func_array )
{
   int x;

   for( x = 0; func_array[x] != NULL; x++ )
      free_lua_func( func_array[x] );
   free( func_array );
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

void free_lua_args( LLIST *list  )
{
   void *content;
   ITERATOR Iter;

   if( SizeOfList( list ) == 0 )
      return;

   AttachIterator( &Iter, list );
   while( ( content = NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( content, list );
      FREE( content );
   }
   DetachIterator( &Iter );
   return;
}

int luaopen_mud( lua_State *L )
{
   luaL_newlib( L, EntityVariablesLib_f );
   return 1;
}

bool prep_stack_handle( lua_State *handle, const char *file, const char *function )
{
   int ret;

   if( !handle )
   {
      bug( "%s: the lua stack isn't initialized", __FUNCTION__ );
      return FALSE;
   }

   lua_pushnil( handle );
   lua_setglobal( handle, function );

   if( ( ret = luaL_loadfile( handle, file ) ) != 0 )
   {
      if( ret != LUA_ERRFILE )
         bug( "%s: %s: %s\n\r", __FUNCTION__, function, lua_tostring( handle, -1 ) );
      lua_pop( handle, 1 );
      return FALSE;
   }

   if( ( ret = lua_pcall( handle, 0, 0, 0 ) ) != 0 )
   {
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, file, lua_tostring( handle, -1 ) );
      lua_pop( handle, 1 );
      return FALSE;
   }

   lua_getglobal( handle, function );
   if( lua_isnil( handle, -1 ) )
   {
      lua_pop( handle, -1 );
      return FALSE;
   }
   return TRUE;

}

const char *get_script_path_from_spec( SPECIFICATION *spec )
{
   int id;

   if( spec->owner[0] == 'f' )
   {
      id = atoi( spec->owner +1 );
      return get_frame_script_path( get_framework_by_id( id ) );
   }
   id = atoi( spec->owner );
   return get_instance_script_path( get_instance_by_id( id ) );
}

inline const char *get_frame_script_path( ENTITY_FRAMEWORK *frame )
{
   return quick_format( "../scripts/frames/%d.lua", frame->tag->id );
}

inline const char *get_instance_script_path( ENTITY_INSTANCE *instance )
{
   return quick_format( "../scripts/instances/%d.lua", instance->tag->id );
}

inline const char *get_stat_framework_script_path( STAT_FRAMEWORK *fstat )
{
   return quick_format( "../scripts/stats/%d.lua", fstat->tag->id );
}

inline const char *get_stat_instance_script_path( STAT_INSTANCE *stat )
{
   return quick_format( "../scripts/stats/%d.lua", stat->framework->tag->id );
}

inline void load_server_script( void )
{
   if( luaL_loadfile( lua_handle, "../scripts/settings/server.lua" ) || lua_pcall( lua_handle, 0, 0, 0 ) )
      return;
}

inline void load_combat_vars_script( void )
{
   if( luaL_loadfile( lua_handle, "../scripts/settings/combat_vars.lua" ) || lua_pcall( lua_handle, 0, 0, 0 ) )
      return;
}

inline void load_lua_command_tables( void )
{
   if( luaL_loadfile( lua_handle, "../scripts/settings/command_table.lua" ) || lua_pcall( lua_handle, 0, 0, 0 ) )
      return;
}
inline void load_lua_misc_vars( void )
{
   if( luaL_loadfile( lua_handle, "../scripts/settings/misc_variables.lua" ) || lua_pcall( lua_handle, 0, 0, 0 ) )
      bug( "%s: could not load misc variables\r\n - %s", __FUNCTION__, lua_tostring( lua_handle, -1 ) );
}

inline void load_lua_misc_funcs( void )
{
   if( luaL_loadfile( lua_handle, "../scripts/settings/misc_functions.lua" ) || lua_pcall( lua_handle, 0, 0, 0 ) )
      bug( "%s: could not load misc functions\r\n - %s", __FUNCTION__, lua_tostring( lua_handle, -1 ) );
}

inline void load_lua_element_table( void )
{
   if( luaL_loadfile( lua_handle, "../scripts/settings/element_table.lua" ) || lua_pcall( lua_handle, 0, 0, 0 ) )
      bug( "%s: could not load element table\r\n - %s", __FUNCTION__, lua_tostring( lua_handle, -1 ) );
}

void lua_server_settings( void )
{
   int top = lua_gettop( lua_handle );

   lua_getglobal( lua_handle, "mudport" );
   MUDPORT = lua_tonumber( lua_handle, -1 );

   lua_settop( lua_handle, top );
}

void lua_database_settings( void )
{
   int top = lua_gettop( lua_handle );

   lua_getglobal( lua_handle, "db_name" );
   DB_NAME = strdup( lua_tostring( lua_handle, -1 ) );
   lua_getglobal( lua_handle, "db_addr" );
   DB_ADDR = strdup( lua_tostring( lua_handle, -1 ) );
   lua_getglobal( lua_handle, "db_login" );
   DB_LOGIN = strdup( lua_tostring( lua_handle, -1 ) );
   lua_getglobal( lua_handle, "db_pass" );
   DB_PASSWORD = strdup( lua_tostring( lua_handle, -1 ) );
   lua_getglobal( lua_handle, "wiki_name" );
   WIKI_NAME = strdup( lua_tostring( lua_handle, -1 ) );

   lua_settop( lua_handle, top );
}

void lua_combat_settings( void )
{
   int top = lua_gettop( lua_handle );

   lua_getglobal( lua_handle, "automelee" );
   AUTOMELEE = lua_toboolean( lua_handle, -1 );
   lua_getglobal( lua_handle, "dodge_on" );
   DODGE_ON = lua_toboolean( lua_handle, -1 );
   lua_getglobal( lua_handle, "parry_on" );
   PARRY_ON = lua_toboolean( lua_handle, -1 );
   lua_getglobal( lua_handle, "miss_on" );
   MISS_ON = lua_toboolean( lua_handle, -1 );
   lua_getglobal( lua_handle, "automelee_delay" );
   BASE_MELEE_DELAY = lua_tonumber( lua_handle, -1 );
   lua_settop( lua_handle, top );
}

void lua_corpse_settings( void )
{
   int top = lua_gettop( lua_handle );

   lua_getglobal( lua_handle, "standard_corpse_decay" );
   CORPSE_DECAY = lua_tonumber( lua_handle, -1 );

   lua_settop( lua_handle, top );
}

void push_instance( ENTITY_INSTANCE *instance, lua_State *L )
{
   ENTITY_INSTANCE **box;

   if( !instance )
   {
       bug( "%s: trying to push a NULL instance.", __FUNCTION__ );
       lua_pushnil( L );
       return;
   }

   if( !strcmp( instance->tag->created_by, "null" ) )
   {
      bug( "%s: bad instance trying to be pushed, created_by null", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   box = (ENTITY_INSTANCE **)lua_newuserdata( L, sizeof( ENTITY_INSTANCE * ) );
   luaL_getmetatable( L, "EntityInstance.meta" );
   if( lua_isnil( L, -1 ) )
   {
      bug( "%s: EntityInstance.meta is missing.", __FUNCTION__ );
      lua_pop( L, -1 ); /* pop meta */
      lua_pop( L, -1 ); /* pop box */
      lua_pushnil( L );
      return;
   }
   lua_setmetatable( L, -2 );

   *box = instance;
   return;
}

void push_framework( ENTITY_FRAMEWORK *frame, lua_State *L )
{
   ENTITY_FRAMEWORK **box;

   if( !frame )
   {
      bug( "%s: trying to push a NULL frame.", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   if( !strcmp( frame->tag->created_by, "null" ) )
   {
      bug( "%s: bad framework trying to be pushed, created_by null", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   box = (ENTITY_FRAMEWORK **)lua_newuserdata( L, sizeof( ENTITY_FRAMEWORK * ) );
   luaL_getmetatable( L, "EntityFramework.meta" );
   if( lua_isnil( L, -1 ) )
   {
      bug( "%s: EntityFramework.meta is missing.", __FUNCTION__ );
      lua_pop( L, -1 );
      lua_pop( L, -1 );
      lua_pushnil( L );
      return;
   }
   lua_setmetatable( L, -2 );

   *box = frame;
   return;
}

void push_specification( SPECIFICATION *spec, lua_State *L )
{
   SPECIFICATION **box;

   if( !spec )
   {
      bug( "%s: trying to push a NULL spec.", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   box = (SPECIFICATION **)lua_newuserdata( L, sizeof( SPECIFICATION * ) );
   luaL_getmetatable( L, "Specification.meta" );
   if( lua_isnil( L, -1 ) )
   {
      bug( "%s: Specification.meta is missing.", __FUNCTION__ );
      lua_pop( L, -1 );
      lua_pop( L, -1 );
      lua_pushnil( L );
      return;
   }
   lua_setmetatable( L, -2 );

   *box = spec;
   return;
}

void push_damage( DAMAGE *dmg, lua_State *L )
{
   DAMAGE **box;

   if( !dmg )
   {
      bug( "%s: trying to push a NULL dmg.", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   box = (DAMAGE **)lua_newuserdata( L, sizeof( DAMAGE * ) );
   luaL_getmetatable( L, "Damage.meta" );
   if( lua_isnil( L, -1 ) )
   {
      bug( "%s: Damage.meta is missing.", __FUNCTION__ );
      lua_pop( L, -1 );
      lua_pop( L, -1 );
      lua_pushnil( L );
      return;
   }
   lua_setmetatable( L, -2 );
   *box = dmg;
   return;
}

void push_timer( TIMER *timer, lua_State *L )
{
   TIMER **box;

   if( !timer )
   {
      bug( "%s: trying to push a NULL timer.", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   box = (TIMER **)lua_newuserdata( L, sizeof( TIMER * ) );
   luaL_getmetatable( L, "Timers.meta" );
   if( lua_isnil( L, -1 ) )
   {
      bug( "%s: Timers.meta is missing.", __FUNCTION__ );
      lua_pop( L, -1 );
      lua_pop( L, -1 );
      lua_pushnil( L );
      return;
   }
   lua_setmetatable( L, -2 );
   *box = timer;
   return;
}

void push_account( ACCOUNT_DATA *account, lua_State *L )
{
   ACCOUNT_DATA **box;

   if( !account )
   {
      bug( "%s: tryin gto push a NULL account.", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   box = (ACCOUNT_DATA **)lua_newuserdata( L, sizeof( ACCOUNT_DATA * ) );
   luaL_getmetatable( L, "Account.meta" );
   if( lua_isnil( L, -1 ) )
   {
      bug( "%s: Account.meta is missing.", __FUNCTION__ );
      lua_pop( L, -1 );
      lua_pop( L, -1 );
      lua_pushnil( L );
      return;
   }
   lua_setmetatable( L, -2 );
   *box = account;
   return;
}

void push_nanny( NANNY_DATA *nanny, lua_State *L )
{
   NANNY_DATA **box;

   if( !nanny )
   {
      bug( "%s: tryin gto push a NULL account.", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   box = (NANNY_DATA **)lua_newuserdata( L, sizeof( NANNY_DATA * ) );
   luaL_getmetatable( L, "Nanny.meta" );
   if( lua_isnil( L, -1 ) )
   {
      bug( "%s: Nanny.meta is missing.", __FUNCTION__ );
      lua_pop( L, -1 );
      lua_pop( L, -1 );
      lua_pushnil( L );
      return;
   }
   lua_setmetatable( L, -2 );
   *box = nanny;
   return;
}

void push_socket( D_SOCKET *socket, lua_State *L )
{
   D_SOCKET **box;

   if( !socket )
   {
      bug( "%s: tryin gto push a NULL socket.", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   box = (D_SOCKET **)lua_newuserdata( L, sizeof( D_SOCKET * ) );
   luaL_getmetatable( L, "Socket.meta" );
   if( lua_isnil( L, -1 ) )
   {
      bug( "%s: Socket.meta is missing.", __FUNCTION__ );
      lua_pop( L, -1 );
      lua_pop( L, -1 );
      lua_pushnil( L );
      return;
   }
   lua_setmetatable( L, -2 );
   *box = socket;
   return;

}

void *check_meta( lua_State *L, int index, const char *meta_name )
{
   void *object = lua_touserdata( L, index );

   if( !object )
      return NULL;

   if( lua_getmetatable( L, index ) )
   {
      luaL_getmetatable( L, meta_name );
      if( !lua_rawequal( L, -1, -2 ) )
         object = NULL;
      lua_pop( L, 2 );
      return object;
   }
   else
      return NULL;
}

int lua_bug( lua_State *L )
{
   switch( lua_type( L, -1 ) )
   {
      case LUA_TNUMBER:
         bug( "%s: Number Error %d.", __FUNCTION__, lua_tonumber( L, -1 ) );
         break;
      case LUA_TSTRING:
         bug( "%s: %s.", __FUNCTION__, lua_tostring( L, -1 ) );
         break;
   }
   return 0;
}

int lua_getGlobalVar( lua_State *L )
{
   EVAR *var;
   const char *var_name;

   if( ( var_name = luaL_checkstring( L, -1 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( var = get_global_var( var_name ) ) == NULL )
   {
      bug( "%s: no global var named %s.", __FUNCTION__, var_name );
      lua_pushnil( L );
      return 1;
   }
   switch( var->type )
   {
      default: lua_pushnil( L ); return 1;
      case VAR_INT:
         lua_pushnumber( L, atoi( var->value ) );
         return 1;
      case VAR_STR:
         lua_pushstring( L, var->value );
         return 1;
   }
   return 0;
}

int lua_setGlobalVar( lua_State *L )
{
   EVAR *var;
   const char *var_name;

   if( ( var_name = luaL_checkstring( L, -2 ) ) == NULL )
   {
      bug( "%s: no variable name passed.", __FUNCTION__ );
      return 0;
   }

   var = get_global_var( var_name );

   switch( lua_type( L, -1 ) )
   {
      default: bug( "%s: bad value passed.", __FUNCTION__ ); return 0;
      case LUA_TNUMBER:
         if( !var )
         {
            var = new_int_var( var_name, lua_tonumber( L, -1 ) );
            new_global_var( var );
            return 0;
         }
         if( var->type != VAR_INT )
            update_var_type( var, VAR_INT );
         update_var_value( var, itos( lua_tonumber( L, -1 ) ) );
         return 0;
      case LUA_TSTRING:
         if( !var )
         {
            var = new_str_var( var_name, lua_tostring( L, -1 ) );
            new_global_var( var );
            return 0;
         }
         if( var->type != VAR_STR )
            update_var_type( var, VAR_STR );
         update_var_value( var, lua_tostring( L, -1 ) );
         return 0;
   }
   return 0;
}

int global_luaCallBack( lua_State *L )
{
   EVENT_DATA *event;
   int callbackwhen; /* server pulses, ie .25 seconds. So 1 second = 4 */
   const char *func_name;
   const char *cypher;
   const char *path;
   int num_args;
   int x;

   if( ( path = luaL_checkstring( L, 1 ) ) == NULL )
   {
      bug( "%s: no path passed.", __FUNCTION__ );
      return 0;
   }

   if( ( callbackwhen = luaL_checknumber( L, 2 ) ) == 0 )
   {
      bug( "%s: having a callback with 0 seconds is not possible.", __FUNCTION__ );
      return 0;
   }

   if( ( func_name = luaL_checkstring( L, 3 ) ) == NULL )
   {
      bug( "%s: no function name passed.", __FUNCTION__ );
      return 0;
   }

   if( ( cypher = luaL_checkstring( L, 4 ) ) == NULL )
   {
      bug( "%s: no cypher string passed.", __FUNCTION__ );
      return 0;
   }
   event = alloc_event();
   num_args = strlen( cypher );

   for( x = num_args; x > 0; x-- )
   {
      ENTITY_FRAMEWORK *arg_frame;
      ENTITY_INSTANCE *arg_instance;
      char *arg_string;
      int  *arg_int;

      switch( cypher[x-1] )
      {
         case 's':
            if( lua_type( L, ( 4 + x ) ) != LUA_TSTRING )
            {
               bug( "%s: bad/cyper passed value, not a string at position %d.", __FUNCTION__, x );
               arg_string = strdup( "nil" );
               AttachToList( arg_string, event->lua_args );
               continue;
            }
            arg_string = strdup( lua_tostring( L, ( 4 + x ) ) );
            AttachToList( arg_string, event->lua_args );
            break;
         case 'n':
            CREATE( arg_int, int, 1 );
            if( lua_type( L, ( 4 + x ) ) != LUA_TNUMBER )
            {
               bug( "%s: bad/cypher passed value, not a number at position %d.", __FUNCTION__, x );
               *arg_int = 0;
               AttachToList( arg_int, event->lua_args );
               continue;
            }
            *arg_int = lua_tonumber( L, ( 4 + x ) );
            AttachToList( arg_int, event->lua_args );
            break;
         case 'i':
            CREATE( arg_int, int, 1 );
            if( ( arg_instance = *(ENTITY_INSTANCE **)luaL_checkudata( L, ( 4 + x ), "EntityInstance.meta" ) ) == NULL )
            {
               bug( "%s: bad/cypher passed value, not an entity instance at position %d.", __FUNCTION__, x );
               *arg_int = 0;
               AttachToList( arg_int, event->lua_args );
               continue;
            }
            *arg_int = arg_instance->tag->id;
            AttachToList( arg_int, event->lua_args );
            break;
         case 'f':
            CREATE( arg_int, int, 1 );
            if( ( arg_frame = *(ENTITY_FRAMEWORK **)luaL_checkudata( L, ( 4 + x ), "EntityFramework.meta" ) ) == NULL )
            {
               bug( "%s: bad/cypher passed value, not an entity framework at position %d.", __FUNCTION__, x );
               *arg_int = -1;
               AttachToList( arg_int, event->lua_args );
               continue;
            }
            *arg_int = arg_frame->tag->id;
            AttachToList( arg_int, event->lua_args );
            break;
      }
   }


   event->argument = strdup( func_name );
   event->lua_cypher = strdup( cypher );
   event->type = GLOBAL_EVENT_LUA_CALLBACK;
   event->fun = &event_global_lua_callback;
   add_event_lua( event, path, callbackwhen );
   return 0;



}

bool autowrite_init( ENTITY_INSTANCE *instance )
{
   typedef enum
   {
      FIND_INIT, FIND_END, FINISH
   } MODE;
   ENTITY_FRAMEWORK *frame = instance->framework;
   STAT_INSTANCE *stat;
   FILE *fp;
   char *script;
   ITERATOR Iter;
   char script_buf[MAX_BUFFER * 8], line[MAX_BUFFER];
   MODE mode;


   if( ( fp = open_f_script( frame, "r" ) ) == NULL )
   {
      bug( "%s: could not open the frameworks script.\r\n", __FUNCTION__ );
      return FALSE;
   }

   script = script_buf;
   snprintf( script_buf, ( MAX_BUFFER * 4 ), "%s", fread_file( fp ) );
   fclose( fp );

   if( ( fp = open_f_script( frame, "w" ) ) == NULL )
   {
      bug( "%s: could not open the framework's script to write.\r\n", __FUNCTION__ );
      return FALSE;
   }

   mode = FIND_INIT;
   while( script && script[0] != '\0' )
   {
      script = one_arg_delim_literal( script, line, '\n' );
      if( mode == FIND_INIT  )
      {
         fprintf( fp, "%s\n", line );
         if( !strcmp( line, AUTOWRITE_INIT ) )
         {
            AttachIterator( &Iter, instance->stats );
            while( ( stat = (STAT_INSTANCE *)NextInList( &Iter ) ) != NULL )
            {
               fprintf( fp, "   instance:setStatPerm( \"%s\", %d )\n", stat->framework->name, stat->perm_stat );
               if( stat->framework->pool )
                  fprintf( fp, "   instance:setStatMod( \"%s\", %d )\n", stat->framework->name, stat->perm_stat );
            }
            DetachIterator( &Iter );
            mode = FIND_END;
         }
         continue;
      }
      else if( mode == FIND_END )
      {
         if( !strcmp( line, ENDAUTOWRITE_INIT ) )
         {
            fprintf( fp, "%s\n", line );
            mode = FINISH;
            continue;
         }
         else
            continue;
      }
      else if( mode == FINISH )
      {
         fprintf( fp, "%s\n", line );
         fprintf( fp, "%s\n", script );
         break;
      }
   }
   fclose( fp );
   return TRUE;
}
