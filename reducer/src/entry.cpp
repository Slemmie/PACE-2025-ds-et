#include "instance.h"
#include "solution.h"
#include "BAB.h"
#include "vertexcover.h"

#include <iostream>

int main() {
	Instance instance(G::read(std::cin));
	if (!vertex_cover_solution(instance)) {
		Solution solution(instance.g().n);
		for (size_t i = 0; i < instance.g().n; i++) solution.insert(i);
		BAB bab(solution);
		bab.solve(instance);
		bab.metrics().log(false);
		std::cout << bab.solution().solution();
	} else {
		std::cout << instance.solution();
	}
}
