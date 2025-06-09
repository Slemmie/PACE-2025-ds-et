#include "graph.h"

#include <sstream>
#include <format>
#include <stdexcept>

G::G(size_t _n, size_t _m) : n(_n), m(_m), adj(_n) { }

void G::add(size_t u, size_t v) {
	if (u == v) {
		throw std::invalid_argument(std::format("attempt to insert self-loop ({}, {}) in graph", u, v));
	}
	if (adj[u].contains(v)) {
		throw std::invalid_argument(std::format("attempt to insert edge ({}, {}) in graph, but it already exists", u, v));
	}
	adj[u].insert(v);
	adj[v].insert(u);
	m++;
}

void G::erase(size_t u, size_t v) {
	if (!adj[u].contains(v)) {
		throw std::invalid_argument(std::format("attempt to erase edge ({}, {}) from graph, but it doesn't exist", u, v));
	}
	adj[u].erase(v);
	adj[v].erase(u);
	m--;
}

size_t G::add_vertex() {
	size_t index = n;
	n++;
	adj.resize(n);
	return index;
}

void G::pop_vertex() {
	if (!adj.back().empty()) {
		throw std::invalid_argument("attempt to delete a vertex with non-zero degree from instance of G");
	}
	n--;
	adj.pop_back();
}

const std::unordered_set <size_t>& G::operator [] (size_t index) const {
	if (index >= n) {
		throw std::invalid_argument(std::format("attempt to access the neighbor set of {}, but graph only contains {} vertices", index, n));
	}
	return adj[index];
}

std::unordered_set <size_t>& G::operator [] (size_t index) {
	if (index >= n) {
		throw std::invalid_argument(std::format("attempt to access the neighbor set of {}, but graph only contains {} vertices", index, n));
	}
	return adj[index];
}

std::ostream& operator << (std::ostream& os, const G& g) {
	os << "graph, n = " << g.n << "\n";
	for (size_t i = 0; i < g.n; i++) {
		os << "\t" << i << ": {";
		for (size_t x : g[i]) os << " " << x;
		os << " }\n";
	}
	return os;
}

G G::read(std::istream& inf) {
	size_t n, m;
	std::vector <std::pair <size_t, size_t>> ed;
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
				size_t u, v;
				std::istringstream edge_stream(line);
				if (edge_stream >> u >> v) {
					if (u != v)
						ed.emplace_back(--u, --v);
				} else {
					throw std::invalid_argument(std::format("failed to interpret line: '{}'", line));
				}
			}
		}
	}
	if (m != ed.size()) {
		throw std::invalid_argument(std::format("input graph has m = {} but {} edges were found", m, ed.size()));
	}
	G g(n, 0);
	for (auto [u, v] : ed) {
		g.add(u, v);
	}
	return g;
}

G G::read(const std::string& data) {
	std::istringstream iss(data);
	return read(iss);
}
