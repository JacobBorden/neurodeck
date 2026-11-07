#include "exec_command.hpp"
#include <cerrno>
#include <sys/wait.h>
#include <vector>
#include <string>
int main(int argc, char** argv) {
    std::vector<std::string> args{"exec"};
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);
    Neurodeck::ExecCommand command;
    command.run(args);
    int status = 0;
    errno = 0;
    return waitpid(-1, &status, WNOHANG) == -1 && errno == ECHILD ? 0 : 90;
}
