#ifndef UTILS
#define UTILS
#include <string>

typedef struct Path_PID
{
    std::string path;
    std::string pid;
} path_pid;

typedef struct Mem_Info
{
    std::string inode;
    std::string name;
} mem_info;

typedef struct FD_Info
{
    std::string fd;
    std::string type;
    std::string inode;
    std::string name;
} fd_info;

void print(std::string msg);
bool is_number(const std::string &str);
bool is_dir(std::string dir);
bool is_reg(std::string dir);
bool is_chr(std::string dir);
bool is_fifo(std::string dir);
bool is_sock(std::string dir);

#endif