/* specifications.h is a header file written by Davenge */

extern const char *const spec_table[];
extern const char *const terrain_table[];

struct typSpec
{
   short type;
   int value;
   char *owner;
};

SPECIFICATION *init_specification( void );
int clear_specification( SPECIFICATION *spec );
int free_specification( SPECIFICATION *spec );
int specification_clear_list( LLIST *spec_list );

int new_specification( SPECIFICATION *spec );
extern inline void update_spec( SPECIFICATION *spec );
int add_spec_to_framework( SPECIFICATION *spec, ENTITY_FRAMEWORK *frame );
int add_spec_to_instance( SPECIFICATION *spec, ENTITY_INSTANCE *instance );
int rem_spec_from_framework( SPECIFICATION *spec, ENTITY_FRAMEWORK *frame );
int rem_spec_from_instance( SPECIFICATION *spec, ENTITY_INSTANCE *instance );
int load_specifications_to_list( LLIST *spec_list, const char *owner );
int db_load_spec( SPECIFICATION *spec, MYSQL_ROW *row );

SPECIFICATION *copy_spec( SPECIFICATION *spec );
LLIST *copy_specification_list( LLIST *spec_list, bool copy_content );
void copy_specifications_into_list( LLIST *spec_list, LLIST *copy_into_list, bool copy_content );
void fwrite_specifications( FILE *fp, LLIST *specifications, int *id_table );
void fwrite_spec( FILE *fp, SPECIFICATION *spec, int *id_table );
SPECIFICATION *fread_specification( FILE *fp, int *id_table );


SPECIFICATION *spec_list_has_by_type( LLIST *spec_list, int type );
SPECIFICATION *spec_list_has_by_name( LLIST *spec_list, const char *name );
SPECIFICATION *has_spec_detailed_by_type( ENTITY_INSTANCE *entity, int type, int *spec_from );
SPECIFICATION *frame_has_spec_detailed_by_type( ENTITY_FRAMEWORK *frame, int type, int *spec_from );
SPECIFICATION *has_spec( ENTITY_INSTANCE *entity, const char *spec_name );
SPECIFICATION *frame_has_spec( ENTITY_FRAMEWORK *frame, const char *spec_name );
SPECIFICATION *frame_has_spec_by_type( ENTITY_FRAMEWORK *frame, int type );
int get_spec_value( ENTITY_INSTANCE *entity, const char *spec_name );
bool inherited_frame_has_any_spec( ENTITY_FRAMEWORK *frame );
