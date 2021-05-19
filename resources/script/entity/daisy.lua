water_required = 1.2
grow_frames = 2000
plant_name = "daisy"
seed_name = "daisy_seed"
description = "The family Asteraceae or Compositae (known as the aster, daisy, or sunflower family) is the largest family of flowering plants, in terms of number of species. They are distinguished by having flower heads (capitula) which are made of hundreds or thousands of tiny individual flowers. This is a pseudanthium (\"false flower\"). It is also known as the Cosmos flower."

self.setGlyph('I')
self.setZ(0.5)
self.setHSV(120, 50, 60)
self.setScale(0.1)

include("script/entity/plant_base.lua")

children = {}
for n=1,6 do
    local child = self.addChild("D", true)
    child.setScale(0.01)
    child.setHSV(0, 0, 100)
    child.setRotation(n*60+90)
    child.setOffset({math.cos(n)*0.25, math.sin(n)*0.25, 0.022})
    children[n] = child
end
center = self.addChild("o", true)
center.setScale(0.01)
center.setHSV(60, 100, 100)

function updateGfx(scale)
    self.setScale(scale*0.3)
    self.setZ(scale*0.15)
    center.setScale(scale*0.3)
    center.setZ(scale*0.15)
    
    for n, child in ipairs(children) do
        child.setScale(0.3*scale)
        child.setOffset({math.cos(n)*0.15*scale, math.sin(n)*0.15*scale, 0.12*scale})
    end
end
