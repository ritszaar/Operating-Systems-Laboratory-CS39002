#include "sb.h"

TrojanDetector::TrojanDetector() {
    build_tree();
    indfs(0, -1);
    handle_edge_cases();
}

pid_t TrojanDetector::get_ppid(pid_t pid) {
    string path = "/proc/" + to_string(pid) + "/stat";
    ifstream fin(path);

    string buffer;
    for (int i = 0; i < 4; i++) {
        fin >> buffer;
    } fin.close();

    pid_t ppid = stoi(buffer);
    return ppid;
}

vector<string> TrojanDetector::at_fields(pid_t pid_, const vector<int>& fieldnos) {
    string path = "/proc/" + to_string(pid_) + "/stat";

    ifstream fin(path);
    string buffer;
    vector<string> v;

    for (int i = 0, j = 0; i < *fieldnos.rbegin(); i++) {

        fin >> buffer;

        if (i + 1 == fieldnos[j]) {
            v.push_back(buffer);
            j++;
        }

    } fin.close();

    return v;
}

void TrojanDetector::build_tree() {
    ppmap[0] = -1;
    for (const auto& entry : fs::directory_iterator("/proc")) {
        string s = entry.path().string().substr(6);
        if (int(s.length()) > 0 && isdigit(s[0])) {
            pid_t pid = stoi(s);
            ppmap[pid] = get_ppid(pid);
            vector<string> time_str = at_fields(pid, {14, 15, 16, 17});
            p_time pt = {stoull(time_str[0]), stoull(time_str[1]), stoull(time_str[2]), stoull(time_str[3])};
            times[pid] = pt;
        }
    }

    for (auto entry : ppmap) {
        pid_t pid = entry.first;
        pid_t ppid = entry.second;
        if (ppid == -1) continue;
        g[pid].push_back(ppid);
        g[ppid].push_back(pid);
    }
}

void TrojanDetector::indfs(pid_t node, pid_t par) {
    descendants[node] = 1;
    total_time[node] = times[node].utime;
    min_time[node] = times[node].utime;
    for (pid_t ch : g[node]) {
        if (ch != par) {
            indfs(ch, node);
            descendants[node] += descendants[ch];
            total_time[node] += total_time[ch];
            min_time[node] = min(min_time[node], min_time[ch]);
        }
    }
}

void TrojanDetector::handle_edge_cases() {
    total_time[0] = 0;
    total_time[1] = 0;
    total_time[2] = 0;
    for (const auto entry : total_time) {
        pid_t pid = entry.first;
        p_time pt = times[pid];

        if (pt.stime > 0 || pt.cutime > 0 || pt.cstime > 0) {
            total_time[pid] = 0;
        }
    }
}

void TrojanDetector::display_ancestors(pid_t pid) {
    cout << "\nThe ancestors of the process with PID " << pid << " are as follows: \n";
    while (pid != 1) {
        pid = ppmap[pid];
        cout << pid << "\n";
    } cout << "\n";
}

void TrojanDetector::find_malware(pid_t pid) {
    ull malwarity = total_time[pid];
    pid_t malware = pid;
    while (pid != 1) {
        pid = ppmap[pid];
        if (malwarity < total_time[pid]) {
            malware = pid;
            malwarity = total_time[pid];
        }
    }

    cout << "\nAmong all the ancestors, the suspected malware is the one with PID " << malware << ".\n\n";
}

void TrojanDetector::display_detection_information(const string & filename) {
    ofstream fout(filename);
    vector<pair<int, int>> data;

    for (const auto entry : total_time) {
        if (entry.first < 3) continue;
        data.push_back({entry.second, entry.first});
    }
    
    sort(data.begin(), data.end(), greater<pair<int, int>>());

    fout << "PID" << " " << "Heuristic\n";
    for (auto entry : data) {
        fout << entry.second << " " << entry.first << "\n";
    } fout.close();
}
