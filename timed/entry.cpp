#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void execute_with_timeout(const std::vector <std::string>& command, std::chrono::seconds timeout) {
	if (command.empty()) {
		std::cerr << "error: command is empty" << std::endl;
		return;
	}
	std::atomic <bool> finished { false };
	std::mutex mtx;
	std::condition_variable cv;
	pid_t pid = fork();
	if (pid == -1) {
		std::cerr << "error: failed to fork()" << std::endl;
		return;
	}
	if (pid == 0) {
		setpgid(0, 0);
		std::vector <const char*> args;
		for (const auto& arg : command) {
			args.push_back(const_cast <const char*> (arg.c_str()));
		}
		args.push_back(nullptr);
		execvp(args[0], const_cast <char* const*> (args.data()));;
		std::cerr << "error: failed to execvp()" << std::endl;
		std::exit(1);
	}
	setpgid(pid, pid);
	std::jthread timeout_thread([&] () {
		std::unique_lock lock(mtx);
		if (!cv.wait_for(lock, timeout, [&] { return finished.load(); })) {
			std::cerr << "TLE" << std::endl;
			killpg(pid, SIGKILL);
		}
	});
	int status;
	waitpid(pid, &status, 0);
	finished.store(true);
	cv.notify_all();
}

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "usage: " << argv[0] << " <timeout (s)> {commmand ...}" << std::endl;
		return 1;
	}
	std::vector <std::string> command;
	for (int i = 2; i < argc; i++) {
		command.push_back(argv[i]);
	}
	execute_with_timeout(command, std::chrono::seconds(std::stoi(argv[1])));
}
