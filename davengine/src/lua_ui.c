/* the library lua_ui handling all UI related calls to lua written by Davenge */

#include "mud.h"

void lua_ui_general( ENTITY_INSTANCE *instance, const char *uiSpec )
{
   SPECIFICATION *script;
   const char *path;
   int ret;

   if( ( script = has_spec( instance, uiSpec ) ) != NULL )
      path = get_script_path_from_spec( script );
   else
      path = "../scripts/settings/ui.lua";

   prep_stack( path, uiSpec );
   push_instance( instance, lua_handle );

   if( ( ret = lua_pcall( lua_handle, 1, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret: %d path: %s\r\n - error message: %s.", __FUNCTION__, ret, path, lua_tostring( lua_handle, -1 ) );
   return;
}

void lua_look( ENTITY_INSTANCE *instance, ENTITY_INSTANCE *looking_at )
{
   SPECIFICATION *script;
   const char *path;
   int ret;

   if( ( script = has_spec( instance, "uiLook" ) ) != NULL )
      path = get_script_path_from_spec( script );
   else
      path = "../scripts/settings/ui.lua";

   prep_stack( path, "uiLook" );
   push_instance( instance, lua_handle );
   if( !looking_at )
      lua_pushnil( lua_handle );
   else
      push_instance( looking_at, lua_handle );

   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret: %d path: %s\r\n - error message: %s.", __FUNCTION__, ret, path, lua_tostring( lua_handle, -1 ) );
   return;
}
