#include<signal.h>
#include<unistd.h>
#include<sys/signal.h>
#include<sys/wait.h>

#include "signal_handler.h"

#include "pipeline.h"

using namespace std;

Pipeline::Pipeline(vector<Command>&cmdlist):cmdlist(cmdlist),pgid(-1),background_process(false),nactive(cmdlist.size()),status(RUNNING){}

void Pipeline::execute()
{
    pid_t fg_pgid=0;
    int next_pipe_fd[2],prev_pipe_fd[2];

    blockSIGCHLD();

    int ncommands=cmdlist.size();
    for(int i=0;i<ncommands;i++)
    {
        // cout<<"New command="<<i<<endl;
        if(i+1<ncommands)
        {
            if(pipe(next_pipe_fd)<0)
            {
                perror("Unable to create pipe\n");
                exit(EXIT_FAILURE);
            }
        }
        pid_t child_pid=fork();
        if(child_pid<0)
        {
            perror("Unable to fork\n");
            exit(EXIT_FAILURE);
        }
        else if(child_pid>0)
        {
            cmdlist[i].pid=child_pid; //Pid of the child process in the pipeline is the pid of the command 
            if(i==0)
            {
                fg_pgid=child_pid;
                pgid=child_pid;
                setpgid(child_pid,fg_pgid);
                pipeline_list.push_back(this);
                tcsetpgrp(STDIN_FILENO,fg_pgid);
            } 
            else
                setpgid(child_pid,fg_pgid);
            if(i>0)
            {
                close(prev_pipe_fd[0]);
                close(prev_pipe_fd[1]);
            }
            // if(waitpid(-1,NULL,0)<0)
            // {
            //     perror("Error occurred\n");
            //     exit(0);
            // }
            prev_pipe_fd[0]=next_pipe_fd[0];
            prev_pipe_fd[1]=next_pipe_fd[1];

            map_pipeline[child_pid] = pipeline_list.size()-1;
        }
        else if(child_pid==0)
        {
            unblockSIGCHLD();
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            if(i==0)
            {
                fg_pgid=getpid();
            }
            cmdlist[i].io_redirect();
            if(i==0)
            {
                setpgrp();
            }
            if(i>0)
            {
                setpgid(0,fg_pgid);
                dup2(prev_pipe_fd[0],cmdlist[i].fin_fd);
                close(prev_pipe_fd[0]);
                close(prev_pipe_fd[1]);
            }
            if(i+1<ncommands)
            {
                dup2(next_pipe_fd[1],cmdlist[i].fout_fd);
                close(next_pipe_fd[0]);
                close(next_pipe_fd[1]);
            }
            cmdlist[i].execute();
            exit(0);
        }
    }
    if (this->background_process) {  // For background processes, we don't wait for them
        unblockSIGCHLD();
    } else {
        // cout<<"HEY\n";
        waitForForegroundProcess(fg_pgid);
        if (pipeline_list.back()->status == STOPPED) {  // If Ctrl-Z was sent, now send SIGCONT to continue the process immediately in the background
            kill(-fg_pgid, SIGCONT);
        }
    }
    // cout<<"The end\n";
    // cout<<getpid()<<endl;
    tcsetpgrp(STDIN_FILENO, getpid());
    // cout<<"Should reach\n";
}