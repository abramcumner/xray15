#include "stdafx.h"
#include "xrCDB.h"

namespace CDB
{
	u32 VertexCollector::addVert(const Fvector& v)
	{
		auto size = verts.size();
		verts.push_back(v);
		return size;
	}

	u32 VertexCollector::addVertPacked(const Fvector& V, float eps)
	{
		xr_vector<Fvector>::iterator I,E;
		I=verts.begin();	E=verts.end();
		for (;I!=E;I++)		if (I->similar(V,eps)) return u32(I-verts.begin());
		verts.push_back		(V);
		return verts.size	()-1;
	}

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#pragma warning(pop)

    //IC BOOL similar(TRI& T1, TRI& T2)
    //{
    //    if ((T1.verts[0]==T2.verts[0]) && (T1.verts[1]==T2.verts[1]) && (T1.verts[2]==T2.verts[2]) && (T1.dummy1==T2.dummy1)) return TRUE;
    //    if ((T1.verts[0]==T2.verts[0]) && (T1.verts[2]==T2.verts[1]) && (T1.verts[1]==T2.verts[2]) && (T1.dummy1==T2.dummy1)) return TRUE;
    //    if ((T1.verts[2]==T2.verts[0]) && (T1.verts[0]==T2.verts[1]) && (T1.verts[1]==T2.verts[2]) && (T1.dummy1==T2.dummy1)) return TRUE;
    //    if ((T1.verts[2]==T2.verts[0]) && (T1.verts[1]==T2.verts[1]) && (T1.verts[0]==T2.verts[2]) && (T1.dummy1==T2.dummy1)) return TRUE;
    //    if ((T1.verts[1]==T2.verts[0]) && (T1.verts[0]==T2.verts[1]) && (T1.verts[2]==T2.verts[2]) && (T1.dummy1==T2.dummy1)) return TRUE;
    //    if ((T1.verts[1]==T2.verts[0]) && (T1.verts[2]==T2.verts[1]) && (T1.verts[0]==T2.verts[2]) && (T1.dummy1==T2.dummy1)) return TRUE;
    //    return FALSE;
    //}
    void Collector_Base::remove_duplicate_T( )
    {
		//for (u32 f=0; f<faces.size(); f++)
		//{
		//	for (u32 t=f+1; t<faces.size();)
		//	{
		//		if (t==f)	continue;
  //              TRI& T1	= faces[f];
  //              TRI& T2	= faces[t];
  //              if (similar(T1,T2)){
  //              	faces[t] = faces.back();
  //                  faces.pop_back();
  //              }else{
  //              	t++;
  //              }
  //          }
  //      }
    }

	VertexCollectorPacked::VertexCollectorPacked(const Fbox &bb, int apx_vertices)
	{
		// Params
		VMscale.set		(bb.max.x-bb.min.x, bb.max.y-bb.min.y, bb.max.z-bb.min.z);
		VMmin.set		(bb.min);
		VMeps.set		(VMscale.x/clpMX/2,VMscale.y/clpMY/2,VMscale.z/clpMZ/2);
		VMeps.x			= (VMeps.x<EPS_L)?VMeps.x:EPS_L;
		VMeps.y			= (VMeps.y<EPS_L)?VMeps.y:EPS_L;
		VMeps.z			= (VMeps.z<EPS_L)?VMeps.z:EPS_L;

		// Preallocate memory
		verts.reserve	(apx_vertices);
		int		_size	= (clpMX+1)*(clpMY+1)*(clpMZ+1);
		int		_average= (apx_vertices/_size)/2;
		for (int ix=0; ix<clpMX+1; ix++)
			for (int iy=0; iy<clpMY+1; iy++)
				for (int iz=0; iz<clpMZ+1; iz++)
					VM[ix][iy][iz].reserve	(_average);
	}

	u32		VertexCollectorPacked::VPack(const Fvector& V)
	{
		u32 P = 0xffffffff;

		u32 ix,iy,iz;
		ix = iFloor(float(V.x-VMmin.x)/VMscale.x*clpMX);
		iy = iFloor(float(V.y-VMmin.y)/VMscale.y*clpMY);
		iz = iFloor(float(V.z-VMmin.z)/VMscale.z*clpMZ);

		//		R_ASSERT(ix<=clpMX && iy<=clpMY && iz<=clpMZ);
		clamp(ix,(u32)0,(u32)clpMX);	clamp(iy,(u32)0,(u32)clpMY);	clamp(iz,(u32)0,(u32)clpMZ);

		{
			DWORDList* vl;
			vl = &(VM[ix][iy][iz]);
			for(DWORDIt it=vl->begin();it!=vl->end(); it++)
				if( verts[*it].similar(V) )	{
					P = *it;
					break;
				}
		}
		if (0xffffffff==P)
		{
			P = verts.size();
			verts.push_back(V);

			VM[ix][iy][iz].push_back(P);

			u32 ixE,iyE,izE;
			ixE = iFloor(float(V.x+VMeps.x-VMmin.x)/VMscale.x*clpMX);
			iyE = iFloor(float(V.y+VMeps.y-VMmin.y)/VMscale.y*clpMY);
			izE = iFloor(float(V.z+VMeps.z-VMmin.z)/VMscale.z*clpMZ);

			//			R_ASSERT(ixE<=clpMX && iyE<=clpMY && izE<=clpMZ);
			clamp(ixE,(u32)0, (u32)clpMX);	clamp(iyE,(u32)0, (u32)clpMY);	clamp(izE,(u32)0, (u32)clpMZ);

			if (ixE!=ix)							VM[ixE][iy][iz].push_back	(P);
			if (iyE!=iy)							VM[ix][iyE][iz].push_back	(P);
			if (izE!=iz)							VM[ix][iy][izE].push_back	(P);
			if ((ixE!=ix)&&(iyE!=iy))				VM[ixE][iyE][iz].push_back	(P);
			if ((ixE!=ix)&&(izE!=iz))				VM[ixE][iy][izE].push_back	(P);
			if ((iyE!=iy)&&(izE!=iz))				VM[ix][iyE][izE].push_back	(P);
			if ((ixE!=ix)&&(iyE!=iy)&&(izE!=iz))	VM[ixE][iyE][izE].push_back	(P);
		}
		return P;
	}

	void	VertexCollectorPacked::clear()
	{
		verts.clear_and_free	();
		for (u32 _x=0; _x<=clpMX; _x++)
			for (u32 _y=0; _y<=clpMY; _y++)
				for (u32 _z=0; _z<=clpMZ; _z++)
					VM[_x][_y][_z].clear_and_free	();
	}
};
