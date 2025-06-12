#include "solution.h"

#include <sstream>

Solution::Solution(szt n) :
m_sol_size(0),
m_set(n, false)
{ }

Solution::Solution(szt n, const Instance& instance) :
m_sol_size(0),
m_set(n, false)
{
	for (szt i = 0; i < n; i++) {
		if (instance.D(i)) {
			insert(i);
		}
	}
}

szt Solution::n() const {
	return m_set.size();
}

szt Solution::size() const {
	return m_sol_size;
}

bool Solution::in(szt v) const {
	return m_set[v];
}

void Solution::insert(szt v) {
	m_sol_size++;
	m_set[v] = true;
}

std::string Solution::solution() const {
	std::ostringstream oss;
	std::vector <int> in_D;
	for (szt v = 0; v < m_set.size(); v++) {
		if (m_set[v]) {
			in_D.push_back(v);
		}
	}
	oss << in_D.size() << "\n";
	for (szt v : in_D) {
		oss << v + 1 << "\n";
	}
	return oss.str();
}
