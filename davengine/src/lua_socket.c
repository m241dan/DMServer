/* the lua_socket library allowing lua limited access to the C structure socket written by Davenge */

#include "mud.h"

const struct luaL_Reg SocketLib_m[] = {
   /* getters */
   { "getAccount", getSocketAccount },
   { "getControlling", getSocketControlling },
   /* setters */
   { "setControlling", setSocketControlling },
   /* actions */
   { "changeState", changeSocketState },
   { NULL, NULL }
};

const struct luaL_Reg SocketLib_f[] = {
   { "getSocket", getSocket },
   { NULL, NULL }
};

int luaopen_SocketLib( lua_State *L )
{
   luaL_newmetatable( L, "Socket.meta" );

   lua_pushvalue( L, -1 );
   lua_setfield( L, -2, "__index" );

   lua_pushcfunction( L, SocketGC );
   lua_setfield( L, -2, "__gc" );

   luaL_setfuncs( L, SocketLib_m, 0 );

   luaL_newlib( L, SocketLib_f );
   return 1;
}

int SocketGC( lua_State *L )
{
   D_SOCKET **socket;
   socket = (D_SOCKET **)lua_touserdata( L, -1 );
   *socket = NULL;
   return 0;
}

/* lib functions */

int getSocket( lua_State *L )
{
   ACCOUNT_DATA **account_ref, *account;
   ENTITY_INSTANCE **instance_ref, *instance;
   NANNY_DATA **nanny_ref, *nanny;

   if( lua_type( L, -1 ) != LUA_TUSERDATA )
   {
      bug( "%s: expecting userdata only.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( account_ref = (ACCOUNT_DATA **)check_meta( L, -1, "Account.meta" ) ) == NULL )
   {
      if( ( instance_ref = (ENTITY_INSTANCE **)check_meta( L, -1, "EntityInstance.meta" ) ) == NULL )
      {
         if( ( nanny_ref = (NANNY_DATA **)check_meta( L, -1, "Nanny.meta" ) ) == NULL )
         {
            bug( "%s: unknown userdata passed.", __FUNCTION__ );
            lua_pushnil( L );
            return 1;
         }
         nanny = *nanny_ref;
         push_socket( nanny->socket, L );
         return 1;
      }
      instance = *instance_ref;
      push_socket( instance->socket, L );
      return 1;
   }
   account = *account_ref;
   push_socket( account->socket, L );
   return 1;
}

/* meta methods */

/* getters */
int getSocketAccount( lua_State *L )
{
   D_SOCKET *socket;
   DAVLUACM_SOCKET_NIL( socket, L );
   push_account( socket->account, L );
   return 1;
}

int getSocketControlling( lua_State *L )
{
   D_SOCKET *socket;
   DAVLUACM_SOCKET_NIL( socket, L );
   push_instance( socket->controlling, L );
   return 1;
}

/* setters */
int setSocketControlling( lua_State *L )
{
   D_SOCKET *socket;
   ENTITY_INSTANCE **instance_ref, *instance;

   DAVLUACM_SOCKET_NONE( socket, L );
   if( ( instance_ref = (ENTITY_INSTANCE **)check_meta( L, -1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: expecting a EntityInstance userdata, did not get.\r\n", __FUNCTION__ );
      return 0;
   }
   instance = *instance_ref;

   if( socket->controlling )
      socket_uncontrol_entity( socket->controlling );
   socket_control_entity( socket, instance );
   return 0;
}

/* actions */
int changeSocketState( lua_State *L )
{
   D_SOCKET *socket;
   int new_state;

   DAVLUACM_SOCKET_NONE( socket, L );

   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: expecting integer only.", __FUNCTION__ );
      return 0;
   }

   new_state = lua_tonumber( L, -1 );
   if( new_state < 0 || new_state >= MAX_STATE )
   {
      bug( "%s: bad state option passed.", __FUNCTION__ );
      return 0;
   }
   change_socket_state( socket, new_state );
   return 0;
}




