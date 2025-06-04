#pragma once

#include "instance.h"

struct Metrics {
	size_t rule1_steps = 0;
	size_t rule1_deletions = 0;

	size_t rule2_steps = 0;
	size_t rule2_deletions = 0;
	size_t rule2_additions_vertices = 0;
	size_t rule2_additions_edges = 0;

	size_t island_vertices_removed = 0;

	size_t leaves_peeled = 0;

	size_t tri_tri_edges_removed = 0;

	size_t white_white_edges_removed = 0;

	std::vector <size_t> articulation_point_reductions;

	size_t cut2_missing_gadget_encounters = 0;

	std::vector <size_t> cut2_hits;

	void log(bool include_stderr = false) const;

	void add(const Metrics& metrics);
};

class Solver {

public:

	Solver(size_t finalize_cutoff = 100, size_t articulation_point_component_size_cutoff = 100, size_t cut2_component_size_cutoff = 100, size_t cut_rules_recs = 1);

	void solve(Instance& instance);

	const Metrics& metrics() const { return m_metrics; }

private:

	Metrics m_metrics;

	size_t m_finalize_cutoff;
	size_t m_articulation_point_component_size_cutoff;
	size_t m_cut2_component_size_cutoff;

	size_t m_cut_rules_recs;

	std::vector <std::pair <size_t, size_t>> m_d3;
	std::unordered_set <size_t> m_rule2_nh_banned;

private:

	bool m_rule1_step(Instance& instance);
	bool m_rule2_step(Instance& instance);
	bool m_articulation_point_rule_step(Instance& instance);
	bool m_cut2_rule_step(Instance& instance);
	void m_branch_by_disconnected_components(Instance& instance); // does not reduce the largest one
	void m_remove_island_vertices(Instance& instance);
	bool m_peel_leaves(Instance& instance);
	bool m_tri_tri_edge_removal(Instance& instance);
	bool m_white_white_edge_removal(Instance& instance);

};
