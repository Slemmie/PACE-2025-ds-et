#include "count_OFFable_by_domination.h"

#include <set>
#include <cassert>

int count_OFFable_by_domination(const G& g) {
	std::vector <std::set <int>> neighborhood(g.n);
	for (int v = 0; v < g.n; v++) {
		for (int x : g[v]) {
			neighborhood[v].insert(x);
		}
		neighborhood[v].insert(v);
	}
	std::vector <std::set <int>> dominates(g.n);
	for (int v = 0; v < g.n; v++) {
		for (int u : g[v]) {
			if (g[u].size() > g[v].size()) continue;
			bool dominates_flag = true;
			for (int x : g[u]) {
				if (!neighborhood[v].contains(x)) {
					dominates_flag = false;
					break;
				}
			}
			if (dominates_flag) {
				dominates[v].insert(u);
			}
		}
	}
	std::vector <bool> removed(g.n, false);
	for (int v = 0; v < g.n; v++) {
		if (removed[v]) continue;
		for (int u : g[v]) {
			if (removed[u]) continue;
			if (dominates[v].contains(u) && dominates[u].contains(v)) {
				removed[u] = true;
			}
		}
	}
	for (int v = 0; v < g.n; v++) {
		if (removed[v]) continue;
		for (int u : g[v]) {
			if (removed[u]) continue;
			if (dominates[u].contains(v)) {
				assert(neighborhood[u].size() > neighborhood[v].size());
				removed[v] = true;
				break;
			}
		}
	}
	int ans = 0;
	for (bool b : removed) {
		ans += b;
	}
	return ans;
}
