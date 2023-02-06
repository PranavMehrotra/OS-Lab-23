#include "exec_job.h"
#include <iostream>
using namespace std;
extern std::set<int> fg_procs,bg_run_procs,bg_stop_procs;
extern void sigchld_blocker(int);
extern int foreground_pgid;
char buf[MAX_RES_LEN];
// extern void exec_proc(process * p, int infd, int outfd, int background);
// execute the given job (list of processes)
void exec_job(process * job , int n_proc , int background){
    
    foreground_pgid = 0;
    // setup pipes between processes
    int connect_fd = 0;             // output of previous pipe (STDIN for first process)
    for(int i=0;i<n_proc;i++){
        int infd = connect_fd;
        int outfd = 1;
        int testfd;
        if(i < n_proc-1){           // no pipes case automatically handled
            int fd[2];
            if(pipe(fd) < 0){
                perror("pipe");
                exit(0);
            }
            outfd = fd[1];
            connect_fd = fd[0];
        }
        if(strcmp(job[i].args[0],"lsof")==0){
            int fd[2];
            if(pipe(fd) < 0){
                perror("pipe");
                exit(0);
            }
            outfd = fd[1];
            testfd = fd[0];
        }
        if(strcmp(job[i].args[0] , "cd") == 0){
            if(job[i].n_args == 1){
                if(chdir(getenv("HOME")) < 0){
                    perror("chdir");
                    // exit(0);
                    continue;
                }
                // Getcwd to get the current working directory
                char cwd[1024];
                if(getcwd(cwd , sizeof(cwd)) != NULL){
                    printf("Current working directory: %s\n" , cwd);
                }
                else{
                    perror("getcwd");
                    // exit(0);
                    continue;
                }
            }
            else if(job[i].n_args > 2)
                printf("Too many arguments to cd\n");
            else{
                // printf("cd %s  \t %d\n",job[i].args[1],job[i].n_args);
                if(chdir(job[i].args[1]) < 0){
                    perror("chdir");
                    // exit(0);
                    continue;
                }
                // Getcwd to get the current working directory
                char cwd[1024];
                if(getcwd(cwd , sizeof(cwd)) != NULL){
                    printf("Current working directory: %s\n" , cwd);
                }
                else{
                    perror("getcwd");
                    // exit(0);
                    continue;
                }
            }
        }
        else 
            exec_proc(&job[i] , infd , outfd,background);

        // printf("Executed: %s\n",job[i].args[0]);
        if(strcmp(job[i].args[0],"lsof")==0){
            int n = read(testfd , buf , MAX_RES_LEN);
            buf[n] = '\0';
            printf("%s\n",buf);
            if(n <= 1)
                printf("No such process\n");
            else{
                printf("Do you want to kill all the processes using the file (yes/no)? ");
                // char *ans=(char *)malloc(10*sizeof(char));
                // scanf("%s",ans);
                // cin>>ans;
                // if(strcmp(ans,"y") == 0){
                    char * token = strtok(buf , " ");
                    while(token != NULL){
                        printf("hell1%shell2\n",token);
                        // int pid = atoi(token);
                        // kill(pid , SIGKILL);
                        token = strtok(NULL , " ");
                    }
                // }
            }
        }
    }
    for(int i=0;i<n_proc;i++){
        for(int j=0;j<job[i].n_args;j++){
            free(job[i].args[j]);
        }
        free(job[i].args);
    }
    free(job);
    if(!background){
        while(!fg_procs.empty());
        tcsetpgrp(STDIN_FILENO , getpid());
    }
    //foreground_pgid = 0;
    
    return;
}

void exec_proc(process * p, int infd, int outfd,int background){    // execute process
    sigchld_blocker(SIG_BLOCK);
    int c_pid = fork();
    if(c_pid < 0){
        perror("fork");
        exit(0);
    }
    else if(c_pid == 0){        // child
        redirect(p , infd , outfd);
        // Handling pwd command
        if(strcmp(p->args[0] , "pwd") == 0){
            char * cwd = (char *) malloc(1024 * sizeof(char));
            if(getcwd(cwd , 1024) == NULL){
                perror("getcwd");
                exit(0);
            }
            printf("%s\n" , cwd);
            free(cwd);
            exit(0);
        }
        sigchld_blocker(SIG_UNBLOCK);
        setpgid(getpid() , foreground_pgid);

        execvp(p->args[0],p->args);
        perror("execvp");
        exit(0);
    }
    else{
        if(!background)fg_procs.insert(c_pid);
        else bg_run_procs.insert(c_pid);
        if(foreground_pgid == 0){
            foreground_pgid = c_pid;
            if(!background) tcsetpgrp(STDIN_FILENO , foreground_pgid);
        }
        sigchld_blocker(SIG_UNBLOCK);
        close(outfd);

        infd = open("/dev/tty", O_RDONLY);
        if (infd == -1) {
            perror("open");
            return;
        }
        close(STDOUT_FILENO);
        outfd = open("/dev/tty", O_WRONLY);
        if (outfd == -1) {
            perror("open");
            return;
        }
    }
    return;
}

void redirect(process * proc , int infd , int outfd){

    char ** final_args = NULL;
    int final_nargs = proc->n_args;

    for(int i=0;i<proc->n_args;i++){
        if(strcmp(proc->args[i] , "<")==0){
            i++;
            final_nargs -= 2;
        }
        else if(strcmp(proc->args[i] , ">")==0){
            i++;
            final_nargs -= 2;
        }
    }

    final_args = (char **) malloc((final_nargs+1) * sizeof(char *));    // free after execvp in exec_proc
    final_nargs = 0;
    
    // check for explicit redirects to specific file names AFTER (possible) piping
    // this is the behavior that actual shells express. Pipe redirects have LESS priority over file redirects
    for(int i=0;i<proc->n_args;i++){

        // check for input redirect
        if(strcmp(proc->args[i] , "<") == 0){
            i++;            // assuming next arg as input file path
            if(infd != STDIN_FILENO)
                close(infd);
            if((infd = open(proc->args[i], O_RDONLY)) < 0){      // ## CHECK permissions!
                perror("open");
                exit(0);
            }
            free(proc->args[i-1]);
            free(proc->args[i]);
        }

        // check for output redirect
        else if(strcmp(proc->args[i] , ">") == 0){
            i++;            // assuming next arg as output file path
            if(outfd != STDOUT_FILENO)
                close(outfd);
            if((outfd = open(proc->args[i] , O_WRONLY | O_TRUNC | O_CREAT , 0666)) < 0){        // ## CHECK permissions!
                perror("open");
                exit(0);
            }
            free(proc->args[i-1]);
            free(proc->args[i]);
        }

        else{
            final_args[final_nargs++] = strdup(proc->args[i]);      // else, make a deep copy of the arguments
            free(proc->args[i]);        // free the original argument (if not a redirect)
        }
    }


    final_args[final_nargs] = NULL;         // argument list end sentinel (for execvp's use)
    int k=0;
    while(final_args[k]!=NULL){
        proc->args[k] = final_args[k];
        k++;
    }
    free(final_args);
    proc->args[k] = NULL;
    proc->n_args = final_nargs;
    k=0;
    // free(final_args);
    //printf("\t\tFinal args: ");
    while(proc->args[k]!=NULL){
        // proc->args[k] = final_args[k];
        //printf("%s " , proc->args[k]);
        k++;
    }
    //printf("\n");
    if(infd != STDIN_FILENO){
        dup2(infd , STDIN_FILENO);
        close(infd);
    }
    if(outfd != STDOUT_FILENO){
        dup2(outfd , STDOUT_FILENO);
        close(outfd);
    }
    return;
}
