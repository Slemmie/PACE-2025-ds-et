#include "solution.h"

#include <sstream>

Solution::Solution(size_t n) :
m_sol_size(0),
m_set(n, false)
{ }

Solution::Solution(size_t n, const Instance& instance) :
m_sol_size(0),
m_set(n, false)
{
	for (size_t i = 0; i < n; i++) {
		if (instance.D(i)) {
			insert(i);
		}
	}
}

size_t Solution::n() const {
	return m_set.size();
}

size_t Solution::size() const {
	return m_sol_size;
}

bool Solution::in(size_t v) const {
	return m_set[v];
}

void Solution::insert(size_t v) {
	m_sol_size++;
	m_set[v] = true;
}

std::string Solution::solution() const {
	std::ostringstream oss;
	std::vector <int> in_D;
	for (size_t v = 0; v < m_set.size(); v++) {
		if (m_set[v]) {
			in_D.push_back(v);
		}
	}
	oss << in_D.size() << "\n";
	for (size_t v : in_D) {
		oss << v + 1 << "\n";
	}
	return oss.str();
}
