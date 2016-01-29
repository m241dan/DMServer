---------------------------------------------------------------------------
-- Data Manger Library, written by Daniel R. Koris                       --
---------------------------------------------------------------------------
-- The Data Manger Library acts as a container for all the data. Its job --
-- is to hold live data and its two controlling entities: the socket,    --
-- which can be a TCP/UDP(functionality coming later for UDP) socket or  --
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
   -- create the dm table and set this library as its metatable
   local dm = {}
   setmetatable( dm, self )

   -- setup dm 
   dm.socket 	= socket
   dm.data 	= {} 		-- table to hold multiple pieces of data
   dm.index 	= 0		-- data at this index is currently "active", 0 means it has no data
   dm.prev      = {}		-- use a table to act as a linked list to track your way back through the data stack	

   -- setup the data indexed tables
   dm.interpreter = {}	-- a table for interpreters

   -- add manager to internal lists and return
   DM.all[#DM.all+1] = dm
   if( socket ) then DM.by_socket[socket] = dm; end
   return dm
end

--cleanup method(technically we let the garbage collector do the deleting)
function DM:delete()
   DM.all[table.getKey( DM.all, self )] = nil	-- removing itself from the masterlist should trigger garbage collection
   -- may be more added under this later... not sure yet
end

-------------------------
-- Data Adding Methods --
-------------------------

-- Raw Add
function DM:addData( data )
   -- a little sanity check
   if( table.contains( self.data, data ) ) then
      print( "DM:addData cannot add duplicate data." )
      return nil
   end

   local position = #self.data+1 -- easy reuse to just put this in a var

   self.data[position] = data
   DM.by_data[data] = self
   return position
end
-- Returns Index

-- Add Data and set as primary index( AAS = add and set )
function DM:AASData( data )
   local position = self:addData( data )

   if( not position ) then
      print( "DM:AASData cannot set to a nil index." )      
   end

   self:setData_byIndex( position )
end
-- Returns Index



---------------------------
-- Data Removing Methods --
---------------------------

-- Raw Removal
function DM:remData( data )
   local DI = table.getKey( self.data, data ) 	-- get the data's current index
   local CD = self.data[self.index] 		-- track current data(for later)
   -- remove the data
   self.data[DI] = nil
   DM.by_data[data] = nil
   -- clear out the "history"
   for k, v in ipairs( self.prev ) do
      if( v == DI ) then
         table.remove( self.prev, k )
      end
   end
   if( DI == self.index ) then
      local NIP = #self.prev			-- "new index position" the position in the prev that has its new index
      if( NIP == 0 ) then			-- if we don't have a prev, do we have any data?
         if( #self.data == 0 ) then		-- if we don't have any data... delete
            self:delete()
         else
            self.index = #self.data		-- if we do have some data, start at the most recent one
         end
      else
         self.index = self.prev[NIP]
         table.remove( self.prev, NIP )
      end
   else
      self.current = table.getKey( self.data, CD )
   end
end

---------------------------
-- Data Settings Methods --
---------------------------
-- Methods for setting what data is currently "active"

-- Set via Index, raw method
function DM:setData_byIndex( index )
   -- sanity check
   if( type( index ) ~= "number" ) then
      print( "DM:setData_byIndex: index values can only be numbers." )
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
   local index = table.getKey( self.data, data )
   if( not index ) then
      print( "DM:setData_byData cannot set to data, DM does not contain the specific data you are trying to set to." )
      return
   end
   self:setData_byIndex( index )
end

--------------------------
-- Data Utility Methods --
--------------------------

-- a little helper function to setup interpreters, will try to use an interpreter path in the data or you can pass it one
function DM:setupInterp( data, i_path )
   local interp, target
   if( not i_path ) then
      if( not data.interpreter ) then
         print( "DM:setupInterp failed because there was no interpreter passed and the data has no interpreter path accompanying it." )
         return false
      end
      target = data.interpreter
   else
      target = i_path
   end
   interp = DRoutine:new()
   interp:wrap( target )
   interp( self, data )
   self.interpreter[data] = interp
   return true
end

-- simplification wrapper, with some sanity
function DM:interp( ... )
   local interpreter = self.interpreter[self.data[self.index]]

   if( not interpreter ) then
      print( "Current Data has no interpreter" )
      return
   end
   interpreter( ... )
end

function DM.dataDump()
   print( "          Number of total DMs: " .. table.getn( DM.all ) )
   print( "  Number of total DMs_by_data: " .. table.getn( DM.by_data ) )
   print( "Number of Total DMs_by_socket: " .. table.getn( DM.by_socket ) )
end

return DM
