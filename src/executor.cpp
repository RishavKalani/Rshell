#include "executor.h"

#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include <iostream>

using namespace std;

string findExecutableInPath(const string &command){
  if(command.find('/')!=string::npos)
  {
    if(access(command.c_str(),X_OK)==0)
    {
      return command;
    }
    return "";
  }
  char *pathenv=getenv("PATH");
  if(!pathenv)
  {
    return "";
  }
  string pathStr=pathenv;
  stringstream ss(pathStr);
  string dir;
  while(getline(ss,dir,':'))
  {
    string fullpath=dir+"/"+command;
    if(access(fullpath.c_str(),X_OK)==0)
    {
      return fullpath;
    }
  }
  return "";
}

void execute_external(const vector<string> &command) {

    string execpath = findExecutableInPath(command[0]);

    if (execpath.empty()) {
        cout << command[0] << ": command not found" << endl;
        return;
    }

    vector<char*> argv;
    for (auto &w : command)
        argv.push_back(const_cast<char*>(w.c_str()));
    argv.push_back(nullptr);

    pid_t pid = fork();

    if (pid == 0) {
        execv(execpath.c_str(), argv.data());
        perror("execv");
        _exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}