function getRaceName( id )
   for index, race in pairs( race_table ) do
      if( race[2] == id ) then return race[1] end
   end
end

function playerLogin( account, character )
   local socket = Socket.getSocket( account )
   account:setControlling( character )
   socket:setControlling( character )
   socket:changeState( 9 )
   character:echoAt( "You log in.\n" )
   character:to( 306 )
   character:interp( "look" )
end
