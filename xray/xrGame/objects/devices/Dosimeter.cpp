#include "stdafx.h"
#include "Dosimeter.h"
#include "../../player_hud.h"
#include "../../../Include/xrRender/UIRender.h"
#include "../../ui/UIXmlInit.h"
#include "../../ui/xrUIXmlParser.h"
#include "../../ui/UIStatic.h"

#include "../../ui/UIHudStatesWnd.h"
#include "../../Level.h"
#include "../../HUDManager.h"
#include "../../ui/UIMainIngameWnd.h"

void CDosimeter::Load(LPCSTR section)
{
	CHudItemObject::Load(section);

	m_sounds.LoadSound(section, "snd_draw", "sndShow");
	m_sounds.LoadSound(section, "snd_holster", "sndHide");
}

void CDosimeter::shedule_Update(u32 dt)
{
	CHudItemObject::shedule_Update(dt);

	if (!IsWorking())			return;

	Position().set(H_Parent()->Position());
}

void CDosimeter::UpdateAf()
{

}

void CDosimeter::CreateUI()
{
	R_ASSERT(NULL == m_ui);
	m_ui = xr_new<CUIDosimeter>();
	ui().construct(this);
}

CUIDosimeter&  CDosimeter::ui()
{
	return *((CUIDosimeter*)m_ui);
}

bool  CDosimeter::render_item_3d_ui_query()
{
	return IsWorking();
}

void CDosimeter::render_item_3d_ui()
{
	R_ASSERT(HudItemData());
	CCustomDetector::render_item_3d_ui();
	ui().Draw();
	//	Restore cull mode
	UIRender->CacheSetCullMode(IUIRender::cmCCW);
}

void CUIDosimeter::construct(CDosimeter* p)
{
	m_parent = p;
	CUIXml								uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, "ui_dosimeter.xml");

	CUIXmlInit							xml_init;

	xml_init.InitWindow(uiXml, "dosimeter", 0, this);

	m_wrk_area = xr_new<CUIStatic>();
	xml_init.InitStatic(uiXml, "dosimeter:wrk_area", 0, m_wrk_area);
	m_wrk_area->SetAutoDelete(true);
	AttachChild(m_wrk_area);

	m_seg1 = xr_new<CUIStatic>();
	xml_init.InitStatic(uiXml, "dosimeter:seg1", 0, m_seg1);
	m_seg1->SetAutoDelete(true);
	AttachChild(m_seg1);

	m_seg2 = xr_new<CUIStatic>();
	xml_init.InitStatic(uiXml, "dosimeter:seg2", 0, m_seg2);
	m_seg2->SetAutoDelete(true);
	AttachChild(m_seg2);

	m_seg3 = xr_new<CUIStatic>();
	xml_init.InitStatic(uiXml, "dosimeter:seg3", 0, m_seg3);
	m_seg3->SetAutoDelete(true);
	AttachChild(m_seg3);

	m_seg4 = xr_new<CUIStatic>();
	xml_init.InitStatic(uiXml, "dosimeter:seg4", 0, m_seg4);
	m_seg4->SetAutoDelete(true);
	AttachChild(m_seg4);

	Fvector _map_attach_p = pSettings->r_fvector3(m_parent->cNameSect(), "ui_p");
	Fvector _map_attach_r = pSettings->r_fvector3(m_parent->cNameSect(), "ui_r");

	_map_attach_r.mul(PI / 180.f);
	m_map_attach_offset.setHPB(_map_attach_r.x, _map_attach_r.y, _map_attach_r.z);
	m_map_attach_offset.translate_over(_map_attach_p);
}

void CUIDosimeter::update()
{
	CUIArtefactDetectorBase::update();

	float rad = HUD().GetUI()->UIMainIngameWnd->get_hud_states()->get_main_sensor_value();
	rad *= 1000;
	if (rad > 9999.0f) rad = 9999.0f;
	string16 s;
	sprintf_s(s, "%05.0lf", rad);
	string16 tex;
	sprintf_s(tex, "green_%c", s[1]);
	m_seg1->InitTextureEx(tex, "hud\\p3d");
	sprintf_s(tex, "green_%c", s[2]);
	m_seg2->InitTextureEx(tex, "hud\\p3d");
	sprintf_s(tex, "green_%c", s[3]);
	m_seg3->InitTextureEx(tex, "hud\\p3d");
	sprintf_s(tex, "green_%c", s[4]);
	m_seg4->InitTextureEx(tex, "hud\\p3d");

	CUIWindow::Update();
}

void CUIDosimeter::Draw()
{
	Fmatrix						LM;
	GetUILocatorMatrix(LM);

	IUIRender::ePointType bk = UI()->m_currentPointType;

	UI()->m_currentPointType = IUIRender::pttLIT;

	UIRender->CacheSetXformWorld(LM);
	UIRender->CacheSetCullMode(IUIRender::cmNONE);

	CUIWindow::Draw();

	UI()->m_currentPointType = bk;
}

void CUIDosimeter::GetUILocatorMatrix(Fmatrix& _m)
{
	Fmatrix	trans = m_parent->HudItemData()->m_item_transform;
	u16 bid = m_parent->HudItemData()->m_model->LL_BoneID("substrate_joint");
	Fmatrix cover_bone = m_parent->HudItemData()->m_model->LL_GetTransform(bid);
	_m.mul(trans, cover_bone);
	_m.mulB_43(m_map_attach_offset);
}