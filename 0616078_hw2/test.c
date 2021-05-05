#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

int main()
{
    char buf[100] = {0};
    creat("test.txt", 0600);
    chmod("test.txt", 0666);
    chown("test.txt", 65534, 65534);
    rename("test.txt", "test2.txt");
    int fd = open("test2.txt", O_APPEND | O_RDWR, 0666);
    write(fd, "cccc\n", 5);
    close(fd);
    fd = open("test2.txt", O_RDONLY, 00);
    read(fd, buf, 100);
    close(fd);
    FILE *f = tmpfile();
    fwrite("12345678901234567890123456789012345\n", 1, 5, f);
    fclose(f);
    f = fopen("test2.txt", "r");
    char buf2[100] = {0};
    fread(buf2, 1, 100, f);
    fclose(f);
    remove("test2.txt");
    printf("sample done.\n");
    return 0;
}