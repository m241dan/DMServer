/* lua_nanny library, allows lua to interact with nannies written by Davenge */

#include "mud.h"

const struct luaL_Reg NannyLib_m[] = {
   /* getters */
   { "getControl", getNannyControl },
   { "getContent", getNannyContent },
   { "getState", getNannyState },
   /* setters */
   { "setControl", setNannyControl },
   { "setContent", setNannyContent },
   { "setState", setNannyState },
   /* actions */
   { "start", nannyStart },
   { "finish", nannyFinish },
   { "echoAt", nannyEchoAt },
   { NULL, NULL }
};

const struct luaL_Reg NannyLib_f[] = {
   { "new", luaNewNanny },
   { NULL, NULL }
};

int luaopen_NannyLib( lua_State *L )
{
   luaL_newmetatable( L, "Nanny.meta" );

   lua_pushvalue( L, -1 );
   lua_setfield( L, -2, "__index" );

   lua_pushcfunction( L, NannyGC );
   lua_setfield( L, -2, "__gc" );

   luaL_setfuncs( L, NannyLib_m, 0 );

   luaL_newlib( L, NannyLib_f );
   return 1;
}

int NannyGC( lua_State *L )
{
   NANNY_DATA **nanny;
   nanny = (NANNY_DATA **)lua_touserdata( L, -1 );
   *nanny = NULL;
   return 0;
}

/* lib functions */

int luaNewNanny( lua_State *L )
{
   NANNY_DATA *nanny;

   if( lua_type( L, -1 ) != LUA_TSTRING )
   {
      bug( "%s: need to pass a path as a string.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   CREATE( nanny, NANNY_DATA, 1 );
   nanny->lua_nanny = TRUE;
   nanny->path = strdup( lua_tostring( L, -1 ) );
   push_nanny( nanny, L );
   return 1;
}

/* meta methods */

/* getters */
int getNannyControl( lua_State *L )
{
   NANNY_DATA *nanny;

   DAVLUACM_NANNY_NIL( nanny, L );
   push_socket( nanny->socket, L );
   return 1;
}

int getNannyContent( lua_State *L )
{
   NANNY_DATA *nanny;

   DAVLUACM_NANNY_NIL( nanny, L );

   switch( nanny->content_type )
   {
      default:
         bug( "%s: bad content type for lua nannies.", __FUNCTION__ );
         lua_pushnil( L );
         return 1;
      case NANNY_ACCOUNT:
         push_account( (ACCOUNT_DATA *)nanny->content, L );
         return 1;
      case NANNY_MOBILE:
         push_instance( (ENTITY_INSTANCE *)nanny->content, L );
         return 1;
      case NANNY_FRAMEWORK:
         push_framework( (ENTITY_FRAMEWORK *)nanny->content, L );
         return 1;
   }
   bug( "%s: should not have gotten here.", __FUNCTION__ );
   lua_pushnil( L );
   return 1;
}

int getNannyState( lua_State *L )
{
   NANNY_DATA *nanny;

   DAVLUACM_NANNY_NIL( nanny, L );

   lua_pushnumber( L, nanny->state );
   return 1;
}

/* setters */
int setNannyControl( lua_State *L )
{
   NANNY_DATA *nanny;
   ACCOUNT_DATA **account;
   ENTITY_INSTANCE **entity;

   DAVLUACM_NANNY_NONE( nanny, L );

   if( ( account = (ACCOUNT_DATA **)check_meta( L, -1, "Account.meta" ) ) == NULL )
   {
      if( ( entity = (ENTITY_INSTANCE **)check_meta( L, -1, "EntityInstance.meta" ) ) == NULL )
      {
         bug( "%s: bad userdata passed.", __FUNCTION__ );
         return 0;
      }
      else
         control_nanny( (*entity)->socket, nanny );
   }
   else
      control_nanny( (*account)->socket, nanny );
   return 0;
}

int setNannyContent( lua_State *L )
{
   NANNY_DATA *nanny;
   ACCOUNT_DATA **account;
   ENTITY_FRAMEWORK **frame;
   ENTITY_INSTANCE **entity;

   DAVLUACM_NANNY_NONE( nanny, L );


   if( ( account = (ACCOUNT_DATA **)check_meta( L, -1, "Account.meta" ) ) == NULL )
   {
      if( ( entity = (ENTITY_INSTANCE **)check_meta( L, -1, "EntityInstance.meta" ) ) == NULL )
      {
         if( ( frame = (ENTITY_FRAMEWORK **)check_meta( L, -1, "EntityFramework.meta" ) ) == NULL )
         {
            bug( "%s: bad userdata passed.", __FUNCTION__ );
            return 0;
         }
         else
         {
            nanny->content = *frame;
            nanny->content_type = NANNY_FRAMEWORK;
         }
      }
      else
      {
         nanny->content = *entity;
         nanny->content_type = NANNY_MOBILE;
      }
   }
   else
   {
      nanny->content = *account;
      nanny->content_type = NANNY_ACCOUNT;
   }
   return 0;

}
int setNannyState( lua_State *L )
{
   NANNY_DATA *nanny;

   DAVLUACM_NANNY_NONE( nanny, L );

   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: passed bad argument, must be a number.", __FUNCTION__ );
      return 0;
   }
   nanny->state = lua_tonumber( L, -1 );
   return 0;
}

/* actions */
int nannyStart( lua_State *L )
{
   NANNY_DATA *nanny;

   DAVLUACM_NANNY_NONE( nanny, L );
   if( !nanny->socket )
   {
      bug( "%s: can't start a nanny without a socket on the nanny.", __FUNCTION__ );
      return 0;
   }
   if( !nanny->socket->nanny )
   {
      bug( "%s: can't start a nanny if the socket controlling it doesnt contain the nanny.", __FUNCTION__ );
      return 0;
   }
   change_socket_state( nanny->socket, STATE_NANNY );
   return 0;
}

int nannyFinish( lua_State *L )
{
   NANNY_DATA *nanny;

   DAVLUACM_NANNY_NONE( nanny, L );
   change_socket_state( nanny->socket, nanny->socket->prev_state );
   uncontrol_nanny( nanny->socket );
   free_nanny( nanny );
   return 0;
}

int nannyEchoAt( lua_State *L )
{
   NANNY_DATA *nanny;

   DAVLUACM_NANNY_NONE( nanny, L );
   if( lua_type( L, -1 ) != LUA_TSTRING )
   {
      bug( "%s: can only echo a string, non-string value passed to echoat.", __FUNCTION__ );
      return 0;
   }
   text_to_nanny( nanny, lua_tostring( L, -1 ) );
   return 0;
}
