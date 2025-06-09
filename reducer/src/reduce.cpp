#include "reduce.h"

#include "finalize.h"
#include "cut2_gadgets.h"

#include <sstream>
#include <queue>
#include <cstdint>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <cassert>

Reducer::Reducer(
	std::function <Metrics (Instance&)> finalize_callback,
	size_t articulation_point_component_size_cutoff,
	size_t cut2_component_size_cutoff,
	size_t cut_rules_recs
) :
m_finalize_callback(finalize_callback),
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

void Reducer::reduce(Instance& instance) {
	while (true) {
		if (instance.alives().empty()) return;

		auto perform_trivials = [&] () -> bool {
			bool did_any = false;
			bool stop_trivials = false;
			while (!stop_trivials) {
				stop_trivials = true;
				if (m_all_W(instance)) return false; // graph gets emptied if successful
				if (m_W_X_vertex_removal(instance)) stop_trivials = false;
				m_remove_island_vertices(instance);
				// this is somehow broken:
				// if (m_W_nh_vertex_removal(instance)) stop_trivials = false;
				if (m_white_white_edge_removal(instance)) stop_trivials = false;
				// seems to be broken, but also is a special case of rule_C, so leave it out!!:
				// if (m_peel_leaves(instance)) stop_trivials = false;
				if (m_tri_tri_edge_removal(instance)) stop_trivials = false;
				did_any |= !stop_trivials;
			}
			return did_any;
		};

		perform_trivials();

		if (instance.alives().empty()) return;

		auto perform_ABC = [&] () -> bool {
			bool reduced = false;
			bool stop = false;
			while (!stop) {
				stop = true;
				if (m_rule_A(instance)) stop = false;
				if (m_rule_B(instance)) stop = false;
				if (m_rule_C(instance)) stop = false;
				reduced |= !stop;
			}
			return reduced;
		};

		{
			bool both = true;
			while (both) {
				both &= perform_ABC();
				both &= perform_trivials();
				if (instance.alives().empty()) return;
			}
		}

		m_branch_by_disconnected_components(instance); // the largest one is reduced below, the rest have been branched by and will not be reduced further

		m_d3.clear();
		for (size_t v : instance.alives()) {
			if (instance.X(v)) continue;
			std::queue <std::pair <size_t, uint8_t>> q;
			q.emplace(v, 0);
			std::unordered_set <size_t> vis;
			while (!q.empty()) {
				auto [u, d] = q.front();
				q.pop();
				if (vis.contains(u)) continue;
				if (d > 3) continue;
				if (!instance.X(u)) m_d3.emplace_back(v, u);
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
			// rule1 is a bit broken after adding vertex-label resetting (fixable), but is also a special case of rule_A+rule_B
			// while (m_rule1_step(instance)) m_metrics.rule1_steps++, stop = false;
			while (m_rule2_step(instance)) m_metrics.rule2_steps++, stop = false;
			if (perform_trivials()) stop = false;
			if (instance.alives().empty()) return;
			if (perform_ABC()) stop = false;
		}

		if (m_cut_rules_recs > 0 && m_articulation_point_rule_step(instance)) continue;
		else if (m_cut_rules_recs > 0 && m_cut2_rule_step(instance)) continue;

		break;
	}

	// before trying to finalize() the entire remaining instance after exhausting reduction steps, try allowing 1cut and 2cut rules to use bigger components
	if (instance.nX().size() >= std::min(m_articulation_point_component_size_cutoff, m_cut2_component_size_cutoff) && m_cut_rules_recs > 0) {
		std::ostringstream oss;
		// oss << "c increased cutoff constants from (" << m_articulation_point_component_size_cutoff << ", " << m_cut2_component_size_cutoff << ")";
		size_t articulation_point_component_size_cutoff = m_articulation_point_component_size_cutoff * 2;
		size_t cut2_component_size_cutoff = m_cut2_component_size_cutoff * 2;
		// oss << " to (" << articulation_point_component_size_cutoff << ", " << cut2_component_size_cutoff << ")";
// #ifdef ENABLE_STDERR
// 		std::cerr << oss.str() << std::endl;
// #endif
// 		std::cout << oss.str() << std::endl;
		Reducer reducer(m_finalize_callback, articulation_point_component_size_cutoff, cut2_component_size_cutoff, m_cut_rules_recs);
		reducer.reduce(instance);
		m_metrics.add(reducer.metrics());
	}
}
