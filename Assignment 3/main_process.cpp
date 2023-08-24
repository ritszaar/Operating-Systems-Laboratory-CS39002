#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <bits/stdc++.h>

using namespace std;

// storing the initial number of nodes and number of consumers
int Num_Nodes = 4039;
const int N_CONSUMERS = 10;

using ll = long long;
using pii = pair<int, int>;

#define F first
#define S second

int main(int argc, char *argv[])
{
    // flag variable to check for optimized algorithm
    int flag = 0;
    if (argc <= 1)
    {
        printf("Insufficient arguments!\n");
        exit(1);
    }
    if (argc > 3)
    {
        printf("Excess arguments!\n");
        exit(1);
    }
    if (argc == 3)
    {
        if (strcmp(argv[2], "-optimize") == 0)
        {
            flag = 1;
        }
        else
        {
            printf("Unknown flag!\n");
            exit(1);
        }
    }

    int n = Num_Nodes;
    vector<pii> edges;
    int m = 0;
    FILE *fptr = fopen("facebook_combined.txt", "r");
    int node_1, node_2;
    // reading the edges from the input file and storing in a vector
    while (fscanf(fptr, "%d %d", &node_1, &node_2) == 2)
    {
        edges.push_back({node_1, node_2});
        m++;
    }
    fclose(fptr);

    // getting the key from argument
    key_t key = atoi(argv[1]);
    // calculating the size of required shared memory
    size_t shmsize = (1 + 2 + 2 * m) * sizeof(int);
    // creating and attaching the shared memory
    int shmid = shmget(key, shmsize, IPC_CREAT | 0666);
    int *shmptr = (int *)shmat(shmid, NULL, 0);

    shmptr[0] = shmsize;
    shmptr[1] = n;
    shmptr[2] = m;
    for (int i = 0; i < int(edges.size()); i++)
    {
        shmptr[2 * (i + 1) + 1] = edges[i].F;
        shmptr[2 * (i + 1) + 2] = edges[i].S;
    }
    shmdt(shmptr);

    char portkeystr[16] = {0};
    sprintf(portkeystr, "%d", key);

    // spawning producer and consumers along with required arguments
    if (fork() == 0)
    {
        execl("./producer", "./producer", portkeystr, NULL);
    }
    else
    {
        pid_t pid = getpid();
        for (int i = 0; i < N_CONSUMERS; i++)
        {
            if (fork() == 0 && getppid() == pid)
            {
                execl("./consumer", "./consumer", portkeystr, to_string(i + 1).c_str(), to_string(flag).c_str(), NULL);
            }
        }
    }

    pid_t wpid;
    int wstatus;
    while ((wpid = wait(&wstatus)) > 0)
        ;

    return 0;
}
