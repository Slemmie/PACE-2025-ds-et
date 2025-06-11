#include "lower_bound.h"

#include "rlp.h"

#include <cmath>

szt lower_bound(const Instance& instance) {
	hash_map <szt, szt> cc, icc;
	for (szt v : instance.alives()) {
		if (instance.X(v)) continue;
		cc[v] = cc.size();
		icc[cc[v]] = v;
	}
	RLP::Expression obj_fun;
	std::vector <RLP::Expression> conditions;
	for (szt v : instance.alives()) {
		if (!instance.X(v)) obj_fun.terms.push_back(RLP::Term { .variable = cc[v], .coefficient = 1 });
		if (instance.W(v)) continue;
		RLP::Expression condition;
		if (!instance.X(v)) condition.terms.push_back(RLP::Term { .variable = cc[v], .coefficient = 1 });
		for (szt nei : instance.g()[v]) {
			if (instance.X(nei)) continue;
			condition.terms.push_back(RLP::Term { .variable = cc[nei], .coefficient = 1 });
		}
		if (condition.terms.empty()) continue;
		conditions.push_back(condition);
	}
	if (conditions.empty()) return 0;
	// if (conditions.size() > 800) return 0;
	RLP rlp(RLP::Objective_sense::MINIMIZE, cc.size(), obj_fun, conditions);
	szt result = std::ceil(rlp.solve() - 1e-4);
	return result + instance.D_size();
}
