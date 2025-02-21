#pragma once

#include <vector>
#include <string>

struct Solution {
	size_t size;
	std::vector <size_t> vertices;

	Solution(std::string_view filepath);
};
