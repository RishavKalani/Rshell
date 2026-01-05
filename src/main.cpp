#include <iostream>
#include <string>
using namespace std;
int main() {
  // Flush after every std::cout / std:cerr
  cout << std::unitbuf;
  cerr << std::unitbuf;
  cout << "$" << endl;
  // TODO: Uncomment the code below to pass the first stage
  // std::cout << "$ ";
}
