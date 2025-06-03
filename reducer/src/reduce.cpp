// #include "reduce.h"
//
// #include <sstream>
// #include <queue>
// #include <cstdint>
// #include <unordered_map>
// #include <algorithm>
// #include <random>
// #include <cassert> /// TEMP
//
// struct Metrics {
// 	size_t rule1_steps = 0;
// 	size_t rule1_deletions = 0;
//
// 	size_t rule2_steps = 0;
// 	size_t rule2_deletions = 0;
// 	size_t rule2_additions_vertices = 0;
// 	size_t rule2_additions_edges = 0;
//
// 	std::vector <size_t> articulation_point_reductions;
//
// 	void log(bool include_stderr = false) {
// 		std::ostringstream oss;
//
// 		oss << "c rule 1 steps: " << rule1_steps << ", deleted " << rule1_deletions << " total vertices\n";
// 		oss << "c rule 2 steps: " << rule2_steps << ", deleted " << rule2_deletions << " total vertices (added "
// 			<< rule2_additions_vertices << " vertices and " << rule2_additions_edges << " edges)\n";
// 		oss << "c articulation point reductions: ";
// 		for (size_t i = 0; i < articulation_point_reductions.size(); i++) {
// 			oss << articulation_point_reductions[i];
// 			if (i + 1 == articulation_point_reductions.size()) oss << "\n";
// 			else oss << ", ";
// 		}
// 		if (articulation_point_reductions.empty()) {
// 			oss << "none\n";
// 		}
//
// 		std::cout << oss.str();
// 		if (include_stderr) {
// 			std::cerr << oss.str();
// 		}
// 	}
// };
//
// static Metrics metrics;
//
// // context that is renewed at each step of reduce()
// struct Context {
// 	std::vector <std::pair <size_t, size_t>> d3;
// };
//
// static bool rule1_step(Instance& instance);
// static bool rule2_step(Instance& instance, const Context& context);
// static bool articulation_point_rule_step(Instance& instance, size_t art_rec_left);
// static void branch_by_disconnected_components(Instance& instance, size_t art_rec_left); // does not reduce the largest one
//
// // static std::vector <std::pair <size_t, size_t>> d3;
// // static std::vector <std::vector <size_t>> dist;
// // static size_t temp = 0;
//
// void reduce(Instance& instance, bool root_rec, size_t art_rec_left) {
// 	// dist.resize(instance.g().n, std::vector <size_t> (instance.g().n, 1ULL << 60));
// 	// for (size_t i = 0; i < instance.g().n; i++) dist[i][i] = 0;
// 	// for (size_t i = 0; i < instance.g().n; i++)
// 	// 	for (size_t j : instance.g()[i])
// 	// 		dist[i][j] = dist[j][i] = 1;
// 	// for (size_t k = 0; k < instance.g().n; k++)
// 	// 	for (size_t i = 0; i < instance.g().n; i++)
// 	// 		for (size_t j = 0; j < instance.g().n; j++)
// 	// 			dist[i][j] = std::min(dist[i][j], dist[i][k] + dist[k][j]);
//
// 	if (instance.alives().empty()) return;
//
// 	branch_by_disconnected_components(instance, art_rec_left); // the largest one is reduced below, the rest have been branched by and will not be reduced further
//
// 	Context context;
// 	for (size_t v : instance.alives()) {
// 		std::queue <std::pair <size_t, uint8_t>> q;
// 		q.emplace(v, 0);
// 		std::unordered_set <size_t> vis;
// 		while (!q.empty()) {
// 			auto [u, d] = q.front();
// 			q.pop();
// 			if (vis.contains(u)) continue;
// 			if (d > 3) continue;
// 			context.d3.emplace_back(v, u);
// 			vis.insert(u);
// 			for (size_t x : instance.g()[u]) {
// 				if (x < v) {
// 					q.emplace(x, d + 1);
// 				}
// 			}
// 		}
// 	}
//
// 	bool stop = false;
// 	while (!stop) {
// 		stop = true;
// 		while (rule1_step(instance)) metrics.rule1_steps++, stop = false;
// 		while (rule2_step(instance, context)) metrics.rule2_steps++, stop = false;
// 	}
//
// 	if (art_rec_left > 0 && articulation_point_rule_step(instance, art_rec_left)) {
// 		std::cerr << "DEBUG: recursing with #alives = " << instance.alives().size() << std::endl;
// 		reduce(instance, false, art_rec_left);
// 	}
//
// 	if (root_rec) {
// 		// std::cerr << "temp = " << temp << std::endl;
//
// 		metrics.log(true);
// 	}
// }
//
// static bool rule1_step(Instance& instance) {
// 	for (size_t v : instance.alives()) {
// 		std::unordered_set <size_t> N_exit, N_guard, N_prison;
// 		for (size_t u : instance.g()[v]) {
// 			bool is_exit = false;
// 			for (size_t w : instance.g()[u]) {
// 				if (!instance.g()[v].contains(w) && w != v) {
// 					is_exit = true;
// 					break;
// 				}
// 			}
// 			if (is_exit) {
// 				N_exit.insert(u);
// 			}
// 		}
// 		for (size_t u : instance.g()[v]) {
// 			if (N_exit.contains(u)) continue;
// 			bool is_guard = false;
// 			for (size_t w : instance.g()[u]) {
// 				if (N_exit.contains(w)) {
// 					is_guard = true;
// 					break;
// 				}
// 			}
// 			if (is_guard) {
// 				N_guard.insert(u);
// 			}
// 		}
// 		for (size_t u : instance.g()[v]) {
// 			if (!N_exit.contains(u) && !N_guard.contains(u)) {
// 				N_prison.insert(u);
// 			}
// 		}
// 		bool candidate = false;
// 		for (size_t u : N_prison) {
// 			if (!instance.W(u)) {
// 				candidate = true;
// 				break;
// 			}
// 		}
// 		if (!candidate) continue;
// 		std::vector <size_t> to_del;
// 		for (size_t u : instance.g()[v]) {
// 			if (N_guard.contains(u) || N_prison.contains(u)) {
// 				to_del.push_back(u);
// 			}
// 		}
// 		instance.insert_D(v);
// 		metrics.rule1_deletions++;
// 		for (size_t u : to_del) {
// 			instance.erase(u);
// 			metrics.rule1_deletions++;
// 		}
// 		return true;
// 	}
// 	return false;
// }
//
// static bool rule2_step(Instance& instance, const Context& context) {
// 	for (auto [v, w] : context.d3) {
// 		// temp++;
// 		std::unordered_set <size_t> N_exit, N_guard, N_prison;
// 		std::unordered_set <size_t> N_vw;
// 		for (size_t u : instance.g()[v]) {
// 			N_vw.insert(u);
// 		}
// 		for (size_t u : instance.g()[w]) {
// 			N_vw.insert(u);
// 		}
// 		for (size_t u : N_vw) {
// 			bool is_exit = false;
// 			for (size_t x : instance.g()[u]) {
// 				if (!N_vw.contains(x) && x != v && x != w) {
// 					is_exit = true;
// 					break;
// 				}
// 			}
// 			if (is_exit) {
// 				N_exit.insert(u);
// 			}
// 		}
// 		for (size_t u : N_vw) {
// 			if (N_exit.contains(u)) continue;
// 			bool is_guard = false;
// 			for (size_t x : instance.g()[u]) {
// 				if (N_exit.contains(x)) {
// 					is_guard = true;
// 					break;
// 				}
// 			}
// 			if (is_guard) {
// 				N_guard.insert(u);
// 			}
// 		}
// 		for (size_t u : N_vw) {
// 			if (!N_exit.contains(u) && !N_guard.contains(u)) {
// 				N_prison.insert(u);
// 			}
// 		}
// 		bool candidate = false;
// 		for (size_t u : N_prison) {
// 			if (!instance.W(u)) {
// 				candidate = true;
// 				break;
// 			}
// 		}
// 		for (size_t u : N_guard) {
// 			if (!candidate) break;
// 			bool dominates = true;
// 			for (size_t x : N_prison) {
// 				if (instance.W(x)) continue;
// 				dominates &= instance.g()[u].contains(x) || u == x;
// 			}
// 			candidate &= !dominates;
// 		}
// 		for (size_t u : N_prison) {
// 			if (!candidate) break;
// 			bool dominates = true;
// 			for (size_t x : N_prison) {
// 				if (instance.W(x)) continue;
// 				dominates &= instance.g()[u].contains(x) || u == x;
// 			}
// 			candidate &= !dominates;
// 		}
// 		if (!candidate) continue;
// 		bool v_dominates = true, w_dominates = true;
// 		for (size_t u : N_prison) {
// 			if (instance.W(u)) continue;
// 			v_dominates &= instance.g()[v].contains(u);
// 			w_dominates &= instance.g()[w].contains(u);
// 		}
// 		if (v_dominates || w_dominates) {
// 			if (v_dominates && w_dominates) {
// 				std::vector <size_t> to_del;
// 				for (size_t u : N_prison) {
// 					to_del.push_back(u);
// 				}
// 				for (size_t u : N_guard) {
// 					if (instance.g()[v].contains(u) && instance.g()[w].contains(u)) {
// 						to_del.push_back(u);
// 					}
// 				}
// 				for (size_t u : to_del) {
// 					instance.erase(u);
// 					metrics.rule2_deletions++;
// 				}
// 				size_t z1 = instance.insert();
// 				size_t z2 = instance.insert();
// 				metrics.rule2_additions_vertices += 2;
// 				instance.insert_W(z1);
// 				instance.insert_W(z2);
// 				instance.add_edge(v, z1);
// 				instance.add_edge(w, z1);
// 				instance.add_edge(v, z2);
// 				instance.add_edge(w, z2);
// 				metrics.rule2_additions_edges += 4;
// 			} else if (v_dominates) {
// 				std::vector <size_t> to_del;
// 				for (size_t u : N_prison) {
// 					to_del.push_back(u);
// 				}
// 				for (size_t u : N_guard) {
// 					if (instance.g()[v].contains(u)) {
// 						to_del.push_back(u);
// 					}
// 				}
// 				instance.insert_D(v);
// 				metrics.rule2_deletions++;
// 				for (size_t u : to_del) {
// 					instance.erase(u);
// 					metrics.rule2_deletions++;
// 				}
// 			} else if (w_dominates) {
// 				std::vector <size_t> to_del;
// 				for (size_t u : N_prison) {
// 					to_del.push_back(u);
// 				}
// 				for (size_t u : N_guard) {
// 					if (instance.g()[w].contains(u)) {
// 						to_del.push_back(u);
// 					}
// 				}
// 				instance.insert_D(w);
// 				metrics.rule2_deletions++;
// 				for (size_t u : to_del) {
// 					instance.erase(u);
// 					metrics.rule2_deletions++;
// 				}
// 			} else {
// 				throw std::runtime_error("unexpected error");
// 			}
// 		} else {
// 			std::vector <size_t> to_del;
// 			for (size_t u : N_guard) {
// 				if (u != v && u != w) {
// 					to_del.push_back(u);
// 				}
// 			}
// 			for (size_t u : N_prison) {
// 				if (u != v && u != w) {
// 					to_del.push_back(u);
// 				}
// 			}
// 			instance.insert_D(v);
// 			instance.insert_D(w);
// 			metrics.rule2_deletions += 2;
// 			for (size_t u : to_del) {
// 				instance.erase(u);
// 				metrics.rule2_deletions++;
// 			}
// 		}
// 		return true;
// 	}
// 	return false;
// }
//
// #include "finalize.h"
//
// bool articulation_point_rule_step(Instance& instance, size_t art_rec_left) {
// 	auto articulation_points = [] (Instance& inst) -> std::vector <size_t> {
// 		std::vector <bool> vis(inst.g().n, false);
// 		std::vector <size_t> time(inst.g().n, ~static_cast <size_t> (0)), furthest(inst.g().n, ~static_cast <size_t> (0));
// 		size_t timer = 0;
// 		std::vector <size_t> result;
// 		auto dfs = [&] (auto&& self, size_t v, size_t p = ~static_cast <size_t> (0)) -> void {
// 			vis[v] = true;
// 			time[v] = furthest[v] = timer++;
// 			size_t chc = 0;
// 			for (size_t u : inst.g()[v]) {
// 				if (u == p) continue;
// 				if (vis[u]) furthest[v] = std::min(furthest[v], time[u]);
// 				else {
// 					self(self, u, v);
// 					furthest[v] = std::min(furthest[v], furthest[u]);
// 					if (furthest[u] >= time[v] && p != ~static_cast <size_t> (0)) result.push_back(v);
// 					chc++;
// 				}
// 			}
// 			if (p == ~static_cast <size_t> (0) && chc > 1) result.push_back(v);
// 		};
// 		for (size_t x : inst.alives()) {
// 			if (!vis[x]) {
// 				dfs(dfs, x);
// 			}
// 		}
// 		return result;
// 	};
// 	auto go = [&] (Instance& inst) -> size_t {
// 		std::cerr << "DEBUG: calling reduce() from g() with inst.alives.size() = " << inst.alives().size() << std::endl;
// 		reduce(inst, false, art_rec_left - 1);
// 		finalize(inst);
// 		size_t cost = 0;
// 		for (size_t i = 0; i < inst.g().n; i++) {
// 			cost += inst.D(i);
// 		}
// 		return cost;
// 	};
// 	// try to find an articulation point that isolates a component of useful size
// 	bool did_find = false;
// 	auto arts = articulation_points(instance);
// 	std::random_device rd;
// 	std::mt19937 mtgen(rd());
// 	std::shuffle(arts.begin(), arts.end(), mtgen);
// 	for (size_t v : arts) {
// 		if (did_find) break; // only process one v successfully at a time to not break anything
// 		// look for a component of size at least 2 and size at most 200
// 		// NOTE for later: switch up the 200 constant a bit to see what we can get away with, also add a time limit when solving it to avoid evil size-200 components ruining the pace
// 		std::vector <std::vector <size_t>> comps;
// 		std::vector <bool> vis(instance.g().n, false);
// 		vis[v] = true;
// 		for (size_t u : instance.g()[v]) {
// 			if (vis[u]) continue;
// 			comps.push_back({ });
// 			std::queue <size_t> q;
// 			q.push(u);
// 			while (!q.empty()) {
// 				size_t x = q.front();
// 				q.pop();
// 				if (vis[x]) continue;
// 				vis[x] = true;
// 				comps.back().push_back(x);
// 				for (size_t y : instance.g()[x]) {
// 					q.push(y);
// 				}
// 			}
// 		}
// 		for (auto comp : comps) {
// 			if (comp.size() < 2 || comp.size() > 250) continue;
// 			// two cases, either v is in B or in W
// 			did_find = true;
// 			metrics.articulation_point_reductions.push_back(comp.size());
// 			if (!instance.W(v)) { // v is not already dominated
// 				// first check if we can just put v in D immediately (i.e. if cost_v_off > cost_v_on
// 				// if not, then cost_v_off == cost_v_on and we must decide whether to also dominate v internally
// 				// we only dominate v interally if it doesn't increase the cost,
// 				//  - if it does increase the cost and the outside prefers we dominate it, then just pay to include v in D instead, this is solved for later
// 				G ng_wov(comp.size());
// 				std::unordered_map <size_t, size_t> cc;
// 				for (size_t x : comp) {
// 					cc[x] = cc.size();
// 				}
// 				assert(cc.count(comp[0]) && cc[comp[0]] == 0);
// 				assert(cc.size() == comp.size());
// 				for (size_t x : comp) {
// 					for (size_t y : instance.g()[x]) {
// 						if (y == v) continue; // skip the boundary vertex
// 						if (x < y) ng_wov.add(cc[x], cc[y]), assert(cc[x] < ng_wov.n && cc[y] < ng_wov.n);
// 					}
// 				}
// 				Instance ni_vninD(ng_wov);
// 				for (size_t x : comp) {
// 					if (instance.W(x)) {
// 						ni_vninD.insert_W(cc[x]);
// 					}
// 				}
// 				Instance ni_vinD = ni_vninD;
// 				size_t cost_v_off = go(ni_vninD);
// 				for (size_t x : instance.g()[v]) {
// 					if (cc.find(x) != cc.end() && !ni_vinD.W(cc[x])) {
// 						ni_vinD.insert_W(cc[x]);
// 					}
// 				}
// 				size_t cost_v_on = go(ni_vinD);
// 				if (cost_v_off > cost_v_on) { // insert v in D, leaving v out of D is never a strictly better choice
// 					instance.insert_D(v);
// 					// pick solution where v in D
// 					for (size_t i = 0; i < comp.size(); i++) {
// 						if (ni_vinD.D(i)) {
// 							assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
// 							instance.insert_D(comp[i]);
// 						}
// 					}
// 				} else { // need to decide whether to dominate v internally
// 					assert(cost_v_off == cost_v_on); // sanity check that the cost with v off is not cheaper than with v on
// 					G ng_wv(comp.size() + 1);
// 					cc[v] = cc.size();
// 					for (size_t x : comp) {
// 						for (size_t y : instance.g()[x]) {
// 							if (x < y) ng_wv.add(cc[x], cc[y]), assert(cc[x] < ng_wv.n && cc[y] < ng_wv.n); // this time also add edges to v
// 						}
// 					}
// 					Instance ni_vninD_v_dominated_internally(ng_wv);
// 					size_t cost_v_off_v_dominated_internally = go(ni_vninD_v_dominated_internally);
// 					if (cost_v_off == cost_v_off_v_dominated_internally) { // might as well dominate v internally
// 						assert(!instance.W(v));
// 						instance.insert_W(v);
// 						for (size_t i = 0; i < comp.size(); i++) {
// 							if (ni_vninD_v_dominated_internally.D(i)) {
// 								assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
// 								instance.insert_D(comp[i]);
// 							}
// 						}
// 					} else { // it costs extra to dominate v internally, so leave it be
// 						for (size_t i = 0; i < comp.size(); i++) {
// 							if (ni_vninD.D(i)) {
// 								assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
// 								instance.insert_D(comp[i]);
// 							}
// 						}
// 					}
// 				}
// 			} else { // v is dominated
// 				// either insert v into D or leave v dominated externally, do not try to dominate it internally, we don't care
// 				G ng_wov(comp.size());
// 				std::unordered_map <size_t, size_t> cc;
// 				for (size_t x : comp) {
// 					cc[x] = cc.size();
// 				}
// 				for (size_t x : comp) {
// 					for (size_t y : instance.g()[x]) {
// 						if (y == v) continue; // skip the boundary vertex
// 						if (x < y) ng_wov.add(cc[x], cc[y]), assert(cc[x] < ng_wov.n && cc[y] < ng_wov.n);
// 					}
// 				}
// 				Instance ni_vninD(ng_wov);
// 				for (size_t x : comp) {
// 					if (instance.W(x)) {
// 						ni_vninD.insert_W(cc[x]);
// 					}
// 				}
// 				Instance ni_vinD = ni_vninD;
// 				size_t cost_v_off = go(ni_vninD);
// 				for (size_t x : instance.g()[v]) {
// 					if (cc.find(x) != cc.end() && !ni_vinD.W(cc[x])) {
// 						ni_vinD.insert_W(cc[x]);
// 					}
// 				}
// 				size_t cost_v_on = go(ni_vinD);
// 				if (cost_v_off > cost_v_on) { // might as well put v in D
// 					instance.insert_D(v);
// 					// pick solution where v in D
// 					for (size_t i = 0; i < comp.size(); i++) {
// 						if (ni_vinD.D(i)) {
// 							assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
// 							instance.insert_D(comp[i]);
// 						}
// 					}
// 				} else { // cost with v in D = cost with v not in D ->
// 						 // v is already dominated, so pick solution that doesn't need v in D, this is cheaper if v never enters D in the future
// 					assert(cost_v_off == cost_v_on); // sanity check that the cost with v off is not cheaper than with v on
// 					for (size_t i = 0; i < comp.size(); i++) {
// 						if (ni_vninD.D(i)) {
// 							assert(i == cc[comp[i]]); // sanity check that i in new instance maps to comp[i] in original instance
// 							instance.insert_D(comp[i]);
// 						}
// 					}
// 				}
// 			}
// 			// at this point, for sure all vertices in comp are accounted for and should not change state, so remove those that are in W (which haven't been removed)
// 			for (size_t x : comp) {
// 				if (instance.alive(x)) {
// 					assert(instance.W(x));
// 					instance.erase(x);
// 				}
// 			}
// 			std::cerr << "DEBUG: #alives after articulation point reduction with comp size " << comp.size() << " is " << instance.alives().size() << std::endl;
// 			// if v has been inserted into D at this point, immediately recurse, the new disconnected components of proper size will be accounted for immediately anyway
// 			if (instance.D(v)) break;
// 		}
// 	}
// 	return did_find;
// }
//
// void branch_by_disconnected_components(Instance& instance, size_t art_rec_left) {
// 	std::vector <std::vector <size_t>> comps;
// 	std::vector <bool> vis(instance.g().n, false);
// 	for (size_t v : instance.alives()) {
// 		if (vis[v]) continue;
// 		comps.push_back({ });
// 		std::queue <size_t> q;
// 		q.push(v);
// 		while (!q.empty()) {
// 			size_t u = q.front();
// 			q.pop();
// 			if (vis[u]) continue;
// 			vis[u] = true;
// 			comps.back().push_back(u);
// 			for (size_t x : instance.g()[u]) {
// 				q.push(x);
// 			}
// 		}
// 	}
// 	std::sort(comps.begin(), comps.end(), [&] (const std::vector <size_t>& lhs, const std::vector <size_t>& rhs) -> bool { return lhs.size() > rhs.size(); });
// 	for (size_t i = 1; i < comps.size(); i++) {
// 		G ng(comps[i].size());
// 		std::unordered_map <size_t, size_t> cc;
// 		for (size_t x : comps[i]) {
// 			cc[x] = cc.size();
// 		}
// 		for (size_t x : comps[i]) {
// 			for (size_t y : instance.g()[x]) {
// 				if (x < y) {
// 					ng.add(cc[x], cc[y]), assert(cc[x] < ng.n && cc[y] < ng.n);
// 				}
// 			}
// 		}
// 		Instance ni(ng);
// 		for (size_t x : comps[i]) {
// 			if (instance.W(x)) ni.insert_W(cc[x]);
// 		}
// 		reduce(ni, false, art_rec_left);
// 		for (size_t x : comps[i]) {
// 			if (ni.D(cc[x])) instance.insert_D(x);
// 			else if (ni.W(cc[x])) {
// 				if (!instance.W(x)) instance.insert_W(x);
// 			} else if (!ni.alive(cc[x])) {
// 				if (instance.alive(x)) instance.erase(x);
// 			}
// 		}
// 	}
// }
