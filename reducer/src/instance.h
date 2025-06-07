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

	size_t current_checkpoint() const;
	void rollback(size_t checkpoint);

	bool is_restored() const;
	void restore();

	void insert_X(size_t v);
	void insert_W(size_t v);
	void insert_D(size_t v);
	void insert_dead_into_D(size_t v); // be careful
	void remove_from_D(size_t v); // be careful, does not update neighbor W status!
	void erase(size_t v);
	size_t insert();
	void delete_edge(size_t u, size_t v);
	void add_edge(size_t u, size_t v);

	const G& g() const;
	bool alive(size_t v) const;
	const std::unordered_set <size_t>& alives() const;
	const std::unordered_set <size_t>& undetermined() const;
	const std::unordered_set <size_t>& undominated() const;
	bool X(size_t v) const;
	bool W(size_t v) const;
	bool D(size_t v) const;

	std::string solution() const;

	std::string current_graph_string() const;

	void add_adjusting_callback(std::function <void (Instance&)> fn);
	void clear_adjusting_callbacks();

private:

	struct History_item {
		enum class Type {
			W_UPDATE,
			D_UPDATE,
			VERTEX_ERASE_UPDATE,
			VERTEX_INSERT_UPDATE,
			EDGE_DELETE_UPDATE,
			EDGE_ADD_UPDATE
		} type;
		size_t vertex;
		std::vector <std::pair <size_t, size_t>> edges;
	};

	std::vector <History_item> m_history;

private:

	G m_g;
	std::unordered_set <size_t> m_alives;
	std::unordered_set <size_t> m_undetermined;
	std::unordered_set <size_t> m_undominated;
	std::vector <bool> m_X;
	std::vector <bool> m_W;
	std::vector <bool> m_D;

	// LIFO
	std::vector <std::function <void (Instance&)>> m_adjusting_callbacks;

};
