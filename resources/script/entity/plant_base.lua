--[[
Expected variables input:
water_required = 1.0
grow_frames = 1000
plant_name = "grass"
seed_name = "grass_seed"
description = "Grass is a plant with narrow leaves growing from the base. A common kind of grass is used to cover the ground in a lawn and other places. Grass gets water from the roots in the ground. Grasses are monocotyledon plant, herbaceous plants."
--]]

self.setScale(0.0)

function updateGfx(scale)
    self.setScale(scale)
    self.setZ(scale*0.5)
end
function doneUpdate()
end

self.is_seed = true
full_grown = false
seed_given = false
function update()
    for n=1,grow_frames do
        yield()
        while self.getCell().water_level < water_required / grow_frames * 10.0 do
            yield()
        end
        self.getCell().water_level = self.getCell().water_level - water_required / grow_frames
        updateGfx(n/grow_frames)
        
        if n > grow_frames / 3 and n > 500 then
            self.is_seed = false
        end
    end
    full_grown = true
    while true do
        yield()
        doneUpdate()
    end
end

function getAction()
    if full_grown then
        return "Harvest " .. plant_name .. " seed"
    end
    return "Cut " .. plant_name
end

function executeAction()
    if full_grown then
        giveItem(seed_name)
        if not seed_given then
            seed_given = true
            return
        end
    end
    self.destroy()
end

function getInfo()
    if full_grown then
        return "Fully grown " .. plant_name .. "\n" .. description
    end
    return "Growing " .. plant_name .. "\n" .. description
end
