#pragma once

#include "instance.h"

#include <vector>

class Solution {

public:

	Solution(size_t n);
	Solution(size_t n, const Instance& instance);

	size_t n() const;

	size_t size() const;

	bool in(size_t v) const;

	void insert(size_t v);

	std::string solution() const;

private:

	size_t m_sol_size;
	std::vector <bool> m_set;

};
