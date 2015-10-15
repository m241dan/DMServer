function onCall( account, argument )
   local nanny = Nanny.new( "../scripts/nannys/charcreatenanny.lua" )
   nanny:setState( 0 )
   nanny:setControl( account )
   account:echoAt( "You begin character creation...\n" )
   nanny:start()
   return
end
