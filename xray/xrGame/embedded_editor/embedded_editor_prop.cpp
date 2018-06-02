#include "pch_script.h"
#include "embedded_editor_prop.h"
#include "../../xrEngine/xr_object.h"
#include "../GameObject.h"
#include "../InventoryOwner.h"
#include "../entity_alive.h"
#include "../script_binder_object_wrapper.h"
#include "embedded_editor_helper.h"
#include "embedded_editor_logic.h"
#include "embedded_editor_main.h"
#include <imgui.h>
#include <luabind/class_info.hpp>
#include <string>
#include <typeinfo>

CObject* object = nullptr;
bool show_lua_binder = false;

void SetPropObject(CObject* obj) { object = obj; }

void ShowPropEditor(bool& show)
{
    ImguiWnd wnd("Properties", &show);
    if (wnd.Collapsed)
        return;

    ImGui::Text("type_info: %s", object ? typeid(*object).name() : "null");
    if (object) {
        ImGui::Spacing();
        if (ImGui::CollapsingHeader("CObject")) {
            ImGui::Text("ID: %u", object->ID());
            ImGui::Text("Section: %s", object->cNameSect().c_str());
            ImGui::Text("Name: %s", object->cName().c_str());
            ImGui::Text("Visual: %s", object->cNameVisual().c_str());
        }
    }
    CGameObject* gameObject = smart_cast<CGameObject*>(object);
    if (gameObject) {
        ImGui::Spacing();
        if (ImGui::CollapsingHeader("CGameObject")) {
            ImGui::Text("Story Id: %lu", gameObject->story_id());
            ImGui::Text("clsid: %d", gameObject->clsid());
        }
    }
    if (gameObject && gameObject->object()) {
        ImGui::Spacing();
        if (ImGui::CollapsingHeader("CScriptBinder")) {
            luabind::wrap_base* base = (CScriptBinderObjectWrapper*)gameObject->object();
            const auto& ref = luabind::detail::wrap_access::ref(*base).m_strong_ref;
            luabind::object binder(ref.state(), ref, true);
            auto info = luabind::get_class_info(binder);
            ImGui::Text("binder: %s", typeid(*gameObject->object()).name());
            ImGui::Text("lua-binder: %s", info.name.c_str());
            ImGui::SameLine();
            if (ImGui::Button("..."))
                show_lua_binder = true;
            binder.pushvalue();
            luabind::detail::object_rep* obj
                = static_cast<luabind::detail::object_rep*>(lua_touserdata(binder.lua_state(), -1));
            lua_pop(binder.lua_state(), 1);
            luabind::object luaBinder(binder.lua_state(), obj->get_lua_table(), true);
            luabind::object st = luaBinder["st"];
            luabind::object iniFile = st["ini_filename"];
            ImGui::Text("ini_filename=%s", to_string(iniFile).c_str());
            luabind::object activeSection = st["active_section"];
            ImGui::Text("active_section=%s", to_string(activeSection).c_str());
            luabind::object gulagName = st["gulag_name"];
            ImGui::Text("gulag_name=%s", to_string(gulagName).c_str());
            if (ImGui::Button("Show logic"))
                show_logic_editor = true;
        }
    }
    CEntity* entity = smart_cast<CEntity*>(object);
    if (entity) {
        ImGui::Spacing();
        if (ImGui::CollapsingHeader("CEntityAlive")) {
            float health = entity->GetfHealth();
            if (ImGui::SliderFloat("Health", &health, 0.0f, 1.0f))
                entity->SetfHealth(health);
        }
    }
    CInventoryOwner* invOwner = smart_cast<CInventoryOwner*>(object);
    if (invOwner) {
        ImGui::Spacing();
        if (ImGui::CollapsingHeader("CInventoryOwner")) {
            ImGui::Text("Name: %s", toUtf8(invOwner->Name()).c_str());
            int money = invOwner->get_money();
            if (ImGui::SliderInt("Money", &money, 0, 999999))
                invOwner->set_money(money, true);
            CHARACTER_RANK_VALUE rank = invOwner->Rank();
            if (ImGui::SliderInt("Rank", &rank, 0, 10000))
                invOwner->SetRank(rank);
            CHARACTER_RANK_VALUE rep = invOwner->Reputation();
            if (ImGui::SliderInt("Reputation", &rep, -10000, 10000))
                invOwner->SetReputation(rep);
        }
    }
}

void ShowLuaBinder(bool& show)
{
    ImguiWnd wnd("Lua binder info", &show);
    if (wnd.Collapsed)
        return;

    CGameObject* gameObject = smart_cast<CGameObject*>(object);
    if (!gameObject || !gameObject->object()) {
        ImGui::Text("no lua binder");
        return;
    }
    ImGui::Text("ID: %u Name: %s", object->ID(), object->cName().c_str());
    ImGui::Separator();

    luabind::wrap_base* base = (CScriptBinderObjectWrapper*)gameObject->object();
    const auto& ref = luabind::detail::wrap_access::ref(*base).m_strong_ref;
    luabind::object binder(ref.state(), ref, true);
    auto info = luabind::get_class_info(binder);
    ImGui::Text("%s", info.name.c_str());
    binder.pushvalue();
    luabind::detail::object_rep* obj
        = static_cast<luabind::detail::object_rep*>(lua_touserdata(binder.lua_state(), -1));
    lua_pop(binder.lua_state(), 1);
    luabind::object luaBinder(binder.lua_state(), obj->get_lua_table(), true);
    xr_string content = toUtf8(to_string(luaBinder).c_str());
    ImGui::InputTextMultiline(
        "##lua-binder-content", &content[0], content.size(), ImVec2(-4.0f, -4.0f), ImGuiInputTextFlags_ReadOnly);
}
