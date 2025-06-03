#include "instance.h"
#include "solve.h"

#include <iostream>

int main() {
	Solver solver;
	Instance instance(G::read(std::cin));
	solver.solve(instance);
	solver.metrics().log(false);
	std::cout << instance.solution();
}
