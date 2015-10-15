-- a collection of events particular to the server

local M = {}

M.accept_new = 1000

function M.acceptNewConnections( server )
   local new_client = server:accept()
   if( new_client ) then
      new_client:send( "You have connected!" )
      local mS = MudSoul:new( new_client )
   end
   return M.accept_new;
end

return M

