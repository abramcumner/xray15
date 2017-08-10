#pragma once

class CGameObject;

void GameObject_OnMouseWheel(CGameObject* obj, int direction);
void GameObject_OnMouseMove(CGameObject* obj, int dx, int dy);
void GameObject_OnKeyboardPress(CGameObject* obj, int key);
void GameObject_OnKeyboardRelease(CGameObject* obj, int key);
void GameObject_OnKeyboardHold(CGameObject* obj, int key);
