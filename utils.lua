local utils = {}

function utils.requireCheck( path )
   if( not path ) then
      error( "requireCheck failed: nil path", 1 )
      return false
   end
   if( not LFS.attribute( path ) ) then
      error( "requireCheck failed: path is not a file", 1 )
      return false
   end
   return true
end

function table.getn( table )
   local count = 0
   for _, _ in pairs( table ) do count = count + 1; end
   return count
end

return utils
