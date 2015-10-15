-- a collection of events particular to the server

local M = {}

function M.acceptNewConnections( server )
   local new_client = server:accept()
   new_client:send( "You have connected!" )
   return 1000;
end

return M
