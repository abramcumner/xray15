////////////////////////////////////////////////////////////////////////////
//	Module 		: game_graph_builder.h
//	Created 	: 14.12.2005
//  Modified 	: 14.12.2005
//	Author		: Dmitriy Iassenev
//	Description : Game graph builder
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../../xrEngine/xrLevel.h"
#include "alife_space.h"

class CLevelGraph;
class CGameLevelCrossTable;

template <
	typename _data_type,
	typename _edge_weight_type,
	typename _vertex_id_type
>
class CGraphAbstract;

namespace GameGraph {
	struct CVertex;
	typedef u16	_GRAPH_ID;
};

class NET_Packet;

class CGraphEngine;

class CGameGraphBuilder {
private:
	typedef GameGraph::CVertex						vertex_type;
	typedef CGraphAbstract<vertex_type,float,u32>	graph_type;
	typedef xr_vector<xr_vector<u32> >				DISTANCES;
	typedef std::pair<u32,u32>						PAIR;
	typedef std::pair<float,PAIR>					TRIPPLE;
	typedef xr_vector<TRIPPLE>						TRIPPLES;

private:
	LPCSTR					m_graph_name;
	LPCSTR					m_cross_table_name;
	LPCSTR					m_short_level_name;

private:
	shared_str				m_level_name;

private:
	CLevelGraph				*m_level_graph;
	graph_type				*m_graph;
	xrGUID					m_graph_guid;
	// cross table generation stuff
	xr_vector<GameGraph::_GRAPH_ID> m_parents;//к каким граф-поинтам относится аи-нода
	xr_vector<u32> m_distances;// расстояние до ближайшего граф-понта
	xr_vector<xr_set<GameGraph::_GRAPH_ID>> m_neighbours;// соседи граф-поинта

	// cross table itself
	CGameLevelCrossTable	*m_cross_table;
	TRIPPLES				m_tripples;
	xr_vector<u32>			m_path;
	CGraphEngine			*m_graph_engine;

private:
			void		create_graph				(const float &start, const float &amount);
			void		load_level_graph			(const float &start, const float &amount);
			void		load_graph_point			(NET_Packet &net_packet);
			void		load_graph_points			(const float &start, const float &amount);

private:
			void		iterate_distances			(const float &start, const float &amount);
			void		check_fill					();
			void		save_cross_table			(const float &start, const float &amount);
			void		build_cross_table			(const float &start, const float &amount);
			void		load_cross_table			(const float &start, const float &amount);
			
private:
			float		path_distance				(const u32 &game_vertex_id0, const u32 &game_vertex_id1);
			void		generate_edges				(const float &start, const float &amount);
			void		connectivity_check			(const float &start, const float &amount);
			void		create_tripples				(const float &start, const float &amount);
			void		process_tripple				(const TRIPPLE &tripple);
			void		optimize_graph				(const float &start, const float &amount);
			void		save_graph					(const float &start, const float &amount);
			void		build_graph					(const float &start, const float &amount);

private:
	IC		CLevelGraph				&level_graph	() const;
	IC		graph_type				&graph			() const;
	IC		CGameLevelCrossTable	&cross			() const;

public:
						CGameGraphBuilder			();
						~CGameGraphBuilder			();
			void		build_graph					(LPCSTR graph_name, LPCSTR cross_table_name, LPCSTR level_name, LPCSTR short_level_name);
};

#include "game_graph_builder_inline.h"