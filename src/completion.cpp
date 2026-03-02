#include "completion.h"

#include <readline/readline.h>
#include <algorithm>
#include <set>
#include <sstream>
#include <unistd.h>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

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