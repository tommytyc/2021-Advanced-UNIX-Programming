#include "open_elf.h"
#include <iostream>
#include <string.h>
#include <capstone/capstone.h>
using namespace std;

int elf_open(elfhandle_t* e, const char* elfFile)
{
    FILE* file = fopen(elfFile, "r");
    if(file)
    {
        e->ehdr = (Elf64_Ehdr*)calloc(1, sizeof(Elf64_Ehdr));
        fread(e->ehdr, sizeof(Elf64_Ehdr), 1, file);
        e->file = file;
        if (memcmp(e->ehdr->e_ident, ELFMAG, SELFMAG) == 0)
        {
            e->ph_cnt = e->ehdr->e_phnum;
            e->sh_cnt = e->ehdr->e_shnum;
            e->sh_stridx = e->ehdr->e_shstrndx;
            e->entry = e->ehdr->e_entry;
            return 0;
        }
        else
            return -1;
    }
    else
        return -1;
}

void elf_close(elfhandle_t* e)
{
    if(e->ehdr)
        free(e->ehdr);
    if(e->phdr)
        free(e->phdr);
    if(e->shdr)
        free(e->shdr);
    if(e->file)
        fclose(e->file);
    if(e)
        free(e);
}

void load_phdr(elfhandle_t* e)
{
    if(e->ph_cnt == 0)
        return;
    e->phdr = (Elf64_Phdr*)calloc(1, e->ehdr->e_phentsize * e->ehdr->e_phnum);
    fseek(e->file, e->ehdr->e_phoff, SEEK_SET);
    fread(e->phdr, e->ehdr->e_phentsize * e->ehdr->e_phnum, 1, e->file);
    
}

void load_shdr(elfhandle_t* e)
{
    if(e->sh_cnt == 0)
        return;
    e->shdr = (Elf64_Shdr*)calloc(1, e->ehdr->e_shentsize * e->ehdr->e_shnum);
    fseek(e->file, e->ehdr->e_shoff, SEEK_SET);
    fread(e->shdr, e->ehdr->e_shentsize * e->ehdr->e_shnum, 1, e->file);
}

void load_strtab(elfhandle_t* e)
{
    strtab_t* strtab = NULL, *now = NULL;
    if(e->shdr == NULL)
    {
        e->strtab = NULL;
        return;
    }
    for(int i = 0; i < e->sh_cnt; i++)
    {
        if(e->shdr[i].sh_type != SHT_STRTAB)
            continue;
        now = (strtab_t*)calloc(1, sizeof(strtab_t));
        now->id = i;
        now->size = e->shdr[i].sh_size;
        now->data = (char*) calloc(1, e->shdr[i].sh_size);\
        fseek(e->file, e->shdr[i].sh_offset, SEEK_SET);
        fread(now->data, e->shdr[i].sh_size, 1, e->file);
        now->next = strtab;
        strtab = now;
    }
    e->strtab = strtab;
}

void elf_load(elfhandle_t* e)
{
    load_phdr(e);
    load_shdr(e);
    load_strtab(e);
}

// int main(int agrc, char** argv)
// {
//     elfhandle_t* e = NULL;
//     e = (elfhandle_t*)calloc(1, sizeof(elfhandle_t));
//     if(e == NULL)
//     {
//         cerr << "** malloc failed." << endl;
//         exit(1);
//     }
//     int ret = elf_open(e, argv[1]);
//     elf_load(e);
//     if(ret < 0)
//     {
//         cerr << "Wrong ELF." << endl;
//         exit(-1);
//     }
//     cout << hex << e->ehdr->e_entry << endl;
//     elf_close(e);
//     return 0;
// }