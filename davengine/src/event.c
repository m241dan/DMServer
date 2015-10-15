#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* include main header file */
#include "mud.h"


/* quick event creation */
inline EVENT_DATA *decay_event( void )
{
   EVENT_DATA *event;
   event = alloc_event();
   event->fun = &event_instance_decay;
   event->type = EVENT_DECAY;
   return event;
}

inline EVENT_DATA *respawn_event( void )
{
   EVENT_DATA *event;
   event = alloc_event();
   event->fun = &event_instance_respawn;
   event->type = EVENT_RESPAWN;
   return event;
}

/* event_game_tick is just to show how to make global events
 * which can be used to update the game.
 */

bool event_game_tick(EVENT_DATA *event)
{
   ITERATOR Iter;
   ENTITY_INSTANCE *instance;

   AttachIterator(&Iter, eInstances_list);
   while ((instance = (ENTITY_INSTANCE *) NextInList(&Iter)) != NULL)
      text_to_entity( instance, "Tick! The event queue is working.\r\n" );
   DetachIterator(&Iter);

   event = alloc_event();
   event->fun = &event_game_tick;
   event->type = EVENT_GAME_TICK;
   add_event_game(event, 10 * 60 * PULSES_PER_SECOND);

   return FALSE;
}

/*
bool event_mobile_save(EVENT_DATA *event)
{
  D_MOBILE *dMob;

  * Check to see if there is an owner of this event.
   * If there is no owner, we return TRUE, because
   * it's the safest - and post a bug message.
   *
  if ((dMob = event->owner.dMob) == NULL)
  {
    bug("event_mobile_save: no owner.");
    return TRUE;
  }

  * save the actual player file *
  save_player(dMob);

  * enqueue a new event to save the pfile in 2 minutes *
  event = alloc_event();
  event->fun = &event_mobile_save;
  event->type = EVENT_MOBILE_SAVE;
  add_event_mobile(event, dMob, 2 * 60 * PULSES_PER_SECOND);

  return FALSE;
}
*/
bool event_socket_idle(EVENT_DATA *event)
{
  D_SOCKET *dSock;

  /* Check to see if there is an owner of this event.
   * If there is no owner, we return TRUE, because
   * it's the safest - and post a bug message.
   */
  if ((dSock = (D_SOCKET *)event->owner) == NULL)
  {
    bug("event_socket_idle: no owner.");
    return TRUE;
  }

  /* tell the socket that it has idled out, and close it */
  text_to_socket(dSock, "You have idled out...\n\n\r");
  close_socket(dSock, FALSE);

  /* since we closed the socket, all events owned
   * by that socket has been dequeued, and we need
   * to return TRUE, so the caller knows this.
   */
  return TRUE;
}

bool event_instance_lua_callback( EVENT_DATA *event )
{
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)event->owner;
   ENTITY_INSTANCE *arg_entity;
   void *content;
   ITERATOR Iter;
   int ret, counter = 0;

   prep_stack( get_frame_script_path( instance->framework ), event->argument );
   if( SizeOfList( event->lua_args ) > 0 )
   {
      AttachIterator( &Iter, event->lua_args );
      while( ( content = NextInList( &Iter ) ) != NULL )
      {
         switch( tolower( event->lua_cypher[counter++] ) )
         {
            case 's':
               lua_pushstring( lua_handle, (const char *)content );
               break;
            case 'n':
               lua_pushnumber( lua_handle, *((int *)content) );
               break;
            case 'i':
               if( ( arg_entity = get_active_instance_by_id( *((int *)content ) ) ) == NULL )
               {
                  bug( "%s: instance with ID:%d is no longer active.", __FUNCTION__, *((int *)content ) );
                  lua_pushnil( lua_handle );
                  break;
               }
               push_instance( arg_entity, lua_handle );
               break;
         }
      }
      DetachIterator( &Iter );
   }
   if( ( ret = lua_pcall( lua_handle, strlen( event->lua_cypher ), LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, get_frame_script_path( instance->framework ), lua_tostring( lua_handle, -1 ) );

   return FALSE;
}

bool event_global_lua_callback( EVENT_DATA *event )
{
   ENTITY_INSTANCE *arg_entity;
   void *content;
   ITERATOR Iter;
   int ret, counter = 0;

   prep_stack( (char *)event->owner, event->argument );
   if( SizeOfList( event->lua_args ) > 0 )
   {
      AttachIterator( &Iter, event->lua_args );
      while( ( content = NextInList( &Iter ) ) != NULL )
      {
         switch( tolower( event->lua_cypher[counter++] ) )
         {
            case 's':
               lua_pushstring( lua_handle, (const char *)content );
               break;
            case 'n':
               lua_pushnumber( lua_handle, *((int *)content) );
               break;
            case 'i':
               if( ( arg_entity = get_active_instance_by_id( *((int *)content ) ) ) == NULL )
               {
                  bug( "%s: instance with ID:%d is no longer active.", __FUNCTION__, *((int *)content ) );
                  lua_pushnil( lua_handle );
                  break;
               }
               push_instance( arg_entity, lua_handle );
               break;
         }
      }
      DetachIterator( &Iter );
   }
   if( ( ret = lua_pcall( lua_handle, strlen( event->lua_cypher ), LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s", __FUNCTION__, ret, (char *)event->owner, lua_tostring( lua_handle, -1 ) );
   return FALSE;
}

bool event_auto_attack( EVENT_DATA *event )
{
   ENTITY_INSTANCE *mob;
   int cd;

   if( event->ownertype != EVENT_OWNER_INSTANCE )
   {
      bug( "%s: bad event owner.", __FUNCTION__ );
      return FALSE;
   }
   if( ( mob = (ENTITY_INSTANCE *)event->owner ) == NULL )
   {
      bug( "%s: NULL event owner.", __FUNCTION__ );
      return FALSE;
   }

   /* check auto delay timer, requeue if necessary */
   if( ( cd = CHECK_MELEE( mob ) ) != 0 )
   {
      event = melee_event();
      add_event_instance( event, mob, cd );
      return FALSE;
   }

   /* otherwise, prep melee on target, if no target dequeue */
   if( !NO_TARGET( mob ) && TARGET_TYPE( mob ) == TARGET_INSTANCE && can_melee( mob, GT_INSTANCE( mob ) ) )
   {
      if( !(GT_INSTANCE( mob ))->primary_dmg_received_stat )
      {
         text_to_entity( mob, "You cannot attack that.\r\n" );
         goto short_melee;
      }
      prep_melee_atk( mob, GT_INSTANCE( mob ) );
      set_melee_timer( mob, FALSE );
      event = melee_event();
      add_event_instance( event, mob, CHECK_MELEE( mob ) );
      return FALSE;
   }
   text_to_entity( mob, "You aren't targetting anything.\r\n" );
short_melee:
   event = melee_event();
   add_event_instance( event, mob, (int)( 1.5 * PULSES_PER_SECOND ) );
   return FALSE;
}

bool event_instance_decay( EVENT_DATA *event )
{
   ENTITY_INSTANCE *corpse;

   if( event->ownertype != EVENT_OWNER_INSTANCE )
   {
      bug( "%s: bad event owner.", __FUNCTION__ );
      return FALSE;
   }
   if( ( corpse = (ENTITY_INSTANCE *)event->owner ) == NULL )
   {
      bug( "%s: event had a NULL owner.", __FUNCTION__ );
      return FALSE;
   }

   text_around_entity( corpse->contained_by, 0, "%s decays into nothingness.\r\n", instance_long_descr( corpse ) );
   dequeue_event( event );
   delete_eInstance( corpse );
   return TRUE;
}

bool event_instance_respawn( EVENT_DATA *event )
{
   ENTITY_INSTANCE *spawning_instance;

   if( event->ownertype != EVENT_OWNER_INSTANCE )
   {
      bug( "%s: bad event owner.", __FUNCTION__ );
      return FALSE;
   }
   if( ( spawning_instance = (ENTITY_INSTANCE *)event->owner ) == NULL )
   {
      bug( "%s: event had a NULL owner.", __FUNCTION__ );
      return FALSE;
   }

   entity_to_world( spawning_instance, spawning_instance->home );
   onSpawn_trigger( spawning_instance );
   text_around_entity( spawning_instance->contained_by, 1, "%s fades into existence.\r\n", spawning_instance, instance_short_descr( spawning_instance ) );
   return FALSE;
}
