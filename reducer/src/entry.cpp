#include "instance.h"
#include "reduce.h"
#include "finalize.h"

int main() {
	Instance instance(G::read(std::cin));
	std::cerr << "dbg: n = " << instance.g().n << ", m = " << instance.g().m << std::endl;
	reduce(instance);
	// finalize(instance);
	// std::cout << instance.solution();
	std::cout << instance.current_graph_string();
}
