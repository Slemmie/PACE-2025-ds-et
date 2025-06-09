#include "graph.h"

#include <sstream>
#include <format>

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
			if (line.empty()) continue;
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
