#include <iostream>
#include <unistd.h>
using namespace std;

#define N 5 
#define M 10 
#define T 120

signed main() {

    while (true) {
        for (int i = 0; i < N; i++) {
            if (fork() == 0) {
                printf("[generation 1] [child] pid %d from [parent] pid %d\n", getpid(), getppid());
                for (int i = 0; i < M; i++) {
                    if (fork() == 0) {
                        printf("[generation 2] [child] pid %d from [parent] pid %d\n",getpid(),getppid());
                        while (true) {}
                    }
                }

                while (true) {}
            }
        }

        sleep(T);
        printf("\n\n");
    }

    return 0;
}