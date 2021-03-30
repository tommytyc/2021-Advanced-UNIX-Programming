#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include "utils.h"
#define BUF_SIZE 1024

using namespace std;

vector<path_pid> process_dir;

void get_command(string* COMMAND, string PID)
{
    char buff[BUF_SIZE];
    FILE* fp = fopen(string("/proc/" + PID + "/status").c_str(), "r");
    if (fp != NULL)
    {
        if( fgets(buff, BUF_SIZE-1, fp)== NULL ){
            fclose(fp);
        }
        fclose(fp);
        string tmp_str = string(buff);
        tmp_str = tmp_str.substr(6);
        tmp_str = tmp_str.substr(0, tmp_str.length() - 1);
        *COMMAND = tmp_str;
    }
}

void get_cwd_info(string* FD)
{
    
}
void get_root_info(vector<string> &files)
{

}
void get_exe_info(string *NAME, string PID)
{
    char buff[1024];
    ssize_t len = readlink(string("/proc/" + PID + "/exe").c_str(), buff, sizeof(buff) - 1);
    if (len != -1)
    {
        buff[len] = '\0';
        *NAME = string(buff);
    }
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
    // six steps to deal with: cwd, root, exe, mem, (del,) rwu, nofd
    // 1. cwd
    // get_cwd_info();

    // 2. root
    // get_root_info();

    // 3. exe
    get_exe_info(&NAME, PID);
    print(COMMAND + "\t\t\t" + PID + "\t\t\t\texe\t\t" + TYPE + "\t\t\t" + NAME);
    
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