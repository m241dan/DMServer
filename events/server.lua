-- a collection of events particular to the server

local M = {}

M.accept_new = 1000
M.ppp_interval = 2000

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
   local status
   -- poll clients for new input
   server:poll()
   -- interpret client input using the associated statemanager's current state's interpreter
   for index, client in ipairs( server.connections ) do
      local sm = StateManager.managers_by_client[client]
      for _, ln in ipairs( client.inbuf ) do
         status = sm.states[sm.current].interpreter( ln )
         if( status == "dead" ) then
            print( "removing state" )
            sm:remState( sm.states[sm.current] )
            goto next_client
         end
      end
      client.inbuf = {}
      ::next_client::
   end
   -- push client outbufs to client
   server:push()
   StateManager.dataDump()
   print( "Number of connections " .. table.getn( mudserver.connections ) );
   return M.ppp_interval
end

return M

