#include "stdafx.h"
#include "embedded_editor_stats.h"
#include "embedded_editor_helper.h"

void ShowStats(bool& show)
{
    ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImguiWnd wnd("Stats", &show);
    if (wnd.Collapsed)
        return;
}
