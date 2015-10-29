local B = {}

local function interpreter( state, data )
   local input, acct
   ::top::
   input = coroutine.yield()
   if( not input ) then
      state:putToOutbuf( "You must enter something." )
      goto top
   end
      
   acct = Account:load( input )
   if( not acct ) then
      state:putToOutbuf( "There is no account with that name, would you like to make one?[(Y)es/(N)o]\n" )
      input = coroutine.yield()
   else
      state:putToOutbuf( "Cannot load existing accounts yet!" )
   end
end

function B.init( state )
   state:putToOutbuf( "\nHello and welcome to Davengine!\nWhat is your account name?" )
end


function B.getInterp()
   return coroutine.wrap( interpreter )
end

return B
