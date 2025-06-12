#include "reduce.h"

#include <sstream>
#include <queue>
#include <cstdint>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <cassert>

bool Reducer::m_rule_A(Instance& instance) { // O(n)
	bool found = 0;
	std::vector <size_t> to_D;
	for (size_t u : instance.undominated()) {
		if(instance.dom(u).size() == 1) {
			to_D.emplace_back(*instance.dom(u).begin());
			found = 1;
		}
	}
	for (size_t u : to_D) {
		if(!instance.D(u)) {
			instance.insert_D(u);
			m_metrics.rule_A++;
		}
	}
	return found;
}

bool Reducer::m_rule_B(Instance& instance) { // O(n)
	bool found = 0;
	std::vector <size_t> to_X;
	std::unordered_set <size_t> lazy_determined;
	/*
	std::unordered_map <size_t, size_t> degree;
	for (size_t u : instance.undetermined()) {
		for (size_t v : instance.g()[u]) {
			degree[v]++;
		}
	}
	*/

	for (size_t u : instance.active_undetermined()) {
		if(instance.cov(u).empty()) continue;
		std::unordered_set <size_t> subset_coverage;
		size_t first = *instance.cov(u).begin();
		/*
		for (size_t v : instance.cov(u)) {
			if(degree[v] < degree[first]) {
				std::swap(v, first);
			}
		}
		*/
		for (size_t w : instance.g()[first]) {
			if (instance.undetermined().contains(w) && !lazy_determined.contains(w)) {
				subset_coverage.insert(w);
			}
		}
		if(instance.undetermined().contains(first) && !lazy_determined.contains(first)) {
			subset_coverage.insert(first);
		}
		subset_coverage.erase(u);
		for (size_t v : instance.cov(u)) {
			if(v == first) continue;
			std::vector <size_t> to_del;
			for (size_t val : subset_coverage) {
				if (!(val == v || instance.g()[v].contains(val))) to_del.emplace_back(val);
			}
			for (size_t val : to_del) {
				subset_coverage.erase(val);
			}
			if(!subset_coverage.size())break;
		}
		if (!subset_coverage.empty()) {
			to_X.emplace_back(u);
			lazy_determined.insert(u);
			/*
			for (size_t v : instance.g()[u]) {
				degree[v]--;
			}
			*/
			found = 1;
		}
	}
	instance.clear_active_undetermined();
	for (size_t u : to_X){
		assert(!instance.X(u));
		instance.insert_X(u);
		m_metrics.rule_B++;
	}
	return found;
}

bool Reducer::m_rule_C(Instance& instance) { // O(n)
	bool found = 0;
	std::vector <size_t> to_W;
	std::unordered_set <size_t> lazy_dominated;

	/*
	std::unordered_map <size_t, size_t> degree;

	for (size_t u : instance.undominated()) {
		for (size_t v : instance.g()[u]) {
			degree[v]++;
		}
	}
	*/

	for (size_t u : instance.active_undominated()) {
		if(instance.dom(u).empty()) continue;
		if(lazy_dominated.contains(u))continue;
		std::unordered_set <size_t> ignorable_vertices;
		size_t first = *instance.dom(u).begin();
		/*
		for (size_t v : instance.dom(u)) {
			if(degree[v] < degree[first]) {
				std::swap(v, first);
			}
		}
		*/
		for (size_t w : instance.g()[first]) {
			if (instance.undominated().contains(w) && !lazy_dominated.contains(w)) {
				ignorable_vertices.insert(w);
			}
		}
		if(instance.undominated().contains(first) && !lazy_dominated.contains(first)) {
			ignorable_vertices.insert(first);
		}
		ignorable_vertices.erase(u);
		for (size_t v : instance.dom(u)) {
			if(v == first) continue;
			std::vector <size_t> to_del;
			for (size_t val : ignorable_vertices) {
				if (!(val == v || instance.g()[v].contains(val))) to_del.emplace_back(val);
			}
			for (size_t val : to_del) {
				ignorable_vertices.erase(val);
			}
			if(!ignorable_vertices.size())break;
		}
		for (size_t v : ignorable_vertices) {
			if(!instance.W(v) && !lazy_dominated.contains(v)) {
				to_W.push_back(v);
				lazy_dominated.insert(v);
				/*
				for (size_t w : instance.g()[v]) {
					degree[w]--;
				}
				*/
			}
		}
	}
	instance.clear_active_undominated();
	if(to_W.size()) {
		found = 1;
		for (size_t u : to_W) {
			assert(!instance.W(u));
			instance.insert_W(u);
			m_metrics.rule_C++;
		}
	}
	return found;
}

bool Reducer::m_rule1_step(Instance& instance) {
	for (size_t v : instance.nX()) {
		std::unordered_set <size_t> N_exit, N_guard, N_prison;
		for (size_t u : instance.g()[v]) {
			if (instance.X(u)) continue;
			bool is_exit = false;
			for (size_t w : instance.g()[u]) {
				if (instance.W(w)) continue;
				if (!instance.g()[v].contains(w) && w != v) {
					is_exit = true;
					break;
				}
			}
			if (is_exit) {
				N_exit.insert(u);
			}
		}
		for (size_t u : instance.g()[v]) {
			if (N_exit.contains(u)) continue;
			if (instance.X(u)) {
				bool outside_connection = false;
				for (size_t w : instance.g()[u]) {
					if (!instance.g()[v].contains(w) && w != v) {
						outside_connection = true;
						break;
					}
				}
				if (outside_connection) {
					N_guard.insert(u);
					continue;
				}
			}
			// if there are any outside connections at this point, they must be white
			// if there are any, instantly become a guard
			bool outside_connection = false;
			for  (size_t w : instance.g()[u]) {
				if (!instance.g()[v].contains(w) && w != v) {
					outside_connection = true;
					break;
				}
			}
			if (outside_connection) {
				N_guard.insert(u);
				continue;
			}
			bool is_guard = false;
			for (size_t w : instance.g()[u]) {
				if (N_exit.contains(w)) {
					is_guard = true;
					break;
				}
			}
			if (is_guard) {
				N_guard.insert(u);
			}
		}
		for (size_t u : instance.g()[v]) {
			if (!N_exit.contains(u) && !N_guard.contains(u)) {
				N_prison.insert(u);
			}
		}
		bool candidate = false;
		for (size_t u : N_prison) {
			if (!instance.W(u)) {
				candidate = true;
				break;
			}
		}
		if (!candidate) continue;
		std::vector <size_t> to_del;
		for (size_t u : instance.g()[v]) {
			if (N_guard.contains(u) || N_prison.contains(u)) {
				to_del.push_back(u);
			}
		}
		instance.insert_D(v);
		m_metrics.rule1_deletions++;
		for (size_t u : to_del) {
			instance.erase(u);
			m_metrics.rule1_deletions++;
		}
		return true;
	}
	return false;
}

bool Reducer::m_rule2_step(Instance& instance) {
	for (auto [v, w] : m_d3) {
		if (v == w) continue;
		if (instance.X(v) || instance.X(w)) continue;
		std::unordered_set <size_t> N_exit, N_guard, N_prison;
		std::unordered_set <size_t> N_vw;
		for (size_t u : instance.g()[v]) {
			N_vw.insert(u);
		}
		for (size_t u : instance.g()[w]) {
			N_vw.insert(u);
		}
		for (size_t u : N_vw) {
			bool is_exit = false;
			for (size_t x : instance.g()[u]) {
				if (!N_vw.contains(x) && x != v && x != w) {
					is_exit = true;
					break;
				}
			}
			if (is_exit) {
				N_exit.insert(u);
			}
		}
		for (size_t u : N_vw) {
			if (N_exit.contains(u)) continue;
			bool is_guard = false;
			for (size_t x : instance.g()[u]) {
				if (N_exit.contains(x)) {
					is_guard = true;
					break;
				}
			}
			if (is_guard) {
				N_guard.insert(u);
			}
		}
		for (size_t u : N_vw) {
			if (!N_exit.contains(u) && !N_guard.contains(u)) {
				N_prison.insert(u);
			}
		}
		bool candidate = false;
		for (size_t u : N_prison) {
			if (!instance.W(u)) {
				candidate = true;
				break;
			}
		}
		for (size_t u : N_guard) {
			if (!candidate) break;
			bool dominates = true;
			for (size_t x : N_prison) {
				if (instance.W(x)) continue;
				assert(u != x);
				dominates &= instance.g()[u].contains(x);
			}
			candidate &= !dominates;
		}
		for (size_t u : N_prison) {
			if (!candidate) break;
			bool dominates = true;
			for (size_t x : N_prison) {
				if (instance.W(x)) continue;
				dominates &= instance.g()[u].contains(x) || u == x;
			}
			candidate &= !dominates;
		}
		if (!candidate) continue;
		bool v_dominates = true, w_dominates = true;
		for (size_t u : N_prison) {
			if (instance.W(u)) continue;
			v_dominates &= instance.g()[v].contains(u);
			w_dominates &= instance.g()[w].contains(u);
		}
		if (v_dominates || w_dominates) {
			if (v_dominates && w_dominates) {
				std::vector <size_t> to_del;
				for (size_t u : N_prison) {
					to_del.push_back(u);
				}
				for (size_t u : N_guard) {
					if (instance.g()[v].contains(u) && instance.g()[w].contains(u)) {
						to_del.push_back(u);
					}
				}
				bool all_banned = true;
				for (size_t u : to_del) {
					all_banned &= m_rule2_nh_banned.contains(u);
				}
				if (all_banned) { // abort, we isolated only vertices that will be replaced by identical vertices z1,z2 -> infinite loop
					// assert(to_del.size() == 2);
					continue;
				}
				for (size_t u : to_del) {
					instance.erase(u);
					m_metrics.rule2_deletions++;
				}
				size_t z1 = instance.insert();
				size_t z2 = instance.insert();
				m_metrics.rule2_additions_vertices += 2;
				m_rule2_nh_banned.insert(z1);
				m_rule2_nh_banned.insert(z2);
				instance.add_edge(v, z1);
				instance.add_edge(w, z1);
				instance.add_edge(v, z2);
				instance.add_edge(w, z2);
				m_metrics.rule2_additions_edges += 4;
				// z1,z2 are not meant to be in D, only meant to force at least one of v,w into D
				instance.insert_X(z1);
				instance.insert_X(z2);
				instance.add_adjusting_callback([v, w, z1, z2] (Instance& inst) -> void {
					if (inst.D(z1) || inst.D(z2)) {
						if (!inst.D(v)) {
							if (inst.alive(v)) inst.insert_D(v);
							else inst.insert_dead_into_D(v);
						}
						if (!inst.D(w)) {
							if (inst.alive(w)) inst.insert_D(w);
							else inst.insert_dead_into_D(w);
						}
						if (inst.D(z1)) inst.remove_from_D(z1);
						if (inst.D(z2)) inst.remove_from_D(z2);
					}
				});
			} else if (v_dominates) {
				std::vector <size_t> to_del;
				for (size_t u : N_prison) {
					to_del.push_back(u);
				}
				for (size_t u : N_guard) {
					if (instance.g()[v].contains(u)) {
						to_del.push_back(u);
					}
				}
				instance.insert_D(v);
				m_metrics.rule2_deletions++;
				for (size_t u : to_del) {
					instance.erase(u);
					m_metrics.rule2_deletions++;
				}
			} else if (w_dominates) {
				std::vector <size_t> to_del;
				for (size_t u : N_prison) {
					to_del.push_back(u);
				}
				for (size_t u : N_guard) {
					if (instance.g()[w].contains(u)) {
						to_del.push_back(u);
					}
				}
				instance.insert_D(w);
				m_metrics.rule2_deletions++;
				for (size_t u : to_del) {
					instance.erase(u);
					m_metrics.rule2_deletions++;
				}
			} else {
				throw std::runtime_error("unexpected error");
			}
		} else {
			std::vector <size_t> to_del;
			for (size_t u : N_guard) {
				if (u != v && u != w) {
					to_del.push_back(u);
				}
			}
			for (size_t u : N_prison) {
				if (u != v && u != w) {
					to_del.push_back(u);
				}
			}
			instance.insert_D(v);
			instance.insert_D(w);
			m_metrics.rule2_deletions += 2;
			for (size_t u : to_del) {
				instance.erase(u);
				m_metrics.rule2_deletions++;
			}
		}
		return true;
	}
	return false;
}

bool Reducer::m_articulation_point_rule_step(Instance& instance) {
	auto articulation_points = [] (const Instance& inst) -> std::vector <size_t> {
		std::vector <bool> vis(inst.g().n, false);
		std::vector <size_t> time(inst.g().n, ~static_cast <size_t> (0)), furthest(inst.g().n, ~static_cast <size_t> (0));
		size_t timer = 0;
		std::vector <size_t> result;
		auto dfs = [&] (auto&& self, size_t v, size_t p = ~static_cast <size_t> (0)) -> void {
			vis[v] = true;
			time[v] = furthest[v] = timer++;
			size_t chc = 0;
			for (size_t u : inst.g()[v]) {
				if (u == p) continue;
				if (vis[u]) furthest[v] = std::min(furthest[v], time[u]);
				else {
					self(self, u, v);
					furthest[v] = std::min(furthest[v], furthest[u]);
					if (furthest[u] >= time[v] && p != ~static_cast <size_t> (0)) result.push_back(v);
					chc++;
				}
			}
			if (p == ~static_cast <size_t> (0) && chc > 1) result.push_back(v);
		};
		for (size_t x : inst.alives()) {
			if (!vis[x]) {
				dfs(dfs, x);
			}
		}
		return result;
	};
	auto go = [&] (Instance& inst) -> size_t {
		m_metrics.add(m_finalize_callback(inst));
		size_t cost = 0;
		for (size_t i = 0; i < inst.g().n; i++) {
			cost += inst.D(i);
		}
		return cost;
	};
	// try to find an articulation point that isolates a component of useful size
	bool did_find = false;
	auto arts = articulation_points(instance);
	std::random_device rd;
	std::mt19937 mtgen(rd());
	std::shuffle(arts.begin(), arts.end(), mtgen);
	for (size_t v : arts) {
		if (did_find) break; // only process one v successfully at a time to not break anything
		std::vector <std::vector <size_t>> comps;
		std::vector <bool> vis(instance.g().n, false);
		vis[v] = true;
		for (size_t u : instance.g()[v]) {
			if (vis[u]) continue;
			comps.push_back({ });
			std::queue <size_t> q;
			q.push(u);
			while (!q.empty()) {
				size_t x = q.front();
				q.pop();
				if (vis[x]) continue;
				vis[x] = true;
				comps.back().push_back(x);
				for (size_t y : instance.g()[x]) {
					q.push(y);
				}
			}
		}
		assert(comps.size() > 1); // sanity check
		// never explore the largest component
		// instead: exhaust the smaller ones first
		// since we are solving the component we pick more than once in order to remove it
		std::sort(comps.begin(), comps.end(), [] (const std::vector <size_t>& lhs, const std::vector <size_t>& rhs) { return lhs.size() < rhs.size(); });
		comps.pop_back();
		for (auto comp : comps) {
			if (comp.size() < 2 || comp.size() > m_articulation_point_component_size_cutoff) continue;
			std::unordered_map <size_t, size_t> cc;
			for (size_t x : comp) {
				cc[x] = cc.size();
			}
			assert(cc.count(comp[0]) && cc[comp[0]] == 0);
			assert(cc.size() == comp.size());
			// three cases, either v is in W or in X or in neither
			did_find = true;
			m_metrics.articulation_point_reductions.push_back(comp.size());
			if (instance.X(v)) { // v cannot be in D, need to determine if we can dominate it internally
				if (instance.W(v)) { // if this somehow happens, just immediately erase v and move on
					instance.erase(v);
					break;
				}
				G ng_wov(comp.size());
				for (size_t x : comp) {
					for (size_t y : instance.g()[x]) {
						if (y == v) continue; // skip the boundary vertex
						if (x < y) ng_wov.add(cc[x], cc[y]), assert(cc[x] < ng_wov.n && cc[y] < ng_wov.n);
					}
				}
				Instance ni_vninD(ng_wov);
				for (size_t x : comp) {
					if (instance.W(x)) {
						ni_vninD.insert_W(cc[x]);
					}
				}
				for (size_t x : comp) {
					if (instance.D_nei(x)) {
						ni_vninD.insert_W_Dnei(cc[x]);
					}
				}
				for (size_t x : comp) {
					if (instance.X(x)) {
						ni_vninD.insert_X(cc[x]);
					}
				}
				bool can_be_dominated_from_outside = false;
				for (size_t x : instance.g()[v]) {
					if (cc.contains(x)) continue;
					if (!instance.X(x)) can_be_dominated_from_outside = true;
				}
				size_t cost_v_off = can_be_dominated_from_outside ? go(ni_vninD) : 1ULL << 60;
				G ng_wv(comp.size() + 1);
				cc[v] = cc.size();
				for (size_t x : comp) {
					for (size_t y : instance.g()[x]) {
						if (x < y || y == v) ng_wv.add(cc[x], cc[y]), assert(cc[x] < ng_wv.n && cc[y] < ng_wv.n); // this time also add edges to v
					}
				}
				Instance ni_vninD_v_dominated_internally(ng_wv);
				// if (instance.W(v)) ni_vninD_v_dominated_internally.insert_W(cc[v]);
				// if (instance.D_nei(v)) ni_vninD_v_dominated_internally.insert_W_Dnei(cc[v]);
				if (instance.W(v)) assert(false);
				if (instance.D_nei(v)) assert(false);
				if (!instance.X(v)) assert(false);
				for (size_t x : comp) {
					if (instance.W(x)) {
						ni_vninD_v_dominated_internally.insert_W(cc[x]);
					}
				}
				for (size_t x : comp) {
					if (instance.D_nei(x)) {
						ni_vninD_v_dominated_internally.insert_W_Dnei(cc[x]);
					}
				}
				for (size_t x : comp) {
					if (instance.X(x)) {
						ni_vninD_v_dominated_internally.insert_X(cc[x]);
					}
				}
				bool v_off_v_dominated_internally_impossible = false;
				if (ni_vninD_v_dominated_internally.can_insert_X(cc[v])) ni_vninD_v_dominated_internally.insert_X(cc[v]);
				else v_off_v_dominated_internally_impossible = true;
				size_t cost_v_off_v_dominated_internally = v_off_v_dominated_internally_impossible ? 1ULL << 60 : go(ni_vninD_v_dominated_internally);
				assert(can_be_dominated_from_outside || !v_off_v_dominated_internally_impossible);
				if (
					!can_be_dominated_from_outside ||
					(!v_off_v_dominated_internally_impossible && cost_v_off == cost_v_off_v_dominated_internally)
				) { // might as well dominate v internally
					for (size_t i = 0; i < comp.size(); i++) {
						if (ni_vninD_v_dominated_internally.D(i)) {
							assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
							if (instance.X(comp[i])) instance.remove_X(comp[i]);
							instance.insert_D(comp[i]);
						}
					}
					assert(instance.W(v)); // at this point, the solution to the internal component should dominate v
				} else { // it costs extra to dominate v internally, so leave it be
					assert(can_be_dominated_from_outside);
					for (size_t i = 0; i < comp.size(); i++) {
						if (ni_vninD.D(i)) {
							assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
							if (instance.X(comp[i])) instance.remove_X(comp[i]);
							instance.insert_D(comp[i]);
						}
					}
				}
			} else if (!instance.W(v)) { // v is not already dominated
				// first check if we can just put v in D immediately (i.e. if cost_v_off > cost_v_on)
				// if not, then cost_v_off == cost_v_on and we must decide whether to also dominate v internally
				// we only dominate v interally if it doesn't increase the cost,
				//  - if it does increase the cost and the outside prefers we dominate it, then just pay to include v in D instead, this is solved for later
				G ng_wov(comp.size());
				for (size_t x : comp) {
					for (size_t y : instance.g()[x]) {
						if (y == v) continue; // skip the boundary vertex
						if (x < y) ng_wov.add(cc[x], cc[y]), assert(cc[x] < ng_wov.n && cc[y] < ng_wov.n);
					}
				}
				Instance ni_vninD(ng_wov);
				for (size_t x : comp) {
					if (instance.W(x)) {
						ni_vninD.insert_W(cc[x]);
					}
				}
				for (size_t x : comp) {
					if (instance.D_nei(x)) {
						ni_vninD.insert_W_Dnei(cc[x]);
					}
				}
				Instance ni_vinD = ni_vninD;
				for (size_t x : instance.g()[v]) {
					if (cc.find(x) != cc.end()) {
						ni_vinD.insert_W_Dnei(cc[x]);
					}
				}
				bool v_off_impossible = false;
				for (size_t x : comp) {
					if (instance.X(x)) {
						if (!ni_vninD.can_insert_X(cc[x])) v_off_impossible = true;
						else ni_vninD.insert_X(cc[x]);
						ni_vinD.insert_X(cc[x]);
					}
				}
				bool all_outside_nei_are_X = true;
				for (size_t x : instance.g()[v]) {
					if (cc.contains(x)) continue;
					all_outside_nei_are_X &= instance.X(x);
				}
				if (all_outside_nei_are_X) v_off_impossible = true;
				size_t cost_v_off = v_off_impossible ? 1ULL << 60 : go(ni_vninD);
				size_t cost_v_on = go(ni_vinD);
				if (all_outside_nei_are_X) {
					G ng_wv(comp.size() + 1);
					cc[v] = cc.size();
					for (size_t x : comp) {
						for (size_t y : instance.g()[x]) {
							if (x < y || y == v) ng_wv.add(cc[x], cc[y]), assert(cc[x] < ng_wv.n && cc[y] < ng_wv.n); // this time also add edges to v
						}
					}
					Instance ni_vninD_v_dominated_internally(ng_wv);
					for (size_t x : comp) {
						if (instance.W(x)) {
							ni_vninD_v_dominated_internally.insert_W(cc[x]);
						}
					}
					for (size_t x : comp) {
						if (instance.D_nei(x)) {
							ni_vninD_v_dominated_internally.insert_W_Dnei(cc[x]);
						}
					}
					for (size_t x : comp) {
						if (instance.X(x)) {
							ni_vninD_v_dominated_internally.insert_X(cc[x]);
						}
					}
					size_t cost_v_off_v_dominated_internally = go(ni_vninD_v_dominated_internally);
					if (cost_v_off_v_dominated_internally <= cost_v_on) {
						if (ni_vninD_v_dominated_internally.D(cc[v])) instance.insert_D(v); // the solution might have decided to use v for D? (disprove?)
						for (size_t i = 0; i < comp.size(); i++) {
							if (ni_vninD_v_dominated_internally.D(i)) {
								assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
								if (instance.X(comp[i])) instance.remove_X(comp[i]);
								instance.insert_D(comp[i]);
							}
						}
						assert(instance.D(v) || instance.W(v)); // at this point, the solution to the internal component should dominate v
					} else {
						instance.insert_D(v);
						// pick solution where v in D
						for (size_t i = 0; i < comp.size(); i++) {
							if (ni_vinD.D(i)) {
								assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
								if (instance.X(comp[i])) instance.remove_X(comp[i]);
								instance.insert_D(comp[i]);
							}
						}
					}
				} else if (cost_v_off > cost_v_on) { // insert v in D, leaving v out of D is never a strictly better choice
					instance.insert_D(v);
					// pick solution where v in D
					for (size_t i = 0; i < comp.size(); i++) {
						if (ni_vinD.D(i)) {
							assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
							if (instance.X(comp[i])) instance.remove_X(comp[i]);
							instance.insert_D(comp[i]);
						}
					}
				} else { // need to decide whether to dominate v internally
					assert(cost_v_off == cost_v_on); // sanity check that the cost with v off is not cheaper than with v on
					G ng_wv(comp.size() + 1);
					cc[v] = cc.size();
					for (size_t x : comp) {
						for (size_t y : instance.g()[x]) {
							if (x < y || y == v) ng_wv.add(cc[x], cc[y]), assert(cc[x] < ng_wv.n && cc[y] < ng_wv.n); // this time also add edges to v
						}
					}
					Instance ni_vninD_v_dominated_internally(ng_wv);
					for (size_t x : comp) {
						if (instance.W(x)) {
							ni_vninD_v_dominated_internally.insert_W(cc[x]);
						}
					}
					for (size_t x : comp) {
						if (instance.D_nei(x)) {
							ni_vninD_v_dominated_internally.insert_W_Dnei(cc[x]);
						}
					}
					for (size_t x : comp) {
						if (instance.X(x)) {
							ni_vninD_v_dominated_internally.insert_X(cc[x]);
						}
					}
					size_t cost_v_off_v_dominated_internally = go(ni_vninD_v_dominated_internally);
					if (cost_v_off == cost_v_off_v_dominated_internally) { // might as well dominate v internally
						assert(!instance.W(v));
						if (ni_vninD_v_dominated_internally.D(cc[v])) instance.insert_D(v); // the solution might have decided to use v for D? (disprove?)
						for (size_t i = 0; i < comp.size(); i++) {
							if (ni_vninD_v_dominated_internally.D(i)) {
								assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
								if (instance.X(comp[i])) instance.remove_X(comp[i]);
								instance.insert_D(comp[i]);
							}
						}
						assert(instance.D(v) || instance.W(v)); // at this point, the solution to the internal component should dominate v
					} else { // it costs extra to dominate v internally, so leave it be
						assert(!v_off_impossible);
						for (size_t i = 0; i < comp.size(); i++) {
							if (ni_vninD.D(i)) {
								assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
								if (instance.X(comp[i])) instance.remove_X(comp[i]);
								instance.insert_D(comp[i]);
							}
						}
					}
				}
			} else { // v is dominated
				// either insert v into D or leave v dominated externally, do not try to dominate it internally, we don't care
				G ng_wov(comp.size());
				for (size_t x : comp) {
					for (size_t y : instance.g()[x]) {
						if (y == v) continue; // skip the boundary vertex
						if (x < y) ng_wov.add(cc[x], cc[y]), assert(cc[x] < ng_wov.n && cc[y] < ng_wov.n);
					}
				}
				Instance ni_vninD(ng_wov);
				for (size_t x : comp) {
					if (instance.W(x)) {
						ni_vninD.insert_W(cc[x]);
					}
				}
				for (size_t x : comp) {
					if (instance.D_nei(x)) {
						ni_vninD.insert_W_Dnei(cc[x]);
					}
				}
				Instance ni_vinD = ni_vninD;
				for (size_t x : instance.g()[v]) {
					if (cc.find(x) != cc.end()) {
						ni_vinD.insert_W_Dnei(cc[x]);
					}
				}
				bool v_off_impossible = false;
				for (size_t x : comp) {
					if (instance.X(x)) {
						if (!ni_vninD.can_insert_X(cc[x])) v_off_impossible = true;
						else ni_vninD.insert_X(cc[x]);
						ni_vinD.insert_X(cc[x]);
					}
				}
				size_t cost_v_off = v_off_impossible ? 1ULL << 60 : go(ni_vninD);
				size_t cost_v_on = go(ni_vinD);
				if (cost_v_off > cost_v_on) { // might as well put v in D
					instance.insert_D(v);
					// pick solution where v in D
					for (size_t i = 0; i < comp.size(); i++) {
						if (ni_vinD.D(i)) {
							assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
							if (instance.X(comp[i])) instance.remove_X(comp[i]);
							instance.insert_D(comp[i]);
						}
					}
				} else { // cost with v in D = cost with v not in D ->
						 // v is already dominated, so pick solution that doesn't need v in D, this is cheaper if v never enters D in the future
					assert(cost_v_off == cost_v_on); // sanity check that the cost with v off is not cheaper than with v on
					assert(!v_off_impossible);
					for (size_t i = 0; i < comp.size(); i++) {
						if (ni_vninD.D(i)) {
							assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
							if (instance.X(comp[i])) instance.remove_X(comp[i]);
							instance.insert_D(comp[i]);
						}
					}
				}
			}
			// at this point, for sure all vertices in comp are accounted for and should not change state, so remove those that are in W (which haven't been removed)
			for (size_t x : comp) {
				if (instance.alive(x)) {
					assert(instance.W(x));
					instance.erase(x);
				}
			}
			// if v has been inserted into D at this point, immediately recurse, the new disconnected components of proper size will be accounted for immediately anyway
			if (instance.D(v)) break;
			// if v ends up in both X and W, erase it and break
			if (instance.X(v) && instance.W(v)) {
				instance.erase(v);
				break;
			}
		}
	}
	return did_find;
}

#include "cut2_gadgets.h"

bool Reducer::m_cut2_rule_step(Instance& instance) {
	auto articulation_points = [] (const Instance& inst, size_t banned) -> std::vector <size_t> {
		std::vector <bool> vis(inst.g().n, false);
		std::vector <size_t> time(inst.g().n, ~static_cast <size_t> (0)), furthest(inst.g().n, ~static_cast <size_t> (0));
		size_t timer = 0;
		std::vector <size_t> result;
		auto dfs = [&] (auto&& self, size_t v, size_t p = ~static_cast <size_t> (0)) -> void {
			vis[v] = true;
			time[v] = furthest[v] = timer++;
			size_t chc = 0;
			for (size_t u : inst.g()[v]) {
				if (u == p) continue;
				if (u == banned) continue;
				if (vis[u]) furthest[v] = std::min(furthest[v], time[u]);
				else {
					self(self, u, v);
					furthest[v] = std::min(furthest[v], furthest[u]);
					if (furthest[u] >= time[v] && p != ~static_cast <size_t> (0)) result.push_back(v);
					chc++;
				}
			}
			if (p == ~static_cast <size_t> (0) && chc > 1) result.push_back(v);
		};
		for (size_t x : inst.alives()) {
			if (!vis[x] && x != banned) {
				dfs(dfs, x);
			}
		}
		return result;
	};
	auto go = [&] (Instance& inst, size_t dont_count_1, size_t dont_count_2) -> size_t {
		m_metrics.add(m_finalize_callback(inst));
		size_t cost = 0;
		for (size_t i = 0; i < inst.g().n; i++) {
			if (i == dont_count_1) continue;
			if (i == dont_count_2) continue;
			cost += inst.D(i);
		}
		return cost;
	};
	// for each vertex v, find all articulation points given V' = V - {v}
	// this is a 2-vertex-cut
	for (size_t v : instance.alives()) {
		auto arts = articulation_points(instance, v);
		for (auto u : arts) {
			// cut is {v,u}
			// find components disconnected by {v,u}
			std::vector <std::vector <size_t>> comps;
			std::vector <bool> vis(instance.g().n, false);
			vis[v] = vis[u] = true;
			std::vector <size_t> cut_nh(instance.g()[v].begin(), instance.g()[v].end());
			cut_nh.insert(cut_nh.end(), instance.g()[u].begin(), instance.g()[u].end());
			for (size_t x : cut_nh) {
				if (vis[x]) continue;
				comps.push_back({ });
				std::queue <size_t> q;
				q.push(x);
				while (!q.empty()) {
					size_t y = q.front();
					q.pop();
					if (vis[y]) continue;
					vis[y] = true;
					comps.back().push_back(y);
					for (size_t z : instance.g()[y]) {
						q.push(z);
					}
				}
			}
			assert(comps.size() > 1); // sanity check
			// never explore the largest component since we are solving the component we pick more than once in order to remove it
			std::sort(comps.begin(), comps.end(), [] (const std::vector <size_t>& lhs, const std::vector <size_t>& rhs) { return lhs.size() < rhs.size(); });
			// first, check if the big one is the only one with size >= 10
			// if so, allow choosing the big one if there are many components < 10 in size
			// (this should be rare)
			size_t too_small = 0;
			size_t too_small_sum = 0;
			for (const auto& comp : comps) too_small += comp.size() < 10, too_small_sum += (comp.size() < 10) * comp.size();
			if (too_small + 1 == comps.size() && too_small_sum > comps.back().size() / 4); // allow keeping the big one
			else comps.pop_back();
			// find one of the components of appropriate size
			for (const auto& comp : comps) {
				if (comp.size() < 10) continue;
				if (comp.size() > m_cut2_component_size_cutoff) continue;
				// check if we have a matching gadget
				std::unordered_map <size_t, size_t> cc;
				for (size_t i = 0; i < comp.size(); i++) cc[comp[i]] = i;
				cc[v] = comp.size();
				cc[u] = comp.size() + 1;
				G cg00(comp.size() + 2);
				G cg10(comp.size() + 2);
				G cg01(comp.size() + 2);
				G cg11(comp.size() + 2);
				for (size_t x : comp) {
					for (size_t y : instance.g()[x]) {
						if (x > y && y != v && y != u) continue;
						if (y == v) cg10.add(cc[x], cc[y]), cg11.add(cc[x], cc[y]);
						else if (y == u) cg01.add(cc[x], cc[y]), cg11.add(cc[x], cc[y]);
						else cg00.add(cc[x], cc[y]), cg10.add(cc[x], cc[y]), cg01.add(cc[x], cc[y]), cg11.add(cc[x], cc[y]);
					}
				}
				std::vector <Instance> sols;
				std::vector <std::tuple <cut2::State, cut2::State, size_t>> unclamped_costs;
				{ // i, i
					Instance inst(cg11);
					for (size_t i = 0; i < comp.size(); i++) if (instance.D_nei(comp[i])) inst.insert_W_Dnei(i);
					unclamped_costs.push_back({ cut2::State::I, cut2::State::I, go(inst, ~0ULL, ~0ULL) });
					sols.push_back(inst);
				}
				{ // i, u
					Instance inst(cg10);
					for (size_t i = 0; i < comp.size(); i++) if (instance.D_nei(comp[i])) inst.insert_W_Dnei(i);
					unclamped_costs.push_back({ cut2::State::I, cut2::State::U, go(inst, ~0ULL, cc[u]) });
					sols.push_back(inst);
				}
				{ // i, d
					Instance inst(cg10);
					for (size_t i = 0; i < comp.size(); i++) if (instance.D_nei(comp[i])) inst.insert_W_Dnei(i);
					for (size_t x : instance.g()[u]) if (cc.contains(x) && x != v && x != u) inst.insert_W_Dnei(cc[x]);
					unclamped_costs.push_back({ cut2::State::I, cut2::State::D, go(inst, ~0ULL, cc[u]) });
					sols.push_back(inst);
				}
				{ // u, i
					Instance inst(cg01);
					for (size_t i = 0; i < comp.size(); i++) if (instance.D_nei(comp[i])) inst.insert_W_Dnei(i);
					unclamped_costs.push_back({ cut2::State::U, cut2::State::I, go(inst, cc[v], ~0ULL) });
					sols.push_back(inst);
				}
				{ // u, u
					Instance inst(cg00);
					for (size_t i = 0; i < comp.size(); i++) if (instance.D_nei(comp[i])) inst.insert_W_Dnei(i);
					unclamped_costs.push_back({ cut2::State::U, cut2::State::U, go(inst, cc[v], cc[u]) });
					sols.push_back(inst);
				}
				{ // u, d
					Instance inst(cg00);
					for (size_t i = 0; i < comp.size(); i++) if (instance.D_nei(comp[i])) inst.insert_W_Dnei(i);
					for (size_t x : instance.g()[u]) if (cc.contains(x) && x != v && x != u) inst.insert_W_Dnei(cc[x]);
					unclamped_costs.push_back({ cut2::State::U, cut2::State::D, go(inst, cc[v], cc[u]) });
					sols.push_back(inst);
				}
				{ // d, i
					Instance inst(cg01);
					for (size_t i = 0; i < comp.size(); i++) if (instance.D_nei(comp[i])) inst.insert_W_Dnei(i);
					for (size_t x : instance.g()[v]) if (cc.contains(x) && x != v && x != u) inst.insert_W_Dnei(cc[x]);
					unclamped_costs.push_back({ cut2::State::D, cut2::State::I, go(inst, cc[v], ~0ULL) });
					sols.push_back(inst);
				}
				{ // d, u
					Instance inst(cg00);
					for (size_t i = 0; i < comp.size(); i++) if (instance.D_nei(comp[i])) inst.insert_W_Dnei(i);
					for (size_t x : instance.g()[v]) if (cc.contains(x) && x != v && x != u) inst.insert_W_Dnei(cc[x]);
					unclamped_costs.push_back({ cut2::State::D, cut2::State::U, go(inst, cc[v], cc[u]) });
					sols.push_back(inst);
				}
				{ // d, d
					Instance inst(cg00);
					for (size_t i = 0; i < comp.size(); i++) if (instance.D_nei(comp[i])) inst.insert_W_Dnei(i);
					for (size_t x : instance.g()[v]) if (cc.contains(x) && x != v && x != u) inst.insert_W_Dnei(cc[x]);
					for (size_t x : instance.g()[u]) if (cc.contains(x) && x != v && x != u) inst.insert_W_Dnei(cc[x]);
					unclamped_costs.push_back({ cut2::State::D, cut2::State::D, go(inst, cc[v], cc[u]) });
					sols.push_back(inst);
				}
				size_t min_cost = std::numeric_limits <size_t>::max();
				for (const auto& t : unclamped_costs) min_cost = std::min(min_cost, std::get <2> (t));
				cut2::Costs costs;
				for (auto& t : unclamped_costs) {
					std::get <2> (t) -= min_cost;
					std::get <2> (t) = std::min(std::get <2> (t), static_cast <size_t> (2));
					costs.set_cost(std::get <0> (t), std::get <1> (t), std::get <2> (t));
				}
				if (!cut2::has_gadget(costs)) {
					m_metrics.cut2_missing_gadget_encounters++;
					continue;
				}
				m_metrics.cut2_hits.push_back(comp.size());
				cut2::Gadget gadget = cut2::get_gadget(costs);
				Instance ni(G(instance.g().n + gadget.N));
				for (size_t x : instance.alives()) for (size_t y : instance.g()[x]) if (x < y) ni.add_edge(x, y);
				for (auto [x, y] : gadget.edges) {
					size_t xx = x + instance.g().n;
					size_t yy = y + instance.g().n;
					if (x == gadget.N) xx = v;
					if (x == gadget.N + 1) xx = u;
					if (y == gadget.N) yy = v;
					if (y == gadget.N + 1) yy = u;
					if (x >= gadget.N && y >= gadget.N) continue; // edge between c1 and c2 is already inserted above
					ni.add_edge(xx, yy);
				}
				for (size_t i = 0; i < instance.g().n; i++) {
					if (instance.alives().contains(i) && instance.D_nei(i)) ni.insert_W_Dnei(i);
					if (instance.D(i)) ni.insert_D(i);
					else if (!instance.alive(i)) ni.erase(i);
				}
				for (size_t x : comp) ni.erase(x);
				m_metrics.add(m_finalize_callback(ni));
				cut2::State c1 = ni.D(v) ? cut2::State::D : cut2::State::U;
				cut2::State c2 = ni.D(u) ? cut2::State::D : cut2::State::U;
				for (auto [x, y] : gadget.edges) {
					size_t xx = x + instance.g().n;
					size_t yy = y + instance.g().n;
					if (x == gadget.N) xx = v;
					if (x == gadget.N + 1) xx = u;
					if (y == gadget.N) yy = v;
					if (y == gadget.N + 1) yy = u;
					if (x >= gadget.N && y >= gadget.N) continue; // don't count has I if one cut-vertex is a dominator to the other, count it as U
					if (c1 != cut2::State::D && xx == v && ni.D(yy)) c1 = cut2::State::I;
					if (c1 != cut2::State::D && yy == v && ni.D(xx)) c1 = cut2::State::I;
					if (c2 != cut2::State::D && xx == u && ni.D(yy)) c2 = cut2::State::I;
					if (c2 != cut2::State::D && yy == u && ni.D(xx)) c2 = cut2::State::I;
				}
				size_t idx = (c1 == cut2::State::I ? 0 : c1 == cut2::State::U ? 1 : 2) * 3 + (c2 == cut2::State::I ? 0 : c2 == cut2::State::U ? 1 : 2);
				const Instance& sol = sols[idx];
				for (size_t i : instance.alives()) if (instance.X(i)) instance.remove_X(i);
				for (size_t i = 0; i < comp.size(); i++) if (sol.D(i)) instance.insert_D(comp[i]);
				if (c1 == cut2::State::D || (c1 == cut2::State::I && sol.D(comp.size() + 0))) instance.insert_D(v);
				if (c2 == cut2::State::D || (c2 == cut2::State::I && sol.D(comp.size() + 1))) instance.insert_D(u);
				std::vector <size_t> to_rem;
				for (size_t x : instance.alives()) if (cc.find(x) == cc.end() && ni.D(x) && !instance.D(x)) to_rem.push_back(x);
				for (size_t x : to_rem) instance.insert_D(x);
				assert(instance.undominated().empty());
				while (!instance.alives().empty()) instance.erase(*instance.alives().begin());
				return true;
			}
		}
	}
	return false;
}

void Reducer::m_branch_by_disconnected_components(Instance& instance) {
	std::vector <std::vector <size_t>> comps;
	std::vector <bool> vis(instance.g().n, false);
	for (size_t v : instance.alives()) {
		if (vis[v]) continue;
		comps.push_back({ });
		std::queue <size_t> q;
		q.push(v);
		while (!q.empty()) {
			size_t u = q.front();
			q.pop();
			if (vis[u]) continue;
			vis[u] = true;
			comps.back().push_back(u);
			for (size_t x : instance.g()[u]) {
				q.push(x);
			}
		}
	}
	std::sort(comps.begin(), comps.end(), [&] (const std::vector <size_t>& lhs, const std::vector <size_t>& rhs) -> bool { return lhs.size() > rhs.size(); });
	for (size_t i = 1; i < comps.size(); i++) {
		G ng(comps[i].size());
		std::unordered_map <size_t, size_t> cc;
		for (size_t x : comps[i]) {
			cc[x] = cc.size();
		}
		for (size_t x : comps[i]) {
			for (size_t y : instance.g()[x]) {
				if (x < y) {
					ng.add(cc[x], cc[y]), assert(cc[x] < ng.n && cc[y] < ng.n);
				}
			}
		}
		Instance ni(ng);
		for (size_t x : comps[i]) {
			if (instance.W(x)) ni.insert_W(cc[x]);
			if (instance.D_nei(x)) ni.insert_W_Dnei(cc[x]);
			if (instance.X(x)) ni.insert_X(cc[x]);
		}
		m_metrics.add(m_finalize_callback(ni));
		for (size_t x : comps[i]) {
			// the component is fully solved, so transfer vertices in D and erase the rest
			if (ni.D(cc[x])) {
				if (instance.X(x)) instance.remove_X(x);
				instance.insert_D(x);
			} else {
				if (instance.alive(x)) instance.erase(x);
			}
		}
	}
}

void Reducer::m_remove_island_vertices(Instance& instance) {
	std::vector <size_t> to_del;
	for (size_t x : instance.alives()) {
		if (instance.g()[x].empty()) {
			to_del.push_back(x);
		}
	}
	for (size_t x : to_del) {
		if (!instance.W(x)) {
			assert(!instance.X(x));
			instance.insert_D(x);
		} else instance.erase(x);
		m_metrics.island_vertices_removed++;
	}
}

bool Reducer::m_peel_leaves(Instance& instance) {
	std::queue <size_t> q;
	for (size_t x : instance.alives()) {
		if (instance.g()[x].size() == 1) {
			q.push(x);
		}
	}
	bool removed_any = false;
	while (!q.empty()) {
		size_t v = q.front();
		q.pop();
		if (instance.g()[v].empty()) continue; // if the parent was removed as a leaf recently
		size_t par = *instance.g()[v].begin();
		if (instance.W(v)) {
			m_metrics.leaves_peeled++;
			removed_any = true;
			if (instance.X(v)) instance.erase(v);
			else if (instance.dom(par).size() == 1 && instance.X(par) && !instance.W(par)) instance.insert_D(v); // par must be dominated and v is the only dominator of par
			else instance.erase(v);
			if (instance.g()[par].size() == 1) {
				q.push(par);
			}
			continue;
		}
		if (instance.X(par)) { // in this case we cannot erase parent, so must erase v
			assert(!instance.W(v));
			assert(!instance.X(v)); // impossible to solve
			m_metrics.leaves_peeled++;
			removed_any = true;
			instance.insert_D(v);
			if (instance.g()[par].size() == 1) {
				q.push(par);
			}
			continue;
		}
		for (size_t x : instance.g()[par]) {
			if (instance.g()[x].size() == 2) { // x will be a leaf after erasing par
				q.push(x);
			}
			// current leaves attached to par (e.g. v) will become island vertices to be removed by the remove_island_vertices() rule
		}
		instance.insert_D(par);
		// count v as peeled in metrics, though
		m_metrics.leaves_peeled++;
		removed_any = true;
		instance.erase(v);
	}
	return removed_any;
}

bool Reducer::m_tri_tri_edge_removal(Instance& instance) {
	// locate two triangles A-B-C-A, D-E-F-D where A and D have degree 2
	// edges B-E, B-F, C-E, C-F can be erased
	// we don't need B or C to dominate E or F, at least one of E and F must be in dominating set due to D, and vice-versa
	// this only works if the isolated degree 2 vertices are black
	// find a set of all triangle vertices (except the degree 2 ones)
	// construct a set of all edges to protect (e.g. B-C edges from example above)
	// the rest of edges between some v,u in from the set can be removed
	std::unordered_set <size_t> tris;
	std::unordered_map <size_t, std::unordered_set <size_t>> protected_edges; // each vertex can be part of multiple triangles,
																			  // so there can be multiple vertices sharing a triangle with the given vertex
	for (size_t x : instance.alives()) {
		if (instance.g()[x].size() != 2) continue;
		if (instance.W(x)) continue;
		size_t v = *instance.g()[x].begin();
		size_t u = *std::next(instance.g()[x].begin());
		if (!instance.g()[v].contains(u)) continue;
		tris.insert(v);
		tris.insert(u);
		protected_edges[v].insert(u);
		protected_edges[u].insert(v);
	}
	std::vector <std::pair <size_t, size_t>> to_del;
	for (size_t v : tris) {
		for (size_t u : instance.g()[v]) {
			if (v > u) continue; // only need to check each edge from one direction
			if (protected_edges[v].contains(u)) continue;
			if (instance.g()[u].size() == 2) continue; // all relavent u's will have degree > 2, degree == 2 might be the third (isolated) vertex of the triangle
			if (!tris.contains(u)) continue; // assure u is a triangle vertex
			to_del.emplace_back(v, u);
		}
	}
	for (auto [v, u] : to_del) {
		instance.delete_edge(v, u);
		m_metrics.tri_tri_edges_removed++;
	}
	return !to_del.empty();
}

bool Reducer::m_white_white_edge_removal(Instance& instance) {
	// if some v,u in W are connected, the edge is redundant and can be removed
	std::vector <std::pair <size_t, size_t>> to_del;
	for (size_t v : instance.alives()) {
		if (!instance.D_nei(v)) continue;
		for (size_t u: instance.g()[v]) {
			if (v > u) continue; // only need to check each edge from one direction
			if (!instance.D_nei(u)) continue;
			to_del.emplace_back(v, u);
		}
	}
	for (auto [v, u] : to_del) {
		instance.delete_edge(v, u);
		m_metrics.white_white_edges_removed++;
	}
	return !to_del.empty();
}

bool Reducer::m_W_X_vertex_removal(Instance& instance) {
	// if some v is in W and X, remove it
	std::vector <size_t> to_del;
	for (size_t v : instance.alives()) {
		if (instance.D_nei(v) && instance.X(v)) to_del.push_back(v);
	}
	for (size_t v : to_del) {
		instance.erase(v);
		m_metrics.W_X_vertices_removed++;
	}
	return !to_del.empty();
}

bool Reducer::m_W_nh_vertex_removal(Instance& instance) {
	std::vector <size_t> to_rem;
	for (size_t x : instance.alives()) {
		if (!instance.W(x)) continue;
		bool good = true;
		for (size_t y : instance.g()[x]) good &= instance.W(y);
		if (good) to_rem.push_back(x), assert(instance.D_nei(x));
	}
	for (size_t x : to_rem) {
		instance.erase(x);
		m_metrics.W_Wnh_vertices_removed++;
	}
	return false;
}

bool Reducer::m_all_W(Instance& instance) {
	bool all_W = true;
	for (size_t v : instance.alives()) all_W &= instance.W(v);
	if (all_W) {
		while (!instance.alives().empty()) instance.erase(*instance.alives().begin()), m_metrics.all_W_removals++;
	}
	return all_W;
}
