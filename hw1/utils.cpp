#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "utils.h"
using namespace std;

void print(string msg)
{
    cout << msg << endl;
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