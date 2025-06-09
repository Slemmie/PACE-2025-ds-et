#include "line_distribution.h"

#include <vector>
#include <numeric>
#include <unordered_set>

std::map <int, int> line_distribution(const G& g) {
	std::vector <int> p(g.n); std::iota(p.begin(), p.end(), 0);
	std::vector <int> s(g.n, 1);
	auto find = [&] (auto&& self, int v) -> int { return v == p[v] ? v : p[v] = self(self, p[v]); };
	auto unite = [&] (int u, int v) -> void {
		if ((u = find(find, u)) == (v = find(find, v))) return;
		s[u] += s[v];
		p[v] = u;
	};
	std::vector <std::unordered_set <int>> adj(g.n);
	for (int v = 0; v < g.n; v++) adj[v].insert(g[v].begin(), g[v].end());
	for (int v = 0; v < g.n; v++) {
		if (g[v].size() != 2) continue;
		if (adj[g[v][0]].contains(g[v][1])) continue;
		if (g[g[v][0]].size() <= 2) unite(v, g[v][0]);
		if (g[g[v][1]].size() <= 2) unite(v, g[v][1]);
	}
	std::map <int, int> res;
	for (int v = 0; v < g.n; v++) {
		if (find(find, v) == v) {
			res[s[v]]++;
		}
	}
	return res;
}
