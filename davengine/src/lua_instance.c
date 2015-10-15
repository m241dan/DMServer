/* methods pertaining to lua boxed entity instances written by Davenge */

#include "mud.h"

const struct luaL_Reg EntityInstanceLib_m[] = {
   /* getters */
   { "getName", getName },
   { "getShort", getShort },
   { "getLong", getLong },
   { "getDesc", getDesc },
   { "getID", getID },
   { "getLevel", getID },
   { "getItemFromInventory", getItemFromInventory },
   { "getSpec", getSpec },
   { "getFramework", getInstanceFramework },
   { "getContainer", getContainer },
   { "getVar", getVar },
   { "getStatMod", getStatMod },
   { "getStatPerm", getStatPerm },
   { "getStat", getStat },
   { "getStatValue", getStatEffectiveValue },
   { "getHome", getHome },
   { "getExitTo", getExitTo },
   { "getHeight", getHeight },
   { "getWeight", getWeight },
   { "getWidth", getWidth },
   { "getTarget", getTarget },
   /* setters */
   { "setLevel", setLevel },
   { "setStatMod", setStatMod },
   { "setStatPerm", setStatPerm },
   { "addStatMod", addStatMod },
   { "addStatPerm", addStatPerm },
   { "setVar", setVar },
   { "addSpec", addSpec },
   { "setHome", setHome },
   { "setHeightMod", setHeightMod },
   { "addHeightMod", addHeightMod },
   { "setWeightMod", setWeightMod },
   { "addWeightMod", addHeightMod },
   { "setWidthMod", setWeightMod },
   { "addWidthMod", addWidthMod },
   { "setTarget", setTarget },
   /* bools */
   { "isLive", isLive },
   { "isBuilder", isBuilder },
   { "hasItemInInventoryFramework", hasItemInInventoryFramework },
   { "isSameRoom", isSameRoom },
   { "isPlayer", isPlayer },
   { "isExit", isExit },
   { "isRoom", isRoom },
   { "isMob", isMob },
   { "isObj", isObj },
   /* actions */
   { "togglePlayer", luaTogglePlayer },
   { "callBack", luaCallBack },
   { "interp", luaEntityInstanceInterp },
   { "to", luaEntityInstanceTeleport },
   { "echo", luaEcho },
   { "echoAt", luaEchoAt },
   { "echoAround", luaEchoAround },
   { "frameCall", luaFrameCall },
   { "restore", luaInstanceRestore },
   /* iterators */
   { "eachInventory", luaEachInventory },
   { NULL, NULL } /* gandalf */
};

const struct luaL_Reg EntityInstanceLib_f[] = {
   { "getInstance", getInstance },
   { "new", luaNewInstance },
   { NULL, NULL } /* gandalf */
};

int luaopen_EntityInstanceLib( lua_State *L )
{
   luaL_newmetatable( L, "EntityInstance.meta" );

   lua_pushvalue( L, -1 );
   lua_setfield( L, -2, "__index" );

   lua_pushcfunction( L, EntityInstanceGC );
   lua_setfield( L, -2, "__gc" );

   luaL_setfuncs( L, EntityInstanceLib_m, 0 );

   luaL_newlib( L, EntityInstanceLib_f );
   return 1;
}

int EntityInstanceGC( lua_State *L )
{
   ENTITY_INSTANCE **instance;
   instance = (ENTITY_INSTANCE **)lua_touserdata( L, -1 );
   *instance = NULL;
   return 0;
}


int getInstance( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   switch( lua_type( L, -1 ) )
   {
      default:
         bug( "%s: passed non-string/integer.", __FUNCTION__ );
         lua_pushnil( L );
         return 1;
      case LUA_TSTRING:
         instance = get_instance_by_name( lua_tostring( L, -1 ) );
         break;
      case LUA_TNUMBER:
         instance = get_instance_by_id( lua_tonumber( L, -1 ) );
         break;
   }

   push_instance( instance, L );
   return 1;
}

int luaNewInstance( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   ENTITY_FRAMEWORK *frame;

   if( ( frame = *(ENTITY_FRAMEWORK **)luaL_checkudata( L, -1, "EntityFramework.meta" ) ) == NULL )
   {
      bug( "%s: bad meta table.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   instance = eInstantiate( frame );
   push_instance( instance, L );
   return 1;
}

int getName( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );

   lua_pushstring( L, instance_name( instance ) );
   return 1;
}

int getShort( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushstring( L, instance_short_descr( instance ) );
   return 1;
}

int getLong( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushstring( L, instance_long_descr( instance ) );
   return 1;
}

int getDesc( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushstring( L, instance_description( instance ) );
   return 1;
}

int getID( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushnumber( L, instance->tag->id );
   return 1;
}

int getLevel( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushnumber( L, instance->level );
   return 1;
}
int getItemFromInventory( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   ENTITY_INSTANCE *item;

   int top = lua_gettop( L );

   if( top != 2 )
   {
      bug( "%s: passed improper amount of arguments.", __FUNCTION__, top );
      lua_pushnil( L );
      return 1;
   }

   DAVLUACM_INSTANCE_NIL( instance, L );

   switch( lua_type( L, 2 ) )
   {
      default:
         bug( "%s: bad argument passed", __FUNCTION__ );
         lua_pushnil( L );
         return 1;
      case LUA_TNUMBER:
         if( ( item = instance_list_has_by_id( instance->contents, lua_tonumber( L, 2 ) ) ) == NULL )
         {
            lua_pushnil( L );
            return 1;
         }
         break;
      case LUA_TSTRING:
         if( ( item = instance_list_has_by_name_regex( instance->contents, lua_tostring( L, 2 ) ) ) == NULL )
         {
            lua_pushnil( L );
            return 1;
         }
         break;
   }
   push_instance( item, L );
   return 1;
}

int getSpec( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   SPECIFICATION *spec;
   const char *spectype;

   DAVLUACM_INSTANCE_NIL( instance, L );

   if( ( spectype = luaL_checkstring( L, 2 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( spec = has_spec( instance, spectype ) ) == NULL )
   {
      bug( "%s: no such spec %s.", __FUNCTION__, spectype );
      lua_pushnil( L );
      return 1;
   }
   push_specification( spec, L );
   return 1;
}

int getInstanceFramework( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   push_framework( instance->framework, L );
   return 1;
}

int getContainer( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   if( instance->contained_by )
      push_instance( instance->contained_by, L );
   else
      lua_pushnil( L );
   return 1;
}

int getVar( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   EVAR *var;
   const char *var_name;

   DAVLUACM_INSTANCE_NIL( instance, L );

   if( instance->tag->id <= -69 )
   {
      bug( "%s: builders have no vars", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( var_name = luaL_checkstring( L, -1 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( var = get_entity_var( instance, var_name ) ) == NULL )
   {
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

int getStatMod( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   STAT_INSTANCE *stat;
   const char *stat_name;

   DAVLUACM_INSTANCE_NIL( instance, L );
   if( instance->tag->id <= -69 )
   {
      bug( "%s: builders have no stats", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( stat_name = luaL_checkstring( L, -1 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( stat = get_stat_from_instance_by_name( instance, stat_name ) ) == NULL )
   {
      lua_pushnil( L );
      return 1;
   }
   lua_pushnumber( L, stat->mod_stat );
   return 1;
}

int getStatPerm( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   STAT_INSTANCE *stat;
   const char *stat_name;

   DAVLUACM_INSTANCE_NIL( instance, L );
   if( instance->tag->id <= -69 )
   {
      bug( "%s: builders have no stats", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( stat_name = luaL_checkstring( L, -1 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( stat = get_stat_from_instance_by_name( instance, stat_name ) ) == NULL )
   {
      lua_pushnil( L );
      return 1;
   }
   lua_pushnumber( L, stat->perm_stat );
   return 1;
}

int getStat( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   STAT_INSTANCE *stat;
   const char *stat_name;

   DAVLUACM_INSTANCE_NIL( instance, L );
   if( instance->tag->id <= -69 )
   {
      bug( "%s: builders have no stats", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( stat_name = luaL_checkstring( L, -1 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( stat = get_stat_from_instance_by_name( instance, stat_name ) ) == NULL )
   {
      lua_pushnil( L );
      return 1;
   }
   lua_pushnumber( L, stat->mod_stat + stat->perm_stat );
   return 1;
}

int getStatEffectiveValue( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   STAT_INSTANCE *stat;
   const char *stat_name;

   DAVLUACM_INSTANCE_NIL( instance, L );
   if( instance->tag->id <= -69 )
   {
      bug( "%s: builders have no stats", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( stat_name = luaL_checkstring( L, -1 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( stat = get_stat_from_instance_by_name( instance, stat_name ) ) == NULL )
   {
      lua_pushnil( L );
      return 1;
   }
   lua_pushnumber( L, get_stat_value( stat ) );
   return 1;
}

int getHome( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   if( !instance->home )
      lua_pushnil( L );
   else
      push_instance( instance->home, L );

   return 1;
}

int getExitTo( lua_State * L)
{
   ENTITY_INSTANCE *exit;
   ENTITY_INSTANCE *exit_to;

   DAVLUACM_INSTANCE_NIL( exit, L );
   if( ( exit_to = get_active_instance_by_id( get_spec_value( exit, "IsExit" ) ) ) == NULL )
      lua_pushnil( L );
   else
      push_instance( exit_to, L );
   return 1;
}

int getHeight( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushnumber( L, get_height( instance ) );
   return 1;
}

int getWeight( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushnumber( L, get_weight( instance ) );
   return 1;
}

int getWidth( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushnumber( L, get_width( instance ) );
   return 1;
}

int getTarget( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   if( !NO_TARGET( instance ) && TARGET_TYPE( instance ) == TARGET_INSTANCE )
      push_instance( GT_INSTANCE( instance ), L );
   else
      lua_pushnil( L );

   return 1;
}

int setLevel( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NONE( instance, L );
   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: expecting integer, did not get.", __FUNCTION__ );
      return 0;
   }
   set_instance_level( instance, lua_tonumber( L, -1 ) );
   return 0;
}

int setStatMod( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   STAT_INSTANCE *stat;
   const char *stat_name;
   int value;

   DAVLUACM_INSTANCE_NONE( instance, L );
   if( instance->tag->id <= -69 )
   {
      bug( "%s: builders have no stats", __FUNCTION__ );
      return 0;
   }

   if( ( stat_name = luaL_checkstring( L, -2 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      return 0;
   }

   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: no number passed.", __FUNCTION__ );
      return 0;
   }
   value = lua_tonumber( L, -1 );

   if( ( stat = get_stat_from_instance_by_name( instance, stat_name ) ) == NULL )
   {
      STAT_FRAMEWORK *fstat;
      if( ( fstat = get_stat_framework_by_name( stat_name ) ) == NULL )
      {
         bug( "%s: no such stat %s exists.", __FUNCTION__, stat_name );
         return 0;
      }
      add_stat_to_frame( fstat, instance->framework );
      stat_instantiate( instance, fstat );
      stat = get_stat_from_instance_by_name( instance, stat_name );
   }

   set_mod_stat( stat, value );
   return 0;
}

int setStatPerm( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   STAT_INSTANCE *stat;
   const char *stat_name;
   int value;

   DAVLUACM_INSTANCE_NONE( instance, L );
   if( instance->tag->id <= -69 )
   {
      bug( "%s: builders have no stats", __FUNCTION__ );
      return 0;
   }

   if( ( stat_name = luaL_checkstring( L, -2 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      return 0;
   }

   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: no number passed.", __FUNCTION__ );
      return 0;
   }
   value = lua_tonumber( L, -1 );

   if( ( stat = get_stat_from_instance_by_name( instance, stat_name ) ) == NULL )
   {
      STAT_FRAMEWORK *fstat;
      if( ( fstat = get_stat_framework_by_name( stat_name ) ) == NULL )
      {
         bug( "%s: no such stat %s exists.", __FUNCTION__, stat_name );
         return 0;
      }
      add_stat_to_frame( fstat, instance->framework );
      stat_instantiate( instance, fstat );
      stat = get_stat_from_instance_by_name( instance, stat_name );
   }

   set_perm_stat( stat, value );
   return 0;
}

int addStatMod( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   STAT_INSTANCE *stat;
   const char *stat_name;
   int value;

   DAVLUACM_INSTANCE_NONE( instance, L );
   if( instance->tag->id <= -69 )
   {
      bug( "%s: builders have no stats", __FUNCTION__ );
      return 0;
   }

   if( ( stat_name = luaL_checkstring( L, -2 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      return 0;
   }

   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: no number passed.", __FUNCTION__ );
      return 0;
   }
   value = lua_tonumber( L, -1 );

   if( ( stat = get_stat_from_instance_by_name( instance, stat_name ) ) == NULL )
   {
      STAT_FRAMEWORK *fstat;
      if( ( fstat = get_stat_framework_by_name( stat_name ) ) == NULL )
      {
         bug( "%s: no such stat %s exists.", __FUNCTION__, stat_name );
         return 0;
      }
      add_stat_to_frame( fstat, instance->framework );
      stat_instantiate( instance, fstat );
      stat = get_stat_from_instance_by_name( instance, stat_name );
   }

   add_mod_stat( stat, value );
   return 0;
}

int addStatPerm( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   STAT_INSTANCE *stat;
   const char *stat_name;
   int value;

   DAVLUACM_INSTANCE_NONE( instance, L );
   if( instance->tag->id <= -69 )
   {
      bug( "%s: builders have no stats", __FUNCTION__ );
      return 0;
   }

   if( ( stat_name = luaL_checkstring( L, -2 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      return 0;
   }

   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: no number passed.", __FUNCTION__ );
      return 0;
   }
   value = lua_tonumber( L, -1 );

   if( ( stat = get_stat_from_instance_by_name( instance, stat_name ) ) == NULL )
   {
      STAT_FRAMEWORK *fstat;
      if( ( fstat = get_stat_framework_by_name( stat_name ) ) == NULL )
      {
         bug( "%s: no such stat %s exists.", __FUNCTION__, stat_name );
         return 0;
      }
      add_stat_to_frame( fstat, instance->framework );
      stat_instantiate( instance, fstat );
      stat = get_stat_from_instance_by_name( instance, stat_name );
   }

   add_perm_stat( stat, value );
   return 0;
}

int setVar( lua_State *L )
{
   EVAR *var;
   ENTITY_INSTANCE *instance;
   const char *var_name;

   DAVLUACM_INSTANCE_NIL( instance, L );

   if( instance->tag->id <= -69 )
   {
      bug( "%s: builders have no vars", __FUNCTION__ );
      return 0;
   }

   if( ( var_name = luaL_checkstring( L, -2 ) ) == NULL )
   {
      bug( "%s: no variable name passed.", __FUNCTION__ );
      return 0;
   }

   var = get_entity_var( instance, var_name );

   switch( lua_type( L, -1 ) )
   {
      default: bug( "%s: bad value passed.", __FUNCTION__ ); return 0;
      case LUA_TNUMBER:
         if( !var )
         {
            var = new_int_var( var_name, lua_tonumber( L, -1 ) );
            new_entity_var( instance, var );
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
         update_var_value( var, lua_tostring( L, -2 ) );
         return 0;
   }
   return 0;
}

int addSpec( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   SPECIFICATION *spec;

   DAVLUACM_INSTANCE_NONE( instance, L );

   if( instance->tag->id <= -69 )
   {
      bug( "%s: don't spec builders plz", __FUNCTION__ );
      return 0;
   }

   if( ( spec = *(SPECIFICATION **)luaL_checkudata( L, 2, "Specification.meta" ) ) == NULL )
   {
      bug( "%s: no spec passed.", __FUNCTION__ );
      return 0;
   }

   if( strcmp( spec->owner, "null" ) )
   {
      bug( "%s: cannot add a spec that already has an owner.", __FUNCTION__ );
      return 0;
   }
   add_spec_to_instance( spec, instance );
   return 0;
}

int setHome( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NONE( instance, L );
   if( !instance->contained_by )
   {
      bug( "%s: cannot set home on something that is not contained.", __FUNCTION__ );
      return 0;
   }
   set_instance_home( instance );
   return 0;
}

int setHeightMod( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   int value;

   DAVLUACM_INSTANCE_NONE( instance, L );

   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: non-number passed as argument.", __FUNCTION__ );
      return 0;
   }
   if( ( value = lua_tonumber( L, -1 ) ) < 0 )
      value = 0;
   set_instance_height_mod( instance, value );
   return 0;
}

int addHeightMod( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   int value;

   DAVLUACM_INSTANCE_NONE( instance, L );

   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: non-number passed as argument.", __FUNCTION__ );
      return 0;
   }
   value = lua_tonumber( L, -1 );
   if( ( value + get_height( instance ) ) < 0 )
      set_instance_height_mod( instance, 0 );
   else
      set_instance_height_mod( instance, value + instance->height_mod );
   return 0;
}

int setWeightMod( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   int value;

   DAVLUACM_INSTANCE_NONE( instance, L );

   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: non-number passed as argument.", __FUNCTION__ );
      return 0;
   }
   if( ( value = lua_tonumber( L, -1 ) ) < 0 )
      value = 0;
   set_instance_weight_mod( instance, value );
   return 0;
}

int addWeightMod( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   int value;

   DAVLUACM_INSTANCE_NONE( instance, L );

   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: non-number passed as argument.", __FUNCTION__ );
      return 0;
   }
   value = lua_tonumber( L, -1 );
   if( ( value + get_weight( instance ) ) < 0 )
      set_instance_weight_mod( instance, 0 );
   else
      set_instance_weight_mod( instance, value + instance->weight_mod );
   return 0;
}

int setWidthMod( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   int value;

   DAVLUACM_INSTANCE_NONE( instance, L );

   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: non-number passed as argument.", __FUNCTION__ );
      return 0;
   }
   if( ( value = lua_tonumber( L, -1 ) ) < 0 )
      value = 0;
   set_instance_width_mod( instance, value );
   return 0;

}

int addWidthMod( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   int value;

   DAVLUACM_INSTANCE_NONE( instance, L );

   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: non-number passed as argument.", __FUNCTION__ );
      return 0;
   }
   value = lua_tonumber( L, -1 );
   if( ( value + get_width( instance ) ) < 0 )
      set_instance_width_mod( instance, 0 );
   else
      set_instance_width_mod( instance, value + instance->width_mod );
   return 0;
}

int setTarget( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   ENTITY_INSTANCE *new_target;

   DAVLUACM_INSTANCE_NONE( instance, L );

   if( ( new_target = luaL_checkudata( L, -1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: bad new target passed.", __FUNCTION__ );
      return 0;
   }
   set_target_i( instance->target, new_target );
   return 0;
}

int isLive( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_BOOL( instance, L );
   lua_pushboolean( L, (int)instance->live );
   return 1;
}

int isBuilder( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_BOOL( instance, L );
   lua_pushboolean( L, (int)instance->builder );
   return 1;
}

int isSameRoom( lua_State *L )
{
   ENTITY_INSTANCE *instance, *oth_instance;

   DAVLUACM_INSTANCE_BOOL( instance, L );

   if( ( oth_instance = *(ENTITY_INSTANCE **)luaL_checkudata( L, 2, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: argument passed is not of EntityInstance.meta", __FUNCTION__ );
      lua_pushboolean( L, 0 );
      return 1;
   }

   if( instance->contained_by == oth_instance->contained_by )
      lua_pushboolean( L, 1 );
   else
      lua_pushboolean( L, 0 );
   return 1;
}

int hasItemInInventoryFramework( lua_State *L )
{
   ENTITY_INSTANCE **instance_ref, *instance;
   ENTITY_INSTANCE *item;
   ENTITY_FRAMEWORK **frame_ref, *frame;
   ITERATOR Iter;
   int top = lua_gettop( L );
   bool found = FALSE;

   if( top != 2 )
   {
      bug( "%s: bad number of arguments passed %d", __FUNCTION__, top );
      lua_pushboolean( L, 0 );
      return 1;
   }

   DAVLUACM_INSTANCE_BOOL( instance, L );

   switch( lua_type( L, 2 ) )
   {
      default:
         bug( "%s: passed bad argument.", __FUNCTION__ );
         lua_pushboolean( L, 0 );
         return 1;
      case LUA_TUSERDATA:
         if( ( frame_ref = (ENTITY_FRAMEWORK **)check_meta( L, 2, "EntityFramework.meta" ) ) == NULL )
         {
            if( ( instance_ref = (ENTITY_INSTANCE **)check_meta( L, 2, "EntityInstance.meta" ) ) == NULL )
            {
               bug( "%s: bad userdata passed.", __FUNCTION__ );
               lua_pushboolean( L, 0 );
               return 1;
            }
            frame = (*instance_ref)->framework;
            break;
         }
         frame = *frame_ref;
         break;
      case LUA_TNUMBER:
         if( ( frame = get_framework_by_id( lua_tonumber( L, 2 ) ) ) == NULL )
         {
            lua_pushboolean( L, 0 );
            return 1;
         }
         break;
      case LUA_TSTRING:
         if( ( frame = get_framework_by_name( lua_tostring( L, 2 ) ) ) == NULL )
         {
            lua_pushboolean( L, 0 );
            return 1;
         }
         break;
   }

   AttachIterator( &Iter, instance->contents );
   while( ( item = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      if( item->framework == frame )
      {
         found = TRUE;
         break;
      }
   DetachIterator( &Iter );

   if( found )
      lua_pushboolean( L, 1 );
   else
      lua_pushboolean( L, 0 );

   return 1;
}

int isPlayer( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   DAVLUACM_INSTANCE_BOOL( instance, L );
   lua_pushboolean( L, (int)instance->isPlayer );
   return 1;
}

int isExit( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   DAVLUACM_INSTANCE_BOOL( instance, L );
   if( get_spec_value( instance, "IsExit" ) > 0 )
      lua_pushboolean( L, 1 );
   else
      lua_pushboolean( L, 0 );
   return 1;
}

int isRoom( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   DAVLUACM_INSTANCE_BOOL( instance, L );
   if( get_spec_value( instance, "IsRoom" ) > 0 )
      lua_pushboolean( L, 1 );
   else
      lua_pushboolean( L, 0 );
   return 1;

}

int isMob( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   DAVLUACM_INSTANCE_BOOL( instance, L );
   if( get_spec_value( instance, "IsMob" ) > 0 )
      lua_pushboolean( L, 1 );
   else
      lua_pushboolean( L, 0 );
   return 1;

}

int isObj( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   DAVLUACM_INSTANCE_BOOL( instance, L );
   if( get_spec_value( instance, "IsObject" ) > 0 )
      lua_pushboolean( L, 1 );
   else
      lua_pushboolean( L, 0 );
   return 1;
}

/* actions */
int luaTogglePlayer( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   DAVLUACM_INSTANCE_NONE( instance, L );
   instance_toggle_player( instance );
   return 0;
}
int luaCallBack( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   EVENT_DATA *event;
   int callbackwhen; /* server pulses, ie .25 seconds. So 1 second = 4 */
   const char *func_name;
   const char *cypher;
   int num_args;
   int x;

   DAVLUACM_INSTANCE_NONE( instance, L );

   if( ( callbackwhen = luaL_checknumber( L, 2 ) ) == 0 )
   {
      bug( "%s: having a callback with 0 pulses is not possible.", __FUNCTION__ );
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
   event->type = EVENT_LUA_CALLBACK;
   event->fun = &event_instance_lua_callback;
   add_event_instance( event, instance, callbackwhen );
   return 0;
}
int luaEntityInstanceInterp( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   char  *order;

   DAVLUACM_INSTANCE_NONE( instance, L );

   if( lua_type( L, 2 ) != LUA_TSTRING )
   {
      bug( "%s: non-string input passed.\r\n", __FUNCTION__ );
      return 0;
   }

   order = strdup( lua_tostring( L, 2 ) );

   entity_handle_cmd( instance, order );
   FREE( order );
   return 0;
}

int luaEntityInstanceTeleport( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   ENTITY_INSTANCE *destination;
   int top = lua_gettop( L );

   if( top != 2 )
   {
      bug( "%s: improper amount of arguments passed %d.", __FUNCTION__, top );
      lua_pushboolean( L, 0 );
      return 1;
   }

   DAVLUACM_INSTANCE_BOOL( instance, L );

   switch( lua_type( L, 2 ) )
   {
      default:
         bug( "%s: bad destination passed", __FUNCTION__ );
         lua_pushboolean( L, 0 );
         return 1;
      case LUA_TUSERDATA:
         if( ( destination = *(ENTITY_INSTANCE **)luaL_checkudata( L, 2, "EntityInstance.meta" ) ) == NULL )
         {
            bug( "%s: passed non-entity instance userdata", __FUNCTION__ );
            lua_pushboolean( L, 0 );
            return 1;
         }
         break;
      case LUA_TSTRING:
         if( ( destination = get_instance_by_name( lua_tostring( L, 2 ) ) ) == NULL )
         {
            lua_pushboolean( L, 0 );
            return 1;
         }
         break;
      case LUA_TNUMBER:
         if( ( destination = get_instance_by_id( lua_tonumber( L, 2 ) ) ) == NULL )
         {
            lua_pushboolean( L, 0 );
            return 1;
         }
         break;
   }

   if( destination == instance->contained_by )
      return 0;

   entity_to_world( instance, destination );

   lua_pushboolean( L, 1 );
   return 1;
}

int luaEcho( lua_State *L )
{
   ENTITY_INSTANCE *room;

   DAVLUACM_INSTANCE_NONE( room, L );
   echo_to_room( room, lua_tostring( L, 2 ) );
   return 0;
}

int luaEchoAt( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NONE( instance, L );
   text_to_entity( instance, lua_tostring( L, 2 ) );
   return 0;
}

int luaEchoAround( lua_State *L )
{
   ENTITY_INSTANCE *room, *instance;
   int x, max = lua_gettop( L );

   DAVLUACM_INSTANCE_NONE( room, L );
   for( x = 1; x < ( max - 1 ); x++ )
   {
      if( ( instance = *(ENTITY_INSTANCE **)luaL_checkudata( L, x, "EntityInstance.meta" ) ) == NULL )
         continue;
      text_to_entity( instance, lua_tostring( L, -1 ) );
   }
   return 0;
}

int luaFrameCall( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   const char *func_name;
   const char *cypher;
   int num_args;
   int x, ret;

   DAVLUACM_INSTANCE_NONE( instance, L );

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

   prep_stack_handle( L, get_frame_script_path( instance->framework ), func_name );

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
      bug( "%s: ret %d: path: %s\r\n - error message: %s\r\n", __FUNCTION__, ret, get_frame_script_path( instance->framework ), lua_tostring( L, -1 ) );
   return 0;

}

int luaInstanceRestore( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   DAVLUACM_INSTANCE_NONE( instance, L );
   restore_pool_stats( instance );
   return 0;
}

int luaEachInventory( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   ITERATOR *Iter;

   DAVLUACM_INSTANCE_NIL( instance, L );

   Iter = (ITERATOR *)lua_newuserdata( L, sizeof( ITERATOR ) );

   luaL_getmetatable( L, "Iter.meta" );
   lua_setmetatable( L, -2 );

   AttachIterator( Iter, instance->contents );

   lua_pushcclosure( L, inv_iter, 1 );
   return 1;
}

int inv_iter( lua_State *L )
{
   ENTITY_INSTANCE *item;
   ITERATOR *Iter = (ITERATOR *)lua_touserdata( L, lua_upvalueindex(1) );

   if( ( item = (ENTITY_INSTANCE *)NextInList( Iter ) ) == NULL )
   {
      DetachIterator( Iter );
      return 0;
   }

   push_instance( item, lua_handle );
   return 1;
}
