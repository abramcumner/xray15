#pragma once

#include "DetectList.h"

class CZoneList : public CDetectList<CCustomZone>
{
protected:
    virtual BOOL feel_touch_contact(CObject* O);

public:
    CZoneList();
    virtual ~CZoneList();
};