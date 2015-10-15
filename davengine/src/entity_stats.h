/* the header file for all methods in entity_stats.c written by Davenge */

extern LLIST *stat_frameworks;

struct stat_framework
{
   ID_TAG *tag;
   char *name;
   int softcap;
   int hardcap;
   int softfloor;
   int hardfloor;
   bool pool;

};

struct stat_instance
{
   ENTITY_INSTANCE *owner;
   STAT_FRAMEWORK *framework;
   int perm_stat;
   int mod_stat;
};

STAT_FRAMEWORK *init_stat_framework( void );
void free_stat_framework( STAT_FRAMEWORK *fstat );
void new_stat_framework( STAT_FRAMEWORK *fstat );
extern inline void new_stat_on_frame( STAT_FRAMEWORK *fstat, ENTITY_FRAMEWORK *frame );
extern inline void add_stat_to_frame( STAT_FRAMEWORK *fstat, ENTITY_FRAMEWORK *frame );
void db_load_stat_framework( STAT_FRAMEWORK *fstat, MYSQL_ROW *row );
void load_framework_stats( ENTITY_FRAMEWORK *frame );

STAT_INSTANCE *init_stat( void );
void free_stat( STAT_INSTANCE *stat );
void new_stat_instance( STAT_INSTANCE *stat );
void db_load_stat_instance( STAT_INSTANCE *stat, MYSQL_ROW *row );
void free_stat_list( LLIST *list );
void stat_instantiate( ENTITY_INSTANCE *owner, STAT_FRAMEWORK *fstat );
void load_entity_stats( ENTITY_INSTANCE *entity );
void instantiate_entity_stats_from_framework( ENTITY_INSTANCE *entity );
void clear_stat_list( LLIST *list );
extern inline void delete_stat_from_instance( STAT_INSTANCE *stat, ENTITY_INSTANCE *instance );
void load_new_stats( ENTITY_INSTANCE *instance );

bool inherited_frame_has_any_stats( ENTITY_FRAMEWORK *frame );

STAT_FRAMEWORK *get_stat_framework_by_query( const char *query );

extern inline STAT_FRAMEWORK *get_stat_framework_by_id( int id );
STAT_FRAMEWORK *get_active_stat_framework_by_id( int id );
extern inline STAT_FRAMEWORK *load_stat_framework_by_id( int id );

extern inline STAT_FRAMEWORK *get_stat_framework_by_name( const char *name );
STAT_FRAMEWORK *get_active_stat_framework_by_name( const char *name );
extern inline STAT_FRAMEWORK *load_stat_framework_by_name( const char *name );

STAT_FRAMEWORK *get_stat_from_framework_by_id( ENTITY_FRAMEWORK *frame, int id, int *spec_from );
STAT_FRAMEWORK *get_stat_from_framework_by_name( ENTITY_FRAMEWORK *frame, const char *name, int *spec_from );
STAT_INSTANCE  *get_stat_from_instance_by_id( ENTITY_INSTANCE *entity, int id );
STAT_INSTANCE  *get_stat_from_instance_by_name( ENTITY_INSTANCE *entity, const char *name );

STAT_FRAMEWORK *get_primary_dmg_stat_from_framework( ENTITY_FRAMEWORK *frame, int *source );

/* utility */
int get_effective_change( int cap, int floor, int raw, int effective_value, int change );

/* inlines */

/* setters */
extern inline void set_softcap( STAT_FRAMEWORK *fstat, int value );
extern inline void set_hardcap( STAT_FRAMEWORK *fstat, int value );
extern inline void set_softfloor( STAT_FRAMEWORK *fstat, int value );
extern inline void set_hardfloor( STAT_FRAMEWORK *fstat, int value );
extern inline void set_name( STAT_FRAMEWORK *fstat, const char *name );
extern inline void set_stat_style( STAT_FRAMEWORK *fstat, bool value );
extern inline void set_stat_current( STAT_INSTANCE *stat, int value );
extern inline void set_stat_max( STAT_INSTANCE *stat, int value );
extern inline void set_perm_stat( STAT_INSTANCE *stat, int value );
extern inline void set_mod_stat( STAT_INSTANCE *stat, int value );
extern inline void set_stat_owner( STAT_INSTANCE *stat, ENTITY_INSTANCE *owner );


/* getters */
extern inline int  get_stat_total( STAT_INSTANCE *stat );
extern inline int  get_stat_effective_perm( STAT_INSTANCE *stat );
extern inline int  get_stat_effective_mod( STAT_INSTANCE *stat );
extern inline int  get_stat_value( STAT_INSTANCE *stat );
extern inline int  get_stat_current( STAT_INSTANCE *stat );
extern inline int  get_stat_max( STAT_INSTANCE *stat );

/* utility */
extern inline void inc_pool_stat( STAT_INSTANCE *stat, int value );
extern inline void dec_pool_stat( STAT_INSTANCE *stat, int value );
extern inline void add_perm_stat( STAT_INSTANCE *stat, int value );
extern inline void add_mod_stat( STAT_INSTANCE *stat, int value );
extern inline void restore_pool_stats( ENTITY_INSTANCE *instance );


void lua_set_stat( STAT_INSTANCE *stat, int change );
FILE *open_s_script( STAT_FRAMEWORK *fstat, const char *permissions );
bool s_script_exists( STAT_FRAMEWORK *fstat );
void init_s_script( STAT_FRAMEWORK *fstat, bool force );
const char *print_s_script( STAT_FRAMEWORK *fstat );
