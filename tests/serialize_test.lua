dofile( "mudlibs.lua" )

td = {}

td.name = "Hi"
td.level = 12
td.behaviour = "this/that/script.lua"
td.stats= { str = 10, dex = 10, agi = 10 }
td[1] = "numberic index test"
td.pho_account_test = { }
function td.pho_account_test:serialize()
   return string.format( "%q", "Hi there" )
end

file = io.open( "tests/.serialized_data.lua", "w+" )
Utils.save( td, file )
file:flush()
file:close()

td = dofile( "tests/.serialized_data.lua" )
print( "Loaded the chunk with no lua noticable errors!\n" )
for line in io.lines( "tests/.serialized_data.lua" ) do
   print( line )
end
