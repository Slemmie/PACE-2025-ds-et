#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>

struct G {
	size_t n, m;
	std::vector <std::unordered_set <size_t>> adj;
	G(size_t _n, size_t _m = 0);
	void add(size_t u, size_t v);
	void erase(size_t u, size_t v);
	size_t add_vertex();
	void pop_vertex();
	const std::unordered_set <size_t>& operator [] (size_t index) const;
	std::unordered_set <size_t>& operator [] (size_t index);
	friend std::ostream& operator << (std::ostream& os, const G& g);
	static G read(std::istream& inf);
	static G read(const std::string& data);
};
