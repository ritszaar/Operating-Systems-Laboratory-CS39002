#ifndef STRUCTS_H
#define STRUCTS_H
#include<vector>
#include<semaphore.h>
#include<time.h>
using namespace std;

struct room{
    int roomno;
    bool availability;
    int prev_occupancy;
    int guest_id;
    int occupied_time;
};

#endif