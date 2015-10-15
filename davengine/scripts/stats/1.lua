function onStatGain( mob, amount )
   mud.bug( string.format( "Gain Amount %d", amount ) )
   mob:addStatMod( "Attack", amount * 2 )
end

function onStatLose( mob, amount )
   mud.bug( string.format( "Lose Amount %d", amount ) )
   mob:addStatMod( "Attack", amount * 2 )
end
