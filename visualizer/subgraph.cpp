#include <bits/stdc++.h>

struct G {
	int n, m;
	std::vector <std::vector <int>> adj;
	G(int _n, int _m = -1);
	void add(int u, int v);
	const std::vector <int>& operator [] (size_t index) const;
	std::vector <int>& operator [] (size_t index);
	friend std::ostream& operator << (std::ostream& os, const G& g);
	static G read(std::istream& inf);
	static G read(const std::string& data);
};

std::vector <int> csz(const G& g) {
	std::vector <int> p(g.n); std::iota(p.begin(), p.end(), 0);
	std::vector <int> s(g.n, 1);
	auto find = [&] (auto&& self, int v) -> int { return v == p[v] ? v : (p[v] = self(self, p[v])); };
	auto unite = [&] (int u, int v) -> void {
		if ((u = find(find, u)) == (v = find(find, v))) return;
		s[v] += s[u]; p[u] = v;
	};
	for (int v = 0; v < g.n; v++) for (int x : g[v]) unite(v, x);
	std::vector <int> res(g.n);
	for (int v = 0; v < g.n; v++) res[v] = s[find(find, v)];
	return res;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "usage: " << argv[0] << " <num vertices> < graph.gr > new_graph.gr" << std::endl;
		return 1;
	}

	int want = std::stoi(argv[1]);

	G g = G::read(std::cin);

	srand(std::chrono::duration_cast <std::chrono::nanoseconds> (std::chrono::steady_clock::now().time_since_epoch()).count());

	std::vector <bool> vis(g.n, false);
	std::vector <int> use;
	std::queue <int> q;

	auto siz = csz(g);
	std::vector <int> al(g.n);
	std::iota(al.begin(), al.end(), 0);
	int start = -1;
	for (int v : al) if (siz[v] >= want && (start == -1 || rand() % 2)) start = v;
	if (start == -1) {
		for (int v : al) {
			if (start != -1 && siz[v] == siz[start] && rand() % 2) start = v;
			if (start == -1 || siz[v] > siz[start]) start = v;
		}
	}
	assert(start != -1);
	q.push(start);

	while (!q.empty() && (int) use.size() < want) {
		int v = q.front();
		q.pop();
		if (vis[v]) continue;
		vis[v] = true;
		use.push_back(v);
		for (int x : g[v]) {
			q.push(x);
		}
	}

	std::vector <std::pair <int, int>> ed;
	for (int v : use) {
		assert(vis[v]);
		for (int x : g[v]) {
			if (!vis[x]) continue;
			if (v < x) {
				ed.emplace_back(v, x);
			}
		}
	}

	std::cerr << "#kept vertices: " << use.size() << std::endl;
	std::cerr << "#kept edges: " << ed.size() << std::endl;

	std::map <int, int> cc; int ccc = 0;
	for (int x : use) cc[x] = ccc++;
	for (auto& [u, v] : ed) u = cc[u], v = cc[v];
	for (int& x : use) x = cc[x];

	std::cout << "p ds " << use.size() << " " << ed.size() << "\n";
	for (auto [u, v] : ed) std::cout << u + 1 << " " << v + 1 << "\n";
}

G::G(int _n, int _m) : n(_n), m(_m), adj(_n) { }

void G::add(int u, int v) {
	adj[u].push_back(v);
	adj[v].push_back(u);
}

const std::vector <int>& G::operator [] (size_t index) const {
	return adj[index];
}

std::vector <int>& G::operator [] (size_t index) {
	return adj[index];
}

std::ostream& operator << (std::ostream& os, const G& g) {
	os << "graph, n = " << g.n << "\n";
	for (int i = 0; i < g.n; i++) {
		os << "\t" << i << ": {";
		for (int x : g[i]) os << " " << x;
		os << " }\n";
	}
	return os;
}

G G::read(std::istream& inf) {
	int n, m;
	std::vector <std::pair <int, int>> ed;
	{
		std::string line;
		while (std::getline(inf, line)) {
			if (line.starts_with('c')) continue;
			std::istringstream iss(line);
			std::string token;
			iss >> token;
			if (token == "p") {
				std::string dummy;
				iss >> dummy >> n >> m;
			} else {
				int u, v;
				std::istringstream edge_stream(line);
				if (edge_stream >> u >> v) {
					ed.emplace_back(--u, --v);
				} else {
					throw std::invalid_argument(std::format("failed to interpret line: '{}'", line));
				}
			}
		}
	}
	if (m != (int) ed.size()) {
		throw std::invalid_argument(std::format("input graph has m = {} but {} edges were found", m, ed.size()));
	}
	G g(n, m);
	for (auto [u, v] : ed) {
		g.add(u, v);
	}
	return g;
}

G G::read(const std::string& data) {
	std::istringstream iss(data);
	return read(iss);
}
