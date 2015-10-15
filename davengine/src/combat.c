/* combat.c is a library of combat specific functions written by Davenge */

#include "mud.h"

/* creation */
DAMAGE *init_damage( void )
{
   DAMAGE *dmg;

   CREATE( dmg, DAMAGE, 1 );
   dmg->attacker = NULL;
   dmg->victim = NULL;
   dmg->dmg_src = NULL;
   dmg->type = DMG_UNKNOWN;
   dmg->amount = 0;
   dmg->timer = init_timer();
   set_timer_owner( dmg->timer, dmg, TIMER_DAMAGE );
   dmg->timer->key = strdup( "dmg key" );
   return dmg;
}

void free_damage( DAMAGE *dmg )
{
   if( is_dmg_queued( dmg ) )
      rem_damage( dmg );
   free_timer( dmg->timer );
   dmg->attacker = NULL;
   dmg->victim = NULL;
   dmg->dmg_src = NULL;
   FREE( dmg );
   return;
}


/* actions */
void prep_melee_atk( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim )
{
   SPECIFICATION *spec;
   DAMAGE *dmg;
   const char *path;
   int ret, top = lua_gettop( lua_handle );

   if( ( spec = has_spec( attacker, "prepMeleeTimer" ) ) != NULL && spec->value > 0 )
      path = get_script_path_from_spec( spec );
   else
      path = "../scripts/settings/combat.lua";

   prep_stack( path, "prepMeleeTimer" );
   push_instance( attacker, lua_handle );

   dmg = init_damage();
   set_dmg_attacker( dmg, attacker );
   set_dmg_victim( dmg, victim );
   set_dmg_src( dmg, attacker, DMG_MELEE );

   push_timer( dmg->timer, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
   {
      bug( "%s: ret: %d path: %s\r\n - error message %s.", __FUNCTION__, ret, path, lua_tostring( lua_handle, -1 ) );
      lua_settop( lua_handle, top );
      return;
   }
   lua_settop( lua_handle, top );
   send_damage( dmg );
   return;
}

void prep_melee_dmg( DAMAGE *dmg )
{
   SPECIFICATION *spec;
   const char *path;
   int ret, top = lua_gettop( lua_handle );

   if( ( spec = has_spec( dmg->attacker, "prepMeleeDamage" ) ) != NULL && spec->value > 0 )
      path = get_script_path_from_spec( spec );
   else
      path = "../scripts/settings/combat.lua";

   prep_stack( path, "prepMeleeDamage" );
   push_instance( dmg->attacker, lua_handle );
   push_damage( dmg, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
   {
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, path, lua_tostring( lua_handle, -1 ) );
      dmg->amount = 0;
   }
   lua_settop( lua_handle, top );
   return;
}

bool receive_damage( DAMAGE *dmg )
{
   SPECIFICATION *spec;
   const char *path;
   int ret, top = lua_gettop( lua_handle );

   if( ( spec = has_spec( dmg->victim, "onReceiveDamage" ) ) != NULL && spec->value > 0 )
      path = get_script_path_from_spec( spec );
   else
      path = "../scripts/settings/combat.lua";

   prep_stack( path, "onReceiveDamage" );
   push_instance( dmg->victim, lua_handle );
   push_damage( dmg, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
   {
      bug( "%s: ret %d: path %s\r\n - error message: %s.", __FUNCTION__, ret, path, lua_tostring( lua_handle, -1 ) );
      dmg->amount = 0;
   }
   lua_settop( lua_handle, top );
   return TRUE;
}


cbt_ret melee_attack( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim )
{
   if( DODGE_ON && does_check( attacker, victim, "dodgeChance" ) )
      return HIT_DODGED;

   if( PARRY_ON && does_check( attacker, victim, "parryChance" ) )
      return HIT_PARRIED;

   if( MISS_ON && does_check( attacker, victim, "missChance" ) )
      return HIT_MISSED;

   return HIT_SUCCESS;
}

bool send_damage( DAMAGE *dmg )
{
   /* safeties */
   if( !dmg->attacker->primary_dmg_received_stat && !dmg->attacker->builder )
   {
      bug( "%s: %s trying to do damage at the utility level but it cannot receive any in return.", __FUNCTION__, instance_name( dmg->attacker ) );
   }
   if( !dmg->victim->primary_dmg_received_stat )
   {
      bug( "%s: %s trying to do damage at the utility level to something that can't take damage.", __FUNCTION__, instance_name( dmg->attacker ) );
      return FALSE;
   }
   add_damage( dmg );
   start_timer( dmg->timer );
   return TRUE;
}

/* checkers */
bool does_check( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim, const char *does )
{
   SPECIFICATION *spec;
   const char *path;
   int ret, chance, top = lua_gettop( lua_handle );

   if( ( spec = has_spec( victim, does ) ) != NULL && spec->value > 0 )
      path = get_script_path_from_spec( spec );
   else
      path = "../scripts/settings/combat.lua";

   prep_stack( path, does );
   push_instance( attacker, lua_handle );
   push_instance( victim, lua_handle );
   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
   {
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, path, lua_tostring( lua_handle, -1 ) );
      lua_settop( lua_handle, top );
      return FALSE;
   }

   if( lua_type( lua_handle, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: bad value returned by lua.", __FUNCTION__ );
      lua_settop( lua_handle, top );
      return FALSE;
   }
   chance = lua_tonumber( lua_handle, -1 );
   if( number_percent() < chance )
      return TRUE;
   return FALSE;
}

bool can_melee( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim )
{
   SPECIFICATION *spec;
   const char *path;
   int ret, top = lua_gettop( lua_handle );
   bool can = FALSE;

   if( ( spec = has_spec( attacker, "meleeCheck" ) ) != NULL && spec->value > 0 )
      path = get_script_path_from_spec( spec );
   else
      path = "../scripts/settings/combat.lua";

   prep_stack( path, "meleeCheck" );
   push_instance( attacker, lua_handle );
   push_instance( victim, lua_handle );

   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path %s\r\n - error message: %s.", __FUNCTION__, ret, path, lua_tostring( lua_handle, -1 ) );
   else if( lua_type( lua_handle, -1 ) == LUA_TSTRING )
      text_to_entity( attacker, "%s\r\n", lua_tostring( lua_handle, -1 ) );
   else
      can = TRUE;

   lua_settop( lua_handle, top );
   return can;
}

/* getters */
int get_auto_cd( ENTITY_INSTANCE *instance )
{
   SPECIFICATION *spec;
   const char *path;
   int cd, ret, top = lua_gettop( lua_handle );
   if( ( spec = has_spec( instance, "meleeCooldown" ) ) != NULL && spec->value > 0 )
      path = get_script_path_from_spec( spec );
   else
      path = "../scripts/settings/combat.lua";

   prep_stack( path, "meleeCooldown" );
   push_instance( instance, lua_handle );

   if( ( ret = lua_pcall( lua_handle, 1, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, path, lua_tostring( lua_handle, -1 ) );
   else
      cd = lua_tonumber( lua_handle, -1 );

   if( cd == 0 )
      cd = 10; /* safe number, should never be zero */

   lua_settop( lua_handle, top );
   return cd;
}

/* utility */

const char *compose_dmg_key( DAMAGE *dmg )
{
   static char buf[MAX_BUFFER];
   memset( &buf[0], 0, sizeof( buf ) );

   if( dmg->attacker )
      strcat( buf, quick_format( "%s's ", instance_name( dmg->attacker ) ) );

   if( dmg->type != DMG_UNKNOWN )
   {
      if( dmg->type == DMG_MELEE )
      {
         strcat( buf, "melee " );
         if( dmg->dmg_src != dmg->attacker )
            strcat( buf, quick_format( "with %s", instance_name( (ENTITY_INSTANCE *)dmg->dmg_src ) ) );
       }
   }

   if( dmg->victim )
      strcat( buf, quick_format( "against %s", instance_name( dmg->victim ) ) );

   buf[strlen( buf )] = '\0';
   return buf;
}

/* actions */

void handle_damage( DAMAGE *dmg )
{
   bool kill = FALSE;
   cbt_ret status;
   switch( dmg->type )
   {
      default: return;
      case DMG_MELEE:
         status = melee_attack( dmg->attacker, dmg->victim );
         break;
   }
   if( status != HIT_SUCCESS )
   {
      dmg->amount = 0;
      combat_message( dmg->attacker, dmg->victim, dmg, status );
      return;
   }

   /* run the necessary lua scripts */
   if( dmg->type == DMG_MELEE )
      prep_melee_dmg( dmg );

   /* damage is received universally via lua, can be global script or local */
   receive_damage( dmg );

   if( dmg->amount )
      kill = do_damage( dmg->victim, dmg );

   /* actual combat messaging */
   combat_message( dmg->attacker, dmg->victim, dmg, status );
   if( kill )
   {
      text_to_entity( dmg->attacker, "You have killed %s.\r\n", downcase( instance_short_descr( dmg->victim ) ) );
      make_dead( dmg->victim );
      end_killing_mode( dmg->attacker, FALSE );
      dmg->attacker->socket->bust_prompt = TRUE;
   }
}

void make_dead( ENTITY_INSTANCE *dead_instance )
{
   ENTITY_INSTANCE *corpse;
   int respawn_time;

   corpse = corpsify( dead_instance );
   entity_to_world( corpse, dead_instance->contained_by );
   onDeath_trigger( dead_instance );
   if( ( respawn_time = get_frame_spawn_time( dead_instance->framework, NULL ) ) > 0 )
      set_for_respawn( dead_instance );
   entity_to_world( dead_instance, NULL );

   return;
}

void combat_message( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim, DAMAGE *dmg, cbt_ret status )
{
   SPECIFICATION *spec;
   const char *path;
   char msg_attacker[MAX_BUFFER], msg_victim[MAX_BUFFER], msg_room[MAX_BUFFER];
   int ret, top = lua_gettop( lua_handle );

   if( ( spec = has_spec( attacker, "combatMessage" ) ) != NULL && spec->value > 0 )
      path = get_script_path_from_spec( spec );
   else
      path = "../scripts/settings/combat.lua";

   prep_stack( path, "combatMessage" );
   push_instance( attacker, lua_handle );
   push_instance( victim, lua_handle );
   push_damage( dmg, lua_handle );
   lua_pushnumber( lua_handle, (int)status );
   if( ( ret = lua_pcall( lua_handle, 4, LUA_MULTRET, 0 ) ) )
   {
      bug( "%s: ret %d: path: %s\r\n - error message: %s", __FUNCTION__, ret, path, lua_tostring( lua_handle, -1 ) );
      lua_settop( lua_handle, top );
      return;
   }

   mud_printf( msg_attacker, "%s\r\n", lua_tostring( lua_handle, -3 ) );
   mud_printf( msg_victim, "%s\r\n", lua_tostring( lua_handle, -2 ) );
   mud_printf( msg_room, "%s\r\n", lua_tostring( lua_handle, -1 ) );
   lua_settop( lua_handle, top );

   text_to_entity( attacker, msg_attacker );
   text_to_entity( victim, msg_victim );
   if( attacker->contained_by == victim->contained_by )
      text_around_entity( attacker->contained_by, 2, msg_room, attacker, victim );
   else
   {
      text_around_entity( attacker->contained_by, 1, msg_room, attacker );
      text_around_entity( victim->contained_by, 1, msg_room, victim );
   }
   return;
}

void start_killing_mode( ENTITY_INSTANCE *instance, bool message )
{
   EVENT_DATA *event;
   int cd;

   if( ( event = event_isset_instance( instance, EVENT_AUTO_ATTACK ) ) != NULL )
      return;

   event = melee_event();
   cd = get_auto_cd( instance );
   add_event_instance( event, instance, cd ? 1 : cd );
   if( message ) text_to_entity( instance, "You will attack anything you target.\r\n" );
   return;
}

void end_killing_mode( ENTITY_INSTANCE *instance, bool message )
{
   EVENT_DATA *event;

   if( ( event = event_isset_instance( instance, EVENT_AUTO_ATTACK ) ) == NULL )
      return;

   dequeue_event( event );
   if( message ) text_to_entity( instance, "You no longer attack anything you target.\r\n" );
   return;
}

/* inlines */

/* creation */
inline void free_damage_list( LLIST *damages )
{
   DAMAGE *dmg;
   ITERATOR Iter;
   AttachIterator( &Iter, damages );
   while( ( dmg = (DAMAGE *)NextInList( &Iter ) ) != NULL )
   {
      free_damage( dmg );
      DetachFromList( dmg, damages );
   }
   DetachIterator( &Iter );
}

inline EVENT_DATA *melee_event( void )
{
   EVENT_DATA *event = alloc_event();
   event->fun = &event_auto_attack;
   event->type = EVENT_AUTO_ATTACK;
   return event;
}

/* utility */
inline void add_damage( DAMAGE *dmg )
{
   if( !dmg->attacker || !dmg->victim )
   {
      bug( "%s: could not add dmg, bad attacker or victim.", __FUNCTION__ );
      return;
   }
   AttachToList( dmg, damage_queue );
   AttachToList( dmg, dmg->attacker->damages_sent );
   AttachToList( dmg, dmg->victim->damages_received );
}

inline void rem_damage( DAMAGE *dmg )
{
   DetachFromList( dmg, damage_queue );
   if( !dmg->attacker || !dmg->victim )
   {
      bug( "%s: could not rem dmg, bad attacker or victim.", __FUNCTION__ );
      return;
   }
   DetachFromList( dmg, dmg->attacker->damages_sent );
   DetachFromList( dmg, dmg->victim->damages_received );
}

/* checker */
inline bool is_dmg_queued( DAMAGE *dmg )
{
   DAMAGE *dmg_q;
   ITERATOR Iter;
   AttachIterator( &Iter, damage_queue );
   while( ( dmg_q = (DAMAGE *)NextInList( &Iter ) ) != NULL )
      if( dmg == dmg_q )
         break;
   DetachIterator( &Iter );
   if( dmg_q ) return TRUE;
   return FALSE;

}

/* setters */

inline void set_dmg_attacker( DAMAGE *dmg, ENTITY_INSTANCE *attacker )
{
   dmg->attacker = attacker;
   if( dmg->type == DMG_UNKNOWN ) return;
   FREE( dmg->timer->key );
   dmg->timer->key = strdup( compose_dmg_key( dmg ) );
}

inline void set_dmg_victim( DAMAGE *dmg, ENTITY_INSTANCE *victim )
{
   dmg->victim = victim;
   if( dmg->type == DMG_UNKNOWN ) return;
   FREE( dmg->timer->key );
   dmg->timer->key = strdup( compose_dmg_key( dmg ) );
}

inline void set_dmg_type( DAMAGE *dmg, DMG_SRC type )
{
   dmg->type = type;
   FREE( dmg->timer->key );
   dmg->timer->key = strdup( compose_dmg_key( dmg ) );
}

inline void set_dmg_src( DAMAGE *dmg, void *dmg_src, DMG_SRC type )
{
   dmg->dmg_src = dmg_src;
   set_dmg_type( dmg, type );
}
