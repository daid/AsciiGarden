water_required = 1.5
grow_frames = 3000
plant_name = "rose"
seed_name = "rose_seed"
description = "The rose is a type of flowering shrub. Its name comes from the Latin word Rosa. The flowers of the rose grow in many different colors, from the well-known red rose or yellow roses and sometimes white or purple roses. Roses belong to the family of plants called Rosaceae. All roses were originally wild and they come from several parts of the world, North America, Europe, northwest Africa and many parts of Asia and Oceania. There are over 100 different species of roses. The wild rose species can be grown in gardens, but most garden roses are cultivars, which have been chosen by people."

self.setGlyph('I')
self.setZ(0.5)
self.setHSV(120, 50, 60)
self.setScale(0.1)

include("script/entity/plant_base.lua")

center = self.addChild("R", false)
center.setScale(0.01)
center.setHSV(random(0, 360), 100, 100)

function updateGfx(scale)
    self.setScale(scale*0.4)
    self.setZ(scale*0.15)
    center.setScale(scale*0.3)
    center.setZ(scale*0.2)
end
