local E = {}

E.ACCOUNT  = 1
E.GAMEOBJ  = 2
E.NANNY    = 3
E.types    = { "account", "gameobj", "nanny" }
E.entities = {}

for i, t in pairs( E.types ) do
   E.entities[i] = {}
end

function E:new( type )
end

function E:load()
end

function E:instance()
end

return E
