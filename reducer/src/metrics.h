#pragma once

#include "common.h"

#include <vector>

struct Metrics {
	szt rule_A = 0;
	szt rule_B = 0;
	szt rule_C = 0;

	szt rule1_steps = 0;
	szt rule1_deletions = 0;

	szt rule2_steps = 0;
	szt rule2_deletions = 0;
	szt rule2_additions_vertices = 0;
	szt rule2_additions_edges = 0;

	szt island_vertices_removed = 0;

	szt leaves_peeled = 0;

	szt tri_tri_edges_removed = 0;

	szt white_white_edges_removed = 0;

	szt W_X_vertices_removed = 0;

	szt W_Wnh_vertices_removed = 0;

	szt all_W_removals = 0;

	std::vector <szt> articulation_point_reductions;

	szt cut2_missing_gadget_encounters = 0;

	std::vector <szt> cut2_hits;

	void log(bool include_stderr = false) const;

	void add(const Metrics& metrics);
};
