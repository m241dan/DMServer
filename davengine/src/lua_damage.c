/* this is the library file for the lua_damage object written by Davenge */

#include "mud.h"

const struct luaL_Reg DamageLib_m[] = {
   /* getters */
   { "getAttacker", getAttacker },
   { "getVictim", getVictim },
   { "getDmgSrc", getDmgSrc },
   { "getAmount", getAmount },
   { "getTimer", getDamageTimer },
   { "getDmgSrcType", getDamageSrcType },
   /* setters */
   { "setAttacker", setAttacker },
   { "setVictim", setVictim },
   { "setDmgSrc", setDmgSrc },
   { "setAmount", setAmount },
   /* checkers */
   { "crit", getDmgCrit },
   /* actions */
   { "send", lua_damageSend },
   { NULL, NULL } /* gandalf */
};

const struct luaL_Reg DamageLib_f[] = {
   { "new", newDamage },
   { "getDamage", getDamage },
   { NULL, NULL } /* gandalf */
};


int luaopen_DamageLib( lua_State *L )
{
   luaL_newmetatable( L, "Damage.meta" );

   lua_pushvalue( L, -1 );
   lua_setfield( L, -2, "__index" );

   lua_pushcfunction( L, DamageGC );
   lua_setfield( L, -2, "__gc" );

   luaL_setfuncs( L, DamageLib_m, 0 );

   luaL_newlib( L, DamageLib_f );
   return 1;
}

int DamageGC( lua_State *L )
{
   DAMAGE **dmg;
   dmg = (DAMAGE **)lua_touserdata( L, -1 );
   *dmg = NULL;
   return 0;
}

/* lib functions */

int newDamage( lua_State *L )
{
   DAMAGE *dmg;

   dmg = init_damage();
   push_damage( dmg, L );
   return 1;
}

int getDamage( lua_State *L )
{
   lua_pushnil( L );
   return 1;
}

/* getters */
int getAttacker( lua_State *L )
{
   DAMAGE *dmg;

   DAVLUACM_DAMAGE_NIL( dmg, L );
   if( !dmg->attacker )
      lua_pushnil( L );
   else
      push_instance( dmg->attacker, L );

   return 1;
}

int getVictim( lua_State *L )
{
   DAMAGE *dmg;

   DAVLUACM_DAMAGE_NIL( dmg, L );
   if( !dmg->victim )
      lua_pushnil( L );
   else
      push_instance( dmg->victim, L );

   return 1;
}

int getDmgSrc( lua_State *L )
{
   DAMAGE *dmg;

   DAVLUACM_DAMAGE_NIL( dmg, L );
   switch( dmg->type )
   {
      default: lua_pushnil( L ); break;
      case DMG_MELEE:
         push_instance( (ENTITY_INSTANCE *)dmg->dmg_src, L );
         break;
   }
   return 1;
}

int getAmount( lua_State *L )
{
   DAMAGE *dmg;
   DAVLUACM_DAMAGE_NIL( dmg, L );
   lua_pushnumber( L, dmg->amount );
   return 1;
}

int getDamageTimer( lua_State *L )
{
   DAMAGE *dmg;
   DAVLUACM_DAMAGE_NIL( dmg, L );
   push_timer( dmg->timer, L );
   return 1;
}

int getDamageSrcType( lua_State *L )
{
   DAMAGE *dmg;
   DAVLUACM_DAMAGE_NIL( dmg, L );
   lua_pushnumber( L, (int)dmg->type );
   return 1;
}

/* setters */
int setAttacker( lua_State *L )
{
   ENTITY_INSTANCE *attacker;
   DAMAGE *dmg;

   DAVLUACM_DAMAGE_NONE( dmg, L );
   if( ( attacker = *(ENTITY_INSTANCE **)luaL_checkudata( L, 2, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: bad object passed as argument two, not entity instance.", __FUNCTION__ );
      return 0;
   }
   if( is_dmg_queued( dmg ) )
   {
      rem_damage( dmg );
      dmg->attacker = attacker;
      add_damage( dmg );
   }
   dmg->attacker = attacker;
   return 0;

}
int setVictim( lua_State *L )
{
   ENTITY_INSTANCE *victim;
   DAMAGE *dmg;

   DAVLUACM_DAMAGE_NONE( dmg, L );
   if( ( victim = *(ENTITY_INSTANCE **)luaL_checkudata( L, 2, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: bad object passed as argument two, not entity instance.", __FUNCTION__ );
      return 0;
   }
   if( is_dmg_queued( dmg ) )
   {
      rem_damage( dmg );
      dmg->victim = victim;
      add_damage( dmg );
      return 0;
   }
   dmg->victim = victim;
   return 0;
}

int setDmgSrc( lua_State *L )
{
   ENTITY_INSTANCE *source_ent;
   DAMAGE *dmg;

   DAVLUACM_DAMAGE_NONE( dmg, L );
   switch( lua_type( L, 2 ) )
   {
      default: dmg->dmg_src = NULL; dmg->type = DMG_UNKNOWN; break;
      case LUA_TUSERDATA:
         if( ( source_ent = *(ENTITY_INSTANCE **)luaL_checkudata( L, 2, "EntityInstance.meta" ) ) != NULL )
         {
            dmg->dmg_src = source_ent;
            dmg->type = DMG_MELEE;
         }
         else
            bug( "%s: bad object passed as arugment two, not acceptable dmg_src.", __FUNCTION__ );
         break;
   }
   return 0;

}

int setAmount( lua_State *L )
{
   DAMAGE *dmg;

   DAVLUACM_DAMAGE_NONE( dmg, L );
   if( lua_type( L , 2 ) != LUA_TNUMBER )
   {
      bug( "%s: bad argument passed, not a number.", __FUNCTION__ );
      return 0;
   }
   dmg->amount = lua_tonumber( L, 2 );
   return 0;
}

/* checkers */
int getDmgCrit( lua_State *L )
{
   DAMAGE *dmg;

   DAVLUACM_DAMAGE_BOOL( dmg, L );
   lua_pushboolean( L, dmg->crit );
   return 1;
}

/* actions */

int lua_damageSend( lua_State *L )
{
   DAMAGE *dmg;

   DAVLUACM_DAMAGE_NONE( dmg, L );
   send_damage( dmg );
   return 0;
}

