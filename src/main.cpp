#include <iostream>
#include <string>
using namespace std;
int main()
{
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  // Uncomment this block to pass the first stage
  while (true)
  {
    string cmd;
    cout << "$ ";
    getline(cin, cmd);
    if (cmd == "exit 0")
    {
      return 0;
    }
    else if (cmd == "exit 1")
      return 1;
    else if (cmd.size() >= 4 && cmd.substr(0, 4) == "echo")
    {
      cout << cmd.substr(5) << endl;
    }
    cout << cmd << ": command not found" << endl;
  }
  return 0;
}
