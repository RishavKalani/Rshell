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
size_t history_written_upto = 0;

vector<string> all_commands;

void initialize_completion_list(){
  static const char* builtin[]={"echo","exit","type","cd","pwd","history",nullptr};
  set<string> seen;
  //builtin commands are added to the all commands list
  for(int i=0;builtin[i];i++)
  {
    all_commands.push_back(builtin[i]);
    seen.insert(builtin[i]);
  }
  //add executables from path
  char *path_env=getenv("PATH");
  if(!path_env)
  {
    return;
  }
  string path(path_env);
  stringstream ss(path);
  string dir;
  while(getline(ss,dir,':'))
  {
    if(!fs::exists(dir)) continue;
    for(const auto &entry : fs::directory_iterator(dir))
    {
      const fs::path &p = entry.path();

      if(!fs::is_regular_file(p)) continue;
      
      if(access(p.c_str(),X_OK)!=0) continue;

      string name = p.filename().string();
      if(seen.insert(name).second){
        all_commands.push_back(name);
      }
    }
  }
  sort(all_commands.begin(),all_commands.end());
}

void read_from_history(const string &path,vector<string> &history){
  ifstream file(path);
  if(!file){
    perror("history");
    return;
  }
  string line;
  while(getline(file,line)){
    if(!line.empty()){
      history.push_back(line);
      add_history(line.c_str());
    }
  }

}

void write_from_history(const string &path,vector<string> &history)
{
  ofstream file(path,ios::out | ios::trunc);
  if(!file){
    perror("history");
    return;
  }
  for(const string &cmd: history){
    file << cmd << endl;
  }
  history_written_upto = history.size();
  file.close();
}

void append_history_to_file(const string &path,vector<string> &history ,size_t &written_upto){
  ofstream file(path,ios::out|ios::app);

  if(!file){
    perror("history");
    return;
  }
  for(size_t i = written_upto;i<history.size();i++){
    file << history[i] << endl;
  }
  written_upto = history.size();
  file.close();
}

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

char* builtin_and_executable_generator(const char* text,int state){
  static int index;
  //static const char *builtins[]={"echo","exit","type","pwd","cd","history",nullptr};
  if(state==0){
    index=0;
  }
  while(index < all_commands.size()){ 
    const string &cmd=all_commands[index++];
    if(cmd.compare(0,strlen(text),text) == 0){
      return strdup(cmd.c_str());
    }
  }
  return nullptr;
}

char** custom_completion(const char * text,int start,int end){
  return rl_completion_matches(text,builtin_and_executable_generator);
}

vector<string> tokenize(const string &input){
  string current;
  vector<string> tokens;  
  int ct1=0,ct2=0;
  for(int i=0;i<input.size();i++)
  {
    if(input[i]=='\\' && ct1%2==1){
      i++;
      if(input[i]=='"' || input[i]=='\\')
      {
        current.push_back(input[i]);
      }
      else{
        i--;
        current.push_back(input[i]);
      }
      continue;
    }
    if(input[i]=='\\' && ct2%2==0){
      i++;
      current.push_back(input[i]);
      continue;
    }
    if(input[i]=='"' && ct2%2==0)
    {
      ct1++;
      continue;
    }
    if(ct1%2==1){
      current.push_back(input[i]);
      continue;
    }
    if(input[i]=='\'' && ct1%2==0)
    {
      ct2++;
      continue;
    }
    if(input[i]==' ' && ct1%2==0 && ct2%2==0)
    {
      if(!current.empty()){
        tokens.push_back(current);
      }
      current.clear();
    }
    else{
      current.push_back(input[i]);
    }
  }
  return tokens;
}

int main(){
  vector<string> history;
  char *histfile= getenv("HISTFILE");
  if(histfile!=nullptr){
    read_from_history(histfile,history);
    history_written_upto = history.size();
  }
  // Flush after every std::cout / std:cerr
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
    vector<string> command=tokenize(cmd);

    //redirection parsing
    bool redirect_out= false,redirect_err=false;
    bool append_out=false,append_err=false;
    string outfile;
    vector<string> cleaned;
    for(int i=0;i<command.size();i++)
    {
      if(command[i]==">" || command[i]==">>" ||command[i]=="1>" || command[i]=="1>>")
      {
        redirect_out=true;
        if(command[i]==">>" || command[i]=="1>>") append_out=true;
        if(i+1 >= command.size()){
          cout << "syntax error" << endl;
          redirect_out = false;
          break;
        }
        outfile=command[i+1];
        i++;
      }
      else if(command[i]=="2>" || command[i]=="2>>")
      {
        redirect_err = true;
        if(command[i]=="2>>")
        {
          append_err=true;
        }
        if(i+1>= command.size())
        {
          cout << "Syntax error" << endl;
          redirect_err=false;
          break;
        }
        outfile=command[i+1];
        i++;
      }
      else
      {
        cleaned.push_back(command[i]);
      }
    }
    command=cleaned;
    //applying redirection
    int saved_stdout=-1,fd=-1;
    if(redirect_out)
    {
      saved_stdout=dup(STDOUT_FILENO);
      if(append_out){
        fd=open(outfile.c_str(),O_WRONLY|O_CREAT|O_APPEND,0644);
      }      
      else{
        fd=open(outfile.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0644);
      }
      if(fd<0)
      {
        perror("open");
        continue;
      }
      dup2(fd,STDOUT_FILENO);
      close(fd);
    }
    if(redirect_err){
      saved_stdout=dup(STDERR_FILENO);
      if(append_err){
        fd=open(outfile.c_str(),O_WRONLY|O_CREAT|O_APPEND,0644);
      }
      else{
        fd=open(outfile.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0644);
      }
      if(fd<0){
        perror("open");
        continue;
      }
      dup2(fd,STDERR_FILENO);
      close(fd);
    }
    if(command[0]=="exit")
    {
      write_from_history(histfile,history);
      return 0;
    }
    else if(command[0]=="pwd")
    {
      char cwd[PATH_MAX];
      if(getcwd(cwd,sizeof(cwd))!=nullptr)
      {
        cout << cwd << endl;
      }
      else{ 
        perror("pwd");
      }
    }
    else if(command[0]=="history"){
      if(command.size()>2){
        if(command[1]=="-a"){
          append_history_to_file(command[2],history,history_written_upto);
          continue;
        }
        if(command[1]=="-r")
        {
          read_from_history(command[2],history);
          continue;
        }
        if(command[1]=="-w")
        {
          write_from_history(command[2],history);
          continue;
        }
      }
      if(command.size()==2)
      {
        int val=stoi(command[1]);
        for(int i=history.size()-val;i<history.size();i++){
          cout << i+1 << " " << history[i] << endl;
        }
        continue;
      }
      for(int i=0;i<history.size();i++){
        cout << i+1 << " " << history[i] << endl;
      }
    }
    else if(command[0]=="type")
    {
      if(command.size()==1)
      {
        cout << command[0] << endl;
      }
      if(kw.count(command[1]))
      {
        cout << command[1] << " is a shell builtin" << endl;
      }
      else{
        string path=findExecutableInPath(command[1]);
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
      auto it=command.begin();
      it++;
      while(it!=command.end())
      {
        cout << *it << " ";
        it++;
      }
      cout << endl;
    }
    else if(command[0]=="cd"){
      if(command.size()==1){
        char *home=getenv("HOME");
        fs::current_path(home);
      }
      else{
        string arg=command[1];
        if(arg=="~")
        {
          char *home=getenv("HOME");
          fs::current_path(home);
          continue;
        }
        fs::path dir_path(arg);
        fs::path p=fs::current_path();
        if(!fs::exists(dir_path) || !fs::is_directory(dir_path)){
          cout << "cd: " << arg << ": No such file or directory" << endl;
        }
        else{
          fs::current_path(dir_path);
        }
      }
    }
    else{
      string s=command[0];
      string execpath=findExecutableInPath(s);
      if(execpath.empty())
      {
        cout << command[0] << ": command not found" << endl;
        continue;
      }
      vector<char *> argv;
      for(auto &w:command){
        argv.push_back(const_cast<char *>(w.c_str()));
      }
      argv.push_back(nullptr);
      pid_t pid=fork();
      if(pid==0)
      {
        //child process
        execv(execpath.c_str(),argv.data());
        perror("execv");
        _exit(1);
      }
      else{
        //parent process
        int status;
        waitpid(pid,&status,0);
        //waits for the status update from child.
      }
    } 
    if(redirect_out){
      dup2(saved_stdout,STDOUT_FILENO);
      close(saved_stdout);
    }
    if(redirect_err){
      dup2(saved_stdout,STDERR_FILENO);
      close(saved_stdout);
    }
  }
  return 0;
}
