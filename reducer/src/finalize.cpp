#include "finalize.h"

#include "lp.h"

#include <unordered_map>
#include <cassert>
// #include <iostream> /// TEMP

void finalize(Instance& instance) {
	// std::cerr << "DEBUG: finalizing " << instance.alives().size() << " vertices" << std::endl;
	std::unordered_map <size_t, size_t> cc, icc;
	for (size_t v : instance.alives()) {
		cc[v] = cc.size();
		icc[cc[v]] = v;
	}
	LP::Expression obj_fun;
	std::vector <LP::Expression> conditions;
	for (size_t v : instance.alives()) {
		obj_fun.terms.push_back(LP::Term { .variable = cc[v], .coefficient = 1 });
		if (instance.W(v)) continue;
		LP::Expression condition;
		condition.terms.push_back(LP::Term { .variable = cc[v], .coefficient = 1 });
		for (size_t nei : instance.g()[v]) {
			condition.terms.push_back(LP::Term { .variable = cc[nei], .coefficient = 1 });
		}
		conditions.push_back(condition);
	}
	if (conditions.empty()) {
		instance.clear_adjusting_callbacks();
		return;
	}
	LP lp(LP::Objective_sense::MINIMIZE, cc.size(), obj_fun, conditions);
	for (size_t i : lp.solve()) {
		instance.insert_D(icc[i]);
	}
	instance.clear_adjusting_callbacks();
	// now remove the remaining (white) vertices
	std::vector <size_t> to_rem;
	for (size_t v : instance.alives()) {
		to_rem.push_back(v);
		assert(instance.W(v));
	}
	for (size_t v : to_rem) {
		instance.erase(v);
	}
}
