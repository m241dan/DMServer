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
      state:putToOutbuf( "There is no account with that name, would you like to make one?[Yes/No]\n" )
      input = coroutine.yield()
   else
      state:putToOutbuf( "Cannot load existing accounts yet!" )
   end

   local yesno = input:lower() == "yes"

   state:putToOutbuf( "yesno is " .. tostring( yesno ) )
   return "dead"
end

return coroutine.create( interpreter )
