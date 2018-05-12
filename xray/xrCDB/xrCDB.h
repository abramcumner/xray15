#ifndef XRCDB_H
#define XRCDB_H

#pragma once
// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the XRCDB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// XRCDB_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#include <xmmintrin.h>

#ifdef XRCDB_EXPORTS
#define XRCDB_API __declspec(dllexport)
#else
#define XRCDB_API __declspec(dllimport)
#endif
#ifdef M_VISUAL
#define ALIGN(a) __declspec(align(a))
#else
#define ALIGN(a)
#endif

// forward declarations
class CFrustum;
namespace Opcode {
	class OPCODE_Model;
	class AABBNoLeafNode;
};
template<bool bClass3, bool bFirst> class box_collider;
template <bool bClass3, bool bFirst> class frustum_collider;
template <bool bUseSSE, bool bCull, bool bFirst, bool bNearest> class _MM_ALIGN16 ray_collider;

#pragma pack(push,8)
namespace CDB
{
	// Triangle
	struct XRCDB_API TRI_Base
	{
		u32		verts[3];		// 3*4 = 12b
		IC u32	IDvert(u32 ID) { return verts[ID]; }
	};

	template<class Payload>
	struct TRI_Generic final: public TRI_Base, Payload
	{
		TRI_Generic() = default;
		TRI_Generic(u32 v0, u32 v1, u32 v2, const Payload& data) : Payload(data)
		{
			verts[0] = v0;
			verts[1] = v1;
			verts[2] = v2;
		}
	};

	struct GamePayload
	{
		GamePayload() = default;
		GamePayload(u16 mat, u16 sect) { material = mat; suppress_shadows = 0; suppress_wm = 0; sector = sect; }

		u32 material : 14;
		u32 suppress_shadows : 1;
		u32 suppress_wm : 1;
		u32 sector : 16;
	};

	struct WorkPayload
	{
		void*	data;
	};

	using TRI = TRI_Generic<GamePayload>;
	using TRI_Work = TRI_Generic<WorkPayload>;

	// Build callback
	typedef		void __stdcall	build_callback	(Fvector* V, int Vcnt, void* T, int Tcnt, void* params);

	// Model definition
	class		XRCDB_API		MODEL_Base
	{
	public:
		MODEL_Base(size_t triSize);
		~MODEL_Base();

		IC void					syncronize()	const
		{
			if (S_READY != status)
			{
				Log("! WARNING: syncronized CDB::query");
				xrCriticalSection*	C = (xrCriticalSection*)&cs;
				C->Enter();
				C->Leave();
			}
		}

		u32						memory();

	protected:
		virtual void* allocTris(int Tcnt) = 0;
		static	void			build_thread(void*);
		void					build_internal(Fvector* V, int Vcnt, void* T, int Tcnt, build_callback* bc = NULL, void* bcp = NULL);
		void					build(Fvector* V, int Vcnt, void* T, int Tcnt, build_callback* bc = NULL, void* bcp = NULL);

		// tris
		void*					tris;
		int						tris_count;
		Fvector*				verts;
		int						verts_count;

	private:
		friend class COLLIDER_Base;
		enum
		{
			S_READY = 0,
			S_INIT = 1,
			S_BUILD = 2,
			S_forcedword = u32(-1)
		};

		xrCriticalSection		cs;
		Opcode::OPCODE_Model*	tree;
		u32						status;		// 0=ready, 1=init, 2=building
		size_t					m_triSize;
	};

	template<class Payload>
	class MODEL_Generic final: public MODEL_Base
	{
	public:
		using Face = TRI_Generic<Payload>;

		MODEL_Generic() : MODEL_Base(sizeof(Face)) {}

		IC Fvector*				get_verts() { return verts; }
		IC const Fvector*		get_verts()	const { return verts; }
		IC int					get_verts_count()	const { return verts_count; }
		IC const Face*			get_tris()	const { return (const Face*)tris; }
		IC Face*				get_tris() { return (Face*)tris; }
		IC int					get_tris_count()	const { return tris_count; }

		void					build(Fvector* V, int Vcnt, Face* T, int Tcnt, build_callback* bc = NULL, void* bcp = NULL)
		{
			MODEL_Base::build(V, Vcnt, T, Tcnt, bc, bcp);
		}

	protected:
		void* allocTris(int count) override
		{
			return xr_alloc<Face>(count);
		}
	};

	using MODEL = MODEL_Generic<GamePayload>;
	using MODEL_Work = MODEL_Generic<WorkPayload>;


	// Collider result
	struct XRCDB_API RESULT_Base
	{
		Fvector			verts[3];
		int				id;
		float			range;
		float			u, v;
	};
	template<class Payload>
	struct RESULT_Generic final: public RESULT_Base, Payload
	{
	};
	using RESULT = RESULT_Generic<GamePayload>;
	using RESULT_Work = RESULT_Generic<WorkPayload>;

	// Collider Options
	enum {
		OPT_CULL		= (1<<0),
		OPT_ONLYFIRST	= (1<<1),
		OPT_ONLYNEAREST	= (1<<2),
		OPT_FULL_TEST   = (1<<3)		// for box & frustum queries - enable class III test(s)
	};

	// Collider itself
	class XRCDB_API COLLIDER_Base
	{
	public:
		COLLIDER_Base(size_t triSize);
		~COLLIDER_Base();

		ICF void		ray_options(u32 f) { ray_mode = f; }
		ICF void		box_options(u32 f) { box_mode = f; }
		ICF void		frustum_options(u32 f) { frustum_mode = f; }

		virtual void r_free() = 0;
		virtual int r_count() = 0;

	protected:
		void			ray_query(const MODEL_Base *m_def, const Fvector& r_start, const Fvector& r_dir, float r_range = 10000.f);
		void			box_query(const MODEL_Base *m_def, const Fvector& b_center, const Fvector& b_dim);
		void			frustum_query(const MODEL_Base *m_def, const CFrustum& F);

		virtual RESULT_Base& r_add(int id, Fvector verts[3], float range, float u, float v, const TRI_Base* tri) = 0;
		virtual RESULT_Base* r_first() = 0;
		size_t triSize() const;

	private:
		// Ray data and methods
		u32				ray_mode;
		u32				box_mode;
		u32				frustum_mode;
		size_t			_triSize;

		template<bool bClass3, bool bFirst> friend class box_collider;
		template <bool bClass3, bool bFirst> friend class frustum_collider;
		template <bool bUseSSE, bool bCull, bool bFirst, bool bNearest> friend class ray_collider;
	};

	template<class Payload>
	class COLLIDER_Generic final: public COLLIDER_Base
	{
	public:
		using Face = TRI_Generic<Payload>;
		using Result = RESULT_Generic<Payload>;

		COLLIDER_Generic() : COLLIDER_Base(sizeof(Face)) {}

		void ray_query(const MODEL_Generic<Payload> *m_def, const Fvector& r_start, const Fvector& r_dir, float r_range = 10000.f)
		{
			COLLIDER_Base::ray_query(m_def, r_start, r_dir, r_range);
		}
		void box_query(const MODEL_Generic<Payload> *m_def, const Fvector& b_center, const Fvector& b_dim)
		{
			COLLIDER_Base::box_query(m_def, b_center, b_dim);
		}
		void frustum_query(const MODEL_Generic<Payload> *m_def, const CFrustum& F)
		{
			COLLIDER_Base::frustum_query(m_def, F);
		}

		void r_free() override { rd.clear_and_free(); }
		int r_count() override { return rd.size(); };

		ICF Result*		r_begin() { return rd.data(); };
		ICF Result*		r_end() { return rd.data() + rd.size(); };
		ICF void		r_clear() { rd.clear_not_free(); };
		ICF void		r_clear_compact() { rd.clear_and_free(); };

	protected:
		RESULT_Base& r_add(int id, Fvector verts[3], float range, float u, float v, const TRI_Base* tri) override
		{
			Result r;
			r.id = id;
			for (int i = 0; i != 3; i++)
				r.verts[i] = verts[i];
			r.range = range;
			r.u = u;
			r.v = v;
			*static_cast<Payload*>(&r) = *static_cast<Payload*>((Face*)tri);
			rd.push_back(r);
			return rd.back();
		}
		RESULT_Base* r_first() override { return rd.data(); };

	private:
		// Result management
		xr_vector<Result>	rd;
	};

	using COLLIDER = COLLIDER_Generic<GamePayload>;
	using COLLIDER_Work = COLLIDER_Generic<WorkPayload>;

	//
	class XRCDB_API VertexCollector
	{
	public:
		Fvector*		getV() { return verts.data(); }
		size_t			getVS() { return verts.size(); }
		void			clear() { verts.clear(); }
		void			reserve(size_t count) { verts.reserve(count); }

	protected:
		u32				addVert(const Fvector& v);
		u32				addVertPacked(const Fvector& v, float eps = EPS);

	private:
		xr_vector<Fvector>	verts;
	};

	class XRCDB_API Collector_Base: public VertexCollector
	{
	private:
        void			remove_duplicate_T	( );

	};

	template<class Payload>
	class Collector_Generic final: public Collector_Base
	{
	public:
		using Face = TRI_Generic<Payload>;

		Face*			getT() { return faces.data(); }
		size_t			getTS() { return faces.size(); }
		void			clear() { Collector_Base::clear(); faces.clear(); }
		void			reserve(size_t vertCount, size_t faceCount) { Collector_Base::.reserve(vertCount); faces.reserve(faceCount); }

		void			add_face(const Fvector& v0, const Fvector& v1, const Fvector& v2, Payload&& payload)
		{
			faces.emplace_back(addVert(v0), addVert(v1), addVert(v2), payload);
		}
		void			add_face_packed(const Fvector& v0, const Fvector& v1, const Fvector& v2, Payload&& payload, float eps = EPS)
		{
			faces.emplace_back(addVertPacked(v0, eps), addVertPacked(v1, eps), addVertPacked(v2, eps), payload);
		}

#pragma pack(push,1)
		struct edge {
			u32		face_id : 30;
			u32		edge_id : 2;
			u16		vertex_id0;
			u16		vertex_id1;
		};
#pragma pack(pop)

		struct sort_predicate {
			IC	bool	operator()	(const edge &edge0, const edge &edge1) const
			{
				if (edge0.vertex_id0 < edge1.vertex_id0)
					return				(true);

				if (edge1.vertex_id0 < edge0.vertex_id0)
					return				(false);

				if (edge0.vertex_id1 < edge1.vertex_id1)
					return				(true);

				if (edge1.vertex_id1 < edge0.vertex_id1)
					return				(false);

				return					(edge0.face_id < edge1.face_id);
			}
		};

		void			calc_adjacency(xr_vector<u32>& dest)
		{
#if 1
			VERIFY(faces.size() < 65536);
			const u32						edge_count = faces.size() * 3;
#ifdef _EDITOR
			xr_vector<edge> _edges(edge_count);
			edge 							*edges = &*_edges.begin();
#else
			edge							*edges = (edge*)_alloca(edge_count * sizeof(edge));
#endif
			edge							*i = edges;
			xr_vector<Face>::const_iterator	B = faces.begin(), I = B;
			xr_vector<Face>::const_iterator	E = faces.end();
			for (; I != E; ++I) {
				u32							face_id = u32(I - B);

				(*i).face_id = face_id;
				(*i).edge_id = 0;
				(*i).vertex_id0 = (u16)(*I).verts[0];
				(*i).vertex_id1 = (u16)(*I).verts[1];
				if ((*i).vertex_id0 > (*i).vertex_id1)
					std::swap((*i).vertex_id0, (*i).vertex_id1);
				++i;

				(*i).face_id = face_id;
				(*i).edge_id = 1;
				(*i).vertex_id0 = (u16)(*I).verts[1];
				(*i).vertex_id1 = (u16)(*I).verts[2];
				if ((*i).vertex_id0 > (*i).vertex_id1)
					std::swap((*i).vertex_id0, (*i).vertex_id1);
				++i;

				(*i).face_id = face_id;
				(*i).edge_id = 2;
				(*i).vertex_id0 = (u16)(*I).verts[2];
				(*i).vertex_id1 = (u16)(*I).verts[0];
				if ((*i).vertex_id0 > (*i).vertex_id1)
					std::swap((*i).vertex_id0, (*i).vertex_id1);
				++i;
			}

			std::sort(edges, edges + edge_count, sort_predicate());

			dest.assign(edge_count, u32(-1));

			{
				edge						*I = edges, *J;
				edge						*E = edges + edge_count;
				for (; I != E; ++I) {
					if (I + 1 == E)
						continue;

					J = I + 1;

					if ((*I).vertex_id0 != (*J).vertex_id0)
						continue;

					if ((*I).vertex_id1 != (*J).vertex_id1)
						continue;

					dest[(*I).face_id * 3 + (*I).edge_id] = (*J).face_id;
					dest[(*J).face_id * 3 + (*J).edge_id] = (*I).face_id;
				}
			}
#	if 0
			xr_vector<u32>	test = dest;

			dest.assign(faces.size() * 3, 0xffffffff);
			// Dumb algorithm O(N^2) :)
			for (u32 f = 0; f<faces.size(); f++)
			{
				for (u32 t = 0; t<faces.size(); t++)
				{
					if (t == f)	continue;

					for (u32 f_e = 0; f_e<3; f_e++)
					{
						u32 f1 = faces[f].verts[(f_e + 0) % 3];
						u32 f2 = faces[f].verts[(f_e + 1) % 3];
						if (f1>f2)	std::swap(f1, f2);

						for (u32 t_e = 0; t_e<3; t_e++)
						{
							u32 t1 = faces[t].verts[(t_e + 0) % 3];
							u32 t2 = faces[t].verts[(t_e + 1) % 3];
							if (t1>t2)	std::swap(t1, t2);

							if (f1 == t1 && f2 == t2)
							{
								// f.edge[f_e] linked to t.edge[t_e]
								dest[f * 3 + f_e] = t;
								break;
							}
						}
					}
				}
			}

			{
				xr_vector<u32>::const_iterator	I = test.begin();
				xr_vector<u32>::const_iterator	E = test.end();
				xr_vector<u32>::const_iterator	J = dest.begin();
				for (; I != E; ++I, ++J) {
					VERIFY(*I == *J);
				}
			}
#	endif
#else
			dest.assign(faces.size() * 3, 0xffffffff);
			// Dumb algorithm O(N^2) :)
			for (u32 f = 0; f<faces.size(); f++)
			{
				for (u32 t = 0; t<faces.size(); t++)
				{
					if (t == f)	continue;

					for (u32 f_e = 0; f_e<3; f_e++)
					{
						u32 f1 = faces[f].verts[(f_e + 0) % 3];
						u32 f2 = faces[f].verts[(f_e + 1) % 3];
						if (f1>f2)	std::swap(f1, f2);

						for (u32 t_e = 0; t_e<3; t_e++)
						{
							u32 t1 = faces[t].verts[(t_e + 0) % 3];
							u32 t2 = faces[t].verts[(t_e + 1) % 3];
							if (t1>t2)	std::swap(t1, t2);

							if (f1 == t1 && f2 == t2)
							{
								// f.edge[f_e] linked to t.edge[t_e]
								dest[f * 3 + f_e] = t;
								break;
							}
						}
					}
				}
			}
#endif
		}
	private:
		xr_vector<Face>		faces;
	};
	using Collector = Collector_Generic<GamePayload>;

	struct non_copyable {
		non_copyable() = default;
	private:
		non_copyable(const non_copyable &) = delete;
		non_copyable& operator=(const non_copyable &) = delete;
	};

	class XRCDB_API VertexCollectorPacked
	{
	public:
		VertexCollectorPacked(const Fbox &bb, int apx_vertices = 5000);

		xr_vector<Fvector>& getV_Vec() { return verts; }
		Fvector*			getV() { return &*verts.begin(); }
		size_t				getVS() { return verts.size(); }

	protected:

		xr_vector<Fvector>	verts;

		void clear();
		u32 VPack(const Fvector& V);

	private:
		enum { clpMX = 24, clpMY = 16, clpMZ = 24 };
		typedef xr_vector<u32>		DWORDList;
		typedef DWORDList::iterator	DWORDIt;

		Fvector				VMmin, VMscale;
		DWORDList			VM[clpMX + 1][clpMY + 1][clpMZ + 1];
		Fvector				VMeps;
	};

	template <class Payload>
	class CollectorPacked final: public VertexCollectorPacked, non_copyable {
	public:
		using Face = TRI_Generic<Payload>;

		CollectorPacked(const Fbox &bb, int apx_vertices = 5000, int apx_faces = 5000) : VertexCollectorPacked(bb, apx_vertices)
		{
			// Preallocate memory
			faces.reserve(apx_faces);
			flags.reserve(apx_faces);
		}

		void add_face(const Fvector& v0, const Fvector& v1, const Fvector& v2, Payload&& payload, u32 flag)
		{
			flags.push_back(flag);
			faces.emplace_back(VPack(v0), VPack(v1), VPack(v2), payload);
		}

		Face*				getT() { return &*faces.begin(); }
		u32					getfFlags(u32 index) { return flags[index]; }
		Face&				getT(u32 index) { return faces[index]; }
		size_t				getTS() { return faces.size(); }
		void				clear()
		{
			VertexCollectorPacked::clear();
			faces.clear_and_free();
			flags.clear_and_free();
		}

	private:
		xr_vector<Face>		faces;
		xr_vector<u32>		flags;
	};
	using CollectorPacked_Game = CollectorPacked<GamePayload>;
	using CollectorPacked_Work = CollectorPacked<WorkPayload>;
};

#pragma pack(pop)
#endif
