/* methods pertaining to lua boxed entity frameworks written by Davenge */

#include "mud.h"

const struct luaL_Reg EntityFrameworkLib_m[] = {
   /* getters */
   { "getID", getFrameID },
   { "getName", getFrameName },
   { "getShort", getFrameShort },
   { "getLong", getFrameLong },
   { "getDesc", getFrameDesc },
   { "getSpec", getFrameSpec },
   { "getInheritance", getFrameInheritance },
   { "getHeight", getFrameHeight },
   { "getWeight", getFrameWeight },
   { "getWidth", getFrameWidth },
   /* setters */
   { "setName", setFrameName },
   { "setShort", setFrameShort },
   { "setLong", setFrameLong },
   { "setDesc", setFrameDesc },
   /* actions */
   { "inherits", luaInherits },
   { NULL, NULL }
};

const struct luaL_Reg EntityFrameworkLib_f[] = {
   { "getFramework", getFramework },
   { "newInheritedFrame", newInheritedFramework },
   { NULL, NULL }
};

int luaopen_EntityFrameworkLib( lua_State *L )
{
   luaL_newmetatable( L, "EntityFramework.meta" );

   lua_pushvalue( L, -1 );
   lua_setfield( L, -2, "__index" );

   lua_pushcfunction( L, EntityFrameworkGC );
   lua_setfield( L, -2, "__gc" );

   luaL_setfuncs( L, EntityFrameworkLib_m, 0 );

   luaL_newlib( L, EntityFrameworkLib_f );
   return 1;
}

int EntityFrameworkGC( lua_State *L )
{
   ENTITY_FRAMEWORK **frame;
   frame = (ENTITY_FRAMEWORK **)lua_touserdata( L, -1 );
   *frame = NULL;
   return 0;

}

/* lib functions */

int getFramework( lua_State *L )
{
   ENTITY_FRAMEWORK *framework;
   ENTITY_INSTANCE *instance;

   switch( lua_type( L, -1 ) )
   {
      default:
         bug( "%s: passed a bad argument.", __FUNCTION__ );
          lua_pushnil( L );
         return 1;
      case LUA_TUSERDATA:
         if( ( instance = *(ENTITY_INSTANCE **)luaL_checkudata( L, -1, "EntityInstance.meta" ) ) == NULL )
         {
            bug( "%s: passed non-instance argument.", __FUNCTION__ );
            lua_pushnil( L );
            return 1;
         }
         framework = instance->framework;
         break;
      case LUA_TNUMBER:
         if( ( framework = get_framework_by_id( lua_tonumber( L, -1 ) ) ) == NULL )
         {
            bug( "%s: no frame with the ID %d", __FUNCTION__, lua_tonumber( L, -1 ) );
            lua_pushnil( L );
            return 1;
         }
         break;
      case LUA_TSTRING:
         if( ( framework = get_framework_by_name( lua_tostring( L, -1 ) ) ) == NULL )
         {
            bug( "%s: no frame with the name %s", __FUNCTION__, lua_tonumber( L, -1 ) );
            lua_pushnil( L );
            return 1;
         }
         break;
   }
   push_framework( framework, L );
   return 1;
}

int newInheritedFramework( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;
   ENTITY_FRAMEWORK *iFrame;

   DAVLUACM_FRAME_NIL( frame, L );

   iFrame = init_eFramework();
   set_to_inherited( iFrame );
   iFrame->inherits = frame;
   new_eFramework( iFrame );
   push_framework( iFrame, L );
   return 1;
}

/* getters */
int getFrameID( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NIL( frame, L );
   lua_pushnumber( L, frame->tag->id );
   return 1;
}

int getFrameName( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NIL( frame, L );
   lua_pushstring( L, chase_name( frame ) );
   return 1;
}

int getFrameShort( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NIL( frame, L );
   lua_pushstring( L, chase_short_descr( frame ) );
   return 1;
}

int getFrameLong( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NIL( frame, L );
   lua_pushstring( L, chase_long_descr( frame ) );
   return 1;
}

int getFrameDesc( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NIL( frame, L );
   lua_pushstring( L, chase_description( frame ) );
   return 1;
}

int getFrameSpec( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;
   SPECIFICATION *spec;
   const char *spectype;

   DAVLUACM_FRAME_NIL( frame, L );
   if( ( spectype = luaL_checkstring( L, 2 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( spec = frame_has_spec( frame, spectype ) ) == NULL )
   {
      bug( "%s: no such spec %s.", __FUNCTION__, spectype );
      lua_pushnil( L );
      return 1;
   }
   push_specification( spec, L );
   return 1;
}

int getFrameInheritance( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NIL( frame, L );
   if( !frame->inherits )
      lua_pushnil( L );
   else
      push_framework( frame->inherits, L );
   return 1;
}

int getFrameHeight( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NIL( frame, L );
   lua_pushnumber( L, get_frame_height( frame, NULL ) );
   return 1;
}

int getFrameWeight( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NIL( frame, L );
   lua_pushnumber( L, get_frame_weight( frame, NULL ) );
   return 1;
}

int getFrameWidth( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NIL( frame, L );
   lua_pushnumber( L, get_frame_width( frame, NULL ) );
   return 1;
}

int setFrameName( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NONE( frame, L );
   if( lua_type( L, -1 ) != LUA_TSTRING )
   {
      bug( "%s: non-string value passed.", __FUNCTION__ );
      return 0;
   }
   set_frame_name( frame, lua_tostring( L, -1 ) );
   return 0;
}

int setFrameShort( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NONE( frame, L );
   if( lua_type( L, -1 ) != LUA_TSTRING )
   {
      bug( "%s: non-string value passed.", __FUNCTION__ );
      return 0;
   }
   set_frame_short_descr( frame, lua_tostring( L, -1 ) );
   return 0;

}

int setFrameLong( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NONE( frame, L );
   if( lua_type( L, -1 ) != LUA_TSTRING )
   {
      bug( "%s: non-string value passed.", __FUNCTION__ );
      return 0;
   }
   set_frame_long_descr( frame, lua_tostring( L, -1 ) );
   return 0;

}

int setFrameDesc( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   DAVLUACM_FRAME_NONE( frame, L );
   if( lua_type( L, -1 ) != LUA_TSTRING )
   {
      bug( "%s: non-string value passed.", __FUNCTION__ );
      return 0;
   }
   set_frame_description( frame, lua_tostring( L, -1 ) );
   return 0;
}


int luaInherits( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;
   const char *func_name;
   const char *cypher;
   int num_args;
   int x, ret;

   DAVLUACM_FRAME_NONE( frame, L );

   if( !frame->inherits )
      return 0;

   if( ( func_name = luaL_checkstring( L, 2 ) ) == NULL )
   {
      bug( "%s: no function name passed.", __FUNCTION__ );
      return 0;
   }
   if( ( cypher = luaL_checkstring( L, 3 ) ) == NULL )
   {
      bug( "%s: no cypher string passed.", __FUNCTION__ );
      return 0;
   }

   prep_stack_handle( L, get_frame_script_path( frame->inherits ), func_name );

   for( x = 0, num_args = strlen( cypher ); x < num_args; x++ )
   {
      ENTITY_FRAMEWORK *arg_frame;
      ENTITY_INSTANCE *arg_instance;

      switch( cypher[x] )
      {
         case 's':
            if( lua_type( L, ( 4 + x ) ) != LUA_TSTRING )
            {
               bug( "%s: bad/cypeer passed value, not a string at position %d.", __FUNCTION__, x );
               lua_pushnil( L );
               continue;
            }
            lua_pushstring( L, lua_tostring( L, ( 4 + x ) ) );
            break;
         case 'n':
            if( lua_type( L, ( 4 + x ) ) != LUA_TNUMBER )
            {
               bug( "%s: bad/cypher passed value, not a number at position %d.", __FUNCTION__, x );
               lua_pushnil( L );
               continue;
            }
            lua_pushnumber( L, lua_tonumber( L, ( 4 + x ) ) );
            break;
         case 'i':
            if( ( arg_instance = *(ENTITY_INSTANCE **)luaL_checkudata( L, ( 4 + x ), "EntityInstance.meta" ) ) == NULL )
            {
               bug( "%s: bad/cypher passed value, not an instance at position %d.", __FUNCTION__, x );
               lua_pushnil( L );
               continue;
            }
           push_instance( arg_instance, L );
            break;
         case 'f':
            if( ( arg_frame = *(ENTITY_FRAMEWORK **)luaL_checkudata( L, ( 4 + x ), "EntityFramework.meta" ) ) == NULL )
            {
               bug( "%s: bad/cypher passed value, not a framework at position %d.", __FUNCTION__, x );
               lua_pushnil( L );
               continue;
            }
            push_framework( arg_frame, L );
            break;
      }
   }
   if( ( ret = lua_pcall( L, num_args, 0, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s\r\n", __FUNCTION__, ret, get_frame_script_path( frame->inherits ), lua_tostring( L, -1 ) );
   return 0;

}

