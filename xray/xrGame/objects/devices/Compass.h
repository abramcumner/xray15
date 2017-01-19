#pragma once

#include "../../AdvancedDetector.h"

class CCompass : public CAdvancedDetector
{
public:
	virtual void Load(LPCSTR section);
	virtual void shedule_Update(u32 dt);

protected:
	virtual void UpdateAf();
};