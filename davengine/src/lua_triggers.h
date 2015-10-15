/* header file for the library lua_triggers written by Davenge */

extern inline void onInstanceInit_trigger	( ENTITY_INSTANCE *entity );
extern inline void onDeath_trigger		( ENTITY_INSTANCE *entity );
extern inline void onSpawn_trigger		( ENTITY_INSTANCE *entity );
extern inline void onEntityEnter_trigger	( ENTITY_INSTANCE *entity );
extern inline void onEntityLeave_trigger	( ENTITY_INSTANCE *entity );
extern inline void onEntering_trigger		( ENTITY_INSTANCE *entity );
extern inline void onLeaving_trigger		( ENTITY_INSTANCE *entity );
extern inline void onGreet_trigger		( ENTITY_INSTANCE *greeter, ENTITY_INSTANCE *enterer );
extern inline void onFarewell_trigger		( ENTITY_INSTANCE *waver, ENTITY_INSTANCE *leaver );
extern inline void onSay_trigger		( ENTITY_INSTANCE *entity );
extern inline void onGive_trigger		( ENTITY_INSTANCE *entity );

