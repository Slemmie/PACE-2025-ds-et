#pragma once

#include "instance.h"
#include "metrics.h"

class Reducer {

public:

	Reducer(
		std::function <Metrics (Instance&)> finalize_callback,
		szt articulation_point_component_size_cutoff = 100,
		szt cut2_component_size_cutoff = 100,
		szt cut_rules_recs = 1
	);

	void reduce(Instance& instance);

	const Metrics& metrics() const { return m_metrics; }

private:

	Metrics m_metrics;

	std::function <Metrics (Instance&)> m_finalize_callback;

	szt m_finalize_cutoff;
	szt m_articulation_point_component_size_cutoff;
	szt m_cut2_component_size_cutoff;

	szt m_cut_rules_recs;

	std::vector <std::pair <szt, szt>> m_d3;
	hash_set <szt> m_rule2_nh_banned;

private:

	bool m_rule_A(Instance& instance);
	bool m_rule_B(Instance& instance);
	bool m_rule_C(Instance& instance);

	bool m_rule1_step(Instance& instance);
	bool m_rule2_step(Instance& instance);
	bool m_articulation_point_rule_step(Instance& instance);
	bool m_cut2_rule_step(Instance& instance);
	void m_branch_by_disconnected_components(Instance& instance); // does not reduce the largest one
	void m_remove_island_vertices(Instance& instance);
	bool m_peel_leaves(Instance& instance);
	bool m_tri_tri_edge_removal(Instance& instance);
	bool m_white_white_edge_removal(Instance& instance);
	bool m_W_X_vertex_removal(Instance& instance);
	bool m_W_nh_vertex_removal(Instance& instance);
	bool m_all_W(Instance& instance);

};
