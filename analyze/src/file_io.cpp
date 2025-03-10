#include "file_io.h"

#include <fstream>
#include <stdexcept>
#include <format>
#include <sstream>

std::string file_read(std::string_view filepath) {
	std::ifstream inf(std::string(filepath), std::ios::in);
	if (!inf) {
		throw std::runtime_error(std::format("failed to open file '{}' for reading", std::string(filepath)));
	}
	std::ostringstream oss;
	oss << inf.rdbuf();
	if (!inf) {
		throw std::runtime_error(std::format("failed to read from file '{}'", std::string(filepath)));
	}
	return oss.str();
}

void file_write(std::string_view filepath, std::string_view data) {
	std::ofstream ouf(std::string(filepath), std::ios::out);
	if (!ouf) {
		throw std::runtime_error(std::format("failed to open file '{}' for writing", std::string(filepath)));
	}
	ouf << data;
	if (!ouf) {
		throw std::runtime_error(std::format("failed to write to file '{}'", std::string(filepath)));
	}
}
