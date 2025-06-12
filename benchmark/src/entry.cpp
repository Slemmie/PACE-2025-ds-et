#include <bits/stdc++.h>

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

void generate_full_random(int n, int m, std::filesystem::path path) {
	std::set <std::pair <int, int>> ed;

	while ((int) ed.size() < m) {
		int u = rnd(0, n - 1);
		int v = rnd(0, n - 1);
		if (u == v) continue;
		if (u > v) std::swap(u, v);
		if (ed.count({ u, v })) continue;
		ed.insert({ u, v });
	}

	std::ostringstream oss;
	oss << "p ds " << n << " " << m << "\n";
	for (auto [u, v] : ed) {
		oss << u + 1 << " " << v + 1 << "\n";
	}

	write(path, oss.str());
}

void generate_low_upper_bound(int n, int m, int mx, std::filesystem::path path) {
	std::set <std::pair <int, int>> ed;

	while ((int) ed.size() < m) {
		int u = rnd(0, mx - 1);
		int v = rnd(0, n - 1);
		if (u == v) continue;
		if (u > v) std::swap(u, v);
		if (ed.count({ u, v })) continue;
		ed.insert({ u, v });
	}

	std::ostringstream oss;
	oss << "p ds " << n << " " << m << "\n";
	for (auto [u, v] : ed) {
		oss << u + 1 << " " << v + 1 << "\n";
	}

	write(path, oss.str());
}

void generate_vertex_cover(int n, int m, std::filesystem::path path) {

	std::set <std::pair <int, int>> ed;

	size_t vccnt = n;

	while ((int) ed.size() / 3 < m) {
		int u = rnd(0, n - 1);
		int v = rnd(0, n - 1);
		if (u == v) continue;
		if (u > v) std::swap(u, v);
		if (ed.count({ u, v })) continue;
		ed.insert({ u, v });
		ed.insert({ u, vccnt });
		ed.insert({ v, vccnt });
		vccnt++;
	}

	std::ostringstream oss;
	oss << "p ds " << vccnt << " " << (int) ed.size() << "\n";
	for (auto [u, v] : ed) {
		oss << u + 1 << " " << v + 1 << "\n";
	}

	write(path, oss.str());
}


int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "usage: " << argv[0] << " testset/ size_multiplier" << std::endl;
		return 1;
	}

	std::string outdir = argv[1];
	if(outdir.back() != '/')outdir += '/';

	std::filesystem::create_directories(outdir);

	double size_multiplier = std::stod(argv[2]);

	for(int i = 1; i <= 10; ++i) generate_full_random(int(i * size_multiplier * 20), int(i * size_multiplier * 30), outdir + std::format("{:03d}", i) + "_full_random.gr");
	for(int i = 1; i <= 10; ++i) generate_full_random(int(i * size_multiplier * 20), int(i * size_multiplier * 43), outdir + std::format("{:03d}", i + 10) + "_full_random.gr");
	for(int i = 1; i <= 10; ++i) generate_full_random(int(i * size_multiplier * 20), int(i * size_multiplier * 60), outdir + std::format("{:03d}", i + 20) + "_full_random.gr");
	for(int i = 1; i <= 10; ++i) generate_full_random(int(i * size_multiplier * 20), int(i * size_multiplier * 90), outdir + std::format("{:03d}", i + 30) + "_full_random.gr");
	for(int i = 1; i <= 10; ++i) generate_full_random(int(i * size_multiplier * 20), int(i * size_multiplier * 120), outdir + std::format("{:03d}", i + 40) + "_full_random.gr");
	for(int i = 1; i <= 10; ++i) generate_low_upper_bound(int(i * size_multiplier * 30), int(i * size_multiplier * 30), int(i * 5 * size_multiplier), outdir + std::format("{:03d}", i + 50) + "_low_upper_bound.gr");
	for(int i = 1; i <= 10; ++i) generate_low_upper_bound(int(i * size_multiplier * 30), int(i * size_multiplier * 43), int(i * 5 * size_multiplier), outdir + std::format("{:03d}", i + 60) + "_low_upper_bound.gr");
	for(int i = 1; i <= 10; ++i) generate_low_upper_bound(int(i * size_multiplier * 30), int(i * size_multiplier * 60), int(i * 5 * size_multiplier), outdir + std::format("{:03d}", i + 70) + "_low_upper_bound.gr");
	for(int i = 1; i <= 10; ++i) generate_low_upper_bound(int(i * size_multiplier * 30), int(i * size_multiplier * 90), int(i * 10 * size_multiplier), outdir + std::format("{:03d}", i + 80) + "_low_upper_bound.gr");
	for(int i = 1; i <= 10; ++i) generate_low_upper_bound(int(i * size_multiplier * 30), int(i * size_multiplier * 120), int(i * 10 * size_multiplier), outdir + std::format("{:03d}", i + 90) + "_low_upper_bound.gr");
	for(int i = 1; i <= 5; ++i) generate_vertex_cover(int(i * size_multiplier * 20), int(i * size_multiplier * 60), outdir + std::format("{:03d}", i + 100) + "_vertex_cover.gr");
	for(int i = 1; i <= 5; ++i) generate_vertex_cover(int(i * size_multiplier * 20), int(i * size_multiplier * 120), outdir + std::format("{:03d}", i + 105) + "_vertex_cover.gr");
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
