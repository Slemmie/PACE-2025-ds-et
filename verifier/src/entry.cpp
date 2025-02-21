#include "solution_obj.h"
#include "instance.h"
#include "verify_ds.h"

#include <iostream>

int main(int argc, char** argv) {
	if (argc != 3 && argc != 4) {
		std::cerr << "usage: " << argv[0] << " input_file expected_output_file output_file" << std::endl;
		std::cerr << "or" << std::endl;
		std::cerr << "usage: " << argv[0] << " input_file output_file" << std::endl;
		return 1;
	}

	Instance instance(argv[1]);
	Solution actual(argv[argc == 3 ? 2 : 3]);

	if (!verify_ds(instance, actual)) {
		std::cerr << "solution is not a dominating set" << std::endl;
		return 1;
	}

	if (argc == 4) {
		Solution expected(argv[2]);

		if (!verify_ds(instance, expected)) {
			std::cerr << "expected solution is not a dominating set" << std::endl;
			return 1;
		}

		if (actual.size != expected.size) {
			std::cerr << "expected solution size " << expected.size << " but was " << actual.size << std::endl;
			if (actual.size < expected.size) {
				std::cerr << "INVESTIGATE: somehow the expected solution is worse" << std::endl;
			}
			return 1;
		}
	} else {
		std::cerr << "note: did not verify solution size" << std::endl;
	}

	std::cerr << "OK" << std::endl;
	return 0;
}
