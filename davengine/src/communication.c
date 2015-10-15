/* communcation.c: methods pertaining to global communication written by Davenge */

#include "mud.h"

int communicate( int level, ACCOUNT_DATA *speaker, const char *message )
{
   ACCOUNT_DATA *account;
   ITERATOR Iter;
   char comm_tag_self[MAX_BUFFER];
   char comm_tag_oth[MAX_BUFFER];
   int act_level;
   int ret = RET_SUCCESS;

   if( !message || message[0] == '\0' )
   {
      BAD_POINTER( "message" );
      return ret;
   }

   if( !speaker )
   {
      BAD_POINTER( "speaker" );
      return ret;
   }

   switch( level )
   {
      default:
         bug( "%s: unknown global communication level: %d", __FUNCTION__, level );
         return RET_FAILED_OTHER;
      case CHAT_LEVEL:
         mud_printf( comm_tag_oth, "chats" );
         mud_printf( comm_tag_self, "chat" );
         act_level = LEVEL_BASIC;
         break;
   }

   AttachIterator( &Iter, account_list );
   while( ( account = NextInList( &Iter ) ) != NULL )
   {
      if( act_level > account->level )
         continue;

      if( account == speaker )
      {
         text_to_account( account, "#PYou %s, '%s'#n\r\n", comm_tag_self, message );
         continue;
      }

      text_to_account( account, "#P%s %s, '%s'#n\r\n", speaker->chatting_as[0] != ' ' ? speaker->chatting_as : speaker->name, comm_tag_oth, message );
   }

   return ret;
}
