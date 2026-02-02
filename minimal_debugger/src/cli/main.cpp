#include <csignal>
#include <cstdio>
#include <exception>
#include <iostream>
#include <limits.h>
#include <runner/debugger.h>
#include <string>
#include <unistd.h>

using std::cout, std::cerr, std::endl;
using std::string;

void handle_sigint(int sig) { (void)sig; }

void printHelp(string bin_file) {
  cout << "Usage : " << bin_file << " [OPTIONS]" << endl;
  cout << "Options:" << endl;
  cout << "  -h, --help        Display this help message" << endl;
  cout << "  -v, --version     Display version information" << endl;
  cout << "  --pid <PID>       Attach to a running process with the given PID"
       << endl;
  cout << "  --target <BINARY> Launch and debug the specified binary" << endl;
}

void printVersion() { cout << "Minimal Debugger v0.1" << endl; }

pid_t parse_pid(const string &arg) {
  try {
    size_t pos;
    pid_t pid = std::stoi(arg, &pos);
    if (pos != arg.length()) {
      throw std::invalid_argument("Trailing non-numerical characters");
    }
    return pid;
  } catch (const std::exception &e) {
    cerr << "Error : `" << arg << "` is not a valid PID" << endl;
    exit(-1);
  }
}

int main(int argc, char *argv[]) {
  signal(SIGINT, handle_sigint);
  string bin_file = argv[0];
  bool help = false, version = false, target = false, pid = false;
  pid_t process_id = 0;
  string target_bin;
  for (int i = 1; i < argc; i++) {
    string arg = argv[i];
    if (arg == "-h" || arg == "--help")
      help = true;
    else if (arg == "-v" || arg == "--version")
      version = true;
    else if (arg == "--pid") {
      if (pid) {
        cerr << "Multiple pids specified" << endl;
        return -1;
      }
      if (target) {
        cerr << "Target binary to launch already specified" << endl;
        return -1;
      }
      if (i == argc - 1) {
        cerr << "Expected a process id after `--pid`" << endl;
        return -1;
      }
      pid = true;
      process_id = parse_pid(argv[++i]);
    } else if (arg == "--target") {
      if (target) {
        cerr << "Multiple targets specified" << endl;
        return -1;
      }
      if (pid) {
        cerr << "Process to attach to already specified" << endl;
        return -1;
      }
      if (i == argc - 1) {
        cerr << "Expected a target binary after `--target`" << endl;
        return -1;
      }
      target = true;
      target_bin = argv[++i];
    } else {
      if (target) {
        cerr << "Multiple targets specified" << endl;
        return -1;
      }
      if (pid) {
        cerr << "Process to attach to already specified" << endl;
        return -1;
      }
      target = true;
      target_bin = argv[i];
    }
  }

  if (help) {
    printHelp(bin_file);
    return 0;
  }
  if (version) {
    printVersion();
    return 0;
  }

  auto debugger = Debugger();

  if (target) {
    debugger.load(target_bin);
  } else if (pid) {
    debugger.attach(process_id);
  }

  debugger.run();
}
