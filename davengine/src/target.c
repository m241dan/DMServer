/* methods pertaining to targets written by Davenge */

#include "mud.h"

TARGET_DATA *init_target( void )
{
   TARGET_DATA *target;

   CREATE( target, TARGET_DATA, 1 );
   target->target = NULL;
   target->type = -1;
   return target;
}

void free_target( TARGET_DATA *target )
{
   target->target = NULL;
   FREE( target );
}

inline void set_target_none( TARGET_DATA *target )
{
   target->target = NULL;
   target->type = -1;
}

inline void set_target_f( TARGET_DATA *target, ENTITY_FRAMEWORK *frame )
{
   target->target = frame;
   target->type = TARGET_FRAMEWORK;
}

inline void set_target_i( TARGET_DATA *target, ENTITY_INSTANCE *instance )
{
   target->target = instance;
   target->type = TARGET_INSTANCE;
}

inline int get_target_id( TARGET_DATA *target )
{
   switch( target->type )
   {
      default: return -1;
      case TARGET_INSTANCE:
         return ((ENTITY_INSTANCE *)target->target)->tag->id;
      case TARGET_FRAMEWORK:
         return ((ENTITY_FRAMEWORK *)target->target)->tag->id;
   }
}

inline const char *get_target_string( TARGET_DATA *target )
{
   switch( target->type )
   {
      default: return NULL;
      case TARGET_INSTANCE:
         return instance_short_descr( (ENTITY_INSTANCE *)target->target );
      case TARGET_FRAMEWORK:
         return chase_name( (ENTITY_FRAMEWORK *)target->target );
   }
}
