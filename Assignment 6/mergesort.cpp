#include "goodmalloc.h"
#include <sys/time.h>

void merge(string name, int start, int mid, int end)
{
    startScope();
    getList(name);
    int len = end - start + 1;
    string tempName = "tempList";
    createList(tempName, len);
    int i = start, j = mid + 1, k = 0;
    while (k < len)
    {
        if (i > mid)
        {
            assignVal(tempName, k, accessVal(name, j));
            j++;
        }
        else if (j > end)
        {
            assignVal(tempName, k, accessVal(name, i));
            i++;
        }
        else if (accessVal(name, j) < accessVal(name, i))
        {
            assignVal(tempName, k, accessVal(name, j));
            j++;
        }
        else
        {
            assignVal(tempName, k, accessVal(name, i));
            i++;
        }
        k++;
    }
    for (int ind = start; ind <= end; ind++)
    {
        assignVal(name, ind, accessVal(tempName, ind - start));
    }
    memoryFootprint();

    freeElem(tempName);
    setList(name);
    endScope();
}
void mergesort(string name, int start, int end, int display)
{
    startScope();
    getList(name);
    if (start >= end)
    {
        return;
    }
    int temp = (start + end) / 2;
    mergesort(name, start, temp, display + 1);
    mergesort(name, temp + 1, end, display + 1);

    merge(name, start, temp, end);
    // cout << "***********************************************************************" << endl;
    // cout << "Merge complete " << display << endl;
    // cout << "***********************************************************************" << endl;
    setList(name);
    endScope();
}
int main()
{
    struct timeval start, end;
    gettimeofday(&start, NULL);
    createMem(3e8);
    startScope();
    //
    string name = "MyList";
    int n = 50000;
    createList(name, n);
    pthread_mutex_lock(&printMutex);
    cout << "initial list: ";
    pthread_mutex_unlock(&printMutex);
    for (int i = 0; i < n; i++)
    {
        assignVal(name, i, rand() % 100000 + 1);
        pthread_mutex_lock(&printMutex);
        cout << accessVal(name, i) << " ";
        pthread_mutex_unlock(&printMutex);
    }
    pthread_mutex_lock(&printMutex);
    cout << endl;
    pthread_mutex_unlock(&printMutex);
    mergesort(name, 0, n - 1, 1);
    pthread_mutex_lock(&printMutex);
    cout << endl;
    cout << "sorted list: ";
    for (int i = 0; i < n; i++)
    {
        cout << accessVal(name, i) << " ";
    }
    cout << endl;
    pthread_mutex_unlock(&printMutex);
    // usleep(200000);
    // print_big_memory();
    freeElem(name);
    freeMem();
    gettimeofday(&end, NULL);
    double runtime = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    pthread_mutex_lock(&printMutex);
    cout << "Time required to run the program: " << runtime << " milliseconds" << endl;
    pthread_mutex_unlock(&printMutex);
    return 0;
}