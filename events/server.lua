-- a collection of events particular to the server

local M = {}

function M.acceptNewConnections( server )
   local new_client = server:accept()
   if( new_client ) then
      new_client:send( "You have connected!" )
   end
   return 1000;
end

return M

