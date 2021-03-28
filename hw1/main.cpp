#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <vector>

using namespace std;

vector<string> proc_dir;

bool isDir(string dir)
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

void listFiles(string baseDir, bool recursive)
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
                if (isDir(baseDir + dirp->d_name) == true && recursive == true)
                {
                    cout << "[DIR]\t" << baseDir << dirp->d_name << "/" << endl;
                    listFiles(baseDir + dirp->d_name + "/", true);
                }
                else
                {
                    cout << "[FILE]\t" << baseDir << dirp->d_name << endl;
                }
            }
        }
        closedir(dp);
    }
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        cout << "[WARNING] At least one argument ( path ) expected .. exiting." << endl;
        return 1;
    }
    else
    {
        listFiles(argv[1], true);
    }

    return 0;
}