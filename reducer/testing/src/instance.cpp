#include "instance.h"

#include <stdexcept>
#include <format>
#include <sstream>
#include <unordered_map>

Instance::Instance(const G& g) :
m_g(g),
m_W(g.n, false),
m_D(g.n, false)
{
	for (size_t v = 0; v < g.n; v++) {
		m_alives.insert(v);
	}
}

size_t Instance::current_checkpoint() const {
	return m_history.size();
}

void Instance::rollback(size_t checkpoint) {
	while (m_history.size() > checkpoint) {
		switch (m_history.back().type) {
			case History_item::Type::W_UPDATE: {
				m_W[m_history.back().vertex] = false;
			}
			case History_item::Type::D_UPDATE: {
				m_D[m_history.back().vertex] = false;
			}
			case History_item::Type::VERTEX_ERASE_UPDATE: {
				m_alives.insert(m_history.back().vertex);
				for (auto [u, v] : m_history.back().edges) {
					m_g.add(u, v);
				}
			}
			case History_item::Type::VERTEX_INSERT_UPDATE: {
				m_g.pop_vertex();
				m_alives.erase(m_history.back().vertex);
				m_W.pop_back();
				m_D.pop_back();
			}
			case History_item::Type::EDGE_DELETE_UPDATE: {
				m_g.add(m_history.back().edges[0].first, m_history.back().edges[0].second);
			}
			case History_item::Type::EDGE_ADD_UPDATE: {
				m_g.erase(m_history.back().edges[0].first, m_history.back().edges[0].second);
			}
		}
		m_history.pop_back();
	}
}

bool Instance::is_restored() const {
	return m_history.empty();
}

void Instance::restore() {
	rollback(0);
}

void Instance::insert_W(size_t v) {
	if (m_W[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into W, but {} already in W", v, v));
	}
	m_W[v] = true;
	m_history.push_back(History_item {
		.type = History_item::Type::W_UPDATE,
		.vertex = v
	});
}

void Instance::insert_D(size_t v) {
	if (m_D[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into D, but {} already in D", v, v));
	}
	m_D[v] = true;
	m_history.push_back(History_item {
		.type = History_item::Type::D_UPDATE,
		.vertex = v
	});
	for (size_t nei : m_g[v]) {
		if (!m_W[nei]) {
			insert_W(nei);
		}
	}
	erase(v);
}

void Instance::erase(size_t v) {
	if (!m_alives.contains(v)) {
		throw std::invalid_argument(std::format("attempt to erase {} from graph, but {} already erased", v, v));
	}
	m_alives.erase(v);
	m_history.push_back(History_item {
		.type = History_item::Type::VERTEX_ERASE_UPDATE,
		.vertex = v
	});
	std::vector <size_t> to_del;
	for (size_t nei : m_g[v]) {
		m_history.back().edges.emplace_back(v, nei);
		to_del.emplace_back(nei);
	}
	for (size_t nei : to_del) {
		m_g.erase(v, nei);
	}
}

size_t Instance::insert() {
	size_t index = m_g.add_vertex();
	m_history.push_back(History_item {
		.type = History_item::Type::VERTEX_INSERT_UPDATE,
		.vertex = index
	});
	m_alives.insert(index);
	m_W.push_back(false);
	m_D.push_back(false);
	return index;
}

void Instance::delete_edge(size_t u, size_t v) {
	if (u >= m_g.n) {
		throw std::invalid_argument(std::format("attempt to delete edge ({}, {}), but {} is out of bounds", u, v, u));
	}
	if (v >= m_g.n) {
		throw std::invalid_argument(std::format("attempt to delete edge ({}, {}), but {} is out of bounds", u, v, v));
	}
	if (!m_g[u].contains(v)) {
		throw std::invalid_argument(std::format("attempt to delete edge ({}, {}), it does not exist", u, v));
	}
	m_history.push_back(History_item {
		.type = History_item::Type::EDGE_DELETE_UPDATE,
		.edges = { { u, v } }
	});
	m_g.erase(u, v);
}

void Instance::add_edge(size_t u, size_t v) {
	if (u >= m_g.n) {
		throw std::invalid_argument(std::format("attempt to add edge ({}, {}), but {} is out of bounds", u, v, u));
	}
	if (v >= m_g.n) {
		throw std::invalid_argument(std::format("attempt to add edge ({}, {}), but {} is out of bounds", u, v, v));
	}
	if (m_g[u].contains(v)) {
		throw std::invalid_argument(std::format("attempt to add edge ({}, {}), it already not exist", u, v));
	}
	m_history.push_back(History_item {
		.type = History_item::Type::EDGE_ADD_UPDATE,
		.edges = { { u, v } }
	});
	m_g.add(u, v);
}

const G& Instance::g() const {
	return m_g;
}

bool Instance::alive(size_t v) const {
	return m_alives.contains(v);
}

const std::unordered_set <size_t>& Instance::alives() const {
	return m_alives;
}

bool Instance::W(size_t v) const {
	return m_W[v];
}

bool Instance::D(size_t v) const {
	return m_D[v];
}

std::string Instance::solution() const {
	std::ostringstream oss;
	std::vector <int> in_D;
	for (size_t v = 0; v < m_g.n; v++) {
		if (m_D[v]) {
			in_D.push_back(v);
		}
	}
	oss << in_D.size() << "\n";
	for (size_t v : in_D) {
		oss << v + 1 << "\n";
	}
	return oss.str();
}

std::string Instance::current_graph_string() const {
	std::ostringstream oss;
	size_t n = m_alives.size();
	std::unordered_map <size_t, size_t> coord_compr;
	for (size_t v : m_alives) {
		size_t now = coord_compr.size();
		coord_compr[v] = now;
	}
	std::vector <std::pair <size_t, size_t>> ed;
	for (size_t v : m_alives) {
		for (size_t u : m_g[v]) {
			if (v < u) {
				ed.emplace_back(coord_compr[v], coord_compr[u]);
			}
		}
	}
	size_t m = ed.size();
	oss << "p ds " << n << " " << m << "\n";
	for (auto [u, v] : ed) {
		oss << u + 1 << " " << v + 1 << "\n";
	}
	return oss.str();
}
