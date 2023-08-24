#include <bits/stdc++.h>
#include <sys/shm.h>
#include <unistd.h>
using namespace std;

using ll = long long;
using pii = pair<int, int>;

#define F first
#define S second

const int N_CONSUMERS = 10;
const int CONSUMER_IDLE_TIME = 30;
const int CONSUMER_CAPACITY = 4;

class Consumer
{
    int id;
    key_t shmkey;
    int n, m, prev_n;
    vector<vector<int>> g;
    int lbs, ubs;
    string savepath;
    vector<ll> dist;
    vector<bool> vis;
    vector<int> par;
    queue<int> pq;
    set<pii> sp;
    vector<int> sources;
    int flag;

public:
    Consumer() : id(0), shmkey(0), n(0), m(0), lbs(0), ubs(0) {}

    Consumer(int id_, key_t shmkey_, const string &savepath_, int flag_) : id(id_), shmkey(shmkey_), savepath(savepath_), flag(flag_), lbs(0), ubs(0), m(0), n(0)
    {
    }

    void consume()
    {
        // getting the shared memory size and accessing it
        size_t shmsize = get_shm_size();
        int shmid = shmget(shmkey, shmsize, 0666);
        int *shmptr = (int *)shmat(shmid, NULL, 0);
        // storing previous n and updating n
        prev_n = n;
        n = shmptr[1];
        // if no new nodes are added then return
        if (n == prev_n)
        {
            return;
        }
        // resize graph and other vectors
        g.resize(n);
        // clear only for unoptimized version
        if (flag == 0)
        {
            dist.clear();
            par.clear();
        }
        vis.clear();
        dist.resize(n, 1e18);
        vis.resize(n, false);
        par.resize(n, -1);
        // reading the newly added edges and storing them in the graph
        for (int i = m; i < shmptr[2]; i++)
        {
            int u = shmptr[2 * (i + 1) + 1];
            int v = shmptr[2 * (i + 1) + 2];
            g[u].push_back(v);
            g[v].push_back(u);
        }
        m = shmptr[2];
        shmdt(shmptr);
        // for first consumption, calculate the 1/10th portion for this particular consumer
        if (prev_n == 0)
        {
            set_shares();
            // push all sources in queue
            for (auto v : sources)
            {
                dist[v] = 0;
                par[v] = v;
                vis[v] = true;
                if (flag)
                    pq.push(v);
            }
        }
        else
        {
            // for newly added nodes, take 1/10th fraction for this consumer
            take_fraction();
            if (flag == 0)
            {
                // push all sources in set for unoptimized version
                for (auto v : sources)
                {
                    dist[v] = 0;
                    par[v] = v;
                }
            }
        }

        ofstream fout(savepath);
        fout << "Total number of nodes : " << n << endl;
        if (flag == 0)
            dijkstra(fout);
        else
            optimised_dijkstra(fout);
    }
    // function which calculates shortest path
    void dijkstra_update(int start)
    {
        set<pii>::iterator iter;
        for (auto v : g[start])
        {
            // check if the distance of node can be updated
            if (dist[v] > dist[start] + 1)
            {
                iter = sp.find({dist[v], v});
                // update distance and parent
                dist[v] = dist[start] + 1;
                par[v] = start;
                // removing the node from set
                sp.erase(iter);
                // inserting the node with updated distance
                sp.insert({dist[v], v});
            }
        }
        // if set is empty then return
        // it means all shortest path have been calculated
        if (sp.size() == 0)
            return;

        // select the first node as the next start node
        int next = sp.begin()->second;
        iter = sp.begin();
        // remove it from the set
        sp.erase(iter);
        // call function with the new start node
        dijkstra_update(next);
    }
    // function to calculate multi-source shortest paths for all nodes without any optimization
    // and printing the paths in required file
    void dijkstra(ofstream &fout)
    {
        // inserting all the nodes in set along with their initial distances
        for (int i = 0; i < n; i++)
        {
            if (dist[i] == 0)
            {
                sp.insert({0, i});
            }
            else
            {
                dist[i] = INT32_MAX;
                sp.insert({INT32_MAX, i});
            }
        }
        // calling dijkstra update function using the first node as start node
        dijkstra_update(sp.begin()->second);

        // printing the shortest paths in files using parent vector
        for (int i = 0; i < n; i++)
        {
            if (dist[i] != 1e18)
            {
                stack<int> path;
                path.push(i);
                while (par[path.top()] != path.top())
                {
                    path.push(par[path.top()]);
                }

                fout << i << " : ";
                while (!path.empty())
                {
                    fout << path.top() << " ";
                    path.pop();
                }
                fout << "\n";
            }
        }
    }
    // function for optimized dijkstra
    void optimised_dijkstra(ofstream &fout)
    {
        // if called for the first time
        // execute and store distances
        if (prev_n == 0)
        {
            while (!pq.empty())
            {
                int fs = pq.front();
                pq.pop();

                for (auto v : g[fs])
                {

                    if (vis[v] == false && dist[v] > dist[fs] + 1)
                    {
                        vis[v] = true;
                        dist[v] = dist[fs] + 1;
                        par[v] = fs;
                        pq.push(v);
                    }
                }
            }
        }
        else
        {
            for (int i = prev_n; i < n; i++)
            {
                vector<ll> new_dist(i + 1, 1e18);
                vector<bool> new_vis(i + 1, false);
                vector<int> new_par(i + 1, -1);
                // bfs from new node
                new_dist[i] = 0;
                new_vis[i] = true;
                new_par[i] = i;
                pq.push(i);
                while (!pq.empty())
                {
                    int fs = pq.front();
                    pq.pop();

                    for (auto v : g[fs])
                    {
                        if (v > i)
                            continue;

                        if (new_vis[v] == false && new_dist[v] > new_dist[fs] + 1)
                        {
                            new_vis[v] = true;
                            new_dist[v] = new_dist[fs] + 1;
                            new_par[v] = fs;
                            pq.push(v);
                        }
                    }
                }
                // storing nearest source and its distance
                ll min_source = -1;
                ll min_dist = 1e18;
                if (i % 10 == id)
                {
                    min_dist = 0;
                    min_source = i;
                }
                else
                {
                    for (auto v : sources)
                    {
                        if (v > i)
                            continue;
                        if (new_dist[v] < min_dist)
                        {
                            min_dist = new_dist[v];
                            min_source = v;
                        }
                    }
                }
                // setting parent of the current node
                if (min_source != i)
                {
                    stack<int> st;
                    st.push(min_source);
                    while (st.top() != i)
                    {
                        st.push(new_par[st.top()]);
                    }
                    st.pop();
                    par[i] = st.top();
                }
                else
                {
                    par[i] = i;
                }
                dist[i] = min_dist;
                // updating the distances for all the nodes
                for (int j = 0; j < i; j++)
                {
                    if (dist[j] == 0)
                        continue;
                    if (dist[j] >= new_dist[j] + min_dist)
                    {
                        dist[j] = new_dist[j] + min_dist;
                        par[j] = new_par[j];
                    }
                }

                new_dist.clear();
                new_vis.clear();
                new_par.clear();
            }
        }
        // printing the shortest paths using the parent vector
        for (int i = 0; i < n; i++)
        {
            if (dist[i] != 1e18)
            {
                stack<int> path;
                path.push(i);
                while (par[path.top()] != path.top())
                {

                    path.push(par[path.top()]);
                }

                fout << i << " : ";
                while (!path.empty())
                {
                    fout << path.top() << " ";
                    path.pop();
                }
                fout << "\n";
            }
        }
    }
    // function to get the size of shared memory
    size_t get_shm_size()
    {
        int shmid = shmget(shmkey, sizeof(int), 0666);
        int *shmptr = (int *)shmat(shmid, NULL, 0);
        size_t shmsize = shmptr[0];
        shmdt(shmptr);
        return shmsize;
    }
    // function to assign the 1/10th portion of the graph
    void set_shares()
    {
        int equal_share = ceil(1.00 * n / N_CONSUMERS);
        int last_share = (n % N_CONSUMERS == 0 ? equal_share : n - equal_share * (N_CONSUMERS - 1));
        if (id < N_CONSUMERS - 1)
        {

            lbs = id * equal_share;
            ubs = (id + 1) * equal_share - 1;
        }
        else
        {
            lbs = id * equal_share;
            ubs = lbs + last_share - 1;
        }
        // push all sources in the sources vector
        for (int i = lbs; i <= ubs; i++)
        {
            sources.push_back(i);
        }
    }
    // function to take a fraction from the newly added nodes
    void take_fraction()
    {
        for (int i = prev_n; i < n; i++)
        {
            // if newly added node is a source and add it to the sources vector
            if (i % 10 == id)
            {
                sources.push_back(i);
            }
        }
    }

    // void print()
    // {
    //     cout << "\nn = " << n << "\n"
    //          << "m = " << m << "\n";
    //     for (int i = 0; i < n; i++)
    //     {
    //         cout << i << ": ";
    //         for (auto j : g[i])
    //         {
    //             cout << j << " ";
    //         }
    //         cout << "\n";
    //     }
    // }
};

int main(int argc, char *argv[])
{
    // getting key,id,flag from the arguments
    key_t portkey = atoi(argv[1]);
    int id = atoi(argv[2]);
    string savepath = "./consumers/consumer_" + to_string(id) + ".txt";
    id--;
    int flag = atoi(argv[3]);

    Consumer consumer = Consumer(id, portkey, savepath, flag);

    pid_t pid = getpid();
    printf("Consumer pid[%d] spawned successfully.\n", getpid());

    int capacity = CONSUMER_CAPACITY;
    while (capacity > 0)
    {
        sleep(CONSUMER_IDLE_TIME);
        consumer.consume();
        printf("Consumer pid[%d] consumed successfully.\n", pid);

        capacity--;
    }

    printf("Consumer pid[%d] exited.\n\n", pid);
    return 0;
}
