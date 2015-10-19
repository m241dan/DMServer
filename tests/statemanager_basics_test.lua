dofile( "mudlibs.lua" )

local behaviour = {}

local function itp( state, data )
   while true do
      ::top::
      input = coroutine.yield()
      if( not input ) then
         local sm = managers_by_state[state]
         print( "Managers by state size " .. #managers_by_state )
         sm:remState( state )
      elseif( input == "dttest" ) then
         print( data.dt )
         goto top
      end
      print( input )
   end
   return
end

function behaviour.getInterp()
   return coroutine.wrap( itp )
end

function main()
   local sm = StateManager:new()
   local state = State:new( { dt = "hi, I'm dt!" }, behaviour, behaviour.getInterp() )
   sm:addState( sm )
   print( managers_by_state )
   print( "Managers by state size " .. #managers_by_state )
   print( "state added" )
   state.interpreter( "Hi" )
   state.interpreter( "Testing!" )
   state.interpreter( "dttest" )
   state.interpreter( nil )
end

main()
StateManager.dataDump()
