/* the library file lua_timers.c written by Davenge */

#include "mud.h"

const struct luaL_Reg TimersLib_m[] = {
   /* getters */
   { "getOwner", getOwner },
   { "getKey", getKey },
   { "getDuration", getDuration },
   { "getFrequency", getFrequency },
   { "getCounter", getCounter },
   { "getUpdateMessage", getUpdateMessage },
   { "getEndMessage", getEndMessage },
   { "getTimerType", getTimerType },
   /* setters */
   { "setOwner", setOwner },
   { "setKey", setKey },
   { "setDuration", setDuration },
   { "setFrequency", setFrequency },
   { "setCounter", setCounter },
   { "setUpdateMessage", setUpdateMessage },
   { "setEndMessage", setEndMessage },
   { "setTimerType", setTimerType },
   /* actions */
   { "start", timerStart },
   { "pause", timerPause },
   { "end", timerEnd },
   { NULL, NULL } /* gandalf */
};

const struct luaL_Reg TimersLib_f[] = {
   { "new", newTimer },
   { "get", getTimer },
   { NULL, NULL }
};

int luaopen_TimersLib( lua_State *L )
{
   luaL_newmetatable( L, "Timers.meta" );

   lua_pushvalue( L , -1 );
   lua_setfield( L, -2, "__index" );

   lua_pushcfunction( L, TimersGC );
   lua_setfield( L, -2, "__gc" );

   luaL_setfuncs( L, TimersLib_m, 0 );

   luaL_newlib( L, TimersLib_f );
   return 1;
}

int TimersGC( lua_State *L )
{
   TIMER **timer;

   timer = (TIMER **)lua_touserdata( L, -1 );
   *timer = NULL;
   return 0;
}

/* lib functions */
int newTimer( lua_State *L )
{
   TIMER *timer;

   timer = init_timer();
   push_timer( timer, L );
   return 0;
}

/* timers.get( (from), (key) ) */
int getTimer( lua_State *L )
{
   TIMER *timer;
   void **owner;
   const char *key;
   int top = lua_gettop( L );
   TIMER_OWNER_TYPES type;

   if( top == 1 )
      type = TIMER_MUD;

   if( lua_type( L, -1 ) != LUA_TSTRING )
   {
      bug( "%s: getTimer needs a string to get by key.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }
   key = lua_tostring( L, -1 );

   if( type != TIMER_MUD )
   {
      if( ( owner = check_meta( L, -2, "EntityInstance.meta" ) ) == NULL || !( type = TIMER_INSTANCE ) )
         if( ( owner = check_meta( L, -2, "Damage.meta" ) ) == NULL || !( type = TIMER_DAMAGE ) )
         {
            bug( "%s: bad user data passed.", __FUNCTION__ );
            lua_pushnil( L );
            return 1;
         }
   }
   switch( type )
   {
      default:
         bug( "%s: bad type...", __FUNCTION__ );
         lua_pushnil( L );
         return 1;
      case TIMER_MUD:
         timer = get_mud_timer( key );
         break;
      case TIMER_INSTANCE:
         timer = get_timer_from_instance( *owner, key );
         break;
      case TIMER_DAMAGE:
         timer = get_timer_from_damage( *owner );
         break;
   }
   if( !timer )
   {
      bug( "%s: Did not find timer.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }
   push_timer( timer, L );
   return 1;
}

/* getters */
int getOwner( lua_State *L )
{
   TIMER *timer;

   DAVLUACM_TIMER_NIL( timer, L );
   switch( timer->owner_type )
   {
      default: lua_pushnil( L ); return 1;
      case TIMER_INSTANCE:
         push_instance( (ENTITY_INSTANCE *)timer->owner, L );
         return 1;
      case TIMER_DAMAGE:
         push_damage( (DAMAGE *)timer->owner, L );
         return 1;
   }
   return 0;
}
int getKey( lua_State *L )
{
   TIMER *timer;

   DAVLUACM_TIMER_NIL( timer, L );
   lua_pushstring( L, timer->key );
   return 1;
}

int getDuration( lua_State *L )
{
   TIMER *timer;

   DAVLUACM_TIMER_NIL( timer, L );
   lua_pushnumber( L, timer->duration );
   return 1;
}

int getFrequency( lua_State *L )
{
   TIMER *timer;

   DAVLUACM_TIMER_NIL( timer, L );
   lua_pushnumber( L, timer->frequency );
   return 1;
}

int getCounter( lua_State *L )
{
   TIMER *timer;

   DAVLUACM_TIMER_NIL( timer, L );
   lua_pushnumber( L, timer->counter );
   return 1;
}

int getUpdateMessage( lua_State *L )
{
   TIMER *timer;

   DAVLUACM_TIMER_NIL( timer, L );
   lua_pushstring( L, timer->update_message );
   return 1;
}

int getEndMessage( lua_State *L )
{
   TIMER *timer;

   DAVLUACM_TIMER_NIL( timer, L );
   lua_pushstring( L, timer->end_message );
   return 1;
}

int getTimerType( lua_State *L )
{
   TIMER *timer;

   DAVLUACM_TIMER_NIL( timer, L );
   lua_pushnumber( L, timer->timer_type );
   return 1;
}

/* setters */
int setOwner( lua_State *L )
{
   TIMER *timer;
   void *owner;
   TIMER_OWNER_TYPES type;

   DAVLUACM_TIMER_NONE( timer, L );

   if( ( owner = check_meta( L, -1, "EntityInstance.meta" ) ) == NULL || !( type = TIMER_INSTANCE ) )
      if( ( owner = check_meta( L, -1, "Damage.meta" ) ) == NULL || !( type = TIMER_DAMAGE ) )
      {
         owner = NULL;
         type = TIMER_MUD;
      }

   set_timer_owner( timer, owner, type );
   return 0;
}

int setKey( lua_State *L )
{
   TIMER *timer;
   DAVLUACM_TIMER_NONE( timer, L );
   set_timer_key( timer, lua_tostring( L, -1 ) );
   return 0;
}

int setDuration( lua_State *L )
{
   TIMER *timer;
   DAVLUACM_TIMER_NONE( timer, L );
   set_timer_duration( timer, lua_tonumber( L, -1 ) );
   return 0;
}

int setFrequency( lua_State *L )
{
   TIMER *timer;
   DAVLUACM_TIMER_NONE( timer, L );
   set_timer_frequency( timer, lua_tonumber( L, -1 ) );
   return 0;
}

int setCounter( lua_State *L )
{
   TIMER *timer;
   DAVLUACM_TIMER_NONE( timer, L );
   set_timer_counter( timer, lua_tonumber( L, -1 ) );
   return 0;
}
int setUpdateMessage( lua_State *L )
{
   TIMER *timer;
   DAVLUACM_TIMER_NONE( timer, L );
   set_timer_update_message( timer, lua_tostring( L, -1 ) );
   return 0;
}

int setEndMessage( lua_State *L )
{
   TIMER *timer;
   DAVLUACM_TIMER_NONE( timer, L );
   set_timer_end_message( timer, lua_tostring( L, -1 ) );
   return 0;
}

int setTimerType( lua_State *L )
{
   TIMER *timer;
   DAVLUACM_TIMER_NONE( timer, L );
   set_timer_type( timer, (char)lua_tonumber( L, -1 ) );
   return 0;
}

/* actions */
int timerStart( lua_State *L )
{
   TIMER *timer;
   DAVLUACM_TIMER_NONE( timer, L );
   start_timer( timer );
   return 0;
}

int timerPause( lua_State *L )
{
   TIMER *timer;
   DAVLUACM_TIMER_NONE( timer, L );
   pause_timer( timer );
   return 0;
}

int timerEnd( lua_State *L )
{
   TIMER *timer;
   DAVLUACM_TIMER_NONE( timer, L );
   end_timer( timer );
   return 0;
}


