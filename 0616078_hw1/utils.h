#ifndef UTILS
#define UTILS
#include <string>
#include <vector>
using namespace std;

typedef struct Path_PID
{
    string path;
    string pid;
} path_pid;

typedef struct FD_Info
{
    string fd;
    string type;
    string inode;
    string name;
} fd_info;

typedef fd_info mem_info;

typedef struct Arguments
{
    bool c;
    string c_expr;
    bool t;
    string t_expr;
    bool f;
    string f_expr;
} arguments;

typedef struct Line
{
    string COMMAND;
    string PID;
    string USER;
    string FD;
    string TYPE;
    string NODE;
    string NAME;
} line;

void print(vector<line> &lines, line tmp_line);
void print_all(vector<line> &lines, bool arg_flag, arguments parse_result);
bool is_number(const string &str);
bool is_dir(string dir);
bool is_reg(string dir);
bool is_chr(string dir);
bool is_fifo(string dir);
bool is_sock(string dir);

#endif