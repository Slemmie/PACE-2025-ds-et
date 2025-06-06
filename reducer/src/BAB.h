#pragma once

#include "instance.h"
#include "solution.h"

class BAB {

public:

	BAB() = default;

	Solution solve(Instance instance, Solution best_solution);

private:

	size_t m_branch_vertex(const Instance& instance) const;

};
