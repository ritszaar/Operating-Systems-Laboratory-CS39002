#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <fcntl.h>
using namespace std;

signed main() {

    int cpid;
    if ((cpid = fork()) == 0) {
        int cfd;
        if ((cfd = open("input.txt", O_WRONLY)) < 0) {
            perror("Unable to open file:");
        } else {
            if (flock(cfd, F_WRLCK) < 0) {
                cout << "Unable to lock the file!\n";
            } else {
                cout << "Child process with pid " << getpid() << " has locked the file.";
            }
        }   

        const char * buffer = "Geralt of Rivia\n";
        while (1) {
            if (write(cfd, buffer, sizeof(buffer)) < 0) {
                cout << "Unable to write to the file.\n";
            }

            sleep(10);
        }   
    }

    return 0;
}