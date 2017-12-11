////////////////////////////////////////////////////////////////////////////
//	Module 		: game_graph_builder.cpp
//	Created 	: 14.12.2005
//  Modified 	: 14.12.2005
//	Author		: Dmitriy Iassenev
//	Description : Game graph builder
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "game_graph_builder.h"
#include "level_graph.h"
#include "graph_abstract.h"
#include "xrMessages.h"
#include "xrServer_Objects_ALife.h"
#include "factory_api.h"
#include "game_level_cross_table.h"
#include "xrCrossTable.h"
#include "guid_generator.h"
#include "graph_engine.h"

CGameGraphBuilder::CGameGraphBuilder		()
{
	m_level_graph			= 0;
	m_graph					= 0;
	m_cross_table			= 0;
}

CGameGraphBuilder::~CGameGraphBuilder		()
{
	Msg						("Freeing resources");

	xr_delete				(m_level_graph);
	xr_delete				(m_graph);
	xr_delete				(m_cross_table);
}

void CGameGraphBuilder::create_graph		(const float &start, const float &amount)
{
	Progress				(start);

	VERIFY					(!m_graph);
	m_graph					= xr_new<graph_type>();

	m_graph_guid			= generate_guid();

	Progress				(start + amount);
}

void CGameGraphBuilder::load_level_graph	(const float &start, const float &amount)
{
	Progress				(start);

	Msg						("Loading AI map");
	
	VERIFY					(!m_level_graph);
	m_level_graph			= xr_new<CLevelGraph>(*m_level_name);
	
	Msg						("%d nodes loaded",level_graph().header().vertex_count());
	
	Progress				(start + amount);
}

void CGameGraphBuilder::load_graph_point	(NET_Packet &net_packet)
{
	string256				section_id;
	u16						id;
	net_packet.r_begin		(id);
	R_ASSERT				(M_SPAWN == id);
	net_packet.r_stringZ	(section_id);

//	if (xr_strcmp("graph_point",section_id))
//		return;

	CSE_Abstract			*entity = F_entity_Create(section_id);
	if (!entity) {
		Msg					("Cannot create entity from section %s, skipping",section_id);
		return;
	}

	CSE_ALifeGraphPoint		*graph_point = smart_cast<CSE_ALifeGraphPoint*>(entity);
	if (!graph_point) {
		F_entity_Destroy	(entity);
		return;
	}

	entity->Spawn_Read		(net_packet);

	vertex_type				vertex;
	vertex.tLocalPoint		= graph_point->o_Position;
	// check for duplicate graph point positions
	{
		graph_type::const_vertex_iterator	I = graph().vertices().begin();
		graph_type::const_vertex_iterator	E = graph().vertices().end();
		for ( ; I != E; ++I) {
			if ((*I).second->data().tLocalPoint.distance_to_sqr(vertex.tLocalPoint) < EPS_L) {
				Msg			("! removing graph point [%s][%f][%f][%f] because it is too close to the another graph point",entity->name_replace(),VPUSH(entity->o_Position));
				F_entity_Destroy(entity);
				return;
			}
		}
	}

	vertex.tGlobalPoint		= graph_point->o_Position;
	vertex.tNodeID			= level_graph().valid_vertex_position(vertex.tLocalPoint) ? level_graph().vertex_id(vertex.tLocalPoint) : u32(-1);
	if (!level_graph().valid_vertex_id(vertex.tNodeID)) {
		Msg					("! removing graph point [%s][%f][%f][%f] because it is outside of the AI map",entity->name_replace(),VPUSH(entity->o_Position));
		F_entity_Destroy	(entity);
		return;
	}

	{
		graph_type::const_vertex_iterator	I = graph().vertices().begin();
		graph_type::const_vertex_iterator	E = graph().vertices().end();
		for ( ; I != E; ++I) {
			if ((*I).second->data().tNodeID == vertex.tNodeID) {
				Msg			("! removing graph point [%s][%f][%f][%f] because it has the same AI node as another graph point",entity->name_replace(),VPUSH(entity->o_Position));
				F_entity_Destroy	(entity);
				return;
			}
		}
	}

	vertex.tNeighbourCount	= 0;
	Memory.mem_copy			(vertex.tVertexTypes,graph_point->m_tLocations,GameGraph::LOCATION_TYPE_COUNT*sizeof(GameGraph::_LOCATION_ID));
	vertex.tLevelID			= 0;
	vertex.tDeathPointCount = 0;
	vertex.dwPointOffset	= 0;

	graph().add_vertex		(vertex,graph().vertices().size());

	F_entity_Destroy		(entity);
}

void CGameGraphBuilder::load_graph_points	(const float &start, const float &amount)
{
	Progress				(start);

	Msg						("Loading graph points");

	string_path				spawn_file_name;
	strconcat				(sizeof(spawn_file_name),spawn_file_name,*m_level_name,"level.spawn");
	IReader					*reader = FS.r_open(spawn_file_name);
	u32						id;
	NET_Packet				net_packet;
	for	(
			IReader *chunk = reader->open_chunk_iterator(id);
			chunk;
			chunk = reader->open_chunk_iterator(id,chunk)
		)
	{
		net_packet.B.count	= chunk->length();
		chunk->r			(net_packet.B.data,net_packet.B.count);
		load_graph_point	(net_packet);
	}
	
	FS.r_close				(reader);

	Msg						("%d graph points loaded",graph().vertices().size());

	Progress				(start + amount);
}

template <typename T>
IC	bool sort_predicate_less(const T &first, const T &second)
{
	return					(first.first < second.first);
}

template <typename T>
IC	bool sort_predicate_greater(const T &first, const T &second)
{
	return					(first.first > second.first);
}

// Бросаем в воды аи-сетки камушки в каждом граф-поинте.
// И смотрим до какой аи-ноды от какого граф-поинта быстрей дойдет волна.
// Значит эта аи-нода относится к этому графпоинту, а время, за которое дошла волна, считаем за расстояние до граф-поинта.
// Заодно засекаем, какая волна с какой встретилась и собираем информацию о соседних граф-поинтах.
void CGameGraphBuilder::iterate_distances	(const float &start, const float &amount)
{
	Progress							(start);

	u32 count = level_graph().header().vertex_count();
	m_parents.assign(count, GameGraph::_GRAPH_ID(-1));
	m_distances.resize(count);
	m_neighbours.resize(graph().vertices().size());

	xr_vector<u32> cur_fringe;// фронт волны

	// бросаем камни в граф-поинтах и заполняем начальный фронт волны
	for (u32 i = 0, n = graph().vertices().size(); i != n; i++) {
		u32 level_vertex_id = graph().vertex(i)->data().level_vertex_id();
		cur_fringe.push_back(level_vertex_id);
		m_parents[level_vertex_id] = i;
		m_distances[level_vertex_id] = 0;
	}

	float step = amount / count;
	u32 processed = 0;
	xr_vector<u32> next_fringe;
	while (!cur_fringe.empty()) {
		//обходим фронт волны и смотрим, в какие аи-ноды волна может пойти дальше
		for (auto lvid : cur_fringe) {
			u32 dist = m_distances[lvid];
			GameGraph::_GRAPH_ID parent = m_parents[lvid];
			CLevelGraph::CVertex *node = level_graph().vertex(lvid);
			CLevelGraph::const_iterator i, e;
			level_graph().begin(lvid, i, e);
			for (; i != e; ++i) {
				u32 next_lvid = node->link(i);

				// тупик
				if (!level_graph().valid_vertex_id(next_lvid))
					continue;

				// встретились две волны - добавляем соседа к граф-поинту
				if (m_parents[next_lvid] != GameGraph::_GRAPH_ID(-1) && m_parents[next_lvid] != parent)
					m_neighbours[parent].insert(m_parents[next_lvid]);

				// оставляем только те аи-ноды, где волна еще не была
				if (m_parents[next_lvid] != GameGraph::_GRAPH_ID(-1))
					continue;

				m_distances[next_lvid] = dist + 1;
				m_parents[next_lvid] = parent;
				next_fringe.push_back(next_lvid);
			}
		}

		processed += cur_fringe.size();
		Progress(start + step * processed);

		cur_fringe.swap(next_fringe);
		next_fringe.clear_not_free();

	}

	Progress							(start + amount);
}

void CGameGraphBuilder::check_fill()
{
	u32 count = 0;
	for (u32 i = 0; i != m_parents.size(); i++) {
		if (m_parents[i] != GameGraph::_GRAPH_ID(-1))
			continue;

		Msg("! AI-node [%.3f, %.3f, %.3f] not connected to AI-map", VPUSH(level_graph().vertex_position(i)));
		count++;
	}
	R_ASSERT2(count == 0, "Some ai-node is not connected to AI-map. See log for details.");
}

void CGameGraphBuilder::save_cross_table	(const float &start, const float &amount)
{
	Progress							(start);

	Msg									("Saving cross table");

//	CTimer								timer;
//	timer.Start							();

	CMemoryWriter						tMemoryStream;
	CGameLevelCrossTable::CHeader		tCrossTableHeader;
	
	tCrossTableHeader.dwVersion			= XRAI_CURRENT_VERSION;
	tCrossTableHeader.dwNodeCount		= level_graph().header().vertex_count();
	tCrossTableHeader.dwGraphPointCount = graph().header().vertex_count();
	tCrossTableHeader.m_level_guid		= level_graph().header().guid();
	tCrossTableHeader.m_game_guid		= m_graph_guid;
	
	tMemoryStream.open_chunk			(CROSS_TABLE_CHUNK_VERSION);
	tMemoryStream.w						(&tCrossTableHeader,sizeof(tCrossTableHeader));
	tMemoryStream.close_chunk			();
	
	tMemoryStream.open_chunk			(CROSS_TABLE_CHUNK_DATA);

	for (int i=0, n=level_graph().header().vertex_count(); i<n; i++) {
		CGameLevelCrossTable::CCell	tCrossTableCell;
		tCrossTableCell.tGraphIndex = m_parents[i];
		VERIFY						(graph().header().vertex_count() > tCrossTableCell.tGraphIndex);
		tCrossTableCell.fDistance	= m_distances[i] * level_graph().header().cell_size();
		tMemoryStream.w				(&tCrossTableCell,sizeof(tCrossTableCell));
	}

	tMemoryStream.close_chunk();
	
//	Msg						("CT:SAVE : %f",timer.GetElapsed_sec());
//	Msg						("Flushing cross table");

	tMemoryStream.save_to	(m_cross_table_name);
//	Msg						("CT:SAVE : %f",timer.GetElapsed_sec());

//	Msg						("Freiing cross table resources");

	m_distances.clear		();
	m_parents.clear			();

//	Msg						("CT:SAVE : %f",timer.GetElapsed_sec());
	Progress				(start + amount);
}

void CGameGraphBuilder::build_cross_table	(const float &start, const float &amount)
{
	Progress				(start);
	
	Msg						("Building cross table");
	iterate_distances		(start + 0.000000f*amount, 0.959659f*amount);
	check_fill();
	save_cross_table		(start + 0.959659f*amount,0.040327f*amount);
	load_cross_table		(start + 0.999986f*amount,0.000014f*amount);
	
	Progress				(start + amount);
}

void CGameGraphBuilder::load_cross_table	(const float &start, const float &amount)
{
	Progress				(start);

	Msg						("Loading cross table");

	VERIFY					(!m_cross_table);
	m_cross_table			= xr_new<CGameLevelCrossTable>(m_cross_table_name);

	Progress				(start + amount);
}

float CGameGraphBuilder::path_distance		(const u32 &game_vertex_id0, const u32 &game_vertex_id1)
{
//	return					(graph().vertex(game_vertex_id0)->data().level_point().distance_to(graph().vertex(game_vertex_id1)->data().level_point()));

	VERIFY					(m_graph_engine);

	graph_type::CVertex		&vertex0 = *graph().vertex(game_vertex_id0);
	graph_type::CVertex		&vertex1 = *graph().vertex(game_vertex_id1);

	typedef GraphEngineSpace::CStraightLineParams	CStraightLineParams;
	CStraightLineParams		parameters(vertex0.data().level_point(),vertex1.data().level_point());

	float					pure_distance = vertex0.data().level_point().distance_to_xz(vertex1.data().level_point());
//	float					pure_distance = vertex0.data().level_point().distance_to(vertex1.data().level_point());
	VERIFY					(pure_distance < parameters.max_range);

	u32						level_vertex_id = level_graph().check_position_in_direction(vertex0.data().level_vertex_id(),vertex0.data().level_point(),vertex1.data().level_point());
	if (level_graph().valid_vertex_id(level_vertex_id))
		return				(pure_distance);

	bool					successfull = 
		m_graph_engine->search(
			level_graph(),
			vertex0.data().level_vertex_id(),
			vertex1.data().level_vertex_id(),
			&m_path,
			parameters
		);

	if (successfull)
		return				(parameters.m_distance);

	Msg						("Cannot build path from [%d] to [%d]",game_vertex_id0,game_vertex_id1);
	Msg						("Cannot build path from [%f][%f][%f] to [%f][%f][%f]",VPUSH(vertex0.data().level_point()),VPUSH(vertex1.data().level_point()));
	R_ASSERT2				(false,"Cannot build path, check AI map");
	return					(flt_max);
}

void CGameGraphBuilder::generate_edges		(const float &start, const float &amount)
{
	Progress				(start);

	Msg						("Generating edges");

	for (u32 i = 0; i != m_neighbours.size(); i++) {
		graph_type::CVertex* vertex = graph().vertex(i);
		for (auto neighbour : m_neighbours[i]) {
			VERIFY(!vertex->edge(neighbour));
			float distance = path_distance(i, neighbour);
			graph().add_edge(i, neighbour, distance);
		}
	}

	Msg						("%d edges built",graph().edge_count());

	Progress				(start + amount);
}

void CGameGraphBuilder::connectivity_check	(const float &start, const float &amount)
{
	Progress				(start);

	Msg						("Checking graph connectivity");


	Progress				(start + amount);
}

void CGameGraphBuilder::create_tripples		(const float &start, const float &amount)
{
	graph_type::const_vertex_iterator	I = graph().vertices().begin();
	graph_type::const_vertex_iterator	E = graph().vertices().end();
	for ( ; I != E; ++I) {
		graph_type::const_iterator	i = (*I).second->edges().begin();
		graph_type::const_iterator	e = (*I).second->edges().end();
		for ( ; i != e; ++i) {
			if (((*i).vertex_id() < (*I).first) && graph().edge((*i).vertex_id(),(*I).first))
				continue;

			const graph_type::CEdge	*edge = graph().vertex((*i).vertex_id())->edge((*I).first);

			m_tripples.push_back	(
				std::make_pair(
					_min(
						(*i).weight(),
						edge ? edge->weight() : (*i).weight()
					),
					std::make_pair(
						(*I).first,
						(*i).vertex_id()
					)
				)
			);
		}
	}

	std::sort				(m_tripples.begin(),m_tripples.end(),sort_predicate_greater<TRIPPLE>);
}

void CGameGraphBuilder::process_tripple		(const TRIPPLE &tripple)
{
	const graph_type::CVertex	&vertex0 = *graph().vertex(tripple.second.first);
	const graph_type::CVertex	&vertex1 = *graph().vertex(tripple.second.second);

	graph_type::const_iterator	I = vertex0.edges().begin();
	graph_type::const_iterator	E = vertex0.edges().end();
	for ( ; I != E; ++I) {
		if ((*I).vertex_id() == tripple.second.second)
			continue;

		const graph_type::CEdge	*edge;

		edge					= vertex1.edge((*I).vertex_id());
		if (edge) {
			VERIFY				(_min((*I).weight(),graph().edge((*I).vertex_id(),tripple.second.first) ? graph().edge((*I).vertex_id(),tripple.second.first)->weight() : (*I).weight()) <= tripple.first);
			VERIFY				(_min(edge->weight(),graph().edge(edge->vertex_id(),tripple.second.second) ? graph().edge(edge->vertex_id(),tripple.second.second)->weight() : (*I).weight()) <= tripple.first);
			if (vertex0.edge(tripple.second.second))
				graph().remove_edge	(tripple.second.first,tripple.second.second);
			if (vertex1.edge(tripple.second.first))
				graph().remove_edge	(tripple.second.second,tripple.second.first);
			return;
		}

		edge					= graph().vertex((*I).vertex_id())->edge(tripple.second.second);
		if (edge) {
			VERIFY				(_min((*I).weight(),graph().edge((*I).vertex_id(),tripple.second.first) ? graph().edge((*I).vertex_id(),tripple.second.first)->weight() : (*I).weight()) <= tripple.first);
			VERIFY				(edge->weight() <= tripple.first);
			if (vertex0.edge(tripple.second.second))
				graph().remove_edge	(tripple.second.first,tripple.second.second);
			if (vertex1.edge(tripple.second.first))
				graph().remove_edge	(tripple.second.second,tripple.second.first);
			return;
		}
	}
	return;
}

void CGameGraphBuilder::optimize_graph		(const float &start, const float &amount)
{
	Progress					(start);

	Msg							("Optimizing graph");

	Msg							("edges before optimization : %d",graph().edge_count());

	create_tripples				(start + .00f, amount*.50f);

	TRIPPLES::const_iterator	I = m_tripples.begin();
	TRIPPLES::const_iterator	E = m_tripples.end();
	for ( ; I != E; ++I)
		process_tripple			(*I);

	Msg							("edges after optimization : %d",graph().edge_count());

	Progress					(start + amount);
}

void CGameGraphBuilder::save_graph			(const float &start, const float &amount)
{
	Progress				(start);

	Msg						("Saving graph");

	// header
	CMemoryWriter				writer;
	CGameGraph::CHeader			header;
	header.m_version			= XRAI_CURRENT_VERSION;
	VERIFY						(graph().vertices().size() < (u32(1) << (8*sizeof(GameGraph::_GRAPH_ID))));
	header.m_vertex_count		= (GameGraph::_GRAPH_ID)graph().vertices().size();
	VERIFY						(graph().edge_count() < (u32(1) << (8*sizeof(GameGraph::_GRAPH_ID))));
	header.m_edge_count			= (GameGraph::_GRAPH_ID)graph().edge_count();
	header.m_death_point_count	= 0;
	header.m_guid				= m_graph_guid;

	// levels
	CGameGraph::SLevel			level;
	level.m_offset.set			(0,0,0);
	level.m_id					= 0;
	level.m_name				= m_level_name;
	level.m_section				= "";
	level.m_guid				= level_graph().header().guid();

	header.m_levels.insert		(std::make_pair(level.m_id,level));

	header.save					(&writer);

	{
		u32								edge_offset = graph().vertices().size()*sizeof(CGameGraph::CVertex);

		graph_type::const_vertex_iterator	I = graph().vertices().begin();
		graph_type::const_vertex_iterator	E = graph().vertices().end();
		for ( ; I != E; ++I) {
			CGameGraph::CVertex		&vertex = (*I).second->data();

			VERIFY					((*I).second->edges().size() < 256);
			vertex.tNeighbourCount	= (u8)(*I).second->edges().size();
			vertex.dwEdgeOffset		= edge_offset;
			edge_offset				+= vertex.tNeighbourCount*sizeof(CGameGraph::CEdge);

			writer.w				(&vertex,sizeof(CGameGraph::CVertex));
		}
	}
	
	{
		graph_type::const_vertex_iterator	I = graph().vertices().begin();
		graph_type::const_vertex_iterator	E = graph().vertices().end();
		for ( ; I != E; ++I) {
			graph_type::const_iterator	i = (*I).second->edges().begin();
			graph_type::const_iterator	e = (*I).second->edges().end();
			for ( ; i != e; ++i) {
				GameGraph::CEdge			edge;
				VERIFY						((*i).vertex_id() < (u32(1) << (8*sizeof(GameGraph::_GRAPH_ID))));
				edge.m_vertex_id			= (GameGraph::_GRAPH_ID)(*i).vertex_id();
				edge.m_path_distance		= (*i).weight();

				writer.w					(&edge.m_vertex_id,sizeof(edge.m_vertex_id));
				writer.w_float				(edge.m_path_distance);
			}
		}
	}

	writer.save_to				(m_graph_name);
	Msg							("%d bytes saved",int(writer.size()));

	Progress					(start + amount);
}

void CGameGraphBuilder::build_graph			(const float &start, const float &amount)
{
	Progress				(start);

	Msg						("Building graph");

	CTimer					timer;
	timer.Start				();

	m_graph_engine			= xr_new<CGraphEngine>(level_graph().header().vertex_count());
	Progress				(start + 0.000000f*amount + amount*0.067204f);
//	Msg						("BG : %f",timer.GetElapsed_sec());

	generate_edges			(start + 0.067204f*amount, amount*0.922647f);
//	Msg						("BG : %f",timer.GetElapsed_sec());

	xr_delete				(m_graph_engine);
	Progress				(start + 0.989851f*amount + amount*0.002150f);
//	Msg						("BG : %f",timer.GetElapsed_sec());

	connectivity_check		(start + 0.992001f*amount, amount*0.000030f);
//	Msg						("BG : %f",timer.GetElapsed_sec());
	optimize_graph			(start + 0.992031f*amount, amount*0.000454f);
//	Msg						("BG : %f",timer.GetElapsed_sec());
	save_graph				(start + 0.992485f*amount, amount*0.007515f);
//	Msg						("BG : %f",timer.GetElapsed_sec());

	Progress				(start + amount);
}

void CGameGraphBuilder::build_graph			(
		LPCSTR graph_name,
		LPCSTR cross_table_name,
		LPCSTR level_name
	)
{
	Phase					("Building level game graph");
	Msg						("level \"%s\"",level_name);

	m_graph_name			= graph_name;
	m_cross_table_name		= cross_table_name;
	m_level_name			= level_name;
	
//	CTimer					timer;
//	timer.Start				();

	create_graph			(0.000000f,0.000047f);
//	Msg						("%f",timer.GetElapsed_sec());
	load_level_graph		(0.000047f,0.002470f);
//	Msg						("%f",timer.GetElapsed_sec());
	load_graph_points		(0.002517f,0.111812f);
//	Msg						("%f",timer.GetElapsed_sec());
	build_cross_table		(0.114329f,0.773423f);
//	Msg						("%f",timer.GetElapsed_sec());
	build_graph				(0.887752f,0.112248f);
//	Msg						("%f",timer.GetElapsed_sec());

	Msg						("Level graph is generated successfully");
}
