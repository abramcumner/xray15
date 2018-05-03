#include "stdafx.h"
#include "embedded_editor_se.h"
#include "embedded_editor_helper.h"

void showSeEditor(bool& show)
{
	ImguiWnd wnd("Shader Editor", &show);
	if (wnd.Collapsed)
		return;
}