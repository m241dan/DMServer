local E = {}

E.ACCOUNT = 1
E.GAMEOBJ = 2
E.NANNY   = 3

E.types = { "account", "gameobj", "nanny" }

E.entities = {}

function E:new( path )
end

function E:load()
end

function E:instance()
end

return E
