#include "pch_script.h"
#include "embedded_editor_weather.h"
#include "../../xrEngine/Environment.h"
#include "../../xrEngine/IGame_Level.h"
#include "../../xrEngine/thunderbolt.h"
#include "../../xrEngine/xr_efflensflare.h"
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

const char* empty = "";
bool enumIniWithEmpty(void* data, int idx, const char** item)
{
    if (idx == 0)
        *item = empty;
    else {
        CInifile* ini = (CInifile*)data;
        *item = ini->sections()[idx - 1]->Name.c_str();
    }
    return true;
}

bool enumIni(void* data, int idx, const char** item)
{
    CInifile* ini = (CInifile*)data;
    *item = ini->sections()[idx]->Name.c_str();
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

xr_set<shared_str> modifiedWeathers;

void saveWeather(shared_str name, const xr_vector<CEnvDescriptor*>& env)
{
    CInifile f(nullptr, FALSE, FALSE, FALSE);
    for (auto el : env) {
        if (el->env_ambient)
            f.w_string(el->m_identifier.c_str(), "ambient", el->env_ambient->name().c_str());
        f.w_fvector3(el->m_identifier.c_str(), "ambient_color", el->ambient);
        f.w_fvector4(el->m_identifier.c_str(), "clouds_color", el->clouds_color);
        f.w_string(el->m_identifier.c_str(), "clouds_texture", el->clouds_texture_name.c_str());
        f.w_float(el->m_identifier.c_str(), "far_plane", el->far_plane);
        f.w_float(el->m_identifier.c_str(), "fog_distance", el->fog_distance);
        f.w_float(el->m_identifier.c_str(), "fog_density", el->fog_density);
        f.w_fvector3(el->m_identifier.c_str(), "fog_color", el->fog_color);
        f.w_fvector3(el->m_identifier.c_str(), "rain_color", el->rain_color);
        f.w_float(el->m_identifier.c_str(), "rain_density", el->rain_density);
        f.w_fvector3(el->m_identifier.c_str(), "sky_color", el->sky_color);
        f.w_float(el->m_identifier.c_str(), "sky_rotation", rad2deg(el->sky_rotation));
        f.w_string(el->m_identifier.c_str(), "sky_texture", el->sky_texture_name.c_str());
        f.w_fvector3(el->m_identifier.c_str(), "sun_color", el->sun_color);
        f.w_float(el->m_identifier.c_str(), "sun_shafts_intensity", el->m_fSunShaftsIntensity);
        f.w_string(el->m_identifier.c_str(), "sun", el->lens_flare_id.c_str());
        f.w_string(el->m_identifier.c_str(), "thunderbolt_collection", el->tb_id.c_str());
        f.w_float(el->m_identifier.c_str(), "thunderbolt_duration", el->bolt_duration);
        f.w_float(el->m_identifier.c_str(), "thunderbolt_period", el->bolt_period);
        f.w_float(el->m_identifier.c_str(), "water_intensity", el->m_fWaterIntensity);
        f.w_float(el->m_identifier.c_str(), "wind_direction", rad2deg(el->wind_direction));
        f.w_float(el->m_identifier.c_str(), "wind_velocity", el->wind_velocity);
        f.w_fvector4(el->m_identifier.c_str(), "hemisphere_color", el->hemi_color);
        f.w_float(el->m_identifier.c_str(), "sun_altitude", rad2deg(el->sun_dir.getH()));
        f.w_float(el->m_identifier.c_str(), "sun_longitude", rad2deg(el->sun_dir.getP()));
    }
    string_path fileName;
    FS.update_path(fileName, "$game_weathers$", name.c_str());
    strconcat(sizeof(fileName), fileName, fileName, ".ltx");
    f.save_as(fileName);
}

void nextTexture(char* tex, int texSize, int offset)
{
    string_path dir, fn;
    _splitpath(tex, nullptr, dir, fn, nullptr);
    strconcat(sizeof(fn), fn, fn, ".dds");
    xr_vector<LPSTR>* files = FS.file_list_open("$game_textures$", dir, FS_ListFiles);
    if (!files)
        return;
    size_t index = 0;
    for (size_t i = 0; i != files->size(); i++)
        if (strcmp((*files)[i], fn) == 0) {
            index = i;
            break;
        }
    size_t newIndex = index;
    while (true) {
        newIndex = (newIndex + offset + files->size()) % files->size();
        if (strstr((*files)[newIndex], "#small") == nullptr && strstr((*files)[newIndex], ".thm") == nullptr)
            break;
    }
    string_path newFn;
    _splitpath((*files)[newIndex], nullptr, nullptr, newFn, nullptr);
    strconcat(texSize, tex, dir, newFn);
    FS.file_list_close(files);
}

void ShowWeatherEditor(bool& show)
{
    if (!ImGui::Begin(modifiedWeathers.empty() ? "Weather###Weather" : "Weather*###Weather", &show)) {
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
    bool changed = false;
    sel = -1;
    for (int i = 0; i != env.m_ambients_config->sections().size(); i++)
        if (cur->env_ambient->name() == env.m_ambients_config->sections()[i]->Name)
            sel = i;
    if (ImGui::Combo("ambient", &sel, enumIni, env.m_ambients_config, env.m_ambients_config->sections().size())) {
        cur->env_ambient = env.AppendEnvAmb(env.m_ambients_config->sections()[sel]->Name);
        changed = true;
    }
    if (ImGui::ColorEdit3("ambient_color", (float*)&cur->ambient))
        changed = true;
    Fvector4 temp1;
    temp1 = convert(cur->clouds_color);
    if (ImGui::ColorEdit4("clouds_color", (float*)&temp1, ImGuiColorEditFlags_AlphaBar))
        changed = true;
    cur->clouds_color = convert(temp1);
    ImGui::Text("clouds_texture");
    ImGui::SameLine();
    char buf[100];
    strcpy(buf, cur->clouds_texture_name.c_str());
    if (ImGui::Button("<##clouds_texture")) {
        nextTexture(buf, sizeof(buf), -1);
        cur->clouds_texture_name = buf;
        cur->on_device_create();
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::InputText("##clouds_texture", buf, 100)) {
        cur->clouds_texture_name = buf;
        cur->on_device_create();
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(">##clouds_texture")) {
        nextTexture(buf, sizeof(buf), +1);
        cur->clouds_texture_name = buf;
        cur->on_device_create();
        changed = true;
    }
    if (ImGui::SliderFloat("far_plane", &cur->far_plane, 0.001f, 10000.0f, "%.3f", 3.0f))
        changed = true;
    if (ImGui::SliderFloat("fog_distance", &cur->fog_distance, 0.0f, 10000.0f, "%.3f", 3.0f))
        changed = true;
    if (ImGui::SliderFloat("fog_density", &cur->fog_density, 0.0f, 10.0f))
        changed = true;
    if (ImGui::ColorEdit3("fog_color", (float*)&cur->fog_color))
        changed = true;
    if (ImGui::ColorEdit4("hemisphere_color", (float*)&cur->hemi_color, ImGuiColorEditFlags_AlphaBar))
        changed = true;
    if (ImGui::SliderFloat("rain_density", &cur->rain_density, 0.0f, 10.0f))
        changed = true;
    if (ImGui::ColorEdit3("rain_color", (float*)&cur->rain_color))
        changed = true;

    Fvector temp;
    temp = convert(cur->sky_color);
    if (ImGui::ColorEdit3("sky_color", (float*)&temp))
        changed = true;
    cur->sky_color = convert(temp);
    if (ImGui::SliderFloat("sky_rotation", &cur->sky_rotation, 0.0f, 6.28318f))
        changed = true;
    ImGui::Text("sky_texture");
    ImGui::SameLine();
    strcpy(buf, cur->sky_texture_name.c_str());
    if (ImGui::Button("<##sky_texture")) {
        nextTexture(buf, sizeof(buf), -1);
        cur->sky_texture_name = buf;
        strconcat(sizeof(buf), buf, buf, "#small");
        cur->sky_texture_env_name = buf;
        cur->on_device_create();
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::InputText("##sky_texture", buf, 100)) {
        cur->sky_texture_name = buf;
        strconcat(sizeof(buf), buf, buf, "#small");
        cur->sky_texture_env_name = buf;
        cur->on_device_create();
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(">##sky_texture")) {
        nextTexture(buf, sizeof(buf), +1);
        cur->sky_texture_name = buf;
        strconcat(sizeof(buf), buf, buf, "#small");
        cur->sky_texture_env_name = buf;
        cur->on_device_create();
        changed = true;
    }
    sel = -1;
    for (int i = 0; i != env.m_suns_config->sections().size(); i++)
        if (cur->lens_flare_id == env.m_suns_config->sections()[i]->Name)
            sel = i;
    if (ImGui::Combo("sun", &sel, enumIni, env.m_suns_config, env.m_suns_config->sections().size())) {
        cur->lens_flare_id
            = env.eff_LensFlare->AppendDef(env, env.m_suns_config, env.m_suns_config->sections()[sel]->Name.c_str());
		env.eff_LensFlare->Invalidate();
        changed = true;
    }
    if (ImGui::ColorEdit3("sun_color", (float*)&cur->sun_color))
        changed = true;
    if (ImGui::SliderFloat("sun_shafts_intensity", &cur->m_fSunShaftsIntensity, 0.0f, 2.0f))
        changed = true;
    sel = 0;
    for (int i = 0; i != env.m_thunderbolt_collections_config->sections().size(); i++)
        if (cur->tb_id == env.m_thunderbolt_collections_config->sections()[i]->Name)
            sel = i + 1;
    if (ImGui::Combo("thunderbolt_collection", &sel, enumIniWithEmpty, env.m_thunderbolt_collections_config,
            env.m_thunderbolt_collections_config->sections().size() + 1)) {
        cur->tb_id = (sel == 0)
            ? env.eff_Thunderbolt->AppendDef(env, env.m_thunderbolt_collections_config, env.m_thunderbolts_config, "")
            : env.eff_Thunderbolt->AppendDef(env, env.m_thunderbolt_collections_config, env.m_thunderbolts_config,
                  env.m_thunderbolt_collections_config->sections()[sel - 1]->Name.c_str());
        changed = true;
    }
    if (ImGui::SliderFloat("thunderbolt_duration", &cur->bolt_duration, 0.0f, 2.0f))
        changed = true;
    if (ImGui::SliderFloat("thunderbolt_period", &cur->bolt_period, 0.0f, 10.0f))
        changed = true;
    if (ImGui::SliderFloat("water_intensity", &cur->m_fWaterIntensity, 0.0f, 2.0f))
        changed = true;
    if (ImGui::SliderFloat("wind_velocity", &cur->wind_velocity, 0.0f, 100.0f))
        changed = true;
    if (ImGui::SliderFloat("wind_direction", &cur->wind_direction, 0.0f, 360.0f))
        changed = true;
    if (changed)
        modifiedWeathers.insert(env.CurrentWeatherName);
    if (ImGui::Button("Save")) {
        for (auto name : modifiedWeathers)
            saveWeather(name, env.WeatherCycles[name]);
        modifiedWeathers.clear();
    }
    ImGui::End();
}
