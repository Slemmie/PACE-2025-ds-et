#include "reduce.h"

#include <sstream>
#include <queue>
#include <cstdint>
#include <cassert> /// TEMP

struct Metrics {
	size_t rule1_steps = 0;
	size_t rule1_deletions = 0;

	size_t rule2_steps = 0;
	size_t rule2_deletions = 0;
	size_t rule2_additions_vertices = 0;
	size_t rule2_additions_edges = 0;

	void log(bool include_stderr = false) {
		std::ostringstream oss;

		oss << "c rule 1 steps: " << rule1_steps << ", deleted " << rule1_deletions << " total vertices\n";
		oss << "c rule 2 steps: " << rule2_steps << ", deleted " << rule2_deletions << " total vertices (added "
			<< rule2_additions_vertices << " vertices and " << rule2_additions_edges << " edges)\n";

		std::cout << oss.str();
		if (include_stderr) {
			std::cerr << oss.str();
		}
	}
};

static Metrics metrics;

static bool rule1_step(Instance& instance);
static bool rule2_step(Instance& instance);

static std::vector <std::pair <size_t, size_t>> d3;
// static std::vector <std::vector <size_t>> dist;
static size_t temp = 0;

void reduce(Instance& instance) {
	// dist.resize(instance.g().n, std::vector <size_t> (instance.g().n, 1ULL << 60));
	// for (size_t i = 0; i < instance.g().n; i++) dist[i][i] = 0;
	// for (size_t i = 0; i < instance.g().n; i++)
	// 	for (size_t j : instance.g()[i])
	// 		dist[i][j] = dist[j][i] = 1;
	// for (size_t k = 0; k < instance.g().n; k++)
	// 	for (size_t i = 0; i < instance.g().n; i++)
	// 		for (size_t j = 0; j < instance.g().n; j++)
	// 			dist[i][j] = std::min(dist[i][j], dist[i][k] + dist[k][j]);

	for (size_t v : instance.alives()) {
		std::queue <std::pair <size_t, uint8_t>> q;
		q.emplace(v, 0);
		std::unordered_set <size_t> vis;
		while (!q.empty()) {
			auto [u, d] = q.front();
			q.pop();
			if (vis.contains(u)) continue;
			if (d > 3) continue;
			d3.emplace_back(v, u);
			vis.insert(u);
			for (size_t x : instance.g()[u]) {
				if (x < v) {
					q.emplace(x, d + 1);
				}
			}
		}
	}

	bool stop = false;
	while (!stop) {
		stop = true;
		while (rule1_step(instance)) metrics.rule1_steps++, stop = false;
		while (rule2_step(instance)) metrics.rule2_steps++, stop = false;
	}

	std::cerr << "temp = " << temp << std::endl;

	metrics.log(true);
}

static bool rule1_step(Instance& instance) {
	for (size_t v : instance.alives()) {
		std::unordered_set <size_t> N_exit, N_guard, N_prison;
		for (size_t u : instance.g()[v]) {
			bool is_exit = false;
			for (size_t w : instance.g()[u]) {
				if (!instance.g()[v].contains(w) && w != v) {
					is_exit = true;
					break;
				}
			}
			if (is_exit) {
				N_exit.insert(u);
			}
		}
		for (size_t u : instance.g()[v]) {
			if (N_exit.contains(u)) continue;
			bool is_guard = false;
			for (size_t w : instance.g()[u]) {
				if (N_exit.contains(w)) {
					is_guard = true;
					break;
				}
			}
			if (is_guard) {
				N_guard.insert(u);
			}
		}
		for (size_t u : instance.g()[v]) {
			if (!N_exit.contains(u) && !N_guard.contains(u)) {
				N_prison.insert(u);
			}
		}
		bool candidate = false;
		for (size_t u : N_prison) {
			if (!instance.W(u)) {
				candidate = true;
				break;
			}
		}
		if (!candidate) continue;
		std::vector <size_t> to_del;
		for (size_t u : instance.g()[v]) {
			if (N_guard.contains(u) || N_prison.contains(u)) {
				to_del.push_back(u);
			}
		}
		instance.insert_D(v);
		metrics.rule1_deletions++;
		for (size_t u : to_del) {
			instance.erase(u);
			metrics.rule1_deletions++;
		}
		return true;
	}
	return false;
}

static bool rule2_step(Instance& instance) {
	for (auto [v, w] : d3) {
		temp++;
		std::unordered_set <size_t> N_exit, N_guard, N_prison;
		std::unordered_set <size_t> N_vw;
		for (size_t u : instance.g()[v]) {
			N_vw.insert(u);
		}
		for (size_t u : instance.g()[w]) {
			N_vw.insert(u);
		}
		for (size_t u : N_vw) {
			bool is_exit = false;
			for (size_t x : instance.g()[u]) {
				if (!N_vw.contains(x) && x != v && x != w) {
					is_exit = true;
					break;
				}
			}
			if (is_exit) {
				N_exit.insert(u);
			}
		}
		for (size_t u : N_vw) {
			if (N_exit.contains(u)) continue;
			bool is_guard = false;
			for (size_t x : instance.g()[u]) {
				if (N_exit.contains(x)) {
					is_guard = true;
					break;
				}
			}
			if (is_guard) {
				N_guard.insert(u);
			}
		}
		for (size_t u : N_vw) {
			if (!N_exit.contains(u) && !N_guard.contains(u)) {
				N_prison.insert(u);
			}
		}
		bool candidate = false;
		for (size_t u : N_prison) {
			if (!instance.W(u)) {
				candidate = true;
				break;
			}
		}
		for (size_t u : N_guard) {
			if (!candidate) break;
			bool dominates = true;
			for (size_t x : N_prison) {
				if (instance.W(x)) continue;
				dominates &= instance.g()[u].contains(x) || u == x;
			}
			candidate &= !dominates;
		}
		for (size_t u : N_prison) {
			if (!candidate) break;
			bool dominates = true;
			for (size_t x : N_prison) {
				if (instance.W(x)) continue;
				dominates &= instance.g()[u].contains(x) || u == x;
			}
			candidate &= !dominates;
		}
		if (!candidate) continue;
		bool v_dominates = true, w_dominates = true;
		for (size_t u : N_prison) {
			if (instance.W(u)) continue;
			v_dominates &= instance.g()[v].contains(u);
			w_dominates &= instance.g()[w].contains(u);
		}
		if (v_dominates || w_dominates) {
			if (v_dominates && w_dominates) {
				std::vector <size_t> to_del;
				for (size_t u : N_prison) {
					to_del.push_back(u);
				}
				for (size_t u : N_guard) {
					if (instance.g()[v].contains(u) && instance.g()[w].contains(u)) {
						to_del.push_back(u);
					}
				}
				for (size_t u : to_del) {
					instance.erase(u);
					metrics.rule2_deletions++;
				}
				size_t z1 = instance.insert();
				size_t z2 = instance.insert();
				metrics.rule2_additions_vertices += 2;
				instance.insert_W(z1);
				instance.insert_W(z2);
				instance.add_edge(v, z1);
				instance.add_edge(w, z1);
				instance.add_edge(v, z2);
				instance.add_edge(w, z2);
				metrics.rule2_additions_edges += 4;
			} else if (v_dominates) {
				std::vector <size_t> to_del;
				for (size_t u : N_prison) {
					to_del.push_back(u);
				}
				for (size_t u : N_guard) {
					if (instance.g()[v].contains(u)) {
						to_del.push_back(u);
					}
				}
				instance.insert_D(v);
				metrics.rule2_deletions++;
				for (size_t u : to_del) {
					instance.erase(u);
					metrics.rule2_deletions++;
				}
			} else if (w_dominates) {
				std::vector <size_t> to_del;
				for (size_t u : N_prison) {
					to_del.push_back(u);
				}
				for (size_t u : N_guard) {
					if (instance.g()[w].contains(u)) {
						to_del.push_back(u);
					}
				}
				instance.insert_D(w);
				metrics.rule2_deletions++;
				for (size_t u : to_del) {
					instance.erase(u);
					metrics.rule2_deletions++;
				}
			} else {
				throw std::runtime_error("unexpected error");
			}
		} else {
			std::vector <size_t> to_del;
			for (size_t u : N_guard) {
				if (u != v && u != w) {
					to_del.push_back(u);
				}
			}
			for (size_t u : N_prison) {
				if (u != v && u != w) {
					to_del.push_back(u);
				}
			}
			instance.insert_D(v);
			instance.insert_D(w);
			metrics.rule2_deletions += 2;
			for (size_t u : to_del) {
				instance.erase(u);
				metrics.rule2_deletions++;
			}
		}
		return true;
	}
	return false;
}
