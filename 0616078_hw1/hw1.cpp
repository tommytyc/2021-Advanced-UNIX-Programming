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
#include <algorithm>
#include "utils.h"
#define BUF_SIZE 1024

using namespace std;

vector<path_pid> process_dir;
vector<line> lines;

bool get_command(string *COMMAND, string PID)
{
    ifstream fin("/proc/" + PID + "/comm");
    string content;
    if (fin >> *COMMAND) return true;
    return false;
}

void get_user(string *USER, string PID)
{
    uid_t UID = 0;
    struct stat fileInfo;
    stat(string("/proc/" + PID).c_str(), &fileInfo);
    UID = fileInfo.st_uid;
    ifstream fin("/etc/passwd");
    string content;
    while(getline(fin, content))
    {
        size_t pos = content.find(":");
        while(pos != content.npos)
        {
            content.replace(pos, 1, " ");
            pos = content.find(":");
        }
        stringstream tt;
        string tmp_name, tmp, tmp_uid;
        tt << content;
        tt >> tmp_name;
        tt >> tmp;
        tt >> tmp_uid;
        if(tmp_uid == to_string(UID))
        {
            *USER = tmp_name;
            break;
        }
    }
}

void get_inode(string *NODE, string dir)
{
    struct stat fileinfo;
    stat(dir.c_str(), &fileinfo);
    *NODE = to_string(fileinfo.st_ino);
}

void get_type(string *TYPE, string dir)
{
    struct stat fileInfo;
    stat(dir.c_str(), &fileInfo);
    if (S_ISDIR(fileInfo.st_mode))
        *TYPE = "DIR";
    else if (S_ISREG(fileInfo.st_mode))
        *TYPE = "REG";
    else if (S_ISCHR(fileInfo.st_mode))
        *TYPE = "CHR";
    else if (S_ISFIFO(fileInfo.st_mode))
        *TYPE = "FIFO";
    else if (S_ISSOCK(fileInfo.st_mode))
        *TYPE = "SOCK";
    else
        *TYPE = "unknown";
}

void get_cwd_info(string *NAME, string *APPEND, string *NODE, string *TYPE, string PID)
{
    char buff[1024];
    string dir = string("/proc/" + PID + "/cwd");
    ssize_t len = readlink(dir.c_str(), buff, sizeof(buff) - 1);
    if (errno == EACCES)
    {
        *NAME = "/proc/" + PID + "/cwd";
        *APPEND = "(readlink: Permission denied)";
        *NODE = "";
        *TYPE = "unknown";
    }
    if (len != -1)
    {
        buff[len] = '\0';
        *NAME = string(buff);
        *APPEND = "";
        get_inode(NODE, dir);
        get_type(TYPE, dir);
    }
}

void get_root_info(string *NAME, string *APPEND, string *NODE, string *TYPE, string PID)
{
    char buff[1024];
    string dir = string("/proc/" + PID + "/root");
    ssize_t len = readlink(dir.c_str(), buff, sizeof(buff) - 1);
    if (errno == EACCES)
    {
        *NAME = "/proc/" + PID + "/root";
        *APPEND = "(readlink: Permission denied)";
        *NODE = "";
        *TYPE = "unknown";
    }
    if (len != -1)
    {
        buff[len] = '\0';
        *NAME = string(buff);
        *APPEND = "";
        get_inode(NODE, dir);
        get_type(TYPE, dir);
    }
}

void get_exe_info(string *NAME, string *APPEND, string *NODE, string *TYPE, string PID)
{
    char buff[BUF_SIZE];
    string dir = string("/proc/" + PID + "/exe");
    ssize_t len = readlink(dir.c_str(), buff, sizeof(buff) - 1);
    if (errno == EACCES)
    {
        *NAME = "/proc/" + PID + "/exe";
        *APPEND = "(readlink: Permission denied)";
        *NODE = "";
        *TYPE = "unknown";
    }
    if (len != -1)
    {
        buff[len] = '\0';
        *NAME = string(buff);
        *APPEND = "";
        get_inode(NODE, dir);
        get_type(TYPE, dir);
    }
}

void get_mem_info(string PID, vector<mem_info> &MI)
{
    vector<string> mem;
    vector<string> mem_line;
    vector<mem_info> mi, no_dup_mi;
    ifstream fin(string("/proc/" + PID + "/maps"));
    
    string tmp;
    while (getline(fin, tmp))
    {
        mem.push_back(tmp);
    }

    for (int i = 0; i < mem.size(); i++)
    {
        stringstream iss(mem[i]);
        for (string s; iss >> s;)
            mem_line.push_back(s);
        if (mem_line[4] != "0")
        {
            mem_info mem_tmp;
            mem_tmp.inode = mem_line[4];
            int idx = 5;
            bool deleted = 0;
            mem_tmp.name = mem_line[idx];
            // larger than 6 means deleted or space
            if(mem_line.size() > 6)
            {
                while(idx < mem_line.size())
                {
                    idx++;
                    if(mem_line[idx] != "(deleted)")
                        mem_tmp.name += (" " + mem_line[idx]);
                    else
                    {
                        deleted = 1;
                        break;
                    }
                }
                
                if (deleted)
                    mem_tmp.append =  mem_line[idx];
            }
                
            mi.push_back(mem_tmp);
        }
        mem_line.clear();
    }

    // deal with duplicate file in maps
    bool dup_flag = 0;
    for (int i = 0; i < mi.size(); i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (mi[j].inode == mi[i].inode)
            {
                dup_flag = 1;
                break;
            }
        }
        if (!dup_flag)
            no_dup_mi.push_back(mi[i]);
        else
            dup_flag = 0;
    }

    // deal with deleted files
    for (int i = 0; i < no_dup_mi.size(); i++)
    {
        if(no_dup_mi[i].append == "(deleted)")
        {
            no_dup_mi[i].fd = "del";
            no_dup_mi[i].type = "unknown";
        }
        else
        {
            no_dup_mi[i].fd = "mem";
            no_dup_mi[i].type = "REG";
        }
    }

    // deal with anon_inode
    for (int i = 0; i < no_dup_mi.size(); i++)
    {
        string::size_type pos;
        pos = no_dup_mi[i].name.find("anon_inode:");
        if (pos != no_dup_mi[i].name.npos)
        {
            no_dup_mi[i].name = "anon_inode:[" + no_dup_mi[i].inode + "]";
        }
    }

    MI = no_dup_mi;
}

void get_rwu_info(string PID, vector<string> &fd_files, vector<fd_info> &FDI)
{
    string fd_mode, fd_type;
    for (int i = 0; i < fd_files.size(); i++)
    {
        ifstream fin;
        fin.open(("/proc/" + PID + "/fdinfo/" + fd_files[i]).c_str(), ios::in);
        string content, garbage;
        long long mode;
        while (fin >> content)
        {
            if (content == "flags:")
            {
                fin >> oct >> mode;
                break;
            }
            else
                fin >> garbage;
        }
        if (mode & O_WRONLY)
            fd_mode = "w";
        else if (mode & O_RDWR)
            fd_mode = "u";
        else
            fd_mode = "r";

        string fname = "/proc/" + PID + "/fd/" + fd_files[i];
        get_type(&fd_type, fname);

        FD_Info fditmp;
        fditmp.fd = fd_files[i] + fd_mode;
        char buff[BUF_SIZE];
        ssize_t len = readlink(fname.c_str(), buff, sizeof(buff) - 1);
        if (len != -1)
        {
            buff[len] = '\0';
            fditmp.name = string(buff);
            get_inode(&fditmp.inode, fname);
        }
        string::size_type pos;
        pos = fditmp.name.find("(deleted");
        if (pos != fditmp.name.npos)
            {
                fd_type = "unknown";
                fditmp.name = fditmp.name.substr(0, pos);
                fditmp.append = "(deleted)";
            }
        fditmp.type = fd_type;

        // deal with anon_inode
        string::size_type anon_pos;
        anon_pos = fditmp.name.find("anon_inode:");
        if (anon_pos != fditmp.name.npos)
        {
            fditmp.name = "anon_inode:[" + fditmp.inode + "]";
        }

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
    string COMMAND = "", PID = dir.pid, USER = "", TYPE = "", NODE = "", NAME = "", APPEND = "";
    line tmp_line;
    bool flag = true;

    flag = get_command(&COMMAND, PID);
    if(!flag){
        return;
    }
    get_user(&USER, PID);
    // five steps to deal with: cwd, root, exe, mem, rwu
    // 1. cwd
    get_cwd_info(&NAME, &APPEND, &NODE, &TYPE, PID);
    tmp_line = {COMMAND, PID, USER, "cwd", TYPE, NODE, NAME, APPEND};
    print(lines, tmp_line);

    // 2. root
    get_root_info(&NAME, &APPEND, &NODE, &TYPE, PID);
    tmp_line = {COMMAND, PID, USER, "root", TYPE, NODE, NAME, APPEND};
    print(lines, tmp_line);

    // 3. exe
    get_exe_info(&NAME, &APPEND, &NODE, &TYPE, PID);
    tmp_line = {COMMAND, PID, USER, "exe", TYPE, NODE, NAME, APPEND};
    print(lines, tmp_line);

    // handle nofd
    DIR *dp;
    struct dirent *dirp;
    vector<string> fd_files;
    bool nofd_flag = 0;
    if ((dp = opendir((dir.path + "fd").c_str())) == NULL && errno == EACCES)
    {
        nofd_flag = 1;
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

    // 4. mem (have to deal with deleted file)
    vector<mem_info> MI;
    get_mem_info(PID, MI);
    for (int i = 0; i < MI.size(); i++)
    {
        tmp_line = {COMMAND, PID, USER, MI[i].fd, MI[i].type, MI[i].inode, MI[i].name, MI[i].append};
        print(lines, tmp_line);
    }
    MI.clear();

    if (nofd_flag)
    {
        tmp_line = {COMMAND, PID, USER, "NOFD", "", "", "/proc/" + PID + "/fd", "(opendir: Permission denied)"};
        print(lines, tmp_line);
        return;
    }
    // 5. rwu
    vector<fd_info> FDI;
    get_rwu_info(PID, fd_files, FDI);
    for (int i = 0; i < fd_files.size(); i++)
    {
        tmp_line = {COMMAND, PID, USER, FDI[i].fd, FDI[i].type, FDI[i].inode, FDI[i].name, FDI[i].append};
        print(lines, tmp_line);
    }
    FDI.clear();
    fd_files.clear();
}

int main(int argc, char *argv[])
{
    bool arg_flag = 0;
    arguments parse_result = {0, "", 0, "", 0, ""};
    const char *opt = "c:t:f:";
    int o;
    while ((o = getopt(argc, argv, opt)) != -1)
    {
        switch (o)
        {
        case 'c':
            parse_result.c = 1;
            parse_result.c_expr = optarg;
            arg_flag = 1;
            break;
        case 't':
            if (string(optarg) != "REG" && string(optarg) != "CHR" && string(optarg) != "DIR" && string(optarg) != "FIFO" && string(optarg) != "SOCK" && string(optarg) != "unknown")
            {
                cerr << "Invalid TYPE option.\n";
                exit(1);
            }
            parse_result.t = 1;
            parse_result.t_expr = optarg;
            arg_flag = 1;
            break;
        case 'f':
            parse_result.f = 1;
            parse_result.f_expr = optarg;
            arg_flag = 1;
            break;
        case '?':
            cerr << "Error arguments.\n";
            exit(1);
        }
    }

    cout << "COMMAND\t\tPID\t\tUSER\t\tFD\t\tTYPE\t\tNODE\t\tNAME\n";
    get_all_process_files("/proc/", true);
    for (int i = 0; i < process_dir.size(); i++)
    {
        get_process_info(process_dir[i]);
    }
    print_all(lines, arg_flag, parse_result);

    return 0;
}
