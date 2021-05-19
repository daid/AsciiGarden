#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <unordered_set>
#include <sp2/scene/scene.h>
#include <sp2/graphics/gui/widget/widget.h>

class Scene : public sp::Scene
{
public:
    Scene();
    ~Scene();

    virtual void onUpdate(float delta) override;

    void addKnownPlant(const sp::string& type);

    sp::P<sp::gui::Widget> hud;
    sp::string current_info;
    float info_display_length = 0.0;
    sp::string known_plants_info;
    std::unordered_set<sp::string> known_plants;
};

#endif//MAIN_SCENE_H
