local A = {}

function A:new()
   local account = {}
   setmetatable( account, self )
   self.__index = self

   account.name = "new_account"
   account.passwd = "passwd"
   account.level = 1
   account.chars = {}

   return account
end

function A:load( account_name )
   local account_path = "accounts/" .. account_name .. ".lua"

   if( not LFS.attributes( account_path ) ) then
      return nil
   end

   local account = dofile( account_path )
   if( type( account ) ~= "table" ) then
      error( "bad account file, no table returned", 2 )
   end   
   setmetatable( account, self )
   self.__index = self

   return account
end

function A:save()
   local file = io.open( string.format( "accounts/%s.lua", self.name ), "w+" )
   Utils.save( self, file )
   file:close()
end

function A:serialize()
   return "Account:load( \"" .. self.name .. "\" )"
end

return A
