#include "lp.h"

#include <coin/OsiClpSolverInterface.hpp>
#include <coin/CbcModel.hpp>
#include <coin/CglGomory.hpp>
#include <coin/CoinPackedMatrix.hpp>

#include <stdexcept>
#include <format>

#include <unistd.h>
#include <fcntl.h>

LP::LP(Objective_sense obj_sense, size_t num_variables, const Expression& obj_fun, const std::vector <Expression>& conditions) :
m_obj_sense(obj_sense),
m_num_variables(num_variables),
m_obj_fun(obj_fun),
m_conditions(conditions)
{ }

std::vector <size_t> LP::solve() {
	std::vector <double> obj(m_num_variables, 0);
	for (Term term : m_obj_fun.terms) {
		if (term.variable >= m_num_variables) {
			throw std::invalid_argument(std::format("variable {} index out of range in system of {} variables", term.variable, m_num_variables));
		}
		obj[term.variable] += term.coefficient;
	}
	std::vector <double> col_lower(m_num_variables, 0);
	std::vector <double> col_upper(m_num_variables, 1);
	CoinPackedMatrix matrix(false, 0, 0);
	matrix.setDimensions(0, m_num_variables);
	for (Expression& expression : m_conditions) {
		expression.simplify();
		if (expression.terms.empty()) continue;
		CoinPackedVector row;
		for (Term term : expression.terms) {
			if (term.variable >= m_num_variables) {
				throw std::invalid_argument(std::format("variable {} index out of range in system of {} variables", term.variable, m_num_variables));
			}
			row.insert(term.variable, term.coefficient);
		}
		matrix.appendRow(row);
	}
	std::vector <double> row_lower(m_conditions.size(), 1);
	std::vector <double> row_upper(m_conditions.size(), 1e18);

	OsiClpSolverInterface solver;
	solver.setObjSense(m_obj_sense == Objective_sense::MINIMIZE ? 1 : -1);
	solver.loadProblem(matrix, col_lower.data(), col_upper.data(), obj.data(), row_lower.data(), row_upper.data());

	for (size_t i = 0; i < m_num_variables; i++) {
		solver.setInteger(i);
	}

	CbcModel model(solver);
	model.messageHandler()->setLogLevel(0);

	CglGomory* gomory = new CglGomory();
	model.addCutGenerator(gomory, 0, "Gomory");

	// TODO: manually do something in the library about printing to stdout, log level 0 does not eliminate all
	// for now just redirect stdout to /dev/null and reset it immediately after the model finishes computing
	static int original_stdout = dup(STDOUT_FILENO);
	{
		fflush(stdout);
		int fd = open("/dev/null", O_WRONLY);
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}
	model.branchAndBound();
	{
		fflush(stdout);
		dup2(original_stdout, STDOUT_FILENO);
	}

	const double* solution = model.bestSolution();

	if (solution) {
		std::vector <size_t> result;
		size_t num_c = solver.getNumCols();
		for (size_t i = 0; i < num_c; i++) {
			if (std::round(solution[i]) == 1) {
				result.push_back(i);
			}
		}
		delete(gomory);
		return result;
	}

	delete(gomory);
	throw std::runtime_error("LP solver failed to find a solution");
}

void LP::Expression::simplify() {
	std::unordered_map <size_t, int> acc_coefs;
	for (Term term : terms) {
		acc_coefs[term.variable] += term.coefficient;
	}
	terms.clear();
	for (auto [index, coefficient] : acc_coefs) {
		if (coefficient) {
			terms.push_back(Term { .variable = index, .coefficient = coefficient });
		}
	}
}
