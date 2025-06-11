#include "BAB.h"

#include "reduce.h"
#include "lower_bound.h"
#include "finalize.h"

#include <limits>
#include <cassert>

void BAB::solve(Instance& instance) {
	// reduce (should also prune lone white components)
	auto finalize_callback = [this] (Instance& inst) -> Metrics {
		Solution initial_sol(inst.g().n);
		for (szt v = 0; v < inst.g().n; v++) if (inst.D(v) || (contains(inst.alives(), v) && !inst.X(v))) initial_sol.insert(v);
		BAB bab(initial_sol);
		auto t = inst.get_checkpoint();
		bab.solve(inst);
		Solution sol = bab.solution();
		inst.restore(t);
		for (szt v = 0; v < sol.n(); v++) if (sol.in(v) && !inst.D(v)) {
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
	// 	std::vector <szt> to_del;
	// 	for (szt v : instance.alives()) {
	// 		if (!instance.W(v)) continue;
	// 		bool all_nei_W = true;
	// 		for (szt nei : instance.g()[v]) {
	// 			all_nei_W &= instance.W(nei);
	// 		}
	// 		if (!all_nei_W) continue;
	// 		to_del.push_back(v);
	// 	}
	// 	for (szt v : to_del) instance.erase(v);
	// }
	// // /// TEMP END
	if (!instance.alives().empty() && instance.alives().size() < 400 && instance.alives().size() > 200) {
		finalize(instance);
	}
	if (instance.alives().empty()) {
		// instance is fully reduced, if it encapsulates a superior solution, return that instead
		instance.clear_adjusting_callbacks();
		if (instance.D_size() < m_best_solution.size()) m_best_solution = Solution(m_best_solution.n(), instance);
		return;
	}
	if (m_best_solution.size() <= lower_bound(instance)) {
		// current instance cannot be solved more minimally than current best_solution
		return;
	}
	// select a branching vertex
	szt bv = m_branch_vertex(instance);
	std::vector <szt> bv_dom(instance.dom(bv).begin(), instance.dom(bv).end());
	std::sort(bv_dom.begin(), bv_dom.end(), [&] (szt lhs, szt rhs) { return instance.cov(lhs).size() > instance.cov(rhs).size(); });
	for (szt u : bv_dom) {
		auto t = instance.get_checkpoint();
		instance.insert_D(u);
		solve(instance);
		instance.restore(t);
		// we have the best solution when u in D, so the remaining ones must have u nin D
		// first check if no solution exists after inserting u into X
		// if u isn't allowed in X, return; the best solution for u nin X has already been explored
		if (instance.dom(u).size() == 1 && !instance.W(u)) return;
		// also check that none of the (closed) neighbors of u become impossible to dominate
		for (szt x : instance.cov(u)) {
			ASSERT(!instance.W(x));
			if (instance.dom(x).size() == 1) return;
		}
		instance.insert_X(u);
	}
}

const Metrics& BAB::metrics() const {
	return m_metrics;
}

szt BAB::m_branch_vertex(const Instance& instance) const {
	szt min_dom = std::numeric_limits <szt>::max();
	for (szt v : instance.alives()) {
		if (instance.W(v)) continue;
		min_dom = std::min(min_dom, (szt) instance.dom(v).size());
	}
	std::vector <std::pair <szt, szt>> candidates;
	for (szt v : instance.alives()) {
		if (instance.W(v)) continue;
		if (instance.dom(v).size() != min_dom) continue;
		candidates.emplace_back(v, 0);
	}
	ASSERT(!candidates.empty());
	if (candidates.size() == 1) return candidates[0].first;
	for (auto& [v, c] : candidates) {
		for (szt nei : instance.dom(v)) {
			c += instance.cov(nei).size();
		}
	}
	for (szt i = 1; i < candidates.size(); i++) {
		if (candidates[i].second > candidates[0].second) {
			std::swap(candidates[i], candidates[0]);
		}
	}
	return candidates[0].first;
}

const Solution& BAB::solution() const {
	return m_best_solution;
}
