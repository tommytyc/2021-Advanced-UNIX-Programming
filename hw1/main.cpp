#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/types.h>
#include <errno.h>
#include <vector>
#include <unistd.h>
#include "utils.h"
#define BUF_SIZE 1024

using namespace std;

vector<path_pid> process_dir;

void get_command(string* COMMAND, string PID)
{
    ifstream fin;
    char buff[BUF_SIZE];
    string dir = string("/proc/" + PID + "/comm");
    fin.open(dir.c_str(), ios::in);
    if(!fin.is_open()){
        cout << "[ERROR: " << errno << " ] Couldn't open " << dir << "." << endl;
        return;
    }
    fin.read(buff, sizeof(buff));
    regex newline("\n+");
    auto result = regex_replace(string(buff), newline, "");
    *COMMAND = result;
}

void get_user(string* USER, string PID)
{
    uid_t UID = 0;
    struct stat fileInfo;
    stat(string("/proc/" + PID + "/exe").c_str(), &fileInfo);
    UID = fileInfo.st_uid;
    struct passwd *user;
    user = getpwuid(UID);
    *USER = user->pw_name;
}

void get_inode(string* NODE, string dir)
{
    struct stat fileinfo;
    stat(dir.c_str(), &fileinfo);
    *NODE = to_string(fileinfo.st_ino);
}

void get_cwd_info(string* NAME, string* NODE, string PID)
{
    char buff[1024];
    string dir = string("/proc/" + PID + "/cwd");
    ssize_t len = readlink(dir.c_str(), buff, sizeof(buff) - 1);
    if (len != -1)
    {
        buff[len] = '\0';
        *NAME = string(buff);
    }
    get_inode(NODE, dir);
}
void get_root_info(string* NAME, string* NODE, string PID)
{
    char buff[1024];
    string dir = string("/proc/" + PID + "/root");
    ssize_t len = readlink(dir.c_str(), buff, sizeof(buff) - 1);
    if (len != -1)
    {
        buff[len] = '\0';
        *NAME = string(buff);
    }
    get_inode(NODE, dir);
}
void get_exe_info(string *NAME, string* NODE, string PID)
{
    char buff[1024];
    string dir = string("/proc/" + PID + "/exe");
    ssize_t len = readlink(dir.c_str(), buff, sizeof(buff) - 1);
    if (len != -1)
    {
        buff[len] = '\0';
        *NAME = string(buff);
    }
    get_inode(NODE, dir);
}
void get_mem_info(vector<string> &files)
{

}
void get_del_info(vector<string> &files)
{

}
void get_rwu_info(vector<string> &files)
{

}
void get_nofd_info(vector<string> &files)
{

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
    string COMMAND = "", PID = dir.pid, USER = "", FD = "", TYPE = "", NODE = "", NAME = "";
    DIR *dp;
    struct dirent *dirp;
    vector<string> files;
    if ((dp = opendir(dir.path.c_str())) == NULL)
    {
        print("[ERROR: " + to_string(errno) + " ] Couldn't open " + dir.path + ".");
        return;
    }
    else
    {
        while ((dirp = readdir(dp)) != NULL)
        {
            if (dirp->d_name != string(".") && dirp->d_name != string(".."))
            {
                files.push_back(dirp->d_name);
            }
        }
        closedir(dp);
    }

    get_command(&COMMAND, PID);
    get_user(&USER, PID);
    // six steps to deal with: cwd, root, exe, mem, (del,) rwu, nofd
    // 1. cwd
    get_cwd_info(&NAME, &NODE, PID);
    print(COMMAND + "\t\t\t" + PID + "\t\t" + USER + "\t\tcwd\t\t" + TYPE + "\t\t" + NODE + "\t\t" + NAME);

    // 2. root
    get_root_info(&NAME, &NODE, PID);
    print(COMMAND + "\t\t\t" + PID + "\t\t" + USER + "\t\troot\t\t" + TYPE + "\t\t" + NODE + "\t\t" + NAME);

    // 3. exe
    get_exe_info(&NAME, &NODE, PID);
    print(COMMAND + "\t\t\t" + PID + "\t\t" + USER + "\t\texe\t\t" + TYPE + "\t\t" + NODE + "\t\t" + NAME);
    
    // 4. mem
    // get_mem_info();

    // 5. del
    // get_del_info();

    // 6. rwu
    // get_rwu_info();

    // nofd
    // get_nofd_info();
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