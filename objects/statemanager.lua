--------------------------------
-- State Manager Module       --
-- Written by Daniel R. Koris --
--------------------------------
-- The State Manager is at the heart of our data management.

local function SMGarbage( state_manager )
   if( state_manager.client ) then
      state_manager.client:close()
   end
end

local SM = { __gc = SMGarbage }

--------------------------------
-- State Manager Locals       --
-- Written by Daniel R. Koris --
--------------------------------
SM.managers_by_client = {}
SM.managers_by_state = {}
SM.managers_all = {}
SM.states_by_data = {}

-- make these weak tables so they don't hold up garbage collection
wm = { __mode = "kv" }
setmetatable( SM.managers_by_client, wm )
setmetatable( SM.managers_by_state, wm )
setmetatable( SM.states_by_data, wm )

---------------------------------
-- Local Function validIndex() --
-- Written by Daniel R. Koris  --
---------------------------------
-- This checks the given index against the given State Manager to test if its valid
-- If the index is a number, it simply checks for nil.
-- If the index is a state, it checks to see if it exists and returns its integer index

local function validIndex( state_manager, index )
   -- if the index is a number, its simple, does that exist? return true and the type
   if( type( index ) == "number" and state_manager.states[index] ) then
      return true
   end

   -- we can also use the index as a state, in this case, we need to do a little lookup magic, but make sure its a state first
   if( type( index ) == "table" and getmetatable( index ) == SM.state ) then
      for i, s in ipairs( self.states ) do
         if( s == index ) then
            return true, i
         end
      end
   end
   return false
end

local function getClient( state_or_data )
   local mt, sm, client
   mt = getmetatable( state_or_data )
   if( mt == SM.state ) then
      sm = SM.managers_by_state[state_or_data]
      if( not sm ) then return nil; end
      return sm.client
--   elseif( mt == Account or mt == Entity ) then -- will write this one later
   end
   return nil
end

--------------------------------
-- State Manager States       --
-- Written by Daniel R. Koris --
--------------------------------

--------------------------------
-- State:new()                --
-- Written by Daniel R. Koris --
--------------------------------
-- A simple constructor for States, which act as wrappers for data
-- When we wrap data in a state we can give it a interpreter and a behaviour

-- The Interpreter is a function that will handle input interpreting
-- It could be a wrapped coroutine or a regular function
-- Note: if its a wrapped coroutine, it will likely need to be initialized before you use it



SM.state = {}

function SM.state:new( data, behaviour, interpreter )
   local state = {}
   setmetatable( state, self )
   self.__index = self

   state.data = data or {} -- what data will this state manage? an account? state specific variables? an entity?

   -- Setup this state's behaviour and interpreter. If one was passed, use that be default.
   -- If one is not passed, well... Look for a behaviour string in the data.
   -- If a state doesn't have a behaviour, it cannot be, return nil
   if( behaviour ) then
      state.behaviour = behaviour
   else
      if( not Utils:requireCheck( data.behaviour ) ) then
         return nil
      end
      state.behaviour = require( data.behaviour )     
   end

   if( interpreter ) then
      state.interpreter = interpreter
   else
      state.interpreter = data.behaviour.getInterp()
   end

   -- interpreters need to be initialized with the state and the data
   state.interpreter( state, state.data )

   -- any data set can only have one state managing it
   local previous_state = SM.states_by_data[data]
   if( previous_state ) then
      previous_state.behaviour.takeOver( previous_state, state, data ) -- takeOver() handles any messaging and data specific operations before takeover occurs
      SM.managers_by_state[previous_state]:remState( previous_state )
   end
   
   SM.states_by_data[data] = state
   return state
end

function SM.state:putToOutbuf( text )
   state_manager = SM.managers_by_state[self]
   if( not state_manager ) then
      error( "State has no manager, cannout put this text into buffer:\n" .. text, 2 )
      return
   end
   if( not state_manager.client ) then return; end
   state_manager.client[#state_manager.client+1] = text
end

function SM.state:directMsg( msg )
end

--------------------------------
-- State Managers methods     --
-- Written by Daniel R. Koris --
--------------------------------

--------------------------------
-- StateManager:new()         --
-- Written by Daniel R. Koris --
--------------------------------

function SM:new( client )
   local state_manager = {}
   setmetatable( state_manager, self )
   self.__index = self

   -- put it in StateManager's internal list
   if( client ) then SM.managers_by_client[client] = state_manager; end
   SM.managers_all[#SM.managers_all+1] = state_manager

   -- setup some basic state manager instance vars
   state_manager.states = {}
   state_manager.current = 0
   state_manager.previous = 0

   return state_manager
end

function SM:removeSelf()
   for i, sm in ipairs( SM.managers_all ) do
      if( sm == self ) then
         table.remove( SM.managers_all, i )
      end
   end
end

function SM:addState( state )
   local len = #self.states 
   self.states[len+1] = state
   if( len == 0 ) then
      self.current = 1
   end
   SM.managers_by_state[state] = self
   return (len+1)     
end

function SM:setCurrent( index )
   local res, new_index = validIndex( self, index )
   if( not res ) then
      return false
   end
   if( new_index ) then
      self.current = new_index
   else
      self.current = index
   end
   return true
end

function SM:remState( state )
   for i, s in ipairs( self.states ) do
      if( s == state ) then
         table.remove( self.states, i )
         if( #self.states == 0 ) then
           self:removeSelf()
         end
         return true
      end
   end
   return false
end

function SM:remStateByIndex( index )
   if( not self.states[index] ) then
      return true
   end
   table.remove( self.states, index )
   if( #self.states == 0 ) then
      self:removeSelf()
   end
   return true
end

function SM.dataDump()
   print( "Size of Managers " .. table.getn( SM.managers_all ) )
   print( "Size of Managers by Client " .. table.getn( SM.managers_by_client ) )
   print( "Size of Managers by State " .. table.getn( SM.managers_by_state ) )
   print( "Size of States by Data " .. table.getn( SM.states_by_data ) )
end

return SM
