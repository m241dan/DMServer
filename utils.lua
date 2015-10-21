local utils = {}

function utils.requireCheck( rpath )
   if( not rpath ) then
      error( "requireCheck failed: nil path", 1 )
      return false
   end
   rpath = rpath .. ".lua"
   if( not LFS.attributes( rpath ) ) then
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

local function serialize( data, indent_amount )
   if( type( data ) == "string" ) then
      return string.format( "%q", data ) 
   elseif( type( data ) == "number" ) then
      return tostring( data )
   else
      if( not data.serialize ) then
         local tab_str, str = { "{" }, nil
         indent_amount = indent_amount + 3
         for k, v in pairs( data ) do
            str = string.format( "%s[%s] = %s,", string.rep( " ", indent_amount ), serialize( k, indent_amount ), serialize( v, indent_amount ) )
            tab_str[#tab_str+1] = str
         end
         tab_str[#tab_str+1] = string.format( "%s}", string.rep( " ", indent_amount - 3 ) )
         return table.concat( tab_str, "\n" )
      else
         return data:serialize()
      end
   end
end

function utils.save( data, file )
   file:write( "return " )
   file:write( serialize( data, 0 ) )
end

return utils

