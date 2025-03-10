#include "count_dominated.h"

#include <set>

int count_dominated(const G& g) {
	std::vector <std::set <int>> neighborhood(g.n);
	for (int v = 0; v < g.n; v++) {
		for (int x : g[v]) {
			neighborhood[v].insert(x);
		}
		neighborhood[v].insert(v);
	}
	std::vector <bool> is_dominated(g.n, false);
	for (int v = 0; v < g.n; v++) {
		for (int u : g[v]) {
			if (is_dominated[u]) continue;
			if (g[u].size() > g[v].size()) continue;
			bool dominates = true;
			for (int x : g[u]) {
				if (!neighborhood[v].contains(x)) {
					dominates = false;
					break;
				}
			}
			is_dominated[u] = dominates;
		}
	}
	int ans = 0;
	for (int i = 0; i < g.n; i++) {
		ans += is_dominated[i];
	}
	return ans;
}
