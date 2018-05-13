// xrXRC.h: interface for the xrXRC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XRXRC_H__9AA25268_621F_4FCA_BD75_AF2E9822B8E3__INCLUDED_)
#define AFX_XRXRC_H__9AA25268_621F_4FCA_BD75_AF2E9822B8E3__INCLUDED_
#pragma once

#include "../xrCDB/xrCDB.h"

template<class Payload>
class xrXRC  
{
	CDB::COLLIDER_Generic<Payload>	CL;
public:
	IC void			ray_options		(u32 f)		
	{ 
		CL.ray_options(f); 
	}
	IC void			ray_query		(const CDB::MODEL_Generic<Payload> *m_def, const Fvector& r_start,  const Fvector& r_dir, float r_range = 10000.f)
	{
#ifdef DEBUG
		Statistic.clRAY.Begin();
#endif
		CL.ray_query(m_def,r_start,r_dir,r_range);
#ifdef DEBUG
		Statistic.clRAY.End	();
#endif
	}
	
	IC void			box_options		(u32 f)	
	{	
		CL.box_options(f);
	}
	IC void			box_query		(const CDB::MODEL_Generic<Payload> *m_def, const Fvector& b_center, const Fvector& b_dim)
	{
#ifdef DEBUG
		Statistic.clBOX.Begin();
#endif
		CL.box_query(m_def,b_center,b_dim);
#ifdef DEBUG
		Statistic.clBOX.End	();
#endif
	}
	
	IC void			frustum_options	(u32 f)
	{
		CL.frustum_options(f);
	}
	IC void			frustum_query	(const CDB::MODEL_Generic<Payload> *m_def, const CFrustum& F)
	{
#ifdef DEBUG
		Statistic.clFRUSTUM.Begin();
#endif
		CL.frustum_query(m_def,F);
#ifdef DEBUG
		Statistic.clFRUSTUM.End	();
#endif
	}
	
	IC CDB::RESULT_Generic<Payload>*	r_begin			()	{	return CL.r_begin();		};
	IC CDB::RESULT_Generic<Payload>*	r_end			()	{	return CL.r_end();			};
	IC void			r_free			()	{	CL.r_free();				}
	IC int			r_count			()	{	return CL.r_count();		};
	IC void			r_clear			()	{	CL.r_clear();				};
	IC void			r_clear_compact	()	{	CL.r_clear_compact();		};
};
extern ENGINE_API xrXRC<CDB::GamePayload> XRC;

#endif // !defined(AFX_XRXRC_H__9AA25268_621F_4FCA_BD75_AF2E9822B8E3__INCLUDED_)
