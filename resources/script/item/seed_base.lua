plant_list = {
    "grass",
    "daisy",
    "sunflower",
    "rose",
    "bamboo",
    "corn",
    "tomato_plant",
    "apple_tree",
    "pear_tree",
    "orange_tree",
}

addKnownPlant(plant_name)

function getAction(target)
    if target.type == "water" then
        return "Drop in water"
    end
    if target.getEntity() == nil then
        return "Plant " .. plant_name
    end
    if target.getEntity().is_seed and target.getEntity().getType() ~= entity_name then
        return "Combine seeds"
    end
    return ""
end

function executeAction(target)
    if target.getCell().type == "water" then
        self.destroy()
        return
    end
    if target.getEntity() != nil then
        if target.getEntity().is_seed then
            local idx0 = nil
            local idx1 = nil
            for n, name in ipairs(plant_list) do
                if name == target.getEntity().getType() then idx0 = n end
                if name == entity_name then idx1 = n end
            end
            if idx0 == idx1 then
                return
            end
            if idx1 < idx0 then idx0, idx1 = idx1, idx0 end
            local newidx = idx1 + (idx1 - idx0)
            while newidx - 1 >= #plant_list do
                newidx = newidx - #plant_list
            end
            if target.replaceEntity(plant_list[newidx]) then
                self.destroy()
            end
        end
        return
    end
    if target.newEntity(entity_name) then
        self.destroy()
    end
end
