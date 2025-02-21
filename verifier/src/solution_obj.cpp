#include "solution_obj.h"

#include <stdexcept>
#include <fstream>
#include <format>
#include <string>
#include <iostream>
#include <cassert>

Solution::Solution(std::string_view filepath) {
	std::ifstream inf((std::string(filepath)));
	if (!inf) {
		throw std::runtime_error(std::format("failed to open '{}' for reading", filepath));
	}

	std::vector <size_t> lines;

	std::string line;
	while (std::getline(inf, line)) {
		if (line.empty() || line[0] == 'c') continue;

		try {
			size_t value = std::stoull(line);
			lines.push_back(value);
		} catch (const std::exception& e) {
			std::cerr << "warning: incorrectly formatted line in solution file '" << filepath << "'" << std::endl;
			std::cerr << "what(): " << e.what() << std::endl;
			std::cerr << "ignoring..." << std::endl;
		}
	}

	if (lines.empty() || lines.size() != lines[0] + 1) {
		throw std::invalid_argument(std::format("number of lines in solution file '{}'", filepath));
	}

	size = lines[0];
	if (size) {
		vertices = std::vector <size_t> (lines.begin() + 1, lines.end());
		for (size_t& x : vertices) {
			assert(x > 0);
			x--;
		}
	}
}
