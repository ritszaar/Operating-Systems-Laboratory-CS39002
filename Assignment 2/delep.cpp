#include "delep.h"

using namespace std;

char * get_filename(ino_t inode, pid_t lock_pid) {
	struct stat sb;

	struct dirent *dp;
	DIR *dirp;

	size_t len;
	int fd;
	char path[PATH_MAX], sym[PATH_MAX], *ret = NULL;

	memset(path, 0, sizeof(path));
	memset(sym, 0, sizeof(sym));

	snprintf(path, sizeof(path), "/proc/%d/fd/", lock_pid);

	try {
        auto subentry = fs::directory_iterator(path);
    } catch (std::filesystem::filesystem_error const& ex) {
        return NULL;
    }

    if (!(dirp = opendir(path))) {
        return NULL;
    }

	if ((len = strlen(path)) >= (sizeof(path) - 2)) {
        closedir(dirp);
	    return NULL;
    }
    

	if ((fd = dirfd(dirp)) < 0 ) {
        closedir(dirp);
	    return NULL;
    }

	while ((dp = readdir(dirp))) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
			continue;
        }

		errno = 0;

		if (!strtol(dp->d_name, (char **) NULL, 10) || errno) {
			continue;
        }

		if (!fstatat(fd, dp->d_name, &sb, 0) && inode != sb.st_ino) {
			continue;
        }

		if ((len = readlinkat(fd, dp->d_name, sym, sizeof(sym) - 1)) < 1) {
            closedir(dirp);
	        return ret;
        }

		sym[len] = '\0';

		ret = strdup(sym);
		break;
	}

    closedir(dirp);
	return ret;
}

vector<pid_t> get_opening_pids(const string& filename) {
    vector<int> pids;
    for (const auto& entry : fs::directory_iterator("/proc")) {
        string s = entry.path().string().substr(6);
        if (int(s.length()) > 0 && isdigit(s[0])) {
            pid_t pid = stoi(s);
            string subpath = entry.path().string() + "/fd";

            try {
                auto subentry = fs::directory_iterator(subpath);
            } catch (std::filesystem::filesystem_error const& ex) {
                continue;
            }

            for (const auto & subentry : fs::directory_iterator(subpath)) {
                char cur_filename[1024] = {0};
                readlink(subentry.path().c_str(), cur_filename, sizeof(cur_filename));
                if (filename == cur_filename) {
                    pids.push_back(pid);
                }
            }
        }
        
    } 

    return pids;
}

vector<pid_t> get_locking_pids(const string& filename) {
    vector<int> pids;

    ifstream fin("/proc/locks");
    string buffer;

    while (!fin.eof()) {
        for (int i = 0; i < 2; i++) {
            fin >> buffer;
        }

        if (buffer == "FLOCK") {
            for (int i = 0; i < 3; i++) {
                fin >> buffer;
            }

            pid_t pid = stoi(buffer);

            fin >> buffer;

            int state = 0;
            ino_t inode;
            for (int i = 0; i < int(buffer.length()); i++) {
                if (buffer[i] == ':') {
                    state = state == 0 ? 1 : 2;
                    if (state == 2) {
                        inode = stoul(buffer.substr(i + 1));
                        break;
                    } 
                }
            }

            char * cur_filename = NULL;
            cur_filename = get_filename(inode, pid);

            if (cur_filename != NULL && filename == cur_filename) {
                pids.push_back(pid);
            }

            fin >> buffer;
            fin >> buffer;
        } else {
            for (int i = 0; i < 6; i++) {
                fin >> buffer;
            }
        }
    } return pids;
}

void exec_delep(const string& filename) {
    int p[2];


    if (pipe(p) < 0) {
        perror("Unable to create pipe:");
    } 

    int c_pid;
    if ((c_pid = fork()) == 0) {

        close(p[0]);

        vector<pid_t> opening_pids = get_opening_pids(filename);
        vector<pid_t> locking_pids = get_locking_pids(filename);

        char buffer[1024] = {0};
        for (pid_t pid : opening_pids) {
            sprintf(buffer + strlen(buffer), "%d\n", pid);
        }  
        
        sprintf(buffer + strlen(buffer), "%d\n", -1);

        for (pid_t pid : locking_pids) {
            sprintf(buffer + strlen(buffer), "%d\n", pid);
        } 

        sprintf(buffer + strlen(buffer), "%d", -1);

        write(p[1], buffer, strlen(buffer) + 1);

        close(p[1]);

        exit(0);

    } else {

        wait(NULL);

        close(p[1]);

        char buffer[1024] = {0};
        int n = read(p[0], buffer, sizeof(buffer));

        close(p[0]);

        vector<pid_t> opening_pids, locking_pids;
        set<pid_t> merged_pids;

        istringstream iss(buffer);
        int state = 0;
        while (true) {
            string pid_str;
            pid_t pid;
            iss >> pid_str;
            pid = stoi(pid_str);
            if (pid == -1) {
                if (state == 0) {
                    state++;
                } else {
                    break;
                }
            } else {
                if (state == 0) {
                    opening_pids.push_back(pid);
                } else {
                    locking_pids.push_back(pid);
                }
            }
        }

        if (!opening_pids.empty()) {
            cout << "\nPIDs of processes that have opened " << filename << ": \n";
            for (int pid : opening_pids) {
                cout << pid << "\n";
                merged_pids.insert(pid);
            } cout << "\n";
        } else {
            cout << "\nNo process has opened the file \"" << filename << "\".\n\n";
        }
    
        if (!locking_pids.empty()) {
            cout << "PIDs of processes that have locked " << filename << ": \n";
            for (int pid : locking_pids) {
                cout << pid << "\n";
                merged_pids.insert(pid);
            } cout << "\n";
        } else {
            cout << "No process has locked the file \"" << filename << "\".\n\n";
        }

        if (!merged_pids.empty()) {
            while (true) {
                cout << "Do you want to kill these processes and delete the file? [Y|N]: ";
                char choice;
                cin >> choice; getchar();
                if (choice == 'Y' || choice == 'y') {
                    for (pid_t pid : merged_pids) {
                        kill(pid, SIGKILL);
                        remove(filename.c_str());
                    } break;
                } else if (choice == 'N' || choice == 'n') {
                    break;
                } else {
                    cout << "\nInvalid choice. Please enter [Y|N].\n\n"; 
                }
            }
        } else {
            cout << "No process has opened or locked the file \"" << filename << "\".\n\n";
        }
    }
}
