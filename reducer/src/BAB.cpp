#include "BAB.h"

#include "lower_bound.h"
#include "reduce.h"

#include <limits>
#include <cassert>

Solution BAB::solve(Instance instance, Solution best_solution) {
	reduce(instance);
	// reduce (should also prune lone white components)
	// ...
	/// TEMP BEGIN
	//{
	//	std::vector <size_t> to_del;
	//	for (size_t v : instance.alives()) {
	//		if (!instance.W(v)) continue;
	//		bool all_nei_W = true;
	//		for (size_t nei : instance.g()[v]) {
	//			all_nei_W &= instance.W(nei);
	//		}
	//		if (!all_nei_W) continue;
	//		to_del.push_back(v);
	//	}
	//	for (size_t v : to_del) instance.erase(v);
	//}
	/// TEMP END
	if (instance.alives().empty()) {
		// instance is fully reduced, if it encapsulates a superior solution, return that instead
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
		instance.insert_X(u); // we have the best solution when u in D, so the remaining ones must have u nin D
	}
	return best_solution;
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
