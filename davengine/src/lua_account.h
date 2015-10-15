/* the header file for library lua_account written by Davenge */

int luaopen_AccountLib( lua_State *L );
int AccountGC( lua_State *L );

/* lib functions */
int loadAccount( lua_State *L );
int getAccount( lua_State *L );

/* meta methods */
/* getters */
int getAccountID( lua_State *L );
int getAccountName( lua_State *L );
int getAccountLevel( lua_State *L );
int getAccountPagewidth( lua_State *L );
int getAccountChatAs( lua_State *L );
int getAccountControlling( lua_State *L );
int getCharacter( lua_State *L );
int getCharacters( lua_State *L );

/* setters */
int setAccountControlling( lua_State *L );
int setAccountPagewidth( lua_State *L );
int setAccountChatAs( lua_State *L );
/* actions */
int luaAccountEchoAt( lua_State * L );
int addCharacter( lua_State *L );

/* iters */
int char_iter( lua_State *L );
