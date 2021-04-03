#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <unistd.h>
#include <iomanip>
#include "utils.h"
#define BUF_SIZE 1024

using namespace std;

vector<path_pid> process_dir;

void get_command(string *COMMAND, string PID)
{
    ifstream fin("/proc/" + PID + "/comm");
    string content((std::istreambuf_iterator<char>(fin)), (std::istreambuf_iterator<char>()));
    vector<string> num;
    istringstream iss(content);
    for (string s; iss >> s;)
    {
        *COMMAND = s;
        break;
    }
}

void get_user(string *USER, string PID)
{
    uid_t UID = 0;
    struct stat fileInfo;
    stat(string("/proc/" + PID + "/status").c_str(), &fileInfo);
    UID = fileInfo.st_uid;
    struct passwd *user;
    user = getpwuid(UID);
    *USER = user->pw_name;
}

void get_inode(string *NODE, string dir)
{
    struct stat fileinfo;
    stat(dir.c_str(), &fileinfo);
    *NODE = to_string(fileinfo.st_ino);
}

void get_type(string *TYPE, string dir)
{
    if (is_dir(dir))
        *TYPE = "DIR";
    else if (is_reg(dir))
        *TYPE = "REG";
    else if (is_chr(dir))
        *TYPE = "CHR";
    else if (is_fifo(dir))
        *TYPE = "FIFO";
    else if (is_sock(dir))
        *TYPE = "SOCK";
    else
        *TYPE = "unkown";
}

void get_cwd_info(string *NAME, string *NODE, string *TYPE, string PID)
{
    char buff[1024];
    string dir = string("/proc/" + PID + "/cwd");
    ssize_t len = readlink(dir.c_str(), buff, sizeof(buff) - 1);
    if (errno == EACCES)
    {
        *NAME = "/proc/" + PID + "/cwd (readlink: Permission denied)";
        *NODE = "";
    }
    if (len != -1)
    {
        buff[len] = '\0';
        *NAME = string(buff);
        get_inode(NODE, dir);
    }
    get_type(TYPE, dir);
}

void get_root_info(string *NAME, string *NODE, string *TYPE, string PID)
{
    char buff[1024];
    string dir = string("/proc/" + PID + "/root");
    ssize_t len = readlink(dir.c_str(), buff, sizeof(buff) - 1);
    if (errno == EACCES)
    {
        *NAME = "/proc/" + PID + "/root (readlink: Permission denied)";
        *NODE = "";
    }
    if (len != -1)
    {
        buff[len] = '\0';
        *NAME = string(buff);
        get_inode(NODE, dir);
    }
    get_type(TYPE, dir);
}

void get_exe_info(string *NAME, string *NODE, string *TYPE, string PID)
{
    char buff[BUF_SIZE];
    string dir = string("/proc/" + PID + "/exe");
    ssize_t len = readlink(dir.c_str(), buff, sizeof(buff) - 1);
    if (errno == EACCES)
    {
        *NAME = "/proc/" + PID + "/exe (readlink: Permission denied)";
        *NODE = "";
    }
    if (len != -1)
    {
        buff[len] = '\0';
        *NAME = string(buff);
        get_inode(NODE, dir);
    }
    get_type(TYPE, dir);
}

void get_mem_info(string PID, vector<mem_info> &MI)
{
    vector<string> mem;
    vector<string> mem_line;
    vector<mem_info> mi;
    ifstream fin(string("/proc/" + PID + "/maps"));
    string content((std::istreambuf_iterator<char>(fin)), (std::istreambuf_iterator<char>()));
    size_t heap_pos = content.find("[heap]");
    size_t stack_pos = content.rfind("[stack]");
    content = content.substr(heap_pos + 7, stack_pos - heap_pos);

    stringstream ss(content);
    string tmp;
    while (getline(ss, tmp, '\n'))
    {
        mem.push_back(tmp);
    }

    for (int i = 0; i < mem.size(); i++)
    {
        istringstream iss(mem[i]);
        for (string s; iss >> s;)
            mem_line.push_back(s);
        if (mem_line[4] != "0")
        {
            mem_info mem_tmp;
            mem_tmp.inode = mem_line[4];
            mem_tmp.name = mem_line[5];
            mi.push_back(mem_tmp);
        }
        mem_line.clear();
    }

    MI = mi;
}

void get_rwu_info(string PID, vector<string> &fd_files, vector<string> &fdinfo_files, vector<fd_info> &FDI)
{
    string fd_mode, fd_type;
    for (int i = 0; i < fdinfo_files.size(); i++)
    {
        ifstream fin("/proc/" + PID + "/fdinfo/" + fdinfo_files[i]);
        string content((std::istreambuf_iterator<char>(fin)), (std::istreambuf_iterator<char>()));
        vector<string> num;
        istringstream iss(content);
        for (string s; iss >> s;)
            if (is_number(s))
                num.push_back(s);

        if (num[1].substr(num[1].length() - 2) == "00")
            fd_mode = "r";
        else if (num[1].substr(num[1].length() - 2) == "01")
            fd_mode = "w";
        else if (num[1].substr(num[1].length() - 2) == "02")
            fd_mode = "u";

        string fname = "/proc/" + PID + "/fd/" + fdinfo_files[i];
        get_type(&fd_type, fname);

        FD_Info fditmp;
        fditmp.fd = fdinfo_files[i] + fd_mode;
        char buff[BUF_SIZE];
        ssize_t len = readlink(fname.c_str(), buff, sizeof(buff) - 1);
        if (len != -1)
        {
            buff[len] = '\0';
            fditmp.name = string(buff);
            get_inode(&fditmp.inode, fname);
        }
        fditmp.type = fd_type;
        FDI.push_back(fditmp);
    }
}

void get_all_process_files(string baseDir, bool recursive)
{
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(baseDir.c_str())) == NULL)
    {
        cout << "[ERROR: " << errno << " ] Couldn't open " << baseDir << "." << endl;
        return;
    }
    else
    {
        while ((dirp = readdir(dp)) != NULL)
        {
            if (dirp->d_name != string(".") && dirp->d_name != string(".."))
            {
                if (is_dir(baseDir + dirp->d_name) == true && is_number(dirp->d_name))
                {
                    path_pid tmp_dir;
                    tmp_dir.path = baseDir + dirp->d_name + "/";
                    tmp_dir.pid = dirp->d_name;
                    process_dir.push_back(tmp_dir);
                }
            }
        }
        closedir(dp);
    }
}

void get_process_info(path_pid dir)
{
    // need to handle COMMAND, PID, USER, FD, TYPE, NODE, NAME
    string COMMAND = "", PID = dir.pid, USER = "", TYPE = "", NODE = "", NAME = "";
    bool nofd_true = 0;

    get_command(&COMMAND, PID);
    get_user(&USER, PID);
    // five steps to deal with: cwd, root, exe, mem, rwu
    // 1. cwd
    get_cwd_info(&NAME, &NODE, &TYPE, PID);
    print(COMMAND + "\t\t\t" + PID + "\t\t" + USER + "\t\tcwd\t\t" + TYPE + "\t\t" + NODE + "\t\t" + NAME);

    // 2. root
    get_root_info(&NAME, &NODE, &TYPE, PID);
    print(COMMAND + "\t\t\t" + PID + "\t\t" + USER + "\t\troot\t\t" + TYPE + "\t\t" + NODE + "\t\t" + NAME);

    // 3. exe
    get_exe_info(&NAME, &NODE, &TYPE, PID);
    print(COMMAND + "\t\t\t" + PID + "\t\t" + USER + "\t\texe\t\t" + TYPE + "\t\t" + NODE + "\t\t" + NAME);

    // handle nofd
    DIR *dp;
    struct dirent *dirp;
    vector<string> fd_files, fdinfo_files;
    if ((dp = opendir((dir.path + "fd").c_str())) == NULL && errno == EACCES)
    {
        print(COMMAND + "\t\t\t" + PID + "\t\t" + USER + "\t\tNOFD\t\t" + "" + "\t\t" + "" + "\t\t" + "/proc/" + PID + "/fd (opendir: Permission denied)");
        nofd_true = 1;
        return;
    }
    else
    {
        while ((dirp = readdir(dp)) != NULL)
        {
            if (dirp->d_name != string(".") && dirp->d_name != string(".."))
            {
                fd_files.push_back(dirp->d_name);
            }
        }
        closedir(dp);
    }
    if ((dp = opendir((dir.path + "fdinfo").c_str())) == NULL && errno == EACCES)
    {
        return;
    }
    else
    {
        while ((dirp = readdir(dp)) != NULL)
        {
            if (dirp->d_name != string(".") && dirp->d_name != string(".."))
            {
                fdinfo_files.push_back(dirp->d_name);
            }
        }
        closedir(dp);
    }
    if (nofd_true)
    {
        return;
    }

    // 4. mem (have to deal with deleted file)
    vector<mem_info> MI;
    get_mem_info(PID, MI);
    TYPE = "REG";
    for (int i = 0; i < MI.size(); i++)
    {
        print(COMMAND + "\t\t\t" + PID + "\t\t" + USER + "\t\tmem\t\t" + TYPE + "\t\t" + MI[i].inode + "\t\t" + MI[i].name);
    }
    MI.clear();

    // 5. rwu
    vector<fd_info> FDI;
    get_rwu_info(PID, fd_files, fdinfo_files, FDI);
    for (int i = 0; i < fdinfo_files.size(); i++)
    {
        print(COMMAND + "\t\t\t" + PID + "\t\t" + USER + "\t\t" + FDI[i].fd + "\t\t" + FDI[i].type + "\t\t" + FDI[i].inode + "\t\t" + FDI[i].name);
    }
    FDI.clear();
    fd_files.clear();
    fdinfo_files.clear();
}

int main(int argc, char *argv[])
{

    print("COMMAND\t\t\tPID\t\tUSER\t\tFD\t\tTYPE\t\tNODE\t\tNAME");
    get_all_process_files("/proc/", true);
    for (int i = 0; i < process_dir.size(); i++)
    {
        get_process_info(process_dir[i]);
    }

    return 0;
}