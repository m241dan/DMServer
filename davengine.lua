-- Load Global Modules
Server = require( "server" )
EventQueue = require( "eventqueue" )
DBuffer = require( "dbuffer" )
MudServerEvents = require( "events/server" )
-- handle the awkard mysql3 module

mudsouls = {}

function main()
   print( "DavEngine starting..." )
   print( "Setting up Server!" )
   -- setup the server on port 6500
   mudserver = assert( Server:new( 6500 ), "could not bind mud server" )
   -- "start" simply means it will accept new connections when :accept() is called
   mudserver:start()
   -- setup our event for accepting new connections
   mudserver.acceptEvent = EventQueue.event:new( MudServerEvents.acceptNewConnections )
   mudserver.acceptEvent:setArgs( mudserver )
   mudserver.acceptEvent.execute_at = EventQueue.time() + ( EventQueue.second * 5 )
   mudserver.acceptEvent.name = "Accepting new connections event."
   EventQueue.insert( mudserver.acceptEvent )

   print( "Running the Event Queue!" )
   EventQueue.run()
   print( "DavEngine finished." )
   
end

main()
