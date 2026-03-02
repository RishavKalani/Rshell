#ifndef COMPLETION_H
#define COMPLETION_H

#include <vector>
#include <string>

using namespace std;

extern vector<string> all_commands;

void initialize_completion_list();
char* builtin_and_executable_generator(const char* text, int state);
char** custom_completion(const char* text, int start, int end);

#endif