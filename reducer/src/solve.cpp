#include "solve.h"

#include "finalize.h"
#include "cut2_gadgets.h"

#include <sstream>
#include <queue>
#include <cstdint>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <cassert>

Solver::Solver(size_t finalize_cutoff, size_t articulation_point_component_size_cutoff, size_t cut2_component_size_cutoff, size_t cut_rules_recs) :
m_finalize_cutoff(finalize_cutoff),
m_articulation_point_component_size_cutoff(articulation_point_component_size_cutoff),
m_cut2_component_size_cutoff(cut2_component_size_cutoff),
m_cut_rules_recs(cut_rules_recs)
{
	static bool first_pass = true;
	if (first_pass) {
		first_pass = false;
		cut2::init_gadgets();
	}
}

void Solver::solve(Instance& instance) {
	while (true) {
		if (instance.alives().empty()) break;

		auto perform_trivials = [&] () -> bool {
			bool did_any = false;
			bool stop_trivials = false;
			while (!stop_trivials) {
				stop_trivials = true;
				m_remove_island_vertices(instance);
				if (m_white_white_edge_removal(instance)) stop_trivials = false;
				if (m_tri_tri_edge_removal(instance)) stop_trivials = false;
				if (m_peel_leaves(instance)) stop_trivials = false;
				did_any |= !stop_trivials;
			}
			return did_any;
		};

		perform_trivials();

		m_branch_by_disconnected_components(instance); // the largest one is reduced below, the rest have been branched by and will not be reduced further

		m_d3.clear();
		for (size_t v : instance.alives()) {
			std::queue <std::pair <size_t, uint8_t>> q;
			q.emplace(v, 0);
			std::unordered_set <size_t> vis;
			while (!q.empty()) {
				auto [u, d] = q.front();
				q.pop();
				if (vis.contains(u)) continue;
				if (d > 3) continue;
				m_d3.emplace_back(v, u);
				vis.insert(u);
				for (size_t x : instance.g()[u]) {
					if (x < v) {
						q.emplace(x, d + 1);
					}
				}
			}
		}

		m_rule2_nh_banned.clear();

		bool stop = false;
		while (!stop) {
			stop = true;
			while (m_rule1_step(instance)) m_metrics.rule1_steps++, stop = false;
			while (m_rule2_step(instance)) m_metrics.rule2_steps++, stop = false;
			if (perform_trivials()) stop = false;
		}

		// at this point, check if we can maybe solve with finalize()
		// to avoid solving e.g. 90%+ of the remaining graph multiple times with the methods below
		if (instance.alives().size() < m_finalize_cutoff) {
			finalize(instance);
			break;
		}

		if (m_cut_rules_recs > 0 && m_articulation_point_rule_step(instance)) continue;
		else if (m_cut_rules_recs > 0 && m_cut2_rule_step(instance)) continue;

		break;
	}

	// before trying to finalize() the entire remaining instance after exhausting reduction steps, try allowing 1cut and 2cut rules to use bigger components
	size_t non_W_cnt = 0;
	for (size_t x : instance.alives()) non_W_cnt += !instance.W(x);
	if (non_W_cnt >= m_finalize_cutoff && m_cut_rules_recs > 0) {
		std::ostringstream oss;
		// oss << "c increased cutoff constants from (" << m_finalize_cutoff << ", " << m_articulation_point_component_size_cutoff << ", " << m_cut2_component_size_cutoff << ")";
		size_t finalize_cutoff = m_finalize_cutoff + 100;
		size_t articulation_point_component_size_cutoff = m_articulation_point_component_size_cutoff + 100;
		size_t cut2_component_size_cutoff = m_cut2_component_size_cutoff + 100;
		// oss << " to (" << finalize_cutoff << ", " << articulation_point_component_size_cutoff << ", " << cut2_component_size_cutoff << ")";
// #ifdef ENABLE_STDERR
// 		std::cerr << oss.str() << std::endl;
// #endif
// 		std::cout << oss.str() << std::endl;
		Solver solver(finalize_cutoff, articulation_point_component_size_cutoff, cut2_component_size_cutoff, m_cut_rules_recs);
		solver.solve(instance);
	}

	finalize(instance);
}

void Metrics::log(bool include_stderr) const {
	std::ostringstream oss;

	oss << "c rule 1 steps: " << rule1_steps << ", deleted " << rule1_deletions << " total vertices\n";
	oss << "c rule 2 steps: " << rule2_steps << ", deleted " << rule2_deletions << " total vertices (added "
		<< rule2_additions_vertices << " vertices and " << rule2_additions_edges << " edges)\n";
	oss << "c island vertices removed: " << island_vertices_removed << "\n";
	oss << "c leaves peeled: " << leaves_peeled << "\n";
	oss << "c edges between triangles with one vertex having degree 2 removed: " << tri_tri_edges_removed << "\n";
	oss << "c edges between two white vertices removed: " << white_white_edges_removed << "\n";
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
	articulation_point_reductions.insert(articulation_point_reductions.end(), metrics.articulation_point_reductions.begin(), metrics.articulation_point_reductions.end());
	cut2_missing_gadget_encounters += metrics.cut2_missing_gadget_encounters;
	cut2_hits.insert(cut2_hits.end(), metrics.cut2_hits.begin(), metrics.cut2_hits.end());
}
