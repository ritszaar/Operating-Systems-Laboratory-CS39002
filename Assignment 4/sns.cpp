#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <map>
#include <cstdlib>
#include <queue>
#include <unistd.h>
#include <cmath>
#include <string.h>
#include <pthread.h>

using namespace std;

// setting all the constants
const int NODES_NUM = 37700;
const int EDGES_NUM = 289003;

const int USER_THREADS_NUM = 1;
const int PUSH_THREADS_NUM = 25;
const int READ_THREADS_NUM = 10;

const int PROPORTIONALITY_CONSTANT = 10; // In userSimulator: for selecting the number of actions to be generated for any node in userSimulator
const int RANDOM_NODES_NUM = 100;
const int SEED = 1234;

pthread_mutex_t sharedQueueMutex;     // mutex for shared queue
pthread_mutex_t modifiedQueueMutex;   // mutex for modified queue
pthread_mutex_t writeToFileMutex;     // mutex for writing in file
pthread_mutex_t feedMutex[NODES_NUM]; // mutex for accessing feed for each node

pthread_cond_t sharedQueueCond;
pthread_cond_t modifiedQueueCond;
pthread_cond_t feedCond[NODES_NUM];
pthread_cond_t actionPrintedCond[NODES_NUM];   // condition to check if action creation statement is printed
pthread_cond_t feedPushPrintedCond[NODES_NUM]; // condition to check if action pushed statement is printed

enum actions
{
    POST,
    COMMENT,
    LIKE
};

typedef struct action
{
    int user_id;
    int action_id;
    actions action_type;
    time_t timestamp;
    bool creation_printed; // flag to check if the action creation statement is printed
} action;

typedef struct
{
    bool flag; // flag to check if the action pushed statement
} feedPushPrinted;

// custom comparator for feed priority queue
struct compareActions
{
    bool operator()(pair<long long int, pair<action *, feedPushPrinted *>> const &p1, pair<long long int, pair<action *, feedPushPrinted *>> const &p2)
    {
        return p1.first < p2.first;
    }
};

typedef struct node
{
    int node_id;
    int order;
    vector<pair<int, int>> neighbours;
    queue<action *> wall;
    priority_queue<pair<long long int, pair<action *, feedPushPrinted *>>, vector<pair<long long int, pair<action *, feedPushPrinted *>>>, compareActions> feed;
    // storing number of each type of actions
    int num_posts;
    int num_comments;
    int num_likes;
} node;

queue<action *> sharedQueue; // shared queue between all threads

vector<node *> nodeList(NODES_NUM); // list of all nodes

queue<pair<int, time_t>> modifiedQueue; // modified queue stores the time when pushUpdate has last pushed into the feed queue of a node

vector<time_t> lastUpdated(NODES_NUM); // stores the last time when the readPost has popped from the feed queue of a node

int getOrder()
{
    // srand(time(nullptr));
    int result = rand() % 2; // chooses a random ordering for the queue (Priority or Chronological ordering)
    return result;
}

int getAction()
{
    // srand(time(nullptr));
    int result = rand() % 3; // chooses a random Action to perform (Post, Comment or Like)
    return result;
}

void updatePriority(int node1, int node2) // calculates number of common neighbours between the two nodes
{
    map<int, int> temp_priority;
    int common_neighbours = 0;
    int pos_node1, pos_node2;
    int counter = 0;
    for (auto i : nodeList[node1]->neighbours) // for the first node, stores each of its neighbours in a map
    {
        if (i.first == node2)
            pos_node2 = counter;
        temp_priority.insert({i.first, 1});
        counter++;
    }
    counter = 0;
    for (auto i : nodeList[node2]->neighbours) // for the second node, for each of its neighbours, checks if the neighbour is also a neighbour of the first node
    {
        if (i.first == node1)
            pos_node1 = counter;
        if (temp_priority.find(i.first) != temp_priority.end())
            common_neighbours++; // counts the number of common neighbours
        counter++;
    }
    nodeList[node1]->neighbours[pos_node2].second = common_neighbours; // stores the number of common neighbours between the pair of nodes
    nodeList[node2]->neighbours[pos_node1].second = common_neighbours;
}
// function to get current time
time_t getCurrentTime()
{
    time_t current_time = time(NULL);
    return current_time;
}

// this function returns the time in milliseconds
long long timeInMilliseconds(time_t currentTime)
{
    chrono::time_point<chrono::system_clock> timePoint = chrono::system_clock::from_time_t(currentTime);
    auto timeMs = chrono::duration_cast<chrono::milliseconds>(timePoint.time_since_epoch());
    return timeMs.count();
}

char *ctime_no_newline(const time_t *currTime)
{
    char *modifiedTime = strtok(ctime(currTime), "\n");
    return modifiedTime;
}
// function to create new action
void createAction(int node_id, int action_type, action *new_action)
{
    new_action->user_id = node_id;
    new_action->timestamp = getCurrentTime();
    new_action->creation_printed = false;
    if (action_type == POST)
    {
        new_action->action_id = nodeList[node_id]->num_posts + 1;
        nodeList[node_id]->num_posts = nodeList[node_id]->num_posts + 1;
        new_action->action_type = POST;
    }
    else if (action_type == COMMENT)
    {
        new_action->action_id = nodeList[node_id]->num_comments + 1;
        nodeList[node_id]->num_comments = nodeList[node_id]->num_comments + 1;
        new_action->action_type = COMMENT;
    }
    else if (action_type == LIKE)
    {
        new_action->action_id = nodeList[node_id]->num_likes + 1;
        nodeList[node_id]->num_likes = nodeList[node_id]->num_likes + 1;
        new_action->action_type = LIKE;
    }
}

void *userSimulator(void *param)
{
    vector<int> node_taken(NODES_NUM, 0);
    while (1)
    {
        vector<int> random_nodes;
        // SELECTING RANDOM 100 NODES
        int count = RANDOM_NODES_NUM;
        while (count)
        {
            int curr_num = rand() % NODES_NUM;
            if (node_taken[curr_num])
                continue;
            count--;
            random_nodes.push_back(curr_num);
            node_taken[curr_num] = 1;
        }
        for (int i = 0; i < RANDOM_NODES_NUM; i++)
            node_taken[i] = 0;

        // GENERATING ACTIONS FOR EACH SELECTED NODE
        for (int i = 0; i < RANDOM_NODES_NUM; i++)
        {
            double n_actions_double = log2(nodeList[random_nodes[i]]->neighbours.size());
            int n_actions = ceil(n_actions_double);
            n_actions++;
            n_actions = PROPORTIONALITY_CONSTANT * n_actions;

            // safely writing to the output file
            pthread_mutex_lock(&writeToFileMutex);
            ofstream file("sns.log", ios_base::app);

            // writing to file
            file << "Selected Node No.: " << (i + 1) << endl;
            file << "Node ID: " << random_nodes[i] << endl;
            file << "Number of actions: " << n_actions << endl;
            file << "Degree of node: " << nodeList[random_nodes[i]]->neighbours.size() << endl;

            // printing to terminal
            cout << "Selected Node No.: " << (i + 1) << endl;
            cout << "Node ID: " << random_nodes[i] << endl;
            cout << "Number of actions: " << n_actions << endl;
            cout << "Degree of node: " << nodeList[random_nodes[i]]->neighbours.size() << endl;
            file.close();
            pthread_mutex_unlock(&writeToFileMutex);

            vector<action *> temp_actions;
            for (int j = 0; j < n_actions; j++)
            {
                // creating actions
                int curr_action = getAction();
                action *new_action = new action();
                createAction(random_nodes[i], curr_action, new_action);
                // pushing actions to the wall
                nodeList[random_nodes[i]]->wall.push(new_action);
                // pushing pointers to the actions in a vector
                temp_actions.push_back(new_action);

                // safely using sharedQueue
                pthread_mutex_lock(&sharedQueueMutex);
                sharedQueue.push(new_action);
                pthread_cond_signal(&sharedQueueCond);
                pthread_mutex_unlock(&sharedQueueMutex);
            }

            // safely writing to the output file
            pthread_mutex_lock(&writeToFileMutex);
            file.open("sns.log", ios_base::app);
            for (int j = 0; j < temp_actions.size(); j++)
            {
                temp_actions[j]->creation_printed = true;
                file << "I created action number " << temp_actions[j]->action_id << " of type " << temp_actions[j]->action_type << " posted by user " << temp_actions[j]->user_id << " at time " << ctime_no_newline(&(temp_actions[j])->timestamp) << endl;
                if (temp_actions[j]->action_id == 1)
                    cout << "I created action number " << temp_actions[j]->action_id << " of type " << temp_actions[j]->action_type << " posted by user " << temp_actions[j]->user_id << " at time " << ctime_no_newline(&(temp_actions[j])->timestamp) << endl;
                pthread_cond_signal(&actionPrintedCond[temp_actions[j]->user_id]);
            }
            file.close();
            pthread_mutex_unlock(&writeToFileMutex);

            temp_actions.clear();
        }
        sleep(120);
    }
    pthread_exit(0);
}

/*
my shared resources:

shared_queue --> 1
feed queue   --> NODES_NUM

*/

void *pushUpdate(void *param)
{
    int node_id, neighbour_id;
    long long int normalisedPriority;
    while (1)
    {
        action *poppedAction;
        // safely using sharedQueue and popping out action if present (from the front of the queue)
        pthread_mutex_lock(&sharedQueueMutex);
        while (sharedQueue.empty())
        {
            pthread_cond_wait(&sharedQueueCond, &sharedQueueMutex);
        }
        // using the sharedQueue resource
        poppedAction = sharedQueue.front();
        sharedQueue.pop();
        pthread_mutex_unlock(&sharedQueueMutex);

        node_id = poppedAction->user_id;
        // setting priority for all the neighbours
        for (auto &neighbourNode : nodeList[node_id]->neighbours)
        {
            neighbour_id = neighbourNode.first;
            if (nodeList[neighbour_id]->order == 0)
            {
                if (neighbourNode.second == -1)
                {
                    // update the priority between node_id and neighbour_id
                    updatePriority(node_id, neighbour_id);
                }
                normalisedPriority = neighbourNode.second;
            }
            else
            {
                normalisedPriority = ((long long)-1) * timeInMilliseconds(poppedAction->timestamp);
            }

            feedPushPrinted *feedFlag = new feedPushPrinted();
            feedFlag->flag = false;
            // safely pushing in the feed queues
            pthread_mutex_lock(&feedMutex[neighbour_id]);
            nodeList[neighbour_id]->feed.push({normalisedPriority, {poppedAction, feedFlag}});
            // safely pushing in the modified queue
            pthread_mutex_lock(&modifiedQueueMutex);
            modifiedQueue.push({neighbour_id, getCurrentTime()});
            pthread_cond_signal(&modifiedQueueCond);
            pthread_mutex_unlock(&modifiedQueueMutex);
            pthread_cond_signal(&feedCond[neighbour_id]);
            pthread_mutex_unlock(&feedMutex[neighbour_id]);

            // safely writing to the output file
            pthread_mutex_lock(&writeToFileMutex);
            ofstream file("sns.log", ios_base::app);
            while (poppedAction->creation_printed == false)
                pthread_cond_wait(&actionPrintedCond[poppedAction->user_id], &writeToFileMutex);
            file << "I pushed action number " << poppedAction->action_id << " of type " << poppedAction->action_type << " posted by user " << poppedAction->user_id << " at time " << ctime_no_newline(&(poppedAction->timestamp)) << " to the feed queue of user " << neighbour_id << endl;
            if (poppedAction->action_id == 1)
                cout << "I pushed action number " << poppedAction->action_id << " of type " << poppedAction->action_type << " posted by user " << poppedAction->user_id << " at time " << ctime_no_newline(&(poppedAction->timestamp)) << " to the feed queue of user " << neighbour_id << endl;
            file.close();
            // setting the flag as true
            feedFlag->flag = true;
            pthread_cond_signal(&feedPushPrintedCond[neighbour_id]);
            pthread_mutex_unlock(&writeToFileMutex);
        }
    }
    pthread_exit(0);
}

void *readPost(void *param)
{
    int node_id;
    srand(SEED);
    while (1)
    {
        // safely popping from the modified queue
        pthread_mutex_lock(&modifiedQueueMutex);
        while (modifiedQueue.empty()) // wait while queue is empty
        {
            pthread_cond_wait(&modifiedQueueCond, &modifiedQueueMutex);
        }
        auto queueElement = modifiedQueue.front();
        modifiedQueue.pop();
        pthread_cond_signal(&modifiedQueueCond);
        pthread_mutex_unlock(&modifiedQueueMutex);

        node_id = queueElement.first;

        if (timeInMilliseconds(queueElement.second) < timeInMilliseconds(lastUpdated[node_id]))
            continue;

        // safely popping from the feed queue
        pthread_mutex_lock(&feedMutex[node_id]);

        vector<pair<action *, feedPushPrinted *>> temp_actions;

        while (!nodeList[node_id]->feed.empty())
        {
            auto currAction = nodeList[node_id]->feed.top().second;
            temp_actions.push_back(currAction);
            nodeList[node_id]->feed.pop();
        }
        lastUpdated[node_id] = getCurrentTime();

        // safely writing to the output file
        pthread_mutex_lock(&writeToFileMutex);
        ofstream file("sns.log", ios_base::app);
        for (int j = 0; j < temp_actions.size(); j++)
        {
            while (temp_actions[j].second->flag == false) // wait until action push statement is not printed
            {
                pthread_cond_wait(&feedPushPrintedCond[queueElement.first], &writeToFileMutex);
            }
            file << "I read action number " << temp_actions[j].first->action_id << " of type " << temp_actions[j].first->action_type << " posted by user " << temp_actions[j].first->user_id << " at time " << ctime_no_newline(&(temp_actions[j].first)->timestamp) << " from the feed queue of user: " << queueElement.first << endl;
            if (temp_actions[j].first->action_id == 1)
                cout << "I read action number " << temp_actions[j].first->action_id << " of type " << temp_actions[j].first->action_type << " posted by user " << temp_actions[j].first->user_id << " at time " << ctime_no_newline(&(temp_actions[j].first)->timestamp) << " from the feed queue of user: " << queueElement.first << endl;
        }
        file.close();
        pthread_mutex_unlock(&writeToFileMutex);

        temp_actions.clear();
        // sending signal
        pthread_cond_signal(&feedCond[node_id]);
        pthread_mutex_unlock(&feedMutex[node_id]);
    }
    pthread_exit(0);
}

void *main_thread(void *param)
{
    // CREATING nodeList
    for (int i = 0; i < NODES_NUM; i++)
    {
        node *tempNode = new node();
        tempNode->node_id = i;
        tempNode->order = getOrder();
        tempNode->num_posts = 0;
        tempNode->num_comments = 0;
        tempNode->num_likes = 0;
        nodeList[i] = tempNode;
    }

    // LOADING THE GRAPH
    ifstream csvFile("./musae_git_edges.csv");
    if (!csvFile.is_open())
    {
        cout << "Can't open the file!\n";
        exit(1);
    }
    string currLine;
    int count = 0;
    while (getline(csvFile, currLine))
    {
        count++;
        int node1, node2;
        string currNode = "";
        for (char ch : currLine)
        {
            if (ch == ',')
            {
                if (count != 1)
                    node1 = stoi(currNode);
                currNode.clear();
                currNode = "";
            }
            else
            {
                currNode += ch;
            }
        }
        if (count != 1)
        {
            node2 = stoi(currNode);
            nodeList[node1]->neighbours.push_back({node2, -1});
            nodeList[node2]->neighbours.push_back({node1, -1});
        }
        currNode.clear();
    }
    csvFile.close();

    // CALCULATING PRIORITIES FOR EVERY EDGE
    // for (int i = 0; i < NODES_NUM; i++)
    // {
    //     for (auto j : nodeList[i]->neighbours)
    //     {
    //         if (j.second != -1)
    //             continue;
    //         updatePriority(i, j.first);
    //     }
    // }

    // SPAWNING OTHER THREADS
    pthread_t user_tid, push_tid[PUSH_THREADS_NUM], read_tid[READ_THREADS_NUM];
    pthread_create(&user_tid, NULL, userSimulator, NULL);
    for (int i = 0; i < PUSH_THREADS_NUM; i++)
    {
        pthread_create(&push_tid[i], NULL, pushUpdate, NULL);
    }
    for (int i = 0; i < READ_THREADS_NUM; i++)
    {
        pthread_create(&read_tid[i], NULL, readPost, NULL);
    }
    // joining threads
    pthread_join(user_tid, NULL);
    for (int i = 0; i < PUSH_THREADS_NUM; i++)
    {
        pthread_join(push_tid[i], NULL);
    }
    for (int i = 0; i < READ_THREADS_NUM; i++)
    {
        pthread_join(read_tid[i], NULL);
    }
    pthread_exit(0);
}

int main()
{
    // initialising mutex and condition variables
    pthread_mutex_init(&sharedQueueMutex, NULL);
    pthread_cond_init(&sharedQueueCond, NULL);
    pthread_mutex_init(&modifiedQueueMutex, NULL);
    pthread_cond_init(&modifiedQueueCond, NULL);
    pthread_mutex_init(&writeToFileMutex, NULL);
    for (int i = 0; i < NODES_NUM; i++)
    {
        pthread_mutex_init(&feedMutex[i], NULL);
        pthread_cond_init(&feedCond[i], NULL);
        pthread_cond_init(&feedPushPrintedCond[i], NULL);
        pthread_cond_init(&actionPrintedCond[i], NULL);
    }

    // initilising lastUpdated vector
    time_t currTime = getCurrentTime();
    for (int i = 0; i < NODES_NUM; i++)
    {
        lastUpdated[i] = currTime;
    }

    pthread_t main_tid;
    pthread_attr_t main_attr;
    pthread_attr_init(&main_attr);
    pthread_create(&main_tid, &main_attr, main_thread, NULL);
    pthread_join(main_tid, NULL);

    // for(int i = 0; i<NODES_NUM; i++)
    // {
    //     for(int j = 0; j<nodeList[i]->neighbours.size(); j++)
    //     {
    //         cout<<i<<" "<<nodeList[i]->neighbours[j].first<<" "<<nodeList[i]->neighbours[j].second<<endl;
    //     }
    // }

    // destroying mutex and condition variables
    pthread_cond_destroy(&sharedQueueCond);
    pthread_mutex_destroy(&sharedQueueMutex);
    pthread_cond_destroy(&modifiedQueueCond);
    pthread_mutex_destroy(&modifiedQueueMutex);
    pthread_mutex_destroy(&writeToFileMutex);
    for (int i = 0; i < NODES_NUM; i++)
    {
        pthread_cond_destroy(&feedCond[i]);
        pthread_cond_destroy(&feedPushPrintedCond[i]);
        pthread_cond_destroy(&actionPrintedCond[i]);
        pthread_mutex_destroy(&feedMutex[i]);
    }
    return 0;
}