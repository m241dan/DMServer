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
DM.__gc		= function( dm )-- garbage collection method

   -- if the socket is closable, close it
   if( dm.socket and dm.socket.close ) then
      dm.socket:close()
   end

end
DM.all 		= {}		-- all active DMs go here
DM.by_socket	= {}		-- all DMs indexed by their socket
DM.by_data	= {}		-- all DMs indexed by the data they are actively in control of

-- use a weak meta table for by_socket and by_data to let garbage collection know that
-- these two tables should not prevent the collection of their data
local wm = { __mode = "kv" }
setmetatable( DM.by_socket, wm )
setmetatable( DM.by_data, wm )

--constructor
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

   -- add manager to internal lists and return
   DM.all[#DM.all+1] = data_manager
   if( socket ) then DM.by_socket[socket] = data_manager
   return data_manager
end

--cleanup method(technically we let the garbage collector do the deleting)
function DM:delete()
   DM.all[DM.all:getKey( self )] = nil	-- removing itself from the masterlist should trigger garbage collection
   -- may be more added under this later... not sure yet
end

-------------------------
-- Data Adding Methods --
-------------------------

-- Raw Add
function DM:addData( data )
   -- a little sanity check
   if( self:contains( data ) then
      print( "DM:addData cannot add duplicate data." )
      return nil
   end

   local position = #self.data+1 -- easy reuse to just put this in a var

   self.data[position] = data
   return position
end
-- Returns Index

-- Add Data and set as primary index( AAS = add and set )
function DM:AASData( data )
   local position = self.addData( data )

   if( not position ) then
      printf( "DM:AASData cannot set to a nil index." )      
   end

   self:setData_byIndex( position )
end
-- Returns Index



---------------------------
-- Data Removing Methods --
---------------------------

-- Raw Removal
function DM:remData( data )
   local DI = self.data:getKey( data ) 	-- get the data's current index
   local CD = self.data[self.index] 	-- track current data(for later)
   -- remove the data
   self.data[DI] = nil
   -- clear out the "history"
   for k, v in ipairs( self.prev ) do
      if( v == DI ) then
         table.remove( self.prev, k )
      end
   end
   if( DI == self.index )
      local NIP = #self.prev -- "new index position" the position in the prev that has its new index
      if( #self.prev == 0 ) then
         self:delete()
      else
         self.index = self.prev[NIP]
         table.remove( self.prev, NIP )
      end
   else
      self.current = self.data:getKey( CD )
   end
end

---------------------------
-- Data Settings Methods --
---------------------------

-- Set via Index, raw method
function DM:setData_byIndex( index )
   -- sanity check
   if( type( index ) ~= "number" ) then
      printf( "DM:setData_byIndex: index values can only be numbers." )
      return
   end

   -- manage prev, ie "history"
   if( self.index ~= 0) then
      self.prev[#self.prev+1] = self.index
   end
   self.index = index
end

-- Set via Data(look it up, then set it)
function DM:setData_byData( data )
   local index = self.data:getKey( data )
   if( not index ) then
      printf( "DM:setData_byData cannot set to data, DM does not contain the specific data you are trying to set to." )
      return
   end
   self:setData_byIndex( index )
end

--------------------------
-- Data Utility Methods --
--------------------------

-- check to see if DM contains this data
function DM:contains( data )
   for _, d in pairs( self.data ) do
      if( d == data ) then
         return true
      end
   end
   return false
end
-- returns bool

return DM
