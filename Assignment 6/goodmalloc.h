#ifndef GOODMALLOC_H
#define GOODMALLOC_H
#include <iostream>
#include <stdio.h>
#include <bitset>
#include <csignal>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <map>

using namespace std;

typedef struct
{
    int size;
    int start;
    int end;
} my_struct;

typedef struct
{
    int val;
    int next, prev;
} element;

extern int CURRENT_SCOPE;
extern int big_memory_sz;
extern int *BIG_MEMORY;
extern map<int, map<string, my_struct *>> mapList;
extern pthread_mutex_t memMutex, mapMutex, printMutex;
void startScope();
void endScope();
void createMem(int);
int CreatePartitionMainMemory(int);
void createList(string, int);
uint32_t accessVal(string, int);
void assignVal(string, int, uint32_t);
void getList(string);
void setList(string);
void freeElem(string);
void freeMem();
int memoryFootprint();
#endif