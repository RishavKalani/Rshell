#include "history.h"

#include <fstream>
#include <readline/history.h>
#include <stdio.h>

using namespace std;

size_t history_written_upto = 0;

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