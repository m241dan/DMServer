dofile( "libs.lua" )

local function itp( dm, data )
   while true do
      ::top::
      input = coroutine.yield()
      if( not input ) then
         dm:remData( data )
      elseif( input == "dttest" ) then
         print( data.dt )
         goto top
      end
      print( input )
   end
   return
end

-- Create Manater
-- Create Data
-- Add and Set data(set makes it the current data)
-- Set up an interpreter using the function we wrote above: itp()
-- Send it some stuff
function main()
   local dm = DataManager:new()
   local data = { dt = "hi, I'm dt!" }
   dm:AASData( data )
   dm:setupInterp( data, itp )
   
   dm:interp( "Hi" )
   dm:interp( "Testing!" )
   dm:interp( "dttest" )
   dm:interp( nil )
end

function second_main()
   local dm = DataManager:new()
   local d1 = { dt = "hi, I'm dt!" }
   local d2 = { dt = "hi, I'm dt!" }
   local d3 = { dt = "hi, I'm dt!" }
   local d4 = { dt = "hi, I'm dt!" }
   dm:AASData( d1 )
   dm:setupInterp( d1, itp )
   dm:addData( d2 )
   dm:setupInterp( d2, itp )
   dm:addData( d3 )
   dm:setupInterp( d3, itp )
   dm:addData( d4 )
   dm:setupInterp( d4, itp )

   dm:remData( d1 )
   dm:interp( "Hi" )
   dm:interp( "Testing!" )
   dm:interp( "dttest" )
   dm:setData_byData( d3 )
   dm:interp( "Hi" )
   dm:interp( "Testing!" )
   dm:interp( "dttest" )
   dm:interp( nil )
end

print( "Starting Test One\n" )
main()
collectgarbage( "collect" )
DataManager.dataDump()
print( "The dump should read all 0s!" )

print( "\nStarting Test Two\n" )
second_main()
collectgarbage()
DataManager.dataDump()
print( "The dump should read 1 all manager and 2 by data!" )
