#include "stdafx.h"
#include "embedded_editor_map.h"
#include "HUDmanager.h"
#include "Level.h"
#include "UIGameCustom.h"
#include "embedded_editor_helper.h"
#include "ui/UIMainIngameWnd.h"
#include "ui/UIMap.h"
#include "ui/UIPdaWnd.h"

void SetLeftTopConer();
void SetRightBottomConer();
void OffsetConer(int what, float dist);

void ShowMapEditor(bool& show)
{
    ImguiWnd wnd("Map Editor", &show);
    if (wnd.Collapsed)
        return;
    ImVec2 size = ImGui::CalcTextSize("LT");
    size.y = size.x + 2;

    ImGui::Indent(size.x);
    if (ImGui::Button("^##lt", size))
        OffsetConer(3, 1.0f);
    ImGui::Unindent(size.x);
    if (ImGui::Button("<##lt", size))
        OffsetConer(0, -1.0f);
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("LT", size))
        SetLeftTopConer();
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button(">##lt", size))
        OffsetConer(0, 1.0f);
    ImGui::Indent(size.x);
    if (ImGui::Button("v##lt", size))
        OffsetConer(3, -1.0f);
    ImGui::Unindent(size.x);

    ImGui::Spacing();

    ImGui::Indent(5 * size.x);
    if (ImGui::Button("^##rb", size))
        OffsetConer(1, 1.0f);
    ImGui::Unindent(size.x);
    if (ImGui::Button("<##rb", size))
        OffsetConer(2, -1.0f);
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("RB", size))
        SetRightBottomConer();
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button(">##rb", size))
        OffsetConer(2, 1.0f);
    ImGui::Indent(size.x);
    if (ImGui::Button("v##rb", size))
        OffsetConer(1, -1.0f);
    ImGui::Unindent(0.0f);

    ImGui::Separator();
}

// static CUIMiniMap* miniMap = nullptr;
//
// CUIMiniMap* FindMiniMap(CUIWindow* parent)
//{
//    for (auto&& el : parent->GetChildWndList()) {
//        CUIMiniMap* map = dynamic_cast<CUIMiniMap*>(el);
//        if (map)
//            return map;
//        map = FindMiniMap(el);
//        if (map)
//            return map;
//    }
//    return nullptr;
//}
//
// CUIMiniMap* GetMiniMap()
//{
//    if (miniMap)
//        return miniMap;
//    miniMap = FindMiniMap(HUD().GetUI()->UIMainIngameWnd);
//    return miniMap;
//}

void SetLeftTopConer()
{
    CUICustomMap* map = HUD().GetUI()->UIMainIngameWnd->MiniMap();
    Frect r = map->BoundRect();
    r.left = Device.vCameraPosition.x;
    r.bottom = Device.vCameraPosition.z;
    map->SetBoundRect(r);
}

void SetRightBottomConer()
{
    CUICustomMap* map = HUD().GetUI()->UIMainIngameWnd->MiniMap();
    Frect r = map->BoundRect();
    r.right = Device.vCameraPosition.x;
    r.top = Device.vCameraPosition.z;
    map->SetBoundRect(r);
}

void OffsetConer(int what, float dist)
{
    CUICustomMap* map = HUD().GetUI()->UIMainIngameWnd->MiniMap();
    Frect r = map->BoundRect();
    r.m[what] += dist;
    map->SetBoundRect(r);
}
