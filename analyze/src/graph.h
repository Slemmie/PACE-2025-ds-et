#pragma once

#include <vector>
#include <iostream>
#include <string>

struct G {
	int n, m;
	std::vector <std::vector <int>> adj;
	G(int _n, int _m = -1);
	void add(int u, int v);
	const std::vector <int>& operator [] (size_t index) const;
	std::vector <int>& operator [] (size_t index);
	friend std::ostream& operator << (std::ostream& os, const G& g);
	static G read(std::istream& inf);
	static G read(const std::string& data);
};
