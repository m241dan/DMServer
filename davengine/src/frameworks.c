/* frameworks.c: methods pertaining to frameworks written by Davenge */

#include "mud.h"

ENTITY_FRAMEWORK *init_eFramework( void )
{
   ENTITY_FRAMEWORK *frame;

   CREATE( frame, ENTITY_FRAMEWORK, 1 );
   frame->tag = init_tag();
   frame->tag->type = ENTITY_FRAMEWORK_IDS;
   frame->fixed_contents = AllocList();
   frame->specifications = AllocList();
   frame->stats = AllocList();
   frame->elements = AllocList();
   frame->composition = AllocList();
   if( clear_eFramework( frame ) != RET_SUCCESS )
   {
      bug( "could not clear memory allocated by %s", __FUNCTION__ );
      free_eFramework( frame );
      return NULL;
   }
   return frame;
}

int clear_eFramework( ENTITY_FRAMEWORK *frame )
{
   int ret = RET_SUCCESS;

   FREE( frame->name );
   frame->name = strdup( "new_frame" );
   FREE( frame->short_descr );
   frame->short_descr = strdup( "a new frame" );
   FREE( frame->long_descr );
   frame->long_descr = strdup( "a new frame is here" );
   FREE( frame->description );
   frame->description = strdup( "none" );

   frame->inherits = NULL;
   frame->f_primary_dmg_received_stat = NULL;
   return ret;
}

int set_to_inherited( ENTITY_FRAMEWORK *frame )
{
   int ret = RET_SUCCESS;

   FREE( frame->name );
   frame->name = strdup( "_inherited_" );
   FREE( frame->short_descr );
   frame->short_descr = strdup( "_inherited_" );
   FREE( frame->long_descr );
   frame->long_descr = strdup( "_inherited_" );
   FREE( frame->description );
   frame->description = strdup( "_inherited_" );

   return ret;
}

int free_eFramework( ENTITY_FRAMEWORK *frame )
{
   int ret = RET_SUCCESS;

   if( frame->tag )
      free_tag( frame->tag );

   clearlist( frame->fixed_contents );
   FreeList( frame->fixed_contents );
   frame->fixed_contents = NULL;

   specification_clear_list( frame->specifications );
   FreeList( frame->specifications );
   frame->specifications = NULL;

   clearlist( frame->stats );
   FreeList( frame->stats );
   frame->stats = NULL;

   clear_eleinfo_list( frame->elements );
   FreeList( frame->elements );
   frame->elements = NULL;

   clear_comp_list( frame->composition );
   FreeList( frame->composition );
   frame->composition = NULL;

   frame->inherits = NULL;
   frame->f_primary_dmg_received_stat = NULL;

   FREE( frame->name );
   FREE( frame->short_descr );
   FREE( frame->long_descr );
   FREE( frame->description );
   FREE( frame );

   return ret;
}

ENTITY_FRAMEWORK *load_eFramework_by_query( const char *query )
{
   ENTITY_FRAMEWORK *frame = NULL;
   MYSQL_ROW row;

   if( ( row = db_query_single_row( query ) ) == NULL )
      return NULL;

   if( ( frame = init_eFramework() ) == NULL )
      return NULL;

   db_load_eFramework( frame, &row );
   AttachToList( frame, active_frameworks );
   load_specifications_to_list( frame->specifications, quick_format( "f%d", frame->tag->id ) );
   load_fixed_possessions_to_list( frame->fixed_contents, frame->tag->id );
   load_framework_stats( frame );
   free( row );
   return frame;
}

ENTITY_FRAMEWORK *get_framework_by_id( int id )
{
   ENTITY_FRAMEWORK *frame;

   if( ( frame = get_active_framework_by_id( id ) ) == NULL )
      frame = load_eFramework_by_id( id );

   return frame;
}

ENTITY_FRAMEWORK *get_active_framework_by_id( int id )
{
   return framework_list_has_by_id( active_frameworks, id );
}

ENTITY_FRAMEWORK *load_eFramework_by_id( int id )
{
   return load_eFramework_by_query( quick_format( "SELECT * FROM `%s` WHERE %s=%d;", tag_table_strings[ENTITY_FRAMEWORK_IDS], tag_table_whereID[ENTITY_FRAMEWORK_IDS], id ) );
}

ENTITY_FRAMEWORK *get_framework_by_name( const char *name )
{
   ENTITY_FRAMEWORK *frame;

   if( ( frame = get_active_framework_by_name( name ) ) == NULL )
      frame = load_eFramework_by_name( name );

   return frame;
}

ENTITY_FRAMEWORK *get_active_framework_by_name( const char *name )
{
   return framework_list_has_by_name( active_frameworks, name );
}

ENTITY_FRAMEWORK *load_eFramework_by_name( const char *name )
{
   return load_eFramework_by_query( quick_format( "SELECT * FROM `%s` WHERE name='%s' LIMIT 1;", tag_table_strings[ENTITY_FRAMEWORK_IDS], name ) );
}

int new_eFramework( ENTITY_FRAMEWORK *frame )
{
   ENTITY_FRAMEWORK *fixed_content;
   SPECIFICATION *spec;
   STAT_FRAMEWORK *fstat;
   ITERATOR Iter;
   int ret = RET_SUCCESS;

   if( !frame )
   {
      BAD_POINTER( "frame" );
      return ret;
   }

   if( !strcmp( frame->tag->created_by, "null" ) )
   {
      if( ( ret = new_tag( frame->tag, "system" ) ) != RET_SUCCESS )
      {
         bug( "%s: failed to pull new tag from handler.", __FUNCTION__ );
         return ret;
      }
   }

   if( !quick_query( "INSERT INTO entity_frameworks VALUES( %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d' );",
         frame->tag->id, frame->tag->type, frame->tag->created_by,
         frame->tag->created_on, frame->tag->modified_by, frame->tag->modified_on,
         frame->name, frame->short_descr,
         frame->long_descr, frame->description,
         ( frame->inherits ? frame->inherits->tag->id : -1 ),
         ( frame->f_primary_dmg_received_stat ? frame->f_primary_dmg_received_stat->tag->id : -1 ),
         (int)frame->tspeed, frame->spawn_time, frame->height, frame->weight, frame->width ) )
      return RET_FAILED_OTHER;

   AttachIterator( &Iter, frame->specifications );
   while( ( spec = (SPECIFICATION *)NextInList( &Iter ) ) != NULL )
   {
      mud_printf( spec->owner, "f%d", frame->tag->id );
      new_specification( spec );
   }
   DetachIterator( &Iter );

   AttachIterator( &Iter, frame->fixed_contents );
   while( ( fixed_content = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      new_fixed_content( frame, fixed_content );
   DetachIterator( &Iter );

   AttachIterator( &Iter, frame->stats );
   while( ( fstat = (STAT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      new_stat_on_frame( fstat, frame );
   DetachIterator( &Iter );

   init_f_script( frame, TRUE );

   AttachToList( frame, active_frameworks );
   return ret;
}

void db_load_eFramework( ENTITY_FRAMEWORK *frame, MYSQL_ROW *row )
{
   int counter;

   counter = db_load_tag( frame->tag, row );
   frame->name = strdup( (*row)[counter++] );
   frame->short_descr = strdup( (*row)[counter++] );
   frame->long_descr = strdup( (*row)[counter++] );
   frame->description = strdup( (*row)[counter++] );
   frame->inherits = get_framework_by_id( atoi( (*row)[counter++] ) );
   frame->f_primary_dmg_received_stat = get_stat_framework_by_id( atoi( (*row)[counter++] ) );
   frame->tspeed = atoi( (*row)[counter++] );
   frame->spawn_time = atoi( (*row)[counter++] );
   frame->height = atoi( (*row)[counter++] );
   frame->weight = atoi( (*row)[counter++] );
   frame->width = atoi( (*row)[counter++] );
   return;
}

int load_fixed_possessions_to_list( LLIST *fixed_contents, int id )
{
   ENTITY_FRAMEWORK *frame;
   LLIST *row_list;
   MYSQL_ROW row;
   ITERATOR Iter;
   int value;

   int ret = RET_SUCCESS;

   if( !fixed_contents )
   {
      BAD_POINTER( "fixed_contents" );
      return ret;
   }

   row_list = AllocList();
   if( !db_query_list_row( row_list, quick_format( "SELECT content_frameworkID FROM `framework_fixed_possessions` WHERE entityFrameworkID=%d;", id ) ) )
   {
      FreeList( row_list );
      return RET_FAILED_OTHER;
   }

   AttachIterator( &Iter, row_list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      value = atoi( (row)[0] );
      if( ( frame = get_framework_by_id( value ) ) == NULL )
         continue;

      AttachToList( frame, fixed_contents );
   }
   DetachIterator( &Iter );
   FreeList( row_list );

  return ret;
}

ENTITY_FRAMEWORK *copy_framework( ENTITY_FRAMEWORK *frame, bool copy_id, bool copy_content, bool copy_specs, bool copy_inheritance )
{
   ENTITY_FRAMEWORK *frame_copy;

   if( !frame )
   {
      bug( "%s: passed a NULL frame.", __FUNCTION__ );
      return NULL;
   }

   CREATE( frame_copy, ENTITY_FRAMEWORK, 1 );
   frame_copy->name = strdup( frame->name );
   frame_copy->short_descr = strdup( frame->short_descr );
   frame_copy->long_descr = strdup( frame->long_descr );
   frame_copy->description = strdup( frame->description );

   if( copy_id )
      frame_copy->tag = copy_tag( frame->tag );
   else
      frame_copy->tag = init_tag();

   if( copy_content )
      frame_copy->fixed_contents = copy_framework_list( frame->fixed_contents, TRUE, TRUE, TRUE, TRUE );
   else
      frame_copy->fixed_contents = AllocList();

   if( copy_specs )
      frame_copy->specifications = copy_specification_list( frame->specifications, TRUE );
   else
      frame_copy->specifications = AllocList();

   if( copy_inheritance )
      frame_copy->inherits = copy_framework( frame->inherits, TRUE, TRUE, TRUE, TRUE );
   else
      frame_copy->inherits = frame->inherits;

   return frame_copy;
}
LLIST *copy_framework_list( LLIST *frameworks, bool copy_id, bool copy_content, bool copy_specs, bool copy_inheritance )
{
   LLIST *list;

   if( !frameworks )
   {
      bug( "%s: passed a NULL frameworks.", __FUNCTION__ );
      return NULL;
   }

   list = AllocList();
   copy_frameworks_into_list( frameworks, list, copy_id, copy_content, copy_specs, copy_inheritance );

   return list;
}
void copy_frameworks_into_list( LLIST *frame_list, LLIST *copy_into_list, bool copy_id, bool copy_content, bool copy_specs, bool copy_inheritance )
{
   ENTITY_FRAMEWORK *frame, *frame_copy;
   ITERATOR Iter;

   if( !frame_list )
   {
      bug( "%s: passed a NULL frame_list.", __FUNCTION__ );
      return;
   }
   if( !copy_into_list )
   {
      bug( "%s: passed a NULL copy_into_list.", __FUNCTION__ );
      return;
   }

   AttachIterator( &Iter, frame_list );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      if( copy_id || copy_content || copy_specs || copy_inheritance )
      {
         frame_copy = copy_framework( frame, copy_id, copy_content, copy_specs, copy_inheritance );
         AttachToList( frame_copy, copy_into_list );
         continue;
      }
      AttachToList( frame, copy_into_list );
   }
   DetachIterator( &Iter );
   return;
}

ENTITY_FRAMEWORK *copy_framework_ndi( ENTITY_FRAMEWORK *frame, LLIST *frame_list )
{
   ENTITY_FRAMEWORK *frame_copy, *frame_content, *frame_content_copy;
   ITERATOR Iter;

   if( !frame )
   {
      bug( "%s: passed a NULL frame.", __FUNCTION__ );
      return NULL;
   }
   if( !frame_list )
   {
      bug( "%s: passed a NULL frame_list.", __FUNCTION__ );
      return NULL;
   }

   if( ( frame_copy = framework_list_has_by_id( frame_list, frame->tag->id ) ) != NULL )
      return frame_copy;

   CREATE( frame_copy, ENTITY_FRAMEWORK, 1 );
   frame_copy->fixed_contents = AllocList();
   frame_copy->specifications = AllocList();

   frame_copy->tag = copy_tag( frame->tag );
   frame_copy->name = strdup( frame->name );
   frame_copy->short_descr = strdup( frame->short_descr );
   frame_copy->long_descr = strdup( frame->long_descr );
   frame_copy->description = strdup( frame->description );

   AttachIterator( &Iter, frame->fixed_contents ); /* again, hate to iterate in this function but it's just so specific */
   while( ( frame_content = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      frame_content_copy = copy_framework_ndi( frame_content, frame_list );
      AttachToList( frame_content_copy, frame_copy->fixed_contents );
   }

   copy_specifications_into_list( frame->specifications, frame_copy->specifications, TRUE );
   if( frame->inherits )
       frame_copy->inherits = copy_framework_ndi( frame->inherits, frame_list );

   AttachToList( frame_copy, frame_list );

   return frame_copy;
}

void copy_framework_list_ndi( LLIST *frame_list, LLIST *copy_into_list )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter;

   if( !frame_list )
   {
      bug( "%s: passed a NULL frame_list.", __FUNCTION__ );
      return;
   }
   if( !copy_into_list )
   {
      bug( "%s: passed a NULL copy_into_list.", __FUNCTION__ );
      return;
   }

   AttachIterator( &Iter, frame_list );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      copy_framework_ndi( frame, copy_into_list );
   DetachIterator( &Iter );

   return;
}

void append_framework_lists_ndi( LLIST *frame_list, LLIST *append_list )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter;

   if( !frame_list )
   {
      bug( "%s: passed a NULL frame_list.", __FUNCTION__ );
      return;
   }
   if( !append_list )
   {
      bug( "%s: passed a NULL append_list.", __FUNCTION__ );
      return;
   }

   AttachIterator( &Iter, frame_list );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      if( framework_list_has_by_id( append_list, frame->tag->id ) )
         continue;
      AttachToList( frame, append_list );
   }
   DetachIterator( &Iter );

   return;
}

ENTITY_FRAMEWORK *framework_list_has_by_id( LLIST *frameworks, int id )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter;

   if( !frameworks )
      return NULL;
   if( SizeOfList( frameworks ) < 1 )
      return NULL;

   AttachIterator( &Iter, frameworks );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( frame->tag->id == id )
         break;
   DetachIterator( &Iter );

   return frame;
}

ENTITY_FRAMEWORK *framework_list_has_by_name( LLIST *frameworks, const char *name )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter;

   if( !frameworks )
      return NULL;
   if( SizeOfList( frameworks ) < 1 )
      return NULL;

   AttachIterator( &Iter, frameworks );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( frame->name, name ) )
         break;
   DetachIterator( &Iter );

   return frame;
}

bool live_frame( ENTITY_FRAMEWORK *frame )
{
   if( !frame )
      return FALSE;
   if( !frame->tag )
      return FALSE;
   if( !frame->tag->created_by )
      return FALSE;
   if( !strcmp( frame->tag->created_by, "null" ) )
      return FALSE;

   return TRUE;
}

bool inherited_frame_has_any_fixed_possession( ENTITY_FRAMEWORK *frame )
{
   if( !frame->inherits )
      return FALSE;

   while( ( frame = frame->inherits ) != NULL )
      if( SizeOfList( frame->fixed_contents ) > 0 )
         return TRUE;

   return FALSE;
}

ENTITY_FRAMEWORK *create_room_framework( const char *name )
{
   ENTITY_FRAMEWORK *framework;
   SPECIFICATION *pre_loaded_spec;

   framework = init_eFramework();
   pre_loaded_spec = init_specification();

   if( name )
   {
      FREE( framework->name );
      FREE( framework->short_descr );
      framework->name = strdup( name );
      framework->short_descr = strdup( quick_format( "A %s", name ) );
   }

   pre_loaded_spec->type = SPEC_ISROOM;
   pre_loaded_spec->value = 1;
   add_spec_to_framework( pre_loaded_spec, framework );

   pre_loaded_spec = init_specification();
   pre_loaded_spec->type = SPEC_TERRAIN;
   pre_loaded_spec->value = TERRAIN_ETHER;
   add_spec_to_framework( pre_loaded_spec, framework );

   new_eFramework( framework );
   return framework;
}

ENTITY_FRAMEWORK *create_exit_framework( const char *name, int dir )
{
   ENTITY_FRAMEWORK *framework;
   SPECIFICATION *pre_loaded_spec;

   framework = init_eFramework();
   pre_loaded_spec = init_specification();

   if( name )
   {
      FREE( framework->name );
      FREE( framework->short_descr );
      framework->name = strdup( name );
      framework->short_descr = strdup( name );
   }

   pre_loaded_spec->type = SPEC_ISEXIT;
   pre_loaded_spec->value = dir;
   add_spec_to_framework( pre_loaded_spec, framework );

   new_eFramework( framework );
   return framework;
}

ENTITY_FRAMEWORK *create_mobile_framework( const char *name )
{
   ENTITY_FRAMEWORK *framework;
   SPECIFICATION *pre_loaded_spec;

   framework = init_eFramework();
   pre_loaded_spec = init_specification();

   if( name )
   {
      FREE( framework->name );
      FREE( framework->short_descr );
      framework->name = strdup( name );
      framework->short_descr = strdup( quick_format( "A %s", name ) );
   }

   pre_loaded_spec->type = SPEC_ISMOB;
   pre_loaded_spec->value = 1;
   add_spec_to_framework( pre_loaded_spec, framework );

   pre_loaded_spec = init_specification();
   pre_loaded_spec->type = SPEC_CANMOVE;
   pre_loaded_spec->value = 1;
   add_spec_to_framework( pre_loaded_spec, framework );

   new_eFramework( framework );
   return framework;

}

ENTITY_FRAMEWORK *create_inherited_framework( ENTITY_FRAMEWORK *inherit_from )
{
   ENTITY_FRAMEWORK *frame;

   frame = init_eFramework();
   set_to_inherited( frame );
   frame->inherits = inherit_from;
   new_eFramework( frame );

   return frame;
}

ENTITY_FRAMEWORK *entity_edit_selection( ENTITY_INSTANCE *entity, const char *arg )
{
   ENTITY_FRAMEWORK *to_edit;
   ENTITY_INSTANCE *to_edit_i;

   if( !interpret_entity_selection( arg ) )
   {
      text_to_entity( entity, STD_SELECTION_ERRMSG_PTR_USED );
      return NULL;
   }

   switch( input_selection_typing )
   {
      default:
         clear_entity_selection();

         /* ugly... brain no worky well right now */
         if( ( !arg || arg[0] == '\0' ) && !entity->contained_by )
         {
            text_to_entity( entity, "You are not being contained, therefor cannot use edit with no argument.\r\n" );
            return NULL;
         }
         else if( ( !arg || arg[0] == '\0' ) && entity->contained_by )
         {
            to_edit = entity->contained_by->framework;
            break;
         }

         if( ( to_edit_i = instance_list_has_by_name( entity->contained_by->contents, arg ) ) == NULL )
         {
            text_to_entity( entity, "There is no %s here.\r\n", arg );
            break;
         }
         to_edit = to_edit_i->framework;
         break;
      case SEL_FRAME:
         to_edit = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
         break;
      case SEL_INSTANCE:
         to_edit_i = (ENTITY_INSTANCE *)retrieve_entity_selection();
         to_edit = to_edit_i->framework;
         break;
   }
   if( !to_edit )
      text_to_entity( entity, "There's been a problem.\r\n" );
   return to_edit;
}

ENTITY_FRAMEWORK *olc_edit_selection( INCEPTION *olc, const char *arg )
{
   ENTITY_FRAMEWORK *to_edit;
   ENTITY_INSTANCE *to_edit_i;

   if( !interpret_entity_selection( arg ) )
   {
      text_to_olc( olc, STD_SELECTION_ERRMSG_PTR_USED );
      olc_short_prompt( olc );
      return NULL;
   }

   switch( input_selection_typing )
   {
      default:
         clear_entity_selection();
         text_to_olc( olc, "There's been a major problem. Contact your nearest admin.\r\n" );
         olc_short_prompt( olc );
         return NULL;
      case SEL_FRAME:
         to_edit = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
         break;
      case SEL_INSTANCE:
         to_edit_i = (ENTITY_INSTANCE *)retrieve_entity_selection();
         to_edit = to_edit_i->framework;
         break;
      case SEL_STRING:
         text_to_olc( olc, (char *)retrieve_entity_selection(), arg );
         return NULL;
   }
   if( !to_edit )
      text_to_olc( olc, "There's been an error.\r\n" );
   return to_edit;
}

const char *chase_name( ENTITY_FRAMEWORK *frame ) /* chase the inheritance chain, if there is one */
{
   if( !strcmp( frame->name, "_inherited_" ) )
   {
      if( !frame->inherits )
         return "inheritance error";
      return chase_name( frame->inherits );
   }
   return frame->name;
}

const char *chase_short_descr( ENTITY_FRAMEWORK *frame )
{
   if( !strcmp( frame->short_descr, "_inherited_" ) )
   {
      if( !frame->inherits )
         return "inheritance error";
      return chase_short_descr( frame->inherits );
   }
   return frame->short_descr;
}

const char *chase_long_descr( ENTITY_FRAMEWORK *frame )
{
   if( !strcmp( frame->long_descr, "_inherited_" ) )
   {
      if( !frame->inherits )
         return "inheritance error";
      return chase_long_descr( frame->inherits );
   }
   return frame->long_descr;
}


const char *chase_description( ENTITY_FRAMEWORK *frame )
{
   if( !strcmp( frame->description, "_inherited_" ) )
   {
      if( !frame->inherits )
         return "inheritance error";
      return chase_long_descr( frame->inherits );
   }
   return frame->description;
}

int get_frame_tspeed( ENTITY_FRAMEWORK *frame, int *source )
{
   if( frame->tspeed != -1 && frame->tspeed > -1 )
      return frame->tspeed;
   else if( frame->tspeed < -1 )
      return 0;
   else if( !frame->inherits )
      return 0;
   else
   {
      if( source )
         *source = 1;
      return get_frame_tspeed( frame->inherits, source );
   }
}

int get_frame_spawn_time( ENTITY_FRAMEWORK *frame, int *source )
{
   if( frame->spawn_time != -1 && frame->spawn_time > -1 )
      return frame->spawn_time;
   else if( frame->spawn_time < -1 )
      return 0;
   else if( !frame->inherits )
      return 0;
   else
   {
      if( source )
         *source = 1;
      return get_frame_spawn_time( frame->inherits, source );
   }
}

int get_frame_height( ENTITY_FRAMEWORK *frame, int *source )
{
   if( frame->height != -1 && frame->height > -1 )
      return frame->height;
   else if( frame->height < -1 )
      return 0;
   else if( !frame->inherits )
      return 0;
   else
   {
      if( source )
         *source = 1;
      return get_frame_height( frame->inherits, source );
   }
}
int get_frame_weight( ENTITY_FRAMEWORK *frame, int *source )
{
   if( frame->weight != -1 && frame->weight > -1 )
      return frame->weight;
   else if( frame->weight < -1 )
      return 0;
   else if( !frame->inherits )
      return 0;
   else
   {
      if( source )
         *source = 1;
      return get_frame_weight( frame->inherits, source );
   }
}

int get_frame_width( ENTITY_FRAMEWORK *frame, int *source )
{
   if( frame->width != -1 && frame->width > -1 )
      return frame->width;
   else if( frame->weight < -1 )
      return 0;
   else if( !frame->inherits )
      return 0;
   else
   {
      if( source )
         *source = 1;
      return get_frame_width( frame->inherits, source );
   }
}
inline void set_frame_name( ENTITY_FRAMEWORK *frame, const char *name )
{
   FREE( frame->name );
   frame->name = strdup( name );
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   if( !quick_query( "UPDATE `entity_frameworks` SET name='%s' WHERE entityFrameworkID=%d;", format_string_for_sql( frame->name ), frame->tag->id ) )
      bug( "%s: could not update database for frame %d with new name.", __FUNCTION__, frame->tag->id );
}

inline void set_frame_short_descr( ENTITY_FRAMEWORK *frame, const char *short_descr )
{
   FREE( frame->short_descr );
   frame->short_descr = strdup( short_descr );
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   if( !quick_query( "UPDATE `entity_frameworks` SET short_descr='%s' WHERE entityFrameworkID=%d;", format_string_for_sql( frame->short_descr ), frame->tag->id ) )
      bug( "%s: could no tupdate database for frame %d with new short_descr.", __FUNCTION__, frame->tag->id );
}

inline void set_frame_long_descr( ENTITY_FRAMEWORK *frame, const char *long_descr )
{
   FREE( frame->long_descr );
   frame->long_descr = strdup( long_descr );
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   if( !quick_query( "UPDATE `entity_frameworks` SET long_descr='%s' WHERE entityFrameworkID=%d;", format_string_for_sql( frame->long_descr ), frame->tag->id ) )
      bug( "%s: could not update database for frame %d with new long_descr.", __FUNCTION__, frame->tag->id );
}

inline void set_frame_description( ENTITY_FRAMEWORK *frame, const char *description )
{
   FREE( frame->description );
   frame->description = strdup( description );
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   if( !quick_query( "UPDATE `entity_frameworks` SET description='%s' WHERE entityFrameworkID=%d;", format_string_for_sql( frame->description ), frame->tag->id ) )
      bug( "%s: could not update databaes for frame %d with new description.", __FUNCTION__, frame->tag->id );
}

inline void set_frame_tspeed( ENTITY_FRAMEWORK *frame, int tspeed )
{
   if( tspeed <= 0 )
   {
      bug( "%s: tspeed value given not valid, must be greater than 0.", __FUNCTION__ );
      return;
   }
   frame->tspeed = tspeed;
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   if( !quick_query( "UPDATE `entity_frameworks` SET tspeed=%d WHERE entityFrameworkID=%d;", (int)frame->tspeed, frame->tag->id ) )
      bug( "%s: could not update database for frame %d with new thought speed.", __FUNCTION__, frame->tag->id );
}

inline void set_frame_spawn_time( ENTITY_FRAMEWORK *frame, int spawn_time )
{
   frame->spawn_time = spawn_time;
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   if( !quick_query( "UPDATE `entity_frameworks` SET spawn_time=%d WHERE entityFrameworkID=%d;", frame->spawn_time, frame->tag->id ) )
      bug( "%s: could not update dtabase for frame %d with new spawn time.", __FUNCTION__, frame->tag->id );
}

inline void set_frame_height( ENTITY_FRAMEWORK *frame, int height )
{
   frame->height = height;
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   if( !quick_query( "UPDATE `entity_frameworks` SET height=%d WHERE entityFrameworkID=%d;", frame->height, frame->tag->id ) )
      bug( "%s: could not update database for frame %d with new height.", __FUNCTION__, frame->tag->id );
}

inline void set_frame_weight( ENTITY_FRAMEWORK *frame, int weight )
{
   frame->weight = weight;
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   if( !quick_query( "UPDATE `entity_frameworks` SET weight=%d WHERE entityFrameworkID=%d;", frame->weight, frame->tag->id ) )
      bug( "%s: could not update database for frame %d with new weight.", __FUNCTION__, frame->tag->id );

}

inline void set_frame_width( ENTITY_FRAMEWORK *frame, int width )
{
   frame->width = width;
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   if( !quick_query( "UPDATE `entity_frameworks` SET width=%d WHERE entityFrameworkID=%d;", frame->width, frame->tag->id ) )
      bug( "%s: could not update database for frame %d with new width.", __FUNCTION__, frame->tag->id );

}

void add_frame_to_fixed_contents( ENTITY_FRAMEWORK *frame_to_add, ENTITY_FRAMEWORK *container )
{
   if( !frame_to_add || !container )
      return;

   if( !container->fixed_contents )
      return;

   AttachToList( frame_to_add, container->fixed_contents );

   new_fixed_content( container, frame_to_add );
   return;
}

void rem_frame_from_fixed_contents( ENTITY_FRAMEWORK *frame_to_rem, ENTITY_FRAMEWORK *container )
{
   if( !frame_to_rem || !container )
      return;

   if( !container->fixed_contents )
      return;

   DetachFromList( frame_to_rem, container->fixed_contents );

   if( !strcmp( container->tag->created_by, "null" ) )
      return;

   delete_fixed_content( container, frame_to_rem );
   return;
}

inline void set_primary_dmg_stat_framework( ENTITY_FRAMEWORK *frame, STAT_FRAMEWORK *fstat )
{
   int source;
   frame->f_primary_dmg_received_stat = fstat;
   if( !get_stat_from_framework_by_id( frame, fstat->tag->id, &source ) )
      add_stat_to_frame( fstat, frame );
   else if( source != 0 )
      add_stat_to_frame( fstat, frame );
   quick_query( "UPDATE `entity_frameworks` SET primary_dmg=%d WHERE entityFrameworkID=%d;", fstat->tag->id, frame->tag->id );
}

FILE *open_f_script( ENTITY_FRAMEWORK *frame, const char *permissions )
{
   FILE *script;
   script = fopen( get_frame_script_path( frame ), permissions );
   return script;
}

bool f_script_exists( ENTITY_FRAMEWORK *frame )
{
   FILE *script;

   if( !strcmp( frame->tag->created_by, "null" ) )
      return FALSE;

   if( ( script = fopen( quick_format( "../scripts/frames/%d.lua", frame->tag->id ), "r" ) ) == NULL )
      return FALSE;

   fclose( script );
   return TRUE;
}

void init_f_script( ENTITY_FRAMEWORK *frame, bool force )
{
   FILE *temp, *dest;

   if( f_script_exists( frame ) && !force )
      return;

   if( ( temp = fopen( "../scripts/templates/frame.lua", "r" ) ) == NULL )
   {
      bug( "%s: could not open the template.", __FUNCTION__ );
      return;
   }

   if( ( dest = fopen( quick_format( "../scripts/frames/%d.lua", frame->tag->id ), "w" ) ) == NULL )
   {
      bug( "%s: could not open the script.", __FUNCTION__ );
      return;
   }

   copy_flat_file( dest, temp );
   fclose( dest );
   fclose( temp );
   return;
}

const char *print_f_script( ENTITY_FRAMEWORK *frame )
{
   const char *buf;
   FILE *fp;

   if( !f_script_exists( frame ) )
      return "This framework has no script.";

   if( ( fp = open_f_script( frame, "r" ) ) == NULL )
      return "There was a pretty big error.";

   buf = fread_file( fp );
   fclose( fp );
   return buf;
}


inline void new_fixed_content( ENTITY_FRAMEWORK *frame, ENTITY_FRAMEWORK *content )
{
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   quick_query( "INSERT INTO `framework_fixed_possessions` VALUES( %d, %d );", frame->tag->id, content->tag->id );
}

inline void delete_fixed_content( ENTITY_FRAMEWORK *frame, ENTITY_FRAMEWORK *content )
{
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   quick_query( "DELETE FROM `framework_fixed_possessions` WHERE frameworkID=%d AND content_frameworkID=%d;", frame->tag->id, content->tag->id );
}
