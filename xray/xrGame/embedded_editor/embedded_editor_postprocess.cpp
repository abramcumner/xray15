#include "stdafx.h"
#include "embedded_editor_postprocess.h"
#include "embedded_editor_helper.h"

void ShowPostProcessEditor(bool& show)
{
    ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImguiWnd wnd("PostProcess", &show);
    if (wnd.Collapsed)
        return;
}