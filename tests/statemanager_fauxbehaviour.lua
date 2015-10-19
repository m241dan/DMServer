local B = {}

local function itp( state, data )
   local input
   while true do
      ::top::
      input = coroutine.yield()
      if( not input ) then goto top; end
      state:putToOutbuf( input )
   end
end

function B.getInterp()
   return coroutine.wrap( itp )
end

return B
