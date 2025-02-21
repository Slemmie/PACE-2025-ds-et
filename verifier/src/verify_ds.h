#pragma once

#include "instance.h"
#include "solution_obj.h"

// return true iff solution is a dominating set, assumes correctly formatted instance and solution
bool verify_ds(const Instance& instance, const Solution& solution, bool verbose = true);
