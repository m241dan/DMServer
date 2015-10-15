/* header file for target.c written by Davenge */

struct target_data
{
   void *target;
   TARGET_TYPE type;
};

#define NO_TARGET( instance ) ( (instance->target->type) == -1 ? TRUE : FALSE )
#define TARGET_TYPE( instance ) ( (instance)->target->type )
#define GT_INSTANCE( instance ) ( (ENTITY_INSTANCE *)(instance)->target->target ) /* get target instance */
TARGET_DATA *init_target( void );
void free_target( TARGET_DATA *target );

extern inline void set_target_none( TARGET_DATA *target );
extern inline void set_target_f( TARGET_DATA *target, ENTITY_FRAMEWORK *frame );
extern inline void set_target_i( TARGET_DATA *target, ENTITY_INSTANCE *instance );
extern inline int get_target_id( TARGET_DATA *target );
extern inline const char *get_target_string( TARGET_DATA *target );
