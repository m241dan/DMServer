function dodgeChance( attacker, dodger )
--    players have an evasiveness bonus, and accuracy bonus, so some sort of formula.
   return 0
end

function parryChance( attacker, parryr )
--    players have a parry bonus, and hit bonus, so some sort of formula.
   return 0
end

function missChance( attacker, victim )
--    all misses will have a default of 15%. do formula.
   return 0
end

-- calculate any factors effecting melee cooldowns
function meleeCooldown( attacker )
--    need a -slightly- random cooldown here for a default.
   return automelee_delay
end

-- design the duration/frequency parts of the damage object
function prepMeleeTimer( attacker, timer )
   timer:setDuration( 1 )
   timer:setFrequency( 1 )
   timer:setCounter( 0 )
end

-- check to see if the attacker can atk the victim, ie same room? can see? etc etc --
-- return anything other than a string if you can attack, otherwise return the reason --
-- for example "%s is not out of range" or "%s is not here" whatever you want chummy --
function meleeCheck( attacker, victim )
   if( attacker:isSameRoom( victim ) == true ) then
      return true
   else
      local ret = string.format( "%s is not here.", vicitm:getShort() )
      return ret
   end
end

-- figure out the damage amount portion of the damage object
function prepMeleeDamage( attacker, damage ) 
--    okay so for melee i need a tier system. will "hard" script it here, but should maybe have it as a player stat too.
--    attacker strength and tier versus defenders strength and tier will determine it. to keep things fun and flowing nicely
--       we will give the attackers a slightly higher bonus on purpose. might even do something even "faster paced" for pvp later.
--    want to keep everything, "slightly random", but evenly for everyone, so need to make sure we have random rolls on everything.
   if( attacker:isBuilder() ) then damage:setAmount( 100 ) return end
   damage:setAmount( 1 )
end

-- analyze the damage object sent to the defender
-- calculator any the actual damage that the object does
-- note that any elemental damage will be handled by C
function onReceiveDamage( defender, damage )
end

-- return order attacker -> defender -> room
function combatMessage( attacker, defender, damage, status )
   local atk_msg
   local def_msg
   local room_msg

   if( status == HIT_SUCCESS ) then
      atk_msg = string.format( "You attack %s doing %d damage.", defender:getShort(), damage:getAmount() )
      def_msg = string.format( "%s attacks you doing %d damage.", attacker:getShort(), damage:getAmount() )
      room_msg = string.format( "%s attacks %s.", attacker:getShort(), defender:getShort() )
   elseif( status == HIT_DODGED ) then
      atk_msg = string.format( "You miss %s with your attack.", defender:getShort() )
      def_msg = string.format( "%s misses you with their attack.", attacker:getShort() )
      room_msg = string.format( "%s dodges %s's attack.", attacker:getShort(), defender:getShort() )
   elseif( status == HIT_PARRIED ) then
      atk_msg = string.format( "%s parries your attack.", defender:getShort()  )
      def_msg = string.format( "You parry %s's attack.", attacker:getShort() )
      room_msg = string.format( "%s parries %s's attack.", defender:getShort(), attacker:getShort() )
   elseif( status == HIT_MISSED ) then
      atk_msg = string.format( "You miss %s with your attack.", defender:getShort() )
      def_msg = string.format( "%s misses you with their attack.", attacker:getShort() )
      room_msg = string.format( "%s misses %s with their attack.", attacker:getShort(), defender:getShort() )
   end
   return atk_msg, def_msg, room_msg
end
