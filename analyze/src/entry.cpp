#include "file_io.h"
#include "graph.h"
#include "count_components.h"
#include "count_dominated.h"
#include "count_true_twins.h"
#include "count_OFFable_by_domination.h"
#include "count_removable_by_domination.h"
#include "component_size_distribution.h"
#include "degree_distribution.h"
#include "line_distribution.h"
#include "biconnected_component_size_distribution.h"

#include <json-cpp/json.h>

#include <filesystem>

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "usage: " << argv[0] << " <file path>.gr output.json" << std::endl;
		return 1;
	}

	G g = G::read(file_read(argv[1]));

	JSON json;

	json["biconnected_component_size_distribution"] = JSON();
	for (auto [k, v] : biconnected_component_size_distribution(g)) {
		json["biconnected_component_size_distribution"][std::to_string(k)] = JSON((int64_t) v);
	}

	json["line_length_distribution"] = JSON();
	for (auto [k, v] : line_distribution(g)) {
		json["line_length_distribution"][std::to_string(k)] = JSON((int64_t) v);
	}

	json["component_size_distribution"] = JSON();
	for (auto [k, v] : component_size_distribution(g)) {
		json["component_size_distribution"][std::to_string(k)] = JSON((int64_t) v);
	}

	json["degree_distribution"] = JSON();
	for (auto [k, v] : degree_distribution(g)) {
		json["degree_distribution"][std::to_string(k)] = JSON((int64_t) v);
	}

	json["num_removable_by_domination"] = JSON((int64_t) count_removable_by_domination(g));

	json["num_forced_OFF_by_domination"] = JSON((int64_t) count_OFFable_by_domination(g));

	json["num_true_twins"] = JSON((int64_t) count_true_twins(g));

	json["num_dominated"] = JSON((int64_t) count_dominated(g));

	json["num_components"] = JSON((int64_t) count_components(g));

	json["m"] = JSON((int64_t) g.m);
	json["n"] = JSON((int64_t) g.n);

	json["gr_rpath"] = JSON(
		std::filesystem::path(
			std::filesystem::relative(
				std::filesystem::absolute(argv[1]),
				std::filesystem::absolute(argv[2]).parent_path()
			)
		).lexically_normal().string()
	);

	file_write(argv[2], json.to_string() + "\n");
}
