#include "finalize.h"

#include "lp.h"

#include <unordered_map>
#include <cassert>
// #include <iostream> /// TEMP

void finalize(Instance& instance) {
	// std::cerr << "DEBUG: finalizing " << instance.alives().size() << " vertices" << std::endl;
	hash_map <szt, szt> cc, icc;
	// reserve(cc, instance.nX().size());
	// reserve(icc, instance.nX().size());
	for (szt v : instance.alives()) {
		if (instance.X(v)) continue;
		cc[v] = cc.size();
		icc[cc[v]] = v;
	}
	LP::Expression obj_fun;
	std::vector <LP::Expression> conditions;
	for (szt v : instance.alives()) {
		if (!instance.X(v)) obj_fun.terms.push_back(LP::Term { .variable = cc[v], .coefficient = 1 });
		if (instance.W(v)) continue;
		LP::Expression condition;
		if (!instance.X(v)) condition.terms.push_back(LP::Term { .variable = cc[v], .coefficient = 1 });
		for (szt nei : instance.g()[v]) {
			if (instance.X(nei)) continue;
			condition.terms.push_back(LP::Term { .variable = cc[nei], .coefficient = 1 });
		}
		if (condition.terms.empty()) continue;
		conditions.push_back(condition);
	}
	if (conditions.empty()) {
		instance.clear_adjusting_callbacks();
		return;
	}
	LP lp(LP::Objective_sense::MINIMIZE, cc.size(), obj_fun, conditions);
	for (szt i : lp.solve()) {
		instance.insert_D(icc[i]);
	}
	instance.clear_adjusting_callbacks();
	// now remove the remaining (white) vertices
	std::vector <szt> to_rem;
	for (szt v : instance.alives()) {
		to_rem.push_back(v);
		ASSERT(instance.W(v));
	}
	for (szt v : to_rem) {
		instance.erase(v);
	}
}
