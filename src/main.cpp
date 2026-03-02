
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>
#include <filesystem>
#include <limits.h>
#include <stdio.h>
#include <set>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>
#include <algorithm>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

// project headers
#include "tokenizer.h"
#include "completion.h"
#include "history.h"
#include "executor.h"
#include "builtins.h"

int main(){
  vector<string> history;
  char *histfile= getenv("HISTFILE");
  if(histfile!=nullptr){
    read_from_history(histfile,history);
    history_written_upto = history.size();
  }

  // Flush after every std::cout / std::cerr
  cout << unitbuf;
  cerr << unitbuf;

  set<string> kw={"echo","type","exit","pwd","cd","history"};
  initialize_completion_list();
  rl_attempted_completion_function = custom_completion;

  while(true)
  {
    char *cmd1 = readline("$ ");
    if(!cmd1){
      break;
    }

    string raw_cmd(cmd1);
    history.push_back(raw_cmd);
    add_history(raw_cmd.c_str());
    free(cmd1);

    string cmd = raw_cmd+" ";
    vector<string> command = tokenize(cmd);

    //redirection parsing
    bool redirect_out = false, redirect_err = false;
    bool append_out = false, append_err = false;
    string outfile;
    vector<string> cleaned;

    for(int i=0;i<(int)command.size();i++)
    {
      if(command[i]==">" || command[i]==">>" || command[i]=="1>" || command[i]=="1>>")
      {
        redirect_out=true;
        if(command[i]==">>" || command[i]=="1>>") append_out=true;

        if(i+1 >= (int)command.size()){
          cout << "syntax error" << endl;
          redirect_out = false;
          break;
        }

        outfile = command[i+1];
        i++;
      }
      else if(command[i]=="2>" || command[i]=="2>>")
      {
        redirect_err = true;
        if(command[i]=="2>>") append_err = true;

        if(i+1 >= (int)command.size()){
          cout << "Syntax error" << endl;
          redirect_err = false;
          break;
        }

        outfile = command[i+1];
        i++;
      }
      else{
        cleaned.push_back(command[i]);
      }
    }

    command = cleaned;

    //applying redirection
    int saved_stdout=-1, saved_stderr=-1, fd=-1;

    if(redirect_out)
    {
      saved_stdout = dup(STDOUT_FILENO);
      if(append_out){
        fd = open(outfile.c_str(),O_WRONLY|O_CREAT|O_APPEND,0644);
      }
      else{
        fd = open(outfile.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0644);
      }
      if(fd<0){
        perror("open");
        continue;
      }
      dup2(fd,STDOUT_FILENO);
      close(fd);
    }

    if(redirect_err)
    {
      saved_stderr = dup(STDERR_FILENO);
      if(append_err){
        fd = open(outfile.c_str(),O_WRONLY|O_CREAT|O_APPEND,0644);
      }
      else{
        fd = open(outfile.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0644);
      }
      if(fd<0){
        perror("open");
        continue;
      }
      dup2(fd,STDERR_FILENO);
      close(fd);
    }

    if(command.size() == 0){
      continue;
    }

    if(command[0]=="exit")
    {
      if(histfile!=nullptr)
        write_from_history(histfile,history);
      return 0;
    }
    else if(command[0]=="pwd")
    {
      char cwd[PATH_MAX];
      if(getcwd(cwd,sizeof(cwd))!=nullptr){
        cout << cwd << endl;
      }
      else{
        perror("pwd");
      }
    }
    else if(command[0]=="history")
    {
      if(command.size()>2){
        if(command[1]=="-a"){
          append_history_to_file(command[2],history,history_written_upto);
          continue;
        }
        if(command[1]=="-r"){
          read_from_history(command[2],history);
          continue;
        }
        if(command[1]=="-w"){
          write_from_history(command[2],history);
          continue;
        }
      }

      if(command.size()==2)
      {
        int val = stoi(command[1]);
        for(int i=(int)history.size()-val;i<(int)history.size();i++){
          cout << i+1 << " " << history[i] << endl;
        }
        continue;
      }

      for(int i=0;i<(int)history.size();i++){
        cout << i+1 << " " << history[i] << endl;
      }
    }
    else if(command[0]=="type")
    {
      if(command.size()==1){
        cout << command[0] << endl;
      }
      if(kw.count(command[1]))
      {
        cout << command[1] << " is a shell builtin" << endl;
      }
      else{
        string path = findExecutableInPath(command[1]);
        if(!path.empty()){
          cout << command[1] << " is " << path << endl;
        }
        else{
          cout << command[1] << ": not found" << endl;
        }
      }
    }
    else if(command[0]=="echo")
    {
      for(int i=1;i<(int)command.size();i++){
        cout << command[i] << " ";
      }
      cout << endl;
    }
    else if(command[0]=="cd")
    {
      if(command.size()==1){
        char *home=getenv("HOME");
        fs::current_path(home);
      }
      else{
        string arg = command[1];
        if(arg=="~"){
          char *home=getenv("HOME");
          fs::current_path(home);
          continue;
        }

        fs::path dir_path(arg);
        if(!fs::exists(dir_path) || !fs::is_directory(dir_path)){
          cout << "cd: " << arg << ": No such file or directory" << endl;
        }
        else{
          fs::current_path(dir_path);
        }
      }
    }
    else{
      execute_external(command);
    }

    //restore stdout
    if(redirect_out){
      dup2(saved_stdout,STDOUT_FILENO);
      close(saved_stdout);
    }

    if(redirect_err){
      dup2(saved_stderr,STDERR_FILENO);
      close(saved_stderr);
    }

  }

  return 0;
}