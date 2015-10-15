/* header file for the lua_nanny library written by Davenge */

int luaopen_NannyLib( lua_State *L );
int NannyGC( lua_State *L );

/* lib functions */

int luaNewNanny( lua_State *L );

/* meta methods */

/* getters */
int getNannyControl( lua_State *L );
int getNannyContent( lua_State *L );
int getNannyState( lua_State *L );

/* setters */
int setNannyControl( lua_State *L );
int setNannyContent( lua_State *L );
int setNannyState( lua_State *L );

/* actions */
int nannyStart( lua_State *L );
int nannyFinish( lua_State *L );
int nannyEchoAt( lua_State *L );
