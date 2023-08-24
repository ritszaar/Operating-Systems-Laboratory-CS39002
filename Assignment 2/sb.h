#pragma once

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include "fsystem.h"
using namespace std;

using ll = long long;
using ull = unsigned long long;

struct p_time {
    ull utime;
    ull stime;
    ull cutime;
    ull cstime;
};

class TrojanDetector {
    map<pid_t, pid_t> ppmap;
    map<pid_t, int> descendants;
    map<pid_t, ull> total_time;
    map<pid_t, vector<pid_t>> g;
    map<pid_t, p_time> times;
    map<pid_t, ull> min_time;

    public:
        TrojanDetector();
        pid_t get_ppid(pid_t);
        vector<string> at_fields(pid_t, const vector<int>&);
        void build_tree();
        void indfs(pid_t, pid_t);
        void display_ancestors(pid_t);
        void find_malware(pid_t);
        void handle_edge_cases();
        void display_detection_information(const string &);
};
