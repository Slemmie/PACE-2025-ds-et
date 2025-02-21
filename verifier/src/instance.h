#pragma once

#include <string>
#include <vector>

struct Instance {
	size_t n, m;
	std::vector <std::vector <size_t>> adj;

	Instance(std::string_view filepath);
};
