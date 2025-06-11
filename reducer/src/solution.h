#pragma once

#include "instance.h"

#include <vector>

class Solution {

public:

	Solution(szt n);
	Solution(szt n, const Instance& instance);

	szt n() const;

	szt size() const;

	bool in(szt v) const;

	void insert(szt v);

	std::string solution() const;

private:

	szt m_sol_size;
	std::vector <bool> m_set;

};
