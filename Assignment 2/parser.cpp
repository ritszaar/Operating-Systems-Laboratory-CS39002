#include "parser.h"

int get_char_count(const string& s, char c) {
    int count = 0;
    for (int i = 0; i < s.length(); s[i] == c ? count++ : 0, i++);
    return count;
}

int get_command_count(const string& command) {
    return get_char_count(command, '|') + 1;
}

void remove_extra_spaces(string& s) {
    int spaces = 0;
    string new_s;
    int i = 0, j = 0;
    while (i < s.length()) {
        for (; s[i] == ' '; i++);
        for (; i < s.length() && s[i] != ' '; i++, j++) new_s += s[i];
        for (; s[i] == ' '; i++);
        if (i < s.length()) {
            new_s += ' '; j++;
        }
    }

    s = new_s;
}

vector<Command> get_commands(const string& user_input) {
    int command_count = get_command_count(user_input); 
    stringstream user_input_stream(user_input);   
    vector<Command> commands(command_count);

    for (int i = 0; i < command_count; i++) {
        Command command;

        string raw_command;
        getline(user_input_stream, raw_command, '|');
        remove_extra_spaces(raw_command);

        int state = 0, j = 0, k = 0;
        while (true) {
            for (; raw_command[k] != ' ' && k < raw_command.length(); k++);
            if (raw_command[j] != '<' && raw_command[j] != '>') {
                if (state == 0) {
                    command.args.push_back(raw_command.substr(j, k - j));
                } else if (state == 1) {
                    command.fin = raw_command.substr(j, k - j);
                    state = 0;
                } else {
                    command.fout = raw_command.substr(j, k - j);
                    state = 0;
                }
            } else if (raw_command[j] == '<') {
                state = 1;
            } else if (raw_command[j] == '>') {
                state = 2;
            } 

            if (k == raw_command.length()) {
                break;
            } j = ++k;
        }
        
        command.args = expand_wildcards(command.args);
        
        commands[i] = command;
    } return commands;
}
