/* event.h
 *
 * This file contains the event data struture, global variables
 * and specially defined values like MAX_EVENT_HASH.
 */

/* the size of the event queue */
#define MAX_EVENT_HASH        128

/* the different types of owners */
#define EVENT_UNOWNED           0
#define EVENT_OWNER_NONE        1
#define EVENT_OWNER_DSOCKET     2
#define EVENT_OWNER_INSTANCE    3
#define EVENT_OWNER_GAME        4
#define EVENT_OWNER_LUA         5

/* the NULL event type */
#define EVENT_NONE              0

/* Mobile events are given a type value here.
 * Each value should be unique and explicit,
 * besides that, there are no restrictions.
 */

typedef enum
{
   EVENT_LUA_CALLBACK = 1, EVENT_AUTO_ATTACK, EVENT_DECAY, EVENT_RESPAWN, MAX_INSTANCE_EVENT
} INSTANCE_EVENTS;

typedef enum
{
   GLOBAL_EVENT_LUA_CALLBACK = 1, MAX_GLOBAL_EVENT
} LUA_EVENTS;

/* Socket events are given a type value here.
 * Each value should be unique and explicit,
 * besides that, there are no restrictions.
 */
#define EVENT_SOCKET_IDLE       1

/* Game events are given a type value here.
 * Each value should be unique and explicit,
 * besides that, there are no restrictions
 */
#define EVENT_GAME_TICK         1

/* the event prototype */
typedef bool EVENT_FUN ( EVENT_DATA *event );

/* the event structure */
struct event_data
{
   EVENT_FUN        * fun;              /* the function being called           */
   char             * argument;         /* the text argument given (if any)    */
   sh_int             passes;           /* how long before this event executes */
   sh_int             type;             /* event type EVENT_XXX_YYY            */
   sh_int             bucket;           /* which bucket is this event in       */
   void             * owner;
   sh_int             ownertype;        /* type of owner (unlinking req)       */
   char             * lua_cypher;
   LLIST            * lua_args;
   int		      time_key;

};

/* functions which can be accessed outside event-handler.c */
EVENT_DATA *alloc_event          ( void );
void free_event                  ( EVENT_DATA *event );
bool (*get_event_func( sh_int ownertype, sh_int type ))( EVENT_DATA *event );

void new_event( EVENT_DATA *event );
void delete_event( EVENT_DATA *event );
void load_events( ENTITY_INSTANCE *instance );
int  db_load_event( EVENT_DATA *event, MYSQL_ROW *row );

/* utility funcs */
int get_event_owner_id( EVENT_DATA *event );
const char *stringify_cypher( const char *lua_cyper, LLIST *list );

EVENT_DATA *event_isset_socket   ( D_SOCKET *dSock, int type );
EVENT_DATA *event_isset_instance ( ENTITY_INSTANCE *instance, int type );
void dequeue_event               ( EVENT_DATA *event );
void init_event_queue            ( int section );
void init_events_socket          ( D_SOCKET *dSock );
void heartbeat                   ( void );
void add_event_socket            ( EVENT_DATA *event, D_SOCKET *dSock, int delay );
void add_event_instance          ( EVENT_DATA *event, ENTITY_INSTANCE *instance, int delay );
void add_event_game              ( EVENT_DATA *event, int delay );
void add_event_lua               ( EVENT_DATA *event, const char *path, int delay );
void strip_event_socket          ( D_SOCKET *dSock, int type );
void strip_event_instance        ( ENTITY_INSTANCE *instance, int type );

/* quick event creation */
	extern inline EVENT_DATA *decay_event	( void );
extern inline EVENT_DATA *respawn_event	( void );

/* all events should be defined here */
bool event_mobile_save           ( EVENT_DATA *event );
bool event_socket_idle           ( EVENT_DATA *event );
bool event_game_tick             ( EVENT_DATA *event );
bool event_instance_lua_callback ( EVENT_DATA *event );
bool event_global_lua_callback   ( EVENT_DATA *event );
bool event_auto_attack		 ( EVENT_DATA *event );
bool event_instance_decay	 ( EVENT_DATA *event );
bool event_instance_respawn	 ( EVENT_DATA *event );

