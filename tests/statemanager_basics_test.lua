dofile( "mudlibs.lua" )

local behaviour = {}

local function itp( state, data )
   while true do
      ::top::
      input = coroutine.yield()
      if( not input ) then
         local sm = StateManager.managers_by_state[state]
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
   sm:addState( state )
   state.interpreter( "Hi" )
   state.interpreter( "Testing!" )
   state.interpreter( "dttest" )
   state.interpreter( nil )
end

main()
collectgarbage()
StateManager.dataDump()
