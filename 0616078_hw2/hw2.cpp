#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
using namespace std;

void print_err()
{
    cerr << "usage: ./logger [-o file] [-p sopath] [--] cmd [cmd args ...]\n";
    cerr << "\t-p: set the path to logger.so, default = ./logger.so\n";
    cerr << "\t-o: print output to file, print to \"stderr\" if no file specified\n";
    cerr << "\t--: separate the arguments for logger and for the command\n";
}

int main(int argc, char *argv[])
{    
    // parse argc and argv
    int dash_idx = -1, log_cmd_argc = 0, cmd_argc = 0;
    char *log_argv[10], *cmd_argv[10];
    for(int i = 0; i < argc; i++)
    {
        if(!strcmp(argv[i], "--"))
        {
            dash_idx = i;
            break;
        }
        else
        {
            log_argv[i] = argv[i];
            log_cmd_argc++;
        }
    }

    // parse log_argv
    int opt = 0, is_out = 0, is_path = 0, log_argc = 0;
    string out_file, so_path;
    while((opt = getopt(log_cmd_argc, log_argv, "o:p:")) != -1)
    {
        switch(opt)
        {
            case 'o':
                is_out = 1;
                out_file = optarg;
                log_argc++;
                break;
            case 'p':
                is_path = 1;
                so_path = optarg;
                log_argc++;
                break;
            default:
                print_err();
                exit(1);
        }
    }
    if(is_path == 0)
        so_path = "./logger.so";
    if(is_out == 0)
    {
        int stderr_fd = open("/dev/fd/2", O_RDWR | O_APPEND);
        dup2(stderr_fd, 2);
    }
    else
    {
        int outfd = open(out_file.c_str(), O_RDWR | O_APPEND);
        dup2(outfd, 2);
    }
    setenv("LD_PRELOAD", so_path.c_str(), 1);

    // parse cmd index
    int cmd_idx = 0;
    if(dash_idx != -1)
        cmd_idx = dash_idx + 1;
    else
        cmd_idx = log_argc * 2 + 1;
    if(cmd_idx >= argc)
        cmd_idx = argc - 1;
    if(cmd_idx == 0)
    {
        print_err();
        exit(1);
    }

    // parse cmd arg
    cmd_argc = argc - cmd_idx;
    for(int i = 0; i < cmd_argc; i++)
        cmd_argv[i] = argv[cmd_idx + i];
    cmd_argv[cmd_argc] = (char*)NULL;
    
    execvp(argv[cmd_idx], cmd_argv);
}