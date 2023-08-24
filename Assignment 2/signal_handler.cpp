#include "signal_handler.h"

#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <map>

using namespace std;

map<pid_t, int> pgid_wd;

// Reference: https://web.stanford.edu/class/archive/cs/cs110/cs110.1206/lectures/07-races-and-deadlock-slides.pdf
// Signal handler for SIGCHILD to prevent zombie processes

void reapProcesses(int signum) {
    while (true) {
        int status;

        // printf("HI1\n");
        pid_t pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
        // pid_t pid = waitpid(-1, NULL, 0);
        // if(pid == fgpid)
        // fgpid = 0;
        if (pid <= 0) {
            break;
        }

        // printf("HI2\n");

        int id = map_pipeline[pid];
        Pipeline* pipeline = pipeline_list[id];

        if (WIFSIGNALED(status) || WIFEXITED(status)) {  // Terminated due to interrupt or normal exit
            pipeline->nactive--;
            if (pipeline->nactive == 0) {
                pipeline->status = DONE;
            }
        } 
        
        else if (WIFSTOPPED(status)) {    // Process was stopped (SIGTSTP)
            pipeline->nactive--;
            if (pipeline->nactive == 0) {
                pipeline->status = STOPPED;
            }
        } 
        
        else if (WIFCONTINUED(status)) {  // Process was continued (SIGCONT)
            pipeline->nactive++;
            if (pipeline->nactive == (int)pipeline->cmdlist.size()) {
                pipeline->status = RUNNING;
            }
        }

        if (pipeline->pgid == fgpid && !WIFCONTINUED(status)) {
            if (pipeline->nactive == 0) {
                fgpid = 0;  // To remove process from foreground
            }
        }
    }
}

// These functions help in avoiding race conditions when SIGCHILD can be sent
void toggleSIGCHLDBlock(int how) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(how, &mask, NULL);
}

void blockSIGCHLD() {
    toggleSIGCHLDBlock(SIG_BLOCK);
}

void unblockSIGCHLD() {
    toggleSIGCHLDBlock(SIG_UNBLOCK);
}

// Ensures no race conditions for foreground processes
void waitForForegroundProcess(pid_t pid) {
    fgpid = pid;
    sigset_t empty;

    // cout<<"HI\n";
    sigemptyset(&empty);
    while (fgpid == pid) {
        sigsuspend(&empty);
    }

    // cout<<"BYE\n";
    unblockSIGCHLD();
}

// Signal handler for SIGTSTP
void Z_handler(int signum) {
    if (signum == SIGTSTP) {
        ctrlZ = 1;
        printf("\n");
    }
} 