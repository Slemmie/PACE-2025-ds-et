#pragma once

#include "common.h"

#include "hash_table.h"

#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>

struct G {
	szt n, m;
	std::vector <hash_set <szt>> adj;
	G(szt _n, szt _m = 0);
	void add(szt u, szt v);
	void erase(szt u, szt v);
	szt add_vertex();
	void pop_vertex();
	const hash_set <szt>& operator [] (szt index) const;
	hash_set <szt>& operator [] (szt index);
	friend std::ostream& operator << (std::ostream& os, const G& g);
	static G read(std::istream& inf);
	static G read(const std::string& data);
};
