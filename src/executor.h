#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <string>
#include <vector>

using namespace std;

string findExecutableInPath(const string &command);
void execute_external(const vector<string> &command);

#endif