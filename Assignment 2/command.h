#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "delep.h"

using namespace std;

class Command {
    public:
        vector<string> args;
        string fin, fout;
        int fin_fd, fout_fd;
        pid_t pid;

        Command();
        void io_redirect();
        int execute();
        // Command(const string& command_name);
        // ~Command();
};