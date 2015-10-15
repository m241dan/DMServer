-- returns the amount of time it takes for this corpse to decay based on the instance it was passed --
function corpseDecay( instance )
--    need a standard 3 stage decay, on 7.5minutes for mobs and 30 mins for players.
   return standard_corpse_decay
end

-- move the items from the instance to the corpse
-- the reason this is scripts is to handle any exceptions
-- for example, maybe unique items don't go to your corpse
function inventoryToCorpse( instance, corpse )
   local item
--    need all mobs to load worn, and carried equipment, except X flagged stuff.
--    need players to lose nothing, except eventually all carried gold, and 10% "exp loss" for mob death, and 5% for player death.
--    quest items should go to corpses too i think. adds a nice tweak for later.
   for item in instance:eachInventory() do
      item:to( corpse )
   end
end


