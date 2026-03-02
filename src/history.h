#ifndef HISTORY_H
#define HISTORY_H

#include <string>
#include <vector>

using namespace std;

extern size_t history_written_upto;

void read_from_history(const string &path, vector<string> &history);
void write_from_history(const string &path, vector<string> &history);
void append_history_to_file(const string &path, vector<string> &history, size_t &written_upto);

#endif