---------------------------------------------------------------------------
-- Data Manger Library, written by Daniel R. Koris                       --
---------------------------------------------------------------------------
-- The Data Manger Library acts as a container for all the data. Its job --
-- is to hold live data and its two controlling entities: the socket,    --
-- which can be a TCP/UDP(functionlity coming later for UDP) socket or   --
-- a rig of pretedermined functions, and the interpreter. These two      --
-- entities send messages back and fourth with each other. This          --
-- communication changes the shape and content of the data and can       --
-- interact with the event queue.                                        --
---------------------------------------------------------------------------


local DM = {}

DM.__index 	= DM		-- for OOP mimicing
DM.all 		= {}		-- all active DMs go here
DM.by_sockets	= {}		-- all DMs indexed by their socket
DM.by_data	= {}		-- all DMs indexed by the data they are actively in control of

-- use a weak meta table for by_sockets and by_data to let garbage collection know that
-- these two tables should not prevent the collection of their data
local wm = { __mode = "kv" }
setmetatable( DM.by_sockets, wm )
setmetatable( DM.by_data, wm )

function DM:new( socket )
   -- create the data_manager table and set this library as its metatable
   local data_manager = {}
   setmetatable( data_manager, self )

   -- setup data_manager 
   data_manager.socket 	= socket
   data_manager.data 	= {} 		-- table to hold multiple pieces of data
   data_manager.index 	= 0		-- current index, what data is in use, if zero the DM has no data
   data_manager.prev    = { 0 }		-- use a table to act as a linked list to track your way back through the data stack	

   -- setup the data indexed tables
   data_manager.interpreter = {}	-- a table for interpreters

   DM.all[#DM.all+1] = data_manager
   return data_manager
end

return DM
