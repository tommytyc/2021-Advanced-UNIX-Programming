#pragma once
#include <vector>
#include <string>
#include <string.h>
#include "open_elf.h"
using namespace std;

typedef enum
{
    LOADED,
    RUNNING,
    OTHERS
} state_t;

typedef enum
{
    ARGS,
    SCRIPT
} cmd_source_t;

typedef struct breakpoint
{
    int id;
    long long addr;
    string s_addr;
    char ori;
    breakpoint(int i=0, long long a=0, string s=" ", char o=0): id(i), addr(a), s_addr(s), ori(o){}
} breakpoint_t;

vector<string> parse_cmd(vector<string>* cmd, string line);
void parse_args(int argc, char** argv);
void error_quit(string msg);
void init_regs_map();
char replace_byte(long long adddr, char byte);
int check_stop();
string disassemble(char* pos, long long addr);
void getcode();

void breakpoint(string s_addr);     // DONE(NEED DISASM)          
void cont();                        // DONE
void del(string s_id);              // DONE
void disasm(string s_addr, int len);// DONE
void dump(string s_addr);           
void quit_exit();                   // DONE
void getreg(string reg);            // DONE
int getregs(bool print_flag);       // DONE
void help();                        // DONE
void list();                        // DONE
void load(const char* elfFile);     // DONE
void run();                         // DONE
void vmmap();                       // DONE
void set(string reg, string s_val); // DONE
void si();                          // DONE
void start();                       // DONE