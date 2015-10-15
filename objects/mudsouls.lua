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

   -- add it to the global module
   S.souls[#S.souls+1] = s

   return s
end

return S
