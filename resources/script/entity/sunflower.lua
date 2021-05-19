water_required = 0.8
grow_frames = 1500
plant_name = "sunflower"
seed_name = "sunflower_seed"
description = "The sunflower (Helianthus annuus) is a living annual plant in the family Asteraceae, with a large flower head (capitulum). The stem of the flower can grow up to 3 metres tall, with a flower head that can be 30 cm wide. The flower head is actually an inflorescence made of hundreds or thousands of tiny flowers called florets. The central florets look like the centre of a normal flower, apseudanthium. The benefit to the plant is that it is very easily seen by the insects and birds which pollinate it, and it produces thousands of seeds."

self.setGlyph('I')
self.setZ(0.5)
self.setHSV(120, 50, 60)
self.setScale(0.1)

include("script/entity/plant_base.lua")

children = {}
for n=1,6 do
    local child = self.addChild("S", true)
    child.setScale(0.1)
    child.setHSV(60, 100, 90)
    child.setRotation(n*60+90)
    child.setOffset({math.cos(n)*0.25, math.sin(n)*0.25, 0.022})
    children[n] = child
end

function updateGfx(scale)
    self.setScale(scale*0.5)
    self.setZ(scale*0.25)
    
    for n, child in ipairs(children) do
        child.setScale(0.5*scale)
        child.setOffset({math.cos(n)*0.25*scale, math.sin(n)*0.25*scale, 0.22*scale})
    end
end
