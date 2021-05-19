water_required = 0.5
grow_frames = 2700
plant_name = "bamboo"
seed_name = "bamboo_seed"
description = ""

self.setGlyph('|')
self.setHSV(40, 80, 80)
self.setScale(0.1)

include("script/entity/plant_base.lua")

function updateGfx(scale)
    self.setZ(scale * 0.5)
    self.setScale(scale)
end
