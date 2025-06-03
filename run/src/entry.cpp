#include <bits/stdc++.h>

enum class Verdict {
	OK,
	TLE,
	RTE,
	WA
};

std::chrono::milliseconds timelimit;

struct Test_case {
	size_t index;
	std::filesystem::path path;
	std::chrono::milliseconds ms;
	bool RTE = false;
	bool WA = false;
	Verdict verdict() const {
		if (ms > timelimit) return Verdict::TLE;
		if (RTE) return Verdict::RTE;
		if (WA) return Verdict::WA;
		return Verdict::OK;
	}
};

struct Test_case_queue {
	void add_test_case(const Test_case& test_case) {
		std::scoped_lock <std::mutex> lock(m_mutex);
		m_queue.push(test_case);
	}
	Test_case front() {
		std::scoped_lock <std::mutex> lock(m_mutex);
		if (m_queue.empty()) throw std::runtime_error("empty queue");
		Test_case test_case = m_queue.front();
		m_queue.pop();
		return test_case;
	}
private:
	std::queue <Test_case> m_queue;
	std::mutex m_mutex;
};

struct Finished_test_cases {
	void update(size_t index, const Test_case& test_case) {
		std::scoped_lock <std::mutex> lock(m_mutex);
		if (index >= m_test_cases.size()) m_test_cases.resize(index + 1);
		m_test_cases[index] = test_case;
	}
	const Test_case& at(size_t index) {
		std::scoped_lock <std::mutex> lock(m_mutex);
		return m_test_cases[index];
	}
private:
	std::vector <Test_case> m_test_cases;
	std::mutex m_mutex;
};

Test_case_queue test_case_queue;
Finished_test_cases finished_test_cases;

static bool check_csv_structure(const std::filesystem::path& csv_path, const std::vector <std::filesystem::path>& test_case_paths);
static bool append_column(const std::filesystem::path& csv_path, const std::vector <Test_case>& test_cases);
static std::tuple <std::string, int, std::chrono::milliseconds>
child_process(const std::filesystem::path& exe_path, const std::filesystem::path& stdin_path, std::chrono::milliseconds timeout);
static bool verify(const std::filesystem::path& verifier_path, const std::filesystem::path& graph_path, const std::filesystem::path& attempt_path);

std::filesystem::path solver_path;
std::filesystem::path verifier_path;

std::filesystem::path tmp_dir;

size_t progress_bar_dots;
void update_progress_bar(size_t toggle_index) {
	if (toggle_index == std::numeric_limits <size_t>::max()) {
		std::cerr << std::endl;
		return;
	}
	std::cerr << "\r\033[K";
	static std::string bar(progress_bar_dots, '.');
	if (toggle_index != std::numeric_limits <size_t>::max() - 1) bar[toggle_index] = '#';
	std::cerr << bar;
	std::cerr.flush();
}

void worker_func() {
	while (true) {
		Test_case test_case;
		try {
			test_case = test_case_queue.front();
		} catch (...) {
			return;
		}
		auto [output, exit_code, time_spent] = child_process(solver_path, test_case.path, timelimit);
		test_case.ms = time_spent;
		if (exit_code != 0) test_case.RTE = true;
		std::filesystem::path attempt = tmp_dir / std::to_string(std::hash <std::thread::id> ()(std::this_thread::get_id()));
		std::ofstream file(attempt, std::ios::trunc);
		if (!file) throw std::runtime_error(std::format("failed to open file '{}' for writing", attempt.string()));
		file << output;
		file.close();
		bool WA = !verify(verifier_path, test_case.path, attempt);
		if (WA) test_case.WA = true;
		finished_test_cases.update(test_case.index, test_case);
		update_progress_bar(test_case.index);
	}
}

int main(int argc, char** argv) {
	if (argc <= 6) {
		std::cerr << "usage: " << argv[0] << " path_to_output_csv thread_count path_to_solver path_to_verifier time_limit_ms {path_to_test_case}+" << std::endl;
		return 1;
	}
	std::filesystem::path csv_path = argv[1];
	size_t thread_cnt = std::stoull(argv[2]);
	solver_path = argv[3];
	verifier_path = argv[4];
	timelimit = std::chrono::milliseconds(std::stoull(argv[5]));
	std::vector <std::filesystem::path> test_case_paths;
	for (int i = 6; i < argc; i++) {
		test_case_paths.push_back(argv[i]);
	}

	{
		std::ofstream file(csv_path, std::ios::app);
		if (!file) throw std::runtime_error(std::format("failed to touch file '{}'", csv_path.string()));
	}

	progress_bar_dots = test_case_paths.size();

	tmp_dir = "/tmp";
	std::random_device dev;
	std::mt19937 gen(dev());
	std::uniform_int_distribution <uint64_t> dis(std::numeric_limits <uint64_t>::min(), std::numeric_limits <uint64_t>::max());
	tmp_dir /= std::to_string(dis(gen));
	std::filesystem::create_directory(tmp_dir);

	if (!check_csv_structure(csv_path, test_case_paths)) return 1;

	for (size_t i = 0; i < test_case_paths.size(); i++) {
		Test_case test_case;
		test_case.index = i;
		test_case.path = test_case_paths[i];
		test_case_queue.add_test_case(test_case);
	}

	update_progress_bar(std::numeric_limits <size_t>::max() - 1);

	std::vector <std::thread> workers;
	for (size_t i = 0; i < thread_cnt; i++) {
		workers.emplace_back(worker_func);
	}

	for (std::thread& thr : workers) {
		thr.join();
	}

	update_progress_bar(std::numeric_limits <size_t>::max());

	std::filesystem::remove_all(tmp_dir);

	std::vector <Test_case> test_cases;
	for (size_t i = 0; i < test_case_paths.size(); i++) {
		test_cases.push_back(finished_test_cases.at(i));
	}
	if (!append_column(csv_path, test_cases)) {
		return 1;
	}

	return 0;
}

bool check_csv_structure(const std::filesystem::path& csv_path, const std::vector <std::filesystem::path>& test_case_paths) {
	std::ifstream file(csv_path);
	if (!file) {
		std::cerr << "failed to open file " << csv_path << " for reading" << std::endl;
		return false;
	}

	std::string line;
	size_t row_idx = 0;
	std::vector <std::vector <std::string>> rows;

	while (std::getline(file, line)) {
		std::vector <std::string> columns;
		std::istringstream iss(line);
		std::string cell;

		while (std::getline(iss, cell, ',')) {
			columns.push_back(cell);
		}

		if (columns.empty()) {
			std::cerr << "csv contains empty row at line " << row_idx + 1 << std::endl;
			return false;
		}

		rows.push_back(columns);
	}

	if (rows.empty()) {
		return true;
	}

	if (rows.size() < test_case_paths.size()) {
		std::cerr << "csv contains fewer rows than the number of supplied test case paths" << std::endl;
		return false;
	}

	if (rows.size() > test_case_paths.size()) {
		std::cerr << "csv contains more rows than the number of supplied test case paths" << std::endl;
		return false;
	}

	const size_t column_cnt = rows.front().size();
	for (size_t i = 0; i < rows.size(); i++) {
		if (rows[i].size() != column_cnt) {
			std::cerr << "csv contains row (" << i + 1 << ") with an inconsistent number of columns" << std::endl;
			return false;
		}

		if (rows[i][0] != test_case_paths[i].string()) {
			std::cerr
				<< "csv test case path mismatch at row " << i + 1
				<< ", cell contains '" << rows[i][0]
				<< "' but command-line argument was '" << test_case_paths[i] << "'" << std::endl;
			return false;
		}
	}

	return true;
}

bool append_column(const std::filesystem::path& csv_path, const std::vector <Test_case>& test_cases) {
	if (std::filesystem::file_size(csv_path) == 0) {
		std::ofstream file(csv_path, std::ios::trunc);
		if (!file) {
			std::cerr << "failed to open file " << csv_path << " for writing" << std::endl;
			return false;
		}

		for (const Test_case& test_case : test_cases) {
			file << test_case.path.string() << "\n";
		}
	}

	std::ifstream file_in(csv_path);
	if (!file_in) {
		std::cerr << "failed to open file " << csv_path << " for reading" << std::endl;
		return false;
	}

	std::vector <std::string> lines;
	std::string line;
	size_t row_idx = 0;

	while (std::getline(file_in, line)) {
		if (row_idx >= test_cases.size()) {
			std::cerr << "not enough test case outputs were computed, somehow..." << std::endl;
			return false;
		}

		std::ostringstream new_line;
		new_line << line << ",";
		Verdict verdict = test_cases[row_idx].verdict();
		if (verdict == Verdict::OK) {
			new_line << test_cases[row_idx].ms.count() << "ms";
		} else if (verdict == Verdict::RTE) {
			new_line << "RTE";
		} else if (verdict == Verdict::TLE) {
			new_line << "TLE";
		} else if (verdict == Verdict::WA) {
			new_line << "WA";
		}

		lines.push_back(new_line.str());

		row_idx++;
	}

	file_in.close();

	std::ofstream file_out(csv_path, std::ios::trunc);
	if (!file_out) {
		std::cerr << "failed to open file " << csv_path << " for writing" << std::endl;
		return false;
	}

	for (const std::string& new_line : lines) {
		file_out << new_line << "\n";
	}

	return true;
}

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

std::tuple <std::string, int, std::chrono::milliseconds>
child_process(const std::filesystem::path& exe_path, const std::filesystem::path& stdin_path, std::chrono::milliseconds timeout) {
	const std::string stdin_path_str = stdin_path.string();
	int input_fd = open(stdin_path_str.c_str(), O_RDONLY);
	if (input_fd < 0) throw std::system_error(errno, std::generic_category(), std::format("failed to open input file '{}'", stdin_path.string()));

	int pipefd[2];
	if (pipe(pipefd) < 0) {
		close(input_fd);
		throw std::system_error(errno, std::generic_category(), "failed to create pipe for stdout capture");
	}

	pid_t pid = fork();
	if (pid < 0) {
		close(input_fd);
		close(pipefd[0]);
		close(pipefd[1]);
		throw std::system_error(errno, std::generic_category(), "fork() failed");
	}

	if (pid == 0) {
		if (dup2(input_fd, STDIN_FILENO) < 0) std::_Exit(1);
		if (dup2(pipefd[1], STDOUT_FILENO) < 0) std::_Exit(1);

		int null_fd = open("/dev/null", O_WRONLY);
		if (null_fd >= 0) {
			dup2(null_fd, STDERR_FILENO);
			close(null_fd);
		}

		close(input_fd);
		close(pipefd[0]);
		close(pipefd[1]);

		const std::string exe_path_str = exe_path.string();
		execl(exe_path_str.c_str(), exe_path_str.c_str(), (char*) nullptr);

		std::_Exit(1);
	}

	auto start = std::chrono::steady_clock::now();

	close(input_fd);
	close(pipefd[1]);

	std::mutex mutex;
	std::condition_variable cv;
	bool child_exited = false;

	std::thread killer([&] () {
		std::unique_lock <std::mutex> lock(mutex);
		bool timed_out = !cv.wait_for(lock, timeout, [&] { return child_exited; });

		if (timed_out) {
			kill(pid, SIGKILL);
		}
	});

	std::string output;
	output.reserve(4096);
	char buffer[4096];
	ssize_t nread;
	while ((nread = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
		output.append(buffer, static_cast <size_t> (nread));
	}

	close(pipefd[0]);

	int status = 0;
	waitpid(pid, &status, 0);

	auto end = std::chrono::steady_clock::now();

	auto duration = duration_cast <std::chrono::milliseconds> (end - start);

	{
		std::lock_guard <std::mutex> lock(mutex);
		child_exited = true;
	}
	cv.notify_one();

	killer.join();

	return { output, status, duration };
}

bool verify(const std::filesystem::path& verifier_path_, const std::filesystem::path& graph_path, const std::filesystem::path& attempt_path) {
	int pipefd[2];
	if (pipe(pipefd) < 0) throw std::system_error(errno, std::generic_category(), "failed to create pipe");

	pid_t pid = fork();
	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		throw std::system_error(errno, std::generic_category(), "fork() failed");
	}

	if (pid == 0) {
		int null_fd = open("/dev/null", O_WRONLY);
		if (null_fd >= 0) {
			dup2(null_fd, STDOUT_FILENO);
			dup2(null_fd, STDERR_FILENO);
			close(null_fd);
		}

		close(pipefd[0]);
		close(pipefd[1]);

		const std::string exe_path_str = verifier_path_.string();
		const std::string graph_path_str = graph_path.string();
		const std::string attempt_path_str = attempt_path.string();
		execl(exe_path_str.c_str(), exe_path_str.c_str(), graph_path_str.c_str(), attempt_path_str.c_str(), (char*) nullptr);

		std::_Exit(1);
	}

	int status = 0;
	waitpid(pid, &status, 0);

	return status == 0;
}
