#include "pch_script.h"
#include "GameObjectEvents.h"
#include "GameObject.h"
#include "script_callback_ex.h"

void GameObject_OnMouseWheel(CGameObject* obj, int direction)
{
    if (obj)
        obj->callback(GameObject::eOnMouseWheel)(direction);
}

void GameObject_OnMouseMove(CGameObject* obj, int dx, int dy)
{
    if (obj)
        obj->callback(GameObject::eOnMouseMove)(dx, dy);
}

void GameObject_OnKeyboardPress(CGameObject* obj, int key)
{
    if (obj)
        obj->callback(GameObject::eOnKeyPress)(key);
}

void GameObject_OnKeyboardRelease(CGameObject* obj, int key)
{
    if (obj)
        obj->callback(GameObject::eOnKeyRelease)(key);
}

void GameObject_OnKeyboardHold(CGameObject* obj, int key)
{
    if (obj)
        obj->callback(GameObject::eOnKeyHold)(key);
}