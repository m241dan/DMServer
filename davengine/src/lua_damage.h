/* written this is the header file for lua_damage.c */

int luaopen_DamageLib( lua_State *L );
int DamageGC( lua_State *L );


/* lib functions */

int newDamage( lua_State *L );
int getDamage( lua_State *L );

/* getters */
int getAttacker( lua_State *L );
int getVictim( lua_State *L );
int getDmgSrc( lua_State *L );
int getAmount( lua_State *L );
int getDuration( lua_State *L );
int getDamageTimer( lua_State *L );
int getDamageSrcType( lua_State *L );

/* setters */
int setAttacker( lua_State *L );
int setVictim( lua_State *L );
int setDmgSrc( lua_State *L );
int setAmount( lua_State *L );
int setDuration( lua_State *L );

/* checkers */
int getDmgCrit( lua_State *L );

/* actions */
int lua_damageSend( lua_State *L );
