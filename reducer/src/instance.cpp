#include "instance.h"

#include <stdexcept>
#include <format>
#include <sstream>
#include <unordered_map>

Instance::Instance(const G& g) :
m_g(g),
m_W(g.n, false),
m_D(g.n, false),
m_X(g.n, false)
{
	for (size_t v = 0; v < g.n; v++) {
		m_alives.insert(v);
	}
}

void Instance::insert_W(size_t v) {
	if (m_W[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into W, but {} already in W", v, v));
	}
	m_W[v] = true;
}

void Instance::insert_D(size_t v) {
	if (m_D[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into D, but {} already in D", v, v));
	}
	if (m_X[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into D, but {} is in X", v, v));
	}
	m_D[v] = true;
	for (size_t nei : m_g[v]) {
		if (!m_W[nei]) {
			insert_W(nei);
		}
	}
	erase(v);
}

void Instance::insert_X(size_t v) {
	if (m_X[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into X, but {} already in X", v, v));
	}
	if (m_D[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into X, but {} is in D", v, v));
	}
	m_X[v] = true;
}

void Instance::insert_dead_into_D(size_t v) {
	if (m_alives.find(v) != m_alives.end()) {
		throw std::invalid_argument(std::format("attempt to insert (alive) {} into D using Instace::insert_dead_into_D()", v));
	}
	if (m_D[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into D, but {} already in D", v, v));
	}
	m_D[v] = true;
	for (size_t nei : m_g[v]) {
		if (!m_W[nei]) {
			insert_W(nei);
		}
	}
}

void Instance::remove_from_D(size_t v) {
	if (!m_D[v]) {
		throw std::invalid_argument(std::format("attempt to remove {} from D, but {} not in D", v, v));
	}
	m_D[v] = false;
}

void Instance::erase(size_t v) {
	if (!m_alives.contains(v)) {
		throw std::invalid_argument(std::format("attempt to erase {} from graph, but {} already erased", v, v));
	}
	m_alives.erase(v);
	std::vector <size_t> to_del;
	for (size_t nei : m_g[v]) {
		to_del.emplace_back(nei);
	}
	for (size_t nei : to_del) {
		m_g.erase(v, nei);
	}
}

size_t Instance::insert() {
	size_t index = m_g.add_vertex();
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
		throw std::invalid_argument(std::format("attempt to add edge ({}, {}), it already exists", u, v));
	}
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

bool Instance::X(size_t v) const {
	return m_X[v];
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

void Instance::add_adjusting_callback(std::function <void (Instance&)> fn) {
	m_adjusting_callbacks.push_back(fn);
}

void Instance::clear_adjusting_callbacks() {
	while (!m_adjusting_callbacks.empty()) {
		m_adjusting_callbacks.back()(*this);
		m_adjusting_callbacks.pop_back();
	}
}
