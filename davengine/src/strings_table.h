/* header filefor strings_table.c written by Davenge */

extern const char *const data_types[MAX_MEMORY_TYPE+1];
extern const char *const tag_table_strings[MAX_ID_HANDLER+1];
extern const char *const tag_table_whereID[MAX_ID_HANDLER+1];
extern const char *const tag_table_extensions[MAX_ID_HANDLER+1];
extern const char const tag_table_characters[MAX_ID_HANDLER+1];
extern const char *const selection_table[MAX_SEL+1];
extern const char *const target_types[MAX_TARGET+1];
/* match string from given table */
int match_string_table( const char *string, const char *const string_table[] );
int match_string_table_no_case( const char *string, const char *const string_table[] );
const char *print_string_table( const char *const string_table[] );
