#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/* include main header file */
#include "mud.h"

LLIST  *eventqueue[MAX_EVENT_HASH];
STACK *event_free = NULL;
LLIST  *global_events = NULL;
int    current_bucket = 0;


/* function   :: enqueue_event()
 * arguments  :: the event to enqueue and the delay time.
 * ======================================================
 * This function takes an event which has _already_ been
 * linked locally to it's owner, and places it in the
 * event queue, thus making it execute in the given time.
 */
bool enqueue_event(EVENT_DATA *event, int game_pulses)
{
  int bucket, passes;

  /* check to see if the event has been attached to an owner */
  if (event->ownertype == EVENT_UNOWNED)
  {
    bug("enqueue_event: event type %d with no owner.", event->type);
    return FALSE;
  }

  /* An event must be enqueued into the future */
  if (game_pulses < 1)
    game_pulses = 1;

  /* calculate which bucket to put the event in,
   * and how many passes the event must stay in the queue.
   */
  bucket = (game_pulses + current_bucket) % MAX_EVENT_HASH;
  passes = game_pulses / MAX_EVENT_HASH;

  /* let the event store this information */
  event->passes = passes;
  event->bucket = bucket;

   {
      time_t now;
      time( &now );
      event->time_key = now + ( game_pulses / PULSES_PER_SECOND );
   }
   new_event( event );

  /* attach the event in the queue */
  AttachToList(event, eventqueue[bucket]);

  /* success */
  return TRUE;
}

/* function   :: dequeue_event()
 * arguments  :: the event to dequeue.
 * ======================================================
 * This function takes an event which has _already_ been
 * enqueued, and removes it both from the event queue, and
 * from the owners local list. This function is usually
 * called when the owner is destroyed or after the event
 * is executed.
 */
void dequeue_event(EVENT_DATA *event)
{
   /* dequeue from the bucket */
   DetachFromList(event, eventqueue[event->bucket]);

   /* dequeue from owners local list */
   switch(event->ownertype)
   {
      default:
         bug("dequeue_event: event type %d has no owner.", event->type);
         break;
      case EVENT_OWNER_GAME:
      case EVENT_OWNER_LUA:
         DetachFromList(event, global_events);
         break;
      case EVENT_OWNER_INSTANCE:
         DetachFromList(event, ((ENTITY_INSTANCE*)event->owner)->events );
         break;
      case EVENT_OWNER_DSOCKET:
         DetachFromList(event, ((D_SOCKET *)event->owner)->events );
         break;
   }

   free_event( event );
   return;
}

/* function   :: alloc_event()
 * arguments  :: none
 * ======================================================
 * This function allocates memory for an event, and
 * makes sure it's values are set to default.
 */
EVENT_DATA *alloc_event()
{
   EVENT_DATA *event;

   CREATE( event, EVENT_DATA, 1 );

   /* clear the event */
   event->fun        = NULL;
   event->argument   = NULL;
   event->owner      = NULL;  /* only need to NULL one of the union members */
   event->passes     = 0;
   event->bucket     = 0;
   event->ownertype  = EVENT_UNOWNED;
   event->type       = EVENT_NONE;
   event->lua_cypher = NULL;
   event->lua_args   = AllocList();
   event->time_key   = -1;

   /* return the allocated and cleared event */
   return event;
}

void free_event( EVENT_DATA *event )
{
   event->fun = NULL;
   FREE( event->argument );
   event->owner = NULL;
   free_lua_args( event->lua_args );
   FreeList( event->lua_args );
   event->lua_args = NULL;
   FREE( event->lua_cypher );
   FREE( event );
}


bool (*get_event_func( sh_int ownertype, sh_int type ))( EVENT_DATA *event )
{
   switch( ownertype )
   {
      default:
         bug( "%s: no functions for ownertype %d.", __FUNCTION__, ownertype );
         return NULL;
      case EVENT_OWNER_INSTANCE:
         switch( type )
         {
            default:
               bug( "%s: no functions for type %d.", __FUNCTION__, type );
               return NULL;
            case EVENT_LUA_CALLBACK:
               return event_instance_lua_callback;
            case EVENT_AUTO_ATTACK:
               return event_auto_attack;
            case EVENT_DECAY:
               return event_instance_decay;
            case EVENT_RESPAWN:
               return event_instance_respawn;
         }
         break;
      case EVENT_OWNER_LUA:
         switch( type )
         {
            default:
               bug( "%s: no functions for type %d.", __FUNCTION__, type );
               return NULL;
            case GLOBAL_EVENT_LUA_CALLBACK:
               return event_global_lua_callback;
         }
         break;
   }
   return NULL;
}

void new_event( EVENT_DATA *event )
{
   char cypher_string[MAX_BUFFER];
   int id;

   if( event->ownertype != EVENT_OWNER_INSTANCE && event->ownertype != EVENT_OWNER_LUA )
      return;

   id = get_event_owner_id( event );
   mud_printf( cypher_string, "%s", stringify_cypher( event->lua_cypher, event->lua_args ) );

   if( !quick_query( "INSERT INTO `events` VALUES( '%s', '%d', '%d', '%d', '%d', '%s' );",
      ( event->argument || event->argument == '\0' ) ? "null" : event->argument,
      event->time_key, id, event->ownertype, event->type, cypher_string ) ) /* endif */
      bug( "%s: could not add event to the database.", __FUNCTION__ );

   return;
}

void delete_event( EVENT_DATA *event )
{
   if( event->ownertype != EVENT_OWNER_INSTANCE && event->ownertype != EVENT_OWNER_LUA )
      return;
   if( !quick_query( "DELETE FROM `events` WHERE time=%d AND owner=%d AND ownertype=%d AND type=%d", event->time_key,
      get_event_owner_id( event ), event->ownertype, event->type ) ) /* endif */
      bug( "%s: could not delete event from database.", __FUNCTION__ );
}

void load_events( ENTITY_INSTANCE *instance )
{
   EVENT_DATA *event;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;
   int owner, ownertype, pulses;

   if( instance )
   {
      owner = instance->tag->id;
      ownertype = EVENT_OWNER_INSTANCE;
   }
   else
   {
      owner = -1;
      ownertype = EVENT_OWNER_LUA;
   }

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT * FROM `events` WHERE ownertype=%d AND owner=%d;", ownertype, owner) ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      event = alloc_event();
      pulses = db_load_event( event, &row );
      enqueue_event( event, pulses );
   }
   DetachIterator( &Iter );
   return;
}

int db_load_event( EVENT_DATA *event, MYSQL_ROW *row )
{
   time_t now;
   char temp_string[MAX_BUFFER], reversed_cypher[MAX_BUFFER], cypher[MAX_BUFFER];
   char *cypher_ptr = cypher;
   int id, time_key, counter = 0;
   if( !strcmp( (*row)[counter], "null" ) )
      counter++;
   else
      event->argument = strdup( (*row)[counter++] );
   time_key = atoi( (*row)[counter++] );
   id = atoi( (*row)[counter++] );
   event->ownertype = atoi( (*row)[counter++] );
   event->type = atoi( (*row)[counter++] );
   event->fun = get_event_func( event->ownertype, event->type );

   if( event->ownertype == EVENT_OWNER_INSTANCE )
      event->owner = get_instance_by_id( id );

   if( strcmp( (*row)[counter], "null" ) )
   {
      mud_printf( cypher, "%s", (*row)[counter++] );
      while( cypher_ptr && cypher_ptr[0] != '\0' )
      {
         cypher_ptr = one_arg_delim( cypher_ptr, temp_string, ' ' );
         strcat( temp_string, reversed_cypher );
         memcpy( reversed_cypher, temp_string, MAX_BUFFER );
      }
      cypher_ptr = reversed_cypher;
      CREATE( event->lua_cypher, char, MAX_BUFFER );
      while( cypher_ptr && cypher_ptr[0] != '\0' )
      {
         char *String;
         int *Integer;

         cypher_ptr = one_arg_delim( cypher_ptr, temp_string, ' ' );
         switch( tolower( temp_string[0] ) )
         {
            default:
               bug( "%s: bad cypher.", __FUNCTION__ );
               continue;
            case 's':
               String = strdup( temp_string+1 );
               AttachToList( String, event->lua_args );
               strcat( event->lua_cypher, "s" );
               break;
            case 'i':
               CREATE( Integer, int, 1 );
               *Integer = atoi( temp_string+1 );
               AttachToList( Integer, event->lua_args );
               strcat( event->lua_cypher, "i" );
               break;
            case 'n':
               CREATE( Integer, int, 1 );
               *Integer = atoi( temp_string+1 );
               AttachToList( Integer, event->lua_args );
               strcat( event->lua_cypher, "n" );
               break;
         }
      }

   }
   time(&now);
   return ( ( time_key - now ) * PULSES_PER_SECOND );
}

/* utility functions */
int get_event_owner_id( EVENT_DATA *event )
{
   switch( event->ownertype )
   {
      default:
         return -1;
      case EVENT_OWNER_INSTANCE:
         if( !event->owner )
         {
            bug( "%s: null owner but ownertype is instance.", __FUNCTION__ );
            return -1;
         }
         return ((ENTITY_INSTANCE *)event->owner)->tag->id;
   }
}

const char *stringify_cypher( const char *lua_cypher, LLIST *list )
{
   void *content;
   ITERATOR Iter;
   static char buf[MAX_BUFFER];
   int size, counter = 0;

   if( ( size = SizeOfList( list ) ) < 1 )
      return "null";

   memset( &buf[0], 0, sizeof( buf ) );

   AttachIterator( &Iter, list );
   while( ( content = NextInList( &Iter ) ) != NULL )
   {
      switch( tolower( lua_cypher[counter++] ) )
      {
         default:
            bug( "%s: bad cypher.", __FUNCTION__ );
            continue;
         case 's':
            strcat( buf, quick_format( "s%s", (const char *)content ) );
            break;
         case 'i':
            strcat( buf, quick_format( "i%d", *((int *)content) ) );
            break;
         case 'n':
            strcat( buf, quick_format( "n%d", *((int *)content) ) );
            break;
      }
      if( counter < size )
         strcat( buf, " " );
   }
   DetachIterator( &Iter );
   return buf;
}

/* function   :: init_event_queue()
 * arguments  :: what section to initialize.
 * ======================================================
 * This function is used to initialize the event queue,
 * and the first section should be initialized at boot,
 * the second section should be called after all areas,
 * players, monsters, etc has been loaded into memory,
 * and it should contain all maintanence events.
 */
void init_event_queue(int section)
{
  EVENT_DATA *event;
  int i;

  if (section == 1)
  {
    for (i = 0; i < MAX_EVENT_HASH; i++)
    {
      eventqueue[i] = AllocList();
    }

    event_free = AllocStack();
    global_events = AllocList();
  }
  else if (section == 2)
  {
    event = alloc_event();
    event->type = EVENT_GAME_TICK;
    event->fun = &event_game_tick;
    add_event_game(event, 10 * 60 * PULSES_PER_SECOND);
  }
}

/* function   :: heartbeat()
 * arguments  :: none
 * ======================================================
 * This function is called once per game pulse, and it will
 * check the queue, and execute any pending events, which
 * has been enqueued to execute at this specific time.
 */
void heartbeat()
{
  EVENT_DATA *event;
  ITERATOR Iter;

  /* current_bucket should be global, it is also used in enqueue_event
   * to figure out what bucket to place the new event in.
   */
  current_bucket = (current_bucket + 1) % MAX_EVENT_HASH;

  AttachIterator(&Iter, eventqueue[current_bucket]);
  while ((event = (EVENT_DATA *) NextInList(&Iter)) != NULL)
  {
    /* Here we use the event->passes integer, to keep track of
     * how many times we have ignored this event.
     */
    if (event->passes-- > 0) continue;

    /* execute event and extract if needed. We assume that all
     * event functions are of the following prototype
     *
     * bool event_function ( EVENT_DATA *event );
     *
     * Any event returning TRUE is not dequeued, it is assumed
     * that the event has dequeued itself.
     */
    delete_event( event );
    if (!((*event->fun)(event)))
      dequeue_event(event);
  }
  DetachIterator(&Iter);
}

/* function   :: add_event_mobile()
 * arguments  :: the event, the owner and the delay
 * ======================================================
 * This function attaches an event to a mobile, and sets
 * all the correct values, and makes sure it is enqueued
 * into the event queue.
 */
void add_event_instance(EVENT_DATA *event, ENTITY_INSTANCE *instance, int delay)
{
  if (event->type == EVENT_NONE)
  {
    bug("%s: no type", __FUNCTION__);
    return;
  }

  if (event->fun == NULL)
  {
    bug("%s: event type %d has no callback function.", __FUNCTION__, event->type);
    return;
  }

  event->ownertype  = EVENT_OWNER_INSTANCE;
  event->owner      = instance;

  AttachToList(event, instance->events);

  if (enqueue_event(event, delay) == FALSE)
    bug("add_event_mobile: event type %d failed to be enqueued.", event->type);
}

/* function   :: add_event_socket()
 * arguments  :: the event, the owner and the delay
 * ======================================================
 * This function attaches an event to a socket, and sets
 * all the correct values, and makes sure it is enqueued
 * into the event queue.
 */
void add_event_socket(EVENT_DATA *event, D_SOCKET *dSock, int delay)
{
  /* check to see if the event has a type */
  if (event->type == EVENT_NONE)
  {
    bug("add_event_socket: no type.");
    return;
  }

  /* check to see of the event has a callback function */
  if (event->fun == NULL)
  {
    bug("add_event_socket: event type %d has no callback function.", event->type);
    return;
  }

  /* set the correct variables for this event */
  event->ownertype   = EVENT_OWNER_DSOCKET;
  event->owner       = dSock;

  /* attach the event to the sockets local list */
  AttachToList(event, dSock->events);

  /* attempt to enqueue the event */
  if (enqueue_event(event, delay) == FALSE)
    bug("add_event_socket: event type %d failed to be enqueued.", event->type);
}

/* function   :: add_event_game()
 * arguments  :: the event and the delay
 * ======================================================
 * This funtion attaches an event to the list og game
 * events, and makes sure it's enqueued with the correct
 * delay time.
 */
void add_event_game(EVENT_DATA *event, int delay)
{
  /* check to see if the event has a type */
  if (event->type == EVENT_NONE)
  {
    bug("add_event_game: no type.");
    return;
  }

  /* check to see of the event has a callback function */
  if (event->fun == NULL)
  {
    bug("add_event_game: event type %d has no callback function.", event->type);
    return;
  }

  /* set the correct variables for this event */
  event->ownertype = EVENT_OWNER_GAME;

  /* attach the event to the gamelist */
  AttachToList(event, global_events);

  /* attempt to enqueue the event */
  if (enqueue_event(event, delay) == FALSE)
    bug("add_event_game: event type %d failed to be enqueued.", event->type);
}

void add_event_lua( EVENT_DATA *event, const char *path, int delay )
{
   if( event->type == EVENT_NONE )
   {
      bug( "%s: no type.", __FUNCTION__ );
      return;
   }

   if( event->fun == NULL )
   {
      bug( "%s: event type %d has no callback function.", __FUNCTION__, event->type );
      return;
   }

   event->owner = strdup( path );
   event->ownertype = EVENT_OWNER_LUA;
   AttachToList( event, global_events );

   if( enqueue_event( event, delay ) == FALSE )
   {
      bug( "%s event type %d failed to be enqueued.", __FUNCTION__, event->type );
      DetachFromList( event, global_events );
   }
}

/* function   :: event_isset_socket()
 * arguments  :: the socket and the type of event
 * ======================================================
 * This function checks to see if a given type of event
 * is enqueued/attached to a given socket, and if it is,
 * it will return a pointer to this event.
 */
EVENT_DATA *event_isset_socket(D_SOCKET *dSock, int type)
{
  EVENT_DATA *event;
  ITERATOR Iter;

  AttachIterator(&Iter, dSock->events);
  while ((event = (EVENT_DATA *) NextInList(&Iter)) != NULL)
  {
    if (event->type == type)
      break;
  }
  DetachIterator(&Iter);

  return event;
}

/* function   :: event_isset_mobile()
 * arguments  :: the mobile and the type of event
 * ======================================================
 * This function checks to see if a given type of event
 * is enqueued/attached to a given mobile, and if it is,
 * it will return a pointer to this event.
 */

EVENT_DATA *event_isset_instance(ENTITY_INSTANCE *instance, int type)
{
  EVENT_DATA *event;
  ITERATOR Iter;

  AttachIterator(&Iter, instance->events);
  while ((event = (EVENT_DATA *) NextInList(&Iter)) != NULL)
  {
    if (event->type == type)
      break;
  }
  DetachIterator(&Iter);

  return event;
}

/* function   :: strip_event_socket()
 * arguments  :: the socket and the type of event
 * ======================================================
 * This function will dequeue all events of a given type
 * from the given socket.
 */
void strip_event_socket(D_SOCKET *dSock, int type)
{
  EVENT_DATA *event;
  ITERATOR Iter;

  AttachIterator(&Iter, dSock->events);
  while ((event = (EVENT_DATA *) NextInList(&Iter)) != NULL)
  {
    if (event->type == type)
      dequeue_event(event);
  }
  DetachIterator(&Iter);
}

/* function   :: strip_event_mobile()
 * arguments  :: the mobile and the type of event
 * ======================================================
 * This function will dequeue all events of a given type
 * from the given mobile.
 */

void strip_event_instance(ENTITY_INSTANCE *instance, int type)
{
  EVENT_DATA *event;
  ITERATOR Iter;

  AttachIterator(&Iter, instance->events);
  while ((event = (EVENT_DATA *) NextInList(&Iter)) != NULL)
  {
    if (event->type == type)
      dequeue_event(event);
  }
  DetachIterator(&Iter);
}

/* function   :: init_events_mobile()
 * arguments  :: the mobile
 * ======================================================
 * this function should be called when a player is loaded,
 * it will initialize all updating events for that player.
 */
/*
void init_events_instance(D_MOBILE *dMob)
{
  EVENT_DATA *event;

  * save the player every 2 minutes *
  event = alloc_event();
  event->fun = &event_mobile_save;
  event->type = EVENT_MOBILE_SAVE;
  add_event_mobile(event, dMob, 2 * 60 * PULSES_PER_SECOND);
}
*/
/* function   :: init_events_socket()
 * arguments  :: the mobile
 * ======================================================
 * this function should be called when a socket connects,
 * it will initialize all updating events for that socket.
 */
void init_events_socket(D_SOCKET *dSock)
{
  EVENT_DATA *event;

  /* disconnect/idle */
  event = alloc_event();
  event->fun = &event_socket_idle;
  event->type = EVENT_SOCKET_IDLE;
  add_event_socket(event, dSock, 5 * 60 * PULSES_PER_SECOND);
}
