#pragma once

class CObject;
void ShowPropEditor(bool& show);
void SetPropObject(CObject* obj);
void ShowLuaBinder(bool& show);
void ShowLogicEditor(bool& show);

extern bool show_lua_binder;
extern bool show_logic_editor;