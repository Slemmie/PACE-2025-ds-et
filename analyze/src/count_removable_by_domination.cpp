#include "count_removable_by_domination.h"

#include <set>

int count_removable_by_domination(const G& g) {
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
	int ans = 0;
	for (int v = 0; v < g.n; v++) {
		for (int u : g[v]) {
			if (!dominates[u].contains(v)) continue;
			bool removable = true;
			for (int w : g[v]) {
				if (u == w) continue;
				removable &= dominates[u].contains(w);
				if (!removable) break;
			}
			if (removable) {
				ans++;
				break;
			}
		}
	}
	return ans;
}
