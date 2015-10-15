/* header file for the lua_socket library written by Davenge */

int luaopen_SocketLib( lua_State *L );
int SocketGC( lua_State *L );

/* lib functions */

int getSocket( lua_State *L );

/* meta methods */

/* getters */
int getSocketAccount( lua_State *L );
int getSocketControlling( lua_State *L );

/* setters */
int setSocketControlling( lua_State *L );

/* actions */
int changeSocketState( lua_State *L );

