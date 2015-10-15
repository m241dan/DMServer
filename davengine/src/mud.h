/*
 * This is the main headerfile
 */

#ifndef MUD_H
#define MUD_H

#include <zlib.h>
#include <pthread.h>
#include <arpa/telnet.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <mysql.h>
#include <regex.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#include "llist.h"
#include "stack.h"
#include "../lua-5.2.3/src/lua.h"
#include "../lua-5.2.3/src/lualib.h"
#include "../lua-5.2.3/src/lauxlib.h"

/************************
 * Standard definitions *
 ************************/

/* define TRUE and FALSE */
#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif

#define eTHIN   0
#define eBOLD   1

#define VAR_INT 0
#define VAR_STR 1

#define MAX_INT 2114125302
#define MIN_INT -2114125302

/* A few globals */
#define PULSES_PER_SECOND     4                   /* must divide 1000 : 4, 5 or 8 works */
#define MAX_FRAMEWORK_NSL   255                   /* max for non-text string sql entrees */
#define MAX_BUFFER         4096                   /* seems like a decent amount         */
#define MAX_OUTPUT       100000                   /* well shoot me if it isn't enough   */
#define MAX_HELP_ENTRY     4096                   /* roughly 40 lines of blocktext      */
#define FILE_TERMINATOR    "EOF"                  /* end of file marker                 */
#define COPYOVER_FILE      "../txt/copyover.dat"  /* tempfile to store copyover data    */
#define EXE_FILE           "../src/SocketMud"     /* the name of the mud binary         */
#define STD_SELECTION_ERRMSG "Improper format: %s. \r\nPlease use a type char 'f' or 'i' followed by _<name> for a lookup by name or <id> for lookup by id.\r\n - Example: f_aframework, f845, i_anInstance, i1000.\r\n"
#define STD_SELECTION_ERRMSG_PTR_USED "There is a problem with the input selection pointer, please contact the nearest Admin or try again in a few seconds.\r\n"
/* Connection states */
typedef enum
{
   STATE_NANNY, STATE_ACCOUNT, STATE_OLC, STATE_EFRAME_EDITOR, STATE_EINSTANCE_EDITOR, STATE_WORKSPACE_EDITOR, STATE_SFRAME_EDITOR, STATE_PROJECT_EDITOR, STATE_BUILDER, STATE_PLAYING, STATE_CLOSED, MAX_STATE
} socket_states;

/* Thread states - please do not change the order of these states    */
#define TSTATE_LOOKUP          0  /* Socket is in host_lookup        */
#define TSTATE_DONE            1  /* The lookup is done.             */
#define TSTATE_WAIT            2  /* Closed while in thread.         */
#define TSTATE_CLOSED          3  /* Closed, ready to be recycled.   */

/* player levels */
#define LEVEL_GUEST            1  /* Dead players and actual guests  */
#define LEVEL_GOD              4  /* Any admin with shell access     */

typedef enum
{
   TARGET_FRAMEWORK, TARGET_INSTANCE, MAX_TARGET
} TARGET_TYPE;

typedef enum
{
   LEVEL_BASIC, LEVEL_PLAYER, LEVEL_ADMIN, LEVEL_DEVELOPER, LEVEL_OWNER, MAX_ACCOUNT_LEVEL
} account_levels;

typedef enum
{
   ACCOUNT_IDS, WORKSPACE_IDS, ENTITY_FRAMEWORK_IDS, ENTITY_INSTANCE_IDS, PROJECT_IDS, ENTITY_STAT_FRAMEWORK_IDS,
   MAX_ID_HANDLER
} id_handler_types;

typedef enum
{
   SAY_LEVEL, CHAT_LEVEL, MAX_COMM_LEVEL
} global_chat_levels;

/* Communication Ranges */
#define COMM_LOCAL             0  /* same room only                  */
#define COMM_LOG              10  /* admins only                     */

/* account globals */
#define DEFAULT_PAGEWIDTH     80
#define MAX_CHARACTER          3

typedef enum
{
   TYPE_INT, TYPE_CHAR, TYPE_SOCKET, MAX_MEMORY_TYPE
} memory_types;

typedef enum
{
   RET_SUCCESS, RET_FAILED_BAD_PATH, RET_FAILED_BAD_FORMAT, RET_FAILED_NULL_POINTER,
   RET_FAILED_NO_LIB_ENTRY,
   RET_NO_SQL, RET_DB_NO_ENTRY,
   RET_LIST_HAS,
   RET_FAILED_OTHER, MAX_RET_CODES
} ret_codes;

typedef enum
{
   /* IS Specs */
   SPEC_ISROOM, SPEC_ISEXIT, SPEC_ISMOB, SPEC_ISOBJECT, SPEC_ISDOOR, SPEC_ISCONTAINER,
   SPEC_ISSILENCED, SPEC_ISDEAFENED,
   /* Can Specs */
   SPEC_CANGET, SPEC_CANGIVE, SPEC_CANDROP, SPEC_CANPUT, SPEC_CANMOVE,
   /* No Specs */
   SPEC_NOGET, SPEC_NOGIVE, SPEC_NODROP, SPEC_NOPUT, SPEC_NOMOVE,
   /* Scripting Specs */
   SPEC_ONENTER, SPEC_ONLEAVE, SPEC_ONENTERING, SPEC_ONLEAVING,
   SPEC_ONGREET, SPEC_ONFAREWELL,
   /* Combat Specs */
   SPEC_DODGECHANCE, SPEC_PARRYCHANCE, SPEC_MISSCHANCE, SPEC_MELEECOOLDOWN, SPEC_MELEECHECK,
   SPEC_PREPMELEETIMER, SPEC_PREPMELEEDAMAGE, SPEC_ONRECEIVEDAMAGE, SPEC_COMBATMESSAGE,
   /* Corpse Specs */
   SPEC_CORPSEDELAY, SPEC_INVENTORYTOCORPSE,
   /* UI Specs */
   SPEC_UIPROMPT, SPEC_UILOOK, SPEC_UIINVENTORY, SPEC_UISCORE,
   /* Misc Specs */
   SPEC_MIRROREXIT, SPEC_TERRAIN,

   MAX_SPEC
} SPEC_IDS;

typedef enum
{
   TERRAIN_ETHER,
   MAX_TERRAIN
} TERRAIN_IDS;

typedef enum
{
   PAK_STAT, PAK_SPEC
} PAK_ENTRY_TYPES;

#define MAX_QUICK_SORT (SPEC_ISOBJECT+1)

typedef enum
{
   NO_PROMPT, NORMAL_PROMPT, SHORT_PROMPT, MAX_PROMPT
} prompt_type;

typedef enum
{
   SEL_NULL = -1, SEL_FRAME = 0, SEL_INSTANCE, SEL_WORKSPACE, SEL_PROJECT, SEL_STRING, SEL_STAT_FRAMEWORK,
   MAX_SEL
} SEL_TYPING;

typedef enum
{
   COM_DROP, COM_GET, COM_PUT, COM_GIVE
} ITEM_MOVE_COM;

/*****************
 * NANNY INDEXES *
 *****************/
typedef enum
{
   NANNY_LOGIN, NANNY_NEW_ACCOUNT, MAX_NANNY
} nanny_indexes;

/* define simple types */
typedef  unsigned char     bool;
typedef  short int         sh_int;
typedef  unsigned long int VALUE;


extern const unsigned char do_echo       [];
extern const unsigned char dont_echo       [];

/******************************
 * End of standard definitons *
 ******************************/

/***********************
 * Defintion of Macros *
 ***********************/
#define BAD_POINTER( pointer )						\
do									\
{									\
   bug( "%s: BAD POINTER %s." , __FUNCTION__, (pointer) );		\
   ret = RET_FAILED_NULL_POINTER;					\
} while(0)

#define BAD_PATH( path )						\
do									\
{									\
   bug( "%s: BAD PATH %s.", __FUNCTION__, (path) );			\
   ret = RET_FAILED_BAD_PATH;						\
} while(0)

#define BAD_FORMAT( word ) 						\
do									\
{									\
   bug( "%s: BAD FORMAT %s.", __FUNCTION__, (word) );					\
   ret = RET_FAILED_BAD_FORMAT;						\
} while(0)

#define DETACHCONTENTS( LLIST, type )					\
do									\
{									\
   (type) *to_detach;							\		\
   AttachIterator( &Iter, (LLIST) );					\
   while( ( to_detach = (type *)NextInLLIST( &Iter ) ) != NULL )		\
      DetachFromLLIST( to_detach, (LLIST) )				\
   DetachIterator( &Iter );						\
} while(0)

#define CREATE(result, type, number)                                    \
do                                                                      \
{                                                                       \
   if (!((result) = (type *) calloc ((number), sizeof(type))))          \
   {                                                                    \
      perror("malloc failure");                                         \
      fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
      abort();                                                          \
   }									\
} while(0)


#define FREE(point)                      \
do                                          \
{                                           \
   if( (point) )                            \
   {                                        \
      free( (void*) (point) );              \
      (point) = NULL;                       \
   }                                        \
} while(0)

#define DAVLUACM_INSTANCE_NIL( instance, L )				\
do									\
{									\
   if( ( (instance) = *(ENTITY_INSTANCE **)luaL_checkudata( (L), 1, "EntityInstance.meta" ) ) == NULL ) \
   {									\
      bug( "%s: bad meta table.", __FUNCTION__ );			\
      lua_pushnil( (L) );						\
      return 1;								\
   }									\
} while(0)

#define DAVLUACM_INSTANCE_BOOL( instance, L )				\
do									\
{									\
   if( ( (instance) = *(ENTITY_INSTANCE **)luaL_checkudata( (L), 1, "EntityInstance.meta" ) ) == NULL ) \
   {									\
      bug( "%s: bad meta table.", __FUNCTION__ );			\
      lua_pushboolean( (L), 0 );					\
      return 1;								\
   }									\
} while(0)

#define DAVLUACM_INSTANCE_NONE( instance, L )				\
do                                                                      \
{                                                                       \
   if( ( (instance) = *(ENTITY_INSTANCE **)luaL_checkudata( (L), 1, "EntityInstance.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      return 0;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_FRAME_NIL( frame, L )					\
do									\
{									\
   if( ( (frame) = *(ENTITY_FRAMEWORK **)luaL_checkudata( (L), 1, "EntityFramework.meta" ) ) == NULL ) \
   {									\
      bug( "%s: bad meta table.", __FUNCTION__ );			\
      lua_pushnil( (L) );						\
      return 1;								\
   }									\
} while(0)

#define DAVLUACM_FRAME_BOOL( frame, L )                                 \
do                                                                      \
{                                                                       \
   if( ( (frame) = *(ENTITY_FRAMEWORK **)luaL_checkudata( (L), 1, "EntityFramework.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      lua_pushboolean( (L), 0 );                                        \
      return 1;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_FRAME_NONE( frame, L )                                 \
do                                                                      \
{                                                                       \
   if( ( (frame) = *(ENTITY_FRAMEWORK **)luaL_checkudata( (L), 1, "EntityFramework.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      return 0;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_DAMAGE_NIL( dmg, L )					\
do									\
{									\
   if( ( (dmg) = *(DAMAGE **)luaL_checkudata( (L), 1, "Damage.meta" ) ) == NULL ) \
   {									\
      bug( "%s: bad meta table.", __FUNCTION__ );			\
      lua_pushnil( (L) );						\
      return 1;								\
   }									\
} while(0)

#define DAVLUACM_DAMAGE_BOOL( dmg, L )                                  \
do                                                                      \
{                                                                       \
   if( ( (dmg) = *(DAMAGE **)luaL_checkudata( (L), 1, "Damage.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      lua_pushboolean( (L), 0 );                                        \
      return 1;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_DAMAGE_NONE( dmg, L )                                  \
do                                                                      \
{                                                                       \
   if( ( (dmg) = *(DAMAGE **)luaL_checkudata( (L), 1, "Damage.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      return 0;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_TIMER_NIL( timer, L )                                  \
do                                                                      \
{                                                                       \
   if( ( (timer) = *(TIMER **)luaL_checkudata( (L), 1, "Timers.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      lua_pushnil( (L) );                                               \
      return 1;                                                         \
   }                                                                    \
} while(0)
#define DAVLUACM_TIMER_BOOL( timer , L )                                \
do                                                                      \
{                                                                       \
   if( ( (timer) = *(TIMER **)luaL_checkudata( (L), 1, "Timers.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      lua_pushboolean( (L), 0 );                                        \
      return 1;                                                         \
   }                                                                    \
} while(0)
#define DAVLUACM_TIMER_NONE( timer, L )                                 \
do                                                                      \
{                                                                       \
   if( ( (timer) = *(TIMER **)luaL_checkudata( (L), 1, "Timers.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      return 0;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_ACCOUNT_NIL( account, L )                              \
do                                                                      \
{                                                                       \
   if( ( (account) = *(ACCOUNT_DATA **)luaL_checkudata( (L), 1, "Account.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      lua_pushnil( (L) );                                               \
      return 1;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_ACCOUNT_BOOL( account, L )                             \
do                                                                      \
{                                                                       \
   if( ( (account) = *(ACCOUNT_DATA **)luaL_checkudata( (L), 1, "Account.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      lua_pushboolean( (L), 0 );                                        \
      return 1;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_ACCOUNT_NONE( account, L )                             \
do                                                                      \
{                                                                       \
   if( ( (account) = *(ACCOUNT_DATA **)luaL_checkudata( (L), 1, "Account.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      return 0;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_NANNY_NIL( nanny, L )                              \
do                                                                      \
{                                                                       \
   if( ( (nanny) = *(NANNY_DATA **)luaL_checkudata( (L), 1, "Nanny.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      lua_pushnil( (L) );                                               \
      return 1;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_NANNY_BOOL( nanny, L )                             \
do                                                                      \
{                                                                       \
   if( ( (nanny) = *(NANNY_DATA **)luaL_checkudata( (L), 1, "Nanny.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      lua_pushboolean( (L), 0 );                                        \
      return 1;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_NANNY_NONE( nanny, L )                             \
do                                                                      \
{                                                                       \
   if( ( (nanny) = *(NANNY_DATA **)luaL_checkudata( (L), 1, "Nanny.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      return 0;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_SOCKET_NIL( socket, L )                              \
do                                                                      \
{                                                                       \
   if( ( (socket) = *(D_SOCKET **)luaL_checkudata( (L), 1, "Socket.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      lua_pushnil( (L) );                                               \
      return 1;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_SOCKET_BOOL( socket, L )                             \
do                                                                      \
{                                                                       \
   if( ( (socket) = *(D_SOCKET **)luaL_checkudata( (L), 1, "Socket.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      lua_pushboolean( (L), 0 );                                        \
      return 1;                                                         \
   }                                                                    \
} while(0)

#define DAVLUACM_SOCKET_NONE( socket, L )                             \
do                                                                      \
{                                                                       \
   if( ( (socket) = *(D_SOCKET **)luaL_checkudata( (L), 1, "Socket.meta" ) ) == NULL ) \
   {                                                                    \
      bug( "%s: bad meta table.", __FUNCTION__ );                       \
      return 0;                                                         \
   }                                                                    \
} while(0)


#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)              ((a) < (b) ? (b) : (a))
#define IS_ADMIN(dMob)          ((dMob->level) > LEVEL_PLAYER ? TRUE : FALSE)
#define IREAD(sKey, sPtr)             \
{                                     \
  if (!strcasecmp(sKey, word))        \
  {                                   \
    int sValue = fread_number(fp);    \
    sPtr = sValue;                    \
    found = TRUE;                     \
    break;                            \
  }                                   \
}
#define SREAD(sKey, sPtr)             \
{                                     \
  if (!strcasecmp(sKey, word))        \
  {                                   \
    sPtr = fread_string(fp);          \
    found = TRUE;                     \
    break;                            \
  }                                   \
}
/* " */

/***********************
 * End of Macros       *
 ***********************/

/******************************
 * New structures             *
 ******************************/

/* type defintions */
typedef struct  dSocket       D_SOCKET;
typedef struct  help_data     HELP_DATA;
typedef struct  lookup_data   LOOKUP_DATA;
typedef struct  event_data    EVENT_DATA;
typedef struct  game_account  ACCOUNT_DATA;
typedef struct  nanny_data    NANNY_DATA;
typedef struct  typCmd        COMMAND;
typedef int     nanny_fun( NANNY_DATA *nanny, char *arg );
typedef const struct nanny_lib_entry NANNY_LIB_ENTRY;
typedef struct  id_handler    ID_HANDLER;
typedef struct  id_tag        ID_TAG;
typedef struct  entity_framework ENTITY_FRAMEWORK;
typedef struct  entity_instance ENTITY_INSTANCE;
typedef struct  inception_olc  INCEPTION;
typedef struct  workspace      WORKSPACE;
typedef struct  typSpec        SPECIFICATION;
typedef struct  project        PROJECT;
typedef struct  dirent         DIR_FILE;
typedef struct  grab_params    GRAB_PARAMS;
typedef struct  workspace_filter WORKSPACE_FILTER;
typedef struct  d_string       STRING;
typedef struct  editor_chain   E_CHAIN;
typedef struct  target_data    TARGET_DATA;
typedef struct  entity_variable EVAR;
typedef struct  stat_framework  STAT_FRAMEWORK;
typedef struct  stat_instance   STAT_INSTANCE;
typedef struct  damage_data     DAMAGE;
typedef struct  timer           TIMER;
typedef struct  character_sheet CHAR_SHEET;
typedef struct  element_framework ELEMENT_FRAMEWORK;
typedef struct  element_info	ELEMENT_INFO;
typedef struct  composition	COMPOSITION;
/* the actual structures */
struct dSocket
{
  LLIST          * events;
  char          * hostname;
  char            inbuf[MAX_BUFFER];
  char            outbuf[MAX_OUTPUT];
  char            next_command[MAX_BUFFER];
  prompt_type     bust_prompt;
  sh_int          lookup_status;
  sh_int          state;
  sh_int          prev_state;
  sh_int          control;
  sh_int          top_output;
  unsigned char   compressing;                 /* MCCP support */
  z_stream      * out_compress;                /* MCCP support */
  unsigned char * out_compress_buf;            /* MCCP support */

   NANNY_DATA   * nanny;
   ACCOUNT_DATA * account;
   STACK        * prev_control_stack;
   ENTITY_INSTANCE *controlling;
};

struct help_data
{
  time_t          load_time;
  char          * keyword;
  char          * text;
};
struct lookup_data
{
  D_SOCKET       * dsock;   /* the socket we wish to do a hostlookup on */
  char           * buf;     /* the buffer it should be stored in        */
};

struct typCmd
{
   char        * cmd_name;
   void       (* cmd_funct)(void *passed, char *arg);
   sh_int       level;
   LLIST       *sub_commands;
   bool      	can_sub;
   const char *(*desc_func)( void *extra );
   COMMAND     *from_table;
   const char *path;
   bool		lua_cmd;
};

typedef struct buffer_type
{
  char   * data;        /* The data                      */
  int      len;         /* The current len of the buffer */
  int      size;        /* The allocated size of data    */
} BUFFER;

/* here we include external structure headers */
#include "strings.h"
#include "event.h"
#include "account.h"
#include "strings_table.h"
#include "nanny.h"
#include "interpret.h"
#include "id-handler.h"
#include "communication.h"
#include "olc.h"
#include "frameworks.h"
#include "instances.h"
#include "editor.h"
#include "specifications.h"
#include "projects.h"
#include "lua_utils.h"
#include "lua_instance.h"
#include "lua_specification.h"
#include "lua_framework.h"
#include "target.h"
#include "entity_variables.h"
#include "entity_stats.h"
#include "pak.h"
#include "combat.h"
#include "lua_damage.h"
#include "timers.h"
#include "lua_timers.h"
#include "lua_iter.h"
#include "lua_triggers.h"
#include "lua_ui.h"
#include "lua_account.h"
#include "lua_nanny.h"
#include "lua_socket.h"
#include "elements.h"

/******************************
 * End of new structures      *
 ******************************/

/***************************
 * Global Variables        *
 ***************************/

extern  STACK        *  dsock_free;       /* the socket free LLIST               */
extern  LLIST        *  dsock_list;       /* the linked LLIST of active sockets  */
extern  LLIST        *  account_list;     /* the linked List of active accounts */
extern  LLIST        *  active_wSpaces;   /* a linked list for active work spaces */
extern  LLIST	     *	active_OLCs;	  /* a linked list of active OLCs */
extern  LLIST        *  active_frameworks; /* a linked list of active frameworks */
extern  LLIST        *  help_list;        /* the linked LLIST of help files      */
extern  LLIST        *  eInstances_list;   /* list of entity instances */
extern  MYSQL        *  sql_handle;       /* global connection to sql database */
extern  MYSQL        *  help_handle;
extern  lua_State    *  lua_handle;       /* global connection to lua */
extern  const struct    typCmd tabCmd[];  /* the command table                  */
extern  bool            shut_down;        /* used for shutdown                  */
extern  char        *   greeting;         /* the welcome greeting               */
extern  char        *   motd;             /* the MOTD help file                 */
extern  int             control;          /* boot control socket thingy         */
extern  time_t          current_time;     /* let's cut down on calls to time()  */


/* server settings */
extern int  MUDPORT;

/* database settings */
extern const char *DB_NAME;
extern const char *DB_ADDR;
extern const char *DB_LOGIN;
extern const char *DB_PASSWORD;
extern const char *WIKI_NAME;

/* combat settings */
extern bool AUTOMELEE;
extern bool DODGE_ON;
extern bool PARRY_ON;
extern bool MISS_ON;
extern int  BASE_MELEE_DELAY;

/* corpse settings */
extern int  CORPSE_DECAY;
/***************************
 * End of Global Variables *
 ***************************/

/***********************
 *    MCCP support     *
 ***********************/

extern const unsigned char compress_will[];
extern const unsigned char compress_will2[];

#define TELOPT_COMPRESS       85
#define TELOPT_COMPRESS2      86
#define COMPRESS_BUF_SIZE   8192

/***********************
 * End of MCCP support *
 ***********************/

/***********************************
 * Prototype function declerations *
 ***********************************/

/* more compact */
#define  D_S         D_SOCKET

#define  buffer_new(size)             __buffer_new     ( size)
#define  buffer_strcat(buffer,text)   __buffer_strcat  ( buffer, text )

char  *crypt                  ( const char *key, const char *salt );

/*
 * socket.c
 */
int   init_socket             ( void );
bool  new_socket              ( int sock );
void  close_socket            ( D_S *dsock, bool reconnect );
bool  read_from_socket        ( D_S *dsock );
bool  text_to_socket          ( D_S *dsock, const char *txt );  /* sends the output directly */
void  text_to_buffer          ( D_S *dsock, const char *txt );  /* buffers the output        */
void  next_cmd_from_buffer    ( D_S *dsock );
bool  flush_output            ( D_S *dsock );
void  handle_new_connections  ( D_S *dsock, char *arg );
void  clear_socket            ( D_S *sock_new, int sock );
void  recycle_sockets         ( void );
void *lookup_address          ( void *arg );
void socket_control_entity( D_SOCKET *socket, ENTITY_INSTANCE *entity );
void socket_uncontrol_entity( ENTITY_INSTANCE *entity );
void  get_info		      ( D_SOCKET *socket );
void  get_commands	      ( D_SOCKET *socket );

/*
 * interpret.c
 */
void  handle_cmd_input        ( D_S *dsock, char *arg );

/*
 * io.c
 */
void    log_string            ( const char *txt, ... );
void    bug                   ( const char *txt, ... );
time_t  last_modified         ( char *helpfile );
char   *read_help_entry       ( const char *helpfile );     /* pointer         */
char   *fread_line            ( FILE *fp );                 /* pointer         */
char   *fread_line_string     ( FILE *fp );                 /* return pointer, not static memory */
char   *fread_string          ( FILE *fp );                 /* allocated data  */
char   *fread_word            ( FILE *fp );                 /* pointer         */
int     fread_number          ( FILE *fp );                 /* just an integer */
int directory_file_count_regex( DIR *directory, const char *regex_string );
void    copy_flat_file        ( FILE *dest, FILE *src );
const char *fread_file        ( FILE *fp );

/*
 * socket.c
 */
int change_socket_state( D_SOCKET *dsock, int state );

/*
 * help.c
 */
void  load_helps              ( void );
void get_help( D_SOCKET *dsock, char *arg );
const char *grab_help_from_wiki( const char *name );

/*
 * utils.c
 */
bool  check_name              ( const char *name );
void  load_muddata            ( bool fCopyOver );
char *get_time                ( void );
bool check_sql( void );
void report_sql_error( MYSQL *con );
bool quick_query( const char *format, ...);
MYSQL_ROW db_query_single_row( const char *query );
bool db_query_list_row( LLIST *list, const char *query );
void debug_row( MYSQL_ROW *row, int size );
void debug_row_list( LLIST *list );
void copy_row( MYSQL_ROW *row_dest, MYSQL_ROW *row_src, int size );
int number_percent();
int number_range( int min, int max );
extern inline void clearlist( LLIST *list );
extern inline int urange( int mincheck, int check, int maxcheck );

/*
 * mccp.c
 */
bool  compressStart           ( D_S *dsock, unsigned char teleopt );
bool  compressEnd             ( D_S *dsock, unsigned char teleopt, bool forced );

/*******************************
 * End of prototype declartion *
 *******************************/

#endif  /* MUD_H */

