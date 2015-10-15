/* header file for the elements library writte by Davenge */

#define COMP_OWNER_NONE     0
#define COMP_OWNER_ELEMENT  1
#define COMP_OWNER_FRAME    2
#define COMP_OWNER_INSTANCE 3

#define INFO_PEN	    0
#define INFO_RES	    1
#define INFO_POTENCY	    2

extern LLIST *element_frameworks;

struct element_framework
{
   char *name;
   LLIST *strong_against;
   LLIST *weak_against;
   LLIST *composition;
};

struct composition
{
   void *owner;
   int ownertype;
   ELEMENT_FRAMEWORK *frame;
   int amount;
};

struct element_info
{
   ENTITY_INSTANCE *owner;
   ELEMENT_FRAMEWORK *frame;
   int pen;
   int res;
   int potency;
};

/* creation | deletion */
ELEMENT_FRAMEWORK *init_element_frame( void );
void free_element_frame( ELEMENT_FRAMEWORK *element );

ELEMENT_INFO *init_element_info( void );
void free_element_info( ELEMENT_INFO *element );
void clear_eleinfo_list( LLIST *list );

COMPOSITION *init_composition( void );
void free_composition( COMPOSITION *comp );
void clear_comp_list( LLIST *list );

/* utility */
void load_elements_table( void );
bool base_load_lua_element( ELEMENT_FRAMEWORK *eleframe, lua_State *L );
void load_elements_list( void );
void load_element( ELEMENT_FRAMEWORK *eleframe, lua_State *L );
void load_element_strengths( ELEMENT_FRAMEWORK *eleframe, lua_State *L );
void load_element_weaknesses( ELEMENT_FRAMEWORK *eleframe, lua_State *L );
void load_element_composition( ELEMENT_FRAMEWORK *eleframe, lua_State *L );
void reload_elements_table( void );

/* getter */
ELEMENT_FRAMEWORK *get_element_framework( const char *name );
bool get_element_from_lua_table( const char *name, lua_State *L );
int get_instance_element_data( ENTITY_INSTANCE *instance, const char *elename, int which );
int get_framework_element_data( ENTITY_FRAMEWORK *frame, const char *elename, int which );
int get_info_data( ELEMENT_INFO *info, int which );
void get_instance_composition( ENTITY_INSTANCE *instance, LLIST *comprised );
void get_framework_composition( ENTITY_FRAMEWORK *frame, LLIST *comprised );
COMPOSITION *get_composition_from_instance( ENTITY_INSTANCE *instance, const char *elename );
COMPOSITION *get_composition_from_framework( ENTITY_FRAMEWORK *frame, const char *elename );

/* setter */

/* action */
extern inline void add_composition_to_element( COMPOSITION *comp, ELEMENT_FRAMEWORK *eleframe );
extern inline void add_composition_to_instance( COMPOSITION *comp, ENTITY_INSTANCE *instance );
extern inline void add_composition_to_framework( COMPOSITION *comp, ENTITY_FRAMEWORK *frame );
extern inline void set_composition_amount( COMPOSITION *comp, int amount );
void print_element_list( LLIST *list );

