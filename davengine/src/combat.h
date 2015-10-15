/* header file for the combat.c library written by Davenge */

extern LLIST *damage_queue; /* queue of sent damages */

typedef enum
{
   HIT_UNKNOWN = -1, HIT_SUCCESS, HIT_DODGED, HIT_PARRIED, HIT_MISSED, MAX_CBT_RET
} cbt_ret;

typedef enum
{
   DMG_UNKNOWN = -1, DMG_MELEE, MAX_DMG_TYPE
} DMG_SRC;

struct damage_data
{
   ENTITY_INSTANCE *attacker;
   ENTITY_INSTANCE *victim;
   void *dmg_src;
   DMG_SRC type;
   int amount;
   bool crit;
   TIMER *timer;
   /* bitvector composition */
   DAMAGE *additional_dmg;
};

/* creation */
DAMAGE *init_damage	( void );
void    free_damage	( DAMAGE *dmg );

/* actions */
void		prep_melee_atk	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim );
void		prep_melee_dmg	( DAMAGE *dmg );
bool		receive_damage	( DAMAGE *dmg );

cbt_ret		melee_attack	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim );
bool		send_damage	( DAMAGE *dmg );

/* checkers */
bool		does_check	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim, const char *does );
bool		can_melee	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim );

/* getters */
int		get_auto_cd	( ENTITY_INSTANCE *instance ); 
const char	*dmg_src_name	( DAMAGE *dmg ); /* not written... */


/* utility */
const char	*compose_dmg_key( DAMAGE *dmg );

/* actions */
void		handle_damage	( DAMAGE *dmg );
void		make_dead	( ENTITY_INSTANCE *dead_person );
void		combat_message	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim, DAMAGE *dmg, cbt_ret status );
void		start_killing_mode( ENTITY_INSTANCE *instance, bool message );
void		end_killing_mode( ENTITY_INSTANCE *instance, bool message );

/* inlines */

/* creation */
extern inline void free_damage_list	( LLIST *damages );
extern inline EVENT_DATA *melee_event	( void );
/* utility */
extern inline void add_damage		( DAMAGE *dmg );
extern inline void rem_damage		( DAMAGE *dmg );

/* checkers */
extern inline bool is_dmg_queued	( DAMAGE *dmg );

/* setters */
extern inline void set_dmg_attacker	( DAMAGE *dmg, ENTITY_INSTANCE *attacker );
extern inline void set_dmg_victim	( DAMAGE *dmg, ENTITY_INSTANCE *victim );
extern inline void set_dmg_type		( DAMAGE *dmg, DMG_SRC type );
extern inline void set_dmg_src		( DAMAGE *dmg, void *dmg_src, DMG_SRC type );

/* actions */
