/* the headerfile for lua_utils.c written by Davenge */

#define AUTOWRITE_INIT "   --- autowrite init ---"
#define ENDAUTOWRITE_INIT "   --- end autowrite init ---"
#define prep_stack( path, func ) prep_stack_handle( lua_handle, (path), (func) )


typedef struct lua_function
{
   char *noise;
   char *header;
   char *body;
   bool wrote;
} LUA_FUNCTION;

typedef struct lua_function ** LUA_FUNCTION_ARRAY;

int count_lua_functions( char *str );
LUA_FUNCTION_ARRAY get_functions( char *str );
LUA_FUNCTION *get_lua_func( char **str );
void free_lua_func( LUA_FUNCTION *func );
void free_lua_func_array( LUA_FUNCTION_ARRAY );
bool until_function( char *str );
bool until_end( char *str );

void free_lua_args( LLIST *list );

int luaopen_mud( lua_State *L );
bool prep_stack_handle( lua_State *handle, const char *file, const char *function );
const char *get_script_path_from_spec( SPECIFICATION *spec );
extern inline const char *get_frame_script_path( ENTITY_FRAMEWORK *frame );
extern inline const char *get_instance_script_path( ENTITY_INSTANCE *instance );
extern inline const char *get_stat_framework_script_path( STAT_FRAMEWORK *fstat );
extern inline const char *get_stat_instance_script_path( STAT_INSTANCE *stat );

extern inline void load_server_script( void );
extern inline void load_combat_vars_script( void );
extern inline void load_lua_command_tables( void );
extern inline void load_lua_misc_vars( void );
extern inline void load_lua_misc_funcs( void );
extern inline void load_lua_element_table( void );
void lua_server_settings( void );
void lua_database_settings( void );
void lua_combat_settings( void );
void lua_corpse_settings( void );

void push_instance( ENTITY_INSTANCE *instance, lua_State *L );
void push_framework( ENTITY_FRAMEWORK *frame, lua_State *L );
void push_specification( SPECIFICATION *spec, lua_State *L );
void push_damage( DAMAGE *dmg, lua_State *L );
void push_timer( TIMER *timer, lua_State *L );
void push_account( ACCOUNT_DATA *account, lua_State *L );
void push_nanny( NANNY_DATA *nanny, lua_State *L );
void push_socket( D_SOCKET *socket, lua_State *L );

void *check_meta( lua_State *L, int index, const char *meta_name );

int lua_bug( lua_State *L );
int lua_getGlobalVar( lua_State *L );
int lua_setGlobalVar( lua_State *L );
int global_luaCallBack( lua_State *L );
bool autowrite_init( ENTITY_INSTANCE *instance );
