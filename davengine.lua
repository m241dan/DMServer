 -- Load Global Modules
Server = require( "server" )
EventQueue = require( "eventqueue" )
DBuffer = require( "dbuffer" )
MudServerEvents = require( "events/server" )
MudSoul = require( "objects/mudsoul" )

-- Global Variables
global_events = {}

function main()
   print( "DavEngine starting..." )
   print( "Setting up Server!" )
   -- setup the server on port 6500
   mudserver = assert( Server:new( 6500 ), "could not bind mud server" )
   -- "start" simply means it will accept new connections when :accept() is called
   mudserver:start()

   -- setup our event for accepting new connections
   mudserver.acceptEvent = EventQueue.event:new( MudServerEvents.acceptNewConnections, EventQueue.second, { mudserver }, "Accepting new connections event." )
   EventQueue.insert( mudserver.acceptEvent )
   global_events[#global_events+1] = mudserver.acceptEvent

   -- setup the polling for new input, processing new input, pushing new input
   local poll_process_push = EventQueue.event:new( MudServerEvents.pollProcessPush, EventQueue.default_tick(), nil, "
   print( "Running the Event Queue!" )
   EventQueue.run()
   print( "DavEngine finished." )
   
end

main()
