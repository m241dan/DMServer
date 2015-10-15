/* methods pertaining to nannys written by Davege */

#include "mud.h"

/*****************
 * NANNY LIBRARY *
 *****************/

const struct nanny_lib_entry nanny_lib[] = {
   { "login", nanny_login_messages, nanny_login_code, FALSE },
   { "new account", nanny_new_account_messages, nanny_new_account_code, FALSE },
   { NULL, NULL, NULL, FALSE } /* gandalf */
};

/***************
 * NANNY LOGIN *
 ***************/
const char *const nanny_login_messages[] = {
   "What's your account name? ", "Password: ",
   NULL /* gandalf */
};

nanny_fun *const nanny_login_code[] = {
   nanny_login, nanny_password,
   NULL /* gandalf */
};

int nanny_login( NANNY_DATA *nanny, char *arg )
{
   int ret = RET_SUCCESS;
   ACCOUNT_DATA *a_new = init_account();

   if( ( ret = load_account( a_new, arg ) ) == RET_DB_NO_ENTRY )
   {
      if( !check_name( arg ) )
      {
         text_to_nanny( nanny, "%s is a bad name, please try again: ", arg );
         return ret;
      }

      FREE( a_new->name );
      a_new->name = strdup( arg );

      if( set_nanny_lib_from_name( nanny, "new account" ) != RET_SUCCESS )
      {
         bug( "%s: 'new account' nanny lib missing.", __FUNCTION__ );
         text_to_nanny( nanny, "Something is really messed up." );
         free_account( a_new );
         close_socket( nanny->socket, FALSE );
         return RET_FAILED_OTHER;
      }
      change_nanny_state( nanny, 0, TRUE );
   }
   else if( ret == RET_SUCCESS )
      nanny_state_next( nanny, TRUE );
   else if( ret == RET_FAILED_OTHER )
   {
      text_to_nanny( nanny, "There's been a major error." );
      free_account( a_new );
      close_socket( nanny->socket, FALSE );
      return ret;
   }

   nanny->content = a_new;
   nanny->socket->account = a_new;
   a_new->socket = nanny->socket;
   text_to_nanny( nanny, (char *) dont_echo );
   return ret;
}

int nanny_password( NANNY_DATA *nanny, char *arg )
{
   D_SOCKET *socket = nanny->socket;
   ACCOUNT_DATA *account;
   int ret = RET_SUCCESS;

   if( !strcmp( crypt( arg, nanny->socket->account->name ), nanny->socket->account->password )  )
   {
      text_to_nanny( nanny, (char *) do_echo );

      if( ( account = check_account_reconnect( nanny->socket->account->name ) ) != NULL )
      {
         free_account( socket->account );
         socket->account = account;
         account->socket = nanny->socket;
         nanny->content = account;
         if( account->controlling )
            socket_control_entity( socket, account->controlling );
         log_string( "%s has reconnected.", account->name );
         text_to_nanny( nanny, "You take over your account already in use.\r\n" );
      }
      else
      {
         nanny->socket->account->sock_state = STATE_ACCOUNT;
         log_string( "Account: %s has logged in.", nanny->socket->account->name );
         AttachToList( nanny->socket->account, account_list );
      }

      change_socket_state( nanny->socket, nanny->socket->account->sock_state );
      strip_event_socket( nanny->socket, EVENT_SOCKET_IDLE );
      nanny->socket->nanny = NULL;
      free_nanny( nanny );
    }
    else
    {
       text_to_socket( nanny->socket, "Bad password!\r\n" );
       nanny_state_prev( nanny, TRUE );

    }
   return ret;
}

/*********************
 * NANNY NEW ACCOUNT *
 *********************/
const char *const nanny_new_account_messages[] = {
   "Please enter a password for your account: ", "Repeat the Password: ",
   NULL /* gandalf */
};

nanny_fun *nanny_new_account_code[] = {
   nanny_new_password, nanny_confirm_new_password,
   NULL /* gandalf */
};

int nanny_new_password( NANNY_DATA *nanny, char *arg )
{
   ACCOUNT_DATA *a_new = (ACCOUNT_DATA *)nanny->content;
   int ret = RET_SUCCESS;
   int i;

   if( strlen( arg ) < 5 || strlen( arg ) > 20 )
   {
      text_to_nanny( nanny, "Passwords should be between 5 and 20 characters please!\n\rPlease enter a new password: " );
      return ret;
   }

   if( !a_new )
   {
      puts( "a_new is NULL" );
      exit(1);
   }

   FREE( a_new->password );
   a_new->password = strdup( crypt( arg, a_new->name ) );

   for( i = 0; a_new->password[i] != '\0'; i++ )
   {
      if( a_new->password[i] == '~' )
      {
         text_to_nanny( nanny, "Illegal password!\n\rPlease enter a new password: " );
         return ret;
      }
   }
   nanny_state_next( nanny, TRUE );
   return ret;
}

int nanny_confirm_new_password( NANNY_DATA *nanny, char *arg )
{
   ACCOUNT_DATA *a_new = (ACCOUNT_DATA *)nanny->content;
   int ret = RET_SUCCESS;

   if( !strcmp( crypt( arg, a_new->name ), a_new->password ) )
   {
      text_to_nanny( nanny, (char *) do_echo );
      AttachToList( a_new, account_list );
      log_string( "A new account: %s has entered the game.", nanny->socket->account->name );

      if( ( ret = new_account( a_new ) ) != RET_SUCCESS )
      {
         bug( "%s: the return code is %d", __FUNCTION__, ret );
         text_to_socket( nanny->socket, "There's been a problem with the database.\r\n" );
         close_socket( nanny->socket, FALSE );
         return ret;
      }
      change_socket_state( nanny->socket, STATE_ACCOUNT );
      strip_event_socket( nanny->socket, EVENT_SOCKET_IDLE );
      nanny->socket->nanny = NULL;
      free_nanny( nanny );
   }
   else
   {
      text_to_nanny( nanny, "Password mismatch!\n\r" );
      nanny_state_prev( nanny, TRUE );
   }

   return ret;
}
/***********************
 * NANNY SPECIFIC CODE *
 ***********************/

/* creation */
NANNY_DATA *init_nanny( void )
{
   NANNY_DATA *nanny;

   CREATE( nanny, NANNY_DATA, 1 );
   clear_nanny( nanny );
   return nanny;
}

int clear_nanny( NANNY_DATA *nanny )
{
   int ret = RET_SUCCESS;

   nanny->socket = NULL;
   nanny->content = NULL;
   nanny->info = NULL;
   nanny->state = -1;

   return ret;
}

/* deletion */
int free_nanny( NANNY_DATA *nanny )
{
   int ret = RET_SUCCESS;

   clear_nanny( nanny );
   FREE( nanny );

   return ret;
}

/* input handling */
int handle_nanny_input( D_SOCKET *dsock, char *arg )
{
   NANNY_DATA *nanny;
   int ret = RET_SUCCESS;

   if( ( nanny = dsock->nanny ) == NULL)
   {
      BAD_POINTER( "nanny" );
      return ret;
   }

   if( !nanny->info && !nanny->lua_nanny )
   {
      BAD_POINTER( "info" );
      return ret;
   }

   if( nanny->state < 0 )
   {
      bug( "%s: BAD STATE %d.", __FUNCTION__, nanny->state );
      return RET_FAILED_OTHER;
   }

   if( nanny->lua_nanny )
   {
      int top = lua_gettop( lua_handle );

      prep_stack( nanny->path, "nannyInterp" );
      push_nanny( nanny, lua_handle );
      lua_pushstring( lua_handle, arg );
      if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
      {
         bug( "%s: ret: %d path: %s\r\n - error message %s.", __FUNCTION__, ret, nanny->path, lua_tostring( lua_handle, -1 ) );
         lua_settop( lua_handle, top );
         return RET_FAILED_OTHER;
      }
      return RET_SUCCESS;
   }

   if( !strcmp( arg, "/back" ) )
   {
      if( !nanny->info->back_allowed )
      {
         text_to_socket( dsock, "Going back is not allowed.\r\n" );
         return ret;
      }
      nanny_state_prev( nanny, TRUE );
      return ret;
   }
   (*nanny->info->nanny_code[nanny->state])( nanny, arg );
   return ret;

}

int change_nanny_state( NANNY_DATA *nanny, int state, bool message )
{
   int ret = RET_SUCCESS;
   nanny->state = state;
   if( message )
      text_to_buffer( nanny->socket, nanny->info->nanny_messages[nanny->state] );
   return ret;
}

int nanny_state_next( NANNY_DATA *nanny, bool message )
{
   int ret = RET_SUCCESS;
   nanny->state++;
   if( message )
      text_to_buffer( nanny->socket, nanny->info->nanny_messages[nanny->state] );
   return ret;
}

int nanny_state_prev( NANNY_DATA *nanny, bool message )
{
   int ret = RET_SUCCESS;
   nanny->state--;
   if( message )
      text_to_buffer( nanny->socket, nanny->info->nanny_messages[nanny->state] );
   return ret;
}

/* controlling */
int control_nanny( D_SOCKET *dsock, NANNY_DATA *nanny )
{
   int ret = RET_SUCCESS;

   if( dsock == NULL )
   {
      BAD_POINTER( "dsock" );
      return ret;
   }

   if( nanny == NULL )
   {
      BAD_POINTER( "nanny" );
      return ret;
   }
   dsock->nanny = nanny;
   nanny->socket = dsock;
   return ret;
}

int uncontrol_nanny( D_SOCKET *dsock )
{
   int ret = RET_SUCCESS;

   if( dsock == NULL )
   {
      BAD_POINTER( "dsock" );
      return ret;
   }
   if( dsock->nanny == NULL )
   {
      BAD_POINTER( "dsock->nanny" );
      return ret;
   }
   dsock->nanny->socket = NULL;
   dsock->nanny = NULL;
   return ret;
}

/* communication */
int text_to_nanny( NANNY_DATA *nanny, const char *fmt, ... )
{
   va_list va;
   int res;
   char dest[MAX_BUFFER];

   va_start( va, fmt );
   res = vsnprintf( dest, MAX_BUFFER, fmt, va );
   va_end( va );

   if( res >= MAX_BUFFER -1 )
   {
      dest[0] = '\0';
      bug( "Overflow when attempting to format string for message." );
   }

   text_to_buffer( nanny->socket, dest );
   return res;
}

int set_nanny_lib_from_name( NANNY_DATA *dest, const char *name )
{
   int x;

   for( x = 0; nanny_lib[x].name != NULL || nanny_lib[x].name[0] != '\0'; x++ )
   {
      if( !strcmp( nanny_lib[x].name, name ) )
      {
         dest->info = &nanny_lib[x];
         return RET_SUCCESS;
      }
   }

   bug( "%s: could not find library entry titled %s.", __FUNCTION__, name );
   return RET_FAILED_NO_LIB_ENTRY;
}

