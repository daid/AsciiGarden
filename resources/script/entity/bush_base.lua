--[[
water_required = 0.8
grow_frames = 1500
plant_name = "apple tree"
seed_name = "apple_seed"
description = ""

fruit_color = {0, 80, 100}
fruit_name = "apple"
fruit_grow_frames = 500
fruit_max = 5
--]]

self.setGlyph('O')
self.setZ(0.5)
self.setHSV(120, 50, 80)
self.setScale(0.1)

include("script/entity/plant_base.lua")

leafs = {}

function updateGfx(scale)
    self.setScale(scale*1.0)
    self.setZ(scale*0.4)
    
    if scale > 0.9 then
        if #leafs == 0 then
            for n=1,6 do
                local child = self.addChild("o", true)
                child.z = random(-0.5, 0.5)
                child.dist = random(0.1, 0.3)
                child.scale = random(0.3, 0.5)
                child.angle = n + random(-0.7, 0.7)
                child.setHSV(120, 50, 80)
                leafs[n] = child
            end
        end
        for n, child in ipairs(leafs) do
            child.setScale((scale - 0.9) * 10.0 * child.scale)
            child.setOffset({math.cos(child.angle)*child.dist*scale, math.sin(child.angle)*child.dist*scale, 0.8*scale*child.z})
        end
    end
end

fruit = nil
fruits = {}

function doneUpdate()
    if not fruit and #fruits < fruit_max then
        fruit = self.addChild(fruit_glyph, false)
        fruit.angle = random(0, 6.28)
        fruit.dist = random(0.05, 0.2)
        fruit.size = 0
        fruit.z = random(-0.3, 0.3)
        fruit.setScale(0.0)
        fruit.setHSV(fruit_color[1], fruit_color[2], fruit_color[3])
        fruit.setOffset({math.cos(fruit.angle)*fruit.dist, math.sin(fruit.angle)*fruit.dist, fruit.z})
    end
    
    if fruit then
        if fruit.size < fruit_grow_frames then
            if self.getCell().water_level > fruit_grow_water / fruit_grow_frames then
                fruit.size = fruit.size + 1
                fruit.setScale(fruit.size / fruit_grow_frames * 0.5)
                self.getCell().water_level = self.getCell().water_level - fruit_grow_water / fruit_grow_frames
            end
        else
            table.insert(fruits, fruit)
            fruit = nil
        end
    end
end

function getAction()
    if #fruits > 0 then
        return "Take " .. fruit_name
    end
    return "Cut down " .. plant_name
end

function executeAction()
    if #fruits > 0 then
        giveItem(seed_name)
        table.remove(fruits, 1).destroy()
    else    
        self.destroy()
    end
end
