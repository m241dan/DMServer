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

function A:load( account_path )
   local account = dofile( "accounts/" .. account_path .. ".lua" )"
   setmetatable( account, self )
   self.__index = self

   return account
end

function A:serialize()
   return "Account:load( \"" .. self.name .. "\" )"
end

return A
