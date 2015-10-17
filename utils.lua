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

return utils
