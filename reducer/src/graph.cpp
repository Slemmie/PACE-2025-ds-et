#include "graph.h"

#include <sstream>
#include <format>
#include <stdexcept>

G::G(szt _n, szt _m) : n(_n), m(_m), adj(_n) { }

void G::add(szt u, szt v) {
	if (u == v) {
		throw std::invalid_argument(std::format("attempt to insert self-loop ({}, {}) in graph", u, v));
	}
	if (contains(adj[u], v)) {
		throw std::invalid_argument(std::format("attempt to insert edge ({}, {}) in graph, but it already exists", u, v));
	}
	adj[u].insert(v);
	adj[v].insert(u);
	m++;
}

void G::erase(szt u, szt v) {
	if (!contains(adj[u], v)) {
		throw std::invalid_argument(std::format("attempt to erase edge ({}, {}) from graph, but it doesn't exist", u, v));
	}
	adj[u].erase(v);
	adj[v].erase(u);
	m--;
}

szt G::add_vertex() {
	szt index = n;
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

const hash_set <szt>& G::operator [] (szt index) const {
	if (index >= n) {
		throw std::invalid_argument(std::format("attempt to access the neighbor set of {}, but graph only contains {} vertices", index, n));
	}
	return adj[index];
}

hash_set <szt>& G::operator [] (szt index) {
	if (index >= n) {
		throw std::invalid_argument(std::format("attempt to access the neighbor set of {}, but graph only contains {} vertices", index, n));
	}
	return adj[index];
}

std::ostream& operator << (std::ostream& os, const G& g) {
	os << "graph, n = " << g.n << "\n";
	for (szt i = 0; i < g.n; i++) {
		os << "\t" << i << ": {";
		for (szt x : g[i]) os << " " << x;
		os << " }\n";
	}
	return os;
}

G G::read(std::istream& inf) {
	szt n, m;
	std::vector <std::pair <szt, szt>> ed;
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
				szt u, v;
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
