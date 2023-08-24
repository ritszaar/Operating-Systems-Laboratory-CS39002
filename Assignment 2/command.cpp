#include "command.h"

Command::Command() : fin_fd(STDIN_FILENO), fout_fd(STDOUT_FILENO), fin(""), fout("") {}

void Command::io_redirect() {
    if (fin != "") {
        fin_fd = open(fin.c_str(), O_RDONLY);
        if (fin_fd < 0) {
            perror("open");
            exit(1);
        }
        int ret = dup2(fin_fd, STDIN_FILENO);
        if (ret < 0) {
            perror("dup2");
            exit(1);
        }
    }

    if (fout != "") {
        fout_fd = open(fout.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fout_fd < 0) {
            perror("open");
            exit(1);
        }
        int ret = dup2(fout_fd, STDOUT_FILENO);
        if (ret < 0) {
            perror("dup2");
            exit(1);
        }
    }
}

int Command::execute() {
    if (args[0] == "cd") {
        string dir = args[1];
        for (int i = 2; i < (int) args.size(); i++) {
            dir += " " + args[i];
        }

        char cwd[1024];
        if (dir[0] != '~' && dir[0] != '/') {
            dir = getcwd(cwd, sizeof(cwd)) + ("/" + dir);
        } else if (dir[0] == '~') {
            string rest_dir = dir.substr(1, dir.length() - 1);
            dir = getpwuid(getuid())->pw_dir + rest_dir;
        }

        if(chdir(dir.c_str()) < 0)
        {
            perror("chdir error\n");
        }
        return 1;
    }

    if (args[0] == "pwd") {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        cout << cwd << "\n";
        return 1;
    }

    if (args[0] == "delep") {
        exec_delep(args[1]);
    }

    vector<char *> formatted_args;
    formatted_args.reserve(args.size() + 1);
    for(string & arg: args) {
        formatted_args.push_back((char *) arg.c_str());
    } formatted_args.push_back(nullptr); 

    return execvp(formatted_args[0], formatted_args.data());
}
