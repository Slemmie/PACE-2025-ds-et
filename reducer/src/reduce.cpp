#include "reduce.h"

#include <iostream>
#include <vector>
#include <unordered_set>

bool rule_1(Instance& instance) { // O(n)
	bool found = 0;
	std::vector <size_t> to_D;
	for (size_t u : instance.undominated()) {
		if(instance.dom(u).size() == 1) {
			to_D.emplace_back(*instance.dom(u).begin());
			found = 1;
		}
	}
	for (size_t u : to_D) {
		if(!instance.D(u)) instance.insert_D(u);
	}
	std::cout << "c rule_1: " << found << std::endl;
	return found;
}

bool rule_2(Instance& instance) { // O(n)
	bool found = 0;
	std::vector <size_t> to_X;
	std::unordered_set <size_t> lazy_determined;
	for (size_t u : instance.undetermined()) {
		bool first = 1;
		std::unordered_set <size_t> subset_coverage;
		for (size_t v : instance.cov(u)) {
			if(first){
				for (size_t w : instance.g()[v]) {
					if (instance.undetermined().contains(w) && !lazy_determined.contains(w)) {
						subset_coverage.insert(w);
					}
				}
				if (instance.undetermined().contains(v) && !lazy_determined.contains(v)) {
					subset_coverage.insert(v);
				}
				subset_coverage.erase(u);
				first = 0;
			}
			else {
				std::vector <size_t> to_del;
				for (size_t val : subset_coverage) {
					if (!(val == v || instance.g()[v].contains(val))) to_del.emplace_back(val);
				}
				for (size_t val : to_del) {
					subset_coverage.erase(val);
				}
			}
			if(!subset_coverage.size())break;
		}
		if (!subset_coverage.empty()) {
			to_X.emplace_back(u);
			lazy_determined.insert(u);
			found = 1;
			break;
		}
	}
	for (size_t u : to_X){
		if(!instance.X(u)) instance.insert_X(u);
	}
	std::cout << "c rule_2: " << found << std::endl;
	return found;
}

bool rule_3(Instance& instance) { // O(n)
	bool found = 0;
	std::unordered_set <size_t> to_W;
	std::unordered_set <size_t> lazy_dominated;
	for (size_t u : instance.undominated()) {
		bool first = 1;
		std::unordered_set <size_t> ignorable_vertices;
		for (size_t v : instance.dom(u)) {
			if(first){
				for (size_t w : instance.g()[v]) {
					if (instance.undominated().contains(w) && !lazy_dominated.contains(w)) {
						ignorable_vertices.insert(w);
					}
				}
				if (instance.undominated().contains(v) && !lazy_dominated.contains(v)) {
					ignorable_vertices.insert(v);
				}
				ignorable_vertices.erase(u);
				first = 0;
			}
			else {
				std::vector <size_t> to_del;
				for (size_t val : ignorable_vertices) {
					if (!(val == v || instance.g()[v].contains(val))) to_del.emplace_back(val);
				}
				for (size_t val : to_del) {
					ignorable_vertices.erase(val);
				}
			}
			if(!ignorable_vertices.size())break;
		}
		for (size_t v : ignorable_vertices) {
			if(!instance.W(v)) {
				to_W.insert(v);
				lazy_dominated.insert(u);
			}
		}
	}
	if(to_W.size()) {
		found = 1;
		for (size_t u : to_W) {
			if(!instance.W(u)) instance.insert_W(u);
		}
	}
	std::cout << "c rule_3: " << found << std::endl;
	return found;
}

void reduce(Instance& instance) {
	std::cout << "c ==================================\n";
	std::cout << "c \t\tCurrent sizes\n";
	std::cout << "c ----------------------------------\n";
	std::cout << "c undetermined: " << instance.undetermined().size() << "\tundominated: " << instance.undominated().size() << "\tinD: " << instance.D_size() << '\n';
	std::cout << "c ==================================\n";
	while(rule_1(instance) || rule_2(instance) || rule_3(instance));
	std::cout << "c ==================================\n";
	std::cout << "c \t\tNew sizes\n";
	std::cout << "c ----------------------------------\n";
	std::cout << "c undetermined: " << instance.undetermined().size() << "\tundominated: " << instance.undominated().size() << "\tinD: " << instance.D_size() << '\n';
	std::cout << "c ==================================\n";
}

