#include <iostream>
#include <string>
using namespace std;
int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;
  cout << "$ ";
  string cmd;
  getline(cin , cmd);
  cout << cmd << ": command not found" << endl;
  // TODO: Uncomment the code below to pass the first stage
}
