#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/wait.h>
#include <limits.h>
#include <algorithm>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <dirent.h>

//
// ------------ PATH SPLITTING ------------------------
//
static const std::vector<std::string> bulletin = {"echo", "exit", "type", "cd", "pwd"};
static int tabc = 0;
struct termios original;
std::vector<std::string> get_path_dirs();
void enabletermios()
{
  tcgetattr(STDIN_FILENO, &original);
  struct termios raw = original;
  raw.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disabletermios()
{
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

void handle_exit(int)
{
  disabletermios();
  _exit(0);
}
std::vector<std::string> autocomext_all(const std::string &pre)
{
  std::vector<std::string> dirs = get_path_dirs();
  std::vector<std::string> names;

  for (const auto &d : dirs)
  {
    DIR *dir = opendir(d.c_str());
    if (!dir)
      continue;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
      std::string name = entry->d_name;
      if (name == "." || name == "..")
        continue;

      std::string full = d + "/" + name;
      if (access(full.c_str(), X_OK) == 0)
      {
        if (name.rfind(pre, 0) == 0)
          names.push_back(name);
      }
    }
    closedir(dir);
  }

  // remove duplicates and sort (same executable name may appear in multiple PATH dirs)
  std::sort(names.begin(), names.end());
  names.erase(std::unique(names.begin(), names.end()), names.end());
  return names;
}
std::string longestCommonPrefix(const std::vector<std::string> &v)
{

  // Start with the first string as reference
  std::string prefix = v[0];

  // Compare prefix with every other string
  for (int i = 1; i < (int)v.size(); i++)
  {
    const std::string &s = v[i];
    int j = 0;

    // Find match length
    while (j < (int)prefix.size() && j < (int)s.size() && prefix[j] == s[j])
      j++;

    // Shrink prefix to the matched length
    prefix = prefix.substr(0, j);

    // If at any point prefix is empty → stop early
    if (prefix.empty())
      break;
  }

  return prefix;
}

// print matches (simple listing). redraw handled by caller.
void print_matches_simple(const std::vector<std::string> &names)
{
  write(1, "\n", 1);
  for (size_t k = 0; k < names.size(); ++k)
  {
    write(1, names[k].c_str(), names[k].size());
    if (k + 1 < names.size())
      write(1, "  ", 2);
  }
  write(1, "\n", 1);
}

// tab state: 0 = no pending double-tab, 1 = one TAB pressed (waiting for second)

void autocomplete(std::string &s, int &cursor)
{
  // find start of current word (simple: split on spaces)
  int start = cursor - 1;
  while (start >= 0 && s[start] != ' ')
    start--;
  start++;
  std::string prefix = s.substr(start);

  // collect candidates: builtins + PATH executables that start with prefix
  std::vector<std::string> candidates;

  // builtins
  for (auto &b : bulletin)
  {
    if (b.rfind(prefix, 0) == 0)
      candidates.push_back(b);
  }

  // PATH executables
  std::vector<std::string> path_matches = autocomext_all(prefix);
  candidates.insert(candidates.end(), path_matches.begin(), path_matches.end());

  // remove duplicates (builtin may also be in PATH)
  std::sort(candidates.begin(), candidates.end());
  candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

  // No candidates -> beep and reset state
  if (candidates.empty())
  {
    write(1, "\a", 1);
    tabc = 0;
    return;
  }

  // Single candidate -> complete immediately and reset
  if (candidates.size() == 1)
  {
    const std::string &full = candidates[0];
    std::string missing = full.substr(prefix.size());
    // add a trailing space to complete the token
    missing += ' ';
    s.insert(cursor, missing);
    write(1, missing.c_str(), missing.size());
    cursor += missing.size();
    tabc = 0; // reset double-tab state
    return;
  }

  // Multiple candidates
  if (tabc == 0)
  {
    std::string pref = longestCommonPrefix(candidates);
    if (pref.size() > prefix.size())
    {
      std::string missing1 = pref.substr(prefix.size());
      s.insert(cursor, missing1);
      write(1, missing1.c_str(), missing1.size());
      cursor += missing1.size();
      tabc = 0;
      return;
    }
    else
    {
      tabc = 1;
      write(1, "\a", 1);
      return;
    }
  }
  else
  {
    // second TAB on same prefix: print list and redraw prompt+buffer
    tabc = 0;

    print_matches_simple(candidates);

    // redraw prompt and buffer (assumes prompt is "$ ")
    // If your prompt differs, call your prompt-print function instead.
    write(1, "$ ", 2);
    write(1, s.c_str(), s.size());

    // move cursor to correct position: we printed entire buffer, cursor is at end visually;
    // if cursor is not at end, move back by (s.size() - cursor) using backspaces
    int to_back = (int)s.size() - cursor;
    for (int k = 0; k < to_back; ++k)
      write(1, "\b", 1);

    return;
  }
}

std::string getstring()
{
  std::string s;
  char c;
  int i = 0;
  while (read(0, &c, 1) == 1)
  {
    if (tabc == 1 && c != '\t')
      tabc = 0;

    if (c == '\n')
    {
      write(1, &c, 1);
      break;
    }
    else if (c == '\t')
    {
      autocomplete(s, i);
    }
    else if (c == 127)
    { // Backspace
      if (i > 0 && i == (int)s.size())
      {
        s.pop_back();
        i--;
        write(1, "\b \b", 3);
      }
      else if (i == 0)
      {
        // do nothing
      }
      else
      {
        s.erase(i - 1, 1);
        i--;

        write(1, "\b", 1);
        write(1, s.c_str() + i, s.size() - i);
        write(1, " ", 1);

        for (int j = 0; j < (int)(s.size() - i + 1); j++)
        {
          write(1, "\b", 1);
        }
      }
    }
    else if (c == '\x1b')
    { // Arrow keys
      char a, b;
      read(0, &a, 1);
      read(0, &b, 1);

      if (a == '[' && b == 'C')
      { // Right arrow
        if (i < (int)s.size())
          i++;
      }
      else if (a == '[' && b == 'D')
      { // Left arrow
        if (i > 0)
          i--;
      }
    }
    else
    { // Normal characters
      if (i == (int)s.size())
      {
        s += c;
        i++;
        write(1, &c, 1);
      }
      else
      {
        s.insert(i, 1, c);

        write(1, &c, 1);
        write(1, s.c_str() + i + 1, s.size() - (i + 1));

        for (int j = 0; j < (int)(s.size() - i - 1); j++)
        {
          write(1, "\b", 1);
        }

        i++;
      }
    }
  }

  return s;
}
std::vector<std::string> ultimategettoken(std::string s, int &phr, int &phr2, int &phr0, int &phr02)
{
  int i = 0;
  int sin = 0;
  int dou = 0;
  int spac = 0;
  std::vector<std::string> args;
  std::string word = "";
  while (i < s.size())
  {
    if (s[i] == '\\')
    {
      if (sin == 1)
      {
        word.push_back(s[i]);
        i++;
      }
      else if (dou == 1)
      {
        if ((i + 1 < s.size()) && (s[i + 1] == '`' || s[i + 1] == '$' || s[i + 1] == '\\' || s[i + 1] == '"'))
        {
          word.push_back(s[++i]);
          i = i + 1;
        }
        else
        {

          word.push_back(s[i]);
          i++;
        }
      }
      else
      {
        if (i + 1 < s.size())
        {
          word.push_back(s[++i]);
          i = i + 1;
        }
        else
        {
          i++;
        }
      }
      spac = 0;
    }
    else if (s[i] == '\'')
    {
      if (sin == 1)
      {
        i++;
        sin = 0;
      }
      else if (dou == 1)
      {
        word.push_back(s[i]);
        i++;
      }
      else
      {
        sin = 1;
        i++;
      }
      spac = 0;
    }
    else if (s[i] == '"')
    {
      if (dou == 1)
      {
        i++;
        dou = 0;
      }
      else if (sin == 1)
      {
        word.push_back(s[i]);
        i++;
      }
      else
      {
        dou = 1;
        i++;
      }
      spac = 0;
    }
    else if (s[i] == ' ')
    {
      if (sin == 1 || dou == 1)
      {
        word.push_back(s[i]);
        i++;
      }
      else
      {
        if (spac == 0)
        {
          spac = 1;

          if (!word.empty())
          {
            if (word == ">" || word == "1>")
            {
              phr = 1;
            }
            if (word == "2>")
            {
              phr2 = 1;
            }
            if (word == ">>" || word == "1>>")
            {
              phr0 = 1;
            }
            if (word == "2>>")
            {
              phr02 = 1;
            }
            args.push_back(word);
          }
          word = "";
        }
        i++;
      }
    }
    else
    {
      word.push_back(s[i]);
      i++;
      spac = 0;
    }
  }
  if (!word.empty())
    if (word == ">" || word == "1>")
    {
      phr = 1;
    }
  if (word == "2>")
  {
    phr2 = 1;
  }
  if (word == ">>" || word == "1>>")
  {
    phr0 = 1;
  }
  if (word == "2>>")
  {
    phr02 = 1;
  }
  args.push_back(word);
  return args;
}

bool checkdirectory(std::string &s)
{
  struct stat info;
  if (stat(s.c_str(), &info) != 0)
  {
    return false;
  }
  return S_ISDIR(info.st_mode);
}
void checkandchangedirectory(std::string &s)
{
  if (s == "~")
  {
    char *home = getenv("HOME");
    s = home;
  }
  s.erase(std::remove(s.begin(), s.end(), '\''), s.end());
  if (checkdirectory(s))
  {
    if (chdir(s.c_str()) != 0)
    {
      perror("cd error");
    }
  }
  else
  {
    std::cerr << "cd: " << s << ": No such file or directory" << std::endl;
  }
  //  char buffer[1024];
  // getcwd(buffer, sizeof(buffer));
  // cout << "Changed directory to: " << buffer << "\n";
}
char **convertToCArray(const std::vector<std::string> &args)
{
  char **argv = new char *[args.size() + 1]; // +1 for NULL

  for (size_t i = 0; i < args.size(); i++)
  {
    argv[i] = strdup(args[i].c_str()); // convert string → char*
  }

  argv[args.size()] = NULL; // last element must be NULL

  return argv;
}

bool is_builtin(std::string followup)
{
  std::vector<std::string> bulletin = {"echo", "exit", "type", "cd", "pwd"};
  int size = bulletin.size();

  int i = 0;
  while (i < size)
  {
    if (followup == bulletin[i])
    {

      return 1;
    }
    i++;
  }
  return 0;
}
std::vector<std::string> get_path_dirs()
{
  const char *path_c = getenv("PATH");
  std::vector<std::string> dirs;

  if (!path_c)
    return dirs;
  std::string path(path_c);
  std::stringstream ss(path);
  std::string part;

  while (std::getline(ss, part, ':'))
  {
    if (part.empty())
      dirs.push_back("."); // empty entry = current directory
    else
      dirs.push_back(part);
  }

  return dirs;
}

//
// ------------ FILE CHECKS ---------------------------
//
bool is_regular(const std::string &path)
{
  struct stat st;
  if (stat(path.c_str(), &st) != 0)
    return false;
  return S_ISREG(st.st_mode);
}

bool is_executable(const std::string &path)
{
  return access(path.c_str(), X_OK) == 0;
}

std::string type_command(const std::string &cmd)
{

  auto dirs = get_path_dirs();

  for (const auto &d : dirs)
  {
    std::string full = d + "/" + cmd;

    // Check file exists and is regular
    if (!is_regular(full))
      continue;

    // Check execute permission
    if (!is_executable(full))
      continue; // skip non-executable files

    // Found an executable

    return full;
  }

  // 3. If nothing found
  return "";
}
void executeExternal(const std::vector<std::string> &args, int phr, int phr2, int phr0, int phr02, std::string s)
{
  if (args.empty())
    return;
  int saved = dup(STDOUT_FILENO);
  int saved_err = dup(2);
  if (phr || phr2 || phr0 || phr02)
  {
    if (phr || phr2)
    {
      int fd = open(s.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (phr)
      {
        dup2(fd, 1);
      }
      if (phr2)
      {
        dup2(fd, 2);
      }
      close(fd);
    }
    if (phr0 || phr02)
    {
      int fd = open(s.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
      if (phr0)
      {
        dup2(fd, 1);
      }
      if (phr02)
      {
        dup2(fd, 2);
      }
      close(fd);
    }
  }
  std::string cmd = args[0];

  std::string fullPath = type_command(cmd);

  if (fullPath == "")
  {
    std::cerr << cmd << ": command not found\n";
    return;
  }

  char **argv = convertToCArray(args);

  pid_t pid = fork();

  if (pid == 0)
  {
    // Child replaces itself with the program
    execvp(fullPath.c_str(), argv);

    // If execvp fails:
    perror("execvp");
    exit(1);
  }
  else if (pid > 0)
  {
    // Parent waits
    wait(NULL);
  }
  else
  {
    perror("fork");
  }

  // Cleanup heap memory (good practice)
  for (int i = 0; argv[i] != NULL; i++)
  {
    free(argv[i]);
  }
  delete[] argv;
  dup2(saved, STDOUT_FILENO);
  close(saved);
  dup2(saved_err, 2);
  close(saved_err);
}

int main()
{
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // TODO: Uncomment the code below to pass the first stage
  signal(SIGINT, handle_exit);
  signal(SIGTERM, handle_exit);

  enabletermios();
  while (1)
  {
    std::cout << "$ " << std::flush;
    std::string input = getstring();

    int phr = 0;
    int phr2 = 0;
    int phr0 = 0;
    int phr02 = 0;
    std::vector<std::string> args = ultimategettoken(input, phr, phr2, phr0, phr02);

    if (args[0] == "exit")
    {
      break;
    }
    std::string s = "";
    if (phr == 1 || phr2 == 1 || phr0 == 1 || phr02 == 1)
    {
      int n = args.size();
      if (n < 3)
      {
        std::cerr << "syntax error" << std::endl;
        continue;
      }
      else if (args[n - 2] != ">" && args[n - 2] != "1>" && args[n - 2] != "2>" && args[n - 2] != ">>" && args[n - 2] != "1>>" && args[n - 2] != "2>>")
      {
        std::cerr << "syntax error" << std::endl;
        continue;
      }
      else
      {
        s = args[n - 1];
        args.pop_back();
        args.pop_back();
      }
    }

    if (args.size() > 1)
    {
      if (args[0] == "type")
      {
        int saved = dup(STDOUT_FILENO);
        int saved_err = dup(2);
        if (phr || phr2 || phr0 || phr02)
        {
          if (phr || phr2)
          {
            int fd = open(s.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (phr)
            {
              dup2(fd, 1);
            }
            if (phr2)
            {
              dup2(fd, 2);
            }
            close(fd);
          }
          if (phr0 || phr02)
          {
            int fd = open(s.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (phr0)
            {
              dup2(fd, 1);
            }
            if (phr02)
            {
              dup2(fd, 2);
            }
            close(fd);
          }
        }
        if (is_builtin(args[1]))
        {
          std::cout << args[1] << " is a shell builtin" << std::endl;
        }
        else
        {
          std::string cnd = type_command(args[1]);
          if (cnd != "")
          {
            std::cout << args[1] << " is " << cnd << "\n";
          }
          else
          {
            std::cerr << args[1] << ": not found\n";
          }
        }
        dup2(saved, STDOUT_FILENO);
        close(saved);
        dup2(saved_err, 2);
        close(saved_err);
      }
      else if (args[0] == "echo")
      {
        int saved = dup(STDOUT_FILENO);
        int saved_err = dup(2);
        if (phr || phr2 || phr0 || phr02)
        {
          if (phr || phr2)
          {
            int fd = open(s.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (phr)
            {
              dup2(fd, 1);
            }
            if (phr2)
            {
              dup2(fd, 2);
            }
            close(fd);
          }
          if (phr0 || phr02)
          {
            int fd = open(s.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (phr0)
            {
              dup2(fd, 1);
            }
            if (phr02)
            {
              dup2(fd, 2);
            }
            close(fd);
          }
        }
        for (int i = 1; i < args.size(); i++)
        {
          std::cout << args[i] << " ";
        }
        std::cout << std::endl;
        dup2(saved, STDOUT_FILENO);
        close(saved);
        dup2(saved_err, 2);
        close(saved_err);
      }
      else if (args[0] == "cd")
      {
        checkandchangedirectory(args[1]);
      }
      else
      {
        executeExternal(args, phr, phr2, phr0, phr02, s);
      }
    }
    else if (args[0] == "pwd")
    {
      int saved = dup(STDOUT_FILENO);
      int saved_err = dup(2);
      if (phr || phr2 || phr0 || phr02)
      {
        if (phr || phr2)
        {
          int fd = open(s.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
          if (phr)
          {
            dup2(fd, 1);
          }
          if (phr2)
          {
            dup2(fd, 2);
          }
          close(fd);
        }
        if (phr0 || phr02)
        {
          int fd = open(s.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
          if (phr0)
          {
            dup2(fd, 1);
          }
          if (phr02)
          {
            dup2(fd, 2);
          }
          close(fd);
        }
      }
      char path_c[PATH_MAX];
      if (getcwd(path_c, sizeof(path_c)) != nullptr)
      {
        std::cout << path_c << std::endl;
      }
      else
      {
        perror("getcwd() error");
      }
      dup2(saved, STDOUT_FILENO);
      close(saved);
      dup2(saved_err, 2);
      close(saved_err);
    }
    else
    {
      int saved = dup(STDOUT_FILENO);
      int saved_err = dup(2);
      if (phr || phr2 || phr0 || phr02)
      {
        if (phr || phr2)
        {
          int fd = open(s.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
          if (phr)
          {
            dup2(fd, 1);
          }
          if (phr2)
          {
            dup2(fd, 2);
          }
          close(fd);
        }
        if (phr0 || phr02)
        {
          int fd = open(s.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
          if (phr0)
          {
            dup2(fd, 1);
          }
          if (phr02)
          {
            dup2(fd, 2);
          }
          close(fd);
        }
      }
      std::cerr << input << ": command not found" << std::endl;
      dup2(saved, STDOUT_FILENO);
      close(saved);
      dup2(saved_err, 2);
      close(saved_err);
    }
  }
  disabletermios();
}