/* header file for lua_specification.c written by Davenge */

int luaopen_SpecificationLib( lua_State *L );
int SpecificationGC( lua_State *L );

int lua_newSpec( lua_State *L );

/* getters */

int getSpecType( lua_State *L );
int getSpecValue( lua_State *L );
int getSpecOwner( lua_State *L );

/* setters */

int setSpecValue( lua_State *L );
