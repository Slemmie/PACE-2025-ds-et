// represents an instance of the "Annotated Minimal Dominating Set" problem
// W represents the set of covered vertices
// D represents the set of vertices that are part of the dominating set
// supports erasing vertices
// alive(v) returns true iff. v is not erased
// is persistent over insertions into W, insertions into D, and erasures of vertices

#pragma once

#include "graph.h"
#include "hash_table.h"

#include <vector>
#include <unordered_set>
#include <string>
#include <functional>

class Instance {

public:

	Instance(const G& g);

	void insert_W(szt v);
	void insert_W_Dnei(szt v);
	void insert_D(szt v);
	void insert_X(szt v);
	void remove_X(szt v);
	void erase(szt v);
	szt insert();
	void delete_edge(szt u, szt v);
	void add_edge(szt u, szt v);
	void clear_active_undetermined();
	void clear_active_undominated();

	// using these will make the instance unstable until history is restored to before their usage
	// ! -> see note at clear_adjusting_callbacks() for when to be aware of this
	void insert_dead_into_D(szt v); // special utility for rule2, don't use! puts the instance in an invalid state (except for this->D(szt))
	void remove_from_D(szt v); // special utility for rule2, don't use! puts the instance in an invalid state (except for this->D(szt))

	const G& g() const;
	bool alive(szt v) const;
	const hash_set <szt>& alives() const;
	const hash_set <szt>& undetermined() const;
	const hash_set <szt>& active_undetermined() const;
	const hash_set <szt>& undominated() const;
	const hash_set <szt>& active_undominated() const;
	bool W(szt v) const;
	bool D(szt v) const;
	bool X(szt v) const;
	bool D_nei(szt v) const;

	const hash_set <szt>& dom(szt v) const; // u in dom -> u is undecided and (closed) neighbor to v
	const hash_set <szt>& cov(szt v) const; // u in cov -> u is undominated and (closed) neighbor to v

	const hash_set <szt>& nX() const; // all alive vertices not in X

	szt D_size() const;

	bool can_insert_X(szt v) const;

	std::string solution() const;

	std::string current_graph_string() const;

	void add_adjusting_callback(std::function <void (Instance&)> fn);
	// calling this might make the instance unstable until history is restored to before the invocation
	void clear_adjusting_callbacks();

	szt get_checkpoint() const;
	void restore(szt checkpoint);

private:

	G m_g;
	G m_g_orig;
	hash_set <szt> m_alives;
	hash_set <szt> m_undetermined;
	hash_set <szt> m_active_undetermined;
	hash_set <szt> m_undominated;
	hash_set <szt> m_active_undominated;
	std::vector <bool> m_W;
	szt m_D_size;
	std::vector <bool> m_D;
	std::vector <bool> m_X;
	std::vector <hash_set <szt>> m_doms, m_covs;
	hash_set <szt> m_nX;
	std::vector <bool> m_D_nei;

	// LIFO
	std::vector <std::function <void (Instance&)>> m_adjusting_callbacks;

	std::vector <std::function <void (Instance&)>> m_history;

};
