#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
using namespace std;

string findExecutableInPath(const string &command)
{
  char *pathenv = getenv("PATH");
  if (!pathenv)
  {
    return "";
  }
  string pathStr = pathenv;
  stringstream ss(pathStr);
  // constrcuting a stringstream of the path
  string dir;
  while (getline(ss, dir, ';'))
  {
    string fullPath = dir + "\\" + command;
    // constructed the fullpath for the file
    if (access(fullPath.c_str(), 0) == 0) // checking the access of the file
    {
      // c_str() converts the string into a char pointer so that i can give access to the file location
      return fullPath;
    }
  }
  return "";
}

int main()
{
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  // Uncomment this block to pass the first stage
  while (true)
  {
    string cmd;
    cout << "$ ";
    getline(cin, cmd);
    if (cmd == "exit 0")
    {
      return 0;
    }
    else if (cmd == "exit 1")
      return 1;
    else if (cmd.size() >= 4 && cmd.substr(0, 4) == "echo")
    {
      cout << cmd.substr(5) << endl;
    }
    else if (cmd.size() >= 4 && cmd.substr(0, 4) == "type")
    {
      string arg = cmd.substr(5);
      if (cmd.substr(5) == "echo" || cmd.substr(5) == "exit" || cmd.substr(5) == "type")
      {
        cout << cmd.substr(5) << " is a shell builtin" << endl;
      }
      else
      {
        string path = findExecutableInPath(arg);
        if (!path.empty())
        {
          cout << cmd << "is" << path << endl;
        }
        else
          cout << arg << ": not found" << endl;
      }
    }
    else
      cout << cmd << ": command not found" << endl;
  }
  return 0;
}
