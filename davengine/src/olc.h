/* olc.h written by Davenge */

struct inception_olc
{
   ACCOUNT_DATA *account;
   LLIST *commands;
   LLIST *editor_commands;
   LLIST *wSpaces;
   PROJECT *project;
   WORKSPACE *using_workspace;
   WORKSPACE_FILTER *using_filter;
   void *editing;
   int editing_state;
   int editor_launch_state;
   LLIST *chain;
   int builder_location;
};

struct workspace
{
   ID_TAG *tag;
   char *name;
   char *description;
   bool Public;

   LLIST *frameworks;
   LLIST *instances;
   LLIST *who_using;
};

struct workspace_filter
{
   GRAB_PARAMS *spec_filters;
   char **filter_name;
   int name_count;
   char **filter_short;
   int short_count;
   char **filter_long;
   int long_count;
   char **filter_desc;
   int desc_count;
   bool hide_frameworks;
   bool hide_instances;
   int limit;
};

struct grab_params
{
   bool no_exits;
   bool no_objects;
   bool no_rooms;
   bool no_mobiles;
};

struct editor_chain
{
   void *to_edit;
   int state;
};

INCEPTION *init_olc( void );
int free_olc( INCEPTION *olc );
int clear_olc( INCEPTION *olc );

WORKSPACE *init_workspace( void );
int free_workspace( WORKSPACE *wSpace );
int clear_workspace( WORKSPACE *wSpace );

WORKSPACE_FILTER *init_wfilter( void );
int free_wfilter( WORKSPACE_FILTER *filter );
WORKSPACE_FILTER *reset_wfilter( WORKSPACE_FILTER *filter );

E_CHAIN *make_editor_chain_link( void *editing, int state );
void free_editor_chain( LLIST *list );
void free_link( E_CHAIN *link );
void add_link_to_chain( E_CHAIN *link, LLIST *chain );

WORKSPACE *load_workspace_by_query( const char *query );
WORKSPACE *get_workspace_by_id( int id );
WORKSPACE *get_active_workspace_by_id( int id );
WORKSPACE *load_workspace_by_id( int id );
WORKSPACE *get_workspace_by_name( const char *name );
WORKSPACE *get_active_workspace_by_name( const char *name );
WORKSPACE *load_workspace_by_name( const char *name );
void db_load_workspace( WORKSPACE *wSpace, MYSQL_ROW *row );
void unuse_workspace( WORKSPACE *wSpace, ACCOUNT_DATA *account );

WORKSPACE *workspace_list_has_by_name( LLIST *workspace_list, const char *name );
WORKSPACE *workspace_list_has_by_id( LLIST *workspace_list, int id );

void inception_open( void *passed, char *arg );
int olc_prompt( D_SOCKET *dsock, bool commands );
int text_to_olc( INCEPTION *olc, const char *fmt, ... );
void olc_no_prompt( INCEPTION *olc );
void olc_short_prompt( INCEPTION *olc );
void olc_show_prompt( INCEPTION *olc );
int new_workspace( WORKSPACE *wSpace );
int add_frame_to_workspace( ENTITY_FRAMEWORK *frame, WORKSPACE *wSpace );
int add_instance_to_workspace( ENTITY_INSTANCE *instance, WORKSPACE *wSpace );
void rem_frame_from_workspace( ENTITY_FRAMEWORK *frame, WORKSPACE *wSpace );
void rem_instance_from_workspace( ENTITY_INSTANCE *instance, WORKSPACE *wSpace );
int add_workspace_to_olc( WORKSPACE *wSpace, INCEPTION *olc );
int new_workspace_entry( WORKSPACE *wSpace, ID_TAG *tag );
int load_workspace_entries( WORKSPACE *wSpace );
void switch_using( INCEPTION *olc, char *arg );

void grab_entity( INCEPTION *olc, char *arg, GRAB_PARAMS *params );
void grab_entity_range( INCEPTION *olc, char *arg );
GRAB_PARAMS grab_params( char *ranges, char *arg, char delim );
bool should_grab_instance( ENTITY_INSTANCE *instance, GRAB_PARAMS *params );
bool should_grab_framework( ENTITY_FRAMEWORK *framework, GRAB_PARAMS *params );
bool should_grab_from_specs( LLIST *specs, GRAB_PARAMS *params );
void reset_params( GRAB_PARAMS *params );

WORKSPACE *copy_workspace( WORKSPACE *wSpace, bool copy_instances, bool copy_frameworks );
LLIST *copy_workspace_list( LLIST *wSpaces, bool copy_instances, bool copy_frameworks );
void copy_workspaces_into_list( LLIST *wSpaces_list, LLIST *copy_into_list, bool copy_instances, bool copy_frameworks );

bool workspace_list_has_name( LLIST *wSpaces, const char *name );

void toggle_no_exit( INCEPTION *olc );
void toggle_no_objects( INCEPTION *olc );
void toggle_no_rooms( INCEPTION *olc );
void toggle_no_mobiles( INCEPTION *olc );
void set_limit_filter( INCEPTION *olc, char *arg );
void toggle_name_filter( INCEPTION *olc, char *arg );
void toggle_short_filter( INCEPTION *olc, char *arg );
void toggle_long_filter( INCEPTION *olc, char *arg );
void toggle_desc_filter( INCEPTION *olc, char *arg );
bool handle_string_filter( char ***filter_string, char *arg, int *count );
bool frame_filter_pass( ENTITY_FRAMEWORK *frame, WORKSPACE_FILTER *filter );
bool instance_filter_pass( ENTITY_INSTANCE *instance, WORKSPACE_FILTER *filter );
bool filter_string_check( const char *arg, char **arg_list, int max, bool precise );

void show_all_frameworks_to_olc( INCEPTION *olc );
void show_all_instances_to_olc( INCEPTION *olc );
void show_all_workspaces_to_olc( INCEPTION *olc );
void show_all_projects_to_olc( INCEPTION *olc );
void show_all_stats_to_olc( INCEPTION *olc );
void show_range_frameworks_to_olc( INCEPTION *olc, int start, int end );
void show_range_instances_to_olc( INCEPTION *olc, int start, int end );
void show_range_workspaces_to_olc( INCEPTION *olc, int start, int end );

void olc_file( void *passed, char *arg );
void olc_workspace( void *passed, char *arg );
void workspace_new( void *passed, char *arg );
void workspace_load( void *passed, char *arg );
void workspace_unload( void *passed, char *arg );
void workspace_grab( void *passed, char *arg );
void workspace_ungrab( void *passed, char *arg );
void olc_frameworks( void *passed, char *arg );
void framework_create( void *passed, char *arg );
void olc_screate( void *passed, char *arg );
void olc_edit( void *passed, char *arg );
void framework_iedit( void *passed, char *arg );
void olc_instantiate( void *passed, char *arg );
void olc_using( void *passed, char *arg );
void olc_builder( void *passed, char *arg );
void olc_show( void *passed, char *arg );
void olc_quit( void *passed, char *arg );
void olc_load( void *passed, char *arg );
void olc_chat( void *passed, char *arg );
void olc_ufilter( void *passed, char *arg );
void olc_list( void *passed, char *arg );
void olc_pak( void *passed, char *arg );
void olc_delete( void *passed, char *arg );
