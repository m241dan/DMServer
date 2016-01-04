local function interpreter( dm, data )
   local input
   ::top::
   input = coroutine.yield()
   goto top
end

return coroutine.wrap( interpreter )
