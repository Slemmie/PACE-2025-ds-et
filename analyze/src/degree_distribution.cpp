#include "component_size_distribution.h"

std::map <int, int> degree_distribution(const G& g) {
	std::map <int, int> res;
	for (int v = 0; v < g.n; v++) {
		res[g[v].size()]++;
	}
	return res;
}
