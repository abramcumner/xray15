#pragma once
#include "../../CustomDetector.h"

class CUIDosimeter;

class CDosimeter : public CCustomDetector
{
public:
	void Load(LPCSTR section) override;
	void shedule_Update(u32 dt) override;

	void render_item_3d_ui() override;
	bool render_item_3d_ui_query() override;

protected:
	void UpdateAf() override;
	void CreateUI() override;
	CUIDosimeter& ui();
};

class CUIDosimeter : public CUIArtefactDetectorBase, public CUIWindow
{
public:
	void update() override;
	void Draw() override;

	void construct(CDosimeter* p);

private:
	CUIStatic* m_wrk_area;
	CUIStatic* m_seg1;
	CUIStatic* m_seg2;
	CUIStatic* m_seg3;
	CUIStatic* m_seg4;
	CDosimeter* m_parent;
	Fmatrix m_map_attach_offset;

	void GetUILocatorMatrix(Fmatrix& _m);
};