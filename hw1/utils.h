#ifndef UTILS
#define UTILS
#include <string>

typedef struct Path_PID{
    std::string path;
    std::string pid;
} path_pid;

void print(std::string msg);
bool is_number(const std::string& str);
bool is_dir(std::string dir);

#endif