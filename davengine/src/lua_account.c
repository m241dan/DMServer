/* the library lua_account handle all lua account stuff written by Davenge */

#include "mud.h"

const struct luaL_Reg AccountLib_m[] = {
   /* getters */
   { "getId", getAccountID },
   { "getName", getAccountName },
   { "getLevel", getAccountLevel },
   { "getPagewidth", getAccountPagewidth },
   { "getChatAs", getAccountChatAs },
   { "getControlling", getAccountControlling },
   { "getCharacter", getCharacter },
   { "getCharacters", getCharacters },
   /* setters */
   { "setControlling", setAccountControlling },
   { "setPagewidth", setAccountPagewidth },
   { "setChatAs", setAccountChatAs },
   /* actions */
   { "echoAt", luaAccountEchoAt },
   { "addCharacter", addCharacter },
   { NULL, NULL }
};

const struct luaL_Reg AccountLib_f[] = {
   { "loadAccount", loadAccount },
   { "getAccount", getAccount },
   { NULL, NULL }
};

int luaopen_AccountLib( lua_State *L )
{
   luaL_newmetatable( L, "Account.meta" );

   lua_pushvalue( L, -1 );
   lua_setfield( L, -2, "__index" );

   lua_pushcfunction( L, AccountGC );
   lua_setfield( L, -2, "__gc" );

   luaL_setfuncs( L, AccountLib_m, 0 );

   luaL_newlib( L, AccountLib_f );
   return 1;
}

int AccountGC( lua_State *L )
{
   ACCOUNT_DATA **account;
   account = (ACCOUNT_DATA **)lua_touserdata( L, -1 );
   *account = NULL;
   return 0;
}

int loadAccount( lua_State *L )
{
   ACCOUNT_DATA *account;
   const char *account_name;

   if( ( account_name = luaL_checkstring( L, -1 ) ) == NULL )
   {
      bug( "%s: no account name passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }
   account = init_account();
   if( load_account( account, account_name ) == RET_DB_NO_ENTRY )
   {
      free_account( account );
      lua_pushnil( L );
      return 1;
   }
   push_account( account, L );
   return 1;
}

int getAccount( lua_State *L )
{
   ACCOUNT_DATA *account = NULL;
   ENTITY_INSTANCE *instance;

   switch( lua_type( L, -1 ) )
   {
      default:
         bug( "%s: passsed a non-string/integer/instance.", __FUNCTION__ );
         lua_pushnil( L );
         return 1;
      case LUA_TSTRING:
         account = get_active_account_by_name( lua_tostring( L, -1 ) );
         break;
      case LUA_TNUMBER:
         account = get_active_account_by_id( lua_tonumber( L, -1 ) );
         break;
      case LUA_TUSERDATA:
         if( ( instance = *(ENTITY_INSTANCE **)luaL_checkudata( L, -1, "EntityInstance.meta" ) ) == NULL )
         {
            bug( "%s: only takes userdata of type instance.", __FUNCTION__ );
            break;
         }
         account = instance->socket->account;
         break;
   }
   if( !account )
      lua_pushnil( L );
   else
      push_account( account, L );
   return 1;
}


/* meta methods */
/* getters */
int getAccountID( lua_State *L )
{
   ACCOUNT_DATA *account;

   DAVLUACM_ACCOUNT_NIL( account, L );
   lua_pushnumber( L, account->idtag->id );
   return 1;
}

int getAccountName( lua_State *L )
{
   ACCOUNT_DATA *account;

   DAVLUACM_ACCOUNT_NIL( account, L );
   lua_pushstring( L, account->name );
   return 1;
}

int getAccountLevel( lua_State *L )
{
   ACCOUNT_DATA *account;

   DAVLUACM_ACCOUNT_NIL( account, L );
   lua_pushnumber( L, account->level );
   return 1;

}

int getAccountPagewidth( lua_State *L )
{
   ACCOUNT_DATA *account;

   DAVLUACM_ACCOUNT_NIL( account, L );
   lua_pushnumber( L, account->pagewidth );
   return 1;

}

int getAccountChatAs( lua_State *L )
{
   ACCOUNT_DATA *account;

   DAVLUACM_ACCOUNT_NIL( account, L );
   lua_pushstring( L, account->chatting_as );
   return 1;
}

int getAccountControlling( lua_State *L )
{
   ACCOUNT_DATA *account;

   DAVLUACM_ACCOUNT_NIL( account, L );
   if( account->controlling )
      push_instance( account->controlling, L );
   else
      lua_pushnil( L );
   return 1;
}

int getCharacter( lua_State *L )
{
   ACCOUNT_DATA *account;
   ENTITY_INSTANCE *character;
   CHAR_SHEET *sheet;
   ITERATOR Iter;
   const char *name;

   DAVLUACM_ACCOUNT_NIL( account, L );
   if( lua_type( L, -1 ) != LUA_TSTRING )
   {
      bug( "%s: expecting a string, didn't get it.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }
   name = lua_tostring( L, -1 );

   AttachIterator( &Iter, account->characters );
   while( ( sheet = (CHAR_SHEET *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( sheet->name, name ) )
         break;
   DetachIterator( &Iter );

   if( !sheet )
   {
      lua_pushnil( L );
      return 1;
   }
   if( ( character = get_instance_by_id( sheet->id ) ) == NULL )
   {
      lua_pushnil( L );
      return 1;
   }
   push_instance( character, L );
   return 1;
}

int getCharacters( lua_State *L )
{
   ACCOUNT_DATA *account;
   ITERATOR *Iter;

   DAVLUACM_ACCOUNT_NIL( account, L );

   Iter = (ITERATOR *)lua_newuserdata( L, sizeof( ITERATOR ) );

   luaL_getmetatable( L, "Iter.meta" );
   lua_setmetatable( L, -2 );

   AttachIterator( Iter, account->characters );

   lua_pushcclosure( L, char_iter, 1 );
   return 1;
}

/* setters */
int setAccountControlling( lua_State *L )
{
   ACCOUNT_DATA *account;
   ENTITY_INSTANCE **instance_ref, *instance;

   DAVLUACM_ACCOUNT_NONE( account, L );

   if( ( instance_ref = (ENTITY_INSTANCE **)check_meta( L, -1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed userdata has bad meta table, expecting EntityInstance.meta", __FUNCTION__ );
      return 0;
   }
   instance = *instance_ref;
   account_control_entity( account, instance );
   return 0;

}
int setAccountPagewidth( lua_State *L )
{
   ACCOUNT_DATA *account;

   DAVLUACM_ACCOUNT_NONE( account, L );
   if( lua_type( L, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: only takes an integer.\r\n", __FUNCTION__ );
      return 0;
   }
   set_account_pagewidth( account, lua_tonumber( L , -1 ) );
   return 0;
}

int setAccountChatAs( lua_State *L )
{
   ACCOUNT_DATA *account;

   DAVLUACM_ACCOUNT_NONE( account, L );
   if( lua_type( L, -1 ) != LUA_TSTRING )
   {
      bug( "%s: only takes a string.\r\n", __FUNCTION__ );
      return 0;
   }
   set_account_chatas( account, lua_tostring( L, -1 ) );
   return 0;
}

/* actions */
int luaAccountEchoAt( lua_State *L )
{
   ACCOUNT_DATA *account;

   DAVLUACM_ACCOUNT_NONE( account, L );
   if( lua_type( L, -1 ) != LUA_TSTRING )
   {
      bug( "%s: only takes a string.\r\n", __FUNCTION__ );
      return 0;
   }
   text_to_account( account, "%s", lua_tostring( L, -1 ) );
   return 0;
}

int addCharacter( lua_State *L )
{
   ACCOUNT_DATA *account;
   ENTITY_INSTANCE **character_box, *character;

   DAVLUACM_ACCOUNT_NONE( account, L );
   if( SizeOfList( account->characters ) > MAX_CHARACTER )
   {
      bug( "%s: account at max characters arleady.", __FUNCTION__ );
      return 0;
   }
   if( ( character_box = (ENTITY_INSTANCE **)luaL_checkudata( L, -1, "EntityInstance.meta" ) ) == NULL )
      return 0;

   if( ( character = *character_box ) == NULL )
   {
      bug( "%s: NULL box.", __FUNCTION__ );
      return 0;
   }
   add_character_to_account( character, account );
   return 0;
}

/* Iterators */

int char_iter( lua_State *L )
{
   CHAR_SHEET *sheet;
   ITERATOR *Iter = (ITERATOR *)lua_touserdata( L, lua_upvalueindex(1) );

   if( ( sheet = (CHAR_SHEET *)NextInList( Iter ) ) == NULL )
   {
      DetachIterator( Iter );
      return 0;
   }
   lua_pushstring( L, sheet->name );
   return 1;
}
