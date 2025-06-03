#pragma once

#include <cassert>
#include <vector>
#include <cstdint>

namespace cut2 {

	enum class State {
		D, // in D
		I, // dominated from inside
		U  // dominated from outside
	};

	struct Costs {
		uint16_t mask = 0;
		// only use set_cost() once per c1,c2 combination
		void set_cost(State c1, State c2, uint16_t cost) {
			assert(0 <= cost && cost <= 2);
			uint16_t idx = (c1 == State::D ? 0 : c1 == State::U ? 1 : 2) * 3 + (c2 == State::D ? 0 : c2 == State::U ? 1 : 2);
			for (uint16_t i = 0; i < idx; i++) cost *= 3;
			mask += cost;
		}
	};

	// in edges: c1 has index N, c2 has index N+1; indices [0, N-1] are the vertices of the gadget itself
	struct Gadget {
		size_t N, M;
		std::vector <std::pair <size_t, size_t>> edges;
	};

	void init_gadgets();

	bool has_gadget(Costs costs);

	Gadget get_gadget(Costs costs);

}
