#include "instance.h"
#include "solve.h"
#include "vertexcover.h"

#include <iostream>

int main() {
	Solver solver;
	Instance instance(G::read(std::cin));

	if(!vertex_cover_solution(instance)) {
		solver.solve(instance);
		solver.metrics().log(false);
	}

	std::cout << instance.solution();
}
