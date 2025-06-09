#pragma once

#include "instance.h"
#include "solution.h"
#include "metrics.h"

class BAB {

public:

	BAB() = default;

	Solution solve(Instance& instance, Solution best_solution);

	const Metrics& metrics() const;

private:

	Metrics m_metrics;

private:

	size_t m_branch_vertex(const Instance& instance) const;

};
