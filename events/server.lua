-- a collection of events particular to the server

local M = {}

M.accept_new = 1000

function M.acceptNewConnections( server )
   local nc, sm, s

   nc = server:accept()
   if( nc ) then
      nc:send( "You have connected!" )
      sm = StateManager:new( nc )
      s = StateManager.state:new( { behaviour = "behaviours/new_connection" } )
      sm:addState( s )
      s.behaviour.init( s )
   end
   return M.accept_new;
end

function M.pollProcessPush( server )
end

return M

