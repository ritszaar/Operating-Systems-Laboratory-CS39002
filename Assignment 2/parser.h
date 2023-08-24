#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include "command.h"
#include "wildcard.h"

using namespace std;

int get_char_count(const string&, char);
int get_command_count(const string&);
void remove_extra_spaces(string&);
vector<Command> get_commands(const string&);
