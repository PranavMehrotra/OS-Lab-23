#include "exec_job.h"

// extern void exec_proc(process * p, int infd, int outfd, int background);
// execute the given job (list of processes)
void exec_job(process * job , int n_proc , int background){
    
    // setup pipes between processes
    int connect_fd = 0;             // output of previous pipe (STDIN for first process)
    for(int i=0;i<n_proc;i++){
        int infd = connect_fd;
        int outfd = 1;
        if(i < n_proc-1){           // no pipes case automatically handled
            int fd[2];
            if(pipe(fd) < 0){
                perror("pipe");
                exit(0);
            }
            outfd = fd[1];
            connect_fd = fd[0];
        }
        exec_proc(&job[i] , infd , outfd);
        // printf("Executed: %s\n",job[i].args[0]);
    }

    // while(!background && n_proc){        // while some procs are running in the foreground, wait
    //     int proc_status;
    //     int child_pid;
    //     if((child_pid = waitpid(-1 , &proc_status , WNOHANG)) < 0){    // wait on all children, blocking, no flag set
    //         perror("waitpid");
    //         exit(0);
    //     }
    //     if(findNode(fg_procs , child_pid) != NULL){
    //         n_proc--;                           // child that exited was a foreground process, so decrement n_procs
    //         deleteNode(fg_procs , child_pid);   // and remove from set
    //     }
    // }
    while(!background && n_proc){
        int status;
        int cpid = waitpid(-1 , &status , WNOHANG);
        n_proc--;
    }
    return;
}

void exec_proc(process * p, int infd, int outfd){    // execute process
    int c_pid = fork();
    if(c_pid < 0){
        perror("fork");
        exit(0);
    }
    else if(c_pid == 0){        // child
        redirect(p , infd , outfd);
        // printf("Executing: %s",p->args[0]);
        execvp(p->args[0],p->args);
        perror("execvp");
        exit(0);
    }
    return;
}

void redirect(process * proc , int infd , int outfd){

    char ** final_args = NULL;
    int final_nargs = proc->n_args;

    // Wrong, will not work for cases like: grep -i "hello" > out.txt
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

    if(infd != STDIN_FILENO){
        dup2(infd , STDIN_FILENO);
        close(infd);
    }
    if(outfd != STDOUT_FILENO){
        dup2(outfd , STDOUT_FILENO);
        close(outfd);
    }

    final_args[final_nargs] = NULL;         // argument list end sentinel (for execvp's use)
    int k=0;
    while(final_args[k]!=NULL){
        proc->args[k] = final_args[k];
        k++;
    }
    proc->args[k] = NULL;
    proc->n_args = final_nargs;

    free(final_args);
    return;
}
