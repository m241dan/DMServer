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
   return true
end

function S:getIndex( entity )
   for i, e in ipairs( self.entity ) do
      if( e == entity )
         return i
      end
   end
   return nil
end

function S:remEntity( entity )
   for i, e in ipairs( self.entity ) do
      if( entity == e ) then
         table.remove( self.entity, i )
         return true
      else
   end
   return false
end

function S:remEntityByIndex( index )
   if( not self.entity[index] ) then
      return false
   end
   table.remove( self.entity, index )
   return true
end

function S:setIndex( index )
   if( self.entity_index[index] ) then
      self.entity_index = index
      return true
   end
   return false
end

return S
