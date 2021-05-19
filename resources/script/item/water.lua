self.setGlyph('O')
self.setHSV(240, 80, 80)

function getAction(target)
    if target.type == "water" then
        return "Drop water"
    end
    if target.getEntity() ~= nil then
        return "Add water to plant"
    end
    return "Add water to soil"
end

function executeAction(target)
    target.water_level = target.water_level + 1.0
    self.destroy()
end
