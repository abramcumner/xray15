#ifndef	FactoryPtr_included
#define FactoryPtr_included
#pragma once

#include "RenderFactory.h"
#include "../../Include/xrAPI/xrAPI.h"

template<class T> 
class FactoryPtr
{
public:
	FactoryPtr() { CreateObject();}
	~FactoryPtr() { DestroyObject();}

	FactoryPtr(const FactoryPtr<T> &_in)
	{
		CreateObject();
		m_pObject->Copy(*_in.m_pObject);
	}

	FactoryPtr& operator=( const FactoryPtr &_in)
	{
		m_pObject->Copy(*_in.m_pObject);
		return *this;
	}

	T& operator*() const {return *m_pObject;}
	T* operator->() const {return m_pObject;}

	// unspecified bool type
	typedef T const * (FactoryPtr::*unspecified_bool_type) () const;
	operator unspecified_bool_type () const	{return (!m_pObject ? 0 : &FactoryPtr::get);}
	bool operator!	() const { return m_pObject == 0;}

private:
	void CreateObject();
	void DestroyObject();
	T const* get() const { return m_pObject; }
private:
	T*					m_pObject;
};


#ifndef _EDITOR
template<>
inline void FactoryPtr<IUISequenceVideoItem>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateUISequenceVideoItem();
}
template<>
inline void FactoryPtr<IUISequenceVideoItem>::DestroyObject(void)
{
	RenderFactory->DestroyUISequenceVideoItem(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IUIShader>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateUIShader();
}
template<>
inline void FactoryPtr<IUIShader>::DestroyObject(void)
{
	RenderFactory->DestroyUIShader(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IStatGraphRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateStatGraphRender();
}
template<>
inline void FactoryPtr<IStatGraphRender>::DestroyObject(void)
{
	RenderFactory->DestroyStatGraphRender(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IConsoleRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateConsoleRender();
}
template<>
inline void FactoryPtr<IConsoleRender>::DestroyObject(void)
{
	RenderFactory->DestroyConsoleRender(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IRenderDeviceRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateRenderDeviceRender();
}
template<>
inline void FactoryPtr<IRenderDeviceRender>::DestroyObject(void)
{
	RenderFactory->DestroyRenderDeviceRender(m_pObject);
	m_pObject = NULL;
}

#	ifdef DEBUG
template<>
inline void FactoryPtr<IObjectSpaceRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateObjectSpaceRender();
}
template<>
inline void FactoryPtr<IObjectSpaceRender>::DestroyObject(void)
{
	RenderFactory->DestroyObjectSpaceRender(m_pObject);
	m_pObject = NULL;
}
#	endif // DEBUG

template<>
inline void FactoryPtr<IApplicationRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateApplicationRender();
}
template<>
inline void FactoryPtr<IApplicationRender>::DestroyObject(void)
{
	RenderFactory->DestroyApplicationRender(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IWallMarkArray>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateWallMarkArray();
}
template<>
inline void FactoryPtr<IWallMarkArray>::DestroyObject(void)
{
	RenderFactory->DestroyWallMarkArray(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IStatsRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateStatsRender();
}
template<>
inline void FactoryPtr<IStatsRender>::DestroyObject(void)
{
	RenderFactory->DestroyStatsRender(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IFlareRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateFlareRender();
}
template<>
inline void FactoryPtr<IFlareRender>::DestroyObject(void)
{
	RenderFactory->DestroyFlareRender(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IThunderboltRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateThunderboltRender();
}
template<>
inline void FactoryPtr<IThunderboltRender>::DestroyObject(void)
{
	RenderFactory->DestroyThunderboltRender(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IThunderboltDescRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateThunderboltDescRender();
}
template<>
inline void FactoryPtr<IThunderboltDescRender>::DestroyObject(void)
{
	RenderFactory->DestroyThunderboltDescRender(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<ILensFlareRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateLensFlareRender();
}
template<>
inline void FactoryPtr<ILensFlareRender>::DestroyObject(void)
{
	RenderFactory->DestroyLensFlareRender(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IRainRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateRainRender();
}
template<>
inline void FactoryPtr<IRainRender>::DestroyObject(void)
{
	RenderFactory->DestroyRainRender(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IEnvironmentRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateEnvironmentRender();
}
template<>
inline void FactoryPtr<IEnvironmentRender>::DestroyObject(void)
{
	RenderFactory->DestroyEnvironmentRender(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IEnvDescriptorRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateEnvDescriptorRender();
}
template<>
inline void FactoryPtr<IEnvDescriptorRender>::DestroyObject(void)
{
	RenderFactory->DestroyEnvDescriptorRender(m_pObject);
	m_pObject = NULL;
}
template<>
inline void FactoryPtr<IEnvDescriptorMixerRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateEnvDescriptorMixerRender();
}
template<>
inline void FactoryPtr<IEnvDescriptorMixerRender>::DestroyObject(void)
{
	RenderFactory->DestroyEnvDescriptorMixerRender(m_pObject);
	m_pObject = NULL;
}

#endif // _EDITOR

template<>
inline void FactoryPtr<IFontRender>::CreateObject(void)
{
	m_pObject = RenderFactory->CreateFontRender();
}
template<>
inline void FactoryPtr<IFontRender>::DestroyObject(void)
{
	RenderFactory->DestroyFontRender(m_pObject);
	m_pObject = NULL;
}

#endif	//	FactoryPtr_included