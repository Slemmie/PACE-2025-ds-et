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
	void insert_W_Dnei(size_t v);
	void insert_D(size_t v);
	void insert_X(size_t v);
	void remove_X(size_t v);
	void erase(size_t v);
	size_t insert();
	void delete_edge(size_t u, size_t v);
	void add_edge(size_t u, size_t v);
	void clear_active_undetermined();
	void clear_active_undominated();

	// using these will make the instance unstable until history is restored to before their usage
	// ! -> see note at clear_adjusting_callbacks() for when to be aware of this
	void insert_dead_into_D(size_t v); // special utility for rule2, don't use! puts the instance in an invalid state (except for this->D(size_t))
	void remove_from_D(size_t v); // special utility for rule2, don't use! puts the instance in an invalid state (except for this->D(size_t))

	const G& g() const;
	bool alive(size_t v) const;
	const std::unordered_set <size_t>& alives() const;
	const std::unordered_set <size_t>& undetermined() const;
	const std::unordered_set <size_t>& undominated() const;
	const std::unordered_set <size_t>& active_undetermined() const;
	const std::unordered_set <size_t>& active_undominated() const;
	bool W(size_t v) const;
	bool D(size_t v) const;
	bool X(size_t v) const;
	bool D_nei(size_t v) const;

	const std::unordered_set <size_t>& dom(size_t v) const; // u in dom -> u is undecided and (closed) neighbor to v
	const std::unordered_set <size_t>& cov(size_t v) const; // u in cov -> u is undominated and (closed) neighbor to v

	const std::unordered_set <size_t>& nX() const; // all alive vertices not in X

	size_t D_size() const;

	bool can_insert_X(size_t v) const;

	std::string solution() const;

	std::string current_graph_string() const;

	void add_adjusting_callback(std::function <void (Instance&)> fn);
	// calling this might make the instance unstable until history is restored to before the invocation
	void clear_adjusting_callbacks();

	size_t get_checkpoint() const;
	void restore(size_t checkpoint);

private:

	G m_g;
	G m_g_orig;
	std::unordered_set <size_t> m_alives;
	std::unordered_set <size_t> m_undetermined;
	std::unordered_set <size_t> m_undominated;
	std::unordered_set <size_t> m_active_undetermined;
	std::unordered_set <size_t> m_active_undominated;
	std::vector <bool> m_W;
	size_t m_D_size;
	std::vector <bool> m_D;
	std::vector <bool> m_X;
	std::vector <std::unordered_set <size_t>> m_doms, m_covs;
	std::unordered_set <size_t> m_nX;
	std::vector <bool> m_D_nei;

	// LIFO
	std::vector <std::function <void (Instance&)>> m_adjusting_callbacks;

	std::vector <std::function <void (Instance&)>> m_history;

};
