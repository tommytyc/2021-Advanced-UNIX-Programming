#include <iostream>
#include <string>
#include <regex>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "utils.h"
using namespace std;

void print(vector<line> &lines, line tmp_line)
{
    lines.push_back(tmp_line);
}

void print_all(vector<line> &lines, bool arg_flag, arguments parse_result)
{
    if (!arg_flag)
    {
        for (int i = 0; i < lines.size(); i++)
            cout << lines[i].COMMAND << "\t\t" << lines[i].PID << "\t\t" << lines[i].USER << "\t\t" << lines[i].FD << "\t\t" << lines[i].TYPE << "\t\t" << lines[i].NODE << "\t\t" << lines[i].NAME << endl;
    }
    else
    {
        for (int i = 0; i < lines.size(); i++)
        {
            bool output_flag[3] = {0, 0, 0};
            if (parse_result.c)
            {
                regex reg(parse_result.c_expr);
                smatch sm;
                while (regex_search(lines[i].COMMAND, sm, reg))
                {
                    output_flag[0] = 1;
                    break;
                }
            }
            else
                output_flag[0] = 1;
            if (parse_result.t)
            {
                if (lines[i].TYPE == parse_result.t_expr)
                    output_flag[1] = 1;
            }
            else
                output_flag[1] = 1;
            if (parse_result.f)
            {
                regex reg(parse_result.f_expr);
                smatch sm;
                while (regex_search(lines[i].NAME, sm, reg))
                {
                    output_flag[2] = 1;
                    break;
                }
            }
            else
                output_flag[2] = 1;
            if (output_flag[0] & output_flag[1] & output_flag[2])
                cout << lines[i].COMMAND << "\t\t" << lines[i].PID << "\t\t" << lines[i].USER << "\t\t" << lines[i].FD << "\t\t" << lines[i].TYPE << "\t\t" << lines[i].NODE << "\t\t" << lines[i].NAME << endl;
        }
    }
}

bool is_number(const string &str)
{
    for (char const &c : str)
    {
        if (isdigit(c) == 0)
            return false;
    }
    return true;
}

bool is_dir(string dir)
{
    struct stat fileInfo;
    stat(dir.c_str(), &fileInfo);
    if (S_ISDIR(fileInfo.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool is_reg(string dir)
{
    struct stat fileInfo;
    stat(dir.c_str(), &fileInfo);
    if (S_ISREG(fileInfo.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool is_chr(string dir)
{
    struct stat fileInfo;
    stat(dir.c_str(), &fileInfo);
    if (S_ISCHR(fileInfo.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool is_fifo(string dir)
{
    struct stat fileInfo;
    stat(dir.c_str(), &fileInfo);
    if (S_ISFIFO(fileInfo.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool is_sock(string dir)
{
    struct stat fileInfo;
    stat(dir.c_str(), &fileInfo);
    if (S_ISSOCK(fileInfo.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }
}