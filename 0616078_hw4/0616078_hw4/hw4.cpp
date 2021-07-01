#include <sys/ptrace.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <capstone/capstone.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include "hw4.h"
using namespace std;

elfhandle_t* e = (elfhandle_t*)calloc(1, sizeof(elfhandle_t));
vector<string> cmd;
string program;
state_t state = OTHERS;
cmd_source_t source = ARGS;
ifstream fin;
bool file_flag = 0;
pid_t child = 0;
struct user_regs_struct regs = {0};
map<string, unsigned long long*> regs_map;
map<string, unsigned long long*>::iterator iter;
Elf64_Shdr text;
vector<breakpoint_t> points;
int bpid = 0;
char* code = NULL;
char restore = 0;
long long dumpaddr = -1, bpaddr_now = 0;
unsigned long long rip_now = 0;

void init_regs_map()
{
    regs_map["rax"] = 0;
    regs_map["rbx"] = 0;
    regs_map["rcx"] = 0;
    regs_map["rdx"] = 0;
    regs_map["r8"] = 0;
    regs_map["r9"] = 0;
    regs_map["r10"] = 0;
    regs_map["r11"] = 0;
    regs_map["r12"] = 0;
    regs_map["r13"] = 0;
    regs_map["r14"] = 0;
    regs_map["r15"] = 0;
    regs_map["rdi"] = 0;
    regs_map["rsi"] = 0;
    regs_map["rbp"] = 0;
    regs_map["rsp"] = 0;
    regs_map["rip"] = 0;
    regs_map["flags"] = 0;
}

vector<string> parse_cmd(string line)
{
    vector<string> v;
    string tmp;
    stringstream tt;
    tt << line;
    while(tt >> tmp)
        v.push_back(tmp);
    return v;
}

void error_quit(string msg)
{
    cout << msg << endl;
    elf_close(e);
    exit(-1);
}

void parse_args(int argc, char** argv)
{
    if(argc >= 2)
    {
        if(string(argv[1]) == "-s")
        {
            source = SCRIPT;
            fin.open(argv[2]);
            file_flag = 1;
            if(argc > 3)
            {
                load(argv[3]);
            }
        }
        else
        {
            load(argv[1]);
        }
    }
    return;
}

void getcode()
{
    ifstream f(program.c_str(), ios::in | ios::binary | ios::ate);
    streampos size;
    size = f.tellg();
    int codesz = size + 1L;
    code = new char [codesz];
    f.seekg(0, ios::beg);
    f.read(code, size);
    code[size] = 0;
    f.close();
}

string disassemble(char* pos, long long* addr)
{
    csh cshandle = 0;
    cs_insn *insn;
    size_t count;
    string inst = "";
    if(cs_open(CS_ARCH_X86, CS_MODE_64, &cshandle) != CS_ERR_OK)
        error_quit("** cs error.");
    if((count = cs_disasm(cshandle, (uint8_t*)pos, 256, *addr, 0, &insn)) > 0)
    {
        stringstream ss;
        string toprint, tmp;
        for(int i = 0; i < insn[0].size; i++)
        {
            stringstream sss;
            int val = int(insn[0].bytes[i]);
            sss << hex << setw(2) << setfill('0') << val;
            sss >> tmp;
            toprint += tmp + " ";
        }
        ss << "\t" << hex << insn[0].address << dec << ": " << hex << toprint
             << dec << "\t\t" << insn[0].mnemonic << "\t" << insn[0].op_str << endl;
        *addr += insn[0].size;
        inst = ss.str();
        cs_free(insn, count);
    }
    else
        error_quit("** cs error.");
    cs_close(&cshandle);
    return inst;
}

char replace_byte(long long addr, char byte)
{
    long long word = ptrace(PTRACE_PEEKTEXT, child, addr, 0);
    ptrace(PTRACE_POKETEXT, child, addr, (word & 0xffffffffffffff00) | (byte & 0xff));
    return word & 0xff;
}

// 1: cont/si at same break, need to replace 0xcc -> step -> replace 0xcc 
// 0: first time at this break
int check_stop()
{
    int status;
    if(waitpid(child, &status, 0) < 0)
        error_quit("** wait error.");
    if(WIFSTOPPED(status))
    {
        getregs(false);
        long long rip = *regs_map["rip"];
        for(int i = 0; i < int(points.size()); i++)
        {
            if(points[i].addr == (rip - 1))
            {
                if(rip_now == points[i].addr)
                {
                    *regs_map["rip"] = (rip - 1);
                    ptrace(PTRACE_SETREGS, child, 0, &regs);
                    restore = points[i].ori;
                    return 1;
                }
                
                stringstream ss;
                string tmp;
                bpaddr_now = points[i].addr;
                cout << "** breakpoint @\t";
                ss << hex << points[i].addr;
                tmp = ss.str();
                disasm(tmp, 1);
                *regs_map["rip"] = (rip - 1);
                rip_now = (rip - 1);
                ptrace(PTRACE_SETREGS, child, 0, &regs);
                return 0;
            }
        }
        return -1;
    }
    else if(WIFEXITED(status))
    {
        cout << "** child process " << child << " " << "terminated normally (code " << status << ")" << endl;
        state = LOADED;
        return -1;
    }
    return -1;
}

void help()
{
    cout << "- break {instruction-address}: add a break point" << endl;
    cout << "- cont: continue execution" << endl;
    cout << "- delete {break-point-id}: remove a break point" << endl;
    cout << "- disasm addr: disassemble instructions in a file or a memory region" << endl;
    cout << "- dump addr [length]: dump memory content" << endl;
    cout << "- exit: terminate the debugger" << endl;
    cout << "- get reg: get a single value from a register" << endl;
    cout << "- getregs: show registers" << endl;
    cout << "- help: show this message" << endl;
    cout << "- list: list break points" << endl;
    cout << "- load {path/to/a/program}: load a program" << endl;
    cout << "- run: run the program" << endl;
    cout << "- vmmap: show memory layout" << endl;
    cout << "- set reg val: get a single value to a register" << endl;
    cout << "- si: step into instruction" << endl;
    cout << "- start: start the program and stop at the first instruction" << endl;
}

void load(const char* elfFile)
{
    if(state == LOADED)
    {
        cout << "** program has already been loaded." << endl;
        return;
    }
    int ret = 0;
    ret = elf_open(e, elfFile);
    if(!ret)
    {
        elf_load(e);
        state = LOADED;
        cout << "** program '" << elfFile << "' loaded. entry point 0x" << hex << e->entry << dec << endl;
        program = elfFile;
    }
    else
    {
        error_quit("** unable to load '" + string(elfFile) + "'.");
    }
    strtab_t* tab;
    for(tab = e->strtab; tab != NULL; tab = tab->next)
        if(tab->id == e->sh_stridx)
            break;
    for(int i = 0; i < e->sh_cnt; i++)
        if(!strcmp(&tab->data[e->shdr[i].sh_name], ".text"))
        {
            text = e->shdr[i];
            break;
        }
}

int getregs(bool print_flag)
{
    if(state != RUNNING)
    {
        cout << "** program is not running." << endl;
        return -1;
    }
    if(ptrace(PTRACE_GETREGS, child, 0, &regs) < 0)
        error_quit("** ptrace error.");
    
    if(print_flag)
    {
        cout << hex;
        cout << "RAX " << regs.rax << "\t\tRBX " << regs.rbx << "\t\tRCX " << regs.rcx << "\t\tRDX " << regs.rdx << endl;
        cout << "R8 "  << regs.r8  << "\t\tR9 "  << regs.r9  << "\t\tR10 " << regs.r10 << "\t\tR11 " << regs.r11 << endl;
        cout << "R12 " << regs.r12 << "\t\tR13 " << regs.r13 << "\t\tR14 " << regs.r14 << "\t\tR15 " << regs.r15 << endl;
        cout << "RDI " << regs.rdi << "\t\tRSI " << regs.rsi << "\t\tRBP " << regs.rbp << "\t\tRSP " << regs.rsp << endl;
        ios::fmtflags f(cout.flags());
        cout << "RIP " << regs.rip << "\t\tFLAGS " << setw(16) << setfill('0') << regs.eflags << endl;
        cout.flags(f);
        cout << dec;
    }
    
    regs_map["rax"] = &regs.rax;
    regs_map["rbx"] = &regs.rbx;
    regs_map["rcx"] = &regs.rcx;
    regs_map["rdx"] = &regs.rdx;
    regs_map["r8"] = &regs.r8;
    regs_map["r9"] = &regs.r9;
    regs_map["r10"] = &regs.r10;
    regs_map["r11"] = &regs.r11;
    regs_map["r12"] = &regs.r12;
    regs_map["r13"] = &regs.r13;
    regs_map["r14"] = &regs.r14;
    regs_map["r15"] = &regs.r15;
    regs_map["rdi"] = &regs.rdi;
    regs_map["rsi"] = &regs.rsi;
    regs_map["rbp"] = &regs.rbp;
    regs_map["rsp"] = &regs.rsp;
    regs_map["rip"] = &regs.rip;
    regs_map["flags"] = &regs.eflags;
    return 0;
}

void getreg(string reg)
{
    int ret = getregs(false);
    if(ret)
        return;
    if(reg == "QQ")
        return;
    iter = regs_map.find(reg);
    if(iter != regs_map.end())
        cout << reg << " = " << *iter->second << " (0x" << hex << *iter->second << dec << ")\n";
    else
        cout << "** no such register." << endl;
}

void set(string reg, string s_val)
{
    if(state != RUNNING)
    {
        cout << "** program is not running." << endl;
        return;
    }
    if(s_val == "QQ")
        return;
    int ret = getregs(false);
    if(ret)
        return;
    long long val = strtoll(s_val.c_str(), NULL, 16);
    *regs_map[reg] = val;
    if(reg == "rip")
        rip_now = val;
    ptrace(PTRACE_SETREGS, child, 0, &regs);
}

void vmmap()
{
    if(state != RUNNING)
    {
        cout << "** program is not running." << endl;
        return;
    }
    ifstream f("/proc/" + to_string(child) + "/maps");
    string tmp;
    while(getline(f, tmp))
    {
        stringstream s(tmp);
        string col;
        s >> col;              // addr
        size_t pos = col.find("-");
        while(pos != col.npos)
        {
            col.replace(pos, 1, " ");
            pos = col.find("-");
        }
        stringstream addrs;
        string addr1, addr2;
        addrs << col;
        addrs >> addr1;
        addrs >> addr2;
        ios::fmtflags f(cout.flags());
        cout << setw(16) << setfill('0') << addr1 << "-" << setw(16) << setfill('0') << addr2 << "\t";
        cout.flags(f);
        s >> col; col.erase(remove(col.begin(), col.end(), 'p'));cout << col << "\t";  // perm
        s >> col; s >> col;             // trash
        s >> col; cout << col << "\t";  // inode
        s >> col; cout << col << endl;  // path
    }
}

void dump(string s_addr)
{
    if(state != RUNNING)
    {
        cout << "** program is not running." << endl;
        return;
    }
    if(s_addr != "NO")
        dumpaddr = strtoll((s_addr.c_str()), NULL, 16);
    else
    {
        if(s_addr == "QQ")
        {
            cout << "** no addr is given." << endl;
            return;
        }
    }
    long long end = text.sh_addr + text.sh_size;
    string toprint = "", bytes = "", content = "";  // content will handle endline
    int line = 5, print_cnt = 0, length = end - dumpaddr;
    if(end - dumpaddr >= 80)
        line = 5;
    else
        line = (end - dumpaddr) / 16 + 1;
    for(int i = 0; i < line; i++)
    {
        cout << "\t" << hex << dumpaddr << dec << ": ";
        bytes = ""; content = "|", toprint = "";
        for(int j = 0; j < 2; j++)
        {
            long long word = ptrace(PTRACE_PEEKTEXT, child, dumpaddr, 0);
            bytes += string((char*)&word, 8);
            dumpaddr += 8;
        }
        unsigned char* c = (unsigned char*)bytes.c_str();
        for(int i = 0; i < 16; i++)
        {
            if(print_cnt >= length)
            {
                toprint += "   ";
                content += " ";
            }
            else
            {
                string tmp;
                stringstream ss;
                int val = int(*(c + i));
                ss << hex << setw(2) << setfill('0') << val;
                ss >> tmp;
                toprint += tmp + " ";
                if(isprint(val))
                    content += *(c + i);
                else
                    content += ".";
                print_cnt++;
            }
        }
        content += "|\n";
        toprint += "\t" + content;
        cout << toprint;
    }
    
}

void disasm(string s_addr, int len)
{
    if(state != RUNNING)
    {
        cout << "** program is not running." << endl;
        return;
    }
    if(s_addr == "QQ")
    {
        cout << "** no addr is given." << endl;
        return;
    }
    if(code == NULL)
        getcode();
    long long addr = strtoll((s_addr.c_str()), NULL, 16);
    for(int i = 0; i < len; i++)
    {
        if(addr >= (text.sh_addr + text.sh_size))
            break;
        long long offset = text.sh_offset + addr - text.sh_addr;
        char* pos = code + offset;
        cout << disassemble(pos, &addr);
    }
}

void start()
{
    if(state == OTHERS)
    {
        cout << "** not in loaded state." << endl;
        return;
    }
    else if(state == RUNNING)
    {
        kill(child, SIGTERM);
        child = 0;
    }
    points.clear();
    child = fork();
    if(child < 0)
        error_quit("** fork error.");
    else if(child == 0)
    {
        if(ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
            error_quit("** ptrace error.");
        char* const argv[] = {NULL};
        if(program.c_str()[0] != '.' && program.c_str()[1] != '/')
        {
            string tmp = "./" + program;
            program = tmp;
        }
        execvp(program.c_str(), argv);
        error_quit("** execvp error.");
    }
    else
    {
        int status;
        if(waitpid(child, &status, 0) < 0)
            error_quit("** wait error.");
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL);          

        cout << "** pid " << child << endl;
        state = RUNNING;
    }
}

void run()
{
    if(state == RUNNING)
    {
        cout << "** program " << program << " is already running." << endl;
        cont();
        return;
    }
    else if(state == LOADED)
    {
        start();
        cont();
    }
    else
    {
        cout << "** not in loaded or running state." << endl;
        return;
    }
}

void cont()
{
    if(state != RUNNING)
    {
        cout << "** program is not running." << endl;
        return;
    }
    ptrace(PTRACE_CONT, child, 0, 0);
    int ret = check_stop();
    if(ret == 1)
    {
        long long addr = bpaddr_now;
        replace_byte(addr, restore);
        ptrace(PTRACE_CONT, child, 0, 0);
        replace_byte(addr, 0xcc);
        bpaddr_now = 0;
        check_stop();
        replace_byte(addr, 0xcc);
    }
    
}

void si()
{
    if(state != RUNNING)
    {
        cout << "** program is not running." << endl;
        return;
    }
    ptrace(PTRACE_SINGLESTEP, child, 0, 0);
    int ret = check_stop();
    if(ret == 1)
    {
        long long addr = bpaddr_now;
        replace_byte(addr, restore);
        ptrace(PTRACE_SINGLESTEP, child, 0, 0);
        bpaddr_now = 0;
        check_stop();
        replace_byte(addr, 0xcc);
    }
}

void list()
{
    for(int i = 0; i < int(points.size()); i++)
        cout << "  " << points[i].id << ":  " << hex << points[i].addr << dec << endl;
}

void breakpoint(string s_addr)
{
    if(state != RUNNING)
    {
        cout << "** program is not running." << endl;
        return;
    }
    long long addr = strtoll((s_addr.c_str()), NULL, 16);
    char ori = replace_byte(addr, 0xcc);
    for(int i = 0; i < int(points.size()); i++)
        if(points[i].addr == addr)
            return;
    breakpoint_t tmp;
    tmp.id = bpid++;
    tmp.addr = addr;
    tmp.s_addr = s_addr;
    tmp.ori = ori;
    points.push_back(tmp);
}

void del(string s_id)
{
    if(state != RUNNING)
    {
        cout << "** program is not running." << endl;
        return;
    }
    if(s_id == "QQ")
        return;
    int id = stoi(s_id);
    for(int i = 0; i < int(points.size()); i++)
    {
        if(points[i].id == id)
        {
            replace_byte(points[i].addr, points[i].ori);
            points.erase(points.begin() + i);
            cout << "** breakpoint " << s_id << " deleted." << endl;
            return;
        }
    }
    cout << "** no such breakpoint." << endl;
}

void quit_exit()
{
    if(child != 0)
        kill(child, SIGTERM);
    elf_close(e);
}

int main(int argc, char** argv)
{
    init_regs_map();
    bool eof_flag = false;
    parse_args(argc, argv);

    while(1)
    {
        if(!file_flag)
            cout << "sdb> ";
        string line;
        if(file_flag)
        {
            if(!getline(fin, line))
                eof_flag = true;
        }
        else
        {
            if(!getline(cin, line))
                eof_flag = true;
        }
        if(eof_flag)
            break;
        cmd = parse_cmd(line);
        if(cmd.empty())
            continue;
        else if(cmd[0] == "load" && cmd.size() == 2)
            load(cmd[1].c_str());
        else if(cmd[0] == "delete")
        {
            if(cmd.size() == 2)
                del(cmd[1]);
            else
                del("QQ");
        }   
        else if((cmd[0] == "help" || cmd[0] == "h") && cmd.size() == 1)
            help();
        else if((cmd[0] == "exit" || cmd[0] == "q") && cmd.size() == 1)
            break;
        else if((cmd[0] == "vmmap" || cmd[0] == "m") && cmd.size() == 1)
            vmmap();
        else if((cmd[0] == "get" || cmd[0] == "g"))
        {
            if(cmd.size() == 2)
                getreg(cmd[1]);
            else
                getreg("QQ");
        }
        else if((cmd[0] == "set" || cmd[0] == "s"))
        {
            if(cmd.size() == 3)
                set(cmd[1], cmd[2]);
            else
                set("QQ", "QQ");
        }
        else if((cmd[0] == "break" || cmd[0] == "b"))
        {
            if(cmd.size() == 2)
                breakpoint(cmd[1]);
            else
                breakpoint("QQ");
        }        
        else if((cmd[0] == "list" || cmd[0] == "l") && cmd.size() == 1)
            list();
        else if((cmd[0] == "cont" || cmd[0] == "c") && cmd.size() == 1)
            cont();
        else if((cmd[0] == "dump" || cmd[0] == "x"))
        {
            if(cmd.size() == 2)
                dump(cmd[1]);
            else if(cmd.size() == 1)
            {
                if(dumpaddr != -1)
                    dump("NO");
                else
                    dump("QQ");     // means no addr is given.
            }
        }
        else if((cmd[0] == "disasm" || cmd[0] == "d"))
        {
            if(cmd.size() == 2)
                disasm(cmd[1], 10);
            else if(cmd.size() == 1)
                disasm("QQ", 10);   // means no addr is given.
        }    
        else if(cmd[0] == "getregs" && cmd.size() == 1)
            int ret = getregs(true);
        else if(cmd[0] == "start" && cmd.size() == 1)
            start();
        else if(cmd[0] == "run" && cmd.size() == 1)
            run();
        else if(cmd[0] == "si" && cmd.size() == 1)
            si();
        
        cmd.clear();
    }

    quit_exit();
    return 0;
}