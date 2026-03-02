#include "builtins.h"
#include "history.h"
#include "executor.h"

#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <limits.h>

using namespace std;
namespace fs = std::filesystem;

bool handle_builtin(const vector<string> &command, vector<string> &history) {

    if (command.empty()) return true;

    const string &cmd = command[0];

    if (cmd == "exit") {
        char *histfile = getenv("HISTFILE");
        if (histfile != nullptr) write_from_history(histfile, history);
        exit(0);
    }

    if (cmd == "pwd") {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr)
            cout << cwd << endl;
        else
            perror("pwd");
        return true;
    }

    if (cmd == "echo") {
        for (int i = 1; i < command.size(); i++)
            cout << command[i] << " ";
        cout << endl;
        return true;
    }

    if (cmd == "cd") {
        if (command.size() == 1) {
            char *home = getenv("HOME");
            fs::current_path(home);
        } else {
            fs::path dir_path(command[1]);
            if (!fs::exists(dir_path) || !fs::is_directory(dir_path))
                cout << "cd: " << command[1] << ": No such file or directory" << endl;
            else
                fs::current_path(dir_path);
        }
        return true;
    }

    return false;
}