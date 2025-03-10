#include "count_components.h"

#include <queue>

int count_components(const G& g) {
	std::vector <bool> vis(g.n, false);
	std::queue <int> q;
	q.push(0);
	int ans = 1;
	int nxt = 0;
	while (!q.empty()) {
		int v = q.front();
		q.pop();
		if (!vis[v]) {
			vis[v] = true;
			for (int u : g[v]) {
				q.push(u);
			}
		}
		if (q.empty()) {
			for (; nxt < g.n; nxt++) {
				if (vis[nxt]) continue;
				q.push(nxt);
				ans++;
				break;
			}
		}
	}
	return ans;
}
