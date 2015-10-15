/* header file for entity_variables.c written by Davenge */

extern LLIST *global_variables;

struct entity_variable
{
   char *name;
   char *value;
   int owner;
   bool type;
};

EVAR *new_int_var( const char *name, int value );
EVAR *new_str_var( const char *name, const char *string );
void free_var( EVAR *var );
void clear_evar_list( LLIST *list );
void db_load_evar( EVAR *var,  MYSQL_ROW *row );
extern inline void delete_variable_from_instance( EVAR *var, ENTITY_INSTANCE *instance );

void new_global_var( EVAR *var );
void new_entity_var( ENTITY_INSTANCE *entity, EVAR *var );
inline EVAR *remove_global_var( const char *name );
inline void remove_and_free_global_var( const char *name );
inline EVAR *remove_entity_var( ENTITY_INSTANCE *entity, const char *name );
inline void remove_and_free_entity_var( ENTITY_INSTANCE *entity, const char *name );
void update_var_name( EVAR *var, const char *name );
void update_var_value( EVAR *var, const char *value );
void update_var_owner( EVAR *var, ENTITY_INSTANCE *entity );
void update_var_type( EVAR *var, bool type );
inline EVAR *get_global_var( const char *name );
inline EVAR *get_entity_var( ENTITY_INSTANCE *entity, const char *name );
EVAR *get_var_list( const char *name, LLIST *list );
void load_global_vars( void );
void load_entity_vars( ENTITY_INSTANCE *entity );
