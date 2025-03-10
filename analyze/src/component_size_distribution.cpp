#include "component_size_distribution.h"

#include <queue>

std::map <int, int> component_size_distribution(const G& g) {
	std::vector <bool> vis(g.n, false);
	std::queue <int> q;
	q.push(0);
	int cur = 0;
	int nxt = 0;
	std::map <int, int> res;
	while (!q.empty()) {
		int v = q.front();
		q.pop();
		if (!vis[v]) {
			vis[v] = true;
			cur++;
			for (int u : g[v]) {
				q.push(u);
			}
		}
		if (q.empty()) {
			res[cur]++;
			cur = 0;
			for (; nxt < g.n; nxt++) {
				if (vis[nxt]) continue;
				q.push(nxt);
				break;
			}
		}
	}
	res[cur]++;
	res.erase(0);
	return res;
}
