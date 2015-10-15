function onCall( account, argument )
   account:echoAt( "It works\n\n" )
   account:echoAt( string.format( " Echo: %s\n", argument ) )
end
