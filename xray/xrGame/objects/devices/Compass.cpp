#include "stdafx.h"
#include "Compass.h"

void CCompass::Load(LPCSTR section)
{
	CHudItemObject::Load(section);

	m_sounds.LoadSound(section, "snd_draw", "sndShow");
	m_sounds.LoadSound(section, "snd_holster", "sndHide");
}

void CCompass::shedule_Update(u32 dt)
{
	CHudItemObject::shedule_Update(dt);

	if (!IsWorking())			return;

	Position().set(H_Parent()->Position());
}

void CCompass::UpdateAf()
{
	ui().SetValue(0.0f, Fvector().set(0.0f, 0.0f, 2.0f));
}