#include "instance.h"

#include <stdexcept>
#include <format>
#include <sstream>
#include <unordered_map>
#include <iostream>

Instance::Instance(const G& g) :
m_g(g),
m_g_orig(g),
m_W(g.n, false),
m_D_size(0),
m_D(g.n, false),
m_X(g.n, false),
m_doms(g.n),
m_covs(g.n),
m_D_nei(g.n, false)
{
	reserve(m_alives, g.n);
	reserve(m_undetermined, g.n);
	reserve(m_undominated, g.n);
	reserve(m_nX, g.n);
	for (szt v = 0; v < g.n; v++) {
		m_alives.insert(v);
		m_undetermined.insert(v);
		m_undominated.insert(v);
		// reserve(m_doms[v], g[v].size() + 1);
		// reserve(m_covs[v], g[v].size() + 1);
		m_doms[v].insert(v);
		m_covs[v].insert(v);
		for (szt nei : g[v]) {
			m_doms[v].insert(nei);
			m_covs[v].insert(nei);
		}
		m_nX.insert(v);
	}
	m_active_undetermined = m_undetermined;
	m_active_undominated = m_undominated;
}

void Instance::insert_W(szt v) {
	if (m_W[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into W, but {} already in W", v, v));
	}
	bool was_erased_from_undominated = contains(m_undominated, v);
	bool was_inserted_into_W = m_W[v] == false;
	std::vector <szt> erased_from_covs;

	m_undominated.erase(v);
	m_active_undominated.erase(v);
	m_W[v] = true;
	if (contains(m_covs[v], v)) {
		erased_from_covs.push_back(v);
		m_covs[v].erase(v);
		if(contains(m_undetermined, v)) m_active_undetermined.insert(v);
	}
	for (szt nei : m_g[v]) {
		if (contains(m_covs[nei], v)) {
			erased_from_covs.push_back(nei);
			m_covs[nei].erase(v);
			if(contains(m_undetermined, nei)) m_active_undetermined.insert(nei);
		}
	}

	auto undo = [v, was_erased_from_undominated, was_inserted_into_W, erased_from_covs] (Instance& inst) -> void {
		if (was_erased_from_undominated) {
			inst.m_undominated.insert(v);
			inst.m_active_undominated.insert(v);
		}
		if (was_inserted_into_W) inst.m_W[v] = false;
		for (szt x : erased_from_covs) {
			inst.m_covs[x].insert(v);
			if(contains(inst.m_undetermined, x)) inst.m_active_undetermined.insert(x);
		}
	};
	m_history.push_back(undo);
}

void Instance::insert_W_Dnei(szt v) {
	bool was_inserted_into_D_nei = m_D_nei[v] == false;

	m_D_nei[v] = true;

	auto undo = [v, was_inserted_into_D_nei] (Instance& inst) -> void {
		if (was_inserted_into_D_nei) inst.m_D_nei[v] = false;
	};
	m_history.push_back(undo);

	// history entry from insert_W(v) should be recovered first (added after ours), as its changed happenes after ours
	if (!m_W[v]) insert_W(v);
}

void Instance::insert_D(szt v) {
	if (m_D[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into D, but {} already in D", v, v));
	}
	if (m_X[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into D, but {} is in X", v, v));
	}
	bool was_erased_from_undetermined = contains(m_undetermined, v);
	std::vector <szt> erased_from_undominated;
	bool was_inserted_into_D = m_D[v] == false;
	std::vector <szt> inserted_into_D_nei;
	bool was_erased_from_nX = contains(m_nX, v);

	m_undetermined.erase(v);
	m_active_undetermined.erase(v);
	m_undominated.erase(v);
	m_active_undominated.erase(v);
	m_D_size++;
	m_D[v] = true;
	for (szt nei : m_g[v]) {
		if (contains(m_undominated, nei)) {
			erased_from_undominated.push_back(nei);
			m_undominated.erase(nei);
			m_active_undominated.erase(nei);
		}
	}
	for (szt nei : m_g_orig[v]) {
		if (m_D_nei[nei] == false) inserted_into_D_nei.push_back(nei);
		m_D_nei[nei] = true;
	}
	m_nX.erase(v);

	auto undo = [v, was_erased_from_undetermined, erased_from_undominated, was_inserted_into_D, inserted_into_D_nei, was_erased_from_nX] (Instance& inst) -> void {
		if (was_erased_from_undetermined) {
			inst.m_undetermined.insert(v);
			inst.m_active_undetermined.insert(v);
		}
		for (szt x : erased_from_undominated) {
			inst.m_undominated.insert(x);
			inst.m_active_undominated.insert(x);
		}
		inst.m_D_size--;
		if (was_inserted_into_D) inst.m_D[v] = false;
		for (szt x : inserted_into_D_nei) inst.m_D_nei[x] = false;
		if (was_erased_from_nX) inst.m_nX.insert(v);
	};
	m_history.push_back(undo);

	// history entries from insert_W(nei)s and erase(v) should be recovered first (added after ours), as its changes happened after ours
	for (szt nei : m_g[v]) {
		if (!m_W[nei]) {
			insert_W(nei);
		}
	}
	erase(v);
}

void Instance::insert_X(szt v) {
	if (m_X[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into X, but {} already in X", v, v));
	}
	if (m_D[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into X, but {} is in D", v, v));
	}
	bool was_erased_from_nX = contains(m_nX, v);
	bool was_erased_from_undetermined = contains(m_undetermined, v);
	bool was_inserted_into_X = m_X[v] == false;
	std::vector <szt> erased_from_doms;

	m_nX.erase(v);
	m_undetermined.erase(v);
	m_active_undetermined.erase(v);
	m_X[v] = true;
	if (contains(m_doms[v], v)) {
		erased_from_doms.push_back(v);
		m_doms[v].erase(v);
		if(contains(m_undominated, v)) m_active_undominated.insert(v);
	}
	for (szt nei : m_g[v]) {
		if (contains(m_doms[nei], v)) {
			erased_from_doms.push_back(nei);
			m_doms[nei].erase(v);
			if(contains(m_undominated, nei)) m_active_undominated.insert(nei);
		}
	}

	auto undo = [v, was_erased_from_nX, was_erased_from_undetermined, was_inserted_into_X, erased_from_doms] (Instance& inst) -> void {
		if (was_erased_from_nX) inst.m_nX.insert(v);
		if (was_erased_from_undetermined) {
			inst.m_undetermined.insert(v);
			inst.m_active_undetermined.insert(v);
		}
		if (was_inserted_into_X) inst.m_X[v] = false;
		for (szt x : erased_from_doms) {
			inst.m_doms[x].insert(v);
			if(contains(inst.m_undominated, x)) inst.m_active_undominated.insert(x);
		}
	};
	m_history.push_back(undo);
}

void Instance::remove_X(szt v) {
	if (!m_X[v]) {
		throw std::invalid_argument(std::format("attempt to remove {} from X, but {} is not in X", v, v));
	}
	bool was_inserted_into_nX = !contains(m_nX, v);
	bool was_inserted_into_undetermined = !contains(m_undetermined, v);
	bool was_erased_from_X = m_X[v] == true;
	std::vector <szt> inserted_into_doms;

	m_nX.insert(v);
	m_undetermined.insert(v);
	m_active_undetermined.insert(v);
	m_X[v] = false;
	if (!contains(m_doms[v], v)) {
		inserted_into_doms.push_back(v);
		m_doms[v].insert(v);
		if(contains(m_undominated, v)) m_active_undominated.insert(v);
	}
	for (szt nei : m_g[v]) {
		if (!contains(m_doms[nei], v)) {
			inserted_into_doms.push_back(nei);
			m_doms[nei].insert(v);
			if(contains(m_undominated, nei)) m_active_undominated.insert(nei);
		}
	}

	auto undo = [v, was_inserted_into_nX, was_inserted_into_undetermined, was_erased_from_X, inserted_into_doms] (Instance& inst) -> void {
		if (was_inserted_into_nX) inst.m_nX.erase(v);
		if (was_inserted_into_undetermined) {
			inst.m_undetermined.erase(v);
			inst.m_active_undetermined.erase(v);
		}
		if (was_erased_from_X) inst.m_X[v] = true;
		for (szt x : inserted_into_doms) {
			inst.m_doms[x].erase(v);
			if(contains(inst.m_undominated, x)) inst.m_active_undominated.insert(x);
		}
	};
	m_history.push_back(undo);
}

void Instance::erase(szt v) {
	if (!contains(m_alives, v)) {
		throw std::invalid_argument(std::format("attempt to erase {} from graph, but {} already erased", v, v));
	}
	bool was_erased_from_undetermined = contains(m_undetermined, v);
	bool was_erased_from_undominated = contains(m_undominated, v);
	bool was_erased_from_nX = contains(m_nX, v);
	bool was_erased_from_alives = contains(m_alives, v);
	std::vector <szt> erased_from_doms;
	std::vector <szt> erased_from_covs;
	std::vector <std::pair <szt, szt>> edges_erased;

	m_undetermined.erase(v);
	m_active_undetermined.erase(v);
	m_undominated.erase(v);
	m_active_undominated.erase(v);
	m_nX.erase(v);
	m_alives.erase(v);
	std::vector <szt> to_del;
	for (szt nei : m_g[v]) {
		if (contains(m_doms[nei], v)) {
			erased_from_doms.push_back(nei);
			m_doms[nei].erase(v);
			if(contains(m_undominated, nei)) m_active_undominated.insert(nei);
		}
		if (contains(m_covs[nei], v)) {
			erased_from_covs.push_back(nei);
			m_covs[nei].erase(v);
			if(contains(m_undetermined, nei)) m_active_undetermined.insert(nei);
		}
		to_del.emplace_back(nei);
	}
	for (szt nei : to_del) {
		edges_erased.emplace_back(v, nei);
		m_g.erase(v, nei);
	}

	auto undo = [
		v,
		was_erased_from_undetermined,
		was_erased_from_undominated,
		was_erased_from_nX,
		was_erased_from_alives,
		erased_from_doms,
		erased_from_covs,
		edges_erased
	] (Instance& inst) -> void {
		if (was_erased_from_undetermined) {
			inst.m_undetermined.insert(v);
			inst.m_active_undetermined.insert(v);
		}
		if (was_erased_from_undominated) {
			inst.m_undominated.insert(v);
			inst.m_active_undominated.insert(v);
		}
		if (was_erased_from_nX) inst.m_nX.insert(v);
		if (was_erased_from_alives) inst.m_alives.insert(v);
		for (szt x : erased_from_doms) {
			inst.m_doms[x].insert(v);
			if(contains(inst.m_undominated, x)) inst.m_active_undominated.insert(x);
		}
		for (szt x : erased_from_covs) {
			inst.m_covs[x].insert(v);
			if(contains(inst.m_undetermined, x)) inst.m_active_undetermined.insert(x);
		}
		for (auto [x, y] : edges_erased) inst.m_g.add(x, y);
	};
	m_history.push_back(undo);
}

szt Instance::insert() {
	szt index = m_g.add_vertex();
	m_g_orig.add_vertex();
	m_alives.insert(index);
	m_undetermined.insert(index);
	m_active_undetermined.insert(index);
	m_undominated.insert(index);
	m_active_undominated.insert(index);
	m_W.push_back(false);
	m_D.push_back(false);
	m_X.push_back(false);
	hash_set <szt> tmp; tmp.insert(index);
	m_doms.push_back(tmp);
	m_covs.push_back(tmp);
	m_nX.insert(index);
	m_D_nei.push_back(false);

	auto undo = [index] (Instance& inst) -> void {
		inst.m_g.pop_vertex();
		inst.m_g_orig.pop_vertex();
		inst.m_alives.erase(index);
		inst.m_undetermined.erase(index);
		inst.m_active_undetermined.erase(index);
		inst.m_undominated.erase(index);
		inst.m_active_undominated.erase(index);
		inst.m_W.pop_back();
		inst.m_D.pop_back();
		inst.m_X.pop_back();
		inst.m_doms.pop_back();
		inst.m_covs.pop_back();
		inst.m_nX.erase(index);
		inst.m_D_nei.pop_back();
	};
	m_history.push_back(undo);

	return index;
}

void Instance::delete_edge(szt u, szt v) {
	if (u >= m_g.n) {
		throw std::invalid_argument(std::format("attempt to delete edge ({}, {}), but {} is out of bounds", u, v, u));
	}
	if (v >= m_g.n) {
		throw std::invalid_argument(std::format("attempt to delete edge ({}, {}), but {} is out of bounds", u, v, v));
	}
	if (!contains(m_g[u], v)) {
		throw std::invalid_argument(std::format("attempt to delete edge ({}, {}), it does not exist", u, v));
	}

	std::vector <std::pair <szt, szt>> doms_erases;
	std::vector <std::pair <szt, szt>> covs_erases;

	m_g.erase(u, v);
	if (contains(m_doms[u], v)) {
		doms_erases.emplace_back(u, v);
		m_doms[u].erase(v);
		if(contains(m_undominated, u)) m_active_undominated.insert(u);
	}
	if (contains(m_covs[u], v)) {
		covs_erases.emplace_back(u, v);
		m_covs[u].erase(v);
		if(contains(m_undetermined, u)) m_active_undetermined.insert(u);
	}
	if (contains(m_doms[v], u)) {
		doms_erases.emplace_back(v, u);
		m_doms[v].erase(u);
		if(contains(m_undominated, v)) m_active_undominated.insert(v);
	}
	if (contains(m_covs[v], u)) {
		covs_erases.emplace_back(v, u);
		m_covs[v].erase(u);
		if(contains(m_undetermined, v)) m_active_undetermined.insert(v);
	}

	auto undo = [u, v, doms_erases, covs_erases] (Instance& inst) -> void {
		inst.m_g.add(u, v);
		for (auto [x, y] : doms_erases) {
			inst.m_doms[x].insert(y);
			if(contains(inst.m_undominated, x)) inst.m_active_undominated.insert(x);
		}
		for (auto [x, y] : covs_erases) {
			inst.m_covs[x].insert(y);
			if(contains(inst.m_undetermined, x)) inst.m_active_undetermined.insert(x);
		}
	};
	m_history.push_back(undo);
}

void Instance::add_edge(szt u, szt v) {
	if (u >= m_g.n) {
		throw std::invalid_argument(std::format("attempt to add edge ({}, {}), but {} is out of bounds", u, v, u));
	}
	if (v >= m_g.n) {
		throw std::invalid_argument(std::format("attempt to add edge ({}, {}), but {} is out of bounds", u, v, v));
	}
	if (contains(m_g[u], v)) {
		throw std::invalid_argument(std::format("attempt to add edge ({}, {}), it already exists", u, v));
	}

	std::vector <std::pair <szt, szt>> doms_insertions;
	std::vector <std::pair <szt, szt>> covs_insertions;
	std::vector <bool> D_nei_insertions;

	m_g.add(u, v);
	if (!m_X[v]) {
		if (!contains(m_doms[u], v)) {
			doms_insertions.emplace_back(u, v);
			m_doms[u].insert(v);
			if(contains(m_undominated, u)) m_active_undominated.insert(u);
		}
	}
	if (!m_X[u]) {
		if (!contains(m_doms[v], u)) {
			doms_insertions.emplace_back(v, u);
			m_doms[v].insert(u);
			if(contains(m_undominated, v)) m_active_undominated.insert(v);
		}
	}
	if (!m_D[v] && !m_W[v]) {
		if (!contains(m_covs[u], v)) {
			covs_insertions.emplace_back(u, v);
			m_covs[u].insert(v);
			if(contains(m_undetermined, u)) m_active_undetermined.insert(u);
		}
	}
	if (!m_D[u] && !m_W[u]) {
		if (!contains(m_covs[v], u)) {
			covs_insertions.emplace_back(v, u);
			m_covs[v].insert(u);
			if(contains(m_undetermined, v)) m_active_undetermined.insert(v);
		}
	}
	m_g_orig.add(u, v);
	if (m_D[u]) {
		if (m_D_nei[v] == false) D_nei_insertions.push_back(v);
		m_D_nei[v] = true;
	}
	if (m_D[v]) {
		if (m_D_nei[u] == false) D_nei_insertions.push_back(u);
		m_D_nei[u] = true;
	}

	auto undo = [u, v, doms_insertions, covs_insertions, D_nei_insertions] (Instance& inst) -> void {
		inst.m_g.erase(u, v);
		for (auto [x, y] : doms_insertions) {
			inst.m_doms[x].erase(y);
			if(contains(inst.m_undominated, x)) inst.m_active_undominated.insert(x);
		}
		for (auto [x, y] : covs_insertions) {
			inst.m_covs[x].erase(y);
			if(contains(inst.m_undetermined, x)) inst.m_active_undetermined.insert(x);
		}
		inst.m_g_orig.erase(u, v);
		for (szt x : D_nei_insertions) inst.m_D_nei[x] = false;
	};
	m_history.push_back(undo);
}

void Instance::clear_active_undetermined() {
	m_active_undetermined.clear();
}

void Instance::clear_active_undominated() {
	m_active_undominated.clear();
}


void Instance::insert_dead_into_D(szt v) {
	if (contains(m_alives, v)) {
		throw std::invalid_argument(std::format("attempt to insert (alive) {} into D using Instace::insert_dead_into_D()", v));
	}
	if (m_D[v]) {
		throw std::invalid_argument(std::format("attempt to insert {} into D, but {} already in D", v, v));
	}

	bool was_inserted_into_D = m_D[v] == false;

	m_D_size++;
	m_D[v] = true;

	auto undo = [v, was_inserted_into_D] (Instance& inst) -> void {
		inst.m_D_size--;
		if (was_inserted_into_D) inst.m_D[v] = false;
	};
	m_history.push_back(undo);

	// history entries from insert_W_Dnei(nei)s should be recovered first (added after ours), as its changes happened after ours
	for (szt nei : m_g[v]) {
		insert_W_Dnei(nei);
	}
}

void Instance::remove_from_D(szt v) {
	if (!m_D[v]) {
		throw std::invalid_argument(std::format("attempt to remove {} from D, but {} not in D", v, v));
	}

	bool was_erased_from_D = m_D[v] == true;

	m_D_size--;
	m_D[v] = false;

	auto undo = [v, was_erased_from_D] (Instance& inst) -> void {
		inst.m_D_size++;
		if (was_erased_from_D) inst.m_D[v] = true;
	};
	m_history.push_back(undo);
}

const G& Instance::g() const {
	return m_g;
}

bool Instance::alive(szt v) const {
	return contains(m_alives, v);
}

const hash_set <szt>& Instance::alives() const {
	return m_alives;
}

const hash_set <szt>& Instance::undetermined() const {
	return m_undetermined;
}

const hash_set <szt>& Instance::active_undetermined() const {
	return m_active_undetermined;
}

const hash_set <szt>& Instance::undominated() const {
	return m_undominated;
}

const hash_set <szt>& Instance::active_undominated() const {
	return m_active_undominated;
}

bool Instance::W(szt v) const {
	return m_W[v];
}

bool Instance::D(szt v) const {
	return m_D[v];
}

bool Instance::X(szt v) const {
	return m_X[v];
}

bool Instance::D_nei(szt v) const {
	return m_D_nei[v];
}

const hash_set <szt>& Instance::dom(szt v) const {
	return m_doms[v];
}

const hash_set <szt>& Instance::cov(szt v) const {
	return m_covs[v];
}

const hash_set <szt>& Instance::nX() const {
	return m_nX;
}

szt Instance::D_size() const {
	return m_D_size;
}

bool Instance::can_insert_X(szt v) const {
	for (szt nei : m_g[v]) {
		if (m_doms[nei].size() == 1 && *m_doms[nei].begin() == v && !m_W[nei]) return false;
	}
	if (m_doms[v].size() == 1 && !m_W[v]) return false;
	return true;
}

std::string Instance::solution() const {
	std::ostringstream oss;
	std::vector <int> in_D;
	for (szt v = 0; v < m_g.n; v++) {
		if (m_D[v]) {
			in_D.push_back(v);
		}
	}
	oss << in_D.size() << "\n";
	for (szt v : in_D) {
		oss << v + 1 << "\n";
	}
	return oss.str();
}

std::string Instance::current_graph_string() const {
	std::ostringstream oss;
	szt n = m_alives.size();
	hash_map <szt, szt> coord_compr;
	// reserve(coord_compr, m_alives.size());
	for (szt v : m_alives) {
		szt now = coord_compr.size();
		coord_compr[v] = now;
	}
	std::vector <std::pair <szt, szt>> ed;
	for (szt v : m_alives) {
		for (szt u : m_g[v]) {
			if (v < u) {
				ed.emplace_back(coord_compr[v], coord_compr[u]);
			}
		}
	}
	szt m = ed.size();
	oss << "p ds " << n << " " << m << "\n";
	for (auto [u, v] : ed) {
		oss << u + 1 << " " << v + 1 << "\n";
	}
	return oss.str();
}

void Instance::add_adjusting_callback(std::function <void (Instance&)> fn) {
	m_adjusting_callbacks.push_back(fn);

	auto undo = [] (Instance& inst) -> void {
		inst.m_adjusting_callbacks.pop_back();
	};
	m_history.push_back(undo);
}

void Instance::clear_adjusting_callbacks() {
	while (!m_adjusting_callbacks.empty()) {
		m_adjusting_callbacks.back()(*this);

		auto fn = m_adjusting_callbacks.back();
		auto undo = [fn] (Instance& inst) -> void {
			inst.m_adjusting_callbacks.push_back(fn);
		};
		m_history.push_back(undo);

		m_adjusting_callbacks.pop_back();
	}
}

szt Instance::get_checkpoint() const {
	return m_history.size();
}

void Instance::restore(szt checkpoint) {
	while (m_history.size() > checkpoint) {
		m_history.back()(*this);
		m_history.pop_back();
	}
}
