-- element frameworks --
flesh_element = {}
flesh_element["name"] = "Flesh"
flesh_element["strong_against"] = {}
flesh_element["weak_against"] = {}
flesh_element["composition"] = { { "Flesh", 100 } } 

fire_element = {}
fire_element["name"] = "Fire"
fire_element["strong_against"] = { "Wind" }
fire_element["weak_against"] = { "Water" }
fire_element["composition"] = { { "Fire", 100 } }

water_element = {}
water_element["name"] = "Water"
water_element["strong_against"] = { "Fire" }
water_element["weak_against"] = { "Earth" }
water_element["composition"] = { { "Water", 100 } }

earth_element = {}
earth_element["name"] = "Earth"
earth_element["strong_against"] = { "Water" }
earth_element["weak_against"] = { "Lightning" }
earth_element["composition"] = { { "Earth", 100 } }

lightning_element = {}
lightning_element["name"] = "Lightning"
lightning_element["strong_against"] = { "Earth" }
lightning_element["weak_against"] = { "Wind" }
lightning_element["composition"] = { { "Lightning", 100 } }

wind_element = {}
wind_element["name"] = "Wind"
wind_element["strong_against"] = { "Lightning" }
wind_element["weak_against"] = { "Fire" }
wind_element["composition"] = { { "Wind", 100 } }

light_element = {}
light_element["name"] = "Light"
light_element["strong_against"] = {}
light_element["weak_against"] = { "Dark" }
light_element["composition"] = { { "Light", 100 } }

dark_element = {}
dark_element["name"] = "Dark"
dark_element["strong_against"] = {}
dark_element["weak_against"] = { "Light" }
dark_element["composition"] = { { "Dark", 100 } }

-- master table --
elements_table = { flesh_element, fire_element, water_element, earth_element, lightning_element, wind_element, light_element, dark_element }

function getElement( element_name )
   local index = getElementalIndex( element_name )
   local element = elements_table[index]
   if( index == nil ) then mud.bug( "Index is NIL" ) return nil end
   if( element == nil ) then mud.bug( "Element is NIL" ) return nil end
   return elements_table[getElementalIndex( element_name )]
end

function getElementalIndex( element_name )
   for index, name in pairs( elements_table ) do
      if( name["name"] == element_name ) then return index end
   end
   return nil
end

function getElementalName( index )
   return elements[index]["name"]
end
