#include<stdlib.h>
#include<unistd.h>
#include<iostream>
#include "structs.h"

extern int N;
extern vector<int>priority;
extern vector<room*>roomlist;
extern sem_t roomAvailabilitySemaphore;
extern sem_t roomAllocationSemaphore;
extern sem_t printSemaphore;
extern vector<sem_t>sleepSemaphore;
extern pthread_cond_t cleanerCondition;
extern int curr_guests;
extern int occupancies;
extern int dirty_rooms;
extern bool being_cleaned;

void allotroom(int& room_allotted,int& vacated,int guest_id)
{
    if(curr_guests<N)
    {
        for(int i=0;i<N;i++)
        {
            if(roomlist[i]->availability==true)
            {
                if(roomlist[i]->prev_occupancy==0)
                {
                    room_allotted=i;
                    break;
                }
                else if(roomlist[i]->prev_occupancy==1)
                    room_allotted=i;
            }
        }
    }
    else if(curr_guests==N)
    {
        int max_lesser_priority=0;
        for(int i=0;i<N;i++)
        {
            int guest_priority=priority[roomlist[i]->guest_id];
            if(roomlist[i]->prev_occupancy==1&&guest_priority>max_lesser_priority&&guest_priority<priority[guest_id])
            {
                max_lesser_priority=priority[roomlist[i]->guest_id];
                room_allotted=i;
            }
        }
        if(room_allotted!=-1)
        {
            vacated=1;
            sem_wait(&printSemaphore);
            cout<<"Due to priority: Vacated room "<<room_allotted+1<<" by guest: "<<roomlist[room_allotted]->guest_id+1<<endl;
            sem_post(&printSemaphore);

            sem_post(&sleepSemaphore[roomlist[room_allotted]->guest_id]);
        }
    }
}

void* guest_thread(void* args)
{
    while(1)
    {
        int guest_id=*(int*)args-1;
        int sleep_time=rand()%11+10;
        int stay_time=0;
        int flag=0;
        int vacated=0;
        int roomno;

        sem_wait(&roomAvailabilitySemaphore);

        sleep(sleep_time);
        int room_allotted=-1;
        sem_wait(&roomAllocationSemaphore);

        sem_wait(&printSemaphore);
        cout<<"\nStarting room allocation for guest: "<<guest_id+1<<endl;
        sem_post(&printSemaphore);

        allotroom(room_allotted,vacated,guest_id);

        if(room_allotted!=-1)
        {
            stay_time=rand()%21+10;
            roomlist[room_allotted]->availability=false;
            roomlist[room_allotted]->prev_occupancy++;
            roomlist[room_allotted]->guest_id=guest_id;
            roomlist[room_allotted]->occupied_time+=stay_time;
            roomno=room_allotted+1;
            occupancies++;
            if(vacated==0)
                curr_guests++;

            sem_wait(&printSemaphore);
            cout<<"Allotted room "<<roomno<<" to guest: "<<guest_id+1<<endl;
            cout<<"Current guests = "<<curr_guests<<endl;
            cout<<"Occupancies = "<<occupancies<<endl;
            sem_post(&printSemaphore);

            if(occupancies==2*N)
            {
                dirty_rooms=N;
                being_cleaned=1;
                occupancies=0;
                for(int i=0;i<N;i++)
                {
                    if(roomlist[i]->availability==false)
                        sem_post(&sleepSemaphore[roomlist[i]->guest_id]);
                }
                pthread_cond_broadcast(&cleanerCondition);
                sem_post(&roomAllocationSemaphore);
                continue;
            }
            flag=1;
        }
        if(flag==0)
        {
            sem_wait(&printSemaphore);
            cout<<"Unable to allocate room to guest: "<<guest_id+1<<endl;
            sem_post(&printSemaphore);

            sem_post(&roomAvailabilitySemaphore);
        }
        sem_post(&roomAllocationSemaphore);
        if(flag==1)
        {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts); 
            ts.tv_sec += stay_time;

            if(sem_timedwait(&sleepSemaphore[guest_id],&ts)!=-1)
                continue;

            sem_wait(&roomAllocationSemaphore);
            if(roomlist[roomno-1]->availability==false&&roomlist[roomno-1]->guest_id==guest_id)
            {
                roomlist[roomno-1]->availability=true;
                roomlist[roomno-1]->guest_id=-1;
                if(being_cleaned==0)
                {
                    curr_guests--;
                    sem_wait(&printSemaphore);
                    cout<<"\nDue to timeout: Vacated room "<<roomno<<" by guest: "<<guest_id+1<<endl; 
                    cout<<"Current guests = "<<curr_guests<<endl;
                    sem_post(&printSemaphore);
                }
            }
            sem_post(&roomAllocationSemaphore);
        }
    }
    return NULL;
}