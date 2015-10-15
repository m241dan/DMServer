/* methods pertaining to lua boxed specification written by Davenge */

#include "mud.h"

const struct luaL_Reg SpecificationLib_m[] = {
   /* getters */
   { "getType", getSpecType },
   { "getValue", getSpecValue },
   { "getOwner", getSpecOwner },
   /* setters */
   { "setValue", setSpecValue },
   { NULL, NULL }
};

const struct luaL_Reg SpecificationLib_f[] = {
   { "new", lua_newSpec },
   { NULL, NULL }
};

int luaopen_SpecificationLib( lua_State *L )
{
   luaL_newmetatable( L, "Specification.meta" );

   lua_pushvalue( L, -1 );
   lua_setfield( L, -2, "__index" );

   lua_pushcfunction( L, SpecificationGC );
   lua_setfield( L, -2, "__gc" );

   luaL_setfuncs( L, SpecificationLib_m, 0 );

   luaL_newlib( L, SpecificationLib_f );
   return 1;
}

int SpecificationGC( lua_State *L )
{
   SPECIFICATION **spec;
   spec = (SPECIFICATION **)lua_touserdata( L, -1 );
   if( !strcmp( (*spec)->owner, "null" ) )
      free_specification( *spec );
   else
      *spec = NULL;
   return 0;
}

/* lib functions */


int lua_newSpec( lua_State *L ) /* takes spec name as argument */
{
   SPECIFICATION *spec;
   const char *spectype;
   int spec_type;

   if( ( spectype = luaL_checkstring( L, -1 ) ) == NULL )
   {
      bug( "%s: no string passed", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( spec_type = match_string_table_no_case( spectype, spec_table ) ) == -1 )
   {
      bug( "%s: no such spec: %s", __FUNCTION__, spectype );
      lua_pushnil( L );
      return 1;
   }

   spec = init_specification();
   spec->type = spec_type;
   spec->value = 0;
   push_specification( spec, L );
   return 1;
}

/* getters */

int getSpecType( lua_State *L )
{
   SPECIFICATION *spec;

   if( ( spec = *(SPECIFICATION **)luaL_checkudata( L, 1, "Specification.meta" ) ) == NULL )
   {
      bug( "%s: spec meta table is messed up.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   lua_pushstring( L, spec_table[spec->type] );
   return 1;
}

int getSpecValue( lua_State *L )
{
   SPECIFICATION *spec;

   if( ( spec = *(SPECIFICATION **)luaL_checkudata( L, 1, "Specification.meta" ) ) == NULL )
   {
      bug( "%s: spec meta table is messed up.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   lua_pushnumber( L, spec->value );
   return 1;
}

int getSpecOwner( lua_State *L )
{
   SPECIFICATION *spec;

   if( ( spec = *(SPECIFICATION **)luaL_checkudata( L, 1, "Specification.meta" ) ) == NULL )
   {
      bug( "%s: spec meta table is messed up.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   lua_pushstring( L, spec->owner );
   return 1;
}

/* setters */

int setSpecValue( lua_State *L )
{
   SPECIFICATION *spec;
   int value;

   if( ( spec = *(SPECIFICATION **)luaL_checkudata( L, 1, "Specification.meta" ) ) == NULL )
   {
      bug( "%s: spec meta table is messed up.", __FUNCTION__ );
      return 0;
   }

   value = luaL_checknumber( L, 2 );

   spec->value = value;
   return 0;

}








