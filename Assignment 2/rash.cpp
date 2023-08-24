#include "parser.h"
#include "command.h"
#include "pipeline.h"
#include "signal_handler.h"
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <termios.h>
#include "history.h"
#include "sb.h"
#include "delep.h"
 
deque<string>history;
vector<Pipeline*> pipeline_list;  // To store all the pipelines in the shell
map<pid_t, int> map_pipeline;              // Mapping process id to index in the vector

bool ctrlC = 0, ctrlZ = 0;  // Indicates whether the user has pressed Ctrl-C, Ctrl-Z, or Ctrl-D
pid_t fgpid = 0;                       // Foreground process group id
int curr;

void initialize_readline_configs() {
    rl_command_func_t showHistory;
    rl_getc_function = getc;
    rl_bind_keyseq("\\e[A",showHistory);
    rl_bind_keyseq("\\e[B",showHistory);
}
void C_handler(int signo) {
  if (signo == SIGINT) {
      ctrlC=1;
    // printf("\nYou pressed Ctrl+C\n");
    // memset(rl_line_buffer, 0, sizeof(rl_line_buffer));

    // printf("%s\n", rl_line_buffer);
    curr=history.size();
    cout<<'\n';
  }
}

void remove_trailing_spaces(string& s)
{
    string new_s = "";
    int n = s.size();
    int f = -1;
    int l = n;
    for(int i = 0; i<n; i++)
    {
        if(s[i] == ' ')
        f=i;
        else
        break;
    }
    f++;
    for(int i = n-1; i>=0; i--)
    {
        if(s[i] == ' ')
        l=i;
        else
        break;
    }
    l--;
    if(f <= l)
    {
        for(int i = f; i<=l; i++)
        new_s += s[i];
    }
    s = new_s;
} 

void exit_terminal() {
    saveHistory();
    exit(0);
}

void execute_cd(string arg) {
    remove_trailing_spaces(arg);
    if (arg == "") {
        perror("No directory specified");
    }
    if (chdir(arg.c_str()) < 0) {
        perror("chdir");
    }
}

signed main() {
 
    initialize_readline_configs();
    getHistory();
    curr=history.size();

    // Specify signal handlers

    struct sigaction action;
    action.sa_handler = Z_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGTSTP, &action, NULL);

    signal(SIGTTOU, SIG_IGN);
    signal(SIGINT, C_handler);
    signal(SIGCHLD, reapProcesses);

    while (1) {

        char prompt[1024] = {0};

        // for(int i=0;i<5;i++){
            getcwd(prompt, sizeof(prompt));
            char * user_input = nullptr;
            // printf("%s $ ", pwd);
            strcat(prompt, " $ ");


            struct termios old_tio, new_tio;
            signed char c;

            // Get the terminal settings for stdin
            tcgetattr(STDIN_FILENO, &old_tio);

            // We want to keep the old setting to restore them at the end
            new_tio = old_tio;

            // Disable canonical mode (bufered i/o) and local echo
            // new_tio.c_lflag &= (~ICANON & ~ECHO);
            new_tio.c_cc[VMIN] = 1;
            new_tio.c_cc[VTIME] = 0;

            // Set the new settings immediately
            tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);


            user_input = readline(prompt);

            // Restore the former settings
            tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

            if (user_input==nullptr) {
                if(ctrlC==1){
                    ctrlC=0;
                    continue;
                }
                else
                {
                    cout<<"\n";
                    exit_terminal();
                }
            }
            addToHistory((string)(user_input));
            curr=history.size();
            // cout << user_input << "\n";

            string user_input_c = user_input;            
            remove_trailing_spaces(user_input_c);

            bool bg = false; 

            if(user_input_c.back() == '&')
            {
                bg = true;
                user_input_c.pop_back();
            }

            vector<Command> commands = get_commands(user_input_c);

            Pipeline p(commands);
            if(bg)
            {
                p.background_process = true;
            }

            if(p.cmdlist[0].args[0] == "exit")
            {
                exit_terminal();
            }

            else if(p.cmdlist[0].args[0] == "cd")
            {
                string param = "";
                if(p.cmdlist[0].args.size() > 1) {
                    param = p.cmdlist[0].args[1];

                    if (p.cmdlist[0].args.size() > 2) {
                        for (int i = 2; i < (int) p.cmdlist[0].args.size(); i++) {
                            param += " " + p.cmdlist[0].args[i]; 
                        }

                        param.erase(0, 1);
                        param.erase(param.length() - 1, 1);
                    }
                    
                } execute_cd(param);
            } 

            else if (p.cmdlist[0].args[0] == "sb") {
                pid_t pid = stoi(p.cmdlist[0].args[1]);
                TrojanDetector trojan_detector; 
                trojan_detector.display_ancestors(pid);
                if (p.cmdlist[0].args.size() == 3 && p.cmdlist[0].args[2] == "-suggest") {
                    trojan_detector.find_malware(pid);
                }
            }

            else if (p.cmdlist[0].args[0] == "delep") {
                string filename = p.cmdlist[0].args[1];
                for (int i = 2; i < (int) p.cmdlist[0].args.size(); i++) {
                    filename += " " + p.cmdlist[0].args[i]; 
                }

                exec_delep(filename);
            }

            else
            p.execute();
            // cout<<"Command executed\n";
        }
    // }

    return 0;
}