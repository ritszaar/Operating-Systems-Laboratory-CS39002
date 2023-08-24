#pragma once

#include<deque>
#include<iostream>
#include<fstream>
#include<string>

using namespace std;

#define HIST_SZ 1000

void getHistory();
void addToHistory(string);
int showHistory(int,int);
void saveHistory();