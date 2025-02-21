#include "instance.h"

#include <stdexcept>
#include <fstream>
#include <format>
#include <string>
#include <sstream>
#include <iostream>

Instance::Instance(std::string_view filepath) {
	std::ifstream inf((std::string(filepath)));
	if (!inf) {
		throw std::runtime_error(std::format("failed to open '{}' for reading", filepath));
	}

	std::vector <std::pair <size_t, size_t>> lines;
	std::vector <size_t> corr_line;

	std::string line;
	for (size_t i = 1; std::getline(inf, line); i++) {
		if (line.empty() || line[0] == 'c') continue;

		try {
			std::istringstream iss(line);
			if (lines.empty()) {
				std::string _p, _ds;
				iss >> _p >> _ds;
				if (_p != "p" || _ds != "ds") {
					std::cerr << "warning: incorrectly formatted line (" << i << ") in instance file '" << filepath << "', expected 'p ds'" << std::endl;
					std::cerr << "ignoring..." << std::endl;
					continue;
				}
			}
			size_t first, second;
			iss >> first >> second;
			lines.emplace_back(first, second);
			corr_line.push_back(i);
		} catch (const std::exception& e) {
			std::cerr << "warning: incorrectly formatted line (" << i << ") in instance file '" << filepath << "'" << std::endl;
			std::cerr << "what(): " << e.what() << std::endl;
			std::cerr << "ignoring..." << std::endl;
		}
	}

	if (lines.empty() || lines.size() != lines[0].second + 1) {
		throw std::invalid_argument(std::format("number of lines in instance file '{}'", filepath));
	}

	n = lines[0].first;
	m = lines[0].second;
	adj.resize(n);
	for (size_t i = 1; i <= m; i++) {
		if (lines[i].first < 1 || lines[i].first > n) {
			throw std::invalid_argument(std::format("invalid index ({} with n = {}) on line {} in instance file '{}'", lines[i].first, n, corr_line[i], filepath));
		}
		if (lines[i].second < 1 || lines[i].second > n) {
			throw std::invalid_argument(std::format("invalid index ({} with n = {}) on line {} in instance file '{}'", lines[i].second, n, corr_line[i], filepath));
		}

		adj[lines[i].first - 1].push_back(lines[i].second - 1);
		adj[lines[i].second - 1].push_back(lines[i].first - 1);
	}
}
