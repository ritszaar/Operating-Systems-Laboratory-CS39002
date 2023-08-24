#include "history.h"
#include <signal.h>
#include <unistd.h>
#include<readline/readline.h>

using namespace std;

extern deque<string>history;
extern int curr;

void getHistory()
{
    history.clear();
    ifstream hist_fin;
    string str;
    hist_fin.open(".shell_history");
    if(!hist_fin.is_open())
        return;
    while(getline(hist_fin,str))
        history.push_back(str);
    hist_fin.close();
}

void addToHistory(string str)
{
    if(history.size()==HIST_SZ)
        history.pop_front();
    history.push_back(str);
}
int showHistory(int count,int key)
{
    int pos=0;
    if(key==65&&curr>0)
        curr--;
    else if(key==66&&curr<history.size()-1)
        curr++;
    for(auto it:history)
    {
        if(pos==curr)
        {
            rl_replace_line(it.c_str(),0);
            return 0;
        }
        pos++;
    }
    return 0;
}
void saveHistory()
{
    ofstream hist_fout;
    hist_fout.open(".shell_history",ios::trunc);
    if(!hist_fout.is_open())
        exit(0);
    while(!history.empty())
    {
        string str=history.front();
        history.pop_front();
        hist_fout<<str<<endl;
    }
    hist_fout.close();
    exit(0);
}

// int main()
// {
    // signal(SIGINT,signal_handler);
    // getHistory();
    // addToHistory("abcd");
    // addToHistory("ehth");
    // addToHistory("arthrth");
    // addToHistory("wrogo");
    // addToHistory("lgrelbe");
    // addToHistory("krkbrebm");
    // addToHistory("arvrekb");
    // addToHistory("rvjreb");
    // addToHistory("rbrebm");
    // addToHistory("kkerbmmwe");
    // char* buf;
    // string str;
    // curr=history.size();
    // rl_command_func_t showHistory;
    // rl_bind_keyseq("\\e[A",showHistory);
    // rl_bind_keyseq("\\e[B",showHistory);
    // while(1)
    // {
    //     buf=readline(">> ");
    //     if(buf==nullptr)
    //         saveHistory();
    //     else if(strlen(buf)>0)
    //     {
    //         str=buf;
    //         addToHistory(str);
    //         curr=history.size();
    //     }
        // else if(buf==nullptr)
        // {
        //     saveHistory();
        // }
    // }
    // saveHistory();
    // return 0;
// }