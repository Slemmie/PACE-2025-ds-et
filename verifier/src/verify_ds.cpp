#include "verify_ds.h"

#include <vector>
#include <iostream>

bool verify_ds(const Instance& instance, const Solution& solution, bool verbose) {
	std::vector <bool> covered(instance.n, false);

	for (size_t i = 0; i < solution.size; i++) {
		if (solution.vertices[i] >= instance.n) {
			std::cout << "solution contains vertex with invalid index (" << solution.vertices[i] + 1 << ", but n = " << instance.n << ")" << std::endl;
			return false;
		}

		covered[solution.vertices[i]] = true;

		for (size_t nei : instance.adj[solution.vertices[i]]) {
			covered[nei] = true;
		}
	}

	size_t cnt_not_covered = 0;
	for (bool b : covered) {
		cnt_not_covered += !b;
	}

	if (cnt_not_covered) {
		if (verbose) {
			std::cout << cnt_not_covered << " vertex(ices) are not covered (of " << instance.n << " total vertex(ices))" << std::endl;
		}
		return false;
	}

	return true;
}
