#include "stdafx.h"
#include "editor_render.h"
#include "editor_render_ai_map.h"

extern bool isRenderAiMap;

void embedded_editor_render()
{
    if (isRenderAiMap)
        renderAiMap();
}
