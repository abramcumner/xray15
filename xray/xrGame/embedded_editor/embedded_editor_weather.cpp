#include "pch_script.h"
#include "embedded_editor_weather.h"
#include "../../xrEngine/Environment.h"
#include "../../xrEngine/IGame_Level.h"
#include "../GamePersistent.h"
#include "../Level.h"
#include "../ai_space.h"
#include "../xrServerEntities/script_engine.h"
#include <imgui.h>

Fvector convert(const Fvector& v)
{
    Fvector result;
    result.set(v.z, v.y, v.x);
    return result;
}

Fvector4 convert(const Fvector4& v)
{
    Fvector4 result;
    result.set(v.z, v.y, v.x, v.w);
    return result;
}

bool enumCycle(void* data, int idx, const char** item)
{
    xr_vector<shared_str>* cycles = (xr_vector<shared_str>*)data;
    *item = (*cycles)[idx].c_str();
    return true;
}

bool enumWeather(void* data, int idx, const char** item)
{
    xr_vector<CEnvDescriptor*>* envs = (xr_vector<CEnvDescriptor*>*)data;
    *item = (*envs)[idx]->m_identifier.c_str();
    return true;
}

bool getScriptWeather()
{
    luabind::object benchmark = ai().script_engine().name_space("benchmark");
    return benchmark["weather"].type() == LUA_TBOOLEAN ? !luabind::object_cast<bool>(benchmark["weather"]) : true;
}

void setScriptWeather(bool b)
{
    luabind::object benchmark = ai().script_engine().name_space("benchmark");
    benchmark["weather"] = !b;
}

void ShowWeatherEditor(bool& show)
{
    if (!ImGui::Begin("Weather", &show)) {
        ImGui::End();
        return;
    }
    CEnvironment& env = GamePersistent().Environment();
    CEnvDescriptor* cur = env.Current[0];
    u64 time = Level().GetEnvironmentGameTime() / 1000;
    ImGui::Text("Time: %02d:%02d:%02d", int(time / (60 * 60) % 24), int(time / 60 % 60), int(time % 60));
    float tf = Level().GetEnvironmentTimeFactor();
    if (ImGui::SliderFloat("Time factor", &tf, 0.0f, 1000.0f, "%.3f", 2.0f))
        Level().SetEnvironmentTimeFactor(tf);
    xr_vector<shared_str> cycles;
    int iCycle = -1;
    for (const auto& el : env.WeatherCycles) {
        cycles.push_back(el.first);
        if (el.first == env.CurrentWeatherName)
            iCycle = cycles.size() - 1;
    }
    if (ImGui::Combo("Weather cycle", &iCycle, enumCycle, &cycles, env.WeatherCycles.size()))
        env.SetWeather(cycles[iCycle], true);
    int sel = -1;
    for (int i = 0; i != env.CurrentWeather->size(); i++)
        if (cur->m_identifier == env.CurrentWeather->at(i)->m_identifier)
            sel = i;
    if (ImGui::Combo("Current section", &sel, enumWeather, env.CurrentWeather, env.CurrentWeather->size())) {
        env.SetGameTime(env.CurrentWeather->at(sel)->exec_time + 0.5f, tf);
        time = time / (24 * 60 * 60) * 24 * 60 * 60 * 1000;
        time += u64(env.CurrentWeather->at(sel)->exec_time * 1000 + 0.5f);
        Level().SetEnvironmentGameTimeFactor(time, tf);
        env.SetWeather(cycles[iCycle], true);
    }
    static bool b = getScriptWeather();
    if (ImGui::Checkbox("Script weather", &b))
        setScriptWeather(b);
    ImGui::Separator();
    ImGui::SliderFloat("rain_density", &cur->rain_density, 0.0f, 10.0f);
    ImGui::ColorEdit3("rain_color", (float*)&cur->rain_color);
    ImGui::SliderFloat("sky_rotation", &cur->sky_rotation, 0.0f, 6.28318f);
    Fvector temp;
    temp = convert(cur->sky_color);
    ImGui::ColorEdit3("sky_color", (float*)&temp);
    cur->sky_color = convert(temp);
    ImGui::ColorEdit3("ambient", (float*)&cur->ambient);
    ImGui::ColorEdit4("hemi_color", (float*)&cur->hemi_color, ImGuiColorEditFlags_AlphaBar);
    ImGui::ColorEdit3("sun_color", (float*)&cur->sun_color);
    ImGui::SliderFloat("far_plane", &cur->far_plane, 0.0f, 10000.0f);
    ImGui::SliderFloat("fog_distance", &cur->fog_distance, 0.0f, 10000.0f);
    ImGui::SliderFloat("fog_density", &cur->fog_density, 0.0f, 10.0f);
    ImGui::ColorEdit3("fog_color", (float*)&cur->fog_color);
    Fvector4 temp1;
    temp1 = convert(cur->clouds_color);
    ImGui::ColorEdit4("clouds_color", (float*)&temp1, ImGuiColorEditFlags_AlphaBar);
    cur->clouds_color = convert(temp1);
    ImGui::SliderFloat("wind_velocity", &cur->wind_velocity, 0.0f, 100.0f);
    ImGui::SliderFloat("wind_direction", &cur->wind_direction, 0.0f, 360.0f);
    ImGui::Button("Save");
    ImGui::End();
}