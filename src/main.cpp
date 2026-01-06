#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
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
int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;
  while(true)
  {
    cout << "$ ";
    string cmd;
    getline(cin , cmd);
    if(cmd=="exit")
    {
      return 0;
    }
    else if(cmd.size()>=4 && cmd.substr(0,4)=="type")
    {
      string arg=cmd.substr(5);
      if(cmd.substr(5)=="echo" || cmd.substr(5)=="exit" || cmd.substr(5)=="type")
      {
        cout << cmd.substr(5) << " is a shell builtin" << endl;
      }
      else{
        string path=findExecutableInPath(arg);
        if(!path.empty())
        {
          cout << arg << " is " << path << endl;
        }
        else{
          cout << cmd.substr(5) << ": not found" << endl;
        }
      }
    }
    else if(cmd.size()>=4 && cmd.substr(0,4)=="echo")
    {
      cout << cmd.substr(5) << endl;
    }
    else{
      cout << cmd << ": command not found" << endl;
    } 
  }
  
  // TODO: Uncomment the code below to pass the first stage
}
