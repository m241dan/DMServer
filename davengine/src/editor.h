/* editor.h written by Davenge */

void editor_global_return( void *passed, char *arg );
void editor_switch( void *passed, char *arg );
int free_editor( INCEPTION *olc );

int init_eFramework_editor( INCEPTION *olc, ENTITY_FRAMEWORK *frame );
void boot_eFramework_editor( INCEPTION *olc, ENTITY_FRAMEWORK *frame );

void editor_eFramework_info( D_SOCKET *dsock );
int editor_eFramework_prompt( D_SOCKET *dsock, bool commands );
const char *return_framework_strings( ENTITY_FRAMEWORK *frame, const char *border, int width );
const char *return_framework_fixed_content( ENTITY_FRAMEWORK *frame, const char *border, int width );
const char *return_fixed_content_list( LLIST *fixed_list, const char *border, int width, bool inherited );
const char *return_framework_specs( ENTITY_FRAMEWORK *frame, int width );
const char *return_framework_stats( ENTITY_FRAMEWORK *frame, int width );
const char *return_framework_specs_and_stats( ENTITY_FRAMEWORK *frame, const char *border, int width );

void eFramework_name( void *passed, char *arg );
void eFramework_short( void *passed, char *arg );
void eFramework_long( void *passed, char *arg );
void eFramework_description( void *passed, char *arg );
void eFramework_set_tspeed( void *passed, char *arg );
void eFramework_set_spawn_time( void *passed, char *arg );
void eFramework_set_height( void *passed, char *arg );
void eFramework_set_weight( void *passed, char *arg );
void eFramework_set_width( void *passed, char *arg );
void eFramework_addStat( void *passed, char *arg );
void eFramework_addSpec( void *passed, char *arg );
void eFramework_remSpec( void *passed, char *arg );
void eFramework_script( void *passed, char *arg );
void eFramework_done( void *passed, char *arg );
void eFramework_save( void *passed, char *arg );
void eFramework_addContent( void *passed, char *arg );
void eFramework_addPak( void *passed, char *arg );
void eFramework_setPrimaryDmg( void *passed, char *arg );

int init_project_editor( INCEPTION *olc, PROJECT *project );
void boot_project_editor( INCEPTION *olc, PROJECT *project );

int editor_project_prompt( D_SOCKET *dsock, bool commands );
const char *return_project_workspaces_string( PROJECT *project, const char *border, int width );

void project_name( void *passed, char *arg );
void project_public( void *passed, char *arg );
void project_done( void *passed, char *arg );

int init_workspace_editor( INCEPTION *olc, WORKSPACE *wSpace );
void boot_workspace_editor( INCEPTION *olc, WORKSPACE *wSpace );

int editor_workspace_prompt( D_SOCKET *dsock, bool commands );

void workspace_name( void *passed, char *arg );
void workspace_description( void *passed, char *arg );
void workspace_public( void *passed, char *arg );
void workspace_done( void *passed, char *arg );

int init_instance_editor( INCEPTION *olc, ENTITY_INSTANCE *instance );
void boot_instance_editor( INCEPTION *olc, ENTITY_INSTANCE *instance );

void editor_instance_info( D_SOCKET *dsock );
int editor_instance_prompt( D_SOCKET *dsock, bool commands );
const char *return_instance_contents_string( ENTITY_INSTANCE *instance, const char *border, int width );
const char *return_instance_specs( ENTITY_INSTANCE *instance, int width );
const char *return_instance_stats( ENTITY_INSTANCE *instance, int width );
const char *return_instance_spec_and_stats( ENTITY_INSTANCE *intance, const char *border, int width );

void instance_load( void *passed, char *arg );
void instance_live( void *passed, char *arg );
void instance_level( void *passed, char *arg );
void instance_setStat( void *passed, char *arg );
void instance_autowrite( void *passed, char *arg );
void instance_addcontent( void *passed, char *arg );
void instance_addspec( void *passed, char *arg );
void instance_remspec( void *passed, char *arg );
void instance_script( void *passed, char *arg );
void instance_restore( void *passed, char *arg );
void instance_done( void *passed, char *arg );
void instance_addPak( void *passed, char *arg );
void instance_reinit( void *passed, char *arg );

int init_sFramework_editor( INCEPTION *olc, STAT_FRAMEWORK *fstat );
void boot_sFramework_editor( INCEPTION *olc, STAT_FRAMEWORK *fstat );
void editor_sFramework_prompt( D_SOCKET *dsock, bool commands );

void sFramework_name( void *passed, char *arg );
void sFramework_softcap( void *passed, char *arg );
void sFramework_hardcap( void *passed, char *arg );
void sFramework_softfloor( void *passed, char *arg );
void sFramework_hardfloor( void *passed, char *arg );
void sFramework_script( void *passed, char *arg );
void sFramework_type( void *passed, char *arg );
void sFramework_save( void *passed, char *arg );
void sFramework_done( void *passed, char *arg );
