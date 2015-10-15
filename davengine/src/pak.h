/* header file for paks.c written by Davenge */

bool load_pak_on_framework( const char *pak_name, ENTITY_FRAMEWORK *frame );
bool load_pak_on_instance( const char *pak_name, ENTITY_INSTANCE *instance );

extern inline bool add_pak_stat( const char *name, const char *stat );
extern inline bool add_pak_spec( const char *name, const char *spec_name, int value );
extern inline bool rem_pak_entry( const char *name, const char *stat );

const char *return_pak_contents( const char *pak_name );
