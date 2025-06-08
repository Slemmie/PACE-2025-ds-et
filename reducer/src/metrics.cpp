#include "metrics.h"

#include <sstream>
#include <iostream>

void Metrics::log(bool include_stderr) const {
	std::ostringstream oss;

	oss << "c rule 1 steps: " << rule1_steps << ", deleted " << rule1_deletions << " total vertices\n";
	oss << "c rule 2 steps: " << rule2_steps << ", deleted " << rule2_deletions << " total vertices (added "
		<< rule2_additions_vertices << " vertices and " << rule2_additions_edges << " edges)\n";
	oss << "c island vertices removed: " << island_vertices_removed << "\n";
	oss << "c leaves peeled: " << leaves_peeled << "\n";
	oss << "c edges between triangles with one vertex having degree 2 removed: " << tri_tri_edges_removed << "\n";
	oss << "c edges between two white vertices removed: " << white_white_edges_removed << "\n";
	oss << "c vertices from both W and X removed directly: " << W_X_vertices_removed << "\n";
	oss << "c articulation point reductions: ";
	for (size_t i = 0; i < articulation_point_reductions.size(); i++) {
		oss << articulation_point_reductions[i];
		if (i + 1 == articulation_point_reductions.size()) oss << "\n";
		else oss << ", ";
	}
	if (articulation_point_reductions.empty()) {
		oss << "none\n";
	}
	oss << "c size-2 vertex-cut attempts where gadget was missing from table: " << cut2_missing_gadget_encounters << "\n";
	oss << "c size-2 vertex-cut reductions: ";
	for (size_t i = 0; i < cut2_hits.size(); i++) {
		oss << cut2_hits[i];
		if (i + 1 == cut2_hits.size()) oss << "\n";
		else oss << ", ";
	}
	if (cut2_hits.empty()) {
		oss << "none\n";
	}

	std::cout << oss.str();
	if (include_stderr) {
		std::cerr << oss.str();
	}
}

void Metrics::add(const Metrics& metrics) {
	rule1_steps += metrics.rule1_steps;
	rule1_deletions += metrics.rule1_deletions;
	rule2_steps += metrics.rule2_steps;
	rule2_deletions += metrics.rule2_deletions;
	rule2_additions_vertices += metrics.rule2_additions_vertices;
	rule2_additions_edges += metrics.rule2_additions_edges;
	island_vertices_removed += metrics.island_vertices_removed;
	leaves_peeled += metrics.leaves_peeled;
	tri_tri_edges_removed += metrics.tri_tri_edges_removed;
	W_X_vertices_removed += metrics.W_X_vertices_removed;
	articulation_point_reductions.insert(articulation_point_reductions.end(), metrics.articulation_point_reductions.begin(), metrics.articulation_point_reductions.end());
	cut2_missing_gadget_encounters += metrics.cut2_missing_gadget_encounters;
	cut2_hits.insert(cut2_hits.end(), metrics.cut2_hits.begin(), metrics.cut2_hits.end());
}
