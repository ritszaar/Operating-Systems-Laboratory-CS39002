#include<stdlib.h>
#include<unistd.h>
#include<iostream>
#include "structs.h"

extern int N;
extern sem_t roomAvailabilitySemaphore;
extern sem_t roomCleanSemaphore;
extern sem_t roomAllocationSemaphore;
extern sem_t printSemaphore;
extern vector<room*>roomlist;
extern vector<pthread_mutex_t>cleanerMutex;
extern vector<sem_t>sleepSemaphore;
extern int dirty_rooms;
extern bool being_cleaned;
extern int curr_guests;
extern pthread_cond_t cleanerCondition;

void* cleaner_thread(void* args)
{
    int cleaner_id=*(int*)args-1;
    while(1)
    {
        pthread_mutex_lock(&cleanerMutex[cleaner_id]);
        while(being_cleaned==0)
        {
            pthread_cond_wait(&cleanerCondition,&cleanerMutex[cleaner_id]);
        }
        pthread_mutex_unlock(&cleanerMutex[cleaner_id]);
        int clean_time=0;
        int flag=0;
        if(dirty_rooms==0)
            continue;
        sem_wait(&roomCleanSemaphore);
        for(int i=0;i<N;i++)
        {
            if(roomlist[i]->prev_occupancy==2)
            {
                sem_wait(&printSemaphore);
                cout<<"Cleaner "<<cleaner_id+1<<" cleaning room "<<i+1<<endl;
                sem_post(&printSemaphore);

                roomlist[i]->availability=true;
                roomlist[i]->prev_occupancy=0;
                roomlist[i]->guest_id=-1;
                clean_time=roomlist[i]->occupied_time;
                roomlist[i]->occupied_time=0;
                flag=1;
                break;
            }
        }
        sem_post(&roomCleanSemaphore);
        if(flag==0)
            continue;

        sleep(clean_time);

        sem_wait(&printSemaphore);
        cout<<"Cleaning complete by cleaner "<<cleaner_id+1<<endl;
        sem_post(&printSemaphore);

        dirty_rooms--;
        if(being_cleaned==1&&dirty_rooms==0)
        {
            curr_guests=0;
            being_cleaned=0;
            sem_wait(&roomAllocationSemaphore);
            for(int i=0;i<2*N;i++)
                sem_post(&roomAvailabilitySemaphore);
            sem_post(&roomAllocationSemaphore);
        }
    }
    return NULL;
}