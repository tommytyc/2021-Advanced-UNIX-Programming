#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <ctype.h>
#include <stdarg.h>
using namespace std;

typedef int (*or_chmod)(const char * path, mode_t mode);
typedef int (*or_chown)(const char *pathname, uid_t owner, gid_t group);
typedef int (*or_close)(int fd);
typedef int (*or_creat)(const char *pathname, mode_t mode);
typedef int (*or_creat64)(const char *pathname, mode_t mode);
typedef int (*or_fclose)(FILE *stream);
typedef FILE* (*or_fopen)(const char *filename, const char * mode);
typedef FILE* (*or_fopen64)(const char *filename, const char * mode);
typedef size_t (*or_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
typedef size_t (*or_fwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream);
typedef int (*or_open)(const char *pathname, int flags, ...);
typedef int (*or_open64)(const char *pathname, int flags, ...);
typedef ssize_t (*or_read)(int fd, void *buf, size_t count);
typedef int (*or_remove)(const char *pathname);
typedef int (*or_rename)(const char *oldpath, const char *newpath);
typedef FILE* (*or_tmpfile)(void);
typedef FILE* (*or_tmpfile64)(void);
typedef ssize_t (*or_write)(int fd, const void *buf, size_t count); //skip

void print(string str)
{
    or_write old_write = (or_write)dlsym(RTLD_NEXT, "write");
    old_write(2, str.c_str(), strlen(str.c_str()));
}

void print_ptr(void *p) {
    or_write old_write = (or_write)dlsym(RTLD_NEXT, "write");
    uintptr_t x = (uintptr_t)p;
    char buf[20];
    sprintf(buf, "%p", p);
    int size = 0;
    for(int i = 0; i < sizeof(buf); i++)
    {
        if(buf[i] == '\0')
        {
            size = i;
            break;
        }
    }
    old_write(2, buf, size);
}

void print_logger()
{
    or_write old_write = (or_write)dlsym(RTLD_NEXT, "write");
    string str = "[logger] ";
    old_write(2, str.c_str(), strlen(str.c_str()));
}

int chmod(const char * path, mode_t mode)
{
    or_chmod or_func = (or_chmod)dlsym(RTLD_NEXT, "chmod");
    string to_print = "chmod(";
    char long_path[100];
    char* exist = realpath(path, long_path);
    if(exist != NULL)
        to_print += "\"" + string(long_path) + "\", ";
    else
        to_print += "\"" + string(path) + "\", ";
    char mode_str[10];
    sprintf(mode_str, "%o", mode);
    to_print += string(mode_str) + ") = ";

    int ret = or_func(path, mode);
    print_logger();
    to_print += to_string(ret) + "\n";
    print(to_print);
    return ret;
}

int chown(const char *pathname, uid_t owner, gid_t group)
{
    or_chown or_func = (or_chown)dlsym(RTLD_NEXT, "chown");
    string to_print = "chown(";
    char long_path[100] = {0};
    char* exist = realpath(pathname, long_path);
    if(exist != NULL)
        to_print += "\"" + string(long_path) + "\", ";
    else
        to_print += "\"" + string(pathname) + "\", ";
    to_print += to_string(owner) + ", " + to_string(group);
    int ret = or_func(pathname, owner, group);
    print_logger();
    to_print += ") = " + to_string(ret) + "\n";
    print(to_print);
    return ret;
}

int close(int fd)
{
    or_close or_func = (or_close)dlsym(RTLD_NEXT, "close");
    string to_print = "close(";
    string str_fd = to_string(fd), path;
    char buf[1024];
    int len = readlink(string("/proc/self/fd/" + str_fd).c_str(), buf, 1023);
    if(len != -1)
    {
        buf[len] = '\0';
        path = string(buf);
    }

    to_print += "\"" + path + "\") = ";

    int ret = or_func(fd);

    print_logger();
    to_print += to_string(ret) + "\n";
    print(to_print);
    return ret;
}

int creat(const char *pathname, mode_t mode)
{
    or_creat or_func = (or_creat)dlsym(RTLD_NEXT, "creat");
    string to_print = "creat(";
    char long_path[100];
    char* exist = realpath(pathname, long_path);
    if(exist != NULL)
        to_print += "\"" + string(long_path) + "\", ";
    else
        to_print += "\"" + string(pathname) + "\", ";
    char mode_str[10];
    sprintf(mode_str, "%o", mode);
    to_print += string(mode_str) + ") = ";

    int ret = or_func(pathname, mode);
    print_logger();
    to_print += to_string(ret) + "\n";
    print(to_print);
    return ret;
}

int creat64(const char *pathname, mode_t mode)
{
    or_creat64 or_func = (or_creat64)dlsym(RTLD_NEXT, "creat64");
    string to_print = "creat64(";
    char long_path[100];
    char* exist = realpath(pathname, long_path);
    if(exist != NULL)
        to_print += "\"" + string(long_path) + "\", ";
    else
        to_print += "\"" + string(pathname) + "\", ";
    char mode_str[10];
    sprintf(mode_str, "%o", mode);
    to_print += string(mode_str) + ") = ";

    int ret = or_func(pathname, mode);
    print_logger();
    to_print += to_string(ret) + "\n";
    print(to_print);
    return ret;
}

int fclose(FILE *stream)
{
    or_fclose or_func = (or_fclose)dlsym(RTLD_NEXT, "fclose");
    string to_print = "fclose(\"";
    int fd = fileno(stream);
    char buf[1024];
    string path;
    int len = readlink(string("/proc/self/fd/" + to_string(fd)).c_str(), buf, 1023);
    if(len != -1)
    {
        buf[len] = '\0';
        path = string(buf);
    }
    to_print += path + "\") = ";
    int ret = or_func(stream);
    print_logger();
    to_print += to_string(ret) + "\n";
    print(to_print);
    return ret;
}

FILE* fopen(const char *filename, const char * mode)
{
    or_fopen or_func = (or_fopen)dlsym(RTLD_NEXT, "fopen");
    string to_print = "fopen(";
    char long_path[100] = {0};
    char* exist = realpath(filename, long_path);
    if(exist != NULL)
        to_print += "\"" + string(long_path) + "\", ";
    else
        to_print += "\"" + string(filename) + "\", ";
    to_print += "\"" + string(mode) + "\") = ";

    FILE* ret = or_func(filename, mode);
    print_logger();
    print(to_print);
    print_ptr(ret);
    print("\n");
    return ret;
}

FILE* fopen64(const char *filename, const char * mode)
{
    or_fopen64 or_func = (or_fopen64)dlsym(RTLD_NEXT, "fopen64");
    string to_print = "fopen64(";
    char long_path[100] = {0};
    char* exist = realpath(filename, long_path);
    if(exist != NULL)
        to_print += "\"" + string(long_path) + "\", ";
    else
        to_print += "\"" + string(filename) + "\", ";
    to_print += "\"" + string(mode) + "\") = ";

    FILE* ret = or_func(filename, mode);
    print_logger();
    print(to_print);
    print_ptr(ret);
    print("\n");
    return ret;
}

ssize_t read(int fd, void *buf, size_t count)
{
    or_read or_func = (or_read)dlsym(RTLD_NEXT, "read");
    string to_print = "read(";
    string str_fd = to_string(fd), path;
    char buff[1024];
    int len = readlink(string("/proc/self/fd/" + str_fd).c_str(), buff, 1023);
    if(len != -1)
    {
        buff[len] = '\0';
        path = string(buff);
    }
    to_print += "\"" + path + "\", \"";

    ssize_t ret = or_func(fd, buf, count);
    print_logger();

    char buff2[100];
    int size = 32;
    if(count < size)
        size = count;
    memcpy(buff2, buf, size);
    for(int i = 0; i < strlen(buff2); i++)
    {
        int tmp = (int)buff2[i];
        if(isprint(tmp))
        {
            to_print += buff2[i];
        }
        else
            to_print += '.';
    }

    to_print += "\", " + to_string(count) + ") = ";
    print(to_print + to_string(ret) + "\n");
    return ret;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    or_fread or_func = (or_fread)dlsym(RTLD_NEXT, "fread");
    string to_print = "fread(\"";

    int fd = fileno(stream);
    char buf[1024];
    string path;
    int len = readlink(string("/proc/self/fd/" + to_string(fd)).c_str(), buf, 1023);
    if(len != -1)
    {
        buf[len] = '\0';
        path = string(buf);
    }
    
    size_t ret = or_func(ptr, size, nmemb, stream);
    
    print_logger();
    char buff[100];
    int new_size = 32;
    if(ret * size < new_size)
        new_size = ret * size;
    memcpy(buff, ptr, new_size);
    for(int i = 0; i < strlen(buff); i++)
    {
        int tmp = (int)buff[i];
        if(isprint(tmp))
        {
            to_print += buff[i];
        }
        else
            to_print += '.';
    }
    to_print += "\", " + to_string(size) + ", " + to_string(nmemb) + ", \"";
    
    to_print += path + "\") = ";
    to_print += to_string(ret) + "\n";
    print(to_print);
    return ret;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    or_fwrite or_func = (or_fwrite)dlsym(RTLD_NEXT, "fwrite");
    string to_print = "fwrite(";

    int fd = fileno(stream);
    char buf[1024];
    string path;
    int len = readlink(string("/proc/self/fd/" + to_string(fd)).c_str(), buf, 1023);
    if(len != -1)
    {
        buf[len] = '\0';
        path = string(buf);
    }

    size_t ret = or_func(ptr, size, nmemb, stream);

    print_logger();
    char buff[100];
    int new_size = 32;
    if(ret * size < new_size)
        new_size = ret * size;
    memcpy(buff, ptr, new_size);
    for(int i = 0; i < strlen(buff); i++)
    {
        int tmp = (int)buff[i];
        if(isprint(tmp))
        {
            to_print += buff[i];
        }
        else
            to_print += '.';
    }
    to_print += "\", " + to_string(size) + ", " + to_string(nmemb) + ", \"";
    
    to_print += path + "\") = ";
    to_print += to_string(ret) + "\n";
    print(to_print);
    return ret;
}

int open(const char *pathname, int flags, ...)
{
    mode_t mode = 0;
    or_open or_func = (or_open)dlsym(RTLD_NEXT, "open");
    string to_print = "open(";
    int ret = 0;
    char long_path[100] = {0}, mode_str[10] = {0};
    char* exist = realpath(pathname, long_path);
    if(exist != NULL)
        to_print += "\"" + string(long_path) + "\", ";
    else
        to_print += "\"" + string(pathname) + "\", ";
    to_print += to_string(flags);    
    
    if(__OPEN_NEEDS_MODE (flags))
    {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, int);
        va_end(args);
        sprintf(mode_str, "%o", mode);
        ret = or_func(pathname, flags, mode);
    }
    else
    {
        ret = or_func(pathname, flags);
        sprintf(mode_str, "%o", mode);
    }
    to_print += ", " + string(mode_str) + ") = ";

    print_logger();
    to_print += to_string(ret) + "\n";
    print(to_print);
    return ret;
}

int open64(const char *pathname, int flags, ...)
{
    mode_t mode = 0;
    or_open or_func = (or_open)dlsym(RTLD_NEXT, "open");
    string to_print = "open(";
    int ret = 0;
    char long_path[100] = {0}, mode_str[10] = {0};
    char* exist = realpath(pathname, long_path);
    if(exist != NULL)
        to_print += "\"" + string(long_path) + "\", ";
    else
        to_print += "\"" + string(pathname) + "\", ";
    to_print += to_string(flags);    
    
    if(__OPEN_NEEDS_MODE (flags))
    {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, int);
        va_end(args);
        sprintf(mode_str, "%o", mode);
        ret = or_func(pathname, flags, mode);
    }
    else
    {
        ret = or_func(pathname, flags);
        sprintf(mode_str, "%o", mode);
    }
    to_print += ", " + string(mode_str) + ") = ";

    print_logger();
    to_print += to_string(ret) + "\n";
    print(to_print);
    return ret;
}

int remove(const char *pathname)
{
    or_remove or_func = (or_remove)dlsym(RTLD_NEXT, "remove");
    string to_print = "remove(";
    char long_path[100];
    char* exist = realpath(pathname, long_path);
    if(exist != NULL)
        to_print += "\"" + string(long_path) + "\") = ";
    else
        to_print += "\"" + string(pathname) + "\") = ";

    int ret = or_func(pathname);
    print_logger();
    to_print += to_string(ret) + "\n";
    print(to_print);
    return ret;
}

int rename(const char *oldpath, const char *newpath)
{
    or_rename or_func = (or_rename)dlsym(RTLD_NEXT, "rename");
    string to_print = "rename(\"";
    char long_path[100];
    char* exist = realpath(oldpath, long_path);
    if(exist != NULL)
        to_print += string(long_path) + "\",";
    else
        to_print += string(oldpath) + "\", ";
    
    char long_path2[100];
    exist = realpath(newpath, long_path2);
    if(exist != NULL)
        to_print += "\"" + string(long_path2) + "\") = ";
    else
        to_print += "\"" + string(newpath) + "\") = ";

    int ret = or_func(oldpath, newpath);
    print_logger();
    print(to_print + to_string(ret) + "\n");
    return ret;
}

FILE* tmpfile(void)
{
    or_tmpfile or_func = (or_tmpfile)dlsym(RTLD_NEXT, "tmpfile");
    string to_print = "tmpfile() = ";
    FILE* ret = or_func();
    print_logger();
    print(to_print);
    print_ptr(ret);
    print("\n");
    return ret;
}

FILE* tmpfile64(void)
{
    or_tmpfile64 or_func = (or_tmpfile64)dlsym(RTLD_NEXT, "tmpfile64");
    string to_print = "tmpfile64() = ";
    FILE* ret = or_func();
    print_logger();
    print(to_print);
    print_ptr(ret);
    print("\n");
    return ret;
}

ssize_t write(int fd, const void *buf, size_t count)
{
    or_write old_write = (or_write)dlsym(RTLD_NEXT, "write");
    string to_print = "write(";
    string str_fd = to_string(fd), path;
    char buff[1024];
    int len = readlink(string("/proc/self/fd/" + str_fd).c_str(), buff, 1023);
    if(len != -1)
    {
        buff[len] = '\0';
        path = string(buff);
    }
    to_print += "\"" + path + "\", \"";

    ssize_t ret = old_write(fd, buf, count);
    print_logger();

    char buff2[100];
    int size = 32;
    if(count < size)
        size = count;
    memcpy(buff2, buf, size);
    for(int i = 0; i < strlen(buff2); i++)
    {
        int tmp = (int)buff2[i];
        if(isprint(tmp))
        {
            to_print += buff2[i];
        }
        else
            to_print += '.';
    }

    to_print += "\", " + to_string(count) + ") = ";
    print(to_print + to_string(ret) + "\n");
    return ret;
}