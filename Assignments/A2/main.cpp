#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <set>
#include <sys/signal.h>
#include <iostream>
#include <termios.h>

#include "parse.h" 
#include "exec_job.h"
#include "history.h"

using namespace std;
#define MAX_COMMANDS 1000

int foreground_pgid;
set <int> fg_procs,bg_run_procs,bg_stop_procs;

void sigint_handler(int signum){
    signal(SIGINT,sigint_handler);
    // printf("Inside signal handler for Ctrl-C\n");
    for(auto &it:fg_procs){
        kill(it,SIGINT);
    }
    printf("\nEnter the command: ");
    return;
}

void sigchild_handler(int signum){
    signal(SIGCHLD,sigchild_handler);
    while(1){
        int status;
        int cpid = waitpid(-1,&status,WUNTRACED|WNOHANG|WCONTINUED); //WCONTINUED check
        if(cpid<=0)break;
        if(fg_procs.find(cpid)!=fg_procs.end()){
            fg_procs.erase(cpid);
            if(WIFSTOPPED(status))bg_stop_procs.insert(cpid);
        }
        else if(bg_run_procs.find(cpid)!=bg_run_procs.end()){
            bg_run_procs.erase(cpid);
            if(WIFSTOPPED(status))bg_stop_procs.insert(cpid);
        }
    
    }
    
}

void sigchld_blocker(int signal_state){
    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set , SIGCHLD);
    sigprocmask(signal_state , &signal_set , NULL);
}

int main(){
    fg_procs.clear();
    bg_run_procs.clear();
    bg_stop_procs.clear();
    //signal(SIGINT,sigint_handler);
    signal(SIGCHLD,sigchild_handler);
    signal(SIGTTOU,SIG_IGN);
    foreground_pgid = 0;
    vector <string> history;
    int historyIndex = 0;
    string currentLine = "";

    // rl_bind_key('1', rl_beg_of_line);
    // rl_bind_key('9', rl_end_of_line);
    
    while(1){
        currentLine.clear();
        getHistory(history,historyIndex,currentLine);
        char *line = (char *)malloc((currentLine.size() + 1)*sizeof(char));
        strcpy(line,currentLine.c_str());
        if(strcmp(line,"exit")==0)break;
        process *job;
        int n_proc;
        int background = 0;

        job = parse(line,&n_proc,&background);
        exec_job(job,n_proc,background);
    }
}