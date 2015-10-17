--------------------------------
-- State Manager Module       --
-- Written by Daniel R. Koris --
--------------------------------
-- The State Manager is at the heart of our data management.


local SM = {}

--------------------------------
-- State Manager Locals       --
-- Written by Daniel R. Koris --
--------------------------------
SM.managers_by_client = {}
SM.managers_by_state = {}
SM.managers_all = {}
SM.states = {}

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

function SM.state:new( data, interpreter, behaviour )
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
      if( not Utils:requireCheck( data.interpreter ) ) then
         return nil
      end
      state.interpreter = require( data.interpreter )            
   end

   -- interpreters need to be initialized with the state and the data
   state.interpreter( state, state.data )

   -- any data set can only have one state managing it
   if( SM.states[data] )
   end
   
   SM.states[data] = state
   return state
end

function SM.state:delete()
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
   setmetatable( state, self )
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

function SM:delete()
end

function SM:addState( state )
   local len = #self.states 
   self.states[len+1] = state
   if( len == 0 ) then
      self.current = 1
   end
   managers_by_states[state] = self
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
      if( s == state )
         table.remove( self.states, i )
         if( #self.states == 0 ) then
            self:delete()
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
   return true
end

return SM
