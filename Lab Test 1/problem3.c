#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_ROW_SIZE 1024
#define MAX_COLUMN_SIZE 1024

int m, n;
int M[MAX_ROW_SIZE][MAX_COLUMN_SIZE];

pthread_mutex_t mutexS, mutexM;
pthread_cond_t aEmpty, aFull;
int * A, * B;
int aindex;

#define PROCESSED 0
#define UNPROCESSED 1

void swap(int * x, int * y) {
    int temp = *x;
    *x = *y;
    *y = temp;
}

void createRandomMatrix() {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            M[i][j] = rand() % 1000 + 1;
        }
    }

    printf("\nRandom matrix M is created\n");
}

void * order_routine(void * arg) {

    printf("\nI am order\n");
    int updates = 30;

    
    while (updates > 0) {
        int row = 0;
        pthread_mutex_lock(&mutexS);
        while (aindex <= 0) {
            pthread_cond_wait(&aEmpty, &mutexS);
        }
        pthread_mutex_unlock(&mutexS);

    
    aindex--;
    row = A[aindex];
    B[aindex] = PROCESSED;
    printf("\nOrder: detected updated element at row %d\n", row);
    

    pthread_mutex_lock(&mutexM);
    int before[1000], after[1000];
    for (int i = 0; i < n; i++) {
        before[i] = M[row][i];
        after[i] = before[i];
    }

    for (int i = 0; i < n - 1; i++) {
        int minIndex = i, min = after[i];
        for (int j = i + 1; j < n; j++) {
            if (after[j] < min) {
                min = after[j];
                minIndex = j;
            }
        } swap(after + i, after + minIndex);
    }

    printf("\nOrder: row %d is sorted now\n", row);
    printf("\nolder row %d: ", row);
    for (int i = 0; i < n; i++) {
        printf("%d ", before[i]);
    } printf("\n");

    printf("\nnew row %d: ", row);
    for (int i = 0; i < n; i++) {
        printf("%d ", after[i]);
    } printf("\n");
    pthread_mutex_unlock(&mutexM);
    pthread_cond_signal(&aFull);
    updates--;
    }

    return NULL;
}

void * chaos_routine(void * arg) {
    printf("\nI am chaos\n");

    int updates = 30;

    while (updates > 0) {
        int i = random() % m;
        int j = random() % n;
        M[i][j] = random() % 1000 + 1;

        printf("\nChaos: updated element at cell %d x %d with value %d\n", i, j, M[i][j]);

        A[aindex] = i; aindex++;
        B[i] = UNPROCESSED;

        pthread_cond_signal(&aEmpty);
        sleep(2);
        updates--;
    }    

    return NULL;
}

int main(int argc, char * argv[]) {

    printf("Enter m: ");
    scanf("%d", &m);
    printf("Enter n: ");
    scanf("%d", &n);

    createRandomMatrix();

    A = (int *) malloc(1000 * sizeof(int));
    B = (int *) malloc(1000 * sizeof(int));
    for (int i = 0; i < 1000; i++) {
        B[i] = PROCESSED;
    } aindex = 0;
    printf("\nShared arrays A and B are created\n");

    printf("\nMatrix M is given below:\n");
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", M[i][j]);
        } printf("\n");
    } 

    pthread_mutex_init(&mutexS, NULL);
    pthread_mutex_init(&mutexM, NULL);
    pthread_cond_init(&aFull, NULL);
    pthread_cond_init(&aEmpty, NULL);

    pthread_t oth[3], cth;

    for (int i = 0; i < 3; i++) {
        pthread_create(oth + i, NULL, &order_routine, NULL);        
    } pthread_create(&cth, NULL, &chaos_routine, NULL);

    for (int i = 0; i < 3; i++) {
        pthread_join(oth[i], NULL);
    } pthread_join(cth, NULL);
    

    return 0;
}