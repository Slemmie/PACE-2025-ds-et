// represents an instance of the "Annotated Minimal Dominating Set" problem
// W represents the set of covered vertices
// D represents the set of vertices that are part of the dominating set
// supports erasing vertices
// alive(v) returns true iff. v is not erased
// is persistent over insertions into W, insertions into D, and erasures of vertices

#pragma once

#include "graph.h"

#include <vector>
#include <unordered_set>
#include <string>
#include <functional>

class Instance {

public:

	Instance(const G& g);

	void insert_W(size_t v);
	void insert_D(size_t v);
	void insert_X(size_t v);
	void insert_dead_into_D(size_t v); // be careful
	void remove_from_D(size_t v); // be careful, does not update neighbor W/covs status!
	void erase(size_t v);
	size_t insert();
	void delete_edge(size_t u, size_t v);
	void add_edge(size_t u, size_t v);

	const G& g() const;
	bool alive(size_t v) const;
	const std::unordered_set <size_t>& alives() const;
	bool W(size_t v) const;
	bool D(size_t v) const;
	bool X(size_t v) const;

	const std::unordered_set <size_t>& dom(size_t v) const; // u in dom -> u is undecided and (closed) neighbor to v
	const std::unordered_set <size_t>& cov(size_t v) const; // u in cov -> u is undominated and (closed) neighbor to v

	size_t D_size() const;

	std::string solution() const;

	std::string current_graph_string() const;

	void add_adjusting_callback(std::function <void (Instance&)> fn);
	void clear_adjusting_callbacks();

private:

	G m_g;
	std::unordered_set <size_t> m_alives;
	std::vector <bool> m_W;
	size_t m_D_size;
	std::vector <bool> m_D;
	std::vector <bool> m_X;
	std::vector <std::unordered_set <size_t>> m_doms, m_covs;

	// LIFO
	std::vector <std::function <void (Instance&)>> m_adjusting_callbacks;

};
