#include <iostream>
#include <string>
using namespace std;
int main()
{
  // Flush after every std::cout / std:cerr
  cout << std::unitbuf;
  cerr << std::unitbuf;

  // Uncomment this block to pass the first stage
  cout << "$ ";
}
