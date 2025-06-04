#include "lp.h"

#ifdef LP_CBC
#include <coin/OsiClpSolverInterface.hpp>
#include <coin/CbcModel.hpp>
#include <coin/CglGomory.hpp>
#include <coin/CoinPackedMatrix.hpp>
#endif

#ifdef LP_HIGHS
#include <highs/Highs.h>
#endif

#ifdef LP_SCIP
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#define SCIPCALL(x) \
	do { \
		SCIP_RETCODE _rcode = (x); \
		if (_rcode != SCIP_OKAY) { \
			std::cerr << "SCIP error " << _rcode << " at " << __FILE__ << ":" << __LINE__ << std::endl;  \
			return { }; \
		} \
	} while (0)
#endif

#include <stdexcept>
#include <format>
#include <sstream>
#include <unordered_map>

#include <unistd.h>
#include <fcntl.h>

LP::LP(Objective_sense obj_sense, size_t num_variables, const Expression& obj_fun, const std::vector <Expression>& conditions) :
m_obj_sense(obj_sense),
m_num_variables(num_variables),
m_obj_fun(obj_fun),
m_conditions(conditions)
{ }

std::vector <size_t> LP::solve() {
#ifdef LP_CBC
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
#else
#ifdef LP_HIGHS
	Highs highs;

	highs.setOptionValue("output_flag", false);

	highs.setOptionValue("presolve", "on");
	highs.setOptionValue("mip_heuristic_effort", 0.2);
	highs.setOptionValue("mip_heuristic_run_rins", "on");
	highs.setOptionValue("mip_heuristic_run_rens", "on");
	highs.setOptionValue("mip_heuristic_run_root_reduced_cost", "on");
	highs.setOptionValue("mip_rel_gap", 0.0);
	highs.setOptionValue("mip_abs_gap", 0.0);

	HighsLp lp;
	lp.sense_ = m_obj_sense == Objective_sense::MINIMIZE ? ObjSense::kMinimize : ObjSense::kMaximize;
	lp.offset_ = 0;

	lp.num_col_ = m_num_variables;
	lp.num_row_ = m_conditions.size();

	lp.col_lower_.assign(m_num_variables, 0);
	lp.col_upper_.assign(m_num_variables, 1);
	lp.integrality_.assign(m_num_variables, HighsVarType::kInteger);

	lp.col_cost_.assign(m_num_variables, 0);
	for (const Term& term : m_obj_fun.terms) {
		lp.col_cost_[term.variable] = static_cast <double> (term.coefficient);
	}

	lp.row_lower_.assign(lp.num_row_, 1);
	lp.row_upper_.assign(lp.num_row_, kHighsInf);

	std::vector <std::vector <std::pair <int, double>>> col_entries(m_num_variables);
	for (int i = 0; i < static_cast <int> (lp.num_row_); i++) {
		for (const Term& term : m_conditions[i].terms) {
			col_entries[term.variable].emplace_back(i, static_cast <double> (term.coefficient));
		}
	}

	int total_nz = 0;
	for (int j = 0; j < static_cast <int> (m_num_variables); j++) {
		total_nz += static_cast <int> (col_entries[j].size());
	}

	lp.a_matrix_.format_ = MatrixFormat::kColwise;
	lp.a_matrix_.num_col_ = lp.num_col_;
	lp.a_matrix_.num_row_ = lp.num_row_;

	lp.a_matrix_.start_.resize(m_num_variables + 1);
	lp.a_matrix_.index_.resize(total_nz);
	lp.a_matrix_.value_.resize(total_nz);

	int nz_idx = 0;
	for (int j = 0; j < static_cast <int> (m_num_variables); j++) {
		lp.a_matrix_.start_[j] = nz_idx;
		for (auto& entry : col_entries[j]) {
			int row = entry.first;
			double coeff = entry.second;
			lp.a_matrix_.index_[nz_idx] = row;
			lp.a_matrix_.value_[nz_idx] = coeff;
			nz_idx++;
		}
	}
	lp.a_matrix_.start_[m_num_variables] = total_nz;

	highs.passModel(lp);
	highs.run();

	HighsSolution solution = highs.getSolution();

	std::vector <size_t> result;
	result.reserve(m_num_variables);
	for (int j = 0; j < static_cast <int> (m_num_variables); j++) {
		if (solution.col_value[j] > 0.5) {
			result.push_back(static_cast <size_t> (j));
		}
	}
	return result;
#else
#ifdef LP_SCIP
	SCIP* scip = nullptr;

	SCIPCALL(SCIPcreate(&scip));
	SCIPCALL(SCIPincludeDefaultPlugins(scip));

	SCIPCALL(SCIPsetIntParam(scip, "display/verblevel", 0));

	SCIPCALL(SCIPcreateProbBasic(scip, "MDS_problem"));

	SCIPCALL(SCIPsetIntParam(scip, "presolving/maxrounds", -1));
	SCIPCALL(SCIPsetIntParam(scip, "separating/knapsackcover/freq", 10));
	SCIPCALL(SCIPsetIntParam(scip, "separating/clique/freq", 10));
	SCIPCALL(SCIPsetIntParam(scip, "separating/clique/maxsepacuts",100));
	SCIPCALL(SCIPsetIntParam(scip, "separating/aggregation/freq", 10));
	SCIPCALL(SCIPsetIntParam(scip, "separating/aggregation/maxrounds", -1));
	SCIPCALL(SCIPsetIntParam(scip, "separating/zerohalf/freq", 10));
	SCIPCALL(SCIPsetIntParam(scip, "separating/zerohalf/maxrounds", -1));
	SCIPCALL(SCIPsetIntParam(scip, "separating/oddcycle/freq", 1));
	SCIPCALL(SCIPsetIntParam(scip, "separating/oddcycle/maxrounds", 10));
	SCIPCALL(SCIPsetIntParam(scip, "branching/fullstrong/priority", 1000));
	SCIPCALL(SCIPsetIntParam(scip, "branching/fullstrong/maxdepth", -1));
	SCIPCALL(SCIPsetIntParam(scip, "branching/relpscost/priority", 50000));
	SCIPCALL(SCIPsetBoolParam(scip, "branching/relpscost/probingbounds", TRUE));
	SCIPCALL(SCIPsetIntParam(scip, "heuristics/rounding/freq", -1));
	SCIPCALL(SCIPsetIntParam(scip, "heuristics/veclendiving/freq", -1));
	SCIPCALL(SCIPsetIntParam(scip, "heuristics/subnlp/freq", -1));
	SCIPCALL(SCIPsetIntParam(scip, "heuristics/multistart/freq", -1));

	if (m_obj_sense == Objective_sense::MAXIMIZE) {
		SCIPCALL(SCIPsetObjsense(scip, SCIP_OBJSENSE_MAXIMIZE));
	} else {
		SCIPCALL(SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE));
	}

	std::vector <double> obj_coefs(m_num_variables, 0);
	for (const Term& term : m_obj_fun.terms) {
		assert(term.variable < m_num_variables);
		obj_coefs[term.variable] = static_cast <double> (term.coefficient);
	}

	std::vector <SCIP_VAR*> vars(m_num_variables, nullptr);
	for (size_t i = 0; i < m_num_variables; i++) {
		std::string var_name = "x" + std::to_string(i);
		SCIP_VAR* v = nullptr;
		SCIPCALL(SCIPcreateVarBasic(scip, &v, var_name.c_str(), 0, 1, obj_coefs[i], SCIP_VARTYPE_BINARY));
		SCIPCALL(SCIPaddVar(scip, v));
		vars[i] = v;
	}

	for (size_t ci = 0; ci < m_conditions.size(); ci++) {
		const Expression& expr = m_conditions[ci];

		std::vector <SCIP_VAR*> cons_vars;
		std::vector <double> cons_coefs;
		cons_vars.reserve(expr.terms.size());
		cons_coefs.reserve(expr.terms.size());

		for (const Term& term : expr.terms) {
			assert(term.variable < m_num_variables);
			cons_vars.push_back(vars[term.variable]);
			cons_coefs.push_back(static_cast <double> (term.coefficient));
		}

		std::string cons_name = "c" + std::to_string(ci);

		SCIP_CONS* cons = nullptr;
		SCIPCALL(SCIPcreateConsBasicLinear(scip, &cons, cons_name.c_str(), static_cast <int> (cons_vars.size()), cons_vars.data(), cons_coefs.data(), 1, SCIPinfinity(scip)));
		SCIPCALL(SCIPaddCons(scip, cons));
		SCIPCALL(SCIPreleaseCons(scip, &cons));
	}

	SCIPCALL(SCIPsolve(scip));
	SCIP_SOL* sol = SCIPgetBestSol(scip);

	std::vector <size_t> result;
	if (sol != nullptr) {
		for (size_t i = 0; i < m_num_variables; i++) {
			double val = SCIPgetSolVal(scip, sol, vars[i]);
			if (val > 0.5) result.push_back(i);
		}
	}

	for (SCIP_VAR* v : vars) SCIPCALL(SCIPreleaseVar(scip, &v));
	SCIPCALL(SCIPfree(&scip));

	return result;
#endif
#endif
#endif
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

std::string LP::to_string() const {
	std::ostringstream oss;
	oss << (m_obj_sense == Objective_sense::MINIMIZE ? "MINIMIZE" : "MAXIMIZE");
	{
		std::ostringstream coss;
		coss << " ";
		for (Term term : m_obj_fun.terms) {
			coss << term << " + ";
		}
		std::string str = coss.str();
		str.pop_back();
		str.pop_back();
		str.pop_back();
		oss << str;
	}
	oss << "\nwhere\n";
	for (const Expression& e : m_conditions) {
		if (e.terms.empty()) continue;
		std::ostringstream coss;
		for (Term term : e.terms) {
			coss << term << " + ";
		}
		std::string str = coss.str();
		str.pop_back();
		str.pop_back();
		str.pop_back();
		str += " > 0";
		oss << str << "\n";
	}
	return oss.str();
}
