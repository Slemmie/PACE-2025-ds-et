#pragma once

#include <vector>

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

	size_t W_X_vertices_removed = 0;

	std::vector <size_t> articulation_point_reductions;

	size_t cut2_missing_gadget_encounters = 0;

	std::vector <size_t> cut2_hits;

	void log(bool include_stderr = false) const;

	void add(const Metrics& metrics);
};
