#include <bits/stdc++.h>
#include <sys/shm.h>
#include <unistd.h>
using namespace std;

using ll = long long;
using pii = pair<int, int>;

#define F first
#define S second

const pii N_NODES = {10, 30};
const pii N_DEGREES = {1, 20};
const int BUFFER_SIZE = 1024;
const int PRODUCER_IDLE_TIME = 50;
const int PRODUCER_CAPACITY = 3;

class Producer
{
    key_t shmkey;
    int n, m;
    vector<int> degrees;
    vector<pii> edges;
    size_t oldsize;

public:
    Producer() : shmkey(0), n(0), m(0), oldsize(0) {}

    Producer(key_t shmkey_) : shmkey(shmkey_), oldsize(0)
    {
        // getting the size of shared memory
        int shmid = shmget(shmkey, sizeof(int), 0666);
        int *shmptr = (int *)shmat(shmid, NULL, 0);
        oldsize = shmptr[0];
        shmdt(shmptr);
        // accessing the shared memory
        shmid = shmget(shmkey, oldsize, 0666);
        shmptr = (int *)shmat(shmid, NULL, 0);
        n = shmptr[1];
        m = shmptr[2];
        degrees.assign(n, 0);
        // storing the edges and updating the degrees of the respective nodes
        for (int i = 0; i < m; i++)
        {
            edges.push_back({shmptr[2 * (i + 1) + 1], shmptr[2 * (i + 1) + 2]});
            degrees[shmptr[2 * (i + 1) + 1]]++;
            degrees[shmptr[2 * (i + 1) + 2]]++;
        }
        shmdt(shmptr);
    }

    ~Producer()
    {
        // deleting the shared memory
        int shmid = shmget(shmkey, oldsize, 0666);
        shmctl(shmid, IPC_RMID, NULL);
    }
    // function to get the size of the shared memory
    size_t evaluate_shm_size()
    {
        size_t actual_size = (1 + 2 + m * 2) * sizeof(int);
        return actual_size;
    }
    // function to update the shared memory with newly added edges
    size_t save_product()
    {
        size_t shmsize = evaluate_shm_size();
        int shmid, *shmptr;

        shmid = shmget(shmkey, oldsize, 0666);
        shmctl(shmid, IPC_RMID, NULL);
        shmid = shmget(shmkey, shmsize, IPC_CREAT | 0666);
        shmptr = (int *)shmat(shmid, NULL, 0);

        shmptr[0] = shmsize;
        shmptr[1] = n;
        shmptr[2] = m;
        for (int i = 0; i < int(edges.size()); i++)
        {
            shmptr[2 * (i + 1) + 1] = edges[i].F;
            shmptr[2 * (i + 1) + 2] = edges[i].S;
        }
        shmdt(shmptr);

        return shmsize;
    }
    // function to produce new nodes
    void produce()
    {
        // generate the number of new nodes
        int count = rand() % (N_NODES.S - N_NODES.F + 1) + N_NODES.F;
        for (int i = 0; i < count; i++)
        {
            // generate random degree for the new node
            int degree = rand() % (N_DEGREES.S - N_DEGREES.F + 1) + N_DEGREES.F;
            vector<pii> bag(n, pii());
            bag[0] = {degrees[0], 0};
            for (int i = 1; i < n; i++)
            {
                bag[i] = {degrees[i] + bag[i - 1].F, i};
            }
            // add edges using the probability
            for (int i = 0; i < degree; i++)
            {
                pii choice = {rand() % bag.back().F + 1, 0};
                auto pos = lower_bound(bag.begin(), bag.end(), choice);
                int u = pos->S;
                for (auto it = pos + 1; it != bag.end(); it++)
                {
                    it->F -= degrees[u];
                }

                edges.push_back({u, n});
                m++;
                degrees[u]++;

                bag.erase(pos);
            }
            degrees.push_back(degree);
            n++;
        }
        // save the updates to the shared memory
        oldsize = save_product();
    }

    // void print()
    // {
    //     cout << "\nn = " << n << "\n"
    //          << "m = " << m << "\n";
    //     cout << "Degrees of the verices are given below: \n";
    //     for (int i = 0; i < n; i++)
    //     {
    //         cout << i << ": " << degrees[i] << "\n";
    //     }
    //     cout << "Edges of the vertices are given below: \n";
    //     for (pii edge : edges)
    //     {
    //         cout << edge.F << " " << edge.S << "\n";
    //     }
    //     cout << "\n";
    // }
};

int main(int argc, char *argv[])
{
    // getting the port key from the argument
    key_t portkey = atoi(argv[1]);

    Producer producer = Producer(portkey);

    pid_t pid = getpid();
    printf("\n\nProducer pid[%d] spawned successfully.\n\n", pid);
    int capacity = PRODUCER_CAPACITY;
    while (capacity > 0)
    {
        sleep(PRODUCER_IDLE_TIME);
        producer.produce();
        printf("\n\nProducer pid[%d] produced successfully.\n\n", pid);

        capacity--;
    }

    printf("Producer pid[%d] exited.\n\n", pid);
    return 0;
}
