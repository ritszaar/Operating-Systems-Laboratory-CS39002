#pragma once

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "fsystem.h"

using namespace std;

bool contains_wildcard_character(const string&);
bool contains_tilde(const string&);
string expand_tilde(const string&);
bool wildcard_mathches(const string&, const string&);
void find_directories(vector<string>&, int, string, vector<string>&);
vector<string> get_directories(string);
vector<string> expand_wildcards(const vector<string>&);
