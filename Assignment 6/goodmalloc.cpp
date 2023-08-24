#include "goodmalloc.h"
int CURRENT_SCOPE = 0;
int big_memory_sz = 0;
int *BIG_MEMORY = NULL;
map<int, map<string, my_struct *>> mapList;
pthread_mutex_t memMutex, mapMutex, printMutex;

void startScope()
{
    CURRENT_SCOPE++;
}
void endScope()
{
    pthread_mutex_lock(&printMutex);
    cout << "/************ Exiting Scope ... Freeing up space ************/" << endl;
    pthread_mutex_unlock(&printMutex);
    pthread_mutex_lock(&mapMutex);
    for (auto it = mapList[CURRENT_SCOPE].begin(); it != mapList[CURRENT_SCOPE].end(); it++)
    {
        cout << ".";
        my_struct *var = it->second;
        int size = var->size;
        pthread_mutex_lock(&memMutex);
        if (!(BIG_MEMORY[var->start + size] & 1))
        {
            size += (BIG_MEMORY[var->start + size] >> 1);
        }
        BIG_MEMORY[var->start] = (size << 1);
        pthread_mutex_unlock(&memMutex);
        free(var);
    }
    mapList[CURRENT_SCOPE].clear();
    mapList.erase(CURRENT_SCOPE);
    pthread_mutex_unlock(&mapMutex);
    CURRENT_SCOPE--;
}
void createMem(int size)
{
    pthread_mutex_init(&memMutex, NULL);
    pthread_mutex_init(&mapMutex, NULL);
    pthread_mutex_init(&printMutex, NULL);

    BIG_MEMORY = new int[size]();
    big_memory_sz = (size);

    BIG_MEMORY[0] = (big_memory_sz << 1);
    cout << (BIG_MEMORY[0] >> 1) << endl;
    pthread_mutex_lock(&printMutex);
    cout << "Memory created!!" << endl;
    pthread_mutex_unlock(&printMutex);
}
int CreatePartitionMainMemory(int size)
{

    // input size in bytes
    // Header: size (31 bits), free (1 bit)
    // Data: size
    // Uses First Fit
    size += 1; // for header
    int temp = 0;
    int i = 0;
    // cout << "required size: " << size << endl;
    pthread_mutex_lock(&memMutex);
    while (i < big_memory_sz && ((BIG_MEMORY[i] & 1) || size > (BIG_MEMORY[i] >> 1)))
    {
        // cout << "inside:" << endl;
        // cout << "is occupied: " << ((BIG_MEMORY[i] & 1)) << endl;
        // cout << "size: " << int(BIG_MEMORY[i] >> 1) << endl;
        // cout << "here" << endl;
        i += (BIG_MEMORY[i] >> 1);
        // temp++;
        // cout << "next i: " << i << endl;
        // if (temp > 5)
        //     break;
    }
    if (i > big_memory_sz)
        i = -1;
    if (i != -1)
    {
        if (i + size < big_memory_sz)
        {
            BIG_MEMORY[i + size] = (((BIG_MEMORY[i] >> 1) - size) << 1);
            // cout << "left size: " << (BIG_MEMORY[i + size] >> 1) << " at " << i + size << endl;
        }
        BIG_MEMORY[i] = (size << 1);
        BIG_MEMORY[i] |= 1;
        // cout << "performed updates at " << i << endl;
        // cout << "new size " << (BIG_MEMORY[i] >> 1) << endl;
        // cout << "occupancy " << (BIG_MEMORY[i] & 1) << endl;
        // cout << "next free at " << i + size << endl;
        // cout << "new size " << (BIG_MEMORY[i + size] >> 1) << endl;
        // cout << "occupancy " << (BIG_MEMORY[i + size] & 1) << endl;
    }
    pthread_mutex_unlock(&memMutex);
    return i;
}

void createList(string name, int sz)
{
    int main_memory_idx, total_size;
    total_size = sz * sizeof(element) * 8;
    main_memory_idx = CreatePartitionMainMemory(total_size / 8);
    if (main_memory_idx == -1)
    {
        pthread_mutex_lock(&printMutex);
        cout << "<<< createList >>>: Failed to create list" << endl;
        pthread_mutex_unlock(&printMutex);
        exit(1);
    }
    else
    {
        pthread_mutex_lock(&memMutex);
        int size = (BIG_MEMORY[main_memory_idx] >> 1);
        pthread_mutex_unlock(&memMutex);
        my_struct *temp = (my_struct *)malloc(sizeof(my_struct));
        temp->size = size;
        temp->start = main_memory_idx;
        temp->end = main_memory_idx + size - 1;
        pthread_mutex_lock(&mapMutex);
        mapList[CURRENT_SCOPE][name] = temp;
        pthread_mutex_unlock(&mapMutex);
    }
    pthread_mutex_lock(&printMutex);
    cout << "<<< createList >>>: List created successfully." << endl;
    pthread_mutex_unlock(&printMutex);
}

uint32_t accessVal(string name, int idx)
{
    pthread_mutex_lock(&mapMutex);
    pthread_mutex_lock(&memMutex);
    my_struct *var = mapList[CURRENT_SCOPE][name];
    int main_idx = var->start + 1 + (idx * 3);
    uint32_t val = BIG_MEMORY[main_idx];
    int ret = val;
    pthread_mutex_unlock(&memMutex);
    pthread_mutex_unlock(&mapMutex);
    return ret;
}
void assignVal(string name, int idx, uint32_t val)
{
    pthread_mutex_lock(&mapMutex);
    pthread_mutex_lock(&memMutex);
    my_struct *var = mapList[CURRENT_SCOPE][name];
    int main_idx = var->start + 1 + (idx * 3);
    BIG_MEMORY[main_idx] = val;
    pthread_mutex_unlock(&memMutex);
    pthread_mutex_unlock(&mapMutex);
    // cout << "<<< assignVal >>>: Assigned " << name << " at index " << idx << endl;
}
void getList(string name)
{
    pthread_mutex_lock(&mapMutex);
    mapList[CURRENT_SCOPE][name] = mapList[CURRENT_SCOPE - 1][name];
    pthread_mutex_unlock(&mapMutex);
}
void setList(string name)
{
    pthread_mutex_lock(&mapMutex);
    mapList[CURRENT_SCOPE].erase(name);
    pthread_mutex_unlock(&mapMutex);
}

int memoryFootprint()
{
    int size = 0;
    int i = 0;
    pthread_mutex_lock(&memMutex);
    while (i < big_memory_sz)
    {
        if ((BIG_MEMORY[i] & 1))
        {
            size += (BIG_MEMORY[i] >> 1);
        }
        i += (BIG_MEMORY[i] >> 1);
    }
    pthread_mutex_unlock(&memMutex);
    pthread_mutex_lock(&printMutex);
    cout << "Current allocated memory size : " << size << endl;
    pthread_mutex_unlock(&printMutex);
    return size;
}
void freeElem(string name)
{
    pthread_mutex_lock(&printMutex);
    cout << "<<< freeElem >>>: Freeing up " << name << endl;
    pthread_mutex_unlock(&printMutex);
    pthread_mutex_lock(&mapMutex);
    pthread_mutex_lock(&memMutex);
    my_struct *var = mapList[CURRENT_SCOPE][name];
    int size = var->size;

    if (!(BIG_MEMORY[var->start + size] & 1))
    {
        size += (BIG_MEMORY[var->start + size] >> 1);
    }
    BIG_MEMORY[var->start] = (size << 1);
    free(var);
    mapList[CURRENT_SCOPE].erase(name);
    pthread_mutex_unlock(&memMutex);
    pthread_mutex_unlock(&mapMutex);
}

void freeMem()
{
    pthread_mutex_lock(&printMutex);
    cout << "<<< freeMem >>>: Freeing up memory." << endl;
    pthread_mutex_unlock(&printMutex);
    pthread_mutex_lock(&mapMutex);
    pthread_mutex_lock(&memMutex);
    mapList.clear();
    free(BIG_MEMORY);
    pthread_mutex_unlock(&memMutex);
    pthread_mutex_unlock(&mapMutex);
}