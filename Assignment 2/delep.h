#pragma once

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fsystem.h"
#include <vector>
#include <fstream>
#include <string.h>
#include <signal.h>
#include <set>
#include <sys/wait.h>
#include <sstream>
#include <linux/kdev_t.h>
#include <stdio.h>
#include <dirent.h>

using namespace std;

char * get_filename(ino_t, pid_t);
vector<pid_t> get_opening_pids(const string&);
vector<pid_t> get_locking_pids(const string&);
void exec_delep(const string&);
