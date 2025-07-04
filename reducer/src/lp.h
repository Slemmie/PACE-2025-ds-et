#pragma once

#include "common.h"

#include <vector>
#include <iostream>

// variables are binary and 0-indexed
class LP {

public:

	enum class Objective_sense {
		MINIMIZE,
		MAXIMIZE
	};

	struct Term {
		szt variable;
		int coefficient;

		friend std::ostream& operator << (std::ostream& os, const Term& term) { return os << term.coefficient << "*x" << term.variable; }
	};

	struct Expression {
		std::vector <Term> terms;

		void simplify();
	};

	LP(Objective_sense obj_sense, szt num_variables, const Expression& obj_fun, const std::vector <Expression>& conditions);

	// returns indices of 1-valued variables
	std::vector <szt> solve();

	std::string to_string() const;

private:

	Objective_sense m_obj_sense;
	szt m_num_variables;
	Expression m_obj_fun;
	std::vector <Expression> m_conditions;

};
