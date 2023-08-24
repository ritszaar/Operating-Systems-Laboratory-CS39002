#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>

void signal_handler(int signo) {
    if (signo == SIGINT) {
        printf("I am unstoppable!\n");
    }
}

int main(int argc, char * argv[]) {

    signal(SIGINT, signal_handler);

    int shouldExit = 0;

    char c;

    while (!shouldExit) {
        scanf("%c", &c); getchar();
        if (!isupper(c) && !islower(c)) {
            printf("Do you speak-a my language?\n");
        } else if (c == 'x') {
            printf("Valar Morghulis\n");
            shouldExit = 1;
        } else {
            printf("%c\n", c);
        }
    }

    return 0;
}