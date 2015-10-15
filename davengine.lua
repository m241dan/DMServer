-- Load Global Modules
Server = require( "modules/davserver/davserver" )
EventQueue = require( "modules/davevent/eventqueue" )
DBuffer = require( "modules/davbuffer/dbuffer" )
MEvents = require( "events/server" )
-- handle the awkard mysql3 module
local driver = require( "luasql.sqlite3" )
local sqlenv = driver.sqlite3()

function main()
   mudserv = assert( Server:new( 6500 ), "could not bind mud server" )
   mudserv:start()
   
end
