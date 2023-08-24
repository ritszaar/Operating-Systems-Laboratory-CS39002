#include<iostream>
#include<pthread.h>
#include<stdlib.h>
#include "structs.h"

extern void* guest_thread(void* args);
extern void* cleaner_thread(void* args);
int N,X,Y;
int curr_guests;
vector<room*>roomlist;
vector<int>priority;
sem_t roomAvailabilitySemaphore;
sem_t roomAllocationSemaphore;
sem_t roomCleanSemaphore;
sem_t printSemaphore;
vector<sem_t>sleepSemaphore;
vector<pthread_mutex_t>cleanerMutex;
pthread_cond_t cleanerCondition;
int occupancies;
int dirty_rooms;
bool being_cleaned;

int main()
{
    srand(time(NULL));
    cout<<"Enter the number of rooms in the hotel: ";
    cin>>N;
    cout<<"Enter the number of cleaning staff: ";
    cin>>X;
    cout<<"Enter the number of guests: ";
    cin>>Y;

    cleanerMutex.resize(X);
    sleepSemaphore.resize(Y);
    
    for(int i=0;i<X;i++)
        pthread_mutex_init(&cleanerMutex[i],NULL);

    sem_init(&roomAvailabilitySemaphore,0,2*N);
    sem_init(&roomAllocationSemaphore,0,1);
    sem_init(&roomCleanSemaphore,0,1);
    sem_init(&printSemaphore,0,1);

    for(int i=0;i<Y;i++)
        sem_init(&sleepSemaphore[i],0,0);

    priority.resize(Y);
    for(int i=0;i<Y;i++)
        priority[i]=rand()%Y+1;

    curr_guests=0;
    occupancies=0;

    for(int i=0;i<N;i++)
    {
        room* addroom=new room();
        addroom->roomno=i+1;
        addroom->availability=true;
        addroom->prev_occupancy=0;
        addroom->guest_id=-1;
        addroom->occupied_time=0;
        roomlist.push_back(addroom);
    }
    
    vector<pthread_t>cleaners(X);
    vector<pthread_t>guests(Y);

    vector<int>guest_ids(Y);
    vector<int>cleaner_ids(Y);
    for(int i=0;i<Y;i++)
    {
        guest_ids[i]=i+1;
        pthread_create(&guests[i],NULL,&guest_thread,(void*)&guest_ids[i]);
    }
    for(int i=0;i<X;i++)
    {
        cleaner_ids[i]=i+1;
        pthread_create(&cleaners[i],NULL,&cleaner_thread,(void*)&cleaner_ids[i]);
    }
    
    for(int i=0;i<Y;i++)
        pthread_join(guests[i],NULL);
    for(int i=0;i<X;i++)
        pthread_join(cleaners[i],NULL);

    sem_destroy(&roomAvailabilitySemaphore);
    sem_destroy(&roomAllocationSemaphore);
    sem_destroy(&roomCleanSemaphore);
    sem_destroy(&printSemaphore);

    for(int i=0;i<Y;i++)
        sem_destroy(&sleepSemaphore[i]);
    
    for(int i=0;i<N;i++)
        free(roomlist[i]);
    
    for(int i=0;i<X;i++)
        pthread_mutex_destroy(&cleanerMutex[i]);

    return 0;
    
}