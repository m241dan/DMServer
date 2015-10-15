local S = {}

S.souls = {}

function S:new( client )
   local s = {}
   setmetatable( s, self )
   self.__index = self

   -- some default values
   s.client = client
   s.entity = {}
   s.entity_index = 0
   s.previous_index = nil
   -- add it to the global module
   S.souls[#S.souls+1] = s

   return s
end

function S:addEntity( entity )
   self.entity[#self.entity+1] = entity
   return #self.entity
end

function S:remEntity( entity )
end

function S:remEntityByIndex( index )
end

function S:setIndex( index )
   if( self.entity_index[index] ) then
      self.entity_index = index
      return true
   end
   return false
end

return S
