water_required = 0.2
grow_frames = 1000
plant_name = "grass"
seed_name = "grass_seed"
description = "Grass is a plant with narrow leaves growing from the base. A common kind of grass is used to cover the ground in a lawn and other places. Grass gets water from the roots in the ground. Grasses are monocotyledon plant, herbaceous plants."

self.setGlyph('"')
self.setZ(-0.1)
self.setHSV(120, 50, 60)
self.setScale(0.1)

include("script/entity/plant_base.lua")

function updateGfx(scale)
    self.setScale(scale)
end
