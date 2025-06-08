#include "instance.h"

#include <stdexcept>
#include <format>
#include <sstream>
#include <unordered_map>

Instance::Instance(const G& g) :
m_g(g),
m_W(g.n, false),
m_D_size(0),
m_D(g.n, false),
m_X(g.n, false),
m_doms(g.n),
m_covs(g.n)
{
	for (size_t v = 0; v < g.n; v++) {
		m_alives.insert(v);
		m_undetermined.insert(v);
		m_undominated.insert(v);
		m_doms[v].insert(v);
		m_covs[v].insert(v);
		for (size_t nei : g[v]) {
			m_doms[v].insert(nei);
			m_covs[v].insert(nei);
		}
		m_nX.insert(v);
	}
}

void Instance::insert_W(size_t v) {
	if (m_W[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into W, but {} already in W", v, v));
	}
	m_undominated.erase(v);
	m_W[v] = true;
	m_covs[v].erase(v);
	for (size_t nei : m_g[v]) {
		m_covs[nei].erase(v);
	}
}

void Instance::insert_D(size_t v) {
	if (m_D[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into D, but {} already in D", v, v));
	}
	if (m_X[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into D, but {} is in X", v, v));
	}
	m_undetermined.erase(v);
	m_undominated.erase(v);
	m_D_size++;
	m_D[v] = true;
	for (size_t nei : m_g[v]) {
		m_undominated.erase(nei);
		if (!m_W[nei]) {
			insert_W(nei);
		}
	}
	m_nX.erase(v);
	erase(v);
}

void Instance::insert_X(size_t v) {
	if (m_X[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into X, but {} already in X", v, v));
	}
	if (m_D[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into X, but {} is in D", v, v));
	}
	m_nX.erase(v);
	m_undetermined.erase(v);
	m_X[v] = true;
	m_doms[v].erase(v);
	for (size_t nei : m_g[v]) {
		m_doms[nei].erase(v);
	}
}

void Instance::erase(size_t v) {
	if (!m_alives.contains(v)) {
		throw std::invalid_argument(std::format("attempt to erase {} from graph, but {} already erased", v, v));
	}
	m_undetermined.erase(v);
	m_undominated.erase(v);
	m_nX.erase(v);
	m_alives.erase(v);
	std::vector <size_t> to_del;
	for (size_t nei : m_g[v]) {
		m_doms[nei].erase(v);
		m_covs[nei].erase(v);
		to_del.emplace_back(nei);
	}
	for (size_t nei : to_del) {
		m_g.erase(v, nei);
	}
}

size_t Instance::insert() {
	size_t index = m_g.add_vertex();
	m_alives.insert(index);
	m_undetermined.insert(index);
	m_undominated.insert(index);
	m_W.push_back(false);
	m_D.push_back(false);
	m_X.push_back(false);
	m_doms.push_back({ index });
	m_covs.push_back({ index });
	m_nX.insert(index);
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
	m_doms[u].erase(v);
	m_covs[u].erase(v);
	m_doms[v].erase(u);
	m_covs[v].erase(u);
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
	if (!m_X[v]) m_doms[u].insert(v);
	if (!m_X[u]) m_doms[v].insert(u);
	if (!m_D[v] && !m_W[v]) m_covs[u].insert(v);
	if (!m_D[u] && !m_W[u]) m_covs[v].insert(u);
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

const std::unordered_set <size_t>& Instance::undetermined() const {
	return m_undetermined;
}

const std::unordered_set <size_t>& Instance::undominated() const {
	return m_undominated;
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

const std::unordered_set <size_t>& Instance::dom(size_t v) const {
	return m_doms[v];
}

const std::unordered_set <size_t>& Instance::cov(size_t v) const {
	return m_covs[v];
}

const std::unordered_set <size_t>& Instance::nX() const {
	return m_nX;
}

size_t Instance::D_size() const {
	return m_D_size;
}

bool Instance::can_insert_X(size_t v) const {
	for (size_t nei : m_g[v]) {
		if (m_doms[nei].size() == 1 && *m_doms[nei].begin() == v && !m_W[nei]) return false;
	}
	if (m_doms[v].size() == 1 && !m_W[v]) return false;
	return true;
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
