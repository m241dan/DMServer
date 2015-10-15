/* instaces.h the headerfile for allthings instanced written by Davenge */

extern int builder_count;

typedef enum
{
   STATE_ZOMBIE, STATE_LIVE, STATE_DEAD, STATE_SPAWNING, MAX_INSTANCE_STATE
} INSTANCE_STATE;

typedef enum
{
   MIND_ZOMBIE, MIND_NEUTRAL, MIND_PANIC, MIND_FIGHTING, MIND_SCARED, MIND_AGGRESSIVE, MAX_INSTANCE_MIND
} INSTANCE_MIND;

struct entity_instance
{
   ID_TAG *tag;
   int corpse_owner;
   bool live;
   bool builder;
   bool isPlayer;
   sh_int level;
   sh_int tspeed;
   int height_mod;
   int weight_mod;
   int width_mod;
   INSTANCE_STATE state;
   INSTANCE_MIND mind;

   LLIST *contents;
   LLIST *contents_sorted[MAX_QUICK_SORT];
   LLIST *specifications;
   LLIST *stats;
   LLIST *evars;
   LLIST *events;
   LLIST *damages_sent;
   LLIST *damages_received;
   LLIST *timers;
   LLIST *elements;
   LLIST *composition;
   STAT_INSTANCE *primary_dmg_received_stat;

   ENTITY_INSTANCE *home;
   ENTITY_FRAMEWORK *framework;

   ENTITY_INSTANCE *contained_by;
   TARGET_DATA *target;

   D_SOCKET *socket;
   ACCOUNT_DATA *account;
   LLIST *commands;
};

ENTITY_INSTANCE *init_eInstance( void );
int clear_eInstance( ENTITY_INSTANCE *eInstance );
int free_eInstance( ENTITY_INSTANCE *eInstance );
int clear_ent_contents( ENTITY_INSTANCE *eInstance );
void delete_eInstance( ENTITY_INSTANCE *instance );
void delete_all_exits_to( ENTITY_INSTANCE *instance );

ENTITY_INSTANCE *init_builder( void );

ENTITY_INSTANCE *load_eInstance_by_query( const char *query );

ENTITY_INSTANCE *get_instance_by_id( int id );
ENTITY_INSTANCE *get_active_instance_by_id( int id );
ENTITY_INSTANCE *load_eInstance_by_id( int id );

ENTITY_INSTANCE *get_instance_by_name( const char *name );
ENTITY_INSTANCE *get_active_instance_by_name( const char *name );
ENTITY_INSTANCE *load_eInstance_by_name( const char *name );

ENTITY_INSTANCE *full_load_eFramework( ENTITY_FRAMEWORK *frame );
extern inline void full_load_workspace( WORKSPACE *wSpace );
extern inline void full_load_project( PROJECT *project );
void full_load_instance( ENTITY_INSTANCE *instance );

void unload_instance( ENTITY_INSTANCE *instance );

int new_eInstance( ENTITY_INSTANCE *eInstance );
void db_load_eInstance( ENTITY_INSTANCE *eInstance, MYSQL_ROW *row );
void entity_from_container( ENTITY_INSTANCE *entity );
void entity_to_world( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container );
void entity_to_contents( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container );
void attach_entity_to_contents( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container );
void detach_entity_from_contents( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container );
void entity_to_contents_quick_sort( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container );
void entity_from_contents_quick_sort( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container );

bool move_item( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *item, ENTITY_INSTANCE *move_to, bool (*test_method)( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *to_test ) );
ENTITY_INSTANCE *move_item_specific( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *target, bool (*test_method)( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *to_test ), char *item, int number, bool look_beyond_contents );
ENTITY_INSTANCE *move_item_single( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *target, bool (*test_method)( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *to_test ), char *item, bool look_beyond_contents );
LLIST *move_item_all( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *target, bool (*test_method)( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *to_test ), char *item );
LLIST *move_all( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *target, bool (*test_method)( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *to_test ) );
void move_item_messaging( ENTITY_INSTANCE *perspective, ENTITY_INSTANCE *to, void *list_or_obj, const char *orig_string, ENTITY_INSTANCE *from, ITEM_MOVE_COM mode, bool isList );
bool can_drop( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *to_drop );
bool can_give( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *to_give );
bool can_put( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *to_put );
bool can_get( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *to_get );
ENTITY_INSTANCE *find_specific_item( ENTITY_INSTANCE *perspective, const char *item, int number );
bool parse_item_movement_string( ENTITY_INSTANCE *entity, char *arg, char *item, ENTITY_INSTANCE **container, bool isGive );

ENTITY_INSTANCE *copy_instance( ENTITY_INSTANCE *instance, bool copy_id, bool copy_contents, bool copy_sepcs, bool copy_frame );
LLIST *copy_instance_list( LLIST *instances, bool copy_id, bool copy_contents, bool copy_specs, bool copy_frame );
void copy_instances_into_list( LLIST *instance_list, LLIST *copy_into_list, bool copy_id, bool copy_contents, bool copy_specs, bool copy_frame );
ENTITY_INSTANCE *copy_instance_ndi( ENTITY_INSTANCE *instance, LLIST *instance_list ); /* no duplicate ids */
void copy_instance_list_ndi( LLIST *instance_list, LLIST *copy_into_list );
void append_instance_lists_ndi( LLIST *instance_list, LLIST *append_list );

ENTITY_INSTANCE *instance_list_has_by_id( LLIST *instance_list, int id );
ENTITY_INSTANCE *instance_list_has_by_name( LLIST *instance_list, const char *name );
ENTITY_INSTANCE *instance_list_has_by_name_prefix( LLIST *instance_list, const char *name );
ENTITY_INSTANCE *instance_list_has_by_name_prefix_specific( LLIST *instance_list, const char *name, int number );
ENTITY_INSTANCE *instance_list_has_by_name_regex( LLIST *instance_list, const char *regex );
ENTITY_INSTANCE *instance_list_has_by_name_regex_specific( LLIST *instance, const char *regex, int number );
ENTITY_INSTANCE *instance_list_has_by_short_prefix( LLIST *instance_list, const char *name );
ENTITY_INSTANCE *eInstantiate( ENTITY_FRAMEWORK *frame );

ENTITY_INSTANCE *create_room_instance( const char *name );
ENTITY_INSTANCE *create_exit_instance( const char *name, int dir );
ENTITY_INSTANCE *create_mobile_instance( const char *name );
ENTITY_INSTANCE *corpsify( ENTITY_INSTANCE *instance );

void move_create( ENTITY_INSTANCE *entity, ENTITY_FRAMEWORK *exit_frame, char *arg );
bool should_move_create( ENTITY_INSTANCE *entity, char *arg );

/* getters */
const char *instance_name		( ENTITY_INSTANCE *instance );
const char *instance_short_descr	( ENTITY_INSTANCE *instance );
const char *instance_long_descr		( ENTITY_INSTANCE *instance );
const char *instance_description	( ENTITY_INSTANCE *instance );
int	    get_corpse_decay		( ENTITY_INSTANCE *instance );
extern inline int  get_height		( ENTITY_INSTANCE *instance );
extern inline int  get_weight		( ENTITY_INSTANCE *instance );
extern inline int  get_width		( ENTITY_INSTANCE *instance );

/* setters */
void instance_toggle_live( ENTITY_INSTANCE *instance );
void instance_toggle_player( ENTITY_INSTANCE *instance );
extern inline void set_instance_level( ENTITY_INSTANCE *instance, int level );
extern inline void set_instance_state( ENTITY_INSTANCE *instance, INSTANCE_STATE state );
extern inline void set_instance_mind( ENTITY_INSTANCE *instance, INSTANCE_MIND mind );
extern inline void set_instance_tspeed( ENTITY_INSTANCE *instance, int tspeed );
extern inline void set_instance_home( ENTITY_INSTANCE *instance );
extern inline void set_instance_corpse_owner( ENTITY_INSTANCE *instance, int id );
extern inline void set_instance_height_mod( ENTITY_INSTANCE *instance, int height_mod );
extern inline void set_instance_weight_mod( ENTITY_INSTANCE *instance, int weight_mod );
extern inline void set_instance_width_mod( ENTITY_INSTANCE *instance, int width_mod );

/* actions */
bool do_damage( ENTITY_INSTANCE *entity, DAMAGE *dmg );
void death_instance( ENTITY_INSTANCE *instance );
void spawn_instance( ENTITY_INSTANCE *instance );
void set_for_decay( ENTITY_INSTANCE *corpse, int delay );
void set_for_respawn( ENTITY_INSTANCE *instance );
void corpsify_inventory( ENTITY_INSTANCE *instance, ENTITY_INSTANCE *corpse );
void builder_takeover( ENTITY_INSTANCE *builder, ENTITY_INSTANCE *mob );
void return_entity( ENTITY_INSTANCE *entity );

/* utility */
int text_to_entity( ENTITY_INSTANCE *entity, const char *fmt, ... );
void text_around_entity( ENTITY_INSTANCE *perspective, int num_around, const char *fmt, ... );
void echo_to_room( ENTITY_INSTANCE *room, const char *msg );
int builder_prompt( D_SOCKET *dsock );
void player_prompt( D_SOCKET *dsock );
int show_ent_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing );
int show_ent_contents_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing );
int show_ent_exits_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing );
int show_ent_mobiles_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing );
int show_ent_objects_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing );
int show_ent_rooms_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing );
int  move_entity( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *move_to );
void mobile_move( ENTITY_INSTANCE *entity, const char *direction );
FILE *open_i_script( ENTITY_INSTANCE *instance, const char *permissions );
bool i_script_exists( ENTITY_INSTANCE *instance );
void init_i_script( ENTITY_INSTANCE *instance, bool force );
const char *print_i_script( ENTITY_INSTANCE *instance );

/* builders commands */
void entity_goto( void *passed, char *arg );
void entity_instance( void *passed, char *arg );
void entity_look( void *passed, char *arg );
void entity_inventory( void *passed, char *arg );
void entity_drop( void *passed, char *arg );
void entity_get( void *passed, char *arg );
void entity_put( void *passed, char *arg  );
void entity_give( void *passed, char *arg );
void entity_enter( void *passed, char *arg );
void entity_leave( void *passed, char *arg );
void entity_quit( void *passed, char *arg );
void entity_create( void *passed, char *arg );
void entity_edit( void *passed, char *arg );
void entity_iedit( void *passed, char *arg );
void entity_load( void *passed, char *arg );
void entity_north( void *passed, char *arg );
void entity_south( void *passed, char *arg );
void entity_west( void *passed, char *arg );
void entity_east( void *passed, char *arg );
void entity_up( void *passed, char *arg );
void entity_down( void *passed, char *arg );
void entity_chat( void *passed, char *arg );
void entity_grab( void *passed, char *arg );
void entity_using( void *passed, char *arg );
void entity_olc( void *passed, char *arg );
void entity_target( void *passed, char *arg );
void entity_show( void *passed, char *arg );
void entity_set_home( void *passed, char *arg );
void entity_restore( void *passed, char *arg );
void entity_takeover( void *passed, char *arg );

/* mobile commands */
void mobile_north( void *passed, char *arg );
void mobile_south( void *passed, char *arg );
void mobile_east( void *passed, char *arg );
void mobile_west( void *passed, char *arg );
void mobile_up( void *passed, char *arg );
void mobile_down( void *passed, char *arg );
void mobile_return( void *passed, char *arg );
void mobile_look( void *passed, char *arg );
void mobile_inventory( void *passed, char *arg );
void mobile_score( void *passed, char *arg );
void mobile_say( void *passed, char *arg );
void mobile_attack( void *passed, char *arg );
void mobile_kill( void *passed, char *arg );

/* player commands */
void player_quit( void *passed, char *arg );
