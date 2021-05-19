water_required = 0.7
grow_frames = 1700
plant_name = "corn"
seed_name = "corn_seed"
description = "Corn is Zea mays, a member of the grass family Poaceae. It is a cereal grain which was first grown by people in ancient Central America. Approximately 1 billion tonnes are harvested every year. However, little of this corn is eaten directly by humans. Most is used to make corn ethanol, animal feed and other corn products, such as corn starch and corn syrup."

self.setGlyph('I')
self.setZ(0.5)
self.setHSV(120, 50, 60)
self.setScale(0.1)

include("script/entity/plant_base.lua")

center = self.addChild("C", false)
center.setScale(0.01)
center.setHSV(60, 100, 100)

top = self.addChild("C", false)
top.setScale(0.01)
top.setHSV(60, 100, 100)

function updateGfx(scale)
    self.setScale(scale*0.3)
    self.setZ(scale*0.15)
    center.setScale(scale*0.3)
    center.setZ(scale*0.2)
    top.setScale(scale*0.3)
    top.setZ(scale*0.4)
end
