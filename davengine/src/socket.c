/*
 * This file contains the socket code, used for accepting
 * new connections as well as reading and writing to
 * sockets, and closing down unused sockets.
 */

#include <mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ctype.h>

/* including main header file */
#include "mud.h"

/* global variables */
fd_set     fSet;                  /* the socket LLISTfor polling       */
LLIST    * dsock_list= NULL;     /* the linked LLISTof active sockets */
LLIST    * dmobile_list= NULL;   /* the mobile LLISTof active mobiles */
LLIST    * account_list = NULL;
LLIST    * active_wSpaces = NULL;
LLIST    * active_OLCs = NULL;
LLIST    * active_frameworks = NULL;
LLIST    * eInstances_list = NULL;
LLIST    * global_variables = NULL;
LLIST    * stat_frameworks = NULL;
LLIST    * damage_queue = NULL;
LLIST    * timer_queue = NULL;
LLIST    * paused_timer_queue = NULL;
LLIST	 * element_frameworks = NULL;
/* server settings */
int        MUDPORT = 0;

/* database settings */
const char *DB_NAME = NULL;
const char *DB_ADDR = NULL;
const char *DB_LOGIN = NULL;
const char *DB_PASSWORD = NULL;
const char *WIKI_NAME = NULL;

/* combat settings */
bool AUTOMELEE = FALSE;
bool DODGE_ON = FALSE;
bool PARRY_ON = FALSE;
bool MISS_ON = FALSE;
int  BASE_MELEE_DELAY = 10;

/* corpse settings */
int  CORPSE_DECAY = 480;

MYSQL    * sql_handle = NULL;
MYSQL    * help_handle = NULL;
lua_State *lua_handle  = NULL;

/* mccp support */
const unsigned char compress_will   [] = { IAC, WILL, TELOPT_COMPRESS,  '\0' };
const unsigned char compress_will2  [] = { IAC, WILL, TELOPT_COMPRESS2, '\0' };
const unsigned char do_echo         [] = { IAC, WONT, TELOPT_ECHO,      '\0' };
const unsigned char dont_echo       [] = { IAC, WILL, TELOPT_ECHO,      '\0' };

/* local procedures */
void GameLoop         ( int control );

/* intialize shutdown state */
bool shut_down = FALSE;
int  control;

/*
 * This is where it all starts, nothing special.
 */
int main(int argc, char **argv)
{
  bool fCopyOver;

  /* get the current time */
  current_time = time(NULL);
  srand( time( NULL ) );

   /* allocate memory for socket and mobile lists'n'stacks */
   dsock_list= AllocList();
   dmobile_list= AllocList();
   account_list = AllocList();
   active_wSpaces = AllocList();
   active_OLCs = AllocList();
   active_frameworks = AllocList();
   eInstances_list = AllocList();
   global_variables = AllocList();
   stat_frameworks = AllocList();
   damage_queue = AllocList();
   timer_queue = AllocList();
   paused_timer_queue = AllocList();
   element_frameworks = AllocList();

   builder_count = 0;

  /* note that we are booting up */
  log_string("Program starting.");


   log_string( "Connecting to Lua" );

   if( ( lua_handle = luaL_newstate() ) == NULL )
   {
      bug( "Could not initialize the Lua handle" );
      exit(1);
   }

   log_string( "Loading Lua Libraries" );
   luaL_openlibs( lua_handle );

   luaL_requiref( lua_handle, "EntityInstance", luaopen_EntityInstanceLib, 1 );
   lua_pop( lua_handle, -1 );

   luaL_requiref( lua_handle, "EntityFramework", luaopen_EntityFrameworkLib, 1 );
   lua_pop( lua_handle, -1 );

   luaL_requiref( lua_handle, "Specification", luaopen_SpecificationLib, 1 );
   lua_pop( lua_handle, -1 );

   luaL_requiref( lua_handle, "Damage", luaopen_DamageLib, 1 );
   lua_pop( lua_handle, -1 );

   luaL_requiref( lua_handle, "Timers", luaopen_TimersLib, 1 );
   lua_pop( lua_handle, -1 );

   luaL_requiref( lua_handle, "Account", luaopen_AccountLib, 1 );
   lua_pop( lua_handle, -1 );

   luaL_requiref( lua_handle, "Nanny", luaopen_NannyLib, 1 );
   lua_pop( lua_handle, -1 );

   luaL_requiref( lua_handle, "Socket", luaopen_SocketLib, 1 );
   lua_pop( lua_handle, -1 );

   luaL_requiref( lua_handle, "mud", luaopen_mud, 1 );
   lua_pop( lua_handle, -1 );

   luaopen_IterLib( lua_handle );

   load_server_script();
   load_combat_vars_script();
   load_lua_command_tables();
   load_lua_misc_vars();
   load_lua_misc_funcs();
   load_lua_element_table();
   lua_server_settings(); /* loading server stuff */
   lua_database_settings(); /* loading the sql variables */
   lua_combat_settings(); /* loading combat settings */

   log_string( "Connecting to Database" );

/* test if lua is working
   {
      lua_pushnil( lua_handle );
      lua_setglobal( lua_handle, "boot" );

      int ret = luaL_loadfile( lua_handle, "../scripts/boot.lua" );
      if( ret )
      {
         if( ret != LUA_ERRFILE )
            bug( "%s: boot(): %s\r\n", __FUNCTION__, lua_tostring( lua_handle, - 1 ) );
         lua_pop( lua_handle, 1 );
         exit(1);
      }
      ret = lua_pcall( lua_handle, 0, 0, 0 );
      if( ret )
      {
         bug( "%s: boot(): %s\n\r", __FUNCTION__, lua_tostring( lua_handle, -1 ) );
         lua_pop( lua_handle, 1 );
         exit(1);
      }
      lua_getglobal( lua_handle, "boot" );
      if( lua_isnil( lua_handle, -1 ) )
      {
         bug( "%s: boot is nil.", __FUNCTION__ );
         lua_pop( lua_handle, 1 );
         exit(1);
      }
      lua_pcall( lua_handle, 0, 0, 0 );
   }
*/
   if( ( sql_handle = mysql_init(NULL) ) == NULL )
   {
      bug( "Could not initialize mysql connection: %s", mysql_error( sql_handle ) );
      exit(1);
   }

   if( mysql_real_connect( sql_handle, DB_ADDR, DB_LOGIN, DB_PASSWORD, DB_NAME, 0, NULL, 0 ) == NULL )
   {
      bug( "Could not connect to database: %s", mysql_error( sql_handle ) );
      mysql_close( sql_handle );
      exit(1);
   }

   if( ( help_handle = mysql_init(NULL) ) == NULL )
   {
      bug( "Could not initialize mysql connection: %s", mysql_error( help_handle ) );
      exit(1);
   }

   if( mysql_real_connect( help_handle, DB_ADDR, DB_LOGIN, DB_PASSWORD, WIKI_NAME, 0, NULL, 0 ) == NULL )
   {
      bug( "Could not connect to database: %s", mysql_error( help_handle ) );
      mysql_close( help_handle );
      mysql_close( sql_handle );
      exit(1);
   }

   log_string( "Loading Global Lua Variables" );
   load_global_vars();

   log_string( "Loading ID Handlers" );

   if( load_id_handlers() != RET_SUCCESS )
   {
      bug( "%s: There was a problem loading ID Handlers from Database.", __FUNCTION__ );
      mysql_close( sql_handle );
      exit(1);
   }

   if( load_recycled_ids() != RET_SUCCESS )
   {
      bug( "%s: There was a problem loading recycled IDs from Database.", __FUNCTION__ );
      mysql_close( sql_handle );
      exit(1);
   }

   log_string( "Loading Mud Timers" );
   load_mud_timers();
   load_events( NULL );

   log_string( "Loading Element Table" );
   load_elements_table();
/*
   log_string( "Loading Workspaces" );
   if( load_workspaces() != RET_SUCCESS )
   {
      bug( "%s: There was a problem loading worksapces from Database.", __FUNCTION__ );
      mysql_close( sql_handle );
      exit(1);
   }
*/
  /* initialize the event queue - part 1 */
  init_event_queue(1);

  if (argc > 2 && !strcmp(argv[argc-1], "copyover") && atoi(argv[argc-2]) > 0)
  {
    fCopyOver = TRUE;
    control = atoi(argv[argc-2]);
  }
  else fCopyOver = FALSE;

  /* initialize the socket */
  if (!fCopyOver)
    control = init_socket();

  /* load all external data */
  load_muddata(fCopyOver);

  /* initialize the event queue - part 2*/
  init_event_queue(2);

  /* main game loop */
  GameLoop(control);


  /* close down the socket */
  close(control);

   lua_close( lua_handle );

  /* terminated without errors */
  log_string("Program terminated without errors.");

  /* and we are done */
  return 0;
}

void GameLoop(int control)   
{
  D_SOCKET *dsock;
  ITERATOR Iter;
  static struct timeval tv;
  struct timeval last_time, new_time;
  extern fd_set fSet;
  fd_set rFd;
  long secs, usecs;

  /* set this for the first loop */
  gettimeofday(&last_time, NULL);

  /* clear out the file socket set */
  FD_ZERO(&fSet);

  /* add control to the set */
  FD_SET(control, &fSet);

  /* copyover recovery */
  AttachIterator(&Iter, dsock_list);
  while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
    FD_SET(dsock->control, &fSet);
  DetachIterator(&Iter);

  /* do this untill the program is shutdown */
  while (!shut_down)
  {
    /* set current_time */
    current_time = time(NULL);

    /* copy the socket set */
    memcpy(&rFd, &fSet, sizeof(fd_set));

    /* wait for something to happen */
    if (select(FD_SETSIZE, &rFd, NULL, NULL, &tv) < 0)
      continue;

    /* check for new connections */
    if (FD_ISSET(control, &rFd))
    {
      struct sockaddr_in sock;
      unsigned int socksize;
      int newConnection;

      socksize = sizeof(sock);
      if ((newConnection = accept(control, (struct sockaddr*) &sock, &socksize)) >=0)
        new_socket(newConnection);
    }

    /* poll sockets in the socket LLIST*/
    AttachIterator(&Iter ,dsock_list);
    while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
    {
      /*
       * Close sockects we are unable to read from.
       */
      if (FD_ISSET(dsock->control, &rFd) && !read_from_socket(dsock))
      {
        close_socket(dsock, FALSE);
        continue;
      }

      /* Ok, check for a new command */
      next_cmd_from_buffer(dsock);

      /* Is there a new command pending ? */
      if (dsock->next_command[0] != '\0')
      {
        if( is_prefix( dsock->next_command, "/help" ) )
           get_help( dsock, dsock->next_command );
        else if( is_prefix( dsock->next_command, "/info" ) )
           get_info( dsock );
        else if( is_prefix( dsock->next_command, "/commands" ) )
           get_commands( dsock );
        else
        {
           switch(dsock->state)
           {
             default:
               bug("Descriptor in bad state.");
               break;
             case STATE_NANNY:
               if( handle_nanny_input( dsock, dsock->next_command ) != RET_SUCCESS )
                  bug( "handle_nanny_input failed to interpret the input." );
               break;
             case STATE_ACCOUNT:
                if( account_handle_cmd( dsock->account, dsock->next_command ) != RET_SUCCESS )
                   bug( "account_handle_cmd failed to interpret the input." );
                break;
             case STATE_OLC:
                if( olc_handle_cmd( dsock->account->olc, dsock->next_command ) != RET_SUCCESS )
                   bug( "olc_handle_cmd failed to interpret the input." );
                break;
             case STATE_EFRAME_EDITOR:
                if( eFrame_editor_handle_command( dsock->account->olc, dsock->next_command ) != RET_SUCCESS )
                   bug( "eFrame_editor_handle_command failed to interpret the input." );
                break;
             case STATE_PROJECT_EDITOR:
                if( project_editor_handle_command( dsock->account->olc, dsock->next_command ) != RET_SUCCESS )
                   bug( "project_editor_handle_command failed to inter[ret the input." );
                break;
             case STATE_WORKSPACE_EDITOR:
                if( workspace_editor_handle_command( dsock->account->olc, dsock->next_command ) != RET_SUCCESS )
                   bug( "workspace_editor_handle_command failed to interpret the input." );
                break;
             case STATE_EINSTANCE_EDITOR:
                if( instance_editor_handle_command( dsock->account->olc, dsock->next_command ) != RET_SUCCESS )
                   bug( "instance_editor_handle_command failed to interpret the input." );
                break;
             case STATE_SFRAME_EDITOR:
                if( sFrame_editor_handle_command( dsock->account->olc, dsock->next_command ) != RET_SUCCESS )
                   bug( "sframe_editor_handle_command failed to interpret the input." );
                break;
             case STATE_BUILDER:
             case STATE_PLAYING:
                if( entity_handle_cmd( dsock->controlling, dsock->next_command ) != RET_SUCCESS )
                   bug( "entity_handle_cmd failed ot interpret the input." );
                break;
           }
        }

        dsock->next_command[0] = '\0';
      }

      /* if the player quits or get's disconnected */
      if (dsock->state == STATE_CLOSED) continue;

      /* Send all new data to the socket and close it if any errors occour */
      if (!flush_output(dsock))
        close_socket(dsock, FALSE);
    }
    DetachIterator(&Iter);

    /* call the timer queue */
    timer_monitor();
    /* call the event queue */
    heartbeat();
    /*
     * Here we sleep out the rest of the pulse, thus forcing
     * SocketMud(tm) to run at PULSES_PER_SECOND pulses each second.
     */
    gettimeofday(&new_time, NULL);

    /* get the time right now, and calculate how long we should sleep */
    usecs = (int) (last_time.tv_usec -  new_time.tv_usec) + 1000000 / PULSES_PER_SECOND;
    secs  = (int) (last_time.tv_sec  -  new_time.tv_sec);

    /*
     * Now we make sure that 0 <= usecs < 1.000.000
     */
    while (usecs < 0)
    {
      usecs += 1000000;
      secs  -= 1;
    }
    while (usecs >= 1000000)
    {
      usecs -= 1000000;
      secs  += 1;
    }

    /* if secs < 0 we don't sleep, since we have encountered a laghole */
    if (secs > 0 || (secs == 0 && usecs > 0))
    {
      struct timeval sleep_time;

      sleep_time.tv_usec = usecs;
      sleep_time.tv_sec  = secs;

      if (select(0, NULL, NULL, NULL, &sleep_time) < 0)
        continue;
    }

    /* reset the last time we where sleeping */
    gettimeofday(&last_time, NULL);

    /* recycle sockets */
    recycle_sockets();
  }
}

/*
 * Init_socket()
 *
 * Used at bootup to get a free
 * socket to run the server from.
 */
int init_socket()
{
  struct sockaddr_in my_addr;
  int sockfd, reuse = 1;

  /* let's grab a socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  /* setting the correct values */
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = INADDR_ANY;
  my_addr.sin_port = htons(MUDPORT);

  /* this actually fixes any problems with threads */
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
  {
    perror("Error in setsockopt()");
    exit(1);
  } 

  /* bind the port */
  bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));

  /* start listening already :) */
  listen(sockfd, 3);

  /* return the socket */
  return sockfd;
}

/* 
 * New_socket()
 *
 * Initializes a new socket, get's the hostname
 * and puts it in the active socket_list.
 */
bool new_socket(int sock)
{
  struct sockaddr_in   sock_addr;
  pthread_attr_t       attr;
  pthread_t            thread_lookup;
  LOOKUP_DATA        * lData;
  D_SOCKET           * sock_new;
  int                  argp = 1;
  socklen_t            size;

  /* initialize threads */
  pthread_attr_init(&attr);   
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  /*
   * allocate some memory for a new socket if
   * there is no free socket in the free_list
   */
   CREATE( sock_new, D_SOCKET, 1 );

   /* allocate a stack for using takeover type commands */

  /* attach the new connection to the socket LLIST*/
  FD_SET(sock, &fSet);

  /* clear out the socket */
  clear_socket(sock_new, sock);

  /* set the socket as non-blocking */
  ioctl(sock, FIONBIO, &argp);

  /* update the linked LLISTof sockets */
  AttachToList(sock_new, dsock_list);

  /* do a host lookup */
  size = sizeof(sock_addr);
  if (getpeername(sock, (struct sockaddr *) &sock_addr, &size) < 0)
  {
    perror("New_socket: getpeername");
    sock_new->hostname = strdup("unknown");
  }
  else
  {
    /* set the IP number as the temporary hostname */
    sock_new->hostname = strdup(inet_ntoa(sock_addr.sin_addr));

    if (strcasecmp(sock_new->hostname, "127.0.0.1"))
    {
      /* allocate some memory for the lookup data */
      if ((lData = malloc(sizeof(*lData))) == NULL)
      {
        bug("New_socket: Cannot allocate memory for lookup data.");
        abort();
      }

      /* Set the lookup_data for use in lookup_address() */
      lData->buf    =  strdup((char *) &sock_addr.sin_addr);
      lData->dsock  =  sock_new;

      /* dispatch the lookup thread */
      pthread_create(&thread_lookup, &attr, &lookup_address, (void*) lData);
    }
    else sock_new->lookup_status++;
  }

  /* negotiate compression */
  text_to_buffer(sock_new, (char *) compress_will2);
  text_to_buffer(sock_new, (char *) compress_will);

  /* send the greeting */
  text_to_buffer(sock_new, greeting);

  /* initialize socket events */
  init_events_socket(sock_new);

   sock_new->prev_control_stack = AllocStack();

  {
     NANNY_DATA *nanny = init_nanny();
     nanny->info = &nanny_lib[NANNY_LOGIN];
     control_nanny( sock_new, nanny );
     change_nanny_state( nanny, 0, TRUE );
     change_socket_state( sock_new, STATE_NANNY );
  }


  /* everything went as it was supposed to */
  return TRUE;
}

/*
 * Close_socket()
 *
 * Will close one socket directly, freeing all
 * resources and making the socket availably on
 * the socket free_list.
 */
void close_socket(D_SOCKET *dsock, bool reconnect)
{
  EVENT_DATA *pEvent;
  ITERATOR Iter;

  if (dsock->lookup_status > TSTATE_DONE) return;
  dsock->lookup_status += 2;

  /* remove the socket from the polling LLIST*/
  FD_CLR(dsock->control, &fSet);

  if (dsock->state == STATE_PLAYING)
  {
    if (reconnect)
      text_to_socket(dsock, "This connection has been taken over.\n\r");
  }

   if( dsock->nanny )
      free_nanny( dsock->nanny );
   dsock->nanny = NULL;

   if( !reconnect )
   {
      if( dsock && dsock->account )
      {
         dsock->account->socket = NULL;
         dsock->account = NULL;
      }
   }

   if( !reconnect )
      dsock->controlling = NULL;

  /* dequeue all events for this socket */
  AttachIterator(&Iter, dsock->events);
  while ((pEvent = (EVENT_DATA *) NextInList(&Iter)) != NULL)
    dequeue_event(pEvent);
  DetachIterator(&Iter);

  while( StackSize( dsock->prev_control_stack ) > 0 )
     PopStack( dsock->prev_control_stack );

  /* set the closed state */
  dsock->state = STATE_CLOSED;

   recycle_sockets();
}

/* 
 * Read_from_socket()
 *
 * Reads one line from the socket, storing it
 * in a buffer for later use. Will also close
 * the socket if it tries a buffer overflow.
 */
bool read_from_socket(D_SOCKET *dsock)
{
  int size;
  extern int errno;

  /* check for buffer overflows, and drop connection in that case */
  size = strlen(dsock->inbuf);
  if (size >= sizeof(dsock->inbuf) - 2)
  {
    text_to_socket(dsock, "\n\r!!!! Input Overflow !!!!\n\r");
    return FALSE;
  }

  /* start reading from the socket */
  for (;;)
  {
    int sInput;
    int wanted = sizeof(dsock->inbuf) - 2 - size;

    sInput = read(dsock->control, dsock->inbuf + size, wanted);

    if (sInput > 0)
    {
      size += sInput;

      if (dsock->inbuf[size-1] == '\n' || dsock->inbuf[size-1] == '\r')
        break;
    }
    else if (sInput == 0)
    {
      log_string("Read_from_socket: EOF");
      return FALSE;
    }
    else if (errno == EAGAIN || sInput == wanted)
      break;
    else
    {
      perror("Read_from_socket");
      return FALSE;
    }     
  }
  dsock->inbuf[size] = '\0';
  return TRUE;
}

/*
 * Text_to_socket()
 *
 * Sends text directly to the socket,
 * will compress the data if needed.
 */
bool text_to_socket(D_SOCKET *dsock, const char *txt)
{
  int iBlck, iPtr, iWrt = 0, length, control = dsock->control;

  length = strlen(txt);

  /* write compressed */
  if (dsock && dsock->out_compress)
  {
    dsock->out_compress->next_in  = (unsigned char *) txt;
    dsock->out_compress->avail_in = length;

    while (dsock->out_compress->avail_in)
    {
      dsock->out_compress->avail_out = COMPRESS_BUF_SIZE - (dsock->out_compress->next_out - dsock->out_compress_buf);

      if (dsock->out_compress->avail_out)
      {
        int status = deflate(dsock->out_compress, Z_SYNC_FLUSH);

        if (status != Z_OK)
        return FALSE;
      }

      length = dsock->out_compress->next_out - dsock->out_compress_buf;
      if (length > 0)
      {
        for (iPtr = 0; iPtr < length; iPtr += iWrt)
        {
          iBlck = UMIN(length - iPtr, 4096);
          if ((iWrt = write(control, dsock->out_compress_buf + iPtr, iBlck)) < 0)
          {
            perror("Text_to_socket (compressed):");
            return FALSE;
          }
        }
        if (iWrt <= 0) break;
        if (iPtr > 0)
        {
          if (iPtr < length)
            memmove(dsock->out_compress_buf, dsock->out_compress_buf + iPtr, length - iPtr);

          dsock->out_compress->next_out = dsock->out_compress_buf + length - iPtr;
        }
      }
    }
    return TRUE;
  }

  /* write uncompressed */
  for (iPtr = 0; iPtr < length; iPtr += iWrt)
  {
    iBlck = UMIN(length - iPtr, 4096);
    if ((iWrt = write(control, txt + iPtr, iBlck)) < 0)
    {
      perror("Text_to_socket:");
      return FALSE;
    }
  }

  return TRUE;
}

/*
 * Text_to_buffer()
 *
 * Stores outbound text in a buffer, where it will
 * stay untill it is flushed in the gameloop.
 *
 * Will also parse ANSI colors and other tags.
 */
void text_to_buffer(D_SOCKET *dsock, const char *txt)
{
  static char output[MAX_OUTPUT];
  bool underline = FALSE, bold = FALSE;
  int iPtr = 0, last = -1, j, k;
  int length = strlen(txt);

   if( !dsock )
      return;

   if( dsock->account )
      txt = handle_pagewidth( dsock->account->pagewidth, txt );

  /* the color struct */
  struct sAnsiColor
  {
    const char    cTag;
    const char  * cString;
    int           aFlag;
  };

  /* the color table... */
  const struct sAnsiColor ansiTable[] =
  {
    { 'd',  "30",  eTHIN },
    { 'D',  "30",  eBOLD },
    { 'r',  "31",  eTHIN },
    { 'R',  "31",  eBOLD },
    { 'g',  "32",  eTHIN },
    { 'G',  "32",  eBOLD },
    { 'y',  "33",  eTHIN },
    { 'Y',  "33",  eBOLD },
    { 'b',  "34",  eTHIN },
    { 'B',  "34",  eBOLD },
    { 'p',  "35",  eTHIN },
    { 'P',  "35",  eBOLD },
    { 'c',  "36",  eTHIN },
    { 'C',  "36",  eBOLD },
    { 'w',  "37",  eTHIN },
    { 'W',  "37",  eBOLD },

    /* the end tag */
    { '\0',  "",   eTHIN }
  };

  if (length >= ( MAX_BUFFER * 6 ) - 1 )
  {
    log_string("text_to_buffer: buffer overflow.");
    return;
  }

  /* always start with a leading space */
  if (dsock->top_output == 0)
  {
    dsock->outbuf[0] = '\n';
    dsock->outbuf[1] = '\r';
    dsock->top_output = 2;
  }

  while (*txt != '\0')
  {
    /* simple bound checking */
    if (iPtr > (8 * MAX_BUFFER - 15))
      break;

    switch(*txt)
    {
      default:
        output[iPtr++] = *txt++;
        break;
      case '#':
        txt++;

        /* toggle underline on/off with #u */
        if (*txt == 'u')
        {
          txt++;
          if (underline)
          {
            underline = FALSE;
            output[iPtr++] =  27; output[iPtr++] = '['; output[iPtr++] = '0';
            if (bold)
            {
              output[iPtr++] = ';'; output[iPtr++] = '1';
            }
            if (last != -1)
            {
              output[iPtr++] = ';';
              for (j = 0; ansiTable[last].cString[j] != '\0'; j++)
              {
                output[iPtr++] = ansiTable[last].cString[j];
              }
            }
            output[iPtr++] = 'm';
          }
          else
          {
            underline = TRUE;
            output[iPtr++] =  27; output[iPtr++] = '[';
            output[iPtr++] = '4'; output[iPtr++] = 'm';
          }
        }

        /* parse ## to # */
        else if (*txt == '#')
        {
          txt++;
          output[iPtr++] = '#';
        }

        /* #n should clear all tags */
        else if (*txt == 'n')
        {
          txt++;
          if (last != -1 || underline || bold)
          {  
            underline = FALSE;
            bold = FALSE;
            output[iPtr++] =  27; output[iPtr++] = '[';
            output[iPtr++] = '0'; output[iPtr++] = 'm';
          }

          last = -1;
        }

        /* check for valid color tag and parse */
        else
        {
          bool validTag = FALSE;

          for (j = 0; ansiTable[j].cString[0] != '\0'; j++)
          {
            if (*txt == ansiTable[j].cTag)
            {
              validTag = TRUE;

              /* we only add the color sequence if it's needed */
              if (last != j)
              {
                bool cSequence = FALSE;

                /* escape sequence */
                output[iPtr++] = 27; output[iPtr++] = '[';

                /* remember if a color change is needed */
                if (last == -1 || last / 2 != j / 2)
                  cSequence = TRUE;

                /* handle font boldness */
                if (bold && ansiTable[j].aFlag == eTHIN)
                {
                  output[iPtr++] = '0';
                  bold = FALSE;

                  if (underline)
                  {
                    output[iPtr++] = ';'; output[iPtr++] = '4';
                  }

                  /* changing to eTHIN wipes the old color */
                  output[iPtr++] = ';';
                  cSequence = TRUE;
                }
                else if (!bold && ansiTable[j].aFlag == eBOLD)
                {
                  output[iPtr++] = '1';
                  bold = TRUE;

                  if (cSequence)
                    output[iPtr++] = ';';
                }

                /* add color sequence if needed */
                if (cSequence)
                {
                  for (k = 0; ansiTable[j].cString[k] != '\0'; k++)
                  {
                    output[iPtr++] = ansiTable[j].cString[k];
                  }
                }

                output[iPtr++] = 'm';
              }

              /* remember the last color */
              last = j;
            }
          }

          /* it wasn't a valid color tag */
          if (!validTag)
            output[iPtr++] = '#';
          else
            txt++;
        }
        break;
    }
  }

  /* and terminate it with the standard color */
  if (last != -1 || underline || bold)
  {
    output[iPtr++] =  27; output[iPtr++] = '[';
    output[iPtr++] = '0'; output[iPtr++] = 'm';
  }
  output[iPtr] = '\0';

  /* check to see if the socket can accept that much data */
  if (dsock->top_output + iPtr >= MAX_OUTPUT)
  {
    bug("Text_to_buffer: ouput overflow on %s.", dsock->hostname);
    return;
  }

  /* add data to buffer */
  strcpy(dsock->outbuf + dsock->top_output, output);
  dsock->top_output += iPtr;
}

void next_cmd_from_buffer(D_SOCKET *dsock)
{
  int size = 0, i = 0, j = 0, telopt = 0;

  /* if theres already a command ready, we return */
  if (dsock->next_command[0] != '\0')
    return;

  /* if there is nothing pending, then return */
  if (dsock->inbuf[0] == '\0')
    return;

  /* check how long the next command is */
  while (dsock->inbuf[size] != '\0' && dsock->inbuf[size] != '\n' && dsock->inbuf[size] != '\r')
    size++;

  /* we only deal with real commands */
  if (dsock->inbuf[size] == '\0')
    return;

  /* copy the next command into next_command */
  for ( ; i < size; i++)
  {
    if (dsock->inbuf[i] == (signed char) IAC)
    {
      telopt = 1;
    }
    else if (telopt == 1 && (dsock->inbuf[i] == (signed char) DO || dsock->inbuf[i] == (signed char) DONT))
    {
      telopt = 2;
    }
    else if (telopt == 2)
    {
      telopt = 0;

      if (dsock->inbuf[i] == (signed char) TELOPT_COMPRESS)         /* check for version 1 */
      {
        if (dsock->inbuf[i-1] == (signed char) DO)                  /* start compressing   */
          compressStart(dsock, TELOPT_COMPRESS);
        else if (dsock->inbuf[i-1] == (signed char) DONT)           /* stop compressing    */
          compressEnd(dsock, TELOPT_COMPRESS, FALSE);
      }
      else if (dsock->inbuf[i] == (signed char) TELOPT_COMPRESS2)   /* check for version 2 */
      {
        if (dsock->inbuf[i-1] == (signed char) DO)                  /* start compressing   */
          compressStart(dsock, TELOPT_COMPRESS2);
        else if (dsock->inbuf[i-1] == (signed char) DONT)           /* stop compressing    */
          compressEnd(dsock, TELOPT_COMPRESS2, FALSE);
      }
    }
    else if (isprint(dsock->inbuf[i]) && isascii(dsock->inbuf[i]))
    {
      dsock->next_command[j++] = dsock->inbuf[i];
    }
  }
  dsock->next_command[j] = '\0';

  /* skip forward to the next line */
  while ( ( dsock->inbuf[size] == '\n' || dsock->inbuf[size] == '\r' ) )
  {
    dsock->bust_prompt = NORMAL_PROMPT;   /* seems like a good place to check */
    size++;
  }

  /* use i as a static pointer */
  i = size;

  /* move the context of inbuf down */
  while (dsock->inbuf[size] != '\0')
  {
    dsock->inbuf[size - i] = dsock->inbuf[size];
    size++;
  }
  dsock->inbuf[size - i] = '\0';
}

bool flush_output(D_SOCKET *dsock)
{
  /* nothing to send */
  if (dsock->top_output <= 0 && !(dsock->bust_prompt && dsock->state == STATE_PLAYING))
    return TRUE;

  /* bust a prompt */
  if( dsock->bust_prompt > NO_PROMPT )
  {
     switch( dsock->state )
     {
        default:
        text_to_buffer(dsock, "\r\nPrompt Error...\r\n");
        break;
        case STATE_ACCOUNT:
           account_prompt( dsock );
           break;
        case STATE_OLC:
           olc_prompt( dsock, TRUE );
           break;
        case STATE_EFRAME_EDITOR:
           editor_eFramework_prompt( dsock, FALSE );
           break;
        case STATE_PROJECT_EDITOR:
           editor_project_prompt( dsock, TRUE );
           break;
        case STATE_WORKSPACE_EDITOR:
           editor_workspace_prompt( dsock, TRUE );
           break;
        case STATE_EINSTANCE_EDITOR:
           editor_instance_prompt( dsock, TRUE );
           break;
        case STATE_BUILDER:
           builder_prompt( dsock );
           break;
        case STATE_PLAYING:
           player_prompt( dsock );
           break;
        case STATE_SFRAME_EDITOR:
           editor_sFramework_prompt( dsock, TRUE );
           break;
        case STATE_NANNY:
           break;
     }
    dsock->bust_prompt = FALSE;
  }

  /* reset the top pointer */
  dsock->top_output = 0;

  /*
   * Send the buffer, and return FALSE
   * if the write fails.
   */
  if (!text_to_socket(dsock, dsock->outbuf))
    return FALSE;

  /* Success */
  return TRUE;
}

void clear_socket(D_SOCKET *sock_new, int sock)
{
  memset(sock_new, 0, sizeof(*sock_new));

  sock_new->control        =  sock;
  sock_new->state          =  -1;
  sock_new->lookup_status  =  TSTATE_LOOKUP;
  sock_new->top_output     =  0;
  sock_new->events         =  AllocList();
}

/* does the lookup, changes the hostname, and dies */
void *lookup_address(void *arg)
{
  LOOKUP_DATA *lData = (LOOKUP_DATA *) arg;
  struct hostent *from = 0;
  struct hostent ent;
  char buf[16384];
  int err;

  /* do the lookup and store the result at &from */
  gethostbyaddr_r(lData->buf, sizeof(lData->buf), AF_INET, &ent, buf, 16384, &from, &err);

  /* did we get anything ? */
  if (from && from->h_name)
  {
    FREE(lData->dsock->hostname);
    lData->dsock->hostname = strdup(from->h_name);
  }

  /* set it ready to be closed or used */
  lData->dsock->lookup_status++;

  /* free the lookup data */
  FREE(lData->buf);
  FREE(lData);

  /* and kill the thread */
  pthread_exit(0);
}

void recycle_sockets()
{
  D_SOCKET *dsock;
  ITERATOR Iter;

  AttachIterator(&Iter, dsock_list);
  while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
  {
    if (dsock->lookup_status != TSTATE_CLOSED) continue;

    /* remove the socket from the socket LLIST*/
    DetachFromList(dsock, dsock_list);

    /* close the socket */
    close(dsock->control);

    /* free the memory */
    FREE(dsock->hostname);

    /* free the LLISTof events */
    FreeList(dsock->events);

    /* stop compression */
    compressEnd(dsock, dsock->compressing, TRUE);

    FREE(dsock);
  }
  DetachIterator(&Iter);
}

int change_socket_state( D_SOCKET *dsock, int state )
{
   int ret = RET_SUCCESS;

   if( dsock == NULL )
   {
      BAD_POINTER( "dsock" );
      return ret;
   }

   dsock->prev_state = dsock->state;
   dsock->state = state;
   if( dsock->account )
      dsock->account->sock_state = state;
   switch( state )
   {
      default:
         bug( "%s: BAD STATE %d.", __FUNCTION__, state );
         return RET_FAILED_OTHER;
      case STATE_NANNY:
         break;
      case STATE_ACCOUNT:
         if( SizeOfList( dsock->account->commands ) < 1 )
         {
            load_commands( dsock->account->commands, account_commands, dsock->account->level );
            load_lua_commands( dsock->account->commands, STATE_ACCOUNT, dsock->account->level );
         }
         break;
      case STATE_OLC:
         if( SizeOfList( dsock->account->olc->commands ) < 1 )
            load_commands( dsock->account->olc->commands, olc_commands, dsock->account->level );
         break;
      case STATE_EFRAME_EDITOR:
         if( SizeOfList( dsock->account->olc->editor_commands ) < 1 )
           load_commands( dsock->account->olc->editor_commands, create_eFramework_commands, dsock->account->level );
         break;
      case STATE_PROJECT_EDITOR:
         if( SizeOfList( dsock->account->olc->editor_commands ) < 1 )
            load_commands( dsock->account->olc->editor_commands, create_project_commands, dsock->account->level );
         break;
      case STATE_WORKSPACE_EDITOR:
         if( SizeOfList( dsock->account->olc->editor_commands ) < 1 )
            load_commands( dsock->account->olc->editor_commands, create_workspace_commands, dsock->account->level );
         break;
      case STATE_EINSTANCE_EDITOR:
         if( SizeOfList( dsock->account->olc->editor_commands ) < 1 )
            load_commands( dsock->account->olc->editor_commands, create_instance_commands, dsock->account->level );
         break;
      case STATE_BUILDER:
         if( SizeOfList( dsock->controlling->commands ) < 1 )
            load_commands( dsock->controlling->commands, builder_commands, dsock->controlling->level );
         break;
      case STATE_SFRAME_EDITOR:
         if( SizeOfList( dsock->account->olc->editor_commands ) < 1 )
            load_commands( dsock->account->olc->editor_commands, create_sFramework_commands, dsock->account->level );
         break;
      case STATE_PLAYING:
         if( SizeOfList( dsock->controlling->commands ) < 1 )
         {
            load_commands( dsock->controlling->commands, mobile_commands, dsock->controlling->level );
            load_lua_commands( dsock->controlling->commands, STATE_PLAYING, dsock->controlling->level );
         }
         break;
   }
   return ret;
}

void socket_control_entity( D_SOCKET *socket, ENTITY_INSTANCE *entity )
{
   if( socket->controlling )
      socket_uncontrol_entity( socket->controlling );
   socket->controlling = entity;
   entity->socket = socket;

   return;
}

void socket_uncontrol_entity( ENTITY_INSTANCE *entity )
{
   D_SOCKET *socket;
   if( ( socket = entity->socket ) == NULL )
      return;

   socket->controlling = NULL;
   entity->socket = NULL;
   return;
}

void get_info( D_SOCKET *socket )
{
   switch( socket->state )
   {
      default:
         text_to_buffer( socket, "No information for this state.\r\n" );
         return;
      case STATE_OLC:
         text_to_buffer( socket, "Almost...\r\n" );
         return;
      case STATE_EFRAME_EDITOR:
         editor_eFramework_info( socket );
         return;
      case STATE_EINSTANCE_EDITOR:
         editor_instance_info( socket );
         return;
   }
}

void get_commands( D_SOCKET *socket )
{
   BUFFER *buf = buffer_new( MAX_BUFFER );
   int space = socket->account ? socket->account->pagewidth - 2 : DEFAULT_PAGEWIDTH - 2;

   bprintf( buf, "/%s\\\r\n", print_header( "Commands", "-", space ) );
   switch( socket->state )
   {
      case STATE_NANNY:
         bprintf( buf, "%s\r\n", fit_string_to_space( "In a nanny, there are no commands only interpreted input." , space ) );
         break;
      case STATE_ACCOUNT:
         print_commands( socket->account, socket->account->commands, buf, 0, socket->account->pagewidth );
         break;
      case STATE_OLC:
         print_commands( socket->account->olc, socket->account->olc->commands, buf, 0, socket->account->pagewidth );
         break;
      case STATE_EFRAME_EDITOR:
      case STATE_EINSTANCE_EDITOR:
      case STATE_WORKSPACE_EDITOR:
      case STATE_SFRAME_EDITOR:
      case STATE_PROJECT_EDITOR:
         print_commands( socket->account->olc, socket->account->olc->editor_commands, buf, 0, socket->account->pagewidth );
         break;
      case STATE_BUILDER:
      case STATE_PLAYING:
         print_commands( socket->controlling, socket->controlling->commands, buf, 0, socket->account->pagewidth );
         break;
   }
   bprintf( buf, "\\%s/\r\n", print_bar( "-", space ) );
   text_to_buffer( socket, buf->data );
}
