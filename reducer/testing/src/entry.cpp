#include <bits/stdc++.h>

#include "graph.h"
#include "instance.h"
#include "finalize.h"

std::string read(std::filesystem::path path);
void write(std::filesystem::path path, std::string_view source);

template <typename T> static T rnd(T low, T high) {
	// static const unsigned SEED = 0xbeefba11;
	// static std::mt19937 generator(SEED);
	static std::random_device rd;
	static std::mt19937 generator(rd());
	static std::uniform_int_distribution <T> distribution;
	return distribution(generator, typename std::uniform_int_distribution <T>::param_type { low, high });
}

int main(int argc, char** argv) {
	if (argc != 5 && argc != 6) {
		std::cerr << "usage: " << argv[0] << " graph.gr (null|solution.sol) n m [OPTIONAL: Y/n: Y = create vertex cover instance, n and m are increased to support encoding]" << std::endl;
		return 1;
	}

	std::string outgr = argv[1];
	std::string outsol = argv[2];
	int n = std::stoi(argv[3]);
	int m = std::stoi(argv[4]);

	bool vc = false;
	if (argc == 6 && (argv[5][0] == 'Y' || argv[5][0] == 'y')) vc = true;
	if (vc) {
		m *= 3;
	}

	if (m > n * (n - 1) / 2) {
		std::cerr << "m > n * (n - 1) / 2: impossible" << std::endl;
		return 1;
	}

	std::set <std::pair <int, int>> ed;

	size_t vccnt = n;

	while ((int) ed.size() < m) {
		int u = rnd(0, n - 1);
		int v = rnd(0, n - 1);
		if (u == v) continue;
		if (u > v) std::swap(u, v);
		if (ed.count({ u, v })) continue;
		ed.insert({ u, v });
		if (vc) {
			ed.insert({ u, vccnt });
			ed.insert({ v, vccnt });
			vccnt++;
		}
	}

	std::ostringstream oss;
	oss << "p ds " << n + vc * (m / 3) << " " << m << "\n";
	for (auto [u, v] : ed) {
		oss << u + 1 << " " << v + 1 << "\n";
	}

	write(outgr, oss.str());

	if (outsol != "null") {
		std::istringstream iss(oss.str());

		Instance instance(G::read(iss));
		finalize(instance);
		oss.str("");
		oss << instance.solution();

		write(outsol, oss.str());
	}
}

std::string read(std::filesystem::path path) {
	std::ifstream file(path.string());
	if (!file) {
		throw std::runtime_error(std::format("failed to open file '{}' for reading", path.string()));
	}

	std::ostringstream oss;
	oss << file.rdbuf();

	return oss.str();
}

void write(std::filesystem::path path, std::string_view source) {
	std::ofstream file(path.string());
	if (!file) {
		throw std::runtime_error(std::format("failed to open file '{}' for writing", path.string()));
	}

	file << source;
}
