 -- Load Global Modules
dofile( "libs.lua" )

-- Global Variables
global_events = {}

function main()
   print( "DavEngine starting..." )
   print( "Setting up Server!" )

   -- setup the server on port 6500
   server = assert( Server:new( 6500 ), "could not bind mud server" )
   -- "start" simply means it will accept new connections when :accept() is called
   server:start()

   -- setup our event for accepting new connections
   server.accept_event = Event:new( ServerEvents.acceptNewConnections, EventQueue.second, { server }, "Accepting new connections event." )
   EventQueue.insert( server.accept_event )

   server.poll_process_push = Event:new( ServerEvents.pollProcessPush, 100, { server }, "Poll. Process. Push." )
   EventQueue.insert( server.poll_process_push )

   -- setup the polling for new input, processing new input, pushing new input
--   local poll_process_push = EventQueue.event:new( MudServerEvents.pollProcessPush, EventQueue.default_tick(), nil, "
   print( "Running the Event Queue!" )
   EventQueue.run()
   print( "DavEngine finished." )
   
end

main()
