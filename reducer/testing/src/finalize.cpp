#include "finalize.h"

#include "lp.h"

void finalize(Instance& instance) {
	LP::Expression obj_fun;
	std::vector <LP::Expression> conditions;
	for (size_t v : instance.alives()) {
		obj_fun.terms.push_back(LP::Term { .variable = v, .coefficient = 1 });
		if (instance.W(v)) continue;
		LP::Expression condition;
		condition.terms.push_back(LP::Term { .variable = v, .coefficient = 1 });
		for (size_t nei : instance.g()[v]) {
			condition.terms.push_back(LP::Term { .variable = nei, .coefficient = 1 });
		}
		conditions.push_back(condition);
	}
	if (conditions.empty()) return;
	LP lp(LP::Objective_sense::MINIMIZE, instance.g().n, obj_fun, conditions);
	for (size_t i : lp.solve()) {
		instance.insert_D(i);
	}
}
