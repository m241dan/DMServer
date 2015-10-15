/* interpret.h written by Davenge */

extern SEL_TYPING input_selection_typing;
extern void *input_selection_ptr;

extern struct typCmd account_commands[];
const char *settings_desc( void *extra );
const char *chat_desc( void *extra );

extern struct typCmd settings_sub_commands[];
const char *pagewidth_desc( void *extra );
const char *chatas_desc( void *extra );

extern struct typCmd olc_commands[];

extern struct typCmd file_sub_commands[];

extern struct typCmd workspace_sub_commands[];

extern struct typCmd frameworks_sub_commands[];

extern struct typCmd create_eFramework_commands[];

extern struct typCmd create_project_commands[];

extern struct typCmd create_workspace_commands[];

extern struct typCmd create_instance_commands[];

const char *editor_return_desc( void *extra );

extern struct typCmd create_sFramework_commands[];

extern struct typCmd builder_commands[];

extern struct typCmd mobile_commands[];

int account_handle_cmd( ACCOUNT_DATA *account, char *arg );
int olc_handle_cmd( INCEPTION *olc, char *arg );
int eFrame_editor_handle_command( INCEPTION *olc, char *arg );
int project_editor_handle_command( INCEPTION *olc, char *arg );
int workspace_editor_handle_command( INCEPTION *olc, char *arg );
int instance_editor_handle_command( INCEPTION *olc, char *arg );
int sFrame_editor_handle_command( INCEPTION *olc, char *arg );
int entity_handle_cmd( ENTITY_INSTANCE *instance, char *arg );


void execute_command( ACCOUNT_DATA *account, COMMAND *com, void *passed, char *arg );
void execute_lua_command( ACCOUNT_DATA *account, COMMAND *com, void *passed, char *arg );
COMMAND *find_loaded_command( LLIST *loaded_list, const char *command );
int load_commands( LLIST *command_list, COMMAND command_table[], int level_compare );
void load_lua_commands( LLIST *command_list, int state, int level_compare );
int copy_command( COMMAND *to_copy, COMMAND *command );
int free_command_list( LLIST *com_list );
int free_command( COMMAND *command );

bool interpret_entity_selection( const char *input );
SEL_TYPING check_selection_type( const char *input );
const char *check_selection_type_string( const char *input );
void *retrieve_entity_selection( void );
bool input_format_is_selection_type( const char *input );
void clear_entity_selection( void );
