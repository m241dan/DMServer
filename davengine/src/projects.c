/* the file containing methods pertaining to all things project related written by Davenge */

#include "mud.h"

PROJECT *init_project( void )
{
   PROJECT *project;

   CREATE( project, PROJECT, 1 );
   project->workspaces = AllocList();
   project->tag = init_tag();
   project->tag->type = PROJECT_IDS;
   clear_project( project );
   return project;
}

int free_project( PROJECT *project )
{
   int ret = RET_SUCCESS;

   if( project->tag )
      free_tag( project->tag );

   clearlist( project->workspaces );
   FreeList( project->workspaces );
   project->workspaces = NULL;

   FREE( project );

   return ret;
}

int clear_project( PROJECT *project )
{
   int ret = RET_SUCCESS;

   project->name = strdup( "new_project" );
   project->Public = FALSE;

   return ret;
}

PROJECT *load_project_by_query( const char *query )
{
   PROJECT *project = NULL;
   MYSQL_ROW row;

   if( ( row = db_query_single_row( query ) ) == NULL )
      return NULL;

   if( ( project = init_project() ) == NULL )
      return NULL;

   db_load_project( project, &row );
   load_project_entries( project );
   free( row ); /* this might be leaky... */

   return project;
}

PROJECT *load_project_by_id( int id )
{
   return load_project_by_query( quick_format( "SELECT * FROM `projects` WHERE projectID=%d;", id ) );
}

PROJECT *load_project_by_name( const char *name )
{
   return load_project_by_query( quick_format( "SELECT * FROM `projects` WHERE name='%s';", format_string_for_sql( name ) ) );
}

int new_project( PROJECT *project )
{
   int ret = RET_SUCCESS;

   if( !project )
   {
      BAD_POINTER( "project" );
      return ret;
   }

   if( !strcmp( project->tag->created_by, "null" ) )
   {
      if( ( ret = new_tag( project->tag, "system" ) ) != RET_SUCCESS )
      {
         bug( "%s: failed to pull new tag from handler.", __FUNCTION__ );
         return ret;
      }
   }

   if( !quick_query( "INSERT INTO projects VALUES( %d, %d, '%s', '%s', '%s', '%s', '%s', '%d' );",
         project->tag->id, project->tag->type, project->tag->created_by, project->tag->created_on,
         project->tag->modified_by, project->tag->modified_on, project->name, (int)project->Public ) )
      return RET_FAILED_OTHER;

   return ret;
}

int new_project_entry( PROJECT *project, ID_TAG *tag )
{
   int ret = RET_SUCCESS;

   if( !project )
   {
      BAD_POINTER( "wSpace" );
      return ret;
   }
   if( !tag )
   {
      BAD_POINTER( "tag" );
      return ret;
   }

   if( !quick_query( "INSERT INTO `project_entries` VALUES( %d, '%c%d' );", project->tag->id, tag_table_characters[tag->type], tag->id ) )
      return RET_FAILED_OTHER;

   return ret;
}

void db_load_project( PROJECT *project, MYSQL_ROW *row )
{
   int counter;

   counter = db_load_tag( project->tag, row );
   project->name = strdup( (*row)[counter++] );
   project->Public = (bool)( atoi( (*row)[counter++] ) );

   return;
}

void load_project_entries( PROJECT *project )
{
   WORKSPACE *wSpace;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;
   int id;

   if( !project )
   {
      bug( "%s: bad project pointer.", __FUNCTION__ );
      return;
   }

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT entry FROM `project_entries` WHERE projectID=%d", project->tag->id ) ) )
   {
      bug( "%s: could not load the list of rows.", __FUNCTION__ );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      switch( row[0][0] )
      {
         default: continue;
         case 'w':
            id = atoi( row[0]+1 );
            if( ( wSpace = get_workspace_by_id( id ) ) == NULL )
            {
               bug( "%s: bad entry in project_entires %d.", __FUNCTION__, id );
               continue;
            }
            AttachToList( wSpace, project->workspaces );
            free( row );
            break;
      }
   }
   FreeList( list );
   return;
}

void load_project_workspaces_into_olc( PROJECT *project, INCEPTION *olc )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( SizeOfList( project->workspaces ) < 1 )
      return;

   AttachIterator( &Iter, project->workspaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      AttachToList( wSpace, olc->wSpaces );
   DetachIterator( &Iter );

   return;
}

void add_workspace_to_project( WORKSPACE *wSpace, PROJECT *project )
{
   if( !wSpace )
   {
      bug( "%s: workspace pointer is NULL.", __FUNCTION__ );
      return;
   }
   if( !project )
   {
      bug( "%s: project pointer is NULL.", __FUNCTION__ );
      return;
   }
   if( workspace_list_has_by_id( project->workspaces, wSpace->tag->id ) )
   {
       bug( "%s: project always has workspace with %d id.", __FUNCTION__, wSpace->tag->id );
       return;
   }

   AttachToList( wSpace, project->workspaces );
   new_project_entry( project, wSpace->tag );
   return;
}

void rem_workspace_from_project( WORKSPACE *wSpace, PROJECT *project )
{
   DetachFromList( wSpace, project->workspaces );
   if( !quick_query( "DELETE FROM `project_entires` WHERE projectID=%d AND entry=%d;", project->tag->id, wSpace->tag->id ) )
      bug( "%s: could not delete from the database the removed entry.", __FUNCTION__ );
   return;
}

void load_project_into_olc( PROJECT *project, INCEPTION *olc )
{
   if( !olc )
   {
      bug( "%s: trying to load project into NULL olc.", __FUNCTION__ );
      return;
   }
   if( !project )
   {
       bug( "%s: trying load add NULL project into olc.", __FUNCTION__ );
       return;
   }
   olc->project = project;
   load_project_workspaces_into_olc( project, olc );
   return;
}

DIR *open_projects_dir( void )
{
   DIR *projects_dir;

   if( ( projects_dir = opendir( "../projects/" ) ) == NULL )
   {
      bug( "%s: there's something wrong with the projects directory.", __FUNCTION__ );
      return NULL;
   }
   return projects_dir;
}

void import_project( DIR *project_directory, const char *dir_name )
{
   PROJECT *project;
   int *workspace_id_table;
   int *framework_id_table;
   int *instance_id_table;

   /* build id tables */
   workspace_id_table = build_id_table_import( project_directory, WORKSPACE_IDS );
   framework_id_table = build_id_table_import( project_directory, ENTITY_FRAMEWORK_IDS );
   instance_id_table = build_id_table_import( project_directory, ENTITY_INSTANCE_IDS );

   project = init_project_from_info( dir_name );

   load_workspaces_from_directory_into_db_and_project( project, project_directory, dir_name, workspace_id_table, framework_id_table, instance_id_table );
   load_frameworks_from_directory_into_db( project_directory, dir_name, framework_id_table );
   load_instances_from_directory_into_db( project_directory, dir_name, instance_id_table, framework_id_table );

   free_project( project );
   FREE( workspace_id_table );
   FREE( framework_id_table );
   FREE( instance_id_table );
}

/* this monster needs factoring */
void export_project( PROJECT *project )
{
   LLIST *framework_list, *instance_list;
   int *workspace_id_table;
   int *framework_id_table;
   int *instance_id_table;
   char directory[MAX_BUFFER];

   /* craft directory for our project */
   mud_printf( directory, "%s", create_project_directory( project ) );

   framework_list = AllocList();
   instance_list = AllocList();

   /* write our grand lists for id tables */
   create_complete_framework_and_instance_list_from_workspace_list( project->workspaces, instance_list, framework_list );

   /* build workspace_id_table first */
   workspace_id_table = build_workspace_id_table( project->workspaces );
   instance_id_table = build_instance_id_table( instance_list );
   framework_id_table = build_framework_id_table( framework_list );

   /* commence writing */
   save_project( project, directory );
   save_workspace_list_export( project->workspaces, directory, workspace_id_table, instance_id_table, framework_id_table );
   save_instance_list_export( instance_list, directory, instance_id_table, framework_id_table );
   save_framework_list_export( framework_list, directory, framework_id_table );

   /* free memory */
   clearlist( framework_list );
   FreeList( framework_list );
   clearlist( instance_list );
   FreeList( instance_list );
   FREE( workspace_id_table );
   FREE( framework_id_table );
   FREE( instance_id_table );
   return;
}

void save_project( PROJECT *project, char *directory )
{
   FILE *fp;

   if( ( fp = fopen( quick_format( "%s/info.project", directory ), "w" ) ) == NULL )
   {
      bug( "%s: Unable to write project info.", __FUNCTION__ );
      return;
   }
   fwrite_id_tag_export( fp, project->tag, NULL );
   fprintf( fp, "#PROJECT\n\n" );
   fprintf( fp, "Name         %s~\n", project->name );
   fprintf( fp, "Public       %d\n", (int)project->Public );
   fprintf( fp, "#END\n" );
   fprintf( fp, "%s\n", FILE_TERMINATOR );
   fclose( fp );
   return;
}

PROJECT *init_project_from_info( const char *dir_name )
{
   PROJECT *project;
   FILE *fp;
   char *word;
   bool found, done = FALSE;

   if( ( fp = fopen( quick_format( "%sinfo.project", dir_name ), "r" ) ) == NULL )
   {
      bug( "%s: Unable to read project info.", __FUNCTION__ );
      return NULL;
   }

   CREATE( project, PROJECT, 1 );
   project->workspaces = AllocList();

   word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   while( !done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case '#':
            if( !strcmp( word, "#END" ) )
            {
               project->tag->id = get_new_id( PROJECT_IDS );
               new_project( project );
               return project;
            }
            if( !strcmp( word, "#IDTAG" ) )
            {
               found = TRUE;
               project->tag = fread_id_tag_import( fp, NULL );
               project->tag->id = get_new_id( PROJECT_IDS );
               break;
            }
            if( !strcmp( word, "#PROJECT" ) )
            {
               found = TRUE;
               break;
            }
            break;
         case 'N':
            SREAD( "Name", project->name );
            break;
         case 'P':
            IREAD( "Public", project->Public );
            break;
      }
      if( !found )
         bug( "%s: bad file format: %s", __FUNCTION__, word );
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   free_project( project );
   return NULL;
}

void load_workspaces_from_directory_into_db_and_project( PROJECT *project, DIR *project_directory, const char *dir_name, int *workspace_id_table, int *framework_id_table, int *instance_id_table )
{
   WORKSPACE *wSpace;
   FILE *fp;
   DIR_FILE *file;

   for( file = readdir( project_directory ); file; file = readdir( project_directory ) )
   {
      if( string_contains( file->d_name, ".workspace" ) )
      {
         if( ( fp = fopen( quick_format( "%s/%s", dir_name, file->d_name ), "r" ) ) == NULL )
         {
            bug( "%s: could not read %s.", __FUNCTION__, file->d_name );
            continue;
         }
         if( ( wSpace = fread_workspace_import( fp, workspace_id_table, framework_id_table, instance_id_table ) ) == NULL )
         {
            bug( "%s: bad workspace file %s.", __FUNCTION__, file->d_name );
            continue;
         }
         fclose( fp );
         add_workspace_to_project( wSpace, project );
      }
   }
   rewinddir( project_directory );
   return;
}

WORKSPACE *fread_workspace_import( FILE *fp, int *workspace_id_table, int *framework_id_table, int *instance_id_table )
{
   WORKSPACE *wSpace;
   char *word;
   int position;
   bool found, done = FALSE;

   CREATE( wSpace, WORKSPACE, 1 );
   wSpace->frameworks = AllocList();
   wSpace->instances = AllocList();
   wSpace->who_using = AllocList();

   word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   while( !done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case '#':
            if( !strcmp( word, "#END" ) )
            {
               new_workspace( wSpace );
               return wSpace;
            }
            if( !strcmp( word, "#IDTAG" ) )
            {
               found = TRUE;
               wSpace->tag = fread_id_tag_import( fp, workspace_id_table );
               break;
            }
            if( !strcmp( word, "#WORKSPACE" ) )
            {
               found = TRUE;
               break;
            }
            break;
         case 'D':
            SREAD( "Descr", wSpace->description );
            break;
         case 'F':
            if( !strcmp( word, "Framework" ) )
            {
               found = TRUE;
               position = fread_number( fp );
               quick_query( "INSERT INTO workspace_entries VALUES ( %d, 'f%d' );", wSpace->tag->id, framework_id_table[position] );
               break;
            }
            break;
         case 'I':
            if( !strcmp( word, "Instance" ) )
            {
               found = TRUE;
               position = fread_number( fp );
               quick_query( "INSERT INTO workspace_entries VALUES ( %d, 'i%d' );", wSpace->tag->id, instance_id_table[position] );
               break;
            }
            break;
         case 'N':
            SREAD( "Name", wSpace->name );
            break;
         case 'P':
            IREAD( "Public", wSpace->Public );
            break;
      }
      if( !found )
         bug( "%s: bad file format: %s", __FUNCTION__, word );
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   free_workspace( wSpace );
   return NULL;
}

void load_frameworks_from_directory_into_db( DIR *project_directory, const char *dir_name, int *framework_id_table )
{
   FILE *fp;
   DIR_FILE *file;

   for( file = readdir( project_directory ); file; file = readdir( project_directory ) )
   {
      if( string_contains( file->d_name, ".framework" ) )
      {
         if( ( fp = fopen( quick_format( "%s/%s", dir_name, file->d_name ), "r" ) ) == NULL )
         {
            bug( "%s: could not read %s.", __FUNCTION__, file->d_name );
            continue;
         }
         fread_framework_import( fp, framework_id_table );
         fclose( fp );
      }
   }
   rewinddir( project_directory );
   return;
}

void fread_framework_import( FILE *fp, int *framework_id_table )
{
   ENTITY_FRAMEWORK *frame;
   char *word;
   int position, inherits;
   bool found, done = FALSE;

   CREATE( frame, ENTITY_FRAMEWORK, 1 );
   frame->specifications = AllocList();

   word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   while( !done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case '#':
            if( !strcmp( word, "#END" ) )
            {
               quick_query( "INSERT INTO `entity_frameworks` VALUES ( %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%d' );",
                  frame->tag->id, frame->tag->type, frame->tag->created_by,
                  frame->tag->created_on, frame->tag->modified_by, frame->tag->modified_on,
                  frame->name, frame->short_descr, frame->long_descr, frame->description,
                  inherits == -1 ? inherits : framework_id_table[inherits] );
               free_eFramework( frame );
               return;
            }
            if( !strcmp( word, "#IDTAG" ) )
            {
               found = TRUE;
               frame->tag = fread_id_tag_import( fp, framework_id_table );
               break;
            }
            if( !strcmp( word, "#FRAMEWORK" ) )
            {
               found = TRUE;
               break;
            }
            break;
         case 'D':
            SREAD( "Description", frame->description );
            break;
         case 'F':
            if( !strcmp( word, "FContent" ) )
            {
               found = TRUE;
               position = fread_number( fp );
               quick_query( "INSERT INTO `framework_fixed_possessions` VALUES ( %d, %d );", frame->tag->id, framework_id_table[position] );
               break;
            }
            break;
         case 'I':
            IREAD( "Inherits", inherits );
            break;
         case 'L':
            SREAD( "Long_Descr", frame->long_descr );
            break;
         case 'N':
            SREAD( "Name", frame->name );
            break;
         case 'S':
            SREAD( "Short_Descr", frame->short_descr );
            if( !strcmp( word, "Spec" ) )
            {
               int type, value;
               found = TRUE;

               type = fread_number( fp );
               value = fread_number( fp );
               if( type == SPEC_ISEXIT )
                  value = framework_id_table[value];
               quick_query( "INSERT INTO live_specs VALUES ( '%s', %d, 'f%d' );", spec_table[type], value, frame->tag->id );
               break;
            }
            if( !strcmp( word, "Script" ) )
            {
               FILE *script;

               if( ( script = open_f_script( frame, "w" ) ) == NULL )
               {
                  bug( "%s: having a problem creating the script for the imported framework.", __FUNCTION__ );
                  break;
               }

               fprintf( script, "%s", fread_file( fp ) );
               quick_query( "INSERT INTO `entity_frameworks` VALUES ( %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%d' );",
                  frame->tag->id, frame->tag->type, frame->tag->created_by,
                  frame->tag->created_on, frame->tag->modified_by, frame->tag->modified_on,
                  frame->name, frame->short_descr, frame->long_descr, frame->description,
                  inherits == -1 ? inherits : framework_id_table[inherits] );
               free_eFramework( frame );
               fclose( script );
               return;
            }
            break;
      }
      if( !found )
         bug( "%s: bad file format: %s", __FUNCTION__, word );
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   free_eFramework( frame );
   return;
}

void load_instances_from_directory_into_db( DIR *project_directory, const char *dir_name, int *instance_id_table, int *framework_id_table )
{
   FILE *fp;
   DIR_FILE *file;

   for( file = readdir( project_directory ); file; file = readdir( project_directory ) )
   {
      if( string_contains( file->d_name, ".instance" ) )
      {
         if( ( fp = fopen( quick_format( "%s/%s", dir_name, file->d_name ), "r" ) ) == NULL )
         {
            bug( "%s: could not read %s.", __FUNCTION__, file->d_name );
            continue;
         }
         fread_instance_import( fp, instance_id_table, framework_id_table );
         fclose( fp );
      }
   }
   rewinddir( project_directory );
   return;
}

void fread_instance_import( FILE *fp, int *instance_id_table, int *framework_id_table )
{
   ENTITY_INSTANCE *instance;
   char *word;
   int position, frame, contained_by = -1;
   bool found, done = FALSE;

   CREATE( instance, ENTITY_INSTANCE, 1 );
   instance->specifications = AllocList();

   word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   while( !done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case '#':
            if( !strcmp( word, "#END" ) )
            {
               found = FALSE;
               quick_query( "INSERT INTO entity_instances VALUES( %d, %d, '%s', '%s', '%s', '%s', %d, %d, %d, %d, %d, %d, %d, %d );",
                  instance->tag->id, instance->tag->type, instance->tag->created_by,
                  instance->tag->created_on, instance->tag->modified_by, instance->tag->modified_on,
                  contained_by, frame,
                  (int)instance->live );
                  free_eInstance( instance );
               return;
            }
            if( !strcmp( word, "#IDTAG" ) )
            {
               found = TRUE;
               instance->tag = fread_id_tag_import( fp, instance_id_table );
               break;
            }
            if( !strcmp( word, "#INSTANCE" ) )
            {
               found = TRUE;
               break;
            }
            break;
         case 'C':
            if( !strcmp( word, "ContainedBy" ) )
            {
               found = TRUE;
               if( ( contained_by = fread_number( fp ) ) != -1 )
                  contained_by = instance_id_table[contained_by];
               break;
            }
            if( !strcmp( word, "Content" ) )
            {
               found = TRUE;
               position = fread_number( fp );
               quick_query( "INSERT INTO `entity_instance_possessions` VALUES ( %d, %d );", instance->tag->id, instance_id_table[position] );
               break;
            }
            break;
         case 'F':
            if( !strcmp( word, "Framework" ) )
            {
               found = TRUE;
               frame = fread_number( fp );
               frame = framework_id_table[frame];
               break;
            }
         case 'L':
            IREAD( "Level", instance->level );
            break;
         case 'S':
            if( !strcmp( word, "Spec" ) )
            {
               int type, value;
               found = TRUE;

               type = fread_number( fp );
               value = fread_number( fp );
               if( type == SPEC_ISEXIT )
                  value = instance_id_table[value];
               quick_query( "INSERT INTO live_specs VALUES ( '%s', %d, '%d' );", spec_table[type], value, instance->tag->id );
               break;
            }
            break;
      }
      if( !found )
         bug( "%s: bad file format: %s", __FUNCTION__, word );
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   free_eInstance( instance );
   return;
}

void save_workspace_list_export( LLIST *workspace_list, char *directory, int *workspace_id_table, int *instance_id_table, int *framework_id_table )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   AttachIterator( &Iter, workspace_list );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      save_workspace_export( directory, wSpace, workspace_id_table, instance_id_table, framework_id_table );
   DetachIterator( &Iter );

   return;
}

void save_workspace_export( char *pDir, WORKSPACE *wSpace, int *workspace_id_table, int *instance_id_table, int *framework_id_table )
{
   FILE *fp;
   int new_id;

   new_id = get_id_table_position( workspace_id_table, wSpace->tag->id );
   if( ( fp = fopen( quick_format( "%s/%d.workspace", pDir, new_id ), "w" ) ) == NULL )
   {
      bug( "%s: Unable to write workspace (%d)%s.", __FUNCTION__, wSpace->tag->id, wSpace->name );
      return;
   }
   fwrite_workspace_export( fp, wSpace, workspace_id_table, instance_id_table, framework_id_table );
   fprintf( fp, "%s\n", FILE_TERMINATOR );
   fclose( fp );
   return;
}

void fwrite_workspace_export( FILE *fp, WORKSPACE *wSpace, int *workspace_id_table, int *instance_id_table, int *framework_id_table )
{
   fwrite_id_tag_export( fp, wSpace->tag, workspace_id_table);
   fprintf( fp, "#WORKSPACE\n" );
   fprintf( fp, "Name         %s~\n", wSpace->name );
   fprintf( fp, "Descr        %s~\n", wSpace->description );
   fprintf( fp, "Public       %d\n", (int)wSpace->Public );

   fwrite_workspace_entries_export( fp, wSpace, instance_id_table, framework_id_table );
   fprintf( fp, "#END\n\n" );
   return;
}

void fwrite_workspace_entries_export( FILE *fp, WORKSPACE *wSpace, int *instance_id_table, int *framework_id_table )
{
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;
   ITERATOR Iter;

   AttachIterator( &Iter, wSpace->instances );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      fprintf( fp, "Instance     %d\n", get_id_table_position( instance_id_table, instance->tag->id ) );
   DetachIterator( &Iter );

   AttachIterator( &Iter, wSpace->frameworks );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      fprintf( fp, "Framework    %d\n", get_id_table_position( framework_id_table, frame->tag->id ) );
   DetachIterator( &Iter );

   return;
}

void save_instance_list_export( LLIST *instance_list, char *directory, int *instance_id_table, int *framework_id_table )
{
   ENTITY_INSTANCE *instance;
   ITERATOR Iter;

   AttachIterator( &Iter, instance_list );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      save_instance_export( directory, instance, instance_id_table, framework_id_table );
   DetachIterator( &Iter );

   return;
}

void save_instance_export( char *pDir, ENTITY_INSTANCE *instance, int *instance_id_table, int *framework_id_table )
{
   FILE *fp;
   int new_id;

   new_id = get_id_table_position( instance_id_table, instance->tag->id );
   if( ( fp = fopen( quick_format( "%s/%d.instance", pDir, new_id ), "w" ) ) == NULL )
   {
      bug( "%s: Unable to write instance (%d)%s.", __FUNCTION__, instance->tag->id, instance_name( instance ) );
      return;
   }
   fwrite_instance_export( fp, instance, instance_id_table, framework_id_table );
   fprintf( fp, "%s\n", FILE_TERMINATOR );
   fclose( fp );
   return;
}

void fwrite_instance_export( FILE *fp, ENTITY_INSTANCE *instance, int *instance_id_table, int *framework_id_table )
{
   fwrite_id_tag_export( fp, instance->tag, instance_id_table);
   fprintf( fp, "#INSTANCE\n" );
   fprintf( fp, "Level        %d\n", instance->level );
   fwrite_instance_content_list_export( fp, instance->contents, instance_id_table );
   fwrite_specifications( fp, instance->specifications, instance_id_table );

   fprintf( fp, "Framework    %d\n", get_id_table_position( framework_id_table, instance->framework->tag->id ) );
   if( instance->contained_by )
      fprintf( fp, "ContainedBy  %d\n", get_id_table_position( instance_id_table, instance->contained_by->tag->id ) );
   else
      fprintf( fp, "ContainedBy  %d\n", -1 );
   fprintf( fp, "#END\n\n" );
}

void fwrite_instance_content_list_export( FILE *fp, LLIST *contents, int *instance_id_table )
{
   ENTITY_INSTANCE *content;
   ITERATOR Iter;
   int position;

   AttachIterator( &Iter, contents );
   while( ( content = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      if( ( position = get_id_table_position( instance_id_table, content->tag->id ) ) != -1 )
         fprintf( fp, "Content      %d\n", position );
   }
   DetachIterator( &Iter );

   return;
}

void save_framework_list_export( LLIST *framework_list, char *directory, int *framework_id_table )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter;

   AttachIterator( &Iter, framework_list );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      save_framework_export( directory, frame, framework_id_table );
   DetachIterator( &Iter );

   return;
}

void save_framework_export( char *pDir, ENTITY_FRAMEWORK *frame, int *framework_id_table )
{
   FILE *fp;
   int new_id;

   new_id = get_id_table_position( framework_id_table, frame->tag->id );
   if( ( fp = fopen( quick_format( "%s/%d.framework", pDir, new_id ), "w" ) ) == NULL )
   {
      bug( "%s: Unable to write framework (%d)%s.", __FUNCTION__, frame->tag->id, chase_name( frame ) );
      return;
   }
   fwrite_framework_export( fp, frame, framework_id_table );
   fclose( fp );
   return;
}

void fwrite_framework_export( FILE *fp, ENTITY_FRAMEWORK *frame, int *framework_id_table )
{
   fwrite_id_tag_export( fp, frame->tag, framework_id_table );

   fprintf( fp, "#FRAMEWORK\n" );
   fprintf( fp, "Name         %s~\n", frame->name );
   fprintf( fp, "Short_Descr  %s~\n", frame->short_descr );
   fprintf( fp, "Long_Descr   %s~\n", frame->long_descr );
   fprintf( fp, "Description  %s~\n", frame->description );

   fwrite_framework_content_list_export( fp, frame->fixed_contents, framework_id_table );
   fwrite_specifications( fp, frame->specifications, framework_id_table );

   fprintf( fp, "Inherits     %d\n", frame->inherits ? get_id_table_position( framework_id_table, frame->inherits->tag->id ) : -1 );

   if( f_script_exists( frame ) )
   {
      fprintf( fp, "Script\n" );
      fprintf( fp, "%s\n", print_f_script( frame ) );
      return;
   }
   fprintf( fp, "#END\n\n" );
   return;
}

void fwrite_framework_content_list_export( FILE *fp, LLIST *contents, int *framework_id_table )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter;
   int position;

   AttachIterator( &Iter, contents );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      if( ( position = get_id_table_position( framework_id_table, frame->tag->id ) ) != -1 )
         fprintf( fp, "FContent     %d\n", get_id_table_position( framework_id_table, frame->tag->id ) );
   }
   DetachIterator( &Iter );

   return;
}

/*
void export_project( PROJECT *project )
{
   LLIST *workspace_list;
   LLIST *framework_list;
   LLIST *instance_list;

   int *workspace_ids;
   int *instance_ids;
   int *framework_ids;

   if( !project )
   {
      bug( "%s: passed a NULL project.", __FUNCTION__ );
      return;
   }

   workspace_list = AllocList();
   framework_list = AllocList();
   instance_list = AllocList();

   copy_all_workspace_and_contents( project, workspace_list, framework_list, instance_list );
   copy_all_instance_frames_into_list_ndi( instance_list, framework_list );

   CREATE( workspace_ids, int, SizeOfList( workspace_list ) );
   CREATE( instance_ids, int, SizeOfList( instance_list ) );
   CREATE( framework_ids, int, SizeOfList( framework_list ) );

   swap_and_track_workspace_ids( workspace_list, workspace_ids );

}
*/

char *create_project_directory( PROJECT *project )
{
   DIR *directory;
   static char pDir[MAX_BUFFER];
   memset( &pDir[0], 0, sizeof( pDir ) );

   mud_printf( pDir, "../projects/%s-%s", project->name, smash_newline( ctime( &current_time ) ) );
   if( ( directory = opendir( pDir ) ) == NULL )
   {
      if( ( mkdir( pDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) ) != 0 ) /* was unsuccessful */
      {
         bug( "%s: unable to create directory: %s.", __FUNCTION__, pDir );
         return NULL;
      }
   }
   closedir( directory );
   return pDir;
}

void create_complete_framework_and_instance_list_from_workspace_list( LLIST *workspace_list, LLIST *instance_list, LLIST *framework_list )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   AttachIterator( &Iter, workspace_list );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
   {
      append_instance_lists_ndi( wSpace->instances, instance_list );
      append_framework_lists_ndi( wSpace->frameworks, framework_list );
   }
   DetachIterator( &Iter );

   append_instance_list_content_to_list_recursive_ndi( instance_list, instance_list );
   append_instance_list_framework_to_list_ndi( instance_list, framework_list );
   append_framework_list_content_to_list_recursive_ndi( framework_list, framework_list );
   append_framework_list_inheritance_to_list_recursive_ndi( framework_list, framework_list );
   return;
}

void append_instance_list_content_to_list_recursive_ndi( LLIST *instance_list, LLIST *append_list )
{
   ENTITY_INSTANCE *instance, *instance_content;
   ITERATOR Iter, IterTwo;

   AttachIterator( &Iter, instance_list );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      AttachIterator( &IterTwo, instance->contents );
      while( ( instance_content = (ENTITY_INSTANCE *)NextInList( &IterTwo ) ) != NULL )
      {
         if( !instance_list_has_by_id( append_list, instance_content->tag->id ) )
            AttachToList( instance_content, append_list );
         append_instance_list_content_to_list_recursive_ndi( instance_content->contents, append_list );
      }
      DetachIterator( &IterTwo );
   }
   DetachIterator( &Iter );

   return;
}

void append_instance_list_framework_to_list_ndi( LLIST *instance_list, LLIST *framework_list )
{
   ENTITY_INSTANCE *instance;
   ITERATOR Iter;

   AttachIterator( &Iter, instance_list );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      if( !framework_list_has_by_id( framework_list, instance->framework->tag->id ) )
         AttachToList( instance->framework, framework_list );
   }
   DetachIterator( &Iter );

   return;
}

void append_framework_list_content_to_list_recursive_ndi( LLIST *framework_list, LLIST *append_list )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter, IterTwo;

   AttachIterator( &Iter, framework_list );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      AttachIterator( &IterTwo, frame->fixed_contents );
      while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &IterTwo ) ) != NULL )
      {
         if( !framework_list_has_by_id( append_list, frame->tag->id ) )
            AttachToList( frame, append_list );
         append_framework_list_content_to_list_recursive_ndi( frame->fixed_contents, append_list );
      }
      DetachIterator( &IterTwo );
   }
   DetachIterator( &Iter );

   return;
}

void append_framework_list_inheritance_to_list_recursive_ndi( LLIST *framework_list, LLIST *append_list )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter;

   AttachIterator( &Iter, framework_list );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      while( ( frame = frame->inherits ) != NULL )
         if( !framework_list_has_by_id( append_list, frame->tag->id ) )
            AttachToList( frame, append_list );
   }
   DetachIterator( &Iter );

   return;
}

void copy_all_workspace_and_contents( PROJECT *project, LLIST *workspace_list, LLIST *framework_list, LLIST *instance_list )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( !project )
   {
      bug( "%s: passed a NULL project.", __FUNCTION__ );
      return;
   }
   if( !workspace_list )
   {
      bug( "%s: passed a NULL workspace_list.", __FUNCTION__ );
      return;
   }
   if( !framework_list )
   {
      bug( "%s: passed a NULL framework_list.", __FUNCTION__ );
      return;
   }
   if( !instance_list )
   {
      bug( "%s: passed a NULL instance_list.", __FUNCTION__ );
      return;
   }

   copy_workspaces_into_list( project->workspaces, workspace_list, FALSE, FALSE );

   /* instances first then frameworks per workspace, this could be factored later */
   AttachIterator( &Iter, workspace_list );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
   {
      copy_instance_list_ndi( wSpace->instances, instance_list );
      copy_framework_list_ndi( wSpace->frameworks, framework_list);
   }
   DetachIterator( &Iter );

   return;
}

void copy_all_instance_frames_into_list_ndi( LLIST *instance_list, LLIST *frame_list )
{
   ENTITY_INSTANCE *instance;
   ITERATOR Iter;

   if( !instance_list )
   {
      bug( "%s: passed a NULL instance_list.", __FUNCTION__ );
      return;
   }

   if( !frame_list )
   {
      bug( "%s: passed a NULL frame_list.", __FUNCTION__ );
      return;
   }

   AttachIterator( &Iter, instance_list );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      copy_framework_ndi( instance->framework, frame_list );
   DetachIterator( &Iter );

   return;
}

void project_newProject( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   PROJECT *project;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "How about a name for that new project?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( SizeOfList( olc->wSpaces ) > 0 )
   {
      text_to_olc( olc, "You can't have any workspaces loaded when creating a new project.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   project = init_project();
   if( new_tag( project->tag, olc->account->name ) != RET_SUCCESS )
   {
      text_to_olc( olc, "You could not get a new tag for your project, therefore, it was not created.\r\n" );
      free_project( project );
      olc_short_prompt( olc );
      return;
   }
   FREE( project->name );
   project->name = strdup( format_string_for_sql( arg ) );
   new_project( project );
   load_project_into_olc( project, olc );
   text_to_olc( olc, "You start a new project named: %s.\r\n", arg );
   return;
}

void project_openProject( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   PROJECT *project;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Open what project?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( ( project = load_project_by_name( arg ) ) == NULL )
   {
      text_to_olc( olc, "There is no project by the name %s.\r\n", arg );
      olc_short_prompt( olc );
      return;
   }

   load_project_into_olc( project, olc );
   text_to_olc( olc, "You open the %s project and load it into your olc.\r\n", project->name );
   return;
}

void project_exportProject( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   PROJECT *project;

   if( !arg || arg[0] == '\0' )
   {
      if( !olc->project )
      {
         text_to_olc( olc, "Export what project?\r\n" );
         olc_short_prompt( olc );
         return;
      }
      project = olc->project;
   }
   else
   {
      if( ( project = load_project_by_name( arg ) ) == NULL )
      {
         text_to_olc( olc, "There is no project by the name %s.\r\n", arg );
         olc_short_prompt( olc );
         return;
      }
   }

   export_project( project );
   text_to_olc( olc, "You export %s.\r\n", project->name );
   olc_short_prompt( olc );
   return;
}

void project_importProject( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   DIR *directory, *project_dir;
   DIR_FILE *file;
   char *dir_name = NULL;

   if( !arg || arg[0] == '\0' )
   {
      /* nanny stuff */
      text_to_olc( olc, "Import what project?\r\n - try \"ImportProject list\"\r\n" );
      return;
   }

   if( ( directory = open_projects_dir() ) == NULL )
      return;

   if( !strcmp( arg, "list" ) )
   {
      text_to_olc( olc, "Projects:\n\r" );
      for( file = readdir( directory ); file; file = readdir( directory ) )
      {
         if( !strcmp( file->d_name, "." ) || !strcmp( file->d_name, ".." ) )
            continue;
         text_to_olc( olc, " %s\n", file->d_name );
         olc_short_prompt( olc );
      }
   }
   else
   {
      for( file = readdir( directory ); file; file = readdir( directory ) )
      {
         if( !strcasecmp( arg, file->d_name ) )
         {
            dir_name = strdup( quick_format( "../projects/%s/", arg ) );
            break;
         }
      }
      if( !dir_name )
         text_to_olc( olc, "No such project to import.\r\n" );
      else
      {
         project_dir = opendir( dir_name );
         import_project( project_dir, dir_name );
         FREE( dir_name );
         closedir( project_dir );
         text_to_olc( olc, "Project Imported, feel free to open it.\r\n" );
      }
   }
   closedir( directory );
   return;
}
