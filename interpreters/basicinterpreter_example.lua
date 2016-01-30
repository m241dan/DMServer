-- this is not meant to work, it's purely a "framework" example

local function interpreter( dm, data )
   local input
   ::top::
   input = coroutine.yield()
   goto top
end

return coroutine.create( interpreter )
