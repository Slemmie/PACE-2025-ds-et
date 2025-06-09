#include "BAB.h"

#include "reduce.h"
#include "lower_bound.h"

#include <limits>
#include <cassert>

Solution BAB::solve(Instance instance, Solution best_solution) {
	// reduce (should also prune lone white components)
	auto finalize_callback = [this] (Instance& inst) -> Metrics {
		Solution initial_sol(inst.g().n);
		for (size_t v = 0; v < inst.g().n; v++) if (inst.D(v) || (inst.alives().contains(v) && !inst.X(v))) initial_sol.insert(v);
		BAB bab;
		Solution sol = bab.solve(inst, initial_sol);
		for (size_t v = 0; v < sol.n(); v++) if (sol.in(v) && !inst.D(v)) {
			if (inst.X(v)) inst.remove_X(v);
			inst.insert_D(v);
		}
		inst.clear_adjusting_callbacks();
		return bab.metrics();
	};
	Reducer reducer(finalize_callback);
	reducer.reduce(instance);
	m_metrics.add(reducer.metrics());
	// // /// TEMP BEGIN
	// {
	// 	std::vector <size_t> to_del;
	// 	for (size_t v : instance.alives()) {
	// 		if (!instance.W(v)) continue;
	// 		bool all_nei_W = true;
	// 		for (size_t nei : instance.g()[v]) {
	// 			all_nei_W &= instance.W(nei);
	// 		}
	// 		if (!all_nei_W) continue;
	// 		to_del.push_back(v);
	// 	}
	// 	for (size_t v : to_del) instance.erase(v);
	// }
	// // /// TEMP END
	if (instance.alives().empty()) {
		// instance is fully reduced, if it encapsulates a superior solution, return that instead
		instance.clear_adjusting_callbacks();
		return instance.D_size() < best_solution.size() ? Solution(best_solution.n(), instance) : best_solution;
	}
	if (best_solution.size() <= lower_bound(instance)) {
		// current instance cannot be solved more minimally than current best_solution
		return best_solution;
	}
	// select a branching vertex
	size_t bv = m_branch_vertex(instance);
	std::vector <size_t> bv_dom(instance.dom(bv).begin(), instance.dom(bv).end());
	std::sort(bv_dom.begin(), bv_dom.end(), [&] (size_t lhs, size_t rhs) { return instance.cov(lhs).size() > instance.cov(rhs).size(); });
	for (size_t u : bv_dom) {
		Instance instance_u = instance;
		instance_u.insert_D(u);
		Solution solution_u = solve(instance_u, best_solution);
		if (solution_u.size() < best_solution.size()) {
			best_solution = solution_u;
		}
		// we have the best solution when u in D, so the remaining ones must have u nin D
		// first check if no solution exists after inserting u into X
		// if u isn't allowed in X, return; the best solution for u nin X has already been explored
		if (instance.dom(u).size() == 1 && !instance.W(u)) return best_solution;
		// also check that none of the (closed) neighbors of u become impossible to dominate
		for (size_t x : instance.cov(u)) {
			assert(!instance.W(x));
			if (instance.dom(x).size() == 1) return best_solution;
		}
		instance.insert_X(u);
	}
	return best_solution;
}

const Metrics& BAB::metrics() const {
	return m_metrics;
}

size_t BAB::m_branch_vertex(const Instance& instance) const {
	size_t min_dom = std::numeric_limits <size_t>::max();
	for (size_t v : instance.alives()) {
		if (instance.W(v)) continue;
		min_dom = std::min(min_dom, instance.dom(v).size());
	}
	std::vector <std::pair <size_t, size_t>> candidates;
	for (size_t v : instance.alives()) {
		if (instance.W(v)) continue;
		if (instance.dom(v).size() != min_dom) continue;
		candidates.emplace_back(v, 0);
	}
	assert(!candidates.empty());
	if (candidates.size() == 1) return candidates[0].first;
	for (auto& [v, c] : candidates) {
		for (size_t nei : instance.dom(v)) {
			c += instance.cov(nei).size();
		}
	}
	for (size_t i = 1; i < candidates.size(); i++) {
		if (candidates[i].second > candidates[0].second) {
			std::swap(candidates[i], candidates[0]);
		}
	}
	return candidates[0].first;
}
