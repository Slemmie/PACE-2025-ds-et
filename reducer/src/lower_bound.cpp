#include "lower_bound.h"

#include "rlp.h"

#include <cmath>

size_t lower_bound(const Instance& instance) {
	std::unordered_map <size_t, size_t> cc, icc;
	for (size_t v : instance.alives()) {
		if (instance.X(v)) continue;
		cc[v] = cc.size();
		icc[cc[v]] = v;
	}
	RLP::Expression obj_fun;
	std::vector <RLP::Expression> conditions;
	for (size_t v : instance.alives()) {
		if (!instance.X(v)) obj_fun.terms.push_back(RLP::Term { .variable = cc[v], .coefficient = 1 });
		if (instance.W(v)) continue;
		RLP::Expression condition;
		if (!instance.X(v)) condition.terms.push_back(RLP::Term { .variable = cc[v], .coefficient = 1 });
		for (size_t nei : instance.g()[v]) {
			if (instance.X(nei)) continue;
			condition.terms.push_back(RLP::Term { .variable = cc[nei], .coefficient = 1 });
		}
		if (condition.terms.empty()) continue;
		conditions.push_back(condition);
	}
	if (conditions.empty()) return 0;
	RLP rlp(RLP::Objective_sense::MINIMIZE, cc.size(), obj_fun, conditions);
	size_t result = std::round(rlp.solve()); // actual lower bound is ceil(rlp.solve()), but use std::round to combat precision errors
	return result + instance.D_size();
}
