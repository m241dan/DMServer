/* the lua_iter library writte by Davenge */

#include "mud.h"

int luaopen_IterLib( lua_State *L )
{
   luaL_newmetatable( L, "Iter.meta" );

   lua_pushcfunction( L, IterGC );
   lua_setfield( L, -2, "__gc" );

   return 0;
}

int IterGC( lua_State *L )
{
   ITERATOR *Iter;
   Iter = (ITERATOR *)lua_touserdata( L, -1 );
   DetachIterator( Iter );
   Iter->_pList = NULL;
   Iter->_pCell = NULL;
   return 0;
}
