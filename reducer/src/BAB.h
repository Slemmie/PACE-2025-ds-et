#pragma once

#include "instance.h"
#include "solution.h"
#include "metrics.h"

class BAB {

public:

	inline BAB(const Solution& initial_solution) : m_best_solution(initial_solution) { }

	void solve(Instance& instance);

	const Metrics& metrics() const;

	const Solution& solution() const;

private:

	Metrics m_metrics;

	Solution m_best_solution;

private:

	size_t m_branch_vertex(const Instance& instance) const;

};
