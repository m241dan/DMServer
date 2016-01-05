dofile( "libs.lua" )

local function itp( dm, data )
   while true do
      ::top::
      input = coroutine.yield()
      if( not input ) then
         dm:remData( data )
      elseif( input == "dttest" ) then
         print( data.dt )
         goto top
      end
      print( input )
   end
   return
end

-- Create Manater
-- Create Data
-- Add and Set data(set makes it the current data)
-- Set up an interpreter using the function we wrote above: itp()
-- Send it some stuff

function main()
   local dm = DataManager:new()
   local data = { dt = "hi, I'm dt!" }
   dm:AASData( data )
   dm:setupInterp( data, itp )
   
   dm:interpreter[data]( "Hi" )
   dm:interpreter[data]( "Testing!" )
   dm:interpreter[data]( "dttest" )
   dm:interpreter[data]( nil )
end
--[[
function second_main()
   local sm = StateManager:new()
   local state = State:new( { dt = "hi, I'm dt!" }, behaviour, behaviour.getInterp() )
   sm:addState( state )
   state.interpreter( "Hi" )
   state.interpreter( "Testing!" )
   state.interpreter( "dttest" )
end
--]]

print( "Starting Test One\n" )
main()
collectgarbage()
StateManager.dataDump()
print( "The dump should read four 0s" )

--[[
print( "\nStarting Test Two\n" )
second_main()
collectgarbage()
StateManager.dataDump()
print( "The dump should read three 1s, 0 on client" )
--]]
