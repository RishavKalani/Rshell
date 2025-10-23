#include <iostream>
#include <string>
using namespace std;
int main()
{
  // Flush after every std::cout / std:cerr
  cout << std::unitbuf;
  cerr << std::unitbuf;

  // Uncomment this block to pass the first stage
  string cmd;
  while (true)
  {
    cout << "$ ";
    getline(cin, cmd);
    cout << cmd << ": command not found" << endl;
  }
  return 0;
}
