/* strings.h written by Davenge */

struct d_string
{
   char *string;
   int length;
};

STRING *new_string( const char *arg );
void free_string( STRING *string );
void change_string( STRING *string, char *change );

size_t mudcat( char *dst, const char *src );
int    number_arg             ( char *fStr, char *bStr );
int    number_arg_single      ( char *string );
char   *one_arg               ( char *fStr, char *bStr );
char   *one_arg_delim         ( char *fStr, char *bStr, char delim );
char   *one_arg_delim_literal ( char *fStr, char *bStr, char delim );
char   *strdup                ( const char *s );
int     strcasecmp            ( const char *s1, const char *s2 );
bool    is_prefix             ( const char *aStr, const char *bStr );
char   *capitalize            ( char *txt );
char   *downcase              ( const char *txt );
BUFFER *__buffer_new          ( int size );
void    __buffer_strcat       ( BUFFER *buffer, const char *text );
void    buffer_free           ( BUFFER *buffer );
void    buffer_clear          ( BUFFER *buffer );
int     bprintf               ( BUFFER *buffer, char *fmt, ... );
int     mud_printf            ( char *dest, const char *format, ... );
const char *print_header( const char *title, const char *pattern, int width );
void bprint_commandline( void *extra, BUFFER *buf, COMMAND *com, int sublevel, int pagewidth );
void print_commands( void *extra, LLIST *commands, BUFFER *buf, int sublevel, int pagewidth );
char *strip_cr( const char *str );
char *strip_nl( const char *str );
const char *handle_pagewidth( int width, const char *txt );
bool is_number( const char *arg );
char *smash_color( const char *str );
char *smash_newline( const char * str );
int color_count( const char *str );
void add_spaces( char *str, int amount );
void add_lead_space( char *str, int amount );
char *center_string( const char *to_center, int length );
char *fit_string_to_space( const char *orig, int space );
char *print_bar( const char *pattern, int width );
const char *itos( int value );
const char *quick_format( const char *Format, ... );
const char *format_string_for_sql( const char *string );
bool string_contains( const char *string, const char *regex_string );
bool grab_range_and_type( char *arg, char *type, int *start, int *end );
