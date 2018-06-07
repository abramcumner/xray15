#include "stdafx.h"
#include "embedded_editor_le.h"
#include "embedded_editor_helper.h"

bool isRenderAiMap = false;
bool isRenderSpawnElement = false;

void showMainWindow(bool& show);

void showLeEditor(bool& show) { showMainWindow(show); }

void showMainWindow(bool& show)
{
    ImguiWnd wnd("Level Editor", &show);
    if (wnd.Collapsed)
        return;
    ImGui::Checkbox("AI Map", &isRenderAiMap);
    ImGui::Checkbox("Spawn Element", &isRenderSpawnElement);
}
