#include "mainScene.h"
#include "ingameMenu.h"
#include "main.h"

#include <sp2/tweak.h>
#include <sp2/script/environment.h>
#include <sp2/collision/2d/circle.h>
#include <sp2/collision/2d/polygon.h>
#include <sp2/collision/2d/box.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/random.h>
#include <sp2/engine.h>
#include <sp2/scene/camera.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>

int luaYield(lua_State* lua)
{
    return lua_yield(lua, 0);
}

int luaInclude(lua_State* lua)
{
    sp::string file = luaL_checkstring(lua, 1);
    sp::string s = sp::io::ResourceProvider::get(file)->readAll();
    if (luaL_loadbuffer(lua, s.data(), s.length(), file.data()))
    {
        LOG(Error, "Failed to load:", file, lua_tostring(lua, -1));
        lua_pop(lua, 1);
        return lua_error(lua);
    }
    lua_pushvalue(lua, lua_upvalueindex(1));
    lua_setupvalue(lua, -2, 1);
    if (lua_pcall(lua, 0, 0, 0))
    {
        sp::string error_string = luaL_checkstring(lua, -1);
        LOG(Error, "LUA: include:", error_string);
        lua_pushstring(lua, ("require:" + error_string).c_str());
        return lua_error(lua);
    }
    return 0;
}

bool luaGiveItem(const sp::string& type);
void luaAddKnownPlant(const sp::string& type);

class Glyph : public sp::Node
{
public:
    Glyph(sp::P<sp::Node> parent, const sp::string& c, bool flat)
    : sp::Node(parent)
    {
        render_data.type = sp::RenderData::Type::Normal;
        if (flat)
            render_data.shader = sp::Shader::get("internal:basic.shader");
        else
            render_data.shader = sp::Shader::get("billboard2.shader");

        setGlyph(c);
    }

    void setGlyph(const sp::string& c)
    {
        auto font = sp::font_manager.get("gui/theme/RobotoMono-Medium.ttf");

        render_data.mesh = font->createString(c, 64, 1.0, sp::Vector2d(0, 0), sp::Alignment::Center);
        render_data.texture = font->getTexture(64);
    }

    void setScale(float f)
    {
        render_data.scale = sp::Vector3f(f, f, f);
    }

    sp::P<Glyph> addChild(const sp::string& s, bool flat)
    {
        return new Glyph(this, s, flat);
    }

    void setZ(double f)
    {
        setPosition(sp::Vector3d(getPosition3D().x, getPosition3D().y, f));
    }

    void setHSV(float h, float s, double v)
    {
        render_data.color = sp::HsvColor(h, s, v);
    }

    void destroy()
    {
        destroy_flag = true;
    }

    void onRegisterScriptBindings(sp::script::BindingClass& script_binding_class) override
    {
        script_binding_class.bind("setGlyph", &Glyph::setGlyph);
        script_binding_class.bind("setOffset", static_cast<void (sp::Node::*)(sp::Vector3d)>(&sp::Node::setPosition));
        script_binding_class.bind("setRotation", static_cast<void (sp::Node::*)(double)>(&sp::Node::setRotation));
        script_binding_class.bind("setZ", &Glyph::setZ);
        script_binding_class.bind("setScale", &Glyph::setScale);
        script_binding_class.bind("addChild", &Glyph::addChild);
        script_binding_class.bind("setHSV", &Glyph::setHSV);
        script_binding_class.bind("destroy", &Glyph::destroy);
    }

    void onFixedUpdate() override
    {
        if (destroy_flag)
            delete this;
    }
private:
    bool destroy_flag = false;
};

class Entity : public Glyph
{
public:
    Entity(sp::P<sp::Node> root, const sp::string& type)
    : Glyph(root, "", false), type(type)
    {
        setPosition(sp::Vector3d(0.0, 0.0, 0.4));
        script.setGlobal("yield", luaYield);
        script.setGlobal("include", luaInclude);
        script.setGlobal("giveItem", luaGiveItem);
        script.setGlobal("random", sp::random);
        script.setGlobal("self", this);
        
        auto result = script.load("script/entity/" + type + ".lua");
        if (result.isErr())
            LOG(Error, "Entity init error: " + result.error());
    }

    ~Entity()
    {
        getParent()->removeCollisionShape();
    }

    void onFixedUpdate() override
    {
        if (update_coroutine)
        {
            auto res = update_coroutine->resume();
            if (res.isErr())
                LOG(Error, "Update error: ", res.error());
            else if (!res.value())
                update_coroutine = nullptr;
        }
        else
        {
            auto res = script.callCoroutine("update");
            if (res.isErr())
                LOG(Error, "Update error: ", res.error());
            else
                update_coroutine = res.value();
        }
        Glyph::onFixedUpdate();
    }

    sp::string getAction()
    {
        auto res = script.call("getAction");
        if (res.isErr())
        {
            LOG(Error, "getAction error: ", res.error());
            return "";
        }
        return res.value().getString();
    }

    void executeAction()
    {
        auto res = script.call("executeAction");
        if (res.isErr())
            LOG(Error, "executeAction error: ", res.error());
    }

    sp::string getInfo()
    {
        auto res = script.call("getInfo");
        if (res.isErr())
        {
            LOG(Error, "getInfo error: ", res.error());
            return "";
        }
        return res.value().getString();
    }

    sp::P<sp::Node> getCell()
    {
        return getParent();
    }

    sp::string getType()
    {
        return type;
    }

    void setCollision(double size)
    {
        if (size <= 0.0)
        {
            getParent()->removeCollisionShape();
        }else{
            sp::collision::Circle2D shape(size);
            shape.type = sp::collision::Shape::Type::Static;
            getParent()->setCollisionShape(shape);
        }
    }

    void onRegisterScriptBindings(sp::script::BindingClass& script_binding_class) override
    {
        Glyph::onRegisterScriptBindings(script_binding_class);
        script_binding_class.bind("getCell", &Entity::getCell);
        script_binding_class.bind("getType", &Entity::getType);
        script_binding_class.bind("setCollision", &Entity::setCollision);
    }

    sp::string type;
    sp::script::Environment script;
    sp::script::CoroutinePtr update_coroutine;
};

class Item : public Glyph
{
public:
    Item(sp::P<sp::Node> root, const sp::string& type)
    : Glyph(root, "", true)
    {
        setPosition(sp::Vector3d(0, 0, 0.5));
        script.setGlobal("yield", luaYield);
        script.setGlobal("include", luaInclude);
        script.setGlobal("addKnownPlant", luaAddKnownPlant);
        script.setGlobal("self", this);
        auto result = script.load("script/item/" + type + ".lua");
        if (result.isErr())
            LOG(Error, "Entity init error: " + result.error());
    }

    sp::string getAction(sp::P<sp::Node> target)
    {
        auto res = script.call("getAction", target);
        if (res.isErr())
        {
            LOG(Error, "getAction error: ", res.error());
            return "";
        }
        return res.value().getString();
    }

    void executeAction(sp::P<sp::Node> target)
    {
        auto res = script.call("executeAction", target);
        if (res.isErr())
            LOG(Error, "executeAction error: ", res.error());
    }

    sp::script::Environment script;
};

class FloorCell : public Glyph
{
public:
    enum class Type {
        Grass,
        Dirt,
        Water,
    };
    Type type = Type::Grass;

    FloorCell(sp::P<sp::Node> parent, int x, int y)
    : Glyph(parent, "#", true)
    {
        render_data.order = -1;
        setPosition(sp::Vector3d(x+0.5, y+0.5, -0.4));
        setType(type);

        water_anim_time = x * 0.9 + y * 0.3;
        water_level = sp::random(0.7, 1.2);
    }

    void setType(Type new_type)
    {
        type = new_type;
        switch(type)
        {
        case Type::Grass:
            type_name = "grass";
            render_data.color = sp::HsvColor(120, 50, 40);
            setPosition(sp::Vector3d(getPosition3D().x, getPosition3D().y, -0.4));
            break;
        case Type::Dirt:
            type_name = "dirt";
            render_data.color = sp::HsvColor(25, 100, 40);
            setPosition(sp::Vector3d(getPosition3D().x, getPosition3D().y, -0.4));
            break;
        case Type::Water: {
            type_name = "water";
            render_data.color = sp::HsvColor(240, 80, 80);
            sp::collision::Box2D shape(0.8, 0.8);
            shape.type = sp::collision::Shape::Type::Static;
            setCollisionShape(shape);
            } break;
        }
    }

    void onFixedUpdate() override
    {
        if (type == Type::Dirt || water_level > 0.7)
            water_level *= 0.9999;
    }

    void onUpdate(float delta) override;

    sp::string getAction()
    {
        if (type == Type::Water)
            return "Take water";
        return "";
    }

    sp::string getInfo()
    {
        switch(type)
        {
        case Type::Grass:
            if (water_level < 0.7)
                return "Dry fertile soil\nSoil is loose material which lies on top of the land. It has many things in it, like tiny grains of rock, minerals, water and air. Soil also has living things and dead things in it: \"organic matter\". Soil is important for life on Earth. Because soil holds water and nutrients, it is an ideal place for plants to grow.";
            return "Fertile soil\nSoil is loose material which lies on top of the land. It has many things in it, like tiny grains of rock, minerals, water and air. Soil also has living things and dead things in it: \"organic matter\". Soil is important for life on Earth. Because soil holds water and nutrients, it is an ideal place for plants to grow.";
        case Type::Dirt:
            if (water_level > 1.5)
                return "Wet soil\nSoil is loose material which lies on top of the land. It has many things in it, like tiny grains of rock, minerals, water and air. Soil also has living things and dead things in it: \"organic matter\". Soil is important for life on Earth. Because soil holds water and nutrients, it is an ideal place for plants to grow.";
            return "Soil\nSoil is loose material which lies on top of the land. It has many things in it, like tiny grains of rock, minerals, water and air. Soil also has living things and dead things in it: \"organic matter\". Soil is important for life on Earth. Because soil holds water and nutrients, it is an ideal place for plants to grow.";
        case Type::Water:
            return "Water\nPlants and animals (including people) are mostly water inside, and must drink water to live. It gives a medium for chemical reactions to take place, and is the main part of blood. It keeps the body temperature the same by sweating from the skin. Water helps blood carry nutrients from the stomach to all parts of the body to keep the body alive. Water also helps the blood carry oxygen from the lungs to the body. Saliva, which helps animals and people digest food, is mostly water. Water helps make urine. Urine helps remove bad chemicals from the body. The human body is between 60% and 70% water, but this value differs with age; i.e. a foetus is 95% water inside.";
        }
        return "";
    }

    void executeAction();

    sp::P<sp::Node> getCell()
    {
        return this;
    }

    sp::P<sp::Node> getEntity()
    {
        return entity;
    }

    bool newEntity(const sp::string& type)
    {
        if (entity)
            return false;
        entity = new Entity(this, type);
        return true;
    }

    bool replaceEntity(const sp::string& type)
    {
        entity.destroy();
        entity = new Entity(this, type);
        return true;
    }

    void onRegisterScriptBindings(sp::script::BindingClass& script_binding_class) override
    {
        Glyph::onRegisterScriptBindings(script_binding_class);
        script_binding_class.bind("getCell", &FloorCell::getCell);
        script_binding_class.bind("getEntity", &FloorCell::getEntity);
        script_binding_class.bind("newEntity", &FloorCell::newEntity);
        script_binding_class.bind("replaceEntity", &FloorCell::replaceEntity);
        script_binding_class.bindProperty("water_level", water_level);
        script_binding_class.bindProperty("type", type_name);
    }

    sp::P<Entity> entity;
    sp::string type_name;
    float water_level = 0.0;
    float water_anim_time = 0.0;
    float update_delay = 0.0;
};

class World : public sp::AutoPointerObject
{
public:
    World(sp::P<sp::Node> root, sp::Vector2i new_size)
    : size(new_size)
    {
        cells.resize(size.x * size.y);

        for(int y=0; y<size.y; y++)
        for(int x=0; x<size.x; x++)
        {
            cells[x+y*size.x] = new FloorCell(root, x, y);

            if (x > new_size.x * 0.3 && sp::chance(20))
                cells[x+y*size.x]->setType(FloorCell::Type::Water);
            else if (x > new_size.x * 0.6 && sp::chance(50))
                cells[x+y*size.x]->setType(FloorCell::Type::Water);
            else if (x > new_size.x * 0.8)
                cells[x+y*size.x]->setType(FloorCell::Type::Water);
            else if (sp::chance(20))
                cells[x+y*size.x]->setType(FloorCell::Type::Dirt);
        }
    }

    sp::P<FloorCell> getCell(sp::Vector2i position)
    {
        if (position.x < 0 || position.x >= size.x) return nullptr;
        if (position.y < 0 || position.y >= size.y) return nullptr;
        return cells[position.x + position.y * size.x];
    }

    sp::P<Entity> newEntity(sp::Vector2i position, const sp::string& type)
    {
        auto cell = getCell(position);
        if (!cell || cell->entity) return nullptr;
        cell->entity = new Entity(cell, type);
        return cell->entity;
    }

    int getEntityCount()
    {
        int count = 0;
        for(auto cell : cells)
            if (cell->entity)
                count += 1;
        return count;
    }

    sp::Vector2i size;
private:
    std::vector<sp::P<FloorCell>> cells;
};
sp::P<World> world;

class Player : public Glyph
{
public:
    Player(sp::P<sp::Node> parent)
    : Glyph(parent, "@", false)
    {
        sp::collision::Polygon2D shape;
        shape.add(sp::Vector2f( 0.3, 0.2));
        shape.add(sp::Vector2f( 0.2, 0.3));
        shape.add(sp::Vector2f(-0.3, 0.2));
        shape.add(sp::Vector2f(-0.2, 0.3));
        shape.add(sp::Vector2f( 0.3,-0.2));
        shape.add(sp::Vector2f( 0.2,-0.3));
        shape.add(sp::Vector2f(-0.3,-0.2));
        shape.add(sp::Vector2f(-0.2,-0.3));
        shape.type = sp::collision::Shape::Type::Dynamic;
        shape.linear_damping = 40.0;
        shape.fixed_rotation = true;
        setCollisionShape(shape);
    }

    void onFixedUpdate() override
    {
        sp::Vector2d velocity = getLinearVelocity2D();
        sp::Vector2d movement{controller.right.getValue() - controller.left.getValue(), controller.up.getValue() - controller.down.getValue()};
        if (movement.length() < 0.1)
            movement = sp::Vector2d(0, 0);
        movement = movement.rotate(getScene()->getCamera()->getRotation2D());
        velocity += movement * 2.0;
        setLinearVelocity(velocity);

        auto position = getPosition2D();
        position.x = std::clamp(position.x, 0.3, world->size.x - 0.3);
        position.y = std::clamp(position.y, 0.3, world->size.y - 0.3);
        setPosition(position);

        if (movement.length() > 0.1)
            updateActionTarget(movement.normalized());
        updateCursor();

        if (controller.primary_action.getDown())
            executeAction();
    }

    void updateActionTarget(sp::Vector2d offset)
    {
        auto position = getPosition2D();
        position += offset * 0.5;
        auto cell_position = sp::Vector2i(position);
        cell_position.x = std::clamp(cell_position.x, 0, world->size.x - 1);
        cell_position.y = std::clamp(cell_position.y, 0, world->size.y - 1);

        if (!cursor)
        {
            cursor = new Glyph(getParent(), "#", true);
            cursor->render_data.type = sp::RenderData::Type::Transparent;
            cursor->render_data.color = sp::Color(1, 1, 1, 0.5);
            cursor->render_data.scale = sp::Vector3f(1.3, 1.3, 1.3);
            cursor->setPosition(sp::Vector3d(0, 0, 0.01));
        }

        auto target_cell = world->getCell(cell_position);
        cursor->setParent(target_cell);
    }

    void updateCursor()
    {
        if (!cursor)
            return;
        auto source = cursor->getParent();
        cursor->render_data.mesh = source->render_data.mesh;
        cursor->render_data.texture = source->render_data.texture;
        cursor->render_data.shader = source->render_data.shader;
        auto color = sp::HsvColor(source->render_data.color);
        color.value = std::min(color.value + 20.0, 100.0);
        color.saturation = std::max(color.saturation - 20.0, 0.0);
        cursor->render_data.color = color;
        cursor->render_data.scale = source->render_data.scale * 1.3f;
    }

    sp::P<Item> giveItem(const sp::string& type)
    {
        if (carry)
            return nullptr;
        carry = new Item(this, type);
        return carry;
    }

    void executeAction()
    {
        if (!cursor)
            return;
        if (carry)
        {
            carry->executeAction(cursor->getParent());
            return;
        }

        if (sp::P<FloorCell> cell = cursor->getParent())
        {
            if (cell->entity)
                cell->entity->executeAction();
            else
                cell->executeAction();
            return;
        }
    }

    sp::P<sp::Node> cursor;
    sp::P<Item> carry;
};
sp::P<Player> player;

bool luaGiveItem(const sp::string& type)
{
    return player->giveItem(type) != nullptr;
}

class Camera : public sp::Camera
{
public:
    Camera(sp::P<sp::Node> parent)
    : sp::Camera(parent)
    {
        setPosition(sp::Vector3d(0, -10, 5));
        setRotation(sp::Quaterniond::fromAxisAngle(sp::Vector3d(0, 0, 1), 0) * sp::Quaterniond::fromAxisAngle(sp::Vector3d(1, 0, 0), 35));
        setPerspective();
    }

    void onUpdate(float delta) override
    {
        auto world_dist = std::max(world->size.x, world->size.y) * 1.2;
        moveTowards(delta * 5.0, sp::Vector3d(world->size.x * 0.5, world->size.y * 0.5, 0), world_dist, world_dist);
        //moveTowards(delta * 5.0, player->getPosition3D(), 5.0, 7.0);

        auto view_vector = player->getPosition3D() - getPosition3D();
        auto view_vector2d = sp::Vector2d(view_vector.x, view_vector.y);
        auto view_dist = view_vector2d.length();
        auto yaw = view_vector2d.angle() - 90.0;
        auto pitch = sp::Vector2d(-view_vector.z, view_dist).angle();
        setRotation(sp::Quaterniond::fromAxisAngle(sp::Vector3d(0, 0, 1), yaw) * sp::Quaterniond::fromAxisAngle(sp::Vector3d(1, 0, 0), pitch));
    }

private:
    void moveTowards(float delta, sp::Vector3d target, double min_dist, double max_dist)
    {
        auto view_vector = target - getPosition3D();
        auto view_vector2d = sp::Vector2d(view_vector.x, view_vector.y);
        auto view_dist = view_vector2d.length();
        if (view_dist < min_dist)
            setPosition(getPosition3D() - sp::Vector3d(view_vector.x, view_vector.y, 0).normalized() * double(delta) * (min_dist - view_dist));
        else if (view_dist > max_dist)
            setPosition(getPosition3D() + sp::Vector3d(view_vector.x, view_vector.y, 0).normalized() * double(delta) * (view_dist - max_dist));
    }
};

void FloorCell::executeAction()
{
    if (type == Type::Water)
    {
        player->giveItem("water");
        return;
    }
}

Scene::Scene()
: sp::Scene("MAIN")
{
    sp::Scene::get("INGAME_MENU")->enable();

    setDefaultCamera(new Camera(getRoot()));

    world = new World(getRoot(), sp::Vector2i(12, 12));
    world->newEntity(sp::Vector2i(4, 4), "apple_tree");

    player = new Player(getRoot());

    hud = sp::gui::Loader::load("gui/hud.gui", "HUD");
    hud->getWidgetWithID("ACTION")->setEventCallback([](sp::Variant v)
    {
        player->executeAction();
    });
}

Scene::~Scene()
{
    sp::Scene::get("INGAME_MENU")->disable();
    hud.destroy();
}

void Scene::onUpdate(float delta)
{
    auto action_ui = hud->getWidgetWithID("ACTION");
    auto info_ui = hud->getWidgetWithID("INFO_BOX");
    action_ui->hide();
    info_ui->hide();
    sp::string action = "";
    sp::string info = "";
    if (player->cursor)
    {
        if (player->carry)
        {
            action = player->carry->getAction(player->cursor->getParent());
        }
        else if (sp::P<FloorCell> cell = player->cursor->getParent())
        {
            if (cell->entity)
                action = cell->entity->getAction();
            else
                action = cell->getAction();
        }

        if (sp::P<FloorCell> cell = player->cursor->getParent())
        {
            if (cell->entity)
                info = cell->entity->getInfo();
            else
                info = cell->getInfo();
        }
    }
    if (!action.empty())
    {
        action_ui->show();
        action_ui->setAttribute("caption", action);
    }

    if (current_info != info)
    {
        info_display_length = 0.0;
        current_info = info;
    }
    if (!info.empty())
    {
        info_display_length += delta * 30.0;
        info_ui->show();
        info_ui->getWidgetWithID("INFO")->setAttribute("caption", info.substr(0, int(info_display_length)));
    }
}

void Scene::addKnownPlant(const sp::string& type)
{
    if (known_plants.find(type) != known_plants.end())
        return;
    known_plants.insert(type);
    known_plants_info += type + "\n";
    sp::string info = known_plants_info + sp::string(known_plants.size()) + "/10";
    hud->getWidgetWithID("KNOWN_LIST")->setAttribute("caption", info);
}

void FloorCell::onUpdate(float delta)
{
    water_anim_time += delta;
    if(type == Type::Water)
        setPosition(sp::Vector3d(getPosition3D().x, getPosition3D().y, -0.5 + std::sin(water_anim_time) * 0.1));
    if (update_delay > 0.0)
        update_delay -= delta;
    else
    {
        update_delay = sp::random(10.0, 24.0);

        if (type == Type::Dirt && water_level > 1.2)
            setType(Type::Grass);
        if (type == Type::Grass && water_level < 0.5)
            setType(Type::Dirt);
        
        if (type == Type::Grass && sp::chance(5) && !entity && world->getEntityCount() < 10)
        {
            if (world->getEntityCount() > 5 && sp::chance(50))
                entity = new Entity(this, "daisy");
            else
                entity = new Entity(this, "grass");
        }
    }
}

void luaAddKnownPlant(const sp::string& type)
{
    sp::P<Scene> scene = sp::Scene::get("MAIN");
    scene->addKnownPlant(type);
}