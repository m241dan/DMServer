/*
 * This file handles command interpreting
 */
#include <sys/types.h>
#include <stdio.h>

/* include main header file */
#include "mud.h"

SEL_TYPING input_selection_typing = SEL_NULL;
void *input_selection_ptr = NULL;

/****************************************************************************
* ACCOUNT COMMAND TABLE                                                     *
****************************************************************************/
struct typCmd account_commands[] = {
   { "quit", account_quit, LEVEL_BASIC, NULL, FALSE, NULL, account_commands },
   { "inception", inception_open, LEVEL_BASIC, NULL, FALSE, NULL, account_commands },
   { "chat", account_chat, LEVEL_BASIC, NULL, FALSE, NULL, account_commands },
   { "settings", account_settings, LEVEL_BASIC, NULL, TRUE, NULL, account_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL } /* gandalf */
};

/******************************************************************************
* DESC_FUNC METHODS                                                           *
******************************************************************************/

const char *chat_desc( void *extra )
{
   return " - global chat";
}

const char * settings_desc( void *extra )
{
   return " - settings, pagewidth, chat stuff, etc";
}
/*****************************************************************************
* SETTINGS SUB COMMANDS TABLE                                                *
*****************************************************************************/

struct typCmd settings_sub_commands[] = {
   { "pagewidth", set_pagewidth, LEVEL_BASIC, NULL, FALSE, pagewidth_desc, settings_sub_commands },
   { "chat_as", account_chatas, LEVEL_BASIC, NULL, FALSE, chatas_desc, settings_sub_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

/******************************************************************************
* DESC_FUNC METHODS                                                           *
******************************************************************************/

const char *pagewidth_desc( void *extra )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)extra;
   static char buf[MAX_BUFFER];
   memset( &buf, 0, sizeof( buf ) );

   if( !extra )
   {
      bug( "%s: passed a bad account pointer.", __FUNCTION__ );
      return "";
   }

   mud_printf( buf, "#R: #B%d#n", account->pagewidth );
   return buf;
}

const char *chatas_desc( void *extra )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)extra;
   static char buf[MAX_BUFFER];
   memset( &buf, 0, sizeof( buf ) );

   if( !extra )
   {
     bug( "%s: passed a bad account pointer.", __FUNCTION__ );
     return "";
   }

   mud_printf( buf, ": %s", account->chatting_as[0] != ' ' ? account->chatting_as : "none" );
   return buf;
}

/*******************************************************************************
* INCEPTION OLC COMMAND TABLE                                                  *
* NAME, CMD_FUNC, LVL, SUB_COMMANDS, CAN_SUB, DESC_FUN, FROM_TABLE             *
*******************************************************************************/
struct typCmd olc_commands[] = {
   { "quit", olc_quit, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "chat", olc_chat, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "show", olc_show, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "builder", olc_builder, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "list", olc_list, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "load", olc_load, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "pak", olc_pak, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "instance", olc_instantiate, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "delete", olc_delete, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "iedit", framework_iedit, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "edit", olc_edit, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "screate", olc_screate, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "create", framework_create, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "ufilter", olc_ufilter, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "using", olc_using, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "workspace", olc_workspace, LEVEL_BASIC, NULL, TRUE, NULL, olc_commands },
   { "file", olc_file, LEVEL_BASIC, NULL, TRUE, NULL, olc_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

/*******************************************************************************
* FILE SUB COMMANDS TABLE                                                      *
* NAME, CMD_FUNC, LEVEL, SUB_COMMANDS, CAN_SUB, DESC_FUN, FROM_TABLE           *
*******************************************************************************/
struct typCmd file_sub_commands[] = {
   { "ImportProject", project_importProject, LEVEL_BASIC, NULL, FALSE, NULL, file_sub_commands },
   { "ExportProject", project_exportProject, LEVEL_BASIC, NULL, FALSE, NULL, file_sub_commands },
   { "OpenProject", project_openProject, LEVEL_BASIC, NULL, FALSE, NULL, file_sub_commands },
   { "NewProject", project_newProject, LEVEL_BASIC, NULL, FALSE, NULL, file_sub_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

/*******************************************************************************
* WORKSPACE SUB COMMANDS TABLE                                                 *
* NAME, CMD_FUNC, LEVEL, SUB_COMMANDS, CAN_SUB, DESC_FUN, FROM_TABLE           *
*******************************************************************************/
struct typCmd workspace_sub_commands[] = {
   { "ungrab", workspace_ungrab, LEVEL_BASIC, NULL, FALSE, NULL, workspace_sub_commands },
   { "grab", workspace_grab, LEVEL_BASIC, NULL, FALSE, NULL, workspace_sub_commands },
   { "wunload", workspace_unload, LEVEL_BASIC, NULL, FALSE, NULL, workspace_sub_commands },
   { "wload", workspace_load, LEVEL_BASIC, NULL, FALSE, NULL, workspace_sub_commands },
   { "wnew", workspace_new, LEVEL_BASIC, NULL, FALSE, NULL, workspace_sub_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

/*******************************************************************************
* FRAMEWORKS SUB COMMANDS TABLE                                                *
* NAME, CMD_FUNC, LEVEL, SUB_COMMANDS, CAN_SUB, DESC_FUN, FROM_TABLE           *
*******************************************************************************/
struct typCmd frameworks_sub_commands[] = {
   { "create", framework_create, LEVEL_BASIC, NULL, FALSE, NULL, frameworks_sub_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

struct typCmd create_eFramework_commands[] = {
   { "done", eFramework_done, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "save", eFramework_save, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "return", editor_global_return, LEVEL_BASIC, NULL, FALSE, editor_return_desc, create_eFramework_commands },
   { "switch", editor_switch, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "script", eFramework_script, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "addpak", eFramework_addPak, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "addcontent", eFramework_addContent, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "addstat", eFramework_addStat, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "remspec", eFramework_remSpec, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "addspec", eFramework_addSpec, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "setprimary", eFramework_setPrimaryDmg, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "setwidth", eFramework_set_width, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "setweight", eFramework_set_weight, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "setheight", eFramework_set_height, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "spawntime", eFramework_set_spawn_time, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "tspeed", eFramework_set_tspeed, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "desc", eFramework_description, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "long", eFramework_long, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "short", eFramework_short, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { "name", eFramework_name, LEVEL_BASIC, NULL, FALSE, NULL, create_eFramework_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

struct typCmd create_project_commands[] = {
   { "done", project_done, LEVEL_BASIC, NULL, FALSE, NULL, create_project_commands },
   { "return", editor_global_return, LEVEL_BASIC, NULL, FALSE, editor_return_desc, create_project_commands },
   { "switch", editor_switch, LEVEL_BASIC, NULL, FALSE, NULL, create_project_commands },
   { "public", project_public, LEVEL_BASIC, NULL, FALSE, NULL, create_project_commands },
   { "name", project_name, LEVEL_BASIC, NULL, FALSE, NULL, create_project_commands },
};

struct typCmd create_workspace_commands[] = {
   { "done", workspace_done, LEVEL_BASIC, NULL, FALSE, NULL, create_workspace_commands },
   { "return", editor_global_return, LEVEL_BASIC, NULL, FALSE, editor_return_desc, create_workspace_commands },
   { "switch", editor_switch, LEVEL_BASIC, NULL, FALSE, NULL, create_workspace_commands },
   { "public", workspace_public, LEVEL_BASIC, NULL, FALSE, NULL, create_workspace_commands },
   { "description", workspace_description, LEVEL_BASIC, NULL, FALSE, NULL, create_workspace_commands },
   { "name", workspace_name, LEVEL_BASIC, NULL, FALSE, NULL, create_workspace_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }

};

struct typCmd create_instance_commands[] = {
   { "done", instance_done, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "return", editor_global_return, LEVEL_BASIC, NULL, FALSE, editor_return_desc, create_instance_commands },
   { "switch", editor_switch, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "script", instance_script, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "restore", instance_restore, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "reinit", instance_reinit, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "autowrite", instance_autowrite, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "setstat", instance_setStat, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "remspec", instance_remspec, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "addspec", instance_addspec, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "addpak", instance_addPak, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "addcontent", instance_addcontent, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "level", instance_level, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "live", instance_live, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "load", instance_load, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

const char *editor_return_desc( void *extra )
{
   INCEPTION *olc = (INCEPTION *)extra;
   E_CHAIN *link;
   ITERATOR Iter;
   static char buf[MAX_BUFFER];

   if( SizeOfList( olc->chain ) <= 0 )
      return "";

   mud_printf( buf, " - " );

   AttachIterator( &Iter, olc->chain );
   link = (E_CHAIN *)NextInList( &Iter );
   DetachIterator( &Iter );

   switch( link->state )
   {
      default:
         bug( "%s: state = %d", __FUNCTION__, link->state );
         break;
      case STATE_EFRAME_EDITOR:
         strcat( buf, quick_format( "(%d)Framework: %s", ((ENTITY_FRAMEWORK *)link->to_edit)->tag->id, chase_name( (ENTITY_FRAMEWORK *)link->to_edit ) ) );
         break;
      case STATE_EINSTANCE_EDITOR:
         strcat( buf, quick_format( "(%d)Instance: %s", ((ENTITY_INSTANCE *)link->to_edit)->tag->id, instance_name( (ENTITY_INSTANCE *)link->to_edit ) ) );
         break;
      case STATE_PROJECT_EDITOR:
         strcat( buf, quick_format( "(%d)Project: %s", ((PROJECT *)link->to_edit)->tag->id, ((PROJECT *)link->to_edit)->name ) );
         break;
      case STATE_WORKSPACE_EDITOR:
         strcat( buf, quick_format( "(%d)Workspace: %s", ((WORKSPACE *)link->to_edit)->tag->id, ((WORKSPACE *)link->to_edit)->name ) );
         break;
      case STATE_SFRAME_EDITOR:
         strcat( buf, quick_format( "(%d)Stat: %s", ((STAT_FRAMEWORK *)link->to_edit)->tag->id, ((STAT_FRAMEWORK *)link->to_edit)->name ) );
   }

   buf[strlen( buf )] = '\0';
   return buf;
}

struct typCmd create_sFramework_commands[] = {
   { "done", sFramework_done, LEVEL_BASIC, NULL, FALSE, NULL, create_sFramework_commands },
   { "return", editor_global_return, LEVEL_BASIC, NULL, FALSE, editor_return_desc, create_instance_commands },
   { "switch", editor_switch, LEVEL_BASIC, NULL, FALSE, NULL, create_instance_commands },
   { "save", sFramework_save, LEVEL_BASIC, NULL, FALSE, NULL, create_sFramework_commands },
   { "script", sFramework_script, LEVEL_BASIC, NULL, FALSE, NULL, create_sFramework_commands },
   { "type", sFramework_type, LEVEL_BASIC, NULL, FALSE, NULL, create_sFramework_commands },
   { "hardfloor", sFramework_hardfloor, LEVEL_BASIC, NULL, FALSE, NULL, create_sFramework_commands },
   { "softfloor", sFramework_softfloor, LEVEL_BASIC, NULL, FALSE, NULL, create_sFramework_commands },
   { "hardcap", sFramework_hardcap, LEVEL_BASIC, NULL, FALSE, NULL, create_sFramework_commands },
   { "softcap", sFramework_softcap, LEVEL_BASIC, NULL, FALSE, NULL, create_sFramework_commands },
   { "name", sFramework_name, LEVEL_BASIC, NULL, FALSE, NULL, create_sFramework_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

struct typCmd builder_commands[] = {
   { "attack", mobile_attack, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "show", entity_show, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "target", entity_target, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "olc", entity_olc, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "using", entity_using, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "give", entity_give, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "put", entity_put, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "grab", entity_grab, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "chat", entity_chat, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "load", entity_load, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "iedit", entity_iedit, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "edit", entity_edit, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "create", entity_create, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "quit", entity_quit, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "get", entity_get, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "drop", entity_drop, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "inventory", entity_inventory, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "look", entity_look, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "instance", entity_instance, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "goto", entity_goto, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "down", entity_down, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "kill", mobile_kill, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "sethome", entity_set_home, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "restore", entity_restore, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "takeover", entity_takeover, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "north", entity_north, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "south", entity_south, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "east", entity_east, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "west", entity_west, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { "up", entity_up, LEVEL_BASIC, NULL, FALSE, NULL, builder_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

struct typCmd mobile_commands[] = {
   { "quit", player_quit, LEVEL_PLAYER, NULL, FALSE, NULL, mobile_commands },
   { "return", mobile_return, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "look", mobile_look, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "inventory", mobile_inventory, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "score", mobile_score, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "get", entity_get, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "drop", entity_drop, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "give", entity_give, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "put", entity_put, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "say", mobile_say, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "kill", mobile_kill, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "north", mobile_north, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "south", mobile_south, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "east", mobile_east, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "west", mobile_west, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "up", mobile_up, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { "down", mobile_down, LEVEL_BASIC, NULL, FALSE, NULL, mobile_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

int account_handle_cmd( ACCOUNT_DATA *account, char *arg )
{
   COMMAND *com;
   char command[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !account )
   {
      BAD_POINTER( "account" );
      return ret;
   }

   arg = one_arg( arg, command );

   if( ( com = find_loaded_command( account->commands, command ) ) == NULL )
      text_to_account( account, "No such command.\r\n" );
   else if( com->lua_cmd )
      execute_lua_command( account, com, account, arg );
   else
      execute_command( account, com, account, arg );

   return ret;
}

int olc_handle_cmd( INCEPTION *olc, char *arg )
{
   COMMAND *com;
   char command[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !olc )
   {
      BAD_POINTER( "olc" );
      return ret;
   }

   arg = one_arg( arg, command );

   if( ( com = find_loaded_command( olc->commands, command ) ) == NULL )
   {
      text_to_olc( olc, "No such command.\r\n" );
      olc_short_prompt( olc );
   }
   else
      execute_command( olc->account, com, olc, arg );

   return ret;
}

int eFrame_editor_handle_command( INCEPTION *olc, char *arg )
{
   COMMAND *com;
   char command[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !olc )
   {
      BAD_POINTER( "olc" );
      return ret;
   }

   arg = one_arg( arg, command );

   if( ( com = find_loaded_command( olc->editor_commands, command ) ) == NULL )
      text_to_olc( olc, "No such command.\r\n" );
   else
      execute_command( olc->account, com, olc, arg );

   return ret;
}

int project_editor_handle_command( INCEPTION *olc, char *arg )
{
   COMMAND *com;
   char command[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !olc )
   {
      BAD_POINTER( "olc" );
      return ret;
   }

   arg = one_arg( arg, command );

   if( ( com = find_loaded_command( olc->editor_commands, command ) ) == NULL )
      text_to_olc( olc, "No such command.\r\n" );
   else
      execute_command( olc->account, com, olc, arg );

   return ret;
}

int workspace_editor_handle_command( INCEPTION *olc, char *arg )
{
   COMMAND *com;
   char command[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !olc )
   {
      BAD_POINTER( "olc" );
      return ret;
   }

   arg = one_arg( arg, command );

   if( ( com = find_loaded_command( olc->editor_commands, command ) ) == NULL )
      text_to_olc( olc, "No such command.\r\n" );
   else
      execute_command( olc->account, com, olc, arg );

   return ret;
}

int instance_editor_handle_command( INCEPTION *olc, char *arg )
{
   COMMAND *com;
   char command[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !olc )
   {
      BAD_POINTER( "olc" );
      return ret;
   }

   arg = one_arg( arg, command );

   if( ( com = find_loaded_command( olc->editor_commands, command ) ) == NULL )
      text_to_olc( olc, "No such command.\r\n" );
   else
      execute_command( olc->account, com, olc, arg );

   return ret;
}

int sFrame_editor_handle_command( INCEPTION *olc, char *arg )
{
   COMMAND *com;
   char command[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !olc )
   {
      BAD_POINTER( "olc" );
      return ret;
   }

   arg = one_arg( arg, command );

   if( ( com = find_loaded_command( olc->editor_commands, command ) ) == NULL )
      text_to_olc( olc, "No such command.\r\n" );
   else
      execute_command( olc->account, com, olc, arg );

   return ret;
}


int entity_handle_cmd( ENTITY_INSTANCE *entity, char *arg )
{
   ACCOUNT_DATA *account;
   ENTITY_INSTANCE *exit;
   COMMAND *com;
   char command[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !entity )
   {
      BAD_POINTER( "entity" );
      return ret;
   }

   arg = one_arg( arg, command );

   if( ( com = find_loaded_command( entity->commands, command ) ) != NULL )
   {
      account = entity->socket ? entity->socket->account : NULL;

      if( com->lua_cmd )
         execute_lua_command( account, com, entity, arg );
      else
         execute_command( account, com, entity, arg );
   }
   else if( entity->contained_by && ( exit = instance_list_has_by_short_prefix( entity->contained_by->contents_sorted[SPEC_ISEXIT], command ) ) != NULL )
      move_entity( entity, exit );
   else
      text_to_entity( entity, "No such command or exit.\r\n" );

   return ret;
}

void execute_command( ACCOUNT_DATA *account, COMMAND *com, void *passed, char *arg )
{
   char *last_command;

   if( account )
   {
      account->executing_command = com;
      last_command = strdup( account->executing_command->cmd_name );
   }
   (*com->cmd_funct)( passed, arg );
   if( account )
   {
      FREE( account->last_command );
      account->last_command = strdup( last_command );
      account->executing_command = NULL;
   }
}

void execute_lua_command( ACCOUNT_DATA *account, COMMAND  *com, void *passed, char *arg )
{
   char *last_command;
   int ret;
   int state;

   if( account )
   {
      account->executing_command = com;
      last_command = strdup( account->executing_command->cmd_name );
      state = account->socket->state;
   }
   else
      state = STATE_PLAYING;

   prep_stack( com->path, "onCall" );
   switch( state )
   {
      default:
         FREE( account->last_command );
         account->last_command = last_command;
         account->executing_command = NULL;
         return;
      case STATE_PLAYING:
         push_instance( (ENTITY_INSTANCE *)passed, lua_handle );
         break;
      case STATE_ACCOUNT:
         push_account( (ACCOUNT_DATA *)passed, lua_handle );
         break;
   }

   while( isspace( *arg ) )
      arg++;
   if( !arg || arg[0] == '\0' )
      lua_pushnil( lua_handle );
   else
      lua_pushstring( lua_handle, arg );
   if( ( ret = lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) ) )
      bug( "%s: ret %d: path: %s\r\n - error message: %s.", __FUNCTION__, ret, com->path, lua_tostring( lua_handle, -1 ) );

   if( account )
   {
      FREE( account->last_command );
      account->last_command = last_command;
      account->executing_command = NULL;
   }

}

COMMAND *find_loaded_command( LLIST *loaded_list, const char *command )
{
   ITERATOR Iter;
   COMMAND *com;

   AttachIterator( &Iter, loaded_list );
   while( ( com = (COMMAND *)NextInList( &Iter ) ) != NULL )
   {
      if( is_prefix( command, com->cmd_name ) )
         break;
      if( com->sub_commands && ( com = find_loaded_command( com->sub_commands, command ) ) != NULL )
         break;
   }

   DetachIterator( &Iter );
   return com;
}

int load_commands( LLIST *command_list, COMMAND command_table[], int level_compare )
{
   int ret = RET_SUCCESS;
   int x;

   if( !command_list )
   {
      BAD_POINTER( "command_list" );
      return ret;
   }

   for( x = 0; command_table[x].cmd_name != '\0'; x++ )
   {
      if( command_table[x].level > level_compare )
         continue;
      else
      {
         COMMAND *com;
         CREATE( com, COMMAND, 1 );

         if( copy_command( &command_table[x], com ) != RET_SUCCESS )
            continue;

         AttachToList( com, command_list );
      }
   }
   return ret;
}

void load_lua_commands( LLIST *command_list, int state, int level_compare )
{
   COMMAND *command;
   int top = lua_gettop( lua_handle );
   int x, table_size, level;

   lua_getglobal( lua_handle, "command_table" );
   if( lua_isnil( lua_handle, -1 ) || lua_type( lua_handle, -1 ) != LUA_TTABLE )
   {
      bug( "%s: missing command table.", __FUNCTION__ );
      return;
   }
   lua_pushnumber( lua_handle, state );
   lua_gettable( lua_handle, -2 );
   if( lua_isnil( lua_handle, -1 ) || lua_type( lua_handle, -1 ) != LUA_TTABLE )
   {
      bug( "%s: missing command table at state %d.", __FUNCTION__, state );
      return;
   }

   lua_len( lua_handle, -1 );
   table_size = lua_tonumber( lua_handle, -1 );
   lua_pop( lua_handle, 1 );
   for( x = 1; x < (table_size+1); x++ )
   {
      /* get the command table */
      lua_pushnumber( lua_handle, x );
      lua_gettable( lua_handle, -2 );
      /* check level first */
      lua_pushnumber( lua_handle, 3 );
      lua_gettable( lua_handle, -2 );
      level = lua_tonumber( lua_handle, -1 );
      lua_pop( lua_handle, 1 );
      if( level > level_compare )
      {
         lua_pop( lua_handle, 1 );
         continue;
      }

      CREATE( command, COMMAND, 1 );
      command->level = level;
      command->lua_cmd = TRUE;
      command->can_sub = FALSE;
      /* name */
      lua_pushnumber( lua_handle, 1 );
      lua_gettable( lua_handle, -2 );
      command->cmd_name = strdup( lua_tostring( lua_handle, -1 ) );
      lua_pop( lua_handle, 1 );

      /* path */
      lua_pushnumber( lua_handle, 2 );
      lua_gettable( lua_handle, -2 );
      command->path = strdup( lua_tostring( lua_handle, -1 ) );
      lua_pop( lua_handle, 1 );

      /* remove the table for the specific command, not needed anymore. */
      lua_pop( lua_handle, 1 );
      AttachToList( command, command_list );
   }
   lua_settop( lua_handle, top );
   return;
}

int copy_command( COMMAND *to_copy, COMMAND *command )
{
   int ret = RET_SUCCESS;

   if( !to_copy )
   {
      BAD_POINTER( "to_copy");
      return ret;
   }

   if( !command )
   {
      BAD_POINTER( "command" );
      return ret;
   }

   command->cmd_name = to_copy->cmd_name;
   command->cmd_funct = to_copy->cmd_funct;
   command->sub_commands = NULL;
   command->can_sub = to_copy->can_sub;
   command->desc_func = to_copy->desc_func;
   command->from_table = to_copy->from_table;
   return ret;
}

int free_command_list( LLIST *com_list )
{
   ITERATOR Iter;
   COMMAND *com;
   int ret = RET_SUCCESS;

   if( !com_list )
   {
      BAD_POINTER( "com_list" );
      return ret;
   }

   AttachIterator( &Iter, com_list );
   while( ( com = (COMMAND *)NextInList( &Iter ) ) != NULL )
      free_command( com );
   DetachIterator( &Iter );

   return ret;
}

int free_command( COMMAND *command )
{
   int ret = RET_SUCCESS;
   command->cmd_name = NULL;
   command->cmd_funct = NULL;
   if( command->sub_commands )
   {
      free_command_list( command->sub_commands );
      FreeList( command->sub_commands );
      command->sub_commands = NULL;
   }
   command->from_table = NULL;
   FREE( command );
   return ret;
}

bool interpret_entity_selection( const char *input )
{
   static char err_msg[MAX_BUFFER];
   int id = 0;

   if( input_selection_typing != SEL_NULL )
   {
      bug( "%s: cannot interpret new selection until previous has been retrieved.", __FUNCTION__ );
      return FALSE;
   }
   if( check_selection_type( input ) == SEL_NULL )
   {
      input_selection_typing = SEL_STRING;
      input_selection_ptr = STD_SELECTION_ERRMSG;
      return TRUE;
   }

   if( input[1] == '_' && ( !(&input[2]) || input[2] != '\0' ) )
   {
      /* lookup by name */
      switch( tolower( input[0] ) )
      {
         case 'f':
            if( ( input_selection_ptr = get_framework_by_name( input+2 ) ) != NULL )
               input_selection_typing = SEL_FRAME;
            break;
         case 'i':
            if( ( input_selection_ptr = get_instance_by_name( input+2 ) ) != NULL )
               input_selection_typing = SEL_INSTANCE;
            break;
         case 'w':
            if( ( input_selection_ptr = get_workspace_by_name( input+2 ) ) != NULL )
               input_selection_typing = SEL_WORKSPACE;
            break;
         case 'p':
            if( ( input_selection_ptr = load_project_by_name( input+2 ) ) != NULL )
               input_selection_typing = SEL_PROJECT;
            break;
         case 's':
            if( ( input_selection_ptr = get_stat_framework_by_name( input + 2 ) ) != NULL )
               input_selection_typing = SEL_STAT_FRAMEWORK;
            break;
      }
   }
   else if( input[1] != '_' && is_number( input+1 ) )
   {
      id = atoi( input+1 );
      /* lookup by id */
      switch( tolower( input[0] ) )
      {
         case 'f':
            if( ( input_selection_ptr = get_framework_by_id( id ) ) != NULL )
               input_selection_typing = SEL_FRAME;
            break;
         case 'i':
            if( ( input_selection_ptr = get_instance_by_id( id ) ) != NULL )
               input_selection_typing = SEL_INSTANCE;
            break;
         case 'w':
            if( ( input_selection_ptr = get_workspace_by_id( id ) ) != NULL )
               input_selection_typing = SEL_WORKSPACE;
            break;
         case 'p':
            if( ( input_selection_ptr = load_project_by_id( id ) ) != NULL )
               input_selection_typing = SEL_PROJECT;
            break;
         case 's':
            if( ( input_selection_ptr = get_stat_framework_by_id( id ) ) != NULL )
               input_selection_typing = SEL_STAT_FRAMEWORK;
            break;
      }
   }
   else
   {
      input_selection_typing = SEL_STRING;
      input_selection_ptr = STD_SELECTION_ERRMSG;
      return TRUE;
   }

   if( !input_selection_ptr )
   {
      input_selection_typing = SEL_STRING;
      input_selection_ptr = err_msg;
      mud_printf( err_msg, "No such %s with the %s %s exists.\r\n", check_selection_type_string( input ),
                  input[1] == '_' ? "name" : "id", input[1] == '_' ? quick_format( "%s", input+2 ) : quick_format( "%d", id ) );
   }
   return TRUE;
}

SEL_TYPING check_selection_type( const char *input )
{
   /* check the format basics */

   if( !input_format_is_selection_type( input ) )
      return SEL_NULL;
   switch( tolower( input[0] ) )
   {
      default:  return SEL_NULL;
      case 'f': return SEL_FRAME;
      case 'i': return SEL_INSTANCE;
      case 'w': return SEL_WORKSPACE;
      case 'p': return SEL_PROJECT;
      case 's': return SEL_STAT_FRAMEWORK;
   }
   return SEL_NULL;
}
const char *check_selection_type_string( const char *input )
{
   if( !input_format_is_selection_type( input ) )
      return "null";
   switch( tolower( input[0] ) )
   {
      default:  return "null";
      case 'f': return "frame";
      case 'i': return "instance";
      case 'w': return "workspace";
      case 'p': return "project";
      case 's': return "stat frame";
   }
   return "null";
}

void *retrieve_entity_selection( void )
{
   void *tmp_ptr = input_selection_ptr;
   clear_entity_selection();
   return tmp_ptr;
}

bool input_format_is_selection_type( const char *input )
{
   if( !input || input[0] == '\0' || strlen( input ) < 2 )
      return FALSE;
   if( input[1] == '_' && ( strlen( input ) < 2 || input[2] == '\0' ) )
      return FALSE;
   if( input[1] != '_' && !is_number( input + 1 ) )
      return FALSE;
   return TRUE;
}

void clear_entity_selection( void )
{
   input_selection_typing = SEL_NULL;
   input_selection_ptr = NULL;
}

