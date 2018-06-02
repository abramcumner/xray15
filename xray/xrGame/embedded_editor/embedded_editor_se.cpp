#include "stdafx.h"
#include "embedded_editor_se.h"
#include "embedded_editor_helper.h"
#include <addons/ImGuiColorTextEdit/TextEditor.h>

#pragma pack(push, 4)
using ref_state = void*;

using R_constant_table = void*;
using ref_input_sign = void*;

struct ECORE_API SPS : public xr_resource_named {
    void* ps;
    R_constant_table constants;
};
typedef resptr_core<SPS, resptr_base<SPS>> ref_ps;

struct ECORE_API SVS_R1 : public xr_resource_named {
    void* vs;
    R_constant_table constants;
};
typedef resptr_core<SVS_R1, resptr_base<SVS_R1>> ref_vs_r1;

struct ECORE_API SVS_R3 : public xr_resource_named {
    void* vs;
    R_constant_table constants;
    ref_input_sign signature;
};
typedef resptr_core<SVS_R3, resptr_base<SVS_R3>> ref_vs_r3;

using ref_gs = void*;
using ref_ctable = void*;
using ref_texture_list = void*;
using ref_constant_list = void*;
using ref_matrix_list = void*;

struct SPassR1R2 : public xr_resource_flagged {
    ref_state state;
    ref_ps ps;
    ref_vs_r1 vs;
    ref_ctable constants;
    ref_texture_list T;
    ref_constant_list C;
};
typedef resptr_core<SPassR1R2, resptr_base<SPassR1R2>> ref_passR1R2;

struct SPassR3 : public xr_resource_flagged {
    ref_state state;
    ref_ps ps;
    ref_vs_r3 vs;
    ref_gs gs;
    ref_ctable constants;
    ref_texture_list T;
    ref_constant_list C;
};
typedef resptr_core<SPassR3, resptr_base<SPassR3>> ref_passR3;

struct ShaderElement : public xr_resource_flagged {
public:
    struct Sflags {
        u32 iPriority : 2;
        u32 bStrictB2F : 1;
        u32 bEmissive : 1;
        u32 bDistort : 1;
        u32 bWmark : 1;
    };

public:
    Sflags flags;
    svector<ref_passR1R2, 2> passes;

    ShaderElement();
    ~ShaderElement();
    BOOL equal(ShaderElement& S);
    BOOL equal(ShaderElement* S);
};
typedef resptr_core<ShaderElement, resptr_base<ShaderElement>> ref_selement;

struct Shader : public xr_resource_named {
public:
    ref_selement
        E[6]; // R1 - 0=norm_lod0(det),	1=norm_lod1(normal),	2=L_point,		3=L_spot,	4=L_for_models,
              // R2 - 0=deffer,			1=norm_lod1(normal),	2=psm,			3=ssm,		4=dsm
    ~Shader();
    BOOL equal(Shader& S);
    BOOL equal(Shader* S);
};
#pragma pack(pop)

ref_ps curPs;
TextEditor editor;
bool isShowSourceEditor = false;
void showSourceEditor();

void showSeEditor(bool& show)
{
    ImguiWnd wnd("Shader Editor", &show);
    if (wnd.Collapsed)
        return;

    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::HLSL());

    const auto& shaders = Device.m_pRender->GetShaders();
    ImGui::Text("Engine shaders(%d):", shaders.size());
    ImGui::Separator();

    xr_set<ref_ps> pss;
    for (const auto& sh : shaders)
        for (const auto& el : sh->E) {
            if (!el)
                continue;
            for (const auto& pass : el->passes)
                pss.insert(pass->ps);
        }
    xr_vector<ref_ps> psList(pss.size());
    int selected = -1;
    int i = 0;
    for (const auto& el : pss) {
        psList[i] = el;
        if (el == curPs)
            selected = i;
        i++;
    }
    ImGui::Text("Pixel shaders(%d):", pss.size());
    if (ImGui::ListBox("", &selected,
            [](void* data, int idx, const char** out_text) -> bool {
                xr_vector<ref_ps>* psList = (xr_vector<ref_ps>*)data;
                *out_text = (*psList)[idx]->cName.c_str();
                return true;
            },
            &psList, pss.size())) {
        if (selected == -1) {
            curPs = ref_ps();
            editor.SetText(std::string());
        } else {
            curPs = psList[selected];
            string_path cname;
            strconcat(sizeof(cname), cname, ::Render->getShaderPath(), curPs->cName.c_str(), ".ps");
            FS.update_path(cname, "$game_shaders$", cname);
            IReader* R = FS.r_open(cname);
            R_ASSERT2(R, cname);
            xr_vector<char> buf(R->length());
            CopyMemory(&buf[0], R->pointer(), R->length());
            FS.r_close(R);
            std::string text(buf.begin(), buf.end());
            editor.SetText(text);
            isShowSourceEditor = true;
        }
    }
    ImGui::Separator();

    showSourceEditor();
}

void showSourceEditor()
{
    if (!isShowSourceEditor)
        return;
    ImGui::SetNextWindowSize(ImVec2(250, 400), ImGuiCond_FirstUseEver);
    ImguiWnd wnd("Source", &isShowSourceEditor);
    if (wnd.Collapsed)
        return;
    editor.Render("", ImVec2(-1.0f, -20.0f), true);
    if (ImGui::Button("Compile")) {
    }
}
