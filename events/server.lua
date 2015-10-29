-- a collection of events particular to the server

local M = {}

M.accept_new = 1000
M.ppp_interval = 100

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
   -- poll clients for new input
   server:poll()
   -- interpret client input using the associated statemanager's current state's interpreter
   for index, client in ipairs( server.connections ) do
      local sm = StateManager.managers_by_client[client]
      for _, ln in ipairs( client.inbuf ) do
         sm.states[sm.current].interpreter( ln )
      end
      client.inbuf = {}
   end
   -- push client outbufs to client
   server:push()
   return M.ppp_interval
end

return M

