/* the library file lua_triggers written by Davenge */

#include "mud.h"

inline void onInstanceInit_trigger( ENTITY_INSTANCE *entity )
{
   int ret;
   prep_stack( get_frame_script_path( entity->framework ), "onInstanceInit" );
   push_framework( entity->framework, lua_handle );
   push_instance( entity, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path %s\r\n - error message: %s.", __FUNCTION__, ret, get_frame_script_path( entity->framework ), lua_tostring( lua_handle, -1 ) );

}

inline void onDeath_trigger( ENTITY_INSTANCE *entity )
{
   int ret;
   prep_stack( get_instance_script_path( entity ), "onDeath" );
   push_instance( entity, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 1, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, get_instance_script_path( entity ), lua_tostring( lua_handle, -1 ) );
}

inline void onSpawn_trigger( ENTITY_INSTANCE *entity )
{
   int ret;
   prep_stack( get_instance_script_path( entity ), "onSpawn" );
   push_instance( entity, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 1, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, get_instance_script_path( entity ), lua_tostring( lua_handle, -1 ) );
}

inline void onEntityEnter_trigger( ENTITY_INSTANCE *entity )
{
   SPECIFICATION *script;
   int ret;
   if( ( script = has_spec( entity->contained_by, "onEntityEnter" ) ) == NULL || ( script != NULL && script->value == 0 ) )
      return;
   prep_stack( get_script_path_from_spec( script ), "onEntityEnter" );
   push_instance( entity->contained_by, lua_handle );
   push_instance( entity, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, get_script_path_from_spec( script ), lua_tostring( lua_handle, -1 ) );
}

inline void onEntityLeave_trigger( ENTITY_INSTANCE *entity )
{
   SPECIFICATION *script;
   int ret;
   if( ( script = has_spec( entity->contained_by, "onEntityLeave" ) ) == NULL || ( script != NULL && script->value == 0 ) )
      return;
   prep_stack( get_script_path_from_spec( script ), "onEntityLeave" );
   push_instance( entity->contained_by, lua_handle );
   push_instance( entity, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, get_script_path_from_spec( script ), lua_tostring( lua_handle, -1 ) );
}

inline void onEntering_trigger( ENTITY_INSTANCE *entity )
{
   SPECIFICATION *script;
   int ret;
   if( ( script = has_spec( entity, "onEntering" ) ) == NULL || ( script != NULL && script->value == 0 ) )
      return;
   prep_stack( get_script_path_from_spec( script ), "onEntering" );
   push_instance( entity->contained_by, lua_handle );
   push_instance( entity, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, get_script_path_from_spec( script ), lua_tostring( lua_handle, -1 ) );
}

inline void onLeaving_trigger( ENTITY_INSTANCE *entity )
{
   SPECIFICATION *script;
   int ret;
   if( ( script = has_spec( entity, "onLeaving" ) ) == NULL || ( script != NULL && script->value == 0 ) )
      return;
   prep_stack( get_script_path_from_spec( script ), "onLeaving" );
   push_instance( entity->contained_by, lua_handle );
   push_instance( entity, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, get_script_path_from_spec( script ), lua_tostring( lua_handle, -1 ) );
}

inline void onGreet_trigger( ENTITY_INSTANCE *greeter, ENTITY_INSTANCE *enterer )
{
   SPECIFICATION *script;
   int ret;
   if( ( script = has_spec( greeter, "onGreet" ) ) == NULL || ( script != NULL && script->value == 0 ) )
      return;
   prep_stack( get_script_path_from_spec( script ), "onGreet" );
   push_instance( greeter, lua_handle );
   push_instance( enterer, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, get_script_path_from_spec( script ), lua_tostring( lua_handle, -1 ) );
}

inline void onFarewell_trigger( ENTITY_INSTANCE *waver, ENTITY_INSTANCE *leaver )
{
   SPECIFICATION *script;
   int ret;
   if( ( script = has_spec( waver, "onFarewell" ) ) == NULL || ( script != NULL && script->value == 0 ) )
      return;
   prep_stack( get_script_path_from_spec( script ), "onFarewell" );
   push_instance( waver, lua_handle );
   push_instance( leaver, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, get_script_path_from_spec( script ), lua_tostring( lua_handle, -1 ) );
}

inline void onSay_trigger( ENTITY_INSTANCE *entity )
{

}

inline void onGive_trigger( ENTITY_INSTANCE *entity )
{

}

