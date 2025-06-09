#include "biconnected_component_size_distribution.h"

#include <vector>
#include <numeric>

std::map <int, int> biconnected_component_size_distribution(const G& g) {
	std::vector <int> p(g.n); std::iota(p.begin(), p.end(), 0);
	std::vector <int> s(g.n, 1);
	auto find = [&] (auto&& self, int v) -> int { return v == p[v] ? v : p[v] = self(self, p[v]); };
	auto unite = [&] (int u, int v) -> void {
		if ((u = find(find, u)) == (v = find(find, v))) return;
		s[u] += s[v];
		p[v] = u;
	};
	std::vector <bool> vis(g.n, false), art(g.n, false);
	std::vector <int> tim(g.n, -1), low(g.n, -1);
	int t = 0;
	auto dfs = [&] (auto&& self, int v, int par = -1) -> void {
		vis[v] = true;
		tim[v] = low[v] = t++;
		int ch = 0;
		for (int u : g[v]) {
			if (u == par) continue;
			if (vis[u]) {
				low[v] = std::min(low[v], tim[u]);
				continue;
				self(self, u, v);
				low[v] = std::min(low[v], low[u]);
				if (low[u] >= tim[v] && par != -1) art[v] = true;
				ch++;
			}
		}
		if (par == -1 && ch > 1) art[v] = true;
	};
	for (int v = 0; v < g.n; v++) if (!vis[v]) dfs(dfs, v);
	for (int v = 0; v < g.n; v++) {
		if (art[v]) continue;
		for (int u : g[v]) {
			if (art[u]) continue;
			unite(v, u);
		}
	}
	std::map <int, int> res;
	for (int v = 0; v < g.n; v++) {
		if (art[v]) continue;
		if (find(find, v) != v) continue;
		res[s[v]]++;
	}
	return res;
}
