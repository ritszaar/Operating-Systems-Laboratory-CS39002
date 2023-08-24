#pragma once

#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <vector>

#include "pipeline.h"

using namespace std;

extern bool ctrlC, ctrlZ;
extern pid_t fgpid;

extern vector<Pipeline*> pipeline_list;
extern map<pid_t, int> map_pipeline;


void reapProcesses(int signum);
void toggleSIGCHLDBlock(int how);
void blockSIGCHLD();
void unblockSIGCHLD();
void waitForForegroundProcess(pid_t pid);
void Z_handler(int signum);
