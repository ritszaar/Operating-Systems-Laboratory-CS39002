#pragma once

#include<vector>
#include<string>

#include "command.h"

#define RUNNING 0
#define STOPPED 1
#define DONE 2

class Pipeline{
    public:
    vector<Command>cmdlist;
    pid_t pgid;
    bool background_process;
    int nactive;
    int status;
    Pipeline(vector<Command>&);
    void execute();
};