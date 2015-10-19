dofile( "mudlibs.lua" )

local dt = { behaviour = "tests/statemanager_fauxbehaviour" }
local fauxclient = { outbuf = {} }

function fauxclient:close()
   print( "Dumping outbuf..." )
   for _, msg in pairs( self.outbuf ) do
      print( msg )
   end
   print( "Closing client" )
end

function main()
   local sm = StateManager:new( fauxclient )
   local state = State:new( dt )
   sm:addState( state )
   state.interpreter( "Echo... echo... echo..." )
   state.interpreter( "Bingpo'n!" )
   state.interpreter( "Osu!!!" )
   sm:remState( state )
end

print( "Starting Test..." )
main()
print( "Pre-Garbage Collection" )
StateManager.dataDump()
collectgarbage()
print( "Post-Garbage Collection" )
StateManager.dataDump()

print( "\nThis should end with the messages dumping, closing client and 1 client left in the Size of Managers By Client, just the nature of garbage collection" )
