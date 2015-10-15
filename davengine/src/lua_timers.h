/* header file for lua_timers.h library written by Davenge */

int luaopen_TimersLib( lua_State *L );
int TimersGC( lua_State *L );

/* lib functions */

int newTimer( lua_State *L );
int getTimer( lua_State *L );

/* getters */
int getOwner( lua_State *L );
int getKey( lua_State *L );
int getDuration( lua_State *L );
int getFrequency( lua_State *L );
int getCounter( lua_State *L );
int getUpdateMessage( lua_State *L );
int getEndMessage( lua_State *L );
int getTimerType( lua_State *L );

/* setters */
int setOwner( lua_State *L );
int setKey( lua_State *L );
int setDuration( lua_State *L );
int setFrequency( lua_State *L );
int setCounter( lua_State *L );
int setUpdateMessage( lua_State *L );
int setEndMessage( lua_State *L );
int setTimerType( lua_State *L );

/* actions */
int timerStart( lua_State *L );
int timerPause( lua_State *L );
int timerEnd( lua_State *L );
