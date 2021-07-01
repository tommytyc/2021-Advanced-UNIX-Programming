#pragma once
#include <elf.h>
#include <stdio.h>

typedef struct strtab
{
    int id;
    int size;
    char* data;
    struct strtab* next;
} strtab_t;

typedef struct elfhandle
{
    FILE* file;
    int ph_cnt;
    int sh_cnt;
    int sh_stridx;
    unsigned long entry;
    Elf64_Ehdr* ehdr;
    Elf64_Phdr* phdr;
    Elf64_Shdr* shdr;
    strtab_t* strtab;
} elfhandle_t;

int elf_open(elfhandle_t* e, const char* elfFile);
void elf_close(elfhandle_t* e);

void elf_load(elfhandle_t* e);
void load_phdr(elfhandle_t* e);
void load_shdr(elfhandle_t* e);
void load_strtab(elfhandle_t* e);