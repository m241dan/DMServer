function nannyInterp( nanny, argument )
   if( argument:lower() == "hello" ) then
      nanny:echoAt( "Hi there\n" )
      nanny:finish()
   else
      nanny:echoAt( "How rude\n" )
   end
end
