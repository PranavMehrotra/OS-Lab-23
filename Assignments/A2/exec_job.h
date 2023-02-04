#ifndef _EXEC_JOB_H
#define _EXEC_JOB_H
#define MAX_ARGS 256

#include <stdlib.h>
#include <unistd.h>
#include "process.h"
#include <sys/wait.h>
#include "avl_tree.h"

extern struct Node *fg_procs;

void exec_job(process * job , int n_proc , int background);

void exec_proc(process * p, int infd, int outfd, int background);

void redirect(process * proc , int infd , int outfd);
#endif