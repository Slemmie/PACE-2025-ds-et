#include "count_true_twins.h"

#include <set>

int count_true_twins(const G& g) {
	std::vector <std::set <int>> neighborhood(g.n);
	for (int v = 0; v < g.n; v++) {
		for (int x : g[v]) {
			neighborhood[v].insert(x);
		}
		neighborhood[v].insert(v);
	}
	int ans = 0;
	for (int v = 0; v < g.n; v++) {
		for (int u : g[v]) {
			ans += neighborhood[v].size() == neighborhood[u].size() && neighborhood[u] == neighborhood[v];
		}
	}
	return ans / 2;
}
